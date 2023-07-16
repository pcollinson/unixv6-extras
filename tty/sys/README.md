# Install new tty driver & device interfaces

This version of _tty.c_ started life in UCLA and made its was in into various UK universities, all of whom hacked it for local use. This version is from the University of Glasgow, chosen because it fits in somewhat better with current usage.

## Summary of changes

The original tty driver for V6 used three character queues: one for output, and two for input. Characters from the devices were placed on an input queue and processed to the 'canonical' queue when the user pressed return. When return was entered, the canonical queue was passed into user space processing  all the delete characters and other editing characters. The new code uses a single input queue, and the characters used to modify the input data operate directly on the queue. There is a new assembler routine _zapc_ that removes characters from the end of the queue to assist with the editing.

The change meant that the _tty_ structure in _tty.h_ was changed significantly and a new system call was introduced to transfer the new data in and out of the kernel, along with the old that was set and retrieved by _stty_ and _gtty_ respectively. There is lots of code expecting to use these two system calls, and the system is backwards compatible.

As well as _tty.c_ essentially being replaced the drivers that use it need slight alterations. The main ones of interest are _kl.c_ which drives the console, _dh.c_ driving the addtional terminal lines. I found that I had written a version of _dc.c_ for Kent which is included.

However, the change means that not all the terminal devices available in _sys/dmr_ will compile. These are generally  esoteric serial interfaces to various devices. These files need to be removed from _dmr_ so that a file expansion ```cc -c -O *.c``` can continue to work. A script _step1.sh_ will move the files into _dmr/unused_.

## Things I've done while working on this

* I've removed parity checking. It's commented out, so you can get it back.
* I've moved the control characters into a new file _deftty.h_ so this can be used in the various programs that need the information, often  the characters are hard coded in the sources.
* I've messed about with the formatting of some of the C, to make it more into the style of the system.

## Files

* _dc.c_:  DC11 driver code. Written by me in 1976 to handle what was probably (at the time) illegal modem.
* _deftty.h_:   new file with control characters and modes abstracted from _tty.h_.
* _dh.c_:  DH11 driver
* _ins_m40.s_:  The _zapc_ code needed to be added to _m40.s_.
* _kl.c_:  KL11/DL11 driver.
* _m40.s_: Edited version of the file containing the _zapc_ code and also the stopunix system call from the new [halt command](../../halt).
* _sysent.c_: System call table containing the new _terms_ and _stopunix_ entries.
* _tty.c_: New terminal driver.
* _tty.h_: New header file.

Original V6 files can be found in the _orig_ directory.

## Installation

These scripts all need an argument which is the destination in the filesystem you are changing. This is to stop running a script without intent and then wondering how to get back to where you were. You''ll get the same error message each script is called with no argument, or the target directory cannot be opened.

Before you start this, I do recommend that you stop the simulator and take a copy of the _rk0_ disk (or any other disk you are planning to edit). I first did this on my spare _rk3_ having used _cptree_ (see the [README](../../tree)).

You will need to run these scripts as root. They must to be run from this directory changing files in _/usr/sys_ or anywhere else.  They are all as defensive as possible and can be run more than once.

* __step1.sh__
provides backup files. It creates directories called _orig_ in the various directories that contain files that will be overwritten or removed.  Files in any _orig_ directory are not overwritten.

* __step2.sh__
removes files from _dmr_ having backed them up in _orig_, it will not remove files unless a backup exists.

* __step3.sh__
installs new files in the various system directories, files are not overwritten unless a backup exists.

* __step4.sh__
is a verification step comparing the files in this directory with the versions in the destination tree.

You can now rebuild your new kernel. Scripts to assist with this can be found on [_../../sysconf_](../../sysconf). Make sure you keep a copy of your running kernel so you can revert in case of problems.
