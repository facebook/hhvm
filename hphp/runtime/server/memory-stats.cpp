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

#include "hphp/runtime/server/memory-stats.h"

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/util/process.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

namespace HPHP {

decltype(MemoryStats::s_allocCounts) MemoryStats::s_allocCounts;
decltype(MemoryStats::s_allocSizes) MemoryStats::s_allocSizes;

void MemoryStats::ReportMemory(std::string& output, Writer::Format format) {
  std::ostringstream out;
  std::unique_ptr<Writer> w;
  if (format == Writer::Format::XML) {
    w.reset(new XMLWriter(out));
  } else if (format == Writer::Format::HTML) {
    w.reset(new HTMLWriter(out));
  } else {
    assertx(format == Writer::Format::JSON);
    w.reset(new JSONWriter(out));
  }

  w->writeFileHeader();

  ProcStatus::update();
  if (!ProcStatus::valid()) {
    w->writeFileFooter();
    return;
  }
  w->beginObject("Memory");

  w->writeEntry("VmSize", ProcStatus::VmSizeKb.load(std::memory_order_relaxed));
  w->writeEntry("VmRSS", ProcStatus::VmRSSKb.load(std::memory_order_relaxed));
  w->writeEntry("mem.rss", ProcStatus::VmRSSKb.load(std::memory_order_relaxed));
  w->writeEntry("PeakUsage",
                ProcStatus::VmHWMKb.load(std::memory_order_relaxed));
  w->writeEntry("VmSwap", ProcStatus::VmSwapKb.load(std::memory_order_relaxed));
  w->writeEntry("HugetlbPages",
                ProcStatus::HugetlbPagesKb.load(std::memory_order_relaxed));
  w->writeEntry("adjustedRSS", ProcStatus::adjustedRssKb());
  w->writeEntry("mem.rss_adjusted", ProcStatus::adjustedRssKb());

  w->writeEntry("static_string_count", makeStaticStringCount());
  w->writeEntry("static_string_size", TotalSize(AllocKind::StaticString));
  w->writeEntry("static_array_count", loadedStaticArrayCount());
  w->writeEntry("static_array_size", TotalSize(AllocKind::StaticArray));
  w->writeEntry("unit_size", TotalSize(AllocKind::Unit));
  w->writeEntry("class_size", TotalSize(AllocKind::Class));
  w->writeEntry("func_size", TotalSize(AllocKind::Func));

  w->endObject("Memory");
  w->writeFileFooter();

  output = out.str();
  return;
}

namespace {

ServiceData::CounterCallback s_counters(
  [](std::map<std::string, int64_t>& counters) {
    counters["mem.low-mapped"] = alloc::getLowMapped();
    // this isn't really a counter, but whatever. we need a way for callers
    // to query if this build is limited by lowptr memory or not
    counters["mem.use-low-ptr"] = use_lowptr ? 1 : 0;
    counters["mem.unit-size"] = MemoryStats::TotalSize(AllocKind::Unit);
    counters["mem.func-size"] = MemoryStats::TotalSize(AllocKind::Func);
    counters["mem.class-size"] = MemoryStats::TotalSize(AllocKind::Class);
    counters["mem.unit-count"] = MemoryStats::Count(AllocKind::Unit);
    counters["mem.func-count"] = MemoryStats::Count(AllocKind::Func);
    counters["mem.class-count"] = MemoryStats::Count(AllocKind::Class);

    counters["mem.static-string-count"] = makeStaticStringCount();
    counters["mem.static-array-count"] = loadedStaticArrayCount();
    counters["mem.static-string-size"] =
      MemoryStats::TotalSize(AllocKind::StaticString);
    counters["mem.static-array-size"] =
      MemoryStats::TotalSize(AllocKind::StaticArray);

    counters["mem.struct-layout-count"] = bespoke::numStructLayouts();

    counters["mem.huge-tlb-pages-kb"] =
      ProcStatus::HugetlbPagesKb.load(std::memory_order_relaxed);
    counters["mem.vm-size-kb"] =
      ProcStatus::VmSizeKb.load(std::memory_order_relaxed);
    counters["mem.vm-rss-kb"] =
      ProcStatus::VmRSSKb.load(std::memory_order_relaxed);
    counters["mem.peak-usage-kb"] =
      ProcStatus::VmHWMKb.load(std::memory_order_relaxed);
    counters["mem.vm-swap-kb"] =
      ProcStatus::VmSwapKb.load(std::memory_order_relaxed);
    if (ProcStatus::valid()) {
      counters["mem.vm-rss-adjusted-kb"] = ProcStatus::adjustedRssKb();
    }
  }
);

}
}
