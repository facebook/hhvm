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
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/workload-stats.h"

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/runtime/ext/server/ext_server.h"
#include "hphp/runtime/server/http-server.h"

#include "hphp/util/boot-stats.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/numa.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen {

namespace {

std::thread s_retranslateAllThread;
std::atomic<bool> s_retranslateAllScheduled{false};
std::atomic<bool> s_retranslateAllComplete{false};

CompactVector<Trace::BumpRelease> bumpTraceFunctions(const Func* func) {
  auto def = [&] {
    CompactVector<Trace::BumpRelease> result;
    result.emplace_back(Trace::hhir_refcount, -10);
    result.emplace_back(Trace::hhir_load, -10);
    result.emplace_back(Trace::hhir_store, -10);
    result.emplace_back(Trace::printir, -10);
    return result;
  };

  auto opt = [&] {
    CompactVector<Trace::BumpRelease> result;
    if (RuntimeOption::EvalJitPrintOptimizedIR) {
      result.emplace_back(Trace::printir,
                          -RuntimeOption::EvalJitPrintOptimizedIR);
    }

    return result;
  };

  if (func->getFuncId() == RuntimeOption::TraceFuncId) {
    return def();
  }
  if (!RuntimeOption::TraceFunctions.empty()) {
    auto const funcName = func->fullName()->slice();
    auto const it =
      RuntimeOption::TraceFunctions.lower_bound(funcName);
    if (it == RuntimeOption::TraceFunctions.end()) return opt();
    folly::StringPiece name = *it;
    if (name.size() >= funcName.size() &&
        bstrcaseeq(name.data(), funcName.data(), funcName.size())) {
      if (name.size() == funcName.size()) return def();
      if (name[funcName.size()] != ';') return opt();
      name.advance(funcName.size() + 1);
      return Trace::bumpSpec(name);
    }
  }

  return opt();
}

void optimize(tc::FuncMetaInfo& info) {
  auto const func = info.func;

  auto const bumpers = bumpTraceFunctions(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  auto const includedBody = regeneratePrologues(func, info);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = includedBody ? std::vector<RegionDescPtr>{}
                                    : regionizeFunc(func, transCFGAnnot);

  auto optIndex = 0;
  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto transArgs = TransArgs{regionSk};
    if (transCFGAnnot.size() > 0) {
      transArgs.annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    transArgs.region = region;
    transArgs.kind = TransKind::Optimize;
    transArgs.optIndex = optIndex++;

    auto const spOff = region->entry()->initialSpOffset();
    tc::createSrcRec(regionSk, spOff);
    auto data = translate(transArgs, spOff, info.tcBuf.view());
    if (data) {
      info.add(std::move(*data));
      transCFGAnnot = ""; // so we don't annotate it again
    }
  }
}

struct TranslateWorker : JobQueueWorker<tc::FuncMetaInfo*, void*, true, true> {
  void doJob(tc::FuncMetaInfo* info) override {
    ProfileNonVMThread nonVM;

    hphp_session_init(Treadmill::SessionKind::TranslateWorker);
    SCOPE_EXIT {
      hphp_context_exit();
      hphp_session_exit();
    };

    // Check if the func was treadmilled before the job started
    if (!Func::isFuncIdValid(info->fid)) return;

    if (profData()->optimized(info->fid)) return;
    profData()->setOptimized(info->fid);

    VMProtect _;
    optimize(*info);
  }

#if USE_JEMALLOC_EXTENT_HOOKS
  void onThreadEnter() override {
    if (auto arena = next_extra_arena(s_numaNode)) {
      arena->bindCurrentThread();
    }
  }
#endif
};

using WorkerDispatcher = JobQueueDispatcher<TranslateWorker>;
std::atomic<WorkerDispatcher*> s_dispatcher;
std::mutex s_dispatcherMutex;

WorkerDispatcher& dispatcher() {
  if (auto ptr = s_dispatcher.load(std::memory_order_acquire)) return *ptr;

  auto dispatcher = new WorkerDispatcher(
    RuntimeOption::EvalJitWorkerThreads,
    RuntimeOption::EvalJitWorkerThreads, 0, false, nullptr
  );
  dispatcher->start();
  s_dispatcher.store(dispatcher, std::memory_order_release);
  return *dispatcher;
}

void enqueueRetranslateOptRequest(tc::FuncMetaInfo* info) {
  dispatcher().enqueue(info);
}

/*
 * This is the main driver for the profile-guided retranslation of all the
 * functions being PGO'd, which enables controlling the order in which the
 * Optimize translations are emitted in the TC.
 *
 * There are 4 main steps in this process:
 *   1) Get ordering of functions in the TC using hfsort on the call graph (or
 *   from a precomputed order when deserializing).
 *   2) Optionally serialize profile data when configured.
 *   3) Generate machine code for each of the profiled functions.
 *   4) Relocate the functions in the TC according to the selected order.
 */
void retranslateAll() {
  // Return true if we have stopped the server in SerializeAndExit mode.
  auto const checkSerializeProfData = [] () -> bool {
    auto const serverMode = RuntimeOption::ServerExecutionMode();
    auto const mode = RuntimeOption::EvalJitSerdesMode;
    if (RuntimeOption::RepoAuthoritative &&
        !RuntimeOption::EvalJitSerdesFile.empty() &&
        isJitSerializing(mode)) {
      if (serverMode) Logger::Info("retranslateAll: serializing profile data");
      std::string errMsg;
      VMWorker([&errMsg] () {
        errMsg = serializeProfData(RuntimeOption::EvalJitSerdesFile);
      }).run();
      if (serverMode) {
        if (errMsg.empty()) {
          Logger::Info("retranslateAll: serializing done");
        } else {
          Logger::Error(errMsg);
        }
        if (mode == JitSerdesMode::SerializeAndExit) {
          HttpServer::Server->stop();
          return true;
        }
      }
    }
    return false;
  };

  const bool serverMode = RuntimeOption::ServerExecutionMode();

  // 1) Obtain function ordering in code.hot.

  if (globalProfData()->sortedFuncs().empty()) {
    auto result = hfsortFuncs();
    ProfData::Session pds;
    profData()->setFuncOrder(std::move(result.first));
    profData()->setBaseProfCount(result.second);
  } else {
    assertx(isJitDeserializing(RuntimeOption::EvalJitSerdesMode));
  }
  setBaseInliningProfCount(globalProfData()->baseProfCount());
  auto const& sortedFuncs = globalProfData()->sortedFuncs();
  auto const nFuncs = sortedFuncs.size();

  // 2) Check if we should dump profile data. We may exit here in
  // SerializeAndExit mode, without really doing the JIT.

  if (checkSerializeProfData()) return;

  // 3) Generate machine code for all the profiled functions.

  auto const initialSize = 512;
  std::vector<tc::FuncMetaInfo> jobs;
  jobs.reserve(nFuncs);
  std::unique_ptr<uint8_t[]> codeBuffer(new uint8_t[nFuncs * initialSize]);

  {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};
    BootStats::Block timer("RTA_translate",
                           RuntimeOption::ServerExecutionMode());
    {
      Treadmill::Session session(Treadmill::SessionKind::Retranslate);
      auto bufp = codeBuffer.get();
      for (auto i = 0u; i < nFuncs; ++i, bufp += initialSize) {
        auto const fid = sortedFuncs[i];
        auto const func = const_cast<Func*>(Func::fromFuncId(fid));
        jobs.emplace_back(
          tc::FuncMetaInfo(func, tc::LocalTCBuffer(bufp, initialSize))
        );
        enqueueRetranslateOptRequest(&jobs.back());
      }
    }

    dispatcher().waitEmpty();
  }

  if (serverMode) {
    Logger::Info("retranslateAll: finished optimizing functions");
  }

  // 4) Relocate the machine code into code.hot in the desired order

  tc::relocatePublishSortedOptFuncs(std::move(jobs));

  if (serverMode) {
    Logger::Info("retranslateAll: finished retranslating all optimized "
                 "translations!");
  }

  // This will enable live translations to happen again.
  s_retranslateAllComplete.store(true, std::memory_order_release);
  tc::reportJitMaturity();

  if (serverMode) {
    ProfData::Session pds;
    // The ReusableTC mode assumes that ProfData is never freed, so don't
    // discard ProfData in this mode.
    if (!RuntimeOption::EvalEnableReusableTC) {
      discardProfData();
      tc::freeProfCode();
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
  assertx(args.kind != TransKind::Invalid);

  if (!tc::shouldTranslate(args.sk.func(), args.kind)) return folly::none;

  Timer timer(Timer::mcg_translate);
  WorkloadStats guard(WorkloadStats::InTrans);

  rqtrace::ScopeGuard trace{"JIT_TRANSLATE"};
  trace.annotate("func_name", args.sk.func()->fullDisplayName()->data());
  trace.annotate("trans_kind", show(args.kind));
  trace.setEventPrefix("JIT_");

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
    trace.annotate(
      "region_size", folly::to<std::string>(args.region->instrSize()));
    auto const transContext =
      TransContext{env.transID, args.kind, args.flags, args.sk,
                   env.initSpOffset, args.optIndex};

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

  if (RID().isJittingDisabled()) {
    SKTRACE(2, args.sk, "punting because jitting code was disabled\n");
    return nullptr;
  }

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
    return tc::profileFunc(args.sk.func()) ? TransKind::Profile
                                           : TransKind::Live;
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
  tc::publishOptFunc(std::move(info));
  tc::checkFreeProfData();

  return true;
}

bool retranslateAllEnabled() {
  return
    RuntimeOption::EvalJitPGO &&
    RuntimeOption::EvalJitRetranslateAllRequest != 0 &&
    RuntimeOption::EvalJitRetranslateAllSeconds != 0;
}

void checkRetranslateAll(bool force) {
  if (s_retranslateAllScheduled.load(std::memory_order_relaxed) ||
      !retranslateAllEnabled()) {
    return;
  }
  auto const serverMode = RuntimeOption::ServerExecutionMode();
  if (!force) {
    auto const uptime = static_cast<int>(f_server_uptime()); // may be -1
    if (uptime >= (int)RuntimeOption::EvalJitRetranslateAllSeconds) {
      assertx(serverMode);
      Logger::FInfo("retranslateAll: scheduled after {} seconds", uptime);
    } else if (requestCount() >= RuntimeOption::EvalJitRetranslateAllRequest) {
      if (serverMode) {
        Logger::FInfo("retranslateAll: scheduled after {} requests",
                      requestCount());
      }
    } else {
      return;
    }
  }

  if (s_retranslateAllScheduled.exchange(true)) {
    // Another thread beat us.
    return;
  }

  if (!force && RuntimeOption::ServerExecutionMode()) {
    // We schedule a one-time call to retranslateAll() via the treadmill.  We
    // use the treadmill to ensure that no additional Profile translations are
    // being emitted when retranslateAll() runs, which avoids the need for
    // additional locking on the ProfData. We use a fresh thread to avoid
    // stalling the treadmill, the thread is joined in the processExit handler
    // for mcgen.
    Treadmill::enqueue([] {
      s_retranslateAllThread = std::thread([] {
        rds::local::init();
        SCOPE_EXIT { rds::local::fini(); };
        retranslateAll();
      });
    });
  } else {
    s_retranslateAllThread = std::thread([] {
      BootStats::Block timer("retranslateall",
                             RuntimeOption::ServerExecutionMode());
      rds::local::init();
      SCOPE_EXIT { rds::local::fini(); };
      retranslateAll();
    });
  }

  if (!serverMode) s_retranslateAllThread.join();
}

bool retranslateAllPending() {
  return
    retranslateAllEnabled() &&
    !s_retranslateAllComplete.load(std::memory_order_acquire);
}

bool retranslateAllScheduled() {
  return s_retranslateAllScheduled.load(std::memory_order_acquire);
}

bool pendingRetranslateAllScheduled() {
  return s_retranslateAllScheduled.load(std::memory_order_acquire) &&
    !s_retranslateAllComplete.load(std::memory_order_acquire);
}

bool retranslateAllComplete() {
  return s_retranslateAllComplete.load(std::memory_order_acquire);
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
