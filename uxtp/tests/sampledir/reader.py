""" Implementation of the tp command for magtape,
to assist in getting things in and out of Unix V6

This module reads tape images then extracts files or
displays a directory listing.

Magnetic Tape images written by UnixV6 must be processed
by the rawtap program to extract the raw file from
the SimH tape image. This code will not deal with
files written with the DecTape simulator.

TP magtape format - works in 512 byte blocks:
block 0 -  bootstrap for the system
block 1 -  start of 62 blocks of directory entries
           entries are 32 bytes long
           maximum of 496 entries
block 63 - start of data files, each file is padded
           to fill up to the nearest

Directory entries - 32 16-bit words

char pathname[32] - file path null terminated - 31 used bytes
int  mode         - UNIX file mode
char uid          - user id
char gid          - group id
char unused1
char size0        - top 8 bits of file size
int  size1        - bottom 16 bits of file size
int  time0        - top 16 bits of modification time
int  time1        - bottom 16 bits of modification time
int  tapa         - block address of the file
char unused2[16]  - set to zero
int  checksum     - set so 'all the words in the block add to zero'

to which the processing in ReadTape adds:
pathstring        - Python string of the pathname
path              - Path object for the filename
size              - int of the file size
time              - int of the time
datetime          - datetime object of the time

If tp is run in the simulator, then times on files will be shown
in the local time of the UNIX kernel, by default EST. Listing the
directory using this code will show your local time.

"""

import sys
import os
import struct
from datetime import datetime
from pathlib import Path
import constants as const
from constants import get_using_format
from constants import to_32, block_to_bytes, blocksused

class Dirls():
    """ List tape contents
    """

    def __init__(self, tapefile:str, fileselect:list, verbose):
        """ Called with a tapefile, a list of files to select,
        and the verbose flag
        """

        self.verbose = verbose

        # add match count to the final table
        # if we have some selected files
        self.printmatched = False
        if fileselect:
            self.printmatched = True

        # get basic information from tape
        self.tape = ReadTape(tapefile, fileselect)

    def dirls(self) -> None:
        """ Print the directory """

        dlist = self.tape.filtered
        if not dlist:
            print('Files not found')
            return

        if not self.verbose:
            for dirent in dlist:
                print(dirent['pathstring'])
            self.footer(len(dlist))
            return

        heading = ['mode', 'uid', 'gid', 'tapa',
                       'size', 'date', 'time', 'name']
        lines = []
        for dirent in dlist:
            new = []
            new.append(self.visiblemode(dirent['mode']))
            new.append(dirent['uid'])
            new.append(dirent['gid'])
            new.append(dirent['tapea'])
            new.append(dirent['size'])
            new.append(dirent['datetime'].strftime('%y/%m/%d'))
            new.append(dirent['datetime'].strftime('%H:%M'))
            new.append(dirent['pathstring'])
            lines.append(new)

        hfmt = '{0:^9s} {1:^3s} {2:3s} {3:4^s} {4:>8s} {5:^8s} {6:>5s} {7:s}'
        print(hfmt.format(*heading))
        hfmt = '{0:9s} {1:^3d} {2:^3d} {3:4d} {4:8d} {5:8s} {6:5s} {7:s}'
        for line in lines:
            print(hfmt.format(*line))
        self.footer(len(dlist))

    def footer(self, matched:int) -> None:
        """ Print footer of listing a la tp """

        if self.printmatched:
            print(f'{matched: 4d} matched', file=sys.stderr)
        print(f'{self.tape.entryct: 4d} entries', file=sys.stderr)
        print(f'{self.tape.used: 4d} used', file=sys.stderr)
        print(f'{self.tape.last: 4d} last', file=sys.stderr)
        print('END', file=sys.stderr)

    def visiblemode(self, mode:int) -> str:
        """ make a visible representaton of the mode """

        mode = mode & 0o777
        rmask = 0o400
        wmask = 0o200
        xmask = 0o100
        out = []
        for _inx in range(3):
            rval = 'r' if (mode&rmask) != 0 else '-'
            wval = 'w' if (mode&wmask) != 0 else '-'
            xval = 'x' if (mode&xmask) != 0 else '-'
            out.append(rval + wval + xval)
            rmask = rmask >> 3
            wmask = wmask >> 3
            xmask = xmask >> 3
        return ''.join(out)

class Extract:
    """ Extract files from the tape """

    def __init__(self, tapefile:str, fileselect:list, verbose):
        """ Called with a tapefile, a list of files to select, and the verbose flag """

        self.verbose = verbose
        # get basic information from tape
        self.tape = ReadTape(tapefile, fileselect)

    def extract(self) -> None:
        """ Extract named files from the tape """

        dlist = self.tape.filtered
        if not dlist:
            print('Files not found')
            return

        for dirent in dlist:
            self.processfile(dirent)

    def processfile(self, dirent:dict) -> None:
        """ Extracts and writes a single file

        creates any directories it needs in the pathname
        will not overwrite any file that exists
        sets the mode, mtime and ctime of the file
        ignores uid and gid
        """

        contents = self.tape.extractfile(dirent['tapea'], dirent['size'])
        # now the exciting process of creating it
        thispath = dirent['path']
        # don't want to pollute root, so remove any leading /
        parts = thispath.parts
        if parts[0] == '/':
            parts = parts[1:]
            thispath = Path("/".join(parts))
        if thispath.exists():
            # don't overwrite files we have already
            print(f'{thispath} exists, ignoring')
            return

        if len(parts) > 1:
            # quick check to see if the directory exists
            if not thispath.parent.is_dir():
                # directory check
                runl = []
                for dname in parts[:-1]:
                    runl.append(dname)
                    runp = Path('/'.join(runl))
                    if not runp.is_dir():
                        if self.verbose:
                            print(f'mkdir {runp}')
                        runp.mkdir()
        if self.verbose:
            print(f'writing {thispath}')
        thispath.write_bytes(contents)
        thispath.chmod(dirent['mode']&0o777)
        os.utime(str(thispath), (dirent['time'], dirent['time']))

    def readfile(self, dirent:dict, fhandle) -> bytes:
        """ Read a file from the tape given a dirent entry

        truncates the file to the size given in the directory
        entry
        assume file is open, so we can read several files
        """

        fhandle.seek(block_to_bytes(dirent['tapea']))
        blocks = blocksused(dirent['size'])
        bytesneeded = block_to_bytes(blocks)
        startsat = block_to_bytes(dirent['tapea'])
        fhandle.seek(startsat)
        blks = fhandle.read(bytesneeded)
        return blks[:dirent['size']]


class ReadTape():
    """ Basics for reading tp formatted tape image

    Reads and creates the tape directory
    If there are any files to select
    will create a filtered set of files to process
    """

    def __init__(self, tapefile:str, fileselect:list):
        """ Establish some variables

        tapebytes - content of tape blocks
        dirblks  - raw blocks of the directory area
        tapedir  - set of processed directory items
        filtered - if there are any selections from
                   the command line in fileselect
                   these are applied to tapedir
                   to make a working list.
                   If not is the tapedir
        """

        self.tapebytes = self.loadtape(tapefile)
        self.dirblks = self.loaddir()
        # parse them into a list of dicts
        self.tapedir = self.parsedir(self.dirblks)
        self.filtered = self.tapedir
        if fileselect:
            self.filtered = self.filter(fileselect)

    def loadtape(self, tapefile:str) -> bytes:
        """ Load a tape into memory
        Two formats: raw block format and SIMH tape format
        SIMH format is assumed if file suffix is .tap
        otherwise block format is assumed
        """

        tapef = Path(tapefile)
        if not tapef.exists():
            print("Cannot find {tapefile}")
            sys.exit(1)
        contents = tapef.read_bytes()
        if tapef.suffix == '.tap':
            contents = self.tapeunblock(contents)
            if not contents:
                print("No data found")
        return contents

    def tapeunblock(self, contents:bytes) -> bytes:
        """ Blocks are <twc>DIRSIZE bytes<twc>
        End of tape is <tef><tef>
        """
        outbytes = bytearray(len(contents))

        in_offset = 0
        out_offset = 0
        while in_offset < len(contents):
            if self.is_twc(contents, in_offset):
                in_offset += 4
                for _inx in range(const.BLKSIZ):
                    outbytes[out_offset] = contents[in_offset]
                    out_offset += 1
                    in_offset += 1
                if self.is_twc(contents, in_offset):
                    in_offset += 4
            elif self.is_tef(contents, in_offset):
                in_offset += 4
                # tape may have more than one file
                # but we'll ignore that
                break
            else:
                print(f"Unknown value @ {in_offset}")
                sys.exit(1)
        return outbytes

    def is_tef(self, contents:bytes, offset:int) -> bool:
        """ Look for a mark at the current position """

        return self.markmatch(contents, offset, const.TEF)

    def is_twc(self, contents:bytes, offset:int) -> bool:
        """ is the mark a tef mark ? """

        return self.markmatch(contents, offset, const.TWC)

    def markmatch(self, contents:bytes, offset:int, mark:list) -> bool:
        """ Match the tape image with the mark """

        lookup = struct.unpack_from('4c', contents, offset)
        for inx in range(4):
            if lookup[inx] != mark[inx]:
                return False
        return True

    def loaddir(self) -> bytes:
        """ Load directory blocks from tape"""

        startat = block_to_bytes(const.DICTSTART)
        endsat = startat + block_to_bytes(const.DICTSIZ)
        return self.tapebytes[startat:endsat]

    def parsedir(self, dblks:bytes) -> list[dict]:
        """ Parse the blocks to generate the dictionary """

        outlist = []
        offset = 0

        # values used in footer()
        self.entryct = 0
        self.used = 0
        self.last = 0

        while offset < len(dblks):
            # The size of processing is dictated by the format in constants.py
            # Process the directory entry as a set of 16bit words first
            # to skip empty entries and evaluate the checksum
            blkaswords, blkoff = get_using_format(dblks, const.CKSUM_FMT, offset)

            # the spec says that if word[0] of the block
            # is zero then you should ignore it

            body = blkaswords['body']
            if body[0] == 0:
                offset = blkoff
                continue

            # validate on checksum
            total = 0
            for word in body:
                total = (total + word) & 0xffff
            if (total & 0xffff) != 0:
                print(f'Checksum failure on dir entry {self.entryct} - ignoring')
                continue

            # now read and decode the contents
            dirent, offset = get_using_format(dblks, const.DIRENT_FMT, offset)

            # Supply some python values
            # Pathname
            pname = b''.join(dirent['pathname'])
            namestr = pname.decode('utf-8')
            dirent['pathstring'] = namestr.rstrip('\0')

            # Path value for matching
            dirent['path'] = Path(dirent['pathstring'])

            # size
            dirent['size'] = to_32([dirent['size0'], dirent['size1']])
            sizeinblocks = blocksused(dirent['size'])
            self.used = self.used + sizeinblocks
            self.last = dirent['tapea'] + sizeinblocks - 1

            # time and datetime
            timeas32 = to_32([dirent['time0'], dirent['time1']])
            dirent['time'] = timeas32
            dirent['datetime'] = datetime.fromtimestamp(timeas32)
            outlist.append(dirent)

            self.entryct += 1

        return outlist

    def filter(self, fileselect:list[str]) -> list[dict]:
        """ Select files from the directory if they match filelist

        This uses the match function in pathlib to look for any * values
        not sure that this is quite what's needed
        """

        tapedir = self.tapedir
        filtered = []

        for dirent in tapedir:
            match = False
            for pattern in fileselect:
                if dirent['path'].match(pattern):
                    match = True
                    break
            if match:
                filtered.append(dirent)
        return filtered

    def extractfile(self, bno:int, filesize:int) -> bytes:
        """ Extract a file from the stored tape
        bno is the block number
        size is filesize in bytes
        """

        startat = block_to_bytes(bno)
        endsat = startat + filesize
        return self.tapebytes[startat:endsat]

if __name__ == '__main__':
    pass
