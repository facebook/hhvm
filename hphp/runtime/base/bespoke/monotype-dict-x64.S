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
#include "hphp/runtime/base/string-data-macros.h"
#include "hphp/runtime/base/bespoke/monotype-dict-x64.h"
#include "hphp/util/etch-helpers.h"

#if defined(__SSE4_2__) && defined(NO_M_DATA) && !defined(NO_HWCRC) && \
    !defined(_MSC_VER)

        .file     "hphp/runtime/base/bespoke/monotype-dict-x64.S"

.macro implement_monotype_getimpl MANGLED_NAME
ETCH_SECTION(\MANGLED_NAME)
        .globl   \MANGLED_NAME

/*
 * HPHP::StringDict::getImpl(const ArrayData* ad, const StringData* k);
 */

        ETCH_ALIGN16
        ETCH_TYPE(ETCH_NAME(\MANGLED_NAME), @function)
ETCH_NAME(\MANGLED_NAME):
        CFI(startproc)

        mov       SD_HASH(%rsi), %edx                // key->hash
        mov       MD_SIZE_CLASS_OFFSET(%rdi), %cl    // size class
        add       $MD_DATA, %rdi                     // rdi: ad->elms()
        push      %rbx

        sub        $MD_MIN_SIZE_CLASS, %cl
        shr        $MD_SH_SIZE_CLASSES, %cl
        mov        $4, %rax
        shl        %cl, %rax

        lea       (%rax, %rax, 2), %r10
        lea       -16(%rdi, %r10, 8), %r10 // ad->indices()
        lea       -1(, %rax, 2), %ebx      // mask
        and       $0x7fffffff, %edx
        jz        ETCH_LABEL(hashHelper_GetImpl\MANGLED_NAME)

ETCH_LABEL(hasHash\MANGLED_NAME):
        xor       %r9, %r9              // probe count
        mov       %edx, %eax            // eax: index into hashTab

ETCH_LABEL(nextProbe\MANGLED_NAME):
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
ETCH_LABEL(cmpHash\MANGLED_NAME):
        lea       (%r8, %r8), %r8
        lea       (%rdi, %r8, 8), %r8
        mov       MD_KEY(%r8), %r11

        cmp       %rsi, %r11            // Same pointer?
        jne       ETCH_LABEL(cmpLength\MANGLED_NAME)
ETCH_LABEL(found\MANGLED_NAME):
        mov       MD_VAL(%r8), %rax
        movzbl    MD_TYPE_NEG(%rdi), %edx
        pop       %rbx
        ret

ETCH_LABEL(fail\MANGLED_NAME):
/*
 * Use the PF to distinguish between EMPTY and TOMBSTONE.
 * If the element is missing, we only need to clear the type register.
 */
        jnp       ETCH_LABEL(nextProbe\MANGLED_NAME) // Tombstone
        mov       $UNINIT_DT_VALUE, %edx
        pop       %rbx
        ret

ETCH_LABEL(cmpLength\MANGLED_NAME):
        mov       SD_LEN(%rsi), %ecx    // string length
        cmp       %ecx, SD_LEN(%r11)
        jne       ETCH_LABEL(nextProbe\MANGLED_NAME)

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
        jmp       ETCH_LABEL(nextProbe\MANGLED_NAME)

ETCH_LABEL(tail\MANGLED_NAME):                       // assert(ecx >= 0)
        shl       $3, %ecx
        shl       %cl, %rdx
        test      %rdx, %rdx
        pop       %rdx
        pop       %rdi
        jnz       ETCH_LABEL(nextProbe\MANGLED_NAME)
        mov       MD_VAL(%r8), %rax
        movzbl    MD_TYPE_NEG(%rdi), %edx
        pop       %rbx
        ret

ETCH_LABEL(hashHelper_GetImpl\MANGLED_NAME):
        mov       SD_LEN(%rsi), %ecx       // key->size()
        lea       SD_DATA(%rsi, %rcx), %r9 // key->data() + key->size()
        not       %edx                     // initialize to -1 (assert: it was 0)
        neg       %rcx
        jnz       ETCH_LABEL(hashHeader\MANGLED_NAME)
        jmp       ETCH_LABEL(hashEnd\MANGLED_NAME)
ETCH_LABEL(hashLoop\MANGLED_NAME):
        crc32q    %r11, %rdx
ETCH_LABEL(hashHeader\MANGLED_NAME):
        movabs    $0xdfdfdfdfdfdfdfdf, %r11
        and       (%r9, %rcx), %r11
        add       $8, %rcx
        js        ETCH_LABEL(hashLoop\MANGLED_NAME)

        shl       $3, %ecx
        shl       %cl, %r11
        crc32q    %r11, %rdx

ETCH_LABEL(hashEnd\MANGLED_NAME):
        shr       %edx
        or        %edx, SD_HASH(%rsi)   // store hash
        jmp       ETCH_LABEL(hasHash\MANGLED_NAME)

        CFI(endproc)
        ETCH_SIZE(\MANGLED_NAME)
.endm

implement_monotype_getimpl _ZNK4HPHP7bespoke12MonotypeDictIPNS_10StringDataEE7getImplES3_

#endif
