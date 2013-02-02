/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef _X64_UTIL_H_
#define _X64_UTIL_H_

#include <util/asm-x64.h>
#include <runtime/vm/translator/translator-inline.h>

namespace HPHP {
namespace VM {
namespace Transl {

static inline void
translator_not_reached(X64Assembler &a) {
  if (debug) {
    a.  ud2();
  }
}

static inline void
translator_debug_break(X64Assembler &a) {
  if (debug) {
    a.  int3();
  }
}

/*
 * TLS access: XXX we currently only support static-style TLS directly
 * linked off of FS.
 *
 * x86 terminology review: "Virtual addresses" are subject to both
 * segmented translation and paged translation. "Linear addresses" are
 * post-segmentation address, subject only to paging. C and C++ generally
 * only have access to bitwise linear addresses.
 *
 * TLS data live at negative virtual addresses off FS: the first datum
 * is typically at VA(FS:-sizeof(datum)). Linux's x64 ABI stores the linear
 * address of the base of TLS at VA(FS:0). While this is just a convention, it
 * is firm: gcc builds binaries that assume it when, e.g., evaluating
 * "&myTlsDatum".
 *
 * The virtual addresses of TLS data are not exposed to C/C++. To figure it
 * out, we take a datum's linear address, and subtract it from the linear
 * address where TLS starts. If you use the void* variant here, it's up to
 * the programmer to ensure that it really is a TLS address.
 */
static inline void
emitTLSLoad(X64Assembler& a, const void* datum, RegNumber reg) {
  uintptr_t virtualAddress = uintptr_t(datum) - tlsBase();
  a.    fs();
  a.    load_disp32_reg64(virtualAddress, reg);
}

template<typename T>
static inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            RegNumber reg) {
  emitTLSLoad(a, &datum.m_node.m_p, reg);
}

} } }
#endif
