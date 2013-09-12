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

#include <stdlib.h>
#include <gtest/gtest.h>

#include "hphp/vixl/a64/decoder-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"


namespace vixl {


TEST(Fuzz, decoder) {
  // Feed noise into the decoder to check that it doesn't crash.
  // 43 million = ~1% of the instruction space.
  static const int instruction_count = 43 * 1024 * 1024;

  uint16_t seed[3] = {1, 2, 3};
  seed48(seed);

  Decoder decoder;
  Instruction buffer[kInstructionSize];

  for (int i = 0; i < instruction_count; i++) {
    uint32_t instr = mrand48();
    buffer->SetInstructionBits(instr);
    decoder.Decode(buffer);
  }
}

TEST(Fuzz, disasm) {
  // Feed noise into the disassembler to check that it doesn't crash.
  // 9 million = ~0.2% of the instruction space.
  static const int instruction_count = 9 * 1024 * 1024;

  uint16_t seed[3] = {42, 43, 44};
  seed48(seed);

  Decoder decoder;
  Disassembler disasm;
  Instruction buffer[kInstructionSize];

  decoder.AppendVisitor(&disasm);
  for (int i = 0; i < instruction_count; i++) {
    uint32_t instr = mrand48();
    buffer->SetInstructionBits(instr);
    decoder.Decode(buffer);
  }
}

#if 0
// These tests are commented out as they take a long time to run, causing the
// test script to timeout. After enabling them, they are best run individually
// using cctest:
//
//     cctest_sim FUZZ_decoder_pedantic
//     cctest_sim FUZZ_disasm_pedantic
//
// or cctest_sim_g for debug builds.

TEST(Fuzz, decoder_pedantic) {
  // Test the entire instruction space.
  Decoder decoder;
  Instruction buffer[kInstructionSize];

  for (uint64_t i = 0; i < (1UL << 32); i++) {
    if ((i & 0xffffff) == 0) {
      fprintf(stderr, "0x%08" PRIx32 "\n", static_cast<uint32_t>(i));
    }
    buffer->SetInstructionBits(static_cast<uint32_t>(i));
    decoder.Decode(buffer);
  }
}

TEST(Fuzz, disasm_pedantic) {
  // Test the entire instruction space. Warning: takes about 30 minutes on a
  // high-end CPU.
  Decoder decoder;
  PrintDisassembler disasm(stdout);
  Instruction buffer[kInstructionSize];

  decoder.AppendVisitor(&disasm);
  for (uint64_t i = 0; i < (1UL << 32); i++) {
    if ((i & 0xffff) == 0) {
      fprintf(stderr, "0x%08" PRIx32 "\n", static_cast<uint32_t>(i));
    }
    buffer->SetInstructionBits(static_cast<uint32_t>(i));
    decoder.Decode(buffer);
  }
}
#endif

}   // namespace vixl
