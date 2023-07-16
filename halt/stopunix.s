.globl	_stopunix
stopunix = 49.

_stopunix:
	mov	r5,-(sp)
	mov	sp,r5
	sys	stopunix
	mov	(sp)+,r5
	rts	pc
