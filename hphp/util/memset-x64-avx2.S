#include "hphp/util/etch-helpers.h"

#if defined(__x86_64__) && defined(ENABLE_AVX2) && !defined(__APPLE__)
	.file	"memset-x64-avx2.S"
	ETCH_SECTION(memset)
	.globl	ETCH_NAME(memset)
	.type	ETCH_NAME(memset), @function
	.p2align 4,,10
ETCH_NAME(memset):
ETCH_LABEL(ENTRY_MEMSET):
	CFI(startproc)
	mov		%rdi, %r8
	vpxor	%xmm0, %xmm0, %xmm0
	vmovd	%esi, %xmm1
	vpshufb	%xmm0, %xmm1, %xmm0
	vmovq	%xmm0, %rsi
	testb	$63, %dl
	movq	%rdi, %rcx
	je		ETCH_LABEL(more_than_63bytes)
	movl	%edx, %edi
	andl	$7, %edi
	leal	-1(%rdi), %eax
	cmpl	$6, %eax
	ja		ETCH_LABEL(more_than_7bytes)
	jmp		*ETCH_LABEL(less_than_7bytes)(,%rax,8)
.section	.rodata
ETCH_ALIGN8
ETCH_ALIGN4
ETCH_LABEL(less_than_7bytes):
	.quad	ETCH_LABEL(1byte_move)
	.quad	ETCH_LABEL(2byte_move)
	.quad	ETCH_LABEL(3byte_move)
	.quad	ETCH_LABEL(4byte_move)
	.quad	ETCH_LABEL(5byte_move)
	.quad	ETCH_LABEL(6byte_move)
	.quad	ETCH_LABEL(7byte_move)
ETCH_SECTION(memset)
ETCH_LABEL(7byte_move):
	movb	%sil, 6(%rcx)
ETCH_LABEL(6byte_move):
	movb	%sil, 5(%rcx)
ETCH_LABEL(5byte_move):
	movb	%sil, 4(%rcx)
ETCH_LABEL(4byte_move):
	movb	%sil, 3(%rcx)
ETCH_LABEL(3byte_move):
	movb	%sil, 2(%rcx)
ETCH_LABEL(2byte_move):
	movb	%sil, 1(%rcx)
ETCH_LABEL(1byte_move):
	movb	%sil, (%rcx)
ETCH_LABEL(more_than_7bytes):
	subq	%rdi, %rdx
	leaq	(%rcx,%rdi), %rcx
	movq	%rdx, %rax
	andl	$63, %eax
	movl	%eax, %edi
	shrl	$3, %edi
	decl	%edi
	cmpl	$6, %edi
	ja		ETCH_LABEL(more_than_63bytes)
	jmp		*ETCH_LABEL(less_than_63bytes)(,%rdi,8)
	.section	.rodata
ETCH_ALIGN8
ETCH_ALIGN4
ETCH_LABEL(less_than_63bytes):
	.quad	ETCH_LABEL(8byte_move)
	.quad	ETCH_LABEL(16byte_move)
	.quad	ETCH_LABEL(24byte_move)
	.quad	ETCH_LABEL(32byte_move)
	.quad	ETCH_LABEL(40byte_move)
	.quad	ETCH_LABEL(48byte_move)
	.quad	ETCH_LABEL(56byte_move)
ETCH_SECTION(memset)
ETCH_LABEL(56byte_move):
	movq	%xmm0, 48(%rcx)
ETCH_LABEL(48byte_move):
	movq	%xmm0, 40(%rcx)
ETCH_LABEL(40byte_move):
	movq	%xmm0, 32(%rcx)
ETCH_LABEL(32byte_move):
	movq	%xmm0, 24(%rcx)
ETCH_LABEL(24byte_move):
	movq	%xmm0, 16(%rcx)
ETCH_LABEL(16byte_move):
	movq	%xmm0, 8(%rcx)
ETCH_LABEL(8byte_move):
	movq	%xmm0, (%rcx)
	subq	%rax, %rdx
	leaq	(%rcx,%rax), %rcx
ETCH_LABEL(more_than_63bytes):
	testq	%rdx, %rdx
	je		ETCH_LABEL(EXIT_MEMSET)
	movq	%rdx, %rax
	vinserti128 $1, %xmm0, %ymm0, %ymm0
	andq	$64, %rax
	je		ETCH_LABEL(128byte_loop_data_guzzler)
	vmovdqu %ymm0, (%rcx)
	vmovdqu %ymm0, 0x20(%rcx)
	addq	%rax, %rcx
	subq	%rax, %rdx
	je		ETCH_LABEL(EXIT_MEMSET)
	
ETCH_ALIGN8
ETCH_ALIGN4
ETCH_LABEL(128byte_loop_data_guzzler):
	vmovdqu		%ymm0, (%rcx)
	vmovdqu		%ymm0, 0x20(%rcx)
	vmovdqu		%ymm0, 0x40(%rcx)
	vmovdqu		%ymm0, 0x60(%rcx)
	addq		$128, %rcx
	subq		$128, %rdx
	ja			ETCH_LABEL(128byte_loop_data_guzzler)
	vzeroupper
ETCH_LABEL(EXIT_MEMSET):
	movq %r8, %rax
	retq
	CFI(endproc)
	.size	memset, .-memset
#endif
