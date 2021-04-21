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

#include <cstdlib>
#include <cstdarg>
#include <cinttypes>
#include <atomic>

#include "hphp/util/portability.h"

namespace HPHP {
namespace Trace {

#define RBTYPES \
  RBTYPE(Uninit) \
  RBTYPE(Msg) \
  RBTYPE(SideExit) \
  RBTYPE(EnterTC) \
  RBTYPE(ResumeTC) \
  RBTYPE(TraceletBody) \
  RBTYPE(TraceletGuards) \
  RBTYPE(FuncEntry) \
  RBTYPE(FuncExit) \
  RBTYPE(FuncPrologue) \
  RBTYPE(BytecodeStart) \
  RBTYPE(ServiceReq) \
  RBTYPE(DispatchBB) \
  RBTYPE(InterpOne) \
  RBTYPE(APCHandleEnqueue) \
  RBTYPE(APCHandleDelete) \
  RBTYPE(Generic)

enum RingBufferType : uint8_t {
#define RBTYPE(x) RBType ## x,
  RBTYPES
#undef RBTYPE
};

/*
 * Thread-shared, binary ringbuffer.
 */
struct RingBufferEntry {
  // 0 - 15
  union {
    // Used by ringbufferEntry() and ringbufferEntryRip()
    struct {
      uint64_t sk;
      uint64_t data;
    } vmPoint;

    // Used by ringbufferMsg()
    struct {
      // msg references thread-private ASCII ringbuffers. Beware that very old
      // entries can be corrupt; still, this is better than nothing.
      const char* msg;
      uint32_t len;

      // Bottom 32 bits of rip from the caller, which is usually enough in
      // practice.
      uint32_t truncatedRip;
    } msg;

    // used by APCHandle tracing
    struct {
      void* handle;
      void* value;
    } apcHandleInfo;

    // Used by ringbufferGeneric()
    struct {
      const char* name;
      uint64_t data;
    } generic;
  };

  // 16-23
  uint32_t threadId;
  uint32_t seq; // sequence number

  // 24 - 31
  RingBufferType type;
};

static_assert(sizeof(RingBufferEntry) == 32,
              "RingBufferEntry expected to be 32 bytes");

constexpr unsigned kMaxRBEntries = (1 << 20); // Must exceed number of threads

extern RingBufferEntry* g_ring_ptr;
extern std::atomic<int> g_ringIdx;

const char* ringbufferName(RingBufferType t);
void vtraceRingbuffer(ATTRIBUTE_PRINTF_STRING const char* fmt, va_list ap)
  ATTRIBUTE_PRINTF(1,0);
void ringbufferMsg(const char* msg, size_t msgLen,
                   RingBufferType t = RBTypeMsg);
void ringbufferEntry(RingBufferType t, uint64_t sk, uint64_t data);
void ringbufferEntryRip(RingBufferType t, uint64_t sk);
void ringbufferAPCEnqueue(void* handle, void* value);
void ringbufferAPCDelete(void* handle, void* value);
void ringbufferGeneric(const char* name, uint64_t data = 0);
void ringbufferGeneric(const char* name, const void* data);
}
}
