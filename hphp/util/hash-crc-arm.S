#include "hphp/util/etch-helpers.h"

#ifndef HHVM_FACEBOOK
#  include "hphp/util/hphp-config.h"
#endif

#if defined(ENABLE_AARCH64_CRC) && !defined(NO_HWCRC)

.file   "hphp/util/hash-crc-arm.S"

#define HASH_FUNC_NAME _ZN4HPHP20hash_string_i_unsafeEPKcj

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        negs    w1, w1
        mov     w9, #0xffffffff
        beq     ETCH_LABEL(iend)
        mov     w3, w1
        mov     x4, #0xdfdfdfdfdfdfdfdf
        b       ETCH_LABEL(iheader)

ETCH_ALIGN16
ETCH_LABEL(iloop):
        crc32cx w9, w9, x2
ETCH_LABEL(iheader):
        ldr     x10, [x0], #8
        adds    w3, w3, #8
        mov     x2, x4
        and     x2, x2, x10
        bcc     ETCH_LABEL(iloop)
        lsl     w3, w3, #3
        lsl     x2, x2, x3
        crc32cx w9, w9, x2

ETCH_LABEL(iend):
        lsr     w0, w9, #1
        ret
        CFI(endproc)

ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#define HASH_FUNC_NAME _ZN4HPHP13hash_string_iEPKcj

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        mov     w9, #0xffffffff
        subs    w3, wzr, w1
        beq     ETCH_LABEL(icend)

        tst     w0, #7
        mov     x4, #0xdfdfdfdfdfdfdfdf
        bne     ETCH_LABEL(iuheader)
        b       ETCH_LABEL(icheader)

ETCH_ALIGN16
ETCH_LABEL(icloop):
        crc32cx w9, w9, x2
ETCH_LABEL(icheader):
        ldr     x10, [x0], #8
        adds    w3, w3, #8
        mov     x2, x4
        and     x2, x2, x10
        bcc     ETCH_LABEL(icloop)

ETCH_LABEL(ictail):
        lsl     w3, w3, #3
        lsl     x2, x2, x3
        crc32cx w9, w9, x2

ETCH_LABEL(icend):
        lsr     w0, w9, #1
        ret

ETCH_ALIGN16
ETCH_LABEL(iuloop):
        ldr     x10, [x0], #8
        and     x2, x2, x10
        crc32cx w9, w9, x2
ETCH_LABEL(iuheader):
        adds    w3, w3, #8
        mov     x2, x4
        bcc     ETCH_LABEL(iuloop)

        subs    w3, w3, #8
        mov     x2, #0
        beq     ETCH_LABEL(iuend)

ETCH_LABEL(iutailloop):
        ldrb    w10, [x0], #1
        adds    w3, w3, #1
        lsr     x2, x2, #8
        bfi     x2, x10, #55, #8
        bne     ETCH_LABEL(iutailloop)

        and     x2, x2, x4
        crc32cx w9, w9, x2
ETCH_LABEL(iuend):
        lsr     w0, w9, #1
        ret
        CFI(endproc)

ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#define HASH_FUNC_NAME _ZN4HPHP21hash_string_cs_unsafeEPKcj

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        negs    w1, w1
        mov     w9, #0xffffffff
        beq     ETCH_LABEL(csend)
        mov     w3, w1
        b       ETCH_LABEL(csheader)

ETCH_ALIGN16
ETCH_LABEL(csloop):
        crc32cx w9, w9, x2
ETCH_LABEL(csheader):
        ldr     x2, [x0], #8
        adds    w3, w3, #8
        bcc     ETCH_LABEL(csloop)

        lsl     w3, w3, #3
        lsl     x2, x2, x3
        crc32cx w9, w9, x2

ETCH_LABEL(csend):
        lsr     w0, w9, #1
        ret
        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#define HASH_FUNC_NAME _ZN4HPHP14hash_string_csEPKcj

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
ETCH_ALIGN16
ETCH_NAME(HASH_FUNC_NAME):
        CFI(startproc)
        subs    w1, w1, #8
        mov     w9, #0xffffffff
        bmi     ETCH_LABEL(csutail)

ETCH_ALIGN16
ETCH_LABEL(csuloop):
        ldr     x2, [x0], #8
        subs    w1, w1, #8
        crc32cx w9, w9, x2
        bpl     ETCH_LABEL(csuloop)

ETCH_LABEL(csutail):
        adds    w1, w1, #8
        beq     ETCH_LABEL(csuend)
        mov     w3, w1
        mov     x2, #0

ETCH_LABEL(csutailloop):
        ldrb    w10, [x0], #1
        subs    w3, w3, #1
        lsr     x2, x2, #8
        bfi     x2, x10, #55, #8
        bne     ETCH_LABEL(csutailloop)
        crc32cx w9, w9, x2

ETCH_LABEL(csuend):
        lsr     w0, w9, #1
        ret
        CFI(endproc)

ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME

#endif
