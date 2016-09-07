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

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include <cinttypes>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <unwind.h>
#endif

#include <algorithm>
#include <exception>
#include <memory>
#include <queue>
#include <string>
#include <strstream>
#include <unordered_set>
#include <vector>

#include <folly/Optional.h>

#include "hphp/util/process.h"
#include "hphp/util/build-info.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/debug-guards.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/source-root-info.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(mcg);

using namespace Trace;

// The global MCGenerator object.
MCGenerator* mcg;

static __thread size_t s_initialTCSize;

///////////////////////////////////////////////////////////////////////////////

bool dumpTCAnnotation(const Func& func, TransKind transKind) {
  return RuntimeOption::EvalDumpTCAnnotationsForAllTrans ||
    (transKind == TransKind::Optimize && (func.attrs() & AttrHot));
}

int MCGenerator::numTranslations(SrcKey sk) const {
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    return sr->translations().size();
  }
  return 0;
}

void MCGenerator::syncWork() {
  assertx(tl_regState != VMRegState::CLEAN);
  m_fixupMap.fixup(g_context.getNoCheck());
  tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}

MCGenerator::MCGenerator()
  : m_catchTraceMap(128)
  , m_literals(128)
{
  TRACE(1, "MCGenerator@%p startup\n", this);
  mcg = this;

  g_unwind_rds.bind();

  static bool profileUp = false;
  if (!profileUp) {
    profileInit();
    profileUp = true;
  }

  if (Trace::moduleEnabledRelease(Trace::printir) &&
      !RuntimeOption::EvalJit) {
    Trace::traceRelease("TRACE=printir is set but the jit isn't on. "
                        "Did you mean to run with -vEval.Jit=1?\n");
  }

  m_ustubs.emitAll(m_code, m_debugInfo);

  // Write an .eh_frame section that covers the whole TC.
  EHFrameWriter ehfw;
  write_tc_cie(ehfw);
  ehfw.begin_fde(m_code.base());
  ehfw.end_fde(m_code.codeSize());
  ehfw.null_fde();

  m_ehFrames.push_back(ehfw.register_and_release());
}

folly::Optional<TCA> MCGenerator::getCatchTrace(CTCA ip) const {
  auto const found = m_catchTraceMap.find(m_code.toOffset(ip));
  if (found && *found != kInvalidCatchTrace) return m_code.toAddr(*found);
  return folly::none;
}

void codeEmittedThisRequest(size_t& requestEntry, size_t& now) {
  requestEntry = s_initialTCSize;
  now = mcg->code().totalUsed();
}

namespace {
__thread std::unordered_map<const ActRec*, TCA>* tl_debuggerCatches{nullptr};
}

void stashDebuggerCatch(const ActRec* fp) {
  if (!tl_debuggerCatches) {
    tl_debuggerCatches = new std::unordered_map<const ActRec*, TCA>();
  }

  auto optCatchBlock = mcg->getCatchTrace(TCA(fp->m_savedRip));
  always_assert(optCatchBlock && *optCatchBlock);
  auto catchBlock = *optCatchBlock;
  FTRACE(1, "Pushing debugger catch {} with fp {}\n", catchBlock, fp);
  tl_debuggerCatches->emplace(fp, catchBlock);
}

TCA unstashDebuggerCatch(const ActRec* fp) {
  always_assert(tl_debuggerCatches);
  auto const it = tl_debuggerCatches->find(fp);
  always_assert(it != tl_debuggerCatches->end());
  auto const catchBlock = it->second;
  tl_debuggerCatches->erase(it);
  FTRACE(1, "Popped debugger catch {} for fp {}\n", catchBlock, fp);
  return catchBlock;
}

void MCGenerator::requestInit() {
  tl_regState = VMRegState::CLEAN;
  Timer::RequestInit();
  memset(&tl_perf_counters, 0, sizeof(tl_perf_counters));
  Stats::init();
  requestInitProfData();
  s_initialTCSize = m_code.totalUsed();
  assert(!g_unwind_rds.isInit());
  memset(g_unwind_rds.get(), 0, sizeof(UnwindRDS));
  g_unwind_rds.markInit();
}

void MCGenerator::requestExit() {
  always_assert(!Translator::WriteLease().amOwner());
  TRACE_MOD(txlease, 2, "%" PRIx64 " write lease stats: %15" PRId64
            " kept, %15" PRId64 " grabbed\n",
            Process::GetThreadIdForTrace(), Translator::WriteLease().hintKept(),
            Translator::WriteLease().hintGrabbed());
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

  delete tl_debuggerCatches;
  tl_debuggerCatches = nullptr;
}

MCGenerator::~MCGenerator() {
}

bool MCGenerator::addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterates through whole SrcDB...
  struct timespec tsBegin, tsEnd;
  {
    auto codeLock = lockCode();
    auto metaLock = lockMetadata();

    auto code = m_code.view();
    auto& main = code.main();
    auto& data = code.data();

    HPHP::Timer::GetMonotonicTime(tsBegin);
    // Doc says even find _could_ invalidate iterator, in pactice it should
    // be very rare, so go with it now.
    CGMeta fixups;
    for (auto& pair : m_srcDB) {
      SrcKey const sk = SrcKey::fromAtomicInt(pair.first);
      // We may have a SrcKey to a deleted function. NB: this may miss a
      // race with deleting a Func. See task #2826313.
      if (!Func::isFuncIdValid(sk.funcID())) continue;
      SrcRec* sr = pair.second;
      if (sr->unitMd5() == unit->md5() &&
          !sr->hasDebuggerGuard() &&
          m_tx.isSrcKeyInBL(sk)) {
        addDbgGuardImpl(sk, sr, main, data, fixups);
      }
    }
    fixups.process(nullptr);
  }

  HPHP::Timer::GetMonotonicTime(tsEnd);
  int64_t elapsed = gettime_diff_us(tsBegin, tsEnd);
  if (Trace::moduleEnabledRelease(Trace::mcg, 5)) {
    Trace::traceRelease("addDbgGuards got lease for %" PRId64 " us\n", elapsed);
  }
  return true;
}

bool MCGenerator::addDbgGuard(const Func* func, Offset offset, bool resumed) {
  SrcKey sk(func, offset, resumed);
  {
    if (SrcRec* sr = m_srcDB.find(sk)) {
      if (sr->hasDebuggerGuard()) {
        return true;
      }
    } else {
      // no translation yet
      return true;
    }
  }
  if (debug) {
    if (!m_tx.isSrcKeyInBL(sk)) {
      TRACE(5, "calling addDbgGuard on PC that is not in blacklist");
      return false;
    }
  }

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  CGMeta fixups;
  if (SrcRec* sr = m_srcDB.find(sk)) {
    auto code = m_code.view();
    addDbgGuardImpl(sk, sr, code.main(), code.data(), fixups);
  }
  fixups.process(nullptr);
  return true;
}

bool MCGenerator::dumpTCCode(const char* filename) {
#define OPEN_FILE(F, SUFFIX)                                    \
  std::string F ## name = std::string(filename).append(SUFFIX); \
  FILE* F = fopen(F ## name .c_str(),"wb");                     \
  if (F == nullptr) return false;                               \
  SCOPE_EXIT{ fclose(F); };

  OPEN_FILE(ahotFile,       "_ahot");
  OPEN_FILE(aFile,          "_a");
  OPEN_FILE(aprofFile,      "_aprof");
  OPEN_FILE(acoldFile,      "_acold");
  OPEN_FILE(afrozenFile,    "_afrozen");
  OPEN_FILE(helperAddrFile, "_helpers_addrs.txt");

#undef OPEN_FILE

  // dump starting from the hot region
  auto result = true;
  auto writeBlock = [&](const CodeBlock& cb, FILE* file) {
    if (result) {
      auto const count = cb.used();
      result = fwrite(cb.base(), 1, count, file) == count;
    }
  };

  writeBlock(m_code.hot(), ahotFile);
  writeBlock(m_code.main(), aFile);
  writeBlock(m_code.prof(), aprofFile);
  writeBlock(m_code.cold(), acoldFile);
  writeBlock(m_code.frozen(), afrozenFile);
  return result;
}

bool MCGenerator::dumpTC(bool ignoreLease /* = false */) {
  std::unique_lock<SimpleMutex> codeLock;
  std::unique_lock<SimpleMutex> metaLock;
  if (!ignoreLease) {
    codeLock = lockCode();
    metaLock = lockMetadata();
  }
  return dumpTCData() && dumpTCCode("/tmp/tc_dump");
}

// Returns true on success
bool MCGenerator::dumpTCData() {
  gzFile tcDataFile = gzopen("/tmp/tc_data.txt.gz", "w");
  if (!tcDataFile) return false;

  if (!gzprintf(tcDataFile,
                "repo_schema      = %s\n"
                "ahot.base        = %p\n"
                "ahot.frontier    = %p\n"
                "a.base           = %p\n"
                "a.frontier       = %p\n"
                "aprof.base       = %p\n"
                "aprof.frontier   = %p\n"
                "acold.base       = %p\n"
                "acold.frontier   = %p\n"
                "afrozen.base     = %p\n"
                "afrozen.frontier = %p\n\n",
                repoSchemaId().begin(),
                m_code.hot().base(), m_code.hot().frontier(),
                m_code.main().base(), m_code.main().frontier(),
                m_code.prof().base(), m_code.prof().frontier(),
                m_code.cold().base(), m_code.cold().frontier(),
                m_code.frozen().base(), m_code.frozen().frontier())) {
    return false;
  }

  if (!gzprintf(tcDataFile, "total_translations = %zu\n\n",
                m_tx.getNumTranslations())) {
    return false;
  }

  // Print all translations, including their execution counters. If global
  // counters are disabled (default), fall back to using ProfData, covering
  // only profiling translations.
  if (!RuntimeOption::EvalJitTransCounters && Translator::isTransDBEnabled()) {
    // Admin requests do not automatically init ProfData, so do it explicitly.
    // No need for matching exit call; data is immortal with trans DB enabled.
    requestInitProfData();
  }
  for (TransID t = 0; t < m_tx.getNumTranslations(); t++) {
    int64_t count = 0;
    if (RuntimeOption::EvalJitTransCounters) {
      count = m_tx.getTransCounter(t);
    } else if (auto prof = profData()) {
      assertx(m_tx.getTransCounter(t) == 0);
      count = prof->transCounter(t);
    }
    if (gzputs(tcDataFile, m_tx.getTransRec(t)->print(count).c_str()) == -1) {
      return false;
    }
  }

  gzclose(tcDataFile);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

}}
