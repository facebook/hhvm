#ifndef incl_HPHP_JIT_FUNC_PROLOGUES_H
#define incl_HPHP_JIT_FUNC_PROLOGUES_H

#include "hphp/runtime/vm/jit/func-prologues-arm.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace JIT {

inline bool funcPrologueHasGuard(Transl::TCA prologue, const Func* func) {
  if (arch() == Arch::X64) {
    return X64::funcPrologueHasGuard(prologue, func);
  } else if (arch() == Arch::ARM) {
    return ARM::funcPrologueHasGuard(prologue, func);
  }
  not_implemented();
}

inline Transl::TCA funcPrologueToGuard(Transl::TCA prologue, const Func* func) {
  if (arch() == Arch::X64) {
    return X64::funcPrologueToGuard(prologue, func);
  } else if (arch() == Arch::ARM) {
    return ARM::funcPrologueToGuard(prologue, func);
  }
  not_implemented();
}

}}

#endif
