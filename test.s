	.data
	.align 8
a:	.quad 0
b:	.quad 0
			# This code was produced by the CERI Compiler
	.text		# The following lines contain the program
.globl main		# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push b
	push $3
Arith0:
	pop %rbx
	pop %rax
	addq	%rbx, %rax
	push %rax
FinArith0: 
	pop a
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
