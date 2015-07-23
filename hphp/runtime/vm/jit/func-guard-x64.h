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

#ifndef incl_HPHP_JIT_FUNC_GUARD_X64_H
#define incl_HPHP_JIT_FUNC_GUARD_X64_H

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ActRec;
class Func;

///////////////////////////////////////////////////////////////////////////////

namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * The func guard gets skipped and patched by other code, so we have some
 * magic offsets:
 *
 * kFuncGuardLen:   Size of the guard.
 * kFuncGuardImm:   Offset of the smashable immediate.
 * kFuncGuardSmash: Number of initial bytes that must be smashable.
 *
 * These come in regular and short versions---the latter are used for Func*'s
 * which fit into a signed 32-bit immediate.
 */
constexpr auto kFuncGuardLen = 20;
constexpr auto kFuncGuardImm = 2;
constexpr auto kFuncGuardSmash = 10;  // mov $func, %rax

constexpr auto kFuncGuardShortLen = 14;
constexpr auto kFuncGuardShortImm = 4;
constexpr auto kFuncGuardShortSmash = 8;  // cmp $func, 0x10(%rbp)

///////////////////////////////////////////////////////////////////////////////

namespace detail {

template<typename T>
T* funcPrologueToGuardImm(TCA prologue) {
  assertx(sizeof(T) == 4 || sizeof(T) == 8);
  T* retval = (T*)(prologue - (sizeof(T) == 8 ?
                               kFuncGuardLen - kFuncGuardImm :
                               kFuncGuardShortLen - kFuncGuardShortImm));
  // We padded these so the immediate would fit inside a cache line
  assertx(((uintptr_t(retval) ^ (uintptr_t(retval + 1) - 1)) &
          ~kCacheLineMask) == 0);

  return retval;
}

}

///////////////////////////////////////////////////////////////////////////////

inline bool funcPrologueHasGuard(TCA prologue, const Func* func) {
  intptr_t iptr = uintptr_t(func);
  if (deltaFits(iptr, sz::dword)) {
    return *detail::funcPrologueToGuardImm<int32_t>(prologue) == iptr;
  }
  return *detail::funcPrologueToGuardImm<int64_t>(prologue) == iptr;
}

inline TCA funcPrologueToGuard(TCA prologue, const Func* func) {
  if (!prologue || prologue == mcg->tx().uniqueStubs.fcallHelperThunk) {
    return prologue;
  }
  return prologue -
    (deltaFits(uintptr_t(func), sz::dword) ? kFuncGuardShortLen :
                                             kFuncGuardLen);
}

inline void funcPrologueSmashGuard(TCA prologue, const Func* func) {
  intptr_t iptr = uintptr_t(func);
  if (deltaFits(iptr, sz::dword)) {
    *detail::funcPrologueToGuardImm<int32_t>(prologue) = 0;
    return;
  }
  *detail::funcPrologueToGuardImm<int64_t>(prologue) = 0;
}

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
