#include "hphp/util/etch-helpers.h"
#if defined(__x86_64__)

.file	"hphp/util/hash-crc.S"

ETCH_SECTION(hash_string_i_crc)
.globl	ETCH_NAME(hash_string_i_crc)
ETCH_TYPE(ETCH_NAME(hash_string_i_crc), @function)
ETCH_ALIGN16
ETCH_NAME(hash_string_i_crc):
ETCH_LABEL(FB800):
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

ETCH_LABEL(FE800):
        ETCH_SIZE(hash_string_i_crc)


ETCH_SECTION(hash_string_i_unaligned_crc)
.globl	ETCH_NAME(hash_string_i_unaligned_crc)
ETCH_TYPE(ETCH_NAME(hash_string_i_unaligned_crc), @function)
ETCH_ALIGN16
ETCH_NAME(hash_string_i_unaligned_crc):
ETCH_LABEL(FB801):
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

ETCH_LABEL(FE801):
        ETCH_SIZE(hash_string_i_unaligned_crc)


ETCH_SECTION(hash_string_cs_crc)
.globl	ETCH_NAME(hash_string_cs_crc)
ETCH_TYPE(ETCH_NAME(hash_string_cs_crc), @function)
ETCH_ALIGN16
ETCH_NAME(hash_string_cs_crc):
ETCH_LABEL(FB802):
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

ETCH_LABEL(FE802):
        ETCH_SIZE(hash_string_cs_crc)


ETCH_SECTION(hash_string_cs_unaligned_crc)
.globl	ETCH_NAME(hash_string_cs_unaligned_crc)
ETCH_TYPE(ETCH_NAME(hash_string_cs_unaligned_crc), @function)
ETCH_ALIGN16
ETCH_NAME(hash_string_cs_unaligned_crc):
ETCH_LABEL(FB803):
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
        ETCH_SIZE(hash_string_cs_unaligned_crc)


/* The following is the asm version of HPHP::StringData::hashHelper */
ETCH_SECTION(g_hashHelper_crc)
.globl	ETCH_NAME(g_hashHelper_crc)
ETCH_TYPE(ETCH_NAME(g_hashHelper_crc), @function)
ETCH_ALIGN16
ETCH_NAME(g_hashHelper_crc):
ETCH_LABEL(FB804):
        CFI(startproc)
        xor     %ecx, %ecx
        or      $-1, %eax
        sub     0x10(%rdi), %ecx
        jz      ETCH_LABEL(hend)

        mov     (%rdi), %rdx
        movabs  $0xdfdfdfdfdfdfdfdf, %r8
        jmp     ETCH_LABEL(hheader)

ETCH_LABEL(hloop):
        add     $8, %rdx
        crc32q  %rsi, %rax
ETCH_LABEL(hheader):
        mov     %r8, %rsi
        and     (%rdx), %rsi
        add     $8, %ecx
        jnc     ETCH_LABEL(hloop)

        shl     $3, %ecx
        shl     %cl, %rsi
        crc32q  %rsi, %rax

ETCH_LABEL(hend):
        shr     %eax
        or      %eax, 0x14(%rdi)
        ret
        CFI(endproc)

ETCH_LABEL(FE804):
        ETCH_SIZE(g_hashHelper_crc)

#endif
