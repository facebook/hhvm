#include "hphp/runtime/vm/jit/etch-helpers.h"

#if defined(__x86_64__) && !defined(__APPLE__)
	.file	"memcpy-x64.S"
        ETCH_SECTION(memcpy)
	.globl	ETCH_NAME(memcpy)
	.type	ETCH_NAME(memcpy), @function
ETCH_NAME(memcpy):
ETCH_LABEL(FB524):
	CFI(startproc)

	testb	$63, %dl
	movq	%rdi, %rax
	movq	%rdi, %rcx
	je	ETCH_LABEL(2)
	movq	%rdx, %rdi
	andl	$7, %edi
	jmp	*ETCH_LABEL(5)(,%rdi,8)
	.section	.rodata
	ETCH_ALIGN8
	ETCH_ALIGN4
ETCH_LABEL(5):
	.quad	ETCH_LABEL(3)
	.quad	ETCH_LABEL(4)
	.quad	ETCH_LABEL(6)
	.quad	ETCH_LABEL(7)
	.quad	ETCH_LABEL(8)
	.quad	ETCH_LABEL(9)
	.quad	ETCH_LABEL(10)
	.quad	ETCH_LABEL(11)

        ETCH_SECTION(memcpy)
	ETCH_ALIGN8
	ETCH_ALIGN4
ETCH_LABEL(11):
	movzbl	6(%rsi), %ecx
	movb	%cl, 6(%rax)
ETCH_LABEL(10):
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
	jmp	*ETCH_LABEL(14)(,%r8,8)
ETCH_LABEL(9):
	movzbl	4(%rsi), %ecx
	movb	%cl, 4(%rax)
ETCH_LABEL(8):
	movl	(%rsi), %ecx
	movl	%ecx, (%rax)

	subq	%rdi, %rdx
	leaq	(%rax,%rdi), %rcx
	addq	%rdi, %rsi
	movl	%edx, %edi
	andl	$63, %edi
	movl	%edi, %r8d
	shrl	$3, %r8d
	jmp	*ETCH_LABEL(14)(,%r8,8)

ETCH_LABEL(7):
	movzbl	2(%rsi), %ecx
	movb	%cl, 2(%rax)
ETCH_LABEL(6):
	movzbl	1(%rsi), %ecx
	movb	%cl, 1(%rax)
ETCH_LABEL(4):
	movzbl	(%rsi), %ecx
	movb	%cl, (%rax)
ETCH_LABEL(3):
	subq	%rdi, %rdx
	leaq	(%rax,%rdi), %rcx
	addq	%rdi, %rsi
	movl	%edx, %edi
	andl	$63, %edi
	movl	%edi, %r8d
	shrl	$3, %r8d
	jmp	*ETCH_LABEL(14)(,%r8,8)
	.section	.rodata
	ETCH_ALIGN8
	ETCH_ALIGN4
ETCH_LABEL(14):
	.quad	ETCH_LABEL(12)
	.quad	ETCH_LABEL(13)
	.quad	ETCH_LABEL(15)
	.quad	ETCH_LABEL(16)
	.quad	ETCH_LABEL(17)
	.quad	ETCH_LABEL(18)
	.quad	ETCH_LABEL(19)
	.quad	ETCH_LABEL(20)
        ETCH_SECTION(memcpy)
ETCH_LABEL(20):
	movq	48(%rsi), %r8
	movq	%r8, 48(%rcx)
ETCH_LABEL(19):
	movq	40(%rsi), %r8
	movq	%r8, 40(%rcx)
ETCH_LABEL(18):
	movq	32(%rsi), %r8
	movq	%r8, 32(%rcx)
ETCH_LABEL(17):
	movq	24(%rsi), %r8
	movq	%r8, 24(%rcx)
ETCH_LABEL(16):
	movq	16(%rsi), %r8
	movq	%r8, 16(%rcx)
ETCH_LABEL(15):
	movq	8(%rsi), %r8
	movq	%r8, 8(%rcx)
ETCH_LABEL(13):
	movq	(%rsi), %r8
	movq	%r8, (%rcx)
ETCH_LABEL(12):
	subq	%rdi, %rdx
	addq	%rdi, %rcx
	addq	%rdi, %rsi
ETCH_LABEL(2):
	testq	%rdx, %rdx
	jne	ETCH_LABEL(28)
	jmp	ETCH_LABEL(26)
	.p2align 4,,10
	.p2align 3
ETCH_LABEL(32):
	movq	%r8, %rcx
ETCH_LABEL(28):
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
	jne	ETCH_LABEL(32)
ETCH_LABEL(26):
	rep; ret
	CFI(endproc)
ETCH_LABEL(FE524):
	ETCH_SIZE(memcpy)
	.ident	"GCC: (GNU) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
#endif
