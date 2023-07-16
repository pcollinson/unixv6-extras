# Run a PDP/11 processor test

Developed from DEC maindecs to perform instruction test on a PDP1140 with EIS by myself in 1978.

To compile:

``` sh
as proc1140.s;mv a.out proc1140.o;ld proc1140.o -lc -la
```

makes an a.out file. Yes - SIMH passes.
