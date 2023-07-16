# uxtp - a Python 3 version of the tp program

When I started this project, the first problem was the question about how to get easily get information in and out of a simulator running V6. I decided to implement a version of the _tp_ program designed to read and write magnetic tapes blocked into 512-byte blocks. The idea of _uxtp_ was born, its name has been shortened as time has passed. It's written in Python 3, which should aid its portability.

## Command interface

``` sh
uxtp -[h|trx] [-v] [-s] [-f] [-o UID:GID] TAPEFILE [files...]
```

* _uxtp_ with no arguments will print a short description of the options to the script.
* -h - prints extended help information.
* -t - lists the file on tape file. The v option provides an extended  listing. The table printed at the end of the listing is output to stderr, so redirecting the output to a file will result only in a list of files.
* -r - writes named files to the tape. The TAPEFILE will be re-initalised, any data on it will be lost.  If no files are specified then all the files and directories in the current directory will be saved. The -v option will print the actions that are taken.
  Files will be written stored owned by root (uid:0, gid:0), unless  overridden on the command line with _-o uid:gid_. The owner must be a pair of numbers separated by a colon. _tp_ on V6 will not set the ownership of files, unless it's being run by root.
  Unix V6 _tp_ V6 will not create directories, add the -s flag to  create a shell script of mkdir commands called _makedirs.sh_ to make any directories needed when unloading the tape. Files on the host called _makedirs.sh_ will be ignored.
  NB. Pathnames stored can contain directories and subdirectories, but the path name is limited in length to 31 characters.  Beware that files cannot use UTF-8 extended characters for use  on UNIX-V6. Files in UTF-8 that essentially use the ASCII character set will work.
* -x - extracts files from the TAPEFILE. If no files are specified, then all files will be extracted. Embedded directories in the filenames will be created if needed. A file argument of a directory will extract all the files in that directory. When asking for files, the UNIX  style '*' and '?' expansions can be used. You will need to use quotes around any filename containing one of the expansions. The -v option will print the actions that are taken. Times on the extracted files will be set. Files that exist will not be overwritten unless the -f (force) flag is given. This differs from V6's _tp_.

Argument decoding is done using Python's getopt routine so command letters can be concatenated - but must start with a `-`.

The script is designed to read and write simulated magnetic tapes and DecTapes. The suffix to the tape file is used to set the format:

* .tap - is a Magnetic Tape image, blocked in 512 blocks.
* .dec - is a DecTape format, which is a essentially a single file.
* .dat - an unblocked Magnetic tape image.

Magnetic tapes and DecTapes have different directory sizes.

## _tp_ on V6

The _uxtp_ command intentionally uses the same command characters as the V6 _tp_ command. The _tp_ command will expect to read from the DecTape device unless _m_ is given on the command line. A typical command to list the contents of a magnetic tape is:

``` sh
tp tvm0
```
The number is needed after the 'm' to give the unit number of the device.

The most frequently printed error message from _tp_ is mostly _Tape read error_.  This can happen when you don't have read access to the device in _/dev/_. Use _chmod 666 device_ to give read and write access to everyone on the system. Sometimes the driver gets confused and the message can be printed after all the files have been extracted. I worried about this being an error with _uxtp_, but I don't believe it is.

## Installation

Copy _uxtp.sh_ to your local _bin_, renaming it _uxtp_. Edit it so that the _UXPATH_ variable contains the path to the _uxtp_ directory containing the Python.

## Adding a mag tape to SIMH

To add mag tape to your simulation insert

``` text
set tm0 writeenabled
attach tm0 tm0.tap
```
into your startup file. The _attach_ command links a file called _tm0.tap`` to the device. If the file doesn't exist one will be created. The tape contents are cached by the simulator so it can be written to while the simulator is running. However, you have to detach and attach the file to see the new contents. So you can type ^E to get to the SIMH command line and enter:

``` text
det tm0
att tm0 tm0.tap
co
```

SIMH allows abbreviated commands. the last _co_ returns to the simulator. The real trick is to put these lines into a file with a short name like _relm_ for reload mag tape. Now after you use ^E  to get to the simulator, you can type

``` text
do relm
```
running the commands from the file.
