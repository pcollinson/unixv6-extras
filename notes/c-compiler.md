# The V6 C compiler

The V6 C compiler is written in assembler, which to my mind is no mean feat. There are several differences between what it provides and what was later supplied for V7, which became K&R C. There was an intermediate version, which I think we ended up running on V6 at Kent, however, my rules for this exercise was to use existing code to supply interested users with a working kernel that implements some of the changes that made V6 more usable.

## The pre-Processor

* To get the pre-processor invoked on your C source, you need to add a '#' as the first character of the file.
* The pre-processor only has _#define_ and _#include_. The _include_ function doesn't support the _<...>_ syntax that allowed for centralised header files, if you look in the code, you'll find repeated definitions of many system structures.
* There is no conditional compilation: _#ifdef_ and friends don't exist.

## The language
These are the differences I've found, there may be more.

* What is now the ```+=``` operator (and friends) was ```=+```, I remember complaining about this when the new compiler came along, it was a portability issue for lots of code. The reason for the change was to assist in parsing ```-=```, which as ```=-``` could cause problems: there's only a space between ```a =- 3``` and ```a = -3```,   while ```-=``` is a symbol that's likely to be unique.

* At the time we all knew about _register_ variables which allowed you as the coder to say "this is an important pointer - please keep it in a register because then you can generate good code". Remember that C is the best high level assembler for the PDP11. In V7 C you can include _register_ in the function definition. In V6, you need to replace the code by a direct assignment

``` C
	FAIL:
	afunction(a, b)
	register char *a;
	register char *b;


	FINE:
	afunction(aa, bb)
	char *aa;
	char *bb;
	{	register char *a;
		register char *b;

		a = aa; b = bb;
```

* There is some _long_ support in the compiler, but it's not mentioned. I suspect that it was not fully implemented. C returns values from functions in _r0_, and long values in _r0_ and _r1_. I couldn't find a way of getting the compiler to pick up the two values on return and store them somewhere sensible.

* If you wanted fully signed 64 bit working, then you needed to use _char *p_. The _ls_ command uses this. My new _ctime.c_ code doesn't, I decided that times should be signed 32 bit numbers, which is effectively what they were on V6.

* I wanted to use a static array in function that was full of pre-defined values, this failed. The solution was to make it global and prefix the name by '_' so it didn't pollute the namespace.

``` C
	dayofweek(y, m, d)
	int y, m, d;
	{
		static int t[]  {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

```
