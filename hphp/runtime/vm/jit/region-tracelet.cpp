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

#include "hphp/util/trace.h"
#include <algorithm>
#include <vector>
#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace JIT {


TRACE_SET_MOD(region);

typedef hphp_hash_set<SrcKey, SrcKey::Hasher> InterpSet;

namespace {
struct RegionDescIter : public RegionIter {
  explicit RegionDescIter(const RegionDesc& region)
    : m_blocks(region.blocks)
    , m_blockIter(region.blocks.begin())
    , m_sk(m_blockIter == m_blocks.end() ? SrcKey() : (*m_blockIter)->start())
  {}

  bool finished() const { return m_blockIter == m_blocks.end(); }

  SrcKey sk() const {
    assert(!finished());
    return m_sk;
  }

  void advance() {
    assert(!finished());
    assert(m_sk.func() == (*m_blockIter)->func());

    if (m_sk == (*m_blockIter)->last()) {
      ++m_blockIter;
      if (!finished()) m_sk = (*m_blockIter)->start();
    } else {
      m_sk.advance();
    }
  }

 private:
  const std::vector<RegionDesc::BlockPtr>& m_blocks;
  std::vector<RegionDesc::BlockPtr>::const_iterator m_blockIter;
  SrcKey m_sk;
};

struct RegionFormer {
  RegionFormer(const RegionContext& ctx, InterpSet& interp, int inlineDepth,
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
  IRTranslator m_irTrans;
  HhbcTranslator& m_ht;
  Unit::MetaHandle m_metaHand;
  smart::vector<ActRecState> m_arStates;
  RefDeps m_refDeps;
  const int m_inlineDepth;
  const bool m_profiling;

  const Func* curFunc() const;
  const Unit* curUnit() const;
  Offset curSpOffset() const;
  int inliningDepth() const;

  bool prepareInstruction();
  void addInstruction();
  bool consumeInput(int i, const InputInfo& ii);
  bool tryInline();
  void recordDependencies();
  void truncateLiterals();
};

RegionFormer::RegionFormer(const RegionContext& ctx, InterpSet& interp,
                           int inlineDepth, bool profiling)
  : m_ctx(ctx)
  , m_interp(interp)
  , m_sk(ctx.func, ctx.bcOffset)
  , m_startSk(m_sk)
  , m_region(std::make_shared<RegionDesc>())
  , m_curBlock(m_region->addBlock(ctx.func, m_sk.offset(), 0, ctx.spOffset))
  , m_blockFinished(false)
  , m_irTrans(ctx.bcOffset, ctx.spOffset, ctx.inGenerator, ctx.func)
  , m_ht(m_irTrans.hhbcTrans())
  , m_arStates(1)
  , m_inlineDepth(inlineDepth)
  , m_profiling(profiling)
{
}

const Func* RegionFormer::curFunc() const {
  return m_ht.curFunc();
}

const Unit* RegionFormer::curUnit() const {
  return m_ht.curUnit();
}

Offset RegionFormer::curSpOffset() const {
  return m_ht.spOffset();
}

int RegionFormer::inliningDepth() const {
  return m_inlineDepth + m_ht.inliningDepth();
}

RegionDescPtr RegionFormer::go() {
  uint32_t numJmps = 0;

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
    if (!prepareInstruction()) break;

    // Instead of translating a Jmp, go to its destination.
    if (!m_profiling && isUnconditionalJmp(m_inst.op()) &&
        m_inst.imm[0].u_BA > 0 && numJmps < Translator::MaxJmpsTracedThrough) {
      // Include the Jmp in the region and continue to its destination.
      ++numJmps;
      m_sk.setOffset(m_sk.offset() + m_inst.imm[0].u_BA);
      m_blockFinished = true;

      continue;
    }

    m_curBlock->setKnownFunc(m_sk, m_inst.funcd);

    m_inst.interp = m_interp.count(m_sk);
    auto const doPrediction =
      m_profiling ? false : outputIsPredicted(m_inst);

    if (tryInline()) {
      // If m_inst is an FCall and the callee is suitable for inlining, we can
      // translate the callee and potentially use its return type to extend the
      // tracelet.

      auto callee = m_inst.funcd;
      FTRACE(1, "\nselectTracelet starting inlined call from {} to "
             "{} with stack:\n{}\n", curFunc()->fullName()->data(),
             callee->fullName()->data(), m_ht.showStack());
      auto returnSk = m_inst.nextSk();
      auto returnFuncOff = returnSk.offset() - curFunc()->base();

      m_arStates.back().pop();
      m_arStates.emplace_back();
      m_curBlock->setInlinedCallee(callee);
      m_ht.beginInlining(m_inst.imm[0].u_IVA, callee, returnFuncOff,
                         doPrediction ? m_inst.outPred : Type::Gen);
      m_metaHand = Unit::MetaHandle();

      m_sk = m_ht.curSrcKey();
      m_blockFinished = true;
      continue;
    }

    auto const inlineReturn = m_ht.isInlining() && isRet(m_inst.op());
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

    if (inlineReturn) {
      // If we just translated an inlined RetC, grab the updated SrcKey from
      // m_ht and clean up.
      m_metaHand = Unit::MetaHandle();
      m_sk = m_ht.curSrcKey().advanced(curUnit());
      m_arStates.pop_back();
      m_blockFinished = true;
      continue;
    } else if (m_inst.breaksTracelet ||
               (m_profiling && instrBreaksProfileBB(&m_inst))) {
      FTRACE(1, "selectTracelet: tracelet broken after {}\n", m_inst);
      break;
    } else {
      assert(m_sk.func() == m_ht.curFunc());
    }

    if (isFCallStar(m_inst.op())) m_arStates.back().pop();

    // Since the current instruction is over, advance HhbcTranslator's sk
    // before emitting the prediction (if any).
    if (doPrediction) {
      m_ht.setBcOff(m_sk.offset(), false);
      m_ht.checkTypeStack(0, m_inst.outPred, m_sk.offset());
    }
  }

  dumpTrace(2, m_ht.unit(), " after tracelet formation ",
            nullptr, nullptr, m_ht.irBuilder().guards());

  if (m_region && !m_region->blocks.empty()) {
    always_assert_log(
      !m_ht.isInlining(),
      [&] {
        return folly::format("Tried to end region while inlining:\n{}",
                             m_ht.unit()).str();
      });

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
  new (&m_inst) NormalizedInstruction();
  m_inst.source = m_sk;
  m_inst.m_unit = curUnit();
  m_inst.breaksTracelet = opcodeBreaksBB(m_inst.op()) ||
                            (dontGuardAnyInputs(m_inst.op()) &&
                             opcodeChangesPC(m_inst.op()));
  m_inst.changesPC = opcodeChangesPC(m_inst.op());
  m_inst.funcd = m_arStates.back().knownFunc();
  populateImmediates(m_inst);
  preInputApplyMetaData(m_metaHand, &m_inst);
  m_ht.setBcOff(m_sk.offset(), false);

  InputInfos inputInfos;
  getInputs(m_startSk, m_inst, inputInfos, m_curBlock->func(), [&](int i) {
    return m_ht.irBuilder().localType(i, DataTypeGeneric);
  });

  // Read types for all the inputs and apply MetaData.
  auto newDynLoc = [&](const InputInfo& ii) {
    auto dl = m_inst.newDynLoc(ii.loc, m_ht.rttFromLocation(ii.loc));
    FTRACE(2, "rttFromLocation: {} -> {}\n",
           ii.loc.pretty(), dl->rtt.pretty());
    return dl;
  };

  for (auto const& ii : inputInfos) m_inst.inputs.push_back(newDynLoc(ii));
  try {
    readMetaData(m_metaHand, m_inst, m_ht, m_profiling);
  } catch (const FailedTraceGen& exn) {
    FTRACE(1, "failed to apply metadata for {}: {}\n",
           m_inst, exn.what());
    return false;
  }

  // This reads valueClass from the inputs so it needs to happen after
  // readMetaData.
  if (inliningDepth() == 0) annotate(&m_inst);

  // Check all the inputs for unknown values.
  assert(inputInfos.size() == m_inst.inputs.size());
  for (unsigned i = 0; i < inputInfos.size(); ++i) {
    if (!consumeInput(i, inputInfos[i])) return false;
  }

  if (!m_inst.noOp && inputInfos.needsRefCheck) {
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
    RegionDesc::Block* newCurBlock = m_region->addBlock(curFunc(),
                                                        m_sk.offset(), 0,
                                                        curSpOffset());
    m_region->addArc(m_curBlock->id(), newCurBlock->id());
    m_curBlock = newCurBlock;
    m_blockFinished = false;
  }

  FTRACE(2, "selectTracelet adding instruction {}\n", m_inst.toString());
  m_curBlock->addInstruction();
}

bool RegionFormer::tryInline() {
  if (!RuntimeOption::RepoAuthoritative ||
      (m_inst.op() != Op::FCall && m_inst.op() != Op::FCallD)) {
    return false;
  }

  auto refuse = [this](const std::string& str) {
    FTRACE(2, "selectTracelet not inlining {}: {}\n",
           m_inst.toString(), str);
    return false;
  };

  if (inliningDepth() >= RuntimeOption::EvalHHIRInliningMaxDepth) {
    return refuse("inlining level would be too deep");
  }

  auto callee = m_inst.funcd;
  if (!callee || callee->isCPPBuiltin()) {
    return refuse("don't know callee or callee is builtin");
  }

  if (callee == curFunc()) {
    return refuse("call is recursive");
  }

  if (m_inst.imm[0].u_IVA != callee->numParams()) {
    return refuse("numArgs doesn't match numParams of callee");
  }

  // For analysis purposes, we require that the FPush* instruction is in the
  // same region.
  auto fpi = curFunc()->findFPI(m_sk.offset());
  const SrcKey pushSk{curFunc(), fpi->m_fpushOff};
  int pushBlock = -1;
  auto& blocks = m_region->blocks;
  for (unsigned i = 0; i < blocks.size(); ++i) {
    if (blocks[i]->contains(pushSk)) {
      pushBlock = i;
      break;
    }
  }
  if (pushBlock == -1) {
    return refuse("FPush* is not in the current region");
  }

  // Calls invalidate all live SSATmps, so don't allow any in the fpi region
  auto findFCall = [&] {
    for (unsigned i = pushBlock; i < blocks.size(); ++i) {
      auto& block = *blocks[i];
      auto sk = i == pushBlock ? pushSk.advanced() : block.start();
      while (sk <= block.last()) {
        if (sk == m_sk) return false;

        auto op = sk.op();
        if (isFCallStar(op) || op == Op::FCallBuiltin) return true;
        sk.advance();
      }
    }
    not_reached();
  };
  if (findFCall()) {
    return refuse("fpi region contains another call");
  }

  switch (pushSk.op()) {
    case OpFPushClsMethodD:
      if (callee->mayHaveThis()) return refuse("callee may have this pointer");
      // fallthrough
    case OpFPushFuncD:
    case OpFPushObjMethodD:
    case OpFPushCtorD:
    case OpFPushCtor:
      break;

    default:
      return refuse(folly::format("unsupported push op {}",
                                  opcodeToName(pushSk.op())).str());
  }

  // Make sure the FPushOp wasn't interpreted.
  auto spillFrame = findSpillFrame(m_ht.irBuilder().sp());
  if (!spillFrame) {
    return refuse("couldn't find SpillFrame for FPushOp");
  }

  // Set up the region context, mapping stack slots in the caller to locals in
  // the callee.
  RegionContext ctx;
  ctx.func = callee;
  ctx.bcOffset = callee->base();
  ctx.spOffset = callee->numSlotsInFrame();
  ctx.inGenerator = false;
  for (int i = 0; i < callee->numParams(); ++i) {
    // DataTypeGeneric is used because we're just passing the locals into the
    // callee. It's up to the callee to constraint further if needed.
    auto type = m_ht.topType(i, DataTypeGeneric);
    uint32_t paramIdx = callee->numParams() - 1 - i;
    typedef RegionDesc::Location Location;
    ctx.liveTypes.push_back({Location::Local{paramIdx}, type});
  }

  FTRACE(1, "selectTracelet analyzing callee {} with context:\n{}",
         callee->fullName()->data(), show(ctx));
  auto region = selectTracelet(ctx, inliningDepth() + 1, m_profiling);
  if (!region) {
    return refuse("failed to select region in callee");
  }

  RegionDescIter iter(*region);
  return shouldIRInline(curFunc(), callee, iter);
}

void RegionFormer::truncateLiterals() {
  if (!m_region || m_region->blocks.empty() ||
      m_region->blocks.back()->empty()) return;

  // Don't finish a region with literal values or values that have a class
  // related to the current context class. They produce valuable information
  // for optimizations that's lost across region boundaries.
  auto& lastBlock = *m_region->blocks.back();
  auto sk = lastBlock.start();
  auto endSk = sk;
  auto unit = lastBlock.unit();
  for (int i = 0, len = lastBlock.length(); i < len; ++i, sk.advance(unit)) {
    auto const op = sk.op();
    if (!isLiteral(op) && !isThisSelfOrParent(op)) {
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
  if (ii.dontGuard || !rtt.isValue()) return true;

  if (m_profiling && rtt.isRef() &&
      (m_region->blocks.size() > 1 || !m_region->blocks[0]->empty())) {
    // We don't want side exits when profiling, so only allow instructions that
    // consume refs at the beginning of the region.
    return false;
  }

  if (!ii.dontBreak && !Type(rtt).isKnownDataType()) {
    // Trying to consume a value without a precise enough type.
    FTRACE(1, "selectTracelet: {} tried to consume {}\n",
           m_inst.toString(), m_inst.inputs[i]->pretty());
    return false;
  }

  if (!rtt.isRef() || m_inst.ignoreInnerType || ii.dontGuardInner) {
    return true;
  }

  if (!Type(rtt.innerType()).isKnownDataType()) {
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
  assert(!m_region->blocks.empty());
  auto& frontBlock = *m_region->blocks.front();
  for (auto const& dep : m_refDeps.m_arMap) {
    frontBlock.addReffinessPred(m_startSk, {dep.second.m_mask,
                                            dep.second.m_vals,
                                            dep.first});
  }

  // Relax guards and record the ones that survived.
  auto& firstBlock = *m_region->blocks.front();
  auto blockStart = firstBlock.start();
  auto& unit = m_ht.unit();
  auto const doRelax = RuntimeOption::EvalHHIRRelaxGuards;
  bool changed = false;
  if (doRelax) {
    Timer _t(Timer::selectTracelet_relaxGuards);
    changed = relaxGuards(unit, *m_ht.irBuilder().guards(), m_profiling);
  }

  visitGuards(unit, [&](const RegionDesc::Location& loc, Type type) {
    if (type <= Type::Cls) return;
    RegionDesc::TypePred pred{loc, type};
    FTRACE(1, "selectTracelet adding guard {}\n", show(pred));
    firstBlock.addPredicted(blockStart, pred);
  });
  if (changed) {
    dumpTrace(3, unit, " after guard relaxation ",
              nullptr, nullptr, m_ht.irBuilder().guards());
  }

}
}

/*
 * Region selector that attempts to form the longest possible region using the
 * given context. The region will be broken before the first instruction that
 * attempts to consume an input with an insufficiently precise type, or after
 * most control flow instructions.
 *
 * May return a null region if the given RegionContext doesn't have
 * enough information to translate at least one instruction.
 */
RegionDescPtr selectTracelet(const RegionContext& ctx, int inlineDepth,
                             bool profiling) {
  Timer _t(Timer::selectTracelet);
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 1;

  while (!(region = RegionFormer(ctx, interp, inlineDepth, profiling).go())) {
    ++tries;
  }

  if (region->blocks.size() == 0 || region->blocks.front()->length() == 0) {
    FTRACE(1, "selectTracelet giving up after {} tries\n", tries);
    return RegionDescPtr { nullptr };
  }

  FTRACE(1, "selectTracelet returning, inlineDepth {}, {} tries:\n{}\n",
         inlineDepth, tries, show(*region));
  if (region->blocks.back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->blocks.pop_back();
  }
  return region;
}

} }
