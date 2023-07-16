/ Source of assembler to be inserted into
/ m40.s, and m45.s if wanted
/ insert before the start label
/ This assembler code halts the machine
/ the code that stops the system is in stopunix()
/ should be added at the end of ken/sys4.c
/
/ Original commment:
/ Modified by Peter Collinson Nov 76
/ To allow killunix program
/ New bit added in to label called stopit
/ Inserted by Peter C to allow full halt of system


halt	= 0		// Halt instruction not in assembler

.globl	_stopit
_stopit:
/ Set illegal instruction trap registers to bootstrap
/ address to cause less pain to idiots who will insist
/ on bootstrapping their systems with bit 0 in the
/ switch register set to 1
	mov	$173000,*$4
	mov	$0340,*$6
	reset
	halt
