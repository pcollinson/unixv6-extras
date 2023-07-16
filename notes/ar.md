# Notes on ar

On the system there are several scripts called _run_ which rebuild  various libraries and systems.  Many of them use _ar_ to generate  a library. However, they are scripts intended to build a release where the library is already established, so their intention is simply to refresh an existing archive file. The _run_ files generate a bunch of _.o_ files, and then say:

``` sh
ar r ../libname.a
```
which asks _ar_ to look for any _.o_ file in the current directory and replace them in the archive. However, it won't add a new file. You need to put the new file on the command line to add it into the library.

``` sh
ar r ../libname.a newfile.o
```
The new file is added to the end of the library, which frequently is not OK,  if the new file probably depends on other files. The C compiler, or more precisely the _ld_ program does a single pass through libraries, and expects dependencies to be carefully ordered.

You might be tempted to delete the library and say:

``` sh
ar r ../libname.a *.o
```
This works but can break the library in such a way to make it unusable, remember that the shell sorts its arguments after it's expanded the star. The existing order of files in many libraries is important, and the only definitive way of knowing the order is by looking at the library listing.

## Working with the kernel libraries

In the distributed _run_ scripts, again you will  see:

``` sh
ar r ../lib1
```
which theoretically replaces any file in the library with a bunch of  _*.o_ files in the current directory. This works as long as the library contains the file, but if the file is absent then it isn't added (which the manual page for _ar_ implies should happen). Also if  _../lib1/_ doesn't exist, then an empty file is created but nothing gets added.

I've worked around this in my _rebuild_ script (see [../sysconf](../sysconf/README.md)) by naming the files in the desired order for _lib1_, which will rebuild the library if it's gone for some reason. However, for _lib2_, the binaries are pulled in by the _c.c_ file compiled at system build time, so the order of the binaries for that is not really important. So for the _dmr_ files, I am using:

``` sh
ar r ../lib2 *.o
```
This is OK because the argument list is not too long (V6 limits expansions to 512 bytes) and also the order of the contents is not critical.

## Making libc.a and friends

The ordering of files in _libc.a_ is very important. Any new binary that derives from C, or assembler that uses the C calling conventions needs to be inserted at least above _csv.o_. The binary will define two global values that C uses to call and return from functions.

To get a new file into the library, it might be possible to extract and delete the last few binaries, insert the new versions and reload the files that have been removed.

System calls generally only have a dependency on _perror_, but other code may need to be further up the library. However, you can now replace _/lib/libc.a_ with your new version having prudently made a backup, of course.

## Next step - _aredit_

However, there is no easy way of inserting files into the middle of _libc.a_.  If you unpack it all into a directory then you can generate a list of files from _ar_ into a file and use the file to make a set of append commands. I've done this, and it's prone to finger trouble as well as being labour intensive.

I do want to add a file or so in the middle of _libc.a_, perhaps replacing an old one and adding a new one after the first at the same time. This need was recognised by V7 where the version of _ar_ supplies the solutions.

I decided to not go into battle with the problem of back porting the code. This is probably possible but the real problem is proving that the code then works, which would be a long job. I started with a shell script which was sort-of working, and then I've written a new program in C that gets around the problem of wanting to insert files in the middle of a library. You can find it on [../aredit/README.md](../aredit).
