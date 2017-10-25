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

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include <folly/portability/Unistd.h>

namespace HPHP {

void MemoryStats::ReportMemory(std::string& output, Writer::Format format) {
  std::ostringstream out;
  std::unique_ptr<Writer> w;
  if (format == Writer::Format::XML) {
    w.reset(new XMLWriter(out));
  } else if (format == Writer::Format::HTML) {
    w.reset(new HTMLWriter(out));
  } else {
    assert(format == Writer::Format::JSON);
    w.reset(new JSONWriter(out));
  }

  w->writeFileHeader();

  MemStatus procStatus;                 // read /proc/self/status
  if (!procStatus.valid()) {
    w->writeFileFooter();
    return;
  }

  w->beginObject("Memory");

  w->writeEntry("VmSize", procStatus.VmSize);
  w->writeEntry("VmRSS", procStatus.VmRSS);
  w->writeEntry("PeakUsage", procStatus.VmHWM);
  w->writeEntry("HugetlbPages", procStatus.HugetlbPages);
  w->writeEntry("adjustedRSS", procStatus.adjustedRSS);

  // static string stats
  w->writeEntry("static_string_count", makeStaticStringCount());
  w->writeEntry("static_string_size", GetStaticStringSize());

  w->endObject("Memory");
  w->writeFileFooter();

  output = out.str();
  return;
}

MemoryStats* MemoryStats::GetInstance() {
  static MemoryStats s_memoryStats;
  return &s_memoryStats;
}

void MemoryStats::ResetStaticStringSize() {
  m_staticStringSize.store(0);
}

void MemoryStats::LogStaticStringAlloc(size_t bytes) {
  m_staticStringSize += bytes;
}

size_t MemoryStats::GetStaticStringSize() {
  return m_staticStringSize.load();
}
}
