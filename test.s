			# This code was produced by the CERI Compiler
.data
FormatString1:	.string "%llu"	# used by printf to display 64-bit unsigned integers
FormatString2:	.string "%lf"	# used by printf to display 64-bit floating point numbers
FormatString3:	.string "%c"	# used by printf to display a 8-bit single character
TrueString:	.string "TRUE"	# used by printf to display the boolean value TRUE
FalseString:	.string "FALSE"	# used by printf to display the boolean value FALSE
a:	.quad 0
i:	.quad 0
s:	.quad 0
b:	.double 0.0
c:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $0
	pop a
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $858993459, (%rsp)	# Conversion of 1.2 (32 bit high part)
	movl $1072902963, 4(%rsp)	# Conversion of 1.2 (32 bit low part)
	pop b
	push $3
	push $4
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	ja Vrai2	# If above
	push $0		# False
	jmp Suite2
Vrai2:	push $0xFFFFFFFFFFFFFFFF		# True
Suite2:
	pop c
	push b
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $2576980378, (%rsp)	# Conversion of 1.1 (32 bit high part)
	movl $1072798105, 4(%rsp)	# Conversion of 1.1 (32 bit low part)
	fldl 8(%rsp)	
	fldl (%rsp)	# first oprand -> %st(0) ; second operand -> %st(1)
	fmulp %st(0),%st(1)	# %st(0) <- op1 + op2 ; %st(1)=null
	fstpl 8(%rsp)
	addq $8,%rsp	# result on stack's top
	pop b
	push $0
	pop s
	push a
	push $3
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	ja Vrai5	# If above
	push $0		# False
	jmp Suite5
Vrai5:	push $0xFFFFFFFFFFFFFFFF		# True
Suite5:
	pop %rdx	# Zero : False, non-zero : true
	cmpq $0,%rdx
	je False3
	movq $TrueString,%rdi	# "TRUE\n"
	jmp Next3
False3:
	movq $FalseString,%rdi	# "FALSE\n"
Next3:
	call puts
	push b
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $0, (%rsp)	# Conversion of 2 (32 bit high part)
	movl $1073741824, 4(%rsp)	# Conversion of 2 (32 bit low part)
	fldl 8(%rsp)	
	fldl (%rsp)	# first oprand -> %st(0) ; second operand -> %st(1)
	fmulp %st(0),%st(1)	# %st(0) <- op1 + op2 ; %st(1)=null
	fstpl 8(%rsp)
	addq $8,%rsp	# result on stack's top
	movsd (%rsp),%xmm0		# &stack top -> %xmm0
	subq $16,%rsp		# allocation for 3 additional doubles
	movsd %xmm0,8(%rsp)
	movq $FormatString2, %rdi	# "%lf\n"
	movq $1,%rax
	call printf
nop
	addq $24,%rsp			# pop nothing
	push a
	pop %rsi	# The value to be displayed
	movq $FormatString1,%rdi	# "%llu\n"
	movl $0,%eax
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
