/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/etch-helpers.h"

/* Guard with FACEBOOK temporarily to avoid breaking OSS users without SSE4.2 */
#if defined(__x86_64__) && defined(FACEBOOK) && !defined(NO_SSE42)
        .file     "hphp/util/strcmp-sse42.S"
        ETCH_SECTION(strcmp)

/*
 * int strcmp (const unsigned char* str1, const unsigned char* str2);
 *
 * Return 0 if equal, negative if str1 is smaller, positive otherwise.
 * Comparison is done for unsigned char.
 *
 * The unsigned behavior is explicitly stated in C99 standard 7.21.4, and also
 * in the IEEE Std. 1003.1: http://pubs.opengroup.org/onlinepubs/9699919799/
 */
        ETCH_ALIGN16
        .globl    ETCH_NAME(strcmp)
        ETCH_TYPE(ETCH_NAME(strcmp), @function)
        CFI(startproc)
ETCH_NAME(strcmp):
/*
 * Check if the first byte is different.
 */
        movzbl    (%rdi), %eax
        movzbl    (%rsi), %ecx
        sub       %ecx, %eax
        jnz       ETCH_LABEL(ret)

/*
 * To avoid risking segfault, compute how many bytes are we away from the next
 * page boundary, and pick the smaller one.  Need to go to the slow path if we
 * may overread the page.
 */
ETCH_LABEL(getsafebytes):
        mov       %edi, %ecx
        and       $0xFFF, %ecx
        mov       %esi, %eax
        and       $0xFFF, %eax
        mov       $0x1000, %edx
        cmp       %ecx, %eax
        cmovb     %ecx, %eax
        sub       %eax, %edx            /* We can safely read %edx bytes */

        xor       %eax, %eax
        sub       %rdi, %rsi            /* (%rdi) vs (%rdi, %rsi) */
        jnz       ETCH_LABEL(cmp16)

ETCH_LABEL(ret):
        repz
        ret

ETCH_LABEL(loop):
        add       $16, %rdi
ETCH_LABEL(cmp16):
        sub       $16, %edx
        js        ETCH_LABEL(slow)      /* read a new page */

        movdqu    (%rdi, %rsi), %xmm1
        pcmpistri $0x18, (%rdi), %xmm1  /* negative polarity, unsigned char */
/*
 * If CF is set, we know that the ecx-th bytes differ.  Because both string
 * must end with 0, and anything beyond the tailing 0 is not included in the
 * comparison due to the polarity setting.
 *
 * If ZF is set, we are seeing the end of a string.
 */
        ja        ETCH_LABEL(loop)      /* !CF && !ZF, keep looking */
        jc        ETCH_LABEL(diff)
        ret                             /* same, assert(eax == 0) */
ETCH_LABEL(diff):                       /* assert rcx < 16 */
        add       %rcx, %rdi
        movzbl    (%rdi), %eax
        movzbl    (%rdi, %rsi), %ecx
        sub       %ecx, %eax
ETCH_LABEL(ret2):
        repz
        ret

ETCH_LABEL(slow):
/*
 * If we reach here, one of %rdi and %rdi + %rsi is within 16 bytes of the
 * next page boundary.  Read till end of the page and go back to the fast path
 * if no difference is found.
 */
#ifdef DEBUG
/*
 * Make sure adding 16 would overread a page, i.e., in the form of 0x.*00[0-F]
 */
        lea       16(%edi), %eax
        test      $0xFE0, %eax
        jz        ETCH_LABEL(slowdbgok) /* rdi + 16 reads a new page*/
ETCH_LABEL(checksi):
        add       %esi, %eax
        test      $0xFE0, %eax
        jnz       ETCH_LABEL(bad)       /* not starting a new page? */
ETCH_LABEL(slowdbgok):
#endif

        add       $16, %edx             /* we can safely read %edx bytes */
        jz        ETCH_LABEL(goback)    /* actually, it was aligned */
ETCH_LABEL(slowloop):
        movzbl    (%rdi), %eax
        movzbl    (%rdi, %rsi), %r8d
        mov       %eax, %ecx
        sub       %r8d, %eax
        jnz       ETCH_LABEL(ret2)
        test      %ecx, %ecx            /* assert(eax == 0) */
        jz        ETCH_LABEL(ret2)      /* same */
        inc       %rdi
        dec       %edx
        jnz       ETCH_LABEL(slowloop)

/*
 * Now try to go back to the fast loop.
 */
ETCH_LABEL(goback):
        add       %rdi, %rsi
        jmp       ETCH_LABEL(getsafebytes)

#ifdef DEBUG
ETCH_LABEL(bad):
        ud2
#endif
        CFI(endproc)
        ETCH_SIZE(strcmp)

        .ident    "GCC: (GNU) 4.8.2"
#ifdef __linux__
        .section  .note.GNU-stack,"",@progbits
#endif

#endif
