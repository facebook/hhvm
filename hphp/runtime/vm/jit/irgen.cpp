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
#include "hphp/runtime/vm/jit/irgen.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/prof-data.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void check_catch_stack_state(IRGS& env, const IRInstruction* inst) {
  always_assert_flog(
    !env.irb->fs().stackModified(),
    "catch block used after writing to stack\n"
    "     inst: {}\n",
    inst->toString()
  );
}

//////////////////////////////////////////////////////////////////////

}

uint64_t curProfCount(const IRGS& env) {
  auto const tid = env.profTransID;
  assertx(tid == kInvalidTransID ||
          (env.region != nullptr && profData() != nullptr));
  return env.profFactor *
    (tid != kInvalidTransID ? env.region->blockProfCount(tid) : 1);
}

uint64_t calleeProfCount(const IRGS& env, const RegionDesc& calleeRegion) {
  auto const tid = calleeRegion.entry()->id();
  if (tid == kInvalidTransID) return 0;
  return env.profFactor * calleeRegion.blockProfCount(tid);
}

//////////////////////////////////////////////////////////////////////

/*
 * This is called when each instruction is generated during initial IR
 * creation.
 *
 * This function inspects the instruction and prepares it for potentially being
 * inserted to the instruction stream.  It then calls IRBuilder optimizeInst,
 * which may or may not insert it depending on a variety of factors.
 */
namespace detail {
SSATmp* genInstruction(IRGS& env, IRInstruction* inst) {
  if (inst->mayRaiseError() && inst->taken()) {
    FTRACE(1, "{}: asserting about catch block\n", inst->toString());
    /*
     * This assertion means you manually created a catch block, but didn't put
     * an exceptionStackBoundary after an update to the stack.  Even if you're
     * manually creating catches we require this just to make sure you're not
     * doing it on accident.
     */
    check_catch_stack_state(env, inst);
  }

  if (inst->mayRaiseError() && !inst->taken()) {
    FTRACE(1, "{}: creating catch block\n", inst->toString());
    /*
     * If you hit this assertion, you're gen'ing an IR instruction that can
     * throw after gen'ing one that could write to the evaluation stack.  This
     * is usually not what HHBC opcodes do, and could be a bug.  See the
     * documentation for exceptionStackBoundary in the header for more
     * information.
     */
    check_catch_stack_state(env, inst);
    inst->setTaken(
      create_catch_block(
        env,
        []{},
        inst->is(Call) ||
        inst->is(CallUnpack) ?
        EndCatchData::SwitchMode : EndCatchData::UnwindOnly
      )
    );
  }

  if (inst->mayRaiseError()) {
    assertx(inst->taken() && inst->taken()->isCatch());
  }

  return env.irb->optimizeInst(inst, IRBuilder::CloneFlag::Yes, nullptr);
}
}

//////////////////////////////////////////////////////////////////////

void incProfCounter(IRGS& env, TransID transId) {
  gen(env, IncProfCounter, TransIDData(transId));
}

void checkCold(IRGS& env, TransID transId) {
  gen(env, CheckCold, makeExitOpt(env), TransIDData(transId));
}

void ringbufferEntry(IRGS& env, Trace::RingBufferType t, SrcKey sk, int level) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, level)) return;
  gen(env, RBTraceEntry, RBEntryData(t, sk));
}

void ringbufferMsg(IRGS& env,
                   Trace::RingBufferType t,
                   const StringData* msg,
                   int level) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, level)) return;
  gen(env, RBTraceMsg, RBMsgData(t, msg));
}

void prepareEntry(IRGS& env) {
  /*
   * If assertions are on, before we do anything, each region makes a call to a
   * C++ function that checks the state of everything.
   */
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const data = IRSPRelOffsetData { spOffBCFromIRSP(env) };
    gen(env, DbgTraceCall, data, fp(env), sp(env));
  }

  /*
   * We automatically hoist a load of the context to the beginning of every
   * region.  The reason is that it's trivially CSEable, so we might as well
   * make it available everywhere.  If nothing uses it, it'll just be DCE'd.
   */
  ldCtx(env);
}

void endRegion(IRGS& env) {
  auto const curSk  = curSrcKey(env);
  if (!instrAllowsFallThru(curSk.op())) return; // nothing to do here

  auto const nextSk = curSk.advanced(curUnit(env));
  endRegion(env, nextSk);
}

void endRegion(IRGS& env, SrcKey nextSk) {
  FTRACE(1, "------------------- endRegion ---------------------------\n");
  if (!fp(env)) {
    // The function already returned.  There's no reason to generate code to
    // try to go to the next part of it.
    return;
  }
  auto const data = ReqBindJmpData {
    nextSk,
    spOffBCFromFP(env),
    spOffBCFromIRSP(env),
    TransFlags{}
  };
  gen(env, ReqBindJmp, data, sp(env), fp(env));
}

void sealUnit(IRGS& env) {
  mandatoryDCE(env.unit);
}

///////////////////////////////////////////////////////////////////////////////

Type publicTopType(const IRGS& env, BCSPRelOffset idx) {
  // It's logically const, because we're using DataTypeGeneric.
  return topType(const_cast<IRGS&>(env), idx, DataTypeGeneric);
}

Type predictedType(const IRGS& env, const Location& loc) {
  auto& fs = env.irb->fs();

  switch (loc.tag()) {
    case LTag::Stack:
      return fs.stack(offsetFromIRSP(env, loc.stackIdx())).predictedType;
    case LTag::Local:
      return fs.local(loc.localId()).predictedType;
    case LTag::MBase:
      return fs.mbase().predictedType;
  }
  not_reached();
}

Type provenType(const IRGS& env, const Location& loc) {
  auto& fs = env.irb->fs();

  switch (loc.tag()) {
    case LTag::Stack:
      return fs.stack(offsetFromIRSP(env, loc.stackIdx())).type;
    case LTag::Local:
      return fs.local(loc.localId()).type;
    case LTag::MBase:
      return fs.mbase().type;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

void endBlock(IRGS& env, Offset next) {
  if (!fp(env)) {
    // If there's no fp, we've already executed a RetCtrl or similar, so
    // there's no reason to try to jump anywhere now.
    return;
  }
  // Don't emit the jump if it would be unreachable.  This avoids
  // unreachable blocks appearing to be reachable, which would cause
  // translateRegion to process them.
  if (auto const curBlock = env.irb->curBlock()) {
    if (!curBlock->empty() && curBlock->back().isTerminal()) return;
  }
  jmpImpl(env, next);
}

void prepareForNextHHBC(IRGS& env, SrcKey newSk) {
  FTRACE(1, "------------------- prepareForNextHHBC ------------------\n");
  always_assert_flog(
    IMPLIES(isInlining(env), !env.firstBcInst),
    "Inlining while still at the first region instruction."
  );

  always_assert(env.inlineState.bcStateStack.size() == inlineDepth(env));
  always_assert_flog(curFunc(env) == newSk.func(),
                     "Tried to update current SrcKey with a different func");

  env.bcState.setOffset(newSk.offset());
  updateMarker(env);
  env.irb->exceptionStackBoundary();
  env.irb->resetCurIROff();
}

void finishHHBC(IRGS& env) {
  env.firstBcInst = false;
}

//////////////////////////////////////////////////////////////////////

}}}
