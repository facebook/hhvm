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

#ifndef incl_HPHP_MEMORY_PROFILE_H_
#define incl_HPHP_MEMORY_PROFILE_H_

#include "hphp/runtime/base/profile-dump.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/crash-reporter.h"

namespace HPHP {

struct MemoryProfile {
  static DECLARE_THREAD_LOCAL(MemoryProfile, s_memory_profile);

  // structs
  struct Allocation {
    // Represents an allocation event. Allocation events are uniquely
    // identified by a pointer and a generation, where the generation
    // is bumped when a pointer is reallocated to something else.
    size_t            m_size;
    ProfileStackTrace m_trace;
  };

  // Start profiling
  static inline void startProfiling() {
    if (!RuntimeOption::HHProfServerEnabled) return;
    s_memory_profile->startProfilingImpl();
  }
  // Dumps profiled data
  static inline void finishProfiling() {
    if (!RuntimeOption::HHProfServerEnabled) return;
    s_memory_profile->finishProfilingImpl();
  }
  // Log allocation event
  static inline void logAllocation(void *ptr, size_t size) {
    if (!RuntimeOption::HHProfServerEnabled || IsCrashing) return;
    s_memory_profile->logAllocationImpl(ptr, size);
  }
  // Log deallocation event
  static inline void logDeallocation(void *ptr) {
    if (!RuntimeOption::HHProfServerEnabled || IsCrashing) return;
    s_memory_profile->logDeallocationImpl(ptr);
  }

  // Gets the size of the memory block most recently allocated at
  // the given pointer.
  static size_t getSizeOfPtr(void *ptr);
  // Gets the amount of heap memory owned by a TypedValue.
  static size_t getSizeOfTV(TypedValue *tv);

private:
  // implementations
  void startProfilingImpl();
  void finishProfilingImpl();
  void logAllocationImpl(void *ptr, size_t size);
  void logDeallocationImpl(void *ptr);

  // some helpers to dive into compound TVs
  static size_t getSizeOfArray(ArrayData *arr);
  static size_t getSizeOfObject(ObjectData *obj);

  // Map of live allocations, keyed on their pointers.
  std::map<void *, Allocation> m_livePointers;
  // Profile dump of the current thread's request.
  ProfileDump m_dump;
};

}
#endif // incl_HPHP_MEMORY_PROFILE_H_
