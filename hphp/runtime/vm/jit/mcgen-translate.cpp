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

#include "hphp/runtime/vm/jit/func-order.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/outlined-sequence-selector.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc-region.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/vasm-block-counters.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/tracing.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/runtime/ext/server/ext_server.h"
#include "hphp/runtime/server/http-server.h"

#include "hphp/util/boot-stats.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/numa.h"
#include "hphp/util/trace.h"

#include "hphp/zend/zend-strtod.h"

#include <folly/system/ThreadName.h>

TRACE_SET_MOD(mcg);

namespace HPHP::jit::mcgen {

namespace {

std::thread s_retranslateAllThread;
std::mutex s_rtaThreadMutex;
std::atomic<bool> s_retranslateAllScheduled{false};
std::atomic<bool> s_retranslateAllComplete{false};
static __thread const CompactVector<Trace::BumpRelease>* s_bumpers;

std::thread s_serializeOptProfThread;
std::atomic<bool> s_serializeOptProfScheduled{false};
std::atomic<bool> s_serializeOptProfTriggered{false};
std::atomic<uint32_t> s_serializeOptProfRequest{0}; // 0 means disabled
std::atomic<uint32_t> s_serializeOptProfSeconds{0}; // 0 means disabled

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

  assertx(!s_bumpers);
  SCOPE_EXIT { s_bumpers = nullptr; };
  auto const bumpers = bumpTraceFunctions(func);
  if (bumpers.size()) {
    s_bumpers = &bumpers;
  }

  tracing::Block _{"optimize", [&] { return traceProps(func); }};

  // Regenerate the prologues before the actual function body.
  regeneratePrologues(func, info);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = regionizeFunc(func, transCFGAnnot);
  tracing::annotateBlock(
    [&] {
      return tracing::Props{}
        .add("num_regions", regions.size());
    }
  );

  FTRACE(4, "Translating {} regions for {}\n",
         regions.size(), func->fullName());

  Optional<uint64_t> maxWeight;
  for (auto region : regions) {
    auto const weight = VasmBlockCounters::getRegionWeight(*region);
    if (weight) {
      FTRACE(5, "  Weight for {}: {}\n", show(region->start()), *weight);
      if (!maxWeight || *weight > *maxWeight) maxWeight = *weight;
    }
  }
  if (maxWeight) {
    FTRACE(4, "  Setting hot weight hint: {}\n", *maxWeight);
    for (auto region : regions) region->setHotWeight(*maxWeight);
  }

  auto optIndex = 0;
  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();

    auto translator = std::make_unique<tc::RegionTranslator>(
      regionSk, TransKind::Optimize
    );
    if (transCFGAnnot.size() > 0) {
      translator->annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    FTRACE(4, "Translating {} with optIndex={}\n",
           showShort(regionSk), optIndex);
    translator->region = region;
    translator->optIndex = optIndex++;
    auto const spOff = region->entry()->initialSpOffset();
    translator->spOff = spOff;
    if (tc::createSrcRec(regionSk, spOff) == nullptr) {
      // ran out of TC space, stop trying to translate regions
      break;
    }
    translator->translate(info.tcBuf.view());
    if (translator->translateSuccess()) {
      info.add(std::move(translator));
      transCFGAnnot = ""; // so we don't annotate it again
    }
  }
}

std::condition_variable s_condVar;
std::mutex s_condVarMutex;

struct TranslateWorker : JobQueueWorker<tc::FuncMetaInfo*, void*, true, true> {
  void doJob(tc::FuncMetaInfo* info) override {
    try {
      ProfileNonVMThread nonVM;
      HphpSession hps{Treadmill::SessionKind::TranslateWorker};

      // Check if the func was treadmilled before the job started
      if (!Func::isFuncIdValid(info->fid)) return;

      always_assert(!profData()->optimized(info->fid));

      VMProtect _;
      optimize(*info);

      {
        std::unique_lock<std::mutex> lock{s_condVarMutex};
        always_assert(!profData()->optimized(info->fid));
        profData()->setOptimized(info->fid);
      }
      s_condVar.notify_one();
    } catch (std::exception& e) {
      always_assert_flog(false,
                         "Uncaught exception {} in RTA thread", e.what());
    } catch (...) {
      always_assert_flog(false, "Uncaught unknown exception in RTA thread");
    }
  }

  void onThreadEnter() override {
    folly::setThreadName("jitworker");
#if USE_JEMALLOC_EXTENT_HOOKS
    if (auto arena = next_extra_arena(s_numaNode)) {
      arena->bindCurrentThread();
    }
#endif
  }
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

void createSrcRecs(const Func* func) {
  auto const profData = globalProfData();
  auto const numParams = func->numNonVariadicParams();

  auto create_one = [&] (uint32_t numArgs) {
    auto const sk = SrcKey { func, numArgs, SrcKey::FuncEntryTag {} };
    if (numArgs == numParams ||
        profData->dvFuncletTransId(sk) != kInvalidTransID) {
      tc::createSrcRec(sk, SBInvOffset{0});
    }
  };

  for (auto i = func->numRequiredParams(); i <= numParams; ++i) {
    create_one(i);
  }
}

void killProcess() {
  auto const pid = getpid();
  if (pid > 0) {
    kill(pid, SIGTERM);
  } else {
    abort();
  }
}

/*
 * Serialize the profile data, logging start/finish/error messages in server
 * mode. This function returns true if retranslate-all should be skipped.
 */
bool serializeProfDataAndLog() {
  auto const serverMode = RuntimeOption::ServerExecutionMode();
  auto const mode = RuntimeOption::EvalJitSerdesMode;
  if (serverMode) {
    Logger::Info("retranslateAll: serializing profile data");
  }
  std::string errMsg;
  VMWorker([&errMsg] () {
    errMsg = serializeProfData(RuntimeOption::EvalJitSerdesFile);
  }).run();

  if (serverMode) {
    if (errMsg.empty()) {
      Logger::Info("retranslateAll: serializing done");
    } else {
      Logger::FError("serializeProfData failed with: {}", errMsg);
    }

    if (mode == JitSerdesMode::Serialize) {
      if (serializeOptProfEnabled() && RO::EvalJitSerializeOptProfRestart) {
        Logger::Info("retranslateAll: deferring retranslate-all until restart");
        return true;
      }
      return false;
    }

    assertx(mode == JitSerdesMode::SerializeAndExit);
    if (!serializeOptProfEnabled() || RO::EvalJitSerializeOptProfRestart) {
      Logger::Info("retranslateAll: deferring retranslate-all until restart");
      killProcess();
      return true;
    }
    return false;
  }

  return serializeOptProfEnabled() && RO::EvalJitSerializeOptProfRestart;
}

/*
 * Schedule serialization of optimized code's profile to happen in the future.
 */
void scheduleSerializeOptProf() {
  assertx(serializeOptProfEnabled());

  if (s_serializeOptProfScheduled.exchange(true)) {
    // someone beat us
    return;
  }

  auto const serverMode    = RuntimeOption::ServerExecutionMode();
  auto const delayRequests = RuntimeOption::EvalJitSerializeOptProfRequests;
  auto const delaySeconds  = RuntimeOption::EvalJitSerializeOptProfSeconds;

  if (delayRequests > 0) {
    s_serializeOptProfRequest = requestCount() + delayRequests;
    if (serverMode) {
      Logger::FInfo("retranslateAll: scheduled serialization of optimized "
                    "code's profile for after running another {} requests",
                    delayRequests);
    }
  }

  // server_uptime will return -1 if the http server is not yet
  // active. In that case, set the uptime to 0, so that we'll trigger
  // exactly delaySeconds after the server becomes active.
  auto const uptime = std::max(0, static_cast<int>(HHVM_FN(server_uptime)()));
  if (delaySeconds > 0) {
    s_serializeOptProfSeconds = uptime + delaySeconds;
    if (serverMode) {
      Logger::FInfo("retranslateAll: scheduled serialization of optimized "
                    "code's profile for after running another {} seconds",
                    delaySeconds);
    }
  }
}

// GCC GCOV API
extern "C" void __gcov_reset() __attribute__((__weak__));
// LLVM/clang API. See llvm-project/compiler-rt/lib/profile/InstrProfiling.h
extern "C" void __llvm_profile_reset_counters() __attribute__((__weak__));

/*
 * This is the main driver for the profile-guided retranslation of all the
 * functions being PGO'd, which enables controlling the order in which the
 * Optimize translations are emitted in the TC.
 *
 * There are 5 main steps in this process:
 *   1) Get ordering of functions in the TC using hfsort on the call graph (or
 *   from a precomputed order when deserializing).
 *   2) Compute a bespoke coloring and finalize the layout hierarchy.
 *   3) Finalize the list of "lazy APC classes".
 *   4) Optionally serialize profile data when configured.
 *   5) Generate machine code for each of the profiled functions.
 *   6) Relocate the functions in the TC according to the selected order.
 */
void retranslateAll(bool skipSerialize) {
  const bool serverMode = RuntimeOption::ServerExecutionMode();
  const bool serialize = RuntimeOption::RepoAuthoritative &&
                         !RuntimeOption::EvalJitSerdesFile.empty() &&
                         isJitSerializing();
  const bool serializeOpt = serialize && serializeOptProfEnabled();

  // 1) Obtain function ordering in code.hot.

  if (FuncOrder::get().empty()) {
    auto const avgProfCount = FuncOrder::compute();
    ProfData::Session pds;
    profData()->setBaseProfCount(avgProfCount);
  } else {
    assertx(isJitDeserializing());
  }
  setBaseInliningProfCount(globalProfData()->baseProfCount());
  auto const& sortedFuncs = FuncOrder::get();
  auto const nFuncs = sortedFuncs.size();

  // 2) Perform bespoke coloring and finalize the layout hierarchy.
  //    Jumpstart consumers use the coloring computed by the seeder.

  if (allowBespokeArrayLikes()) bespoke::selectBespokeLayouts();

  // 3) Stop adding new classes to the "lazy APC classes" list. After we
  //    finalize this list, we can skip lazy deserialization checks for any
  //    classes that are NOT on the list when JIT-ing access to them.

  Class::finalizeLazyAPCClasses();

  // 4) Check if we should dump profile data. We may exit here in
  //    SerializeAndExit mode, without really doing the JIT, unless
  //    serialization of optimized code's profile is also enabled.

  if (serialize && !skipSerialize && serializeProfDataAndLog()) return;

  // 5) Generate machine code for all the profiled functions.

  auto const initialSize = 512;
  std::vector<tc::FuncMetaInfo> jobs;
  jobs.reserve(nFuncs);
  std::unique_ptr<uint8_t[]> codeBuffer(new uint8_t[nFuncs * initialSize]);

  {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};
    BootStats::Block timer("RTA_translate_and_relocate",
                           RuntimeOption::ServerExecutionMode());
    auto const runParallelRetranslate = [&] {
      {
        Treadmill::Session session(Treadmill::SessionKind::Retranslate);
        auto bufp = codeBuffer.get();
        for (auto i = 0u; i < nFuncs; ++i, bufp += initialSize) {
          auto const fid = sortedFuncs[i];
          auto const func = const_cast<Func*>(Func::fromFuncId(fid));
          if (!RuntimeOption::EvalJitSerdesDebugFunctions.empty()) {
            // Only run specified functions
            if (!RuntimeOption::EvalJitSerdesDebugFunctions.
                count(func->fullName()->toCppString())) {
              continue;
            }
          }

          jobs.emplace_back(
            tc::FuncMetaInfo(func, tc::LocalTCBuffer(bufp, initialSize))
          );

          createSrcRecs(func);
          enqueueRetranslateOptRequest(&jobs.back());
        }
      }
      if (RuntimeOption::EvalJitBuildOutliningHashes) {
        auto const dispatcher = s_dispatcher.load(std::memory_order_acquire);
        if (dispatcher) {
          dispatcher->waitEmpty(false);
        }
        buildOptimizedHashes();
      }
    };
    runParallelRetranslate();

    if (RuntimeOption::EvalJitRerunRetranslateAll) {
      if (auto const dispatcher = s_dispatcher.load(std::memory_order_acquire)) {
        dispatcher->waitEmpty(false);
      }
      jobs.clear();
      jobs.reserve(nFuncs);
      {
        ProfData::Session pds;
        for (auto i = 0u; i < nFuncs; ++i) {
          auto const fid = sortedFuncs[i];
          profData()->unsetOptimized(fid);
        }
        profData()->clearAllOptimizedSKs();
        clearCachedInliningCost();
      }
      runParallelRetranslate();
    }

    // 6) Relocate the machine code into code.hot in the desired order
    tc::relocatePublishSortedOptFuncs(std::move(jobs));

    if (auto const dispatcher = s_dispatcher.load(std::memory_order_acquire)) {
      s_dispatcher.store(nullptr, std::memory_order_release);
      dispatcher->waitEmpty(true);
      delete dispatcher;
    }
  }

  if (serverMode) {
    auto const uptime = HHVM_FN(server_uptime)();
    if (uptime > 0) {
      BootStats::set("jit_profile_and_optimize", uptime);
      BootStats::done();
      Logger::FInfo("retranslateAll finished {} seconds after server started",
                    uptime);
    } else {
      Logger::Info("retranslateAll finished");
    }
  }

  // This will enable live translations to happen again.
  s_retranslateAllComplete.store(true, std::memory_order_release);
  tc::reportJitMaturity();

  if (serverMode && !transdb::enabled() && !serializeOpt) {
    ProfData::Session pds;
    // The ReusableTC mode assumes that ProfData is never freed, so don't
    // discard ProfData in this mode.
    if (!RuntimeOption::EvalEnableReusableTC) {
      discardProfData();
    }
  }

  if (serializeOpt) {
    scheduleSerializeOptProf();
  }

  if (__gcov_reset) {
    if (serverMode) {
      Logger::Info("Calling __gcov_reset (retranslateAll finished)");
    }
    __gcov_reset();
  }
  if (__llvm_profile_reset_counters) {
    if (serverMode) {
      Logger::Info("Calling __llvm_profile_reset_counters (retranslateAll "
                   "finished)");
    }
    __llvm_profile_reset_counters();
  }

}

////////////////////////////////////////////////////////////////////////////////
}

void waitForTranslate(const tc::FuncMetaInfo& info) {
  if (profData()->optimized(info.fid)) return;

  std::unique_lock<std::mutex> lock{s_condVarMutex};
  s_condVar.wait(
    lock,
    [&] {
      return profData()->optimized(info.fid);
    }
  );
}

void joinWorkerThreads() {
  if (s_dispatcher.load(std::memory_order_acquire)) {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};
    if (auto dispatcher = s_dispatcher.load(std::memory_order_acquire)) {
      s_dispatcher.store(nullptr, std::memory_order_release);
      dispatcher->stop();
      delete dispatcher;
    }
  }

  {
    std::unique_lock<std::mutex> lock{s_rtaThreadMutex};
    if (s_retranslateAllThread.joinable()) {
      s_retranslateAllThread.join();
    }
  }

  if (s_serializeOptProfThread.joinable()) {
    s_serializeOptProfThread.join();
  }
}

TranslationResult retranslate(TransArgs args, const RegionContext& ctx) {
  VMProtect _;

  tc::RegionTranslator translator(args.sk);
  translator.spOff = ctx.spOffset;
  translator.liveTypes = ctx.liveTypes;

  auto const tcAddr = translator.acquireLeaseAndRequisitePaperwork();
  if (tcAddr) return *tcAddr;

  tracing::Block _b{
    "retranslate",
      [&] {
        return traceProps(args)
          .add("initial_num_trans", translator.prevNumTranslations);
      }
  };
  tracing::Pause _p;

  if (auto const res = translator.translate()) return *res;
  if (auto const res = translator.relocate(false)) return *res;
  if (auto const res = translator.bindOutgoingEdges()) return *res;
  return translator.publish();
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

  tracing::Block _b{"retranslate-opt", [&] { return traceProps(func); }};
  tracing::Pause _p;

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

void checkRetranslateAll(bool force, bool skipSerialize) {
  assertx(IMPLIES(skipSerialize, force));

  if (s_retranslateAllScheduled.load(std::memory_order_relaxed) ||
      !retranslateAllEnabled()) {
    assertx(!force);
    return;
  }

  auto const serverMode = RuntimeOption::ServerExecutionMode();
  if (!force) {
    auto const uptime = static_cast<int>(HHVM_FN(server_uptime)()); // may be -1
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
    assertx(!force);
    return;
  }

  if (!force && serverMode) {
    // We schedule a one-time call to retranslateAll() via the treadmill.  We
    // use the treadmill to ensure that no additional Profile translations are
    // being emitted when retranslateAll() runs, which avoids the need for
    // additional locking on the ProfData. We use a fresh thread to avoid
    // stalling the treadmill, the thread is joined in the processExit handler
    // for mcgen.
    Treadmill::enqueue([] {
      std::unique_lock<std::mutex> lock{s_rtaThreadMutex};
      s_retranslateAllThread = std::thread([] {
        folly::setThreadName("jit.rta");
        rds::local::init();
        zend_get_bigint_data();
        SCOPE_EXIT { rds::local::fini(); };
        retranslateAll(false);
      });
    });
  } else {
    std::unique_lock<std::mutex> lock{s_rtaThreadMutex};
    s_retranslateAllThread = std::thread([skipSerialize] {
      folly::setThreadName("jit.rta");
      BootStats::Block timer("retranslateall",
                             RuntimeOption::ServerExecutionMode());
      rds::local::init();
      zend_get_bigint_data();
      SCOPE_EXIT { rds::local::fini(); };
      retranslateAll(skipSerialize);
    });
    if (!serverMode) s_retranslateAllThread.join();
  }
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

CompactVector<Trace::BumpRelease> unbumpFunctions() {
  CompactVector<Trace::BumpRelease> result;
  if (s_bumpers) {
    for (auto& bump : *s_bumpers) {
      result.emplace_back(bump.negate());
    }
  }
  return result;
}

void checkSerializeOptProf() {
  if (!s_serializeOptProfScheduled.load(std::memory_order_relaxed) ||
      s_serializeOptProfTriggered.load(std::memory_order_relaxed)) {
    return;
  }

  assertx(RuntimeOption::RepoAuthoritative &&
          !RuntimeOption::EvalJitSerdesFile.empty() &&
          isJitSerializing());

  auto const uptime = HHVM_FN(server_uptime)(); // may be -1
  auto const triggerSeconds =
    s_serializeOptProfSeconds.load(std::memory_order_relaxed);
  auto const triggerRequest =
    s_serializeOptProfRequest.load(std::memory_order_relaxed);
  const bool trigger =
    ((triggerSeconds > 0 && uptime >= 0 && uptime >= triggerSeconds) ||
     (triggerRequest > 0 && requestCount() >= triggerRequest));

  if (!trigger) return;

  if (s_serializeOptProfTriggered.exchange(true)) {
    // Another thread beat us.
    return;
  }

  // Create a thread to serialize the profile for the optimized code.
  s_serializeOptProfThread = std::thread([] {
    rds::local::init();
    SCOPE_EXIT { rds::local::fini(); };
    auto const serverMode = RuntimeOption::ServerExecutionMode();
    if (serverMode) {
      Logger::FInfo("retranslateAll: serialization of optimized code's "
                    "profile triggered");
    }

    auto const errMsg = serializeOptProfData(RuntimeOption::EvalJitSerdesFile);

    if (serverMode) {
      if (errMsg.empty()) {
        Logger::FInfo("retranslateAll: serializeOptProfData completed");
      } else {
        Logger::FInfo("retranslateAll: serializeOptProfData failed: {}",
                      errMsg);
      }
    }

    if (!transdb::enabled()) {
      discardProfData();
    }

    auto const mode = RuntimeOption::EvalJitSerdesMode;
    if (mode == JitSerdesMode::SerializeAndExit) {
      killProcess();
    }
  });
}

}
