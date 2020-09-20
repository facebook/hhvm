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

#pragma once

#include "hphp/runtime/base/backtrace.h"
#include "hphp/util/alloc-defs.h"
#include "hphp/util/stack-trace.h"

#include <string>
#include <vector>

namespace HPHP {

struct HeapAllocSample {
  HeapAllocSample(size_t c, size_t s) : curr(c), size(s) {}
  size_t curr;                          // total alloc so far
  size_t size;                          // size of current allocation
  CompactTraceData::Ptr phpStack{nullptr};
  StackTrace nativeStack{false};
};

struct AllocSamples : public std::vector<HeapAllocSample,
                                         LocalAllocator<HeapAllocSample>> {
  TYPE_SCAN_IGNORE_ALL;

  void requestShutdown() {
    if (empty()) return;
    logSamples();
    clear();
  }
  void addAllocSample(size_t curr, size_t size) {
    emplace_back(curr, size);
  }
  void addStack(bool skipTop = false);

 private:
  void logSamples();
};

}

