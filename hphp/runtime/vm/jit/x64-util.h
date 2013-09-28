/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef X64_UTIL_H_
#define X64_UTIL_H_

#include "hphp/util/asm-x64.h"
#ifndef USE_GCC_FAST_TLS
#include <pthread.h>
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#endif
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
namespace Transl {

static inline void
translator_not_reached(X64Assembler &a) {
  if (debug) {
    a.  ud2();
  }
}

#ifdef USE_GCC_FAST_TLS

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
 * address where TLS starts.
 */
template<typename T>
static inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            RegNumber reg) {
  uintptr_t virtualAddress = uintptr_t(&datum.m_node.m_p) - tlsBase();
  a.    fs();
  a.    load_disp32_reg64(virtualAddress, reg);
}

#else // USE_GCC_FAST_TLS

template<typename T>
static inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            RegNumber reg) {
  PhysRegSaver(a, kGPCallerSaved); // we don't know for sure what's alive
  a.    emitImmReg(&datum.m_key, argNumToRegName[0]);
  a.    call((TCA)pthread_getspecific);
  if (reg != reg::rax) {
    a.    mov_reg64_reg64(reg::rax, reg);
  }
}

#endif // USE_GCC_FAST_TLS

} }
#endif
