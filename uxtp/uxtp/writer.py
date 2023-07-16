""" Implementation of the tp command for magtape,
to assist in getting things in and out of Unix V6

Writer creates a tp format tape image, with a zero
boot block, a directory of the files that are present
and the data for the files.

If output file ends in .tap use SimH Tape format in blocks,
which can be read on simulated PDP from a magtape interface
otherwise writes a raw file.

See reader.py for a description of tape formats

Peter Collinson
July 2023

"""

import sys
import os
from typing import Any, List
from pathlib import Path
from datetime import datetime
import constants as const
from constants import blocksused, put_using_format, from_32, block_to_bytes

class Writer:
    """ Write files into a new TP tape volume """

    # pylint: disable=too-many-instance-attributes

    def __init__(self, tapefile:str,
                     fileselect:list,
                     verbose:bool,
                     owner:list,
                     shellscript:str):
        """ Called with a tapefile, a list of files to load,
        verbose flag, owner[uid,gid] and shellfile name

      """

        # pylint: disable=too-many-arguments

        self.verbose = verbose
        self.tapefile = tapefile
        self.fileselect = fileselect

        self.tapepath = Path(tapefile)
        # basic check on name
        if self.tapepath.suffix not in ('.tap', '.dat', '.dec'):
            print(f"{self.tapefile} should end in .tap, .dec or .dat")
            sys.exit(1)

        if self.tapepath.exists():
            if  not self.tapepath.is_file():
                print(f"{self.tapefile} is exists and isn't a file\n")
                sys.exit(1)
            else:
                self.tapepath.unlink()

        # set DictSiz
        const.DictSiz = const.dirsize(self.tapepath.suffix)

        self.simh_format = False
        # .dec and .tap are not tape blocked
        if self.tapepath.suffix == '.tap':
            self.simh_format = True

        if owner:
            self.owner = owner
        else:
            self.owner = [const.DEF_UID, const.DEF_GID]

        self.shellscript = shellscript

        # list of directories found in the filescan
        # can be used to add file to make
        self.dirlist:List[str] = []

    def writer(self) -> None:
        """ Sequence the writing process """

        # organise a list of files, possibly moderated by
        # fileselect
        files = self.getfileinfo()

        # files is a list of dicts, indexes are
        # fname - stored file name
        # fpath - Path object for the file
        # inline - used when contents supplied by code
        # mode - the mode of the file -
        #        only file permissions
        # time - timestamp of the mtime of the file
        # size - the size of the file
        # tapeblks = the number of blocks the file will occupy
        # Other values are added in examine

        # create the directory image
        dirimage = bytearray(block_to_bytes(const.DictSiz))
        offset = 0
        for dirent in files:
            offset = self.installentry(dirent, dirimage, offset)

        # new tape image
        # files will be added to the end of this
        tapeimage = bytearray()

        # add a zero bootstrap block
        boot = bytearray(const.BLKSIZ)
        tapeimage = self.blocked_save(tapeimage, boot)

        # Now add the directory
        tapeimage = self.blocked_save(tapeimage, dirimage)

        # That done, read and load the files
        # idea is to add files using concatenate
        # which hopefully should be faster than copying data
        # byte by byte
        for dirent in files:
            if self.verbose:
                print(f"Adding {dirent['fname']}")
            if 'inline' in dirent:
                contents = dirent['inline']
            else:
                contents = dirent['fpath'].read_bytes()
                # rough check that the size remains the same
                if len(contents) != dirent['size']:
                    print(f"Size of {dirent['fname']} has changed, abandoning")
                    sys.exit(1)
            tapeimage = self.blocked_save(tapeimage, contents)

        # add final eof marks
        if self.simh_format:
            tapeimage = tapeimage + self.mark(const.TEF)
            # Don't add end of tape marker
            # tapeimage = tapeimage + self.mark(const.TEF)

        self.tapepath.write_bytes(tapeimage)

    def blocked_save(self, tapeimage:bytearray,
                         data:bytearray) -> bytearray:
        """ Create a portion of the output file from
        data padding the data upto a multiple of blocks.
        If using simh format, will split data into
        BLKSIZ blocks and surround them with file marks

        Returns: bytearray of the output image
        """

        # check that the data is a set of BLKSIZ blocks
        # and if not add zero padding
        padding_needed = len(data)&const.BLKMSK
        if padding_needed > 0:
            data = data + bytearray(512 - padding_needed)

        if not self.simh_format:
            # no blocking needed
            tapeimage = tapeimage + data
        else:
            offset = 0
            while offset < len(data):
                tapeimage = tapeimage + self.mark(const.TWC)
                blk = data[offset:offset + const.BLKSIZ]
                offset += const.BLKSIZ
                tapeimage = tapeimage + blk
                tapeimage = tapeimage + self.mark(const.TWC)
        return tapeimage

    def mark(self, marktype: list) -> bytes:
        """ Create a mark """

        return b''.join(marktype)

    def installentry(self, dirent:dict,
                    tapeimage:bytearray,
                    offset:int) -> int:
        """ Install file entry as a binary image onto the tapeimage
        directory.

        Some of the needed information was found when the file
        was found, add further information needed by the tp
        directory. Compute and add checksum to the entry.

        Returns: offset of next position in tapeimage
        """

        # create values in dirent that can be inserted
        # into the image
        # filename needs to be 32 bytes
        # pathlist needs to be a list of single byte values

        pathlist = [bytes(byt, "ascii")
                        for byt in dirent['fname']]
        extend = 32 - len(pathlist)
        if extend > 0:
            filler = [b'\0' for _i in range(extend)]
            dirent['pathname'] = pathlist + filler
        else:
            dirent['pathname'] = pathlist
        # Owner
        dirent['uid'] = self.owner[0]
        dirent['gid'] = self.owner[1]
        # size
        dirent['size0'], dirent['size1'] = from_32(dirent['size'])
        dirent['time0'], dirent['time1'] = from_32(dirent['time'])
        # empty data entries
        dirent['spare'] = 0
        dirent['unused'] = [0, 0, 0, 0, 0, 0, 0, 0]
        dirent['cksum'] = 0

        # insert it into a temporary area so we can organise the cksum
        tmp = bytearray(64)
        put_using_format(dirent, const.DIRENT_FMT, tmp, 0)
        blkaswords, _blkoffset = const.get_using_format(tmp, const.CKSUM_FMT, 0)
        body = blkaswords['body']
        # calculate cksum
        total = 0
        # there are 32 words, sum 31 of them
        for word in body[:31]:
            total = (total + word) & 0xffff
        cksum = -total & 0xffff
        assert (cksum + total)&0xffff == 0
        dirent['cksum'] = cksum

        # now insert into tape image
        _blks, offset =  put_using_format(dirent, const.DIRENT_FMT, tapeimage, offset)
        return offset

    #
    #  Directory creation code
    #

    def getfileinfo(self) -> list[dict[str, Any]]:
        """ Find files to be written and create a list of
        dicts containing a dict of values

        Returns: list of dicts with values from the inode
                 for each file
        Side effect: retains list of directories
        """

        # First generate a list of files - starting from
        # where we are now, use fileselect to select from that list
        # also storing a list of directories
        # to be
        (scanlist, self.dirlist) = self.filescan(self.fileselect)

        # can only store 496 files
        # check that we can do this
        if len(scanlist) > 496:
            print(f'tp format only supports 496 files, {len(scanlist)} requested')
            sys.exit()

        # now get all the information about the files
        investigated = self.examine(scanlist)

        # filenames cannot be longer than 31 bytes
        # no return on failure
        self.bignames(investigated)

        # if needed create shellscript to
        # make directories
        if self.shellscript != "" \
          and self.dirlist:
            script = self.create_shellscript()
            dirent = self.shellscript_direntry(script)
            investigated.insert(0, dirent)

        # assign tape addresses
        tapea = const.DICTSTART + const.DictSiz
        for inx, _value in enumerate(investigated):
            investigated[inx]['tapea'] = tapea
            tapea = tapea + investigated[inx]['tapeblks']
        # tapea is now the last block needed on the tape
        return investigated

    def filescan(self, fileselect:list) -> tuple[list,list]:
        """ Use the fileselect list to find files
        and directories

        Return a tuple (list of files, list of directories)
        """

        filelist = []
        dirlist = []
        if not fileselect:
            fileselect = ['.']
        for sfile in  fileselect:
            fpath = Path(sfile)
            if not fpath.exists():
                print(f'Cannot find {sfile}')
                continue
            if fpath.is_file():
                filelist.append(sfile)
            elif fpath.is_dir():
                # walk this tree looking for files
                (scanres, dirres) = self.dirscan(sfile)
                for afile in scanres:
                    filelist.append(afile)
                for adir in dirres:
                    dirlist.append(adir)
            else:
                print(f'Not a directory or a file {sfile}')

        # remove any ./ from the start of any filename
        # also ignore files called makedirs.sh
        flist = []
        for filename in filelist:
            if filename[0] == '.' \
              and filename[1] == '/':
                filename = filename[2:]
            if filename != 'makedirs.sh':
                flist.append(filename)

        return (flist, dirlist)

    def dirscan(self, sdir:str) -> tuple[list, list]:
        """ Given a directory name use os.walk to generate the tree
        ignore any directory starting with '.'

        Returns: tuple(list of files, list of directories)
        """

        filesout = []
        dirsout = []
        for basename, dirs, files in os.walk(sdir):
            # ignore any directories starting with '.'
            deldirs = []
            for inx, value in enumerate(dirs):
                if value[0] == '.':
                    deldirs.append(inx)
            if deldirs:
                deldirs.reverse()
                for inx in deldirs:
                    del dirs[inx]
            # save the directory name
            dirsout.append(basename)
            # and the full path
            # also lose files starting with '.'
            for file in files:
                if file[0] != '.':
                    filesout.append(basename + '/' + file)
        return (filesout, dirsout)

    def examine(self, scanlist:list) -> list[dict]:
        """ given a list of files find out the things we need to
        know about them.

        Each dict index:
        fname - stored file name
        fpath - Path object for the file
        mode - the mode of the file -
               only file permissions
        time - timestamp of the mtime of the file
        size - the size of the file
        tapeblks - the number of blocks the file will occupy

        Returns: list of files as dicts
        """

        finfo = []
        for fname in scanlist:
            info = {}
            info['fname'] = fname
            info['fpath'] = Path(fname)
            sta = info['fpath'].stat()
            # only return file permissions
            info['mode'] = sta.st_mode&0o777
            info['time'] = int(sta.st_mtime)
            info['size'] = sta.st_size
            info['tapeblks'] = blocksused(sta.st_size)
            finfo.append(info)
        return finfo

    def bignames(self, investigated:list) -> None:
        """ check that all the names will fit into tp
        directory entry.

        Exit if the test fails
        """

        oversize = [lst['fname'] for lst in investigated
                        if len(lst['fname']) > 31]
        if oversize:
            print("The following file names are longer than 31 characters")
            print("They cannot be stored in tp format")
            for name in oversize:
                print(name)
            sys.exit(1)

    def create_shellscript(self) -> bytearray:
        """ Create a shell script to make any directories needed
        for the tape archive

        Returns: a bytearray of the contents
        """

        lines = [': Create directories needed by the archive',
                    ': mkdir will output a ? if the directory exists']
        for dname in self.dirlist:
            lines.append(f'mkdir {dname}')
        joined = "\n".join(lines)
        joined = joined + '\n'
        contents = bytes(joined, 'ascii')
        return bytearray(contents)

    def shellscript_direntry(self, script:bytearray) -> dict[str, Any]:
        """ Create the 'inode' information for the script

        returns information like examine for disk files
        """

        dire:dict[str, Any] = {}
        dire['fname'] = self.shellscript
        dire['mode'] = 0o777
        dire['time'] = int(datetime.timestamp(datetime.now()))
        dire['size'] = len(script)
        dire['tapeblks'] = blocksused(dire['size'])
        dire['inline'] = script
        return dire

if __name__ == '__main__':

    writer = Writer('abce', ['sys'], False, [], "")
    writer.writer()
    sys.exit()
