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
#ifndef incl_HPHP_JIT_FUNC_PROLOGUES_X64_H
#define incl_HPHP_JIT_FUNC_PROLOGUES_X64_H

#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct ActRec;
class Func;

namespace JIT { namespace X64 {

//////////////////////////////////////////////////////////////////////

// The funcGuard gets skipped and patched by other code, so we have some
// magic offsets.
constexpr auto kFuncMovImm = 6; // Offset to the immediate for 8 byte Func*
constexpr auto kFuncCmpImm = 4; // Offset to the immediate for 4 byte Func*
constexpr auto kFuncGuardLen = 23;
constexpr auto kFuncGuardShortLen = 14;

template<typename T>
T* funcPrologueToGuardImm(Transl::TCA prologue) {
  assert(arch() == Arch::X64);
  assert(sizeof(T) == 4 || sizeof(T) == 8);
  T* retval = (T*)(prologue - (sizeof(T) == 8 ?
                               kFuncGuardLen - kFuncMovImm :
                               kFuncGuardShortLen - kFuncCmpImm));
  // We padded these so the immediate would fit inside a cache line
  assert(((uintptr_t(retval) ^ (uintptr_t(retval + 1) - 1)) &
          ~(kX64CacheLineSize - 1)) == 0);

  return retval;
}

inline bool funcPrologueHasGuard(Transl::TCA prologue, const Func* func) {
  assert(arch() == Arch::X64);
  intptr_t iptr = uintptr_t(func);
  if (deltaFits(iptr, sz::dword)) {
    return *funcPrologueToGuardImm<int32_t>(prologue) == iptr;
  }
  return *funcPrologueToGuardImm<int64_t>(prologue) == iptr;
}

inline TCA funcPrologueToGuard(TCA prologue, const Func* func) {
  assert(arch() == Arch::X64);
  if (!prologue || prologue == tx64->uniqueStubs.fcallHelperThunk) {
    return prologue;
  }
  return prologue -
    (deltaFits(uintptr_t(func), sz::dword) ?
     kFuncGuardShortLen :
     kFuncGuardLen);
}

inline void funcPrologueSmashGuard(Transl::TCA prologue, const Func* func) {
  intptr_t iptr = uintptr_t(func);
  if (deltaFits(iptr, sz::dword)) {
    *funcPrologueToGuardImm<int32_t>(prologue) = 0;
    return;
  }
  *funcPrologueToGuardImm<int64_t>(prologue) = 0;
}

//////////////////////////////////////////////////////////////////////

Transl::TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs);

SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& stubsCode,
                        Func* func, bool funcIsMagic, int nPassed,
                        TCA& start, TCA& aStart);

}}}

#endif
