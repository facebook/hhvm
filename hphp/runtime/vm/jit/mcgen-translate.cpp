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
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen {

namespace {

enum class RetranslateResult {
  /* Retranslation failed permanently because ProfData has been deleted. */
  FAIL,
  /* Retranslation failed for a transient reason like failure to acquire a
   * write lease. */
  RETRY,
  /* The function has been successfully reoptimized, either by this call or a
   * previous one. */
  DONE,
};

RetranslateResult retranslateOptImpl(FuncId funcId, bool isWorker) {
  VMProtect _;

  if (isDebuggerAttachedProcess()) return RetranslateResult::RETRY;

  auto const func = const_cast<Func*>(Func::fromFuncId(funcId));
  if (profData() == nullptr) return RetranslateResult::FAIL;
  if (profData()->optimized(funcId)) return RetranslateResult::DONE;

  LeaseHolder writer(func, TransKind::Optimize, isWorker);
  if (!writer) return RetranslateResult::RETRY;

  if (profData()->optimized(funcId)) return RetranslateResult::DONE;
  profData()->setOptimized(funcId);

  func->setFuncBody(tc::ustubs().funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  tc::invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  auto const includedBody = regeneratePrologues(func);

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
    translate(transArgs, spOff);
    transCFGAnnot = ""; // so we don't annotate it again
  }

  tc::checkFreeProfData();
  return RetranslateResult::DONE;
}

void enqueueRetranslateOptRequest(FuncId funcId);

struct TranslateWorker : JobQueueWorker<FuncId> {
  void doJob(FuncId id) {
    hphp_session_init();
    SCOPE_EXIT {
      hphp_context_exit();
      hphp_session_exit();
    };

    if (!Func::isFuncIdValid(id)) return;

    if (retranslateOptImpl(id, true) == RetranslateResult::RETRY) {
      enqueueRetranslateOptRequest(id);
    }
  }
};

using WorkerDispatcher = JobQueueDispatcher<TranslateWorker>;
std::atomic<WorkerDispatcher*> s_dispatcher;
std::mutex s_dispatcherCreate;

WorkerDispatcher& dispatcher() {
  if (auto ptr = s_dispatcher.load(std::memory_order_acquire)) return *ptr;

  std::lock_guard<std::mutex> lock{s_dispatcherCreate};
  if (auto ptr = s_dispatcher.load(std::memory_order_relaxed)) return *ptr;

  auto dispatcher = new WorkerDispatcher(
    RuntimeOption::EvalJitWorkerThreads, 0, false, nullptr
  );
  dispatcher->start();
  s_dispatcher.store(dispatcher, std::memory_order_release);
  return *dispatcher;
}

void enqueueRetranslateOptRequest(FuncId id) {
  dispatcher().enqueue(id);
}

/*
 * Returns true iff we already have Eval.JitMaxTranslations translations
 * recorded in srcRec.
 */
bool reachedTranslationLimit(SrcKey sk, const SrcRec& srcRec) {
  if (srcRec.translations().size() != RuntimeOption::EvalJitMaxTranslations) {
    return false;
  }
  INC_TPC(max_trans);

  if (debug && Trace::moduleEnabled(Trace::mcg, 2)) {
    const auto& tns = srcRec.translations();
    TRACE(1, "Too many (%zd) translations: %s, BC offset %d\n",
          tns.size(), sk.unit()->filepath()->data(),
          sk.offset());
    SKTRACE(2, sk, "{\n");
    TCA topTrans = srcRec.getTopTranslation();
    for (size_t i = 0; i < tns.size(); ++i) {
      auto const rec = transdb::getTransRec(tns[i].mainStart());
      assertx(rec);
      SKTRACE(2, sk, "%zd %p\n", i, tns[i].mainStart());
      if (tns[i].mainStart() == topTrans) {
        SKTRACE(2, sk, "%zd: *Top*\n", i);
      }
      if (rec->kind == TransKind::Anchor) {
        SKTRACE(2, sk, "%zd: Anchor\n", i);
      } else {
        SKTRACE(2, sk, "%zd: guards {\n", i);
        for (unsigned j = 0; j < rec->guards.size(); ++j) {
          FTRACE(2, "{}\n", rec->guards[j]);
        }
        SKTRACE(2, sk, "%zd } guards\n", i);
      }
    }
    SKTRACE(2, sk, "} /* Too many translations */\n");
  }
  return true;
}

hfsort::TargetGraph
createCallGraph(jit::hash_map<hfsort::TargetId, FuncId>& funcID) {
  using namespace hfsort;
  TargetGraph cg;
  jit::hash_map<FuncId, TargetId> targetID;
  auto pd = profData();

  // Create one node (aka target) for each function that was profiled.
  const auto maxFuncId = pd->maxProfilingFuncId();
  FTRACE(3, "createCallGraph: maxFuncId = {}\n", maxFuncId);
  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!pd->profiling(fid)) continue;
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
      const auto callerTargetId = targetID[callerFuncId];
      const auto callCount = pd->transCounter(callerTransId);
      cg.incArcWeight(callerTargetId, calleeTargetId, callCount);
      totalCalls += callCount;
      FTRACE(3, "  - adding arc @ {} : {} => {} [weight = {}] \n",
             callAddr, callerTargetId, calleeTargetId, callCount);
    }
  };

  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!pd->profiling(fid)) continue;

    auto func = Func::fromFuncId(fid);
    const auto calleeTargetId = targetID[fid];
    const auto transIds = pd->funcProfTransIDs(fid);
    uint32_t totalCalls = 0;
    uint32_t profCount  = 0;
    for (int nargs = 0; nargs <= func->numNonVariadicParams() + 1; nargs++) {
      auto transId = pd->proflogueTransId(func, nargs);
      if (transId == kInvalidTransID) continue;

      FTRACE(3, "  - processing ProfPrologue w/ transId = {}\n", transId);
      const auto trec = pd->transRec(transId);
      assertx(trec->kind() == TransKind::ProfPrologue);
      addCallersCount(calleeTargetId, trec->mainCallers(),  totalCalls);
      addCallersCount(calleeTargetId, trec->guardCallers(), totalCalls);
      profCount += pd->transCounter(transId);
    }
    cg.setSamples(calleeTargetId, std::max(totalCalls, profCount));
  }
  cg.normalizeArcWeights();
  return cg;
}

void print(hfsort::TargetGraph& cg, const char* fileName,
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
 * There are 3 main steps in this process:
 *   1) Generating machine code for each of the functions.
 *   2) Selecting an order for the functions.
 *      2.1) Build a call graph for all the profiled functions.
 *      2.2) Run hfsort to sort the functions.
 *   3) Relocating the functions in the TC according to the selected order.
 */
void retranslateAll() {
  assertx(profData() != nullptr);
  const bool serverMode = RuntimeOption::ServerExecutionMode();

  if (serverMode) {
    Logger::Info("Starting to retranslate all optimized translations...");
  }

  // TODO: 1) Generate machine code for all the profiled functions.

  // 2.1) Create the call graph
  jit::hash_map<hfsort::TargetId, FuncId> target2FuncId;
  auto cg = createCallGraph(target2FuncId);

  if (serverMode) {
    Logger::Info("retranslateAll: finished building the call graph");
  }
  if (RuntimeOption::EvalJitPGODumpCallGraph) {
    cg.printDot("/tmp/cg-pgo.dot",
                [&](hfsort::TargetId targetId) -> const char* {
                  const auto fid = target2FuncId[targetId];
                  const auto func = Func::fromFuncId(fid);
                  return func->fullName()->data();
                });
    if (serverMode) {
      Logger::Info("retranslateAll: saved call graph at /tmp/cg-pgo.dot");
    }
  }

  // 2.2) Pass the call graph to hfsort to obtain the order in which things
  //      should be placed in code.hot
  auto clusters = hfsort::clusterize(cg);
  sort(clusters.begin(), clusters.end(), hfsort::compareClustersDensity);

  if (serverMode) {
    Logger::Info("retranslateAll: finished clusterizing the functions");
  }

  if (RuntimeOption::EvalJitPGODumpCallGraph) {
    print(cg, "/tmp/hotfuncs-pgo.txt", clusters, target2FuncId);
    if (serverMode) {
      Logger::Info("retranslateAll: saved sorted list of hot functions at "
                   "/tmp/hotfuncs-pgo.dot");
    }
  }

  // TODO: 3) Relocate the machine code into code.hot in the desired order

  if (serverMode) {
    Logger::Info("Finished retranslating all optimized translations!");
    discardProfData();
  }

}

////////////////////////////////////////////////////////////////////////////////
}

void processExit() {
  if (auto dispatcher = s_dispatcher.load(std::memory_order_acquire)) {
    dispatcher->stop();
  }
}

TCA translate(TransArgs args, FPInvOffset spOff, ProfTransRec* prologue) {
  INC_TPC(translate);
  assert(args.kind != TransKind::Invalid);

  if (!tc::shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  Timer timer(Timer::mcg_translate);

  auto const srcRec = tc::findSrcRec(args.sk);
  always_assert(srcRec);

  TransEnv env{args};
  env.prologue = prologue;
  env.initSpOffset = spOff;
  env.annotations.insert(env.annotations.end(),
                         args.annotations.begin(), args.annotations.end());

  // Lower the RegionDesc to an IRUnit, then lower that to a Vunit.
  if (args.region && !reachedTranslationLimit(args.sk, *srcRec)) {
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
  return tc::emitTranslation(std::move(env));
}

TCA retranslate(TransArgs args, const RegionContext& ctx) {
  VMProtect _;

  auto sr = tc::findSrcRec(args.sk);
  always_assert(sr);
  bool locked = sr->tryLock();
  SCOPE_EXIT {
    if (locked) sr->freeLock();
  };
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

  LeaseHolder writer(args.sk.func(), args.kind);
  if (!writer || !tc::shouldTranslate(args.sk.func(), kind())) {
    return nullptr;
  }

  if (!locked) {
    // Even though we knew above that we were going to skip doing another
    // translation, we wait until we get the write lease, to avoid spinning
    // through the tracelet guards again and again while another thread is
    // writing to it.
    return sr->getTopTranslation();
  }
  if (sr->translations().size() > RuntimeOption::EvalJitMaxTranslations) {
    always_assert(sr->translations().size() ==
                  RuntimeOption::EvalJitMaxTranslations + 1);
    return sr->getTopTranslation();
  }
  SKTRACE(1, args.sk, "retranslate\n");

  args.kind = kind();
  if (!writer.checkKind(args.kind)) return nullptr;

  args.region = selectRegion(ctx, args.kind);
  auto result = translate(args, ctx.spOffset);

  tc::checkFreeProfData();
  return result;
}

bool retranslateOpt(FuncId funcId) {
  if (RuntimeOption::EvalJitWorkerThreads > 0) {
    if (profData() && profData()->shouldQueue(funcId)) {
      enqueueRetranslateOptRequest(funcId);
    }
    return false;
  }

  return retranslateOptImpl(funcId, false) == RetranslateResult::DONE;
}

void checkRetranslateAll() {
  static std::atomic<bool> scheduled(false);

  if (!RuntimeOption::ServerExecutionMode() ||
      !RuntimeOption::EvalJitPGO ||
      RuntimeOption::EvalJitRetranslateAllRequest == 0 ||
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
  // locking on the ProfData.
  Logger::Info("Scheduling the retranslation of all profiled translations");
  Treadmill::enqueue([]{ retranslateAll(); });
}

}}}
