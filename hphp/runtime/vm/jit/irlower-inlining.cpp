/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

bool isResumedParent(const IRInstruction* inst) {
  auto const fp = inst->src(0);
  assertx(fp->inst()->is(BeginInlining));

  auto const callerFp = fp->inst()->src(1);
  return callerFp->inst()->marker().resumeMode() != ResumeMode::None;
}

}

void cgBeginInlining(IRLS& env, const IRInstruction* inst) {
  auto const callerSP = srcLoc(env, inst, 0).reg();
  auto const calleeFP = dstLoc(env, inst, 0).reg();
  auto const extra = inst->extra<BeginInlining>();
  auto& v = vmain(env);

  v << inlinestart{extra->func, extra->cost};

  // We could use callerFP in a non-resumed context but vasm-copy should clean
  // this up for us since callerSP was computed as an lea{} from rvmfp.
  v << lea{callerSP[cellsToBytes(extra->spOffset.offset)], calleeFP};
}

void cgInlineCall(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<InlineCall>();
  auto const calleeFP = srcLoc(env, inst, 0).reg();
  auto const callerFP = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  assertx(inst->src(0)->inst()->is(BeginInlining));
  auto const off = [&] () -> int32_t {
    if (isResumedParent(inst)) return 0;
    auto const be = inst->src(0)->inst();
    auto const defsp = be->src(0)->inst();
    auto const parentToSp = [&] {
      if (be->src(1)->inst()->is(BeginInlining)) {
        auto const extra = be->src(1)->inst()->extra<BeginInlining>();
        return FPInvOffset{extra->spOffset.offset};
      }
      return defsp->extra<FPInvOffsetData>()->offset;
    }();
    auto const spoff = inst->src(0)->inst()->extra<BeginInlining>()->spOffset;
    return spoff.to<FPInvOffset>(parentToSp).offset;
  }();

  // Do roughly the same work as an HHIR Call.
  v << store{callerFP, calleeFP[AROFF(m_sfp)]};
  emitImmStoreq(v, uintptr_t(tc::ustubs().retInlHelper),
                calleeFP[AROFF(m_savedRip)]);

  if (extra->syncVmpc) {
    // If we are in a catch block, update the vmfp() to point to the inlined
    // frame if it was pointing to the parent frame, letting the unwinder see
    // the inlined frame.
    auto const sf = v.makeReg();
    v << cmpqm{callerFP, rvmtl()[rds::kVmfpOff], sf};

    // Do this store now to hopefully allow vasm-copy to replace the store of
    // calleeFP below with rvmfp.
    v << pushvmfp{calleeFP, cellsToBytes(off)};

    ifThen(v, CC_E, sf, [&](Vout& v) {
      v << store{calleeFP, rvmtl()[rds::kVmfpOff]};
      emitImmStoreq(v, intptr_t(extra->syncVmpc), rvmtl()[rds::kVmpcOff]);
    });
  } else {
    v << pushvmfp{calleeFP, cellsToBytes(off)};
  }

  v << pushframe{};
}

void cgInlineReturn(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const callerFp = srcLoc(env, inst, 1).reg();
  if (isResumedParent(inst)) {
    v << popvmfp{callerFp};
  } else {
    auto const tmp = v.makeReg();
    auto const callerFPOff = inst->extra<InlineReturn>()->offset;
    v << lea{fp[cellsToBytes(callerFPOff.offset)], tmp};
    v << popvmfp{tmp};
  }
  v << popframe{};
}

void cgEndInlining(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const fp = srcLoc(env, inst, 0).reg();

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    if (env.unit.context().initSrcKey.resumeMode() == ResumeMode::None) {
      for (auto i = 0; i < kNumActRecCells; ++i) {
        trashFullTV(v, fp[i], kTVTrashJITFrame);
      }
    }
  }

  v << inlineend{};
}

void cgSyncReturnBC(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<SyncReturnBC>();
  auto const coaf = extra->callBCOffset << ActRec::CallOffsetStart;
  auto const spOffset = cellsToBytes(extra->spOffset.offset);
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const mask = (1 << ActRec::CallOffsetStart) - 1;

  auto& v = vmain(env);
  auto const oldCoaf = v.makeReg();
  auto const newCoaf = v.makeReg();
  auto const flags = v.makeReg();
  v << loadl{sp[spOffset + AROFF(m_callOffAndFlags)], oldCoaf};
  v << andli{mask, oldCoaf, flags, v.makeReg()};
  v << orli{coaf, flags, newCoaf, v.makeReg()};
  v << storel{newCoaf, sp[spOffset + AROFF(m_callOffAndFlags)]};
  v << store{fp, sp[spOffset + AROFF(m_sfp)]};
}

///////////////////////////////////////////////////////////////////////////////

void cgConjure(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0);
  auto& v = vmain(env);
  if (dst.hasReg(0)) {
    v << conjure{dst.reg(0)};
  }
  if (dst.hasReg(1)) {
    v << conjure{dst.reg(1)};
  }
}

void cgConjureUse(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0);
  auto& v = vmain(env);
  if (src.hasReg(0)) {
    v << conjureuse{src.reg(0)};
  }
  if (src.hasReg(1)) {
    v << conjureuse{src.reg(1)};
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
