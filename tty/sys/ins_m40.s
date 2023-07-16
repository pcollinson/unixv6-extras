//  character from the end of clist

.globl   _zapc

_zapc:
	mov	2(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	bis	$340,PS
	bic	$100,PS		/ spl5

	tst	(r1)		/ empty
	beq	9f
	mov	4(r1),r2	/ r2 points at last char +1
	dec	r2
	movb	(r2),r0		/ fetch char
	bic	$!377,r0
	cmp	$377,r0		/ is char a CDELIM
	beq	9f

	dec	(r1)+		/ count
	bne	4f		/ queue now empty ?
	clr	(r1)+		/ zero first and last char
	clr	(r1)
	bic	$7,r2		/ address of block
	br	5f
4:
	mov	r2,2(r1)	/ update last char
	dec	r2		/ last block is empty ?
	dec	r2
	bit	$7,r2
	bne	3f

	mov	(r1)+,r3	/ afraid so, get
	bic	$7,r3		/ address of first block
1:	cmp	(r3),r2		/ trog down the links,
	beq	2f		/ until r3 points at r2
	mov	(r3),r3
	br	1b
2:	clr	(r3)		/ unlink block r2
	add	$10,r3		/ set r3 to char beyond block
	mov	r3,(r1)		/ update last char
5:
	mov	_cfreelist,(r2)	/ free r2
	mov	r2,_cfreelist

3:
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc
9:
	mov	$-1,r0		/ failed
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc
