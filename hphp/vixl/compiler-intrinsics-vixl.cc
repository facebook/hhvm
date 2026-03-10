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

#include "compiler-intrinsics-vixl.h"

#include "utils-vixl.h"

namespace vixl {


int CountLeadingSignBitsFallBack(int64_t value, int width) {
  VIXL_ASSERT(IsPowerOf2(width) && (width <= 64));
  if (width < 64) VIXL_ASSERT(IsIntN(width, value));
  if (value >= 0) {
    return CountLeadingZeros(value, width) - 1;
  } else {
    return CountLeadingZeros(~value, width) - 1;
  }
}


int CountLeadingZerosFallBack(uint64_t value, int width) {
  VIXL_ASSERT(IsPowerOf2(width) && (width <= 64));
  if (value == 0) {
    return width;
  }
  int count = 0;
  value = value << (64 - width);
  if ((value & UINT64_C(0xffffffff00000000)) == 0) {
    count += 32;
    value = value << 32;
  }
  if ((value & UINT64_C(0xffff000000000000)) == 0) {
    count += 16;
    value = value << 16;
  }
  if ((value & UINT64_C(0xff00000000000000)) == 0) {
    count += 8;
    value = value << 8;
  }
  if ((value & UINT64_C(0xf000000000000000)) == 0) {
    count += 4;
    value = value << 4;
  }
  if ((value & UINT64_C(0xc000000000000000)) == 0) {
    count += 2;
    value = value << 2;
  }
  if ((value & UINT64_C(0x8000000000000000)) == 0) {
    count += 1;
  }
  count += (value == 0);
  return count;
}


int CountSetBitsFallBack(uint64_t value, int width) {
  VIXL_ASSERT(IsPowerOf2(width) && (width <= 64));

  // Mask out unused bits to ensure that they are not counted.
  value &= (UINT64_C(0xffffffffffffffff) >> (64 - width));

  // Add up the set bits.
  // The algorithm works by adding pairs of bit fields together iteratively,
  // where the size of each bit field doubles each time.
  // An example for an 8-bit value:
  // Bits:  h  g  f  e  d  c  b  a
  //         \ |   \ |   \ |   \ |
  // value = h+g   f+e   d+c   b+a
  //            \    |      \    |
  // value =   h+g+f+e     d+c+b+a
  //                  \          |
  // value =       h+g+f+e+d+c+b+a
  const uint64_t kMasks[] = {
      UINT64_C(0x5555555555555555),
      UINT64_C(0x3333333333333333),
      UINT64_C(0x0f0f0f0f0f0f0f0f),
      UINT64_C(0x00ff00ff00ff00ff),
      UINT64_C(0x0000ffff0000ffff),
      UINT64_C(0x00000000ffffffff),
  };

  for (unsigned i = 0; i < (sizeof(kMasks) / sizeof(kMasks[0])); i++) {
    int shift = 1 << i;
    value = ((value >> shift) & kMasks[i]) + (value & kMasks[i]);
  }

  return static_cast<int>(value);
}


int CountTrailingZerosFallBack(uint64_t value, int width) {
  VIXL_ASSERT(IsPowerOf2(width) && (width <= 64));
  int count = 0;
  value = value << (64 - width);
  if ((value & UINT64_C(0xffffffff)) == 0) {
    count += 32;
    value = value >> 32;
  }
  if ((value & 0xffff) == 0) {
    count += 16;
    value = value >> 16;
  }
  if ((value & 0xff) == 0) {
    count += 8;
    value = value >> 8;
  }
  if ((value & 0xf) == 0) {
    count += 4;
    value = value >> 4;
  }
  if ((value & 0x3) == 0) {
    count += 2;
    value = value >> 2;
  }
  if ((value & 0x1) == 0) {
    count += 1;
  }
  count += (value == 0);
  return count - (64 - width);
}


}  // namespace vixl
