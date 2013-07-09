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

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace JIT {

using Transl::DynLocation;

TRACE_SET_MOD(region);

typedef hphp_hash_set<SrcKey, SrcKey::Hasher> InterpSet;

namespace {
struct State {
  const Unit* curUnit;
  const SrcKey startSk;
  HhbcTranslator& ht;
  Unit::MetaHandle metaHand;
};

/*
 * Populate most fields of the NormalizedInstruciton, assuming its sk
 * has already been set. Returns false iff the region should be
 * truncated before inst's SrcKey.
 */
bool prepareInstruction(NormalizedInstruction& inst, State& state) {
  inst.m_unit = state.curUnit;
  inst.breaksTracelet = false;
  inst.changesPC = Transl::opcodeChangesPC(inst.op());
  inst.funcd = nullptr;
  Transl::populateImmediates(inst);
  Transl::preInputApplyMetaData(state.metaHand, &inst);

  Transl::InputInfos inputInfos;
  getInputs(state.startSk, inst, inputInfos, [&](int i) {
    return state.ht.traceBuilder()->getLocalType(i);
  });

  auto newDynLoc = [&](const Transl::InputInfo& ii) {
    auto dl = inst.newDynLoc(ii.loc, state.ht.rttFromLocation(ii.loc));
    FTRACE(2, "rttFromLocation: {} -> {}\n",
           ii.loc.pretty(), dl->rtt.pretty());
    return dl;
  };

  for (auto const& ii : inputInfos) {
    auto* dl = newDynLoc(ii);
    auto const& rtt = dl->rtt;
    inst.inputs.push_back(dl);

    if (ii.dontBreak || ii.dontGuard) continue;
    if (rtt.isVagueValue()) {
      // Trying to consume a value without a precise enough type.
      return false;
    }
    if (inst.ignoreInnerType || ii.dontGuardInner) continue;
    if (rtt.isValue() && rtt.isRef() &&
        (rtt.innerType() == KindOfInvalid || rtt.innerType() == KindOfAny)) {
      // Trying to consume a boxed value without a guess for the inner type.
      return false;
    }
  }

  readMetaData(state.metaHand, inst, state.ht);
  if (!inst.noOp && inputInfos.needsRefCheck) {
    // not supported yet
    return false;
  }

  return true;
}

RegionDescPtr regionTraceletImpl(const RegionContext& ctx,
                                 InterpSet& toInterp) {
  IRTranslator irTrans(ctx.bcOffset, ctx.spOffset, ctx.func);
  auto* curFunc = ctx.func;
  State state{
    curFunc->unit(),
    {ctx.func, ctx.bcOffset},
    irTrans.hhbcTrans(),
  };
  auto& ht = state.ht;
  uint32_t numJmps = 0;

  SrcKey sk(state.startSk);
  auto region = smart::make_unique<RegionDesc>();
  auto* curBlock = region->addBlock(curFunc, sk.offset(), 0);
  bool blockFinished = false;
  uint32_t pendingLiterals = 0;

  auto addInstruction = [&] {
    if (blockFinished) {
      curBlock = region->addBlock(curFunc, sk.offset(), 0);
      blockFinished = false;
    }

    auto op = toOp(*state.curUnit->at(sk.offset()));
    // Don't finish a region with literal values or values that have a class
    // related to the current context class. They produce valuable information
    // for optimizations that's lost across region boundaries.
    if (isLiteral(op) || isThisSelfOrParent(op)) {
      ++pendingLiterals;
      return;
    }

    // This op isn't a literal so add any that are pending before the current
    // instruction.
    for (; pendingLiterals; --pendingLiterals) curBlock->addInstruction();
    curBlock->addInstruction();
  };

  for (auto const& lt : ctx.liveTypes) {
    auto t = lt.type;
    if (t.strictSubtypeOf(Type::Obj)) t = t.unspecialize();

    ht.guardTypeLocation(lt.location, t);
    curBlock->addPredicted(sk, RegionDesc::TypePred{lt.location, t});
  }

  while (true) {
    NormalizedInstruction inst;
    inst.source = sk;
    if (!prepareInstruction(inst, state)) return region;

    // Before doing the translation, check for tracelet-ending control flow.
    if (inst.op() == OpJmp && inst.imm[0].u_BA > 0 &&
        numJmps < Transl::Translator::MaxJmpsTracedThrough) {
      // Include the Jmp in the region and continue to its destination.
      ++numJmps;
      addInstruction();
      sk.setOffset(sk.offset() + inst.imm[0].u_BA);
      blockFinished = true;

      ht.setBcOff(sk.offset(), false);
      continue;
    } else if (Transl::opcodeBreaksBB(inst.op()) ||
               (Transl::dontGuardAnyInputs(inst.op()) &&
                Transl::opcodeChangesPC(inst.op()))) {
      // This instruction ends the tracelet. Include it in the region and
      // return.
      addInstruction();
      return region;
    }

    inst.interp = toInterp.count(sk);
    auto const doPrediction = Transl::outputIsPredicted(state.startSk, inst);

    try {
      irTrans.translateInstr(inst);
    } catch (const FailedIRGen& exn) {
      FTRACE(1, "ir generation for {} failed with {}\n",
             inst.toString(), exn.what());
      always_assert(!toInterp.count(sk));
      toInterp.insert(sk);
      return RegionDescPtr{ nullptr };
    }

    if (isFCallStar(inst.op()) || inst.op() == OpFCallBuiltin) {
      // This is much more conservative than it needs to be.
      ht.emitSmashLocals();
    }

    if (doPrediction) {
      ht.checkTypeStack(0, inst.outPred,
                        sk.advanced(curBlock->unit()).offset());
    }

    addInstruction();
    sk.advance(curBlock->unit());
  }
}
}

/*
 * Region selector that attempts to form the longest possible region using the
 * given context. The region will be broken before the first instruction that
 * attempts to consume an input with an insufficiently precise type.
 *
 */
RegionDescPtr regionTracelet(const RegionContext& ctx) {
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 1;

  while (!(region = regionTraceletImpl(ctx, interp))) {
    ++tries;
  }
  FTRACE(1, "regionTracelet returning after {} tries:\n{}\n",
         tries, show(*region));

  if (region->blocks.size() == 1 && region->blocks.front()->length() == 0) {
    // If we had to break the region at the first instruction, punt.
    return RegionDescPtr{ nullptr };
  } else if (region->blocks.back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->blocks.pop_back();
  }
  return region;
}

} }
