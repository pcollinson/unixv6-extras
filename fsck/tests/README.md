# Testing fsck

This directory contains testing programs that will induce problems in a file system.

## How to compile

* ```chdir lib```
* ```compile``` - will create a library called _testlib_ in the current directory.
* ```chdir  ..```
* ```compile``` will generate three binaries.

## How to use

The full release of Unix V6 for the PDP-11 comes on three RK05 disks. The standard _rk.c_ driver in the system will support four disks, so enable the last one: _rk3_ and make a file system on the newly created file (see [mkfs](../../mkfs)). This becomes a useful place to work, because it has lots of free space and you are less at danger of breaking the root file system. If you want to check the validity of the root file system, when the simulator is stopped you can copy the _rk0_ file to _rk3_ check and fix it before copying it back.

The tests here are intended to be done on a 'disk' whose file system is sacrificial.

## Tests

To avoid finger trouble, the file _conf.ini_ holds the name of the disk, the directory used to mount it, and the name of a safety file that must be present for the disk to be modified. Doing this avoids using the command line which is subject to finger trouble.

Use the commands below one at a time and fix the filesystem using _fsck_ after the damage is caused.

### confpr
Prints out the current settings in conf.ini.

### superb
Will print superblock contents and can cause a duplicate block to be added to the free block list in the superblock on the disk. Fix the filesystem when you break it.

``` C
Usage: superb [-z][-v] [device]
-z - corrupt superblock freelist - uses conf.ini for settings
-v - verbose, default is to just print error messages

device - print superblock info for the device, -v not needed
superb with no arguments, prints out the superblock on the current
testing filesystem, and puts the verbose flag on.

The -z flag is needed to corrupts the filesystem.
```

### badino
Introduces various errors into the filesystem. They are intended to be  used and then the filesystem repaired before using another action.

The general format:

``` C
badino [-l][-z][-v] ID

ID is an action number in the range 0-5
Other config information in conf.ini
-z is needed to create the bad file or directory
-v verbose, print all text not just error messages
-l list action numbers and their actions

The -z flag is needed to break the file system. The code will
create a directory called baddir, a file called zero, and a file
called fiveblks - depending on what it wants to do.

badino -l prints

0: Clear link in empty file
1: Clear link in file with contents
2: Clear link in empty directory
3: Clear link in a directory with content
4: Create incorrect link count in a directory
5: Corrupt directory entry, set entry in directory to an illegal value

```

## Library

I put a lot of useful code into the [library](lib).
