#ifndef incl_HPHP_JIT_CODE_GEN_HELPERS_ARM_H
#define incl_HPHP_JIT_CODE_GEN_HELPERS_ARM_H

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace JIT { namespace ARM {

/*
 * Intelligently chooses between Add, Mov, and no-op.
 */
void emitRegGetsRegPlusImm(vixl::MacroAssembler& as,
                           const vixl::Register& dstReg,
                           const vixl::Register& srcReg,
                           int64_t imm);

/*
 * The callee's half of within-TC calls. The caller uses a native call
 * instruction to get to either the bind-call stub (which needs to know the
 * address to smash) or the eventual destination (which needs to put the return
 * address into the ActRec). This fulfills both needs.
 */
void emitStoreRetIntoActRec(vixl::MacroAssembler& a);

/*
 * All calls should go through here, because they need to be implemented
 * differently depending on whether we're simulating ARM or running native.
 */
void emitCall(vixl::MacroAssembler& a, CppCall call);

/*
 * Swaps two registers. Uses XOR swap, so will not touch memory, flags, or any
 * other registers. XOR swap is the best.
 */
void emitXorSwap(vixl::MacroAssembler& a,
                 const vixl::Register& r1, const vixl::Register& r2);

/*
 * Check the surprise flags. If surprised, call functionEnterHelper.
 */
void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& stubsCode,
                                 bool inTracelet, Transl::FixupMap& fixupMap,
                                 Transl::Fixup fixup);

/*
 * Increments the current (at translation time) translation counter.
 */
void emitTransCounterInc(vixl::MacroAssembler& a);

}}}

#endif
