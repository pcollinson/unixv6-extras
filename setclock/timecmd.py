#!/usr/bin/env python3
""" Make a paper tape
should start and end with 100 blank values
"""
import time
from pathlib import Path


def cmd():
    """ make a date cmd """

    tspec = time.strftime("%m%d%H%M%y")
    cmd = "date " + tspec + '\n'
    body = bytearray(cmd, "ascii")
    f = Path("ptr")
    f.write_bytes(body)

if __name__ == '__main__':

    cmd()
