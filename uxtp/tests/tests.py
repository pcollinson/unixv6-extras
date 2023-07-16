"""
uxtp tests using pytest
"""

import os
import sys
from random import randint
from pathlib import Path
import pytest
sys.path.insert(0, "../uxtp")
from constants import *
from reader import Dirls, Extract, ReadTape
from writer import Writer

def test_to_and_from_32():
    """ Basic tools in constants """

    # only needs to deal with +ve numbers
    # test say 100 random integers
    for inx in range(1000):
        testv = randint(0, 0x7fffffff)
        high, low = from_32(testv)
        if high == 0:
            assert low == testv
        initial = to_32([high, low])
        assert initial == testv

def test_fileselect():
    """ Test file select code """

    lookups = {'makedirs.sh': ['makedirs.sh'],
              '*':            ['makedirs.sh', 'sampledir/reader.py',
                                  'sampledir/sub/writer.py'],
              '*writer*':     ['sampledir/sub/writer.py'],
              'sampledir':    [],
              'unknown':      [],
              }
    dirls = Dirls('sample.tap', [], False)
    for lookup, answer in lookups.items():
        filtered = dirls.tape.filter([lookup])
        result = [ent['pathstring'] for ent in filtered]
        assert answer == result

@pytest.fixture
def dirargs(capfd):

    # dataset is
    # filename, fileselect, verbose, stdout, stderr
    stdlisting = sampleout('stdlisting')
    errlisting = sampleout('errlisting')
    verbose_dat = sampleout('verbose_dat')
    verbose_tap = sampleout('verbose_tap')
    makedirs_dat = sampleout('makedirs_dat')
    makedirs_tap = sampleout('makedirs_tap')
    match1 = sampleout('match1')
    sampledir = sampleout('sampledir')

    dataset = [["sample.dat", [], False, stdlisting, errlisting],
               ["sample.tap", [], False, stdlisting, errlisting],
               ["sample.dat", [], True, verbose_dat, errlisting],
               ["sample.tap", [], True, verbose_tap, errlisting],
               ["sample.tap", ['makedirs.sh'], True, makedirs_tap, match1],
               ["sample.tap", ['sampledir/*'], True, sampledir, match1],
               ]
    return(capfd, dataset)

def test_dirls(dirargs):
    """ Test dirls """

    capfd, dataset = dirargs

    for filename, fileselect, verbose, stdtxt, errtxt in dataset:
        dirls = Dirls(filename, fileselect, verbose)
        dirls.dirls()
        captured = capfd.readouterr()
        assert captured.out == stdtxt
        assert captured.err == errtxt

def test_extraction(capfd):
    """ Extract a file and check that it seems OK """

    # tidy up
    tidyup()
    # make the testdir and change into it
    maketestdir()

    extract = Extract("../sample.tap", ['makedirs.sh'], True, False)
    extract.extract()

    captured = capfd.readouterr()
    assert captured.out == "writing makedirs.sh\n"

    # find the dirent
    dirent = [ent for ent in extract.tape.filtered
                  if ent['pathstring'] == "makedirs.sh"]
    assert len(dirent) == 1
    assert dirent[0]['size'] != 0
    assert dirent[0]['time'] != 0
    # stat the file to compare the data in dirent
    mdpath = Path('makedirs.sh')
    stat = mdpath.stat()
    assert dirent[0]['size'] == stat.st_size
    assert dirent[0]['time'] == stat.st_mtime
    assert dirent[0]['mode'] == stat.st_mode&0o777
\
    # test directory creation
    extract = Extract("../sample.tap", ['sampledir/reader.py'], True, False)
    extract.extract()

    captured = capfd.readouterr()
    assert captured.out == "mkdir sampledir\nwriting sampledir/reader.py\n"

def test_extraction_fail(capfd):
    """ Check for extraction fails """

    extract = Extract("../sample.tap", ['makedirs.sh'], True, False)
    extract.extract()

    captured = capfd.readouterr()
    assert captured.out == "makedirs.sh exists, ignoring\n"

    tidyup()

def test_creation():
    """ Create a tape file """

    writer = Writer("testout.tap", ['sampledir'], False, [], "makedirs.sh")
    writer.writer()

    fpath = Path("testout.tap")
    assert fpath.exists()
    fstat = fpath.stat()

    refpath = Path("sample.tap")
    rstat = refpath.stat()

    assert fstat.st_size == rstat.st_size

    writer = Writer("testout.dat", ['sampledir'], False, [], "makedirs.sh")
    writer.writer()

    fpath = Path("testout.dat")
    assert fpath.exists()
    fstat = fpath.stat()

    refpath = Path("sample.dat")
    rstat = refpath.stat()

    assert fstat.st_size == rstat.st_size

def test_data_match():
    """ Match values in two files """

    for suffix in ['.tap', '.dat']:
        newfile = 'testout' + suffix
        reffile = 'sample' + suffix

        newt = ReadTape(newfile, [])
        reft = ReadTape(reffile, [])
        assert len(newt.filtered) == len(reft.filtered)

        for inx in range(len(newt.filtered)):
            newent = newt.filtered[inx]
            refent = reft.filtered[inx]
            assert newent['pathstring'] == refent['pathstring']
            assert newent['size'] == refent['size']

        fpath = Path(newfile)
        fpath.unlink()

def maketestdir():
    """ Make a test directory so that the extraction will work """

    tidyup()
    dirpath = Path('testdir')
    if not dirpath.is_dir():
        dirpath.mkdir()
    os.chdir(dirpath)

def tidyup():
    """ Remove any files that are created by the tests """

    cwd = Path(os.getcwd())
    parts = cwd.parts
    lastelem = parts[::-1][0]
    # if we are not in testdir
    # does testdir exist?
    if lastelem != 'testdir':
        if not Path('testdir').is_dir():
            return
        os.chdir("testdir")
    # remove any file if it was left by previous run
    mdpath = Path('makedirs.sh')
    if mdpath.exists():
        mdpath.unlink()
    rdfile = Path('sampledir/reader.py')
    if rdfile.exists():
        rdfile.unlink()
    samdir = Path('sampledir')
    if samdir.is_dir():
        samdir.rmdir()
    os.chdir('..')
    td = Path('testdir')
    if td.is_dir():
        td.rmdir()

def sampleout(select):

    collection = {
'stdlisting': """makedirs.sh
sampledir/reader.py
sampledir/sub/writer.py
""",
 'errlisting': """   3 entries
  55 used
 117 last
END
""",
 'verbose_dat': """  mode    uid gid tapa     size   date    time name
rwxrwxrwx  0   0    63      127 23/07/12 17:47 makedirs.sh
rw-r--r--  0   0    64    13724 23/07/12 16:36 sampledir/reader.py
rw-r--r--  0   0    91    13382 23/07/12 16:36 sampledir/sub/writer.py
""",
 'verbose_tap': """  mode    uid gid tapa     size   date    time name
rwxrwxrwx  0   0    63      127 23/07/12 17:48 makedirs.sh
rw-r--r--  0   0    64    13724 23/07/12 16:36 sampledir/reader.py
rw-r--r--  0   0    91    13382 23/07/12 16:36 sampledir/sub/writer.py
""",
 'makedirs_dat': """  mode    uid gid tapa     size   date    time name
rwxrwxrwx  0   0    63      127 23/07/12 17:47 makedirs.sh
""",
 'match1': """   1 matched
   3 entries
  55 used
 117 last
END
""",
 'makedirs_tap': """  mode    uid gid tapa     size   date    time name
rwxrwxrwx  0   0    63      127 23/07/12 17:48 makedirs.sh
""",
 'match1': """   1 matched
   3 entries
  55 used
 117 last
END
""",
 'sampledir': """  mode    uid gid tapa     size   date    time name
rw-r--r--  0   0    64    13724 23/07/12 16:36 sampledir/reader.py
"""
        }
    return(collection[select])
