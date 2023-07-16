# System maintenance

This section contains useful files for building and maintaining the system.

##Install and rebuild kernel

You need to be a bit suspicious of the _run_ scripts found in several places in the distribution. They are intended to rebuild the entire distribution and sometimes they are not helpful when you want to just rebuild a small number of objects. I use a small set of files that I install on _/usr/sys/conf_ to make rebuilding the kernel easier.

* __devices__:
The file is used in the rebuild script. It contains the devices I want to install in the system. This file is read by the _mkconf_ command to create the table of devices. The file is a list of devices ending with 'done'.  The lines populate  _c.c_ containing the device table.

```
	rk - RK05 driver
	tm - tape driver
	8dc - 8 addition terminals accessible from telnet to localhost 5555
	pc - Paper tape reader/writer
	done
```

* __rebuild__:
Script to rebuild the system into a.out. Files from   _ken_ will be compiled and stored in _sys/lib1_, and those from _dmr_ will be placed into _sys/lib2_. The _ar_ commands in the script name the files explicitly so it is safe to delete _lib1_ and _lib2_. You may need to delete the files if a component is removed and doesn't need to be present in the library. In addition any new file will be added.

* __installux__:  (installed in _/usr/conf_ as _install_)
    * moves _/unix_ to _/unix.bak_
    * copies a.out /unix _/usr/conf_

* __replace__:
Script to copy _a.out_ _/unix_

* __install__:
Script to copy _/unix_ to _/unix.bak_ and then copies _a.out_ to _/unix_.

* __makenodes__
A shell script that can make nodes in _/dev_ matching the devices that are in the _devices_ file.
