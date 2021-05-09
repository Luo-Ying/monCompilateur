			# This code was produced by the CERI Compiler
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $4
	push $3
Exp :
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	jb True
False:
	push $0
	jmp FinExp
True:
	push $1
FinExp:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
