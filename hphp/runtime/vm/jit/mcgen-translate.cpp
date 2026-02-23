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
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc-region.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/vasm-block-counters.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/property-profile.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/tracing.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/runtime/ext/server/ext_server.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/arch.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#if USE_JEMALLOC
#include "hphp/util/managed-arena.h"
#endif
#include "hphp/util/roar.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/instructions-a64.h"

#include "hphp/zend/zend-strtod.h"

#include <folly/system/ThreadName.h>

#include <algorithm>
#include <iomanip>
#include <sstream>

TRACE_SET_MOD(mcg)

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
    if (Cfg::Jit::PrintOptimizedIR) {
      result.emplace_back(Trace::printir,
                          -Cfg::Jit::PrintOptimizedIR);
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
        fstrcmp_slice(funcName, name.subpiece(0, funcName.size())) == 0) {
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
#if USE_JEMALLOC
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
    Cfg::Jit::WorkerThreads,
    Cfg::Jit::WorkerThreads, 0, false, nullptr
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
  auto const serverMode = Cfg::Server::Mode;
  auto const mode = RuntimeOption::EvalJitSerdesMode;
  if (serverMode) {
    Logger::Info("retranslateAll: serializing profile data");
  }
  std::string errMsg;
  VMWorker([&errMsg] () {
    errMsg = serializeProfData(Cfg::Jit::SerdesFile);
  }).run();

  if (serverMode) {
    if (errMsg.empty()) {
      Logger::Info("retranslateAll: serializing done");
    } else {
      Logger::FError("serializeProfData failed with: {}", errMsg);
    }

    if (mode == JitSerdesMode::Serialize) {
      if (serializeOptProfEnabled() && Cfg::Jit::SerializeOptProfRestart) {
        Logger::Info("retranslateAll: deferring retranslate-all until restart");
        return true;
      }
      return false;
    }

    assertx(mode == JitSerdesMode::SerializeAndExit);
    if (!serializeOptProfEnabled() || Cfg::Jit::SerializeOptProfRestart) {
      Logger::Info("retranslateAll: deferring retranslate-all until restart");
      killProcess();
      return true;
    }
    return false;
  }

  return serializeOptProfEnabled() && Cfg::Jit::SerializeOptProfRestart;
}

void debugDumpProfDataAndLog() {
  auto const serverMode = Cfg::Server::Mode;

  if (Cfg::Jit::SerializeDebugLocation.empty()) return;

  if (serverMode) {
    Logger::Info("retranslateAll: serializing profile data for debugging");
  }

  std::string errMsg;
  VMWorker([&errMsg] () {
    errMsg = serializeProfData(Cfg::Jit::SerializeDebugLocation);
  }).run();

  if (serverMode) {
    if (errMsg.empty()) {
      Logger::Info("retranslateAll: serializing done");
    } else {
      Logger::FError("serializeProfData failed with: {}", errMsg);
    }
  }
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

  auto const serverMode    = Cfg::Server::Mode;
  auto const delayRequests = Cfg::Jit::SerializeOptProfRequests;
  auto const delaySeconds  = Cfg::Jit::SerializeOptProfSeconds;

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

/*
 * Per-function runtime call density info, computed by scanning the generated
 * machine code for runtime (non-JIT) call sites and weighing them by profile
 * block execution counts.
 *
 * density = weighted_runtime_calls / hot_bytes
 * where weighted_runtime_calls = sum over all call sites of max(blockWeight, 0)
 */
struct FuncRuntimeCallDensity {
  size_t hotBytes{0};
  int64_t weightedCalls{0};   // sum of block weights for runtime call sites
  size_t numCalls{0};         // total number of runtime call sites
  double density() const {
    return hotBytes > 0 ? (double)weightedCalls / (double)hotBytes : 0.0;
  }
};

struct RuntimeCallSiteInfo {
  TCA callAddr;         // fake TC address of the movz instruction
  uint64_t target;      // absolute address being called
  std::string symbol;   // resolved symbol name
  int64_t blockWeight;  // profile count of enclosing block
  SrcKey blockSk;       // SrcKey of enclosing block
};

/*
 * Scan a single FuncMetaInfo for runtime (non-JIT) call sites.
 * Returns per-call-site details and aggregate density info.
 */
std::pair<std::vector<RuntimeCallSiteInfo>, FuncRuntimeCallDensity>
scanRuntimeCalls(tc::FuncMetaInfo& info, const ProfData* pd) {
  using namespace vixl;

  std::vector<RuntimeCallSiteInfo> runtimeCalls;
  FuncRuntimeCallDensity density;

  auto const func = info.func;
  if (!func) return {runtimeCalls, density};

  auto view = info.tcBuf.view();
  if (!view) return {runtimeCalls, density};

  for (auto const& translator : info.translators) {
    if (!translator->translateSuccess()) continue;

    auto const range = translator->range();
    auto const mainBegin = range.main.begin();
    auto const mainEnd   = range.main.end();
    if (!mainBegin || !mainEnd || mainBegin >= mainEnd) {
      continue;
    }

    density.hotBytes += (mainEnd - mainBegin);

    // Get the region for block weight lookup.
    auto* rt = dynamic_cast<tc::RegionTranslator*>(translator.get());
    auto const& meta = translator->meta();

    // Scan addressImmediates entries within this translation's main range
    // for movz/movk/blr sequences targeting runtime (non-JIT) addresses.
    for (auto aiAddr : meta.addressImmediates) {
      // Skip entries outside this translation's main range.
      if (aiAddr < mainBegin || aiAddr >= mainEnd) continue;

      // Convert fake TC address to real (readable) memory.
      auto const realAddr = view->main().toDestAddress(aiAddr);
      auto const realEnd  = view->main().toDestAddress(mainEnd);
      auto const inst = Instruction::Cast(realAddr);
      auto const instEnd = Instruction::Cast(realEnd);

      uint64_t target = 0;
      uint32_t rd = 0;
      auto const seqLen = [&]() -> size_t {
        // Inline decode of movz/movk sequence (same logic as
        // decodePossibleMovSequence in relocation-arm.cpp).
        if (inst->IsMovz()) {
          target = (uint64_t)inst->ImmMoveWide()
                   << (16 * inst->ShiftMoveWide());
        } else if (inst->IsMovn()) {
          target = ~((uint64_t)inst->ImmMoveWide()
                     << (16 * inst->ShiftMoveWide()));
          if (!inst->SixtyFourBits()) {
            target &= (1UL << 32) - 1;
          }
        } else if (inst->IsLogicalImmediate() &&
                   inst->Rn() == kZeroRegCode) {
          target = inst->ImmLogical();
        } else {
          return 0;
        }
        size_t length = 1;
        rd = inst->Rd();
        auto next = inst->NextInstruction();
        while ((next < instEnd && next->IsMovk() && next->Rd() == rd) ||
               (next < instEnd && next->IsNop())) {
          if (next->IsMovk()) {
            auto const shift = 16 * next->ShiftMoveWide();
            auto const mask = 0xffffLL << shift;
            target &= ~mask;
            target |= (uint64_t)next->ImmMoveWide() << shift;
          }
          length++;
          next = next->NextInstruction();
        }
        return length;
      }();

      if (seqLen == 0) continue;

      // Check if the instruction after the mov sequence is BLR with
      // the same register.
      auto const afterSeq = inst + seqLen * kInstructionSize;
      if (afterSeq >= instEnd) continue;
      if (!afterSeq->IsUncondBranchReg()) continue;
      if (afterSeq->Rn() != rd) continue;
      // Only count BLR (calls), not BR (jumps).
      if (afterSeq->Mask(UnconditionalBranchToRegisterMask) != BLR)
        continue;

      // Check if target is a runtime call (not JIT code).
      if (tc::isValidCodeAddress(reinterpret_cast<TCA>(target))) continue;

      // Resolve the symbol name.
      auto symbol = getNativeFunctionName(
          reinterpret_cast<void*>(target));
      if (symbol.empty()) {
        std::ostringstream tmp;
        tmp << "0x" << std::hex << target;
        symbol = tmp.str();
      }

      // Look up block weight via bcMap.
      int64_t blockWeight = -1;
      SrcKey blockSk;
      if (rt && rt->region) {
        // Find the bcMap entry covering this TCA.
        auto const& bcMap = meta.bcMap;
        for (size_t i = 0; i < bcMap.size(); ++i) {
          auto const entryStart = bcMap[i].aStart;
          auto const entryEnd = (i + 1 < bcMap.size())
            ? bcMap[i + 1].aStart : mainEnd;
          if (aiAddr >= entryStart && aiAddr < entryEnd) {
            blockSk = bcMap[i].sk;
            // Find the region block matching this SrcKey and get its
            // profile execution count.
            for (auto const& block : rt->region->blocks()) {
              if (block->start() == blockSk ||
                  (block->start().func() == blockSk.func() &&
                   block->start().offset() <= blockSk.offset() &&
                   blockSk.offset() <= block->last().offset())) {
                auto const bid = block->profTransID();
                if (pd && bid != kInvalidTransID) {
                  blockWeight = pd->transCounter(bid);
                }
                break;
              }
            }
            break;
          }
        }
      }

      runtimeCalls.push_back(
        RuntimeCallSiteInfo{aiAddr, target, std::move(symbol),
                            blockWeight, blockSk});

      // Accumulate weighted count (use 0 for unknown weights).
      density.numCalls++;
      if (blockWeight > 0) {
        density.weightedCalls += blockWeight;
      }
    }
  }

  return {std::move(runtimeCalls), density};
}

/*
 * Dump per-function runtime call info for all functions in jobs.
 * For each function, scan the machine code for movz/movk/blr sequences
 * (non-smashable calls emitted by vasm `call` on ARM) and report:
 *   - function name, hot bytes size, runtime call density
 *   - for each runtime call: the call-site TCA, target address + symbol name,
 *     and the weight (profile execution count) of the enclosing block.
 *
 * Gated by FTRACE level 6.
 */
void dumpRuntimeCallInfo(std::vector<tc::FuncMetaInfo>& jobs) {
  auto const pd = globalProfData();

  std::ostringstream os;
  os << "=== Runtime Call Info (pre-relocation) ===\n";

  int funcIdx = 0;
  for (auto& info : jobs) {
    auto const func = info.func;
    if (!func) { funcIdx++; continue; }

    auto [runtimeCalls, densityInfo] = scanRuntimeCalls(info, pd);

    // Only print functions that have runtime calls.
    if (runtimeCalls.empty()) { funcIdx++; continue; }

    auto const funcName = func->fullName()->data();
    os << "\n  func[" << funcIdx << "] " << funcName
       << "  hot_bytes=" << densityInfo.hotBytes
       << "  runtime_calls=" << runtimeCalls.size()
       << "  runtime_call_density="
       << std::fixed << std::setprecision(3) << densityInfo.density()
       << " (" << densityInfo.weightedCalls
       << " / " << densityInfo.hotBytes << ")\n";

    for (auto const& ci : runtimeCalls) {
      os << "    call @ " << (void*)ci.callAddr
         << " -> " << ci.symbol
         << "  weight=" << ci.blockWeight;
      if (ci.blockSk.valid()) {
        os << "  block=" << showShort(ci.blockSk);
      }
      os << "\n";
    }

    funcIdx++;
  }

  os << "\n=== End Runtime Call Info ===\n";
  FTRACE(6, "{}", os.str());
}

/*
 * Reorder functions in `jobs` by runtime call density so that functions with
 * the highest density (weighted_runtime_calls / hot_bytes) are placed first.
 *
 * The greedy algorithm sorts functions by density in descending order and
 * promotes them to the front of the jobs vector until the cumulative hot bytes
 * of promoted functions reaches a configurable limit.
 *
 * The limit is controlled by the Cfg::Jit::RuntimeCallReorderLimitMB
 * config option (default: 64 MB).
 *
 * Functions that have no runtime calls or zero density retain their original
 * relative order after the promoted functions.
 */
void reorderByRuntimeCallDensity(std::vector<tc::FuncMetaInfo>& jobs) {
  // Wait for all worker threads to finish translating.
  if (auto const disp = s_dispatcher.load(std::memory_order_acquire)) {
    disp->waitEmpty(false);
  }

  auto const limitMB = Cfg::Jit::RuntimeCallReorderLimitMB;
  auto const limitBytes = static_cast<size_t>(limitMB) * 1024UL * 1024UL;

  auto const pd = globalProfData();
  auto const n = jobs.size();

  // Compute density for each function.
  struct IndexedDensity {
    size_t idx;
    FuncRuntimeCallDensity info;
  };
  std::vector<IndexedDensity> densities;
  densities.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    auto [_, densityInfo] = scanRuntimeCalls(jobs[i], pd);
    densities.push_back(IndexedDensity{i, densityInfo});
  }

  // Separate functions with positive density from the rest.
  std::vector<size_t> promoted;
  std::vector<size_t> remaining;

  // Sort candidates by density descending.
  std::vector<size_t> candidates;
  for (size_t i = 0; i < n; ++i) {
    if (densities[i].info.density() > 0.0) {
      candidates.push_back(i);
    }
  }
  std::sort(candidates.begin(), candidates.end(),
            [&](size_t a, size_t b) {
              return densities[a].info.density() >
                     densities[b].info.density();
            });

  // Greedily select functions until we reach the size limit.
  size_t cumulativeBytes = 0;
  hphp_fast_set<size_t> promotedSet;
  for (auto idx : candidates) {
    auto const hotBytes = densities[idx].info.hotBytes;
    if (cumulativeBytes + hotBytes > limitBytes && !promoted.empty()) {
      break;
    }
    promoted.push_back(idx);
    promotedSet.insert(idx);
    cumulativeBytes += hotBytes;
  }

  // Collect remaining indices in original order.
  for (size_t i = 0; i < n; ++i) {
    if (promotedSet.count(i) == 0) {
      remaining.push_back(i);
    }
  }

  // Build the reordered jobs vector.
  std::vector<tc::FuncMetaInfo> reordered;
  reordered.reserve(n);
  for (auto idx : promoted) {
    reordered.push_back(std::move(jobs[idx]));
  }
  for (auto idx : remaining) {
    reordered.push_back(std::move(jobs[idx]));
  }
  jobs = std::move(reordered);

  FTRACE(4, "reorderByRuntimeCallDensity: promoted {} functions "
            "({} bytes) to the front (limit {} MB)\n",
            promoted.size(), cumulativeBytes, limitMB);
}

// GCC GCOV API
extern "C" void __gcov_reset() __attribute__((__weak__));
// LLVM/clang API. See llvm-project/compiler-rt/lib/profile/InstrProfiling.h
extern "C" void __llvm_profile_reset_counters() __attribute__((__weak__));
// ROAR API
extern "C" int  __roar_api_pending_warmups() __attribute__((__weak__));
extern "C" void __roar_api_trigger_warmup()  __attribute__((__weak__));
extern "C" bool __roar_api_pgo_enabled()     __attribute__((__weak__));
extern "C" bool __roar_api_cspgo_enabled()   __attribute__((__weak__));
extern "C" void __roar_api_wait_for_pgo()    __attribute__((__weak__));
extern "C" void __roar_api_wait_for_cspgo()  __attribute__((__weak__));
extern "C" int  __roar_api_finalize_jumpstart_serialization()
                                             __attribute__((__weak__));

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
  const bool serverMode = Cfg::Server::Mode;
  const bool serialize = Cfg::Repo::Authoritative &&
                         !Cfg::Jit::SerdesFile.empty() &&
                         isJitSerializing();
  const bool serializeOpt = serialize && serializeOptProfEnabled();

  // 1) Obtain function ordering in code.hot.

  if (FuncOrder::get().empty()) {
    BootStats::Block timer("RTA_compute_func_order", Cfg::Server::Mode, true);
    auto const avgProfCount = FuncOrder::compute();
    ProfData::Session pds;
    profData()->setBaseProfCount(avgProfCount);
  } else {
    assertx(isJitDeserializing());
  }
  setInliningMetadata(globalProfData()->baseProfCount(), FuncOrder::getTargetCounts());
  auto const& sortedFuncs = FuncOrder::get();
  auto const nFuncs = sortedFuncs.size();

  // 2) Perform bespoke coloring and finalize the layout hierarchy.
  //    Jumpstart consumers use the coloring computed by the seeder.

  if (allowBespokeArrayLikes()) {
    BootStats::Block timer("RTA_select_bespoke_layouts", Cfg::Server::Mode, true);
    bespoke::selectBespokeLayouts();
  }
  // 3) Stop adding new classes to the "lazy APC classes" list. After we
  //    finalize this list, we can skip lazy deserialization checks for any
  //    classes that are NOT on the list when JIT-ing access to them.

  {
    BootStats::Block timer("RTA_finalize_lazy_apc_classes", Cfg::Server::Mode,
                           true);
    Class::finalizeLazyAPCClasses();
  }

  // 4) Check if we should dump profile data. We may exit here in
  //    SerializeAndExit mode, without really doing the JIT, unless
  //    serialization of optimized code's profile is also enabled.

  if (serialize && !skipSerialize && serializeProfDataAndLog()) return;
  if (!skipSerialize && !isJitSerializing() && !isJitDeserializing() &&
      Cfg::Repo::Authoritative) {
    debugDumpProfDataAndLog();
  }

  // 5) Generate machine code for all the profiled functions.

  auto const initialSize = 512;
  std::vector<tc::FuncMetaInfo> jobs;
  jobs.reserve(nFuncs);
  std::unique_ptr<uint8_t[]> codeBuffer(new uint8_t[nFuncs * initialSize]);

  {
    std::lock_guard<std::mutex> lock{s_dispatcherMutex};
    BootStats::Block timer("RTA_translate_and_relocate",
                           Cfg::Server::Mode, true);
    auto const runParallelRetranslate = [&] {
      {
        Treadmill::Session session(Treadmill::SessionKind::Retranslate);
        auto bufp = codeBuffer.get();
        for (auto i = 0u; i < nFuncs; ++i, bufp += initialSize) {
          auto const fid = sortedFuncs[i];
          auto const func = const_cast<Func*>(Func::fromFuncId(fid));
          if (!Cfg::Jit::SerdesDebugFunctions.empty()) {
            // Only run specified functions
            if (!Cfg::Jit::SerdesDebugFunctions.
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
    };
    runParallelRetranslate();

    if (Cfg::Jit::RerunRetranslateAll) {
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
        clearCachedInliningMetadata();
      }
      runParallelRetranslate();
    }

    // 6) Relocate the machine code into code.hot in the desired order
    if (arch() == Arch::ARM && Cfg::Jit::RuntimeCallReorder) {
      reorderByRuntimeCallDensity(jobs);
      if (Trace::moduleEnabled(Trace::mcg, 6)) {
        dumpRuntimeCallInfo(jobs);
      }
    }

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
      BootStats::done(Cfg::Server::LogBootStats);
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
    if (!Cfg::Eval::EnableReusableTC) {
      discardProfData();
      PropertyProfile::clear();
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
  // If running with ROAR, we call __roar_api_trigger_warmup(), which will reset
  // ROAR's profile counters and start collecting profile data for the specified
  // collection period before optimizing the code.
  // Note that, when ROAR does its own jumpstart, it'll have already produced
  // optimized code at this point and, in this case, we don't want to trigger
  // another warmup cycle.  This situation is detected by checking that
  // __roar_api_pending_warmups() returns 0.
  if (use_roar) {
    always_assert(__roar_api_pending_warmups);
    if (__roar_api_pending_warmups() != 0) {
      if (serverMode) {
        Logger::Info("Calling __roar_api_trigger_warmup (retranslateAll "
                     "finished)");
      }
      always_assert(__roar_api_trigger_warmup);
      __roar_api_trigger_warmup();
    } else {
      if (serverMode) {
        Logger::Info("Skipping call to __roar_api_trigger_warmup due to no "
                     "pending warmups");
      }
    }
  } else {
    if (__llvm_profile_reset_counters) {
      if (serverMode) {
        Logger::Info("Calling __llvm_profile_reset_counters (retranslateAll "
                     "finished)");
      }
      __llvm_profile_reset_counters();
    }
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

Optional<SBInvOffset> offsetAtLocation(SrcKey sk) {
  auto const f = sk.func();
  auto const end = sk.pc();
  auto const start = f->entry();

  std::vector<std::pair<PC, SBInvOffset>> toProcess;
  hphp_fast_set<PC> seen;

  for (auto& pi : f->params()) {
    if (pi.funcletOff != kInvalidOffset) {
      toProcess.emplace_back(start + pi.funcletOff, SBInvOffset{0});
    }
  }

  for (auto& eh : f->ehtab()) {
    toProcess.emplace_back(start + eh.m_handler, SBInvOffset{0});
  }

  // Process start first
  toProcess.emplace_back(start, SBInvOffset{0});

  while (!toProcess.empty()) {
    auto [pc, off] = toProcess.back();
    toProcess.pop_back();
    if (!seen.emplace(pc).second) continue;

    while (true) {
      if (pc == end) return off;

      off += instrNumPushes(pc);
      off -= instrNumPops(pc);

      auto const targets = instrJumpTargets(start, pc - start);
      for (auto t : targets) toProcess.emplace_back(start + t, off);
      if (!instrAllowsFallThru(peek_op(pc))) break;
      pc += instrLen(pc);
    }
  }

  return {};
}

std::string debug_translate_live(SrcKey sk,
                                 const std::vector<std::string>& types) {
  auto off = offsetAtLocation(sk);
  if (!off) return "Unable to compute stack offset for start location";
  if (!tc::createSrcRec(sk, *off)) return "Unable to create SrcRec";
  RegionContext ctx{sk, *off};

  for (auto& tloc : types) {
    std::string locStr, typeStr;
    if (!folly::split('|', tloc, locStr, typeStr)) {
      return folly::to<std::string>("Cannot parse guard: ", tloc);
    }

    int32_t id;
    Optional<Location> loc;
    if (sscanf(locStr.data(), "loc(%i)", &id) == 1) {
      loc = Location::Local{static_cast<uint32_t>(id)};
    } else if (sscanf(locStr.data(), "stk(%i)", &id) == 1) {
      loc = Location::Stack{BCSPRelOffset{id}.to<SBInvOffset>(*off)};
    } else {
      return folly::to<std::string>("Cannot parse guard location: ", locStr);
    }

    DataType dt;
#define DT(t, ...) else if (typeStr == tname(DataType::t)) dt = DataType::t;
    if (false) {}
    DATATYPES
    else {
      return folly::to<std::string>("Cannot parse guard type: ", typeStr);
    }
#undef DT

    ctx.liveTypes.emplace_back(*loc, Type{dt});
  }

  auto const res = mcgen::retranslate(TransArgs{sk}, ctx);
  switch (res.scope()) {
  case TranslationResult::Scope::Success: return "";
  case TranslationResult::Scope::Transient:
    return "Encountered transient retranslate failure";
  case TranslationResult::Scope::Request:
    return "Encountered request persistent retranslate failure";
  case TranslationResult::Scope::Process:
    return "Encountered process persistent retranslate failure";
  }
}

bool retranslateOpt(FuncId funcId) {
  VMProtect _;

  if (Cfg::Jit::DisabledByVSDebug && isDebuggerAttachedProcess()) {
    return false;
  }

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

void optimizeFunc(Func* func) {
  tc::FuncMetaInfo info(func, tc::LocalTCBuffer());
  optimize(info);
  tc::publishOptFunc(std::move(info));
  tc::checkFreeProfData();
}

bool retranslateAllEnabled() {
  return
    Cfg::Jit::PGO &&
    Cfg::Jit::RetranslateAllRequest != 0 &&
    Cfg::Jit::RetranslateAllSeconds != 0;
}

void checkRetranslateAll(bool force, bool skipSerialize) {
  assertx(IMPLIES(skipSerialize, force));

  if (s_retranslateAllScheduled.load(std::memory_order_acquire) ||
      !retranslateAllEnabled()) {
    assertx(!force);
    return;
  }

  auto const serverMode = Cfg::Server::Mode;
  if (!force) {
    auto const uptime = static_cast<int>(HHVM_FN(server_uptime)()); // may be -1
    if (uptime >= (int)Cfg::Jit::RetranslateAllSeconds) {
      assertx(serverMode);
      Logger::FInfo("retranslateAll: scheduled after {} seconds", uptime);
    } else if (requestCount() >= Cfg::Jit::RetranslateAllRequest) {
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
                             Cfg::Server::Mode, true);
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
  if (s_retranslateAllComplete.load(std::memory_order_acquire)) {
    return 0;
  }
  if (auto disp = s_dispatcher.load(std::memory_order_acquire)) {
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
  if (!s_serializeOptProfScheduled.load(std::memory_order_acquire) ||
      s_serializeOptProfTriggered.load(std::memory_order_acquire)) {
    return;
  }

  assertx(Cfg::Repo::Authoritative &&
          !Cfg::Jit::SerdesFile.empty() &&
          isJitSerializing());

  auto const uptime = HHVM_FN(server_uptime)(); // may be -1
  auto const triggerSeconds =
    s_serializeOptProfSeconds.load(std::memory_order_acquire);
  auto const triggerRequest =
    s_serializeOptProfRequest.load(std::memory_order_acquire);
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
    auto const serverMode = Cfg::Server::Mode;
    if (serverMode) {
      Logger::FInfo("retranslateAll: serialization of optimized code's "
                    "profile triggered");
    }

    auto const errMsg = serializeOptProfData(Cfg::Jit::SerdesFile);

    if (serverMode) {
      if (errMsg.empty()) {
        Logger::FInfo("retranslateAll: serializeOptProfData completed");
      } else {
        Logger::FInfo("retranslateAll: serializeOptProfData failed: {}",
                      errMsg);
      }
    }

    // When running under ROAR, we trigger ROAR's PGO/CSPGO after
    // retranslate-all, and we want to wait for ROAR to finish generating its
    // PGO/CSPGO code so that it can serialize that code to the jumpstart file
    // on disk.
    if (use_roar) {
      if (__roar_api_cspgo_enabled && __roar_api_cspgo_enabled()) {
        __roar_api_wait_for_cspgo();
      } else if (__roar_api_pgo_enabled && __roar_api_pgo_enabled()) {
        __roar_api_wait_for_pgo();
      }
      if (__roar_api_finalize_jumpstart_serialization) {
        __roar_api_finalize_jumpstart_serialization();
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
