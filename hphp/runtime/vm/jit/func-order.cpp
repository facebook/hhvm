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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/assertions.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/hfsort.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#include <tbb/concurrent_hash_map.h>

#include <vector>

TRACE_SET_MOD(funcorder);

namespace HPHP { namespace jit { namespace FuncOrder {

////////////////////////////////////////////////////////////////////////////////

namespace {

// Cached function order.
std::vector<FuncId> s_funcOrder;

// Map from calls' return address to the the FuncId of the top-level function
// containing that call.
using CallAddrFuncs = tbb::concurrent_hash_map<TCA,FuncId>;
CallAddrFuncs s_callToFuncId;

using FuncPair = std::pair<FuncId,FuncId>;
using CallCounters = tbb::concurrent_hash_map<FuncPair,uint32_t>;
CallCounters s_callCounters;

// Map that keeps track of the size of optimized translations/prologues for each
// function.
using FuncSizes = tbb::concurrent_hash_map<FuncId,uint32_t>;
FuncSizes s_funcSizes;

////////////////////////////////////////////////////////////////////////////////

uint32_t getFuncSize(FuncId funcId) {
  FuncSizes::const_accessor acc;
  return s_funcSizes.find(acc, funcId) ? acc->second : 1; // avoid div by 0
}

hfsort::TargetGraph
createCallGraphFromProfCode(jit::hash_map<hfsort::TargetId, FuncId>& funcID) {
  ProfData::Session pds;
  assertx(profData() != nullptr);

  using hfsort::TargetId;

  hfsort::TargetGraph cg;
  jit::hash_map<FuncId, TargetId> targetID;
  auto pd = profData();

  // Create one node (aka target) for each function that was profiled.
  const auto maxFuncId = pd->maxProfilingFuncId();

  FTRACE(1, "createCallGraph: maxFuncId = {}\n", maxFuncId);
  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!Func::isFuncIdValid(fid) || !pd->profiling(fid)) continue;
    const auto func = Func::fromFuncId(fid);
    const auto baseOffset = func->base();
    const auto transIds = pd->funcProfTransIDs(fid);
    uint32_t size = 1; // avoid zero-sized functions
    uint32_t profCount = 0;
    for (auto transId : transIds) {
      const auto trec = pd->transRec(transId);
      assertx(trec->kind() == TransKind::Profile);
      size += trec->asmSize();
      if (trec->srcKey().offset() == baseOffset) {
        profCount += pd->transCounter(transId);
      }
    }
    // NB: avoid division by 0
    const auto targetId = cg.addTarget(size, profCount ? profCount : 1);
    targetID[fid] = targetId;
    funcID[targetId] = fid;
    FTRACE(1, "  - adding node FuncId = {} => TargetId = {}\n", fid, targetId);
  }

  // Add arcs with weights

  auto addCallerCount = [&] (TCA callAddr,
                             TargetId calleeTargetId,
                             TransID callerTransId,
                             uint64_t& totalCalls) {
    assertx(callerTransId != kInvalidTransID);
    auto const callerFuncId = pd->transRec(callerTransId)->funcId();
    if (!Func::isFuncIdValid(callerFuncId)) return;
    auto const callerTargetId = targetID[callerFuncId];
    auto const callCount = pd->transCounter(callerTransId);
    // Don't create arcs with zero weight
    if (callCount) {
      cg.incArcWeight(callerTargetId, calleeTargetId, callCount);
      totalCalls += callCount;
      FTRACE(1, "  - adding arc @ {} : {} => {} [weight = {}] \n",
             callAddr, callerTargetId, calleeTargetId, callCount);
    }
  };

  auto addCallersCount = [&] (TargetId calleeTargetId,
                              const auto& callerAddrs,
                              uint64_t& totalCalls) {
    for (auto callAddr : callerAddrs) {
      if (!tc::isProfileCodeAddress(callAddr)) continue;
      auto const callerTransId = pd->jmpTransID(callAddr);
      addCallerCount(callAddr, calleeTargetId, callerTransId, totalCalls);
    }
  };

  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!Func::isFuncIdValid(fid) || !pd->profiling(fid)) continue;

    auto func = Func::fromFuncId(fid);
    const auto calleeTargetId = targetID[fid];
    const auto transIds = pd->funcProfTransIDs(fid);
    uint64_t totalCalls = 0;
    for (int nargs = 0; nargs <= func->numNonVariadicParams() + 1; nargs++) {
      auto transId = pd->proflogueTransId(func, nargs);
      if (transId == kInvalidTransID) continue;

      FTRACE(1, "  - processing ProfPrologue w/ transId = {}\n", transId);
      const auto trec = pd->transRec(transId);
      assertx(trec->kind() == TransKind::ProfPrologue);
      auto lock = trec->lockCallerList();
      for (auto const callerTransId : trec->profCallers()) {
        addCallerCount(nullptr, calleeTargetId, callerTransId, totalCalls);
      }
      addCallersCount(calleeTargetId, trec->mainCallers(),  totalCalls);
    }
    auto samples = cg.getSamples(calleeTargetId);
    cg.setSamples(calleeTargetId, std::max(totalCalls, samples));
  }
  cg.normalizeArcWeights();
  return cg;
}

hfsort::TargetGraph
createCallGraphFromOptCode(jit::hash_map<hfsort::TargetId, FuncId>& funcID) {
  using hfsort::TargetId;

  hfsort::TargetGraph cg;
  jit::hash_map<FuncId, TargetId> targetID;

  auto getTargetID = [&] (FuncId fid) {
    auto it = targetID.find(fid);
    if (it != targetID.end()) return it->second;
    auto const funcSize = getFuncSize(fid);
    auto tid = cg.addTarget(funcSize);
    targetID[fid] = tid;
    funcID[tid] = fid;
    return tid;
  };

  // Set Arc weights, adding nodes to the call graph on demand.
  FTRACE(1, "createCallGraphFromOptCode:\n");
  for (auto const& it : s_callCounters) {
    FTRACE(1, "  - weight({} -> {}) = {}\n",
           it.first.first, it.first.second, it.second);
    auto const weight = it.second;
    if (weight == 0) continue; // don't create arcs with zero weight
    auto const callerFid = it.first.first;
    auto const calleeFid = it.first.second;
    auto const callerTid = getTargetID(callerFid);
    auto const calleeTid = getTargetID(calleeFid);
    cg.incArcWeight(callerTid, calleeTid, weight);
    cg.addSamples(calleeTid, weight);
  }

  cg.normalizeArcWeights();
  return cg;
}

hfsort::TargetGraph
createCallGraph(jit::hash_map<hfsort::TargetId, FuncId>& funcID) {
  BootStats::Block timer("RTA_create_callgraph",
                         RuntimeOption::ServerExecutionMode());

  // If we have the call counters collected from optimized code use them to
  // build the call graph; otherwise, use the estimates based on profile code.
  return s_callCounters.empty() ? createCallGraphFromProfCode(funcID)
                                : createCallGraphFromOptCode(funcID);
}

void print(const char* fileName,
           const std::vector<hfsort::Cluster>& clusters,
           jit::hash_map<hfsort::TargetId, FuncId>& target2FuncId) {
  FILE* outfile = fopen(fileName, "wt");
  if (!outfile) return;

  for (auto const& cluster : clusters) {
    fprintf(outfile,
            "-------- density = %.3lf (%u / %u) --------\n",
            (double) cluster.samples / cluster.size,
            cluster.samples, cluster.size);
    for (auto const targetId : cluster.targets) {
      auto funcId = target2FuncId[targetId];
      if (!Func::isFuncIdValid(funcId)) continue;
      fprintf(outfile, "%s\n", Func::fromFuncId(funcId)->fullName()->data());
    }
  }
  fclose(outfile);
}

std::pair<std::vector<FuncId>, uint64_t> hfsortFuncs() {
  // Return ordered function id, as well as the base profile count used by
  // inlining.
  std::pair<std::vector<FuncId>, uint64_t> ret;

  const bool serverMode = RuntimeOption::ServerExecutionMode();
  jit::hash_map<hfsort::TargetId, FuncId> target2FuncId;
  auto cg = createCallGraph(target2FuncId);

  if (RuntimeOption::EvalJitPGODumpCallGraph) {
    Treadmill::Session ts(Treadmill::SessionKind::Retranslate);
    cg.printDot("/tmp/cg-pgo.dot",
                [&](hfsort::TargetId targetId) -> const char* {
                  const auto fid = target2FuncId[targetId];
                  if (!Func::isFuncIdValid(fid)) return "<invalid>";
                  const auto func = Func::fromFuncId(fid);
                  return func->fullName()->data();
                });
    if (serverMode) {
      Logger::Info("retranslateAll: saved call graph at /tmp/cg-pgo.dot");
    }
  }

  auto clusters = hfsort::clusterize(cg);
  sort(clusters.begin(), clusters.end(), hfsort::compareClustersDensity);
  if (serverMode) {
    Logger::Info("retranslateAll: finished clusterizing the functions");
  }

  if (RuntimeOption::EvalJitPGODumpCallGraph) {
    Treadmill::Session ts(Treadmill::SessionKind::Retranslate);

    print("/tmp/hotfuncs-pgo.txt", clusters, target2FuncId);
    if (serverMode) {
      Logger::Info("retranslateAll: saved sorted list of hot functions at "
                   "/tmp/hotfuncs-pgo.txt");
    }
  }

  uint64_t total = 0;
  for (auto const& target : cg.targets) {
    total += target.samples;
  }
  if (cg.targets.size() > 0) {
    ret.second = total / cg.targets.size();
  }

  ret.first.reserve(cg.targets.size());

  auto const addFuncId = [&] (int tid) {
    auto const funcId = target2FuncId[tid];
    if (!Func::isFuncIdValid(funcId)) return;
    ret.first.push_back(funcId);
  };

  jit::fast_set<hfsort::TargetId> seen;
  for (auto& cluster : clusters) {
    for (auto tid : cluster.targets) {
      seen.emplace(tid);
      addFuncId(tid);
    }
  }
  if (auto const extra = cg.targets.size() - seen.size()) {
    Logger::Info("retranslateAll: %lu functions had no samples!", extra);
    for (int i = 0; i < cg.targets.size(); ++i) {
      if (!seen.count(i)) {
        addFuncId(i);
      }
    }
  }
  // assert that there is no duplicate in sortedFuncIds.
  assertx(ret.first.size() ==
          std::set<FuncId>(ret.first.begin(), ret.first.end()).size());
  return ret;
}

////////////////////////////////////////////////////////////////////////////////

}

const std::vector<FuncId>& get() {
  return s_funcOrder;
}

uint64_t compute() {
  auto ret = hfsortFuncs();
  // Append any functions previously in s_funcOrder that are missing in the
  // optimized order, in the same order that s_funcOrder contains them.
  hphp_hash_set<FuncId> optSet;
  optSet.insert(ret.first.begin(), ret.first.end());
  for (auto fid : s_funcOrder) {
    if (optSet.count(fid) == 0) ret.first.push_back(fid);
  }
  s_funcOrder = std::move(ret.first);
  return ret.second;
}

void serialize(ProfDataSerializer& ser) {
  write_raw(ser, safe_cast<uint32_t>(s_funcOrder.size()));
  for (auto const funcId : s_funcOrder) {
    write_raw(ser, funcId);
  }
}

void deserialize(ProfDataDeserializer& des) {
  auto const sz = read_raw<uint32_t>(des);
  s_funcOrder.clear();
  s_funcOrder.reserve(sz);
  for (auto i = sz; i > 0; --i) {
    auto const origId = read_raw<FuncId>(des);
    s_funcOrder.push_back(des.getFid(origId));
  }
}

////////////////////////////////////////////////////////////////////////////////

void setCallFuncId(TCA callRetAddr, FuncId funcId) {
  CallAddrFuncs::accessor acc;
  CallAddrFuncs::value_type val(callRetAddr, funcId);
  if (!s_callToFuncId.insert(acc, val)) {
    always_assert(acc->second == funcId);
  }
}

FuncId getCallFuncId(TCA callRetAddr) {
  CallAddrFuncs::const_accessor acc;
  return s_callToFuncId.find(acc, callRetAddr) ? acc->second : InvalidFuncId;
}

void clearCallFuncId(TCA callRetAddr) {
  s_callToFuncId.erase(callRetAddr);
}

////////////////////////////////////////////////////////////////////////////////

void incCount(const ActRec* fp) {
  auto const callerRip = reinterpret_cast<TCA>(fp->m_savedRip);
  if (callerRip == nullptr) return;
  auto const callee = fp->func()->getFuncId();
  auto const caller = getCallFuncId(callerRip);
  if (caller == InvalidFuncId) {
    // assert callerRip is not in the hot code area, where only optimized code
    // lives
    assert_flog(!tc::isHotCodeAddress(callerRip),
                "callerRip not found: {}", callerRip);
    return;
  }
  auto pair = FuncPair(caller, callee);
  {
    CallCounters::accessor acc;
    if (!s_callCounters.insert(acc, CallCounters::value_type(pair, 1))) {
      acc->second++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void recordTranslation(const TransRec& transRec) {
  auto const kind = transRec.kind;
  if (kind != TransKind::Optimize && kind != TransKind::OptPrologue) return;

  auto const funcId = transRec.src.funcID();
  auto const size = transRec.aLen ? transRec.aLen
                                  : transRec.acoldLen ? transRec.acoldLen
                                                      : transRec.afrozenLen;
  FuncSizes::accessor acc;
  if (!s_funcSizes.insert(acc, FuncSizes::value_type(funcId, size))) {
    acc->second += size;
  }
}

////////////////////////////////////////////////////////////////////////////////

}}}
