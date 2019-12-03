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
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/assertions.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/hfsort.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#include <vector>

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen {

namespace {

hfsort::TargetGraph
createCallGraph(jit::hash_map<hfsort::TargetId, FuncId>& funcID) {
  BootStats::Block timer("RTA_create_callgraph",
                         RuntimeOption::ServerExecutionMode());
  ProfData::Session pds;
  assertx(profData() != nullptr);

  using hfsort::TargetId;

  hfsort::TargetGraph cg;
  jit::hash_map<FuncId, TargetId> targetID;
  auto pd = profData();

  // Create one node (aka target) for each function that was profiled.
  const auto maxFuncId = pd->maxProfilingFuncId();

  FTRACE(3, "createCallGraph: maxFuncId = {}\n", maxFuncId);
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
    FTRACE(3, "  - adding node FuncId = {} => TargetId = {}\n", fid, targetId);
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
      FTRACE(3, "  - adding arc @ {} : {} => {} [weight = {}] \n",
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

      FTRACE(3, "  - processing ProfPrologue w/ transId = {}\n", transId);
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

void print(hfsort::TargetGraph& /*cg*/, const char* fileName,
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

    print(cg, "/tmp/hotfuncs-pgo.txt", clusters, target2FuncId);
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

}}}
