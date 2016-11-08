#include "hphp/runtime/base/string-data-macros.h"  // NO_M_DATA
#include "hphp/util/etch-helpers.h"

#if defined(__x86_64__)

        .file     "hphp/runtime/base/string-data-x64.S"

/* The following is the asm version of HPHP::StringData::hashHelper */
ETCH_SECTION(g_hashHelper_crc)
.globl	ETCH_NAME(g_hashHelper_crc)
ETCH_TYPE(ETCH_NAME(g_hashHelper_crc), @function)
ETCH_ALIGN16
ETCH_NAME(g_hashHelper_crc):
ETCH_LABEL(FB804):
        CFI(startproc)
        mov     SD_LEN(%rdi), %ecx
        or      $-1, %eax
        lea     SD_DATA(%rdi, %rcx), %rdx
        neg     %rcx
        jnz     ETCH_LABEL(hashHelper_crc_header)
        jmp     ETCH_LABEL(hashHelper_crc_end)

ETCH_LABEL(hashHelper_crc_loop):
        crc32q  %rsi, %rax
ETCH_LABEL(hashHelper_crc_header):
        movabs  $0xdfdfdfdfdfdfdfdf, %rsi
        and     (%rdx, %rcx), %rsi
        add     $8, %rcx
        jnc     ETCH_LABEL(hashHelper_crc_loop)

        shl     $3, %ecx
        shl     %cl, %rsi
        crc32q  %rsi, %rax

ETCH_LABEL(hashHelper_crc_end):
        shr     %eax
        or      %eax, SD_HASH(%rdi)
        ret
        CFI(endproc)

ETCH_LABEL(FE804):
        ETCH_SIZE(g_hashHelper_crc)

#endif
