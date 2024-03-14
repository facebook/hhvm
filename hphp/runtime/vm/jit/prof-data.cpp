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

#include "hphp/util/configs/jit.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/vasm-block-counters.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/verifier/cfg.h"

namespace HPHP::jit {

TRACE_SET_MOD(pgo);

////////////////////////////////////////////////////////////////////////////////

ProfTransRec::ProfTransRec(SrcKey lastSk, SrcKey sk, RegionDescPtr region,
                           uint32_t asmSize)
    : m_kind(TransKind::Profile)
    , m_asmSize(asmSize)
    , m_lastSk(lastSk)
    , m_sk(sk)
    , m_region(region) {
  assertx(region != nullptr && !region->empty() && region->start() == sk);
}

ProfTransRec::ProfTransRec(SrcKey sk, int nArgs, uint32_t asmSize)
    : m_kind(TransKind::ProfPrologue)
    , m_asmSize(asmSize)
    , m_prologueArgs(nArgs)
    , m_sk(sk)
    , m_callers{}
{
  m_callers = std::make_unique<CallerRec>();
}

ProfTransRec::~ProfTransRec() {
  if (m_kind == TransKind::Profile) {
    m_region.~RegionDescPtr();
    return;
  }
  assertx(m_kind == TransKind::ProfPrologue);
  m_callers.~CallerRecPtr();
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
                 : Cfg::Jit::PGOThreshold)
  , m_profilingFuncs(RuntimeOption::EvalPGOFuncCountHint,
                     makeAHMConfig<decltype(m_profilingFuncs)>())
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
  std::unique_lock lock{m_transLock};
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
  assertx(sk.funcEntry());
  return folly::get_default(
    m_dvFuncletDB,
    sk.toAtomicInt(),
    kInvalidTransID
  );
}

void ProfData::addTransProfile(TransID transID,
                               const RegionDescPtr& region,
                               const PostConditions& pconds,
                               uint32_t asmSize) {
  auto const lastSk = region->lastSrcKey();

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

  if (startSk.funcEntry() && startSk.entryOffset() != 0) {
    assertx(func->isDVEntry(startSk.entryOffset()));
    // Normal DV funclets don't have type guards, and thus have a single
    // translation.  However, some special functions written in hhas
    // (e.g. array_map) have complex DV funclets that get retranslated for
    // different types.  For those functions, m_dvFuncletDB keeps the TransID
    // for their first translation.
    m_dvFuncletDB.emplace(startSk.toAtomicInt(), transID);
  }

  {
    std::unique_lock lock{m_transLock};
    m_transRecs[transID].reset(new ProfTransRec(lastSk, startSk, region,
                                                asmSize));
  }

  // Putting transID in m_funcProfTrans makes it visible to other threads, so
  // this has to happen after we've already put its metadata in m_transRecs.
  std::unique_lock lock{m_funcProfTransLock};
  m_funcProfTrans[funcId].push_back(transID);
}

void ProfData::addTransProfPrologue(TransID transID, SrcKey sk, int nArgs,
                                    uint32_t asmSize) {
  m_proflogueDB.emplace(PrologueID{sk.funcID(), nArgs}, transID);

  std::unique_lock lock{m_transLock};
  m_transRecs[transID].reset(new ProfTransRec(sk, nArgs, asmSize));
}

void ProfData::addProfTrans(TransID transID,
                            std::unique_ptr<ProfTransRec> ptr) {
  assertx(transID >= m_transRecs.size());
  if (transID > m_transRecs.size()) m_transRecs.resize(transID);
  auto const sk = ptr->srcKey();
  if (ptr->kind() == TransKind::Profile) {
    if (sk.funcEntry() && sk.entryOffset() != 0) {
      assertx(sk.func()->isDVEntry(sk.entryOffset()));
      m_dvFuncletDB.emplace(sk.toAtomicInt(), transID);
    }
    m_funcProfTrans[sk.funcID()].push_back(transID);
  } else {
    m_proflogueDB.emplace(PrologueID{sk.funcID(), ptr->prologueArgs()},
                          transID);
  }
  m_transRecs.emplace_back(std::move(ptr));
}

bool ProfData::anyBlockEndsAt(const Func* func, Offset offset) {
  auto it = m_blockEndOffsets.find(func->getFuncId().toInt());
  if (it == m_blockEndOffsets.end()) {
    Arena arena;
    Verifier::GraphBuilder builder{arena, func};
    auto cfg = builder.build();
    jit::fast_set<Offset> offsets;

    for (auto blocks = linearBlocks(cfg); !blocks.empty(); ) {
      auto last = blocks.popFront()->last - func->entry();
      offsets.insert(last);
    }

    it = m_blockEndOffsets.emplace(func->getFuncId().toInt(),
                                   std::move(offsets)).first;
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
 * Used to free ProfData from the Treadmill.
 */
struct ProfDataTreadmillDeleter {
  void operator()() {
    if (RuntimeOption::ServerExecutionMode()) {
      Logger::Info("Deleting JIT ProfData");
    }
  }

  std::unique_ptr<ProfData> data;
};
}

std::atomic_bool ProfData::s_triedDeserialization{false};
std::atomic_bool ProfData::s_wasDeserialized{false};
std::atomic<StringData*> ProfData::s_buildHost{nullptr};
std::atomic<StringData*> ProfData::s_tag{nullptr};
std::atomic<int64_t> ProfData::s_buildTime{0};
std::atomic<size_t> ProfData::s_prevProfSize{0};

ServiceData::ExportedCounter* ProfData::s_optimized_funcs_counter =
  ServiceData::createCounter("jit.optimized_funcs");
ServiceData::ExportedCounter* ProfData::s_tried_deserialze =
  ServiceData::createCounter("jit.tried_deser");
ServiceData::ExportedCounter* ProfData::s_deserialize_succ =
  ServiceData::createCounter("jit.succeeded_deser");

RDS_LOCAL_NO_CHECK(ProfData*, rl_profData)(nullptr);

void processInitProfData() {
  if (!Cfg::Jit::PGO) return;

  s_profData.store(new ProfData(), std::memory_order_relaxed);
}

void requestInitProfData() {
  *rl_profData = s_profData.load(std::memory_order_relaxed);
}

void requestExitProfData() {
  *rl_profData = nullptr;
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
  Treadmill::enqueue(VasmBlockCounters::free);
}

void ProfData::maybeResetCounters() {
  if (m_countersReset.load(std::memory_order_acquire)) return;
  if (requestCount() < Cfg::Jit::ResetProfCountersRequest) return;

  std::unique_lock lock{m_transLock};
  if (m_countersReset.load(std::memory_order_relaxed)) return;
  m_counters.resetAllCounters(Cfg::Jit::PGOThreshold);
  m_countersReset.store(true, std::memory_order_release);
}

void ProfData::addTargetProfile(const ProfData::TargetProfileInfo& info) {
  std::unique_lock lock{m_targetProfilesLock};
  m_targetProfiles[info.key.transId].push_back(info);
}

std::vector<ProfData::TargetProfileInfo> ProfData::getTargetProfiles(
  TransID transID) const {
  std::shared_lock lock{m_targetProfilesLock};
  auto it = m_targetProfiles.find(transID);
  if (it != m_targetProfiles.end()) {
    return it->second;
  } else {
    return std::vector<TargetProfileInfo>{};
  }
}

////////////////////////////////////////////////////////////////////////////////

}
