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

#include "hphp/runtime/base/heap-profiling.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-usage-stats.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/util/logger.h"

namespace HPHP {

RDS_LOCAL(AllocSamples, s_samples);

///////////////////////////////////////////////////////////////////////////////

/*
 * Heap allocation sampling: get a sample every EvalHeapAllocSampleBytes of heap
 * allocation (without considering deallocation at all for now). The sample
 * contains both PHP stack and native stack (to help understand the nature of
 * the allocations).
 */
void MemoryManager::checkSampling(size_t bytes) {
  auto const allocated = m_stats.mmAllocated();
  if (allocated < m_nextSample) return;
  auto const usage = m_stats.mmUsage();
  s_samples->addAllocSample(usage, bytes);
  if (RuntimeOption::EvalHeapAllocSampleNativeStack) {
    // Get native stacktrace here.
    StackTrace st(StackTrace::Force{});
    s_samples->back().nativeStack = st;
  }
  assertx(RuntimeOption::EvalHeapAllocSampleBytes > 0);
  do {
    m_nextSample += RuntimeOption::EvalHeapAllocSampleBytes;
  } while (m_nextSample < allocated);
  // Gather PHP stack later.
  setSurpriseFlag(SurpriseFlag::HeapSamplingFlag);
}

void AllocSamples::addStack(bool skipTop) {
  if (empty()) {
    Logger::Error("no prior sampled allocation?");
    return;
  }
  if (back().phpStack) {
    Logger::Error("sampled allocation already has associated php stack");
    return;
  }
  auto stack = CompactTraceData::Create();
  fillCompactBacktrace(stack.get(), skipTop);
  for (auto iter = rbegin(); iter != rend(); ++iter) {
    if (iter->phpStack) break;
    iter->phpStack = stack;
  }
  const StaticString s_DYNO_SCRIPT_FILENAME("DYNO_SCRIPT_FILENAME");
  auto const name = ServerNote::Get(s_DYNO_SCRIPT_FILENAME);
  if (!name.empty()) {
    script = name.toCppString();
  }
}

void AllocSamples::logSamples() {
  if (empty()) return;
  if (!StructuredLog::enabled()) return;
  for (auto& sample : *this) {
    StructuredLogEntry entry;
    entry.setInt("size", sample.size);
    entry.setInt("total_alloc", sample.time);
    if (sample.phpStack) {
      std::vector<folly::StringPiece> stack;
      auto const& frames = sample.phpStack->frames();
      unsigned frameCount = 0;
      for (auto const& frame : frames) {
        auto const func = frame.func;
        auto const name = func->fullName()->slice();
        if (!stack.empty() && (stack.back() == name)) continue;
        stack.emplace_back(func->fullName()->slice());
        if (++frameCount > 255) break;    // too deep
      }
      entry.setVec("php_stack", stack);
    }
    entry.setStackTrace("native_stack", sample.nativeStack);
    entry.setStr("script", script);
    StructuredLog::log("hhvm_xenon_heap_profiler", entry);
  }
  clear();
}

void gather_alloc_stack(bool skipTop) {
  s_samples->addStack(skipTop);
  clearSurpriseFlag(HeapSamplingFlag);
}

void reset_alloc_sampling() {
  s_samples->requestShutdown();
}

}
