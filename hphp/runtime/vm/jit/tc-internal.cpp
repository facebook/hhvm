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

#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/mutex.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <atomic>

namespace HPHP { namespace jit { namespace tc {

///////////////////////////////////////////////////////////////////////////////

namespace {

std::atomic<uint64_t> s_numTrans;
CodeCache* s_code{nullptr};
SimpleMutex s_codeLock{false, RankCodeCache};
SimpleMutex s_metadataLock{false, RankCodeMetadata};
__thread size_t s_initialTCSize;
UniqueStubs s_ustubs;
SrcDB s_srcDB;

bool shouldPGOFunc(const Func& func) {
  if (profData() == nullptr) return false;

  // JITing pseudo-mains requires extra checks that blow the IR.  PGO
  // can significantly increase the size of the regions, so disable it for
  // pseudo-mains (so regions will be just tracelets).
  if (func.isPseudoMain()) return false;

  if (!RuntimeOption::EvalJitPGOHotOnly) return true;
  return func.attrs() & AttrHot;
}

}

bool canTranslate() {
  return s_numTrans.load(std::memory_order_relaxed) <
    RuntimeOption::EvalJitGlobalTranslationLimit;
}

const StaticString
  s_php_errormsg("php_errormsg"),
  s_http_response_header("http_response_header");

bool shouldTranslateNoSizeLimit(const Func* func) {
  // If we've hit Eval.JitGlobalTranslationLimit, then we stop translating.
  if (!canTranslate()) {
    return false;
  }

  // Do not translate functions from units marked as interpret-only.
  if (func->unit()->isInterpretOnly()) {
    return false;
  }

  /*
   * We don't support JIT compiling functions that use some super-dynamic php
   * variables.
   */
  if (func->lookupVarId(s_php_errormsg.get()) != -1 ||
      func->lookupVarId(s_http_response_header.get()) != -1) {
    return false;
  }

  return true;
}

static std::atomic_flag s_did_log = ATOMIC_FLAG_INIT;

bool shouldTranslate(const Func* func, TransKind kind) {
  if (!shouldTranslateNoSizeLimit(func)) return false;

  auto const main_under = code().main().used() < CodeCache::AMaxUsage;
  auto const cold_under = code().cold().used() < CodeCache::AColdMaxUsage;
  auto const froz_under = code().frozen().used() < CodeCache::AFrozenMaxUsage;

  // Otherwise, follow the Eval.JitAMaxUsage limits.  However, we do allow PGO
  // translations past that limit if there's still space in code.hot.
  if (main_under && cold_under && froz_under) return true;

  switch (kind) {
    case TransKind::ProfPrologue:
    case TransKind::Profile:
    case TransKind::OptPrologue:
    case TransKind::Optimize:
      return code().hotEnabled();
    default:
      break;
  }

  if (main_under && !s_did_log.test_and_set()) {
    // If we ran out of TC space in cold or frozen but not in main, something
    // unexpected is happening and we should take note of it.
    if (!cold_under && RuntimeOption::EvalProfBranchSampleFreq == 0) {
      // We skip logging cold-full if TC branch profiling is on, since it
      // causes us to fill up cold code at a much higher rate.
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

void assertOwnsCodeLock() { s_codeLock.assertOwnedBySelf(); }
void assertOwnsMetadataLock() { s_metadataLock.assertOwnedBySelf(); }

void requestInit() {
  tl_regState = VMRegState::CLEAN;
  Timer::RequestInit();
  memset(&tl_perf_counters, 0, sizeof(tl_perf_counters));
  Stats::init();
  requestInitProfData();
  s_initialTCSize = s_code->totalUsed();
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
  now = s_code->totalUsed();
}

void processInit() {
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  s_code = new CodeCache();
  s_ustubs.emitAll(*s_code, *Debug::DebugInfo::Get());

  // Write an .eh_frame section that covers the whole TC.
  initUnwinder(s_code->base(), s_code->codeSize());
}

CodeCache& code() {
  assert(s_code);
  return *s_code;
}
const UniqueStubs& ustubs() { return s_ustubs; }
SrcDB& srcDB() { return s_srcDB; }

TCA offsetToAddr(uint32_t off) { return s_code->toAddr(off); }
uint32_t addrToOffset(CTCA addr) { return s_code->toOffset(addr); }

bool isValidCodeAddress(TCA addr) {
  return s_code->isValidCodeAddress(addr);
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
  if (profData() &&
      !RuntimeOption::EvalEnableReusableTC &&
      code().main().used() >= CodeCache::AMaxUsage &&
      (!code().hotEnabled() ||
       profData()->profilingFuncs() == profData()->optimizedFuncs()) &&
      !transdb::enabled()) {
    discardProfData();
  }
}

bool profileSrcKey(SrcKey sk) {
  if (!shouldPGOFunc(*sk.func())) return false;
  if (profData()->optimized(sk.funcID())) return false;
  if (profData()->profiling(sk.funcID())) return true;

  // Don't start profiling new functions if the size of either main or
  // prof is already above Eval.JitAMaxUsage and we already filled hot.
  auto tcUsage = std::max(code().main().used(), code().prof().used());
  if (tcUsage >= CodeCache::AMaxUsage && !code().hotEnabled()) {
    return false;
  }

  // We have two knobs to control the number of functions we're allowed to
  // profile: Eval.JitProfileRequests and Eval.JitProfileBCSize. We profile new
  // functions until either of these limits is exceeded. In practice we expect
  // to hit the bytecode size limit first but we keep the request limit around
  // as a safety net.
  if (RuntimeOption::EvalJitProfileBCSize > 0 &&
      profData()->profilingBCSize() >= RuntimeOption::EvalJitProfileBCSize) {
    return false;
  }

  return requestCount() <= RuntimeOption::EvalJitProfileRequests;
}

///////////////////////////////////////////////////////////////////////////////

}}}
