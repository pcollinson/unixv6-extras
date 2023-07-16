# Setting the V6 system clock automatically

Once we have the time reading code working for V6, it's nice to be able to set the system clock when the system boots.

I decided to use the paper tape reader for this. A call to _timecmd.py_  writes a date setting command to the _ptr_ file by adding

``` sh
!timecmd.py
```
just before the _boot rk0_ statement. The ! prefix to the simulator runs a command.  The paper tape reader is now ready to read the tape contents.

It's read by the _settime.sh_ script that should be installed in _/etc_, it reads the paper tape onto a file in _/tmp_ and execute the _date_ command. To make this work at bootstrap time, add a call to _settime.sh_ to the start of _/etc/rc_ to set the clock.
