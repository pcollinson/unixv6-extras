/ proc1140
/ Developed from DEC maindecs to perform instruction test
/ on a PDP1140 with EIS
/ Peter Collinson UKC 17/1/78
/ Compile thus:
/ as proc1140.s;mv a.out proc1140.o;ld proc1140.o -lc -la

.globl	_printf
	setd
	clr	passno		/ clear pass number
1:	inc	passno
	jsr	pc,tcycle	/ call tst cycle code
	cmp	$12500.,passno
	bne	1b
	sys	exit

/ error routine - calling printf
/ called by
/ 	jsr	pc,error ;<errno>

error:
	mov	r0,-(sp)
	mov	2(sp), r0		/ pick up argument
	inc	errct			/ increment the error count
	mov	(r0), -(sp)
	mov	$errst, -(sp)
	jsr	pc,_printf
	mov	passno,2(sp)
	mov	$psst,(sp)
	jsr	pc,_printf
	mov	errct,2(sp)
	mov	$ctst,(sp)
	jsr	pc,_printf
	cmp	(sp)+,(sp)+
	mov	(sp)+,r0	/ pick up old r0
	add	$2,(sp)
	rts	pc
	.data
errst:	<Error No: %d\n\0>
psst:	<Pass  No: %d\n\0>
ctst:	<Error ct: %d\n\0>
	.even
	.bss
passno:	. = . + 2
errct:	. = . + 2


/ main test cycle
	.text
tcycle:



/ First check branch instructions
	clc
	bcs	1f
	bvs	1f
	beq	1f
	bmi	1f
	blt	1f
	ble	1f
	blos	1f
	bhi	2f
/*******************************************************************
1:	jsr	pc,error ;1
/*******************************************************************
2:

/ set condition codes to 1010
	sev|sen
	bvc	1f
	blt	1f
	ble	1f
	bge	2f
/*******************************************************************
1:	jsr	pc,error ;2
/*******************************************************************
2:

/ set condition codes to 1111
	sez|sec
	bne	1f
	bgt	1f
	bhi	1f
	ble	2f
/*******************************************************************
1:	jsr	pc,error ;3
/*******************************************************************
2:

/ check unary word ops using address modes 3 and 4
	.bss
	.=.+2
9:	.=.+2
	.text
	mov	$9b,r3
	clr	(r3)
	mov	r3,r0
	tst	-(r3)
	mov	r0,(r3)
	mov	r3,r4
	clc
	tst	*(r3)+
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;4
/*******************************************************************
2:

	sec
	ror	*-(r3)	/ (r0) = 100000, cc = 1010
	bcs	1f
	bvc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;5
/*******************************************************************
2:
	clc|clv|clz|cln
	asr	*(r4)+	/ (r0) = 140000, cc = 1010
	bvc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;6
/*******************************************************************
2:
	cln
	asl	*(r3)+	/ (r0) = 10000,cc = 1001
	bcc	1f
	bvs	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;7
/*******************************************************************
2:
	sec|sev|sez|sen
	dec	*-(r4)	/ (r0) = 077777, cc = 0010
	bcc	1f
	bvc	1f
	beq	1f
	bpl	2f
/*******************************************************************
1:	jsr	pc,error ;8.
/*******************************************************************
2:
	neg	*-(r3)	/ (r0) = 100001, cc=1001
	bcc	1f
	bvs	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;9.
/*******************************************************************
2:
	sev
	com	*(r4)+	/ (r0) = 077776, cc = 0001
	bcc	1f
	bvc	2f
/*******************************************************************
1:	jsr	pc,error ;10.
/*******************************************************************
2:
	inc	*(r3)+	/ (r0) = 077777, cc = 0001
	bcc	1f
	bpl	2f
/*******************************************************************
1:	jsr	pc,error ;11.
/*******************************************************************
2:
	adc	*-(r4)	/ (r0) = 100000, cc = 1010
	bcs	1f
	bvc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;12.
/*******************************************************************
2:
	clc|clv|clz|cln
	rol	*(r4)+	/ (r0)=000000,cc=0011
	bcc	1f
	bvc	1f
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;13.
/*******************************************************************
2:
	inc	*-(r3)	/ (r0)=000001,cc=0001
	sbc	*-(r4)	/ (r0)=000000,cc=0100
	bcs	1f
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;14.
/*******************************************************************
2:


/ Check Unary byte ops using address modes 3 and 5
	.data
	.=.+2	/ Even address word
	.=.+2	/ Odd address word
9:	.=.+2	/ Data word
	.text
	mov	$9b,r2	/ set r2 to point at data word
	mov	r2,r0		/ set r0 as reference pointer
	clr	(r0)		/ clear it
	tst	-(r2)		/ back r2 up to even address word
	tst	-(r2)
	mov	r0,(r2)+	/ load address
	inc	r0		/ odd byte address
	mov	r0,(r2)+	/ load odd byte address
	mov	r2,r0		/ reset r0
	mov	r2,r4

	comb	*-(r2)	/ (r0) = 177400,cc=1001
	bcc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;15.
/*******************************************************************
2:
	tstb	*-(r2)	/ (r0)=177400,cc=0100
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;16.
/*******************************************************************
2:
	sev
	asrb	*-(r4)	/ (r0)=177400,cc=1001
	bcc	1f
	bvs	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;17.
/*******************************************************************
2:
	incb	*(r2)+	/ (r0)=177401,cc=0000
	bcc	1f
	bpl	2f
/*******************************************************************
1:	jsr	pc,error ;18.
/*******************************************************************
2:
	clc
	rorb	*-(r4)	/ (r0)=177400,cc=0111
	bcc	1f
	bvc	1f
	bne	1f
	bpl	2f
/*******************************************************************
1:	jsr	pc,error ;19.
/*******************************************************************
2:
	aslb	*(r2)+	/ (r0)=177000,cc=1001
	bcc	1f
	bvs	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;20.
/*******************************************************************
2:
	adcb	*-(r2)	/ (r0)=177400,cc=1000
	bcs	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;21.
/*******************************************************************
2:
	sec|sev|sez|sen
	rolb	*(r4)+	/ (r0)=177401, cc=0000
	blos	1f	/ branch if c or z set
	bvs	1f
	bpl	2f
/*******************************************************************
1:	jsr	pc,error ;22.
/*******************************************************************
2:
	swab	*-(r2)	/ (r0)=000777, cc = 1000
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;23.
/*******************************************************************
2:
	sec
	sbcb	*(r4)+	/ (r0)=00377,cc=0100
	bcs	1f
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;24.
/*******************************************************************
2:
	negb	*(r2)+	/ (r0)=000001
	decb	*-(r2)	/ (r0)=000000,cc=0101
	bcc	1f
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;25.
/*******************************************************************
2:


/ check binary word ops using address modes 3 and 5
	.data
6:	.=.+2		/ address of source data
7:	.=.+2		/ address of destination data
8:	.=.+2		/ source data
9:	.=.+2		/ destination data
	.text
	mov	$8b,r3	/ r3 points to source data
	mov	$6b,r2	/ set up source pointers r2 points at address word
	mov	r3,(r2)	/ r2 used as working pointer
	mov	$9b,r4	/ r4 points to destination data
	mov	$7b,r5	/ r5 points at address word
	mov	r4,(r5)	/ r5 used as working pointer
	clr	(r3)	/ preset the data
	clr	(r4)

	sec|sev|sez|sen
	clz
	sub	*(r2)+,*(r5)+	/ (r3)=000000,(r4)=000000, cc = 0100
	bcs	1f
	bvs	1f
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;26.
/*******************************************************************
2:
	bis	$100000,*-(r2)	/ (r3)=100000
	add	$1,*-(r5)	/ (r4)=000001
	sub	*(r2)+,*(r5)+	/ (r3)=100000,(r4)=100001,cc=1011
	bcc	1f
	bvc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;27.
/*******************************************************************
2:
	neg	(r4)		/ (r4)=077777
	bit	*-(r2),*-(r5)	/ (r3)=100000,(r4)=077777
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;28.
/*******************************************************************
2:
	cmp	*(r2)+,*(r5)+
	bvs	2f
/*******************************************************************
1:	jsr	pc,error ;29.
/*******************************************************************
2:
	com	*-(r2)
	clc|clv|clz|cln
	add	*(r2)+,*-(r5)
	bvc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;30.
/*******************************************************************
2:
	sec
	bic	*-(r2),*(r5)+	/ (r3)=077777,(r4)=100000
	bcc	1f
	bmi	2f
/*******************************************************************
1:	jsr	pc,error ;31.
/*******************************************************************
2:
	com	*-(r5)		/ (r4)=077777
	cmp	*(r2)+,*(r5)+
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;32.
/*******************************************************************
2:

/ check binary byte ops using address modes 3 and 5
	.data
	.=.+2		/ address of source data (even byte)
	.=.+2		/ address of source data (odd byte)
	.= .+2		/ address of destination data (even byte)
	.=.+2		/ address of destination data (odd byte)
	.=.+2		/ source data word
9:	.=.+2		/ destination data word
	.text
	mov	$9b,r0		/ r0 = addrees of dsst data
	mov	r0,r3		/ r3 = address of dest data
	mov	r3,r5		/ r5 = address of dest data
	tst	-(r3)
	mov	r0,-(r3)	/ set up dest address
	inc	(r3)		/ odd address
	mov	r0,-(r3)	/ even address
	mov	r3,r4
	tst	-(r0)		/ r0 = address of source data
	mov	r0,-(r4)	/ set up source addresses
	inc	(r4)		/ odd address
	mov	r0,-(r4)	/ and even address

	sec
	mov	$177001,*(r4)+
	movb	$200,*(r4)+	/ source data = 100001
	movb	*-(r4),*(r3)+
	movb	*-(r4),*(r3)+	/ dest data = 000600
	bcs	2f
/*******************************************************************
1:	jsr	pc,error ;33.
/*******************************************************************
2:
	cmp	$600,(r5)	/ check destination data
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;34.
/*******************************************************************
2:
	cmp	-(r3),-(r3)	/ point	r4 back at even byte
	bisb	*(r4)+,*(r3)+
	bisb	*(r4)+,*(r3)+	/ dest data = 100601
	cmp	$100601,(r5)		/ test it
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;35.
/*******************************************************************
2:
	bicb	*-(r4),*-(r3)
	bicb	*-(r4),*-(r3)
	bitb	*(r4)+,*(r3)+
	bne	1f
	bitb	*-(r4),*(r3)+
	bne	2f
/*******************************************************************
1:	jsr	pc,error ;36.
/*******************************************************************
2:
	cmpb	*(r4)+,*-(r3)
	bne	1f
	cmpb	*(r4)+,*-(r3)
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;37.
/*******************************************************************
2:

/ check jmp instructions
	mov	pc,r0
	add	$12,r0
	sec|sev|sen|sez
	jmp	(r0)
2:	br	3f
	cln
	br	2b
3:
	bcc	1f
	bvc	1f
	bne	1f
	bpl	2f
/*******************************************************************
1:	jsr	pc,error ;38.
/*******************************************************************
2:
	clr	r2		/ set indicator
	mov	pc,r4		/ set up jmp register
	mov	r4,r0		/ and check register
	br	1f
	com	r2		/ complement indicator
	br	2f
1:	cmp	(r4)+,(r4)+
	tst	(r4)+
	jmp	-(r4)		/ use r4 as address
	inc	r2		/ check indicator
	bne	1f
	cmp	(r0)+,(r0)+
	cmp	r0,r4		/ check auto-dec r4
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;39.
/*******************************************************************
2:

/ check jsr instructions
	mov	$9f,r2		/ form destination address
	sec|sev|sez|sen
	clv
	jsr	r5,(r2)		/ goto 3: via r2
7:	tst	r2		/ check indicator
	bne	1f		/ r2 should be = 0
	br	2f
8:	rts	r5		/ return from subroutine
9:	bcc	1f		/ check that jsr did not
	bvs	1f		/ affect ccs
	bne	1f
	bpl	1f
	clr	r2		/ clear indicators
	mov	$7b,r4	/ get return address
	br	8b
/*******************************************************************
1:	jsr	pc,error ;40.
/*******************************************************************
2:

/ END OF MAIN INSTRUCTION TESTING CODE


/ EIS INSTRUCTION TESTS
	.data			/ data area
b:	125252
	b			/ address of b
	052525
.=b+10
a:	-1
	a+4
	125252
	a+10
	052525
c:	0
	c
.=c+10
temp:	0
	temp
d:	0
y:	0
x:	.=.+22.
plier:	0
spoint:	.=.+2
	.text
	mov	$10,r2		/ set up index register
	mov	$52525,r1	/ load source operand
	mov	$-1,temp
	xor	r1,c(r2)	/ xor source dest
	cmp	$125252,temp
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;41.		/ xor failed
/*******************************************************************
2:
 / test xor using index deferred
	mov	$125252,r3
	mov	$10,r4
	mov	$y,x+10
	mov	$-1,y
	xor	r3,*x(r4)
	cmp	$52525,y
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;42.
/*******************************************************************
2:
/ test sob for branch
	br 	9f
8:	br	2f
9:	mov	$10,r1
	sob	r1,8b
/*******************************************************************
1:	jsr	pc,error ;43.
/*******************************************************************
2:
/ test  sob for branch
	br	3f
/*******************************************************************
1:	jsr	pc,error ;44.
/*******************************************************************
2:
	br	6f
3:	mov	$1,r1
	sob	r1,1b
6:
/ test mark instruction
mk=6401
	mov	sp,spoint		/ store current stack location
	clr	r1
	mov	$7,-(sp)
	mov	$1,-(sp)		/ push parameters onto stack
	mov	$mk,-(sp)		/ mov mark 1 onto stack
	mov	sp,r5			/ load parameter pointer
	jsr	pc,mk0			/ goto to subroutine
	br	3f
mk0:	mov	2(r5),r1		/ get the parameter
	rts	r5			/ exit from subroutine
3:
	cmp	$1,r1			/ did subroutine get the parameter
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;45.
/*******************************************************************
2:
	cmp	spoint,sp		/ is stack pointer correct
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;46.
/*******************************************************************
2:
	cmp	$7,r5			/ . is r5 correct
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;47.
/*******************************************************************
2:

/ test sxt instruction
	mov	$10,r2			/ set up index register
	mov	$-1,temp
	clc|clv|clz|cln
	sxt	c(r2)
	tst	temp
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;48.
/*******************************************************************
2:
	clr	temp
	sec|sev|sez|sen
	sxt	temp
	cmp	$-1,temp
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;49.
/*******************************************************************
2:

/ test the mul instruction
	clr	r3
	mov	$5,r2			/ load multiplicand
	clr	plier
	mov	$2,plier		/ load the multiplier
	sec|sev|sez|sen
	mul	plier,r2		/ mult 2*5 result in r2(msh), r3(lsh)
	bmi	1f
	blos	1f
	bvc	2f
/*******************************************************************
1:	jsr	pc,error ;50.
/*******************************************************************
2:
	cmp	$12,r3			/ result != 12?
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;51.
/*******************************************************************
2:
	tst	r2			/ correct result (msh)
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;52.
/*******************************************************************
2:
	clr	r3
	clr	plier
	mov	$125252,r2		/ load multiplicand
	mov	$2,plier
	mul	plier,r2		/ 2*125252 = -1 in r2
					/ 52524 in r3 n = 1, z = 0, v = 0, c = 1
	bcc	1f
	bpl	1f
	bvs	1f
	bne	2f
/*******************************************************************
1:	jsr	pc,error ;53.
/*******************************************************************
2:
	cmp	$-1,r2
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;54.
/*******************************************************************
2:
	cmp	$52524,r3		/ correct lsh result?
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;55.
/*******************************************************************
2:

/ test ash instruction
	clr	plier
	mov	$1,plier		/ load shift value (+1 or 1 place left)
	mov	$125252,r2		/ value to be shifted
	clc|clv|clz|cln
	sez
	sen
	ash	plier,r2		/ shift 1 place left result = 52524
	bmi	1f
	beq	1f
	bvc	1f
	bcs	2f
/*******************************************************************
1:	jsr	pc,error ;56.
/*******************************************************************
2:
	cmp	$52524,r2		/ result correct?
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;57.
/*******************************************************************
2:
	clr	plier
	mov	$-1,plier		/ shift value -1 1 place right
	mov	$52525,r1
	ash	plier,r1		/ shift 1 place right - result = 25252
	cmp	r1,$25252
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;58.
/*******************************************************************
2:

/ test ashc instruction
	clr	r2			/ clear MSH result register
	clr	plier
	mov	$16.,plier		/ load shift count 16 places left
	mov	$125252,r3		/ value to be shifted
	clc|clv|clz|cln
	sez
	sec
	ashc	plier,r2		/ shift no in r3 by r2
					/ cc: n=1,z=0,v=1,c=0
	bpl	1f			/ if n = 0, report error
	beq	1f
	bvc	1f
	bcc	2f
/*******************************************************************
1:	jsr	pc,error ;59.
/*******************************************************************
2:
	cmp	$125252,r2
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;60.
/*******************************************************************
2:
	tst	r3
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;61.
/*******************************************************************
2:
	clr	r3			/ clear result register
	clr	plier
	mov	$-16.,plier		/ 16 places right
	mov	$125252,r2
	cln
	sez
	sev
	sec
	ashc	plier,r2		/ shift r2 16 places right into r3
					/ cc: n=1,z=v=c=0
	bpl	1f
	blos	1f
	bvc	2f
/*******************************************************************
1:	jsr	pc,error ;62.
/*******************************************************************
2:

/ test the divide instruction
	mov	$4,r1			/ load index register
	clr	r2			/ clear quotient register
	mov	$52525,r3		/ load lsh dividend
	sec|sez|sen|sev
	div	b(r1),r2		/ 52525/b(r1) (b(r1) = 52525)
					/ quotient =1,rem = 0, c=n=z=v=0
	bmi	1f
	blos	1f
	bvc	2f
/*******************************************************************
1:	jsr	pc,error ;63.
/*******************************************************************
2:
	cmp	r2,$1			/ quotient correct?
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;64.
/*******************************************************************
2:
	tst	r3			/ remainder correct?
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;65.
/*******************************************************************
2:
	clr	plier
	mov	$157777,r4		/ load msh dividend
	mov	$100001,r5		/ load lsh dividend
	mov	$100000,plier		/ load divisor
	sec|sen|sez|sev
	div	plier,r4		/ divide 157777 100001 by 100000
					/ quotient = 40000, rem = 100001, c=n=z=v=0
	bmi	1f
	blos	1f
	bvc	2f
/*******************************************************************
1:	jsr	pc,error ;66.
/*******************************************************************
2:
	cmp	r4,$40000		/ correct quotient
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;67.
/*******************************************************************
2:
	cmp	r5,$100001		/ remainder correct?
	beq	2f
/*******************************************************************
1:	jsr	pc,error ;68.
/*******************************************************************
2:
	rts	pc
