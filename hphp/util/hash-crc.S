#include "hphp/util/etch-helpers.h"
#if defined(__x86_64__)

.file	"hash-crc.S"

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
        jmp     ETCH_LABEL(iheader)

ETCH_LABEL(iloop):
        add     $8, %rdi
        crc32q  %r8, %rax
ETCH_LABEL(iheader):
        mov     %rdx, %r8
        and     (%rdi), %r8
        add     $8, %ecx
        jnc     ETCH_LABEL(iloop)

        shl     $3, %ecx
        shl     %cl, %r8
        crc32q  %r8, %rax

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
        or      $-1, %eax
        sub     $8, %esi
        js      ETCH_LABEL(iutail)

ETCH_LABEL(iuloop):
        mov     (%rdi), %rcx
        and     %rdx, %rcx
        add     $8, %rdi
        crc32q  %rcx, %rax
        sub     $8, %esi
        jns     ETCH_LABEL(iuloop)

ETCH_LABEL(iutail):
        add     $8, %esi
        je      ETCH_LABEL(iuend)
        xor     %ecx, %ecx

ETCH_LABEL(iutailloop):
        mov     (%rdi), %cl
        inc     %rdi
        ror     $8, %rcx
        dec     %esi 
        jne     ETCH_LABEL(iutailloop)
        and     %rdx, %rcx
        crc32q  %rcx, %rax

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
        mov     0x10(%rdi), %ecx
        or      $-1, %eax
        jecxz   ETCH_LABEL(hend)
        mov     (%rdi), %rdx

ETCH_LABEL(hloop):
        movabs  $0xdfdfdfdfdfdfdfdf, %rsi
        and     (%rdx), %rsi
        sub     $8, %ecx
        jle     ETCH_LABEL(htail)
        crc32q  %rsi, %rax
        add     $8, %rdx
        jmp     ETCH_LABEL(hloop)

ETCH_LABEL(htail):
        shl     $3, %ecx
        neg     %ecx
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
