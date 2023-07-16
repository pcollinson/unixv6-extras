# _aredit_ -  better editing of _ar_  libraries

The _aredit_ program is a solution to the problems that I outlined in [Notes on ar](../notes/ar.md). It allows you to name a file in an _ar_ library and insert or append another file before or after it. You can also ask it to replace the marker file with a new binary.

### Example

Here's an example of use:

The new _ctime.c_ code in [../time](../time) replaces _ctime.o_ and also introduces some assembler code from V7 in _ctsup.o_. It turns out that this code needs to be some way up the library, so to be safe I want to put it directly after _ctime.o_.

To make this insertion happen, go to some convenient directory, compile to get the new binary files. Copy _lib/libc.a_ into the directory. Now:

```sh
aredit ca libc.a ctime.o ctsup.o
```
which says in _libc.a_, change _ctime.o_ and append _ctsup.o_ after it. The program now:

* Creates a working directory called _aredit.tmp_
* Changes into it
* unpacks _../libc.a_ - remembering the order of files
* copies _../ctime.o_ into the directory because it's been asked  to replace it by the _c_ flag
* copies _../ctsup.o_ into the directory
* starts to process the files in its list, adding them to a new archive file. When it gets to _ctime.o_, it is loaded into the archive, and will also load the new file _ctsup.o_ after it. The _ctime.o_ is the new version because it's been copied earlier.
* when it's done with all the files, it replaces _libc.a_ with the archive it's built
* then it tidies up removing all the files in _aredit.tmp_ and finally the directory itself.

If you are happy you can replace the _libc.a_ in _/lib_ with the new copy.

Here's a sample output of this process:

``` sh
# aredit ca libc.a ctime.o ctsup.o
Making aredit.tmp
Changing dir into aredit.tmp
Cleaning aredit.tmp
Extracting ../libc.a contents
Copying ../ctime.o to ctime.o
Copying ../ctsup.o to ctsup.o
Rebuilding archive into templib.a
Found ctime.o
Append ctsup.o
Copying templib.a to ../libc.a
Removing temporary contents
Cleaning aredit.tmp
Change dir to parent
Removing aredit.tmp
```

Notice that the working directory has to be a child of the directory you are in because the program makes use of the '..' links to move back up to its parent, and also to access files in its parent.

### Moving a file in a library

 You can use aredit to move a file in a library:

 * Copy the library to a local directory
 * extract the file you want to move - ```ar x lib.a tomove.o```
 * append the file in a new place - ```aredit a lib.a afterme.o tomove.o```
   _aredit_ will notice that _tomove.o_ is a file to be appended and will ignore the copy that it obtained by extracting the library. It will tell you that this has happened.
