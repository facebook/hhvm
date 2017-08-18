#include "hphp/util/etch-helpers.h"

#if defined(__x86_64__) && !defined(NO_HWCRC) && !defined(_MSC_VER)

.file	"hphp/util/hash-crc-x64.S"

/*
 * If SSE4.2 is explicitly specified, we define the HPHP::hash_string* functions
 * here, otherwise those functions are the same as the 'fallback' version, which
 * check CPUID and possibly jumps here.   Thus the function name is different
 * depending on whether __SSE4_2__ is defined.
 */
#ifdef __SSE4_2__
#define IF_SSE42(x, y) ETCH_NAME(x)
#else
#define IF_SSE42(x, y) ETCH_NAME(y)
#endif

#define HASH_FUNC_NAME IF_SSE42(_ZN4HPHP20hash_string_i_unsafeEPKcj, hash_string_i_crc)

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
HASH_FUNC_NAME:
        CFI(startproc)
        or      $-1, %eax
        neg     %esi
        je      ETCH_LABEL(iend)
        mov     %esi, %ecx
        movabs  $0xdfdfdfdfdfdfdfdf, %r8
        jmp     ETCH_LABEL(iheader)

ETCH_LABEL(iloop):
        add     $8, %rdi
        crc32q  %rdx, %rax
ETCH_LABEL(iheader):
        mov     %r8, %rdx
        and     (%rdi), %rdx
        add     $8, %ecx
        jnc     ETCH_LABEL(iloop)

        shl     $3, %ecx
        shl     %cl, %rdx
        crc32q  %rdx, %rax

ETCH_LABEL(iend):
        shr     %eax
        ret
        CFI(endproc)

ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#define HASH_FUNC_NAME IF_SSE42(_ZN4HPHP13hash_string_iEPKcj, hash_string_i_unaligned_crc)

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
HASH_FUNC_NAME:
        CFI(startproc)
        xor     %ecx, %ecx
        or      $-1, %eax
        sub     %esi, %ecx
        je      ETCH_LABEL(icend)

        movabs  $0xdfdfdfdfdfdfdfdf, %r8
        test    $7, %dil
        jnz     ETCH_LABEL(iuheader)
        jmp     ETCH_LABEL(icheader)
        nop

ETCH_LABEL(icloop):
        add     $8, %rdi
        crc32   %rdx, %rax
ETCH_LABEL(icheader):
        mov     %r8, %rdx
        and     (%rdi), %rdx
        add     $8, %ecx
        jnc     ETCH_LABEL(icloop)

ETCH_LABEL(ictail):
        shl     $3, %ecx
        shl     %cl, %rdx
        crc32   %rdx, %rax

ETCH_LABEL(icend):
        shr     %eax
        ret

ETCH_ALIGN16
ETCH_LABEL(iuloop):
        and     (%rdi), %rdx
        add     $8, %rdi
        crc32   %rdx, %rax
ETCH_LABEL(iuheader):
        mov     %r8, %rdx
        add     $8, %ecx
        jnc     ETCH_LABEL(iuloop)

        xor     %edx, %edx
        sub     $8, %ecx
        jz      ETCH_LABEL(iuend)

ETCH_ALIGN16
ETCH_LABEL(iutailloop):
        mov     (%rdi), %dl
        add     $1, %rdi
        ror     $8, %rdx
        add     $1, %ecx
        jnz     ETCH_LABEL(iutailloop)

        and     %r8, %rdx
        crc32   %rdx, %rax
ETCH_LABEL(iuend):
        shr     %eax
        ret

        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#define HASH_FUNC_NAME IF_SSE42(_ZN4HPHP21hash_string_cs_unsafeEPKcj, hash_string_cs_crc)

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
HASH_FUNC_NAME:
        CFI(startproc)
        or      $-1, %eax
        neg     %esi
        je      ETCH_LABEL(csend)
        mov     %esi, %ecx
        jmp     ETCH_LABEL(csheader)

ETCH_LABEL(csloop):
        add     $8, %rdi
        crc32q  %rdx, %rax
ETCH_LABEL(csheader):
        mov     (%rdi), %rdx
        add     $8, %ecx
        jnc     ETCH_LABEL(csloop)

        shl     $3, %ecx
        shl     %cl, %rdx
        crc32q  %rdx, %rax

ETCH_LABEL(csend):
        shr     %eax
        ret
        CFI(endproc)
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#define HASH_FUNC_NAME IF_SSE42(_ZN4HPHP14hash_string_csEPKcj, hash_string_cs_unaligned_crc)

ETCH_SECTION(HASH_FUNC_NAME)
.globl    HASH_FUNC_NAME
ETCH_TYPE(HASH_FUNC_NAME, @function)
HASH_FUNC_NAME:
        CFI(startproc)
        or      $-1, %eax
        sub     $8, %esi
        js      ETCH_LABEL(csutail)

ETCH_LABEL(csuloop):
        mov     (%rdi), %rdx
        add     $8, %rdi
        crc32q  %rdx, %rax
        sub     $8, %esi
        jns     ETCH_LABEL(csuloop)

ETCH_LABEL(csutail):
        add     $8, %esi
        je      ETCH_LABEL(csuend)
        mov     %esi, %ecx
        xor     %edx, %edx

ETCH_LABEL(csutailloop):
        mov     (%rdi), %dl
        inc     %rdi
        ror     $8, %rdx
        loop    ETCH_LABEL(csutailloop)
        crc32q  %rdx, %rax

ETCH_LABEL(csuend):
        shr     %eax
        ret
        CFI(endproc)

ETCH_LABEL(FE803):
ETCH_SIZE(HASH_FUNC_NAME)
#undef HASH_FUNC_NAME


#endif
