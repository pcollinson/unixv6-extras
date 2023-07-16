""" Constants and shared code for uxtp

The tape file created by UnixV6 is in SIMH tape format
and that needs extracting by rawtap found in the SimH
extractor tools. This will create a single binary file
that this code can deal with. rawtap will also be needed
to wrap the file created by this script back to SimH tape
format.

Peter Collinson
July 2023
"""
import struct

# PDP11 Block size in bytes
BLKSIZ = 512
# divide or multiply by 512
BLKSHIFT = 9
# Mask to see if a block is the right size
BLKMSK = 0x1ff

# PDP11s are little endian when dealing with ints
PACK_PREFIX = '<'

# Tp tape format is
# Block 0: Bootstrap
# Block 1: Start of dictionary for contents
# Block 63: End of dictionary on Mag tape
#           So 62 blocks holding 496 entries
# or
# Block 25: End of dictionary for DecTape
#			So 24 blocks holding 192 Entries
# Rest of tape has the contents
# Tapes suffixes are
# .tap - blocked tp tape format
# .dat - unblocked tp tape format
# .dec - DecTape Format - which is unblocked
DICTSTART = 1
dictsizes = {'.tap': 62, '.dat': 62, '.dec': 24}
#
# this is now a global variable and not a constant
# pylint: disable=invalid-name
DictSiz = 62

def dirsize(suffix: str) -> int:
    """ return size based on suffix """

    if suffix.lower() in dictsizes:
        dsiz = dictsizes[suffix]
    else:
        dsiz = dictsizes['.tap']
    return dsiz

# SIMH tape marks
TWC = [b'\0', b'\2', b'\0', b'\0']
TEF = [b'\0', b'\0', b'\0', b'\0']

# when creating a tape we need to set uid/gid
# this can be overridden on the command line
# and defaults to these values
DEF_UID = 0
DEF_GID = 0

# Dictionary layout, from the V7 tp(5) page
# Don't think that UnixV6 had longs
# so I've changed this to how UnixV6 would
# have written it
# struct {
#   	char pathname[32];
#       int  mode;
#       char uid;
#       char gid;
#       char unused1;
#       char size0
#       int size[1];
#       int modtime0;
#       int modtime1
#       int  tapeadd
#       char unused2[16];
#       int  checksum;
# };
# struct pack/unpack Types
# H - unsigned short
# B - byte

DIRENT_FMT =     {'pathname': {'fmt': '32c', 'type': 'vec'},
    	          'mode':     {'fmt': 'H', 'type': 'int'},
                  'uid':      {'fmt': 'B', 'type': 'int'},
                  'gid':      {'fmt': 'B', 'type': 'int'},
                  'spare':    {'fmt': 'B', 'type': 'int'},
                  'size0':    {'fmt': 'B', 'type': 'int'},
                  'size1':    {'fmt': 'H', 'type': 'int'},
                  'time0':    {'fmt': 'H', 'type': 'int'},
                  'time1':    {'fmt': 'H', 'type': 'int'},
                  'tapea':    {'fmt': 'H', 'type': 'int'},
                  'unused':   {'fmt': '8H', 'type': 'vec'},
                  'cksum':    {'fmt': 'H', 'type': 'int'}}
CKSUM_FMT =      {'body': {'fmt': '32H', 'type': 'vec'}}

# Packing and unpacking binary data using the formats above
def get_using_format(source:bytes,
                         layout:dict,
                         offset:int = 0) -> tuple[dict, int]:
    """ Get data from source, by unpacking using layout, start
    from offset
    Return: dict with results, offset
    """

    out:dict = {}
    for inx, conf in layout.items():
        fmt = PACK_PREFIX + conf['fmt']
        value = struct.unpack_from(fmt, source, offset)
        if conf['type'] == 'int':
            wanted = value[0]
        else:
            wanted = list(value)
        out[inx] = wanted
        offset = offset + struct.calcsize(fmt)
    return out, offset

def put_using_format(src:dict,
                         layout:dict,
                         target:bytearray,
                         offset=0) -> tuple[bytearray, int]:
    """ Put data from src dict, by packing using layout, start
    from offset
    Return: bytearray, offset
    """

    for inx, conf in layout.items():
        value = src[inx]
        fmt = PACK_PREFIX + conf['fmt']
        if conf['type'] == 'int':
            struct.pack_into(fmt, target, offset, value)
        else:
            struct.pack_into(fmt, target, offset, *value)
        offset = offset + struct.calcsize(fmt)
    return target, offset

# Various times and sizes in Unix V6 are stored as two word arrays
# making a 32 bit integer
# these two routines encode and decode
def to_32(shorts:list) -> int:
    """ Various values in V6 are stored as pairs of shorts
    the first value is the high 16 bits.
    Return a 32 bit int
    """

    return (shorts[0]<<16) + shorts[1]

def from_32(value: int) -> tuple[int, int]:
    """ Convert 32 bit ints back to a pair of 16 bits """

    out0 = value >> 16
    out1 = value & 0xffff
    return (out0, out1)

def block_to_bytes(bno: int) -> int:
    """ Convert a block number to a byte offset """

    return bno << BLKSHIFT

def blocksused(size:int) -> int:
    """ Compute how many blocks this file uses """

    blocks = (size + 511) >> BLKSHIFT
    return blocks
