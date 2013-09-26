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
#ifndef incl_HPHP_RINGBUFFER_H_
#define incl_HPHP_RINGBUFFER_H_

#include <cstdlib>
#include <cstdarg>
#include <cinttypes>

#include "hphp/util/util.h"

namespace HPHP {
namespace Trace {

#define RBTYPES \
  RBTYPE(Uninit) \
  RBTYPE(Msg) \
  RBTYPE(SideExit) \
  RBTYPE(EnterTC) \
  RBTYPE(TraceletBody) \
  RBTYPE(TraceletGuards) \
  RBTYPE(FuncEntry) \
  RBTYPE(FuncExit) \
  RBTYPE(FuncPrologueTry)

enum RingBufferType : uint8_t {
#define RBTYPE(x) RBType ## x,
  RBTYPES
#undef RBTYPE
};

/*
 * Thread-shared, binary ringbuffer. Includes thread-private ASCII ringbuffers
 * by reference. Beware that very old ASCII entries can be corrupt; still, this
 * is better than nothing.
 */
struct RingBufferEntry {
  // 0 - 15
  union {
    struct {
      uint64_t m_sk;
      uint64_t m_data;
    };
    struct {
      const char* m_msg;
      uint32_t m_len;
    };
  };

  // 16-23
  uint32_t m_threadId;
  uint32_t m_seq; // sequence number

  // 24 - 31
  RingBufferType m_type;
};

static_assert(sizeof(RingBufferEntry) == 32,
              "RingBufferEntry expected to be 32 bytes");

constexpr int kMaxRBEntries = (1 << 20); // Must exceed number of threads

extern RingBufferEntry g_ring[];
extern std::atomic<int> g_ringIdx;

const char* ringbufferName(RingBufferType t);
void vtraceRingbuffer(const char* fmt, va_list ap) ATTRIBUTE_PRINTF(1,0);
void ringbufferMsg(const char* msg, size_t msgLen,
                   RingBufferType t = RBTypeMsg);
void ringbufferEntry(RingBufferType t, uint64_t sk, uint64_t data);

}
}

#endif
