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
  w->writeEntry("static_string_size", totalSize(AllocKind::StaticString));
  w->writeEntry("static_array_count", loadedStaticArrayCount());
  w->writeEntry("static_array_size", totalSize(AllocKind::StaticArray));
  w->writeEntry("unit_size", totalSize(AllocKind::Unit));
  w->writeEntry("class_size", totalSize(AllocKind::Class));
  w->writeEntry("func_size", totalSize(AllocKind::Func));

  w->endObject("Memory");
  w->writeFileFooter();

  output = out.str();
  return;
}

}
