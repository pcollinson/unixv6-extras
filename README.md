# unixv6-extras - making V6 more usable

A long long time ago in a university not too far away, in Canterbury, UK, I was a newish teaching/research member of the Computer Science department. The mainframe computer was about to be replaced and I was suddenly the person asked to get this brand new system, Unix on the PDP11/40, working. I remember sitting down at the [DecWriter](https://en.wikipedia.org/wiki/DECwriter), typing a few characters and there was this system, the like of which I'd never used before. It was interactive, it was multi-user, and wow, it worked in both upper and lower case. It was one of the moments where life changes profoundly.

A friend recently pointed me at [SIMH](http://simh.trailing-edge.com) which provides a simulator for the PDP11 (and several other computers) and that could run Unix Version 6. To provide something 'real', I decided to resurrect some of the changes that we made to V6 to tailor it to our local environment.

I must acknowledge the work of the folks at [The Unix Heritage Society](https://www.tuhs.org) who have made it their business to collect old source and versions of the Unix system. Along with all the people that made tapes and so left us some history. The  [TUHS archive](https://www.tuhs.org/Archive/) is a goldmine for people looking back at what was done.

## What's here

All of the below are fully documented with installation instructions. Directories called _orig_ contain the original code, shell scripts contain compilation and installation commands.

This list is pretty much in the order that I worked on things.

* __[simh](simh)__ - getting  started with SIMH to run Unix V6 on a PDP11/40 simulation.

* __[sh](sh)__ - a slightly changed version of the Unix shell that allows me to type _cd_ rather than _chdir_.

* __[uxtp](uxtp)__ - I needed something to get files in and out of V6 so I decided to implement a version of _tp_, V6's magnetic tape archive program. It's proved to be a fast way to get whole file trees in and out of V6. It seems to work for V7 too.

* __[fsck](fsck)__ - The Unix file system can break and sometimes needs fixing. Clem Cole sent me a version of _fsck_  written for a later C compiler. I back ported it and wrote some tests to make sure it  works.

* __[sysconf](sysconf)__ - Scripts to assist with rebuilding and installing the kernel.

* __[halt](halt)__ -  Back in the 1970's, I realised that the file system mostly failed after Unix  was shut down. I created a command that would kill all the running processes, and added a system call to the kernel to halt the system safely. Doing this made the filesystem breakages mostly disappear. You tend to take a simulated system up and down considerably more frequently than a physical system, so re-implementing this feature was a must.

* __[trees](trees)__ - vanilla V6 doesn't handle trees of directories and files very well, you have to do too much typing. This section contains two programs: _cptree_ copies a tree, and _rmtree_ is a shell script to remove a tree. There's also some help here about the _rm_ command.

* __[man](man)__ - I was really missing the _man_ command, I took the code from _1bsd_ written by Bill Joy and  back ported the C. i included a program of his to suppress multiple blank lines _ssp_, I put some work into the macros used by _nroff_ to display manual pages.

* __[tty](tty)__ - The V6 terminal driver was designed to work with printing devices and really doesn't work well with screens. I hated it in the 70's and that hate hasn't gone away. A new _tty.c_ appeared (I think originally from UCLA) and was worked on by many people in the UK. The version here allows you to use the delete key to delete characters, and for the characters to disappear as you delete. For me, this was a game changer and one of the reasons I started this project. The driver needs to get out there.

* __[time](time)__ - Programs displaying time are apparently stuck in the 1970s. Trying to change the _date_ command to accept dates post 2000 won't work because today the seconds since 1970 are too large a number for the division the current code uses. I've reworked the _ctime_ routines to get the mathematics working (with help from V7). A few commands need recompiling to use the code.

* __[aredit](aredit)__ - While working on installing the new _ctime_ code I came across a deficiency in V6's _ar_ program. This new program allows you to safely install binaries in the middle of archives.

* __[setclock](setclock)__ - Set the system clock at boot time. There may be a better way of doing this.

* __[rdar](rdar)__ - Several old tapes on  [The TUHS archive](https://www.tuhs.org/Archive/) have files called _cont.a_ which are old archive files with an unknown magic number at the start. It's unclear what can unpack these, so I wrote some code.

* __[Notes](notes)__ - some notes on running things.
    * [Notes on ar](notes/ar.md) - some discussion on the problems presented by the _ar_ program.
	* [The V6 Compiler](notes/c-compiler.md) - why C on V6 might not be what you expect.
	* [Using retro-fuse](notes/retro-fuse.md) - Retro-fuse allows you to mount a V6 file system in a file as a working filesystem on your Mac or Linux system
	* [Using stty](notes/using-stty.md) - How you can cope with V6 before changing the terminal driver.
    * [Creating a user account](notes/user-acct.md) - Make yourself a user


## Licenses

Several licenses apply to this repo - see [License](License.md).
