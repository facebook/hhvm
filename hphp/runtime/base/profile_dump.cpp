/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/profile_dump.h"

#include "folly/Format.h"
#include "folly/Conv.h"

namespace HPHP {

std::string ProfileDump::toPProfFormat() const {
  size_t currentCountSum = 0, currentBytesSum = 0;
  size_t accumCountSum   = 0, accumBytesSum   = 0;

  // aggregate for totals at top
  for (const auto &current : m_currentlyAllocated) {
    currentCountSum += current.second.m_count;
    currentBytesSum += current.second.m_bytes;
  }
  for (const auto &accum : m_accumAllocated) {
    accumCountSum += accum.second.m_count;
    accumBytesSum += accum.second.m_bytes;
  }

  std::string res;
  // print header, with the following format
  // heap profile: curobjs: curbytes [accumobjs: accumbytes] @ heapprofile
  folly::toAppend(
    folly::format(
      "heap profile: {}: {} [{}: {}] @ heapprofile\n",
      currentCountSum, currentBytesSum, accumCountSum, accumBytesSum
    ).str(), &res
  );

  // print information for allocations we have recorded for each call stack
  // each line has the following format
  // curobjs: curbytes [accumobjs: accumbytes] stacktrace...
  for (const auto &accum : m_accumAllocated) {
    const auto &trace = accum.first;
    // skip this information if we have a zero-length stack trace, because
    // that means we allocated this outside of PHP userland.
    if (trace.size() == 0) continue;
    // get current allocation count/bytes for the current stack trace. we
    // know this is present in the current allocations map because we did
    // insert it at the same time we inserted something into the cumulative
    // allocations map
    const auto &it = m_currentlyAllocated.find(trace);
    assert(it != m_currentlyAllocated.end());
    // dump current and cumulative count/bytes
    folly::toAppend(
      folly::format(
        "{}: {} [{}: {}] @",
        it->second.m_count, it->second.m_bytes,
        accum.second.m_count, accum.second.m_bytes
      ).str(), &res
    );
    // walk stack trace and pack srckeys into 64-bit ints so
    // they look like addresses
    for (const auto &sk : trace) {
      folly::toAppend(folly::format(" {:#x}", sk.toAtomicInt()), &res);
    }
    folly::toAppend("\n", &res);
  }

  // dump maps from /proc/pid/maps. we aren't going to use them because
  // we are going to manually resolve symbols ourselves later, and the
  // addresses we are dumping as part of the stack trace aren't even real
  // addresses anyway
  size_t buflen = 64;
  folly::toAppend("\nMAPPED_LIBRARIES:\n", &res);
  char buf[buflen];
  snprintf(buf, buflen, "/proc/%d/maps", getpid());
  FILE *f = fopen(buf, "r");
  size_t bytesRead;
  while ((bytesRead = fread(buf, 1, buflen, f)) > 0) {
    folly::toAppend(folly::StringPiece(buf, bytesRead), &res);
  }
  fclose(f);
  return res;
}

}
