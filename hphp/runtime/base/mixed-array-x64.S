#include "hphp/util/etch-helpers.h"

#if defined(__x86_64__) && defined(FACEBOOK) && !defined(NO_SSE42)

        .file     "hphp/runtime/base/mixed-array-x64.S"

ETCH_SECTION(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE)
        .globl    _ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE

ETCH_TYPE(ETCH_NAME(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE), @function)

ETCH_NAME(hashHelper_NvGetStr):
ETCH_LABEL(hashHelper_NvGetStr):
        mov       0x10(%rsi), %ecx      // key->size()
        mov       (%rsi), %r9           // key->data()
        not       %edx                  // initialize to -1 (assert: it was 0)
        movabs    $0xdfdfdfdfdfdfdfdf, %r8
        neg       %ecx
        jnz       ETCH_LABEL(hheader)
        jmp       ETCH_LABEL(hend)
ETCH_LABEL(hloop):
        add       $8, %r9
        crc32q    %r11, %rdx
ETCH_LABEL(hheader):
        mov       (%r9), %r11
        and       %r8, %r11
        add       $8, %ecx
        js        ETCH_LABEL(hloop)

        shl       $3, %ecx
        shl       %cl, %r11
        crc32q    %r11, %rdx

ETCH_LABEL(hend):
        shr       %edx
        or        %edx, 0x14(%rsi)
        jmp       ETCH_LABEL(hasHash)

/*
 * HPHP::MixedArray::NvGetStr(const ArrayData* ad, const StringData* k);
 */
        ETCH_ALIGN16
ETCH_NAME(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE):
        CFI(startproc)

        mov       0x14(%rsi), %edx      // hash
        mov       0x10(%rdi), %eax      // scale
        push      %rbp
        mov       %rsp, %rbp
        add       $0x20, %rdi           // rdi: ad->data()
        push      %rbx
        or        $-1, %ebx
        lea       (%ebx, %eax, 4), %ebx // mask = scale * 4 -1
        lea       (%rax, %rax, 8), %rax
        lea       (%rdi, %rax, 8), %r10 // ad->hashTab()
        and       $0x7fffffff, %edx
        jz        ETCH_LABEL(hashHelper_NvGetStr)

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
        cmp       %edx, 0x14(%r8)       // Same hash?
        jne       ETCH_LABEL(nextprobe)

        mov       (%r8), %r11           // StringData* in the table
        cmp       %rsi, %r11            // Same pointer?
        jne       ETCH_LABEL(cmplength)
ETCH_LABEL(found):
        lea       0x8(%r8), %rax
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
        mov       0x10(%rsi), %ecx      // string length
        cmp       %ecx, 0x10(%r11)
        jne       ETCH_LABEL(nextprobe)

        neg       %rcx
        jz        ETCH_LABEL(found)     // both emtpy strings

        push      %rdi
        mov       (%r11), %r11
        mov       (%rsi), %rdi
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

        lea       0x8(%r8), %rax
        pop       %rbx
        pop       %rbp
        ret

        CFI(endproc)
        ETCH_SIZE(_ZN4HPHP10MixedArray8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE)

#endif
