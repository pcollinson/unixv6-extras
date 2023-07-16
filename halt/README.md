# Halt the running UNIX system safely

This code provides a small program and a system call to halt the UNIX V6 processor safely.

Back in 1976, I came to the conclusion that the V6 filesystem tended to get corrupted when the system was closed down.

I implemented program called _killunix_, that would:

1.  Find and kill all the running processes.
2.  Then make a system call that would sync the disks and then call a piece of assembler in the kernel that halted the processor.

Closure was clean, and filesystem problems became rare. I now find on a simulated PDP11, I've been taking the system up and down very frequently causing disk problems, so implementing this system was an early step.

I've recoded the _killunix_ program as _halt_ for simpler use on simh. I've called it _halt_ so if it's typed on the host system by accident, it does nothing.

To install this system you need to:

1.  Run installsys, this:
    a. Installs _m40.s_ into _/usr/sys/conf/m40.s_. The code that's already added into  _m40.s_ is found in _ins_m40.s_. The original _m40.s_ is in  the _orig_ directory.
	NB I've not edited the _m45.s_ code because  I've not used the code on a PDP11/45.
	Also, the version of _m40.s_ here also contains the _zapc_ function   needed to support the [new terminal driver](../tty).

    b. Replaces _/usr/sys/ken/sys4.c_ with _sys4.c_. A new function from _ins_sys4.c_ is appended at the end of the original file.

	c. Replaces /usr/sys/ken/sysent.c with sysent.c. This nominates an  unused system call id, 49 to link to the stopunix() function in   sys4.c. If you change the number, alter stopunix.s.

2.  Now recompile and install your kernel. The scripts I use to do this  can be found on _[../sys/conf](../conf)_.


3.  Compile the halt command: _compile_  will generate the command. Install  it in /bin and halt will now take your machine  down relatively safely.

## The _halt_ command

When the command is run, it lists the process ids of the processes that it kills. You may get the output from any shell you are running as it gets nuked.

The _halt_ command gets addresses from _/unix_ and reads data from the running kernel. If you've update the _/unix_ file, then processes will not be killed because the addresses it needs are not available but the processor will still halt.
