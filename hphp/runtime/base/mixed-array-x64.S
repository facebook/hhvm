#include "hphp/runtime/base/mixed-array-x64.h"
#include "hphp/runtime/base/string-data-macros.h"
#include "hphp/util/etch-helpers.h"

#if defined(__SSE4_2__) && defined(NO_M_DATA) && !defined(NO_HWCRC) && \
    !defined(_MSC_VER)

        .file     "hphp/runtime/base/mixed-array-x64.S"

ETCH_SECTION(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE)
        .globl    _ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE

        ETCH_TYPE(ETCH_NAME(hashHelper_NvGetStr), @function)
ETCH_NAME(hashHelper_NvGetStr):
        CFI(startproc)

        mov       SD_LEN(%rsi), %ecx       // key->size()
        lea       SD_DATA(%rsi, %rcx), %r9 // key->data() + key->size()
        not       %edx                     // initialize to -1 (assert: it was 0)
        neg       %rcx
        jnz       ETCH_LABEL(hheader)
        jmp       ETCH_LABEL(hend)
ETCH_LABEL(hloop):
        crc32q    %r11, %rdx
ETCH_LABEL(hheader):
        movabs    $0xdfdfdfdfdfdfdfdf, %r11
        and       (%r9, %rcx), %r11
        add       $8, %rcx
        js        ETCH_LABEL(hloop)

        shl       $3, %ecx
        shl       %cl, %r11
        crc32q    %r11, %rdx

ETCH_LABEL(hend):
        shr       %edx
        or        %edx, SD_HASH(%rsi)   // store hash
        jmp       ETCH_LABEL(hasHash)

        CFI(endproc)
        ETCH_SIZE(hashHelper_NvGetStr)

/*
 * HPHP::MixedArray::NvGetStr(const ArrayData* ad, const StringData* k);
 */

        ETCH_ALIGN16
        ETCH_TYPE(ETCH_NAME(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE), @function)
ETCH_NAME(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE):
        CFI(startproc)

        mov       SD_HASH(%rsi), %edx   // key->hash
        mov       MA_SCALE(%rdi), %eax  // scale
        push      %rbp
        mov       %rsp, %rbp
        add       $MA_DATA, %rdi        // rdi: ad->data()
        push      %rbx
        or        $-1, %ebx
        lea       (%ebx, %eax, 4), %ebx // mask = scale * 4 -1
        lea       (%rax, %rax, 8), %rax
        lea       (%rdi, %rax, 8), %r10 // ad->hashTab()
        and       $0x7fffffff, %edx
        jz        ETCH_NAME(hashHelper_NvGetStr)

ETCH_LABEL(hasHash):
        xor       %r9, %r9              // probe count
        mov       %edx, %eax            // eax: index into hashTab

ETCH_LABEL(nextprobe):
        add       %r9d, %eax
        and       %ebx, %eax
        mov       (%r10, %rax, 4), %r8d
        inc       %r9d
        test      %r8d, %r8d
        js        ETCH_LABEL(fail)      // EMPTY or TOMBSTONE

/*
 * Now try to see if we find it.
 *
 * %r8: index into the array to test against key
 * %edx: key->hash()
 * %rsi: StringData* key
 *
 * Preserves: rax, rbx, rdi, rsi, r9, r10
 * Can use: rcx, r8, r11
 */
ETCH_LABEL(cmphash):
        lea       (%r8, %r8, 2), %r8
        lea       (%rdi, %r8, 8), %r8
        cmp       %edx, ELM_HASH(%r8)   // Same hash?
        jne       ETCH_LABEL(nextprobe)

        mov       ELM_KEY(%r8), %r11    // StringData* in the table
        cmp       %rsi, %r11            // Same pointer?
        jne       ETCH_LABEL(cmplength)
ETCH_LABEL(found):
        lea       ELM_DATA(%r8), %rax
        pop       %rbx
        pop       %rbp
        ret

ETCH_LABEL(fail):
/*
 * Use the PF to distinguish between EMPTY and TOMBSTONE.
 */
        jnp       ETCH_LABEL(nextprobe) // Tombstone
        xor       %eax, %eax
        pop       %rbx
        pop       %rbp
        ret

ETCH_LABEL(cmplength):
        mov       SD_LEN(%rsi), %ecx    // string length
        cmp       %ecx, SD_LEN(%r11)
        jne       ETCH_LABEL(nextprobe)

        neg       %rcx
        jz        ETCH_LABEL(found)     // both emtpy strings

        push      %rdi
        lea       SD_DATA(%r11), %r11      // s->data()
        lea       SD_DATA(%rsi), %rdi      // key->data()
        push      %rdx
        sub       %rcx, %r11
        sub       %rcx, %rdi

ETCH_LABEL(next8bytes):
        mov       (%r11, %rcx), %rdx
        xor       (%rdi, %rcx), %rdx
        add       $8, %rcx
        jns       ETCH_LABEL(tail)

        test      %rdx, %rdx
        jz        ETCH_LABEL(next8bytes)
        pop       %rdx
        pop       %rdi
        jmp       ETCH_LABEL(nextprobe)

ETCH_LABEL(tail):                       // assert(ecx >= 0)
        shl       $3, %ecx
        shl       %cl, %rdx
        test      %rdx, %rdx
        pop       %rdx
        pop       %rdi
        jnz       ETCH_LABEL(nextprobe)

        lea       ELM_DATA(%r8), %rax
        pop       %rbx
        pop       %rbp
        ret

        CFI(endproc)
        ETCH_SIZE(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE)

#endif
