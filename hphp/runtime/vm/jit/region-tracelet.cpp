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

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/ref-deps.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/util/trace.h"

#include <algorithm>
#include <vector>

namespace HPHP { namespace jit {

TRACE_SET_MOD(region);

typedef hphp_hash_set<SrcKey, SrcKey::Hasher> InterpSet;

namespace {
///////////////////////////////////////////////////////////////////////////////

struct RegionFormer {
  RegionFormer(const RegionContext& ctx,
               InterpSet& interp,
               bool profiling, bool inlining);

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
  IRTranslator m_irTrans;
  HhbcTranslator& m_ht;
  jit::vector<ActRecState> m_arStates;
  RefDeps m_refDeps;
  uint32_t m_numJmps;
  uint32_t m_numBCInstrs{0};

  const bool m_profiling;
  const bool m_inlining;
  InliningDecider m_inl;

  const Func* curFunc() const;
  const Unit* curUnit() const;
  Offset curSpOffset() const;
  bool resumed() const;

  bool prepareInstruction();
  void addInstruction();
  bool consumeInput(int i, const InputInfo& ii);
  bool traceThroughJmp();
  void maybeConstrainForInlining();
  void recordDependencies();
  void truncateLiterals();
};

RegionFormer::RegionFormer(const RegionContext& ctx,
                           InterpSet& interp,
                           bool profiling, bool inlining)
  : m_ctx(ctx)
  , m_interp(interp)
  , m_sk(ctx.func, ctx.bcOffset, ctx.resumed)
  , m_startSk(m_sk)
  , m_region(std::make_shared<RegionDesc>())
  , m_curBlock(m_region->addBlock(m_sk, 0, ctx.spOffset))
  , m_blockFinished(false)
  , m_irTrans(TransContext { kInvalidTransID,
                             ctx.bcOffset,
                             ctx.spOffset,
                             ctx.resumed,
                             ctx.func })
  , m_ht(m_irTrans.hhbcTrans())
  , m_arStates(1)
  , m_numJmps(0)
  , m_profiling(profiling)
  , m_inlining(inlining)
  , m_inl(ctx.func)
{}

const Func* RegionFormer::curFunc() const {
  return m_ht.curFunc();
}

const Unit* RegionFormer::curUnit() const {
  return m_ht.curUnit();
}

Offset RegionFormer::curSpOffset() const {
  return m_ht.spOffset();
}

bool RegionFormer::resumed() const {
  return m_ht.resumed();
}

RegionDescPtr RegionFormer::go() {
  for (auto const& lt : m_ctx.liveTypes) {
    auto t = lt.type;
    if (t <= Type::Cls) {
      m_ht.assertTypeStack(lt.location.stackOffset(), t);
      m_curBlock->addPredicted(m_sk, RegionDesc::TypePred{lt.location, t});
    } else {
      m_ht.guardTypeLocation(lt.location, t, true /* outerOnly */);
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
    auto const doPrediction =
      m_profiling ? false : outputIsPredicted(m_inst);

    maybeConstrainForInlining();

    try {
      m_irTrans.translateInstr(m_inst);
    } catch (const FailedIRGen& exn) {
      FTRACE(1, "ir generation for {} failed with {}\n",
             m_inst.toString(), exn.what());
      always_assert(!m_interp.count(m_sk));
      m_interp.insert(m_sk);
      m_region.reset();
      break;
    }

    // We successfully translated the instruction, so update m_sk.
    m_sk.advance(m_curBlock->unit());

    auto const endsRegion = m_inst.endsRegion;

    if (endsRegion) {
      FTRACE(1, "selectTracelet: tracelet broken after {}\n", m_inst);
      break;
    } else {
      assert(m_sk.func() == m_ht.curFunc());
    }

    if (isFCallStar(m_inst.op())) m_arStates.back().pop();

    // Since the current instruction is over, advance HhbcTranslator's sk
    // before emitting the prediction (if any).
    if (doPrediction &&
        m_ht.topType(0, DataTypeGeneric).maybe(m_inst.outPred)) {
      m_ht.setBcOff(m_sk.offset(), false);
      m_ht.checkTypeStack(0, m_inst.outPred, m_sk.offset());
    }
  }

  printUnit(kTraceletLevel, m_ht.unit(),
            m_inlining ? " after inlining tracelet formation "
                       : " after tracelet formation ",
            nullptr, m_ht.irBuilder().guards());

  if (m_region && !m_region->empty()) {
    m_ht.end(m_sk.offset());
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
  m_ht.setBcOff(m_sk.offset(), false);

  auto const inputInfos = getInputs(m_startSk, m_inst);

  // Read types for all the inputs and apply MetaData.
  auto newDynLoc = [&](const InputInfo& ii) {
    auto dl = m_inst.newDynLoc(ii.loc, m_ht.predictedTypeFromLocation(ii.loc));
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
      (m_ht.spOffset() - m_ctx.spOffset);
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
}

bool RegionFormer::traceThroughJmp() {
  // We only trace through unconditional jumps and conditional jumps with const
  // inputs while inlining.
  if (!isUnconditionalJmp(m_inst.op()) &&
      !(m_inlining && isConditionalJmp(m_inst.op()) &&
        m_ht.topType(0, DataTypeGeneric).isConst())) {
    return false;
  }

  // Normally we want to keep profiling translations to basic blocks, but if
  // we're inlining we want to analyze as much of the callee as possible.
  if (m_profiling && !m_inlining) return false;

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
    auto value = m_ht.popC();
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

/*
 * If we are at the FCall of a potentially inlinable callee, we constrain the
 * types of all arguments to the function in order to ensure that type
 * informational necessary for inlining is not lost by translation time due to
 * guard relaxation.
 *
 * TODO(#5656429): Once it's possible to turn off guard relaxation during
 * tracelet formation, and leave Translator to handle it, we should be able to
 * remove this call.
 */
void RegionFormer::maybeConstrainForInlining() {
  if (m_sk.op() != Op::FCall &&
      m_sk.op() != Op::FCallD) {
    return;
  }
  auto const callee = m_inst.funcd;

  if (!m_region || m_region->empty()) return;
  if (!m_inl.canInlineAt(m_sk, callee, *m_region)) return;

  // Make sure the FPushOp wasn't interpreted.
  auto const sframe = findSpillFrame(m_ht.irBuilder().sp());
  if (!sframe) return;

  auto calleeRegion = selectCalleeRegion(m_sk, callee, m_ht, m_profiling);
  if (!calleeRegion) return;

  if (!m_inl.shouldInline(callee, *calleeRegion)) return;
  m_inl.registerEndInlining(callee);

  auto const op = reinterpret_cast<const Op*>(m_sk.pc());
  auto const numArgs = getImm(op, 0).u_IVA;

  auto tcFor = [] (const Type& t) {
    if (t.canSpecializeArray()) {
      return TypeConstraint(DataTypeSpecialized).setWantArrayKind();
    }
    if (t.canSpecializeClass() && t.getClass()) {
      return TypeConstraint(t.getClass());
    }
    if (t < Type::Cls && t.isConst()) {
      return TypeConstraint(t.clsVal());
    }
    if (t < Type::Cctx && t.isConst()) {
      return TypeConstraint(t.cctxVal().cls());
    }
    return TypeConstraint(DataTypeSpecific);
  };

  // Constrain all the argument types.
  for (unsigned i = 0; i < numArgs; ++i) {
    auto const tc = tcFor(m_ht.topType(i));
    if (tc != DataTypeSpecific) m_ht.topType(i, tc);
  }

  // Constrain the this, class, or context class type if there is one.
  if (auto const ssa = sframe->src(2)) {
    m_ht.irBuilder().constrainValue(ssa, tcFor(ssa->type()));
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
  auto& unit = m_ht.unit();
  auto const doRelax = RuntimeOption::EvalHHIRRelaxGuards;
  bool changed = false;
  if (doRelax) {
    Timer _t(Timer::selectTracelet_relaxGuards);
    // The IR is going to be discarded immediately, so skip reflowing
    // the types in relaxGuards to save JIT time.
    RelaxGuardsFlags flags = m_profiling ? RelaxSimple : RelaxNormal;
    changed = relaxGuards(unit, *m_ht.irBuilder().guards(), flags);
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
              m_ht.irBuilder().guards());
  }

}

///////////////////////////////////////////////////////////////////////////////
}

RegionDescPtr selectTracelet(const RegionContext& ctx, bool profiling,
                             bool inlining /* = false */) {
  Timer _t(Timer::selectTracelet);
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 1;

  while (!(region = RegionFormer(ctx, interp, profiling, inlining).go())) {
    ++tries;
  }

  if (region->empty() || region->blocks().front()->length() == 0) {
    FTRACE(1, "selectTracelet giving up after {} tries\n", tries);
    return RegionDescPtr { nullptr };
  }

  FTRACE(1, "selectTracelet returning, {}, {} tries:\n{}\n",
         inlining ? "inlining" : "not inlining", tries, show(*region));
  if (region->blocks().back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->deleteBlock(region->blocks().back()->id());
  }
  return region;
}

} }
