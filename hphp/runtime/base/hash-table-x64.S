/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/datatype-macros.h"
#include "hphp/runtime/base/hash-table-x64.h"
#include "hphp/runtime/base/string-data-macros.h"
#include "hphp/util/etch-helpers.h"

#if defined(__SSE4_2__) && defined(NO_M_DATA) && !defined(NO_HWCRC) && \
    !defined(_MSC_VER)

        .file     "hphp/runtime/base/hash-table-x64.S"
.macro implement_nvgetstr MANGLED_NAME SCALE DATA ELM_HASH ELM_KEY ELM_DATA ELM_TYPE ELM_QUADWORDS
ETCH_SECTION(\MANGLED_NAME)
        .globl   \MANGLED_NAME

/*
 * HPHP::MixedArray::NvGetStr(const ArrayData* ad, const StringData* k);
 */

        ETCH_ALIGN16
        ETCH_TYPE(ETCH_NAME(\MANGLED_NAME), @function)
ETCH_NAME(\MANGLED_NAME):
        CFI(startproc)

        mov       SD_HASH(%rsi), %edx   // key->hash
        mov       \SCALE(%rdi), %eax  // scale
        and       $0x7fffffff, %edx
        jz        ETCH_LABEL(hashHelper_NvGetStr\MANGLED_NAME)
ETCH_LABEL(hasHash\MANGLED_NAME):
        add       $\DATA, %rdi        // rdi: ad->data()
        lea       -1(, %eax, 4), %r9d // mask = scale * 4 -1
.if \ELM_QUADWORDS==2
        lea       (%rax, %rax, 2), %rax
        lea       (%rax, %rax), %rax
.else
        lea       (%rax, %rax, 8), %rax
.endif
        lea       (%rdi, %rax, 8), %r10 // ad->hashTab()

ETCH_LABEL(unrollFirst\MANGLED_NAME):
        mov       %edx, %eax            // eax: index into hashTab
        and       %r9d, %eax
        mov       (%r10, %rax, 4), %r8d
        test      %r8d, %r8d
        js        ETCH_LABEL(failTrampoline\MANGLED_NAME)      // EMPTY or TOMBSTONE
        lea       (%r8, %r8, \ELM_QUADWORDS - 1), %r8
        lea       (%rdi, %r8, 8), %r8
        cmp       %edx, \ELM_HASH(%r8)   // Same hash?
        jne       ETCH_LABEL(firstprobe\MANGLED_NAME)
        mov       \ELM_KEY(%r8), %r11    // StringData* in the table
        cmp       %rsi, %r11            // Same pointer?
        jne       ETCH_LABEL(cmplengthTrampoline\MANGLED_NAME)
        mov       \ELM_DATA(%r8), %rax
        movzbl    \ELM_TYPE(%r8), %edx
        ret

ETCH_LABEL(failTrampoline\MANGLED_NAME):
        jnp       ETCH_LABEL(firstprobe\MANGLED_NAME) // Tombstone
        mov       $UNINIT_DT_VALUE, %edx
        ret

ETCH_LABEL(cmplengthTrampoline\MANGLED_NAME):
        push      %rbx
        mov       %r9d, %ebx
        mov       $1, %r9d
        jmp       ETCH_LABEL(cmplength\MANGLED_NAME)

ETCH_LABEL(firstprobe\MANGLED_NAME):
        push      %rbx
        mov       %r9d, %ebx
        mov       $1, %r9d

ETCH_LABEL(nextprobe\MANGLED_NAME):
        add       %r9d, %eax
        and       %ebx, %eax
        mov       (%r10, %rax, 4), %r8d
        inc       %r9d
        test      %r8d, %r8d
        js        ETCH_LABEL(fail\MANGLED_NAME)      // EMPTY or TOMBSTONE

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
ETCH_LABEL(cmphash\MANGLED_NAME):
        lea       (%r8, %r8, \ELM_QUADWORDS - 1), %r8
        lea       (%rdi, %r8, 8), %r8
        cmp       %edx, \ELM_HASH(%r8)   // Same hash?
        jne       ETCH_LABEL(nextprobe\MANGLED_NAME)

        mov       \ELM_KEY(%r8), %r11    // StringData* in the table
        cmp       %rsi, %r11            // Same pointer?
        jne       ETCH_LABEL(cmplength\MANGLED_NAME)
ETCH_LABEL(found\MANGLED_NAME):
        mov       \ELM_DATA(%r8), %rax
        movzbl    \ELM_TYPE(%r8), %edx
        pop       %rbx
        ret

ETCH_LABEL(fail\MANGLED_NAME):
/*
 * Use the PF to distinguish between EMPTY and TOMBSTONE.
 * If the element is missing, we only need to clear the type register.
 */
        jnp       ETCH_LABEL(nextprobe\MANGLED_NAME) // Tombstone
        mov       $UNINIT_DT_VALUE, %edx
        pop       %rbx
        ret

ETCH_LABEL(cmplength\MANGLED_NAME):
        mov       SD_LEN(%rsi), %ecx    // string length
        cmp       %ecx, SD_LEN(%r11)
        jne       ETCH_LABEL(nextprobe\MANGLED_NAME)

        neg       %rcx
        jz        ETCH_LABEL(found\MANGLED_NAME)     // both emtpy strings

        push      %rdi
        lea       SD_DATA(%r11), %r11      // s->data()
        lea       SD_DATA(%rsi), %rdi      // key->data()
        push      %rdx
        sub       %rcx, %r11
        sub       %rcx, %rdi

ETCH_LABEL(next8bytes\MANGLED_NAME):
        mov       (%r11, %rcx), %rdx
        xor       (%rdi, %rcx), %rdx
        add       $8, %rcx
        jns       ETCH_LABEL(tail\MANGLED_NAME)

        test      %rdx, %rdx
        jz        ETCH_LABEL(next8bytes\MANGLED_NAME)
        pop       %rdx
        pop       %rdi
        jmp       ETCH_LABEL(nextprobe\MANGLED_NAME)

ETCH_LABEL(tail\MANGLED_NAME):                       // assert(ecx >= 0)
        shl       $3, %ecx
        shl       %cl, %rdx
        test      %rdx, %rdx
        pop       %rdx
        pop       %rdi
        jnz       ETCH_LABEL(nextprobe\MANGLED_NAME)
        mov       \ELM_DATA(%r8), %rax
        movzbl    \ELM_TYPE(%r8), %edx
        pop       %rbx
        ret

ETCH_LABEL(hashHelper_NvGetStr\MANGLED_NAME):
        mov       SD_LEN(%rsi), %ecx       // key->size()
        lea       SD_DATA(%rsi, %rcx), %r9 // key->data() + key->size()
        not       %edx                     // initialize to -1 (assert: it was 0)
        neg       %rcx
        jnz       ETCH_LABEL(hheader\MANGLED_NAME)
        jmp       ETCH_LABEL(hend\MANGLED_NAME)
ETCH_LABEL(hloop\MANGLED_NAME):
        crc32q    %r11, %rdx
ETCH_LABEL(hheader\MANGLED_NAME):
        movabs    $0xdfdfdfdfdfdfdfdf, %r11
        and       (%r9, %rcx), %r11
        add       $8, %rcx
        js        ETCH_LABEL(hloop\MANGLED_NAME)

        shl       $3, %ecx
        shl       %cl, %r11
        crc32q    %r11, %rdx

ETCH_LABEL(hend\MANGLED_NAME):
        shr       %edx
        or        %edx, SD_HASH(%rsi)   // store hash
        jmp       ETCH_LABEL(hasHash\MANGLED_NAME)

        CFI(endproc)
        ETCH_SIZE(\MANGLED_NAME)
.endm

#define IMPLEMENT_NV_GET_STR(MangledName, ArrayType, ElmType) \
  implement_nvgetstr MangledName ArrayType ## _SCALE ArrayType ## _DATA ElmType ## _HASH ElmType ## _KEY  ElmType ## _DATA ElmType ## _TYPE ElmType ## _QUADWORDS

IMPLEMENT_NV_GET_STR(_ZN4HPHP5array9HashTableINS_10MixedArrayENS_13MixedArrayElmEE8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE, MixedArray, MixedArrayElm)

IMPLEMENT_NV_GET_STR(_ZN4HPHP5array9HashTableINS_8SetArrayENS_11SetArrayElmEE8NvGetStrEPKNS_9ArrayDataEPKNS_10StringDataE, SetArray, SetArrayElm)

#undef IMPLEMENT_NV_GET_STR

#endif
