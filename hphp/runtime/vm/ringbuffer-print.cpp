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
#include "hphp/runtime/vm/ringbuffer-print.h"

#include <iostream>

#include <folly/Format.h>

#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace Trace {

void dumpEntry(const RingBufferEntry* e) {
  if (e->m_type == RBTypeUninit) return;

  std::cerr <<
    folly::format("{:#x} {:10} {:20}",
                  e->m_threadId, e->m_seq, ringbufferName(e->m_type));
  auto const msgFormat = "{:50} {:#16x}\n";

  switch (e->m_type) {
    case RBTypeUninit: return;
    case RBTypeMsg:
    case RBTypeFuncPrologue: {
      // The strings in thread-private ring buffers are not null-terminated;
      // we also can't trust their length, since they might wrap around.
      auto len = std::min(size_t(e->m_len), strlen(e->m_msg));

      // We append our own newline so ignore any newlines in the msg.
      while (len > 0 && e->m_msg[len - 1] == '\n') --len;
      std::cerr <<
        folly::format(msgFormat,
                      folly::StringPiece(e->m_msg, e->m_msg + len),
                      e->m_truncatedRip);
      break;
    }
    case RBTypeFuncEntry:
    case RBTypeFuncExit: {
      static __thread int indentDepth = 0;
      // Quick and dirty attempt at dtrace -F style function nesting.
      // Looks like:
      //
      //    ... FuncEntry    caller
      //    ... FuncEntry        callee
      //    ... FuncExit         callee
      //    ... FuncExit     caller
      //
      // Take this indentation with a grain of salt; it's only reliable
      // within a single thread, and since we still miss some function
      // entries and exits can get confused.
      indentDepth -= e->m_type == RBTypeFuncExit;
      if (indentDepth < 0) indentDepth = 0;
      auto const indentedName =
        folly::sformat("{}{}", std::string(indentDepth * 4, ' '), e->m_msg);
      std::cerr << folly::format(msgFormat,
                                 indentedName, e->m_truncatedRip);
      indentDepth += e->m_type == RBTypeFuncEntry;
      break;
    }
    case RBTypeServiceReq: {
      auto req = static_cast<jit::ServiceRequest>(e->m_sk);
      std::cerr << folly::format(msgFormat,
                                 jit::svcreq::to_name(req), e->m_data);
      break;
    }
    default: {
      std::cerr <<
        folly::format(msgFormat,
                      showShort(SrcKey::fromAtomicInt(e->m_sk)),
                      e->m_data);
      break;
    }
  }
}

// From gdb:
//    (gdb) set language c++
//    (gdb) call HPHP::Trace::dumpRingBuffer(100)
//
//    or
//
//    (gdb) call HPHP::Trace::dumpRingBufferMasked(100,
//       (1 << HPHP::Trace::RBTypeFuncEntry))
void dumpRingBufferMasked(int numEntries, uint32_t types, uint32_t threadId) {
  if (!g_ring_ptr) return;
  int startIdx = (g_ringIdx.load() - numEntries) % kMaxRBEntries;
  while (startIdx < 0) {
    startIdx += kMaxRBEntries;
  }
  assert(startIdx >= 0 && startIdx < kMaxRBEntries);
  int numDumped = 0;
  for (int i = 0; i < kMaxRBEntries && numDumped < numEntries; i++) {
    RingBufferEntry* rb = &g_ring_ptr[(startIdx + i) % kMaxRBEntries];
    if ((1 << rb->m_type) & types &&
        (!threadId || threadId == rb->m_threadId)) {
      numDumped++;
      dumpEntry(rb);
    }
  }
}

KEEP_SECTION
void dumpRingBuffer(int numEntries, uint32_t threadId) {
  dumpRingBufferMasked(numEntries, -1u, threadId);
}

} }
