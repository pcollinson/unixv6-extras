# Getting started with V6 on Simh

I got my start for running UNIX on the SIMH PDP11 simulator by finding [Installing UNIX v6 (PDP-11) on SIMH](http://gunkies.org/wiki/Installing_UNIX_v6_%28PDP-11%29_on_SIMH) which is an excellent step-by-step guide to installing from tape. If you want to do that, look in the _fromtape_ directory, you'll find the two command files that the script uses: _tboot.init_ is the tape bootstrap using _dist.tap_  and _dboot.ini_ is the single user system bootstrap.

Alternatively. you can use run _pdp11 unixv6.ini_ with the three RK05 images here. These copies were made directly after extracting them from the tape. The _unixv6.ini_ is the file I am using. It enable two mag tape devices, four RK05 devices, 8 terminal lines that can be accessed via _telnet_ to _localhost:5555_.

To get a working simulator, _git clone_ [https://github.com/simh/simh](https://github.com/simh/simh), change into the directory and run ```make pdp11```, you'll find the _pdp11_ command in the _BIN_ directory.

Here's what happens and how you boot unix:

``` sh
$ pdp11 unixv6.ini

PDP-11 simulator V4.0-0 Current        git commit id: 37bc857b
Disabling XQ
./unixv6.ini-10> attach tm0 tm0.tap
%SIM-INFO: TM0: creating new file
%SIM-INFO: TM0: Tape Image 'tm0.tap' scanned as SIMH format
./unixv6.ini-11> attach tm1 tm1.tap
%SIM-INFO: TM1: creating new file
%SIM-INFO: TM1: Tape Image 'tm1.tap' scanned as SIMH format
./unixv6.ini-18> attach rk3 rk3
%SIM-INFO: RK3: Creating new file: rk3
./unixv6.ini-20> attach ptr ptr
%SIM-INFO: PTR: creating new file
./unixv6.ini-27> att dci 5555
%SIM-INFO: Listening on port 5555
@unix
mem = 1026
RESTRICTED RIGHTS

Use, duplication or disclosure is subject to
restrictions stated in Contract with Western
Electric Company, Inc.

login: root
#

```
You'll see that it creates empty files for the devices that didn't originally exist in this directory: _tm0.tap_, _tm1.tap_, _rk3_ and _ptr_. It waits at the '@' prompt and you need to type _unix_ to boot the system. It then prints the old license which tells you that the kernel is printing messages to the console, controlled by the non-zero setting in the switch register (see comments in _unixv6.ini_). It's good to have this on, you'll now get hardware error messages.

You can login as _root_,  but in general it's a good idea to set up a login as you - see [user-acct in notes](../notes/user-acct.md).

To  close down, type _sync_ several times to make sure the buffered by the kernel for the disks is all written out. Then type ^E to get back to the simulator. The _co_ command to the simulator will continue it running and _q_ exits.
``` sh
# sync
# sync
# sync^E
Simulation stopped, PC: 002650 (MOV (SP)+,177776)
sim> q

```
I added the ^E above, you don't get to see this.

If you want to use the paper-tape reader, then you'll need to enable it in the kernel. See [sysconf](../sysconf).

The new _rk3_ disk is blank and needs a filesystem made on it, see [mkfs](../mkfs).

To get this disk to mount automatically at boot time, you need to edit  _/etc/rc_ on the system, it's a shell script that's run just before the system goes multi-suser.

## Booting single user

You can boot the system into single user mode using the _usinglev6.ini_. It sets a magic number in the switch register that means that the kernel won't boot into multi-user mode. When this system boots, it's aimed at being used on a terminal that only supports upper case, typing

``` sh
STTY -LCASE
```
shifts things into lower (and upper) case.
