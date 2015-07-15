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

#if defined(__x86_64__) && !defined(__CYGWIN__) && !defined(__MINGW__)
              .file "hphp/util/memcpy-x64.S"
              ETCH_SECTION(memcpy)

/*
 * _memcpy_short is a local helper used when length < 8. It cannot be called
 * from outside, because it expects a non-standard calling convention:
 *
 *    %rax:  destination buffer address.
 *    %rsi:  source buffer address.
 *    %edx:  length, in the range of [0, 7]
 */
              ETCH_TYPE(ETCH_NAME(_memcpy_short), @function)
ETCH_NAME(_memcpy_short):
ETCH_LABEL(SHORT):
              CFI(startproc)
              //    if (length == 0) return;
              test  %edx, %edx
              jz    ETCH_LABEL(END)

              //    We can safely read a byte here.
              movzbl (%rsi), %ecx
              //    if (length - 4 < 0) goto S4 ;
              sub   $4, %edx
              jb    ETCH_LABEL(S4)
              mov   (%rsi), %ecx
              mov   (%rsi, %rdx), %edi
              mov   %ecx, (%rax)
              mov   %edi, (%rax, %rdx)
ETCH_LABEL(END):
              rep
              ret
              nop

ETCH_LABEL(S4):
              //    At this point, length can be 1 or 2 or 3, and $cl contains
              //    the first byte.
              mov   %cl, (%rax)
              //    if (length - 4 + 2 < 0) return;
              add   $2, %edx
              jnc   ETCH_LABEL(END)

              //    length is 2 or 3 here. In either case, just copy the last
              //    two bytes.
              movzwl (%rsi, %rdx), %ecx
              mov   %cx, (%rax, %rdx)
              ret

              CFI(endproc)
              ETCH_SIZE(_memcpy_short)


/*
 * void* _memcpy8(void* dst, void* src, uint32_t length);
 *
 * This is the same as
 *     memcpy(dst, src, (length + 7) / 8 * 8)
 * except that it returns dst + length instead of dst. It always copy 8-byte
 * groups, and could overrun the buffers. If both src and dst starts at
 * addresses aligned to 8-byte boundary, it is generally safe.
 *
 * Note: we limit the length to a 32-bit integer, because we don't
 * really copy very long things, and this helps reduce code size.
 */
              ETCH_ALIGN16
              .globl ETCH_NAME(_memcpy8)
              ETCH_TYPE(ETCH_NAME(_memcpy8), @function)
ETCH_NAME(_memcpy8):
              CFI(startproc)
              lea   (%rdi, %rdx), %rax
              add   $7, %edx
              mov   %edx, %ecx
              and   $-8, %edx
              jnz   ETCH_LABEL(L8)
              ret

              CFI(endproc)
              ETCH_SIZE(_memcpy8)

/*
 * void* memcpy(void* dst, void* src, uint32_t length);
 *
 * Note: we limit the length to a 32-bit integer here to save code
 * size. It works for hhvm.
 */
              ETCH_ALIGN16
              .globl ETCH_NAME(memcpy)
              ETCH_TYPE(ETCH_NAME(memcpy), @function)
ETCH_NAME(memcpy):
              CFI(startproc)

              mov   %rdx, %rcx
              mov   %rdi, %rax
              cmp   $8, %edx
              jb    ETCH_LABEL(SHORT)

              mov   -8(%rsi, %rdx), %r8
ETCH_LABEL(L8):
              mov   (%rsi), %r9
              //    This stores garbage if coming from _memcpy8, but it won't
              //    cause corretness problem. The address is writable, and the
              //    data there will be overwritten later. I don't want to move
              //    this store before L8 because that will slow down the
              //    loading of (%rsi).
              mov   %r8, -8(%rdi, %rdx)
              and   $24, %ecx
              jz    ETCH_LABEL(L32)

              mov   %r9, (%rdi)
              mov   %ecx, %r8d
              sub   $16, %ecx
              jb    ETCH_LABEL(T32)

              movdqu (%rsi, %rcx), %xmm1
              movdqu %xmm1, (%rdi, %rcx)

              // Test if there are 32-byte groups
ETCH_LABEL(T32):
              add   %r8, %rsi
              and   $-32, %edx
              jnz   ETCH_LABEL(R32_adjDI)
ETCH_LABEL(END2):
              rep
              ret
              nop

ETCH_LABEL(R32_adjDI):
              add   %r8, %rdi
/*
 * void* _bcopy32(void* dst, void* src, uint32_t length);
 *
 * This is the same as
 *     assert(length >= 32);
 *     memcpy(dst, src, length / 32 * 32);
 * except that the return value cannot be used.
 */
              .globl ETCH_NAME(_bcopy32)
              ETCH_TYPE(ETCH_NAME(_bcopy32), @function)
ETCH_NAME(_bcopy32):
ETCH_LABEL(L32):
              // Multiples of 32 bytes.
              movdqu (%rsi), %xmm0
ETCH_LABEL(L32_16read):
              movdqu 16(%rsi), %xmm1

              //    if ((edx & 32) == 0) goto R64Byte_32read
              shr   $6, %edx
              jnc   ETCH_LABEL(R64Byte_32read)

              movdqu %xmm0, (%rdi)
              movdqu %xmm1, 16(%rdi)
              lea   32(%rsi), %rsi
              jnz   ETCH_LABEL(R64_adjDI)
              ret

ETCH_LABEL(R64_adjDI):
              add   $32, %rdi

/*
 * void _bcopy_in_64(void* dst, void* src, uint32_t lengthIn64Bytes);
 *
 * This is the same as
 *     assert(lengthIn64Bytes > 0);
 *     memcpy(dst, src, 64 * lengthIn64Bytes);
 * except that the return value cannot be used.
 *
 * Note that the length being copied is 64 * %edx.
 */
              .globl ETCH_NAME(_bcopy_in_64)
              ETCH_TYPE(ETCH_NAME(_bcopy_in_64), @function)
ETCH_NAME(_bcopy_in_64):
ETCH_LABEL(R64Byte):
              // Multiples of 64 bytes.
              movdqu (%rsi), %xmm0
              movdqu 16(%rsi), %xmm1
ETCH_LABEL(R64Byte_32read):
              movdqu 32(%rsi), %xmm2
              movdqu 48(%rsi), %xmm3
              add   $64, %rsi
              movdqu %xmm0, (%rdi)
              movdqu %xmm1, 16(%rdi)
              movdqu %xmm2, 32(%rdi)
              movdqu %xmm3, 48(%rdi)
              add   $64, %rdi
              dec   %edx
              jnz   ETCH_LABEL(R64Byte)
              ret

/*
 * void* _memcpy16(void* dst, void* src, uint32_t length);
 *
 * This is the same as
 *     assert(length % 16 == 0 && length > 0);
 *     memcpy(dst, src, length);
 */
              .globl ETCH_NAME(_memcpy16)
              ETCH_TYPE(ETCH_NAME(_memcpy16), @function)
ETCH_NAME(_memcpy16):
              movdqu -16(%rsi, %rdx), %xmm3
              movdqu (%rsi), %xmm0
              mov   %rdi, %rax

              //    if (length & 16 == 0), it must be at least 32 bytes.
              test  $16, %dl
              jz    ETCH_LABEL(L32_16read)

              movdqu %xmm3, -16(%rdi, %rdx)
              shr   $6, %edx
              jz    ETCH_LABEL(32_OR_0)

              //    We have at least 64 bytes remaining.  CF indicates whether
              //    we need to copy 32 bytes first.
              movdqu 16(%rsi), %xmm1
              jnc   ETCH_LABEL(R64Byte_32read)

              //    Need to copy another 32 bytes and adjust rdi/rsi
              movdqu %xmm0, (%rdi)
              add   $32, %rsi
              movdqu %xmm1, 16(%rdi)
              add   $32, %rdi
              jmp   ETCH_LABEL(R64Byte)

ETCH_LABEL(32_OR_0):
              //    The path for length == 16 comes through several conditional
              //    jumps. We expect (and should make) such sitatuions rare
              //    when this is invoked.
              jnc   ETCH_LABEL(END16)
              movdqu 16(%rsi), %xmm1
              movdqu %xmm0, (%rdi)
              movdqu %xmm1, 16(%rdi)

ETCH_LABEL(END16):
              rep
              ret

              CFI(endproc)
              ETCH_SIZE(memcpy)

              .ident "GCC: (GNU) 4.8.2"
#ifdef __linux__
              .section .note.GNU-stack,"",@progbits
#endif

#endif
