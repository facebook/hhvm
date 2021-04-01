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

  auto const off = [&] () -> int32_t {
    auto const callerFPOff = offsetOfFrame(inst->src(1));
    if (!callerFPOff) return 0;

    auto const calleeFPInst = inst->src(0)->inst();
    assertx(calleeFPInst->is(BeginInlining));
    auto const calleeFPOff = calleeFPInst->extra<BeginInlining>()->spOffset;
    return *callerFPOff - calleeFPOff;
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
  auto const callerFPOff = offsetOfFrame(inst->src(1));
  if (!callerFPOff) {
    // Offset to the caller's FP not known, use callerFP SSA.
    auto const callerFP = srcLoc(env, inst, 1).reg();
    v << popvmfp{callerFP};
  } else {
    // Calculate the offset to the caller's FP and use it to update FP.
    auto const calleeFPInst = inst->src(0)->inst();
    assertx(calleeFPInst->is(BeginInlining));
    auto const calleeFPOff = calleeFPInst->extra<BeginInlining>()->spOffset;
    auto const calleeFP = srcLoc(env, inst, 0).reg();
    auto const tmp = v.makeReg();
    v << lea{calleeFP[cellsToBytes(*callerFPOff - calleeFPOff)], tmp};
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
