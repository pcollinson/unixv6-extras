# Making time Y2K compliant

Applications on the system that deal with time use a library routine _ctime.c_. It contains _gmtime_ that computes meaningful human time from the kernel clock running in seconds from 1970. This is a 32 bit value, and the problems with it running out of bits are well known. The routine also contains the timezone information, the offset from GMT and the usual names, that are set to suit users on the east side of the USA. You are expected to edit this. There are other routines in _ctime.c_ that are wrappers for _gmtime_ delivering the decoded information in various formats.

The job of  _gmtime_ is to reduce seconds to years, months etc. It was failing because the two word 32-bit assembler division and remainder routines couldn't handle the set of large numbers that are needed to represent time 'today' .

The solution is to pick up the division and assembler routines from V7, which do work, and recode the _gmtime_ code to use them. I renamed them to make the names unique. There are actually bugs in the V7 assembler routines flagged by Henry Spencer in 1981 which I've fixed using the suggested changes.  This may or may not be necessary. The C code in the new _gmtime_  isn't particularly pretty when compared with the v7 code. There is new assembler module that contains the V7 code which i've called _ctsup.s_ and this needs installing in the C library (_libc.a_) in addition to the revised _ctime.c_ code. Of  course, this will all die in 2038.

## Installation

The _compile_ script is used to make a version of _date_ using the code in this directory, the intention is to provide a way of testing date setting using the new routines without having to change the C library.

However, other programs need recompiling to use the new _ctime_ routine and so _ctime.o_ and _ctsup.o_ should be installed in _/lib/libc.a_. The _install_ script will do that, it copies _/lib/libc.a_ to the current directory before compiling the library routines. After compiling, it edits the _libc.a_ copy using [_aredit_](../aredit), which needs to be installed. The idea is to replace the _time.o_ binary and append _ctsup.o_ after  it. The final _libc.a_ should be checked and installed by hand in _/lib_.

FInally, once the library has been updated, use _complib_ to compile _date.c_ using the library.

### Changes to _ctime.c_:

* The timezone is changed to GMT, and BST. The EST, EDT time is commented out. Also the UK rules for when daylight saving time starts are modified. Previous US code is just commented out.

* Use the new long division and remainder functions, so the  timestamp is now divided by 86400 to generate the number of days, and use the remainder to evaluate the hours, minutes seconds

* Years are then worked out (as before) by iterating through the years from 1970 using _dysize_ to generate the number of days in the year sequence. Initially I redid the _dysize_ code to use the approved algorithm. However the current one works until 2100, and since 32 bit time runs out in 2038, there seems little point in changing the code.

* The day of the week is evaluated by a neat routine that I found on Wikipedia, the reference is in the code.

* I have not attempted to deal with negative timestamps which V7 supports, but V6 never did.

### Changes to _date.c_

The year in the date is set using two digits. I've installed a heuristic rule: if the year is greater than 70 than the century is 1900, otherwise it's 2000.

## How the kernel remembers the date

When the system is booted, the system clock is set from the date stored in the superblock of the root filesystem. This is updated in all mounted devices by the _sync_ system call.

I've used the paper tape reader to set the time in V6 at startup, see [../setclock](../setclock).

## Programs that use _ctime_

There are not many programs that use _ctime_, the most prominent one is **ls**. In addition there are **ac**, **cron**, **dump**,  the Fortran compiler, **mail**, **nroff**, **restor**,  **sa**, **tp** and **who**..
