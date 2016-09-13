
/* Optimized strcmp implementation for PowerPC64/POWER8.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


#include "hphp/util/etch-helpers.h"

#if defined(__powerpc__)
        .file       "hphp/util/strcmp-power8.S"
        ETCH_SECTION(strcmp)

/**
 *
 *  int size_t [%r3] strcmp (const char *s1 [%r3], const char *s2 [%r4])
 *
 *  The implementation uses unaligned doubleword access to avoid specialized
 *  code paths depending of data alignment.  Although recent powerpc64
 *  64K as default, the page cross handling assumes minimum page size of
 *  4k.
 *
 *  This is an adaptation from the optimized version from glibc code
*/
        ETCH_ALIGN4 // on POWER this is 2^value
        .globl      ETCH_NAME(strcmp)
        ETCH_TYPE(ETCH_NAME(strcmp), @function)
        CFI(startproc)
ETCH_NAME(strcmp): 
    li      %r0,0

/* Check if [s1]+32 or [s2]+32 will cross a 4K page boundary using
 * the code:
 *
 * (((size_t) s1) % PAGE_SIZE > (PAGE_SIZE - ITER_SIZE))
 *
 * with PAGE_SIZE being 4096 and ITER_SIZE begin 32.
 */

    rldicl   %r7,%r3,0,52
    rldicl   %r9,%r4,0,52
    cmpldi   cr7,%r7,4096-32
    bgt      cr7, ETCH_LABEL(pagecross_check)
    cmpldi   cr5,%r9,4096-32
    bgt      cr5, ETCH_LABEL(pagecross_check)

/* For short string up to 32 bytes, load both s1 and s2 using
 * unaligned dwords and compare.
 */
    ld       %r8,0(%r3)
    ld       %r10,0(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    ld       %r8,8(%r3)
    ld       %r10,8(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    ld       %r8,16(%r3)
    ld       %r10,16(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    ld       %r8,24(%r3)
    ld       %r10,24(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    addi     %r7,%r3,32
    addi     %r4,%r4,32

ETCH_LABEL(align_8b):
/* Now it has checked for first 32 bytes, align source1 to doubleword
 * and adjust source2 address.
 */
    rldicl   %r9,%r7,0,61  /* source1 alignment to doubleword  */
    subf     %r4,%r9,%r4   /* Adjust source2 address based on source1 alignment.*/
    rldicr   %r7,%r7,0,60  /* Align source1 to doubleword.  */

/* At this point, source1 alignment is 0 and source2 alignment is
 * between 0 and 7.  Check is source2 alignment is 0, meaning both
 * sources have the same alignment.
 */
    andi.    %r9,%r4,0x7
    bne      cr0, ETCH_LABEL(loop_diff_align)

/* If both source1 and source2 are doubleword aligned, there is no
 * need for page boundary cross checks.
 */

    ld       %r8,0(%r7)
    ld       %r10,0(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    ETCH_ALIGN4
ETCH_LABEL(loop_equal_align):
    ld       %r8,8(%r7)
    ld       %r10,8(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    ld       %r8,16(%r7)
    ld       %r10,16(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    ldu      %r8,24(%r7)
    ldu      %r10,24(%r4)
    cmpb     %r12,%r8,%r0
    cmpb     %r11,%r8,%r10
    orc.     %r9,%r12,%r11
    bne      cr0, ETCH_LABEL(different_nocmpb)

    b        ETCH_LABEL(loop_equal_align)

/* 
 * A zero byte was found in %r8 (s1 dword), %r9 contains the cmpb
 * result and %r10 the dword from s2.  To code isolate the byte
 * up to end (including the '\0'), masking with 0xFF the remaining ones:
 *
 *     (__builtin_ffsl (x) - 1) = counting trailing zero bits
 *     %r9 = (__builtin_ffsl (%r9) - 1) + 8;
 *     %r9 = -1UL << %r9
 *     %r8  = %r8  | %r9
 *     %r10 = %r10 | %r9  
 */

    nor     %r9,%r9,%r9
ETCH_LABEL(different_nocmpb):
    neg     %r3,%r9
    and     %r9,%r9,%r3
    cntlzd  %r9,%r9
    subfic  %r9,%r9,63
    srd     %r3,%r8,%r9
    srd     %r10,%r10,%r9
    rldicl  %r10,%r10,0,56
    rldicl  %r3,%r3,0,56
    subf    %r3,%r10,%r3
    extsw   %r3,%r3
    blr

    ETCH_ALIGN4
ETCH_LABEL(pagecross_check):
    subfic  %r9,%r9,4096
    subfic  %r7,%r7,4096
    cmpld   cr7,%r7,%r9
    bge     cr7, ETCH_LABEL(pagecross)
    mr      %r7,%r9

/*
 * If unaligned 16 bytes reads across a 4K page boundary, it uses
 * a simple byte a byte comparison until the page alignment for s1
 * is reached.
 */
ETCH_LABEL(pagecross):
    add     %r7,%r3,%r7
    subf    %r9,%r3,%r7
    mtctr   %r9

    ETCH_ALIGN4
ETCH_LABEL(pagecross_loop):
/*
 * Loads a byte from s1 and s2, compare if *s1 is equal to *s2
 * and if *s1 is '\0'.
 */
    lbz     %r9,0(%r3)
    lbz     %r10,0(%r4)
    addi    %r3,%r3,1
    addi    %r4,%r4,1
    cmplw   cr7,%r9,%r10
    cmpdi   cr5,%r9,%r0
    bne     cr7, ETCH_LABEL(pagecross_ne)
    beq     cr5, ETCH_LABEL(pagecross_nullfound)
    bdnz    ETCH_LABEL(pagecross_loop)
    b       ETCH_LABEL(align_8b)

    ETCH_ALIGN4
/*
 * The unaligned read of source2 will cross a 4K page boundary,
 * and the different byte or NULL maybe be in the remaining page
 * bytes. Since it can not use the unaligned load, the algorithm
 * reads and compares 8 bytes to keep source1 doubleword aligned.
 */
ETCH_LABEL(check_source2_byte):
    li      %r9,8
    mtctr   %r9

    ETCH_ALIGN4
ETCH_LABEL(check_source2_byte_loop):
    lbz     %r9,0(%r7)
    lbz     %r10,0(%r4)
    addi    %r7,%r7,1
    addi    %r4,%r4,1
    cmplw   cr7,%r9,10
    cmpdi   %r5,%r9,0
    bne     cr7, ETCH_LABEL(pagecross_ne)
    beq     cr5, ETCH_LABEL(pagecross_nullfound)
    bdnz    ETCH_LABEL(check_source2_byte_loop)

/*
 * If source2 is unaligned to doubleword, the code needs to check
 * on each interation if the unaligned doubleword access will cross
 * a 4k page boundary.
 */
    .align  5
ETCH_LABEL(loop_unaligned):
    ld      %r8,0(%r7)
    ld      %r10,0(%r4)
    cmpb    %r12,%r8,%r0
    cmpb    %r11,%r8,%r10
    orc.    %r9,%r12,%r11
    bne     cr0, ETCH_LABEL(different_nocmpb)
    addi    %r7,%r7,8
    addi    %r4,%r4,8

ETCH_LABEL(loop_diff_align):
/*
 * Check if [src2]+8 cross a 4k page boundary:
 *
 * srcin2 % PAGE_SIZE > (PAGE_SIZE - 8)
 * with PAGE_SIZE being 4096.
 */
    rldicl  %r9,%r4,0,52
    cmpldi  cr7,%r9,4088
    ble     cr7, ETCH_LABEL(loop_unaligned)
    b       ETCH_LABEL(check_source2_byte)

    ETCH_ALIGN4
ETCH_LABEL(pagecross_ne):
    extsw   %r3,%r9
    mr      %r9,%r10
ETCH_LABEL(pagecross_retdiff):
    subf    %r9,%r9,%r3
    extsw   %r3,%r9
    blr

    ETCH_ALIGN4
ETCH_LABEL(pagecross_nullfound):
    li     %r3,0
    b      ETCH_LABEL(pagecross_retdiff)

    CFI(endproc)
    ETCH_SIZE(strcmp)
    .ident    "GCC: (GNU) 4.8.2"
#ifdef __linux__ 
    .section  .note.GNU-stack,"",@progbits
#endif

#endif
