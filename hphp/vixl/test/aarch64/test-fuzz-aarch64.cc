// Copyright 2014, VIXL authors
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

#include <cstdlib>
#include <string>

#include "test-runner.h"

#include "aarch64/decoder-aarch64.h"
#include "aarch64/disasm-aarch64.h"

#define TEST(name) TEST_(AARCH64_FUZZ_##name)


namespace vixl {
namespace aarch64 {

static void FuzzHelper(std::string mode, int step_size, int offset, int shift) {
  Decoder decoder;
  PrintDisassembler disasm(stdout);
  Instruction buffer[kInstructionSize];

  if (mode == "disasm") {
    decoder.AppendVisitor(&disasm);
  } else {
    VIXL_CHECK(mode == "decoder");
  }

  for (uint64_t i = offset << shift; i < (UINT64_C(1) << 32); i += step_size) {
    buffer->SetInstructionBits(static_cast<uint32_t>(i));
    decoder.Decode(buffer);
  }
}

// Number of shards used to split fuzz tests. This value isn't used in the macro
// below, so if you change this, ensure more FUZZ_SHARD instances are
// instantiated.
static const int kShardCount = 16;

// Test approximately 1% of the instruction space for the decoder, and 0.2% for
// the disassembler. Multiply the step size by the number of shards issued.
static const int kDecoderStep = 100 * kShardCount + 1;
static const int kDisasmStep = 500 * kShardCount + 1;

// Shift the offset argument into the top-level opcode bits, which helps to
// spread the fuzz coverage across instruction classes.
static const int kOpFieldShift = 25;

#define FUZZ_SHARD(mode, step, i, shift) \
  TEST(mode##_##i) { FuzzHelper(#mode, step, i, shift); }

FUZZ_SHARD(decoder, kDecoderStep, 0, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 1, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 2, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 3, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 4, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 5, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 6, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 7, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 8, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 9, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 10, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 11, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 12, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 13, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 14, kOpFieldShift)
FUZZ_SHARD(decoder, kDecoderStep, 15, kOpFieldShift)

FUZZ_SHARD(disasm, kDisasmStep, 0, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 1, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 2, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 3, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 4, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 5, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 6, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 7, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 8, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 9, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 10, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 11, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 12, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 13, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 14, kOpFieldShift)
FUZZ_SHARD(disasm, kDisasmStep, 15, kOpFieldShift)

}  // namespace aarch64
}  // namespace vixl
