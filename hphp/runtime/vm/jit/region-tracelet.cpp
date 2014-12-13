/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/guard-relaxation.h"
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
               InliningDecider& inl,
               bool profiling);

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
  HTS m_hts;
  jit::vector<ActRecState> m_arStates;
  RefDeps m_refDeps;
  uint32_t m_numJmps;
  uint32_t m_numBCInstrs{0};
  uint32_t m_pendingInlinedInstrs{0};

  InliningDecider& m_inl;
  const bool m_profiling;

  const Func* curFunc() const;
  const Unit* curUnit() const;
  Offset curSpOffset() const;
  bool resumed() const;

  bool prepareInstruction();
  void addInstruction();
  bool consumeInput(int i, const InputInfo& ii);
  bool traceThroughJmp();
  bool tryInline(uint32_t& instrSize);
  void recordDependencies();
  void truncateLiterals();
};

RegionFormer::RegionFormer(const RegionContext& ctx,
                           InterpSet& interp,
                           InliningDecider& inl,
                           bool profiling)
  : m_ctx(ctx)
  , m_interp(interp)
  , m_sk(ctx.func, ctx.bcOffset, ctx.resumed)
  , m_startSk(m_sk)
  , m_region(std::make_shared<RegionDesc>())
  , m_curBlock(m_region->addBlock(m_sk, 0, ctx.spOffset))
  , m_blockFinished(false)
  , m_hts(// TODO(#5703534): this is using a different TransContext than actual
          // translation will use.
          TransContext { kInvalidTransID,
                         ctx.bcOffset,
                         ctx.spOffset,
                         ctx.resumed,
                         ctx.func,
                         nullptr })
  , m_arStates(1)
  , m_numJmps(0)
  , m_inl(inl)
  , m_profiling(profiling)
{}

const Func* RegionFormer::curFunc() const {
  return irgen::curFunc(m_hts);
}

const Unit* RegionFormer::curUnit() const {
  return irgen::curUnit(m_hts);
}

Offset RegionFormer::curSpOffset() const {
  return irgen::spOffset(m_hts);
}

bool RegionFormer::resumed() const {
  return irgen::resumed(m_hts);
}

RegionDescPtr RegionFormer::go() {
  for (auto const& lt : m_ctx.liveTypes) {
    auto t = lt.type;
    if (t <= Type::Cls) {
      irgen::assertTypeLocation(m_hts, lt.location, t);
      m_curBlock->addPredicted(m_sk, RegionDesc::TypePred{lt.location, t});
    } else {
      irgen::guardTypeLocation(m_hts, lt.location, t, true /* outerOnly */);
    }
  }

  while (true) {
    assert(m_numBCInstrs <= RuntimeOption::EvalJitMaxRegionInstrs);
    if (m_numBCInstrs == RuntimeOption::EvalJitMaxRegionInstrs) {
      FTRACE(1, "selectTracelet: breaking region due to size limit ({})\n",
             m_numBCInstrs);
      break;
    }

    if (!prepareInstruction()) break;

    if (traceThroughJmp()) continue;

    m_curBlock->setKnownFunc(m_sk, m_inst.funcd);

    m_inst.interp = m_interp.count(m_sk);

    uint32_t calleeInstrSize;
    if (tryInline(calleeInstrSize)) {
      // If m_inst is an FCall and the callee is suitable for inlining, we can
      // translate the callee and potentially use its return type to extend the
      // tracelet.

      auto callee = m_inst.funcd;
      FTRACE(1, "\nselectTracelet starting inlined call from {} to "
             "{} with stack:\n{}\n", curFunc()->fullName()->data(),
             callee->fullName()->data(), show(m_hts));
      auto returnSk = m_inst.nextSk();
      auto returnFuncOff = returnSk.offset() - curFunc()->base();

      m_arStates.back().pop();
      m_arStates.emplace_back();
      m_curBlock->setInlinedCallee(callee);
      irgen::beginInlining(m_hts, m_inst.imm[0].u_IVA, callee, returnFuncOff);

      m_sk = irgen::curSrcKey(m_hts);
      m_blockFinished = true;
      m_pendingInlinedInstrs += calleeInstrSize;
      continue;
    }

    auto const inlineReturn = irgen::isInlining(m_hts) &&
      (isRet(m_inst.op()) || m_inst.op() == OpNativeImpl);
    try {
      translateInstr(m_hts, m_inst);
    } catch (const FailedIRGen& exn) {
      FTRACE(1, "ir generation for {} failed with {}\n",
             m_inst.toString(), exn.what());
      always_assert(!m_interp.count(m_sk));
      m_interp.insert(m_sk);
      m_region.reset();
      break;
    }

    // If we just translated a return from an inlined call, grab the updated
    // SrcKey from m_ht and clean up.
    if (inlineReturn) {
      m_inl.registerEndInlining(m_sk.func());
      m_sk = irgen::curSrcKey(m_hts).advanced(curUnit());
      m_arStates.pop_back();
      m_blockFinished = true;
      continue;
    }

    // We successfully translated the instruction, so update m_sk.
    m_sk.advance(m_curBlock->unit());

    auto const endsRegion = m_inst.endsRegion;

    if (endsRegion) {
      FTRACE(1, "selectTracelet: tracelet broken after {}\n", m_inst);
      break;
    } else {
      assert(m_sk.func() == curFunc());
    }

    if (isFCallStar(m_inst.op())) m_arStates.back().pop();
  }

  // If we failed while trying to inline, trigger retry without inlining.
  if (m_region && !m_region->empty() && irgen::isInlining(m_hts)) {
    // We can recover from this situation just fine, but it's more often
    // than not indicative of a real bug somewhere else in the system.
    // TODO: 5515310 investigate whether legit bugs cause this.
    FTRACE(1, "selectTracelet: Failed while inlining:\n{}\n{}",
           show(*m_region), m_hts.irb->unit());
    m_inl.disable();
    m_region.reset();
  }

  if (m_region && !m_region->empty()) {
    // Make sure we end the region before trying to print the IRUnit.
    irgen::endRegion(m_hts, m_sk.offset());

    printUnit(
      kTraceletLevel, m_hts.irb->unit(),
      m_inl.depth() || m_inl.disabled() ? " after inlining tracelet formation "
                                        : " after tracelet formation ",
      nullptr,
      m_hts.irb->guards()
    );

    recordDependencies();
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
    (mcg->useLLVM() ? opcodeChangesPC(m_inst.op())
                    : opcodeBreaksBB(m_inst.op()));
  m_inst.endsRegion = breaksBB ||
    (dontGuardAnyInputs(m_inst.op()) && opcodeChangesPC(m_inst.op()));
  m_inst.funcd = m_arStates.back().knownFunc();
  irgen::prepareForNextHHBC(m_hts, &m_inst, m_sk.offset(), false);

  auto const inputInfos = getInputs(m_startSk, m_inst);

  // Read types for all the inputs and apply MetaData.
  auto newDynLoc = [&](const InputInfo& ii) {
    auto dl = m_inst.newDynLoc(
      ii.loc,
      irgen::predictedTypeFromLocation(m_hts, ii.loc)
    );
    FTRACE(2, "predictedTypeFromLocation: {} -> {}\n",
           ii.loc.pretty(), dl->rtt);
    return dl;
  };

  for (auto const& ii : inputInfos) m_inst.inputs.push_back(newDynLoc(ii));

  // This reads valueClass from the inputs so it used to need to
  // happen after readMetaData.  But now readMetaData is gone ...
  annotate(&m_inst);

  // Check all the inputs for unknown values.
  assert(inputInfos.size() == m_inst.inputs.size());
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
    size_t entryArDelta = instrSpToArDelta((Op*)m_inst.pc()) -
      (irgen::spOffset(m_hts) - m_ctx.spOffset);
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
    RegionDesc::Block* newCurBlock = m_region->addBlock(m_sk, 0, curSpOffset());
    m_region->addArc(m_curBlock->id(), newCurBlock->id());
    m_curBlock = newCurBlock;
    m_blockFinished = false;
  }

  FTRACE(2, "selectTracelet adding instruction {}\n", m_inst.toString());
  m_curBlock->addInstruction();
  m_numBCInstrs++;
  if (irgen::isInlining(m_hts)) m_pendingInlinedInstrs--;
}

bool RegionFormer::traceThroughJmp() {
  // This flag means "if we are currently inlining, or we are generating a
  // tracelet for inlining analysis".  The latter is the case when inlining is
  // disabled (because our caller wants to peek at our region without us
  // inlining ourselves).
  bool inlining = m_inl.depth() || m_inl.disabled();

  // We only trace through unconditional jumps and conditional jumps with const
  // inputs while inlining.
  if (!isUnconditionalJmp(m_inst.op()) &&
      !(inlining && isConditionalJmp(m_inst.op()) &&
        irgen::publicTopType(m_hts, 0).isConst())) {
    return false;
  }

  // Normally we want to keep profiling translations to basic blocks, but if
  // we're inlining we want to analyze as much of the callee as possible.
  if (m_profiling && !inlining) return false;

  // Don't trace through too many jumps, unless we're inlining. We want to make
  // sure we don't break a tracelet in the middle of an inlined call; if the
  // inlined callee becomes too big that's caught in shouldIRInline.
  if (m_numJmps == Translator::MaxJmpsTracedThrough && !inlining) {
    return false;
  }

  auto offset = m_inst.imm[0].u_BA;
  // Only trace through backwards jumps if it's a JmpNS and we're
  // inlining. This is to get DV funclets.
  if (offset <= 0 && (m_inst.op() != OpJmpNS || !inlining)) {
    return false;
  }

  // Ok we're good. For unconditional jumps, just set m_sk to the dest. For
  // known conditional jumps we have to consume the const value on the top of
  // the stack and figure out which branch to go to.
  if (isUnconditionalJmp(m_inst.op())) {
    m_sk.setOffset(m_sk.offset() + offset);
  } else {
    auto value = irgen::popC(m_hts);
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

bool RegionFormer::tryInline(uint32_t& instrSize) {
  assert(m_inst.source == m_sk);
  assert(m_inst.func() == curFunc());
  assert(m_sk.resumed() == resumed());

  instrSize = 0;

  if (!m_inl.canInlineAt(m_inst.source, m_inst.funcd, *m_region)) {
    return false;
  }

  auto refuse = [this](const std::string& str) {
    FTRACE(2, "selectTracelet not inlining {}: {}\n",
           m_inst.toString(), str);
    return false;
  };

  auto callee = m_inst.funcd;

  // Make sure the FPushOp wasn't interpreted.
  auto spillFrame = findSpillFrame(m_hts.irb->sp());
  if (!spillFrame) {
    return refuse("couldn't find SpillFrame for FPushOp");
  }

  auto numArgs = m_inst.imm[0].u_IVA;
  auto numParams = callee->numParams();

  // Set up the region context, mapping stack slots in the caller to locals in
  // the callee.
  RegionContext ctx;
  ctx.func = callee;
  ctx.bcOffset = callee->getEntryForNumArgs(numArgs);
  ctx.spOffset = callee->numSlotsInFrame();
  ctx.resumed = false;
  for (int i = 0; i < numArgs; ++i) {
    auto type = irgen::publicTopType(m_hts, i);
    uint32_t paramIdx = numArgs - 1 - i;
    ctx.liveTypes.push_back({RegionDesc::Location::Local{paramIdx}, type});
  }

  for (unsigned i = numArgs; i < numParams; ++i) {
    // These locals will be populated by DV init funclets but they'll start
    // out as Uninit.
    ctx.liveTypes.push_back({RegionDesc::Location::Local{i}, Type::Uninit});
  }

  FTRACE(1, "selectTracelet analyzing callee {} with context:\n{}",
         callee->fullName()->data(), show(ctx));
  auto region = selectTracelet(ctx, m_profiling, false /* noinline */);
  if (!region) {
    return refuse("failed to select region in callee");
  }

  instrSize = region->instrSize();
  auto newInstrSize = instrSize + m_numBCInstrs + m_pendingInlinedInstrs;
  if (newInstrSize > RuntimeOption::EvalJitMaxRegionInstrs) {
    return refuse("new region would be too large");
  }
  if (!m_inl.shouldInline(callee, *region)) {
    return refuse("shouldIRInline failed");
  }
  return true;
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
  FTRACE(1, "selectTracelet truncating block after offset {}:\n{}\n",
         endSk.offset(), show(lastBlock));
  lastBlock.truncateAfter(endSk);
}

/*
 * Check if the current type for the location in ii is specific enough for what
 * the current opcode wants. If not, return false.
 */
bool RegionFormer::consumeInput(int i, const InputInfo& ii) {
  auto& rtt = m_inst.inputs[i]->rtt;
  if (ii.dontGuard) return true;

  if (m_profiling && rtt.isBoxed() &&
      (m_region->blocks().size() > 1 || !m_region->entry()->empty())) {
    // We don't want side exits when profiling, so only allow instructions that
    // consume refs at the beginning of the region.
    return false;
  }

  if (!ii.dontBreak && !rtt.isKnownDataType()) {
    // Trying to consume a value without a precise enough type.
    FTRACE(1, "selectTracelet: {} tried to consume {}\n",
           m_inst.toString(), m_inst.inputs[i]->pretty());
    return false;
  }

  if (!rtt.isBoxed() || m_inst.ignoreInnerType || ii.dontGuardInner) {
    return true;
  }

  if (!rtt.innerType().isKnownDataType()) {
    // Trying to consume a boxed value without a guess for the inner type.
    FTRACE(1, "selectTracelet: {} tried to consume ref {}\n",
           m_inst.toString(), m_inst.inputs[i]->pretty());
    return false;
  }

  return true;
}

/*
 * Records any type/reffiness predictions we depend on in the region. Guards
 * for locals and stack cells that are not used will be eliminated by the call
 * to relaxGuards.
 */
void RegionFormer::recordDependencies() {
  // Record the incrementally constructed reffiness predictions.
  assert(!m_region->empty());
  auto& frontBlock = *m_region->blocks().front();
  for (auto const& dep : m_refDeps.m_arMap) {
    frontBlock.addReffinessPred(m_startSk, {dep.second.m_mask,
                                            dep.second.m_vals,
                                            dep.first});
  }

  // Relax guards and record the ones that survived.
  auto& firstBlock = *m_region->blocks().front();
  auto blockStart = firstBlock.start();
  auto& unit = m_hts.unit;
  auto const doRelax = RuntimeOption::EvalHHIRRelaxGuards;
  bool changed = false;
  if (doRelax) {
    Timer _t(Timer::selectTracelet_relaxGuards);
    // The IR is going to be discarded immediately, so skip reflowing
    // the types in relaxGuards to save JIT time.
    RelaxGuardsFlags flags = m_profiling ? RelaxSimple : RelaxNormal;
    changed = relaxGuards(unit, *m_hts.irb->guards(), flags);
  }

  auto guardMap = std::map<RegionDesc::Location,Type>{};
  visitGuards(unit, [&](const RegionDesc::Location& loc, Type type) {
    if (type <= Type::Cls) return;
    auto inret = guardMap.insert(std::make_pair(loc, type));
    if (inret.second) return;
    auto& oldTy = inret.first->second;
    if (oldTy == Type::Gen) {
      // This is the case that we see an inner type prediction for a GuardLoc
      // that got relaxed to Gen.
      return;
    }
    oldTy &= type;
  });

  for (auto& kv : guardMap) {
    if (kv.second == Type::Gen) {
      // Guard was relaxed to Gen---don't record it.
      continue;
    }
    auto const pred = RegionDesc::TypePred { kv.first, kv.second };
    FTRACE(1, "selectTracelet adding guard {}\n", show(pred));
    firstBlock.addPredicted(blockStart, pred);
  }

  if (changed) {
    printUnit(3, unit, " after guard relaxation ", nullptr,
              m_hts.irb->guards());
  }

}

///////////////////////////////////////////////////////////////////////////////
}

RegionDescPtr selectTracelet(const RegionContext& ctx, bool profiling,
                             bool allowInlining /* = true */) {
  Timer _t(Timer::selectTracelet);
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 1;

  InliningDecider inl(ctx.func);
  if (!allowInlining) inl.disable();

  while (!(region = RegionFormer(ctx, interp, inl, profiling).go())) {
    ++tries;
    inl.resetState();
  }

  if (region->empty() || region->blocks().front()->length() == 0) {
    FTRACE(1, "selectTracelet giving up after {} tries\n", tries);
    return RegionDescPtr { nullptr };
  }

  FTRACE(1, "selectTracelet returning, inlining {}, {} tries:\n{}\n",
         allowInlining ? "allowed" : "disallowed", tries, show(*region));
  if (region->blocks().back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->deleteBlock(region->blocks().back()->id());
  }
  return region;
}

} }
