# Commands helping with filesystem trees

## cptree

On the Unix Sources archive on [The Unix Heritage Society](https://www.tuhs.org/Archive/Applications/Spencer_Tapes/) website there are several tapes that Henry Spencer rescued. Among them is _uk1.tar.gz_ which was created from several universities in the UK in 1980. There's a ton of my code there.

This tape has several files called _cont.a_ - which are UNIX archive files - but they don't match V6 because they have a different magic number. I wrote a tape lister and extractor in Python 3 to read these - see [rdar](../rdar).

The _cptree_ came from the Glasgow section of the tape - and seems a useful addition to V6.

The _compile_ script compiles _cptree_.

## rmtree

You can delete all the files in a directory tree using:

``` sh
rm -r DIR
```

If you need to add the _-f_ flag, then it must be added as a separate argument:

``` sh
rm -r -f DIR
```
But there is no _-r_ flag to _rmdir_. The _rmtree_ shell script will remove a whole tree. This script does:
* ```rm -r -f  DIR``` deleting all the _files_ under the directory.
* generate a list of directories using _find_, and sort into reverse order
* edit the file to add _rmdir_ to the start of each line
* execute the file to delete all the directories
* clean up the temporary files on /tmp
