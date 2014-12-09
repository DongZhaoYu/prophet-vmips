/* setup.S
 *
 * A simple kernel to run one function void FUNCTION(void) in uncached,
 * unmapped kernel space. This program should be the first one linked in.
 */
#include "asm_regnames.h"

/* These values should match the values given in ld.script. */
#define MEM_BASE		0xa0000000
#define MEM_SIZE		0x100000
#define DATA_START		MEM_BASE + (MEM_SIZE*3/4)

#define INIT_STACK_BASE	DATA_START - 4
#define NTLBENTRIES		64
#define FUNCTION 		entry

	.text
	.set noreorder
	.globl __start

	.ent __start
__start:
	j begin
	.end __start

	/* Halt on user tlb exceptions. */
	.org 0x100
	break 0x0

	/* Halt on exceptions. */
	.org 0x180
	break 0x0

	.org 0x200
	.globl begin
	.ent begin
begin:
/* Start by clearing everything out. */
	.set noat
	move $1, $0
	.set at
	move $2, $0
	move $3, $0
	move $4, $0
	move $5, $0
	move $6, $0
	move $7, $0
	move $8, $0
	move $9, $0
	move $10, $0
	move $11, $0
	move $12, $0
	move $13, $0
	move $14, $0
	move $15, $0
	move $16, $0
	move $17, $0
	move $18, $0
	move $19, $0
	move $20, $0
	move $21, $0
	move $22, $0
	move $23, $0
	move $24, $0
	move $25, $0
	move $26, $0
	move $27, $0
	move $28, $0
	move $29, $0
	move $30, $0
	mtc0 zero, $4
	mtc0 zero, $8
	mtc0 zero, $14

	/* Clear out the TLB. */
	li t2, NTLBENTRIES	/* t2 = TLB entry number */
	li t3, 0x00000fc0	/* t3 = (VPN 0x0, ASID 0x3f) */
1:
	addiu t2, t2, -1	/* Decrement TLB entry number */
	sll t1, t2, 8		/* Shift entry number into Index field position */
	mtc0 t1, $0		/* set Index */
	mtc0 zero, $2		/* clear EntryLo */
	mtc0 t3, $10		/* set EntryHi */
	tlbwi				/* write TLB[Index] with (EntryHi, EntryLo) */
	bnez t2, 1b		/* Go back if we're not done yet. */
	nop
	mtc0 zero, $10		/* clear EntryHi (sets effective ASID=0x0) */

	/* Set up the stack and globals pointer. */
	li sp, INIT_STACK_BASE
	la gp, _gp

	/* Copy writeable data to writeable RAM. */
	la t1, _copystart		/* t1 = beginning source address for copy */
	la t2, _copyend
	addiu t2, t2, 4		/* t2 = one word past ending source address */
	move t3, gp			/* t3 = beginning dest address */
1:
	lw t4, 0(t1)			/* load t4 from ROM */
	sw t4, 0(t3)			/* store it in RAM */
	addiu t1, t1, 4		/* increment both pointers */
	addiu t3, t3, 4
	bne t1, t2, 1b		/* if we're not finished, loop. */
	nop

	/* Call the function. */
	jal FUNCTION

	/* Wait a minute, wait a minute, stop the execution! */
	break 0x0
	.end begin

