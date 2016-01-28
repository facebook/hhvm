/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/region-selection.h"

#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/ref-deps.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/analysis.h"

#include "hphp/util/trace.h"

#include <folly/MapUtil.h>

#include <algorithm>
#include <vector>

#include "hphp/runtime/vm/jit/irgen.h"

// TODO(#5710324): it seems a little odd that region-tracelet is not part of
// irgen:: but needs access to this.  Probably we don't have the right
// abstraction boundaries.  We'll resolve this somehow later.
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(region);

typedef hphp_hash_set<SrcKey, SrcKey::Hasher> InterpSet;

namespace {

///////////////////////////////////////////////////////////////////////////////

struct Env {
  Env(const RegionContext& ctx,
      InterpSet& interp,
      SrcKey& breakAt,
      int32_t maxBCInstrs,
      bool profiling,
      bool inlining)
    : ctx(ctx)
    , interp(interp)
    , breakAt(breakAt)
    , sk(ctx.func, ctx.bcOffset, ctx.resumed)
    , startSk(sk)
    , region(std::make_shared<RegionDesc>())
    , curBlock(region->addBlock(sk, 0, ctx.spOffset))
    , blockFinished(false)
    // TODO(#5703534): this is using a different TransContext than actual
    // translation will use.
    , irgs(TransContext{kInvalidTransID, sk, ctx.spOffset}, TransFlags{0})
    , arStates(1)
    , numJmps(0)
    , numBCInstrs(maxBCInstrs)
    , profiling(profiling)
    , inlining(inlining)
  {}

  const RegionContext& ctx;
  InterpSet& interp;
  SrcKey& breakAt;
  SrcKey sk;
  const SrcKey startSk;
  NormalizedInstruction inst;
  RegionDescPtr region;
  RegionDesc::Block* curBlock;
  bool blockFinished;
  IRGS irgs;
  jit::vector<ActRecState> arStates;
  RefDeps refDeps;
  uint32_t numJmps;
  int32_t numBCInstrs;
  SrcKey minstrStart;
  // This map memoizes reachability of IR blocks during tracelet
  // formation.  A block won't have it's reachability stored in this
  // map until it's been computed.
  jit::hash_map<unsigned,bool> irReachableBlocks;

  const bool profiling;
  const bool inlining;

 private:
  Env(const Env&) = delete;
  Env& operator=(const Env&) = delete;
};

const Func* curFunc(const Env& env) {
  return irgen::curFunc(env.irgs);
}

const Unit* curUnit(const Env& env) {
  return irgen::curUnit(env.irgs);
}

FPInvOffset curSpOffset(const Env& env) {
  return irgen::logicalStackDepth(env.irgs);
}

bool irBlockReachable(Env& env, Block* block) {
  auto const blockId = block->id();
  auto it = env.irReachableBlocks.find(blockId);
  if (it != env.irReachableBlocks.end()) return it->second;
  bool result = block == env.irgs.irb->unit().entry();
  for (auto& pred : block->preds()) {
    if (irBlockReachable(env, pred.from())) {
      result = true;
      break;
    }
  }
  env.irReachableBlocks[blockId] = result;
  return result;
}

/*
 * Check if the current predicted type for the location in ii is specific
 * enough for what the current opcode wants. If not, return false.
 */
bool consumeInput(Env& env, int i, const InputInfo& ii) {
  if (ii.dontGuard) return true;
  auto const type = irgen::predictedTypeFromLocation(env.irgs, ii.loc);

  if (env.profiling && type <= TBoxedCell &&
      (env.region->blocks().size() > 1 || !env.region->entry()->empty())) {
    // We don't want side exits when profiling, so only allow instructions that
    // consume refs at the beginning of the region.
    return false;
  }

  if (!ii.dontBreak && !type.isKnownDataType()) {
    // Trying to consume a value without a precise enough type.
    FTRACE(1, "selectTracelet: {} tried to consume {}\n",
           env.inst.toString(), env.inst.inputs[i].pretty());
    return false;
  }

  if (!(type <= TBoxedCell) || env.inst.ignoreInnerType || ii.dontGuardInner) {
    return true;
  }

  if (!type.inner().isKnownDataType()) {
    // Trying to consume a boxed value without a guess for the inner type.
    FTRACE(1, "selectTracelet: {} tried to consume ref {}\n",
           env.inst.toString(), env.inst.inputs[i].pretty());
    return false;
  }

  return true;
}

/*
 * Add the current instruction to the region.
 */
void addInstruction(Env& env) {
  if (env.blockFinished) {
    FTRACE(2, "selectTracelet adding new block at {} after:\n{}\n",
           showShort(env.sk), show(*env.curBlock));
    always_assert(env.sk.func() == curFunc(env));
    auto newCurBlock = env.region->addBlock(env.sk, 0, curSpOffset(env));
    env.region->addArc(env.curBlock->id(), newCurBlock->id());
    env.curBlock = newCurBlock;
    env.blockFinished = false;
  }

  FTRACE(2, "selectTracelet adding instruction {}\n", env.inst.toString());
  env.curBlock->addInstruction();
  env.numBCInstrs--;
}

/*
 * Populate most fields of the NormalizedInstruction, assuming its sk
 * has already been set. Returns false iff the region should be
 * truncated before inst's SrcKey.
 */
bool prepareInstruction(Env& env) {
  env.inst.~NormalizedInstruction();
  new (&env.inst) NormalizedInstruction(env.sk, curUnit(env));
  if (RuntimeOption::EvalFailJitPrologs && env.inst.op() == Op::FCallAwait) {
    return false;
  }
  auto const breaksBB =
    (env.profiling && instrBreaksProfileBB(&env.inst)) ||
    opcodeBreaksBB(env.inst.op());
  env.inst.endsRegion = breaksBB ||
    (dontGuardAnyInputs(env.inst.op()) && opcodeChangesPC(env.inst.op()));
  env.inst.funcd = env.arStates.back().knownFunc();
  irgen::prepareForNextHHBC(env.irgs, &env.inst, env.sk, false);

  auto const inputInfos = getInputs(env.inst);

  for (auto const& ii : inputInfos) env.inst.inputs.push_back(ii.loc);

  // This reads valueClass from the inputs so it used to need to
  // happen after readMetaData.  But now readMetaData is gone ...
  annotate(&env.inst);

  // Check all the inputs for unknown values.
  assertx(inputInfos.size() == env.inst.inputs.size());
  for (unsigned i = 0; i < inputInfos.size(); ++i) {
    if (!consumeInput(env, i, inputInfos[i])) {
      FTRACE(2, "Stopping tracelet consuming {} input {}\n",
        opcodeToName(env.inst.op()), i);
      return false;
    }
  }

  if (inputInfos.needsRefCheck) {
    // Reffiness guards are always at the beginning of the trace for now, so
    // calculate the delta from the original sp to the ar. The FPI delta from
    // instrFpToArDelta includes locals and iterators, so when we're in a
    // resumed context we have to adjust for the fact that they're in a
    // different place.
    auto argNum = env.inst.imm[0].u_IVA;
    auto entryArDelta = env.ctx.spOffset.offset -
      instrFpToArDelta(curFunc(env), env.inst.pc());
    if (env.sk.resumed()) entryArDelta += curFunc(env)->numSlotsInFrame();

    try {
      env.inst.preppedByRef =
        env.arStates.back().checkByRef(argNum, entryArDelta, &env.refDeps);
    } catch (const UnknownInputExc& exn) {
      // We don't have a guess for the current ActRec.
      FTRACE(1, "selectTracelet: don't have reffiness guess for {}\n",
             env.inst.toString());
      return false;
    }
    addInstruction(env);
    env.curBlock->setParamByRef(env.inst.source, env.inst.preppedByRef);
  } else {
    addInstruction(env);
  }

  if (isFPush(env.inst.op())) env.arStates.back().pushFunc(env.inst);

  return true;
}

bool traceThroughJmp(Env& env) {
  // We only trace through unconditional jumps and conditional jumps with const
  // inputs while inlining.
  if (!isUnconditionalJmp(env.inst.op()) &&
      !(env.inlining && isConditionalJmp(env.inst.op()) &&
        irgen::publicTopType(env.irgs, BCSPOffset{0}).hasConstVal())) {
    return false;
  }

  // We want to keep profiling translations to basic blocks, inlining shouldn't
  // happen in profiling translations
  if (env.profiling) {
    assert(!env.inlining);
    return false;
  }

  // Don't trace through too many jumps, unless we're inlining. We want to make
  // sure we don't break a tracelet in the middle of an inlined call; if the
  // inlined callee becomes too big that's caught in shouldIRInline.
  if (env.numJmps == Translator::MaxJmpsTracedThrough && !env.inlining) {
    return false;
  }

  auto offset = env.inst.imm[0].u_BA;
  // Only trace through backwards jumps if it's a JmpNS and we're
  // inlining. This is to get DV funclets.
  if (offset <= 0 && (env.inst.op() != OpJmpNS || !env.inlining)) {
    return false;
  }

  // Ok we're good. For unconditional jumps, just set env.sk to the dest. For
  // known conditional jumps we have to consume the const value on the top of
  // the stack and figure out which branch to go to.
  if (isUnconditionalJmp(env.inst.op())) {
    env.sk.setOffset(env.sk.offset() + offset);
  } else {
    auto value = irgen::popC(env.irgs);
    auto taken =
      value->variantVal().toBoolean() == (env.inst.op() == OpJmpNZ);
    FTRACE(2, "Tracing through {}taken Jmp(N)Z on constant {}\n",
           taken ? "" : "not ", *value->inst());

    env.sk.setOffset(taken ? env.sk.offset() + offset
                           : env.sk.advanced().offset());
  }

  env.numJmps++;
  env.blockFinished = true;
  return true;
}

bool isLiteral(Op op) {
  switch (op) {
    case OpNull:
    case OpNullUninit:
    case OpTrue:
    case OpFalse:
    case OpInt:
    case OpDouble:
    case OpString:
    case OpArray:
      return true;

    default:
      return false;
  }
}

bool isThisSelfOrParent(Op op) {
  switch (op) {
    case OpThis:
    case OpSelf:
    case OpParent:
      return true;

    default:
      return false;
  }
}

/*
 * For every instruction in trace representing a tracelet guard, call func with
 * its location and type, and whether or not it's an inner hint.
 */
template<typename F>
void visitGuards(IRUnit& unit, F func) {
  using L = RegionDesc::Location;
  auto blocks = rpoSortCfg(unit);
  for (auto* block : blocks) {
    for (auto& inst : *block) {
      switch (inst.op()) {
        case EndGuards:
          return;
        case HintLocInner:
        case CheckLoc:
          func(&inst,
               L::Local{inst.extra<LocalId>()->locId},
               inst.typeParam(),
               inst.is(HintLocInner));
          break;
        case HintStkInner:
        case CheckStk:
        {
          /*
           * BCSPOffset is optional but should --always-- be set for CheckStk
           * instructions that appear within the guards for a translation.
           */
          auto bcSpOffset = inst.extra<RelOffsetData>()->bcSpOffset;
          assertx(inst.extra<RelOffsetData>()->hasBcSpOffset);

          auto offsetFromFp = inst.marker().spOff() - bcSpOffset;
          func(&inst,
               L::Stack{offsetFromFp},
               inst.typeParam(),
               inst.is(HintStkInner));
          break;
        }
        default: break;
      }
    }
  }
}

/*
 * Records any type/reffiness predictions we depend on in the region.
 */
void recordDependencies(Env& env) {
  // Record the incrementally constructed reffiness predictions.
  assertx(!env.region->empty());
  auto& frontBlock = *env.region->blocks().front();
  for (auto const& dep : env.refDeps.m_arMap) {
    frontBlock.addReffinessPred({dep.second.m_mask, dep.second.m_vals,
                                 dep.first});
  }

  // Relax guards and record the ones that survived.
  auto& firstBlock = *env.region->blocks().front();
  auto& unit = env.irgs.unit;
  auto guardMap = std::map<RegionDesc::Location,Type>{};
  ITRACE(2, "Visiting guards\n");
  auto hintMap = std::map<RegionDesc::Location,Type>{};
  auto catMap = std::map<RegionDesc::Location,DataTypeCategory>{};
  const auto& guards = env.irgs.irb->guards()->guards;
  visitGuards(unit, [&](IRInstruction* guard, const RegionDesc::Location& loc,
                        Type type, bool hint) {
    Trace::Indent indent;
    ITRACE(3, "{}: {}\n", show(loc), type);
    if (type <= TCls) return;
    auto& whichMap = hint ? hintMap : guardMap;
    auto inret = whichMap.insert(std::make_pair(loc, type));
    if (inret.second) {
      if (!hint) {
        catMap[loc] = folly::get_default(guards, guard).category;
      }
      return;
    }
    auto& oldTy = inret.first->second;
    oldTy &= type;
    if (!hint) {
      auto& oldCat = catMap[loc];
      auto newCat = folly::get_default(guards, guard).category;
      oldCat = std::max(oldCat, newCat);
    }
  });

  for (auto& kv : guardMap) {
    auto const hint_it = hintMap.find(kv.first);
    // If we have a hinted type that's better than the guarded type, we want to
    // keep it around.  This can really only when a guard is relaxed away to
    // Gen because we knew something was a BoxedCell statically, but we may
    // need to keep information about what inner type we were predicting.
    if (hint_it != end(hintMap) && hint_it->second < kv.second) {
      auto const pred = RegionDesc::TypedLocation {
        hint_it->first,
        hint_it->second
      };
      FTRACE(1, "selectTracelet adding prediction {}\n", show(pred));
      firstBlock.addPredicted(pred);
    }
    if (kv.second == TGen) {
      // Guard was relaxed to Gen---don't record it.  But if there's a hint, we
      // may have needed that (recorded already above).
      continue;
    }
    auto const preCond = RegionDesc::GuardedLocation { kv.first, kv.second,
                                                       catMap[kv.first] };
    ITRACE(1, "selectTracelet adding guard {}\n", show(preCond));
    firstBlock.addPreCondition(preCond);
  }
}

void truncateLiterals(Env& env) {
  if (!env.region || env.region->empty() ||
      env.region->blocks().back()->empty()) return;

  // Don't finish a region with literal values or values that have a class
  // related to the current context class. They produce valuable information
  // for optimizations that's lost across region boundaries.
  auto& lastBlock = *env.region->blocks().back();
  auto sk = lastBlock.start();
  auto endSk = sk;
  auto unit = lastBlock.unit();
  for (int i = 0, len = lastBlock.length(); i < len; ++i, sk.advance(unit)) {
    auto const op = sk.op();
    if (!isLiteral(op) && !isThisSelfOrParent(op) && !isTypeAssert(op)) {
      if (i == len - 1) return;
      endSk = sk;
    }
  }
  // Don't truncate if we've decided we want to truncate the entire block.
  // That'll mean we'll chop off the trailing N-1 opcodes, then in the next
  // region we'll select N-1 opcodes and chop off N-2 opcodes, and so forth...
  if (endSk != lastBlock.start()) {
    FTRACE(1, "selectTracelet truncating block after offset {}:\n{}\n",
           endSk.offset(), show(lastBlock));
    lastBlock.truncateAfter(endSk);
  }
}

RegionDescPtr form_region(Env& env) {
  SCOPE_ASSERT_DETAIL("Tracelet Selector") {
    return folly::sformat("Region:\n{}\n\nUnit:\n{}\n",
                          *env.region, show(env.irgs.irb->unit()));
  };

  for (auto const& lt : env.ctx.liveTypes) {
    auto t = lt.type;
    if (t <= TCls) {
      irgen::assertTypeLocation(env.irgs, lt.location, t);
      env.curBlock->addPreCondition({lt.location, t, DataTypeGeneric});
    } else {
      irgen::checkTypeLocation(env.irgs, lt.location, t, env.ctx.bcOffset,
                               true /* outerOnly */);
    }
  }

  irgen::gen(env.irgs, EndGuards);

  for (bool firstInst = true; true; firstInst = false) {
    assertx(env.numBCInstrs >= 0);
    if (env.numBCInstrs == 0) {
      FTRACE(1, "selectTracelet: breaking region due to size limit\n");
      break;
    }

    if (!firstInst && env.sk == env.breakAt) {
      FTRACE(1, "selectTracelet: breaking region at breakAt: {}\n",
             show(env.sk));
      break;
    }

    if (!prepareInstruction(env)) break;

    // Until the rest of the pipeline can handle it without regressing, try to
    // not break tracelets in the middle of new minstr sequences. Note that
    // this is just an optimization; we can generate correct code regardless of
    // how the minstrs are chopped up.
    if (!firstInst && isMemberBaseOp(env.inst.op())) {
      env.minstrStart = env.sk;
    } else if (isMemberFinalOp(env.inst.op())) {
      env.minstrStart = SrcKey{};
    }

    env.curBlock->setKnownFunc(env.sk, env.inst.funcd);

    if (traceThroughJmp(env)) continue;

    env.inst.interp = env.interp.count(env.sk);

    try {
      translateInstr(env.irgs, env.inst, true /* checkOuterTypeOnly */,
                     firstInst);
    } catch (const FailedIRGen& exn) {
      FTRACE(1, "ir generation for {} failed with {}\n",
             env.inst.toString(), exn.what());
      always_assert_flog(
        !env.interp.count(env.sk),
        "Double PUNT trying to translate {}\n", env.inst
      );
      env.interp.insert(env.sk);
      env.region.reset();
      break;
    }

    irgen::finishHHBC(env.irgs);

    if (!instrAllowsFallThru(env.inst.op())) {
      FTRACE(1, "selectTracelet: tracelet broken after instruction with no "
             "fall-through {}\n", env.inst);
      break;
    }

    // We successfully translated the instruction, so update env.sk.
    env.sk.advance(env.curBlock->unit());

    auto const endsRegion = env.inst.endsRegion;

    if (endsRegion) {
      FTRACE(1, "selectTracelet: tracelet broken after {}\n", env.inst);
      break;
    } else {
      assertx(env.sk.func() == curFunc(env));
    }

    auto const curIRBlock = env.irgs.irb->curBlock();
    if (!irBlockReachable(env, curIRBlock)) {
      FTRACE(1,
        "selectTracelet: tracelet broken due to unreachable code (block {})\n",
        curIRBlock->id());
      break;
    }

    if (curIRBlock->isExitNoThrow()) {
      FTRACE(1, "selectTracelet: tracelet broken due to exiting IR instruction:"
             "{}\n", curIRBlock->back());
      break;
    }

    if (isFCallStar(env.inst.op())) env.arStates.back().pop();
  }

  // Return an empty region to restart region formation at env.minstrStart.
  if (env.minstrStart.valid()) {
    env.breakAt = env.minstrStart;
    env.region.reset();
  }

  if (env.region && !env.region->empty()) {
    // Make sure we end the region before trying to print the IRUnit.
    irgen::endRegion(env.irgs, env.sk);

    printUnit(
      kTraceletLevel, env.irgs.irb->unit(),
      env.inlining ? " after inlining tracelet formation "
                   : " after tracelet formation ",
      nullptr,
      env.irgs.irb->guards()
    );

    recordDependencies(env);

    // Make sure that the IR unit contains a main exit corresponding
    // to the last bytecode instruction in the region.  Note that this
    // check has to happen before the call to truncateLiterals()
    // because that updates the region but not the IR unit.
    if (!env.region->blocks().back()->empty()) {
      auto lastSk = env.region->lastSrcKey();
      always_assert_flog(findMainExitBlock(env.irgs.irb->unit(), lastSk),
                         "No main exits found!");
    }

    truncateLiterals(env);
  }

  return std::move(env.region);
}

///////////////////////////////////////////////////////////////////////////////
}

RegionDescPtr selectTracelet(const RegionContext& ctx, int32_t maxBCInstrs,
                             bool profiling, bool inlining /* = false */) {
  Timer _t(Timer::selectTracelet);
  InterpSet interp;
  SrcKey breakAt;
  RegionDescPtr region;
  uint32_t tries = 0;

  FTRACE(1, "selectTracelet: starting with maxBCInstrs = {}\n", maxBCInstrs);

  do {
    Env env{ctx, interp, breakAt, maxBCInstrs, profiling, inlining};
    region = form_region(env);
    ++tries;
  } while (!region);

  if (region->empty() || region->blocks().front()->length() == 0) {
    FTRACE(1, "selectTracelet giving up after {} tries\n", tries);
    return RegionDescPtr { nullptr };
  }

  if (region->blocks().back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->deleteBlock(region->blocks().back()->id());
  }

  if (RuntimeOption::EvalRegionRelaxGuards) {
    FTRACE(1, "selectTracelet: before optimizeGuards:\n{}\n",
           show(*region));
    optimizeGuards(*region, profiling);
  }

  FTRACE(1, "selectTracelet returning, {}, {} tries:\n{}\n",
         inlining ? "inlining" : "not inlining", tries, show(*region));
  return region;
}

} }
