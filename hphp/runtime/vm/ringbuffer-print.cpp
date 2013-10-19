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

#include "hphp/runtime/vm/ringbuffer-print.h"

#include "folly/Format.h"

#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace Trace {

void dumpEntry(const RingBufferEntry* e) {
  std::cerr <<
    folly::format("{:#x} {:10} {:20}",
                  e->m_threadId, e->m_seq, ringbufferName(e->m_type));

  switch (e->m_type) {
    case RBTypeUninit: return;
    case RBTypeMsg:
    case RBTypeFuncPrologueTry: {
      // The strings in thread-private ring buffers are not null-terminated;
      // we also can't trust their length, since they might wrap around.
      fwrite(e->m_msg,
             std::min(size_t(e->m_len), strlen(e->m_msg)),
             1,
             stderr);
      fprintf(stderr, "\n");
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
      std::cerr << folly::format("{}{}\n",
                                 std::string(indentDepth * 4, ' '), e->m_msg);
      indentDepth += e->m_type == RBTypeFuncEntry;
      break;
    }
    default: {
      std::cerr <<
        folly::format("{:50} {:#16x}\n",
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
void dumpRingBufferMasked(int numEntries, uint32_t types) {
  int startIdx = (g_ringIdx.load() - numEntries) % kMaxRBEntries;
  while (startIdx < 0) {
    startIdx += kMaxRBEntries;
  }
  assert(startIdx >= 0 && startIdx < kMaxRBEntries);
  int numDumped = 0;
  for (int i = 0; i < kMaxRBEntries && numDumped < numEntries; i++) {
    RingBufferEntry* rb = &g_ring[(startIdx + i) % kMaxRBEntries];
    if ((1 << rb->m_type) & types) {
      numDumped++;
      dumpEntry(rb);
    }
  }
}

KEEP_SECTION
void dumpRingBuffer(int numEntries) {
  dumpRingBufferMasked(numEntries, -1u);
}

} }
