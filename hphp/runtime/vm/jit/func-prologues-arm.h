#ifndef incl_HPHP_JIT_FUNC_PROLOGUES_ARM_H
#define incl_HPHP_JIT_FUNC_PROLOGUES_ARM_H

#include "hphp/util/data-block.h"

#include "hphp/vixl/a64/instructions-a64.h"

#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct Func;

namespace JIT { namespace ARM {

inline const Func** funcPrologueToGuardImmPtr(Transl::TCA prologue) {
  assert(arch() == Arch::ARM);
  return reinterpret_cast<const Func**>(prologue) - 1;
}

inline bool funcPrologueHasGuard(Transl::TCA prologue, const Func* func) {
  assert(arch() == Arch::ARM);
  return *funcPrologueToGuardImmPtr(prologue) == func;
}

inline TCA funcPrologueToGuard(TCA prologue, const Func* func) {
  assert(arch() == Arch::ARM);
  if (!prologue || prologue == tx64->uniqueStubs.fcallHelperThunk) {
    return prologue;
  }

  using namespace vixl;
  // Skip backwards over:
  // - The guarded Func* (8 bytes)
  // - The address of the redispatch stub (8 bytes)
  // - OPTIONALLY a Nop (4 bytes)
  auto beforeImm = Instruction::Cast(prologue - 20);
  if (beforeImm->Mask(~ImmHint_mask) == HINT &&
      beforeImm->Mask(ImmHint_mask) == NOP) {
    // There was a nop before the immediate. Now skip backwards over 6
    // instructions. See emitFuncGuard().
    return prologue - 20 - 6 * kInstructionSize;
  } else {
    // No nop. Only need to skip over 5 instructions.
    return prologue - 20 - 5 * kInstructionSize;
  }
}

inline void funcPrologueSmashGuard(Transl::TCA prologue, const Func* func) {
  *funcPrologueToGuardImmPtr(prologue) = nullptr;
}

//////////////////////////////////////////////////////////////////////

Transl::TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs);

SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& stubsCode,
                        Func* func, bool funcIsMagic, int nPassed,
                        Transl::TCA& start, Transl::TCA& aStart);

}}}

#endif
