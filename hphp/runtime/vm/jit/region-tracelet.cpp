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

#include "hphp/runtime/vm/jit/region-selection.h"

#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/irgen-bespoke.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
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

using InterpSet = hphp_hash_set<SrcKey, SrcKey::Hasher>;

namespace {

///////////////////////////////////////////////////////////////////////////////

constexpr int MaxJmpsTracedThrough = 5;

struct Env {
  Env(const RegionContext& ctx,
      TransKind kind,
      InterpSet& interp,
      int32_t maxBCInstrs,
      bool inlining)
    : ctx(ctx)
    , interp(interp)
    , sk{ctx.sk}
    , startSk(sk)
    , region(std::make_shared<RegionDesc>())
    , curBlock(region->addBlock(sk, 0, ctx.spOffset))
    , prevBlocks()
    // TODO(#5703534): this is using a different TransContext than actual
    // translation will use.
    , unit(TransContext{
              TransIDSet{}
            , 0 /* optIndex */
            , kind
            , sk
            , nullptr
            , sk.packageInfo()}
          , std::make_unique<AnnotationData>())
    , irgs(unit, nullptr, 0, nullptr)
    , numJmps(0)
    , numBCInstrs(maxBCInstrs)
    , profiling(kind == TransKind::Profile)
    , inlining(inlining)
  {
    irgen::defineFrameAndStack(irgs, ctx.spOffset);
    irgs.formingRegion = true;
    irgs.irb->enableConstrainGuards();
  }

  const RegionContext& ctx;
  InterpSet& interp;
  SrcKey sk;
  const SrcKey startSk;
  NormalizedInstruction inst;
  RegionDescPtr region;
  RegionDesc::Block* curBlock;
  jit::hash_map<Offset, jit::vector<RegionDesc::Block*>> prevBlocks;
  IRUnit unit;
  irgen::IRGS irgs;
  uint32_t numJmps;
  int32_t numBCInstrs;
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

SBInvOffset curSpOffset(const Env& env) {
  return env.irgs.irb->fs().bcSPOff();
}

/*
 * Check if the current predicted type for the location in ii is specific
 * enough for what the current opcode wants. If not, return false.
 */
bool consumeInput(Env& env, const InputInfo& input) {
  if (input.dontGuard) return true;
  auto const type = irgen::provenType(env.irgs, input.loc);

  if (!input.dontBreak && !type.isKnownDataType()) {
    // Trying to consume a value without a precise enough type.
    FTRACE(1, "selectTracelet: {} tried to consume {}, type {}\n",
           env.inst.toString(), show(input.loc), type.toString());
    return false;
  }

  return true;
}

/*
 * Add the current instruction to the region.
 */
void addInstruction(Env& env) {
  if (!env.sk.funcEntry()) {
    auto prevBlocksIt = env.prevBlocks.find(env.sk.offset());
    if (prevBlocksIt != env.prevBlocks.end()) {
      FTRACE(2, "selectTracelet adding new block at {} after:\n{}\n",
             showShort(env.sk), show(*env.curBlock));
      always_assert(env.sk.func() == curFunc(env));
      env.curBlock = env.region->addBlock(env.sk, 0, curSpOffset(env));
      for (auto block : prevBlocksIt->second) {
        env.region->addArc(block->id(), env.curBlock->id());
      }
    }
  }

  FTRACE(2, "selectTracelet adding instruction {}\n", env.inst.toString());
  env.curBlock->addInstruction();
  env.numBCInstrs--;
}

bool instructionEndsRegion(const Env& env) {
  auto const& inst = env.inst;
  if (opcodeBreaksBB(inst.op(), env.inlining)) return true;
  if (env.profiling && instrBreaksProfileBB(inst)) return true;
  if (dontGuardAnyInputs(inst) && opcodeChangesPC(inst.op())) return true;
  return false;
}

Type getLiveType(const jit::vector<RegionContext::LiveType>& liveTypes,
                 const Location& loc) {
  for (auto const& lt : liveTypes) {
    if (lt.location == loc) return lt.type;
  }
  return TCell;
}

/*
 * Populate most fields of the NormalizedInstruction, assuming its sk
 * has already been set. Returns false iff the region should be
 * truncated before inst's SrcKey.
 */
bool prepareInstruction(Env& env) {
  env.inst.~NormalizedInstruction();
  new (&env.inst) NormalizedInstruction(env.sk, curUnit(env));
  irgen::prepareForNextHHBC(env.irgs, env.sk);

  if (env.sk.funcEntry()) {
    addInstruction(env);
    return true;
  }

  auto inputInfos = getInputs(env.inst, env.irgs.irb->fs().bcSPOff());
  for (auto const loc : irgen::guardsForBespoke(env.irgs, env.sk)) {
    FTRACE(1, "prepareInstruction: adding bespoke guard: {}\n", show(loc));
    inputInfos.emplace_back(loc);
  }

  auto const op = env.inst.op();
  auto& fs = env.irgs.irb->fs();

  Block* guardFailBlock = nullptr;
  auto addGuardIfUntracked = [&](Location loc) {
    FTRACE(1, "prepareInstruction: input: {}\n", show(loc));
    if (!fs.tracked(loc) &&
        (loc.tag() != LTag::Local || !fs.localsCleared())) {
      auto const type = getLiveType(env.ctx.liveTypes, loc);
      assert_flog(type <= TCell, "loc = {}: type = {}", show(loc), type);
      if (guardFailBlock == nullptr) guardFailBlock = irgen::makeExit(env.irgs);
      irgen::checkType(env.irgs, loc, type, guardFailBlock);
    }
  };

  // Guard any input that hasn't been guarded yet.
  for (auto const& input : inputInfos) {
    addGuardIfUntracked(input.loc);
  }

  // Guard any output local that hasn't been guarded yet -- they'll be read to
  // be decref'd.
  auto const outputLocals = getLocalOutputs(env.inst);
  for (auto locId : outputLocals) {
    addGuardIfUntracked(Location::Local{locId});
  }

  // AssertRAT* instructions are special: they refine the type of a location
  // without taking it as an input.  The location will start to be tracked by
  // FrameState after these instructions, so we need to first guard them since
  // the guards may provide additional type information.
  if (op == OpAssertRATL) {
    auto loc = Location::Local{safe_cast<uint32_t>(env.inst.imm[0].u_ILA)};
    addGuardIfUntracked(loc);
  }
  if (op == OpAssertRATStk) {
    auto const bcSPOff = env.irgs.irb->fs().bcSPOff();
    auto const sbInvOff =
      BCSPRelOffset{safe_cast<int32_t>(env.inst.imm[0].u_IVA)}.
        to<SBInvOffset>(bcSPOff);
    addGuardIfUntracked(Location::Stack{sbInvOff});
  }

  // Check all the inputs for unknown values.
  for (auto const& input : inputInfos) {
    if (!consumeInput(env, input)) {
      FTRACE(2, "Stopping tracelet consuming {} input {}\n",
             opcodeToName(env.inst.op()), show(input.loc));
      return false;
    }
  }

  addInstruction(env);

  if (isFCall(op)) {
    auto const asyncEagerOffset = env.inst.imm[0].u_FCA.asyncEagerOffset;
    if (asyncEagerOffset != kInvalidOffset) {
      // Note that the arc between the block containing asyncEagerOffset and
      // the previous block is not added to the region on purpose, as it comes
      // from the slow path (await of a finished Awaitable after failed async
      // eager return, which usually produces unfinished Awaitable) with
      // possibly unknown type pessimizing next execution.
      auto const sk = env.sk;
      env.prevBlocks[sk.advanced().offset()].push_back(env.curBlock);
      env.prevBlocks[sk.offset() + asyncEagerOffset].push_back(env.curBlock);
    }
  }

  return true;
}

bool traceThroughJmp(Env& env) {
  // Func entry is not a jmp.
  if (env.sk.funcEntry()) return false;

  // We only trace through unconditional jumps and conditional jumps with const
  // inputs while inlining.
  if (!(isUnconditionalJmp(env.inst.op()) || isInterceptableJmp(env.inst.op())) &&
      !(env.inlining && isConditionalJmp(env.inst.op()) &&
        irgen::publicTopType(env.irgs, BCSPRelOffset{0}).hasConstVal())) {
    return false;
  }

  // We want to keep profiling translations to basic blocks, inlining shouldn't
  // happen in profiling translations
  if (env.profiling) {
    assertx(!env.inlining);
    return false;
  }

  // Don't trace through too many jumps, unless we're inlining. We want to make
  // sure we don't break a tracelet in the middle of an inlined call; if the
  // inlined callee becomes too big that's caught in shouldIRInline.
  if (env.numJmps == MaxJmpsTracedThrough && !env.inlining) {
    return false;
  }

  auto offset = env.inst.imm[0].u_BA;
  // Only trace through backwards jumps if it's an Enter and we're
  // inlining. This is to get DV funclets.
  if (offset <= 0 && (env.inst.op() != OpEnter || !env.inlining)) {
    return false;
  }

  // Ok we're good. For unconditional jumps, just set env.sk to the dest. For
  // known conditional jumps we have to consume the const value on the top of
  // the stack and figure out which branch to go to.
  if (isUnconditionalJmp(env.inst.op()) || isInterceptableJmp(env.inst.op())) {
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
  env.prevBlocks[env.sk.offset()].push_back(env.curBlock);
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
    case OpDict:
    case OpVec:
    case OpKeyset:
      return true;

    default:
      return false;
  }
}

bool isThisSelfOrParent(Op op) {
  switch (op) {
    case OpThis:
    case OpSelfCls:
    case OpParentCls:
      return true;

    default:
      return false;
  }
}

/*
 * For every instruction in trace representing a tracelet guard, call func with
 * its location and type.
 */
template<typename F>
void visitGuards(IRUnit& unit, F func) {
  auto blocks = rpoSortCfg(unit);

  for (auto const block : blocks) {
    for (auto const& inst : *block) {
      switch (inst.op()) {
        case EndGuards:
          return;
        case CheckLoc:
          func(&inst,
               Location::Local{inst.extra<LocalId>()->locId},
               inst.typeParam());
          break;
        case CheckStk: {
          auto const irSPRel = inst.extra<IRSPRelOffsetData>()->offset;

          auto const defSP = inst.src(0)->inst();
          assertx(defSP->is(DefFrameRelSP, DefRegSP));
          auto const irSPOff = defSP->extra<DefStackData>()->irSPOff;

          func(&inst,
               Location::Stack{irSPRel.to<SBInvOffset>(irSPOff)},
               inst.typeParam());
          break;
        }
        case CheckMBase:
          func(&inst, Location::MBase{}, inst.typeParam());
          break;
        default: break;
      }
    }
  }
}

/*
 * Records any type predictions we depend on in the region.
 */
void recordDependencies(Env& env) {
  // Relax guards and record the ones that survived.
  auto& firstBlock = *env.region->blocks().front();
  auto& unit = env.irgs.unit;
  auto guardMap = std::map<Location,Type>{};
  ITRACE(2, "Visiting guards\n");
  auto catMap = std::map<Location,DataTypeCategory>{};
  const auto& guards = env.irgs.irb->guards()->guards;
  visitGuards(unit, [&] (const IRInstruction* guard,
                         const Location& loc,
                         Type type) {
    Trace::Indent indent;
    assertx(type <= TCell);
    auto const gc = folly::get_default(guards, guard);
    auto gcToRelax = gc;
    if (DataTypeGeneric < gc.category && gc.category < DataTypeSpecific) {
      gcToRelax = DataTypeSpecific;
    }
    auto const relaxedType = relaxToConstraint(type, gcToRelax);
    ITRACE(3, "{}: {} -> {} {}\n",
           show(loc), type, relaxedType, gcToRelax.toString());

    auto inret = guardMap.insert(std::make_pair(loc, relaxedType));
    if (inret.second) {
      catMap[loc] = gc.category;
      return;
    }
    inret.first->second &= relaxedType;
    auto& oldCat = catMap[loc];
    oldCat = std::max(oldCat, gc.category);
  });

  for (auto& kv : guardMap) {
    if (kv.second == TCell) {
      // Guard was relaxed to Cell---don't record it.
      continue;
    }
    auto const preCond = RegionDesc::GuardedLocation {
      kv.first, kv.second,
      catMap[kv.first]
    };
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
  auto func = lastBlock.func();
  for (int i = 0, len = lastBlock.length(); i < len; ++i, sk.advance(func)) {
    if (!sk.funcEntry()) {
      auto const op = sk.op();
      if (isLiteral(op) || isThisSelfOrParent(op) || isTypeAssert(op)) continue;
    }

    if (i == len - 1) return;
    endSk = sk;
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

  auto const eager =
    env.ctx.liveTypes.size() <= RuntimeOption::EvalJitTraceletEagerGuardsLimit;

  Block* guardFailBlock = nullptr;
  for (auto const& lt : env.ctx.liveTypes) {
    // Local and stack slots are lazily guarded when there are too many live
    // locations; but MBase is always eagerly guarded.
    if (eager || lt.location.tag() == LTag::MBase) {
      auto t = lt.type;
      assertx(t <= TCell);
      if (guardFailBlock == nullptr) guardFailBlock = irgen::makeExit(env.irgs);
      irgen::checkType(env.irgs, lt.location, t, guardFailBlock);
    }
  }

  // EndGuards is used to mark the end of the guards, allowing visitGuards to
  // avoid scanning through the entire unit.  We only insert EndGuards if all
  // guards were eagerly inserted because, with lazy guarding, the guards will
  // be emitted later.
  if (eager) irgen::gen(env.irgs, EndGuards);

  for (bool firstInst = true; true; firstInst = false) {
    assertx(env.numBCInstrs >= 0);
    if (env.numBCInstrs == 0) {
      FTRACE(1, "selectTracelet: breaking region due to size limit\n");
      break;
    }

    // Break translation if there's already a translation starting at the
    // current SrcKey.
    if (!firstInst) {
      auto const sr = tc::findSrcRec(env.sk);
      if (sr != nullptr && sr->getTopTranslation() != nullptr) {
        FTRACE(1, "selectTracelet: breaking region at TC entry: {}\n",
               show(env.sk));
        break;
      }
    }

    if (!prepareInstruction(env)) break;
    if (traceThroughJmp(env)) continue;
    env.inst.interp = env.interp.count(env.sk);

    try {
      translateInstr(env.irgs, env.inst);
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

    if (!env.sk.funcEntry() && !instrAllowsFallThru(env.inst.op())) {
      FTRACE(1, "selectTracelet: tracelet broken after instruction with no "
             "fall-through {}\n", env.inst);
      break;
    }

    // We successfully translated the instruction, so update env.sk.
    assertx(env.sk.func() == curFunc(env));
    env.sk.advance(env.curBlock->func());

    if (env.inst.source.funcEntry()) {
      auto const func = curFunc(env);
      if (env.inst.source.trivialDVFuncEntry() ||
          func->hasNonTrivialDVFuncEntry()) {
        FTRACE(1, "selectTracelet: tracelet broken after func entry\n");
        break;
      }
      continue;
    }

    if (instructionEndsRegion(env)) {
      FTRACE(1, "selectTracelet: tracelet broken after {}\n", env.inst);
      break;
    } else if (isIteratorOp(env.sk.op())) {
      FTRACE(1, "selectTracelet: tracelet broken before iterator op\n");
      break;
    }

    if (env.irgs.irb->inUnreachableState()) {
      FTRACE(1, "selectTracelet: tracelet ending at unreachable state\n");
      break;
    }

    const auto numGuards = env.irgs.irb->numGuards();
    if (numGuards >= RuntimeOption::EvalJitTraceletGuardsLimit) {
      FTRACE(1, "selectTracelet: tracelet broken due to too many guards ({})\n",
             numGuards);
      break;
    }
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

    auto const truncate = [&] () -> bool {
      // Make sure that the IR unit contains a main exit corresponding
      // to the last bytecode instruction in the region.  Note that this
      // check has to happen before the call to truncateLiterals()
      // because that updates the region but not the IR unit.
      if (env.region->blocks().back()->empty()) return true;
      auto lastSk = env.region->lastSrcKey();
      auto const mainExits = findMainExitBlocks(env.irgs.irb->unit(), lastSk);
      /*
       * If the last instruction is an Unreachable, its probably due to
       * unreachable code. We don't want to truncate the tracelet in that case,
       * because we could lose the assertion (eg if the Unreachable is due to a
       * failed AssertRAT).
       */
      for (auto& me : mainExits) {
        if (me->back().is(Unreachable)) return false;
      }
      return true;
    }();

    if (truncate) {
      truncateLiterals(env);
    }
  }

  return std::move(env.region);
}

///////////////////////////////////////////////////////////////////////////////
}

RegionDescPtr selectTracelet(const RegionContext& ctx, TransKind kind,
                             int32_t maxBCInstrs, bool inlining /* = false */) {
  Timer _t(Timer::selectTracelet, nullptr);
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 0;

  FTRACE(1, "selectTracelet: starting with maxBCInstrs = {}\n", maxBCInstrs);

  tracing::Block _{
    "select-tracelet",
    [&] {
      return tracing::Props{}
        .add("sk", show(ctx.sk))
        .add("trans_kind", show(kind))
        .add("inlining", inlining)
        .add("max_bc_instrs", maxBCInstrs);
    }
  };

  if (ctx.liveTypes.size() > RuntimeOption::EvalJitTraceletLiveLocsLimit) {
    return nullptr;
  }

  do {
    Env env{ctx, kind, interp, maxBCInstrs, inlining};
    region = form_region(env);
    ++tries;
  } while (!region);

  if (region->empty() || region->blocks().front()->length() == 0) {
    tracing::addPoint("select-tracelet-giving-up");
    FTRACE(1, "selectTracelet giving up after {} tries\n", tries);
    return nullptr;
  }

  if (region->blocks().back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->deleteBlock(region->blocks().back()->id());
  }

  tracing::annotateBlock(
    [&] {
      return tracing::Props{}
        .add("region_size", region->instrSize())
        .add("tries", tries);
    }
  );

  FTRACE(1, "selectTracelet returning, {}, {} tries:\n{}\n",
         inlining ? "inlining" : "not inlining", tries, show(*region));
  return region;
}

} }
