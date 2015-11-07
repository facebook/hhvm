/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
// irgen:: but needs access to this.  Probably we don't have the right abstraction
// boundaries.  We'll resolve this somehow later.
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(region);

typedef hphp_hash_set<SrcKey, SrcKey::Hasher> InterpSet;

namespace {

///////////////////////////////////////////////////////////////////////////////

struct RegionFormer {
  RegionFormer(const RegionContext& ctx,
               InterpSet& interp,
               int32_t maxBCInstrs,
               bool profiling,
               bool inlining);

  RegionDescPtr go();

private:
  const RegionContext& m_ctx;
  InterpSet& m_interp;
  SrcKey m_sk;
  const SrcKey m_startSk;
  NormalizedInstruction m_inst;
  RegionDescPtr m_region;
  RegionDesc::Block* m_curBlock;
  bool m_blockFinished;
  IRGS m_irgs;
  jit::vector<ActRecState> m_arStates;
  RefDeps m_refDeps;
  uint32_t m_numJmps;
  int32_t m_numBCInstrs;
  // This map memoizes reachability of IR blocks during tracelet
  // formation.  A block won't have it's reachability stored in this
  // map until it's been computed.
  jit::hash_map<unsigned,bool> m_irReachableBlocks;

  const bool m_profiling;
  const bool m_inlining;

  const Func* curFunc() const;
  const Unit* curUnit() const;
  FPInvOffset curSpOffset() const;
  bool resumed() const;

  bool prepareInstruction();
  void addInstruction();
  bool consumeInput(int i, const InputInfo& ii);
  bool traceThroughJmp();
  void recordDependencies();
  void truncateLiterals();
  bool irBlockReachable(Block* block);
};

RegionFormer::RegionFormer(const RegionContext& ctx,
                           InterpSet& interp,
                           int32_t maxBCInstrs,
                           bool profiling,
                           bool inlining)
  : m_ctx(ctx)
  , m_interp(interp)
  , m_sk(ctx.func, ctx.bcOffset, ctx.resumed)
  , m_startSk(m_sk)
  , m_region(std::make_shared<RegionDesc>())
  , m_curBlock(m_region->addBlock(m_sk, 0, ctx.spOffset))
  , m_blockFinished(false)
  // TODO(#5703534): this is using a different TransContext than actual
  // translation will use.
  , m_irgs(TransContext{kInvalidTransID, m_sk, ctx.spOffset}, TransFlags{0})
  , m_arStates(1)
  , m_numJmps(0)
  , m_numBCInstrs(maxBCInstrs)
  , m_profiling(profiling)
  , m_inlining(inlining)
{}

const Func* RegionFormer::curFunc() const {
  return irgen::curFunc(m_irgs);
}

const Unit* RegionFormer::curUnit() const {
  return irgen::curUnit(m_irgs);
}

FPInvOffset RegionFormer::curSpOffset() const {
  return irgen::logicalStackDepth(m_irgs);
}

bool RegionFormer::resumed() const {
  return irgen::resumed(m_irgs);
}

bool RegionFormer::irBlockReachable(Block* block) {
  auto const blockId = block->id();
  auto it = m_irReachableBlocks.find(blockId);
  if (it != m_irReachableBlocks.end()) return it->second;
  bool result = block == m_irgs.irb->unit().entry();
  for (auto& pred : block->preds()) {
    if (irBlockReachable(pred.from())) {
      result = true;
      break;
    }
  }
  m_irReachableBlocks[blockId] = result;
  return result;
}

RegionDescPtr RegionFormer::go() {
  SCOPE_ASSERT_DETAIL("Tracelet Selector") {
    return folly::sformat("Region:\n{}\n\nUnit:\n{}\n",
                          *m_region, show(m_irgs.irb->unit()));
  };

  for (auto const& lt : m_ctx.liveTypes) {
    auto t = lt.type;
    if (t <= TCls) {
      irgen::assertTypeLocation(m_irgs, lt.location, t);
      m_curBlock->addPreCondition({lt.location, t, DataTypeGeneric});
    } else {
      irgen::checkTypeLocation(m_irgs, lt.location, t, m_ctx.bcOffset,
                               true /* outerOnly */);
    }
  }

  irgen::gen(m_irgs, EndGuards);

  for (bool firstInst = true; true; firstInst = false) {
    assertx(m_numBCInstrs >= 0);
    if (m_numBCInstrs == 0) {
      FTRACE(1, "selectTracelet: breaking region due to size limit\n");
      break;
    }

    if (!prepareInstruction()) break;

    m_curBlock->setKnownFunc(m_sk, m_inst.funcd);

    if (traceThroughJmp()) continue;

    m_inst.interp = m_interp.count(m_sk);

    try {
      translateInstr(m_irgs, m_inst, true /* checkOuterTypeOnly */, firstInst);
    } catch (const FailedIRGen& exn) {
      FTRACE(1, "ir generation for {} failed with {}\n",
             m_inst.toString(), exn.what());
      always_assert_flog(
        !m_interp.count(m_sk),
        "Double PUNT trying to translate {}\n", m_inst
      );
      m_interp.insert(m_sk);
      m_region.reset();
      break;
    }

    irgen::finishHHBC(m_irgs);

    if (!instrAllowsFallThru(m_inst.op())) {
      FTRACE(1, "selectTracelet: tracelet broken after instruction with no "
             "fall-through {}\n", m_inst);
      break;
    }

    // We successfully translated the instruction, so update m_sk.
    m_sk.advance(m_curBlock->unit());

    auto const endsRegion = m_inst.endsRegion;

    if (endsRegion) {
      FTRACE(1, "selectTracelet: tracelet broken after {}\n", m_inst);
      break;
    } else {
      assertx(m_sk.func() == curFunc());
    }

    auto const curIRBlock = m_irgs.irb->curBlock();
    if (!irBlockReachable(curIRBlock)) {
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

    if (isFCallStar(m_inst.op())) m_arStates.back().pop();
  }

  if (m_region && !m_region->empty()) {
    // Make sure we end the region before trying to print the IRUnit.
    irgen::endRegion(m_irgs, m_sk);

    printUnit(
      kTraceletLevel, m_irgs.irb->unit(),
      m_inlining ? " after inlining tracelet formation "
                 : " after tracelet formation ",
      nullptr,
      m_irgs.irb->guards()
    );

    recordDependencies();

    // Make sure that the IR unit contains a main exit corresponding
    // to the last bytecode instruction in the region.  Note that this
    // check has to happen before the call to truncateLiterals()
    // because that updates the region but not the IR unit.
    if (!m_region->blocks().back()->empty()) {
      auto lastSk = m_region->lastSrcKey();
      always_assert_flog(findMainExitBlock(m_irgs.irb->unit(), lastSk),
                         "No main exits found!");
    }

    truncateLiterals();
  }

  return std::move(m_region);
}

/*
 * Populate most fields of the NormalizedInstruction, assuming its sk
 * has already been set. Returns false iff the region should be
 * truncated before inst's SrcKey.
 */
bool RegionFormer::prepareInstruction() {
  m_inst.~NormalizedInstruction();
  new (&m_inst) NormalizedInstruction(m_sk, curUnit());
  auto const breaksBB =
    (m_profiling && instrBreaksProfileBB(&m_inst)) ||
    opcodeBreaksBB(m_inst.op());
  m_inst.endsRegion = breaksBB ||
    (dontGuardAnyInputs(m_inst.op()) && opcodeChangesPC(m_inst.op()));
  m_inst.funcd = m_arStates.back().knownFunc();
  irgen::prepareForNextHHBC(m_irgs, &m_inst, m_sk, false);

  auto const inputInfos = getInputs(m_inst);

  for (auto const& ii : inputInfos) m_inst.inputs.push_back(ii.loc);

  // This reads valueClass from the inputs so it used to need to
  // happen after readMetaData.  But now readMetaData is gone ...
  annotate(&m_inst);

  // Check all the inputs for unknown values.
  assertx(inputInfos.size() == m_inst.inputs.size());
  for (unsigned i = 0; i < inputInfos.size(); ++i) {
    if (!consumeInput(i, inputInfos[i])) {
      FTRACE(2, "Stopping tracelet consuming {} input {}\n",
        opcodeToName(m_inst.op()), i);
      return false;
    }
  }

  if (inputInfos.needsRefCheck) {
    // Reffiness guards are always at the beginning of the trace for now, so
    // calculate the delta from the original sp to the ar.
    auto argNum = m_inst.imm[0].u_IVA;
    size_t entryArDelta = instrSpToArDelta(m_inst.pc()) -
      (irgen::logicalStackDepth(m_irgs) - m_ctx.spOffset);
    FTRACE(5, "entryArDelta info: {} {} {}\n",
      instrSpToArDelta(m_inst.pc()),
      irgen::logicalStackDepth(m_irgs).offset,
      m_ctx.spOffset.offset);
    try {
      m_inst.preppedByRef = m_arStates.back().checkByRef(argNum, entryArDelta,
                                                         &m_refDeps);
    } catch (const UnknownInputExc& exn) {
      // We don't have a guess for the current ActRec.
      FTRACE(1, "selectTracelet: don't have reffiness guess for {}\n",
             m_inst.toString());
      return false;
    }
    addInstruction();
    m_curBlock->setParamByRef(m_inst.source, m_inst.preppedByRef);
  } else {
    addInstruction();
  }

  if (isFPush(m_inst.op())) m_arStates.back().pushFunc(m_inst);

  return true;
}

/*
 * Add the current instruction to the region.
 */
void RegionFormer::addInstruction() {
  if (m_blockFinished) {
    FTRACE(2, "selectTracelet adding new block at {} after:\n{}\n",
           showShort(m_sk), show(*m_curBlock));
    always_assert(m_sk.func() == curFunc());
    auto newCurBlock = m_region->addBlock(m_sk, 0, curSpOffset());
    m_region->addArc(m_curBlock->id(), newCurBlock->id());
    m_curBlock = newCurBlock;
    m_blockFinished = false;
  }

  FTRACE(2, "selectTracelet adding instruction {}\n", m_inst.toString());
  m_curBlock->addInstruction();
  m_numBCInstrs--;
}

bool RegionFormer::traceThroughJmp() {
  // We only trace through unconditional jumps and conditional jumps with const
  // inputs while inlining.
  if (!isUnconditionalJmp(m_inst.op()) &&
      !(m_inlining && isConditionalJmp(m_inst.op()) &&
        irgen::publicTopType(m_irgs, BCSPOffset{0}).hasConstVal())) {
    return false;
  }

  // We want to keep profiling translations to basic blocks, inlining shouldn't
  // happen in profiling translations
  if (m_profiling) {
    assert(!m_inlining);
    return false;
  }

  // Don't trace through too many jumps, unless we're inlining. We want to make
  // sure we don't break a tracelet in the middle of an inlined call; if the
  // inlined callee becomes too big that's caught in shouldIRInline.
  if (m_numJmps == Translator::MaxJmpsTracedThrough && !m_inlining) {
    return false;
  }

  auto offset = m_inst.imm[0].u_BA;
  // Only trace through backwards jumps if it's a JmpNS and we're
  // inlining. This is to get DV funclets.
  if (offset <= 0 && (m_inst.op() != OpJmpNS || !m_inlining)) {
    return false;
  }

  // Ok we're good. For unconditional jumps, just set m_sk to the dest. For
  // known conditional jumps we have to consume the const value on the top of
  // the stack and figure out which branch to go to.
  if (isUnconditionalJmp(m_inst.op())) {
    m_sk.setOffset(m_sk.offset() + offset);
  } else {
    auto value = irgen::popC(m_irgs);
    auto taken =
      value->variantVal().toBoolean() == (m_inst.op() == OpJmpNZ);
    FTRACE(2, "Tracing through {}taken Jmp(N)Z on constant {}\n",
           taken ? "" : "not ", *value->inst());

    m_sk.setOffset(taken ? m_sk.offset() + offset
                         : m_sk.advanced().offset());
  }

  m_numJmps++;
  m_blockFinished = true;
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

void RegionFormer::truncateLiterals() {
  if (!m_region || m_region->empty() ||
      m_region->blocks().back()->empty()) return;

  // Don't finish a region with literal values or values that have a class
  // related to the current context class. They produce valuable information
  // for optimizations that's lost across region boundaries.
  auto& lastBlock = *m_region->blocks().back();
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

/*
 * Check if the current predicted type for the location in ii is specific
 * enough for what the current opcode wants. If not, return false.
 */
bool RegionFormer::consumeInput(int i, const InputInfo& ii) {
  if (ii.dontGuard) return true;
  auto const type = irgen::predictedTypeFromLocation(m_irgs, ii.loc);

  if (m_profiling && type <= TBoxedCell &&
      (m_region->blocks().size() > 1 || !m_region->entry()->empty())) {
    // We don't want side exits when profiling, so only allow instructions that
    // consume refs at the beginning of the region.
    return false;
  }

  if (!ii.dontBreak && !type.isKnownDataType()) {
    // Trying to consume a value without a precise enough type.
    FTRACE(1, "selectTracelet: {} tried to consume {}\n",
           m_inst.toString(), m_inst.inputs[i].pretty());
    return false;
  }

  if (!(type <= TBoxedCell) || m_inst.ignoreInnerType || ii.dontGuardInner) {
    return true;
  }

  if (!type.inner().isKnownDataType()) {
    // Trying to consume a boxed value without a guess for the inner type.
    FTRACE(1, "selectTracelet: {} tried to consume ref {}\n",
           m_inst.toString(), m_inst.inputs[i].pretty());
    return false;
  }

  return true;
}


using VisitGuardFn =
  std::function<void(IRInstruction*, const RegionDesc::Location&, Type, bool)>;

/*
 * For every instruction in trace representing a tracelet guard, call func with
 * its location and type, and whether or not it's an inner hint.
 */
void visitGuards(IRUnit& unit, const VisitGuardFn& func) {
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
          auto bcSpOffset = inst.extra<RelOffsetData>()->bcSpOffset;
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
void RegionFormer::recordDependencies() {
  // Record the incrementally constructed reffiness predictions.
  assertx(!m_region->empty());
  auto& frontBlock = *m_region->blocks().front();
  for (auto const& dep : m_refDeps.m_arMap) {
    frontBlock.addReffinessPred({dep.second.m_mask, dep.second.m_vals,
                                 dep.first});
  }

  // Relax guards and record the ones that survived.
  auto& firstBlock = *m_region->blocks().front();
  auto& unit = m_irgs.unit;
  auto guardMap = std::map<RegionDesc::Location,Type>{};
  ITRACE(2, "Visiting guards\n");
  auto hintMap = std::map<RegionDesc::Location,Type>{};
  auto catMap = std::map<RegionDesc::Location,DataTypeCategory>{};
  const auto& guards = m_irgs.irb->guards()->guards;
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

///////////////////////////////////////////////////////////////////////////////
}

RegionDescPtr selectTracelet(const RegionContext& ctx, int32_t maxBCInstrs,
                             bool profiling, bool inlining /* = false */) {
  Timer _t(Timer::selectTracelet);
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 1;

  FTRACE(1, "selectTracelet: starting with maxBCInstrs = {}\n", maxBCInstrs);

  while (!(region = RegionFormer(ctx, interp, maxBCInstrs, profiling,
                                 inlining).go())) {
    ++tries;
  }

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
