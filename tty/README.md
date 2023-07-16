# Unix V6 Replacement terminal driver

The original V6 terminal driver was designed to be used with printing devices and doesn't play too well with the 'glass terminals' that we all use today. Personally I find that I don't get on with the use of '#' to delete a character, and 'delete' to send the interrupt signal. This was as true in the 1980's as it is now.

Luckily, help is at hand. In the late 1970's, various versions of an alternative driver began circulating. It started at UCLA,  and was passed about the UK Universities that were using UNIX: Queen Mary College London, University of Glasgow, Swansea University, and to myself at the University of Kent.  We all hacked them to fit into our local environment. The version I have adapted here came from Glasgow and can be found on a tape rescued by [Henry Spencer](https://www.tuhs.org/Archive/Applications/Spencer_Tapes/uk1.tar.gz) on the [www.tuhs.org archive](https://www.tuhs.org/Archive).

By now, I had created my own user account, which means I login to my home directory, see [user_acct](../notes/user_acct.md).

## Installation steps

* Install and build the new kernel code replacing the old _tty.c_, its header and drivers. This uses files in the _sys_ directory. For more on that see its [sys/README](sys).

* Run _mkestty_ to compile _estty_ , install in your home directory.  Once you've installed the kernel code, you'll be surprised that the new erase/kill characters you have installed in the kernel are not set up when you login. This will continue until you replace _/etc/getty_ and _/bin/login_. This program can be compiled and installed to set the values to what are now the defaults. It's a temporary measure but makes your life easier. On the plus side, your Interrupt character (by default ^c) will work now.

* If you've not built your system in _/usr/sys_, then you'll want to install _sys/deftty.h_ in _/usr/sys_. You can now compile and install the other programs without messing with your system. You will need to change _/usr/sys_ at some point, this is where most of the code looks to find definitions.

* Install the glue code in _s5_ to put the _terms_ system call into _/usr/lib/libc.a_. See [s5/README](s5). This is needed to some of the other new programs. The _rawst.c_ program uses this system call and will dump out the values it returns.

* With _terms.o_ installed run _compile_

* Install _getty_ in _/etc/getty_,  reboot and check that it works.

* Install _login_ in  _/bin/login_,  reboot and check that it works. Now the default character controls will all be available to you on login.

* Install _stty_ in _/bin/stty.c_. You can compile and run this without installing it.

* The _/bin/ps_ command uses _/usr/sys/tty.h_, and needs to be recompiled and re-installed, once your system code is set up.

* Consider installing the manual pages in _man_. The V6 system doesn't come with working _man_ command, I supply one, see [../man/README](../man).

## Directories

* [man](man) - manual pages

* [s5](s5) - Glue code for the _terms_ system call

* [sys](sys) - Files for installing the new terminal interface and associated drivers into your kernel.
