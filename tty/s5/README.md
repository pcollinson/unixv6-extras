# Install terms.s - system call glue

This short piece of assembler is the glue that allows C code to make the terms system call. I suggest that you do this after you have a working kernel.

It needs to be compiled and added to _/lib/libc.a_, preferably above  _cerror.o_ which it refers to. So simply adding the _terms.o_ file directly into the library cannot be done using the _ar_ program.

I recommend that you copy _/lib/libc.a_ to this directory and then run the _install_ script with

``` sh
install libc.a
```

The script uses the [aredit](../../aredit.md) program that should be installed, this is needed to position the terms.o
