/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_CODE_GEN_TLS_H_
#define incl_HPHP_VM_CODE_GEN_TLS_H_

#include "hphp/util/thread-local.h"

namespace HPHP { namespace jit {

struct Vout;
struct Vreg;

///////////////////////////////////////////////////////////////////////////////

/*
 * Thread-local variable abstraction.
 *
 * This serves as a typed wrapper for the thread-local variables accepted by
 * the emit routines below.
 *
 * It exists in order to provide some encapsulation for the tricky macrology
 * required on x64 OSX to obtain the memory locations of thread locals.
 */
template<typename T>
struct TLSDatum {
  explicit TLSDatum(const T& var) : tls{&var} {}
  explicit TLSDatum(const long* addr) : raw{addr} {}

  union {
    const T* tls;
    const long* raw;
  };
};

#if !(defined(__x86_64__) && defined(__APPLE__))
/*
 * Wrapper helper for TLSDatum.
 */
template<typename T>
TLSDatum<T> tls_datum(const T& var) { return TLSDatum<T>(var); }

#else
/*
 * See code-gen-tls-x64.h for an explanation of this assembly.
 */
#define tls_datum(var) ([] {                \
  long* ret;                                \
  __asm__("lea %1, %%rax\nmov %%rdi, %0" :  \
          "=r"(ret) : "m"(var));            \
  return TLSDatum<decltype(var)>(ret);      \
}())

#endif

///////////////////////////////////////////////////////////////////////////////

/*
 * Return the Vptr for the location of a __thread variable `datum'.
 */
template<typename T>
Vptr emitTLSAddr(Vout& v, TLSDatum<T> datum);

/*
 * Load the value of the ThreadLocalNoCheck `datum' into `d'.
 */
template<typename T>
void emitTLSLoad(Vout& v, TLSDatum<ThreadLocalNoCheck<T>> datum, Vreg d);

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/code-gen-tls-x64.h"

// This has to follow all the arch-specific includes.
#include "hphp/runtime/vm/jit/code-gen-tls-inl.h"

#endif
