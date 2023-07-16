#!/usr/bin/env python3
""" Unpack old archive files

Requires python3.

rdar.py FILE [-l|-x] [-d dir] [files]
will list or extract files from an ar file whose
magic number is 0177545. These appear on the TUHS site:
https://www.tuhs.org/Archive/

it's unclear what system wrote these, but they do seem
to be from a 16-bit machine.

FILE is the name of the file to use for reading, then
-l   lists the all the contents, unless files are
     specified at the end of the line where glob
     rules will be used to match names in the ar
     file. Names are only 14 characters long.
-x   extracts all the files, or selected ones
     as above. They are placed in the current
     directory, -d dir can be used to extract
     them to a subdirectory.
     Files are written preserving their file mode
     and times. However no attempt is made to
     change their ownership.

Format:
First file short:
	#define	ARMAG	0177545

    struct	ar_hdr {
        char	ar_name[14];	/* 0 - 14chars */
        int     ar_date;	/* 14 - 4b */
        char	ar_uid;		/* 18 - 1b */
        char	ar_gid;		/* 19 - 1b*/
        int	    ar_mode;	/* 20 - 2b */
        long	ar_size;	/* 22 - 4b*/
    };

Peter Collinson
July 2023

"""

import sys
import os
from pathlib import Path
from struct import unpack_from, calcsize
from datetime import datetime
from getopt import getopt, GetoptError
from fnmatch import fnmatch

# magic start of the file
ARMAG = 0o177545

# pylint: disable=global-statement, invalid-name
# current offset in file
offset = 0


def getFile(fpath: Path) -> bytes:
    """ Load the file as bytes """

    buf = fpath.read_bytes()
    return buf

def check_magic(buf: bytes) -> None:
    """ Check that the first 16-bit word equals ARMAG """

    global offset

    fmt = "<H"
    tup = unpack_from(fmt, buf[offset:])
    if tup[0] != ARMAG:
        print("Cannot find magic value")

        sys.exit()
    offset += calcsize(fmt)

def filehdr(buf: bytes) -> dict:
    """ Read a file header """

    global offset

    fmt = "<14sHHBBHHH"
    names = ['name', 'd0', 'd1', 'uid', 'gid', 'mode', 's0', 's1']
    tup = unpack_from(fmt, buf[offset:])
    vals = {v[0]:v[1] for v in zip(names, tup)}
    out = {}
    out['name'] = vals['name'].decode("latin-1").rstrip('\x00')
    dt = make32(vals['d0'], vals['d1'])
    out['date'] = datetime.fromtimestamp(dt)
    for cp in ['uid', 'gid', 'mode']:
        out[cp] = vals[cp]
    out['size'] = make32(vals['s0'], vals['s1'])
    offset += calcsize(fmt)
    return out

def ls(buf: bytes, matchlist: list) -> None:
    """ List the file information in the archive """

    global offset

    while offset < len(buf):
        fhdr = filehdr(buf)
        if not matchlist or matchall(fhdr['name'], matchlist):
            display(fhdr['mode'], fhdr['size'],
                        fhdr['date'], fhdr['name'])
        offset = offset + fhdr['size']
        if offset&1:
            offset += 1

def display(mode: int, size:int,
             fdate: datetime,
             name:str) -> None:
    """ list the file info """

    print(visiblemode(mode), end='')
    print(f" {size:6d}  ", end='')
    dtfmt = "%d %b %Y %H:%M"
    print(fdate.strftime(dtfmt), end='')
    print(f"  {name}")

def visiblemode(mode:int) -> str:
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

def matchall(name: str, matchl: list) -> bool:
    """ Match name against the list of files in matchl

    return True if there is a match, false otherwise
    """

    for tomatch in matchl:
        if fnmatch(name, tomatch):
            return True
    return False

def extract(buf: bytes, matchlist: list,
                destdir: str) -> None:
    """ Extract files """

    global offset

    if destdir is not None:
        dirpath = Path(destdir)
        if dirpath.exists() and \
            not dirpath.is_dir():
            print(f"{destdir} exists and is not a directory")
            sys.exit()

    while offset < len(buf):
        fhdr = filehdr(buf)
        if not matchlist or matchall(fhdr['name'], matchlist):
            if destdir is not None:
                if not dirpath.exists():
                    dirpath.mkdir(parents=True)
                filep = dirpath / fhdr['name']
            else:
                filep = Path(fhdr['name'])
            writef(buf, fhdr, filep)

        offset = offset + fhdr['size']
        if offset&1:
            offset += 1

def writef(buf: bytes, fhdr: dict, ofile: Path):
    """ Write a file from the buffer

    set permissions and times
    """

    # pylint: disable=global-variable-not-assigned
    global offset

    if ofile.exists():
        print(f"{ofile} exists - not overwriting")
        return
    ofile.write_bytes(buf[offset:offset+fhdr['size']])
    ofile.chmod(fhdr['mode']&0o777)
    ts = fhdr['date'].timestamp()
    os.utime(ofile, (ts, ts))
    print("-> ", end="")
    display(fhdr['mode'], fhdr['size'], fhdr['date'], ofile)

def make32(hibyte:int, lobyte:int) -> int:
    """ Make a PDP-11 long """

    return (hibyte<<16) + lobyte

def usage(msg: str):
    """ print usage info """

    info = """Usage
rdar.py FILE [-l|-x] [-d dir] [files]
will list or extract files from an ar file whose
magic number is 0177545. These appear on the TUHS site,
it's unclear what system wrote these, but they do seem
to be from a 16-bit machine.

FILE is the name of the file to use for reading, then
-l   lists the all the contents, unless files are
     specified at the end of the line where glob
     rules will be used to match names in the ar
     file. Names are only 14 characters long.
-x   extracts all the files, or selected ones
     as above. They are placed in the current
     directory, -d dir can be used to extract
     them to a subdirectory.
     Files are written preserving their file mode
     and times. However no attempt is made to
     change their ownership.
"""
    if msg is not None:
        print(f"*** {msg}")
    print(info)
    sys.exit()


def main():
    """ Main code decode arguments and do the stuff """

    # pylint: disable=too-many-branches

    if len(sys.argv) == 1:
        usage(None)


    fname = sys.argv[1]
    fpath = Path(fname)
    if not fpath.exists():
        usage(f"{fname} not found")

    try:
        opts, args = getopt(sys.argv[2:], "hxld:", "")
    except GetoptError as err:
        usage(err)
        sys.exit(1)

    action = None
    extdir = None
    for opt, arg in opts:
        if opt == '-h':
            usage(None)
        elif opt == '-l':
            if action:
                usage("Only one of -l or -x is allowed")
            action = 'ls'
        elif opt == '-x':
            if action:
                usage("Only one of -l or -x is allowed")
            action = 'extract'
        elif opt == '-d':
            if arg:
                extdir = arg
            else:
                usage("Missing directory after -d\n")

    if action is None:
        usage("Supply one of -l or -x")

    matchlist = []
    if args:
        matchlist = list(args)

    # check the file
    buf = getFile(fpath)
    check_magic(buf)

    if action == 'ls':
        ls(buf, matchlist)
    else:
        extract(buf, matchlist, extdir)


if __name__ == '__main__':
    main()
    sys.exit()
