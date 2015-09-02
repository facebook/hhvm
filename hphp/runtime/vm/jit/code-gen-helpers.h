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

#ifndef incl_HPHP_VM_CODEGENHELPERS_H_
#define incl_HPHP_VM_CODEGENHELPERS_H_

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/abi-cxx.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Information about an array key.
 *
 * This represents however much we know about whether the key is going to
 * behave like an integer or a string.
 */
struct ArrayKeyInfo {
  int64_t convertedInt{0};
  KeyType type{KeyType::Any};

  // If true, the string could dynamically contain an integer-like string,
  // which needs to be checked.
  bool checkForInt{false};

  // If true, useKey is an integer constant we've materialized, by converting a
  // string `key' that was strictly an integer.
  bool converted{false};
};

inline ArrayKeyInfo checkStrictlyInteger(Type key) {
  auto ret = ArrayKeyInfo{};

  if (key <= TInt) {
    ret.type = KeyType::Int;
    return ret;
  }
  assertx(key <= TStr);
  ret.type = KeyType::Str;
  if (key.hasConstVal()) {
    int64_t i;
    if (key.strVal()->isStrictlyInteger(i)) {
      ret.converted    = true;
      ret.type         = KeyType::Int;
      ret.convertedInt = i;
    }
  } else {
    ret.checkForInt = true;
  }

  return ret;
}

#ifdef USE_GCC_FAST_TLS
#ifdef __APPLE__
#define getGlobalAddrForTls(datum) ([] {                \
    long* ret;                                          \
    __asm__("lea %1, %%rax\nmov %%rdi, %0" :            \
            "=r"(ret) : "m"(datum));                    \
    return ret;                                         \
  }())

#define emitTLSAddr(x, datum, r)                        \
  detail::implTLSAddr((x), getGlobalAddrForTls(datum), (r))
#define emitTLSLoad(x, datum, reg)                      \
  detail::implTLSLoad((x), (datum), getGlobalAddrForTls(datum), (reg))
#else
#define emitTLSAddr(x, datum, r)                        \
  detail::implTLSAddr((x),(datum),(r))
#define emitTLSLoad(x, datum, reg)                      \
  detail::implTLSLoad((x), (datum), nullptr, (reg))
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

}}

#endif
