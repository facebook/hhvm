// Copyright 2013, ARM Limited
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

#include "hphp/vixl/a64/cpu-a64.h"
#include "hphp/vixl/utils.h"

namespace vixl {

// Initialise to smallest possible cache size.
unsigned CPU::dcache_line_size_ = 1;
unsigned CPU::icache_line_size_ = 1;


// Currently computes I and D cache line size.
void CPU::SetUp() {
  uint32_t cache_type_register = GetCacheType();

  // The cache type register holds information about the caches, including I
  // D caches line size.
  static const int kDCacheLineSizeShift = 16;
  static const int kICacheLineSizeShift = 0;
  static const uint32_t kDCacheLineSizeMask = 0xf << kDCacheLineSizeShift;
  static const uint32_t kICacheLineSizeMask = 0xf << kICacheLineSizeShift;

  // The cache type register holds the size of the I and D caches as a power of
  // two.
  uint32_t dcache_line_size_power_of_two =
      (cache_type_register & kDCacheLineSizeMask) >> kDCacheLineSizeShift;
  uint32_t icache_line_size_power_of_two =
      (cache_type_register & kICacheLineSizeMask) >> kICacheLineSizeShift;

  dcache_line_size_ = 1 << dcache_line_size_power_of_two;
  icache_line_size_ = 1 << icache_line_size_power_of_two;
}


uint32_t CPU::GetCacheType() {
#ifdef USE_SIMULATOR
  // This will lead to a cache with 1 byte long lines, which is fine since the
  // simulator will not need this information.
  return 0;
#else
  uint32_t cache_type_register;
  // Copy the content of the cache type register to a core register.
  __asm__ __volatile__ ("mrs %[ctr], ctr_el0"  // NOLINT
                        : [ctr] "=r" (cache_type_register));
  return cache_type_register;
#endif
}


void CPU::EnsureIAndDCacheCoherency(void *address, size_t length) {
#ifdef USE_SIMULATOR
  USE(address);
  USE(length);
  // TODO: consider adding cache simulation to ensure every address run has been
  // synchronised.
#else
  // The code below assumes user space cache operations are allowed.

  uintptr_t start = reinterpret_cast<uintptr_t>(address);
  // Sizes will be used to generate a mask big enough to cover a pointer.
  uintptr_t dsize = static_cast<uintptr_t>(dcache_line_size_);
  uintptr_t isize = static_cast<uintptr_t>(icache_line_size_);
  // Cache line sizes are always a power of 2.
  assert(CountSetBits(dsize, 64) == 1);
  assert(CountSetBits(isize, 64) == 1);
  uintptr_t dstart = start & ~(dsize - 1);
  uintptr_t istart = start & ~(isize - 1);
  uintptr_t end = start + length;

  __asm__ __volatile__ (  // NOLINT
    // Clean every line of the D cache containing the target data.
    "0:                                \n\t"
    // dc      : Data Cache maintenance
    //    c    : Clean
    //     va  : by (Virtual) Address
    //       u : to the point of Unification
    // The point of unification for a processor is the point by which the
    // instruction and data caches are guaranteed to see the same copy of a
    // memory location. See ARM DDI 0406B page B2-12 for more information.
    "dc   cvau, %[dline]                \n\t"
    "add  %[dline], %[dline], %[dsize]  \n\t"
    "cmp  %[dline], %[end]              \n\t"
    "b.lt 0b                            \n\t"
    // Barrier to make sure the effect of the code above is visible to the rest
    // of the world.
    // dsb    : Data Synchronisation Barrier
    //    ish : Inner SHareable domain
    // The point of unification for an Inner Shareable shareability domain is
    // the point by which the instruction and data caches of all the processors
    // in that Inner Shareable shareability domain are guaranteed to see the
    // same copy of a memory location.  See ARM DDI 0406B page B2-12 for more
    // information.
    "dsb  ish                           \n\t"
    // Invalidate every line of the I cache containing the target data.
    "1:                                 \n\t"
    // ic      : instruction cache maintenance
    //    i    : invalidate
    //     va  : by address
    //       u : to the point of unification
    "ic   ivau, %[iline]                \n\t"
    "add  %[iline], %[iline], %[isize]  \n\t"
    "cmp  %[iline], %[end]              \n\t"
    "b.lt 1b                            \n\t"
    // Barrier to make sure the effect of the code above is visible to the rest
    // of the world.
    "dsb  ish                           \n\t"
    // Barrier to ensure any prefetching which happened before this code is
    // discarded.
    // isb : Instruction Synchronisation Barrier
    "isb                                \n\t"
    : [dline] "+r" (dstart),
      [iline] "+r" (istart)
    : [dsize] "r"  (dsize),
      [isize] "r"  (isize),
      [end]   "r"  (end)
    // This code does not write to memory but without the dependency gcc might
    // move this code before the code is generated.
    : "cc", "memory"
  );  // NOLINT
#endif
}

}  // namespace vixl
