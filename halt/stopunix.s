.globl	_stopunix
.globl cerror

stopunix = 49.

_stopunix:
	mov	r5,-(sp)
	mov	sp,r5
	sys	stopunix
        bec     1f
        jmp     cerror
1:
        clr     r0
        mov     (sp)+,r5
        rts     pc
