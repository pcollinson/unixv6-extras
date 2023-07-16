# _fsck_ for V6

This is a version of _fsck_ originally supplied to me by Clem Cole (from an early BSD system?). It will now compile and run on UNIX V6. The changes are all marked.

This version is not as forgiving as more recent versions of _fsck_. For example, it cannot recover files and put them into a _lost+found_ directory. I think that it's probably more destructive. However, it's easier to use than the standard _icheck_ and _dcheck_ programs.

Standard caveats still apply - don't run this on a mounted file system. With the simulator you have the ability to copy your disk containing the root filesystem to say _rk3_ and check it. If it needs fixing, you can copy it back into place.

V6 will print console messages when filesystems break, but you need to ensure that the virtual switches on the PDP11 are non-zero, add

``` C
; sr needs to be non zero to get console output
; change last digit to 1 to get magtape core dump
; on panics
dep sr 177700
```
to your SIMH configuration file. The number I chose is a random selection. Incidentally, If you want to boot the system into single user mode then

``` C
dep system sr 173030
```
in the startup file does the trick.

The _compile_ script will compile the code and create an _fsck_ binary.

The [tests](tests) directory contains several testing programs that can induce a file system problem.
