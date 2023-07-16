/ ctsup.s - support for ctime.c
/ This file contains replacements for the v6 ldiv calls
/ dealing with dates. I've opted to steal code from v7
/ which does the really hard stuff.
/
/ The problem with V6 ctime now is that the dates - ie
/ the number of seconds between 1970 and now has become
/ too large for the div instruction to work. Or at
/ least that's what I think.
/
/ So I'm lifting the code from V7 that does long division
/ which has problems fixed that were reported by Henry
/ Spencer in 1981
/
/ This is V7's aldiv, called by
/ aldiv(&dividend, quotient[0], quotient[1])
/ result in dividend
/ so is basically destructive
.globl	csv, cret

.globl	_v7aldiv

_v7aldiv:
	jsr	r5,csv
	mov	8.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,6.(r5)
	bne	hardldiv
	mov	4(r5),r1
	mov	2(r1),r2
	mov	(r1),r1
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
	com	r4
1:
	mov	r4,-(sp)
	clr	r0
	div	r3,r0
	mov	r0,r4		/high quotient
	mov	r1,r0
	mov	r2,r1
	mov	r0,-(sp)	/ Henry Spencer fix - hang onto r0
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ Fix cont.pick up the values
	mov	(sp),r0
	sub	r3,r0		/ this is the clever part
	div	r3,r0
	tst	r1
	sxt	r1
	add	r1,r0		/ cannot overflow!
1:
	tst	(sp)+		/ fix part 3
	mov	r0,r1
	mov	r4,r0
	tst	(sp)+
	bpl	9f
	neg	r0
	neg	r1
	sbc	r0
9:
	mov	4.(r5),r2
	mov	r0,(r2)+
	mov	r1,(r2)
	jmp	cret

/ The divisor is known to be >= 2^15 so only 16 cycles are needed.
hardldiv:
	clr	-(sp)
	mov	4.(r5),r0
	mov	2(r0),r2
	mov	(r0),r1
	bpl	1f
	com	(sp)
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	mov	6.(r5),r3
	bge	1f
	neg	r3
	neg	8.(r5)
	sbc	r3
	com	(sp)
1:
	mov	$16.,r4
1:
	clc
	rol	r2
	rol	r1
	rol	r0
	cmp	r3,r0
	bgt	3f
	blt	2f
	cmp	8.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	8.(r5),r1
	sbc	r0
	sub	r3,r0
	inc	r2
	sob	r4,1b
1:
	mov	r2,r1
	clr	r0
	tst	(sp)+
	beq	1f
	neg	r0
	neg	r1
	sbc	r0
1:
	mov	4.(r5),r2
	mov	r0,(r2)+
	mov	r1,(r2)
	jmp	cret


/ This is V7's alrem
/ seems like it's
/ alrem(&dividend, quotient[0], quotient[1])
/ result in dividend
/ Long remainder

.globl	_v7alrem

_v7alrem:
	jsr	r5,csv
	mov	8.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,6.(r5)
	bne	hardlrem
	mov	4.(r5),r0
	mov	2(r0),r2
	mov	(r0),r1
	mov	r1,r4
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	div	r3,r0
	mov	r1,r0
	mov	r2,r1
	mov	r0,-(sp)	/ Henry Spencer fix - hang onto r0
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ Fix cont.pick up the values
	mov	(sp),r0
	sub	r3,r0
	div	r3,r0
	tst	r1
	beq	9f
	add	r3,r1
1:
	tst	r4
	bpl	9f
	neg	r1
9:
	tst	(sp)+		/ fix part 3
	sxt	r0
	mov	4.(r5),r3
	mov	r0,(r3)+
	mov	r1,(r3)
	jmp	cret

/ The divisor is known to be >= 2^15.  Only 16 cycles are
/ needed to get a remainder.
hardlrem:
	mov	4.(r5),r0
	mov	2(r0),r2
	mov	(r0),r1
	bpl	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	mov	6.(r5),r3
	bge	1f
	neg	r3
	neg	8.(r5)
	sbc	r3
1:
	mov	$16.,r4
1:
	clc
	rol	r2
	rol	r1
	rol	r0
	cmp	r3,r0
	blt	2f
	bgt	3f
	cmp	8.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	8.(r5),r1
	sbc	r0
	sub	r3,r0
	sob	r4,1b
1:
	mov	4.(r5),r3
	tst	(r3)
	bge	1f
	neg	r0
	neg	r1
	sbc	r0
1:
	mov	r0,(r3)+
	mov	r1,(r3)
	jmp	cret
