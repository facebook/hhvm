/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "util/ringbuffer.h"
#include "util/pathtrack.h"
#include "util/util.h"
#include "util/atomic.h"
#include "util/assertions.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace HPHP {
namespace Trace {

/*
 * Thread-private ascii ringbuffer.
 */
static const int kMaxRBBytes = 1 << 20; // 1MB
__thread int  tl_rbPtr;
__thread char tl_ring[kMaxRBBytes];
__thread const char _unused[] = "\n----END OF RINGBUFFER---\n";

void vtraceRingbuffer(const char* fmt, va_list ap) {
  char buf[4096];
  char* bufP = &buf[0];
  // Silently truncate long inputs.
  int msgBytes = std::min(int(sizeof(buf)) - 1,
                          vsnprintf(buf, sizeof(buf) - 1, fmt, ap));
  // Remember these for the binary ringbuffer.
  char* start = &tl_ring[tl_rbPtr];
  int totalLen = msgBytes;
  // Include the nulls; we will sometimes include these strings
  // by reference from the global ringbuffer.
  buf[msgBytes++] = '\0';
  while(msgBytes) {
    int leftInCurPiece = kMaxRBBytes - tl_rbPtr;
    int toWrite = std::min(msgBytes, leftInCurPiece);
    memcpy(&tl_ring[tl_rbPtr], bufP, toWrite);
    msgBytes -= toWrite;
    bufP += toWrite;
    tl_rbPtr = (tl_rbPtr + toWrite) % kMaxRBBytes;
  }
  ringbufferMsg(start, totalLen);
}

// From GDB:
//  (gdb) call HPHP::Trace::dumpRingBuffer()
void dumpRingbuffer() {
  write(1, tl_ring + tl_rbPtr, kMaxRBBytes - tl_rbPtr);
  write(1, tl_ring, tl_rbPtr);
}

/*
 * Thread-shared, binary ringbuffer. Includes thread-private ASCII
 * ringbuffers by reference. Beware that very old ASCII entries can
 * be corrupt; still, this is better than nothing.
 */
struct RingBufferEntry {
  // 0 - 7
  uint32_t m_threadId;
  uint16_t m_type;
  uint16_t m_offset;

  // 8 - 15
  uint64_t m_funcId;

  // 16-31
  const char* m_msg;
  uint32_t m_len; // m_msg and m_len are specific to Msg
  uint32_t m_ts;  // low-order bits of timestamp
};

static const int kMaxRBEntries = (1 << 20); // Must exceed number of threads
RingBufferEntry g_ring[kMaxRBEntries];
volatile int g_ringIdx;

RingBufferEntry*
allocEntry(RingBufferType t) {
  ASSERT(Util::isPowerOfTwo(kMaxRBEntries));
  RingBufferEntry* rb;
  int newRingPos, oldRingPos;
  do {
    oldRingPos = g_ringIdx;
    rb = &g_ring[oldRingPos];
    newRingPos = (oldRingPos + 1) % kMaxRBEntries;
  } while (!atomic_cas(&g_ringIdx, oldRingPos, newRingPos));
  rb->m_ts = uint32_t(_rdtsc());
  rb->m_type = t;
  rb->m_threadId = (uint32_t)((int64_t)pthread_self() & 0xFFFFFFFF);
  return rb;
}

static inline RingBufferEntry*
initEntry(RingBufferType t) {
  RingBufferEntry* rb = allocEntry(t);
  return rb;
}

static inline RingBufferEntry*
initEntry(RingBufferType t, uint64_t funcId, int offset) {
  RingBufferEntry* rb = allocEntry(t);
  rb->m_funcId = funcId;
  rb->m_offset = offset;
  return rb;
}

void
ringbufferMsg(const char* msg, size_t msgLen, RingBufferType t) {
  RingBufferEntry* rb = initEntry(t);
  rb->m_msg = msg;
  rb->m_len = msgLen;
}

void
ringbufferEntry(RingBufferType t, uint64_t funcId, int offset) {
  (void) initEntry(t, funcId, offset);
}

void dumpEntry(const RingBufferEntry* e) {
  static const char* names[] = {
#define RBTYPE(x) #x,
    RBTYPES
#undef RBTYPE
  };
  switch(e->m_type) {
    case RBTypeUninit: return;
    case RBTypeMsg: {
      printf("%#x %10u ", e->m_threadId, e->m_ts);
      // The strings in thread-private ring buffers are not null-terminated;
      // we also can't trust their length, since they might wrap around.
      fwrite(e->m_msg,
             std::min(size_t(e->m_len), strlen(e->m_msg)),
             1,
             stdout);
      printf("\n");
      break;
    }
    case RBTypeFuncEntry:
    case RBTypeFuncExit: {
      static __thread int indentDepth;
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
      printf("%#x %10u %20s %*s%s\n",
             e->m_threadId, e->m_ts,
             names[e->m_type], 4*indentDepth, " ", e->m_msg);
      indentDepth += e->m_type == RBTypeFuncEntry;
      break;
    }
    default: {
      printf("%#x %10u %#lx %d %20s\n",
             e->m_threadId, e->m_ts, e->m_funcId, e->m_offset,
             names[e->m_type]);
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
  int startIdx = (g_ringIdx - numEntries) % kMaxRBEntries;
  while (startIdx < 0) {
    startIdx += kMaxRBEntries;
  }
  ASSERT(startIdx >= 0 && startIdx < kMaxRBEntries);
  int numDumped = 0;
  for (int i = 0; i < kMaxRBEntries && numDumped < numEntries; i++) {
    RingBufferEntry* rb = &g_ring[(startIdx + i) % kMaxRBEntries];
    if ((1 << rb->m_type) & types) {
      numDumped++;
      dumpEntry(rb);
    }
  }
}

void dumpRingBuffer(int numEntries) {
  dumpRingBufferMasked(numEntries, -1u);
}

}
}
