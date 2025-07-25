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

#include "hphp/runtime/vm/jit/inline-stitching.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/mutation.h"


namespace HPHP::jit::irgen {

TRACE_SET_MOD(inline_stitching);

void ftraceInlineStitchMetadata(InlineStitchingContext& ctx) {
  UNUSED auto const callerSk = ctx.callerSk;
  UNUSED auto const calleeSk = ctx.calleeSk;

  FTRACE(2, "Stitching callee unit for caller={} callee={}\n",
         callerSk.func()->name()->toCppString(),
         calleeSk.func()->name()->toCppString());
  FTRACE(2, "Caller irSPOff={}, bcSPOff={}\n",
         irgen::offsetFromIRSP(ctx.env, BCSPRelOffset{0}).offset,
         ctx.irb->fs().bcSPOff().offset);
  FTRACE(2, "Caller numSlotsInFrame={} numFuncEntryInputs={} numInOutParams={}\n", callerSk.func()->numSlotsInFrame(),
         callerSk.func()->numFuncEntryInputs(), callerSk.func()->numInOutParams());
  FTRACE(2, "Callee numSlotsInFrame={} numFuncEntryInputs={} numInOutParams={}\n", calleeSk.func()->numSlotsInFrame(),
         calleeSk.func()->numFuncEntryInputs(), calleeSk.func()->numInOutParams());

  FTRACE(4, "Caller unit before stitching:\n{}\n", show(ctx.callerUnit));
  FTRACE(4, "Callee unit:\n{}\n", show(ctx.calleeUnit));
}

int32_t spDeltaToAdjustCallee(InlineStitchingContext& ctx) {
  // Right before inlining:
  // The caller's stack depth includes 
  // - caller number of slots in frame (locals and iterators)
  // - various eval slots
  // - callee inouts
  // - callee ActRec
  // - callee number of function inputs

  // The callee's stack depth includes
  // - caller number of slots in frame (locals and iterators)
  // - callee inouts
  // - callee ActRec

  // In order to adjust callee's stack offsets to match caller's, we need to
  // cancel current caller irspOff by the following:
  // - caller number of slots in frame (locals and iterators)
  // - callee inouts
  // - callee ActRec
  // - callee number of function inputs
  // This will leave us with eval slots to adjust by. The callee unit
  // can clobber the stack offsets used by caller to store callee function inputs, but
  // that is okay because we will stitch in input SSATmp during stitching.
  int irspOff = irgen::offsetFromIRSP(ctx.env, BCSPRelOffset{0}).offset;
  auto const caller = ctx.callerSk.func();
  auto const callee = ctx.calleeSk.func();

  return irspOff + caller->numSlotsInFrame() + callee->numInOutParams() +
         kNumActRecCells + callee->numFuncEntryInputs();
}

void stitchCalleeUnit(InlineStitchingContext& ctx) {
  ftraceInlineStitchMetadata(ctx);
  FTRACE(2, "Delta to adjust callee's stack offsets: {}\n", spDeltaToAdjustCallee(ctx));
}

//////////////////////////////////////////////////////////////////////

}
