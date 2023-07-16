#!/usr/bin/env python3
""" Command decode for uxtp.py

Peter Collinson
July 2023
"""

import sys
from getopt import getopt, GetoptError
from reader import Dirls, Extract
from writer import Writer

def extendedhelp() -> None:
    """ Print usage information """

    info = """uxtp command help

This command is a python version of the tp program supplied with
UnixV6. It was written to get information in and out of a simulated
PDP-11. This script will not function correctly on versions of Python
before 3.6. It's been developed on 3.10.

The script is designed to read and write simulated magnetic tapes and
DecTapes. The suffix to the tape file is used to set the format:
.tap - is a Magnetic Tape image, blocked in 512 blocks.
.dec - is a DecTape format, which is a set of raw blocks.
.dat - an unblocked Magnetic tape image
Magnetic tapes and DecTapes have different directory sizes.

When writing using this script, a new 'tape' file is created on every
use. The script cannot be used to append files to a partially loaded
tape.

tp on UNIX V6 needs the 'm' flag in the argument list to select
a magnetic tape reader/writer.

Arguments to the uxtp script:

uxtp with no arguments will print a short description of the
options to the script.

uxtp -h prints this extended usage text.

Command synopis:

uxtp -[h|trx] [-v] [-s] [-f] [-o UID:GID] TAPEFILE [files...]

The suffix on TAPEFILE is used to indicate format
.tap - tp mag tape format blocked into 512 byte blocks
.dec - tp DecTape format, unblocked
.dat - tp mag tape format with no blocking

h - prints this text

or one of:

t - lists the file on tape file. The v option provides an extended
    listing. The table printed at the end of the listing is output
    to stderr, so redirecting the output to a file will result
    only in a list of files.

r - writes named files to the tape. The TAPEFILE will be
    re-initalised, any data on it will be lost.
    If no files are specified then all the files and directories in
    the current directory will be saved.
    The v option will print the actions that are taken.

    Files will be written stored owned by root (uid:0, gid:0), unless
    overridden on the command line. uid:gid must be a pair of numbers
    separated by a colon.

    tp in Unix V6 will not create directories, add the -s flag to
    create a shell script of mkdir commands called makedirs.sh
    to make any directories needed when unloading the tape. Actually
    files called makedirs.sh will not be written.

    SimH format is used for the output image if TAPEFILE ends in .tap,
    otherwise a raw file is expected or generated.

    NB. Pathnames stored can contain directories and subdirectories, but
    the name is limited in length to 31 characters.
    Beware that files cannot use UTF-8 extended characters for use
    on UNIX-V6. Files in UTF-8 that essentially use the ASCII character
    set will work.

x - extracts files from the TAPEFILE. If no files are specified, then
    all files will be extracted. Embedded directories in the filenames
    will be created if needed. A target of a directory will extract
    all the files in that directory. When asking for files, the UNIX
    style '*' and '?' expansions can be used. You will need to use
    quotes around any filename containing one of the expansions. The v
    option will print the actions that are taken. Times on the
    extracted files will be set. Files that exist will not be
    overwritten unless the -f (force) flag is given.

"""
    print(info)

def usage(msg:str) -> None:
    """ Print short usage """

    if msg != "":
        print(f'*** {msg}')
        print()

    info = """Usage:
uxtp -[h|trx] [-v] [-s] [-f] [-o UID:GID] TAPEFILE [files...]
h - print extended help
One of:
t - list files
r - write files to TAPEFILE (replace)
x - extract files from TAPEFILE

v - verbose flag - for t option print extended listing
    for r or e print what files are being dealt with
o - followed by a UID:GID pair used to set owner of file
    'r' option only
s - add a shell script called makedirs.sh containing
    mkdir commands that can be extracted first
    to create any needed directories.  tp on UNIX V6
    doesn't do this automatically.
f - always write files when extracting, usually
    files that exist are not changed

SimH blocked magtape format is used if TAPEFILE ends in .tap,
DecTape format is used if TAPEFILE ends in .dec,
otherwise use .dat for raw file.
"""
    print(info)

def decode_owner(ownerarg:str) -> tuple[int, int]:
    """ Decode uid:gid from arg
    do some validity checking
    """
    uid, gid = ownerarg.split(':')
    if not uid or not uid.isnumeric():
        msg = 'uid is not numeric'
        usage(msg)
        sys.exit(0)

    if not gid or not gid.isnumeric():
        msg = 'gid is not numeric'
        usage(msg)
        sys.exit(0)

    return (int(uid), int(gid))

def main():
    """ Decode arguments and call appropriate action """

    # pylint: disable=too-many-locals,too-many-branches,too-many-statements
    if len(sys.argv) == 1:
        usage("")
        sys.exit()
    try:
        opts, args = getopt(sys.argv[1:], "trxhvsfo:", "")
    except GetoptError as err:
        usage(err)
        sys.exit(1)
    forcewrite = False
    command = ""
    verbose = ""
    shellfile = ""
    owner = []
    ownerarg = ""
    commandmap = {
        '-r': 'insert',
        '-t': 'ls',
        '-x': 'extract'
        }

    for opt, arg in opts:
        if opt == '-h':
            extendedhelp()
            sys.exit()
        elif opt == '-v':
            verbose = True
        elif opt == '-f':
            forcewrite = True
        elif opt == '-o':
            if ':' in arg:
                ownerarg = arg
            else:
                usage('The o option must have an argument of two numbers separated by :')
                sys.exit(1)
        elif opt == '-s':
            shellfile = 'makedirs.sh'
        elif opt in ('-t', '-r', '-x'):
            if not command:
                command = commandmap[opt]
            else:
                usage('Only one of t, r, e can be specified')
                sys.exit(1)
    if not command:
        usage('One of t, r, x must be specified')
        sys.exit(1)

    if command != 'insert':
        if shellfile != "":
            usage('The -s shellfile option can only be used with -r')
        if ownerarg != "":
            usage('The -r shellfile option can only be used with -r')
    elif ownerarg != "":
        owner = decode_owner(ownerarg)

    if not args:
        usage('A filetape name is required')
        sys.exit(1)

    tapefile = args[0]
    fileselect = args[1:]

    if command == 'ls':
        dirls = Dirls(tapefile, fileselect, verbose)
        dirls.dirls()
        return
    if command == 'extract':
        extract = Extract(tapefile, fileselect, verbose, forcewrite)
        extract.extract()
        return
    if command == 'insert':
        writer = Writer(tapefile, fileselect, verbose,
                            owner, shellfile)
        writer.writer()

if __name__ == '__main__':
    main()
    sys.exit()
