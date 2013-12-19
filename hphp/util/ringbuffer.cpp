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

#include "hphp/util/ringbuffer.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "hphp/util/util.h"
#include "hphp/util/assertions.h"

namespace HPHP {
namespace Trace {

/*
 * Thread-private ascii ringbuffer.
 */
static const int kMaxRBBytes = 1 << 19; // 512KB
__thread int  tl_rbPtr;
__thread char tl_ring[kMaxRBBytes];
UNUSED __thread const char _unused[] = "\n----END OF RINGBUFFER---\n";

const char* ringbufferName(RingBufferType t) {
  switch (t) {
#   define RBTYPE(n) case RBType##n: return #n;
    RBTYPES
#   undef RBTYPE
  }
  not_reached();
}

KEEP_SECTION
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

RingBufferEntry g_ring[kMaxRBEntries];
std::atomic<int> g_ringIdx(0);
std::atomic<uint32_t> g_seqnum(0);

RingBufferEntry*
allocEntry(RingBufferType t) {
  assert(Util::isPowerOfTwo(kMaxRBEntries));
  RingBufferEntry* rb;
  int newRingPos, oldRingPos;
  do {
    oldRingPos = g_ringIdx.load(std::memory_order_acquire);
    rb = &g_ring[oldRingPos];
    newRingPos = (oldRingPos + 1) % kMaxRBEntries;
  } while (!g_ringIdx.compare_exchange_weak(oldRingPos, newRingPos,
                                            std::memory_order_acq_rel));
  rb->m_seq = g_seqnum.fetch_add(1, std::memory_order_relaxed);
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
initEntry(RingBufferType t, uint64_t sk, uint64_t data) {
  RingBufferEntry* rb = allocEntry(t);
  rb->m_sk = sk;
  rb->m_data = data;
  return rb;
}

void
ringbufferMsg(const char* msg, size_t msgLen, RingBufferType t) {
  RingBufferEntry* rb = initEntry(t);
  rb->m_msg = msg;
  rb->m_len = msgLen;
}

void
ringbufferEntry(RingBufferType t, uint64_t sk, uint64_t data) {
  (void) initEntry(t, sk, data);
}

}
}
