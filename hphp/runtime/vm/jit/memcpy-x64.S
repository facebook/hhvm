	.file	"memcpy-x64.S"
	.text
	.globl	_Z10memcpyPvPKvm
	.type	_Z10memcpyPvPKvm, @function
_Z10memcpyPvPKvm:
.LFB524:
	.cfi_startproc

	testb	$63, %dl
	movq	%rdi, %rax
	movq	%rdi, %rcx
	je	.L2
	movq	%rdx, %rdi
	andl	$7, %edi
	jmp	*.L5(,%rdi,8)
	.section	.rodata
	.align 8
	.align 4
.L5:
	.quad	.L3
	.quad	.L4
	.quad	.L6
	.quad	.L7
	.quad	.L8
	.quad	.L9
	.quad	.L10
	.quad	.L11
	
    .text
    .align 8
	.align 4 
.L11:
	movzbl	6(%rsi), %ecx
	movb	%cl, 6(%rax)
.L10:
	movl	(%rsi), %ecx
	movl	%ecx, (%rax)
	movzwl	4(%rsi), %ecx
	movw	%cx, 4(%rax)

	subq	%rdi, %rdx
	leaq	(%rax,%rdi), %rcx
	addq	%rdi, %rsi
	movl	%edx, %edi
	andl	$63, %edi
	movl	%edi, %r8d
	shrl	$3, %r8d
	jmp	*.L14(,%r8,8)
.L9:
	movzbl	4(%rsi), %ecx
	movb	%cl, 4(%rax)
.L8:
	movl	(%rsi), %ecx
	movl	%ecx, (%rax)

	subq	%rdi, %rdx
	leaq	(%rax,%rdi), %rcx
	addq	%rdi, %rsi
	movl	%edx, %edi
	andl	$63, %edi
	movl	%edi, %r8d
	shrl	$3, %r8d
	jmp	*.L14(,%r8,8)

.L7:
	movzbl	2(%rsi), %ecx
	movb	%cl, 2(%rax)   
.L6:
	movzbl	1(%rsi), %ecx
	movb	%cl, 1(%rax)
.L4:
	movzbl	(%rsi), %ecx
	movb	%cl, (%rax)   
.L3:
	subq	%rdi, %rdx
	leaq	(%rax,%rdi), %rcx
	addq	%rdi, %rsi
	movl	%edx, %edi
	andl	$63, %edi
	movl	%edi, %r8d
	shrl	$3, %r8d
	jmp	*.L14(,%r8,8)
	.section	.rodata
	.align 8
	.align 4
.L14:
	.quad	.L12
	.quad	.L13
	.quad	.L15
	.quad	.L16
	.quad	.L17
	.quad	.L18
	.quad	.L19
	.quad	.L20
	.text
.L20:
	movq	48(%rsi), %r8
	movq	%r8, 48(%rcx)
.L19:
	movq	40(%rsi), %r8
	movq	%r8, 40(%rcx)
.L18:
	movq	32(%rsi), %r8
	movq	%r8, 32(%rcx)
.L17:
	movq	24(%rsi), %r8
	movq	%r8, 24(%rcx)
.L16:
	movq	16(%rsi), %r8
	movq	%r8, 16(%rcx)
.L15:
	movq	8(%rsi), %r8
	movq	%r8, 8(%rcx)
.L13:
	movq	(%rsi), %r8
	movq	%r8, (%rcx)
.L12:
	subq	%rdi, %rdx
	addq	%rdi, %rcx
	addq	%rdi, %rsi
.L2:
	testq	%rdx, %rdx
	jne	.L28
	jmp	.L26
	.p2align 4,,10
	.p2align 3
.L32:
	movq	%r8, %rcx
.L28:
	movdqu	(%rsi), %xmm3
	addq	$64, %rsi
	leaq	64(%rcx), %r8
	movdqu	-48(%rsi), %xmm2
	movdqu	-32(%rsi), %xmm1
	movdqu	-16(%rsi), %xmm0
	subq	$64, %rdx
	movdqu	%xmm3, (%rcx)
	movdqu	%xmm2, 16(%rcx)
	movdqu	%xmm1, 32(%rcx)
	movdqu	%xmm0, 48(%rcx)
	jne	.L32
.L26:
	rep; ret
    .cfi_endproc
.LFE524:
	.size	_Z10memcpyPvPKvm, .-_Z10memcpyPvPKvm
	.ident	"GCC: (GNU) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
