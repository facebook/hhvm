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

void MCGenerator::syncWork() {
  assertx(tl_regState != VMRegState::CLEAN);
  m_fixupMap.fixup(g_context.getNoCheck());
  tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}

MCGenerator::MCGenerator() {
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

  auto optCatchBlock = getCatchTrace(TCA(fp->m_savedRip));
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

///////////////////////////////////////////////////////////////////////////////

}}
