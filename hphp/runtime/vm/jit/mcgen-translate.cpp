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

#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/mcgen-prologue.h"

#include "hphp/runtime/vm/jit/mcgen.h"

#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/base/program-functions.h"

#include "hphp/util/hfsort.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen {

namespace {

std::thread s_retranslateAllThread;
std::atomic<bool> s_retranslateAllComplete{false};

void optimize(tc::FuncMetaInfo& info) {
  auto const func = info.func;

  folly::Optional<Trace::BumpRelease> bumpLoads;
  folly::Optional<Trace::BumpRelease> bumpStores;
  folly::Optional<Trace::BumpRelease> bumpPrint;
  if (!RuntimeOption::TraceFunctions.empty() &&
      RuntimeOption::TraceFunctions.count(func->fullName()->toCppString())) {
    bumpLoads.emplace(Trace::hhir_load, -10);
    bumpStores.emplace(Trace::hhir_store, -10);
    bumpPrint.emplace(Trace::printir, -10);
  }

  // Regenerate the prologues and DV funclets before the actual function body.
  auto const includedBody = regeneratePrologues(func, info);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = includedBody ? std::vector<RegionDescPtr>{}
                                    : regionizeFunc(func, transCFGAnnot);

  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto transArgs = TransArgs{regionSk};
    if (transCFGAnnot.size() > 0) {
      transArgs.annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    transArgs.region = region;
    transArgs.kind = TransKind::Optimize;

    auto const spOff = region->entry()->initialSpOffset();
    auto data = translate(transArgs, spOff, info.tcBuf.view());
    if (data) {
      info.translations.emplace_back(std::move(*data));
      transCFGAnnot = ""; // so we don't annotate it again
    }
  }
}

struct OptimizeData {
  FuncId id;
  tc::FuncMetaInfo info;
};

struct TranslateWorker : JobQueueWorker<OptimizeData*, void*, true, true> {
  void doJob(OptimizeData* d) override {
    hphp_session_init();
    SCOPE_EXIT {
      hphp_context_exit();
      hphp_session_exit();
    };

    // Check if the func was treadmilled before the job started
    if (!Func::isFuncIdValid(d->id)) return;

    VMProtect _;

    if (profData()->optimized(d->id)) return;
    profData()->setOptimized(d->id);

    optimize(d->info);
  }
};

using WorkerDispatcher = JobQueueDispatcher<TranslateWorker>;
std::atomic<WorkerDispatcher*> s_dispatcher;
std::mutex s_dispatcherMutex;

WorkerDispatcher& dispatcher() {
  if (auto ptr = s_dispatcher.load(std::memory_order_acquire)) return *ptr;

  auto dispatcher = new WorkerDispatcher(
    RuntimeOption::EvalJitWorkerThreads, 0, false, nullptr
  );
  dispatcher->start();
  s_dispatcher.store(dispatcher, std::memory_order_release);
  return *dispatcher;
}

void enqueueRetranslateOptRequest(OptimizeData* d) {
  dispatcher().enqueue(d);
}

hfsort::TargetGraph
createCallGraph(jit::hash_map<hfsort::TargetId, FuncId>& funcID) {
  ProfData::Session pds;
  assertx(profData() != nullptr);

  using namespace hfsort;
  TargetGraph cg;
  jit::hash_map<FuncId, TargetId> targetID;
  auto pd = profData();

  // Create one node (aka target) for each function that was profiled.
  const auto maxFuncId = pd->maxProfilingFuncId();

  FTRACE(3, "createCallGraph: maxFuncId = {}\n", maxFuncId);
  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!Func::isFuncIdValid(fid) || !pd->profiling(fid)) continue;
    const auto transIds = pd->funcProfTransIDs(fid);
    uint32_t size = 1; // avoid zero-sized functions
    for (auto transId : transIds) {
      const auto trec = pd->transRec(transId);
      assertx(trec->kind() == TransKind::Profile);
      // GO: TODO: maybe save the size of the machine code for prof
      //           translations and use it here
      size += trec->lastBcOff() - trec->startBcOff();
    }
    const auto targetId = cg.addTarget(size);
    targetID[fid] = targetId;
    funcID[targetId] = fid;
    FTRACE(3, "  - adding node FuncId = {} => TargetId = {}\n", fid, targetId);
  }

  // Add arcs with weights

  auto addCallersCount = [&](TargetId calleeTargetId,
                             const std::vector<TCA>& callerAddrs,
                             uint32_t& totalCalls) {
    for (auto callAddr : callerAddrs) {
      if (!tc::isProfileCodeAddress(callAddr)) continue;
      const auto callerTransId = pd->jmpTransID(callAddr);
      assertx(callerTransId != kInvalidTransID);
      const auto callerFuncId = pd->transRec(callerTransId)->funcId();
      if (!Func::isFuncIdValid(callerFuncId)) continue;
      const auto callerTargetId = targetID[callerFuncId];
      const auto callCount = pd->transCounter(callerTransId);
      // Don't create arcs with zero weight
      if (callCount) {
        cg.incArcWeight(callerTargetId, calleeTargetId, callCount);
        totalCalls += callCount;
        FTRACE(3, "  - adding arc @ {} : {} => {} [weight = {}] \n",
               callAddr, callerTargetId, calleeTargetId, callCount);
      }
    }
  };

  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!Func::isFuncIdValid(fid) || !pd->profiling(fid)) continue;

    auto func = Func::fromFuncId(fid);
    const auto calleeTargetId = targetID[fid];
    const auto transIds = pd->funcProfTransIDs(fid);
    uint32_t totalCalls = 0;
    uint32_t profCount  = 1; // avoid zero sample counts
    for (int nargs = 0; nargs <= func->numNonVariadicParams() + 1; nargs++) {
      auto transId = pd->proflogueTransId(func, nargs);
      if (transId == kInvalidTransID) continue;

      FTRACE(3, "  - processing ProfPrologue w/ transId = {}\n", transId);
      const auto trec = pd->transRec(transId);
      assertx(trec->kind() == TransKind::ProfPrologue);
      auto lock = trec->lockCallerList();
      addCallersCount(calleeTargetId, trec->mainCallers(),  totalCalls);
      addCallersCount(calleeTargetId, trec->guardCallers(), totalCalls);
      profCount += pd->transCounter(transId);
    }
    cg.setSamples(calleeTargetId, std::max(totalCalls, profCount));
  }
  cg.normalizeArcWeights();
  return cg;
}

void print(hfsort::TargetGraph& /*cg*/, const char* fileName,
           const std::vector<hfsort::Cluster>& clusters,
           jit::hash_map<hfsort::TargetId, FuncId>& target2FuncId) {
  FILE* outfile = fopen(fileName, "wt");
  if (!outfile) return;

  for (auto& cluster : clusters) {
    fprintf(outfile,
            "-------- density = %.3lf (%u / %u) --------\n",
            (double) cluster.samples / cluster.size,
            cluster.samples, cluster.size);
    for (auto targetId : cluster.targets) {
      auto funcId = target2FuncId[targetId];
      if (!Func::isFuncIdValid(funcId)) continue;
      fprintf(outfile, "%s\n", Func::fromFuncId(funcId)->fullName()->data());
    }
  }
  fclose(outfile);
}

/*
 * This is the main driver for the profile-guided retranslation of all the
 * functions being PGO'd, which enables controlling the order in which the
 * Optimize translations are emitted in the TC.
 *
 * There are 4 main steps in this process:
 *   1) Build a call graph for all the profiled functions.
 *   2) Generate machine code for each of the functions.
 *   3) Select an order for the functions (hfsort to sort the functions).
 *   4) Relocate the functions in the TC according to the selected order.
 */
void retranslateAll() {
  const bool serverMode = RuntimeOption::ServerExecutionMode();

  // 1) Create the call graph

  if (serverMode) {
    Logger::Info("retranslateAll: starting to build the call graph");
  }

  jit::hash_map<hfsort::TargetId, FuncId> target2FuncId;
  auto cg = createCallGraph(target2FuncId);

  if (serverMode) {
    Logger::Info("retranslateAll: finished building the call graph");
  }
  if (RuntimeOption::EvalJitPGODumpCallGraph) {
    Treadmill::Session ts;

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

  // 2) Generate machine code for all the profiled functions.

  auto const initialSize = 512;
  auto const ntargets = cg.targets.size();
  std::vector<OptimizeData> jobs;
  jobs.reserve(ntargets);
  std::unique_ptr<uint8_t[]> codeBuffer(new uint8_t[ntargets * initialSize]);

  {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};

    {
      Treadmill::Session session;
      auto bufp = codeBuffer.get();
      for (int tid = 0; tid < cg.targets.size(); ++tid, bufp += initialSize) {
        auto const fid = target2FuncId[tid];
        auto const func = const_cast<Func*>(Func::fromFuncId(fid));
        jobs.emplace_back(OptimizeData{
          fid,
          tc::FuncMetaInfo(func, tc::LocalTCBuffer(bufp, initialSize))
        });
        enqueueRetranslateOptRequest(&jobs.back());
      }
    }

    dispatcher().waitEmpty();
  }

  if (serverMode) {
    Logger::Info("retranslateAll: finished optimizing functions");
  }

  // 3) Pass the call graph to hfsort to obtain the order in which things
  //    should be placed in code.hot

  auto clusters = hfsort::clusterize(cg);
  sort(clusters.begin(), clusters.end(), hfsort::compareClustersDensity);

  if (serverMode) {
    Logger::Info("retranslateAll: finished clusterizing the functions");
  }

  if (RuntimeOption::EvalJitPGODumpCallGraph) {
    Treadmill::Session ts;

    print(cg, "/tmp/hotfuncs-pgo.txt", clusters, target2FuncId);
    if (serverMode) {
      Logger::Info("retranslateAll: saved sorted list of hot functions at "
                   "/tmp/hotfuncs-pgo.dot");
    }
  }

  // 4) Relocate the machine code into code.hot in the desired order

  std::unordered_set<hfsort::TargetId> seen;
  std::vector<tc::FuncMetaInfo> infos;
  infos.reserve(ntargets);

  for (auto& cluster : clusters) {
    for (auto tid : cluster.targets) {
      seen.emplace(tid);
      infos.emplace_back(std::move(jobs[tid].info));
    }
  }

  for (int i = 0; i < ntargets; ++i) {
    if (!seen.count(i)) {
      infos.emplace_back(std::move(jobs[i].info));
    }
  }

  if (auto const extra = ntargets - seen.size()) {
    Logger::Info("retranslateAll: %lu functions had no samples!", extra);
  }

  tc::publishSortedOptFunctions(std::move(infos));

  s_retranslateAllComplete.store(true, std::memory_order_release);

  if (serverMode) {
    ProfData::Session pds;

    Logger::Info("retranslateAll: finished retranslating all optimized "
                 "translations!");
    // The ReusableTC mode assumes that ProfData is never freed, so don't
    // discard ProfData in this mode.
    if (!RuntimeOption::EvalEnableReusableTC) {
      discardProfData();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
}

void joinWorkerThreads() {
  if (s_dispatcher.load(std::memory_order_acquire)) {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};
    if (auto dispatcher = s_dispatcher.load(std::memory_order_acquire)) {
      dispatcher->stop();
    }
  }

  if (s_retranslateAllThread.joinable()) {
    s_retranslateAllThread.join();
  }
}

folly::Optional<tc::TransMetaInfo>
translate(TransArgs args, FPInvOffset spOff,
          folly::Optional<CodeCache::View> optView) {
  INC_TPC(translate);
  assert(args.kind != TransKind::Invalid);

  if (!tc::shouldTranslate(args.sk.func(), args.kind)) return folly::none;

  Timer timer(Timer::mcg_translate);

  auto const srcRec = tc::findSrcRec(args.sk);
  always_assert(srcRec);

  TransEnv env{args};
  env.initSpOffset = spOff;
  env.annotations.insert(env.annotations.end(),
                         args.annotations.begin(), args.annotations.end());

  // Lower the RegionDesc to an IRUnit, then lower that to a Vunit.
  if (args.region &&
      !tc::reachedTranslationLimit(args.kind, args.sk, *srcRec)) {
    if (args.kind == TransKind::Profile || (profData() && transdb::enabled())) {
      env.transID = profData()->allocTransID();
    }
    auto const transContext =
      TransContext{env.transID, args.kind, args.flags, args.sk,
                   env.initSpOffset};

    env.unit = irGenRegion(*args.region, transContext,
                           env.pconds, env.annotations);
    if (env.unit) {
      env.vunit = irlower::lowerUnit(*env.unit);
    }
  }

  timer.stop();
  return tc::emitTranslation(std::move(env), optView);
}

TCA retranslate(TransArgs args, const RegionContext& ctx) {
  VMProtect _;

  auto sr = tc::findSrcRec(args.sk);
  auto const initialNumTrans = sr->numTrans();

  if (isDebuggerAttachedProcess() && isSrcKeyInDbgBL(args.sk)) {
    // We are about to translate something known to be blacklisted by
    // debugger, exit early
    SKTRACE(1, args.sk, "retranslate abort due to debugger\n");
    return nullptr;
  }

  // We need to recompute the kind after acquiring the write lease in case the
  // answer to profileFunc() changes, so use a lambda rather than just
  // storing the result.
  auto kind = [&] {
    return tc::profileFunc(args.sk.func()) ?
      TransKind::Profile : TransKind::Live;
  };
  args.kind = kind();

  // Only start profiling new functions at their entry point. This reduces the
  // chances of profiling the body of a function but not its entry (where we
  // trigger retranslation) and helps remove bias towards larger functions that
  // can cause variations in the size of code.prof.
  if (args.kind == TransKind::Profile &&
      !profData()->profiling(args.sk.funcID()) &&
      !args.sk.func()->isEntry(args.sk.offset())) {
    return nullptr;
  }

  // Don't create live translations of functions that are being optimized. This
  // prevents a race where retranslate may attempt to form a live translation of
  // args.func while retranslateAll is still optimizing, which will either fail
  // the assert below (if the SrcRec has not been cleared and max profile trans
  // is larger than max trans) or emit a live translation in the retranslation
  // chain ahead of the optimized translations.
  if (retranslateAllPending() && args.kind != TransKind::Profile &&
      profData() &&
      (profData()->profiling(args.sk.funcID()) ||
       profData()->optimized(args.sk.funcID()))) {
    return nullptr;
  }

  LeaseHolder writer(args.sk.func(), args.kind);
  if (!writer || !tc::shouldTranslate(args.sk.func(), kind())) {
    return nullptr;
  }

  const auto numTrans = sr->numTrans();
  if (numTrans != initialNumTrans) return sr->getTopTranslation();
  if (args.kind == TransKind::Profile) {
    if (numTrans > RuntimeOption::EvalJitMaxProfileTranslations) {
      always_assert(numTrans ==
                    RuntimeOption::EvalJitMaxProfileTranslations + 1);
      return sr->getTopTranslation();
    }
  } else if (numTrans > RuntimeOption::EvalJitMaxTranslations) {
    always_assert(numTrans == RuntimeOption::EvalJitMaxTranslations + 1);
    return sr->getTopTranslation();
  }
  SKTRACE(1, args.sk, "retranslate\n");

  args.kind = kind();
  if (!writer.checkKind(args.kind)) return nullptr;

  args.region = selectRegion(ctx, args.kind);
  auto data = translate(args, ctx.spOffset);

  TCA result = nullptr;
  if (data) {
    if (auto loc = tc::publishTranslation(std::move(*data))) {
      result = loc->mainStart();
    }
  }

  tc::checkFreeProfData();
  return result;
}

bool retranslateOpt(FuncId funcId) {
  VMProtect _;

  if (isDebuggerAttachedProcess()) return false;

  auto const func = const_cast<Func*>(Func::fromFuncId(funcId));
  if (profData() == nullptr) return false;
  if (profData()->optimized(funcId)) return true;

  LeaseHolder writer(func, TransKind::Optimize, false);
  if (!writer) return false;

  if (profData()->optimized(funcId)) return true;
  profData()->setOptimized(funcId);

  tc::FuncMetaInfo info(func, tc::LocalTCBuffer());
  optimize(info);
  tc::publishOptFunction(std::move(info));
  tc::checkFreeProfData();

  return true;
}

bool retranslateAllEnabled() {
  return
    RuntimeOption::EvalJitPGO &&
    RuntimeOption::EvalJitRetranslateAllRequest != 0;
}

void checkRetranslateAll() {
  static std::atomic<bool> scheduled(false);

  if (!retranslateAllEnabled() ||
      scheduled.load(std::memory_order_relaxed) ||
      !hasEnoughProfDataToRetranslateAll()) {
    return;
  }
  if (scheduled.exchange(true)) {
    // Another thread beat us.
    return;
  }
  // We schedule a one-time call to retranslateAll() via the treadmill.  We use
  // the treadmill to ensure that no additional Profile translations are being
  // emitted when retranslateAll() runs, which avoids the need for additional
  // locking on the ProfData. We use a fresh thread to avoid stalling the
  // treadmill, the thread is joined in the processExit handler for mcgen.
  if (RuntimeOption::ServerExecutionMode()) {
    Logger::Info("Scheduling the retranslation of all profiled translations");
  }
  Treadmill::enqueue([] {
    s_retranslateAllThread = std::thread([] { retranslateAll(); });
  });
}

bool retranslateAllPending() {
  return
    retranslateAllEnabled() &&
    !s_retranslateAllComplete.load(std::memory_order_acquire);
}

int getActiveWorker() {
  if (s_retranslateAllComplete.load(std::memory_order_relaxed)) {
    return 0;
  }
  if (auto disp = s_dispatcher.load(std::memory_order_relaxed)) {
    return disp->getActiveWorker();
  }
  return 0;
}

}}}
