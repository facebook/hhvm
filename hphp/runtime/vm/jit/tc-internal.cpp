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

#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/disasm.h"
#include "hphp/util/mutex.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <atomic>

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

CodeCache* g_code{nullptr};
SrcDB g_srcDB;
UniqueStubs g_ustubs;

///////////////////////////////////////////////////////////////////////////////

namespace {

std::atomic<uint64_t> s_numTrans;
SimpleMutex s_codeLock{false, RankCodeCache};
SimpleMutex s_metadataLock{false, RankCodeMetadata};
__thread size_t s_initialTCSize;

bool shouldPGOFunc(const Func* func) {
  if (profData() == nullptr) return false;

  // JITing pseudo-mains requires extra checks that blow the IR.  PGO
  // can significantly increase the size of the regions, so disable it for
  // pseudo-mains (so regions will be just tracelets).
  if (func->isPseudoMain()) return false;

  if (!RuntimeOption::EvalJitPGOHotOnly) return true;
  return func->attrs() & AttrHot;
}

}

TransLoc TransRange::loc() const {
  TransLoc loc;
  loc.setMainStart(main.begin());
  loc.setColdStart(cold.begin() - sizeof(uint32_t));
  loc.setFrozenStart(frozen.begin() - sizeof(uint32_t));
  loc.setMainSize(main.size());

  assertx(loc.coldCodeSize() == cold.size());
  assertx(loc.frozenCodeSize() == frozen.size());
  return loc;
}

bool canTranslate() {
  return s_numTrans.load(std::memory_order_relaxed) <
    RuntimeOption::EvalJitGlobalTranslationLimit;
}

const StaticString
  s_php_errormsg("php_errormsg"),
  s_http_response_header("http_response_header");

bool shouldTranslateNoSizeLimit(const Func* func, TransKind kind) {
  // If we've hit Eval.JitGlobalTranslationLimit, then we stop translating.
  if (!canTranslate()) {
    return false;
  }

  // Do not translate functions from units marked as interpret-only.
  if (func->unit()->isInterpretOnly()) {
    return false;
  }

  // We don't support JIT compiling functions that use some super-dynamic php
  // variables.
  if (func->lookupVarId(s_php_errormsg.get()) != -1 ||
      func->lookupVarId(s_http_response_header.get()) != -1) {
    return false;
  }

  // Refuse to JIT Live translations if Eval.JitPGOOnly is enabled.
  if (RuntimeOption::EvalJitPGOOnly &&
      (kind == TransKind::Live || kind == TransKind::LivePrologue)) {
    return false;
  }

  return true;
}

static std::atomic_flag s_did_log = ATOMIC_FLAG_INIT;
static std::atomic<bool> s_TCisFull{false};

bool shouldTranslate(const Func* func, TransKind kind) {
  if (s_TCisFull.load(std::memory_order_relaxed) ||
      !shouldTranslateNoSizeLimit(func, kind)) {
    return false;
  }

  const auto serverMode = RuntimeOption::ServerExecutionMode();
  const auto maxTransTime = RuntimeOption::EvalJitMaxRequestTranslationTime;
  const auto transCounter = Timer::CounterValue(Timer::mcg_translate);

  if (serverMode && maxTransTime >= 0 &&
      transCounter.wall_time_elapsed >= maxTransTime) {

    if (Trace::moduleEnabledRelease(Trace::mcg, 1)) {
      Trace::traceRelease("Skipping translation. "
                          "Time budget of %" PRId64 " exceeded. "
                          "%" PRId64 "us elapsed. "
                          "%" PRId64 " translations completed\n",
                          maxTransTime,
                          transCounter.wall_time_elapsed,
                          transCounter.count);
    }
    return false;
  }

  auto const main_under = code().main().used() < CodeCache::AMaxUsage;
  auto const cold_under = code().cold().used() < CodeCache::AColdMaxUsage;
  auto const froz_under = code().frozen().used() < CodeCache::AFrozenMaxUsage;

  // Otherwise, follow the Eval.JitAMaxUsage limits.
  if (main_under && cold_under && froz_under) return true;

  // We use cold and frozen for all kinds of translations, but we allow PGO
  // translations past the limit for main if there's still space in code.hot.
  if (cold_under && froz_under) {
    switch (kind) {
      case TransKind::ProfPrologue:
      case TransKind::Profile:
      case TransKind::OptPrologue:
      case TransKind::Optimize:
        return code().hotEnabled();
      default:
        break;
    }
  }

  // Set a flag so we quickly bail from trying to generate new translations next
  // time.
  s_TCisFull.store(true, std::memory_order_relaxed);

  if (main_under && !s_did_log.test_and_set() &&
      RuntimeOption::EvalProfBranchSampleFreq == 0) {
    // If we ran out of TC space in cold or frozen but not in main, something
    // unexpected is happening and we should take note of it.  We skip this
    // logging if TC branch profiling is on, since it fills up code and frozen
    // at a much higher rate.
    if (!cold_under) {
      logPerfWarning("cold_full", 1, [] (StructuredLogEntry&) {});
    }
    if (!froz_under) {
      logPerfWarning("frozen_full", 1, [] (StructuredLogEntry&) {});
    }
  }
  return false;
}

bool newTranslation() {
  if (s_numTrans.fetch_add(1, std::memory_order_relaxed) >=
      RuntimeOption::EvalJitGlobalTranslationLimit) {
    return false;
  }
  return true;
}

std::unique_lock<SimpleMutex> lockCode() {
  return std::unique_lock<SimpleMutex>{s_codeLock};
}

std::unique_lock<SimpleMutex> lockMetadata() {
  return std::unique_lock<SimpleMutex>{s_metadataLock};
}

void assertOwnsCodeLock(OptView v) {
  if (!v || !v->isLocal()) s_codeLock.assertOwnedBySelf();
}
void assertOwnsMetadataLock() { s_metadataLock.assertOwnedBySelf(); }

void requestInit() {
  tl_regState = VMRegState::CLEAN;
  Timer::RequestInit();
  memset(&tl_perf_counters, 0, sizeof(tl_perf_counters));
  Stats::init();
  requestInitProfData();
  s_initialTCSize = g_code->totalUsed();
  assert(!g_unwind_rds.isInit());
  memset(g_unwind_rds.get(), 0, sizeof(UnwindRDS));
  g_unwind_rds.markInit();
}

void requestExit() {
  Stats::dump();
  Stats::clear();
  Timer::RequestExit();
  if (profData()) profData()->maybeResetCounters();
  requestExitProfData();

  if (Trace::moduleEnabledRelease(Trace::mcgstats, 1)) {
    Trace::traceRelease("MCGenerator perf counters for %s:\n",
                        g_context->getRequestUrl(50).c_str());
    for (int i = 0; i < tpc_num_counters; i++) {
      Trace::traceRelease("%-20s %10" PRId64 "\n",
                          kPerfCounterNames[i], tl_perf_counters[i]);
    }
    Trace::traceRelease("\n");
  }

  clearDebuggerCatches();
}

void codeEmittedThisRequest(size_t& requestEntry, size_t& now) {
  requestEntry = s_initialTCSize;
  now = g_code->totalUsed();
}

void processInit() {
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  g_code = new(low_malloc_data(sizeof(CodeCache))) CodeCache();
  g_ustubs.emitAll(*g_code, *Debug::DebugInfo::Get());

  // Write an .eh_frame section that covers the whole TC.
  initUnwinder(g_code->base(), g_code->codeSize());
  Disasm::ExcludedAddressRange(g_code->base(), g_code->codeSize());

  recycleInit();
}

void processExit() {
  recycleStop();
}

bool isValidCodeAddress(TCA addr) {
  return g_code->isValidCodeAddress(addr);
}

bool isProfileCodeAddress(TCA addr) {
  return g_code->prof().contains(addr);
}

void freeTCStub(TCA stub) {
  // We need to lock the code because s_freeStubs.push() writes to the stub and
  // the metadata to protect s_freeStubs itself.
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  assertx(code().frozen().contains(stub));
  Debug::DebugInfo::Get()->recordRelocMap(stub, 0, "FreeStub");

  markStubFreed(stub);
}

void checkFreeProfData() {
  // In PGO mode, we free all the profiling data once the main code area reaches
  // its maximum usage and either the hot area is also full or all the functions
  // that were profiled have already been optimized.
  //
  // However, we keep the data around indefinitely in a few special modes:
  // * Eval.EnableReusableTC
  // * TC dumping enabled (Eval.DumpTC/DumpIR/etc.)
  //
  // Finally, when the RetranslateAll mode is enabled, the ProfData is discarded
  // via a different mechanism, after all the optimized translations are
  // generated.
  if (profData() &&
      !RuntimeOption::EvalEnableReusableTC &&
      code().main().used() >= CodeCache::AMaxUsage &&
      (!code().hotEnabled() ||
       profData()->profilingFuncs() == profData()->optimizedFuncs()) &&
      !transdb::enabled() &&
      !RuntimeOption::EvalJitRetranslateAllRequest) {
    discardProfData();
  }
}

bool shouldProfileNewFuncs() {
  // Don't start profiling new functions if the size of either main or
  // prof is already above Eval.JitAMaxUsage and we already filled hot.
  auto tcUsage = std::max(code().main().used(), code().prof().used());
  if (tcUsage >= CodeCache::AMaxUsage && !code().hotEnabled()) {
    return false;
  }

  // We have two knobs to control the number of functions we're allowed to
  // profile: Eval.JitProfileRequests and Eval.JitProfileBCSize. We profile new
  // functions until either of these limits is exceeded. In practice, we expect
  // to hit the bytecode size limit first, but we keep the request limit around
  // as a safety net.
  if (RuntimeOption::EvalJitProfileBCSize > 0 &&
      profData() &&
      profData()->profilingBCSize() >= RuntimeOption::EvalJitProfileBCSize) {
    return false;
  }

  return requestCount() <= RuntimeOption::EvalJitProfileRequests;
}

bool profileFunc(const Func* func) {
  if (!shouldPGOFunc(func)) return false;

  // If retranslateAll is enabled and we already passed the point that it should
  // be scheduled to execute (via the treadmill), then we can't emit more
  // Profile translations.  This is to ensure that, when retranslateAll() runs,
  // no more Profile translations are being added to ProfData.
  if (RuntimeOption::EvalJitRetranslateAllRequest != 0 &&
      hasEnoughProfDataToRetranslateAll()) {
    return false;
  }

  if (profData()->optimized(func->getFuncId())) return false;

  // If we already started profiling `func', then we return true and skip the
  // other checks below.
  if (profData()->profiling(func->getFuncId())) return true;

  return shouldProfileNewFuncs();
}

///////////////////////////////////////////////////////////////////////////////

LocalTCBuffer::LocalTCBuffer(Address start, size_t initialSize) {
  TCA fakeStart = code().threadLocalStart();
  auto const sz = initialSize / 4;
  auto initBlock = [&] (DataBlock& block, size_t mxSz, const char* nm) {
    always_assert(sz <= mxSz);
    block.init(fakeStart, start, sz, mxSz, nm);
    fakeStart += mxSz;
    start += sz;
  };
  initBlock(m_main, RuntimeOption::EvalThreadTCMainBufferSize,
            "thread local main");
  initBlock(m_cold, RuntimeOption::EvalThreadTCColdBufferSize,
            "thread local cold");
  initBlock(m_frozen, RuntimeOption::EvalThreadTCFrozenBufferSize,
            "thread local frozen");
  initBlock(m_data, RuntimeOption::EvalThreadTCDataBufferSize,
            "thread local data");
}

OptView LocalTCBuffer::view() {
  if (!valid()) return folly::none;
  return CodeCache::View(m_main, m_cold, m_frozen, m_data, true);
}

bool reachedTranslationLimit(TransKind kind, SrcKey sk, const SrcRec& srcRec) {
  const auto numTrans = srcRec.numTrans();

  // Optimized translations perform this check at relocation time to avoid
  // invalidating all of their SrcKeys early.
  if (kind == TransKind::Optimize) return false;

  if ((kind == TransKind::Profile &&
       numTrans != RuntimeOption::EvalJitMaxProfileTranslations) ||
      (kind != TransKind::Profile &&
       numTrans != RuntimeOption::EvalJitMaxTranslations)) {
    return false;
  }
  INC_TPC(max_trans);

  if (debug && Trace::moduleEnabled(Trace::mcg, 2)) {
    auto srLock = srcRec.readlock();
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

////////////////////////////////////////////////////////////////////////////////

}}}
