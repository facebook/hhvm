// Copyright 2015, VIXL authors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "test-utils.h"

#include <string.h>

extern "C" {
#include <sys/mman.h>
}

#include "globals-vixl.h"

#ifdef VIXL_INCLUDE_TARGET_AARCH64
#include "aarch64/cpu-aarch64.h"
#endif

namespace vixl {

// BSD uses `MAP_ANON` instead of the Linux `MAP_ANONYMOUS`. The `MAP_ANONYMOUS`
// alias should generally be available, but is not always, so define it manually
// if necessary.
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif


void ExecuteMemory(byte* buffer, size_t size, int byte_offset) {
  void (*test_function)(void);

  VIXL_ASSERT((byte_offset >= 0) && (static_cast<size_t>(byte_offset) < size));
  VIXL_STATIC_ASSERT(sizeof(buffer) == sizeof(test_function));
  VIXL_STATIC_ASSERT(sizeof(uintptr_t) == sizeof(test_function));
  uintptr_t entry_point = reinterpret_cast<uintptr_t>(buffer);
  entry_point += byte_offset;
  memcpy(&test_function, &entry_point, sizeof(test_function));

  USE(size);

#if defined(__aarch64__) && defined(VIXL_INCLUDE_TARGET_AARCH64)
  aarch64::CPU::EnsureIAndDCacheCoherency(buffer, size);
#elif defined(__arm__) && \
    (defined(VIXL_INCLUDE_TARGET_A32) || defined(VIXL_INCLUDE_TARGET_T32))
  // TODO: Do not use __builtin___clear_cache and instead implement
  // `CPU::EnsureIAndDCacheCoherency` for aarch32.
  __builtin___clear_cache(buffer, reinterpret_cast<char*>(buffer) + size);
#else
  // This helper requires a native (non-Simulator) environment.
  VIXL_ABORT_WITH_MSG("Cannot ExecuteMemory(...): unsupported platform.");
#endif
  test_function();
}

}  // namespace vixl
