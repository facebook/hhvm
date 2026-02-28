#include "hphp/util/etch-helpers.h"

#ifndef HHVM_FACEBOOK
#  include "hphp/util/hphp-config.h"
#endif

#if defined(__aarch64__) && !defined(NO_HWCRC)

.file "hphp/util/hash-crc-arm.S"
.arch armv8-a+crc

/*
 * HPHP Hardware Accelerated Hashing for Arm Systems
 *
 * The Big Theory Statement(tm)
 *
 * First up, we avoid callee-saved registers so as to avoid stack manipulation.
 * In this case, we restrict ourselves to [wx]0-[wx]6, so only the first 7
 * caller-saved registers. In the case-sensitive code we only use up to [wx]5.
 * - x0: the pointer we're reading from, truncated to be 8-byte aligned where
 *       necessary.
 * - x1: the number of bytes remaining.
 * - x2: the bit offset we're working on in alignment loops.
 * - x3: briefly contains the aligned pointer, but nominally contains the
 *       datum we're working on in unaligned loops.
 * - w4: is the hash accumulator and destination, later copied out to w0 before
 *       we return to the caller.
 * - x5: the datum we're working on in the aligned case.
 * - x6: the mask used to uppercase all characters (in the "i"nsensitive
 *       functions).
 *
 * Generally speaking, the flow through a function is:
 * a. A header sets up a few constants and decides the next step.
 * b. Leading (unaligned) bytes are processed byte-by-byte.
 * c. Aligned data is processed 8 bytes at a time.
 * d. Trailing bytes (length 7 or less) are processed byte-by-byte.
 * e. The function returns.
 *
 * The detailed logic is:
 * ([a] header)
 * 1. Set the accumulator to 0xffffffff (all ones)
 * 2. If the passed length is 0, we're done. Go to step 36.
 * 3. If the passed pointer is aligned OR this is the unsafe variant, go to
 *    step 13.
 * ([b] leading unaligned bytes processing)
 * 4. Align to passed pointer to 8 bytes
 * 5. Calculate the number of leading bits that are irrelevant (shift amount)
 * 6. If there are no further bytes to process, we're done. Go to step 36.
 * 7. Use the shift amount and a mask to extract the byte we're processing
 * 8. Process the extracted byte
 * 9. Adjust the length, if now zero we're done. Go to step 36.
 * 10. Increment the shift amount by 8 (bits).
 * 11. If the new shift amount is 64 we're aligned, go to step 13.
 * 12. Loop to step 6.
 * ([c] aligned processing)
 * 13. Load a register from the passed pointer, post-incrementing the pointer
 *     by 8 bytes.
 * 14. If there are fewer than 8 bytes remaining to process, go to the tail
 *     processing. Go to step 19.
 * 15. Process the extracted 8 byte value.
 * 16. Load a register from the passed pointer, post-incrementing the pointer
 *     by 8 bytes.
 * 17. Decrement the number of bytes remaining by 8 (bytes).
 * 18. Loop to step 14.
 * ([d] trailing bytes processing)
 * 19. Set the shift amount (value extraction shift amount) to zero.
 * 20. If there are no bytes left to process we're done, go to step 36.
 * 21. Extract the current byte we're processing.
 * 22. Process the byte.
 * 23. Decrement the remaining length.
 * 34. Add 8 (bits) the shift amount.
 * 35. Loop to step 20.
 * ([e] trailer)
 * 36. Set the return value to the accumulator value.
 * 37. Shift the return value one bit right so that it will be interpreted as
 *     a positive value if cast to a signed integer.
 * 38. Return to the caller.
 */

/*
 * unsigned int HPHP::hash_string_cs(char const*, unsigned int);
 */
#define HASH_FUNC_NAME _ZN4HPHP14hash_string_csEPKcj
ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        mov     w4, #0xffffffff
        cbz     x1, ETCH_LABEL(cdone)
        and     x2, x0, #0x7
        cbz     x2, ETCH_LABEL(caligned)

        bic     x3, x0, #0x7
        ldr     x5, [x3], #8

        sub     x2, x3, x0
        mov     x0, x3
        lsl     x2, x2, #3
        mov     x3, #64
        sub     x2, x3, x2

ETCH_ALIGN16
ETCH_LABEL(cchomp_head):
        cbz     x1, ETCH_LABEL(cdone)
        lsr     x3, x5, x2
        and     x3, x3, #0xff
        crc32cb w4, w4, w3
        sub     x1, x1, #1
        cbz     x1, ETCH_LABEL(cdone)
        add     x2, x2, #8
        cmp     x2, #64
        beq     ETCH_LABEL(caligned)
        b       ETCH_LABEL(cchomp_head)

ETCH_ALIGN16
ETCH_LABEL(caligned):
        subs    x1, x1, #8
        blt     ETCH_LABEL(cchomp_tail)
ETCH_LABEL(caligned_loop):
        ldr     x5, [x0], #8
        crc32cx w4, w4, x5
        subs    x1, x1, #8
        bge     ETCH_LABEL(caligned_loop)

ETCH_ALIGN16
ETCH_LABEL(cchomp_tail):
        adds    x1, x1, #8
        beq     ETCH_LABEL(cdone)
        ldr     x5, [x0], #8
        mov     x2, #0
ETCH_LABEL(cchomp_tail_loop):
        lsr     x3, x5, x2
        and     x3, x3, #0xff
        crc32cb w4, w4, w3
        sub     x1, x1, #1
        cbz     x1, ETCH_LABEL(cdone)
        add     x2, x2, #8
        b       ETCH_LABEL(cchomp_tail_loop)

ETCH_ALIGN16
ETCH_LABEL(cdone):
        mvn     w0, w4
        lsr     w0, w0, #1
        ret

        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME

/*
 * unsigned int HPHP::hash_string_cs_unsafe(char const*, unsigned int);
 */
#define HASH_FUNC_NAME _ZN4HPHP21hash_string_cs_unsafeEPKcj
ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        mov     w4, #0xffffffff
        cbz     x1, ETCH_LABEL(cudone)

ETCH_LABEL(cualigned):
        subs    x1, x1, #8
        blt     ETCH_LABEL(cuchomp_tail)
ETCH_LABEL(cualigned_loop):
        ldr     x5, [x0], #8
        crc32cx w4, w4, x5
        subs    x1, x1, #8
        bge     ETCH_LABEL(cualigned_loop)

ETCH_ALIGN16
ETCH_LABEL(cuchomp_tail):
        adds    x1, x1, #8
        beq     ETCH_LABEL(cudone)
        ldr     x5, [x0], #8
        mov     x2, #0
ETCH_LABEL(cuchomp_tail_loop):
        lsr     x3, x5, x2
        and     x3, x3, #0xff
        crc32cb w4, w4, w3
        sub     x1, x1, #1
        cbz     x1, ETCH_LABEL(cudone)
        add     x2, x2, #8
        b       ETCH_LABEL(cuchomp_tail_loop)

ETCH_ALIGN16
ETCH_LABEL(cudone):
        mvn     w0, w4
        lsr     w0, w0, #1
        ret

        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME

/*
 * unsigned int HPHP::hash_string_i(char const*, unsigned int);
 */
#define HASH_FUNC_NAME _ZN4HPHP13hash_string_iEPKcj
ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        mov     w4, #0xffffffff
        cbz     x1, ETCH_LABEL(idone)
        mov     x6, #0xdfdfdfdfdfdfdfdf
        and     x2, x0, #0x7
        cbz     x2, ETCH_LABEL(ialigned)

        bic     x3, x0, #0x7
        ldr     x5, [x3], #8

        sub     x2, x3, x0
        mov     x0, x3
        lsl     x2, x2, #3
        mov     x3, #64
        sub     x2, x3, x2

ETCH_ALIGN16
ETCH_LABEL(ichomp_head):
        cbz     x1, ETCH_LABEL(idone)
        lsr     x3, x5, x2
        and     x3, x3, x6
        and     x3, x3, #0xff
        crc32cb w4, w4, w3
        sub     x1, x1, #1
        cbz     x1, ETCH_LABEL(idone)
        add     x2, x2, #8
        cmp     x2, #64
        beq     ETCH_LABEL(ialigned)
        b       ETCH_LABEL(ichomp_head)

ETCH_ALIGN16
ETCH_LABEL(ialigned):
        subs    x1, x1, #8
        blt     ETCH_LABEL(ichomp_tail)
ETCH_LABEL(ialigned_loop):
        ldr     x5, [x0], #8
        and     x5, x5, x6
        crc32cx w4, w4, x5
        subs    x1, x1, #8
        bge     ETCH_LABEL(ialigned_loop)

ETCH_ALIGN16
ETCH_LABEL(ichomp_tail):
        adds    x1, x1, #8
        beq     ETCH_LABEL(idone)
        mov     x2, #0
        ldr     x5, [x0], #8
ETCH_LABEL(ichomp_tail_loop):
        lsr     x3, x5, x2
        and     x3, x3, x6
        and     x3, x3, #0xff
        crc32cb w4, w4, w3
        sub     x1, x1, #1
        cbz     x1, ETCH_LABEL(idone)
        add     x2, x2, #8
        b       ETCH_LABEL(ichomp_tail_loop)

ETCH_ALIGN16
ETCH_LABEL(idone):
        mvn     w0, w4
        lsr     w0, w0, #1
        ret

        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME

/*
 * unsigned int HPHP::hash_string_i_unsafe(char const*, unsigned int);
 */
#define HASH_FUNC_NAME _ZN4HPHP20hash_string_i_unsafeEPKcj
ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        mov     w4, #0xffffffff
        cbz     x1, ETCH_LABEL(iudone)
        mov     x6, #0xdfdfdfdfdfdfdfdf

ETCH_LABEL(iualigned):
        subs    x1, x1, #8
        blt     ETCH_LABEL(iuchomp_tail)
ETCH_LABEL(iualigned_loop):
        ldr     x5, [x0], #8
        and     x5, x5, x6
        crc32cx w4, w4, x5
        subs    x1, x1, #8
        bge     ETCH_LABEL(iualigned_loop)

ETCH_ALIGN16
ETCH_LABEL(iuchomp_tail):
        adds    x1, x1, #8
        beq     ETCH_LABEL(iudone)
        ldr     x5, [x0], #8
        mov     x2, #0
ETCH_LABEL(iuchomp_tail_loop):
        lsr     x3, x5, x2
        and     x3, x3, x6
        and     x3, x3, #0xff
        crc32cb w4, w4, w3
        sub     x1, x1, #1
        cbz     x1, ETCH_LABEL(iudone)
        add     x2, x2, #8
        b       ETCH_LABEL(iuchomp_tail_loop)

ETCH_ALIGN16
ETCH_LABEL(iudone):
        mvn     w0, w4
        lsr     w0, w0, #1
        ret

        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME

#endif
