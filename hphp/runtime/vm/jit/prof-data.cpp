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

#include "hphp/runtime/vm/jit/prof-data.h"

#include <vector>
#include <algorithm>

#include <folly/MapUtil.h>

#include "hphp/util/logger.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/verifier/cfg.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

////////////////////////////////////////////////////////////////////////////////

ProfTransRec::ProfTransRec(Offset lastBcOff, SrcKey sk, RegionDescPtr region)
    : m_kind(TransKind::Profile)
    , m_lastBcOff(lastBcOff)
    , m_sk(sk)
    , m_region(region) {
  assertx(region != nullptr && !region->empty() && region->start() == sk);
}

ProfTransRec::ProfTransRec(SrcKey sk, int nArgs)
    : m_kind(TransKind::ProfPrologue)
    , m_prologueArgs(nArgs)
    , m_sk(sk)
    , m_callers()
{}

ProfTransRec::~ProfTransRec() {
  if (m_kind == TransKind::Profile) {
    m_region.~RegionDescPtr();
    return;
  }
  assertx(m_kind == TransKind::ProfPrologue);
  m_callers.~CallerRec();
}

////////////////////////////////////////////////////////////////////////////////

ProfData::ProfData() :
  m_counters(RuntimeOption::ServerExecutionMode()
               ? std::numeric_limits<int64_t>::max()
               : RuntimeOption::EvalJitPGOThreshold)
{ }

TransID ProfData::proflogueTransId(const Func* func, int nArgs) const {
  auto const numParams = func->numNonVariadicParams();
  if (nArgs > numParams) nArgs = numParams + 1;
  auto const funcId = func->getFuncId();
  return folly::get_default(
    m_proflogueDB,
    std::make_tuple(funcId, nArgs),
    kInvalidTransID);
}

TransID ProfData::dvFuncletTransId(const Func* func, int nArgs) const {
  return folly::get_default(
    m_dvFuncletDB,
    std::make_tuple(func->getFuncId(), nArgs),
    kInvalidTransID);
}

void ProfData::setProfiling(FuncId funcId) {
  m_profilingFuncs.insert(funcId);
  if (m_funcProfTrans.find(funcId) == m_funcProfTrans.end()) {
    m_funcProfTrans[funcId] = TransIDVec();
  }
}

void ProfData::addTransProfile(const RegionDescPtr& region,
                               const PostConditions& pconds) {
  TransID transId = m_numTrans++;
  auto const lastBcOff = region->lastSrcKey().offset();

  assertx(region);
  DEBUG_ONLY auto const nBlocks = region->blocks().size();
  assertx(nBlocks == 1);
  region->renumberBlock(region->entry()->id(), transId);
  for (auto& b : region->blocks()) b->setProfTransID(transId);
  region->blocks().back()->setPostConds(pconds);
  auto const startSk = region->start();
  m_transRecs.emplace_back(new ProfTransRec(lastBcOff, startSk, region));

  // If the translation corresponds to a DV Funclet, then add an entry
  // into dvFuncletDB.
  auto const func = startSk.func();
  auto const funcId = func->getFuncId();
  auto const bcOffset = startSk.offset();
  if (func->isDVEntry(bcOffset)) {
    auto const nParams = func->getDVEntryNumParams(bcOffset);
    // Normal DV funclets don't have type guards, and thus have a
    // single translation.  However, some special functions written
    // in hhas (e.g. array_map) have complex DV funclets that get
    // retranslated for different types.  For those functions,
    // m_dvFuncletDB keeps the TransID for their first translation.
    if (!m_dvFuncletDB.count(std::make_tuple(funcId, nParams))) {
      m_dvFuncletDB.emplace(std::make_tuple(funcId, nParams), transId);
    }
  }

  m_funcProfTrans[funcId].push_back(transId);
}

void ProfData::addTransNonProf() {
  if (Translator::isTransDBEnabled()) {
    m_numTrans++;
    m_transRecs.emplace_back(nullptr);
  }
}

TransID ProfData::addTransProflogue(SrcKey sk, int nArgs) {
  auto transId = m_numTrans++;
  m_transRecs.emplace_back(new ProfTransRec(sk, nArgs));
  m_proflogueDB.emplace(std::make_tuple(sk.funcID(), nArgs), transId);
  return transId;
}

void ProfData::freeFuncData(FuncId funcId) {
  // Free ProfTransRecs for Profile translations.
  for (auto tid : funcProfTransIDs(funcId)) {
    m_transRecs[tid].reset();
  }

  // Free ProfTransRecs for Proflogue translations.
  const Func* func = Func::fromFuncId(funcId);
  for (int nArgs = 0; nArgs < func->numPrologues(); nArgs++) {
    auto tid = proflogueTransId(func, nArgs);
    if (tid != kInvalidTransID) {
      m_transRecs[tid].reset();
    }
  }

  // We don't need the cached block offsets anymore.  They are only used when
  // generating profiling translations.
  m_blockEndOffsets.erase(funcId);
}

void ProfData::free() {
  if (m_freed) return;
  m_freed = true;
  Logger::Info("Freeing JIT profiling data");
  for (auto& trec : m_transRecs) {
    trec.reset();
  }
  m_blockEndOffsets.clear();
}

bool ProfData::anyBlockEndsAt(const Func* func, Offset offset) {
  auto const mapIt = m_blockEndOffsets.find(func->getFuncId());
  if (mapIt != end(m_blockEndOffsets)) {
    return mapIt->second.count(offset);
  }

  using namespace Verifier;

  Arena arena;
  GraphBuilder builder{arena, func};
  auto cfg = builder.build();
  auto& offsets = m_blockEndOffsets[func->getFuncId()];

  for (LinearBlocks blocks = linearBlocks(cfg); !blocks.empty(); ) {
    auto last = blocks.popFront()->last - func->unit()->entry();
    offsets.insert(last);
  }

  return offsets.count(offset);
}

void ProfData::maybeResetCounters() {
  if (m_countersReset) return;
  if (requestCount() < RuntimeOption::EvalJitResetProfCountersRequest) return;

  BlockingLeaseHolder writer(Translator::WriteLease());
  assert(writer.canWrite());
  if (m_countersReset) return;
  m_counters.resetAllCounters(RuntimeOption::EvalJitPGOThreshold);
  m_countersReset = true;
}

////////////////////////////////////////////////////////////////////////////////
}}
