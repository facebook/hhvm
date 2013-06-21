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
#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace JIT {

using Transl::DynLocation;
using Transl::ActRecState;
using Transl::RefDeps;

TRACE_SET_MOD(region);

typedef hphp_hash_set<SrcKey, SrcKey::Hasher> InterpSet;

namespace {
struct RegionFormer {
  RegionFormer(const RegionContext& ctx, InterpSet& interp);

  RegionDescPtr go();

private:
  const RegionContext& m_ctx;
  InterpSet& m_interp;
  const Func* m_curFunc;
  const Unit* m_curUnit;
  SrcKey m_sk;
  const SrcKey m_startSk;
  NormalizedInstruction m_inst;
  RegionDescPtr m_region;
  RegionDesc::Block* m_curBlock;
  bool m_blockFinished;
  int m_pendingLiterals;
  IRTranslator m_irTrans;
  HhbcTranslator& m_ht;
  Unit::MetaHandle m_metaHand;
  ActRecState m_arState;
  RefDeps m_refDeps;

  bool prepareInstruction();
  void addInstruction();
};

RegionFormer::RegionFormer(const RegionContext& ctx, InterpSet& interp)
  : m_ctx(ctx)
  , m_interp(interp)
  , m_curFunc(ctx.func)
  , m_curUnit(m_curFunc->unit())
  , m_sk(m_curFunc, ctx.bcOffset)
  , m_startSk(m_sk)
  , m_region(smart::make_unique<RegionDesc>())
  , m_curBlock(m_region->addBlock(m_curFunc, m_sk.offset(), 0))
  , m_blockFinished(false)
  , m_pendingLiterals(0)
  , m_irTrans(ctx.bcOffset, ctx.spOffset, ctx.func)
  , m_ht(m_irTrans.hhbcTrans())
{
}

RegionDescPtr RegionFormer::go() {
  uint32_t numJmps = 0;
  for (auto const& lt : m_ctx.liveTypes) {
    auto t = lt.type;
    if (t.strictSubtypeOf(Type::Obj)) t = t.unspecialize();

    if (t.subtypeOf(Type::Cls)) {
      m_ht.assertTypeStack(lt.location.stackOffset(), t);
    } else {
      m_ht.guardTypeLocation(lt.location, t);
    }
    m_curBlock->addPredicted(m_sk, RegionDesc::TypePred{lt.location, t});
  }

  while (true) {
    if (!prepareInstruction()) break;
    Transl::annotate(&m_inst);

    // Before doing the translation, check for tracelet-ending control flow.
    if (m_inst.op() == OpJmp && m_inst.imm[0].u_BA > 0 &&
        numJmps < Transl::Translator::MaxJmpsTracedThrough) {
      // Include the Jmp in the region and continue to its destination.
      ++numJmps;
      m_sk.setOffset(m_sk.offset() + m_inst.imm[0].u_BA);
      m_blockFinished = true;

      m_ht.setBcOff(m_sk.offset(), false);
      continue;
    } else if (Transl::opcodeBreaksBB(m_inst.op()) ||
               (Transl::dontGuardAnyInputs(m_inst.op()) &&
                Transl::opcodeChangesPC(m_inst.op()))) {
      // This instruction ends the tracelet.
      break;
    }

    m_inst.interp = m_interp.count(m_sk);
    auto const doPrediction = Transl::outputIsPredicted(m_startSk, m_inst);

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

    if (isFCallStar(m_inst.op())) m_arState.pop();

    // Advance sk and check the prediction, if any.
    m_sk.advance(m_curBlock->unit());
    if (doPrediction) m_ht.checkTypeStack(0, m_inst.outPred, m_sk.offset());
  }

  if (m_region) {
    // Record the incrementally constructed reffiness predictions.
    assert(!m_region->blocks.empty());
    auto& frontBlock = *m_region->blocks.front();
    for (auto const& dep : m_refDeps.m_arMap) {
      frontBlock.addReffinessPred(m_startSk, {dep.second.m_mask,
                                              dep.second.m_vals,
                                              dep.first});
    }
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
  m_inst.m_unit = m_curUnit;
  m_inst.breaksTracelet = false;
  m_inst.changesPC = Transl::opcodeChangesPC(m_inst.op());
  m_inst.funcd = m_arState.knownFunc();
  Transl::populateImmediates(m_inst);
  Transl::preInputApplyMetaData(m_metaHand, &m_inst);

  Transl::InputInfos inputInfos;
  getInputs(m_startSk, m_inst, inputInfos, m_curBlock->func(), [&](int i) {
    return m_ht.traceBuilder()->getLocalType(i);
  });

  // Read types for all the inputs and apply MetaData.
  auto newDynLoc = [&](const Transl::InputInfo& ii) {
    auto dl = m_inst.newDynLoc(ii.loc, m_ht.rttFromLocation(ii.loc));
    FTRACE(2, "rttFromLocation: {} -> {}\n",
           ii.loc.pretty(), dl->rtt.pretty());
    return dl;
  };

  for (auto const& ii : inputInfos) m_inst.inputs.push_back(newDynLoc(ii));
  readMetaData(m_metaHand, m_inst, m_ht);

  // Check all the inputs for unknown values.
  assert(inputInfos.size() == m_inst.inputs.size());
  for (unsigned i = 0; i < inputInfos.size(); ++i) {
    auto const& ii = inputInfos[i];
    auto const& rtt = m_inst.inputs[i]->rtt;

    if (ii.dontBreak || ii.dontGuard) continue;
    if (rtt.isVagueValue()) {
      // Trying to consume a value without a precise enough type.
      return false;
    }
    if (m_inst.ignoreInnerType || ii.dontGuardInner) continue;
    if (rtt.isValue() && rtt.isRef() &&
        (rtt.innerType() == KindOfInvalid || rtt.innerType() == KindOfAny)) {
      // Trying to consume a boxed value without a guess for the inner type.
      return false;
    }
  }

  if (!m_inst.noOp && inputInfos.needsRefCheck) {
    // Reffiness guards are always at the beginning of the trace for now, so
    // calculate the delta from the original sp to the ar.
    auto argNum = m_inst.imm[0].u_IVA;
    size_t entryArDelta = instrSpToArDelta((Op*)m_inst.pc()) -
      (m_ht.spOffset() - m_ctx.spOffset);
    try {
      m_inst.preppedByRef = m_arState.checkByRef(argNum, entryArDelta,
                                               &m_refDeps);
    } catch (const Transl::UnknownInputExc& exn) {
      // We don't have a guess for the current ActRec.
      return false;
    }
    addInstruction();
    m_curBlock->setParamByRef(m_inst.source, m_inst.preppedByRef);
  } else {
    addInstruction();
  }

  if (isFPush(m_inst.op())) m_arState.pushFunc(m_inst);

  return true;
}

/*
 * Add the current instruction to the region. Instructions that push constant
 * values aren't pushed unless more instructions come after them.
 */
void RegionFormer::addInstruction() {
  if (m_blockFinished) {
    m_curBlock = m_region->addBlock(m_curFunc, m_inst.source.offset(), 0);
    m_blockFinished = false;
  }

  auto op = m_curUnit->getOpcode(m_inst.source.offset());
  if (isLiteral(op) || isThisSelfOrParent(op)) {
    // Don't finish a region with literal values or values that have a class
    // related to the current context class. They produce valuable information
    // for optimizations that's lost across region boundaries.
    ++m_pendingLiterals;
  } else {
    // This op isn't a literal so add any that are pending before the current
    // instruction.
    for (; m_pendingLiterals; --m_pendingLiterals) {
      m_curBlock->addInstruction();
    }
    m_curBlock->addInstruction();
  }
}
}

/*
 * Region selector that attempts to form the longest possible region using the
 * given context. The region will be broken before the first instruction that
 * attempts to consume an input with an insufficiently precise type.
 *
 * Always returns a RegionDesc containing at least one instruction.
 */
RegionDescPtr selectTracelet(const RegionContext& ctx) {
  InterpSet interp;
  RegionDescPtr region;
  uint32_t tries = 1;

  while (!(region = RegionFormer(ctx, interp).go())) {
    ++tries;
  }
  FTRACE(1, "regionTracelet returning after {} tries:\n{}\n",
         tries, show(*region));

  assert(region->blocks.size() > 0 && region->blocks.front()->length() > 0);
  if (region->blocks.back()->length() == 0) {
    // If the final block is empty because it would've only contained
    // instructions producing literal values, kill it.
    region->blocks.pop_back();
  }
  return region;
}

} }
