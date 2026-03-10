// Copyright 2018, VIXL authors
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

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64

#include "utils-vixl.h"

#include "simulator-aarch64.h"

namespace vixl {
namespace aarch64 {

// Randomly generated example keys for simulating only.
const Simulator::PACKey Simulator::kPACKeyIA = {0xc31718727de20f71,
                                                0xab9fd4e14b2fec51,
                                                0};
const Simulator::PACKey Simulator::kPACKeyIB = {0xeebb163b474e04c8,
                                                0x5267ac6fc280fb7c,
                                                1};
const Simulator::PACKey Simulator::kPACKeyDA = {0x5caef808deb8b1e2,
                                                0xd347cbc06b7b0f77,
                                                0};
const Simulator::PACKey Simulator::kPACKeyDB = {0xe06aa1a949ba8cc7,
                                                0xcfde69e3db6d0432,
                                                1};

// The general PAC key isn't intended to be used with AuthPAC so we ensure the
// key number is invalid and asserts if used incorrectly.
const Simulator::PACKey Simulator::kPACKeyGA = {0xfcd98a44d564b3d5,
                                                0x6c56df1904bf0ddc,
                                                -1};

static uint64_t GetNibble(uint64_t in_data, int position) {
  return (in_data >> position) & 0xf;
}

static uint64_t ShuffleNibbles(uint64_t in_data) {
  static int in_positions[16] =
      {4, 36, 52, 40, 44, 0, 24, 12, 56, 60, 8, 32, 16, 28, 20, 48};
  uint64_t out_data = 0;
  for (int i = 0; i < 16; i++) {
    out_data |= GetNibble(in_data, in_positions[i]) << (4 * i);
  }
  return out_data;
}

static uint64_t SubstituteNibbles(uint64_t in_data) {
  // Randomly chosen substitutes.
  static uint64_t subs[16] =
      {4, 7, 3, 9, 10, 14, 0, 1, 15, 2, 8, 6, 12, 5, 11, 13};
  uint64_t out_data = 0;
  for (int i = 0; i < 16; i++) {
    int index = (in_data >> (4 * i)) & 0xf;
    out_data |= subs[index] << (4 * i);
  }
  return out_data;
}

// Rotate nibble to the left by the amount specified.
static uint64_t RotNibble(uint64_t in_cell, int amount) {
  VIXL_ASSERT((amount >= 0) && (amount <= 3));

  in_cell &= 0xf;
  uint64_t temp = (in_cell << 4) | in_cell;
  return (temp >> (4 - amount)) & 0xf;
}

static uint64_t BigShuffle(uint64_t in_data) {
  uint64_t out_data = 0;
  for (int i = 0; i < 4; i++) {
    uint64_t n12 = GetNibble(in_data, 4 * (i + 12));
    uint64_t n8 = GetNibble(in_data, 4 * (i + 8));
    uint64_t n4 = GetNibble(in_data, 4 * (i + 4));
    uint64_t n0 = GetNibble(in_data, 4 * (i + 0));

    uint64_t t0 = RotNibble(n8, 2) ^ RotNibble(n4, 1) ^ RotNibble(n0, 1);
    uint64_t t1 = RotNibble(n12, 1) ^ RotNibble(n4, 2) ^ RotNibble(n0, 1);
    uint64_t t2 = RotNibble(n12, 2) ^ RotNibble(n8, 1) ^ RotNibble(n0, 1);
    uint64_t t3 = RotNibble(n12, 1) ^ RotNibble(n8, 1) ^ RotNibble(n4, 2);

    out_data |= t3 << (4 * (i + 0));
    out_data |= t2 << (4 * (i + 4));
    out_data |= t1 << (4 * (i + 8));
    out_data |= t0 << (4 * (i + 12));
  }
  return out_data;
}

// A simple, non-standard hash function invented for simulating. It mixes
// reasonably well, however it is unlikely to be cryptographically secure and
// may have a higher collision chance than other hashing algorithms.
uint64_t Simulator::ComputePAC(uint64_t data, uint64_t context, PACKey key) {
  uint64_t working_value = data ^ key.high;
  working_value = BigShuffle(working_value);
  working_value = ShuffleNibbles(working_value);
  working_value ^= key.low;
  working_value = ShuffleNibbles(working_value);
  working_value = BigShuffle(working_value);
  working_value ^= context;
  working_value = SubstituteNibbles(working_value);
  working_value = BigShuffle(working_value);
  working_value = SubstituteNibbles(working_value);

  return working_value;
}

// The TTBR is selected by bit 63 or 55 depending on TBI for pointers without
// codes, but is always 55 once a PAC code is added to a pointer. For this
// reason, it must be calculated at the call site.
uint64_t Simulator::CalculatePACMask(uint64_t ptr, PointerType type, int ttbr) {
  int bottom_pac_bit = GetBottomPACBit(ptr, ttbr);
  int top_pac_bit = GetTopPACBit(ptr, type);
  return ExtractUnsignedBitfield64(top_pac_bit,
                                   bottom_pac_bit,
                                   0xffffffffffffffff & ~kTTBRMask)
         << bottom_pac_bit;
}

uint64_t Simulator::AuthPAC(uint64_t ptr,
                            uint64_t context,
                            PACKey key,
                            PointerType type) {
  VIXL_ASSERT((key.number == 0) || (key.number == 1));

  uint64_t pac_mask = CalculatePACMask(ptr, type, (ptr >> 55) & 1);
  uint64_t original_ptr =
      ((ptr & kTTBRMask) == 0) ? (ptr & ~pac_mask) : (ptr | pac_mask);

  uint64_t pac = ComputePAC(original_ptr, context, key);

  uint64_t error_code = uint64_t{1} << key.number;
  if ((pac & pac_mask) == (ptr & pac_mask)) {
    return original_ptr;
  } else {
    int error_lsb = GetTopPACBit(ptr, type) - 2;
    uint64_t error_mask = UINT64_C(0x3) << error_lsb;
    return (original_ptr & ~error_mask) | (error_code << error_lsb);
  }
}

uint64_t Simulator::AddPAC(uint64_t ptr,
                           uint64_t context,
                           PACKey key,
                           PointerType type) {
  int top_pac_bit = GetTopPACBit(ptr, type);

  // TODO: Properly handle the case where extension bits are bad and TBI is
  // turned off, and also test me.
  VIXL_ASSERT(HasTBI(ptr, type));
  int ttbr = (ptr >> 55) & 1;
  uint64_t pac_mask = CalculatePACMask(ptr, type, ttbr);
  uint64_t ext_ptr = (ttbr == 0) ? (ptr & ~pac_mask) : (ptr | pac_mask);

  uint64_t pac = ComputePAC(ext_ptr, context, key);

  // If the pointer isn't all zeroes or all ones in the PAC bitfield, corrupt
  // the resulting code.
  if (((ptr & (pac_mask | kTTBRMask)) != 0x0) &&
      ((~ptr & (pac_mask | kTTBRMask)) != 0x0)) {
    pac ^= UINT64_C(1) << (top_pac_bit - 1);
  }

  uint64_t ttbr_shifted = static_cast<uint64_t>(ttbr) << 55;
  return (pac & pac_mask) | ttbr_shifted | (ptr & ~pac_mask);
}

uint64_t Simulator::StripPAC(uint64_t ptr, PointerType type) {
  uint64_t pac_mask = CalculatePACMask(ptr, type, (ptr >> 55) & 1);
  return ((ptr & kTTBRMask) == 0) ? (ptr & ~pac_mask) : (ptr | pac_mask);
}
}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_INCLUDE_SIMULATOR_AARCH64
