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

#ifndef incl_HPHP_VM_CODE_GEN_TLS_H_
#define incl_HPHP_VM_CODE_GEN_TLS_H_

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_GCC_FAST_TLS
#ifdef __APPLE__
#define getGlobalAddrForTLS(datum) ([] {                \
    long* ret;                                          \
    __asm__("lea %1, %%rax\nmov %%rdi, %0" :            \
            "=r"(ret) : "m"(datum));                    \
    return ret;                                         \
  }())

#define emitTLSAddr(x, datum, r)                        \
  detail::implTLSAddr((x), getGlobalAddrForTLS(datum), (r))
#define emitTLSLoad(x, datum, reg)                      \
  detail::implTLSLoad((x), (datum), getGlobalAddrForTLS(datum), (reg))
#else // __APPLE__
#define emitTLSAddr(x, datum, r)                        \
  detail::implTLSAddr((x), (datum), (r))
#define emitTLSLoad(x, datum, reg)                      \
  detail::implTLSLoad((x), (datum), nullptr, (reg))
#endif // __APPLE__
#endif // USE_GCC_FAST_TLS

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/code-gen-tls-x64.h"

#endif
