/* setup.S
 *
 * A simple kernel to run one function void FUNCTION(void) in uncached,
 * unmapped kernel $space. This program should be the first one linked in.
 */
/*#define zero $0	/* always zero */
/*#define at $1	/* assembler temporary */
/*#define v0 $2	/* function values */
/*#define v1 $3
/*#define a0 $4	/* function arguments */
/*#define a1 $5
#define a2 $6
#define a3 $7
#define t0 $8	/* temporary registers; not preserved across func calls */
/*#define t1 $9
#define t2 $10
#define t3 $11
#define t4 $12
#define t5 $13
#define t6 $14
#define t7 $15
#define s0 $16	/* "saved" regs - must preserve these if you use them */
/*#define s1 $17
#define s2 $18
#define s3 $19
#define s4 $20
#define s5 $21
#define s6 $22
#define s7 $23
#define t8 $24	/* more temporary regs */
/*#define t9 $25
#define k0 $26	/* kernel temporary variables */
/*#define kt0 $26
#define k1 $27
#define kt1 $27
#define s8 $30	/* another "saved" reg */
/*#define ra $31	/* return address */

/* These values should match the values given in ld.script. */
/*#define MEM_BASE		0xa0000000
#define MEM_SIZE		0x100000 
#define DATA_START		0xa00c0000 /* MEM_BASE+(MEM_SIZE*3/4) */
/*#define INIT_STACK_BASE	        0xa00bfffc /* DATA_START-4 */
/*#define NTLBENTRIES		64
#define FUNCTION 		entry */
	.text
	.set noreorder
	.globl __start

	.ent __start
__start:
	j	begin                    	
	.end __start

	/* Halt on user tlb exceptions. */
	.org 0x100
	break	0x0

	/* Halt on exceptions. */
	.org 0x180
	break	0x0

	.org 0x200
	.globl begin
	.ent begin
begin:
/* Start by clearing everything out. */
	.set noat
	move	$1,$0
	.set at
	move	$2,$0
	move	$3,$0
	move	$4,$0
	move	$5,$0
	move	$6,$0
	move	$7,$0
	move	$8,$0
	move	$9,$0
	move	$10,$0
	move	$11,$0
	move	$12,$0
	move	$13,$0
	move	$14,$0
	move	$15,$0
	move	$16,$0
	move	$17,$0
	move	$18,$0
	move	$19,$0
	move	$20,$0
	move	$21,$0
	move	$22,$0
	move	$23,$0
	move	$24,$0
	move	$25,$0
	move	$26,$0
	move	$27,$0
	move	$28,$0
	move	$29,$0
	move	$30,$0
	mtc0	$0, $4
	mtc0	$0, $8
	mtc0	$0, $14

	/* Clear out the TLB. */
	li	$10, 64	/* t2 = TLB entry number */
	li	$11, 0x00000fc0	/* t3 = (VPN 0x0, ASID 0x3f) */
setup1:
	addiu	$10, $10, -1	/* Decrement TLB entry number */
	sll	$9, $10, 8		/* Shift entry number into Index field position */
	mtc0	$9, $0		/* set Index */
	mtc0	$0, $2		/* clear EntryLo */
	mtc0	$11, $10		/* set EntryHi */
	tlbwi				/* write TLB[Index] with (EntryHi, EntryLo) */
	bnez	$10, setup1		/* Go back if we're not done yet. */
	nop
	mtc0	$0, $10		/* clear EntryHi (sets effective ASID=0x0) */

	/* Set up the stack and globals pointer. */
	li	$29, 0xa00bfffc
	la	$28, _gp

	/* Copy writeable data to writeable RAM. */
	la	$9, _copystart		/* t1 = beginning source address for copy */
	la	$10, _copyend
	/*addiu	$10, $10, 4		/* t2 = one word past ending source address */
	move	$11, $28			/* t3 = beginning dest address */
setup2:
	lw	$12, 0($9)			/* load t4 from ROM */
	sw	$12, 0($11)			/* store it in RAM */
	addiu	$9, $9, 4		/* increment both pointers */
	addiu	$11, $11, 4
	bne	$9, $10, setup2		/* if we're not finished, loop. */
	nop

	/* Call the function. */
	jal	entry
	nop

	/* Wait a minute, wait a minute, stop the execution! */
	break	0x0
	.end begin

