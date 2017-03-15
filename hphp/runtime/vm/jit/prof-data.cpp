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

#include "hphp/runtime/vm/jit/prof-data.h"

#include <vector>
#include <algorithm>

#include <folly/MapUtil.h>

#include "hphp/util/logger.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/treadmill.h"
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

template<typename Map>
typename Map::Config makeAHMConfig() {
  typename Map::Config config;
  config.growthFactor = 1;
  config.entryCountThreadCacheSize = 10;
  return config;
}

ProfData::ProfData()
  : m_counters(RuntimeOption::ServerExecutionMode()
                 ? std::numeric_limits<int64_t>::max()
                 : RuntimeOption::EvalJitPGOThreshold)
  , m_profilingFuncs(RuntimeOption::EvalFuncCountHint, false)
  , m_optimizedFuncs(RuntimeOption::EvalFuncCountHint, false)
  , m_queuedFuncs(RuntimeOption::EvalFuncCountHint, false)
  , m_optimizedSKs(RuntimeOption::EvalPGOFuncCountHint,
                   makeAHMConfig<decltype(m_optimizedSKs)>())
  , m_proflogueDB(RuntimeOption::EvalPGOFuncCountHint * 2,
                  makeAHMConfig<decltype(m_proflogueDB)>())
  , m_dvFuncletDB(RuntimeOption::EvalPGOFuncCountHint * 2,
                  makeAHMConfig<decltype(m_dvFuncletDB)>())
  , m_jmpToTransID(RuntimeOption::EvalPGOFuncCountHint * 10,
                   makeAHMConfig<decltype(m_jmpToTransID)>())
  , m_blockEndOffsets(RuntimeOption::EvalPGOFuncCountHint,
                      makeAHMConfig<decltype(m_blockEndOffsets)>())
{}

TransID ProfData::allocTransID() {
  WriteLock lock{m_transLock};
  m_transRecs.emplace_back();
  return m_transRecs.size() - 1;
}

TransID ProfData::proflogueTransId(const Func* func, int nArgs) const {
  auto const numParams = func->numNonVariadicParams();
  if (nArgs > numParams) nArgs = numParams + 1;

  return folly::get_default(
    m_proflogueDB,
    PrologueID{func->getFuncId(), nArgs},
    kInvalidTransID
  );
}

TransID ProfData::dvFuncletTransId(SrcKey sk) const {
  return folly::get_default(
    m_dvFuncletDB,
    sk.toAtomicInt(),
    kInvalidTransID
  );
}

void ProfData::addTransProfile(TransID transID,
                               const RegionDescPtr& region,
                               const PostConditions& pconds) {
  auto const lastBcOff = region->lastSrcKey().offset();

  assertx(region);
  DEBUG_ONLY auto const nBlocks = region->blocks().size();
  assertx(nBlocks == 1);
  region->renumberBlock(region->entry()->id(), transID);
  for (auto& b : region->blocks()) b->setProfTransID(transID);
  region->blocks().back()->setPostConds(pconds);
  auto const startSk = region->start();

  // If the translation corresponds to a DV Funclet, then add an entry
  // into dvFuncletDB.
  auto const func = startSk.func();
  auto const funcId = func->getFuncId();
  auto const bcOffset = startSk.offset();

  if (func->isDVEntry(bcOffset)) {
    // Normal DV funclets don't have type guards, and thus have a single
    // translation.  However, some special functions written in hhas
    // (e.g. array_map) have complex DV funclets that get retranslated for
    // different types.  For those functions, m_dvFuncletDB keeps the TransID
    // for their first translation.
    m_dvFuncletDB.emplace(startSk.toAtomicInt(), transID);
  }

  {
    WriteLock lock{m_transLock};
    m_transRecs[transID].reset(new ProfTransRec(lastBcOff, startSk, region));
  }

  // Putting transID in m_funcProfTrans makes it visible to other threads, so
  // this has to happen after we've already put its metadata in m_transRecs.
  WriteLock lock{m_funcProfTransLock};
  m_funcProfTrans[funcId].push_back(transID);
}

void ProfData::addTransProfPrologue(TransID transID, SrcKey sk, int nArgs) {
  m_proflogueDB.emplace(PrologueID{sk.funcID(), nArgs}, transID);

  WriteLock lock{m_transLock};
  m_transRecs[transID].reset(new ProfTransRec(sk, nArgs));
}

bool ProfData::anyBlockEndsAt(const Func* func, Offset offset) {
  auto it = m_blockEndOffsets.find(func->getFuncId());
  if (it == m_blockEndOffsets.end()) {
    Arena arena;
    Verifier::GraphBuilder builder{arena, func};
    auto cfg = builder.build();
    std::unordered_set<Offset> offsets;

    for (auto blocks = linearBlocks(cfg); !blocks.empty(); ) {
      auto last = blocks.popFront()->last - func->unit()->entry();
      offsets.insert(last);
    }

    it = m_blockEndOffsets.emplace(func->getFuncId(), std::move(offsets)).first;
  }

  return it->second.count(offset);
}

////////////////////////////////////////////////////////////////////////////////

namespace {
std::atomic<ProfData*> s_profData{nullptr};
struct ProfDataShutdownDeleter {
  ~ProfDataShutdownDeleter() {
    delete s_profData.load();
  }
} s_profDataShutdownDeleter;

/*
 * Used to free ProfData from the Treadmill. Since we currently do nothing to
 * make Profile translations unreachable when we're otherwise done with
 * ProfData, we move the profiling counters out of ProfData before deleting it.
 */
ProfCounters<int64_t> s_persistentCounters{0};
struct ProfDataTreadmillDeleter {
  void operator()() {
    s_persistentCounters = data->takeCounters();
    if (RuntimeOption::ServerExecutionMode()) {
      Logger::Info("Deleting JIT ProfData");
    }
  }

  std::unique_ptr<ProfData> data;
};
}

__thread ProfData* tl_profData{nullptr};

void processInitProfData() {
  if (!RuntimeOption::EvalJitPGO) return;

  s_profData.store(new ProfData(), std::memory_order_relaxed);
}

void requestInitProfData() {
  tl_profData = s_profData.load(std::memory_order_relaxed);
}

void requestExitProfData() {
  tl_profData = nullptr;
}

const ProfData* globalProfData() {
  return s_profData.load(std::memory_order_relaxed);
}

void discardProfData() {
  if (s_profData.load(std::memory_order_relaxed) == nullptr) return;

  // Make sure s_profData is nullptr so any new requests won't try to use the
  // object we're deleting, then send it to the Treadmill for deletion.
  std::unique_ptr<ProfData> data{
    s_profData.exchange(nullptr, std::memory_order_relaxed)
  };
  if (data != nullptr) {
    if (RuntimeOption::ServerExecutionMode()) {
      Logger::Info("Putting JIT ProfData on Treadmill");
    }
    Treadmill::enqueue(ProfDataTreadmillDeleter{std::move(data)});
  }
}

void ProfData::maybeResetCounters() {
  if (m_countersReset.load(std::memory_order_acquire)) return;
  if (requestCount() < RuntimeOption::EvalJitResetProfCountersRequest) return;

  WriteLock lock{m_transLock};
  if (m_countersReset.load(std::memory_order_relaxed)) return;
  m_counters.resetAllCounters(RuntimeOption::EvalJitPGOThreshold);
  m_countersReset.store(true, std::memory_order_release);
}

void ProfData::addTargetProfile(const ProfData::TargetProfileInfo& info) {
  WriteLock lock{m_targetProfilesLock};
  m_targetProfiles[info.key.transId].push_back(info);
}

std::vector<ProfData::TargetProfileInfo> ProfData::getTargetProfiles(
  TransID transID) const {
  ReadLock lock{m_targetProfilesLock};
  auto it = m_targetProfiles.find(transID);
  if (it != m_targetProfiles.end()) {
    return it->second;
  } else {
    return std::vector<TargetProfileInfo>{};
  }
}

////////////////////////////////////////////////////////////////////////////////

bool hasEnoughProfDataToRetranslateAll() {
  return requestCount() >= RuntimeOption::EvalJitRetranslateAllRequest;
}

////////////////////////////////////////////////////////////////////////////////

}}
