/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <stdio.h>
#include <unistd.h>
#include <ios>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "hphp/runtime/base/static-string-table.h"

namespace HPHP {

  void MemoryStats::ReportMemory(std::string &output, Writer::Format format) {
    std::ostringstream out;
    Writer *w;
    if (format == Writer::Format::XML) {
      w = new XMLWriter(out);
    } else if (format == Writer::Format::HTML) {
      w = new HTMLWriter(out);
    } else {
      assert(format == Writer::Format::JSON);
      w = new JSONWriter(out);
    }

    w->writeFileHeader();
    // read and fill StatM structure
    StatM procStatM;
    if (!FillProcessStatM(&procStatM)) {
      // failed to read statm file
      w->writeEntry("Success", false);
      w->writeFileFooter();
      return;
    }

    // successfully read statm file
    w->writeEntry("Success", true);

    // Calculate unknown size
    size_t totalBytes = GetStaticStringSize();
    totalBytes += procStatM.m_text;

   w->beginObject("Memory");

    // process memory statistics
    w->beginObject("Process Stats (bytes)");
    w->writeEntry("VmSize", procStatM.m_vmSize);
    w->writeEntry("VmRss", procStatM.m_vmRss);
    w->writeEntry("Shared", procStatM.m_share);
    w->writeEntry("Text(Code)", procStatM.m_text);
    w->writeEntry("Data", procStatM.m_data);
    w->endObject("Process Stats");

    // Start HHVM internal memory buckets
    w->beginObject("Breakdown");

    // static string stats
    w->beginObject("Static Strings");
    w->writeEntry("Bytes", GetStaticStringSize());
    w->writeEntry("Count", makeStaticStringCount());
    w->endObject("Static Strings");

    // code segment stats
    w->beginObject("Code");
    w->writeEntry("Bytes", procStatM.m_text);
    w->endObject("Code");

    // currently unknown portion of vmSize
    w->writeEntry("Unknown", procStatM.m_vmSize - totalBytes);

    w->endObject("Breakdown");
    // End HHVM internal memory buckets

    w->endObject("Memory");
    w->writeFileFooter();
    delete w;

    output = out.str();
    return;
  }

  MemoryStats* MemoryStats::GetInstance() {
    static MemoryStats memoryStatsInstance;
    return &memoryStatsInstance;
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

  bool MemoryStats::FillProcessStatM(MemoryStats::StatM* pStatM) {

    std::ifstream statm_stream("/proc/self/statm", std::ios_base::in);
    if (!statm_stream.good()){
      return false;
    }

    uint64_t pageSizeBytes = sysconf(_SC_PAGE_SIZE);
    size_t deprecatedField;
    statm_stream >> pStatM->m_vmSize >> pStatM->m_vmRss
                 >> pStatM->m_share  >> pStatM->m_text
                 >> deprecatedField  >> pStatM->m_data;

    pStatM->m_vmSize     *= pageSizeBytes;
    pStatM->m_vmRss      *= pageSizeBytes;
    pStatM->m_share      *= pageSizeBytes;
    pStatM->m_text       *= pageSizeBytes;
    pStatM->m_data       *= pageSizeBytes;
    return true;
  }
}
