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

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen {

namespace {

std::thread s_retranslateAllThread;
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

  assertx(!s_bumpers);
  SCOPE_EXIT { s_bumpers = nullptr; };
  auto const bumpers = bumpTraceFunctions(func);
  if (bumpers.size()) {
    s_bumpers = &bumpers;
  }

  tracing::Block _{"optimize", [&] { return traceProps(func); }};

  // Regenerate the prologues and DV funclets before the actual function body.
  auto const includedBody = regeneratePrologues(func, info);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = includedBody ? std::vector<RegionDescPtr>{}
                                    : regionizeFunc(func, transCFGAnnot);
  tracing::annotateBlock(
    [&] {
      return tracing::Props{}
        .add("num_regions", regions.size())
        .add("included_body", includedBody);
    }
  );

  FTRACE(4, "Translating {} regions for {} (includedBody={})\n",
         regions.size(), func->fullName(), includedBody);

  auto optIndex = 0;
  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto transArgs = TransArgs{regionSk};
    if (transCFGAnnot.size() > 0) {
      transArgs.annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    FTRACE(4, "Translating {} with optIndex={}\n",
           func->fullName(), optIndex);
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

std::condition_variable s_condVar;
std::mutex s_condVarMutex;

struct TranslateWorker : JobQueueWorker<tc::FuncMetaInfo*, void*, true, true> {
  void doJob(tc::FuncMetaInfo* info) override {
    ProfileNonVMThread nonVM;
    HphpSession hps{Treadmill::SessionKind::TranslateWorker};
    ARRPROV_USE_POISONED_LOCATION();

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

void createSrcRecs(const Func* func) {
  auto spOff = FPInvOffset { func->numSlotsInFrame() };
  auto const profData = globalProfData();

  auto create_one = [&] (Offset off) {
    auto const sk = SrcKey { func, off, ResumeMode::None };
    if (off == func->base() ||
        profData->dvFuncletTransId(sk) != kInvalidTransID) {
      tc::createSrcRec(sk, spOff);
    }
  };

  create_one(func->base());
  for (auto const& pi : func->params()) {
    if (pi.hasDefaultValue()) create_one(pi.funcletOff);
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
 * mode.  This function returns true iff we have stopped the server in
 * SerializeAndExit mode.
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
    if (mode == JitSerdesMode::SerializeAndExit && !serializeOptProfEnabled()) {
      killProcess();
      return true;
    }
  }
  return false;
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

  auto const uptime = static_cast<int>(f_server_uptime()); // may be -1
  if (delaySeconds > 0 && uptime >= 0) {
    s_serializeOptProfSeconds = uptime + delaySeconds;
    if (serverMode) {
      Logger::FInfo("retranslateAll: scheduled serialization of optimized "
                    "code's profile for after running another {} seconds",
                    delaySeconds);
    }
  }
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
  const bool serverMode = RuntimeOption::ServerExecutionMode();
  const bool serialize = RuntimeOption::RepoAuthoritative &&
                         !RuntimeOption::EvalJitSerdesFile.empty() &&
                         isJitSerializing();
  const bool serializeOpt = serialize && serializeOptProfEnabled();

  // 1) Obtain function ordering in code.hot.

  if (globalProfData()->sortedFuncs().empty()) {
    auto result = hfsortFuncs();
    ProfData::Session pds;
    profData()->setFuncOrder(std::move(result.first));
    profData()->setBaseProfCount(result.second);
  } else {
    assertx(isJitDeserializing());
  }
  setBaseInliningProfCount(globalProfData()->baseProfCount());
  auto const& sortedFuncs = globalProfData()->sortedFuncs();
  auto const nFuncs = sortedFuncs.size();

  // 2) Check if we should dump profile data. We may exit here in
  //    SerializeAndExit mode, without really doing the JIT, unless
  //    serialization of optimized code's profile is also enabled.

  if (serialize && serializeProfDataAndLog()) return;

  // 3) Generate machine code for all the profiled functions.

  auto const initialSize = 512;
  std::vector<tc::FuncMetaInfo> jobs;
  jobs.reserve(nFuncs);
  std::unique_ptr<uint8_t[]> codeBuffer(new uint8_t[nFuncs * initialSize]);

  {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};
    BootStats::Block timer("RTA_translate_and_relocate",
                           RuntimeOption::ServerExecutionMode());
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

    // 4) Relocate the machine code into code.hot in the desired order

    tc::relocatePublishSortedOptFuncs(std::move(jobs));

    if (auto const dispatcher = s_dispatcher.load(std::memory_order_acquire)) {
      s_dispatcher.store(nullptr, std::memory_order_release);
      dispatcher->waitEmpty(true);
      delete dispatcher;
    }
  }

  if (serverMode) {
    Logger::Info("retranslateAll: finished retranslating all optimized "
                 "translations!");
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
      tc::freeProfCode();
    }
  }

  if (serializeOpt) {
    scheduleSerializeOptProf();
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

  if (s_retranslateAllThread.joinable()) {
    s_retranslateAllThread.join();
  }

  if (s_serializeOptProfThread.joinable()) {
    s_serializeOptProfThread.join();
  }
}

folly::Optional<tc::TransMetaInfo>
translate(TransArgs args, FPInvOffset spOff,
          folly::Optional<CodeCache::View> optView) {
  INC_TPC(translate);
  assertx(args.kind != TransKind::Invalid);

  if (!tc::shouldTranslate(args.sk, args.kind)) return folly::none;

  Timer timer(Timer::mcg_translate);
  WorkloadStats guard(WorkloadStats::InTrans);

  tracing::Block _{"translate", [&] { return traceProps(args); }};

  rqtrace::ScopeGuard trace{"JIT_TRANSLATE"};
  trace.annotate("func_name", args.sk.func()->fullName()->data());
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
    tracing::annotateBlock(
      [&] {
        return tracing::Props{}
          .add("region_size", args.region->instrSize());
      }
    );
    auto const transContext =
      TransContext{env.transID, args.kind, args.flags, args.sk,
                   env.initSpOffset, args.optIndex, args.region.get()};

    env.unit = irGenRegion(*args.region, transContext, env.pconds);
    auto const unitAnnotations = env.unit->annotationData->getAllAnnotations();
    env.annotations.insert(env.annotations.end(),
                           unitAnnotations.begin(), unitAnnotations.end());
    if (env.unit) {
      env.vunit = irlower::lowerUnit(*env.unit);
    }
  }

  timer.stop();
  return tc::emitTranslation(std::move(env), optView);
}

TCA retranslate(TransArgs args, const RegionContext& ctx) {
  ARRPROV_USE_POISONED_LOCATION();
  VMProtect _;

  if (RID().isJittingDisabled()) {
    SKTRACE(2, args.sk, "punting because jitting code was disabled\n");
    return nullptr;
  }

  auto sr = tc::findSrcRec(args.sk);
  auto const initialNumTrans = sr->numTrans();
  auto const funcId = args.sk.funcID();

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
      !profData()->profiling(funcId) &&
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
      (profData()->profiling(funcId) ||
       profData()->optimized(funcId))) {
    return nullptr;
  }

  LeaseHolder writer(args.sk.func(), args.kind);
  if (!writer) return nullptr;

  // Update kind now that we have the write lease.
  args.kind = kind();

  if (!tc::shouldTranslate(args.sk, args.kind)) return nullptr;

  // If the function is being profiled and hasn't been optimized yet, we don't
  // want to emit Live translations for it, as these would be made unreachable
  // once we clear the SrcRec to publish the optimized code.
  if (args.kind != TransKind::Profile && profData() &&
      profData()->profiling(funcId) && !profData()->optimized(funcId)) {
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

  tracing::Block _b{
    "retranslate",
    [&] {
      return traceProps(args)
        .add("initial_num_trans", initialNumTrans);
    }
  };
  tracing::Pause _p;

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
  ARRPROV_USE_POISONED_LOCATION();
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

  auto const uptime = f_server_uptime(); // may be -1
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

}}}
