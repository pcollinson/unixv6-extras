# rdar.py - extract files from vintage _ar_ archives

There are several files on the archive pages of [The Unix Heritage Society](https://www.tuhs.org/Archive/) that are called _cont.a_. They are often embedded in _tar_ files of old tapes.

The files appear to be _ar_ files, but they have magic number of   0177545, which doesn't match the V6 _ar_ version.
The Python 3 script _rdar.py_ extracts files from these files on your host machine.

## Usage
```
rdar.py FILE [-l|-x] [-d dir] [files]

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
```

## Processing a tree

If you unpack a _tar_ archive and find _cont.a_ files, then it's helpful to be able to traverse the whole tree.

Change to top of the tree and run _unpack-cont.sh_. It expects _rdar.py_ to be called _rdar_ which can be accomplished using

``` sh
ln -s rdar.py rdar
```
