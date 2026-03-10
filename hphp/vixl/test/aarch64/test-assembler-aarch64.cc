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

#include "test-assembler-aarch64.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>

#include "test-runner.h"
#include "test-utils.h"

#include "aarch64/cpu-aarch64.h"
#include "aarch64/disasm-aarch64.h"
#include "aarch64/macro-assembler-aarch64.h"
#include "aarch64/simulator-aarch64.h"
#include "aarch64/test-utils-aarch64.h"

namespace vixl {
namespace aarch64 {

TEST(preshift_immediates) {
  SETUP();

  START();
  // Test operations involving immediates that could be generated using a
  // pre-shifted encodable immediate followed by a post-shift applied to
  // the arithmetic or logical operation.

  // Save sp.
  __ Mov(x29, sp);

  // Set the registers to known values.
  __ Mov(x0, 0x1000);
  __ Mov(sp, 0x1004);

  // Arithmetic ops.
  __ Add(x1, x0, 0x1f7de);
  __ Add(w2, w0, 0xffffff1);
  __ Adds(x3, x0, 0x18001);
  __ Adds(w4, w0, 0xffffff1);
  __ Sub(x5, x0, 0x1f7de);
  __ Sub(w6, w0, 0xffffff1);
  __ Subs(x7, x0, 0x18001);
  __ Subs(w8, w0, 0xffffff1);

  // Logical ops.
  __ And(x9, x0, 0x1f7de);
  __ Orr(w10, w0, 0xffffff1);
  __ Eor(x11, x0, 0x18001);

  // Ops using the stack pointer.
  __ Add(sp, sp, 0x18001);
  __ Mov(x12, sp);
  __ Mov(sp, 0x1004);

  __ Add(sp, sp, 0x1f7de);
  __ Mov(x13, sp);
  __ Mov(sp, 0x1004);

  __ Adds(x14, sp, 0x1f7de);

  __ Orr(sp, x0, 0x1f7de);
  __ Mov(x15, sp);

  //  Restore sp.
  __ Mov(sp, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1000, x0);
    ASSERT_EQUAL_64(0x207de, x1);
    ASSERT_EQUAL_64(0x10000ff1, x2);
    ASSERT_EQUAL_64(0x19001, x3);
    ASSERT_EQUAL_64(0x10000ff1, x4);
    ASSERT_EQUAL_64(0xfffffffffffe1822, x5);
    ASSERT_EQUAL_64(0xf000100f, x6);
    ASSERT_EQUAL_64(0xfffffffffffe8fff, x7);
    ASSERT_EQUAL_64(0xf000100f, x8);
    ASSERT_EQUAL_64(0x1000, x9);
    ASSERT_EQUAL_64(0xffffff1, x10);
    ASSERT_EQUAL_64(0x19001, x11);
    ASSERT_EQUAL_64(0x19005, x12);
    ASSERT_EQUAL_64(0x207e2, x13);
    ASSERT_EQUAL_64(0x207e2, x14);
    ASSERT_EQUAL_64(0x1f7de, x15);
  }
}


TEST(stack_ops) {
  SETUP();

  START();
  // save sp.
  __ Mov(x29, sp);

  // Set the sp to a known value.
  __ Mov(sp, 0x1004);
  __ Mov(x0, sp);

  // Add immediate to the sp, and move the result to a normal register.
  __ Add(sp, sp, 0x50);
  __ Mov(x1, sp);

  // Add extended to the sp, and move the result to a normal register.
  __ Mov(x17, 0xfff);
  __ Add(sp, sp, Operand(x17, SXTB));
  __ Mov(x2, sp);

  // Create an sp using a logical instruction, and move to normal register.
  __ Orr(sp, xzr, 0x1fff);
  __ Mov(x3, sp);

  // Write wsp using a logical instruction.
  __ Orr(wsp, wzr, 0xfffffff8);
  __ Mov(x4, sp);

  // Write sp, and read back wsp.
  __ Orr(sp, xzr, 0xfffffff8);
  __ Mov(w5, wsp);

  // Test writing into wsp in cases where the immediate isn't encodable.
  VIXL_ASSERT(!Assembler::IsImmLogical(0x1234, kWRegSize));
  __ Orr(wsp, w5, 0x1234);
  __ Mov(w6, wsp);

  //  restore sp.
  __ Mov(sp, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1004, x0);
    ASSERT_EQUAL_64(0x1054, x1);
    ASSERT_EQUAL_64(0x1053, x2);
    ASSERT_EQUAL_64(0x1fff, x3);
    ASSERT_EQUAL_64(0xfffffff8, x4);
    ASSERT_EQUAL_64(0xfffffff8, x5);
    ASSERT_EQUAL_64(0xfffffffc, x6);
  }
}


TEST(mvn) {
  SETUP();

  START();
  __ Mvn(w0, 0xfff);
  __ Mvn(x1, 0xfff);
  __ Mvn(w2, Operand(w0, LSL, 1));
  __ Mvn(x3, Operand(x1, LSL, 2));
  __ Mvn(w4, Operand(w0, LSR, 3));
  __ Mvn(x5, Operand(x1, LSR, 4));
  __ Mvn(w6, Operand(w0, ASR, 11));
  __ Mvn(x7, Operand(x1, ASR, 12));
  __ Mvn(w8, Operand(w0, ROR, 13));
  __ Mvn(x9, Operand(x1, ROR, 14));
  __ Mvn(w10, Operand(w2, UXTB));
  __ Mvn(x11, Operand(x2, SXTB, 1));
  __ Mvn(w12, Operand(w2, UXTH, 2));
  __ Mvn(x13, Operand(x2, SXTH, 3));
  __ Mvn(x14, Operand(w2, UXTW, 4));
  __ Mvn(x15, Operand(w2, SXTW, 4));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xfffff000, x0);
    ASSERT_EQUAL_64(0xfffffffffffff000, x1);
    ASSERT_EQUAL_64(0x00001fff, x2);
    ASSERT_EQUAL_64(0x0000000000003fff, x3);
    ASSERT_EQUAL_64(0xe00001ff, x4);
    ASSERT_EQUAL_64(0xf0000000000000ff, x5);
    ASSERT_EQUAL_64(0x00000001, x6);
    ASSERT_EQUAL_64(0x0000000000000000, x7);
    ASSERT_EQUAL_64(0x7ff80000, x8);
    ASSERT_EQUAL_64(0x3ffc000000000000, x9);
    ASSERT_EQUAL_64(0xffffff00, x10);
    ASSERT_EQUAL_64(0x0000000000000001, x11);
    ASSERT_EQUAL_64(0xffff8003, x12);
    ASSERT_EQUAL_64(0xffffffffffff0007, x13);
    ASSERT_EQUAL_64(0xfffffffffffe000f, x14);
    ASSERT_EQUAL_64(0xfffffffffffe000f, x15);
  }
}


TEST(mov_imm_w) {
  SETUP();

  START();
  __ Mov(w0, 0xffffffff);
  __ Mov(w1, 0xffff1234);
  __ Mov(w2, 0x1234ffff);
  __ Mov(w3, 0x00000000);
  __ Mov(w4, 0x00001234);
  __ Mov(w5, 0x12340000);
  __ Mov(w6, 0x12345678);
  __ Mov(w7, (int32_t)0x80000000);
  __ Mov(w8, (int32_t)0xffff0000);
  __ Mov(w9, kWMinInt);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffff, x0);
    ASSERT_EQUAL_64(0xffff1234, x1);
    ASSERT_EQUAL_64(0x1234ffff, x2);
    ASSERT_EQUAL_64(0x00000000, x3);
    ASSERT_EQUAL_64(0x00001234, x4);
    ASSERT_EQUAL_64(0x12340000, x5);
    ASSERT_EQUAL_64(0x12345678, x6);
    ASSERT_EQUAL_64(0x80000000, x7);
    ASSERT_EQUAL_64(0xffff0000, x8);
    ASSERT_EQUAL_32(kWMinInt, w9);
  }
}


TEST(mov_imm_x) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffff);
  __ Mov(x1, 0xffffffffffff1234);
  __ Mov(x2, 0xffffffff12345678);
  __ Mov(x3, 0xffff1234ffff5678);
  __ Mov(x4, 0x1234ffffffff5678);
  __ Mov(x5, 0x1234ffff5678ffff);
  __ Mov(x6, 0x12345678ffffffff);
  __ Mov(x7, 0x1234ffffffffffff);
  __ Mov(x8, 0x123456789abcffff);
  __ Mov(x9, 0x12345678ffff9abc);
  __ Mov(x10, 0x1234ffff56789abc);
  __ Mov(x11, 0xffff123456789abc);
  __ Mov(x12, 0x0000000000000000);
  __ Mov(x13, 0x0000000000001234);
  __ Mov(x14, 0x0000000012345678);
  __ Mov(x15, 0x0000123400005678);
  __ Mov(x18, 0x1234000000005678);
  __ Mov(x19, 0x1234000056780000);
  __ Mov(x20, 0x1234567800000000);
  __ Mov(x21, 0x1234000000000000);
  __ Mov(x22, 0x123456789abc0000);
  __ Mov(x23, 0x1234567800009abc);
  __ Mov(x24, 0x1234000056789abc);
  __ Mov(x25, 0x0000123456789abc);
  __ Mov(x26, 0x123456789abcdef0);
  __ Mov(x27, 0xffff000000000001);
  __ Mov(x28, 0x8000ffff00000000);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffffffff1234, x1);
    ASSERT_EQUAL_64(0xffffffff12345678, x2);
    ASSERT_EQUAL_64(0xffff1234ffff5678, x3);
    ASSERT_EQUAL_64(0x1234ffffffff5678, x4);
    ASSERT_EQUAL_64(0x1234ffff5678ffff, x5);
    ASSERT_EQUAL_64(0x12345678ffffffff, x6);
    ASSERT_EQUAL_64(0x1234ffffffffffff, x7);
    ASSERT_EQUAL_64(0x123456789abcffff, x8);
    ASSERT_EQUAL_64(0x12345678ffff9abc, x9);
    ASSERT_EQUAL_64(0x1234ffff56789abc, x10);
    ASSERT_EQUAL_64(0xffff123456789abc, x11);
    ASSERT_EQUAL_64(0x0000000000000000, x12);
    ASSERT_EQUAL_64(0x0000000000001234, x13);
    ASSERT_EQUAL_64(0x0000000012345678, x14);
    ASSERT_EQUAL_64(0x0000123400005678, x15);
    ASSERT_EQUAL_64(0x1234000000005678, x18);
    ASSERT_EQUAL_64(0x1234000056780000, x19);
    ASSERT_EQUAL_64(0x1234567800000000, x20);
    ASSERT_EQUAL_64(0x1234000000000000, x21);
    ASSERT_EQUAL_64(0x123456789abc0000, x22);
    ASSERT_EQUAL_64(0x1234567800009abc, x23);
    ASSERT_EQUAL_64(0x1234000056789abc, x24);
    ASSERT_EQUAL_64(0x0000123456789abc, x25);
    ASSERT_EQUAL_64(0x123456789abcdef0, x26);
    ASSERT_EQUAL_64(0xffff000000000001, x27);
    ASSERT_EQUAL_64(0x8000ffff00000000, x28);
  }
}


TEST(mov) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffff);
  __ Mov(x1, 0xffffffffffffffff);
  __ Mov(x2, 0xffffffffffffffff);
  __ Mov(x3, 0xffffffffffffffff);

  __ Mov(x0, 0x0123456789abcdef);

  {
    ExactAssemblyScope scope(&masm, 3 * kInstructionSize);
    __ movz(x1, UINT64_C(0xabcd) << 16);
    __ movk(x2, UINT64_C(0xabcd) << 32);
    __ movn(x3, UINT64_C(0xabcd) << 48);
  }

  __ Mov(x4, 0x0123456789abcdef);
  __ Mov(x5, x4);

  __ Mov(w6, -1);

  // Test that moves back to the same register have the desired effect. This
  // is a no-op for X registers, and a truncation for W registers.
  __ Mov(x7, 0x0123456789abcdef);
  __ Mov(x7, x7);
  __ Mov(x8, 0x0123456789abcdef);
  __ Mov(w8, w8);
  __ Mov(x9, 0x0123456789abcdef);
  __ Mov(x9, Operand(x9));
  __ Mov(x10, 0x0123456789abcdef);
  __ Mov(w10, Operand(w10));

  __ Mov(w11, 0xfff);
  __ Mov(x12, 0xfff);
  __ Mov(w13, Operand(w11, LSL, 1));
  __ Mov(x14, Operand(x12, LSL, 2));
  __ Mov(w15, Operand(w11, LSR, 3));
  __ Mov(x18, Operand(x12, LSR, 4));
  __ Mov(w19, Operand(w11, ASR, 11));
  __ Mov(x20, Operand(x12, ASR, 12));
  __ Mov(w21, Operand(w11, ROR, 13));
  __ Mov(x22, Operand(x12, ROR, 14));
  __ Mov(w23, Operand(w13, UXTB));
  __ Mov(x24, Operand(x13, SXTB, 1));
  __ Mov(w25, Operand(w13, UXTH, 2));
  __ Mov(x26, Operand(x13, SXTH, 3));
  __ Mov(x27, Operand(w13, UXTW, 4));

  __ Mov(x28, 0x0123456789abcdef);
  __ Mov(w28, w28, kDiscardForSameWReg);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0123456789abcdef, x0);
    ASSERT_EQUAL_64(0x00000000abcd0000, x1);
    ASSERT_EQUAL_64(0xffffabcdffffffff, x2);
    ASSERT_EQUAL_64(0x5432ffffffffffff, x3);
    ASSERT_EQUAL_64(x4, x5);
    ASSERT_EQUAL_32(-1, w6);
    ASSERT_EQUAL_64(0x0123456789abcdef, x7);
    ASSERT_EQUAL_32(0x89abcdef, w8);
    ASSERT_EQUAL_64(0x0123456789abcdef, x9);
    ASSERT_EQUAL_32(0x89abcdef, w10);
    ASSERT_EQUAL_64(0x00000fff, x11);
    ASSERT_EQUAL_64(0x0000000000000fff, x12);
    ASSERT_EQUAL_64(0x00001ffe, x13);
    ASSERT_EQUAL_64(0x0000000000003ffc, x14);
    ASSERT_EQUAL_64(0x000001ff, x15);
    ASSERT_EQUAL_64(0x00000000000000ff, x18);
    ASSERT_EQUAL_64(0x00000001, x19);
    ASSERT_EQUAL_64(0x0000000000000000, x20);
    ASSERT_EQUAL_64(0x7ff80000, x21);
    ASSERT_EQUAL_64(0x3ffc000000000000, x22);
    ASSERT_EQUAL_64(0x000000fe, x23);
    ASSERT_EQUAL_64(0xfffffffffffffffc, x24);
    ASSERT_EQUAL_64(0x00007ff8, x25);
    ASSERT_EQUAL_64(0x000000000000fff0, x26);
    ASSERT_EQUAL_64(0x000000000001ffe0, x27);
    ASSERT_EQUAL_64(0x0123456789abcdef, x28);
  }
}


TEST(mov_negative) {
  SETUP();

  START();
  __ Mov(w11, 0xffffffff);
  __ Mov(x12, 0xffffffffffffffff);

  __ Mov(w13, Operand(w11, LSL, 1));
  __ Mov(w14, Operand(w11, LSR, 1));
  __ Mov(w15, Operand(w11, ASR, 1));
  __ Mov(w18, Operand(w11, ROR, 1));
  __ Mov(w19, Operand(w11, UXTB, 1));
  __ Mov(w20, Operand(w11, SXTB, 1));
  __ Mov(w21, Operand(w11, UXTH, 1));
  __ Mov(w22, Operand(w11, SXTH, 1));

  __ Mov(x23, Operand(x12, LSL, 1));
  __ Mov(x24, Operand(x12, LSR, 1));
  __ Mov(x25, Operand(x12, ASR, 1));
  __ Mov(x26, Operand(x12, ROR, 1));
  __ Mov(x27, Operand(x12, UXTH, 1));
  __ Mov(x28, Operand(x12, SXTH, 1));
  __ Mov(x29, Operand(x12, UXTW, 1));
  __ Mov(x30, Operand(x12, SXTW, 1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xfffffffe, x13);
    ASSERT_EQUAL_64(0x7fffffff, x14);
    ASSERT_EQUAL_64(0xffffffff, x15);
    ASSERT_EQUAL_64(0xffffffff, x18);
    ASSERT_EQUAL_64(0x000001fe, x19);
    ASSERT_EQUAL_64(0xfffffffe, x20);
    ASSERT_EQUAL_64(0x0001fffe, x21);
    ASSERT_EQUAL_64(0xfffffffe, x22);

    ASSERT_EQUAL_64(0xfffffffffffffffe, x23);
    ASSERT_EQUAL_64(0x7fffffffffffffff, x24);
    ASSERT_EQUAL_64(0xffffffffffffffff, x25);
    ASSERT_EQUAL_64(0xffffffffffffffff, x26);
    ASSERT_EQUAL_64(0x000000000001fffe, x27);
    ASSERT_EQUAL_64(0xfffffffffffffffe, x28);
    ASSERT_EQUAL_64(0x00000001fffffffe, x29);
    ASSERT_EQUAL_64(0xfffffffffffffffe, x30);
  }
}


TEST(orr) {
  SETUP();

  START();
  __ Mov(x0, 0xf0f0);
  __ Mov(x1, 0xf00000ff);

  __ Orr(x2, x0, Operand(x1));
  __ Orr(w3, w0, Operand(w1, LSL, 28));
  __ Orr(x4, x0, Operand(x1, LSL, 32));
  __ Orr(x5, x0, Operand(x1, LSR, 4));
  __ Orr(w6, w0, Operand(w1, ASR, 4));
  __ Orr(x7, x0, Operand(x1, ASR, 4));
  __ Orr(w8, w0, Operand(w1, ROR, 12));
  __ Orr(x9, x0, Operand(x1, ROR, 12));
  __ Orr(w10, w0, 0xf);
  __ Orr(x11, x0, 0xf0000000f0000000);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00000000f000f0ff, x2);
    ASSERT_EQUAL_64(0xf000f0f0, x3);
    ASSERT_EQUAL_64(0xf00000ff0000f0f0, x4);
    ASSERT_EQUAL_64(0x000000000f00f0ff, x5);
    ASSERT_EQUAL_64(0xff00f0ff, x6);
    ASSERT_EQUAL_64(0x000000000f00f0ff, x7);
    ASSERT_EQUAL_64(0x0ffff0f0, x8);
    ASSERT_EQUAL_64(0x0ff00000000ff0f0, x9);
    ASSERT_EQUAL_64(0x0000f0ff, x10);
    ASSERT_EQUAL_64(0xf0000000f000f0f0, x11);
  }
}


TEST(orr_extend) {
  SETUP();

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x8000000080008080);
  __ Orr(w6, w0, Operand(w1, UXTB));
  __ Orr(x7, x0, Operand(x1, UXTH, 1));
  __ Orr(w8, w0, Operand(w1, UXTW, 2));
  __ Orr(x9, x0, Operand(x1, UXTX, 3));
  __ Orr(w10, w0, Operand(w1, SXTB));
  __ Orr(x11, x0, Operand(x1, SXTH, 1));
  __ Orr(x12, x0, Operand(x1, SXTW, 2));
  __ Orr(x13, x0, Operand(x1, SXTX, 3));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00000081, x6);
    ASSERT_EQUAL_64(0x0000000000010101, x7);
    ASSERT_EQUAL_64(0x00020201, x8);
    ASSERT_EQUAL_64(0x0000000400040401, x9);
    ASSERT_EQUAL_64(0xffffff81, x10);
    ASSERT_EQUAL_64(0xffffffffffff0101, x11);
    ASSERT_EQUAL_64(0xfffffffe00020201, x12);
    ASSERT_EQUAL_64(0x0000000400040401, x13);
  }
}


TEST(bitwise_wide_imm) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0xf0f0f0f0f0f0f0f0);

  __ Orr(x10, x0, 0x1234567890abcdef);
  __ Orr(w11, w1, 0x90abcdef);

  __ Orr(w12, w0, kWMinInt);
  __ Eor(w13, w0, kWMinInt);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(0xf0f0f0f0f0f0f0f0, x1);
    ASSERT_EQUAL_64(0x1234567890abcdef, x10);
    ASSERT_EQUAL_64(0x00000000f0fbfdff, x11);
    ASSERT_EQUAL_32(kWMinInt, w12);
    ASSERT_EQUAL_32(kWMinInt, w13);
  }
}


TEST(orn) {
  SETUP();

  START();
  __ Mov(x0, 0xf0f0);
  __ Mov(x1, 0xf00000ff);

  __ Orn(x2, x0, Operand(x1));
  __ Orn(w3, w0, Operand(w1, LSL, 4));
  __ Orn(x4, x0, Operand(x1, LSL, 4));
  __ Orn(x5, x0, Operand(x1, LSR, 1));
  __ Orn(w6, w0, Operand(w1, ASR, 1));
  __ Orn(x7, x0, Operand(x1, ASR, 1));
  __ Orn(w8, w0, Operand(w1, ROR, 16));
  __ Orn(x9, x0, Operand(x1, ROR, 16));
  __ Orn(w10, w0, 0x0000ffff);
  __ Orn(x11, x0, 0x0000ffff0000ffff);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffff0ffffff0, x2);
    ASSERT_EQUAL_64(0xfffff0ff, x3);
    ASSERT_EQUAL_64(0xfffffff0fffff0ff, x4);
    ASSERT_EQUAL_64(0xffffffff87fffff0, x5);
    ASSERT_EQUAL_64(0x07fffff0, x6);
    ASSERT_EQUAL_64(0xffffffff87fffff0, x7);
    ASSERT_EQUAL_64(0xff00ffff, x8);
    ASSERT_EQUAL_64(0xff00ffffffffffff, x9);
    ASSERT_EQUAL_64(0xfffff0f0, x10);
    ASSERT_EQUAL_64(0xffff0000fffff0f0, x11);
  }
}


TEST(orn_extend) {
  SETUP();

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x8000000080008081);
  __ Orn(w6, w0, Operand(w1, UXTB));
  __ Orn(x7, x0, Operand(x1, UXTH, 1));
  __ Orn(w8, w0, Operand(w1, UXTW, 2));
  __ Orn(x9, x0, Operand(x1, UXTX, 3));
  __ Orn(w10, w0, Operand(w1, SXTB));
  __ Orn(x11, x0, Operand(x1, SXTH, 1));
  __ Orn(x12, x0, Operand(x1, SXTW, 2));
  __ Orn(x13, x0, Operand(x1, SXTX, 3));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffff7f, x6);
    ASSERT_EQUAL_64(0xfffffffffffefefd, x7);
    ASSERT_EQUAL_64(0xfffdfdfb, x8);
    ASSERT_EQUAL_64(0xfffffffbfffbfbf7, x9);
    ASSERT_EQUAL_64(0x0000007f, x10);
    ASSERT_EQUAL_64(0x000000000000fefd, x11);
    ASSERT_EQUAL_64(0x00000001fffdfdfb, x12);
    ASSERT_EQUAL_64(0xfffffffbfffbfbf7, x13);
  }
}


TEST(and_) {
  SETUP();

  START();
  __ Mov(x0, 0xfff0);
  __ Mov(x1, 0xf00000ff);

  __ And(x2, x0, Operand(x1));
  __ And(w3, w0, Operand(w1, LSL, 4));
  __ And(x4, x0, Operand(x1, LSL, 4));
  __ And(x5, x0, Operand(x1, LSR, 1));
  __ And(w6, w0, Operand(w1, ASR, 20));
  __ And(x7, x0, Operand(x1, ASR, 20));
  __ And(w8, w0, Operand(w1, ROR, 28));
  __ And(x9, x0, Operand(x1, ROR, 28));
  __ And(w10, w0, Operand(0xff00));
  __ And(x11, x0, Operand(0xff));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x000000f0, x2);
    ASSERT_EQUAL_64(0x00000ff0, x3);
    ASSERT_EQUAL_64(0x00000ff0, x4);
    ASSERT_EQUAL_64(0x00000070, x5);
    ASSERT_EQUAL_64(0x0000ff00, x6);
    ASSERT_EQUAL_64(0x00000f00, x7);
    ASSERT_EQUAL_64(0x00000ff0, x8);
    ASSERT_EQUAL_64(0x00000000, x9);
    ASSERT_EQUAL_64(0x0000ff00, x10);
    ASSERT_EQUAL_64(0x000000f0, x11);
  }
}


TEST(and_extend) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffff);
  __ Mov(x1, 0x8000000080008081);
  __ And(w6, w0, Operand(w1, UXTB));
  __ And(x7, x0, Operand(x1, UXTH, 1));
  __ And(w8, w0, Operand(w1, UXTW, 2));
  __ And(x9, x0, Operand(x1, UXTX, 3));
  __ And(w10, w0, Operand(w1, SXTB));
  __ And(x11, x0, Operand(x1, SXTH, 1));
  __ And(x12, x0, Operand(x1, SXTW, 2));
  __ And(x13, x0, Operand(x1, SXTX, 3));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00000081, x6);
    ASSERT_EQUAL_64(0x0000000000010102, x7);
    ASSERT_EQUAL_64(0x00020204, x8);
    ASSERT_EQUAL_64(0x0000000400040408, x9);
    ASSERT_EQUAL_64(0xffffff81, x10);
    ASSERT_EQUAL_64(0xffffffffffff0102, x11);
    ASSERT_EQUAL_64(0xfffffffe00020204, x12);
    ASSERT_EQUAL_64(0x0000000400040408, x13);
  }
}


TEST(ands) {
  SETUP();

  START();
  __ Mov(x1, 0xf00000ff);
  __ Ands(w0, w1, Operand(w1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
    ASSERT_EQUAL_64(0xf00000ff, x0);
  }

  START();
  __ Mov(x0, 0xfff0);
  __ Mov(x1, 0xf00000ff);
  __ Ands(w0, w0, Operand(w1, LSR, 4));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZFlag);
    ASSERT_EQUAL_64(0x00000000, x0);
  }

  START();
  __ Mov(x0, 0x8000000000000000);
  __ Mov(x1, 0x00000001);
  __ Ands(x0, x0, Operand(x1, ROR, 1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
    ASSERT_EQUAL_64(0x8000000000000000, x0);
  }

  START();
  __ Mov(x0, 0xfff0);
  __ Ands(w0, w0, Operand(0xf));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZFlag);
    ASSERT_EQUAL_64(0x00000000, x0);
  }

  START();
  __ Mov(x0, 0xff000000);
  __ Ands(w0, w0, Operand(0x80000000));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
    ASSERT_EQUAL_64(0x80000000, x0);
  }
}


TEST(bic) {
  SETUP();

  START();
  __ Mov(x0, 0xfff0);
  __ Mov(x1, 0xf00000ff);

  __ Bic(x2, x0, Operand(x1));
  __ Bic(w3, w0, Operand(w1, LSL, 4));
  __ Bic(x4, x0, Operand(x1, LSL, 4));
  __ Bic(x5, x0, Operand(x1, LSR, 1));
  __ Bic(w6, w0, Operand(w1, ASR, 20));
  __ Bic(x7, x0, Operand(x1, ASR, 20));
  __ Bic(w8, w0, Operand(w1, ROR, 28));
  __ Bic(x9, x0, Operand(x1, ROR, 24));
  __ Bic(x10, x0, Operand(0x1f));
  __ Bic(x11, x0, Operand(0x100));

  // Test bic into sp when the constant cannot be encoded in the immediate
  // field.
  // Use x20 to preserve sp. We check for the result via x21 because the
  // test infrastructure requires that sp be restored to its original value.
  __ Mov(x20, sp);
  __ Mov(x0, 0xffffff);
  __ Bic(sp, x0, Operand(0xabcdef));
  __ Mov(x21, sp);
  __ Mov(sp, x20);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0000ff00, x2);
    ASSERT_EQUAL_64(0x0000f000, x3);
    ASSERT_EQUAL_64(0x0000f000, x4);
    ASSERT_EQUAL_64(0x0000ff80, x5);
    ASSERT_EQUAL_64(0x000000f0, x6);
    ASSERT_EQUAL_64(0x0000f0f0, x7);
    ASSERT_EQUAL_64(0x0000f000, x8);
    ASSERT_EQUAL_64(0x0000ff00, x9);
    ASSERT_EQUAL_64(0x0000ffe0, x10);
    ASSERT_EQUAL_64(0x0000fef0, x11);

    ASSERT_EQUAL_64(0x543210, x21);
  }
}


TEST(bic_extend) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffff);
  __ Mov(x1, 0x8000000080008081);
  __ Bic(w6, w0, Operand(w1, UXTB));
  __ Bic(x7, x0, Operand(x1, UXTH, 1));
  __ Bic(w8, w0, Operand(w1, UXTW, 2));
  __ Bic(x9, x0, Operand(x1, UXTX, 3));
  __ Bic(w10, w0, Operand(w1, SXTB));
  __ Bic(x11, x0, Operand(x1, SXTH, 1));
  __ Bic(x12, x0, Operand(x1, SXTW, 2));
  __ Bic(x13, x0, Operand(x1, SXTX, 3));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffff7e, x6);
    ASSERT_EQUAL_64(0xfffffffffffefefd, x7);
    ASSERT_EQUAL_64(0xfffdfdfb, x8);
    ASSERT_EQUAL_64(0xfffffffbfffbfbf7, x9);
    ASSERT_EQUAL_64(0x0000007e, x10);
    ASSERT_EQUAL_64(0x000000000000fefd, x11);
    ASSERT_EQUAL_64(0x00000001fffdfdfb, x12);
    ASSERT_EQUAL_64(0xfffffffbfffbfbf7, x13);
  }
}


TEST(bics) {
  SETUP();

  START();
  __ Mov(x1, 0xffff);
  __ Bics(w0, w1, Operand(w1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZFlag);
    ASSERT_EQUAL_64(0x00000000, x0);
  }

  START();
  __ Mov(x0, 0xffffffff);
  __ Bics(w0, w0, Operand(w0, LSR, 1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
    ASSERT_EQUAL_64(0x80000000, x0);
  }

  START();
  __ Mov(x0, 0x8000000000000000);
  __ Mov(x1, 0x00000001);
  __ Bics(x0, x0, Operand(x1, ROR, 1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZFlag);
    ASSERT_EQUAL_64(0x00000000, x0);
  }

  START();
  __ Mov(x0, 0xffffffffffffffff);
  __ Bics(x0, x0, 0x7fffffffffffffff);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
    ASSERT_EQUAL_64(0x8000000000000000, x0);
  }

  START();
  __ Mov(w0, 0xffff0000);
  __ Bics(w0, w0, 0xfffffff0);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZFlag);
    ASSERT_EQUAL_64(0x00000000, x0);
  }
}


TEST(eor) {
  SETUP();

  START();
  __ Mov(x0, 0xfff0);
  __ Mov(x1, 0xf00000ff);

  __ Eor(x2, x0, Operand(x1));
  __ Eor(w3, w0, Operand(w1, LSL, 4));
  __ Eor(x4, x0, Operand(x1, LSL, 4));
  __ Eor(x5, x0, Operand(x1, LSR, 1));
  __ Eor(w6, w0, Operand(w1, ASR, 20));
  __ Eor(x7, x0, Operand(x1, ASR, 20));
  __ Eor(w8, w0, Operand(w1, ROR, 28));
  __ Eor(x9, x0, Operand(x1, ROR, 28));
  __ Eor(w10, w0, 0xff00ff00);
  __ Eor(x11, x0, 0xff00ff00ff00ff00);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00000000f000ff0f, x2);
    ASSERT_EQUAL_64(0x0000f000, x3);
    ASSERT_EQUAL_64(0x0000000f0000f000, x4);
    ASSERT_EQUAL_64(0x000000007800ff8f, x5);
    ASSERT_EQUAL_64(0xffff00f0, x6);
    ASSERT_EQUAL_64(0x000000000000f0f0, x7);
    ASSERT_EQUAL_64(0x0000f00f, x8);
    ASSERT_EQUAL_64(0x00000ff00000ffff, x9);
    ASSERT_EQUAL_64(0xff0000f0, x10);
    ASSERT_EQUAL_64(0xff00ff00ff0000f0, x11);
  }
}

TEST(eor_extend) {
  SETUP();

  START();
  __ Mov(x0, 0x1111111111111111);
  __ Mov(x1, 0x8000000080008081);
  __ Eor(w6, w0, Operand(w1, UXTB));
  __ Eor(x7, x0, Operand(x1, UXTH, 1));
  __ Eor(w8, w0, Operand(w1, UXTW, 2));
  __ Eor(x9, x0, Operand(x1, UXTX, 3));
  __ Eor(w10, w0, Operand(w1, SXTB));
  __ Eor(x11, x0, Operand(x1, SXTH, 1));
  __ Eor(x12, x0, Operand(x1, SXTW, 2));
  __ Eor(x13, x0, Operand(x1, SXTX, 3));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x11111190, x6);
    ASSERT_EQUAL_64(0x1111111111101013, x7);
    ASSERT_EQUAL_64(0x11131315, x8);
    ASSERT_EQUAL_64(0x1111111511151519, x9);
    ASSERT_EQUAL_64(0xeeeeee90, x10);
    ASSERT_EQUAL_64(0xeeeeeeeeeeee1013, x11);
    ASSERT_EQUAL_64(0xeeeeeeef11131315, x12);
    ASSERT_EQUAL_64(0x1111111511151519, x13);
  }
}


TEST(eon) {
  SETUP();

  START();
  __ Mov(x0, 0xfff0);
  __ Mov(x1, 0xf00000ff);

  __ Eon(x2, x0, Operand(x1));
  __ Eon(w3, w0, Operand(w1, LSL, 4));
  __ Eon(x4, x0, Operand(x1, LSL, 4));
  __ Eon(x5, x0, Operand(x1, LSR, 1));
  __ Eon(w6, w0, Operand(w1, ASR, 20));
  __ Eon(x7, x0, Operand(x1, ASR, 20));
  __ Eon(w8, w0, Operand(w1, ROR, 28));
  __ Eon(x9, x0, Operand(x1, ROR, 28));
  __ Eon(w10, w0, 0x03c003c0);
  __ Eon(x11, x0, 0x0000100000001000);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffff0fff00f0, x2);
    ASSERT_EQUAL_64(0xffff0fff, x3);
    ASSERT_EQUAL_64(0xfffffff0ffff0fff, x4);
    ASSERT_EQUAL_64(0xffffffff87ff0070, x5);
    ASSERT_EQUAL_64(0x0000ff0f, x6);
    ASSERT_EQUAL_64(0xffffffffffff0f0f, x7);
    ASSERT_EQUAL_64(0xffff0ff0, x8);
    ASSERT_EQUAL_64(0xfffff00fffff0000, x9);
    ASSERT_EQUAL_64(0xfc3f03cf, x10);
    ASSERT_EQUAL_64(0xffffefffffff100f, x11);
  }
}


TEST(eon_extend) {
  SETUP();

  START();
  __ Mov(x0, 0x1111111111111111);
  __ Mov(x1, 0x8000000080008081);
  __ Eon(w6, w0, Operand(w1, UXTB));
  __ Eon(x7, x0, Operand(x1, UXTH, 1));
  __ Eon(w8, w0, Operand(w1, UXTW, 2));
  __ Eon(x9, x0, Operand(x1, UXTX, 3));
  __ Eon(w10, w0, Operand(w1, SXTB));
  __ Eon(x11, x0, Operand(x1, SXTH, 1));
  __ Eon(x12, x0, Operand(x1, SXTW, 2));
  __ Eon(x13, x0, Operand(x1, SXTX, 3));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xeeeeee6f, x6);
    ASSERT_EQUAL_64(0xeeeeeeeeeeefefec, x7);
    ASSERT_EQUAL_64(0xeeececea, x8);
    ASSERT_EQUAL_64(0xeeeeeeeaeeeaeae6, x9);
    ASSERT_EQUAL_64(0x1111116f, x10);
    ASSERT_EQUAL_64(0x111111111111efec, x11);
    ASSERT_EQUAL_64(0x11111110eeececea, x12);
    ASSERT_EQUAL_64(0xeeeeeeeaeeeaeae6, x13);
  }
}


TEST(mul) {
  SETUP();

  START();
  __ Mov(x25, 0);
  __ Mov(x26, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffff);

  __ Mul(w0, w25, w25);
  __ Mul(w1, w25, w26);
  __ Mul(w2, w26, w18);
  __ Mul(w3, w18, w19);
  __ Mul(x4, x25, x25);
  __ Mul(x5, x26, x18);
  __ Mul(x6, x18, x19);
  __ Mul(x7, x19, x19);
  __ Smull(x8, w26, w18);
  __ Smull(x9, w18, w18);
  __ Smull(x10, w19, w19);
  __ Mneg(w11, w25, w25);
  __ Mneg(w12, w25, w26);
  __ Mneg(w13, w26, w18);
  __ Mneg(w14, w18, w19);
  __ Mneg(x20, x25, x25);
  __ Mneg(x21, x26, x18);
  __ Mneg(x22, x18, x19);
  __ Mneg(x23, x19, x19);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(0xffffffff, x2);
    ASSERT_EQUAL_64(1, x3);
    ASSERT_EQUAL_64(0, x4);
    ASSERT_EQUAL_64(0xffffffff, x5);
    ASSERT_EQUAL_64(0xffffffff00000001, x6);
    ASSERT_EQUAL_64(1, x7);
    ASSERT_EQUAL_64(0xffffffffffffffff, x8);
    ASSERT_EQUAL_64(1, x9);
    ASSERT_EQUAL_64(1, x10);
    ASSERT_EQUAL_64(0, x11);
    ASSERT_EQUAL_64(0, x12);
    ASSERT_EQUAL_64(1, x13);
    ASSERT_EQUAL_64(0xffffffff, x14);
    ASSERT_EQUAL_64(0, x20);
    ASSERT_EQUAL_64(0xffffffff00000001, x21);
    ASSERT_EQUAL_64(0xffffffff, x22);
    ASSERT_EQUAL_64(0xffffffffffffffff, x23);
  }
}


static void SmullHelper(int64_t expected, int64_t a, int64_t b) {
  SETUP();
  START();
  __ Mov(w0, a);
  __ Mov(w1, b);
  __ Smull(x2, w0, w1);
  END();
  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(expected, x2);
  }
}


TEST(smull) {
  SmullHelper(0, 0, 0);
  SmullHelper(1, 1, 1);
  SmullHelper(-1, -1, 1);
  SmullHelper(1, -1, -1);
  SmullHelper(0xffffffff80000000, 0x80000000, 1);
  SmullHelper(0x0000000080000000, 0x00010000, 0x00008000);
}


TEST(madd) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffff);

  __ Madd(w0, w16, w16, w16);
  __ Madd(w1, w16, w16, w17);
  __ Madd(w2, w16, w16, w18);
  __ Madd(w3, w16, w16, w19);
  __ Madd(w4, w16, w17, w17);
  __ Madd(w5, w17, w17, w18);
  __ Madd(w6, w17, w17, w19);
  __ Madd(w7, w17, w18, w16);
  __ Madd(w8, w17, w18, w18);
  __ Madd(w9, w18, w18, w17);
  __ Madd(w10, w18, w19, w18);
  __ Madd(w11, w19, w19, w19);

  __ Madd(x12, x16, x16, x16);
  __ Madd(x13, x16, x16, x17);
  __ Madd(x14, x16, x16, x18);
  __ Madd(x15, x16, x16, x19);
  __ Madd(x20, x16, x17, x17);
  __ Madd(x21, x17, x17, x18);
  __ Madd(x22, x17, x17, x19);
  __ Madd(x23, x17, x18, x16);
  __ Madd(x24, x17, x18, x18);
  __ Madd(x25, x18, x18, x17);
  __ Madd(x26, x18, x19, x18);
  __ Madd(x27, x19, x19, x19);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(1, x1);
    ASSERT_EQUAL_64(0xffffffff, x2);
    ASSERT_EQUAL_64(0xffffffff, x3);
    ASSERT_EQUAL_64(1, x4);
    ASSERT_EQUAL_64(0, x5);
    ASSERT_EQUAL_64(0, x6);
    ASSERT_EQUAL_64(0xffffffff, x7);
    ASSERT_EQUAL_64(0xfffffffe, x8);
    ASSERT_EQUAL_64(2, x9);
    ASSERT_EQUAL_64(0, x10);
    ASSERT_EQUAL_64(0, x11);

    ASSERT_EQUAL_64(0, x12);
    ASSERT_EQUAL_64(1, x13);
    ASSERT_EQUAL_64(0x00000000ffffffff, x14);
    ASSERT_EQUAL_64(0xffffffffffffffff, x15);
    ASSERT_EQUAL_64(1, x20);
    ASSERT_EQUAL_64(0x0000000100000000, x21);
    ASSERT_EQUAL_64(0, x22);
    ASSERT_EQUAL_64(0x00000000ffffffff, x23);
    ASSERT_EQUAL_64(0x00000001fffffffe, x24);
    ASSERT_EQUAL_64(0xfffffffe00000002, x25);
    ASSERT_EQUAL_64(0, x26);
    ASSERT_EQUAL_64(0, x27);
  }
}


TEST(msub) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffff);

  __ Msub(w0, w16, w16, w16);
  __ Msub(w1, w16, w16, w17);
  __ Msub(w2, w16, w16, w18);
  __ Msub(w3, w16, w16, w19);
  __ Msub(w4, w16, w17, w17);
  __ Msub(w5, w17, w17, w18);
  __ Msub(w6, w17, w17, w19);
  __ Msub(w7, w17, w18, w16);
  __ Msub(w8, w17, w18, w18);
  __ Msub(w9, w18, w18, w17);
  __ Msub(w10, w18, w19, w18);
  __ Msub(w11, w19, w19, w19);

  __ Msub(x12, x16, x16, x16);
  __ Msub(x13, x16, x16, x17);
  __ Msub(x14, x16, x16, x18);
  __ Msub(x15, x16, x16, x19);
  __ Msub(x20, x16, x17, x17);
  __ Msub(x21, x17, x17, x18);
  __ Msub(x22, x17, x17, x19);
  __ Msub(x23, x17, x18, x16);
  __ Msub(x24, x17, x18, x18);
  __ Msub(x25, x18, x18, x17);
  __ Msub(x26, x18, x19, x18);
  __ Msub(x27, x19, x19, x19);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(1, x1);
    ASSERT_EQUAL_64(0xffffffff, x2);
    ASSERT_EQUAL_64(0xffffffff, x3);
    ASSERT_EQUAL_64(1, x4);
    ASSERT_EQUAL_64(0xfffffffe, x5);
    ASSERT_EQUAL_64(0xfffffffe, x6);
    ASSERT_EQUAL_64(1, x7);
    ASSERT_EQUAL_64(0, x8);
    ASSERT_EQUAL_64(0, x9);
    ASSERT_EQUAL_64(0xfffffffe, x10);
    ASSERT_EQUAL_64(0xfffffffe, x11);

    ASSERT_EQUAL_64(0, x12);
    ASSERT_EQUAL_64(1, x13);
    ASSERT_EQUAL_64(0x00000000ffffffff, x14);
    ASSERT_EQUAL_64(0xffffffffffffffff, x15);
    ASSERT_EQUAL_64(1, x20);
    ASSERT_EQUAL_64(0x00000000fffffffe, x21);
    ASSERT_EQUAL_64(0xfffffffffffffffe, x22);
    ASSERT_EQUAL_64(0xffffffff00000001, x23);
    ASSERT_EQUAL_64(0, x24);
    ASSERT_EQUAL_64(0x0000000200000000, x25);
    ASSERT_EQUAL_64(0x00000001fffffffe, x26);
    ASSERT_EQUAL_64(0xfffffffffffffffe, x27);
  }
}


TEST(smulh) {
  SETUP();

  START();
  __ Mov(x20, 0);
  __ Mov(x21, 1);
  __ Mov(x22, 0x0000000100000000);
  __ Mov(x23, 0x0000000012345678);
  __ Mov(x24, 0x0123456789abcdef);
  __ Mov(x25, 0x0000000200000000);
  __ Mov(x26, 0x8000000000000000);
  __ Mov(x27, 0xffffffffffffffff);
  __ Mov(x28, 0x5555555555555555);
  __ Mov(x29, 0xaaaaaaaaaaaaaaaa);

  __ Smulh(x0, x20, x24);
  __ Smulh(x1, x21, x24);
  __ Smulh(x2, x22, x23);
  __ Smulh(x3, x22, x24);
  __ Smulh(x4, x24, x25);
  __ Smulh(x5, x23, x27);
  __ Smulh(x6, x26, x26);
  __ Smulh(x7, x26, x27);
  __ Smulh(x8, x27, x27);
  __ Smulh(x9, x28, x28);
  __ Smulh(x10, x28, x29);
  __ Smulh(x11, x29, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(0, x2);
    ASSERT_EQUAL_64(0x0000000001234567, x3);
    ASSERT_EQUAL_64(0x0000000002468acf, x4);
    ASSERT_EQUAL_64(0xffffffffffffffff, x5);
    ASSERT_EQUAL_64(0x4000000000000000, x6);
    ASSERT_EQUAL_64(0, x7);
    ASSERT_EQUAL_64(0, x8);
    ASSERT_EQUAL_64(0x1c71c71c71c71c71, x9);
    ASSERT_EQUAL_64(0xe38e38e38e38e38e, x10);
    ASSERT_EQUAL_64(0x1c71c71c71c71c72, x11);
  }
}


TEST(umulh) {
  SETUP();

  START();
  __ Mov(x20, 0);
  __ Mov(x21, 1);
  __ Mov(x22, 0x0000000100000000);
  __ Mov(x23, 0x0000000012345678);
  __ Mov(x24, 0x0123456789abcdef);
  __ Mov(x25, 0x0000000200000000);
  __ Mov(x26, 0x8000000000000000);
  __ Mov(x27, 0xffffffffffffffff);
  __ Mov(x28, 0x5555555555555555);
  __ Mov(x29, 0xaaaaaaaaaaaaaaaa);

  __ Umulh(x0, x20, x24);
  __ Umulh(x1, x21, x24);
  __ Umulh(x2, x22, x23);
  __ Umulh(x3, x22, x24);
  __ Umulh(x4, x24, x25);
  __ Umulh(x5, x23, x27);
  __ Umulh(x6, x26, x26);
  __ Umulh(x7, x26, x27);
  __ Umulh(x8, x27, x27);
  __ Umulh(x9, x28, x28);
  __ Umulh(x10, x28, x29);
  __ Umulh(x11, x29, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(0, x2);
    ASSERT_EQUAL_64(0x0000000001234567, x3);
    ASSERT_EQUAL_64(0x0000000002468acf, x4);
    ASSERT_EQUAL_64(0x0000000012345677, x5);
    ASSERT_EQUAL_64(0x4000000000000000, x6);
    ASSERT_EQUAL_64(0x7fffffffffffffff, x7);
    ASSERT_EQUAL_64(0xfffffffffffffffe, x8);
    ASSERT_EQUAL_64(0x1c71c71c71c71c71, x9);
    ASSERT_EQUAL_64(0x38e38e38e38e38e3, x10);
    ASSERT_EQUAL_64(0x71c71c71c71c71c6, x11);
  }
}


TEST(smaddl_umaddl_umull) {
  SETUP();

  START();
  __ Mov(x17, 1);
  __ Mov(x18, 0x00000000ffffffff);
  __ Mov(x19, 0xffffffffffffffff);
  __ Mov(x20, 4);
  __ Mov(x21, 0x0000000200000000);

  __ Smaddl(x9, w17, w18, x20);
  __ Smaddl(x10, w18, w18, x20);
  __ Smaddl(x11, w19, w19, x20);
  __ Smaddl(x12, w19, w19, x21);
  __ Umaddl(x13, w17, w18, x20);
  __ Umaddl(x14, w18, w18, x20);
  __ Umaddl(x15, w19, w19, x20);
  __ Umaddl(x22, w19, w19, x21);
  __ Umull(x24, w19, w19);
  __ Umull(x25, w17, w18);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(3, x9);
    ASSERT_EQUAL_64(5, x10);
    ASSERT_EQUAL_64(5, x11);
    ASSERT_EQUAL_64(0x0000000200000001, x12);
    ASSERT_EQUAL_64(0x0000000100000003, x13);
    ASSERT_EQUAL_64(0xfffffffe00000005, x14);
    ASSERT_EQUAL_64(0xfffffffe00000005, x15);
    ASSERT_EQUAL_64(1, x22);
    ASSERT_EQUAL_64(0xfffffffe00000001, x24);
    ASSERT_EQUAL_64(0x00000000ffffffff, x25);
  }
}


TEST(smsubl_umsubl) {
  SETUP();

  START();
  __ Mov(x17, 1);
  __ Mov(x18, 0x00000000ffffffff);
  __ Mov(x19, 0xffffffffffffffff);
  __ Mov(x20, 4);
  __ Mov(x21, 0x0000000200000000);

  __ Smsubl(x9, w17, w18, x20);
  __ Smsubl(x10, w18, w18, x20);
  __ Smsubl(x11, w19, w19, x20);
  __ Smsubl(x12, w19, w19, x21);
  __ Umsubl(x13, w17, w18, x20);
  __ Umsubl(x14, w18, w18, x20);
  __ Umsubl(x15, w19, w19, x20);
  __ Umsubl(x22, w19, w19, x21);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(5, x9);
    ASSERT_EQUAL_64(3, x10);
    ASSERT_EQUAL_64(3, x11);
    ASSERT_EQUAL_64(0x00000001ffffffff, x12);
    ASSERT_EQUAL_64(0xffffffff00000005, x13);
    ASSERT_EQUAL_64(0x0000000200000003, x14);
    ASSERT_EQUAL_64(0x0000000200000003, x15);
    ASSERT_EQUAL_64(0x00000003ffffffff, x22);
  }
}


TEST(div) {
  SETUP();

  START();
  __ Mov(x16, 1);
  __ Mov(x17, 0xffffffff);
  __ Mov(x18, 0xffffffffffffffff);
  __ Mov(x19, 0x80000000);
  __ Mov(x20, 0x8000000000000000);
  __ Mov(x21, 2);

  __ Udiv(w0, w16, w16);
  __ Udiv(w1, w17, w16);
  __ Sdiv(w2, w16, w16);
  __ Sdiv(w3, w16, w17);
  __ Sdiv(w4, w17, w18);

  __ Udiv(x5, x16, x16);
  __ Udiv(x6, x17, x18);
  __ Sdiv(x7, x16, x16);
  __ Sdiv(x8, x16, x17);
  __ Sdiv(x9, x17, x18);

  __ Udiv(w10, w19, w21);
  __ Sdiv(w11, w19, w21);
  __ Udiv(x12, x19, x21);
  __ Sdiv(x13, x19, x21);
  __ Udiv(x14, x20, x21);
  __ Sdiv(x15, x20, x21);

  __ Udiv(w22, w19, w17);
  __ Sdiv(w23, w19, w17);
  __ Udiv(x24, x20, x18);
  __ Sdiv(x25, x20, x18);

  __ Udiv(x26, x16, x21);
  __ Sdiv(x27, x16, x21);
  __ Udiv(x28, x18, x21);
  __ Sdiv(x29, x18, x21);

  __ Mov(x17, 0);
  __ Udiv(w18, w16, w17);
  __ Sdiv(w19, w16, w17);
  __ Udiv(x20, x16, x17);
  __ Sdiv(x21, x16, x17);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x0);
    ASSERT_EQUAL_64(0xffffffff, x1);
    ASSERT_EQUAL_64(1, x2);
    ASSERT_EQUAL_64(0xffffffff, x3);
    ASSERT_EQUAL_64(1, x4);
    ASSERT_EQUAL_64(1, x5);
    ASSERT_EQUAL_64(0, x6);
    ASSERT_EQUAL_64(1, x7);
    ASSERT_EQUAL_64(0, x8);
    ASSERT_EQUAL_64(0xffffffff00000001, x9);
    ASSERT_EQUAL_64(0x40000000, x10);
    ASSERT_EQUAL_64(0xc0000000, x11);
    ASSERT_EQUAL_64(0x0000000040000000, x12);
    ASSERT_EQUAL_64(0x0000000040000000, x13);
    ASSERT_EQUAL_64(0x4000000000000000, x14);
    ASSERT_EQUAL_64(0xc000000000000000, x15);
    ASSERT_EQUAL_64(0, x22);
    ASSERT_EQUAL_64(0x80000000, x23);
    ASSERT_EQUAL_64(0, x24);
    ASSERT_EQUAL_64(0x8000000000000000, x25);
    ASSERT_EQUAL_64(0, x26);
    ASSERT_EQUAL_64(0, x27);
    ASSERT_EQUAL_64(0x7fffffffffffffff, x28);
    ASSERT_EQUAL_64(0, x29);
    ASSERT_EQUAL_64(0, x18);
    ASSERT_EQUAL_64(0, x19);
    ASSERT_EQUAL_64(0, x20);
    ASSERT_EQUAL_64(0, x21);
  }
}


TEST(rbit_rev) {
  SETUP();

  START();
  __ Mov(x24, 0xfedcba9876543210);
  __ Rbit(w0, w24);
  __ Rbit(x1, x24);
  __ Rev16(w2, w24);
  __ Rev16(x3, x24);
  __ Rev(w4, w24);
  __ Rev32(x5, x24);
  __ Rev64(x6, x24);
  __ Rev(x7, x24);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x084c2a6e, x0);
    ASSERT_EQUAL_64(0x084c2a6e195d3b7f, x1);
    ASSERT_EQUAL_64(0x54761032, x2);
    ASSERT_EQUAL_64(0xdcfe98ba54761032, x3);
    ASSERT_EQUAL_64(0x10325476, x4);
    ASSERT_EQUAL_64(0x98badcfe10325476, x5);
    ASSERT_EQUAL_64(0x1032547698badcfe, x6);
    ASSERT_EQUAL_64(0x1032547698badcfe, x7);
  }
}

typedef void (MacroAssembler::*TestBranchSignature)(const Register& rt,
                                                    unsigned bit_pos,
                                                    Label* label);

static void TbzRangePoolLimitHelper(TestBranchSignature test_branch) {
  const int kTbzRange = 32768;
  const int kNumLdrLiteral = kTbzRange / 4;
  const int fuzz_range = 2;
  for (int n = kNumLdrLiteral - fuzz_range; n <= kNumLdrLiteral + fuzz_range;
       ++n) {
    for (int margin = -32; margin < 32; margin += 4) {
      SETUP();

      START();

      // Emit 32KB of literals (equal to the range of TBZ).
      for (int i = 0; i < n; ++i) {
        __ Ldr(w0, 0x12345678);
      }

      const int kLiteralMargin = 128 * KBytes;

      // Emit enough NOPs to be just about to emit the literal pool.
      ptrdiff_t end =
          masm.GetCursorOffset() + (kLiteralMargin - n * 4 + margin);
      while (masm.GetCursorOffset() < end) {
        __ Nop();
      }

      // Add a TBZ instruction.
      Label label;

      (masm.*test_branch)(x0, 2, &label);

      // Add enough NOPs to surpass its range, to make sure we can encode the
      // veneer.
      end = masm.GetCursorOffset() + (kTbzRange - 4);
      {
        ExactAssemblyScope scope(&masm,
                                 kTbzRange,
                                 ExactAssemblyScope::kMaximumSize);
        while (masm.GetCursorOffset() < end) __ nop();
      }

      // Finally, bind the label.
      __ Bind(&label);

      END();

      if (CAN_RUN()) {
        RUN();
      }
    }
  }
}

TEST(test_branch_limits_literal_pool_size_tbz) {
  TbzRangePoolLimitHelper(&MacroAssembler::Tbz);
}

TEST(test_branch_limits_literal_pool_size_tbnz) {
  TbzRangePoolLimitHelper(&MacroAssembler::Tbnz);
}

TEST(clz_cls) {
  SETUP();

  START();
  __ Mov(x24, 0x0008000000800000);
  __ Mov(x25, 0xff800000fff80000);
  __ Mov(x26, 0);
  __ Clz(w0, w24);
  __ Clz(x1, x24);
  __ Clz(w2, w25);
  __ Clz(x3, x25);
  __ Clz(w4, w26);
  __ Clz(x5, x26);
  __ Cls(w6, w24);
  __ Cls(x7, x24);
  __ Cls(w8, w25);
  __ Cls(x9, x25);
  __ Cls(w10, w26);
  __ Cls(x11, x26);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(8, x0);
    ASSERT_EQUAL_64(12, x1);
    ASSERT_EQUAL_64(0, x2);
    ASSERT_EQUAL_64(0, x3);
    ASSERT_EQUAL_64(32, x4);
    ASSERT_EQUAL_64(64, x5);
    ASSERT_EQUAL_64(7, x6);
    ASSERT_EQUAL_64(11, x7);
    ASSERT_EQUAL_64(12, x8);
    ASSERT_EQUAL_64(8, x9);
    ASSERT_EQUAL_64(31, x10);
    ASSERT_EQUAL_64(63, x11);
  }
}


TEST(pacia_pacib_autia_autib) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Register pointer = x24;
  Register retry_limit = x25;
  Register modifier = x26;
  Label retry;

  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide, so retry a few times with different
  // pointers.
  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);
  __ Mov(modifier, 0x477d469dec0b8760);

  __ Bind(&retry);

  // Generate PACs using keys A and B.
  __ Mov(x0, pointer);
  __ Pacia(x0, modifier);

  __ Mov(x1, pointer);
  __ Pacib(x1, modifier);

  // Authenticate the pointers above.
  __ Mov(x2, x0);
  __ Autia(x2, modifier);

  __ Mov(x3, x1);
  __ Autib(x3, modifier);

  // Attempt to authenticate incorrect pointers.
  __ Mov(x4, x1);
  __ Autia(x4, modifier);

  __ Mov(x5, x0);
  __ Autib(x5, modifier);

  // Retry on collisions.
  __ Cmp(x0, x1);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x4, ZFlag, ne);
  __ Ccmp(pointer, x5, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(x0, x1);

    // Pointers correctly authenticated.
    ASSERT_EQUAL_64(pointer, x2);
    ASSERT_EQUAL_64(pointer, x3);

    // Pointers corrupted after failing to authenticate.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(0x0020000012345678, x4);
    ASSERT_EQUAL_64(0x0040000012345678, x5);
#else
    ASSERT_NOT_EQUAL_64(pointer, x4);
    ASSERT_NOT_EQUAL_64(pointer, x5);
#endif
  }
}


TEST(paciza_pacizb_autiza_autizb) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Register pointer = x24;
  Register retry_limit = x25;
  Label retry;

  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide, so retry a few times with different
  // pointers.
  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);

  __ Bind(&retry);

  // Generate PACs using keys A and B.
  __ Mov(x0, pointer);
  __ Paciza(x0);

  __ Mov(x1, pointer);
  __ Pacizb(x1);

  // Authenticate the pointers above.
  __ Mov(x2, x0);
  __ Autiza(x2);

  __ Mov(x3, x1);
  __ Autizb(x3);

  // Attempt to authenticate incorrect pointers.
  __ Mov(x4, x1);
  __ Autiza(x4);

  __ Mov(x5, x0);
  __ Autizb(x5);

  // Retry on collisions.
  __ Cmp(x0, x1);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x4, ZFlag, ne);
  __ Ccmp(pointer, x5, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(x0, x1);

    // Pointers correctly authenticated.
    ASSERT_EQUAL_64(pointer, x2);
    ASSERT_EQUAL_64(pointer, x3);

    // Pointers corrupted after failing to authenticate.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(0x0020000012345678, x4);
    ASSERT_EQUAL_64(0x0040000012345678, x5);
#else
    ASSERT_NOT_EQUAL_64(pointer, x4);
    ASSERT_NOT_EQUAL_64(pointer, x5);
#endif
  }
}


TEST(pacda_pacdb_autda_autdb) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Register pointer = x24;
  Register retry_limit = x25;
  Register modifier = x26;
  Label retry;

  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide, so retry a few times with different
  // pointers.
  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);
  __ Mov(modifier, 0x477d469dec0b8760);

  __ Bind(&retry);

  // Generate PACs using keys A and B.
  __ Mov(x0, pointer);
  __ Pacda(x0, modifier);

  __ Mov(x1, pointer);
  __ Pacdb(x1, modifier);

  // Authenticate the pointers above.
  __ Mov(x2, x0);
  __ Autda(x2, modifier);

  __ Mov(x3, x1);
  __ Autdb(x3, modifier);

  // Attempt to authenticate incorrect pointers.
  __ Mov(x4, x1);
  __ Autda(x4, modifier);

  __ Mov(x5, x0);
  __ Autdb(x5, modifier);

  // Retry on collisions.
  __ Cmp(x0, x1);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x4, ZFlag, ne);
  __ Ccmp(pointer, x5, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(x0, x1);

    // Pointers correctly authenticated.
    ASSERT_EQUAL_64(pointer, x2);
    ASSERT_EQUAL_64(pointer, x3);

    // Pointers corrupted after failing to authenticate.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(0x0020000012345678, x4);
    ASSERT_EQUAL_64(0x0040000012345678, x5);
#else
    ASSERT_NOT_EQUAL_64(pointer, x4);
    ASSERT_NOT_EQUAL_64(pointer, x5);
#endif
  }
}


TEST(pacdza_pacdzb_autdza_autdzb) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Register pointer = x24;
  Register retry_limit = x25;
  Label retry;

  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide, so retry a few times with different
  // pointers.
  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);

  __ Bind(&retry);

  // Generate PACs using keys A and B.
  __ Mov(x0, pointer);
  __ Pacdza(x0);

  __ Mov(x1, pointer);
  __ Pacdzb(x1);

  // Authenticate the pointers above.
  __ Mov(x2, x0);
  __ Autdza(x2);

  __ Mov(x3, x1);
  __ Autdzb(x3);

  // Attempt to authenticate incorrect pointers.
  __ Mov(x4, x1);
  __ Autdza(x4);

  __ Mov(x5, x0);
  __ Autdzb(x5);

  // Retry on collisions.
  __ Cmp(x0, x1);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x4, ZFlag, ne);
  __ Ccmp(pointer, x5, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(x0, x1);

    // Pointers correctly authenticated.
    ASSERT_EQUAL_64(pointer, x2);
    ASSERT_EQUAL_64(pointer, x3);

    // Pointers corrupted after failing to authenticate.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(0x0020000012345678, x4);
    ASSERT_EQUAL_64(0x0040000012345678, x5);
#else
    ASSERT_NOT_EQUAL_64(pointer, x4);
    ASSERT_NOT_EQUAL_64(pointer, x5);
#endif
  }
}


TEST(pacga_xpaci_xpacd) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth, CPUFeatures::kPAuthGeneric);

  START();

  Register pointer = x24;
  Register retry_limit = x25;
  Register modifier = x26;
  Label retry;

  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide, so retry a few times with different
  // pointers.
  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);
  __ Mov(modifier, 0x477d469dec0b8760);

  __ Bind(&retry);

  // Generate generic PAC.
  __ Pacga(x0, pointer, modifier);

  // Generate PACs using key A.
  __ Mov(x1, pointer);
  __ Mov(x2, pointer);
  __ Pacia(x1, modifier);
  __ Pacda(x2, modifier);

  // Strip PACs.
  __ Mov(x3, x1);
  __ Mov(x4, x2);
  __ Xpaci(x3);
  __ Xpacd(x4);

  // Retry on collisions.
  __ Cmp(x1, x2);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x2, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(pointer, x2);
    ASSERT_NOT_EQUAL_64(x1, x2);

    ASSERT_EQUAL_64(pointer, x3);
    ASSERT_EQUAL_64(pointer, x4);
  }
}

TEST(pac_sp_modifier) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  __ Mov(x0, 0x0000000012345678);
  __ Mov(x1, x0);
  __ Mov(x10, sp);

  // Generate PACs using sp and register containing a copy of sp.
  __ Pacia(x0, x10);
  __ Pacia(x1, sp);

  // Authenticate the pointers, exchanging (equal) modifiers.
  __ Mov(x2, x0);
  __ Mov(x3, x1);
  __ Autia(x2, sp);
  __ Autia(x3, x10);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(x0, x1);
    ASSERT_EQUAL_64(x2, x3);
  }
}

TEST(label) {
  SETUP();

  Label label_1, label_2, label_3, label_4;

  START();
  __ Mov(x0, 0x1);
  __ Mov(x1, 0x0);
  __ Mov(x22, lr);  // Save lr.

  __ B(&label_1);
  __ B(&label_1);
  __ B(&label_1);  // Multiple branches to the same label.
  __ Mov(x0, 0x0);
  __ Bind(&label_2);
  __ B(&label_3);  // Forward branch.
  __ Mov(x0, 0x0);
  __ Bind(&label_1);
  __ B(&label_2);  // Backward branch.
  __ Mov(x0, 0x0);
  __ Bind(&label_3);
  __ Bl(&label_4);
  END();

  __ Bind(&label_4);
  __ Mov(x1, 0x1);
  __ Mov(lr, x22);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1, x0);
    ASSERT_EQUAL_64(0x1, x1);
  }
}


TEST(label_2) {
  SETUP();

  Label label_1, label_2, label_3;
  Label first_jump_to_3;

  START();
  __ Mov(x0, 0x0);

  __ B(&label_1);
  ptrdiff_t offset_2 = masm.GetCursorOffset();
  __ Orr(x0, x0, 1 << 1);
  __ B(&label_3);
  ptrdiff_t offset_1 = masm.GetCursorOffset();
  __ Orr(x0, x0, 1 << 0);
  __ B(&label_2);
  ptrdiff_t offset_3 = masm.GetCursorOffset();
  __ Tbz(x0, 2, &first_jump_to_3);
  __ Orr(x0, x0, 1 << 3);
  __ Bind(&first_jump_to_3);
  __ Orr(x0, x0, 1 << 2);
  __ Tbz(x0, 3, &label_3);

  // Labels 1, 2, and 3 are bound before the current buffer offset. Branches to
  // label_1 and label_2 branch respectively forward and backward. Branches to
  // label 3 include both forward and backward branches.
  masm.BindToOffset(&label_1, offset_1);
  masm.BindToOffset(&label_2, offset_2);
  masm.BindToOffset(&label_3, offset_3);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xf, x0);
  }
}


TEST(adr) {
  SETUP();

  Label label_1, label_2, label_3, label_4;

  START();
  __ Mov(x0, 0x0);       // Set to non-zero to indicate failure.
  __ Adr(x1, &label_3);  // Set to zero to indicate success.

  __ Adr(x2, &label_1);  // Multiple forward references to the same label.
  __ Adr(x3, &label_1);
  __ Adr(x4, &label_1);

  __ Bind(&label_2);
  __ Eor(x5, x2, Operand(x3));  // Ensure that x2,x3 and x4 are identical.
  __ Eor(x6, x2, Operand(x4));
  __ Orr(x0, x0, Operand(x5));
  __ Orr(x0, x0, Operand(x6));
  __ Br(x2);  // label_1, label_3

  __ Bind(&label_3);
  __ Adr(x2, &label_3);  // Self-reference (offset 0).
  __ Eor(x1, x1, Operand(x2));
  __ Adr(x2, &label_4);  // Simple forward reference.
  __ Br(x2);             // label_4

  __ Bind(&label_1);
  __ Adr(x2, &label_3);  // Multiple reverse references to the same label.
  __ Adr(x3, &label_3);
  __ Adr(x4, &label_3);
  __ Adr(x5, &label_2);  // Simple reverse reference.
  __ Br(x5);             // label_2

  __ Bind(&label_4);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x0);
    ASSERT_EQUAL_64(0x0, x1);
  }
}


// Simple adrp tests: check that labels are linked and handled properly.
// This is similar to the adr test, but all the adrp instructions are put on the
// same page so that they return the same value.
TEST(adrp) {
  Label start;
  Label label_1, label_2, label_3;

  SETUP_CUSTOM(2 * kPageSize, PageOffsetDependentCode);
  START();

  // Waste space until the start of a page.
  {
    ExactAssemblyScope scope(&masm,
                             kPageSize,
                             ExactAssemblyScope::kMaximumSize);
    const uintptr_t kPageOffsetMask = kPageSize - 1;
    while ((masm.GetCursorAddress<uintptr_t>() & kPageOffsetMask) != 0) {
      __ b(&start);
    }
    __ bind(&start);
  }

  // Simple forward reference.
  __ Adrp(x0, &label_2);

  __ Bind(&label_1);

  // Multiple forward references to the same label.
  __ Adrp(x1, &label_3);
  __ Adrp(x2, &label_3);
  __ Adrp(x3, &label_3);

  __ Bind(&label_2);

  // Self-reference (offset 0).
  __ Adrp(x4, &label_2);

  __ Bind(&label_3);

  // Simple reverse reference.
  __ Adrp(x5, &label_1);

  // Multiple reverse references to the same label.
  __ Adrp(x6, &label_2);
  __ Adrp(x7, &label_2);
  __ Adrp(x8, &label_2);

  VIXL_ASSERT(masm.GetSizeOfCodeGeneratedSince(&start) < kPageSize);
  END();
  if (CAN_RUN()) {
    RUN();

    uint64_t expected = reinterpret_cast<uint64_t>(
        AlignDown(masm.GetLabelAddress<uint64_t*>(&start), kPageSize));
    ASSERT_EQUAL_64(expected, x0);
    ASSERT_EQUAL_64(expected, x1);
    ASSERT_EQUAL_64(expected, x2);
    ASSERT_EQUAL_64(expected, x3);
    ASSERT_EQUAL_64(expected, x4);
    ASSERT_EQUAL_64(expected, x5);
    ASSERT_EQUAL_64(expected, x6);
    ASSERT_EQUAL_64(expected, x7);
    ASSERT_EQUAL_64(expected, x8);
  }
}


static void AdrpPageBoundaryHelper(unsigned offset_into_page) {
  VIXL_ASSERT(offset_into_page < kPageSize);
  VIXL_ASSERT((offset_into_page % kInstructionSize) == 0);

  const uintptr_t kPageOffsetMask = kPageSize - 1;

  // The test label is always bound on page 0. Adrp instructions are generated
  // on pages from kStartPage to kEndPage (inclusive).
  const int kStartPage = -16;
  const int kEndPage = 16;
  const int kMaxCodeSize = (kEndPage - kStartPage + 2) * kPageSize;

  SETUP_CUSTOM(kMaxCodeSize, PageOffsetDependentCode);
  START();

  Label test;
  Label start;

  {
    ExactAssemblyScope scope(&masm,
                             kMaxCodeSize,
                             ExactAssemblyScope::kMaximumSize);
    // Initialize NZCV with `eq` flags.
    __ cmp(wzr, wzr);
    // Waste space until the start of a page.
    while ((masm.GetCursorAddress<uintptr_t>() & kPageOffsetMask) != 0) {
      __ b(&start);
    }

    // The first page.
    VIXL_STATIC_ASSERT(kStartPage < 0);
    {
      ExactAssemblyScope scope_page(&masm, kPageSize);
      __ bind(&start);
      __ adrp(x0, &test);
      __ adrp(x1, &test);
      for (size_t i = 2; i < (kPageSize / kInstructionSize); i += 2) {
        __ ccmp(x0, x1, NoFlag, eq);
        __ adrp(x1, &test);
      }
    }

    // Subsequent pages.
    VIXL_STATIC_ASSERT(kEndPage >= 0);
    for (int page = (kStartPage + 1); page <= kEndPage; page++) {
      ExactAssemblyScope scope_page(&masm, kPageSize);
      if (page == 0) {
        for (size_t i = 0; i < (kPageSize / kInstructionSize);) {
          if (i++ == (offset_into_page / kInstructionSize)) __ bind(&test);
          __ ccmp(x0, x1, NoFlag, eq);
          if (i++ == (offset_into_page / kInstructionSize)) __ bind(&test);
          __ adrp(x1, &test);
        }
      } else {
        for (size_t i = 0; i < (kPageSize / kInstructionSize); i += 2) {
          __ ccmp(x0, x1, NoFlag, eq);
          __ adrp(x1, &test);
        }
      }
    }
  }

  // Every adrp instruction pointed to the same label (`test`), so they should
  // all have produced the same result.

  END();
  if (CAN_RUN()) {
    RUN();

    uintptr_t expected =
        AlignDown(masm.GetLabelAddress<uintptr_t>(&test), kPageSize);
    ASSERT_EQUAL_64(expected, x0);
    ASSERT_EQUAL_64(expected, x1);
    ASSERT_EQUAL_NZCV(ZCFlag);
  }
}


// Test that labels are correctly referenced by adrp across page boundaries.
TEST(adrp_page_boundaries) {
  VIXL_STATIC_ASSERT(kPageSize == 4096);
  AdrpPageBoundaryHelper(kInstructionSize * 0);
  AdrpPageBoundaryHelper(kInstructionSize * 1);
  AdrpPageBoundaryHelper(kInstructionSize * 512);
  AdrpPageBoundaryHelper(kInstructionSize * 1022);
  AdrpPageBoundaryHelper(kInstructionSize * 1023);
}


static void AdrpOffsetHelper(int64_t offset) {
  const size_t kPageOffsetMask = kPageSize - 1;
  const int kMaxCodeSize = 2 * kPageSize;

  SETUP_CUSTOM(kMaxCodeSize, PageOffsetDependentCode);
  START();

  Label page;

  {
    ExactAssemblyScope scope(&masm,
                             kMaxCodeSize,
                             ExactAssemblyScope::kMaximumSize);
    // Initialize NZCV with `eq` flags.
    __ cmp(wzr, wzr);
    // Waste space until the start of a page.
    while ((masm.GetCursorAddress<uintptr_t>() & kPageOffsetMask) != 0) {
      __ b(&page);
    }
    __ bind(&page);

    {
      ExactAssemblyScope scope_page(&masm, kPageSize);
      // Every adrp instruction on this page should return the same value.
      __ adrp(x0, offset);
      __ adrp(x1, offset);
      for (size_t i = 2; i < kPageSize / kInstructionSize; i += 2) {
        __ ccmp(x0, x1, NoFlag, eq);
        __ adrp(x1, offset);
      }
    }
  }

  END();
  if (CAN_RUN()) {
    RUN();

    uintptr_t expected =
        masm.GetLabelAddress<uintptr_t>(&page) + (kPageSize * offset);
    ASSERT_EQUAL_64(expected, x0);
    ASSERT_EQUAL_64(expected, x1);
    ASSERT_EQUAL_NZCV(ZCFlag);
  }
}


// Check that adrp produces the correct result for a specific offset.
TEST(adrp_offset) {
  AdrpOffsetHelper(0);
  AdrpOffsetHelper(1);
  AdrpOffsetHelper(-1);
  AdrpOffsetHelper(4);
  AdrpOffsetHelper(-4);
  AdrpOffsetHelper(0x000fffff);
  AdrpOffsetHelper(-0x000fffff);
  AdrpOffsetHelper(-0x00100000);
}


TEST(branch_cond) {
  SETUP();

  Label done, wrong;

  START();
  __ Mov(x0, 0x1);
  __ Mov(x1, 0x1);
  __ Mov(x2, 0x8000000000000000);

  // For each 'cmp' instruction below, condition codes other than the ones
  // following it would branch.

  __ Cmp(x1, 0);
  __ B(&wrong, eq);
  __ B(&wrong, lo);
  __ B(&wrong, mi);
  __ B(&wrong, vs);
  __ B(&wrong, ls);
  __ B(&wrong, lt);
  __ B(&wrong, le);
  Label ok_1;
  __ B(&ok_1, ne);
  __ Mov(x0, 0x0);
  __ Bind(&ok_1);

  __ Cmp(x1, 1);
  __ B(&wrong, ne);
  __ B(&wrong, lo);
  __ B(&wrong, mi);
  __ B(&wrong, vs);
  __ B(&wrong, hi);
  __ B(&wrong, lt);
  __ B(&wrong, gt);
  Label ok_2;
  __ B(&ok_2, pl);
  __ Mov(x0, 0x0);
  __ Bind(&ok_2);

  __ Cmp(x1, 2);
  __ B(&wrong, eq);
  __ B(&wrong, hs);
  __ B(&wrong, pl);
  __ B(&wrong, vs);
  __ B(&wrong, hi);
  __ B(&wrong, ge);
  __ B(&wrong, gt);
  Label ok_3;
  __ B(&ok_3, vc);
  __ Mov(x0, 0x0);
  __ Bind(&ok_3);

  __ Cmp(x2, 1);
  __ B(&wrong, eq);
  __ B(&wrong, lo);
  __ B(&wrong, mi);
  __ B(&wrong, vc);
  __ B(&wrong, ls);
  __ B(&wrong, ge);
  __ B(&wrong, gt);
  Label ok_4;
  __ B(&ok_4, le);
  __ Mov(x0, 0x0);
  __ Bind(&ok_4);

  // The MacroAssembler does not allow al as a branch condition.
  Label ok_5;
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ b(&ok_5, al);
  }
  __ Mov(x0, 0x0);
  __ Bind(&ok_5);

  // The MacroAssembler does not allow nv as a branch condition.
  Label ok_6;
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ b(&ok_6, nv);
  }
  __ Mov(x0, 0x0);
  __ Bind(&ok_6);

  __ B(&done);

  __ Bind(&wrong);
  __ Mov(x0, 0x0);

  __ Bind(&done);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1, x0);
  }
}


TEST(branch_to_reg) {
  SETUP();

  // Test br.
  Label fn1, after_fn1;

  START();
  __ Mov(x29, lr);

  __ Mov(x1, 0);
  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Mov(x0, lr);
  __ Mov(x1, 42);
  __ Br(x0);

  __ Bind(&after_fn1);
  __ Bl(&fn1);

  // Test blr.
  Label fn2, after_fn2, after_bl2;

  __ Mov(x2, 0);
  __ B(&after_fn2);

  __ Bind(&fn2);
  __ Mov(x0, lr);
  __ Mov(x2, 84);
  __ Blr(x0);

  __ Bind(&after_fn2);
  __ Bl(&fn2);
  __ Bind(&after_bl2);
  __ Mov(x3, lr);
  __ Adr(x4, &after_bl2);
  __ Adr(x5, &after_fn2);

  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(x4, x0);
    ASSERT_EQUAL_64(x5, x3);
    ASSERT_EQUAL_64(42, x1);
    ASSERT_EQUAL_64(84, x2);
  }
}

TEST(branch_to_reg_auth_a) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Label fn1, after_fn1;

  __ Mov(x28, 0x477d469dec0b8760);
  __ Mov(x29, lr);

  __ Mov(x1, 0);
  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Mov(x0, lr);
  __ Mov(x1, 42);
  __ Pacia(x0, x28);
  __ Braa(x0, x28);

  __ Bind(&after_fn1);
  __ Bl(&fn1);

  Label fn2, after_fn2, after_bl2;

  __ Mov(x2, 0);
  __ B(&after_fn2);

  __ Bind(&fn2);
  __ Mov(x0, lr);
  __ Mov(x2, 84);
  __ Pacia(x0, x28);
  __ Blraa(x0, x28);

  __ Bind(&after_fn2);
  __ Bl(&fn2);
  __ Bind(&after_bl2);
  __ Mov(x3, lr);
  __ Adr(x4, &after_bl2);
  __ Adr(x5, &after_fn2);

  __ Xpaci(x0);
  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(x4, x0);
    ASSERT_EQUAL_64(x5, x3);
    ASSERT_EQUAL_64(42, x1);
    ASSERT_EQUAL_64(84, x2);
  }
}

TEST(return_to_reg_auth) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Label fn1, after_fn1;

  __ Mov(x28, sp);
  __ Mov(x29, lr);
  __ Mov(sp, 0x477d469dec0b8760);

  __ Mov(x0, 0);
  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Mov(x0, 42);
  __ Paciasp();
  __ Retaa();

  __ Bind(&after_fn1);
  __ Bl(&fn1);

  Label fn2, after_fn2;

  __ Mov(x1, 0);
  __ B(&after_fn2);

  __ Bind(&fn2);
  __ Mov(x1, 84);
  __ Pacibsp();
  __ Retab();

  __ Bind(&after_fn2);
  __ Bl(&fn2);

  __ Mov(sp, x28);
  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(42, x0);
    ASSERT_EQUAL_64(84, x1);
  }
}

TEST(return_to_reg_auth_guarded) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Label fn1, after_fn1;

  __ Mov(x28, sp);
  __ Mov(x29, lr);
  __ Mov(sp, 0x477d469dec0b8760);

  __ Mov(x0, 0);
  __ B(&after_fn1);

  __ Bind(&fn1, EmitPACIASP);
  __ Mov(x0, 42);
  __ Retaa();

  __ Bind(&after_fn1);
  __ Adr(x2, &fn1);
  __ Blr(x2);

  Label fn2, after_fn2;

  __ Mov(x1, 0);
  __ B(&after_fn2);

  __ Bind(&fn2, EmitPACIBSP);
  __ Mov(x1, 84);
  __ Retab();

  __ Bind(&after_fn2);
  __ Adr(x2, &fn2);
  __ Blr(x2);

  __ Mov(sp, x28);
  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
#endif
    // On hardware, we'll run the test anyway, but mark it as SKIPPED until
    // we've implemented a mechanism for marking Guarded pages.

    RUN();

    ASSERT_EQUAL_64(42, x0);
    ASSERT_EQUAL_64(84, x1);

#ifndef VIXL_INCLUDE_SIMULATOR_AARCH64
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

#ifdef VIXL_NEGATIVE_TESTING
TEST(branch_to_reg_auth_fail) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Label fn1, after_fn1;

  __ Mov(x29, lr);

  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Mov(x0, lr);
  __ Pacizb(x0);
  __ Blraaz(x0);

  __ Bind(&after_fn1);
  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide and BLRAAZ won't abort. To mitigate
  // this, we simply repeat the test a few more times.
  for (unsigned i = 0; i < 32; i++) {
    __ Bl(&fn1);
  }

  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    MUST_FAIL_WITH_MESSAGE(RUN(), "Failed to authenticate pointer.");
#else
    printf("SKIPPED: negative PAuth tests are unimplemented on hardware.");
#endif
  }
}
#endif  // VIXL_NEGATIVE_TESTING

#ifdef VIXL_NEGATIVE_TESTING
TEST(return_to_reg_auth_fail) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Label fn1, after_fn1;

  __ Mov(x28, sp);
  __ Mov(x29, lr);
  __ Mov(sp, 0x477d469dec0b8760);

  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Paciasp();
  __ Retab();

  __ Bind(&after_fn1);
  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide and RETAB won't abort. To mitigate
  // this, we simply repeat the test a few more times.
  for (unsigned i = 0; i < 32; i++) {
    __ Bl(&fn1);
  }

  __ Mov(sp, x28);
  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    MUST_FAIL_WITH_MESSAGE(RUN(), "Failed to authenticate pointer.");
#else
    printf("SKIPPED: negative PAuth tests are unimplemented on hardware.");
#endif
  }
}
#endif  // VIXL_NEGATIVE_TESTING

TEST(branch_to_reg_auth_a_zero) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();

  Label fn1, after_fn1;

  __ Mov(x29, lr);

  __ Mov(x1, 0);
  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Mov(x0, lr);
  __ Mov(x1, 42);
  __ Paciza(x0);
  __ Braaz(x0);

  __ Bind(&after_fn1);
  __ Bl(&fn1);

  Label fn2, after_fn2, after_bl2;

  __ Mov(x2, 0);
  __ B(&after_fn2);

  __ Bind(&fn2);
  __ Mov(x0, lr);
  __ Mov(x2, 84);
  __ Paciza(x0);
  __ Blraaz(x0);

  __ Bind(&after_fn2);
  __ Bl(&fn2);
  __ Bind(&after_bl2);
  __ Mov(x3, lr);
  __ Adr(x4, &after_bl2);
  __ Adr(x5, &after_fn2);

  __ Xpaci(x0);
  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(x4, x0);
    ASSERT_EQUAL_64(x5, x3);
    ASSERT_EQUAL_64(42, x1);
    ASSERT_EQUAL_64(84, x2);
  }
}


TEST(compare_branch) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0);
  __ Mov(x2, 0);
  __ Mov(x3, 0);
  __ Mov(x4, 0);
  __ Mov(x5, 0);
  __ Mov(x16, 0);
  __ Mov(x17, 42);

  Label zt, zt_end;
  __ Cbz(w16, &zt);
  __ B(&zt_end);
  __ Bind(&zt);
  __ Mov(x0, 1);
  __ Bind(&zt_end);

  Label zf, zf_end;
  __ Cbz(x17, &zf);
  __ B(&zf_end);
  __ Bind(&zf);
  __ Mov(x1, 1);
  __ Bind(&zf_end);

  Label nzt, nzt_end;
  __ Cbnz(w17, &nzt);
  __ B(&nzt_end);
  __ Bind(&nzt);
  __ Mov(x2, 1);
  __ Bind(&nzt_end);

  Label nzf, nzf_end;
  __ Cbnz(x16, &nzf);
  __ B(&nzf_end);
  __ Bind(&nzf);
  __ Mov(x3, 1);
  __ Bind(&nzf_end);

  __ Mov(x18, 0xffffffff00000000);

  Label a, a_end;
  __ Cbz(w18, &a);
  __ B(&a_end);
  __ Bind(&a);
  __ Mov(x4, 1);
  __ Bind(&a_end);

  Label b, b_end;
  __ Cbnz(w18, &b);
  __ B(&b_end);
  __ Bind(&b);
  __ Mov(x5, 1);
  __ Bind(&b_end);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x0);
    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(1, x2);
    ASSERT_EQUAL_64(0, x3);
    ASSERT_EQUAL_64(1, x4);
    ASSERT_EQUAL_64(0, x5);
  }
}


TEST(test_branch) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0);
  __ Mov(x2, 0);
  __ Mov(x3, 0);
  __ Mov(x16, 0xaaaaaaaaaaaaaaaa);

  Label bz, bz_end;
  __ Tbz(w16, 0, &bz);
  __ B(&bz_end);
  __ Bind(&bz);
  __ Mov(x0, 1);
  __ Bind(&bz_end);

  Label bo, bo_end;
  __ Tbz(x16, 63, &bo);
  __ B(&bo_end);
  __ Bind(&bo);
  __ Mov(x1, 1);
  __ Bind(&bo_end);

  Label nbz, nbz_end;
  __ Tbnz(x16, 61, &nbz);
  __ B(&nbz_end);
  __ Bind(&nbz);
  __ Mov(x2, 1);
  __ Bind(&nbz_end);

  Label nbo, nbo_end;
  __ Tbnz(w16, 2, &nbo);
  __ B(&nbo_end);
  __ Bind(&nbo);
  __ Mov(x3, 1);
  __ Bind(&nbo_end);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x0);
    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(1, x2);
    ASSERT_EQUAL_64(0, x3);
  }
}


TEST(branch_type) {
  SETUP();

  Label fail, done;

  START();
  __ Mov(x0, 0x0);
  __ Mov(x10, 0x7);
  __ Mov(x11, 0x0);

  // Test non taken branches.
  __ Cmp(x10, 0x7);
  __ B(&fail, ne);
  __ B(&fail, never);
  __ B(&fail, reg_zero, x10);
  __ B(&fail, reg_not_zero, x11);
  __ B(&fail, reg_bit_clear, x10, 0);
  __ B(&fail, reg_bit_set, x10, 3);

  // Test taken branches.
  Label l1, l2, l3, l4, l5;
  __ Cmp(x10, 0x7);
  __ B(&l1, eq);
  __ B(&fail);
  __ Bind(&l1);
  __ B(&l2, always);
  __ B(&fail);
  __ Bind(&l2);
  __ B(&l3, reg_not_zero, x10);
  __ B(&fail);
  __ Bind(&l3);
  __ B(&l4, reg_bit_clear, x10, 15);
  __ B(&fail);
  __ Bind(&l4);
  __ B(&l5, reg_bit_set, x10, 1);
  __ B(&fail);
  __ Bind(&l5);

  __ B(&done);

  __ Bind(&fail);
  __ Mov(x0, 0x1);

  __ Bind(&done);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x0);
  }
}

enum MTEStgAttribute {
  StgNoSideEffect = 0,
  StgPairTag = 1,
  StgZeroing = 2,
  StgPairReg = 4
};

// Support st2g, stg, stz2g and stzg.
template <typename Op>
static void MTEStoreTagHelper(Op op,
                              AddrMode addr_mode,
                              int attr = StgNoSideEffect) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);
  START();

  // This method does nothing when the size is zero. i.e. stg and st2g.
  // Reserve x9 and x10.
  auto LoadDataAndSum = [&](Register reg, int off, unsigned size_in_bytes) {
    for (unsigned j = 0; j < size_in_bytes / kXRegSizeInBytes; j++) {
      __ Ldr(x9, MemOperand(reg, off));
      __ Add(x10, x9, x10);
      off += kXRegSizeInBytes;
    }
  };

  // Initialize registers to zero.
  for (int i = 0; i < 29; i++) {
    __ Mov(XRegister(i), 0);
  }

  Register base = x28;
  Register base_tag = x27;
  uint32_t* data_ptr = nullptr;
  const int data_size = 640;
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  data_ptr = reinterpret_cast<uint32_t*>(
      simulator.Mmap(NULL,
                     data_size * sizeof(uint32_t),
                     PROT_READ | PROT_WRITE | PROT_MTE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1,
                     0));

  VIXL_ASSERT(data_ptr != nullptr);
  uint32_t* untagged_ptr = AddressUntag(data_ptr);
  memset(untagged_ptr, 0xae, data_size * sizeof(uint32_t));
#else
// TODO: Port the memory allocation to work on MTE supported platform natively.
// Note that `CAN_RUN` prevents running in MTE-unsupported environments.
#endif

  __ Mov(base, reinterpret_cast<uint64_t>(&data_ptr[data_size / 2]));

  VIXL_STATIC_ASSERT(kMTETagGranuleInBytes == 16);
  const int tag_granule = kMTETagGranuleInBytes;
  int size = ((attr & StgZeroing) != 0) ? tag_granule : 0;
  // lsb of MTE tag field.
  const int tag_lsb = 56;

  for (int i = 1; i < 7; i++) {
    uint64_t tag = static_cast<uint64_t>(i) << tag_lsb;
    int offset = 2 * i * tag_granule;
    __ Mov(XRegister(i), tag);
    (masm.*op)(XRegister(i), MemOperand(base, offset, addr_mode));

    // The address tag has been changed after the execution of store tag
    // instructions, so update the pointer tag as well.
    __ Bic(base_tag, base, 0x0f00000000000000);
    __ Orr(base_tag, base_tag, XRegister(i));

    switch (addr_mode) {
      case Offset:
        __ Ldg(XRegister(i + 10), MemOperand(base_tag, offset));
        LoadDataAndSum(base_tag, offset, size);
        if ((attr & StgPairTag) != 0) {
          __ Ldg(XRegister(i + 20), MemOperand(base_tag, offset + tag_granule));
          LoadDataAndSum(base_tag, offset + tag_granule, size);
        }
        break;

      case PreIndex:
        __ Ldg(XRegister(i + 10), MemOperand(base_tag));
        LoadDataAndSum(base_tag, 0, size);
        if ((attr & StgPairTag) != 0) {
          __ Ldg(XRegister(i + 20), MemOperand(base_tag, tag_granule));
          LoadDataAndSum(base_tag, tag_granule, size);
        }
        break;

      case PostIndex:
        __ Ldg(XRegister(i + 10), MemOperand(base_tag, -offset));
        LoadDataAndSum(base_tag, -offset, size);
        if ((attr & StgPairTag) != 0) {
          __ Ldg(XRegister(i + 20),
                 MemOperand(base_tag, -offset + tag_granule));
          LoadDataAndSum(base_tag, -offset + tag_granule, size);
        }
        break;

      default:
        VIXL_UNIMPLEMENTED();
        break;
    }

    // Switch the sign to test both positive and negative offsets.
    offset = -offset;
  }

  int pos_offset = 304;
  int neg_offset = -256;

  // Backup stack pointer and others.
  __ Mov(x7, sp);
  __ Mov(base_tag, base);

  // Test the cases where operand is the stack pointer.
  __ Mov(x8, 11UL << tag_lsb);
  __ Mov(sp, x8);
  (masm.*op)(sp, MemOperand(base, neg_offset, addr_mode));

  // Synthesise a new address with new tag and assign to the stack pointer.
  __ Add(sp, base_tag, 32);
  (masm.*op)(x8, MemOperand(sp, pos_offset, addr_mode));

  switch (addr_mode) {
    case Offset:
      __ Ldg(x17, MemOperand(base, neg_offset));
      __ Ldg(x19, MemOperand(sp, pos_offset));
      if ((attr & StgPairTag) != 0) {
        __ Ldg(x18, MemOperand(base, neg_offset + tag_granule));
        __ Ldg(x20, MemOperand(sp, pos_offset + tag_granule));
      }
      break;
    case PreIndex:
      __ Ldg(x17, MemOperand(base));
      __ Ldg(x19, MemOperand(sp));
      if ((attr & StgPairTag) != 0) {
        __ Ldg(x18, MemOperand(base, tag_granule));
        __ Ldg(x20, MemOperand(sp, tag_granule));
      }
      break;
    case PostIndex:
      __ Ldg(x17, MemOperand(base, -neg_offset));
      __ Ldg(x19, MemOperand(sp, -pos_offset));
      if ((attr & StgPairTag) != 0) {
        __ Ldg(x18, MemOperand(base, -neg_offset + tag_granule));
        __ Ldg(x20, MemOperand(sp, -pos_offset + tag_granule));
      }
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }

  // Restore stack pointer.
  __ Mov(sp, x7);

  END();

  if (CAN_RUN()) {
#ifndef VIXL_INCLUDE_SIMULATOR_AARCH64
    VIXL_UNIMPLEMENTED();
#endif
    RUN();

    ASSERT_EQUAL_64(1UL << tag_lsb, x11);
    ASSERT_EQUAL_64(2UL << tag_lsb, x12);
    ASSERT_EQUAL_64(3UL << tag_lsb, x13);
    ASSERT_EQUAL_64(4UL << tag_lsb, x14);
    ASSERT_EQUAL_64(5UL << tag_lsb, x15);
    ASSERT_EQUAL_64(6UL << tag_lsb, x16);
    ASSERT_EQUAL_64(11UL << tag_lsb, x17);
    ASSERT_EQUAL_64(11UL << tag_lsb, x19);

    if ((attr & StgPairTag) != 0) {
      ASSERT_EQUAL_64(1UL << tag_lsb, x21);
      ASSERT_EQUAL_64(2UL << tag_lsb, x22);
      ASSERT_EQUAL_64(3UL << tag_lsb, x23);
      ASSERT_EQUAL_64(4UL << tag_lsb, x24);
      ASSERT_EQUAL_64(5UL << tag_lsb, x25);
      ASSERT_EQUAL_64(6UL << tag_lsb, x26);
      ASSERT_EQUAL_64(11UL << tag_lsb, x18);
      ASSERT_EQUAL_64(11UL << tag_lsb, x20);
    }

    if ((attr & StgZeroing) != 0) {
      ASSERT_EQUAL_64(0, x10);
    }
  }

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  simulator.Munmap(data_ptr, data_size, PROT_MTE);
#endif
}

TEST(st2g_ldg) {
  MTEStoreTagHelper(&MacroAssembler::St2g, Offset, StgPairTag);
  MTEStoreTagHelper(&MacroAssembler::St2g, PreIndex, StgPairTag);
  MTEStoreTagHelper(&MacroAssembler::St2g, PostIndex, StgPairTag);
}

TEST(stg_ldg) {
  MTEStoreTagHelper(&MacroAssembler::Stg, Offset);
  MTEStoreTagHelper(&MacroAssembler::Stg, PreIndex);
  MTEStoreTagHelper(&MacroAssembler::Stg, PostIndex);
}

TEST(stz2g_ldg) {
  MTEStoreTagHelper(&MacroAssembler::Stz2g, Offset, StgPairTag | StgZeroing);
  MTEStoreTagHelper(&MacroAssembler::Stz2g, PreIndex, StgPairTag | StgZeroing);
  MTEStoreTagHelper(&MacroAssembler::Stz2g, PostIndex, StgPairTag | StgZeroing);
}

TEST(stzg_ldg) {
  MTEStoreTagHelper(&MacroAssembler::Stzg, Offset, StgZeroing);
  MTEStoreTagHelper(&MacroAssembler::Stzg, PreIndex, StgZeroing);
  MTEStoreTagHelper(&MacroAssembler::Stzg, PostIndex, StgZeroing);
}

TEST(stgp_ldg) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);
  START();

  // Initialize registers to zero.
  for (int i = 0; i < 29; i++) {
    __ Mov(XRegister(i), 0);
  }

  // Reserve x14 and x15.
  auto LoadDataAndSum = [&](Register reg, int off) {
    __ Ldr(x14, MemOperand(reg, off));
    __ Add(x15, x14, x15);
    __ Ldr(x14, MemOperand(reg, off + static_cast<int>(kXRegSizeInBytes)));
    __ Add(x15, x14, x15);
  };

  Register base = x28;
  uint32_t* data_ptr = nullptr;
  const int data_size = 640;
  uint64_t init_tag = 17;
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  data_ptr = reinterpret_cast<uint32_t*>(
      simulator.Mmap(NULL,
                     data_size * sizeof(uint32_t),
                     PROT_READ | PROT_WRITE | PROT_MTE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1,
                     0));

  VIXL_ASSERT(data_ptr != nullptr);
  init_tag = CPU::GetPointerTag(data_ptr);
  uint32_t* untagged_ptr = AddressUntag(data_ptr);
  memset(untagged_ptr, 0xc9, data_size * sizeof(uint32_t));
#else
// TODO: Port the memory allocation to work on MTE supported platform natively.
// Note that `CAN_RUN` prevents running in MTE-unsupported environments.
#endif

  __ Mov(base, reinterpret_cast<uint64_t>(&data_ptr[data_size / 2]));

  // lsb of MTE tag field.
  const int tag_lsb = 56;
  for (int i = 0; i < 11; i++) {
    // <63..60> <59..56> <55........5> <4..0>
    //        0       i             0      i
    __ Mov(XRegister(i), i | (static_cast<uint64_t>(i) << tag_lsb));
  }

  // Backup stack pointer.
  __ Mov(x0, sp);

  int offset = -16;
  __ Addg(base, base, 0, 1);
  __ Stgp(x1, x2, MemOperand(base, offset, Offset));
  // Make sure `ldg` works well with address that isn't tag-granule aligned.
  __ Add(x29, base, 8);
  __ Ldg(x18, MemOperand(x29, offset));
  LoadDataAndSum(base, offset);

  offset = -304;
  __ Addg(base, base, 0, 1);
  __ Stgp(x2, x3, MemOperand(base, offset, Offset));
  __ Add(x29, base, 4);
  __ Ldg(x19, MemOperand(x29, offset));
  LoadDataAndSum(base, offset);

  offset = 128;
  __ Addg(base, base, 0, 1);
  __ Stgp(x3, x4, MemOperand(base, offset, Offset));
  __ Mov(sp, base);
  __ Ldg(x20, MemOperand(sp, offset));
  LoadDataAndSum(base, offset);

  offset = -48;
  __ Addg(base, base, 0, 1);
  __ Stgp(x4, x5, MemOperand(base, offset, PreIndex));
  __ Add(x29, base, 8);
  __ Ldg(x21, MemOperand(x29));
  LoadDataAndSum(base, 0);

  offset = 64;
  __ Addg(base, base, 0, 1);
  __ Stgp(x5, x6, MemOperand(base, offset, PreIndex));
  __ Add(x29, base, 4);
  __ Ldg(x22, MemOperand(x29));
  LoadDataAndSum(base, 0);

  offset = -288;
  __ Addg(base, base, 0, 1);
  __ Stgp(x6, x7, MemOperand(base, offset, PreIndex));
  __ Mov(sp, base);
  __ Ldg(x23, MemOperand(sp));
  LoadDataAndSum(base, 0);

  offset = -96;
  __ Addg(base, base, 0, 1);
  __ Stgp(x7, x8, MemOperand(base, offset, PostIndex));
  __ Add(x29, base, 8);
  __ Ldg(x24, MemOperand(x29, -offset));
  LoadDataAndSum(base, -offset);

  offset = 80;
  __ Addg(base, base, 0, 1);
  __ Stgp(x8, x9, MemOperand(base, offset, PostIndex));
  __ Add(x29, base, 4);
  __ Ldg(x25, MemOperand(x29, -offset));
  LoadDataAndSum(base, -offset);

  offset = -224;
  __ Addg(base, base, 0, 1);
  __ Stgp(x9, x10, MemOperand(base, offset, PostIndex));
  __ Mov(sp, base);
  __ Ldg(x26, MemOperand(sp, -offset));
  LoadDataAndSum(base, -offset);

  __ Mov(sp, x0);

  END();

  if (CAN_RUN()) {
#ifndef VIXL_INCLUDE_SIMULATOR_AARCH64
    VIXL_UNIMPLEMENTED();
#endif
    RUN();

    const uint64_t k = kMTETagGranuleInBytes;
    USE(k);
    ASSERT_EQUAL_64(((init_tag + 1) % k) << tag_lsb, x18);
    ASSERT_EQUAL_64(((init_tag + 2) % k) << tag_lsb, x19);
    ASSERT_EQUAL_64(((init_tag + 3) % k) << tag_lsb, x20);
    ASSERT_EQUAL_64(((init_tag + 4) % k) << tag_lsb, x21);
    ASSERT_EQUAL_64(((init_tag + 5) % k) << tag_lsb, x22);
    ASSERT_EQUAL_64(((init_tag + 6) % k) << tag_lsb, x23);
    ASSERT_EQUAL_64(((init_tag + 7) % k) << tag_lsb, x24);
    ASSERT_EQUAL_64(((init_tag + 8) % k) << tag_lsb, x25);
    ASSERT_EQUAL_64(((init_tag + 9) % k) << tag_lsb, x26);

    // We store 1, 2, 2, 3, 3, 4, ....9, 9, 10 to memory, so the total sum of
    // these values is 1 + (2 * (2 + 9) * 8 / 2) + 10 = 99.
    ASSERT_EQUAL_64((99UL << tag_lsb | 99UL), x15);
  }

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  simulator.Munmap(data_ptr, data_size, PROT_MTE);
#endif
}

TEST(ldr_str_offset) {
  SETUP();

  uint64_t src[2] = {0xfedcba9876543210, 0x0123456789abcdef};
  uint64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base);
  __ Mov(x18, dst_base);
  __ Ldr(w0, MemOperand(x17));
  __ Str(w0, MemOperand(x18));
  __ Ldr(w1, MemOperand(x17, 4));
  __ Str(w1, MemOperand(x18, 12));
  __ Ldr(x2, MemOperand(x17, 8));
  __ Str(x2, MemOperand(x18, 16));
  __ Ldrb(w3, MemOperand(x17, 1));
  __ Strb(w3, MemOperand(x18, 25));
  __ Ldrh(w4, MemOperand(x17, 2));
  __ Strh(w4, MemOperand(x18, 33));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x76543210, x0);
    ASSERT_EQUAL_64(0x76543210, dst[0]);
    ASSERT_EQUAL_64(0xfedcba98, x1);
    ASSERT_EQUAL_64(0xfedcba9800000000, dst[1]);
    ASSERT_EQUAL_64(0x0123456789abcdef, x2);
    ASSERT_EQUAL_64(0x0123456789abcdef, dst[2]);
    ASSERT_EQUAL_64(0x32, x3);
    ASSERT_EQUAL_64(0x3200, dst[3]);
    ASSERT_EQUAL_64(0x7654, x4);
    ASSERT_EQUAL_64(0x765400, dst[4]);
    ASSERT_EQUAL_64(src_base, x17);
    ASSERT_EQUAL_64(dst_base, x18);
  }
}


TEST(ldr_str_wide) {
  SETUP();

  uint32_t src[8192];
  uint32_t dst[8192];
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);
  memset(src, 0xaa, 8192 * sizeof(src[0]));
  memset(dst, 0xaa, 8192 * sizeof(dst[0]));
  src[0] = 0;
  src[6144] = 6144;
  src[8191] = 8191;

  START();
  __ Mov(x22, src_base);
  __ Mov(x23, dst_base);
  __ Mov(x24, src_base);
  __ Mov(x25, dst_base);
  __ Mov(x26, src_base);
  __ Mov(x27, dst_base);

  __ Ldr(w0, MemOperand(x22, 8191 * sizeof(src[0])));
  __ Str(w0, MemOperand(x23, 8191 * sizeof(dst[0])));
  __ Ldr(w1, MemOperand(x24, 4096 * sizeof(src[0]), PostIndex));
  __ Str(w1, MemOperand(x25, 4096 * sizeof(dst[0]), PostIndex));
  __ Ldr(w2, MemOperand(x26, 6144 * sizeof(src[0]), PreIndex));
  __ Str(w2, MemOperand(x27, 6144 * sizeof(dst[0]), PreIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(8191, w0);
    ASSERT_EQUAL_32(8191, dst[8191]);
    ASSERT_EQUAL_64(src_base, x22);
    ASSERT_EQUAL_64(dst_base, x23);
    ASSERT_EQUAL_32(0, w1);
    ASSERT_EQUAL_32(0, dst[0]);
    ASSERT_EQUAL_64(src_base + 4096 * sizeof(src[0]), x24);
    ASSERT_EQUAL_64(dst_base + 4096 * sizeof(dst[0]), x25);
    ASSERT_EQUAL_32(6144, w2);
    ASSERT_EQUAL_32(6144, dst[6144]);
    ASSERT_EQUAL_64(src_base + 6144 * sizeof(src[0]), x26);
    ASSERT_EQUAL_64(dst_base + 6144 * sizeof(dst[0]), x27);
  }
}


TEST(ldr_str_preindex) {
  SETUP();

  uint64_t src[2] = {0xfedcba9876543210, 0x0123456789abcdef};
  uint64_t dst[6] = {0, 0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base);
  __ Mov(x18, dst_base);
  __ Mov(x19, src_base);
  __ Mov(x20, dst_base);
  __ Mov(x21, src_base + 16);
  __ Mov(x22, dst_base + 40);
  __ Mov(x23, src_base);
  __ Mov(x24, dst_base);
  __ Mov(x25, src_base);
  __ Mov(x26, dst_base);
  __ Ldr(w0, MemOperand(x17, 4, PreIndex));
  __ Str(w0, MemOperand(x18, 12, PreIndex));
  __ Ldr(x1, MemOperand(x19, 8, PreIndex));
  __ Str(x1, MemOperand(x20, 16, PreIndex));
  __ Ldr(w2, MemOperand(x21, -4, PreIndex));
  __ Str(w2, MemOperand(x22, -4, PreIndex));
  __ Ldrb(w3, MemOperand(x23, 1, PreIndex));
  __ Strb(w3, MemOperand(x24, 25, PreIndex));
  __ Ldrh(w4, MemOperand(x25, 3, PreIndex));
  __ Strh(w4, MemOperand(x26, 41, PreIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xfedcba98, x0);
    ASSERT_EQUAL_64(0xfedcba9800000000, dst[1]);
    ASSERT_EQUAL_64(0x0123456789abcdef, x1);
    ASSERT_EQUAL_64(0x0123456789abcdef, dst[2]);
    ASSERT_EQUAL_64(0x01234567, x2);
    ASSERT_EQUAL_64(0x0123456700000000, dst[4]);
    ASSERT_EQUAL_64(0x32, x3);
    ASSERT_EQUAL_64(0x3200, dst[3]);
    ASSERT_EQUAL_64(0x9876, x4);
    ASSERT_EQUAL_64(0x987600, dst[5]);
    ASSERT_EQUAL_64(src_base + 4, x17);
    ASSERT_EQUAL_64(dst_base + 12, x18);
    ASSERT_EQUAL_64(src_base + 8, x19);
    ASSERT_EQUAL_64(dst_base + 16, x20);
    ASSERT_EQUAL_64(src_base + 12, x21);
    ASSERT_EQUAL_64(dst_base + 36, x22);
    ASSERT_EQUAL_64(src_base + 1, x23);
    ASSERT_EQUAL_64(dst_base + 25, x24);
    ASSERT_EQUAL_64(src_base + 3, x25);
    ASSERT_EQUAL_64(dst_base + 41, x26);
  }
}


TEST(ldr_str_postindex) {
  SETUP();

  uint64_t src[2] = {0xfedcba9876543210, 0x0123456789abcdef};
  uint64_t dst[6] = {0, 0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base + 4);
  __ Mov(x18, dst_base + 12);
  __ Mov(x19, src_base + 8);
  __ Mov(x20, dst_base + 16);
  __ Mov(x21, src_base + 8);
  __ Mov(x22, dst_base + 32);
  __ Mov(x23, src_base + 1);
  __ Mov(x24, dst_base + 25);
  __ Mov(x25, src_base + 3);
  __ Mov(x26, dst_base + 41);
  __ Ldr(w0, MemOperand(x17, 4, PostIndex));
  __ Str(w0, MemOperand(x18, 12, PostIndex));
  __ Ldr(x1, MemOperand(x19, 8, PostIndex));
  __ Str(x1, MemOperand(x20, 16, PostIndex));
  __ Ldr(x2, MemOperand(x21, -8, PostIndex));
  __ Str(x2, MemOperand(x22, -32, PostIndex));
  __ Ldrb(w3, MemOperand(x23, 1, PostIndex));
  __ Strb(w3, MemOperand(x24, 5, PostIndex));
  __ Ldrh(w4, MemOperand(x25, -3, PostIndex));
  __ Strh(w4, MemOperand(x26, -41, PostIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xfedcba98, x0);
    ASSERT_EQUAL_64(0xfedcba9800000000, dst[1]);
    ASSERT_EQUAL_64(0x0123456789abcdef, x1);
    ASSERT_EQUAL_64(0x0123456789abcdef, dst[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, x2);
    ASSERT_EQUAL_64(0x0123456789abcdef, dst[4]);
    ASSERT_EQUAL_64(0x32, x3);
    ASSERT_EQUAL_64(0x3200, dst[3]);
    ASSERT_EQUAL_64(0x9876, x4);
    ASSERT_EQUAL_64(0x987600, dst[5]);
    ASSERT_EQUAL_64(src_base + 8, x17);
    ASSERT_EQUAL_64(dst_base + 24, x18);
    ASSERT_EQUAL_64(src_base + 16, x19);
    ASSERT_EQUAL_64(dst_base + 32, x20);
    ASSERT_EQUAL_64(src_base, x21);
    ASSERT_EQUAL_64(dst_base, x22);
    ASSERT_EQUAL_64(src_base + 2, x23);
    ASSERT_EQUAL_64(dst_base + 30, x24);
    ASSERT_EQUAL_64(src_base, x25);
    ASSERT_EQUAL_64(dst_base, x26);
  }
}


TEST(ldr_str_largeindex) {
  SETUP();

  // This value won't fit in the immediate offset field of ldr/str instructions.
  int largeoffset = 0xabcdef;

  int64_t data[3] = {0x1122334455667788, 0, 0};
  uint64_t base_addr = reinterpret_cast<uintptr_t>(data);
  uint64_t drifted_addr = base_addr - largeoffset;

  // This test checks that we we can use large immediate offsets when
  // using PreIndex or PostIndex addressing mode of the MacroAssembler
  // Ldr/Str instructions.

  START();
  __ Mov(x19, drifted_addr);
  __ Ldr(x0, MemOperand(x19, largeoffset, PreIndex));

  __ Mov(x20, base_addr);
  __ Ldr(x1, MemOperand(x20, largeoffset, PostIndex));

  __ Mov(x21, drifted_addr);
  __ Str(x0, MemOperand(x21, largeoffset + 8, PreIndex));

  __ Mov(x22, base_addr + 16);
  __ Str(x0, MemOperand(x22, largeoffset, PostIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1122334455667788, data[0]);
    ASSERT_EQUAL_64(0x1122334455667788, data[1]);
    ASSERT_EQUAL_64(0x1122334455667788, data[2]);
    ASSERT_EQUAL_64(0x1122334455667788, x0);
    ASSERT_EQUAL_64(0x1122334455667788, x1);

    ASSERT_EQUAL_64(base_addr, x19);
    ASSERT_EQUAL_64(base_addr + largeoffset, x20);
    ASSERT_EQUAL_64(base_addr + 8, x21);
    ASSERT_EQUAL_64(base_addr + 16 + largeoffset, x22);
  }
}


TEST(load_signed) {
  SETUP();

  uint32_t src[2] = {0x80008080, 0x7fff7f7f};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);

  START();
  __ Mov(x24, src_base);
  __ Ldrsb(w0, MemOperand(x24));
  __ Ldrsb(w1, MemOperand(x24, 4));
  __ Ldrsh(w2, MemOperand(x24));
  __ Ldrsh(w3, MemOperand(x24, 4));
  __ Ldrsb(x4, MemOperand(x24));
  __ Ldrsb(x5, MemOperand(x24, 4));
  __ Ldrsh(x6, MemOperand(x24));
  __ Ldrsh(x7, MemOperand(x24, 4));
  __ Ldrsw(x8, MemOperand(x24));
  __ Ldrsw(x9, MemOperand(x24, 4));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffff80, x0);
    ASSERT_EQUAL_64(0x0000007f, x1);
    ASSERT_EQUAL_64(0xffff8080, x2);
    ASSERT_EQUAL_64(0x00007f7f, x3);
    ASSERT_EQUAL_64(0xffffffffffffff80, x4);
    ASSERT_EQUAL_64(0x000000000000007f, x5);
    ASSERT_EQUAL_64(0xffffffffffff8080, x6);
    ASSERT_EQUAL_64(0x0000000000007f7f, x7);
    ASSERT_EQUAL_64(0xffffffff80008080, x8);
    ASSERT_EQUAL_64(0x000000007fff7f7f, x9);
  }
}


TEST(load_store_regoffset) {
  SETUP();

  uint32_t src[3] = {1, 2, 3};
  uint32_t dst[4] = {0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Mov(x18, src_base + 3 * sizeof(src[0]));
  __ Mov(x19, dst_base + 3 * sizeof(dst[0]));
  __ Mov(x20, dst_base + 4 * sizeof(dst[0]));
  __ Mov(x24, 0);
  __ Mov(x25, 4);
  __ Mov(x26, -4);
  __ Mov(x27, 0xfffffffc);  // 32-bit -4.
  __ Mov(x28, 0xfffffffe);  // 32-bit -2.
  __ Mov(x29, 0xffffffff);  // 32-bit -1.

  __ Ldr(w0, MemOperand(x16, x24));
  __ Ldr(x1, MemOperand(x16, x25));
  __ Ldr(w2, MemOperand(x18, x26));
  __ Ldr(w3, MemOperand(x18, x27, SXTW));
  __ Ldr(w4, MemOperand(x18, x28, SXTW, 2));
  __ Str(w0, MemOperand(x17, x24));
  __ Str(x1, MemOperand(x17, x25));
  __ Str(w2, MemOperand(x20, x29, SXTW, 2));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x0);
    ASSERT_EQUAL_64(0x0000000300000002, x1);
    ASSERT_EQUAL_64(3, x2);
    ASSERT_EQUAL_64(3, x3);
    ASSERT_EQUAL_64(2, x4);
    ASSERT_EQUAL_32(1, dst[0]);
    ASSERT_EQUAL_32(2, dst[1]);
    ASSERT_EQUAL_32(3, dst[2]);
    ASSERT_EQUAL_32(3, dst[3]);
  }
}


TEST(load_pauth) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  uint64_t src[4] = {1, 2, 3, 4};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, src_base);
  __ Mov(x18, src_base + 4 * sizeof(src[0]));
  __ Mov(x19, src_base + 4 * sizeof(src[0]));

  // Add PAC codes to addresses
  __ Pacdza(x16);
  __ Pacdzb(x17);
  __ Pacdza(x18);
  __ Pacdzb(x19);

  __ Ldraa(x0, MemOperand(x16));
  __ Ldraa(x1, MemOperand(x16, sizeof(src[0])));
  __ Ldraa(x2, MemOperand(x16, 2 * sizeof(src[0]), PreIndex));
  __ Ldraa(x3, MemOperand(x18, -sizeof(src[0])));
  __ Ldrab(x4, MemOperand(x17));
  __ Ldrab(x5, MemOperand(x17, sizeof(src[0])));
  __ Ldrab(x6, MemOperand(x17, 2 * sizeof(src[0]), PreIndex));
  __ Ldrab(x7, MemOperand(x19, -sizeof(src[0])));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x0);
    ASSERT_EQUAL_64(2, x1);
    ASSERT_EQUAL_64(3, x2);
    ASSERT_EQUAL_64(4, x3);
    ASSERT_EQUAL_64(1, x4);
    ASSERT_EQUAL_64(2, x5);
    ASSERT_EQUAL_64(3, x6);
    ASSERT_EQUAL_64(4, x7);
    ASSERT_EQUAL_64(src_base + 2 * sizeof(src[0]), x16);
    ASSERT_EQUAL_64(src_base + 2 * sizeof(src[0]), x17);
  }
}


#ifdef VIXL_NEGATIVE_TESTING
TEST(load_pauth_negative_test) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  uint64_t src[4] = {1, 2, 3, 4};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);

  START();
  __ Mov(x16, src_base);

  // There is a small but not negligible chance (1 in 127 runs) that the PAC
  // codes for keys A and B will collide and LDRAB won't abort. To mitigate
  // this, we simply repeat the test a few more times.
  for (unsigned i = 0; i < 32; i++) {
    __ Add(x17, x16, i);
    __ Pacdza(x17);
    __ Ldrab(x0, MemOperand(x17));
  }
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    MUST_FAIL_WITH_MESSAGE(RUN(), "Failed to authenticate pointer.");
#else
    printf("SKIPPED: negative PAuth tests are unimplemented on hardware.");
#endif
  }
}
#endif  // VIXL_NEGATIVE_TESTING


TEST(ldp_stp_offset) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988};
  uint64_t dst[7] = {0, 0, 0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Mov(x18, src_base + 24);
  __ Mov(x19, dst_base + 56);
  __ Ldp(w0, w1, MemOperand(x16));
  __ Ldp(w2, w3, MemOperand(x16, 4));
  __ Ldp(x4, x5, MemOperand(x16, 8));
  __ Ldp(w6, w7, MemOperand(x18, -12));
  __ Ldp(x8, x9, MemOperand(x18, -16));
  __ Stp(w0, w1, MemOperand(x17));
  __ Stp(w2, w3, MemOperand(x17, 8));
  __ Stp(x4, x5, MemOperand(x17, 16));
  __ Stp(w6, w7, MemOperand(x19, -24));
  __ Stp(x8, x9, MemOperand(x19, -16));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x44556677, x0);
    ASSERT_EQUAL_64(0x00112233, x1);
    ASSERT_EQUAL_64(0x0011223344556677, dst[0]);
    ASSERT_EQUAL_64(0x00112233, x2);
    ASSERT_EQUAL_64(0xccddeeff, x3);
    ASSERT_EQUAL_64(0xccddeeff00112233, dst[1]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x4);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[2]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x5);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[3]);
    ASSERT_EQUAL_64(0x8899aabb, x6);
    ASSERT_EQUAL_64(0xbbaa9988, x7);
    ASSERT_EQUAL_64(0xbbaa99888899aabb, dst[4]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x8);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[5]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x9);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[6]);
    ASSERT_EQUAL_64(src_base, x16);
    ASSERT_EQUAL_64(dst_base, x17);
    ASSERT_EQUAL_64(src_base + 24, x18);
    ASSERT_EQUAL_64(dst_base + 56, x19);
  }
}


TEST(ldp_stp_offset_wide) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988};
  uint64_t dst[7] = {0, 0, 0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);
  // Move base too far from the array to force multiple instructions
  // to be emitted.
  const int64_t base_offset = 1024;

  START();
  __ Mov(x20, src_base - base_offset);
  __ Mov(x21, dst_base - base_offset);
  __ Mov(x18, src_base + base_offset + 24);
  __ Mov(x19, dst_base + base_offset + 56);
  __ Ldp(w0, w1, MemOperand(x20, base_offset));
  __ Ldp(w2, w3, MemOperand(x20, base_offset + 4));
  __ Ldp(x4, x5, MemOperand(x20, base_offset + 8));
  __ Ldp(w6, w7, MemOperand(x18, -12 - base_offset));
  __ Ldp(x8, x9, MemOperand(x18, -16 - base_offset));
  __ Stp(w0, w1, MemOperand(x21, base_offset));
  __ Stp(w2, w3, MemOperand(x21, base_offset + 8));
  __ Stp(x4, x5, MemOperand(x21, base_offset + 16));
  __ Stp(w6, w7, MemOperand(x19, -24 - base_offset));
  __ Stp(x8, x9, MemOperand(x19, -16 - base_offset));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x44556677, x0);
    ASSERT_EQUAL_64(0x00112233, x1);
    ASSERT_EQUAL_64(0x0011223344556677, dst[0]);
    ASSERT_EQUAL_64(0x00112233, x2);
    ASSERT_EQUAL_64(0xccddeeff, x3);
    ASSERT_EQUAL_64(0xccddeeff00112233, dst[1]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x4);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[2]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x5);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[3]);
    ASSERT_EQUAL_64(0x8899aabb, x6);
    ASSERT_EQUAL_64(0xbbaa9988, x7);
    ASSERT_EQUAL_64(0xbbaa99888899aabb, dst[4]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x8);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[5]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x9);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[6]);
    ASSERT_EQUAL_64(src_base - base_offset, x20);
    ASSERT_EQUAL_64(dst_base - base_offset, x21);
    ASSERT_EQUAL_64(src_base + base_offset + 24, x18);
    ASSERT_EQUAL_64(dst_base + base_offset + 56, x19);
  }
}


TEST(ldnp_stnp_offset) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  uint64_t src[4] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988,
                     0x7766554433221100};
  uint64_t dst[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Mov(x18, src_base + 24);
  __ Mov(x19, dst_base + 64);
  __ Mov(x20, src_base + 32);

  // Ensure address set up has happened before executing non-temporal ops.
  __ Dmb(InnerShareable, BarrierAll);

  __ Ldnp(w0, w1, MemOperand(x16));
  __ Ldnp(w2, w3, MemOperand(x16, 4));
  __ Ldnp(x4, x5, MemOperand(x16, 8));
  __ Ldnp(w6, w7, MemOperand(x18, -12));
  __ Ldnp(x8, x9, MemOperand(x18, -16));
  __ Ldnp(q16, q17, MemOperand(x16));
  __ Ldnp(q19, q18, MemOperand(x20, -32));
  __ Stnp(w0, w1, MemOperand(x17));
  __ Stnp(w2, w3, MemOperand(x17, 8));
  __ Stnp(x4, x5, MemOperand(x17, 16));
  __ Stnp(w6, w7, MemOperand(x19, -32));
  __ Stnp(x8, x9, MemOperand(x19, -24));
  __ Stnp(q17, q16, MemOperand(x19));
  __ Stnp(q18, q19, MemOperand(x19, 32));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x44556677, x0);
    ASSERT_EQUAL_64(0x00112233, x1);
    ASSERT_EQUAL_64(0x0011223344556677, dst[0]);
    ASSERT_EQUAL_64(0x00112233, x2);
    ASSERT_EQUAL_64(0xccddeeff, x3);
    ASSERT_EQUAL_64(0xccddeeff00112233, dst[1]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x4);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[2]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x5);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[3]);
    ASSERT_EQUAL_64(0x8899aabb, x6);
    ASSERT_EQUAL_64(0xbbaa9988, x7);
    ASSERT_EQUAL_64(0xbbaa99888899aabb, dst[4]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x8);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[5]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x9);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[6]);
    ASSERT_EQUAL_128(0x8899aabbccddeeff, 0x0011223344556677, q16);
    ASSERT_EQUAL_128(0x7766554433221100, 0xffeeddccbbaa9988, q17);
    ASSERT_EQUAL_128(0x7766554433221100, 0xffeeddccbbaa9988, q18);
    ASSERT_EQUAL_128(0x8899aabbccddeeff, 0x0011223344556677, q19);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[8]);
    ASSERT_EQUAL_64(0x7766554433221100, dst[9]);
    ASSERT_EQUAL_64(0x0011223344556677, dst[10]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[11]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[12]);
    ASSERT_EQUAL_64(0x7766554433221100, dst[13]);
    ASSERT_EQUAL_64(0x0011223344556677, dst[14]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[15]);
    ASSERT_EQUAL_64(src_base, x16);
    ASSERT_EQUAL_64(dst_base, x17);
    ASSERT_EQUAL_64(src_base + 24, x18);
    ASSERT_EQUAL_64(dst_base + 64, x19);
    ASSERT_EQUAL_64(src_base + 32, x20);
  }
}

TEST(ldp_stp_preindex) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988};
  uint64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Mov(x18, dst_base + 16);
  __ Ldp(w0, w1, MemOperand(x16, 4, PreIndex));
  __ Mov(x19, x16);
  __ Ldp(w2, w3, MemOperand(x16, -4, PreIndex));
  __ Stp(w2, w3, MemOperand(x17, 4, PreIndex));
  __ Mov(x20, x17);
  __ Stp(w0, w1, MemOperand(x17, -4, PreIndex));
  __ Ldp(x4, x5, MemOperand(x16, 8, PreIndex));
  __ Mov(x21, x16);
  __ Ldp(x6, x7, MemOperand(x16, -8, PreIndex));
  __ Stp(x7, x6, MemOperand(x18, 8, PreIndex));
  __ Mov(x22, x18);
  __ Stp(x5, x4, MemOperand(x18, -8, PreIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00112233, x0);
    ASSERT_EQUAL_64(0xccddeeff, x1);
    ASSERT_EQUAL_64(0x44556677, x2);
    ASSERT_EQUAL_64(0x00112233, x3);
    ASSERT_EQUAL_64(0xccddeeff00112233, dst[0]);
    ASSERT_EQUAL_64(0x0000000000112233, dst[1]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x4);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x5);
    ASSERT_EQUAL_64(0x0011223344556677, x6);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x7);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[2]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[3]);
    ASSERT_EQUAL_64(0x0011223344556677, dst[4]);
    ASSERT_EQUAL_64(src_base, x16);
    ASSERT_EQUAL_64(dst_base, x17);
    ASSERT_EQUAL_64(dst_base + 16, x18);
    ASSERT_EQUAL_64(src_base + 4, x19);
    ASSERT_EQUAL_64(dst_base + 4, x20);
    ASSERT_EQUAL_64(src_base + 8, x21);
    ASSERT_EQUAL_64(dst_base + 24, x22);
  }
}


TEST(ldp_stp_preindex_wide) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988};
  uint64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);
  // Move base too far from the array to force multiple instructions
  // to be emitted.
  const int64_t base_offset = 1024;

  START();
  __ Mov(x24, src_base - base_offset);
  __ Mov(x25, dst_base + base_offset);
  __ Mov(x18, dst_base + base_offset + 16);
  __ Ldp(w0, w1, MemOperand(x24, base_offset + 4, PreIndex));
  __ Mov(x19, x24);
  __ Mov(x24, src_base - base_offset + 4);
  __ Ldp(w2, w3, MemOperand(x24, base_offset - 4, PreIndex));
  __ Stp(w2, w3, MemOperand(x25, 4 - base_offset, PreIndex));
  __ Mov(x20, x25);
  __ Mov(x25, dst_base + base_offset + 4);
  __ Mov(x24, src_base - base_offset);
  __ Stp(w0, w1, MemOperand(x25, -4 - base_offset, PreIndex));
  __ Ldp(x4, x5, MemOperand(x24, base_offset + 8, PreIndex));
  __ Mov(x21, x24);
  __ Mov(x24, src_base - base_offset + 8);
  __ Ldp(x6, x7, MemOperand(x24, base_offset - 8, PreIndex));
  __ Stp(x7, x6, MemOperand(x18, 8 - base_offset, PreIndex));
  __ Mov(x22, x18);
  __ Mov(x18, dst_base + base_offset + 16 + 8);
  __ Stp(x5, x4, MemOperand(x18, -8 - base_offset, PreIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00112233, x0);
    ASSERT_EQUAL_64(0xccddeeff, x1);
    ASSERT_EQUAL_64(0x44556677, x2);
    ASSERT_EQUAL_64(0x00112233, x3);
    ASSERT_EQUAL_64(0xccddeeff00112233, dst[0]);
    ASSERT_EQUAL_64(0x0000000000112233, dst[1]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x4);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x5);
    ASSERT_EQUAL_64(0x0011223344556677, x6);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x7);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[2]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[3]);
    ASSERT_EQUAL_64(0x0011223344556677, dst[4]);
    ASSERT_EQUAL_64(src_base, x24);
    ASSERT_EQUAL_64(dst_base, x25);
    ASSERT_EQUAL_64(dst_base + 16, x18);
    ASSERT_EQUAL_64(src_base + 4, x19);
    ASSERT_EQUAL_64(dst_base + 4, x20);
    ASSERT_EQUAL_64(src_base + 8, x21);
    ASSERT_EQUAL_64(dst_base + 24, x22);
  }
}


TEST(ldp_stp_postindex) {
  SETUP();

  uint64_t src[4] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988,
                     0x7766554433221100};
  uint64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Mov(x18, dst_base + 16);
  __ Ldp(w0, w1, MemOperand(x16, 4, PostIndex));
  __ Mov(x19, x16);
  __ Ldp(w2, w3, MemOperand(x16, -4, PostIndex));
  __ Stp(w2, w3, MemOperand(x17, 4, PostIndex));
  __ Mov(x20, x17);
  __ Stp(w0, w1, MemOperand(x17, -4, PostIndex));
  __ Ldp(x4, x5, MemOperand(x16, 8, PostIndex));
  __ Mov(x21, x16);
  __ Ldp(x6, x7, MemOperand(x16, -8, PostIndex));
  __ Stp(x7, x6, MemOperand(x18, 8, PostIndex));
  __ Mov(x22, x18);
  __ Stp(x5, x4, MemOperand(x18, -8, PostIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x44556677, x0);
    ASSERT_EQUAL_64(0x00112233, x1);
    ASSERT_EQUAL_64(0x00112233, x2);
    ASSERT_EQUAL_64(0xccddeeff, x3);
    ASSERT_EQUAL_64(0x4455667700112233, dst[0]);
    ASSERT_EQUAL_64(0x0000000000112233, dst[1]);
    ASSERT_EQUAL_64(0x0011223344556677, x4);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x5);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x6);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x7);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[2]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[3]);
    ASSERT_EQUAL_64(0x0011223344556677, dst[4]);
    ASSERT_EQUAL_64(src_base, x16);
    ASSERT_EQUAL_64(dst_base, x17);
    ASSERT_EQUAL_64(dst_base + 16, x18);
    ASSERT_EQUAL_64(src_base + 4, x19);
    ASSERT_EQUAL_64(dst_base + 4, x20);
    ASSERT_EQUAL_64(src_base + 8, x21);
    ASSERT_EQUAL_64(dst_base + 24, x22);
  }
}


TEST(ldp_stp_postindex_wide) {
  SETUP();

  uint64_t src[4] = {0x0011223344556677,
                     0x8899aabbccddeeff,
                     0xffeeddccbbaa9988,
                     0x7766554433221100};
  uint64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);
  // Move base too far from the array to force multiple instructions
  // to be emitted.
  const int64_t base_offset = 1024;

  START();
  __ Mov(x24, src_base);
  __ Mov(x25, dst_base);
  __ Mov(x18, dst_base + 16);
  __ Ldp(w0, w1, MemOperand(x24, base_offset + 4, PostIndex));
  __ Mov(x19, x24);
  __ Sub(x24, x24, base_offset);
  __ Ldp(w2, w3, MemOperand(x24, base_offset - 4, PostIndex));
  __ Stp(w2, w3, MemOperand(x25, 4 - base_offset, PostIndex));
  __ Mov(x20, x25);
  __ Sub(x24, x24, base_offset);
  __ Add(x25, x25, base_offset);
  __ Stp(w0, w1, MemOperand(x25, -4 - base_offset, PostIndex));
  __ Ldp(x4, x5, MemOperand(x24, base_offset + 8, PostIndex));
  __ Mov(x21, x24);
  __ Sub(x24, x24, base_offset);
  __ Ldp(x6, x7, MemOperand(x24, base_offset - 8, PostIndex));
  __ Stp(x7, x6, MemOperand(x18, 8 - base_offset, PostIndex));
  __ Mov(x22, x18);
  __ Add(x18, x18, base_offset);
  __ Stp(x5, x4, MemOperand(x18, -8 - base_offset, PostIndex));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x44556677, x0);
    ASSERT_EQUAL_64(0x00112233, x1);
    ASSERT_EQUAL_64(0x00112233, x2);
    ASSERT_EQUAL_64(0xccddeeff, x3);
    ASSERT_EQUAL_64(0x4455667700112233, dst[0]);
    ASSERT_EQUAL_64(0x0000000000112233, dst[1]);
    ASSERT_EQUAL_64(0x0011223344556677, x4);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x5);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, x6);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x7);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, dst[2]);
    ASSERT_EQUAL_64(0x8899aabbccddeeff, dst[3]);
    ASSERT_EQUAL_64(0x0011223344556677, dst[4]);
    ASSERT_EQUAL_64(src_base + base_offset, x24);
    ASSERT_EQUAL_64(dst_base - base_offset, x25);
    ASSERT_EQUAL_64(dst_base - base_offset + 16, x18);
    ASSERT_EQUAL_64(src_base + base_offset + 4, x19);
    ASSERT_EQUAL_64(dst_base - base_offset + 4, x20);
    ASSERT_EQUAL_64(src_base + base_offset + 8, x21);
    ASSERT_EQUAL_64(dst_base - base_offset + 24, x22);
  }
}


TEST(ldp_sign_extend) {
  SETUP();

  uint32_t src[2] = {0x80000000, 0x7fffffff};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);

  START();
  __ Mov(x24, src_base);
  __ Ldpsw(x0, x1, MemOperand(x24));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffff80000000, x0);
    ASSERT_EQUAL_64(0x000000007fffffff, x1);
  }
}


TEST(ldur_stur) {
  SETUP();

  int64_t src[2] = {0x0123456789abcdef, 0x0123456789abcdef};
  int64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base);
  __ Mov(x18, dst_base);
  __ Mov(x19, src_base + 16);
  __ Mov(x20, dst_base + 32);
  __ Mov(x21, dst_base + 40);
  __ Ldr(w0, MemOperand(x17, 1));
  __ Str(w0, MemOperand(x18, 2));
  __ Ldr(x1, MemOperand(x17, 3));
  __ Str(x1, MemOperand(x18, 9));
  __ Ldr(w2, MemOperand(x19, -9));
  __ Str(w2, MemOperand(x20, -5));
  __ Ldrb(w3, MemOperand(x19, -1));
  __ Strb(w3, MemOperand(x21, -1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x6789abcd, x0);
    ASSERT_EQUAL_64(0x00006789abcd0000, dst[0]);
    ASSERT_EQUAL_64(0xabcdef0123456789, x1);
    ASSERT_EQUAL_64(0xcdef012345678900, dst[1]);
    ASSERT_EQUAL_64(0x000000ab, dst[2]);
    ASSERT_EQUAL_64(0xabcdef01, x2);
    ASSERT_EQUAL_64(0x00abcdef01000000, dst[3]);
    ASSERT_EQUAL_64(0x00000001, x3);
    ASSERT_EQUAL_64(0x0100000000000000, dst[4]);
    ASSERT_EQUAL_64(src_base, x17);
    ASSERT_EQUAL_64(dst_base, x18);
    ASSERT_EQUAL_64(src_base + 16, x19);
    ASSERT_EQUAL_64(dst_base + 32, x20);
  }
}


TEST(ldur_stur_neon) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  int64_t src[3] = {0x0123456789abcdef, 0x0123456789abcdef, 0x0123456789abcdef};
  int64_t dst[5] = {0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base);
  __ Mov(x18, dst_base);
  __ Ldr(b0, MemOperand(x17));
  __ Str(b0, MemOperand(x18));
  __ Ldr(h1, MemOperand(x17, 1));
  __ Str(h1, MemOperand(x18, 1));
  __ Ldr(s2, MemOperand(x17, 2));
  __ Str(s2, MemOperand(x18, 3));
  __ Ldr(d3, MemOperand(x17, 3));
  __ Str(d3, MemOperand(x18, 7));
  __ Ldr(q4, MemOperand(x17, 4));
  __ Str(q4, MemOperand(x18, 15));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_128(0, 0xef, q0);
    ASSERT_EQUAL_128(0, 0xabcd, q1);
    ASSERT_EQUAL_128(0, 0x456789ab, q2);
    ASSERT_EQUAL_128(0, 0xabcdef0123456789, q3);
    ASSERT_EQUAL_128(0x89abcdef01234567, 0x89abcdef01234567, q4);
    ASSERT_EQUAL_64(0x89456789ababcdef, dst[0]);
    ASSERT_EQUAL_64(0x67abcdef01234567, dst[1]);
    ASSERT_EQUAL_64(0x6789abcdef012345, dst[2]);
    ASSERT_EQUAL_64(0x0089abcdef012345, dst[3]);
  }
}


TEST(ldr_literal) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  START();
  __ Ldr(x2, 0x1234567890abcdef);
  __ Ldr(w3, 0xfedcba09);
  __ Ldrsw(x4, 0x7fffffff);
  __ Ldrsw(x5, 0x80000000);
  __ Ldr(q11, 0x1234000056780000, 0xabcd0000ef000000);
  __ Ldr(d13, 1.234);
  __ Ldr(s25, 2.5);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1234567890abcdef, x2);
    ASSERT_EQUAL_64(0xfedcba09, x3);
    ASSERT_EQUAL_64(0x7fffffff, x4);
    ASSERT_EQUAL_64(0xffffffff80000000, x5);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q11);
    ASSERT_EQUAL_FP64(1.234, d13);
    ASSERT_EQUAL_FP32(2.5, s25);
  }
}


TEST(ldr_literal_range) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  START();
  // Make sure the pool is empty;
  masm.EmitLiteralPool(LiteralPool::kBranchRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  // Create some literal pool entries.
  __ Ldr(x0, 0x1234567890abcdef);
  __ Ldr(w1, 0xfedcba09);
  __ Ldrsw(x2, 0x7fffffff);
  __ Ldrsw(x3, 0x80000000);
  __ Ldr(q2, 0x1234000056780000, 0xabcd0000ef000000);
  __ Ldr(d0, 1.234);
  __ Ldr(s1, 2.5);
  ASSERT_LITERAL_POOL_SIZE(48);

  // Emit more code than the maximum literal load range to ensure the pool
  // should be emitted.
  const ptrdiff_t end = masm.GetCursorOffset() + 2 * kMaxLoadLiteralRange;
  while (masm.GetCursorOffset() < end) {
    __ Nop();
  }

  // The pool should have been emitted.
  ASSERT_LITERAL_POOL_SIZE(0);

  // These loads should be after the pool (and will require a new one).
  __ Ldr(x4, 0x34567890abcdef12);
  __ Ldr(w5, 0xdcba09fe);
  __ Ldrsw(x6, 0x7fffffff);
  __ Ldrsw(x7, 0x80000000);
  __ Ldr(q6, 0x1234000056780000, 0xabcd0000ef000000);
  __ Ldr(d4, 123.4);
  __ Ldr(s5, 250.0);
  ASSERT_LITERAL_POOL_SIZE(48);
  END();

  if (CAN_RUN()) {
    RUN();

    // Check that the literals loaded correctly.
    ASSERT_EQUAL_64(0x1234567890abcdef, x0);
    ASSERT_EQUAL_64(0xfedcba09, x1);
    ASSERT_EQUAL_64(0x7fffffff, x2);
    ASSERT_EQUAL_64(0xffffffff80000000, x3);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q2);
    ASSERT_EQUAL_FP64(1.234, d0);
    ASSERT_EQUAL_FP32(2.5, s1);
    ASSERT_EQUAL_64(0x34567890abcdef12, x4);
    ASSERT_EQUAL_64(0xdcba09fe, x5);
    ASSERT_EQUAL_64(0x7fffffff, x6);
    ASSERT_EQUAL_64(0xffffffff80000000, x7);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q6);
    ASSERT_EQUAL_FP64(123.4, d4);
    ASSERT_EQUAL_FP32(250.0, s5);
  }
}


template <typename T>
void LoadIntValueHelper(T values[], int card) {
  SETUP();

  const bool is_32bit = (sizeof(T) == 4);
  Register tgt1 = is_32bit ? Register(w1) : Register(x1);
  Register tgt2 = is_32bit ? Register(w2) : Register(x2);

  START();
  __ Mov(x0, 0);

  // If one of the values differ then x0 will be one.
  for (int i = 0; i < card; ++i) {
    __ Mov(tgt1, values[i]);
    __ Ldr(tgt2, values[i]);
    __ Cmp(tgt1, tgt2);
    __ Cset(x0, ne);
  }
  END();

  if (CAN_RUN()) {
    RUN();

    // If one of the values differs, the trace can be used to identify which
    // one.
    ASSERT_EQUAL_64(0, x0);
  }
}


TEST(ldr_literal_values_x) {
  static const uint64_t kValues[] = {0x8000000000000000,
                                     0x7fffffffffffffff,
                                     0x0000000000000000,
                                     0xffffffffffffffff,
                                     0x00ff00ff00ff00ff,
                                     0x1234567890abcdef};

  LoadIntValueHelper(kValues, sizeof(kValues) / sizeof(kValues[0]));
}


TEST(ldr_literal_values_w) {
  static const uint32_t kValues[] = {0x80000000,
                                     0x7fffffff,
                                     0x00000000,
                                     0xffffffff,
                                     0x00ff00ff,
                                     0x12345678,
                                     0x90abcdef};

  LoadIntValueHelper(kValues, sizeof(kValues) / sizeof(kValues[0]));
}

TEST(ldr_literal_custom) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  Label end_of_pool_before;
  Label end_of_pool_after;

  const size_t kSizeOfPoolInBytes = 44;

  Literal<uint64_t> before_x(0x1234567890abcdef);
  Literal<uint32_t> before_w(0xfedcba09);
  Literal<uint32_t> before_sx(0x80000000);
  Literal<uint64_t> before_q(0x1234000056780000, 0xabcd0000ef000000);
  Literal<double> before_d(1.234);
  Literal<float> before_s(2.5);

  Literal<uint64_t> after_x(0x1234567890abcdef);
  Literal<uint32_t> after_w(0xfedcba09);
  Literal<uint32_t> after_sx(0x80000000);
  Literal<uint64_t> after_q(0x1234000056780000, 0xabcd0000ef000000);
  Literal<double> after_d(1.234);
  Literal<float> after_s(2.5);

  START();

  // Manually generate a pool.
  __ B(&end_of_pool_before);
  {
    ExactAssemblyScope scope(&masm, kSizeOfPoolInBytes);
    __ place(&before_x);
    __ place(&before_w);
    __ place(&before_sx);
    __ place(&before_q);
    __ place(&before_d);
    __ place(&before_s);
  }
  __ Bind(&end_of_pool_before);

  {
    ExactAssemblyScope scope(&masm, 12 * kInstructionSize);
    __ ldr(x2, &before_x);
    __ ldr(w3, &before_w);
    __ ldrsw(x5, &before_sx);
    __ ldr(q11, &before_q);
    __ ldr(d13, &before_d);
    __ ldr(s25, &before_s);

    __ ldr(x6, &after_x);
    __ ldr(w7, &after_w);
    __ ldrsw(x8, &after_sx);
    __ ldr(q18, &after_q);
    __ ldr(d14, &after_d);
    __ ldr(s26, &after_s);
  }

  // Manually generate a pool.
  __ B(&end_of_pool_after);
  {
    ExactAssemblyScope scope(&masm, kSizeOfPoolInBytes);
    __ place(&after_x);
    __ place(&after_w);
    __ place(&after_sx);
    __ place(&after_q);
    __ place(&after_d);
    __ place(&after_s);
  }
  __ Bind(&end_of_pool_after);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1234567890abcdef, x2);
    ASSERT_EQUAL_64(0xfedcba09, x3);
    ASSERT_EQUAL_64(0xffffffff80000000, x5);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q11);
    ASSERT_EQUAL_FP64(1.234, d13);
    ASSERT_EQUAL_FP32(2.5, s25);

    ASSERT_EQUAL_64(0x1234567890abcdef, x6);
    ASSERT_EQUAL_64(0xfedcba09, x7);
    ASSERT_EQUAL_64(0xffffffff80000000, x8);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q18);
    ASSERT_EQUAL_FP64(1.234, d14);
    ASSERT_EQUAL_FP32(2.5, s26);
  }
}


TEST(ldr_literal_custom_shared) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  Label end_of_pool_before;
  Label end_of_pool_after;

  const size_t kSizeOfPoolInBytes = 40;

  Literal<uint64_t> before_x(0x1234567890abcdef);
  Literal<uint32_t> before_w(0xfedcba09);
  Literal<uint64_t> before_q(0x1234000056780000, 0xabcd0000ef000000);
  Literal<double> before_d(1.234);
  Literal<float> before_s(2.5);

  Literal<uint64_t> after_x(0x1234567890abcdef);
  Literal<uint32_t> after_w(0xfedcba09);
  Literal<uint64_t> after_q(0x1234000056780000, 0xabcd0000ef000000);
  Literal<double> after_d(1.234);
  Literal<float> after_s(2.5);

  START();

  // Manually generate a pool.
  __ B(&end_of_pool_before);
  {
    ExactAssemblyScope scope(&masm, kSizeOfPoolInBytes);
    __ place(&before_x);
    __ place(&before_w);
    __ place(&before_q);
    __ place(&before_d);
    __ place(&before_s);
  }
  __ Bind(&end_of_pool_before);

  // Load the entries several times to test that literals can be shared.
  for (int i = 0; i < 50; i++) {
    ExactAssemblyScope scope(&masm, 12 * kInstructionSize);
    __ ldr(x2, &before_x);
    __ ldr(w3, &before_w);
    __ ldrsw(x5, &before_w);  // Re-use before_w.
    __ ldr(q11, &before_q);
    __ ldr(d13, &before_d);
    __ ldr(s25, &before_s);

    __ ldr(x6, &after_x);
    __ ldr(w7, &after_w);
    __ ldrsw(x8, &after_w);  // Re-use after_w.
    __ ldr(q18, &after_q);
    __ ldr(d14, &after_d);
    __ ldr(s26, &after_s);
  }

  // Manually generate a pool.
  __ B(&end_of_pool_after);
  {
    ExactAssemblyScope scope(&masm, kSizeOfPoolInBytes);
    __ place(&after_x);
    __ place(&after_w);
    __ place(&after_q);
    __ place(&after_d);
    __ place(&after_s);
  }
  __ Bind(&end_of_pool_after);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1234567890abcdef, x2);
    ASSERT_EQUAL_64(0xfedcba09, x3);
    ASSERT_EQUAL_64(0xfffffffffedcba09, x5);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q11);
    ASSERT_EQUAL_FP64(1.234, d13);
    ASSERT_EQUAL_FP32(2.5, s25);

    ASSERT_EQUAL_64(0x1234567890abcdef, x6);
    ASSERT_EQUAL_64(0xfedcba09, x7);
    ASSERT_EQUAL_64(0xfffffffffedcba09, x8);
    ASSERT_EQUAL_128(0x1234000056780000, 0xabcd0000ef000000, q18);
    ASSERT_EQUAL_FP64(1.234, d14);
    ASSERT_EQUAL_FP32(2.5, s26);
  }
}

static const PrefetchOperation kPrfmOperations[] = {PLDL1KEEP,
                                                    PLDL1STRM,
                                                    PLDL2KEEP,
                                                    PLDL2STRM,
                                                    PLDL3KEEP,
                                                    PLDL3STRM,

                                                    PLIL1KEEP,
                                                    PLIL1STRM,
                                                    PLIL2KEEP,
                                                    PLIL2STRM,
                                                    PLIL3KEEP,
                                                    PLIL3STRM,

                                                    PSTL1KEEP,
                                                    PSTL1STRM,
                                                    PSTL2KEEP,
                                                    PSTL2STRM,
                                                    PSTL3KEEP,
                                                    PSTL3STRM};

TEST(prfm_offset) {
  SETUP();

  START();
  // The address used in prfm doesn't have to be valid.
  __ Mov(x0, 0x0123456789abcdef);

  for (int op = 0; op < (1 << ImmPrefetchOperation_width); op++) {
    // Unallocated prefetch operations are ignored, so test all of them.
    // We have to use the Assembler directly for this.
    ExactAssemblyScope guard(&masm, 3 * kInstructionSize);
    __ prfm(op, MemOperand(x0));
    __ prfm(op, MemOperand(x0, 8));
    __ prfm(op, MemOperand(x0, 32760));
  }

  for (PrefetchOperation op : kPrfmOperations) {
    // Also test named operations.
    __ Prfm(op, MemOperand(x0, 32768));
    __ Prfm(op, MemOperand(x0, 1));
    __ Prfm(op, MemOperand(x0, 9));
    __ Prfm(op, MemOperand(x0, 255));
    __ Prfm(op, MemOperand(x0, 257));
    __ Prfm(op, MemOperand(x0, -1));
    __ Prfm(op, MemOperand(x0, -9));
    __ Prfm(op, MemOperand(x0, -255));
    __ Prfm(op, MemOperand(x0, -257));

    __ Prfm(op, MemOperand(x0, 0xfedcba9876543210));
  }

  END();
  if (CAN_RUN()) {
    RUN();
  }
}


TEST(prfm_regoffset) {
  SETUP();

  START();
  // The address used in prfm doesn't have to be valid.
  __ Mov(x0, 0x0123456789abcdef);

  CPURegList inputs(CPURegister::kRegister, kXRegSize, 10, 18);
  __ Mov(x10, 0);
  __ Mov(x11, 1);
  __ Mov(x12, 8);
  __ Mov(x13, 255);
  __ Mov(x14, -0);
  __ Mov(x15, -1);
  __ Mov(x16, -8);
  __ Mov(x17, -255);
  __ Mov(x18, 0xfedcba9876543210);

  for (int op = 0; op < (1 << ImmPrefetchOperation_width); op++) {
    // Unallocated prefetch operations are ignored, so test all of them.
    // We have to use the Assembler directly for this.

    // Prefetch operations of the form 0b11xxx are allocated to another
    // instruction.
    if (op >= 0b11000) continue;

    ExactAssemblyScope guard(&masm, inputs.GetCount() * kInstructionSize);
    CPURegList loop = inputs;
    while (!loop.IsEmpty()) {
      __ prfm(op, MemOperand(x0, Register(loop.PopLowestIndex())));
    }
  }

  for (PrefetchOperation op : kPrfmOperations) {
    // Also test named operations.
    CPURegList loop = inputs;
    while (!loop.IsEmpty()) {
      Register input(loop.PopLowestIndex());
      __ Prfm(op, MemOperand(x0, input, UXTW));
      __ Prfm(op, MemOperand(x0, input, UXTW, 3));
      __ Prfm(op, MemOperand(x0, input, LSL));
      __ Prfm(op, MemOperand(x0, input, LSL, 3));
      __ Prfm(op, MemOperand(x0, input, SXTW));
      __ Prfm(op, MemOperand(x0, input, SXTW, 3));
      __ Prfm(op, MemOperand(x0, input, SXTX));
      __ Prfm(op, MemOperand(x0, input, SXTX, 3));
    }
  }

  END();
  if (CAN_RUN()) {
    RUN();
  }
}


TEST(prfm_literal_imm19) {
  SETUP();
  START();

  for (int op = 0; op < (1 << ImmPrefetchOperation_width); op++) {
    // Unallocated prefetch operations are ignored, so test all of them.
    // We have to use the Assembler directly for this.
    ExactAssemblyScope guard(&masm, 3 * kInstructionSize);
    __ prfm(op, INT64_C(0));
    __ prfm(op, 1);
    __ prfm(op, -1);
  }

  for (PrefetchOperation op : kPrfmOperations) {
    // Also test named operations.
    ExactAssemblyScope guard(&masm, 4 * kInstructionSize);
    // The address used in prfm doesn't have to be valid.
    __ prfm(op, 1000);
    __ prfm(op, -1000);
    __ prfm(op, 0x3ffff);
    __ prfm(op, -0x40000);
  }

  END();
  if (CAN_RUN()) {
    RUN();
  }
}


TEST(prfm_literal) {
  SETUP();

  Label end_of_pool_before;
  Label end_of_pool_after;
  Literal<uint64_t> before(0);
  Literal<uint64_t> after(0);

  START();

  // Manually generate a pool.
  __ B(&end_of_pool_before);
  {
    ExactAssemblyScope scope(&masm, before.GetSize());
    __ place(&before);
  }
  __ Bind(&end_of_pool_before);

  for (int op = 0; op < (1 << ImmPrefetchOperation_width); op++) {
    // Unallocated prefetch operations are ignored, so test all of them.
    // We have to use the Assembler directly for this.
    ExactAssemblyScope guard(&masm, 2 * kInstructionSize);
    __ prfm(op, &before);
    __ prfm(op, &after);
  }

  for (PrefetchOperation op : kPrfmOperations) {
    // Also test named operations.
    ExactAssemblyScope guard(&masm, 2 * kInstructionSize);
    __ prfm(op, &before);
    __ prfm(op, &after);
  }

  // Manually generate a pool.
  __ B(&end_of_pool_after);
  {
    ExactAssemblyScope scope(&masm, after.GetSize());
    __ place(&after);
  }
  __ Bind(&end_of_pool_after);

  END();
  if (CAN_RUN()) {
    RUN();
  }
}


TEST(prfm_wide) {
  SETUP();

  START();
  // The address used in prfm doesn't have to be valid.
  __ Mov(x0, 0x0123456789abcdef);

  for (PrefetchOperation op : kPrfmOperations) {
    __ Prfm(op, MemOperand(x0, 0x40000));
    __ Prfm(op, MemOperand(x0, -0x40001));
    __ Prfm(op, MemOperand(x0, UINT64_C(0x5555555555555555)));
    __ Prfm(op, MemOperand(x0, UINT64_C(0xfedcba9876543210)));
  }

  END();
  if (CAN_RUN()) {
    RUN();
  }
}


TEST(load_prfm_literal) {
  // Test literals shared between both prfm and ldr.
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

  Label end_of_pool_before;
  Label end_of_pool_after;

  const size_t kSizeOfPoolInBytes = 28;

  Literal<uint64_t> before_x(0x1234567890abcdef);
  Literal<uint32_t> before_w(0xfedcba09);
  Literal<uint32_t> before_sx(0x80000000);
  Literal<double> before_d(1.234);
  Literal<float> before_s(2.5);
  Literal<uint64_t> after_x(0x1234567890abcdef);
  Literal<uint32_t> after_w(0xfedcba09);
  Literal<uint32_t> after_sx(0x80000000);
  Literal<double> after_d(1.234);
  Literal<float> after_s(2.5);

  START();

  // Manually generate a pool.
  __ B(&end_of_pool_before);
  {
    ExactAssemblyScope scope(&masm, kSizeOfPoolInBytes);
    __ place(&before_x);
    __ place(&before_w);
    __ place(&before_sx);
    __ place(&before_d);
    __ place(&before_s);
  }
  __ Bind(&end_of_pool_before);

  for (int op = 0; op < (1 << ImmPrefetchOperation_width); op++) {
    // Unallocated prefetch operations are ignored, so test all of them.
    ExactAssemblyScope scope(&masm, 10 * kInstructionSize);

    __ prfm(op, &before_x);
    __ prfm(op, &before_w);
    __ prfm(op, &before_sx);
    __ prfm(op, &before_d);
    __ prfm(op, &before_s);

    __ prfm(op, &after_x);
    __ prfm(op, &after_w);
    __ prfm(op, &after_sx);
    __ prfm(op, &after_d);
    __ prfm(op, &after_s);
  }

  for (PrefetchOperation op : kPrfmOperations) {
    // Also test named operations.
    ExactAssemblyScope scope(&masm, 10 * kInstructionSize);

    __ prfm(op, &before_x);
    __ prfm(op, &before_w);
    __ prfm(op, &before_sx);
    __ prfm(op, &before_d);
    __ prfm(op, &before_s);

    __ prfm(op, &after_x);
    __ prfm(op, &after_w);
    __ prfm(op, &after_sx);
    __ prfm(op, &after_d);
    __ prfm(op, &after_s);
  }

  {
    ExactAssemblyScope scope(&masm, 10 * kInstructionSize);
    __ ldr(x2, &before_x);
    __ ldr(w3, &before_w);
    __ ldrsw(x5, &before_sx);
    __ ldr(d13, &before_d);
    __ ldr(s25, &before_s);

    __ ldr(x6, &after_x);
    __ ldr(w7, &after_w);
    __ ldrsw(x8, &after_sx);
    __ ldr(d14, &after_d);
    __ ldr(s26, &after_s);
  }

  // Manually generate a pool.
  __ B(&end_of_pool_after);
  {
    ExactAssemblyScope scope(&masm, kSizeOfPoolInBytes);
    __ place(&after_x);
    __ place(&after_w);
    __ place(&after_sx);
    __ place(&after_d);
    __ place(&after_s);
  }
  __ Bind(&end_of_pool_after);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1234567890abcdef, x2);
    ASSERT_EQUAL_64(0xfedcba09, x3);
    ASSERT_EQUAL_64(0xffffffff80000000, x5);
    ASSERT_EQUAL_FP64(1.234, d13);
    ASSERT_EQUAL_FP32(2.5, s25);

    ASSERT_EQUAL_64(0x1234567890abcdef, x6);
    ASSERT_EQUAL_64(0xfedcba09, x7);
    ASSERT_EQUAL_64(0xffffffff80000000, x8);
    ASSERT_EQUAL_FP64(1.234, d14);
    ASSERT_EQUAL_FP32(2.5, s26);
  }
}


TEST(add_sub_imm) {
  SETUP();

  START();
  __ Mov(x0, 0x0);
  __ Mov(x1, 0x1111);
  __ Mov(x2, 0xffffffffffffffff);
  __ Mov(x3, 0x8000000000000000);

  __ Add(x10, x0, Operand(0x123));
  __ Add(x11, x1, Operand(0x122000));
  __ Add(x12, x0, Operand(0xabc << 12));
  __ Add(x13, x2, Operand(1));

  __ Add(w14, w0, Operand(0x123));
  __ Add(w15, w1, Operand(0x122000));
  __ Add(w16, w0, Operand(0xabc << 12));
  __ Add(w17, w2, Operand(1));

  __ Sub(x20, x0, Operand(0x1));
  __ Sub(x21, x1, Operand(0x111));
  __ Sub(x22, x1, Operand(0x1 << 12));
  __ Sub(x23, x3, Operand(1));

  __ Sub(w24, w0, Operand(0x1));
  __ Sub(w25, w1, Operand(0x111));
  __ Sub(w26, w1, Operand(0x1 << 12));
  __ Sub(w27, w3, Operand(1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x123, x10);
    ASSERT_EQUAL_64(0x123111, x11);
    ASSERT_EQUAL_64(0xabc000, x12);
    ASSERT_EQUAL_64(0x0, x13);

    ASSERT_EQUAL_32(0x123, w14);
    ASSERT_EQUAL_32(0x123111, w15);
    ASSERT_EQUAL_32(0xabc000, w16);
    ASSERT_EQUAL_32(0x0, w17);

    ASSERT_EQUAL_64(0xffffffffffffffff, x20);
    ASSERT_EQUAL_64(0x1000, x21);
    ASSERT_EQUAL_64(0x111, x22);
    ASSERT_EQUAL_64(0x7fffffffffffffff, x23);

    ASSERT_EQUAL_32(0xffffffff, w24);
    ASSERT_EQUAL_32(0x1000, w25);
    ASSERT_EQUAL_32(0x111, w26);
    ASSERT_EQUAL_32(0xffffffff, w27);
  }
}


TEST(add_sub_wide_imm) {
  SETUP();

  START();
  __ Mov(x0, 0x0);
  __ Mov(x1, 0x1);

  __ Add(x10, x0, Operand(0x1234567890abcdef));
  __ Add(x11, x1, Operand(0xffffffff));

  __ Add(w12, w0, Operand(0x12345678));
  __ Add(w13, w1, Operand(0xffffffff));

  __ Add(w18, w0, Operand(kWMinInt));
  __ Sub(w19, w0, Operand(kWMinInt));

  __ Sub(x20, x0, Operand(0x1234567890abcdef));
  __ Sub(w21, w0, Operand(0x12345678));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1234567890abcdef, x10);
    ASSERT_EQUAL_64(0x100000000, x11);

    ASSERT_EQUAL_32(0x12345678, w12);
    ASSERT_EQUAL_64(0x0, x13);

    ASSERT_EQUAL_32(kWMinInt, w18);
    ASSERT_EQUAL_32(kWMinInt, w19);

    ASSERT_EQUAL_64(-0x1234567890abcdef, x20);
    ASSERT_EQUAL_32(-0x12345678, w21);
  }
}


TEST(add_sub_shifted) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x0123456789abcdef);
  __ Mov(x2, 0xfedcba9876543210);
  __ Mov(x3, 0xffffffffffffffff);

  __ Add(x10, x1, Operand(x2));
  __ Add(x11, x0, Operand(x1, LSL, 8));
  __ Add(x12, x0, Operand(x1, LSR, 8));
  __ Add(x13, x0, Operand(x1, ASR, 8));
  __ Add(x14, x0, Operand(x2, ASR, 8));
  __ Add(w15, w0, Operand(w1, ASR, 8));
  __ Add(w18, w3, Operand(w1, ROR, 8));
  __ Add(x19, x3, Operand(x1, ROR, 8));

  __ Sub(x20, x3, Operand(x2));
  __ Sub(x21, x3, Operand(x1, LSL, 8));
  __ Sub(x22, x3, Operand(x1, LSR, 8));
  __ Sub(x23, x3, Operand(x1, ASR, 8));
  __ Sub(x24, x3, Operand(x2, ASR, 8));
  __ Sub(w25, w3, Operand(w1, ASR, 8));
  __ Sub(w26, w3, Operand(w1, ROR, 8));
  __ Sub(x27, x3, Operand(x1, ROR, 8));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffffffffffff, x10);
    ASSERT_EQUAL_64(0x23456789abcdef00, x11);
    ASSERT_EQUAL_64(0x000123456789abcd, x12);
    ASSERT_EQUAL_64(0x000123456789abcd, x13);
    ASSERT_EQUAL_64(0xfffedcba98765432, x14);
    ASSERT_EQUAL_64(0xff89abcd, x15);
    ASSERT_EQUAL_64(0xef89abcc, x18);
    ASSERT_EQUAL_64(0xef0123456789abcc, x19);

    ASSERT_EQUAL_64(0x0123456789abcdef, x20);
    ASSERT_EQUAL_64(0xdcba9876543210ff, x21);
    ASSERT_EQUAL_64(0xfffedcba98765432, x22);
    ASSERT_EQUAL_64(0xfffedcba98765432, x23);
    ASSERT_EQUAL_64(0x000123456789abcd, x24);
    ASSERT_EQUAL_64(0x00765432, x25);
    ASSERT_EQUAL_64(0x10765432, x26);
    ASSERT_EQUAL_64(0x10fedcba98765432, x27);
  }
}


TEST(add_sub_extended) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x0123456789abcdef);
  __ Mov(x2, 0xfedcba9876543210);
  __ Mov(w3, 0x80);

  __ Add(x10, x0, Operand(x1, UXTB, 0));
  __ Add(x11, x0, Operand(x1, UXTB, 1));
  __ Add(x12, x0, Operand(x1, UXTH, 2));
  __ Add(x13, x0, Operand(x1, UXTW, 4));

  __ Add(x14, x0, Operand(x1, SXTB, 0));
  __ Add(x15, x0, Operand(x1, SXTB, 1));
  __ Add(x16, x0, Operand(x1, SXTH, 2));
  __ Add(x17, x0, Operand(x1, SXTW, 3));
  __ Add(x18, x0, Operand(x2, SXTB, 0));
  __ Add(x19, x0, Operand(x2, SXTB, 1));
  __ Add(x20, x0, Operand(x2, SXTH, 2));
  __ Add(x21, x0, Operand(x2, SXTW, 3));

  __ Add(x22, x1, Operand(x2, SXTB, 1));
  __ Sub(x23, x1, Operand(x2, SXTB, 1));

  __ Add(w24, w1, Operand(w2, UXTB, 2));
  __ Add(w25, w0, Operand(w1, SXTB, 0));
  __ Add(w26, w0, Operand(w1, SXTB, 1));
  __ Add(w27, w2, Operand(w1, SXTW, 3));

  __ Add(w28, w0, Operand(w1, SXTW, 3));
  __ Add(x29, x0, Operand(w1, SXTW, 3));

  __ Sub(x30, x0, Operand(w3, SXTB, 1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xef, x10);
    ASSERT_EQUAL_64(0x1de, x11);
    ASSERT_EQUAL_64(0x337bc, x12);
    ASSERT_EQUAL_64(0x89abcdef0, x13);

    ASSERT_EQUAL_64(0xffffffffffffffef, x14);
    ASSERT_EQUAL_64(0xffffffffffffffde, x15);
    ASSERT_EQUAL_64(0xffffffffffff37bc, x16);
    ASSERT_EQUAL_64(0xfffffffc4d5e6f78, x17);
    ASSERT_EQUAL_64(0x10, x18);
    ASSERT_EQUAL_64(0x20, x19);
    ASSERT_EQUAL_64(0xc840, x20);
    ASSERT_EQUAL_64(0x3b2a19080, x21);

    ASSERT_EQUAL_64(0x0123456789abce0f, x22);
    ASSERT_EQUAL_64(0x0123456789abcdcf, x23);

    ASSERT_EQUAL_32(0x89abce2f, w24);
    ASSERT_EQUAL_32(0xffffffef, w25);
    ASSERT_EQUAL_32(0xffffffde, w26);
    ASSERT_EQUAL_32(0xc3b2a188, w27);

    ASSERT_EQUAL_32(0x4d5e6f78, w28);
    ASSERT_EQUAL_64(0xfffffffc4d5e6f78, x29);

    ASSERT_EQUAL_64(256, x30);
  }
}


TEST(add_sub_negative) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 4687);
  __ Mov(x2, 0x1122334455667788);
  __ Mov(w3, 0x11223344);
  __ Mov(w4, 400000);

  __ Add(x10, x0, -42);
  __ Add(x11, x1, -687);
  __ Add(x12, x2, -0x88);

  __ Sub(x13, x0, -600);
  __ Sub(x14, x1, -313);
  __ Sub(x15, x2, -0x555);

  __ Add(w19, w3, -0x344);
  __ Add(w20, w4, -2000);

  __ Sub(w21, w3, -0xbc);
  __ Sub(w22, w4, -2000);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(-42, x10);
    ASSERT_EQUAL_64(4000, x11);
    ASSERT_EQUAL_64(0x1122334455667700, x12);

    ASSERT_EQUAL_64(600, x13);
    ASSERT_EQUAL_64(5000, x14);
    ASSERT_EQUAL_64(0x1122334455667cdd, x15);

    ASSERT_EQUAL_32(0x11223000, w19);
    ASSERT_EQUAL_32(398000, w20);

    ASSERT_EQUAL_32(0x11223400, w21);
    ASSERT_EQUAL_32(402000, w22);
  }
}


TEST(add_sub_zero) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0);
  __ Mov(x2, 0);

  Label blob1;
  __ Bind(&blob1);
  __ Add(x0, x0, 0);
  __ Sub(x1, x1, 0);
  __ Sub(x2, x2, xzr);
  VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&blob1) == 0);

  Label blob2;
  __ Bind(&blob2);
  __ Add(w3, w3, 0);
  VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&blob2) != 0);

  Label blob3;
  __ Bind(&blob3);
  __ Sub(w3, w3, wzr);
  VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&blob3) != 0);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(0, x2);
  }
}


TEST(claim_drop_zero) {
  SETUP();

  START();

  Label start;
  __ Bind(&start);
  __ Claim(Operand(0));
  __ Drop(Operand(0));
  __ Claim(Operand(xzr));
  __ Drop(Operand(xzr));
  VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&start) == 0);

  END();

  if (CAN_RUN()) {
    RUN();
  }
}


TEST(neg) {
  SETUP();

  START();
  __ Mov(x0, 0xf123456789abcdef);

  // Immediate.
  __ Neg(x1, 0x123);
  __ Neg(w2, 0x123);

  // Shifted.
  __ Neg(x3, Operand(x0, LSL, 1));
  __ Neg(w4, Operand(w0, LSL, 2));
  __ Neg(x5, Operand(x0, LSR, 3));
  __ Neg(w6, Operand(w0, LSR, 4));
  __ Neg(x7, Operand(x0, ASR, 5));
  __ Neg(w8, Operand(w0, ASR, 6));

  // Extended.
  __ Neg(w9, Operand(w0, UXTB));
  __ Neg(x10, Operand(x0, SXTB, 1));
  __ Neg(w11, Operand(w0, UXTH, 2));
  __ Neg(x12, Operand(x0, SXTH, 3));
  __ Neg(w13, Operand(w0, UXTW, 4));
  __ Neg(x14, Operand(x0, SXTW, 4));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xfffffffffffffedd, x1);
    ASSERT_EQUAL_64(0xfffffedd, x2);
    ASSERT_EQUAL_64(0x1db97530eca86422, x3);
    ASSERT_EQUAL_64(0xd950c844, x4);
    ASSERT_EQUAL_64(0xe1db97530eca8643, x5);
    ASSERT_EQUAL_64(0xf7654322, x6);
    ASSERT_EQUAL_64(0x0076e5d4c3b2a191, x7);
    ASSERT_EQUAL_64(0x01d950c9, x8);
    ASSERT_EQUAL_64(0xffffff11, x9);
    ASSERT_EQUAL_64(0x0000000000000022, x10);
    ASSERT_EQUAL_64(0xfffcc844, x11);
    ASSERT_EQUAL_64(0x0000000000019088, x12);
    ASSERT_EQUAL_64(0x65432110, x13);
    ASSERT_EQUAL_64(0x0000000765432110, x14);
  }
}


template <typename T, typename Op>
static void AdcsSbcsHelper(
    Op op, T left, T right, int carry, T expected, StatusFlags expected_flags) {
  int reg_size = sizeof(T) * 8;
  Register left_reg(0, reg_size);
  Register right_reg(1, reg_size);
  Register result_reg(2, reg_size);

  SETUP();
  START();

  __ Mov(left_reg, left);
  __ Mov(right_reg, right);
  __ Mov(x10, (carry ? CFlag : NoFlag));

  __ Msr(NZCV, x10);
  (masm.*op)(result_reg, left_reg, right_reg);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(left, left_reg.X());
    ASSERT_EQUAL_64(right, right_reg.X());
    ASSERT_EQUAL_64(expected, result_reg.X());
    ASSERT_EQUAL_NZCV(expected_flags);
  }
}


TEST(adcs_sbcs_x) {
  uint64_t inputs[] = {
      0x0000000000000000,
      0x0000000000000001,
      0x7ffffffffffffffe,
      0x7fffffffffffffff,
      0x8000000000000000,
      0x8000000000000001,
      0xfffffffffffffffe,
      0xffffffffffffffff,
  };
  static const size_t input_count = sizeof(inputs) / sizeof(inputs[0]);

  struct Expected {
    uint64_t carry0_result;
    StatusFlags carry0_flags;
    uint64_t carry1_result;
    StatusFlags carry1_flags;
  };

  static const Expected expected_adcs_x[input_count][input_count] =
      {{{0x0000000000000000, ZFlag, 0x0000000000000001, NoFlag},
        {0x0000000000000001, NoFlag, 0x0000000000000002, NoFlag},
        {0x7ffffffffffffffe, NoFlag, 0x7fffffffffffffff, NoFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0x8000000000000000, NFlag, 0x8000000000000001, NFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag}},
       {{0x0000000000000001, NoFlag, 0x0000000000000002, NoFlag},
        {0x0000000000000002, NoFlag, 0x0000000000000003, NoFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0x8000000000000000, NVFlag, 0x8000000000000001, NVFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0x8000000000000002, NFlag, 0x8000000000000003, NFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag}},
       {{0x7ffffffffffffffe, NoFlag, 0x7fffffffffffffff, NoFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0xfffffffffffffffc, NVFlag, 0xfffffffffffffffd, NVFlag},
        {0xfffffffffffffffd, NVFlag, 0xfffffffffffffffe, NVFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x7ffffffffffffffc, CFlag, 0x7ffffffffffffffd, CFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag}},
       {{0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0x8000000000000000, NVFlag, 0x8000000000000001, NVFlag},
        {0xfffffffffffffffd, NVFlag, 0xfffffffffffffffe, NVFlag},
        {0xfffffffffffffffe, NVFlag, 0xffffffffffffffff, NVFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x7ffffffffffffffe, CFlag, 0x7fffffffffffffff, CFlag}},
       {{0x8000000000000000, NFlag, 0x8000000000000001, NFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x0000000000000000, ZCVFlag, 0x0000000000000001, CVFlag},
        {0x0000000000000001, CVFlag, 0x0000000000000002, CVFlag},
        {0x7ffffffffffffffe, CVFlag, 0x7fffffffffffffff, CVFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag}},
       {{0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0x8000000000000002, NFlag, 0x8000000000000003, NFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0x0000000000000001, CVFlag, 0x0000000000000002, CVFlag},
        {0x0000000000000002, CVFlag, 0x0000000000000003, CVFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0x8000000000000000, NCFlag, 0x8000000000000001, NCFlag}},
       {{0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x7ffffffffffffffc, CFlag, 0x7ffffffffffffffd, CFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x7ffffffffffffffe, CVFlag, 0x7fffffffffffffff, CVFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0xfffffffffffffffc, NCFlag, 0xfffffffffffffffd, NCFlag},
        {0xfffffffffffffffd, NCFlag, 0xfffffffffffffffe, NCFlag}},
       {{0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x7ffffffffffffffe, CFlag, 0x7fffffffffffffff, CFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0x8000000000000000, NCFlag, 0x8000000000000001, NCFlag},
        {0xfffffffffffffffd, NCFlag, 0xfffffffffffffffe, NCFlag},
        {0xfffffffffffffffe, NCFlag, 0xffffffffffffffff, NCFlag}}};

  static const Expected expected_sbcs_x[input_count][input_count] =
      {{{0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0x8000000000000000, NFlag, 0x8000000000000001, NFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0x7ffffffffffffffe, NoFlag, 0x7fffffffffffffff, NoFlag},
        {0x0000000000000001, NoFlag, 0x0000000000000002, NoFlag},
        {0x0000000000000000, ZFlag, 0x0000000000000001, NoFlag}},
       {{0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x8000000000000002, NFlag, 0x8000000000000003, NFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0x8000000000000000, NVFlag, 0x8000000000000001, NVFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0x0000000000000002, NoFlag, 0x0000000000000003, NoFlag},
        {0x0000000000000001, NoFlag, 0x0000000000000002, NoFlag}},
       {{0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x7ffffffffffffffc, CFlag, 0x7ffffffffffffffd, CFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0xfffffffffffffffd, NVFlag, 0xfffffffffffffffe, NVFlag},
        {0xfffffffffffffffc, NVFlag, 0xfffffffffffffffd, NVFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag},
        {0x7ffffffffffffffe, NoFlag, 0x7fffffffffffffff, NoFlag}},
       {{0x7ffffffffffffffe, CFlag, 0x7fffffffffffffff, CFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0xfffffffffffffffe, NVFlag, 0xffffffffffffffff, NVFlag},
        {0xfffffffffffffffd, NVFlag, 0xfffffffffffffffe, NVFlag},
        {0x8000000000000000, NVFlag, 0x8000000000000001, NVFlag},
        {0x7fffffffffffffff, NoFlag, 0x8000000000000000, NVFlag}},
       {{0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0x7ffffffffffffffe, CVFlag, 0x7fffffffffffffff, CVFlag},
        {0x0000000000000001, CVFlag, 0x0000000000000002, CVFlag},
        {0x0000000000000000, ZCVFlag, 0x0000000000000001, CVFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag},
        {0x8000000000000000, NFlag, 0x8000000000000001, NFlag}},
       {{0x8000000000000000, NCFlag, 0x8000000000000001, NCFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0x0000000000000002, CVFlag, 0x0000000000000003, CVFlag},
        {0x0000000000000001, CVFlag, 0x0000000000000002, CVFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0x8000000000000002, NFlag, 0x8000000000000003, NFlag},
        {0x8000000000000001, NFlag, 0x8000000000000002, NFlag}},
       {{0xfffffffffffffffd, NCFlag, 0xfffffffffffffffe, NCFlag},
        {0xfffffffffffffffc, NCFlag, 0xfffffffffffffffd, NCFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0x7ffffffffffffffe, CVFlag, 0x7fffffffffffffff, CVFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x7ffffffffffffffc, CFlag, 0x7ffffffffffffffd, CFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag},
        {0xfffffffffffffffe, NFlag, 0xffffffffffffffff, NFlag}},
       {{0xfffffffffffffffe, NCFlag, 0xffffffffffffffff, NCFlag},
        {0xfffffffffffffffd, NCFlag, 0xfffffffffffffffe, NCFlag},
        {0x8000000000000000, NCFlag, 0x8000000000000001, NCFlag},
        {0x7fffffffffffffff, CVFlag, 0x8000000000000000, NCFlag},
        {0x7ffffffffffffffe, CFlag, 0x7fffffffffffffff, CFlag},
        {0x7ffffffffffffffd, CFlag, 0x7ffffffffffffffe, CFlag},
        {0x0000000000000000, ZCFlag, 0x0000000000000001, CFlag},
        {0xffffffffffffffff, NFlag, 0x0000000000000000, ZCFlag}}};

  for (size_t left = 0; left < input_count; left++) {
    for (size_t right = 0; right < input_count; right++) {
      const Expected& expected = expected_adcs_x[left][right];
      AdcsSbcsHelper(&MacroAssembler::Adcs,
                     inputs[left],
                     inputs[right],
                     0,
                     expected.carry0_result,
                     expected.carry0_flags);
      AdcsSbcsHelper(&MacroAssembler::Adcs,
                     inputs[left],
                     inputs[right],
                     1,
                     expected.carry1_result,
                     expected.carry1_flags);
    }
  }

  for (size_t left = 0; left < input_count; left++) {
    for (size_t right = 0; right < input_count; right++) {
      const Expected& expected = expected_sbcs_x[left][right];
      AdcsSbcsHelper(&MacroAssembler::Sbcs,
                     inputs[left],
                     inputs[right],
                     0,
                     expected.carry0_result,
                     expected.carry0_flags);
      AdcsSbcsHelper(&MacroAssembler::Sbcs,
                     inputs[left],
                     inputs[right],
                     1,
                     expected.carry1_result,
                     expected.carry1_flags);
    }
  }
}


TEST(adcs_sbcs_w) {
  uint32_t inputs[] = {
      0x00000000,
      0x00000001,
      0x7ffffffe,
      0x7fffffff,
      0x80000000,
      0x80000001,
      0xfffffffe,
      0xffffffff,
  };
  static const size_t input_count = sizeof(inputs) / sizeof(inputs[0]);

  struct Expected {
    uint32_t carry0_result;
    StatusFlags carry0_flags;
    uint32_t carry1_result;
    StatusFlags carry1_flags;
  };

  static const Expected expected_adcs_w[input_count][input_count] =
      {{{0x00000000, ZFlag, 0x00000001, NoFlag},
        {0x00000001, NoFlag, 0x00000002, NoFlag},
        {0x7ffffffe, NoFlag, 0x7fffffff, NoFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0x80000000, NFlag, 0x80000001, NFlag},
        {0x80000001, NFlag, 0x80000002, NFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag}},
       {{0x00000001, NoFlag, 0x00000002, NoFlag},
        {0x00000002, NoFlag, 0x00000003, NoFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0x80000000, NVFlag, 0x80000001, NVFlag},
        {0x80000001, NFlag, 0x80000002, NFlag},
        {0x80000002, NFlag, 0x80000003, NFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag}},
       {{0x7ffffffe, NoFlag, 0x7fffffff, NoFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0xfffffffc, NVFlag, 0xfffffffd, NVFlag},
        {0xfffffffd, NVFlag, 0xfffffffe, NVFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x7ffffffc, CFlag, 0x7ffffffd, CFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag}},
       {{0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0x80000000, NVFlag, 0x80000001, NVFlag},
        {0xfffffffd, NVFlag, 0xfffffffe, NVFlag},
        {0xfffffffe, NVFlag, 0xffffffff, NVFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x7ffffffe, CFlag, 0x7fffffff, CFlag}},
       {{0x80000000, NFlag, 0x80000001, NFlag},
        {0x80000001, NFlag, 0x80000002, NFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x00000000, ZCVFlag, 0x00000001, CVFlag},
        {0x00000001, CVFlag, 0x00000002, CVFlag},
        {0x7ffffffe, CVFlag, 0x7fffffff, CVFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag}},
       {{0x80000001, NFlag, 0x80000002, NFlag},
        {0x80000002, NFlag, 0x80000003, NFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag},
        {0x00000001, CVFlag, 0x00000002, CVFlag},
        {0x00000002, CVFlag, 0x00000003, CVFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0x80000000, NCFlag, 0x80000001, NCFlag}},
       {{0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x7ffffffc, CFlag, 0x7ffffffd, CFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x7ffffffe, CVFlag, 0x7fffffff, CVFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0xfffffffc, NCFlag, 0xfffffffd, NCFlag},
        {0xfffffffd, NCFlag, 0xfffffffe, NCFlag}},
       {{0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x7ffffffe, CFlag, 0x7fffffff, CFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0x80000000, NCFlag, 0x80000001, NCFlag},
        {0xfffffffd, NCFlag, 0xfffffffe, NCFlag},
        {0xfffffffe, NCFlag, 0xffffffff, NCFlag}}};

  static const Expected expected_sbcs_w[input_count][input_count] =
      {{{0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0x80000001, NFlag, 0x80000002, NFlag},
        {0x80000000, NFlag, 0x80000001, NFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0x7ffffffe, NoFlag, 0x7fffffff, NoFlag},
        {0x00000001, NoFlag, 0x00000002, NoFlag},
        {0x00000000, ZFlag, 0x00000001, NoFlag}},
       {{0x00000000, ZCFlag, 0x00000001, CFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x80000002, NFlag, 0x80000003, NFlag},
        {0x80000001, NFlag, 0x80000002, NFlag},
        {0x80000000, NVFlag, 0x80000001, NVFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0x00000002, NoFlag, 0x00000003, NoFlag},
        {0x00000001, NoFlag, 0x00000002, NoFlag}},
       {{0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x7ffffffc, CFlag, 0x7ffffffd, CFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0xfffffffd, NVFlag, 0xfffffffe, NVFlag},
        {0xfffffffc, NVFlag, 0xfffffffd, NVFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag},
        {0x7ffffffe, NoFlag, 0x7fffffff, NoFlag}},
       {{0x7ffffffe, CFlag, 0x7fffffff, CFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0xfffffffe, NVFlag, 0xffffffff, NVFlag},
        {0xfffffffd, NVFlag, 0xfffffffe, NVFlag},
        {0x80000000, NVFlag, 0x80000001, NVFlag},
        {0x7fffffff, NoFlag, 0x80000000, NVFlag}},
       {{0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0x7ffffffe, CVFlag, 0x7fffffff, CVFlag},
        {0x00000001, CVFlag, 0x00000002, CVFlag},
        {0x00000000, ZCVFlag, 0x00000001, CVFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag},
        {0x80000001, NFlag, 0x80000002, NFlag},
        {0x80000000, NFlag, 0x80000001, NFlag}},
       {{0x80000000, NCFlag, 0x80000001, NCFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0x00000002, CVFlag, 0x00000003, CVFlag},
        {0x00000001, CVFlag, 0x00000002, CVFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0x80000002, NFlag, 0x80000003, NFlag},
        {0x80000001, NFlag, 0x80000002, NFlag}},
       {{0xfffffffd, NCFlag, 0xfffffffe, NCFlag},
        {0xfffffffc, NCFlag, 0xfffffffd, NCFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0x7ffffffe, CVFlag, 0x7fffffff, CVFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x7ffffffc, CFlag, 0x7ffffffd, CFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag},
        {0xfffffffe, NFlag, 0xffffffff, NFlag}},
       {{0xfffffffe, NCFlag, 0xffffffff, NCFlag},
        {0xfffffffd, NCFlag, 0xfffffffe, NCFlag},
        {0x80000000, NCFlag, 0x80000001, NCFlag},
        {0x7fffffff, CVFlag, 0x80000000, NCFlag},
        {0x7ffffffe, CFlag, 0x7fffffff, CFlag},
        {0x7ffffffd, CFlag, 0x7ffffffe, CFlag},
        {0x00000000, ZCFlag, 0x00000001, CFlag},
        {0xffffffff, NFlag, 0x00000000, ZCFlag}}};

  for (size_t left = 0; left < input_count; left++) {
    for (size_t right = 0; right < input_count; right++) {
      const Expected& expected = expected_adcs_w[left][right];
      AdcsSbcsHelper(&MacroAssembler::Adcs,
                     inputs[left],
                     inputs[right],
                     0,
                     expected.carry0_result,
                     expected.carry0_flags);
      AdcsSbcsHelper(&MacroAssembler::Adcs,
                     inputs[left],
                     inputs[right],
                     1,
                     expected.carry1_result,
                     expected.carry1_flags);
    }
  }

  for (size_t left = 0; left < input_count; left++) {
    for (size_t right = 0; right < input_count; right++) {
      const Expected& expected = expected_sbcs_w[left][right];
      AdcsSbcsHelper(&MacroAssembler::Sbcs,
                     inputs[left],
                     inputs[right],
                     0,
                     expected.carry0_result,
                     expected.carry0_flags);
      AdcsSbcsHelper(&MacroAssembler::Sbcs,
                     inputs[left],
                     inputs[right],
                     1,
                     expected.carry1_result,
                     expected.carry1_flags);
    }
  }
}


TEST(adc_sbc_shift) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x2, 0x0123456789abcdef);
  __ Mov(x3, 0xfedcba9876543210);
  __ Mov(x4, 0xffffffffffffffff);

  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));

  __ Adc(x5, x2, Operand(x3));
  __ Adc(x6, x0, Operand(x1, LSL, 60));
  __ Sbc(x7, x4, Operand(x3, LSR, 4));
  __ Adc(x8, x2, Operand(x3, ASR, 4));
  __ Adc(x9, x2, Operand(x3, ROR, 8));

  __ Adc(w10, w2, Operand(w3));
  __ Adc(w11, w0, Operand(w1, LSL, 30));
  __ Sbc(w12, w4, Operand(w3, LSR, 4));
  __ Adc(w13, w2, Operand(w3, ASR, 4));
  __ Adc(w14, w2, Operand(w3, ROR, 8));

  // Set the C flag.
  __ Cmp(w0, Operand(w0));

  __ Adc(x18, x2, Operand(x3));
  __ Adc(x19, x0, Operand(x1, LSL, 60));
  __ Sbc(x20, x4, Operand(x3, LSR, 4));
  __ Adc(x21, x2, Operand(x3, ASR, 4));
  __ Adc(x22, x2, Operand(x3, ROR, 8));

  __ Adc(w23, w2, Operand(w3));
  __ Adc(w24, w0, Operand(w1, LSL, 30));
  __ Sbc(w25, w4, Operand(w3, LSR, 4));
  __ Adc(w26, w2, Operand(w3, ASR, 4));
  __ Adc(w27, w2, Operand(w3, ROR, 8));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xffffffffffffffff, x5);
    ASSERT_EQUAL_64(INT64_C(1) << 60, x6);
    ASSERT_EQUAL_64(0xf0123456789abcdd, x7);
    ASSERT_EQUAL_64(0x0111111111111110, x8);
    ASSERT_EQUAL_64(0x1222222222222221, x9);

    ASSERT_EQUAL_32(0xffffffff, w10);
    ASSERT_EQUAL_32(INT32_C(1) << 30, w11);
    ASSERT_EQUAL_32(0xf89abcdd, w12);
    ASSERT_EQUAL_32(0x91111110, w13);
    ASSERT_EQUAL_32(0x9a222221, w14);

    ASSERT_EQUAL_64(0xffffffffffffffff + 1, x18);
    ASSERT_EQUAL_64((INT64_C(1) << 60) + 1, x19);
    ASSERT_EQUAL_64(0xf0123456789abcdd + 1, x20);
    ASSERT_EQUAL_64(0x0111111111111110 + 1, x21);
    ASSERT_EQUAL_64(0x1222222222222221 + 1, x22);

    ASSERT_EQUAL_32(0xffffffff + 1, w23);
    ASSERT_EQUAL_32((INT32_C(1) << 30) + 1, w24);
    ASSERT_EQUAL_32(0xf89abcdd + 1, w25);
    ASSERT_EQUAL_32(0x91111110 + 1, w26);
    ASSERT_EQUAL_32(0x9a222221 + 1, w27);
  }
}


TEST(adc_sbc_extend) {
  SETUP();

  START();
  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));

  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x2, 0x0123456789abcdef);

  __ Adc(x10, x1, Operand(w2, UXTB, 1));
  __ Adc(x11, x1, Operand(x2, SXTH, 2));
  __ Sbc(x12, x1, Operand(w2, UXTW, 4));
  __ Adc(x13, x1, Operand(x2, UXTX, 4));

  __ Adc(w14, w1, Operand(w2, UXTB, 1));
  __ Adc(w15, w1, Operand(w2, SXTH, 2));
  __ Adc(w9, w1, Operand(w2, UXTW, 4));

  // Set the C flag.
  __ Cmp(w0, Operand(w0));

  __ Adc(x20, x1, Operand(w2, UXTB, 1));
  __ Adc(x21, x1, Operand(x2, SXTH, 2));
  __ Sbc(x22, x1, Operand(w2, UXTW, 4));
  __ Adc(x23, x1, Operand(x2, UXTX, 4));

  __ Adc(w24, w1, Operand(w2, UXTB, 1));
  __ Adc(w25, w1, Operand(w2, SXTH, 2));
  __ Adc(w26, w1, Operand(w2, UXTW, 4));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1df, x10);
    ASSERT_EQUAL_64(0xffffffffffff37bd, x11);
    ASSERT_EQUAL_64(0xfffffff765432110, x12);
    ASSERT_EQUAL_64(0x123456789abcdef1, x13);

    ASSERT_EQUAL_32(0x1df, w14);
    ASSERT_EQUAL_32(0xffff37bd, w15);
    ASSERT_EQUAL_32(0x9abcdef1, w9);

    ASSERT_EQUAL_64(0x1df + 1, x20);
    ASSERT_EQUAL_64(0xffffffffffff37bd + 1, x21);
    ASSERT_EQUAL_64(0xfffffff765432110 + 1, x22);
    ASSERT_EQUAL_64(0x123456789abcdef1 + 1, x23);

    ASSERT_EQUAL_32(0x1df + 1, w24);
    ASSERT_EQUAL_32(0xffff37bd + 1, w25);
    ASSERT_EQUAL_32(0x9abcdef1 + 1, w26);
  }

  // Check that adc correctly sets the condition flags.
  START();
  __ Mov(x0, 0xff);
  __ Mov(x1, 0xffffffffffffffff);
  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));
  __ Adcs(x10, x0, Operand(x1, SXTX, 1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(CFlag);
  }

  START();
  __ Mov(x0, 0x7fffffffffffffff);
  __ Mov(x1, 1);
  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));
  __ Adcs(x10, x0, Operand(x1, UXTB, 2));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NVFlag);
  }

  START();
  __ Mov(x0, 0x7fffffffffffffff);
  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));
  __ Adcs(x10, x0, Operand(1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NVFlag);
  }
}


TEST(adc_sbc_wide_imm) {
  SETUP();

  START();
  __ Mov(x0, 0);

  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));

  __ Adc(x7, x0, Operand(0x1234567890abcdef));
  __ Adc(w8, w0, Operand(0xffffffff));
  __ Sbc(x9, x0, Operand(0x1234567890abcdef));
  __ Sbc(w10, w0, Operand(0xffffffff));
  __ Ngc(x11, Operand(0xffffffff00000000));
  __ Ngc(w12, Operand(0xffff0000));

  // Set the C flag.
  __ Cmp(w0, Operand(w0));

  __ Adc(x18, x0, Operand(0x1234567890abcdef));
  __ Adc(w19, w0, Operand(0xffffffff));
  __ Sbc(x20, x0, Operand(0x1234567890abcdef));
  __ Sbc(w21, w0, Operand(0xffffffff));
  __ Ngc(x22, Operand(0xffffffff00000000));
  __ Ngc(w23, Operand(0xffff0000));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1234567890abcdef, x7);
    ASSERT_EQUAL_64(0xffffffff, x8);
    ASSERT_EQUAL_64(0xedcba9876f543210, x9);
    ASSERT_EQUAL_64(0, x10);
    ASSERT_EQUAL_64(0xffffffff, x11);
    ASSERT_EQUAL_64(0xffff, x12);

    ASSERT_EQUAL_64(0x1234567890abcdef + 1, x18);
    ASSERT_EQUAL_64(0, x19);
    ASSERT_EQUAL_64(0xedcba9876f543211, x20);
    ASSERT_EQUAL_64(1, x21);
    ASSERT_EQUAL_64(0x0000000100000000, x22);
    ASSERT_EQUAL_64(0x0000000000010000, x23);
  }
}


TEST(rmif) {
  SETUP_WITH_FEATURES(CPUFeatures::kFlagM);

  START();
  __ Mov(x0, 0x0123456789abcdef);

  // Clear bits of `rmif` masks leave NZCV unmodified, so we need to initialise
  // it to a known state to make the test reproducible.
  __ Msr(NZCV, x0);

  // Set NZCV to 0b1011 (0xb)
  __ Rmif(x0, 0, NCVFlag);
  __ Mrs(x1, NZCV);

  // Set NZCV to 0b0111 (0x7)
  __ Rmif(x0, 6, NZCVFlag);
  __ Mrs(x2, NZCV);

  // Set Z to 0, NZCV = 0b0011 (0x3)
  __ Rmif(x0, 60, ZFlag);
  __ Mrs(x3, NZCV);

  // Set N to 1 and C to 0, NZCV = 0b1001 (0x9)
  __ Rmif(x0, 62, NCFlag);
  __ Mrs(x4, NZCV);

  // No change to NZCV
  __ Rmif(x0, 0, NoFlag);
  __ Mrs(x5, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_32(NCVFlag, w1);
    ASSERT_EQUAL_32(ZCVFlag, w2);
    ASSERT_EQUAL_32(CVFlag, w3);
    ASSERT_EQUAL_32(NVFlag, w4);
    ASSERT_EQUAL_32(NVFlag, w5);
  }
}


TEST(setf8_setf16) {
  SETUP_WITH_FEATURES(CPUFeatures::kFlagM);

  START();
  __ Mov(x0, 0x0);
  __ Mov(x1, 0x1);
  __ Mov(x2, 0xff);
  __ Mov(x3, 0x100);
  __ Mov(x4, 0x101);
  __ Mov(x5, 0xffff);
  __ Mov(x6, 0x10000);
  __ Mov(x7, 0x10001);
  __ Mov(x8, 0xfffffffff);

  // These instruction don't modify 'C', so give it a consistent value.
  __ Ands(xzr, xzr, 0);

  __ Setf8(w0);
  __ Mrs(x9, NZCV);
  __ Setf8(w1);
  __ Mrs(x10, NZCV);
  __ Setf8(w2);
  __ Mrs(x11, NZCV);
  __ Setf8(w3);
  __ Mrs(x12, NZCV);
  __ Setf8(w4);
  __ Mrs(x13, NZCV);
  __ Setf8(w8);
  __ Mrs(x14, NZCV);

  __ Setf16(w0);
  __ Mrs(x15, NZCV);
  __ Setf16(w1);
  __ Mrs(x16, NZCV);
  __ Setf16(w5);
  __ Mrs(x17, NZCV);
  __ Setf16(w6);
  __ Mrs(x18, NZCV);
  __ Setf16(w7);
  __ Mrs(x19, NZCV);
  __ Setf16(w8);
  __ Mrs(x20, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(ZFlag, w9);    // Zero
    ASSERT_EQUAL_32(NoFlag, w10);  // Regular int8
    ASSERT_EQUAL_32(NVFlag, w11);  // Negative but not sign-extended (overflow)
    ASSERT_EQUAL_32(ZVFlag, w12);  // Overflow with zero remainder
    ASSERT_EQUAL_32(VFlag, w13);   // Overflow with non-zero remainder
    ASSERT_EQUAL_32(NFlag, w14);   // Negative and sign-extended

    ASSERT_EQUAL_32(ZFlag, w15);   // Zero
    ASSERT_EQUAL_32(NoFlag, w16);  // Regular int16
    ASSERT_EQUAL_32(NVFlag, w17);  // Negative but not sign-extended (overflow)
    ASSERT_EQUAL_32(ZVFlag, w18);  // Overflow with zero remainder
    ASSERT_EQUAL_32(VFlag, w19);   // Overflow with non-zero remainder
    ASSERT_EQUAL_32(NFlag, w20);   // Negative and sign-extended
  }
}


TEST(flags) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x1111111111111111);
  __ Neg(x10, Operand(x0));
  __ Neg(x11, Operand(x1));
  __ Neg(w12, Operand(w1));
  // Clear the C flag.
  __ Adds(x0, x0, Operand(0));
  __ Ngc(x13, Operand(x0));
  // Set the C flag.
  __ Cmp(x0, Operand(x0));
  __ Ngc(w14, Operand(w0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x10);
    ASSERT_EQUAL_64(-0x1111111111111111, x11);
    ASSERT_EQUAL_32(-0x11111111, w12);
    ASSERT_EQUAL_64(-1, x13);
    ASSERT_EQUAL_32(0, w14);
  }

  START();
  __ Mov(x0, 0);
  __ Cmp(x0, Operand(x0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZCFlag);
  }

  START();
  __ Mov(w0, 0);
  __ Cmp(w0, Operand(w0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZCFlag);
  }

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x1111111111111111);
  __ Cmp(x0, Operand(x1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
  }

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 0x11111111);
  __ Cmp(w0, Operand(w1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
  }

  START();
  __ Mov(x1, 0x1111111111111111);
  __ Cmp(x1, Operand(0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(CFlag);
  }

  START();
  __ Mov(w1, 0x11111111);
  __ Cmp(w1, Operand(0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(CFlag);
  }

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x7fffffffffffffff);
  __ Cmn(x1, Operand(x0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NVFlag);
  }

  START();
  __ Mov(w0, 1);
  __ Mov(w1, 0x7fffffff);
  __ Cmn(w1, Operand(w0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NVFlag);
  }

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0xffffffffffffffff);
  __ Cmn(x1, Operand(x0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZCFlag);
  }

  START();
  __ Mov(w0, 1);
  __ Mov(w1, 0xffffffff);
  __ Cmn(w1, Operand(w0));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZCFlag);
  }

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 1);
  // Clear the C flag.
  __ Adds(w0, w0, Operand(0));
  __ Ngcs(w0, Operand(w1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(NFlag);
  }

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 0);
  // Set the C flag.
  __ Cmp(w0, Operand(w0));
  __ Ngcs(w0, Operand(w1));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZCFlag);
  }
}


TEST(cmp_shift) {
  SETUP();

  START();
  __ Mov(x18, 0xf0000000);
  __ Mov(x19, 0xf000000010000000);
  __ Mov(x20, 0xf0000000f0000000);
  __ Mov(x21, 0x7800000078000000);
  __ Mov(x22, 0x3c0000003c000000);
  __ Mov(x23, 0x8000000780000000);
  __ Mov(x24, 0x0000000f00000000);
  __ Mov(x25, 0x00000003c0000000);
  __ Mov(x26, 0x8000000780000000);
  __ Mov(x27, 0xc0000003);

  __ Cmp(w20, Operand(w21, LSL, 1));
  __ Mrs(x0, NZCV);

  __ Cmp(x20, Operand(x22, LSL, 2));
  __ Mrs(x1, NZCV);

  __ Cmp(w19, Operand(w23, LSR, 3));
  __ Mrs(x2, NZCV);

  __ Cmp(x18, Operand(x24, LSR, 4));
  __ Mrs(x3, NZCV);

  __ Cmp(w20, Operand(w25, ASR, 2));
  __ Mrs(x4, NZCV);

  __ Cmp(x20, Operand(x26, ASR, 3));
  __ Mrs(x5, NZCV);

  __ Cmp(w27, Operand(w22, ROR, 28));
  __ Mrs(x6, NZCV);

  __ Cmp(x20, Operand(x21, ROR, 31));
  __ Mrs(x7, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(ZCFlag, w0);
    ASSERT_EQUAL_32(ZCFlag, w1);
    ASSERT_EQUAL_32(ZCFlag, w2);
    ASSERT_EQUAL_32(ZCFlag, w3);
    ASSERT_EQUAL_32(ZCFlag, w4);
    ASSERT_EQUAL_32(ZCFlag, w5);
    ASSERT_EQUAL_32(ZCFlag, w6);
    ASSERT_EQUAL_32(ZCFlag, w7);
  }
}


TEST(cmp_extend) {
  SETUP();

  START();
  __ Mov(w20, 0x2);
  __ Mov(w21, 0x1);
  __ Mov(x22, 0xffffffffffffffff);
  __ Mov(x23, 0xff);
  __ Mov(x24, 0xfffffffffffffffe);
  __ Mov(x25, 0xffff);
  __ Mov(x26, 0xffffffff);

  __ Cmp(w20, Operand(w21, LSL, 1));
  __ Mrs(x0, NZCV);

  __ Cmp(x22, Operand(x23, SXTB, 0));
  __ Mrs(x1, NZCV);

  __ Cmp(x24, Operand(x23, SXTB, 1));
  __ Mrs(x2, NZCV);

  __ Cmp(x24, Operand(x23, UXTB, 1));
  __ Mrs(x3, NZCV);

  __ Cmp(w22, Operand(w25, UXTH));
  __ Mrs(x4, NZCV);

  __ Cmp(x22, Operand(x25, SXTH));
  __ Mrs(x5, NZCV);

  __ Cmp(x22, Operand(x26, UXTW));
  __ Mrs(x6, NZCV);

  __ Cmp(x24, Operand(x26, SXTW, 1));
  __ Mrs(x7, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(ZCFlag, w0);
    ASSERT_EQUAL_32(ZCFlag, w1);
    ASSERT_EQUAL_32(ZCFlag, w2);
    ASSERT_EQUAL_32(NCFlag, w3);
    ASSERT_EQUAL_32(NCFlag, w4);
    ASSERT_EQUAL_32(ZCFlag, w5);
    ASSERT_EQUAL_32(NCFlag, w6);
    ASSERT_EQUAL_32(ZCFlag, w7);
  }
}


TEST(ccmp) {
  SETUP();

  START();
  __ Mov(w16, 0);
  __ Mov(w17, 1);
  __ Cmp(w16, w16);
  __ Ccmp(w16, w17, NCFlag, eq);
  __ Mrs(x0, NZCV);

  __ Cmp(w16, w16);
  __ Ccmp(w16, w17, NCFlag, ne);
  __ Mrs(x1, NZCV);

  __ Cmp(x16, x16);
  __ Ccmn(x16, 2, NZCVFlag, eq);
  __ Mrs(x2, NZCV);

  __ Cmp(x16, x16);
  __ Ccmn(x16, 2, NZCVFlag, ne);
  __ Mrs(x3, NZCV);

  // The MacroAssembler does not allow al as a condition.
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ ccmp(x16, x16, NZCVFlag, al);
  }
  __ Mrs(x4, NZCV);

  // The MacroAssembler does not allow nv as a condition.
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ ccmp(x16, x16, NZCVFlag, nv);
  }
  __ Mrs(x5, NZCV);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(NFlag, w0);
    ASSERT_EQUAL_32(NCFlag, w1);
    ASSERT_EQUAL_32(NoFlag, w2);
    ASSERT_EQUAL_32(NZCVFlag, w3);
    ASSERT_EQUAL_32(ZCFlag, w4);
    ASSERT_EQUAL_32(ZCFlag, w5);
  }
}


TEST(ccmp_wide_imm) {
  SETUP();

  START();
  __ Mov(w20, 0);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(w20, Operand(0x12345678), NZCVFlag, eq);
  __ Mrs(x0, NZCV);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(x20, Operand(0xffffffffffffffff), NZCVFlag, eq);
  __ Mrs(x1, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(NFlag, w0);
    ASSERT_EQUAL_32(NoFlag, w1);
  }
}


TEST(ccmp_shift_extend) {
  SETUP();

  START();
  __ Mov(w20, 0x2);
  __ Mov(w21, 0x1);
  __ Mov(x22, 0xffffffffffffffff);
  __ Mov(x23, 0xff);
  __ Mov(x24, 0xfffffffffffffffe);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(w20, Operand(w21, LSL, 1), NZCVFlag, eq);
  __ Mrs(x0, NZCV);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(x22, Operand(x23, SXTB, 0), NZCVFlag, eq);
  __ Mrs(x1, NZCV);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(x24, Operand(x23, SXTB, 1), NZCVFlag, eq);
  __ Mrs(x2, NZCV);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(x24, Operand(x23, UXTB, 1), NZCVFlag, eq);
  __ Mrs(x3, NZCV);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(x24, Operand(x23, UXTB, 1), NZCVFlag, ne);
  __ Mrs(x4, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(ZCFlag, w0);
    ASSERT_EQUAL_32(ZCFlag, w1);
    ASSERT_EQUAL_32(ZCFlag, w2);
    ASSERT_EQUAL_32(NCFlag, w3);
    ASSERT_EQUAL_32(NZCVFlag, w4);
  }
}


TEST(csel_reg) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x24, 0x0000000f0000000f);
  __ Mov(x25, 0x0000001f0000001f);

  __ Cmp(w16, Operand(0));
  __ Csel(w0, w24, w25, eq);
  __ Csel(w1, w24, w25, ne);
  __ Csinc(w2, w24, w25, mi);
  __ Csinc(w3, w24, w25, pl);

  // The MacroAssembler does not allow al or nv as a condition.
  {
    ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
    __ csel(w13, w24, w25, al);
    __ csel(x14, x24, x25, nv);
  }

  __ Cmp(x16, Operand(1));
  __ Csinv(x4, x24, x25, gt);
  __ Csinv(x5, x24, x25, le);
  __ Csneg(x6, x24, x25, hs);
  __ Csneg(x7, x24, x25, lo);

  __ Cset(w8, ne);
  __ Csetm(w9, ne);
  __ Cinc(x10, x25, ne);
  __ Cinv(x11, x24, ne);
  __ Cneg(x12, x24, ne);

  // The MacroAssembler does not allow al or nv as a condition.
  {
    ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
    __ csel(w15, w24, w25, al);
    __ csel(x17, x24, x25, nv);
  }

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0000000f, x0);
    ASSERT_EQUAL_64(0x0000001f, x1);
    ASSERT_EQUAL_64(0x00000020, x2);
    ASSERT_EQUAL_64(0x0000000f, x3);
    ASSERT_EQUAL_64(0xffffffe0ffffffe0, x4);
    ASSERT_EQUAL_64(0x0000000f0000000f, x5);
    ASSERT_EQUAL_64(0xffffffe0ffffffe1, x6);
    ASSERT_EQUAL_64(0x0000000f0000000f, x7);
    ASSERT_EQUAL_64(0x00000001, x8);
    ASSERT_EQUAL_64(0xffffffff, x9);
    ASSERT_EQUAL_64(0x0000001f00000020, x10);
    ASSERT_EQUAL_64(0xfffffff0fffffff0, x11);
    ASSERT_EQUAL_64(0xfffffff0fffffff1, x12);
    ASSERT_EQUAL_64(0x0000000f, x13);
    ASSERT_EQUAL_64(0x0000000f0000000f, x14);
    ASSERT_EQUAL_64(0x0000000f, x15);
    ASSERT_EQUAL_64(0x0000000f0000000f, x17);
  }
}

TEST(csel_zero) {
  SETUP();

  START();

  __ Mov(x15, 0x0);
  __ Mov(x16, 0x0000001f0000002f);

  // Check results when zero registers are used as inputs
  // for Csinc, Csinv and Csneg for both true and false conditions.
  __ Cmp(x15, 0);
  __ Csinc(x0, x16, xzr, eq);
  __ Csinc(x1, xzr, x16, eq);
  __ Cmp(x15, 1);
  __ Csinc(w2, w16, wzr, eq);
  __ Csinc(w3, wzr, w16, eq);

  __ Csinc(x4, xzr, xzr, eq);

  __ Cmp(x15, 0);
  __ Csinv(x5, x16, xzr, eq);
  __ Csinv(x6, xzr, x16, eq);
  __ Cmp(x15, 1);
  __ Csinv(w7, w16, wzr, eq);
  __ Csinv(w8, wzr, w16, eq);

  __ Csinv(x9, xzr, xzr, eq);

  __ Cmp(x15, 0);
  __ Csneg(x10, x16, xzr, eq);
  __ Csneg(x11, xzr, x16, eq);
  __ Cmp(x15, 1);
  __ Csneg(w12, w16, wzr, eq);
  __ Csneg(w13, wzr, w16, eq);

  __ Csneg(x14, xzr, xzr, eq);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0000001f0000002f, x0);
    ASSERT_EQUAL_64(0x0, x1);
    ASSERT_EQUAL_32(0x1, w2);
    ASSERT_EQUAL_32(0x30, w3);
    ASSERT_EQUAL_64(0x1, x4);
    ASSERT_EQUAL_64(0x0000001f0000002f, x5);
    ASSERT_EQUAL_64(0x0, x6);
    ASSERT_EQUAL_32(0xffffffff, w7);
    ASSERT_EQUAL_32(0xffffffd0, w8);
    ASSERT_EQUAL_64(0xffffffffffffffff, x9);
    ASSERT_EQUAL_64(0x0000001f0000002f, x10);
    ASSERT_EQUAL_64(0x0, x11);
    ASSERT_EQUAL_32(0x0, w12);
    ASSERT_EQUAL_32(0xffffffd1, w13);
    ASSERT_EQUAL_64(0x0, x14);
  }
}


TEST(csel_imm) {
  SETUP();

  int values[] = {-123, -2, -1, 0, 1, 2, 123};
  int n_values = sizeof(values) / sizeof(values[0]);

  for (int i = 0; i < n_values; i++) {
    for (int j = 0; j < n_values; j++) {
      int left = values[i];
      int right = values[j];

      START();
      __ Mov(x10, 0);
      __ Cmp(x10, 0);
      __ Csel(w0, left, right, eq);
      __ Csel(w1, left, right, ne);
      __ Csel(x2, left, right, eq);
      __ Csel(x3, left, right, ne);

      END();

      if (CAN_RUN()) {
        RUN();

        ASSERT_EQUAL_32(left, w0);
        ASSERT_EQUAL_32(right, w1);
        ASSERT_EQUAL_64(left, x2);
        ASSERT_EQUAL_64(right, x3);
      }
    }
  }
}


TEST(csel_mixed) {
  SETUP();

  START();
  __ Mov(x18, 0);
  __ Mov(x19, 0x80000000);
  __ Mov(x20, 0x8000000000000000);

  __ Cmp(x18, Operand(0));
  __ Csel(w0, w19, -2, ne);
  __ Csel(w1, w19, -1, ne);
  __ Csel(w2, w19, 0, ne);
  __ Csel(w3, w19, 1, ne);
  __ Csel(w4, w19, 2, ne);
  __ Csel(w5, w19, Operand(w19, ASR, 31), ne);
  __ Csel(w6, w19, Operand(w19, ROR, 1), ne);
  __ Csel(w7, w19, 3, eq);

  __ Csel(x8, x20, -2, ne);
  __ Csel(x9, x20, -1, ne);
  __ Csel(x10, x20, 0, ne);
  __ Csel(x11, x20, 1, ne);
  __ Csel(x12, x20, 2, ne);
  __ Csel(x13, x20, Operand(x20, ASR, 63), ne);
  __ Csel(x14, x20, Operand(x20, ROR, 1), ne);
  __ Csel(x15, x20, 3, eq);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(-2, w0);
    ASSERT_EQUAL_32(-1, w1);
    ASSERT_EQUAL_32(0, w2);
    ASSERT_EQUAL_32(1, w3);
    ASSERT_EQUAL_32(2, w4);
    ASSERT_EQUAL_32(-1, w5);
    ASSERT_EQUAL_32(0x40000000, w6);
    ASSERT_EQUAL_32(0x80000000, w7);

    ASSERT_EQUAL_64(-2, x8);
    ASSERT_EQUAL_64(-1, x9);
    ASSERT_EQUAL_64(0, x10);
    ASSERT_EQUAL_64(1, x11);
    ASSERT_EQUAL_64(2, x12);
    ASSERT_EQUAL_64(-1, x13);
    ASSERT_EQUAL_64(0x4000000000000000, x14);
    ASSERT_EQUAL_64(0x8000000000000000, x15);
  }
}


TEST(lslv) {
  SETUP();

  uint64_t value = 0x0123456789abcdef;
  int shift[] = {1, 3, 5, 9, 17, 33};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  // The MacroAssembler does not allow zr as an argument.
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ lslv(x0, x0, xzr);
  }

  __ Lsl(x16, x0, x1);
  __ Lsl(x17, x0, x2);
  __ Lsl(x18, x0, x3);
  __ Lsl(x19, x0, x4);
  __ Lsl(x20, x0, x5);
  __ Lsl(x21, x0, x6);

  __ Lsl(w22, w0, w1);
  __ Lsl(w23, w0, w2);
  __ Lsl(w24, w0, w3);
  __ Lsl(w25, w0, w4);
  __ Lsl(w26, w0, w5);
  __ Lsl(w27, w0, w6);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(value, x0);
    ASSERT_EQUAL_64(value << (shift[0] & 63), x16);
    ASSERT_EQUAL_64(value << (shift[1] & 63), x17);
    ASSERT_EQUAL_64(value << (shift[2] & 63), x18);
    ASSERT_EQUAL_64(value << (shift[3] & 63), x19);
    ASSERT_EQUAL_64(value << (shift[4] & 63), x20);
    ASSERT_EQUAL_64(value << (shift[5] & 63), x21);
    ASSERT_EQUAL_32(value << (shift[0] & 31), w22);
    ASSERT_EQUAL_32(value << (shift[1] & 31), w23);
    ASSERT_EQUAL_32(value << (shift[2] & 31), w24);
    ASSERT_EQUAL_32(value << (shift[3] & 31), w25);
    ASSERT_EQUAL_32(value << (shift[4] & 31), w26);
    ASSERT_EQUAL_32(value << (shift[5] & 31), w27);
  }
}


TEST(lsrv) {
  SETUP();

  uint64_t value = 0x0123456789abcdef;
  int shift[] = {1, 3, 5, 9, 17, 33};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  // The MacroAssembler does not allow zr as an argument.
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ lsrv(x0, x0, xzr);
  }

  __ Lsr(x16, x0, x1);
  __ Lsr(x17, x0, x2);
  __ Lsr(x18, x0, x3);
  __ Lsr(x19, x0, x4);
  __ Lsr(x20, x0, x5);
  __ Lsr(x21, x0, x6);

  __ Lsr(w22, w0, w1);
  __ Lsr(w23, w0, w2);
  __ Lsr(w24, w0, w3);
  __ Lsr(w25, w0, w4);
  __ Lsr(w26, w0, w5);
  __ Lsr(w27, w0, w6);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(value, x0);
    ASSERT_EQUAL_64(value >> (shift[0] & 63), x16);
    ASSERT_EQUAL_64(value >> (shift[1] & 63), x17);
    ASSERT_EQUAL_64(value >> (shift[2] & 63), x18);
    ASSERT_EQUAL_64(value >> (shift[3] & 63), x19);
    ASSERT_EQUAL_64(value >> (shift[4] & 63), x20);
    ASSERT_EQUAL_64(value >> (shift[5] & 63), x21);

    value &= 0xffffffff;
    ASSERT_EQUAL_32(value >> (shift[0] & 31), w22);
    ASSERT_EQUAL_32(value >> (shift[1] & 31), w23);
    ASSERT_EQUAL_32(value >> (shift[2] & 31), w24);
    ASSERT_EQUAL_32(value >> (shift[3] & 31), w25);
    ASSERT_EQUAL_32(value >> (shift[4] & 31), w26);
    ASSERT_EQUAL_32(value >> (shift[5] & 31), w27);
  }
}


TEST(asrv) {
  SETUP();

  int64_t value = 0xfedcba98fedcba98;
  int shift[] = {1, 3, 5, 9, 17, 33};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  // The MacroAssembler does not allow zr as an argument.
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ asrv(x0, x0, xzr);
  }

  __ Asr(x16, x0, x1);
  __ Asr(x17, x0, x2);
  __ Asr(x18, x0, x3);
  __ Asr(x19, x0, x4);
  __ Asr(x20, x0, x5);
  __ Asr(x21, x0, x6);

  __ Asr(w22, w0, w1);
  __ Asr(w23, w0, w2);
  __ Asr(w24, w0, w3);
  __ Asr(w25, w0, w4);
  __ Asr(w26, w0, w5);
  __ Asr(w27, w0, w6);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(value, x0);
    ASSERT_EQUAL_64(value >> (shift[0] & 63), x16);
    ASSERT_EQUAL_64(value >> (shift[1] & 63), x17);
    ASSERT_EQUAL_64(value >> (shift[2] & 63), x18);
    ASSERT_EQUAL_64(value >> (shift[3] & 63), x19);
    ASSERT_EQUAL_64(value >> (shift[4] & 63), x20);
    ASSERT_EQUAL_64(value >> (shift[5] & 63), x21);

    int32_t value32 = static_cast<int32_t>(value & 0xffffffff);
    ASSERT_EQUAL_32(value32 >> (shift[0] & 31), w22);
    ASSERT_EQUAL_32(value32 >> (shift[1] & 31), w23);
    ASSERT_EQUAL_32(value32 >> (shift[2] & 31), w24);
    ASSERT_EQUAL_32(value32 >> (shift[3] & 31), w25);
    ASSERT_EQUAL_32(value32 >> (shift[4] & 31), w26);
    ASSERT_EQUAL_32(value32 >> (shift[5] & 31), w27);
  }
}


TEST(rorv) {
  SETUP();

  uint64_t value = 0x0123456789abcdef;
  int shift[] = {4, 8, 12, 16, 24, 36};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  // The MacroAssembler does not allow zr as an argument.
  {
    ExactAssemblyScope scope(&masm, kInstructionSize);
    __ rorv(x0, x0, xzr);
  }

  __ Ror(x16, x0, x1);
  __ Ror(x17, x0, x2);
  __ Ror(x18, x0, x3);
  __ Ror(x19, x0, x4);
  __ Ror(x20, x0, x5);
  __ Ror(x21, x0, x6);

  __ Ror(w22, w0, w1);
  __ Ror(w23, w0, w2);
  __ Ror(w24, w0, w3);
  __ Ror(w25, w0, w4);
  __ Ror(w26, w0, w5);
  __ Ror(w27, w0, w6);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(value, x0);
    ASSERT_EQUAL_64(0xf0123456789abcde, x16);
    ASSERT_EQUAL_64(0xef0123456789abcd, x17);
    ASSERT_EQUAL_64(0xdef0123456789abc, x18);
    ASSERT_EQUAL_64(0xcdef0123456789ab, x19);
    ASSERT_EQUAL_64(0xabcdef0123456789, x20);
    ASSERT_EQUAL_64(0x789abcdef0123456, x21);
    ASSERT_EQUAL_32(0xf89abcde, w22);
    ASSERT_EQUAL_32(0xef89abcd, w23);
    ASSERT_EQUAL_32(0xdef89abc, w24);
    ASSERT_EQUAL_32(0xcdef89ab, w25);
    ASSERT_EQUAL_32(0xabcdef89, w26);
    ASSERT_EQUAL_32(0xf89abcde, w27);
  }
}


TEST(bfm) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdef);

  __ Mov(x10, 0x8888888888888888);
  __ Mov(x11, 0x8888888888888888);
  __ Mov(x12, 0x8888888888888888);
  __ Mov(x13, 0x8888888888888888);
  __ Mov(x14, 0xffffffffffffffff);
  __ Mov(w20, 0x88888888);
  __ Mov(w21, 0x88888888);

  __ Bfm(x10, x1, 16, 31);
  __ Bfm(x11, x1, 32, 15);

  __ Bfm(w20, w1, 16, 23);
  __ Bfm(w21, w1, 24, 15);

  // Aliases.
  __ Bfi(x12, x1, 16, 8);
  __ Bfxil(x13, x1, 16, 8);
  __ Bfc(x14, 16, 8);
  END();

  if (CAN_RUN()) {
    RUN();


    ASSERT_EQUAL_64(0x88888888888889ab, x10);
    ASSERT_EQUAL_64(0x8888cdef88888888, x11);

    ASSERT_EQUAL_32(0x888888ab, w20);
    ASSERT_EQUAL_32(0x88cdef88, w21);

    ASSERT_EQUAL_64(0x8888888888ef8888, x12);
    ASSERT_EQUAL_64(0x88888888888888ab, x13);
    ASSERT_EQUAL_64(0xffffffffff00ffff, x14);
  }
}


TEST(sbfm) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdef);
  __ Mov(x2, 0xfedcba9876543210);

  __ Sbfm(x10, x1, 16, 31);
  __ Sbfm(x11, x1, 32, 15);
  __ Sbfm(x12, x1, 32, 47);
  __ Sbfm(x13, x1, 48, 35);

  __ Sbfm(w14, w1, 16, 23);
  __ Sbfm(w15, w1, 24, 15);
  __ Sbfm(w16, w2, 16, 23);
  __ Sbfm(w17, w2, 24, 15);

  // Aliases.
  __ Asr(x18, x1, 32);
  __ Asr(x19, x2, 32);
  __ Sbfiz(x20, x1, 8, 16);
  __ Sbfiz(x21, x2, 8, 16);
  __ Sbfx(x22, x1, 8, 16);
  __ Sbfx(x23, x2, 8, 16);
  __ Sxtb(x24, w1);
  __ Sxtb(x25, x2);
  __ Sxth(x26, w1);
  __ Sxth(x27, x2);
  __ Sxtw(x28, w1);
  __ Sxtw(x29, x2);
  END();

  if (CAN_RUN()) {
    RUN();


    ASSERT_EQUAL_64(0xffffffffffff89ab, x10);
    ASSERT_EQUAL_64(0xffffcdef00000000, x11);
    ASSERT_EQUAL_64(0x0000000000004567, x12);
    ASSERT_EQUAL_64(0x000789abcdef0000, x13);

    ASSERT_EQUAL_32(0xffffffab, w14);
    ASSERT_EQUAL_32(0xffcdef00, w15);
    ASSERT_EQUAL_32(0x00000054, w16);
    ASSERT_EQUAL_32(0x00321000, w17);

    ASSERT_EQUAL_64(0x0000000001234567, x18);
    ASSERT_EQUAL_64(0xfffffffffedcba98, x19);
    ASSERT_EQUAL_64(0xffffffffffcdef00, x20);
    ASSERT_EQUAL_64(0x0000000000321000, x21);
    ASSERT_EQUAL_64(0xffffffffffffabcd, x22);
    ASSERT_EQUAL_64(0x0000000000005432, x23);
    ASSERT_EQUAL_64(0xffffffffffffffef, x24);
    ASSERT_EQUAL_64(0x0000000000000010, x25);
    ASSERT_EQUAL_64(0xffffffffffffcdef, x26);
    ASSERT_EQUAL_64(0x0000000000003210, x27);
    ASSERT_EQUAL_64(0xffffffff89abcdef, x28);
    ASSERT_EQUAL_64(0x0000000076543210, x29);
  }
}


TEST(ubfm) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdef);
  __ Mov(x2, 0xfedcba9876543210);

  __ Mov(x10, 0x8888888888888888);
  __ Mov(x11, 0x8888888888888888);

  __ Ubfm(x10, x1, 16, 31);
  __ Ubfm(x11, x1, 32, 15);
  __ Ubfm(x12, x1, 32, 47);
  __ Ubfm(x13, x1, 48, 35);

  __ Ubfm(w25, w1, 16, 23);
  __ Ubfm(w26, w1, 24, 15);
  __ Ubfm(w27, w2, 16, 23);
  __ Ubfm(w28, w2, 24, 15);

  // Aliases
  __ Lsl(x15, x1, 63);
  __ Lsl(x16, x1, 0);
  __ Lsr(x17, x1, 32);
  __ Ubfiz(x18, x1, 8, 16);
  __ Ubfx(x19, x1, 8, 16);
  __ Uxtb(x20, x1);
  __ Uxth(x21, x1);
  __ Uxtw(x22, x1);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00000000000089ab, x10);
    ASSERT_EQUAL_64(0x0000cdef00000000, x11);
    ASSERT_EQUAL_64(0x0000000000004567, x12);
    ASSERT_EQUAL_64(0x000789abcdef0000, x13);

    ASSERT_EQUAL_32(0x000000ab, w25);
    ASSERT_EQUAL_32(0x00cdef00, w26);
    ASSERT_EQUAL_32(0x00000054, w27);
    ASSERT_EQUAL_32(0x00321000, w28);

    ASSERT_EQUAL_64(0x8000000000000000, x15);
    ASSERT_EQUAL_64(0x0123456789abcdef, x16);
    ASSERT_EQUAL_64(0x0000000001234567, x17);
    ASSERT_EQUAL_64(0x0000000000cdef00, x18);
    ASSERT_EQUAL_64(0x000000000000abcd, x19);
    ASSERT_EQUAL_64(0x00000000000000ef, x20);
    ASSERT_EQUAL_64(0x000000000000cdef, x21);
    ASSERT_EQUAL_64(0x0000000089abcdef, x22);
  }
}


TEST(extr) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdef);
  __ Mov(x2, 0xfedcba9876543210);

  __ Extr(w10, w1, w2, 0);
  __ Extr(w11, w1, w2, 1);
  __ Extr(x12, x2, x1, 2);

  __ Ror(w13, w1, 0);
  __ Ror(w14, w2, 17);
  __ Ror(w15, w1, 31);
  __ Ror(x18, x2, 0);
  __ Ror(x19, x2, 1);
  __ Ror(x20, x1, 63);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x76543210, x10);
    ASSERT_EQUAL_64(0xbb2a1908, x11);
    ASSERT_EQUAL_64(0x0048d159e26af37b, x12);
    ASSERT_EQUAL_64(0x89abcdef, x13);
    ASSERT_EQUAL_64(0x19083b2a, x14);
    ASSERT_EQUAL_64(0x13579bdf, x15);
    ASSERT_EQUAL_64(0xfedcba9876543210, x18);
    ASSERT_EQUAL_64(0x7f6e5d4c3b2a1908, x19);
    ASSERT_EQUAL_64(0x02468acf13579bde, x20);
  }
}


TEST(system_mrs) {
  SETUP();

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 1);
  __ Mov(w2, 0x80000000);

  // Set the Z and C flags.
  __ Cmp(w0, w0);
  __ Mrs(x3, NZCV);

  // Set the N flag.
  __ Cmp(w0, w1);
  __ Mrs(x4, NZCV);

  // Set the Z, C and V flags.
  __ Adds(w0, w2, w2);
  __ Mrs(x5, NZCV);

  // Read the default FPCR.
  __ Mrs(x6, FPCR);
  END();

  if (CAN_RUN()) {
    RUN();

    // NZCV
    ASSERT_EQUAL_32(ZCFlag, w3);
    ASSERT_EQUAL_32(NFlag, w4);
    ASSERT_EQUAL_32(ZCVFlag, w5);

    // FPCR
    // The default FPCR on Linux-based platforms is 0.
    ASSERT_EQUAL_32(0, w6);
  }
}

TEST(system_rng) {
  SETUP_WITH_FEATURES(CPUFeatures::kRNG);

  START();
  // Random number.
  __ Mrs(x1, RNDR);
  // Assume that each generation is successful now.
  // TODO: Return failure occasionally.
  __ Mrs(x2, NZCV);
  __ Mrs(x3, RNDR);
  __ Mrs(x4, NZCV);

  // Reseeded random number.
  __ Mrs(x5, RNDRRS);
  // Assume that each generation is successful now.
  // TODO: Return failure occasionally.
  __ Mrs(x6, NZCV);
  __ Mrs(x7, RNDRRS);
  __ Mrs(x8, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();
    // Random number generation series.
    // Check random numbers have been generated and aren't equal when reseed has
    // happened.
    // NOTE: With a different architectural implementation, there may be a
    // collison.
    // TODO: Return failure occasionally. Set ZFlag and return UNKNOWN value.
    ASSERT_NOT_EQUAL_64(x1, x3);
    ASSERT_EQUAL_64(NoFlag, x2);
    ASSERT_EQUAL_64(NoFlag, x4);
    ASSERT_NOT_EQUAL_64(x5, x7);
    ASSERT_EQUAL_64(NoFlag, x6);
    ASSERT_EQUAL_64(NoFlag, x8);
  }
}

TEST(cfinv) {
  SETUP_WITH_FEATURES(CPUFeatures::kFlagM);

  START();
  __ Mov(w0, 1);

  // Set the C flag.
  __ Cmp(w0, 0);
  __ Mrs(x1, NZCV);

  // Invert the C flag.
  __ Cfinv();
  __ Mrs(x2, NZCV);

  // Invert the C flag again.
  __ Cfinv();
  __ Mrs(x3, NZCV);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(CFlag, w1);
    ASSERT_EQUAL_32(NoFlag, w2);
    ASSERT_EQUAL_32(CFlag, w3);
  }
}


TEST(axflag_xaflag) {
  // The AXFLAG and XAFLAG instructions are designed for converting the FP
  // conditional flags from Arm format to an alternate format efficiently.
  // There are only 4 cases which are relevant for this conversion but we test
  // the behaviour for all 16 cases anyway. The 4 important cases are labelled
  // below.
  StatusFlags expected_x[16] = {NoFlag,
                                ZFlag,
                                CFlag,  // Greater than
                                ZFlag,  // Unordered
                                ZFlag,
                                ZFlag,
                                ZCFlag,  // Equal to
                                ZFlag,
                                NoFlag,  // Less than
                                ZFlag,
                                CFlag,
                                ZFlag,
                                ZFlag,
                                ZFlag,
                                ZCFlag,
                                ZFlag};
  StatusFlags expected_a[16] = {NFlag,  // Less than
                                NFlag,
                                CFlag,  // Greater than
                                CFlag,
                                CVFlag,  // Unordered
                                CVFlag,
                                ZCFlag,  // Equal to
                                ZCFlag,
                                NFlag,
                                NFlag,
                                CFlag,
                                CFlag,
                                CVFlag,
                                CVFlag,
                                ZCFlag,
                                ZCFlag};

  for (unsigned i = 0; i < 16; i++) {
    SETUP_WITH_FEATURES(CPUFeatures::kAXFlag);

    START();
    __ Mov(x0, i << Flags_offset);
    __ Msr(NZCV, x0);
    __ Axflag();
    __ Mrs(x1, NZCV);
    __ Msr(NZCV, x0);
    __ Xaflag();
    __ Mrs(x2, NZCV);
    END();

    if (CAN_RUN()) {
      RUN();
      ASSERT_EQUAL_32(expected_x[i], w1);
      ASSERT_EQUAL_32(expected_a[i], w2);
    }
  }
}


TEST(system_msr) {
  // All FPCR fields that must be implemented: AHP, DN, FZ, RMode
  const uint64_t fpcr_core = (0b1 << 26) |  // AHP
                             (0b1 << 25) |  // DN
                             (0b1 << 24) |  // FZ
                             (0b11 << 22);  // RMode

  SETUP();

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 0x7fffffff);

  __ Mov(x7, 0);

  __ Mov(x10, NVFlag);
  __ Cmp(w0, w0);     // Set Z and C.
  __ Msr(NZCV, x10);  // Set N and V.
  // The Msr should have overwritten every flag set by the Cmp.
  __ Cinc(x7, x7, mi);  // N
  __ Cinc(x7, x7, ne);  // !Z
  __ Cinc(x7, x7, lo);  // !C
  __ Cinc(x7, x7, vs);  // V

  __ Mov(x10, ZCFlag);
  __ Cmn(w1, w1);     // Set N and V.
  __ Msr(NZCV, x10);  // Set Z and C.
  // The Msr should have overwritten every flag set by the Cmn.
  __ Cinc(x7, x7, pl);  // !N
  __ Cinc(x7, x7, eq);  // Z
  __ Cinc(x7, x7, hs);  // C
  __ Cinc(x7, x7, vc);  // !V

  Register old_fpcr = x15;
  __ Mrs(old_fpcr, FPCR);

  // All core FPCR fields must be writable.
  __ Mov(x8, fpcr_core);
  __ Msr(FPCR, x8);
  __ Mrs(x8, FPCR);

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  // All FPCR fields that aren't `RES0`:
  const uint64_t fpcr_all = fpcr_core | (0b11 << 20) |  // Stride
                            (0b1 << 19) |               // FZ16
                            (0b111 << 16) |             // Len
                            (0b1 << 15) |               // IDE
                            (0b1 << 12) |               // IXE
                            (0b1 << 11) |               // UFE
                            (0b1 << 10) |               // OFE
                            (0b1 << 9) |                // DZE
                            (0b1 << 8);                 // IOE

  // All FPCR fields, including optional ones. This part of the test doesn't
  // achieve much other than ensuring that supported fields can be cleared by
  // the next test.
  __ Mov(x9, fpcr_all);
  __ Msr(FPCR, x9);
  __ Mrs(x9, FPCR);
  __ And(x9, x9, fpcr_core);

  // The undefined bits must ignore writes.
  // It's conceivable that a future version of the architecture could use these
  // fields (making this test fail), but in the meantime this is a useful test
  // for the simulator.
  __ Mov(x10, ~fpcr_all);
  __ Msr(FPCR, x10);
  __ Mrs(x10, FPCR);
#endif

  __ Msr(FPCR, old_fpcr);

  END();

  if (CAN_RUN()) {
    RUN();

    // We should have incremented x7 (from 0) exactly 8 times.
    ASSERT_EQUAL_64(8, x7);

    ASSERT_EQUAL_64(fpcr_core, x8);

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(fpcr_core, x9);
    ASSERT_EQUAL_64(0, x10);
#endif
  }
}


TEST(system_pauth_a) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);
  START();

  // Exclude x16 and x17 from the scratch register list so we can use
  // Pac/Autia1716 safely.
  UseScratchRegisterScope temps(&masm);
  temps.Exclude(x16, x17);
  temps.Include(x10, x11);

  Register pointer = x21;
  Register retry_limit = x22;
  Label retry;

  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);

  // Back up stack pointer.
  __ Mov(x20, sp);

  // Modifiers
  __ Mov(x16, 0x477d469dec0b8760);
  __ Mov(sp, 0x477d469dec0b8760);

  __ Bind(&retry);

  // Generate PACs using the 3 system instructions.
  __ Mov(x17, pointer);
  __ Pacia1716();
  __ Mov(x0, x17);

  __ Mov(lr, pointer);
  __ Paciaz();
  __ Mov(x1, lr);

  __ Mov(lr, pointer);
  __ Paciasp();
  __ Mov(x2, lr);

  // Authenticate the pointers above.
  __ Mov(x17, x0);
  __ Autia1716();
  __ Mov(x3, x17);

  __ Mov(lr, x1);
  __ Autiaz();
  __ Mov(x4, lr);

  __ Mov(lr, x2);
  __ Autiasp();
  __ Mov(x5, lr);

  // Attempt to authenticate incorrect pointers.
  __ Mov(x17, x1);
  __ Autia1716();
  __ Mov(x6, x17);

  __ Mov(lr, x0);
  __ Autiaz();
  __ Mov(x7, lr);

  __ Mov(lr, x1);
  __ Autiasp();
  __ Mov(x8, lr);

  // Strip the pac code from the pointer in x0.
  __ Mov(lr, x0);
  __ Xpaclri();
  __ Mov(x9, lr);

  // Retry on collisions.
  __ Cmp(x0, x1);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x2, ZFlag, ne);
  __ Ccmp(pointer, x6, ZFlag, ne);
  __ Ccmp(pointer, x7, ZFlag, ne);
  __ Ccmp(pointer, x8, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  // Restore stack pointer.
  __ Mov(sp, x20);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(pointer, x2);
    ASSERT_NOT_EQUAL_64(x0, x1);
    ASSERT_EQUAL_64(x0, x2);

    // Pointers correctly authenticated.
    ASSERT_EQUAL_64(pointer, x3);
    ASSERT_EQUAL_64(pointer, x4);
    ASSERT_EQUAL_64(pointer, x5);

    // Pointers corrupted after failing to authenticate.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(0x0020000012345678, x6);
    ASSERT_EQUAL_64(0x0020000012345678, x7);
    ASSERT_EQUAL_64(0x0020000012345678, x8);
#else
    ASSERT_NOT_EQUAL_64(pointer, x6);
    ASSERT_NOT_EQUAL_64(pointer, x7);
    ASSERT_NOT_EQUAL_64(pointer, x8);
#endif

    // Pointer with code stripped.
    ASSERT_EQUAL_64(pointer, x9);
  }
}


TEST(system_pauth_b) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);
  START();

  // Exclude x16 and x17 from the scratch register list so we can use
  // Pac/Autia1716 safely.
  UseScratchRegisterScope temps(&masm);
  temps.Exclude(x16, x17);
  temps.Include(x10, x11);

  Register pointer = x21;
  Register retry_limit = x22;
  Label retry;

  __ Mov(pointer, 0x0000000012345678);
  __ Mov(retry_limit, 0x0000000012345678 + 32);

  // Back up stack pointer.
  __ Mov(x20, sp);

  // Modifiers
  __ Mov(x16, 0x477d469dec0b8760);
  __ Mov(sp, 0x477d469dec0b8760);

  __ Bind(&retry);

  // Generate PACs using the 3 system instructions.
  __ Mov(x17, 0x0000000012345678);
  __ Pacib1716();
  __ Mov(x0, x17);

  __ Mov(lr, 0x0000000012345678);
  __ Pacibz();
  __ Mov(x1, lr);

  __ Mov(lr, 0x0000000012345678);
  __ Pacibsp();
  __ Mov(x2, lr);

  // Authenticate the pointers above.
  __ Mov(x17, x0);
  __ Autib1716();
  __ Mov(x3, x17);

  __ Mov(lr, x1);
  __ Autibz();
  __ Mov(x4, lr);

  __ Mov(lr, x2);
  __ Autibsp();
  __ Mov(x5, lr);

  // Attempt to authenticate incorrect pointers.
  __ Mov(x17, x1);
  __ Autib1716();
  __ Mov(x6, x17);

  __ Mov(lr, x0);
  __ Autibz();
  __ Mov(x7, lr);

  __ Mov(lr, x1);
  __ Autibsp();
  __ Mov(x8, lr);

  // Strip the pac code from the pointer in x0.
  __ Mov(lr, x0);
  __ Xpaclri();
  __ Mov(x9, lr);

  // Retry on collisions.
  __ Cmp(x0, x1);
  __ Ccmp(pointer, x0, ZFlag, ne);
  __ Ccmp(pointer, x1, ZFlag, ne);
  __ Ccmp(pointer, x2, ZFlag, ne);
  __ Ccmp(pointer, x6, ZFlag, ne);
  __ Ccmp(pointer, x7, ZFlag, ne);
  __ Ccmp(pointer, x8, ZFlag, ne);
  __ Ccmp(pointer, retry_limit, ZFlag, eq);
  __ Cinc(pointer, pointer, ne);
  __ B(ne, &retry);

  // Restore stack pointer.
  __ Mov(sp, x20);

  END();

  if (CAN_RUN()) {
    RUN();

    // Check PAC codes have been generated and aren't equal.
    // NOTE: with a different ComputePAC implementation, there may be a
    // collision.
    ASSERT_NOT_EQUAL_64(pointer, x0);
    ASSERT_NOT_EQUAL_64(pointer, x1);
    ASSERT_NOT_EQUAL_64(pointer, x2);
    ASSERT_NOT_EQUAL_64(x0, x1);
    ASSERT_EQUAL_64(x0, x2);

    // Pointers correctly authenticated.
    ASSERT_EQUAL_64(pointer, x3);
    ASSERT_EQUAL_64(pointer, x4);
    ASSERT_EQUAL_64(pointer, x5);

    // Pointers corrupted after failing to authenticate.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    ASSERT_EQUAL_64(0x0040000012345678, x6);
    ASSERT_EQUAL_64(0x0040000012345678, x7);
    ASSERT_EQUAL_64(0x0040000012345678, x8);
#else
    ASSERT_NOT_EQUAL_64(pointer, x6);
    ASSERT_NOT_EQUAL_64(pointer, x7);
    ASSERT_NOT_EQUAL_64(pointer, x8);
#endif

    // Pointer with code stripped.
    ASSERT_EQUAL_64(pointer, x9);
  }
}

#ifdef VIXL_NEGATIVE_TESTING
TEST(system_pauth_negative_test) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);
  START();

  // Test for an assert (independent of order).
  MUST_FAIL_WITH_MESSAGE(__ Pacia1716(),
                         "Assertion failed "
                         "(!GetScratchRegisterList()->IncludesAliasOf(");

  // Test for x16 assert.
  {
    UseScratchRegisterScope temps(&masm);
    temps.Exclude(x17);
    temps.Include(x16);
    MUST_FAIL_WITH_MESSAGE(__ Pacia1716(),
                           "Assertion failed "
                           "(!GetScratchRegisterList()->IncludesAliasOf(x16))");
  }

  // Test for x17 assert.
  {
    UseScratchRegisterScope temps(&masm);
    temps.Exclude(x16);
    temps.Include(x17);
    MUST_FAIL_WITH_MESSAGE(__ Pacia1716(),
                           "Assertion failed "
                           "(!GetScratchRegisterList()->IncludesAliasOf(x17))");
  }

  // Repeat first test for other 1716 instructions.
  MUST_FAIL_WITH_MESSAGE(__ Pacib1716(),
                         "Assertion failed "
                         "(!GetScratchRegisterList()->IncludesAliasOf(");
  MUST_FAIL_WITH_MESSAGE(__ Autia1716(),
                         "Assertion failed "
                         "(!GetScratchRegisterList()->IncludesAliasOf(");
  MUST_FAIL_WITH_MESSAGE(__ Autib1716(),
                         "Assertion failed "
                         "(!GetScratchRegisterList()->IncludesAliasOf(");

  END();
}
#endif  // VIXL_NEGATIVE_TESTING


TEST(system) {
  // RegisterDump::Dump uses NEON.
  SETUP_WITH_FEATURES(CPUFeatures::kNEON, CPUFeatures::kRAS);
  RegisterDump before;

  START();
  before.Dump(&masm);
  __ Nop();
  __ Esb();
  __ Csdb();
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_REGISTERS(before);
    ASSERT_EQUAL_NZCV(before.flags_nzcv());
  }
}

static void BtiHelper(Register ipreg) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  Label jump_target, jump_call_target, call_target, done;
  START();
  UseScratchRegisterScope temps(&masm);
  temps.Exclude(ipreg);
  __ Adr(x0, &jump_target);
  __ Br(x0);
  __ Nop();
  __ Bind(&jump_target, EmitBTI_j);
  __ Adr(x0, &call_target);
  __ Blr(x0);
  __ Adr(ipreg, &jump_call_target);
  __ Blr(ipreg);
  __ Mov(lr, 0);  // Zero lr so we branch to done.
  __ Br(ipreg);
  __ Bind(&call_target, EmitBTI_c);
  __ Ret();
  __ Bind(&jump_call_target, EmitBTI_jc);
  __ Cbz(lr, &done);
  __ Ret();
  __ Bind(&done);
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
#endif
    // On hardware, we'll run the test anyway, but mark it as SKIPPED until
    // we've implemented a mechanism for marking Guarded pages.

    RUN();

#ifndef VIXL_INCLUDE_SIMULATOR_AARCH64
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

TEST(bti) {
  BtiHelper(x16);
  BtiHelper(x17);
}

TEST(unguarded_bti_is_nop) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  Label start, none, c, j, jc;
  Label jump_to_c, call_to_j;
  START();
  __ B(&start);
  __ Bind(&none, EmitBTI);
  __ Bind(&c, EmitBTI_c);
  __ Bind(&j, EmitBTI_j);
  __ Bind(&jc, EmitBTI_jc);
  __ Hint(BTI);
  __ Hint(BTI_c);
  __ Hint(BTI_j);
  __ Hint(BTI_jc);
  VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&none) == 8 * kInstructionSize);
  __ Cmp(x1, 1);
  __ B(lt, &jump_to_c);
  __ B(eq, &call_to_j);
  __ Ret();

  __ Bind(&start);
  __ Adr(x0, &none);
  __ Mov(x1, 0);
  __ Br(x0);

  __ Bind(&jump_to_c);
  __ Adr(x0, &c);
  __ Mov(x1, 1);
  __ Br(x0);

  __ Bind(&call_to_j);
  __ Adr(x0, &j);
  __ Mov(x1, 2);
  __ Blr(x0);
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(false);
#endif
    RUN();
  }
}

#ifdef VIXL_NEGATIVE_TESTING
TEST(bti_jump_to_ip_unidentified) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  START();
  UseScratchRegisterScope temps(&masm);
  temps.Exclude(x17);
  Label l;
  __ Adr(x17, &l);
  __ Br(x17);
  __ Nop();
  __ Bind(&l);
  __ Nop();
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
    MUST_FAIL_WITH_MESSAGE(RUN(),
                           "Executing non-BTI instruction with wrong "
                           "BType.");
#else
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

TEST(bti_jump_to_unidentified) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  START();
  Label l;
  __ Adr(x0, &l);
  __ Br(x0);
  __ Nop();
  __ Bind(&l);
  __ Nop();
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
    MUST_FAIL_WITH_MESSAGE(RUN(),
                           "Executing non-BTI instruction with wrong "
                           "BType.");
#else
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

TEST(bti_call_to_unidentified) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  START();
  Label l;
  __ Adr(x0, &l);
  __ Blr(x0);
  __ Nop();
  __ Bind(&l);
  __ Nop();
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
    MUST_FAIL_WITH_MESSAGE(RUN(),
                           "Executing non-BTI instruction with wrong "
                           "BType.");
#else
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

TEST(bti_jump_to_c) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  START();
  // Jumping to a "BTI c" target must fail.
  Label jump_target;
  __ Adr(x0, &jump_target);
  __ Br(x0);
  __ Nop();
  __ Bind(&jump_target, EmitBTI_c);
  __ Nop();
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
    MUST_FAIL_WITH_MESSAGE(RUN(), "Executing BTI c with wrong BType.");
#else
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

TEST(bti_call_to_j) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI);

  START();
  // Calling a "BTI j" target must fail.
  Label call_target;
  __ Adr(x0, &call_target);
  __ Blr(x0);
  __ Nop();
  __ Bind(&call_target, EmitBTI_j);
  __ Nop();
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
    MUST_FAIL_WITH_MESSAGE(RUN(), "Executing BTI j with wrong BType.");
#else
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}
#endif  // VIXL_NEGATIVE_TESTING

TEST(fall_through_bti) {
  SETUP_WITH_FEATURES(CPUFeatures::kBTI, CPUFeatures::kPAuth);

  START();
  Label target, target_j, target_c, target_jc;
  __ Mov(x0, 0);  // 'Normal' instruction sets BTYPE to zero.
  __ Bind(&target, EmitBTI);
  __ Add(x0, x0, 1);
  __ Bind(&target_j, EmitBTI_j);
  __ Add(x0, x0, 1);
  __ Bind(&target_c, EmitBTI_c);
  __ Add(x0, x0, 1);
  __ Bind(&target_jc, EmitBTI_jc);
  __ Add(x0, x0, 1);
  __ Paciasp();
  END();

  if (CAN_RUN()) {
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
    simulator.SetGuardedPages(true);
#endif
    // On hardware, we'll run the test anyway, but mark it as SKIPPED until
    // we've implemented a mechanism for marking Guarded pages.

    RUN();

    ASSERT_EQUAL_64(4, x0);

#ifndef VIXL_INCLUDE_SIMULATOR_AARCH64
    printf("SKIPPED: marking guarded pages is unimplemented on hardware");
#endif
  }
}

TEST(zero_dest) {
  // RegisterDump::Dump uses NEON.
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);
  RegisterDump before;

  START();
  // Preserve the stack pointer, in case we clobber it.
  __ Mov(x30, sp);
  // Initialize the other registers used in this test.
  uint64_t literal_base = 0x0100001000100101;
  __ Mov(x0, 0);
  __ Mov(x1, literal_base);
  for (unsigned i = 2; i < x30.GetCode(); i++) {
    __ Add(XRegister(i), XRegister(i - 1), x1);
  }
  before.Dump(&masm);

  // All of these instructions should be NOPs in these forms, but have
  // alternate forms which can write into the stack pointer.
  {
    ExactAssemblyScope scope(&masm, 3 * 7 * kInstructionSize);
    __ add(xzr, x0, x1);
    __ add(xzr, x1, xzr);
    __ add(xzr, xzr, x1);

    __ and_(xzr, x0, x2);
    __ and_(xzr, x2, xzr);
    __ and_(xzr, xzr, x2);

    __ bic(xzr, x0, x3);
    __ bic(xzr, x3, xzr);
    __ bic(xzr, xzr, x3);

    __ eon(xzr, x0, x4);
    __ eon(xzr, x4, xzr);
    __ eon(xzr, xzr, x4);

    __ eor(xzr, x0, x5);
    __ eor(xzr, x5, xzr);
    __ eor(xzr, xzr, x5);

    __ orr(xzr, x0, x6);
    __ orr(xzr, x6, xzr);
    __ orr(xzr, xzr, x6);

    __ sub(xzr, x0, x7);
    __ sub(xzr, x7, xzr);
    __ sub(xzr, xzr, x7);
  }

  // Swap the saved stack pointer with the real one. If sp was written
  // during the test, it will show up in x30. This is done because the test
  // framework assumes that sp will be valid at the end of the test.
  __ Mov(x29, x30);
  __ Mov(x30, sp);
  __ Mov(sp, x29);
  // We used x29 as a scratch register, so reset it to make sure it doesn't
  // trigger a test failure.
  __ Add(x29, x28, x1);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_REGISTERS(before);
    ASSERT_EQUAL_NZCV(before.flags_nzcv());
  }
}


TEST(zero_dest_setflags) {
  // RegisterDump::Dump uses NEON.
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);
  RegisterDump before;

  START();
  // Preserve the stack pointer, in case we clobber it.
  __ Mov(x30, sp);
  // Initialize the other registers used in this test.
  uint64_t literal_base = 0x0100001000100101;
  __ Mov(x0, 0);
  __ Mov(x1, literal_base);
  for (int i = 2; i < 30; i++) {
    __ Add(XRegister(i), XRegister(i - 1), x1);
  }
  before.Dump(&masm);

  // All of these instructions should only write to the flags in these forms,
  // but have alternate forms which can write into the stack pointer.
  {
    ExactAssemblyScope scope(&masm, 6 * kInstructionSize);
    __ adds(xzr, x0, Operand(x1, UXTX));
    __ adds(xzr, x1, Operand(xzr, UXTX));
    __ adds(xzr, x1, 1234);
    __ adds(xzr, x0, x1);
    __ adds(xzr, x1, xzr);
    __ adds(xzr, xzr, x1);
  }

  {
    ExactAssemblyScope scope(&masm, 5 * kInstructionSize);
    __ ands(xzr, x2, ~0xf);
    __ ands(xzr, xzr, ~0xf);
    __ ands(xzr, x0, x2);
    __ ands(xzr, x2, xzr);
    __ ands(xzr, xzr, x2);
  }

  {
    ExactAssemblyScope scope(&masm, 5 * kInstructionSize);
    __ bics(xzr, x3, ~0xf);
    __ bics(xzr, xzr, ~0xf);
    __ bics(xzr, x0, x3);
    __ bics(xzr, x3, xzr);
    __ bics(xzr, xzr, x3);
  }

  {
    ExactAssemblyScope scope(&masm, 6 * kInstructionSize);
    __ subs(xzr, x0, Operand(x3, UXTX));
    __ subs(xzr, x3, Operand(xzr, UXTX));
    __ subs(xzr, x3, 1234);
    __ subs(xzr, x0, x3);
    __ subs(xzr, x3, xzr);
    __ subs(xzr, xzr, x3);
  }

  // Swap the saved stack pointer with the real one. If sp was written
  // during the test, it will show up in x30. This is done because the test
  // framework assumes that sp will be valid at the end of the test.
  __ Mov(x29, x30);
  __ Mov(x30, sp);
  __ Mov(sp, x29);
  // We used x29 as a scratch register, so reset it to make sure it doesn't
  // trigger a test failure.
  __ Add(x29, x28, x1);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_REGISTERS(before);
  }
}


TEST(stack_pointer_override) {
  // This test generates some stack maintenance code, but the test only checks
  // the reported state.
  SETUP();
  START();

  // The default stack pointer in VIXL is sp.
  VIXL_CHECK(sp.Is(__ StackPointer()));
  __ SetStackPointer(x0);
  VIXL_CHECK(x0.Is(__ StackPointer()));
  __ SetStackPointer(x28);
  VIXL_CHECK(x28.Is(__ StackPointer()));
  __ SetStackPointer(sp);
  VIXL_CHECK(sp.Is(__ StackPointer()));

  END();
  if (CAN_RUN()) {
    RUN();
  }
}


TEST(peek_poke_simple) {
  SETUP();
  START();

  static const RegList x0_to_x3 =
      x0.GetBit() | x1.GetBit() | x2.GetBit() | x3.GetBit();
  static const RegList x10_to_x13 =
      x10.GetBit() | x11.GetBit() | x12.GetBit() | x13.GetBit();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101;

  // Initialize the registers.
  __ Mov(x0, literal_base);
  __ Add(x1, x0, x0);
  __ Add(x2, x1, x0);
  __ Add(x3, x2, x0);

  __ Claim(32);

  // Simple exchange.
  //  After this test:
  //    x0-x3 should be unchanged.
  //    w10-w13 should contain the lower words of x0-x3.
  __ Poke(x0, 0);
  __ Poke(x1, 8);
  __ Poke(x2, 16);
  __ Poke(x3, 24);
  Clobber(&masm, x0_to_x3);
  __ Peek(x0, 0);
  __ Peek(x1, 8);
  __ Peek(x2, 16);
  __ Peek(x3, 24);

  __ Poke(w0, 0);
  __ Poke(w1, 4);
  __ Poke(w2, 8);
  __ Poke(w3, 12);
  Clobber(&masm, x10_to_x13);
  __ Peek(w10, 0);
  __ Peek(w11, 4);
  __ Peek(w12, 8);
  __ Peek(w13, 12);

  __ Drop(32);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(literal_base * 1, x0);
    ASSERT_EQUAL_64(literal_base * 2, x1);
    ASSERT_EQUAL_64(literal_base * 3, x2);
    ASSERT_EQUAL_64(literal_base * 4, x3);

    ASSERT_EQUAL_64((literal_base * 1) & 0xffffffff, x10);
    ASSERT_EQUAL_64((literal_base * 2) & 0xffffffff, x11);
    ASSERT_EQUAL_64((literal_base * 3) & 0xffffffff, x12);
    ASSERT_EQUAL_64((literal_base * 4) & 0xffffffff, x13);
  }
}


TEST(peek_poke_unaligned) {
  SETUP();
  START();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101;

  // Initialize the registers.
  __ Mov(x0, literal_base);
  __ Add(x1, x0, x0);
  __ Add(x2, x1, x0);
  __ Add(x3, x2, x0);
  __ Add(x4, x3, x0);
  __ Add(x5, x4, x0);
  __ Add(x6, x5, x0);

  __ Claim(32);

  // Unaligned exchanges.
  //  After this test:
  //    x0-x6 should be unchanged.
  //    w10-w12 should contain the lower words of x0-x2.
  __ Poke(x0, 1);
  Clobber(&masm, x0.GetBit());
  __ Peek(x0, 1);
  __ Poke(x1, 2);
  Clobber(&masm, x1.GetBit());
  __ Peek(x1, 2);
  __ Poke(x2, 3);
  Clobber(&masm, x2.GetBit());
  __ Peek(x2, 3);
  __ Poke(x3, 4);
  Clobber(&masm, x3.GetBit());
  __ Peek(x3, 4);
  __ Poke(x4, 5);
  Clobber(&masm, x4.GetBit());
  __ Peek(x4, 5);
  __ Poke(x5, 6);
  Clobber(&masm, x5.GetBit());
  __ Peek(x5, 6);
  __ Poke(x6, 7);
  Clobber(&masm, x6.GetBit());
  __ Peek(x6, 7);

  __ Poke(w0, 1);
  Clobber(&masm, w10.GetBit());
  __ Peek(w10, 1);
  __ Poke(w1, 2);
  Clobber(&masm, w11.GetBit());
  __ Peek(w11, 2);
  __ Poke(w2, 3);
  Clobber(&masm, w12.GetBit());
  __ Peek(w12, 3);

  __ Drop(32);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(literal_base * 1, x0);
    ASSERT_EQUAL_64(literal_base * 2, x1);
    ASSERT_EQUAL_64(literal_base * 3, x2);
    ASSERT_EQUAL_64(literal_base * 4, x3);
    ASSERT_EQUAL_64(literal_base * 5, x4);
    ASSERT_EQUAL_64(literal_base * 6, x5);
    ASSERT_EQUAL_64(literal_base * 7, x6);

    ASSERT_EQUAL_64((literal_base * 1) & 0xffffffff, x10);
    ASSERT_EQUAL_64((literal_base * 2) & 0xffffffff, x11);
    ASSERT_EQUAL_64((literal_base * 3) & 0xffffffff, x12);
  }
}


TEST(peek_poke_endianness) {
  SETUP();
  START();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101;

  // Initialize the registers.
  __ Mov(x0, literal_base);
  __ Add(x1, x0, x0);

  __ Claim(32);

  // Endianness tests.
  //  After this section:
  //    x4 should match x0[31:0]:x0[63:32]
  //    w5 should match w1[15:0]:w1[31:16]
  __ Poke(x0, 0);
  __ Poke(x0, 8);
  __ Peek(x4, 4);

  __ Poke(w1, 0);
  __ Poke(w1, 4);
  __ Peek(w5, 2);

  __ Drop(32);

  END();
  if (CAN_RUN()) {
    RUN();

    uint64_t x0_expected = literal_base * 1;
    uint64_t x1_expected = literal_base * 2;
    uint64_t x4_expected = (x0_expected << 32) | (x0_expected >> 32);
    uint64_t x5_expected =
        ((x1_expected << 16) & 0xffff0000) | ((x1_expected >> 16) & 0x0000ffff);

    ASSERT_EQUAL_64(x0_expected, x0);
    ASSERT_EQUAL_64(x1_expected, x1);
    ASSERT_EQUAL_64(x4_expected, x4);
    ASSERT_EQUAL_64(x5_expected, x5);
  }
}


TEST(peek_poke_mixed) {
  SETUP();
  START();

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101;

  // Initialize the registers.
  __ Mov(x0, literal_base);
  __ Add(x1, x0, x0);
  __ Add(x2, x1, x0);
  __ Add(x3, x2, x0);

  __ Claim(32);

  // Mix with other stack operations.
  //  After this section:
  //    x0-x3 should be unchanged.
  //    x6 should match x1[31:0]:x0[63:32]
  //    w7 should match x1[15:0]:x0[63:48]
  __ Poke(x1, 8);
  __ Poke(x0, 0);
  {
    VIXL_ASSERT(__ StackPointer().Is(sp));
    __ Mov(x4, __ StackPointer());
    __ SetStackPointer(x4);

    __ Poke(wzr, 0);  // Clobber the space we're about to drop.
    __ Drop(4);
    __ Peek(x6, 0);
    __ Claim(8);
    __ Peek(w7, 10);
    __ Poke(x3, 28);
    __ Poke(xzr, 0);  // Clobber the space we're about to drop.
    __ Drop(8);
    __ Poke(x2, 12);
    __ Push(w0);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  __ Pop(x0, x1, x2, x3);

  END();
  if (CAN_RUN()) {
    RUN();

    uint64_t x0_expected = literal_base * 1;
    uint64_t x1_expected = literal_base * 2;
    uint64_t x2_expected = literal_base * 3;
    uint64_t x3_expected = literal_base * 4;
    uint64_t x6_expected = (x1_expected << 32) | (x0_expected >> 32);
    uint64_t x7_expected =
        ((x1_expected << 16) & 0xffff0000) | ((x0_expected >> 48) & 0x0000ffff);

    ASSERT_EQUAL_64(x0_expected, x0);
    ASSERT_EQUAL_64(x1_expected, x1);
    ASSERT_EQUAL_64(x2_expected, x2);
    ASSERT_EQUAL_64(x3_expected, x3);
    ASSERT_EQUAL_64(x6_expected, x6);
    ASSERT_EQUAL_64(x7_expected, x7);
  }
}


TEST(peek_poke_reglist) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

  START();

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t base = 0x0100001000100101;

  // Initialize the registers.
  __ Mov(x1, base);
  __ Add(x2, x1, x1);
  __ Add(x3, x2, x1);
  __ Add(x4, x3, x1);

  CPURegList list_1(x1, x2, x3, x4);
  CPURegList list_2(x11, x12, x13, x14);
  int list_1_size = list_1.GetTotalSizeInBytes();

  __ Claim(2 * list_1_size);

  __ PokeCPURegList(list_1, 0);
  __ PokeXRegList(list_1.GetList(), list_1_size);
  __ PeekCPURegList(list_2, 2 * kXRegSizeInBytes);
  __ PeekXRegList(x15.GetBit(), kWRegSizeInBytes);
  __ PeekWRegList(w16.GetBit() | w17.GetBit(), 3 * kXRegSizeInBytes);

  __ Drop(2 * list_1_size);


  uint64_t base_d = 0x1010010001000010;

  // Initialize the registers.
  __ Mov(x1, base_d);
  __ Add(x2, x1, x1);
  __ Add(x3, x2, x1);
  __ Add(x4, x3, x1);
  __ Fmov(d1, x1);
  __ Fmov(d2, x2);
  __ Fmov(d3, x3);
  __ Fmov(d4, x4);

  CPURegList list_d_1(d1, d2, d3, d4);
  CPURegList list_d_2(d11, d12, d13, d14);
  int list_d_1_size = list_d_1.GetTotalSizeInBytes();

  __ Claim(2 * list_d_1_size);

  __ PokeCPURegList(list_d_1, 0);
  __ PokeDRegList(list_d_1.GetList(), list_d_1_size);
  __ PeekCPURegList(list_d_2, 2 * kDRegSizeInBytes);
  __ PeekDRegList(d15.GetBit(), kSRegSizeInBytes);
  __ PeekSRegList(s16.GetBit() | s17.GetBit(), 3 * kDRegSizeInBytes);

  __ Drop(2 * list_d_1_size);


  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(3 * base, x11);
    ASSERT_EQUAL_64(4 * base, x12);
    ASSERT_EQUAL_64(1 * base, x13);
    ASSERT_EQUAL_64(2 * base, x14);
    ASSERT_EQUAL_64(((1 * base) >> kWRegSize) | ((2 * base) << kWRegSize), x15);
    ASSERT_EQUAL_64(2 * base, x14);
    ASSERT_EQUAL_32((4 * base) & kWRegMask, w16);
    ASSERT_EQUAL_32((4 * base) >> kWRegSize, w17);

    ASSERT_EQUAL_FP64(RawbitsToDouble(3 * base_d), d11);
    ASSERT_EQUAL_FP64(RawbitsToDouble(4 * base_d), d12);
    ASSERT_EQUAL_FP64(RawbitsToDouble(1 * base_d), d13);
    ASSERT_EQUAL_FP64(RawbitsToDouble(2 * base_d), d14);
    ASSERT_EQUAL_FP64(RawbitsToDouble((base_d >> kSRegSize) |
                                      ((2 * base_d) << kSRegSize)),
                      d15);
    ASSERT_EQUAL_FP64(RawbitsToDouble(2 * base_d), d14);
    ASSERT_EQUAL_FP32(RawbitsToFloat((4 * base_d) & kSRegMask), s16);
    ASSERT_EQUAL_FP32(RawbitsToFloat((4 * base_d) >> kSRegSize), s17);
  }
}


TEST(load_store_reglist) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

  START();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t high_base = UINT32_C(0x01000010);
  uint64_t low_base = UINT32_C(0x00100101);
  uint64_t base = (high_base << 32) | low_base;
  uint64_t array[21];
  memset(array, 0, sizeof(array));

  // Initialize the registers.
  __ Mov(x1, base);
  __ Add(x2, x1, x1);
  __ Add(x3, x2, x1);
  __ Add(x4, x3, x1);
  __ Fmov(d1, x1);
  __ Fmov(d2, x2);
  __ Fmov(d3, x3);
  __ Fmov(d4, x4);
  __ Fmov(d5, x1);
  __ Fmov(d6, x2);
  __ Fmov(d7, x3);
  __ Fmov(d8, x4);

  Register reg_base = x20;
  Register reg_index = x21;
  int size_stored = 0;

  __ Mov(reg_base, reinterpret_cast<uintptr_t>(&array));

  // Test aligned accesses.
  CPURegList list_src(w1, w2, w3, w4);
  CPURegList list_dst(w11, w12, w13, w14);
  CPURegList list_fp_src_1(d1, d2, d3, d4);
  CPURegList list_fp_dst_1(d11, d12, d13, d14);

  __ StoreCPURegList(list_src, MemOperand(reg_base, 0 * sizeof(uint64_t)));
  __ LoadCPURegList(list_dst, MemOperand(reg_base, 0 * sizeof(uint64_t)));
  size_stored += 4 * kWRegSizeInBytes;

  __ Mov(reg_index, size_stored);
  __ StoreCPURegList(list_src, MemOperand(reg_base, reg_index));
  __ LoadCPURegList(list_dst, MemOperand(reg_base, reg_index));
  size_stored += 4 * kWRegSizeInBytes;

  __ StoreCPURegList(list_fp_src_1, MemOperand(reg_base, size_stored));
  __ LoadCPURegList(list_fp_dst_1, MemOperand(reg_base, size_stored));
  size_stored += 4 * kDRegSizeInBytes;

  __ Mov(reg_index, size_stored);
  __ StoreCPURegList(list_fp_src_1, MemOperand(reg_base, reg_index));
  __ LoadCPURegList(list_fp_dst_1, MemOperand(reg_base, reg_index));
  size_stored += 4 * kDRegSizeInBytes;

  // Test unaligned accesses.
  CPURegList list_fp_src_2(d5, d6, d7, d8);
  CPURegList list_fp_dst_2(d15, d16, d17, d18);

  __ Str(wzr, MemOperand(reg_base, size_stored));
  size_stored += 1 * kWRegSizeInBytes;
  __ StoreCPURegList(list_fp_src_2, MemOperand(reg_base, size_stored));
  __ LoadCPURegList(list_fp_dst_2, MemOperand(reg_base, size_stored));
  size_stored += 4 * kDRegSizeInBytes;

  __ Mov(reg_index, size_stored);
  __ StoreCPURegList(list_fp_src_2, MemOperand(reg_base, reg_index));
  __ LoadCPURegList(list_fp_dst_2, MemOperand(reg_base, reg_index));

  END();
  if (CAN_RUN()) {
    RUN();

    VIXL_CHECK(array[0] == (1 * low_base) + (2 * low_base << kWRegSize));
    VIXL_CHECK(array[1] == (3 * low_base) + (4 * low_base << kWRegSize));
    VIXL_CHECK(array[2] == (1 * low_base) + (2 * low_base << kWRegSize));
    VIXL_CHECK(array[3] == (3 * low_base) + (4 * low_base << kWRegSize));
    VIXL_CHECK(array[4] == 1 * base);
    VIXL_CHECK(array[5] == 2 * base);
    VIXL_CHECK(array[6] == 3 * base);
    VIXL_CHECK(array[7] == 4 * base);
    VIXL_CHECK(array[8] == 1 * base);
    VIXL_CHECK(array[9] == 2 * base);
    VIXL_CHECK(array[10] == 3 * base);
    VIXL_CHECK(array[11] == 4 * base);
    VIXL_CHECK(array[12] == ((1 * low_base) << kSRegSize));
    VIXL_CHECK(array[13] == (((2 * low_base) << kSRegSize) | (1 * high_base)));
    VIXL_CHECK(array[14] == (((3 * low_base) << kSRegSize) | (2 * high_base)));
    VIXL_CHECK(array[15] == (((4 * low_base) << kSRegSize) | (3 * high_base)));
    VIXL_CHECK(array[16] == (((1 * low_base) << kSRegSize) | (4 * high_base)));
    VIXL_CHECK(array[17] == (((2 * low_base) << kSRegSize) | (1 * high_base)));
    VIXL_CHECK(array[18] == (((3 * low_base) << kSRegSize) | (2 * high_base)));
    VIXL_CHECK(array[19] == (((4 * low_base) << kSRegSize) | (3 * high_base)));
    VIXL_CHECK(array[20] == (4 * high_base));

    ASSERT_EQUAL_64(1 * low_base, x11);
    ASSERT_EQUAL_64(2 * low_base, x12);
    ASSERT_EQUAL_64(3 * low_base, x13);
    ASSERT_EQUAL_64(4 * low_base, x14);
    ASSERT_EQUAL_FP64(RawbitsToDouble(1 * base), d11);
    ASSERT_EQUAL_FP64(RawbitsToDouble(2 * base), d12);
    ASSERT_EQUAL_FP64(RawbitsToDouble(3 * base), d13);
    ASSERT_EQUAL_FP64(RawbitsToDouble(4 * base), d14);
    ASSERT_EQUAL_FP64(RawbitsToDouble(1 * base), d15);
    ASSERT_EQUAL_FP64(RawbitsToDouble(2 * base), d16);
    ASSERT_EQUAL_FP64(RawbitsToDouble(3 * base), d17);
    ASSERT_EQUAL_FP64(RawbitsToDouble(4 * base), d18);
  }
}


// This enum is used only as an argument to the push-pop test helpers.
enum PushPopMethod {
  // Push or Pop using the Push and Pop methods, with blocks of up to four
  // registers. (Smaller blocks will be used if necessary.)
  PushPopByFour,

  // Use Push<Size>RegList and Pop<Size>RegList to transfer the registers.
  PushPopRegList
};


// For the PushPop* tests, use the maximum number of registers that the test
// supports (where a reg_count argument would otherwise be provided).
static int const kPushPopUseMaxRegCount = -1;

// Test a simple push-pop pattern:
//  * Claim <claim> bytes to set the stack alignment.
//  * Push <reg_count> registers with size <reg_size>.
//  * Clobber the register contents.
//  * Pop <reg_count> registers to restore the original contents.
//  * Drop <claim> bytes to restore the original stack pointer.
//
// Different push and pop methods can be specified independently to test for
// proper word-endian behaviour.
static void PushPopSimpleHelper(int reg_count,
                                int claim,
                                int reg_size,
                                PushPopMethod push_method,
                                PushPopMethod pop_method) {
  SETUP();

  START();

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x20;
  const RegList allowed = ~stack_pointer.GetBit();
  if (reg_count == kPushPopUseMaxRegCount) {
    reg_count = CountSetBits(allowed, kNumberOfRegisters);
  }
  // Work out which registers to use, based on reg_size.
  Register r[kNumberOfRegisters];
  Register x[kNumberOfRegisters];
  RegList list =
      PopulateRegisterArray(NULL, x, r, reg_size, reg_count, allowed);

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101;

  {
    VIXL_ASSERT(__ StackPointer().Is(sp));
    __ Mov(stack_pointer, __ StackPointer());
    __ SetStackPointer(stack_pointer);

    int i;

    // Initialize the registers.
    for (i = 0; i < reg_count; i++) {
      // Always write into the X register, to ensure that the upper word is
      // properly ignored by Push when testing W registers.
      __ Mov(x[i], literal_base * i);
    }

    // Claim memory first, as requested.
    __ Claim(claim);

    switch (push_method) {
      case PushPopByFour:
        // Push high-numbered registers first (to the highest addresses).
        for (i = reg_count; i >= 4; i -= 4) {
          __ Push(r[i - 1], r[i - 2], r[i - 3], r[i - 4]);
        }
        // Finish off the leftovers.
        switch (i) {
          case 3:
            __ Push(r[2], r[1], r[0]);
            break;
          case 2:
            __ Push(r[1], r[0]);
            break;
          case 1:
            __ Push(r[0]);
            break;
          default:
            VIXL_ASSERT(i == 0);
            break;
        }
        break;
      case PushPopRegList:
        __ PushSizeRegList(list, reg_size);
        break;
    }

    // Clobber all the registers, to ensure that they get repopulated by Pop.
    Clobber(&masm, list);

    switch (pop_method) {
      case PushPopByFour:
        // Pop low-numbered registers first (from the lowest addresses).
        for (i = 0; i <= (reg_count - 4); i += 4) {
          __ Pop(r[i], r[i + 1], r[i + 2], r[i + 3]);
        }
        // Finish off the leftovers.
        switch (reg_count - i) {
          case 3:
            __ Pop(r[i], r[i + 1], r[i + 2]);
            break;
          case 2:
            __ Pop(r[i], r[i + 1]);
            break;
          case 1:
            __ Pop(r[i]);
            break;
          default:
            VIXL_ASSERT(i == reg_count);
            break;
        }
        break;
      case PushPopRegList:
        __ PopSizeRegList(list, reg_size);
        break;
    }

    // Drop memory to restore stack_pointer.
    __ Drop(claim);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  END();

  if (CAN_RUN()) {
    RUN();

    // Check that the register contents were preserved.
    // Always use ASSERT_EQUAL_64, even when testing W registers, so we can test
    // that the upper word was properly cleared by Pop.
    literal_base &= (0xffffffffffffffff >> (64 - reg_size));
    for (int i = 0; i < reg_count; i++) {
      if (x[i].Is(xzr)) {
        ASSERT_EQUAL_64(0, x[i]);
      } else {
        ASSERT_EQUAL_64(literal_base * i, x[i]);
      }
    }
  }
}


TEST(push_pop_xreg_simple_32) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopSimpleHelper(count,
                          claim,
                          kWRegSize,
                          PushPopByFour,
                          PushPopByFour);
      PushPopSimpleHelper(count,
                          claim,
                          kWRegSize,
                          PushPopByFour,
                          PushPopRegList);
      PushPopSimpleHelper(count,
                          claim,
                          kWRegSize,
                          PushPopRegList,
                          PushPopByFour);
      PushPopSimpleHelper(count,
                          claim,
                          kWRegSize,
                          PushPopRegList,
                          PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kWRegSize,
                        PushPopByFour,
                        PushPopByFour);
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kWRegSize,
                        PushPopByFour,
                        PushPopRegList);
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kWRegSize,
                        PushPopRegList,
                        PushPopByFour);
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kWRegSize,
                        PushPopRegList,
                        PushPopRegList);
  }
}


TEST(push_pop_xreg_simple_64) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopSimpleHelper(count,
                          claim,
                          kXRegSize,
                          PushPopByFour,
                          PushPopByFour);
      PushPopSimpleHelper(count,
                          claim,
                          kXRegSize,
                          PushPopByFour,
                          PushPopRegList);
      PushPopSimpleHelper(count,
                          claim,
                          kXRegSize,
                          PushPopRegList,
                          PushPopByFour);
      PushPopSimpleHelper(count,
                          claim,
                          kXRegSize,
                          PushPopRegList,
                          PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kXRegSize,
                        PushPopByFour,
                        PushPopByFour);
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kXRegSize,
                        PushPopByFour,
                        PushPopRegList);
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kXRegSize,
                        PushPopRegList,
                        PushPopByFour);
    PushPopSimpleHelper(kPushPopUseMaxRegCount,
                        claim,
                        kXRegSize,
                        PushPopRegList,
                        PushPopRegList);
  }
}

// For the PushPopFP* tests, use the maximum number of registers that the test
// supports (where a reg_count argument would otherwise be provided).
static int const kPushPopFPUseMaxRegCount = -1;

// Test a simple push-pop pattern:
//  * Claim <claim> bytes to set the stack alignment.
//  * Push <reg_count> FP registers with size <reg_size>.
//  * Clobber the register contents.
//  * Pop <reg_count> FP registers to restore the original contents.
//  * Drop <claim> bytes to restore the original stack pointer.
//
// Different push and pop methods can be specified independently to test for
// proper word-endian behaviour.
static void PushPopFPSimpleHelper(int reg_count,
                                  int claim,
                                  int reg_size,
                                  PushPopMethod push_method,
                                  PushPopMethod pop_method) {
  SETUP_WITH_FEATURES((reg_count == 0) ? CPUFeatures::kNone : CPUFeatures::kFP);

  START();

  // We can use any floating-point register. None of them are reserved for
  // debug code, for example.
  static RegList const allowed = ~0;
  if (reg_count == kPushPopFPUseMaxRegCount) {
    reg_count = CountSetBits(allowed, kNumberOfVRegisters);
  }
  // Work out which registers to use, based on reg_size.
  VRegister v[kNumberOfRegisters];
  VRegister d[kNumberOfRegisters];
  RegList list =
      PopulateVRegisterArray(NULL, d, v, reg_size, reg_count, allowed);

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x10;

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied (using an integer) by small values (such as a register
  //    index), this value is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  //  * It is never a floating-point NaN, and will therefore always compare
  //    equal to itself.
  uint64_t literal_base = 0x0100001000100101;

  {
    VIXL_ASSERT(__ StackPointer().Is(sp));
    __ Mov(stack_pointer, __ StackPointer());
    __ SetStackPointer(stack_pointer);

    int i;

    // Initialize the registers, using X registers to load the literal.
    __ Mov(x0, 0);
    __ Mov(x1, literal_base);
    for (i = 0; i < reg_count; i++) {
      // Always write into the D register, to ensure that the upper word is
      // properly ignored by Push when testing S registers.
      __ Fmov(d[i], x0);
      // Calculate the next literal.
      __ Add(x0, x0, x1);
    }

    // Claim memory first, as requested.
    __ Claim(claim);

    switch (push_method) {
      case PushPopByFour:
        // Push high-numbered registers first (to the highest addresses).
        for (i = reg_count; i >= 4; i -= 4) {
          __ Push(v[i - 1], v[i - 2], v[i - 3], v[i - 4]);
        }
        // Finish off the leftovers.
        switch (i) {
          case 3:
            __ Push(v[2], v[1], v[0]);
            break;
          case 2:
            __ Push(v[1], v[0]);
            break;
          case 1:
            __ Push(v[0]);
            break;
          default:
            VIXL_ASSERT(i == 0);
            break;
        }
        break;
      case PushPopRegList:
        __ PushSizeRegList(list, reg_size, CPURegister::kVRegister);
        break;
    }

    // Clobber all the registers, to ensure that they get repopulated by Pop.
    ClobberFP(&masm, list);

    switch (pop_method) {
      case PushPopByFour:
        // Pop low-numbered registers first (from the lowest addresses).
        for (i = 0; i <= (reg_count - 4); i += 4) {
          __ Pop(v[i], v[i + 1], v[i + 2], v[i + 3]);
        }
        // Finish off the leftovers.
        switch (reg_count - i) {
          case 3:
            __ Pop(v[i], v[i + 1], v[i + 2]);
            break;
          case 2:
            __ Pop(v[i], v[i + 1]);
            break;
          case 1:
            __ Pop(v[i]);
            break;
          default:
            VIXL_ASSERT(i == reg_count);
            break;
        }
        break;
      case PushPopRegList:
        __ PopSizeRegList(list, reg_size, CPURegister::kVRegister);
        break;
    }

    // Drop memory to restore the stack pointer.
    __ Drop(claim);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  END();

  if (CAN_RUN()) {
    RUN();

    // Check that the register contents were preserved.
    // Always use ASSERT_EQUAL_FP64, even when testing S registers, so we can
    // test that the upper word was properly cleared by Pop.
    literal_base &= (0xffffffffffffffff >> (64 - reg_size));
    for (int i = 0; i < reg_count; i++) {
      uint64_t literal = literal_base * i;
      double expected;
      memcpy(&expected, &literal, sizeof(expected));
      ASSERT_EQUAL_FP64(expected, d[i]);
    }
  }
}


TEST(push_pop_fp_xreg_simple_32) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopFPSimpleHelper(count,
                            claim,
                            kSRegSize,
                            PushPopByFour,
                            PushPopByFour);
      PushPopFPSimpleHelper(count,
                            claim,
                            kSRegSize,
                            PushPopByFour,
                            PushPopRegList);
      PushPopFPSimpleHelper(count,
                            claim,
                            kSRegSize,
                            PushPopRegList,
                            PushPopByFour);
      PushPopFPSimpleHelper(count,
                            claim,
                            kSRegSize,
                            PushPopRegList,
                            PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kSRegSize,
                          PushPopByFour,
                          PushPopByFour);
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kSRegSize,
                          PushPopByFour,
                          PushPopRegList);
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kSRegSize,
                          PushPopRegList,
                          PushPopByFour);
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kSRegSize,
                          PushPopRegList,
                          PushPopRegList);
  }
}


TEST(push_pop_fp_xreg_simple_64) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopFPSimpleHelper(count,
                            claim,
                            kDRegSize,
                            PushPopByFour,
                            PushPopByFour);
      PushPopFPSimpleHelper(count,
                            claim,
                            kDRegSize,
                            PushPopByFour,
                            PushPopRegList);
      PushPopFPSimpleHelper(count,
                            claim,
                            kDRegSize,
                            PushPopRegList,
                            PushPopByFour);
      PushPopFPSimpleHelper(count,
                            claim,
                            kDRegSize,
                            PushPopRegList,
                            PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kDRegSize,
                          PushPopByFour,
                          PushPopByFour);
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kDRegSize,
                          PushPopByFour,
                          PushPopRegList);
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kDRegSize,
                          PushPopRegList,
                          PushPopByFour);
    PushPopFPSimpleHelper(kPushPopFPUseMaxRegCount,
                          claim,
                          kDRegSize,
                          PushPopRegList,
                          PushPopRegList);
  }
}


// Push and pop data using an overlapping combination of Push/Pop and
// RegList-based methods.
static void PushPopMixedMethodsHelper(int claim, int reg_size) {
  SETUP();

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x5;
  const RegList allowed = ~stack_pointer.GetBit();
  // Work out which registers to use, based on reg_size.
  Register r[10];
  Register x[10];
  PopulateRegisterArray(NULL, x, r, reg_size, 10, allowed);

  // Calculate some handy register lists.
  RegList r0_to_r3 = 0;
  for (int i = 0; i <= 3; i++) {
    r0_to_r3 |= x[i].GetBit();
  }
  RegList r4_to_r5 = 0;
  for (int i = 4; i <= 5; i++) {
    r4_to_r5 |= x[i].GetBit();
  }
  RegList r6_to_r9 = 0;
  for (int i = 6; i <= 9; i++) {
    r6_to_r9 |= x[i].GetBit();
  }

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101;

  START();
  {
    VIXL_ASSERT(__ StackPointer().Is(sp));
    __ Mov(stack_pointer, __ StackPointer());
    __ SetStackPointer(stack_pointer);

    // Claim memory first, as requested.
    __ Claim(claim);

    __ Mov(x[3], literal_base * 3);
    __ Mov(x[2], literal_base * 2);
    __ Mov(x[1], literal_base * 1);
    __ Mov(x[0], literal_base * 0);

    __ PushSizeRegList(r0_to_r3, reg_size);
    __ Push(r[3], r[2]);

    Clobber(&masm, r0_to_r3);
    __ PopSizeRegList(r0_to_r3, reg_size);

    __ Push(r[2], r[1], r[3], r[0]);

    Clobber(&masm, r4_to_r5);
    __ Pop(r[4], r[5]);
    Clobber(&masm, r6_to_r9);
    __ Pop(r[6], r[7], r[8], r[9]);

    // Drop memory to restore stack_pointer.
    __ Drop(claim);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  END();

  if (CAN_RUN()) {
    RUN();

    // Always use ASSERT_EQUAL_64, even when testing W registers, so we can test
    // that the upper word was properly cleared by Pop.
    literal_base &= (0xffffffffffffffff >> (64 - reg_size));

    ASSERT_EQUAL_64(literal_base * 3, x[9]);
    ASSERT_EQUAL_64(literal_base * 2, x[8]);
    ASSERT_EQUAL_64(literal_base * 0, x[7]);
    ASSERT_EQUAL_64(literal_base * 3, x[6]);
    ASSERT_EQUAL_64(literal_base * 1, x[5]);
    ASSERT_EQUAL_64(literal_base * 2, x[4]);
  }
}


TEST(push_pop_xreg_mixed_methods_64) {
  for (int claim = 0; claim <= 8; claim++) {
    PushPopMixedMethodsHelper(claim, kXRegSize);
  }
}


TEST(push_pop_xreg_mixed_methods_32) {
  for (int claim = 0; claim <= 8; claim++) {
    PushPopMixedMethodsHelper(claim, kWRegSize);
  }
}


// Push and pop data using overlapping X- and W-sized quantities.
static void PushPopWXOverlapHelper(int reg_count, int claim) {
  SETUP();

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x10;
  const RegList allowed = ~stack_pointer.GetBit();
  if (reg_count == kPushPopUseMaxRegCount) {
    reg_count = CountSetBits(allowed, kNumberOfRegisters);
  }
  // Work out which registers to use, based on reg_size.
  Register w[kNumberOfRegisters];
  Register x[kNumberOfRegisters];
  RegList list = PopulateRegisterArray(w, x, NULL, 0, reg_count, allowed);

  // The number of W-sized slots we expect to pop. When we pop, we alternate
  // between W and X registers, so we need reg_count*1.5 W-sized slots.
  int const requested_w_slots = reg_count + reg_count / 2;

  // Track what _should_ be on the stack, using W-sized slots.
  static int const kMaxWSlots = kNumberOfRegisters + kNumberOfRegisters / 2;
  uint32_t stack[kMaxWSlots];
  for (int i = 0; i < kMaxWSlots; i++) {
    stack[i] = 0xdeadbeef;
  }

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  static uint64_t const literal_base = 0x0100001000100101;
  static uint64_t const literal_base_hi = literal_base >> 32;
  static uint64_t const literal_base_lo = literal_base & 0xffffffff;
  static uint64_t const literal_base_w = literal_base & 0xffffffff;

  START();
  {
    VIXL_ASSERT(__ StackPointer().Is(sp));
    __ Mov(stack_pointer, __ StackPointer());
    __ SetStackPointer(stack_pointer);

    // Initialize the registers.
    for (int i = 0; i < reg_count; i++) {
      // Always write into the X register, to ensure that the upper word is
      // properly ignored by Push when testing W registers.
      __ Mov(x[i], literal_base * i);
    }

    // Claim memory first, as requested.
    __ Claim(claim);

    // The push-pop pattern is as follows:
    // Push:           Pop:
    //  x[0](hi)   ->   w[0]
    //  x[0](lo)   ->   x[1](hi)
    //  w[1]       ->   x[1](lo)
    //  w[1]       ->   w[2]
    //  x[2](hi)   ->   x[2](hi)
    //  x[2](lo)   ->   x[2](lo)
    //  x[2](hi)   ->   w[3]
    //  x[2](lo)   ->   x[4](hi)
    //  x[2](hi)   ->   x[4](lo)
    //  x[2](lo)   ->   w[5]
    //  w[3]       ->   x[5](hi)
    //  w[3]       ->   x[6](lo)
    //  w[3]       ->   w[7]
    //  w[3]       ->   x[8](hi)
    //  x[4](hi)   ->   x[8](lo)
    //  x[4](lo)   ->   w[9]
    // ... pattern continues ...
    //
    // That is, registers are pushed starting with the lower numbers,
    // alternating between x and w registers, and pushing i%4+1 copies of each,
    // where i is the register number.
    // Registers are popped starting with the higher numbers one-by-one,
    // alternating between x and w registers, but only popping one at a time.
    //
    // This pattern provides a wide variety of alignment effects and overlaps.

    // ---- Push ----

    int active_w_slots = 0;
    for (int i = 0; active_w_slots < requested_w_slots; i++) {
      VIXL_ASSERT(i < reg_count);
      // In order to test various arguments to PushMultipleTimes, and to try to
      // exercise different alignment and overlap effects, we push each
      // register a different number of times.
      int times = i % 4 + 1;
      if (i & 1) {
        // Push odd-numbered registers as W registers.
        __ PushMultipleTimes(times, w[i]);
        // Fill in the expected stack slots.
        for (int j = 0; j < times; j++) {
          if (w[i].Is(wzr)) {
            // The zero register always writes zeroes.
            stack[active_w_slots++] = 0;
          } else {
            stack[active_w_slots++] = literal_base_w * i;
          }
        }
      } else {
        // Push even-numbered registers as X registers.
        __ PushMultipleTimes(times, x[i]);
        // Fill in the expected stack slots.
        for (int j = 0; j < times; j++) {
          if (x[i].Is(xzr)) {
            // The zero register always writes zeroes.
            stack[active_w_slots++] = 0;
            stack[active_w_slots++] = 0;
          } else {
            stack[active_w_slots++] = literal_base_hi * i;
            stack[active_w_slots++] = literal_base_lo * i;
          }
        }
      }
    }
    // Because we were pushing several registers at a time, we probably pushed
    // more than we needed to.
    if (active_w_slots > requested_w_slots) {
      __ Drop((active_w_slots - requested_w_slots) * kWRegSizeInBytes);
      // Bump the number of active W-sized slots back to where it should be,
      // and fill the empty space with a placeholder value.
      do {
        stack[active_w_slots--] = 0xdeadbeef;
      } while (active_w_slots > requested_w_slots);
    }

    // ---- Pop ----

    Clobber(&masm, list);

    // If popping an even number of registers, the first one will be X-sized.
    // Otherwise, the first one will be W-sized.
    bool next_is_64 = !(reg_count & 1);
    for (int i = reg_count - 1; i >= 0; i--) {
      if (next_is_64) {
        __ Pop(x[i]);
        active_w_slots -= 2;
      } else {
        __ Pop(w[i]);
        active_w_slots -= 1;
      }
      next_is_64 = !next_is_64;
    }
    VIXL_ASSERT(active_w_slots == 0);

    // Drop memory to restore stack_pointer.
    __ Drop(claim);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  END();

  if (CAN_RUN()) {
    RUN();

    int slot = 0;
    for (int i = 0; i < reg_count; i++) {
      // Even-numbered registers were written as W registers.
      // Odd-numbered registers were written as X registers.
      bool expect_64 = (i & 1);
      uint64_t expected;

      if (expect_64) {
        uint64_t hi = stack[slot++];
        uint64_t lo = stack[slot++];
        expected = (hi << 32) | lo;
      } else {
        expected = stack[slot++];
      }

      // Always use ASSERT_EQUAL_64, even when testing W registers, so we can
      // test that the upper word was properly cleared by Pop.
      if (x[i].Is(xzr)) {
        ASSERT_EQUAL_64(0, x[i]);
      } else {
        ASSERT_EQUAL_64(expected, x[i]);
      }
    }
    VIXL_ASSERT(slot == requested_w_slots);
  }
}


TEST(push_pop_xreg_wx_overlap) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 1; count <= 8; count++) {
      PushPopWXOverlapHelper(count, claim);
    }
    // Test with the maximum number of registers.
    PushPopWXOverlapHelper(kPushPopUseMaxRegCount, claim);
  }
}


TEST(push_pop_sp) {
  SETUP();

  START();

  VIXL_ASSERT(sp.Is(__ StackPointer()));

  // Acquire all temps from the MacroAssembler. They are used arbitrarily below.
  UseScratchRegisterScope temps(&masm);
  temps.ExcludeAll();

  __ Mov(x3, 0x3333333333333333);
  __ Mov(x2, 0x2222222222222222);
  __ Mov(x1, 0x1111111111111111);
  __ Mov(x0, 0x0000000000000000);
  __ Claim(2 * kXRegSizeInBytes);
  __ PushXRegList(x0.GetBit() | x1.GetBit() | x2.GetBit() | x3.GetBit());
  __ Push(x3, x2);
  __ PopXRegList(x0.GetBit() | x1.GetBit() | x2.GetBit() | x3.GetBit());
  __ Push(x2, x1, x3, x0);
  __ Pop(x4, x5);
  __ Pop(x6, x7, x8, x9);

  __ Claim(2 * kXRegSizeInBytes);
  __ PushWRegList(w0.GetBit() | w1.GetBit() | w2.GetBit() | w3.GetBit());
  __ Push(w3, w1, w2, w0);
  __ PopWRegList(w10.GetBit() | w11.GetBit() | w12.GetBit() | w13.GetBit());
  __ Pop(w14, w15, w16, w17);

  __ Claim(2 * kXRegSizeInBytes);
  __ Push(w2, w2, w1, w1);
  __ Push(x3, x3);
  __ Pop(w18, w19, w20, w21);
  __ Pop(x22, x23);

  __ Claim(2 * kXRegSizeInBytes);
  __ PushXRegList(x1.GetBit() | x22.GetBit());
  __ PopXRegList(x24.GetBit() | x26.GetBit());

  __ Claim(2 * kXRegSizeInBytes);
  __ PushWRegList(w1.GetBit() | w2.GetBit() | w4.GetBit() | w22.GetBit());
  __ PopWRegList(w25.GetBit() | w27.GetBit() | w28.GetBit() | w29.GetBit());

  __ Claim(2 * kXRegSizeInBytes);
  __ PushXRegList(0);
  __ PopXRegList(0);
  __ PushXRegList(0xffffffff);
  __ PopXRegList(0xffffffff);
  __ Drop(12 * kXRegSizeInBytes);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x1111111111111111, x3);
    ASSERT_EQUAL_64(0x0000000000000000, x2);
    ASSERT_EQUAL_64(0x3333333333333333, x1);
    ASSERT_EQUAL_64(0x2222222222222222, x0);
    ASSERT_EQUAL_64(0x3333333333333333, x9);
    ASSERT_EQUAL_64(0x2222222222222222, x8);
    ASSERT_EQUAL_64(0x0000000000000000, x7);
    ASSERT_EQUAL_64(0x3333333333333333, x6);
    ASSERT_EQUAL_64(0x1111111111111111, x5);
    ASSERT_EQUAL_64(0x2222222222222222, x4);

    ASSERT_EQUAL_32(0x11111111U, w13);
    ASSERT_EQUAL_32(0x33333333U, w12);
    ASSERT_EQUAL_32(0x00000000U, w11);
    ASSERT_EQUAL_32(0x22222222U, w10);
    ASSERT_EQUAL_32(0x11111111U, w17);
    ASSERT_EQUAL_32(0x00000000U, w16);
    ASSERT_EQUAL_32(0x33333333U, w15);
    ASSERT_EQUAL_32(0x22222222U, w14);

    ASSERT_EQUAL_32(0x11111111U, w18);
    ASSERT_EQUAL_32(0x11111111U, w19);
    ASSERT_EQUAL_32(0x11111111U, w20);
    ASSERT_EQUAL_32(0x11111111U, w21);
    ASSERT_EQUAL_64(0x3333333333333333, x22);
    ASSERT_EQUAL_64(0x0000000000000000, x23);

    ASSERT_EQUAL_64(0x3333333333333333, x24);
    ASSERT_EQUAL_64(0x3333333333333333, x26);

    ASSERT_EQUAL_32(0x33333333U, w25);
    ASSERT_EQUAL_32(0x00000000U, w27);
    ASSERT_EQUAL_32(0x22222222U, w28);
    ASSERT_EQUAL_32(0x33333333U, w29);
  }
}


TEST(printf) {
  // RegisterDump::Dump uses NEON.
  // Printf uses FP to cast FP arguments to doubles.
  SETUP_WITH_FEATURES(CPUFeatures::kNEON, CPUFeatures::kFP);

  START();

  char const* test_plain_string = "Printf with no arguments.\n";
  char const* test_substring = "'This is a substring.'";
  RegisterDump before;

  // Initialize x29 to the value of the stack pointer. We will use x29 as a
  // temporary stack pointer later, and initializing it in this way allows the
  // RegisterDump check to pass.
  __ Mov(x29, __ StackPointer());

  // Test simple integer arguments.
  __ Mov(x0, 1234);
  __ Mov(x1, 0x1234);

  // Test simple floating-point arguments.
  __ Fmov(d0, 1.234);

  // Test pointer (string) arguments.
  __ Mov(x2, reinterpret_cast<uintptr_t>(test_substring));

  // Test the maximum number of arguments, and sign extension.
  __ Mov(w3, 0xffffffff);
  __ Mov(w4, 0xffffffff);
  __ Mov(x5, 0xffffffffffffffff);
  __ Mov(x6, 0xffffffffffffffff);
  __ Fmov(s1, 1.234);
  __ Fmov(s2, 2.345);
  __ Fmov(d3, 3.456);
  __ Fmov(d4, 4.567);

  // Test printing callee-saved registers.
  __ Mov(x28, 0x123456789abcdef);
  __ Fmov(d10, 42.0);

  // Test with three arguments.
  __ Mov(x10, 3);
  __ Mov(x11, 40);
  __ Mov(x12, 500);

  // A single character.
  __ Mov(w13, 'x');

  // Check that we don't clobber any registers.
  before.Dump(&masm);

  __ Printf(test_plain_string);  // NOLINT(runtime/printf)
  __ Printf("x0: %" PRId64 ", x1: 0x%08" PRIx64 "\n", x0, x1);
  __ Printf("w5: %" PRId32 ", x5: %" PRId64 "\n", w5, x5);
  __ Printf("d0: %f\n", d0);
  __ Printf("Test %%s: %s\n", x2);
  __ Printf("w3(uint32): %" PRIu32 "\nw4(int32): %" PRId32
            "\n"
            "x5(uint64): %" PRIu64 "\nx6(int64): %" PRId64 "\n",
            w3,
            w4,
            x5,
            x6);
  __ Printf("%%f: %f\n%%g: %g\n%%e: %e\n%%E: %E\n", s1, s2, d3, d4);
  __ Printf("0x%" PRIx32 ", 0x%" PRIx64 "\n", w28, x28);
  __ Printf("%g\n", d10);
  __ Printf("%%%%%s%%%c%%\n", x2, w13);

  // Print the stack pointer (sp).
  __ Printf("StackPointer(sp): 0x%016" PRIx64 ", 0x%08" PRIx32 "\n",
            __ StackPointer(),
            __ StackPointer().W());

  // Test with a different stack pointer.
  const Register old_stack_pointer = __ StackPointer();
  __ Mov(x29, old_stack_pointer);
  __ SetStackPointer(x29);
  // Print the stack pointer (not sp).
  __ Printf("StackPointer(not sp): 0x%016" PRIx64 ", 0x%08" PRIx32 "\n",
            __ StackPointer(),
            __ StackPointer().W());
  __ Mov(old_stack_pointer, __ StackPointer());
  __ SetStackPointer(old_stack_pointer);

  // Test with three arguments.
  __ Printf("3=%u, 4=%u, 5=%u\n", x10, x11, x12);

  // Mixed argument types.
  __ Printf("w3: %" PRIu32 ", s1: %f, x5: %" PRIu64 ", d3: %f\n",
            w3,
            s1,
            x5,
            d3);
  __ Printf("s1: %f, d3: %f, w3: %" PRId32 ", x5: %" PRId64 "\n",
            s1,
            d3,
            w3,
            x5);

  END();
  if (CAN_RUN()) {
    RUN();

    // We cannot easily test the output of the Printf sequences, and because
    // Printf preserves all registers by default, we can't look at the number of
    // bytes that were printed. However, the printf_no_preserve test should
    // check
    // that, and here we just test that we didn't clobber any registers.
    ASSERT_EQUAL_REGISTERS(before);
  }
}


TEST(printf_no_preserve) {
  // PrintfNoPreserve uses FP to cast FP arguments to doubles.
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

  START();

  char const* test_plain_string = "Printf with no arguments.\n";
  char const* test_substring = "'This is a substring.'";

  __ PrintfNoPreserve(test_plain_string);
  __ Mov(x19, x0);

  // Test simple integer arguments.
  __ Mov(x0, 1234);
  __ Mov(x1, 0x1234);
  __ PrintfNoPreserve("x0: %" PRId64 ", x1: 0x%08" PRIx64 "\n", x0, x1);
  __ Mov(x20, x0);

  // Test simple floating-point arguments.
  __ Fmov(d0, 1.234);
  __ PrintfNoPreserve("d0: %f\n", d0);
  __ Mov(x21, x0);

  // Test pointer (string) arguments.
  __ Mov(x2, reinterpret_cast<uintptr_t>(test_substring));
  __ PrintfNoPreserve("Test %%s: %s\n", x2);
  __ Mov(x22, x0);

  // Test the maximum number of arguments, and sign extension.
  __ Mov(w3, 0xffffffff);
  __ Mov(w4, 0xffffffff);
  __ Mov(x5, 0xffffffffffffffff);
  __ Mov(x6, 0xffffffffffffffff);
  __ PrintfNoPreserve("w3(uint32): %" PRIu32 "\nw4(int32): %" PRId32
                      "\n"
                      "x5(uint64): %" PRIu64 "\nx6(int64): %" PRId64 "\n",
                      w3,
                      w4,
                      x5,
                      x6);
  __ Mov(x23, x0);

  __ Fmov(s1, 1.234);
  __ Fmov(s2, 2.345);
  __ Fmov(d3, 3.456);
  __ Fmov(d4, 4.567);
  __ PrintfNoPreserve("%%f: %f\n%%g: %g\n%%e: %e\n%%E: %E\n", s1, s2, d3, d4);
  __ Mov(x24, x0);

  // Test printing callee-saved registers.
  __ Mov(x28, 0x123456789abcdef);
  __ PrintfNoPreserve("0x%" PRIx32 ", 0x%" PRIx64 "\n", w28, x28);
  __ Mov(x25, x0);

  __ Fmov(d10, 42.0);
  __ PrintfNoPreserve("%g\n", d10);
  __ Mov(x26, x0);

  // Test with a different stack pointer.
  const Register old_stack_pointer = __ StackPointer();
  __ Mov(x29, old_stack_pointer);
  __ SetStackPointer(x29);
  // Print the stack pointer (not sp).
  __ PrintfNoPreserve("StackPointer(not sp): 0x%016" PRIx64 ", 0x%08" PRIx32
                      "\n",
                      __ StackPointer(),
                      __ StackPointer().W());
  __ Mov(x27, x0);
  __ Mov(old_stack_pointer, __ StackPointer());
  __ SetStackPointer(old_stack_pointer);

  // Test with three arguments.
  __ Mov(x3, 3);
  __ Mov(x4, 40);
  __ Mov(x5, 500);
  __ PrintfNoPreserve("3=%u, 4=%u, 5=%u\n", x3, x4, x5);
  __ Mov(x28, x0);

  // Mixed argument types.
  __ Mov(w3, 0xffffffff);
  __ Fmov(s1, 1.234);
  __ Mov(x5, 0xffffffffffffffff);
  __ Fmov(d3, 3.456);
  __ PrintfNoPreserve("w3: %" PRIu32 ", s1: %f, x5: %" PRIu64 ", d3: %f\n",
                      w3,
                      s1,
                      x5,
                      d3);
  __ Mov(x29, x0);

  END();
  if (CAN_RUN()) {
    RUN();

    // We cannot easily test the exact output of the Printf sequences, but we
    // can
    // use the return code to check that the string length was correct.

    // Printf with no arguments.
    ASSERT_EQUAL_64(strlen(test_plain_string), x19);
    // x0: 1234, x1: 0x00001234
    ASSERT_EQUAL_64(25, x20);
    // d0: 1.234000
    ASSERT_EQUAL_64(13, x21);
    // Test %s: 'This is a substring.'
    ASSERT_EQUAL_64(32, x22);
    // w3(uint32): 4294967295
    // w4(int32): -1
    // x5(uint64): 18446744073709551615
    // x6(int64): -1
    ASSERT_EQUAL_64(23 + 14 + 33 + 14, x23);
    // %f: 1.234000
    // %g: 2.345
    // %e: 3.456000e+00
    // %E: 4.567000E+00
    ASSERT_EQUAL_64(13 + 10 + 17 + 17, x24);
    // 0x89abcdef, 0x123456789abcdef
    ASSERT_EQUAL_64(30, x25);
    // 42
    ASSERT_EQUAL_64(3, x26);
    // StackPointer(not sp): 0x00007fb037ae2370, 0x37ae2370
    // Note: This is an example value, but the field width is fixed here so the
    // string length is still predictable.
    ASSERT_EQUAL_64(53, x27);
    // 3=3, 4=40, 5=500
    ASSERT_EQUAL_64(17, x28);
    // w3: 4294967295, s1: 1.234000, x5: 18446744073709551615, d3: 3.456000
    ASSERT_EQUAL_64(69, x29);
  }
}


TEST(trace) {
  // The Trace helper should not generate any code unless the simulator is being
  // used.
  SETUP();
  START();

  Label start;
  __ Bind(&start);
  __ Trace(LOG_ALL, TRACE_ENABLE);
  __ Trace(LOG_ALL, TRACE_DISABLE);
  if (masm.GenerateSimulatorCode()) {
    VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&start) > 0);
  } else {
    VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&start) == 0);
  }

  END();
}


TEST(log) {
  // The Log helper should not generate any code unless the simulator is being
  // used.
  SETUP();
  START();

  Label start;
  __ Bind(&start);
  __ Log(LOG_ALL);
  if (masm.GenerateSimulatorCode()) {
    VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&start) > 0);
  } else {
    VIXL_CHECK(__ GetSizeOfCodeGeneratedSince(&start) == 0);
  }

  END();
}


TEST(blr_lr) {
  // A simple test to check that the simulator correctly handle "blr lr".
  SETUP();

  START();
  Label target;
  Label end;

  __ Mov(x0, 0x0);
  __ Adr(lr, &target);

  __ Blr(lr);
  __ Mov(x0, 0xdeadbeef);
  __ B(&end);

  __ Bind(&target);
  __ Mov(x0, 0xc001c0de);

  __ Bind(&end);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xc001c0de, x0);
  }
}


TEST(barriers) {
  // Generate all supported barriers, this is just a smoke test
  SETUP();

  START();

  // DMB
  __ Dmb(FullSystem, BarrierAll);
  __ Dmb(FullSystem, BarrierReads);
  __ Dmb(FullSystem, BarrierWrites);
  __ Dmb(FullSystem, BarrierOther);

  __ Dmb(InnerShareable, BarrierAll);
  __ Dmb(InnerShareable, BarrierReads);
  __ Dmb(InnerShareable, BarrierWrites);
  __ Dmb(InnerShareable, BarrierOther);

  __ Dmb(NonShareable, BarrierAll);
  __ Dmb(NonShareable, BarrierReads);
  __ Dmb(NonShareable, BarrierWrites);
  __ Dmb(NonShareable, BarrierOther);

  __ Dmb(OuterShareable, BarrierAll);
  __ Dmb(OuterShareable, BarrierReads);
  __ Dmb(OuterShareable, BarrierWrites);
  __ Dmb(OuterShareable, BarrierOther);

  // DSB
  __ Dsb(FullSystem, BarrierAll);
  __ Dsb(FullSystem, BarrierReads);
  __ Dsb(FullSystem, BarrierWrites);
  __ Dsb(FullSystem, BarrierOther);

  __ Dsb(InnerShareable, BarrierAll);
  __ Dsb(InnerShareable, BarrierReads);
  __ Dsb(InnerShareable, BarrierWrites);
  __ Dsb(InnerShareable, BarrierOther);

  __ Dsb(NonShareable, BarrierAll);
  __ Dsb(NonShareable, BarrierReads);
  __ Dsb(NonShareable, BarrierWrites);
  __ Dsb(NonShareable, BarrierOther);

  __ Dsb(OuterShareable, BarrierAll);
  __ Dsb(OuterShareable, BarrierReads);
  __ Dsb(OuterShareable, BarrierWrites);
  __ Dsb(OuterShareable, BarrierOther);

  // ISB
  __ Isb();

  END();

  if (CAN_RUN()) {
    RUN();
  }
}


TEST(ldar_stlr) {
  // The middle value is read, modified, and written. The padding exists only to
  // check for over-write.
  uint8_t b[] = {0, 0x12, 0};
  uint16_t h[] = {0, 0x1234, 0};
  uint32_t w[] = {0, 0x12345678, 0};
  uint64_t x[] = {0, 0x123456789abcdef0, 0};

  SETUP();
  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(&b[1]));
  __ Ldarb(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stlrb(w0, MemOperand(x10));

  __ Mov(x10, reinterpret_cast<uintptr_t>(&h[1]));
  __ Ldarh(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stlrh(w0, MemOperand(x10));

  __ Mov(x10, reinterpret_cast<uintptr_t>(&w[1]));
  __ Ldar(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stlr(w0, MemOperand(x10));

  __ Mov(x10, reinterpret_cast<uintptr_t>(&x[1]));
  __ Ldar(x0, MemOperand(x10));
  __ Add(x0, x0, 1);
  __ Stlr(x0, MemOperand(x10));

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(0x13, b[1]);
    ASSERT_EQUAL_32(0x1235, h[1]);
    ASSERT_EQUAL_32(0x12345679, w[1]);
    ASSERT_EQUAL_64(0x123456789abcdef1, x[1]);

    // Check for over-write.
    ASSERT_EQUAL_32(0, b[0]);
    ASSERT_EQUAL_32(0, b[2]);
    ASSERT_EQUAL_32(0, h[0]);
    ASSERT_EQUAL_32(0, h[2]);
    ASSERT_EQUAL_32(0, w[0]);
    ASSERT_EQUAL_32(0, w[2]);
    ASSERT_EQUAL_64(0, x[0]);
    ASSERT_EQUAL_64(0, x[2]);
  }
}


TEST(ldlar_stllr) {
  // The middle value is read, modified, and written. The padding exists only to
  // check for over-write.
  uint8_t b[] = {0, 0x12, 0};
  uint16_t h[] = {0, 0x1234, 0};
  uint32_t w[] = {0, 0x12345678, 0};
  uint64_t x[] = {0, 0x123456789abcdef0, 0};

  SETUP_WITH_FEATURES(CPUFeatures::kLORegions);

  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(&b[1]));
  __ Ldlarb(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stllrb(w0, MemOperand(x10));

  __ Mov(x10, reinterpret_cast<uintptr_t>(&h[1]));
  __ Ldlarh(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stllrh(w0, MemOperand(x10));

  __ Mov(x10, reinterpret_cast<uintptr_t>(&w[1]));
  __ Ldlar(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stllr(w0, MemOperand(x10));

  __ Mov(x10, reinterpret_cast<uintptr_t>(&x[1]));
  __ Ldlar(x0, MemOperand(x10));
  __ Add(x0, x0, 1);
  __ Stllr(x0, MemOperand(x10));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(0x13, b[1]);
    ASSERT_EQUAL_32(0x1235, h[1]);
    ASSERT_EQUAL_32(0x12345679, w[1]);
    ASSERT_EQUAL_64(0x123456789abcdef1, x[1]);

    // Check for over-write.
    ASSERT_EQUAL_32(0, b[0]);
    ASSERT_EQUAL_32(0, b[2]);
    ASSERT_EQUAL_32(0, h[0]);
    ASSERT_EQUAL_32(0, h[2]);
    ASSERT_EQUAL_32(0, w[0]);
    ASSERT_EQUAL_32(0, w[2]);
    ASSERT_EQUAL_64(0, x[0]);
    ASSERT_EQUAL_64(0, x[2]);
  }
}


TEST(ldxr_stxr) {
  // The middle value is read, modified, and written. The padding exists only to
  // check for over-write.
  uint8_t b[] = {0, 0x12, 0};
  uint16_t h[] = {0, 0x1234, 0};
  uint32_t w[] = {0, 0x12345678, 0};
  uint64_t x[] = {0, 0x123456789abcdef0, 0};

  // As above, but get suitably-aligned values for ldxp and stxp.
  uint32_t wp_data[] = {0, 0, 0, 0, 0};
  uint32_t* wp = AlignUp(wp_data + 1, kWRegSizeInBytes * 2) - 1;
  wp[1] = 0x12345678;  // wp[1] is 64-bit-aligned.
  wp[2] = 0x87654321;
  uint64_t xp_data[] = {0, 0, 0, 0, 0};
  uint64_t* xp = AlignUp(xp_data + 1, kXRegSizeInBytes * 2) - 1;
  xp[1] = 0x123456789abcdef0;  // xp[1] is 128-bit-aligned.
  xp[2] = 0x0fedcba987654321;

  SETUP();
  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(&b[1]));
  Label try_b;
  __ Bind(&try_b);
  __ Ldxrb(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stxrb(w5, w0, MemOperand(x10));
  __ Cbnz(w5, &try_b);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&h[1]));
  Label try_h;
  __ Bind(&try_h);
  __ Ldxrh(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stxrh(w5, w0, MemOperand(x10));
  __ Cbnz(w5, &try_h);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&w[1]));
  Label try_w;
  __ Bind(&try_w);
  __ Ldxr(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stxr(w5, w0, MemOperand(x10));
  __ Cbnz(w5, &try_w);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&x[1]));
  Label try_x;
  __ Bind(&try_x);
  __ Ldxr(x0, MemOperand(x10));
  __ Add(x0, x0, 1);
  __ Stxr(w5, x0, MemOperand(x10));
  __ Cbnz(w5, &try_x);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&wp[1]));
  Label try_wp;
  __ Bind(&try_wp);
  __ Ldxp(w0, w1, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Add(w1, w1, 1);
  __ Stxp(w5, w0, w1, MemOperand(x10));
  __ Cbnz(w5, &try_wp);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&xp[1]));
  Label try_xp;
  __ Bind(&try_xp);
  __ Ldxp(x0, x1, MemOperand(x10));
  __ Add(x0, x0, 1);
  __ Add(x1, x1, 1);
  __ Stxp(w5, x0, x1, MemOperand(x10));
  __ Cbnz(w5, &try_xp);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(0x13, b[1]);
    ASSERT_EQUAL_32(0x1235, h[1]);
    ASSERT_EQUAL_32(0x12345679, w[1]);
    ASSERT_EQUAL_64(0x123456789abcdef1, x[1]);
    ASSERT_EQUAL_32(0x12345679, wp[1]);
    ASSERT_EQUAL_32(0x87654322, wp[2]);
    ASSERT_EQUAL_64(0x123456789abcdef1, xp[1]);
    ASSERT_EQUAL_64(0x0fedcba987654322, xp[2]);

    // Check for over-write.
    ASSERT_EQUAL_32(0, b[0]);
    ASSERT_EQUAL_32(0, b[2]);
    ASSERT_EQUAL_32(0, h[0]);
    ASSERT_EQUAL_32(0, h[2]);
    ASSERT_EQUAL_32(0, w[0]);
    ASSERT_EQUAL_32(0, w[2]);
    ASSERT_EQUAL_64(0, x[0]);
    ASSERT_EQUAL_64(0, x[2]);
    ASSERT_EQUAL_32(0, wp[0]);
    ASSERT_EQUAL_32(0, wp[3]);
    ASSERT_EQUAL_64(0, xp[0]);
    ASSERT_EQUAL_64(0, xp[3]);
  }
}


TEST(ldaxr_stlxr) {
  // The middle value is read, modified, and written. The padding exists only to
  // check for over-write.
  uint8_t b[] = {0, 0x12, 0};
  uint16_t h[] = {0, 0x1234, 0};
  uint32_t w[] = {0, 0x12345678, 0};
  uint64_t x[] = {0, 0x123456789abcdef0, 0};

  // As above, but get suitably-aligned values for ldxp and stxp.
  uint32_t wp_data[] = {0, 0, 0, 0, 0};
  uint32_t* wp = AlignUp(wp_data + 1, kWRegSizeInBytes * 2) - 1;
  wp[1] = 0x12345678;  // wp[1] is 64-bit-aligned.
  wp[2] = 0x87654321;
  uint64_t xp_data[] = {0, 0, 0, 0, 0};
  uint64_t* xp = AlignUp(xp_data + 1, kXRegSizeInBytes * 2) - 1;
  xp[1] = 0x123456789abcdef0;  // xp[1] is 128-bit-aligned.
  xp[2] = 0x0fedcba987654321;

  SETUP();
  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(&b[1]));
  Label try_b;
  __ Bind(&try_b);
  __ Ldaxrb(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stlxrb(w5, w0, MemOperand(x10));
  __ Cbnz(w5, &try_b);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&h[1]));
  Label try_h;
  __ Bind(&try_h);
  __ Ldaxrh(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stlxrh(w5, w0, MemOperand(x10));
  __ Cbnz(w5, &try_h);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&w[1]));
  Label try_w;
  __ Bind(&try_w);
  __ Ldaxr(w0, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Stlxr(w5, w0, MemOperand(x10));
  __ Cbnz(w5, &try_w);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&x[1]));
  Label try_x;
  __ Bind(&try_x);
  __ Ldaxr(x0, MemOperand(x10));
  __ Add(x0, x0, 1);
  __ Stlxr(w5, x0, MemOperand(x10));
  __ Cbnz(w5, &try_x);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&wp[1]));
  Label try_wp;
  __ Bind(&try_wp);
  __ Ldaxp(w0, w1, MemOperand(x10));
  __ Add(w0, w0, 1);
  __ Add(w1, w1, 1);
  __ Stlxp(w5, w0, w1, MemOperand(x10));
  __ Cbnz(w5, &try_wp);

  __ Mov(x10, reinterpret_cast<uintptr_t>(&xp[1]));
  Label try_xp;
  __ Bind(&try_xp);
  __ Ldaxp(x0, x1, MemOperand(x10));
  __ Add(x0, x0, 1);
  __ Add(x1, x1, 1);
  __ Stlxp(w5, x0, x1, MemOperand(x10));
  __ Cbnz(w5, &try_xp);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(0x13, b[1]);
    ASSERT_EQUAL_32(0x1235, h[1]);
    ASSERT_EQUAL_32(0x12345679, w[1]);
    ASSERT_EQUAL_64(0x123456789abcdef1, x[1]);
    ASSERT_EQUAL_32(0x12345679, wp[1]);
    ASSERT_EQUAL_32(0x87654322, wp[2]);
    ASSERT_EQUAL_64(0x123456789abcdef1, xp[1]);
    ASSERT_EQUAL_64(0x0fedcba987654322, xp[2]);

    // Check for over-write.
    ASSERT_EQUAL_32(0, b[0]);
    ASSERT_EQUAL_32(0, b[2]);
    ASSERT_EQUAL_32(0, h[0]);
    ASSERT_EQUAL_32(0, h[2]);
    ASSERT_EQUAL_32(0, w[0]);
    ASSERT_EQUAL_32(0, w[2]);
    ASSERT_EQUAL_64(0, x[0]);
    ASSERT_EQUAL_64(0, x[2]);
    ASSERT_EQUAL_32(0, wp[0]);
    ASSERT_EQUAL_32(0, wp[3]);
    ASSERT_EQUAL_64(0, xp[0]);
    ASSERT_EQUAL_64(0, xp[3]);
  }
}


TEST(clrex) {
  // This data should never be written.
  uint64_t data[] = {0, 0, 0};
  uint64_t* data_aligned = AlignUp(data, kXRegSizeInBytes * 2);

  SETUP();
  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(data_aligned));
  __ Mov(w6, 0);

  __ Ldxrb(w0, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Stxrb(w5, w0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldxrh(w0, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Stxrh(w5, w0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldxr(w0, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Stxr(w5, w0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldxr(x0, MemOperand(x10));
  __ Clrex();
  __ Add(x0, x0, 1);
  __ Stxr(w5, x0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldxp(w0, w1, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Add(w1, w1, 1);
  __ Stxp(w5, w0, w1, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldxp(x0, x1, MemOperand(x10));
  __ Clrex();
  __ Add(x0, x0, 1);
  __ Add(x1, x1, 1);
  __ Stxp(w5, x0, x1, MemOperand(x10));
  __ Add(w6, w6, w5);

  // Acquire-release variants.

  __ Ldaxrb(w0, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Stlxrb(w5, w0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldaxrh(w0, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Stlxrh(w5, w0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldaxr(w0, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Stlxr(w5, w0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldaxr(x0, MemOperand(x10));
  __ Clrex();
  __ Add(x0, x0, 1);
  __ Stlxr(w5, x0, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldaxp(w0, w1, MemOperand(x10));
  __ Clrex();
  __ Add(w0, w0, 1);
  __ Add(w1, w1, 1);
  __ Stlxp(w5, w0, w1, MemOperand(x10));
  __ Add(w6, w6, w5);

  __ Ldaxp(x0, x1, MemOperand(x10));
  __ Clrex();
  __ Add(x0, x0, 1);
  __ Add(x1, x1, 1);
  __ Stlxp(w5, x0, x1, MemOperand(x10));
  __ Add(w6, w6, w5);

  END();
  if (CAN_RUN()) {
    RUN();

    // None of the 12 store-exclusives should have succeeded.
    ASSERT_EQUAL_32(12, w6);

    ASSERT_EQUAL_64(0, data[0]);
    ASSERT_EQUAL_64(0, data[1]);
    ASSERT_EQUAL_64(0, data[2]);
  }
}


#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
// Check that the simulator occasionally makes store-exclusive fail.
TEST(ldxr_stxr_fail) {
  uint64_t data[] = {0, 0, 0};
  uint64_t* data_aligned = AlignUp(data, kXRegSizeInBytes * 2);

  // Impose a hard limit on the number of attempts, so the test cannot hang.
  static const uint64_t kWatchdog = 10000;
  Label done;

  SETUP();
  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(data_aligned));
  __ Mov(x11, kWatchdog);

  // This loop is the opposite of what we normally do with ldxr and stxr; we
  // keep trying until we fail (or the watchdog counter runs out).
  Label try_b;
  __ Bind(&try_b);
  __ Ldxrb(w0, MemOperand(x10));
  __ Stxrb(w5, w0, MemOperand(x10));
  // Check the watchdog counter.
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  // Check the exclusive-store result.
  __ Cbz(w5, &try_b);

  Label try_h;
  __ Bind(&try_h);
  __ Ldxrh(w0, MemOperand(x10));
  __ Stxrh(w5, w0, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_h);

  Label try_w;
  __ Bind(&try_w);
  __ Ldxr(w0, MemOperand(x10));
  __ Stxr(w5, w0, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_w);

  Label try_x;
  __ Bind(&try_x);
  __ Ldxr(x0, MemOperand(x10));
  __ Stxr(w5, x0, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_x);

  Label try_wp;
  __ Bind(&try_wp);
  __ Ldxp(w0, w1, MemOperand(x10));
  __ Stxp(w5, w0, w1, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_wp);

  Label try_xp;
  __ Bind(&try_xp);
  __ Ldxp(x0, x1, MemOperand(x10));
  __ Stxp(w5, x0, x1, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_xp);

  __ Bind(&done);
  // Trigger an error if x11 (watchdog) is zero.
  __ Cmp(x11, 0);
  __ Cset(x12, eq);

  END();
  if (CAN_RUN()) {
    RUN();

    // Check that the watchdog counter didn't run out.
    ASSERT_EQUAL_64(0, x12);
  }
}
#endif


#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
// Check that the simulator occasionally makes store-exclusive fail.
TEST(ldaxr_stlxr_fail) {
  uint64_t data[] = {0, 0, 0};
  uint64_t* data_aligned = AlignUp(data, kXRegSizeInBytes * 2);

  // Impose a hard limit on the number of attempts, so the test cannot hang.
  static const uint64_t kWatchdog = 10000;
  Label done;

  SETUP();
  START();

  __ Mov(x10, reinterpret_cast<uintptr_t>(data_aligned));
  __ Mov(x11, kWatchdog);

  // This loop is the opposite of what we normally do with ldxr and stxr; we
  // keep trying until we fail (or the watchdog counter runs out).
  Label try_b;
  __ Bind(&try_b);
  __ Ldxrb(w0, MemOperand(x10));
  __ Stxrb(w5, w0, MemOperand(x10));
  // Check the watchdog counter.
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  // Check the exclusive-store result.
  __ Cbz(w5, &try_b);

  Label try_h;
  __ Bind(&try_h);
  __ Ldaxrh(w0, MemOperand(x10));
  __ Stlxrh(w5, w0, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_h);

  Label try_w;
  __ Bind(&try_w);
  __ Ldaxr(w0, MemOperand(x10));
  __ Stlxr(w5, w0, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_w);

  Label try_x;
  __ Bind(&try_x);
  __ Ldaxr(x0, MemOperand(x10));
  __ Stlxr(w5, x0, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_x);

  Label try_wp;
  __ Bind(&try_wp);
  __ Ldaxp(w0, w1, MemOperand(x10));
  __ Stlxp(w5, w0, w1, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_wp);

  Label try_xp;
  __ Bind(&try_xp);
  __ Ldaxp(x0, x1, MemOperand(x10));
  __ Stlxp(w5, x0, x1, MemOperand(x10));
  __ Sub(x11, x11, 1);
  __ Cbz(x11, &done);
  __ Cbz(w5, &try_xp);

  __ Bind(&done);
  // Trigger an error if x11 (watchdog) is zero.
  __ Cmp(x11, 0);
  __ Cset(x12, eq);

  END();
  if (CAN_RUN()) {
    RUN();

    // Check that the watchdog counter didn't run out.
    ASSERT_EQUAL_64(0, x12);
  }
}
#endif

TEST(cas_casa_casl_casal_w) {
  uint64_t data1 = 0x0123456789abcdef;
  uint64_t data2 = 0x0123456789abcdef;
  uint64_t data3 = 0x0123456789abcdef;
  uint64_t data4 = 0x0123456789abcdef;
  uint64_t data5 = 0x0123456789abcdef;
  uint64_t data6 = 0x0123456789abcdef;
  uint64_t data7 = 0x0123456789abcdef;
  uint64_t data8 = 0x0123456789abcdef;

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);

  START();

  __ Mov(x21, reinterpret_cast<uintptr_t>(&data1) + 0);
  __ Mov(x22, reinterpret_cast<uintptr_t>(&data2) + 0);
  __ Mov(x23, reinterpret_cast<uintptr_t>(&data3) + 4);
  __ Mov(x24, reinterpret_cast<uintptr_t>(&data4) + 4);
  __ Mov(x25, reinterpret_cast<uintptr_t>(&data5) + 0);
  __ Mov(x26, reinterpret_cast<uintptr_t>(&data6) + 0);
  __ Mov(x27, reinterpret_cast<uintptr_t>(&data7) + 4);
  __ Mov(x28, reinterpret_cast<uintptr_t>(&data8) + 4);

  __ Mov(x0, 0xffffffff);

  __ Mov(x1, 0xfedcba9876543210);
  __ Mov(x2, 0x0123456789abcdef);
  __ Mov(x3, 0xfedcba9876543210);
  __ Mov(x4, 0x89abcdef01234567);
  __ Mov(x5, 0xfedcba9876543210);
  __ Mov(x6, 0x0123456789abcdef);
  __ Mov(x7, 0xfedcba9876543210);
  __ Mov(x8, 0x89abcdef01234567);

  __ Cas(w1, w0, MemOperand(x21));
  __ Cas(w2, w0, MemOperand(x22));
  __ Casa(w3, w0, MemOperand(x23));
  __ Casa(w4, w0, MemOperand(x24));
  __ Casl(w5, w0, MemOperand(x25));
  __ Casl(w6, w0, MemOperand(x26));
  __ Casal(w7, w0, MemOperand(x27));
  __ Casal(w8, w0, MemOperand(x28));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x89abcdef, x1);
    ASSERT_EQUAL_64(0x89abcdef, x2);
    ASSERT_EQUAL_64(0x01234567, x3);
    ASSERT_EQUAL_64(0x01234567, x4);
    ASSERT_EQUAL_64(0x89abcdef, x5);
    ASSERT_EQUAL_64(0x89abcdef, x6);
    ASSERT_EQUAL_64(0x01234567, x7);
    ASSERT_EQUAL_64(0x01234567, x8);

    ASSERT_EQUAL_64(0x0123456789abcdef, data1);
    ASSERT_EQUAL_64(0x01234567ffffffff, data2);
    ASSERT_EQUAL_64(0x0123456789abcdef, data3);
    ASSERT_EQUAL_64(0xffffffff89abcdef, data4);
    ASSERT_EQUAL_64(0x0123456789abcdef, data5);
    ASSERT_EQUAL_64(0x01234567ffffffff, data6);
    ASSERT_EQUAL_64(0x0123456789abcdef, data7);
    ASSERT_EQUAL_64(0xffffffff89abcdef, data8);
  }
}

TEST(cas_casa_casl_casal_x) {
  uint64_t data1 = 0x0123456789abcdef;
  uint64_t data2 = 0x0123456789abcdef;
  uint64_t data3 = 0x0123456789abcdef;
  uint64_t data4 = 0x0123456789abcdef;
  uint64_t data5 = 0x0123456789abcdef;
  uint64_t data6 = 0x0123456789abcdef;
  uint64_t data7 = 0x0123456789abcdef;
  uint64_t data8 = 0x0123456789abcdef;

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);

  START();

  __ Mov(x21, reinterpret_cast<uintptr_t>(&data1));
  __ Mov(x22, reinterpret_cast<uintptr_t>(&data2));
  __ Mov(x23, reinterpret_cast<uintptr_t>(&data3));
  __ Mov(x24, reinterpret_cast<uintptr_t>(&data4));
  __ Mov(x25, reinterpret_cast<uintptr_t>(&data5));
  __ Mov(x26, reinterpret_cast<uintptr_t>(&data6));
  __ Mov(x27, reinterpret_cast<uintptr_t>(&data7));
  __ Mov(x28, reinterpret_cast<uintptr_t>(&data8));

  __ Mov(x0, 0xffffffffffffffff);

  __ Mov(x1, 0xfedcba9876543210);
  __ Mov(x2, 0x0123456789abcdef);
  __ Mov(x3, 0xfedcba9876543210);
  __ Mov(x4, 0x0123456789abcdef);
  __ Mov(x5, 0xfedcba9876543210);
  __ Mov(x6, 0x0123456789abcdef);
  __ Mov(x7, 0xfedcba9876543210);
  __ Mov(x8, 0x0123456789abcdef);

  __ Cas(x1, x0, MemOperand(x21));
  __ Cas(x2, x0, MemOperand(x22));
  __ Casa(x3, x0, MemOperand(x23));
  __ Casa(x4, x0, MemOperand(x24));
  __ Casl(x5, x0, MemOperand(x25));
  __ Casl(x6, x0, MemOperand(x26));
  __ Casal(x7, x0, MemOperand(x27));
  __ Casal(x8, x0, MemOperand(x28));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0123456789abcdef, x1);
    ASSERT_EQUAL_64(0x0123456789abcdef, x2);
    ASSERT_EQUAL_64(0x0123456789abcdef, x3);
    ASSERT_EQUAL_64(0x0123456789abcdef, x4);
    ASSERT_EQUAL_64(0x0123456789abcdef, x5);
    ASSERT_EQUAL_64(0x0123456789abcdef, x6);
    ASSERT_EQUAL_64(0x0123456789abcdef, x7);
    ASSERT_EQUAL_64(0x0123456789abcdef, x8);

    ASSERT_EQUAL_64(0x0123456789abcdef, data1);
    ASSERT_EQUAL_64(0xffffffffffffffff, data2);
    ASSERT_EQUAL_64(0x0123456789abcdef, data3);
    ASSERT_EQUAL_64(0xffffffffffffffff, data4);
    ASSERT_EQUAL_64(0x0123456789abcdef, data5);
    ASSERT_EQUAL_64(0xffffffffffffffff, data6);
    ASSERT_EQUAL_64(0x0123456789abcdef, data7);
    ASSERT_EQUAL_64(0xffffffffffffffff, data8);
  }
}

TEST(casb_casab_caslb_casalb) {
  uint32_t data1 = 0x01234567;
  uint32_t data2 = 0x01234567;
  uint32_t data3 = 0x01234567;
  uint32_t data4 = 0x01234567;
  uint32_t data5 = 0x01234567;
  uint32_t data6 = 0x01234567;
  uint32_t data7 = 0x01234567;
  uint32_t data8 = 0x01234567;

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);

  START();

  __ Mov(x21, reinterpret_cast<uintptr_t>(&data1) + 0);
  __ Mov(x22, reinterpret_cast<uintptr_t>(&data2) + 0);
  __ Mov(x23, reinterpret_cast<uintptr_t>(&data3) + 1);
  __ Mov(x24, reinterpret_cast<uintptr_t>(&data4) + 1);
  __ Mov(x25, reinterpret_cast<uintptr_t>(&data5) + 2);
  __ Mov(x26, reinterpret_cast<uintptr_t>(&data6) + 2);
  __ Mov(x27, reinterpret_cast<uintptr_t>(&data7) + 3);
  __ Mov(x28, reinterpret_cast<uintptr_t>(&data8) + 3);

  __ Mov(x0, 0xff);

  __ Mov(x1, 0x76543210);
  __ Mov(x2, 0x01234567);
  __ Mov(x3, 0x76543210);
  __ Mov(x4, 0x67012345);
  __ Mov(x5, 0x76543210);
  __ Mov(x6, 0x45670123);
  __ Mov(x7, 0x76543210);
  __ Mov(x8, 0x23456701);

  __ Casb(w1, w0, MemOperand(x21));
  __ Casb(w2, w0, MemOperand(x22));
  __ Casab(w3, w0, MemOperand(x23));
  __ Casab(w4, w0, MemOperand(x24));
  __ Caslb(w5, w0, MemOperand(x25));
  __ Caslb(w6, w0, MemOperand(x26));
  __ Casalb(w7, w0, MemOperand(x27));
  __ Casalb(w8, w0, MemOperand(x28));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x00000067, x1);
    ASSERT_EQUAL_64(0x00000067, x2);
    ASSERT_EQUAL_64(0x00000045, x3);
    ASSERT_EQUAL_64(0x00000045, x4);
    ASSERT_EQUAL_64(0x00000023, x5);
    ASSERT_EQUAL_64(0x00000023, x6);
    ASSERT_EQUAL_64(0x00000001, x7);
    ASSERT_EQUAL_64(0x00000001, x8);

    ASSERT_EQUAL_64(0x01234567, data1);
    ASSERT_EQUAL_64(0x012345ff, data2);
    ASSERT_EQUAL_64(0x01234567, data3);
    ASSERT_EQUAL_64(0x0123ff67, data4);
    ASSERT_EQUAL_64(0x01234567, data5);
    ASSERT_EQUAL_64(0x01ff4567, data6);
    ASSERT_EQUAL_64(0x01234567, data7);
    ASSERT_EQUAL_64(0xff234567, data8);
  }
}

TEST(cash_casah_caslh_casalh) {
  uint64_t data1 = 0x0123456789abcdef;
  uint64_t data2 = 0x0123456789abcdef;
  uint64_t data3 = 0x0123456789abcdef;
  uint64_t data4 = 0x0123456789abcdef;
  uint64_t data5 = 0x0123456789abcdef;
  uint64_t data6 = 0x0123456789abcdef;
  uint64_t data7 = 0x0123456789abcdef;
  uint64_t data8 = 0x0123456789abcdef;

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);

  START();

  __ Mov(x21, reinterpret_cast<uintptr_t>(&data1) + 0);
  __ Mov(x22, reinterpret_cast<uintptr_t>(&data2) + 0);
  __ Mov(x23, reinterpret_cast<uintptr_t>(&data3) + 2);
  __ Mov(x24, reinterpret_cast<uintptr_t>(&data4) + 2);
  __ Mov(x25, reinterpret_cast<uintptr_t>(&data5) + 4);
  __ Mov(x26, reinterpret_cast<uintptr_t>(&data6) + 4);
  __ Mov(x27, reinterpret_cast<uintptr_t>(&data7) + 6);
  __ Mov(x28, reinterpret_cast<uintptr_t>(&data8) + 6);

  __ Mov(x0, 0xffff);

  __ Mov(x1, 0xfedcba9876543210);
  __ Mov(x2, 0x0123456789abcdef);
  __ Mov(x3, 0xfedcba9876543210);
  __ Mov(x4, 0xcdef0123456789ab);
  __ Mov(x5, 0xfedcba9876543210);
  __ Mov(x6, 0x89abcdef01234567);
  __ Mov(x7, 0xfedcba9876543210);
  __ Mov(x8, 0x456789abcdef0123);

  __ Cash(w1, w0, MemOperand(x21));
  __ Cash(w2, w0, MemOperand(x22));
  __ Casah(w3, w0, MemOperand(x23));
  __ Casah(w4, w0, MemOperand(x24));
  __ Caslh(w5, w0, MemOperand(x25));
  __ Caslh(w6, w0, MemOperand(x26));
  __ Casalh(w7, w0, MemOperand(x27));
  __ Casalh(w8, w0, MemOperand(x28));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0000cdef, x1);
    ASSERT_EQUAL_64(0x0000cdef, x2);
    ASSERT_EQUAL_64(0x000089ab, x3);
    ASSERT_EQUAL_64(0x000089ab, x4);
    ASSERT_EQUAL_64(0x00004567, x5);
    ASSERT_EQUAL_64(0x00004567, x6);
    ASSERT_EQUAL_64(0x00000123, x7);
    ASSERT_EQUAL_64(0x00000123, x8);

    ASSERT_EQUAL_64(0x0123456789abcdef, data1);
    ASSERT_EQUAL_64(0x0123456789abffff, data2);
    ASSERT_EQUAL_64(0x0123456789abcdef, data3);
    ASSERT_EQUAL_64(0x01234567ffffcdef, data4);
    ASSERT_EQUAL_64(0x0123456789abcdef, data5);
    ASSERT_EQUAL_64(0x0123ffff89abcdef, data6);
    ASSERT_EQUAL_64(0x0123456789abcdef, data7);
    ASSERT_EQUAL_64(0xffff456789abcdef, data8);
  }
}

TEST(casp_caspa_caspl_caspal_w) {
  uint64_t data1[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data2[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data3[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data4[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data5[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data6[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data7[] = {0x7766554433221100, 0xffeeddccbbaa9988};
  uint64_t data8[] = {0x7766554433221100, 0xffeeddccbbaa9988};

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);

  START();

  __ Mov(x21, reinterpret_cast<uintptr_t>(data1) + 0);
  __ Mov(x22, reinterpret_cast<uintptr_t>(data2) + 0);
  __ Mov(x23, reinterpret_cast<uintptr_t>(data3) + 8);
  __ Mov(x24, reinterpret_cast<uintptr_t>(data4) + 8);
  __ Mov(x25, reinterpret_cast<uintptr_t>(data5) + 8);
  __ Mov(x26, reinterpret_cast<uintptr_t>(data6) + 8);
  __ Mov(x27, reinterpret_cast<uintptr_t>(data7) + 0);
  __ Mov(x28, reinterpret_cast<uintptr_t>(data8) + 0);

  __ Mov(x0, 0xfff00fff);
  __ Mov(x1, 0xfff11fff);

  __ Mov(x2, 0x77665544);
  __ Mov(x3, 0x33221100);
  __ Mov(x4, 0x33221100);
  __ Mov(x5, 0x77665544);

  __ Mov(x6, 0xffeeddcc);
  __ Mov(x7, 0xbbaa9988);
  __ Mov(x8, 0xbbaa9988);
  __ Mov(x9, 0xffeeddcc);

  __ Mov(x10, 0xffeeddcc);
  __ Mov(x11, 0xbbaa9988);
  __ Mov(x12, 0xbbaa9988);
  __ Mov(x13, 0xffeeddcc);

  __ Mov(x14, 0x77665544);
  __ Mov(x15, 0x33221100);
  __ Mov(x16, 0x33221100);
  __ Mov(x17, 0x77665544);

  __ Casp(w2, w3, w0, w1, MemOperand(x21));
  __ Casp(w4, w5, w0, w1, MemOperand(x22));
  __ Caspa(w6, w7, w0, w1, MemOperand(x23));
  __ Caspa(w8, w9, w0, w1, MemOperand(x24));
  __ Caspl(w10, w11, w0, w1, MemOperand(x25));
  __ Caspl(w12, w13, w0, w1, MemOperand(x26));
  __ Caspal(w14, w15, w0, w1, MemOperand(x27));
  __ Caspal(w16, w17, w0, w1, MemOperand(x28));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x33221100, x2);
    ASSERT_EQUAL_64(0x77665544, x3);
    ASSERT_EQUAL_64(0x33221100, x4);
    ASSERT_EQUAL_64(0x77665544, x5);
    ASSERT_EQUAL_64(0xbbaa9988, x6);
    ASSERT_EQUAL_64(0xffeeddcc, x7);
    ASSERT_EQUAL_64(0xbbaa9988, x8);
    ASSERT_EQUAL_64(0xffeeddcc, x9);
    ASSERT_EQUAL_64(0xbbaa9988, x10);
    ASSERT_EQUAL_64(0xffeeddcc, x11);
    ASSERT_EQUAL_64(0xbbaa9988, x12);
    ASSERT_EQUAL_64(0xffeeddcc, x13);
    ASSERT_EQUAL_64(0x33221100, x14);
    ASSERT_EQUAL_64(0x77665544, x15);
    ASSERT_EQUAL_64(0x33221100, x16);
    ASSERT_EQUAL_64(0x77665544, x17);

    ASSERT_EQUAL_64(0x7766554433221100, data1[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data1[1]);
    ASSERT_EQUAL_64(0xfff11ffffff00fff, data2[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data2[1]);
    ASSERT_EQUAL_64(0x7766554433221100, data3[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data3[1]);
    ASSERT_EQUAL_64(0x7766554433221100, data4[0]);
    ASSERT_EQUAL_64(0xfff11ffffff00fff, data4[1]);
    ASSERT_EQUAL_64(0x7766554433221100, data5[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data5[1]);
    ASSERT_EQUAL_64(0x7766554433221100, data6[0]);
    ASSERT_EQUAL_64(0xfff11ffffff00fff, data6[1]);
    ASSERT_EQUAL_64(0x7766554433221100, data7[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data7[1]);
    ASSERT_EQUAL_64(0xfff11ffffff00fff, data8[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data8[1]);
  }
}

TEST(casp_caspa_caspl_caspal_x) {
  alignas(kXRegSizeInBytes * 2) uint64_t data1[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data2[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data3[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data4[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data5[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data6[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data7[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};
  alignas(kXRegSizeInBytes * 2) uint64_t data8[] = {0x7766554433221100,
                                                    0xffeeddccbbaa9988,
                                                    0xfedcba9876543210,
                                                    0x0123456789abcdef};

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);

  START();

  __ Mov(x21, reinterpret_cast<uintptr_t>(data1) + 0);
  __ Mov(x22, reinterpret_cast<uintptr_t>(data2) + 0);
  __ Mov(x23, reinterpret_cast<uintptr_t>(data3) + 16);
  __ Mov(x24, reinterpret_cast<uintptr_t>(data4) + 16);
  __ Mov(x25, reinterpret_cast<uintptr_t>(data5) + 16);
  __ Mov(x26, reinterpret_cast<uintptr_t>(data6) + 16);
  __ Mov(x27, reinterpret_cast<uintptr_t>(data7) + 0);
  __ Mov(x28, reinterpret_cast<uintptr_t>(data8) + 0);

  __ Mov(x0, 0xfffffff00fffffff);
  __ Mov(x1, 0xfffffff11fffffff);

  __ Mov(x2, 0xffeeddccbbaa9988);
  __ Mov(x3, 0x7766554433221100);
  __ Mov(x4, 0x7766554433221100);
  __ Mov(x5, 0xffeeddccbbaa9988);

  __ Mov(x6, 0x0123456789abcdef);
  __ Mov(x7, 0xfedcba9876543210);
  __ Mov(x8, 0xfedcba9876543210);
  __ Mov(x9, 0x0123456789abcdef);

  __ Mov(x10, 0x0123456789abcdef);
  __ Mov(x11, 0xfedcba9876543210);
  __ Mov(x12, 0xfedcba9876543210);
  __ Mov(x13, 0x0123456789abcdef);

  __ Mov(x14, 0xffeeddccbbaa9988);
  __ Mov(x15, 0x7766554433221100);
  __ Mov(x16, 0x7766554433221100);
  __ Mov(x17, 0xffeeddccbbaa9988);

  __ Casp(x2, x3, x0, x1, MemOperand(x21));
  __ Casp(x4, x5, x0, x1, MemOperand(x22));
  __ Caspa(x6, x7, x0, x1, MemOperand(x23));
  __ Caspa(x8, x9, x0, x1, MemOperand(x24));
  __ Caspl(x10, x11, x0, x1, MemOperand(x25));
  __ Caspl(x12, x13, x0, x1, MemOperand(x26));
  __ Caspal(x14, x15, x0, x1, MemOperand(x27));
  __ Caspal(x16, x17, x0, x1, MemOperand(x28));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x7766554433221100, x2);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x3);
    ASSERT_EQUAL_64(0x7766554433221100, x4);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x5);

    ASSERT_EQUAL_64(0xfedcba9876543210, x6);
    ASSERT_EQUAL_64(0x0123456789abcdef, x7);
    ASSERT_EQUAL_64(0xfedcba9876543210, x8);
    ASSERT_EQUAL_64(0x0123456789abcdef, x9);

    ASSERT_EQUAL_64(0xfedcba9876543210, x10);
    ASSERT_EQUAL_64(0x0123456789abcdef, x11);
    ASSERT_EQUAL_64(0xfedcba9876543210, x12);
    ASSERT_EQUAL_64(0x0123456789abcdef, x13);

    ASSERT_EQUAL_64(0x7766554433221100, x14);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x15);
    ASSERT_EQUAL_64(0x7766554433221100, x16);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, x17);

    ASSERT_EQUAL_64(0x7766554433221100, data1[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data1[1]);
    ASSERT_EQUAL_64(0xfedcba9876543210, data1[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, data1[3]);

    ASSERT_EQUAL_64(0xfffffff00fffffff, data2[0]);
    ASSERT_EQUAL_64(0xfffffff11fffffff, data2[1]);
    ASSERT_EQUAL_64(0xfedcba9876543210, data2[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, data2[3]);

    ASSERT_EQUAL_64(0x7766554433221100, data3[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data3[1]);
    ASSERT_EQUAL_64(0xfedcba9876543210, data3[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, data3[3]);

    ASSERT_EQUAL_64(0x7766554433221100, data4[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data4[1]);
    ASSERT_EQUAL_64(0xfffffff00fffffff, data4[2]);
    ASSERT_EQUAL_64(0xfffffff11fffffff, data4[3]);

    ASSERT_EQUAL_64(0x7766554433221100, data5[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data5[1]);
    ASSERT_EQUAL_64(0xfedcba9876543210, data5[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, data5[3]);

    ASSERT_EQUAL_64(0x7766554433221100, data6[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data6[1]);
    ASSERT_EQUAL_64(0xfffffff00fffffff, data6[2]);
    ASSERT_EQUAL_64(0xfffffff11fffffff, data6[3]);

    ASSERT_EQUAL_64(0x7766554433221100, data7[0]);
    ASSERT_EQUAL_64(0xffeeddccbbaa9988, data7[1]);
    ASSERT_EQUAL_64(0xfedcba9876543210, data7[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, data7[3]);

    ASSERT_EQUAL_64(0xfffffff00fffffff, data8[0]);
    ASSERT_EQUAL_64(0xfffffff11fffffff, data8[1]);
    ASSERT_EQUAL_64(0xfedcba9876543210, data8[2]);
    ASSERT_EQUAL_64(0x0123456789abcdef, data8[3]);
  }
}


typedef void (MacroAssembler::*AtomicMemoryLoadSignature)(
    const Register& rs, const Register& rt, const MemOperand& src);
typedef void (MacroAssembler::*AtomicMemoryStoreSignature)(
    const Register& rs, const MemOperand& src);

void AtomicMemoryWHelper(AtomicMemoryLoadSignature* load_funcs,
                         AtomicMemoryStoreSignature* store_funcs,
                         uint64_t arg1,
                         uint64_t arg2,
                         uint64_t expected,
                         uint64_t result_mask) {
  uint64_t data0[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data1[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data2[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data3[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data4[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data5[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);
  START();

  __ Mov(x20, reinterpret_cast<uintptr_t>(data0));
  __ Mov(x21, reinterpret_cast<uintptr_t>(data1));
  __ Mov(x22, reinterpret_cast<uintptr_t>(data2));
  __ Mov(x23, reinterpret_cast<uintptr_t>(data3));

  __ Mov(x0, arg1);
  __ Mov(x1, arg1);
  __ Mov(x2, arg1);
  __ Mov(x3, arg1);

  (masm.*(load_funcs[0]))(w0, w10, MemOperand(x20));
  (masm.*(load_funcs[1]))(w1, w11, MemOperand(x21));
  (masm.*(load_funcs[2]))(w2, w12, MemOperand(x22));
  (masm.*(load_funcs[3]))(w3, w13, MemOperand(x23));

  if (store_funcs != NULL) {
    __ Mov(x24, reinterpret_cast<uintptr_t>(data4));
    __ Mov(x25, reinterpret_cast<uintptr_t>(data5));
    __ Mov(x4, arg1);
    __ Mov(x5, arg1);

    (masm.*(store_funcs[0]))(w4, MemOperand(x24));
    (masm.*(store_funcs[1]))(w5, MemOperand(x25));
  }

  END();

  if (CAN_RUN()) {
    RUN();

    uint64_t stored_value = arg2 & result_mask;
    ASSERT_EQUAL_64(stored_value, x10);
    ASSERT_EQUAL_64(stored_value, x11);
    ASSERT_EQUAL_64(stored_value, x12);
    ASSERT_EQUAL_64(stored_value, x13);

    // The data fields contain arg2 already then only the bits masked by
    // result_mask are overwritten.
    uint64_t final_expected = (arg2 & ~result_mask) | (expected & result_mask);
    ASSERT_EQUAL_64(final_expected, data0[0]);
    ASSERT_EQUAL_64(final_expected, data1[0]);
    ASSERT_EQUAL_64(final_expected, data2[0]);
    ASSERT_EQUAL_64(final_expected, data3[0]);

    if (store_funcs != NULL) {
      ASSERT_EQUAL_64(final_expected, data4[0]);
      ASSERT_EQUAL_64(final_expected, data5[0]);
    }
  }
}

void AtomicMemoryXHelper(AtomicMemoryLoadSignature* load_funcs,
                         AtomicMemoryStoreSignature* store_funcs,
                         uint64_t arg1,
                         uint64_t arg2,
                         uint64_t expected) {
  uint64_t data0[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data1[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data2[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data3[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data4[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};
  uint64_t data5[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {arg2, 0};

  SETUP_WITH_FEATURES(CPUFeatures::kAtomics);
  START();

  __ Mov(x20, reinterpret_cast<uintptr_t>(data0));
  __ Mov(x21, reinterpret_cast<uintptr_t>(data1));
  __ Mov(x22, reinterpret_cast<uintptr_t>(data2));
  __ Mov(x23, reinterpret_cast<uintptr_t>(data3));

  __ Mov(x0, arg1);
  __ Mov(x1, arg1);
  __ Mov(x2, arg1);
  __ Mov(x3, arg1);

  (masm.*(load_funcs[0]))(x0, x10, MemOperand(x20));
  (masm.*(load_funcs[1]))(x1, x11, MemOperand(x21));
  (masm.*(load_funcs[2]))(x2, x12, MemOperand(x22));
  (masm.*(load_funcs[3]))(x3, x13, MemOperand(x23));

  if (store_funcs != NULL) {
    __ Mov(x24, reinterpret_cast<uintptr_t>(data4));
    __ Mov(x25, reinterpret_cast<uintptr_t>(data5));
    __ Mov(x4, arg1);
    __ Mov(x5, arg1);

    (masm.*(store_funcs[0]))(x4, MemOperand(x24));
    (masm.*(store_funcs[1]))(x5, MemOperand(x25));
  }

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(arg2, x10);
    ASSERT_EQUAL_64(arg2, x11);
    ASSERT_EQUAL_64(arg2, x12);
    ASSERT_EQUAL_64(arg2, x13);

    ASSERT_EQUAL_64(expected, data0[0]);
    ASSERT_EQUAL_64(expected, data1[0]);
    ASSERT_EQUAL_64(expected, data2[0]);
    ASSERT_EQUAL_64(expected, data3[0]);

    if (store_funcs != NULL) {
      ASSERT_EQUAL_64(expected, data4[0]);
      ASSERT_EQUAL_64(expected, data5[0]);
    }
  }
}

// clang-format off
#define MAKE_LOADS(NAME)           \
    {&MacroAssembler::Ld##NAME,    \
     &MacroAssembler::Ld##NAME##a, \
     &MacroAssembler::Ld##NAME##l, \
     &MacroAssembler::Ld##NAME##al}
#define MAKE_STORES(NAME) \
    {&MacroAssembler::St##NAME, &MacroAssembler::St##NAME##l}

#define MAKE_B_LOADS(NAME)          \
    {&MacroAssembler::Ld##NAME##b,  \
     &MacroAssembler::Ld##NAME##ab, \
     &MacroAssembler::Ld##NAME##lb, \
     &MacroAssembler::Ld##NAME##alb}
#define MAKE_B_STORES(NAME) \
    {&MacroAssembler::St##NAME##b, &MacroAssembler::St##NAME##lb}

#define MAKE_H_LOADS(NAME)          \
    {&MacroAssembler::Ld##NAME##h,  \
     &MacroAssembler::Ld##NAME##ah, \
     &MacroAssembler::Ld##NAME##lh, \
     &MacroAssembler::Ld##NAME##alh}
#define MAKE_H_STORES(NAME) \
    {&MacroAssembler::St##NAME##h, &MacroAssembler::St##NAME##lh}
// clang-format on

TEST(atomic_memory_add) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(add);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(add);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(add);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(add);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(add);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(add);

  // The arguments are chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t arg1 = 0x0100001000100101;
  uint64_t arg2 = 0x0200002000200202;
  uint64_t expected = arg1 + arg2;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_clr) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(clr);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(clr);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(clr);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(clr);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(clr);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(clr);

  uint64_t arg1 = 0x0300003000300303;
  uint64_t arg2 = 0x0500005000500505;
  uint64_t expected = arg2 & ~arg1;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_eor) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(eor);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(eor);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(eor);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(eor);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(eor);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(eor);

  uint64_t arg1 = 0x0300003000300303;
  uint64_t arg2 = 0x0500005000500505;
  uint64_t expected = arg1 ^ arg2;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_set) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(set);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(set);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(set);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(set);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(set);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(set);

  uint64_t arg1 = 0x0300003000300303;
  uint64_t arg2 = 0x0500005000500505;
  uint64_t expected = arg1 | arg2;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_smax) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(smax);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(smax);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(smax);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(smax);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(smax);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(smax);

  uint64_t arg1 = 0x8100000080108181;
  uint64_t arg2 = 0x0100001000100101;
  uint64_t expected = 0x0100001000100101;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_smin) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(smin);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(smin);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(smin);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(smin);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(smin);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(smin);

  uint64_t arg1 = 0x8100000080108181;
  uint64_t arg2 = 0x0100001000100101;
  uint64_t expected = 0x8100000080108181;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_umax) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(umax);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(umax);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(umax);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(umax);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(umax);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(umax);

  uint64_t arg1 = 0x8100000080108181;
  uint64_t arg2 = 0x0100001000100101;
  uint64_t expected = 0x8100000080108181;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_umin) {
  AtomicMemoryLoadSignature loads[] = MAKE_LOADS(umin);
  AtomicMemoryStoreSignature stores[] = MAKE_STORES(umin);
  AtomicMemoryLoadSignature b_loads[] = MAKE_B_LOADS(umin);
  AtomicMemoryStoreSignature b_stores[] = MAKE_B_STORES(umin);
  AtomicMemoryLoadSignature h_loads[] = MAKE_H_LOADS(umin);
  AtomicMemoryStoreSignature h_stores[] = MAKE_H_STORES(umin);

  uint64_t arg1 = 0x8100000080108181;
  uint64_t arg2 = 0x0100001000100101;
  uint64_t expected = 0x0100001000100101;

  AtomicMemoryWHelper(b_loads, b_stores, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, h_stores, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, stores, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, stores, arg1, arg2, expected);
}

TEST(atomic_memory_swp) {
  AtomicMemoryLoadSignature loads[] = {&MacroAssembler::Swp,
                                       &MacroAssembler::Swpa,
                                       &MacroAssembler::Swpl,
                                       &MacroAssembler::Swpal};
  AtomicMemoryLoadSignature b_loads[] = {&MacroAssembler::Swpb,
                                         &MacroAssembler::Swpab,
                                         &MacroAssembler::Swplb,
                                         &MacroAssembler::Swpalb};
  AtomicMemoryLoadSignature h_loads[] = {&MacroAssembler::Swph,
                                         &MacroAssembler::Swpah,
                                         &MacroAssembler::Swplh,
                                         &MacroAssembler::Swpalh};

  uint64_t arg1 = 0x0100001000100101;
  uint64_t arg2 = 0x0200002000200202;
  uint64_t expected = 0x0100001000100101;

  // SWP functions have equivalent signatures to the Atomic Memory LD functions
  // so we can use the same helper but without the ST aliases.
  AtomicMemoryWHelper(b_loads, NULL, arg1, arg2, expected, kByteMask);
  AtomicMemoryWHelper(h_loads, NULL, arg1, arg2, expected, kHalfWordMask);
  AtomicMemoryWHelper(loads, NULL, arg1, arg2, expected, kWordMask);
  AtomicMemoryXHelper(loads, NULL, arg1, arg2, expected);
}


TEST(ldaprb_ldaprh_ldapr) {
  uint64_t data0[] = {0x1010101010101010, 0x1010101010101010};
  uint64_t data1[] = {0x1010101010101010, 0x1010101010101010};
  uint64_t data2[] = {0x1010101010101010, 0x1010101010101010};
  uint64_t data3[] = {0x1010101010101010, 0x1010101010101010};

  uint64_t* data0_aligned = AlignUp(data0, kXRegSizeInBytes * 2);
  uint64_t* data1_aligned = AlignUp(data1, kXRegSizeInBytes * 2);
  uint64_t* data2_aligned = AlignUp(data2, kXRegSizeInBytes * 2);
  uint64_t* data3_aligned = AlignUp(data3, kXRegSizeInBytes * 2);

  SETUP_WITH_FEATURES(CPUFeatures::kRCpc);
  START();

  __ Mov(x20, reinterpret_cast<uintptr_t>(data0_aligned));
  __ Mov(x21, reinterpret_cast<uintptr_t>(data1_aligned));
  __ Mov(x22, reinterpret_cast<uintptr_t>(data2_aligned));
  __ Mov(x23, reinterpret_cast<uintptr_t>(data3_aligned));

  __ Ldaprb(w0, MemOperand(x20));
  __ Ldaprh(w1, MemOperand(x21));
  __ Ldapr(w2, MemOperand(x22));
  __ Ldapr(x3, MemOperand(x23));

  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(0x10, x0);
    ASSERT_EQUAL_64(0x1010, x1);
    ASSERT_EQUAL_64(0x10101010, x2);
    ASSERT_EQUAL_64(0x1010101010101010, x3);
  }
}


TEST(ldapurb_ldapurh_ldapur) {
  uint64_t data[]
      __attribute__((aligned(kXRegSizeInBytes * 2))) = {0x0123456789abcdef,
                                                        0xfedcba9876543210};

  uintptr_t data_base = reinterpret_cast<uintptr_t>(data);

  SETUP_WITH_FEATURES(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm);
  START();

  __ Mov(x20, data_base);
  __ Mov(x21, data_base + 2 * sizeof(data[0]));

  __ Ldaprb(w0, MemOperand(x20));
  __ Ldaprh(w1, MemOperand(x20));
  __ Ldapr(w2, MemOperand(x20));
  __ Ldapr(x3, MemOperand(x20));
  __ Ldaprb(w4, MemOperand(x20, 12));
  __ Ldaprh(w5, MemOperand(x20, 8));
  __ Ldapr(w6, MemOperand(x20, 10));
  __ Ldapr(x7, MemOperand(x20, 7));
  __ Ldaprb(w8, MemOperand(x21, -1));
  __ Ldaprh(w9, MemOperand(x21, -3));
  __ Ldapr(w10, MemOperand(x21, -9));
  __ Ldapr(x11, MemOperand(x21, -12));

  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(0xef, x0);
    ASSERT_EQUAL_64(0xcdef, x1);
    ASSERT_EQUAL_64(0x89abcdef, x2);
    ASSERT_EQUAL_64(0x0123456789abcdef, x3);
    ASSERT_EQUAL_64(0x98, x4);
    ASSERT_EQUAL_64(0x3210, x5);
    ASSERT_EQUAL_64(0xba987654, x6);
    ASSERT_EQUAL_64(0xdcba987654321001, x7);
    ASSERT_EQUAL_64(0xfe, x8);
    ASSERT_EQUAL_64(0xdcba, x9);
    ASSERT_EQUAL_64(0x54321001, x10);
    ASSERT_EQUAL_64(0x7654321001234567, x11);
  }
}


TEST(ldapursb_ldapursh_ldapursw) {
  uint64_t data[]
      __attribute__((aligned(kXRegSizeInBytes * 2))) = {0x0123456789abcdef,
                                                        0xfedcba9876543210};

  uintptr_t data_base = reinterpret_cast<uintptr_t>(data);

  SETUP_WITH_FEATURES(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm);
  START();

  __ Mov(x20, data_base);
  __ Mov(x21, data_base + 2 * sizeof(data[0]));

  __ Ldapursb(w0, MemOperand(x20));
  __ Ldapursb(x1, MemOperand(x20));
  __ Ldapursh(w2, MemOperand(x20));
  __ Ldapursh(x3, MemOperand(x20));
  __ Ldapursw(x4, MemOperand(x20));
  __ Ldapursb(w5, MemOperand(x20, 12));
  __ Ldapursb(x6, MemOperand(x20, 12));
  __ Ldapursh(w7, MemOperand(x20, 13));
  __ Ldapursh(x8, MemOperand(x20, 13));
  __ Ldapursw(x9, MemOperand(x20, 10));
  __ Ldapursb(w10, MemOperand(x21, -1));
  __ Ldapursb(x11, MemOperand(x21, -1));
  __ Ldapursh(w12, MemOperand(x21, -4));
  __ Ldapursh(x13, MemOperand(x21, -4));
  __ Ldapursw(x14, MemOperand(x21, -5));

  __ Ldapursb(x15, MemOperand(x20, 8));
  __ Ldapursh(x16, MemOperand(x20, 8));
  __ Ldapursw(x17, MemOperand(x20, 8));

  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(0xffffffef, x0);
    ASSERT_EQUAL_64(0xffffffffffffffef, x1);
    ASSERT_EQUAL_64(0xffffcdef, x2);
    ASSERT_EQUAL_64(0xffffffffffffcdef, x3);
    ASSERT_EQUAL_64(0xffffffff89abcdef, x4);
    ASSERT_EQUAL_64(0xffffff98, x5);
    ASSERT_EQUAL_64(0xffffffffffffff98, x6);
    ASSERT_EQUAL_64(0xffffdcba, x7);
    ASSERT_EQUAL_64(0xffffffffffffdcba, x8);
    ASSERT_EQUAL_64(0xffffffffba987654, x9);
    ASSERT_EQUAL_64(0xfffffffe, x10);
    ASSERT_EQUAL_64(0xfffffffffffffffe, x11);
    ASSERT_EQUAL_64(0xffffba98, x12);
    ASSERT_EQUAL_64(0xffffffffffffba98, x13);
    ASSERT_EQUAL_64(0xffffffffdcba9876, x14);

    ASSERT_EQUAL_64(0x0000000000000010, x15);
    ASSERT_EQUAL_64(0x0000000000003210, x16);
    ASSERT_EQUAL_64(0x0000000076543210, x17);
  }
}


TEST(stlurb_stlurh_strlur) {
  uint64_t data[] __attribute__((aligned(kXRegSizeInBytes * 2))) = {0x0, 0x0};

  uintptr_t data_base = reinterpret_cast<uintptr_t>(data);

  SETUP_WITH_FEATURES(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm);
  START();

  __ Mov(x0, 0x0011223344556677);
  __ Mov(x20, data_base);
  __ Mov(x21, data_base + 2 * sizeof(data[0]));

  __ Stlrb(w0, MemOperand(x20));
  __ Stlrh(w0, MemOperand(x20, 1));
  __ Stlr(w0, MemOperand(x20, 3));
  __ Stlr(x0, MemOperand(x21, -8));

  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(0x0044556677667777, data[0]);
    ASSERT_EQUAL_64(0x0011223344556677, data[1]);
  }
}


#define SIMPLE_ATOMIC_OPS(V, DEF) \
  V(DEF, add)                     \
  V(DEF, clr)                     \
  V(DEF, eor)                     \
  V(DEF, set)                     \
  V(DEF, smax)                    \
  V(DEF, smin)                    \
  V(DEF, umax)                    \
  V(DEF, umin)

#define SIMPLE_ATOMIC_STORE_MODES(V, NAME) \
  V(NAME)                                  \
  V(NAME##l)

#define SIMPLE_ATOMIC_LOAD_MODES(V, NAME) \
  SIMPLE_ATOMIC_STORE_MODES(V, NAME)      \
  V(NAME##a)                              \
  V(NAME##al)


TEST(unaligned_single_copy_atomicity) {
  uint64_t data0[] = {0x1010101010101010, 0x1010101010101010};
  uint64_t dst[] = {0x0000000000000000, 0x0000000000000000};

  uint64_t* data0_aligned = AlignUp(data0, kAtomicAccessGranule);
  uint64_t* dst_aligned = AlignUp(dst, kAtomicAccessGranule);

  CPUFeatures features(CPUFeatures::kAtomics,
                       CPUFeatures::kLORegions,
                       CPUFeatures::kRCpc,
                       CPUFeatures::kRCpcImm);
  features.Combine(CPUFeatures::kUSCAT);
  SETUP_WITH_FEATURES(features);
  START();

  __ Mov(x0, 0x0123456789abcdef);
  __ Mov(x1, 0x456789abcdef0123);
  __ Mov(x2, 0x89abcdef01234567);
  __ Mov(x3, 0xcdef0123456789ab);
  __ Mov(x18, reinterpret_cast<uintptr_t>(data0_aligned));
  __ Mov(x19, reinterpret_cast<uintptr_t>(dst_aligned));
  __ Mov(x20, x18);
  __ Mov(x21, x19);

  for (unsigned i = 0; i < kAtomicAccessGranule; i++) {
    __ Stxrb(w0, w1, MemOperand(x20));
    __ Stlxrb(w0, w1, MemOperand(x20));
    __ Ldxrb(w0, MemOperand(x20));
    __ Ldaxrb(w0, MemOperand(x20));
    __ Stllrb(w0, MemOperand(x20));
    __ Stlrb(w0, MemOperand(x20));
    __ Casb(w0, w1, MemOperand(x20));
    __ Caslb(w0, w1, MemOperand(x20));
    __ Ldlarb(w0, MemOperand(x20));
    __ Ldarb(w0, MemOperand(x20));
    __ Casab(w0, w1, MemOperand(x20));
    __ Casalb(w0, w1, MemOperand(x20));

    __ Swpb(w0, w1, MemOperand(x20));
    __ Swplb(w0, w1, MemOperand(x20));
    __ Swpab(w0, w1, MemOperand(x20));
    __ Swpalb(w0, w1, MemOperand(x20));
    __ Ldaprb(w0, MemOperand(x20));
    // Use offset instead of Add to test Stlurb and Ldapurb.
    __ Stlrb(w0, MemOperand(x19, i));
    __ Ldaprb(w0, MemOperand(x19, i));
    __ Ldapursb(w0, MemOperand(x20));
    __ Ldapursb(x0, MemOperand(x20));

#define ATOMIC_LOAD_B(NAME) __ Ld##NAME##b(w0, w1, MemOperand(x20));
#define ATOMIC_STORE_B(NAME) __ St##NAME##b(w0, MemOperand(x20));
    SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_B)
    SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_B)
#undef ATOMIC_LOAD_B
#undef ATOMIC_STORE_B

    if (i <= (kAtomicAccessGranule - kHRegSizeInBytes)) {
      __ Stxrh(w0, w1, MemOperand(x20));
      __ Stlxrh(w0, w1, MemOperand(x20));
      __ Ldxrh(w0, MemOperand(x20));
      __ Ldaxrh(w0, MemOperand(x20));
      __ Stllrh(w0, MemOperand(x20));
      __ Stlrh(w0, MemOperand(x20));
      __ Cash(w0, w1, MemOperand(x20));
      __ Caslh(w0, w1, MemOperand(x20));
      __ Ldlarh(w0, MemOperand(x20));
      __ Ldarh(w0, MemOperand(x20));
      __ Casah(w0, w1, MemOperand(x20));
      __ Casalh(w0, w1, MemOperand(x20));

      __ Swph(w0, w1, MemOperand(x20));
      __ Swplh(w0, w1, MemOperand(x20));
      __ Swpah(w0, w1, MemOperand(x20));
      __ Swpalh(w0, w1, MemOperand(x20));
      __ Ldaprh(w0, MemOperand(x20));
      // Use offset instead of Add to test Stlurh and Ldapurh.
      __ Stlrh(w0, MemOperand(x19, i));
      __ Ldaprh(w0, MemOperand(x19, i));
      __ Ldapursh(w0, MemOperand(x20));
      __ Ldapursh(x0, MemOperand(x20));

#define ATOMIC_LOAD_H(NAME) __ Ld##NAME##h(w0, w1, MemOperand(x20));
#define ATOMIC_STORE_H(NAME) __ St##NAME##h(w0, MemOperand(x20));
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_H)
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_H)
#undef ATOMIC_LOAD_H
#undef ATOMIC_STORE_H
    }

    if (i <= (kAtomicAccessGranule - kWRegSizeInBytes)) {
      __ Stxr(w0, w1, MemOperand(x20));
      __ Stlxr(w0, w1, MemOperand(x20));
      __ Ldxr(w0, MemOperand(x20));
      __ Ldaxr(w0, MemOperand(x20));
      __ Stllr(w0, MemOperand(x20));
      __ Stlr(w0, MemOperand(x20));
      __ Cas(w0, w1, MemOperand(x20));
      __ Casl(w0, w1, MemOperand(x20));
      __ Ldlar(w0, MemOperand(x20));
      __ Ldar(w0, MemOperand(x20));
      __ Casa(w0, w1, MemOperand(x20));
      __ Casal(w0, w1, MemOperand(x20));

      __ Swp(w0, w1, MemOperand(x20));
      __ Swpl(w0, w1, MemOperand(x20));
      __ Swpa(w0, w1, MemOperand(x20));
      __ Swpal(w0, w1, MemOperand(x20));
      __ Ldapr(w0, MemOperand(x20));
      // Use offset instead of Add to test Stlur and Ldapur.
      __ Stlr(w0, MemOperand(x19, i));
      __ Ldapr(w0, MemOperand(x19, i));
      __ Ldapursw(x0, MemOperand(x20));

#define ATOMIC_LOAD_W(NAME) __ Ld##NAME(w0, w1, MemOperand(x20));
#define ATOMIC_STORE_W(NAME) __ St##NAME(w0, MemOperand(x20));
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_W)
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_W)
#undef ATOMIC_LOAD_W
#undef ATOMIC_STORE_W
    }

    if (i <= (kAtomicAccessGranule - (kWRegSizeInBytes * 2))) {
      __ Casp(w0, w1, w2, w3, MemOperand(x20));
      __ Caspl(w0, w1, w2, w3, MemOperand(x20));
      __ Caspa(w0, w1, w2, w3, MemOperand(x20));
      __ Caspal(w0, w1, w2, w3, MemOperand(x20));
      __ Stxp(w0, w1, w2, MemOperand(x20));
      __ Stlxp(w0, w1, w2, MemOperand(x20));
      __ Ldxp(w0, w1, MemOperand(x20));
      __ Ldaxp(w0, w1, MemOperand(x20));
    }

    if (i <= (kAtomicAccessGranule - kXRegSizeInBytes)) {
      __ Stxr(x0, x1, MemOperand(x20));
      __ Stlxr(x0, x1, MemOperand(x20));
      __ Ldxr(x0, MemOperand(x20));
      __ Ldaxr(x0, MemOperand(x20));
      __ Stllr(x0, MemOperand(x20));
      __ Stlr(x0, MemOperand(x20));
      __ Cas(x0, x1, MemOperand(x20));
      __ Casl(x0, x1, MemOperand(x20));
      __ Ldlar(x0, MemOperand(x20));
      __ Ldar(x0, MemOperand(x20));
      __ Casa(x0, x1, MemOperand(x20));
      __ Casal(x0, x1, MemOperand(x20));

      __ Swp(x0, x1, MemOperand(x20));
      __ Swpl(x0, x1, MemOperand(x20));
      __ Swpa(x0, x1, MemOperand(x20));
      __ Swpal(x0, x1, MemOperand(x20));
      __ Ldapr(x0, MemOperand(x20));
      // Use offset instead of Add to test Stlur and Ldapur.
      __ Stlr(x0, MemOperand(x19, i));
      __ Ldapr(x0, MemOperand(x19, i));

#define ATOMIC_LOAD_X(NAME) __ Ld##NAME(x0, x1, MemOperand(x20));
#define ATOMIC_STORE_X(NAME) __ St##NAME(x0, MemOperand(x20));
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_X)
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_X)
#undef ATOMIC_LOAD_X
#undef ATOMIC_STORE_X
    }

    if (i <= (kAtomicAccessGranule - (kXRegSizeInBytes * 2))) {
      __ Casp(x0, x1, x2, x3, MemOperand(x20));
      __ Caspl(x0, x1, x2, x3, MemOperand(x20));
      __ Caspa(x0, x1, x2, x3, MemOperand(x20));
      __ Caspal(x0, x1, x2, x3, MemOperand(x20));
      __ Stxp(x0, x1, x2, MemOperand(x20));
      __ Stlxp(x0, x1, x2, MemOperand(x20));
      __ Ldxp(x0, x1, MemOperand(x20));
      __ Ldaxp(x0, x1, MemOperand(x20));
    }

    __ Add(x20, x20, 1);
    __ Add(x21, x21, 1);
  }
  END();

  if (CAN_RUN()) {
    // We can't detect kUSCAT with the CPUFeaturesAuditor so it fails the seen
    // check.
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
  }
}


#if defined(VIXL_NEGATIVE_TESTING) && defined(VIXL_INCLUDE_SIMULATOR_AARCH64)

#define CHECK_ALIGN_FAIL(i, expr)                                              \
  {                                                                            \
    CPUFeatures features(CPUFeatures::kAtomics,                                \
                         CPUFeatures::kLORegions,                              \
                         CPUFeatures::kRCpc,                                   \
                         CPUFeatures::kRCpcImm);                               \
    features.Combine(CPUFeatures::kUSCAT);                                     \
    SETUP_WITH_FEATURES(features);                                             \
    START();                                                                   \
    __ Mov(x0, 0x0123456789abcdef);                                            \
    __ Mov(x1, 0x456789abcdef0123);                                            \
    __ Mov(x2, 0x89abcdef01234567);                                            \
    __ Mov(x3, 0xcdef0123456789ab);                                            \
    __ Mov(x20, reinterpret_cast<uintptr_t>(data0_aligned));                   \
    __ Mov(x21, reinterpret_cast<uintptr_t>(dst_aligned));                     \
    __ Add(x20, x20, i);                                                       \
    __ Add(x21, x21, i);                                                       \
    expr;                                                                      \
    END();                                                                     \
    if (CAN_RUN()) {                                                           \
      /* We can't detect kUSCAT with the CPUFeaturesAuditor so it fails the */ \
      /* seen check. */                                                        \
      MUST_FAIL_WITH_MESSAGE(RUN_WITHOUT_SEEN_FEATURE_CHECK(),                 \
                             "ALIGNMENT EXCEPTION");                           \
    }                                                                          \
  }

TEST(unaligned_single_copy_atomicity_negative_test) {
  uint64_t data0[] = {0x1010101010101010, 0x1010101010101010};
  uint64_t dst[] = {0x0000000000000000, 0x0000000000000000};

  uint64_t* data0_aligned = AlignUp(data0, kAtomicAccessGranule);
  uint64_t* dst_aligned = AlignUp(dst, kAtomicAccessGranule);

  for (unsigned i = 0; i < kAtomicAccessGranule; i++) {
    if (i > (kAtomicAccessGranule - kHRegSizeInBytes)) {
      CHECK_ALIGN_FAIL(i, __ Stxrh(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlxrh(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldxrh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldaxrh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stllrh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlrh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Cash(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caslh(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldlarh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldarh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casah(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casalh(w0, w1, MemOperand(x20)));

      CHECK_ALIGN_FAIL(i, __ Swph(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swplh(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpah(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpalh(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldaprh(w0, MemOperand(x20)));
      // Use offset instead of Add to test Stlurh and Ldapurh.
      CHECK_ALIGN_FAIL(0, __ Stlrh(w0, MemOperand(x20, i)));
      CHECK_ALIGN_FAIL(0, __ Ldaprh(w0, MemOperand(x20, i)));
      CHECK_ALIGN_FAIL(i, __ Ldapursh(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldapursh(x0, MemOperand(x20)));

#define ATOMIC_LOAD_H(NAME) \
  CHECK_ALIGN_FAIL(i, __ Ld##NAME##h(w0, w1, MemOperand(x20)));
#define ATOMIC_STORE_H(NAME) \
  CHECK_ALIGN_FAIL(i, __ St##NAME##h(w0, MemOperand(x20)));
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_H)
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_H)
#undef ATOMIC_LOAD_H
#undef ATOMIC_STORE_H
    }

    if (i > (kAtomicAccessGranule - kWRegSizeInBytes)) {
      CHECK_ALIGN_FAIL(i, __ Stxr(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlxr(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldxr(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldaxr(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stllr(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlr(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Cas(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casl(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldlar(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldar(w0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casa(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casal(w0, w1, MemOperand(x20)));

      CHECK_ALIGN_FAIL(i, __ Swp(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpl(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpa(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpal(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldapr(w0, MemOperand(x20)));
      // Use offset instead of add to test Stlur and Ldapur.
      CHECK_ALIGN_FAIL(0, __ Stlr(w0, MemOperand(x20, i)));
      CHECK_ALIGN_FAIL(0, __ Ldapr(w0, MemOperand(x20, i)));
      CHECK_ALIGN_FAIL(i, __ Ldapursw(x0, MemOperand(x20)));

#define ATOMIC_LOAD_W(NAME) \
  CHECK_ALIGN_FAIL(i, __ Ld##NAME(w0, w1, MemOperand(x20)));
#define ATOMIC_STORE_W(NAME) \
  CHECK_ALIGN_FAIL(i, __ St##NAME(w0, MemOperand(x20)));
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_W)
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_W)
#undef ATOMIC_LOAD_W
#undef ATOMIC_STORE_W
    }

    if (i > (kAtomicAccessGranule - (kWRegSizeInBytes * 2))) {
      CHECK_ALIGN_FAIL(i, __ Casp(w0, w1, w2, w3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caspl(w0, w1, w2, w3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caspa(w0, w1, w2, w3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caspal(w0, w1, w2, w3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stxp(w0, w1, w2, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlxp(w0, w1, w2, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldxp(w0, w1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldaxp(w0, w1, MemOperand(x20)));
    }

    if (i > (kAtomicAccessGranule - kXRegSizeInBytes)) {
      CHECK_ALIGN_FAIL(i, __ Stxr(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlxr(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldxr(x0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldaxr(x0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stllr(x0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlr(x0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Cas(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casl(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldlar(x0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldar(x0, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casa(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Casal(x0, x1, MemOperand(x20)));

      CHECK_ALIGN_FAIL(i, __ Swp(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpl(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpa(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Swpal(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldapr(x0, MemOperand(x20)));
      // Use offset instead of add to test Stlur and Ldapur.
      CHECK_ALIGN_FAIL(0, __ Stlr(x0, MemOperand(x20, i)));
      CHECK_ALIGN_FAIL(0, __ Ldapr(x0, MemOperand(x20, i)));

#define ATOMIC_LOAD_X(NAME) \
  CHECK_ALIGN_FAIL(i, __ Ld##NAME(x0, x1, MemOperand(x20)));
#define ATOMIC_STORE_X(NAME) \
  CHECK_ALIGN_FAIL(i, __ St##NAME(x0, MemOperand(x20)));
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_LOAD_MODES, ATOMIC_LOAD_X)
      SIMPLE_ATOMIC_OPS(SIMPLE_ATOMIC_STORE_MODES, ATOMIC_STORE_X)
#undef ATOMIC_LOAD_X
#undef ATOMIC_STORE_X
    }

    if (i > (kAtomicAccessGranule - (kXRegSizeInBytes * 2))) {
      CHECK_ALIGN_FAIL(i, __ Casp(x0, x1, x2, x3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caspl(x0, x1, x2, x3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caspa(x0, x1, x2, x3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Caspal(x0, x1, x2, x3, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stxp(x0, x1, x2, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Stlxp(x0, x1, x2, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldxp(x0, x1, MemOperand(x20)));
      CHECK_ALIGN_FAIL(i, __ Ldaxp(x0, x1, MemOperand(x20)));
    }
  }
}

TEST(unaligned_single_copy_atomicity_negative_test_2) {
  uint64_t data[] = {0x1010101010101010, 0x1010101010101010};

  uint64_t* data_aligned = AlignUp(data, kAtomicAccessGranule);

  // Check that the same code doesn't fail with USCAT enabled but does
  // fail when not enabled.
  {
    SETUP_WITH_FEATURES(CPUFeatures::kUSCAT);
    START();
    __ Mov(x0, reinterpret_cast<uintptr_t>(data_aligned));
    __ Add(x0, x0, 1);
    __ Ldxrh(w1, MemOperand(x0));
    END();
    if (CAN_RUN()) {
      RUN_WITHOUT_SEEN_FEATURE_CHECK();
    }
  }
  {
    SETUP();
    START();
    __ Mov(x0, reinterpret_cast<uintptr_t>(data_aligned));
    __ Add(x0, x0, 1);
    __ Ldxrh(w1, MemOperand(x0));
    END();
    if (CAN_RUN()) {
      MUST_FAIL_WITH_MESSAGE(RUN(), "ALIGNMENT EXCEPTION");
    }
  }
}
#endif  // VIXL_NEGATIVE_TESTING && VIXL_INCLUDE_SIMULATOR_AARCH64


TEST(load_store_tagged_immediate_offset) {
  uint64_t tags[] = {0x00, 0x1, 0x55, 0xff};
  int tag_count = sizeof(tags) / sizeof(tags[0]);

  const int kMaxDataLength = 160;

  for (int i = 0; i < tag_count; i++) {
    unsigned char src[kMaxDataLength];
    uint64_t src_raw = reinterpret_cast<uint64_t>(src);
    uint64_t src_tag = tags[i];
    uint64_t src_tagged = CPU::SetPointerTag(src_raw, src_tag);

    for (int k = 0; k < kMaxDataLength; k++) {
      src[k] = k + 1;
    }

    for (int j = 0; j < tag_count; j++) {
      unsigned char dst[kMaxDataLength];
      uint64_t dst_raw = reinterpret_cast<uint64_t>(dst);
      uint64_t dst_tag = tags[j];
      uint64_t dst_tagged = CPU::SetPointerTag(dst_raw, dst_tag);

      memset(dst, 0, kMaxDataLength);

      SETUP_WITH_FEATURES(CPUFeatures::kNEON);
      START();

      __ Mov(x0, src_tagged);
      __ Mov(x1, dst_tagged);

      int offset = 0;

      // Scaled-immediate offsets.
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(q0, q1, MemOperand(x0, offset));
        __ stp(q0, q1, MemOperand(x1, offset));
      }
      offset += 2 * kQRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(x2, x3, MemOperand(x0, offset));
        __ stp(x2, x3, MemOperand(x1, offset));
      }
      offset += 2 * kXRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldpsw(x2, x3, MemOperand(x0, offset));
        __ stp(w2, w3, MemOperand(x1, offset));
      }
      offset += 2 * kWRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(d0, d1, MemOperand(x0, offset));
        __ stp(d0, d1, MemOperand(x1, offset));
      }
      offset += 2 * kDRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(w2, w3, MemOperand(x0, offset));
        __ stp(w2, w3, MemOperand(x1, offset));
      }
      offset += 2 * kWRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(s0, s1, MemOperand(x0, offset));
        __ stp(s0, s1, MemOperand(x1, offset));
      }
      offset += 2 * kSRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(x2, MemOperand(x0, offset), RequireScaledOffset);
        __ str(x2, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += kXRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(d0, MemOperand(x0, offset), RequireScaledOffset);
        __ str(d0, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += kDRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(w2, MemOperand(x0, offset), RequireScaledOffset);
        __ str(w2, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += kWRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(s0, MemOperand(x0, offset), RequireScaledOffset);
        __ str(s0, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += kSRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrh(w2, MemOperand(x0, offset), RequireScaledOffset);
        __ strh(w2, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += 2;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrsh(w2, MemOperand(x0, offset), RequireScaledOffset);
        __ strh(w2, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += 2;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrb(w2, MemOperand(x0, offset), RequireScaledOffset);
        __ strb(w2, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += 1;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrsb(w2, MemOperand(x0, offset), RequireScaledOffset);
        __ strb(w2, MemOperand(x1, offset), RequireScaledOffset);
      }
      offset += 1;

      // Unscaled-immediate offsets.

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldur(x2, MemOperand(x0, offset), RequireUnscaledOffset);
        __ stur(x2, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += kXRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldur(d0, MemOperand(x0, offset), RequireUnscaledOffset);
        __ stur(d0, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += kDRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldur(w2, MemOperand(x0, offset), RequireUnscaledOffset);
        __ stur(w2, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += kWRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldur(s0, MemOperand(x0, offset), RequireUnscaledOffset);
        __ stur(s0, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += kSRegSizeInBytes;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldurh(w2, MemOperand(x0, offset), RequireUnscaledOffset);
        __ sturh(w2, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += 2;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldursh(w2, MemOperand(x0, offset), RequireUnscaledOffset);
        __ sturh(w2, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += 2;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldurb(w2, MemOperand(x0, offset), RequireUnscaledOffset);
        __ sturb(w2, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += 1;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldursb(w2, MemOperand(x0, offset), RequireUnscaledOffset);
        __ sturb(w2, MemOperand(x1, offset), RequireUnscaledOffset);
      }
      offset += 1;

      // Extract the tag (so we can test that it was preserved correctly).
      __ Ubfx(x0, x0, kAddressTagOffset, kAddressTagWidth);
      __ Ubfx(x1, x1, kAddressTagOffset, kAddressTagWidth);

      VIXL_ASSERT(kMaxDataLength >= offset);

      END();
      if (CAN_RUN()) {
        RUN();

        ASSERT_EQUAL_64(src_tag, x0);
        ASSERT_EQUAL_64(dst_tag, x1);

        for (int k = 0; k < offset; k++) {
          VIXL_CHECK(src[k] == dst[k]);
        }
      }
    }
  }
}


TEST(load_store_tagged_immediate_preindex) {
  uint64_t tags[] = {0x00, 0x1, 0x55, 0xff};
  int tag_count = sizeof(tags) / sizeof(tags[0]);

  const int kMaxDataLength = 128;

  for (int i = 0; i < tag_count; i++) {
    unsigned char src[kMaxDataLength];
    uint64_t src_raw = reinterpret_cast<uint64_t>(src);
    uint64_t src_tag = tags[i];
    uint64_t src_tagged = CPU::SetPointerTag(src_raw, src_tag);

    for (int k = 0; k < kMaxDataLength; k++) {
      src[k] = k + 1;
    }

    for (int j = 0; j < tag_count; j++) {
      unsigned char dst[kMaxDataLength];
      uint64_t dst_raw = reinterpret_cast<uint64_t>(dst);
      uint64_t dst_tag = tags[j];
      uint64_t dst_tagged = CPU::SetPointerTag(dst_raw, dst_tag);

      for (int k = 0; k < kMaxDataLength; k++) {
        dst[k] = 0;
      }

      SETUP_WITH_FEATURES(CPUFeatures::kNEON);
      START();

      // Each MemOperand must apply a pre-index equal to the size of the
      // previous access.

      // Start with a non-zero preindex.
      int preindex = 62 * kXRegSizeInBytes;
      int data_length = 0;

      __ Mov(x0, src_tagged - preindex);
      __ Mov(x1, dst_tagged - preindex);

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(q0, q1, MemOperand(x0, preindex, PreIndex));
        __ stp(q0, q1, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2 * kQRegSizeInBytes;
      data_length = preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(x2, x3, MemOperand(x0, preindex, PreIndex));
        __ stp(x2, x3, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2 * kXRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldpsw(x2, x3, MemOperand(x0, preindex, PreIndex));
        __ stp(w2, w3, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2 * kWRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(d0, d1, MemOperand(x0, preindex, PreIndex));
        __ stp(d0, d1, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2 * kDRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(w2, w3, MemOperand(x0, preindex, PreIndex));
        __ stp(w2, w3, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2 * kWRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(s0, s1, MemOperand(x0, preindex, PreIndex));
        __ stp(s0, s1, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2 * kSRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(x2, MemOperand(x0, preindex, PreIndex));
        __ str(x2, MemOperand(x1, preindex, PreIndex));
      }
      preindex = kXRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(d0, MemOperand(x0, preindex, PreIndex));
        __ str(d0, MemOperand(x1, preindex, PreIndex));
      }
      preindex = kDRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(w2, MemOperand(x0, preindex, PreIndex));
        __ str(w2, MemOperand(x1, preindex, PreIndex));
      }
      preindex = kWRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(s0, MemOperand(x0, preindex, PreIndex));
        __ str(s0, MemOperand(x1, preindex, PreIndex));
      }
      preindex = kSRegSizeInBytes;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrh(w2, MemOperand(x0, preindex, PreIndex));
        __ strh(w2, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrsh(w2, MemOperand(x0, preindex, PreIndex));
        __ strh(w2, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 2;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrb(w2, MemOperand(x0, preindex, PreIndex));
        __ strb(w2, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 1;
      data_length += preindex;

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrsb(w2, MemOperand(x0, preindex, PreIndex));
        __ strb(w2, MemOperand(x1, preindex, PreIndex));
      }
      preindex = 1;
      data_length += preindex;

      VIXL_ASSERT(kMaxDataLength >= data_length);

      END();
      if (CAN_RUN()) {
        RUN();

        // Check that the preindex was correctly applied in each operation, and
        // that the tag was preserved.
        ASSERT_EQUAL_64(src_tagged + data_length - preindex, x0);
        ASSERT_EQUAL_64(dst_tagged + data_length - preindex, x1);

        for (int k = 0; k < data_length; k++) {
          VIXL_CHECK(src[k] == dst[k]);
        }
      }
    }
  }
}


TEST(load_store_tagged_immediate_postindex) {
  uint64_t tags[] = {0x00, 0x1, 0x55, 0xff};
  int tag_count = sizeof(tags) / sizeof(tags[0]);

  const int kMaxDataLength = 128;

  for (int i = 0; i < tag_count; i++) {
    unsigned char src[kMaxDataLength];
    uint64_t src_raw = reinterpret_cast<uint64_t>(src);
    uint64_t src_tag = tags[i];
    uint64_t src_tagged = CPU::SetPointerTag(src_raw, src_tag);

    for (int k = 0; k < kMaxDataLength; k++) {
      src[k] = k + 1;
    }

    for (int j = 0; j < tag_count; j++) {
      unsigned char dst[kMaxDataLength];
      uint64_t dst_raw = reinterpret_cast<uint64_t>(dst);
      uint64_t dst_tag = tags[j];
      uint64_t dst_tagged = CPU::SetPointerTag(dst_raw, dst_tag);

      for (int k = 0; k < kMaxDataLength; k++) {
        dst[k] = 0;
      }

      SETUP_WITH_FEATURES(CPUFeatures::kNEON);
      START();

      int postindex = 2 * kXRegSizeInBytes;
      int data_length = 0;

      __ Mov(x0, src_tagged);
      __ Mov(x1, dst_tagged);

      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(x2, x3, MemOperand(x0, postindex, PostIndex));
        __ stp(x2, x3, MemOperand(x1, postindex, PostIndex));
      }
      data_length = postindex;

      postindex = 2 * kQRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(q0, q1, MemOperand(x0, postindex, PostIndex));
        __ stp(q0, q1, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 2 * kWRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldpsw(x2, x3, MemOperand(x0, postindex, PostIndex));
        __ stp(w2, w3, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 2 * kDRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(d0, d1, MemOperand(x0, postindex, PostIndex));
        __ stp(d0, d1, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 2 * kWRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(w2, w3, MemOperand(x0, postindex, PostIndex));
        __ stp(w2, w3, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 2 * kSRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldp(s0, s1, MemOperand(x0, postindex, PostIndex));
        __ stp(s0, s1, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = kXRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(x2, MemOperand(x0, postindex, PostIndex));
        __ str(x2, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = kDRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(d0, MemOperand(x0, postindex, PostIndex));
        __ str(d0, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = kWRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(w2, MemOperand(x0, postindex, PostIndex));
        __ str(w2, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = kSRegSizeInBytes;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldr(s0, MemOperand(x0, postindex, PostIndex));
        __ str(s0, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 2;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrh(w2, MemOperand(x0, postindex, PostIndex));
        __ strh(w2, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 2;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrsh(w2, MemOperand(x0, postindex, PostIndex));
        __ strh(w2, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 1;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrb(w2, MemOperand(x0, postindex, PostIndex));
        __ strb(w2, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      postindex = 1;
      {
        ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
        __ ldrsb(w2, MemOperand(x0, postindex, PostIndex));
        __ strb(w2, MemOperand(x1, postindex, PostIndex));
      }
      data_length += postindex;

      VIXL_ASSERT(kMaxDataLength >= data_length);

      END();
      if (CAN_RUN()) {
        RUN();

        // Check that the postindex was correctly applied in each operation, and
        // that the tag was preserved.
        ASSERT_EQUAL_64(src_tagged + data_length, x0);
        ASSERT_EQUAL_64(dst_tagged + data_length, x1);

        for (int k = 0; k < data_length; k++) {
          VIXL_CHECK(src[k] == dst[k]);
        }
      }
    }
  }
}


TEST(load_store_tagged_register_offset) {
  uint64_t tags[] = {0x00, 0x1, 0x55, 0xff};
  int tag_count = sizeof(tags) / sizeof(tags[0]);

  const int kMaxDataLength = 128;

  for (int i = 0; i < tag_count; i++) {
    unsigned char src[kMaxDataLength];
    uint64_t src_raw = reinterpret_cast<uint64_t>(src);
    uint64_t src_tag = tags[i];
    uint64_t src_tagged = CPU::SetPointerTag(src_raw, src_tag);

    for (int k = 0; k < kMaxDataLength; k++) {
      src[k] = k + 1;
    }

    for (int j = 0; j < tag_count; j++) {
      unsigned char dst[kMaxDataLength];
      uint64_t dst_raw = reinterpret_cast<uint64_t>(dst);
      uint64_t dst_tag = tags[j];
      uint64_t dst_tagged = CPU::SetPointerTag(dst_raw, dst_tag);

      // Also tag the offset register; the operation should still succeed.
      for (int o = 0; o < tag_count; o++) {
        uint64_t offset_base = CPU::SetPointerTag(UINT64_C(0), tags[o]);
        int data_length = 0;

        for (int k = 0; k < kMaxDataLength; k++) {
          dst[k] = 0;
        }

        SETUP_WITH_FEATURES(CPUFeatures::kNEON);
        START();

        __ Mov(x0, src_tagged);
        __ Mov(x1, dst_tagged);

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldr(x2, MemOperand(x0, x10));
          __ str(x2, MemOperand(x1, x10));
        }
        data_length += kXRegSizeInBytes;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldr(d0, MemOperand(x0, x10));
          __ str(d0, MemOperand(x1, x10));
        }
        data_length += kDRegSizeInBytes;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldr(w2, MemOperand(x0, x10));
          __ str(w2, MemOperand(x1, x10));
        }
        data_length += kWRegSizeInBytes;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldr(s0, MemOperand(x0, x10));
          __ str(s0, MemOperand(x1, x10));
        }
        data_length += kSRegSizeInBytes;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldrh(w2, MemOperand(x0, x10));
          __ strh(w2, MemOperand(x1, x10));
        }
        data_length += 2;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldrsh(w2, MemOperand(x0, x10));
          __ strh(w2, MemOperand(x1, x10));
        }
        data_length += 2;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldrb(w2, MemOperand(x0, x10));
          __ strb(w2, MemOperand(x1, x10));
        }
        data_length += 1;

        __ Mov(x10, offset_base + data_length);
        {
          ExactAssemblyScope scope(&masm, 2 * kInstructionSize);
          __ ldrsb(w2, MemOperand(x0, x10));
          __ strb(w2, MemOperand(x1, x10));
        }
        data_length += 1;

        VIXL_ASSERT(kMaxDataLength >= data_length);

        END();
        if (CAN_RUN()) {
          RUN();

          // Check that the postindex was correctly applied in each operation,
          // and that the tag was preserved.
          ASSERT_EQUAL_64(src_tagged, x0);
          ASSERT_EQUAL_64(dst_tagged, x1);
          ASSERT_EQUAL_64(offset_base + data_length - 1, x10);

          for (int k = 0; k < data_length; k++) {
            VIXL_CHECK(src[k] == dst[k]);
          }
        }
      }
    }
  }
}


TEST(load_store_tagged_register_postindex) {
  uint64_t src[] = {0x0706050403020100, 0x0f0e0d0c0b0a0908};
  uint64_t tags[] = {0x00, 0x1, 0x55, 0xff};
  int tag_count = sizeof(tags) / sizeof(tags[0]);

  for (int j = 0; j < tag_count; j++) {
    for (int i = 0; i < tag_count; i++) {
      SETUP_WITH_FEATURES(CPUFeatures::kNEON);

      uint64_t src_base = reinterpret_cast<uint64_t>(src);
      uint64_t src_tagged = CPU::SetPointerTag(src_base, tags[i]);
      uint64_t offset_tagged = CPU::SetPointerTag(UINT64_C(0), tags[j]);

      START();
      __ Mov(x10, src_tagged);
      __ Mov(x11, offset_tagged);
      __ Ld1(v0.V16B(), MemOperand(x10, x11, PostIndex));
      // TODO: add other instructions (ld2-4, st1-4) as they become available.
      END();

      if (CAN_RUN()) {
        RUN();

        ASSERT_EQUAL_128(0x0f0e0d0c0b0a0908, 0x0706050403020100, q0);
        ASSERT_EQUAL_64(src_tagged + offset_tagged, x10);
      }
    }
  }
}


TEST(branch_tagged) {
  SETUP();
  START();

  Label loop, loop_entry, done;
  __ Adr(x0, &loop);
  __ Mov(x1, 0);
  __ B(&loop_entry);

  __ Bind(&loop);
  __ Add(x1, x1, 1);  // Count successful jumps.

  // Advance to the next tag, then bail out if we've come back around to tag 0.
  __ Add(x0, x0, UINT64_C(1) << kAddressTagOffset);
  __ Tst(x0, kAddressTagMask);
  __ B(eq, &done);

  __ Bind(&loop_entry);
  __ Br(x0);

  __ Bind(&done);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1 << kAddressTagWidth, x1);
  }
}


TEST(branch_and_link_tagged) {
  SETUP();
  START();

  Label loop, loop_entry, done;
  __ Adr(x0, &loop);
  __ Mov(x1, 0);
  __ B(&loop_entry);

  __ Bind(&loop);

  // Bail out (before counting a successful jump) if lr appears to be tagged.
  __ Tst(lr, kAddressTagMask);
  __ B(ne, &done);

  __ Add(x1, x1, 1);  // Count successful jumps.

  // Advance to the next tag, then bail out if we've come back around to tag 0.
  __ Add(x0, x0, UINT64_C(1) << kAddressTagOffset);
  __ Tst(x0, kAddressTagMask);
  __ B(eq, &done);

  __ Bind(&loop_entry);
  __ Blr(x0);

  __ Bind(&done);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1 << kAddressTagWidth, x1);
  }
}


TEST(branch_tagged_and_adr_adrp) {
  SETUP_CUSTOM(kPageSize, PageOffsetDependentCode);
  START();

  Label loop, loop_entry, done;
  __ Adr(x0, &loop);
  __ Mov(x1, 0);
  __ B(&loop_entry);

  __ Bind(&loop);

  // Bail out (before counting a successful jump) if `adr x10, ...` is tagged.
  __ Adr(x10, &done);
  __ Tst(x10, kAddressTagMask);
  __ B(ne, &done);

  // Bail out (before counting a successful jump) if `adrp x11, ...` is tagged.
  __ Adrp(x11, &done);
  __ Tst(x11, kAddressTagMask);
  __ B(ne, &done);

  __ Add(x1, x1, 1);  // Count successful iterations.

  // Advance to the next tag, then bail out if we've come back around to tag 0.
  __ Add(x0, x0, UINT64_C(1) << kAddressTagOffset);
  __ Tst(x0, kAddressTagMask);
  __ B(eq, &done);

  __ Bind(&loop_entry);
  __ Br(x0);

  __ Bind(&done);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1 << kAddressTagWidth, x1);
  }
}

TEST(system_sys) {
  SETUP();
  const char* msg = "SYS test!";
  uintptr_t msg_addr = reinterpret_cast<uintptr_t>(msg);

  START();
  __ Mov(x4, msg_addr);
  __ Sys(3, 0x7, 0x5, 1, x4);
  __ Mov(x3, x4);
  __ Sys(3, 0x7, 0xa, 1, x3);
  __ Mov(x2, x3);
  __ Sys(3, 0x7, 0xb, 1, x2);
  __ Mov(x1, x2);
  __ Sys(3, 0x7, 0xe, 1, x1);
  // TODO: Add tests to check ZVA equivalent.
  END();

  if (CAN_RUN()) {
    RUN();
  }
}


TEST(system_ic) {
  SETUP();
  const char* msg = "IC test!";
  uintptr_t msg_addr = reinterpret_cast<uintptr_t>(msg);

  START();
  __ Mov(x11, msg_addr);
  __ Ic(IVAU, x11);
  END();

  if (CAN_RUN()) {
    RUN();
  }
}


TEST(system_dc) {
  SETUP();
  const char* msg = "DC test!";
  uintptr_t msg_addr = reinterpret_cast<uintptr_t>(msg);

  START();
  __ Mov(x20, msg_addr);
  __ Dc(CVAC, x20);
  __ Mov(x21, msg_addr);
  __ Dc(CVAU, x21);
  __ Mov(x22, msg_addr);
  __ Dc(CIVAC, x22);
  // TODO: Add tests to check ZVA.
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(msg_addr, x20);
    ASSERT_EQUAL_64(msg_addr, x21);
    ASSERT_EQUAL_64(msg_addr, x22);
  }
}


TEST(system_dcpop) {
  SETUP_WITH_FEATURES(CPUFeatures::kDCPoP);
  const char* msg = "DCPoP test!";
  uintptr_t msg_addr = reinterpret_cast<uintptr_t>(msg);

  START();
  __ Mov(x20, msg_addr);
  __ Dc(CVAP, x20);
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(msg_addr, x20);
  }
}

TEST(system_dccvadp) {
  SETUP_WITH_FEATURES(CPUFeatures::kDCCVADP);
  const char* msg = "DCCVADP test!";
  uintptr_t msg_addr = reinterpret_cast<uintptr_t>(msg);

  START();
  __ Mov(x20, msg_addr);
  __ Dc(CVADP, x20);
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(msg_addr, x20);
  }
}

TEST(system_dc_mte) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);
  const char* msg = "DC MTE test!";
  uintptr_t msg_addr = reinterpret_cast<uintptr_t>(msg);

  START();
  __ Mov(x20, msg_addr);
  __ Dc(CGVAC, x20);
  __ Dc(CGDVAC, x20);
  __ Dc(CGVAP, x20);
  __ Dc(CGDVAP, x20);
  __ Dc(CIGVAC, x20);
  __ Dc(CIGDVAC, x20);
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(msg_addr, x20);
  }
}

// We currently disable tests for CRC32 instructions when running natively.
// Support for this family of instruction is optional, and so native platforms
// may simply fail to execute the test.
TEST(crc32b) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(w1, 0);
  __ Crc32b(w10, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x138);
  __ Crc32b(w11, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x38);
  __ Crc32b(w12, w0, w1);

  __ Mov(w0, 0);
  __ Mov(w1, 128);
  __ Crc32b(w13, w0, w1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(w1, 255);
  __ Crc32b(w14, w0, w1);

  __ Mov(w0, 0x00010001);
  __ Mov(w1, 0x10001000);
  __ Crc32b(w15, w0, w1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0x5f058808, x11);
    ASSERT_EQUAL_64(0x5f058808, x12);
    ASSERT_EQUAL_64(0xedb88320, x13);
    ASSERT_EQUAL_64(0x00ffffff, x14);
    ASSERT_EQUAL_64(0x77073196, x15);
  }
}


TEST(crc32h) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(w1, 0);
  __ Crc32h(w10, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x10038);
  __ Crc32h(w11, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x38);
  __ Crc32h(w12, w0, w1);

  __ Mov(w0, 0);
  __ Mov(w1, 128);
  __ Crc32h(w13, w0, w1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(w1, 255);
  __ Crc32h(w14, w0, w1);

  __ Mov(w0, 0x00010001);
  __ Mov(w1, 0x10001000);
  __ Crc32h(w15, w0, w1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0x0e848dba, x11);
    ASSERT_EQUAL_64(0x0e848dba, x12);
    ASSERT_EQUAL_64(0x3b83984b, x13);
    ASSERT_EQUAL_64(0x2d021072, x14);
    ASSERT_EQUAL_64(0x04ac2124, x15);
  }
}


TEST(crc32w) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(w1, 0);
  __ Crc32w(w10, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x80000031);
  __ Crc32w(w11, w0, w1);

  __ Mov(w0, 0);
  __ Mov(w1, 128);
  __ Crc32w(w13, w0, w1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(w1, 255);
  __ Crc32w(w14, w0, w1);

  __ Mov(w0, 0x00010001);
  __ Mov(w1, 0x10001000);
  __ Crc32w(w15, w0, w1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0x1d937b81, x11);
    ASSERT_EQUAL_64(0xed59b63b, x13);
    ASSERT_EQUAL_64(0x00be2612, x14);
    ASSERT_EQUAL_64(0xa036e530, x15);
  }
}


TEST(crc32x) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(x1, 0);
  __ Crc32x(w10, w0, x1);

  __ Mov(w0, 0x1);
  __ Mov(x1, UINT64_C(0x0000000800000031));
  __ Crc32x(w11, w0, x1);

  __ Mov(w0, 0);
  __ Mov(x1, 128);
  __ Crc32x(w13, w0, x1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(x1, 255);
  __ Crc32x(w14, w0, x1);

  __ Mov(w0, 0x00010001);
  __ Mov(x1, UINT64_C(0x1000100000000000));
  __ Crc32x(w15, w0, x1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0x40797b92, x11);
    ASSERT_EQUAL_64(0x533b85da, x13);
    ASSERT_EQUAL_64(0xbc962670, x14);
    ASSERT_EQUAL_64(0x0667602f, x15);
  }
}


TEST(crc32cb) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(w1, 0);
  __ Crc32cb(w10, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x138);
  __ Crc32cb(w11, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x38);
  __ Crc32cb(w12, w0, w1);

  __ Mov(w0, 0);
  __ Mov(w1, 128);
  __ Crc32cb(w13, w0, w1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(w1, 255);
  __ Crc32cb(w14, w0, w1);

  __ Mov(w0, 0x00010001);
  __ Mov(w1, 0x10001000);
  __ Crc32cb(w15, w0, w1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0x4851927d, x11);
    ASSERT_EQUAL_64(0x4851927d, x12);
    ASSERT_EQUAL_64(0x82f63b78, x13);
    ASSERT_EQUAL_64(0x00ffffff, x14);
    ASSERT_EQUAL_64(0xf26b8203, x15);
  }
}


TEST(crc32ch) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(w1, 0);
  __ Crc32ch(w10, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x10038);
  __ Crc32ch(w11, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x38);
  __ Crc32ch(w12, w0, w1);

  __ Mov(w0, 0);
  __ Mov(w1, 128);
  __ Crc32ch(w13, w0, w1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(w1, 255);
  __ Crc32ch(w14, w0, w1);

  __ Mov(w0, 0x00010001);
  __ Mov(w1, 0x10001000);
  __ Crc32ch(w15, w0, w1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0xcef8494c, x11);
    ASSERT_EQUAL_64(0xcef8494c, x12);
    ASSERT_EQUAL_64(0xfbc3faf9, x13);
    ASSERT_EQUAL_64(0xad7dacae, x14);
    ASSERT_EQUAL_64(0x03fc5f19, x15);
  }
}


TEST(crc32cw) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(w1, 0);
  __ Crc32cw(w10, w0, w1);

  __ Mov(w0, 0x1);
  __ Mov(w1, 0x80000031);
  __ Crc32cw(w11, w0, w1);

  __ Mov(w0, 0);
  __ Mov(w1, 128);
  __ Crc32cw(w13, w0, w1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(w1, 255);
  __ Crc32cw(w14, w0, w1);

  __ Mov(w0, 0x00010001);
  __ Mov(w1, 0x10001000);
  __ Crc32cw(w15, w0, w1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0xbcb79ece, x11);
    ASSERT_EQUAL_64(0x52a0c93f, x13);
    ASSERT_EQUAL_64(0x9f9b5c7a, x14);
    ASSERT_EQUAL_64(0xae1b882a, x15);
  }
}


TEST(crc32cx) {
  SETUP_WITH_FEATURES(CPUFeatures::kCRC32);

  START();

  __ Mov(w0, 0);
  __ Mov(x1, 0);
  __ Crc32cx(w10, w0, x1);

  __ Mov(w0, 0x1);
  __ Mov(x1, UINT64_C(0x0000000800000031));
  __ Crc32cx(w11, w0, x1);

  __ Mov(w0, 0);
  __ Mov(x1, 128);
  __ Crc32cx(w13, w0, x1);

  __ Mov(w0, UINT32_MAX);
  __ Mov(x1, 255);
  __ Crc32cx(w14, w0, x1);

  __ Mov(w0, 0x00010001);
  __ Mov(x1, UINT64_C(0x1000100000000000));
  __ Crc32cx(w15, w0, x1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x0, x10);
    ASSERT_EQUAL_64(0x7f320fcb, x11);
    ASSERT_EQUAL_64(0x34019664, x13);
    ASSERT_EQUAL_64(0x6cc27dd0, x14);
    ASSERT_EQUAL_64(0xc6f0acdb, x15);
  }
}

TEST(regress_cmp_shift_imm) {
  SETUP();

  START();

  __ Mov(x0, 0x3d720c8d);
  __ Cmp(x0, Operand(0x3d720c8d));

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_NZCV(ZCFlag);
  }
}


TEST(compute_address) {
  SETUP();

  START();
  int64_t base_address = INT64_C(0x123000000abc);
  int64_t reg_offset = INT64_C(0x1087654321);
  Register base = x0;
  Register offset = x1;

  __ Mov(base, base_address);
  __ Mov(offset, reg_offset);


  __ ComputeAddress(x2, MemOperand(base, 0));
  __ ComputeAddress(x3, MemOperand(base, 8));
  __ ComputeAddress(x4, MemOperand(base, -100));

  __ ComputeAddress(x5, MemOperand(base, offset));
  __ ComputeAddress(x6, MemOperand(base, offset, LSL, 2));
  __ ComputeAddress(x7, MemOperand(base, offset, LSL, 4));
  __ ComputeAddress(x8, MemOperand(base, offset, LSL, 8));

  __ ComputeAddress(x9, MemOperand(base, offset, SXTW));
  __ ComputeAddress(x10, MemOperand(base, offset, UXTW, 1));
  __ ComputeAddress(x11, MemOperand(base, offset, SXTW, 2));
  __ ComputeAddress(x12, MemOperand(base, offset, UXTW, 3));

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(base_address, base);

    ASSERT_EQUAL_64(INT64_C(0x123000000abc), x2);
    ASSERT_EQUAL_64(INT64_C(0x123000000ac4), x3);
    ASSERT_EQUAL_64(INT64_C(0x123000000a58), x4);

    ASSERT_EQUAL_64(INT64_C(0x124087654ddd), x5);
    ASSERT_EQUAL_64(INT64_C(0x12721d951740), x6);
    ASSERT_EQUAL_64(INT64_C(0x133876543ccc), x7);
    ASSERT_EQUAL_64(INT64_C(0x22b765432bbc), x8);

    ASSERT_EQUAL_64(INT64_C(0x122f87654ddd), x9);
    ASSERT_EQUAL_64(INT64_C(0x12310eca90fe), x10);
    ASSERT_EQUAL_64(INT64_C(0x122e1d951740), x11);
    ASSERT_EQUAL_64(INT64_C(0x12343b2a23c4), x12);
  }
}


TEST(far_branch_backward) {
  // Test that the MacroAssembler correctly resolves backward branches to labels
  // that are outside the immediate range of branch instructions.
  // Take into account that backward branches can reach one instruction further
  // than forward branches.
  const int overflow_size =
      kInstructionSize +
      std::max(Instruction::GetImmBranchForwardRange(TestBranchType),
               std::max(Instruction::GetImmBranchForwardRange(
                            CompareBranchType),
                        Instruction::GetImmBranchForwardRange(CondBranchType)));

  SETUP();
  START();

  Label done, fail;
  Label test_tbz, test_cbz, test_bcond;
  Label success_tbz, success_cbz, success_bcond;

  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x10, 0);

  __ B(&test_tbz);
  __ Bind(&success_tbz);
  __ Orr(x0, x0, 1 << 0);
  __ B(&test_cbz);
  __ Bind(&success_cbz);
  __ Orr(x0, x0, 1 << 1);
  __ B(&test_bcond);
  __ Bind(&success_bcond);
  __ Orr(x0, x0, 1 << 2);

  __ B(&done);

  // Generate enough code to overflow the immediate range of the three types of
  // branches below.
  for (unsigned i = 0; i < overflow_size / kInstructionSize; ++i) {
    if (i % 100 == 0) {
      // If we do land in this code, we do not want to execute so many nops
      // before reaching the end of test (especially if tracing is activated).
      __ B(&fail);
    } else {
      __ Nop();
    }
  }
  __ B(&fail);

  __ Bind(&test_tbz);
  __ Tbz(x10, 7, &success_tbz);
  __ Bind(&test_cbz);
  __ Cbz(x10, &success_cbz);
  __ Bind(&test_bcond);
  __ Cmp(x10, 0);
  __ B(eq, &success_bcond);

  // For each out-of-range branch instructions, at least two instructions should
  // have been generated.
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&test_tbz) >=
             7 * kInstructionSize);

  __ Bind(&fail);
  __ Mov(x1, 0);
  __ Bind(&done);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x7, x0);
    ASSERT_EQUAL_64(0x1, x1);
  }
}


TEST(single_veneer) {
  SETUP();
  START();

  const int max_range = Instruction::GetImmBranchForwardRange(TestBranchType);

  Label success, fail, done;

  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x10, 0);

  __ Tbz(x10, 7, &success);

  // Generate enough code to overflow the immediate range of the `tbz`.
  for (unsigned i = 0; i < max_range / kInstructionSize + 1; ++i) {
    if (i % 100 == 0) {
      // If we do land in this code, we do not want to execute so many nops
      // before reaching the end of test (especially if tracing is activated).
      __ B(&fail);
    } else {
      __ Nop();
    }
  }
  __ B(&fail);

  __ Bind(&success);
  __ Mov(x0, 1);

  __ B(&done);
  __ Bind(&fail);
  __ Mov(x1, 0);
  __ Bind(&done);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x0);
    ASSERT_EQUAL_64(1, x1);
  }
}


TEST(simple_veneers) {
  // Test that the MacroAssembler correctly emits veneers for forward branches
  // to labels that are outside the immediate range of branch instructions.
  const int max_range =
      std::max(Instruction::GetImmBranchForwardRange(TestBranchType),
               std::max(Instruction::GetImmBranchForwardRange(
                            CompareBranchType),
                        Instruction::GetImmBranchForwardRange(CondBranchType)));

  SETUP();
  START();

  Label done, fail;
  Label test_tbz, test_cbz, test_bcond;
  Label success_tbz, success_cbz, success_bcond;

  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x10, 0);

  __ Bind(&test_tbz);
  __ Tbz(x10, 7, &success_tbz);
  __ Bind(&test_cbz);
  __ Cbz(x10, &success_cbz);
  __ Bind(&test_bcond);
  __ Cmp(x10, 0);
  __ B(eq, &success_bcond);

  // Generate enough code to overflow the immediate range of the three types of
  // branches below.
  for (unsigned i = 0; i < max_range / kInstructionSize + 1; ++i) {
    if (i % 100 == 0) {
      // If we do land in this code, we do not want to execute so many nops
      // before reaching the end of test (especially if tracing is activated).
      __ B(&fail);
    } else {
      __ Nop();
    }
  }
  __ B(&fail);

  __ Bind(&success_tbz);
  __ Orr(x0, x0, 1 << 0);
  __ B(&test_cbz);
  __ Bind(&success_cbz);
  __ Orr(x0, x0, 1 << 1);
  __ B(&test_bcond);
  __ Bind(&success_bcond);
  __ Orr(x0, x0, 1 << 2);

  __ B(&done);
  __ Bind(&fail);
  __ Mov(x1, 0);
  __ Bind(&done);

  END();
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x7, x0);
    ASSERT_EQUAL_64(0x1, x1);
  }
}


TEST(veneers_stress) {
  SETUP();
  START();

  // This is a code generation test stressing the emission of veneers. The code
  // generated is not executed.

  Label target;
  const unsigned max_range =
      Instruction::GetImmBranchForwardRange(CondBranchType);
  const unsigned iterations =
      (max_range + max_range / 4) / (4 * kInstructionSize);
  for (unsigned i = 0; i < iterations; i++) {
    __ B(&target);
    __ B(eq, &target);
    __ Cbz(x0, &target);
    __ Tbz(x0, 0, &target);
  }
  __ Bind(&target);

  END();
}


TEST(veneers_two_out_of_range) {
  SETUP();
  START();

  // This is a code generation test. The code generated is not executed.
  // Ensure that the MacroAssembler considers unresolved branches to chose when
  // a veneer pool should be emitted. We generate two branches that go out of
  // range at the same offset. When the MacroAssembler decides to emit the
  // veneer pool, the emission of a first veneer should not cause the other
  // branch to go out of range.

  int range_cbz = Instruction::GetImmBranchForwardRange(CompareBranchType);
  int range_tbz = Instruction::GetImmBranchForwardRange(TestBranchType);
  int max_target = static_cast<int>(masm.GetCursorOffset()) + range_cbz;

  Label done;

  // We use different labels to prevent the MacroAssembler from sharing veneers.
  Label target_cbz, target_tbz;

  __ Cbz(x0, &target_cbz);
  while (masm.GetCursorOffset() < max_target - range_tbz) {
    __ Nop();
  }
  __ Tbz(x0, 0, &target_tbz);
  while (masm.GetCursorOffset() < max_target) {
    __ Nop();
  }

  // This additional nop makes the branches go out of range.
  __ Nop();

  __ Bind(&target_cbz);
  __ Bind(&target_tbz);

  END();
}


TEST(veneers_hanging) {
  SETUP();
  START();

  // This is a code generation test. The code generated is not executed.
  // Ensure that the MacroAssembler considers unresolved branches to chose when
  // a veneer pool should be emitted. This is similar to the
  // 'veneers_two_out_of_range' test. We try to trigger the following situation:
  //   b.eq label
  //   b.eq label
  //   ...
  //   nop
  //   ...
  //   cbz x0, label
  //   cbz x0, label
  //   ...
  //   tbz x0, 0 label
  //   nop
  //   ...
  //   nop    <- From here the `b.eq` and `cbz` instructions run out of range,
  //             so a literal pool is required.
  //   veneer
  //   veneer
  //   veneer <- The `tbz` runs out of range somewhere in the middle of the
  //   veneer    veneer pool.
  //   veneer

  const int range_bcond = Instruction::GetImmBranchForwardRange(CondBranchType);
  const int range_cbz =
      Instruction::GetImmBranchForwardRange(CompareBranchType);
  const int range_tbz = Instruction::GetImmBranchForwardRange(TestBranchType);
  const int max_target = static_cast<int>(masm.GetCursorOffset()) + range_bcond;

  Label done;
  const int n_bcond = 100;
  const int n_cbz = 100;
  const int n_tbz = 1;
  const int kNTotalBranches = n_bcond + n_cbz + n_tbz;

  // We use different labels to prevent the MacroAssembler from sharing veneers.
  Label labels[kNTotalBranches];
  for (int i = 0; i < kNTotalBranches; i++) {
    new (&labels[i]) Label();
  }

  for (int i = 0; i < n_bcond; i++) {
    __ B(eq, &labels[i]);
  }

  while (masm.GetCursorOffset() < max_target - range_cbz) {
    __ Nop();
  }

  for (int i = 0; i < n_cbz; i++) {
    __ Cbz(x0, &labels[n_bcond + i]);
  }

  // Ensure the 'tbz' will go out of range after some of the previously
  // generated branches.
  int margin = (n_bcond / 2) * kInstructionSize;
  while (masm.GetCursorOffset() < max_target - range_tbz + margin) {
    __ Nop();
  }

  __ Tbz(x0, 0, &labels[n_bcond + n_cbz]);

  while (masm.GetCursorOffset() < max_target) {
    __ Nop();
  }

  // This additional nop makes the 'b.eq' and 'cbz' instructions go out of range
  // and forces the emission of a veneer pool. The 'tbz' is not yet out of
  // range, but will go out of range while veneers are emitted for the other
  // branches.
  // The MacroAssembler should ensure that veneers are correctly emitted for all
  // the branches, including the 'tbz'. Checks will fail if the target of a
  // branch is out of range.
  __ Nop();

  for (int i = 0; i < kNTotalBranches; i++) {
    __ Bind(&labels[i]);
  }

  END();
}


TEST(collision_literal_veneer_pools) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP);
  START();

  // This is a code generation test. The code generated is not executed.

  // Make sure the literal pool is empty;
  masm.EmitLiteralPool(LiteralPool::kBranchRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  // We chose the offsets below to (try to) trigger the following situation:
  // buffer offset
  //              0:   tbz x0, 0, target_tbz ----------------------------------.
  //              4:   nop                                                     |
  //                   ...                                                     |
  //                   nop                                                     |
  //    literal gen:   ldr s0, [pc + ...]   ; load from `pool start + 0`       |
  //                   ldr s0, [pc + ...]   ; load from `pool start + 4`       |
  //                   ...                                                     |
  //                   ldr s0, [pc + ...]                                      |
  //     pool start:   floating-point literal (0.1)                            |
  //                   floating-point literal (1.1)                            |
  //                   ...                                                     |
  //                   floating-point literal (<n>.1)     <-----tbz-max-range--'
  //                   floating-point literal (<n+1>.1)
  //                   ...

  const int range_tbz = Instruction::GetImmBranchForwardRange(TestBranchType);
  const int max_target = static_cast<int>(masm.GetCursorOffset()) + range_tbz;

  const size_t target_literal_pool_size = 100 * kInstructionSize;
  const int offset_start_literal_gen =
      target_literal_pool_size + target_literal_pool_size / 2;


  Label target_tbz;

  __ Tbz(x0, 0, &target_tbz);
  VIXL_CHECK(masm.GetNumberOfPotentialVeneers() == 1);
  while (masm.GetCursorOffset() < max_target - offset_start_literal_gen) {
    __ Nop();
  }
  VIXL_CHECK(masm.GetNumberOfPotentialVeneers() == 1);

  for (int i = 0; i < 100; i++) {
    // Use a different value to force one literal pool entry per iteration.
    __ Ldr(s0, i + 0.1);
  }
  VIXL_CHECK(masm.GetLiteralPoolSize() >= target_literal_pool_size);

  // Force emission of a literal pool.
  masm.EmitLiteralPool(LiteralPool::kBranchRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  // The branch should not have gone out of range during the emission of the
  // literal pool.
  __ Bind(&target_tbz);

  VIXL_CHECK(masm.GetNumberOfPotentialVeneers() == 0);

  END();
}

static void VeneerBackwardBranchHelper(ImmBranchType type, int limit) {
  SETUP();
  START();

  // This is a code generation test. The code generated is not executed.

  __ Mov(x0, 1);

  // Non-veneer case: generate 'limit' instructions, plus the branch itself.
  Label start0;
  __ Bind(&start0);
  for (int i = 0; i < limit; i++) {
    __ Nop();
  }
  switch (type) {
    case CompareBranchType:
      __ Cbz(x0, &start0);
      break;
    case TestBranchType:
      __ Tbz(x0, 0, &start0);
      break;
    default:
      VIXL_ASSERT(type == CondBranchType);
      __ B(eq, &start0);
  }
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&start0) ==
             ((limit + 1) * kInstructionSize));

  // Veneer case: As above, plus one extra nop and a branch for the veneer; we
  // expect a total of limit + 3 instructions.
  //
  //  start1:
  //    nop x (limit + 1)
  //    tbnz skip_veneer
  //    b start1
  //  skip_veneer:
  //
  Label start1;
  __ Bind(&start1);
  for (int i = 0; i < limit; i++) {
    __ Nop();
  }
  __ Nop();  // One extra instruction to exceed branch range.
  switch (type) {
    case CompareBranchType:
      __ Cbz(x0, &start0);
      break;
    case TestBranchType:
      __ Tbz(x0, 0, &start0);
      break;
    default:
      VIXL_ASSERT(type == CondBranchType);
      __ B(eq, &start0);
  }
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&start1) ==
             ((limit + 3) * kInstructionSize));

  END();
  DISASSEMBLE();
}

TEST(veneer_backward_tbz) { VeneerBackwardBranchHelper(TestBranchType, 8192); }

TEST(veneer_backward_cbz) {
  VeneerBackwardBranchHelper(CompareBranchType, 262144);
}

TEST(veneer_backward_bcond) {
  VeneerBackwardBranchHelper(CondBranchType, 262144);
}

TEST(ldr_literal_explicit) {
  SETUP();

  START();
  Literal<int64_t> automatically_placed_literal(1, masm.GetLiteralPool());
  Literal<int64_t> manually_placed_literal(2);
  {
    ExactAssemblyScope scope(&masm, kInstructionSize + sizeof(int64_t));
    Label over_literal;
    __ b(&over_literal);
    __ place(&manually_placed_literal);
    __ bind(&over_literal);
  }
  __ Ldr(x1, &manually_placed_literal);
  __ Ldr(x2, &automatically_placed_literal);
  __ Add(x0, x1, x2);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(3, x0);
  }
}


TEST(ldr_literal_automatically_placed) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

  START();

  // We start with an empty literal pool.
  ASSERT_LITERAL_POOL_SIZE(0);

  // Create a literal that should be placed by the literal pool.
  Literal<int64_t> explicit_literal(2, masm.GetLiteralPool());
  // It should not appear in the literal pool until its first use.
  ASSERT_LITERAL_POOL_SIZE(0);

  // Check that using standard literals does not break the use of explicitly
  // created literals.
  __ Ldr(d1, 1.1);
  ASSERT_LITERAL_POOL_SIZE(8);
  masm.EmitLiteralPool(LiteralPool::kBranchRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  __ Ldr(x2, &explicit_literal);
  ASSERT_LITERAL_POOL_SIZE(8);
  masm.EmitLiteralPool(LiteralPool::kBranchRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  __ Ldr(d3, 3.3);
  ASSERT_LITERAL_POOL_SIZE(8);
  masm.EmitLiteralPool(LiteralPool::kBranchRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  // Re-use our explicitly created literal. It has already been placed, so it
  // should not impact the literal pool.
  __ Ldr(x4, &explicit_literal);
  ASSERT_LITERAL_POOL_SIZE(0);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_FP64(1.1, d1);
    ASSERT_EQUAL_64(2, x2);
    ASSERT_EQUAL_FP64(3.3, d3);
    ASSERT_EQUAL_64(2, x4);
  }
}


TEST(literal_update_overwrite) {
  SETUP();

  START();

  ASSERT_LITERAL_POOL_SIZE(0);
  LiteralPool* literal_pool = masm.GetLiteralPool();

  Literal<int32_t> lit_32_update_before_pool(0xbad, literal_pool);
  Literal<int32_t> lit_32_update_after_pool(0xbad, literal_pool);
  Literal<int64_t> lit_64_update_before_pool(0xbad, literal_pool);
  Literal<int64_t> lit_64_update_after_pool(0xbad, literal_pool);

  ASSERT_LITERAL_POOL_SIZE(0);

  lit_32_update_before_pool.UpdateValue(32);
  lit_64_update_before_pool.UpdateValue(64);

  __ Ldr(w1, &lit_32_update_before_pool);
  __ Ldr(x2, &lit_64_update_before_pool);
  __ Ldr(w3, &lit_32_update_after_pool);
  __ Ldr(x4, &lit_64_update_after_pool);

  masm.EmitLiteralPool(LiteralPool::kBranchRequired);

  VIXL_ASSERT(lit_32_update_after_pool.IsPlaced());
  VIXL_ASSERT(lit_64_update_after_pool.IsPlaced());
  lit_32_update_after_pool.UpdateValue(128, &masm);
  lit_64_update_after_pool.UpdateValue(256, &masm);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(32, x1);
    ASSERT_EQUAL_64(64, x2);
    ASSERT_EQUAL_64(128, x3);
    ASSERT_EQUAL_64(256, x4);
  }
}


TEST(literal_deletion_policies) {
  SETUP();

  START();

  // We cannot check exactly when the deletion of the literals occur, but we
  // check that usage of the deletion policies is not broken.

  ASSERT_LITERAL_POOL_SIZE(0);
  LiteralPool* literal_pool = masm.GetLiteralPool();

  Literal<int32_t> lit_manual(0xbad, literal_pool);
  Literal<int32_t>* lit_deleted_on_placement =
      new Literal<int32_t>(0xbad,
                           literal_pool,
                           RawLiteral::kDeletedOnPlacementByPool);
  Literal<int32_t>* lit_deleted_on_pool_destruction =
      new Literal<int32_t>(0xbad,
                           literal_pool,
                           RawLiteral::kDeletedOnPoolDestruction);

  ASSERT_LITERAL_POOL_SIZE(0);

  lit_manual.UpdateValue(32);
  lit_deleted_on_placement->UpdateValue(64);

  __ Ldr(w1, &lit_manual);
  __ Ldr(w2, lit_deleted_on_placement);
  __ Ldr(w3, lit_deleted_on_pool_destruction);

  masm.EmitLiteralPool(LiteralPool::kBranchRequired);

  VIXL_ASSERT(lit_manual.IsPlaced());
  VIXL_ASSERT(lit_deleted_on_pool_destruction->IsPlaced());
  lit_deleted_on_pool_destruction->UpdateValue(128, &masm);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(32, x1);
    ASSERT_EQUAL_64(64, x2);
    ASSERT_EQUAL_64(128, x3);
  }
}


TEST(generic_operand) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

  int32_t data_32_array[5] = {0xbadbeef,
                              0x11111111,
                              0xbadbeef,
                              0x33333333,
                              0xbadbeef};
  int64_t data_64_array[5] = {INT64_C(0xbadbadbadbeef),
                              INT64_C(0x1111111111111111),
                              INT64_C(0xbadbadbadbeef),
                              INT64_C(0x3333333333333333),
                              INT64_C(0xbadbadbadbeef)};
  size_t size_32 = sizeof(data_32_array[0]);
  size_t size_64 = sizeof(data_64_array[0]);

  START();

  intptr_t data_32_address = reinterpret_cast<intptr_t>(&data_32_array[0]);
  intptr_t data_64_address = reinterpret_cast<intptr_t>(&data_64_array[0]);
  Register data_32 = x27;
  Register data_64 = x28;
  __ Mov(data_32, data_32_address);
  __ Mov(data_64, data_64_address);

  __ Move(GenericOperand(w0),
          GenericOperand(MemOperand(data_32, 1 * size_32), size_32));
  __ Move(GenericOperand(s0),
          GenericOperand(MemOperand(data_32, 3 * size_32), size_32));
  __ Move(GenericOperand(x10),
          GenericOperand(MemOperand(data_64, 1 * size_64), size_64));
  __ Move(GenericOperand(d10),
          GenericOperand(MemOperand(data_64, 3 * size_64), size_64));

  __ Move(GenericOperand(w1), GenericOperand(w0));
  __ Move(GenericOperand(s1), GenericOperand(s0));
  __ Move(GenericOperand(x11), GenericOperand(x10));
  __ Move(GenericOperand(d11), GenericOperand(d10));

  __ Move(GenericOperand(MemOperand(data_32, 0 * size_32), size_32),
          GenericOperand(w1));
  __ Move(GenericOperand(MemOperand(data_32, 2 * size_32), size_32),
          GenericOperand(s1));
  __ Move(GenericOperand(MemOperand(data_64, 0 * size_64), size_64),
          GenericOperand(x11));
  __ Move(GenericOperand(MemOperand(data_64, 2 * size_64), size_64),
          GenericOperand(d11));

  __ Move(GenericOperand(MemOperand(data_32, 4 * size_32), size_32),
          GenericOperand(MemOperand(data_32, 0 * size_32), size_32));
  __ Move(GenericOperand(MemOperand(data_64, 4 * size_64), size_64),
          GenericOperand(MemOperand(data_64, 0 * size_64), size_64));
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(data_32_address, data_32);
    ASSERT_EQUAL_64(data_64_address, data_64);

    ASSERT_EQUAL_32(0x11111111, w0);
    ASSERT_EQUAL_32(0x33333333, core.sreg_bits(0));
    ASSERT_EQUAL_64(INT64_C(0x1111111111111111), x10);
    ASSERT_EQUAL_64(INT64_C(0x3333333333333333), core.dreg_bits(10));

    ASSERT_EQUAL_32(0x11111111, w1);
    ASSERT_EQUAL_32(0x33333333, core.sreg_bits(1));
    ASSERT_EQUAL_64(INT64_C(0x1111111111111111), x11);
    ASSERT_EQUAL_64(INT64_C(0x3333333333333333), core.dreg_bits(11));

    VIXL_CHECK(data_32_array[0] == 0x11111111);
    VIXL_CHECK(data_32_array[1] == 0x11111111);
    VIXL_CHECK(data_32_array[2] == 0x33333333);
    VIXL_CHECK(data_32_array[3] == 0x33333333);
    VIXL_CHECK(data_32_array[4] == 0x11111111);

    VIXL_CHECK(data_64_array[0] == INT64_C(0x1111111111111111));
    VIXL_CHECK(data_64_array[1] == INT64_C(0x1111111111111111));
    VIXL_CHECK(data_64_array[2] == INT64_C(0x3333333333333333));
    VIXL_CHECK(data_64_array[3] == INT64_C(0x3333333333333333));
    VIXL_CHECK(data_64_array[4] == INT64_C(0x1111111111111111));
  }
}


// Test feature detection of calls to runtime functions.

// C++11 should be sufficient to provide simulated runtime calls, except for a
// GCC bug before 4.9.1.
#if defined(VIXL_INCLUDE_SIMULATOR_AARCH64) && (__cplusplus >= 201103L) && \
    (defined(__clang__) || GCC_VERSION_OR_NEWER(4, 9, 1)) &&               \
    !defined(VIXL_HAS_SIMULATED_RUNTIME_CALL_SUPPORT)
#error \
    "C++11 should be sufficient to provide support for simulated runtime calls."
#endif  // #if defined(VIXL_INCLUDE_SIMULATOR_AARCH64) && ...

#if (__cplusplus >= 201103L) && \
    !defined(VIXL_HAS_MACROASSEMBLER_RUNTIME_CALL_SUPPORT)
#error \
    "C++11 should be sufficient to provide support for `MacroAssembler::CallRuntime()`."
#endif  // #if (__cplusplus >= 201103L) && ...

#ifdef VIXL_HAS_MACROASSEMBLER_RUNTIME_CALL_SUPPORT
int32_t runtime_call_add_one(int32_t a) { return a + 1; }

double runtime_call_add_doubles(double a, double b, double c) {
  return a + b + c;
}

int64_t runtime_call_one_argument_on_stack(int64_t arg1 __attribute__((unused)),
                                           int64_t arg2 __attribute__((unused)),
                                           int64_t arg3 __attribute__((unused)),
                                           int64_t arg4 __attribute__((unused)),
                                           int64_t arg5 __attribute__((unused)),
                                           int64_t arg6 __attribute__((unused)),
                                           int64_t arg7 __attribute__((unused)),
                                           int64_t arg8 __attribute__((unused)),
                                           int64_t arg9) {
  return arg9;
}

double runtime_call_two_arguments_on_stack(int64_t arg1 __attribute__((unused)),
                                           int64_t arg2 __attribute__((unused)),
                                           int64_t arg3 __attribute__((unused)),
                                           int64_t arg4 __attribute__((unused)),
                                           int64_t arg5 __attribute__((unused)),
                                           int64_t arg6 __attribute__((unused)),
                                           int64_t arg7 __attribute__((unused)),
                                           int64_t arg8 __attribute__((unused)),
                                           double arg9,
                                           double arg10) {
  return arg9 - arg10;
}

void runtime_call_store_at_address(int64_t* address) { *address = 0xf00d; }

int32_t runtime_call_no_args() { return 1; }

enum RuntimeCallTestEnum { Enum0 };

RuntimeCallTestEnum runtime_call_enum(RuntimeCallTestEnum e) { return e; }

enum class RuntimeCallTestEnumClass { Enum0 };

RuntimeCallTestEnumClass runtime_call_enum_class(RuntimeCallTestEnumClass e) {
  return e;
}

int8_t test_int8_t(int8_t x) { return x; }
uint8_t test_uint8_t(uint8_t x) { return x; }
int16_t test_int16_t(int16_t x) { return x; }
uint16_t test_uint16_t(uint16_t x) { return x; }

TEST(runtime_calls) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP);

#ifndef VIXL_HAS_SIMULATED_RUNTIME_CALL_SUPPORT
  if (masm.GenerateSimulatorCode()) {
    // This configuration is unsupported and a `VIXL_UNREACHABLE()` would fire
    // while trying to generate `CallRuntime`. This configuration should only be
    // reachable with C++11 and a (buggy) version of GCC pre-4.9.1.
    return;
  }
#endif

  START();

  // Test `CallRuntime`.

  __ Mov(w0, 0);
  __ CallRuntime(runtime_call_add_one);
  __ Mov(w20, w0);

  __ Fmov(d0, 0.0);
  __ Fmov(d1, 1.5);
  __ Fmov(d2, 2.5);
  __ CallRuntime(runtime_call_add_doubles);
  __ Fmov(d20, d0);

  __ Mov(x0, 0x123);
  __ Push(x0, x0);
  __ CallRuntime(runtime_call_one_argument_on_stack);
  __ Mov(x21, x0);
  __ Pop(x0, x1);

  __ Fmov(d0, 314.0);
  __ Fmov(d1, 4.0);
  __ Push(d1, d0);
  __ CallRuntime(runtime_call_two_arguments_on_stack);
  __ Fmov(d21, d0);
  __ Pop(d1, d0);

  // Test that the template mechanisms don't break with enums.
  __ Mov(w0, 0);
  __ CallRuntime(runtime_call_enum);
  __ Mov(w0, 0);
  __ CallRuntime(runtime_call_enum_class);

  // Test `TailCallRuntime`.

  Label function, after_function;
  __ B(&after_function);
  __ Bind(&function);
  __ Mov(x22, 0);
  __ Mov(w0, 123);
  __ TailCallRuntime(runtime_call_add_one);
  // Control should not fall through.
  __ Mov(x22, 0xbad);
  __ Ret();
  __ Bind(&after_function);

  // Call our placeholder function, taking care to preserve the link register.
  __ Push(ip0, lr);
  __ Bl(&function);
  __ Pop(lr, ip0);
  // Save the result.
  __ Mov(w23, w0);

  __ Mov(x24, 0);
  int test_values[] = {static_cast<int8_t>(-1),
                       static_cast<uint8_t>(-1),
                       static_cast<int16_t>(-1),
                       static_cast<uint16_t>(-1),
                       -256,
                       -1,
                       0,
                       1,
                       256};
  for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); ++i) {
    Label pass_int8, pass_uint8, pass_int16, pass_uint16;
    int x = test_values[i];
    __ Mov(w0, x);
    __ CallRuntime(test_int8_t);
    __ Sxtb(w0, w0);
    __ Cmp(w0, ExtractSignedBitfield32(7, 0, x));
    __ Cinc(x24, x24, ne);
    __ Mov(w0, x);
    __ CallRuntime(test_uint8_t);
    __ Uxtb(w0, w0);
    __ Cmp(w0, ExtractUnsignedBitfield32(7, 0, x));
    __ Cinc(x24, x24, ne);
    __ Mov(w0, x);
    __ CallRuntime(test_int16_t);
    __ Sxth(w0, w0);
    __ Cmp(w0, ExtractSignedBitfield32(15, 0, x));
    __ Cinc(x24, x24, ne);
    __ Mov(w0, x);
    __ CallRuntime(test_uint16_t);
    __ Uxth(w0, w0);
    __ Cmp(w0, ExtractUnsignedBitfield32(15, 0, x));
    __ Cinc(x24, x24, ne);
  }


  int64_t value = 0xbadbeef;
  __ Mov(x0, reinterpret_cast<uint64_t>(&value));
  __ CallRuntime(runtime_call_store_at_address);

  __ Mov(w0, 0);
  __ CallRuntime(runtime_call_no_args);
  __ Mov(w25, w0);

  END();

#if defined(VIXL_HAS_SIMULATED_RUNTIME_CALL_SUPPORT) || \
    !defined(VIXL_INCLUDE_SIMULATOR_AARCH64)
  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(1, w20);
    ASSERT_EQUAL_FP64(4.0, d20);
    ASSERT_EQUAL_64(0x123, x21);
    ASSERT_EQUAL_FP64(310.0, d21);
    VIXL_CHECK(value == 0xf00d);
    ASSERT_EQUAL_64(0, x22);
    ASSERT_EQUAL_32(124, w23);
    ASSERT_EQUAL_64(0, x24);
    ASSERT_EQUAL_32(1, w25);
  }
#endif  // #if defined(VIXL_HAS_SIMULATED_RUNTIME_CALL_SUPPORT) || ...
}

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
void void_func() {}
uint32_t uint32_func() { return 2; }
void void_param_func(uint32_t x) { USE(x); }
uint32_t uint32_param_func(uint32_t x) { return ++x; }

void void_placeholder() {}
uint32_t uint32_placeholder() { return 4; }
void void_param_placeholder(uint32_t x) { USE(x); }
uint32_t uint32_param_placeholder(uint32_t x) { return ++x; }

#define DO_TEST_BRANCH_INTERCEPTION(func)        \
  __ Mov(x16, reinterpret_cast<uint64_t>(func)); \
  __ Blr(x16);

TEST(branch_interception) {
  SETUP();
  START();

  // Test default branch interception, i.e: do a runtime call to the function.
  DO_TEST_BRANCH_INTERCEPTION(void_func);
  DO_TEST_BRANCH_INTERCEPTION(uint32_func);
  __ Mov(w20, w0);
  DO_TEST_BRANCH_INTERCEPTION(void_param_func);
  __ Mov(w0, 2);
  DO_TEST_BRANCH_INTERCEPTION(uint32_param_func);
  __ Mov(w21, w0);

  // Test interceptions with callbacks.
  DO_TEST_BRANCH_INTERCEPTION(void_placeholder);
  __ Mov(w22, w0);
  DO_TEST_BRANCH_INTERCEPTION(uint32_placeholder);
  __ Mov(w23, w0);
  __ Mov(w0, 4);
  DO_TEST_BRANCH_INTERCEPTION(uint32_placeholder);
  __ Mov(w24, w0);
  DO_TEST_BRANCH_INTERCEPTION(uint32_placeholder);
  __ Mov(w25, w0);

  END();

  simulator.RegisterBranchInterception(void_func);
  simulator.RegisterBranchInterception(uint32_func);
  simulator.RegisterBranchInterception(void_param_func);
  simulator.RegisterBranchInterception(uint32_param_func);

  auto callback = [&simulator](uint64_t original_target) {
    USE(original_target);
    simulator.WriteWRegister(0, 1);
  };

  simulator.RegisterBranchInterception(void_placeholder, callback);
  simulator.RegisterBranchInterception(uint32_placeholder, callback);
  simulator.RegisterBranchInterception(void_param_placeholder, callback);
  simulator.RegisterBranchInterception(uint32_param_placeholder, callback);

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_32(2, w20);
    ASSERT_EQUAL_32(3, w21);
    ASSERT_EQUAL_32(1, w22);
    ASSERT_EQUAL_32(1, w23);
    ASSERT_EQUAL_32(1, w24);
    ASSERT_EQUAL_32(1, w25);
  }
}
#endif  // #ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
#endif  // #ifdef VIXL_HAS_MACROASSEMBLER_RUNTIME_CALL_SUPPORT


TEST(optimised_mov_register) {
  SETUP();

  START();
  Label start;
  __ Bind(&start);
  __ Mov(x0, x0);
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&start) == 0);
  __ Mov(w0, w0, kDiscardForSameWReg);
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&start) == 0);
  __ Mov(w0, w0);
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&start) == kInstructionSize);

  END();

  if (CAN_RUN()) {
    RUN();
  }
}


TEST(nop) {
  MacroAssembler masm;

  Label start;
  __ Bind(&start);
  __ Nop();
  // `MacroAssembler::Nop` must generate at least one nop.
  VIXL_CHECK(masm.GetSizeOfCodeGeneratedSince(&start) >= kInstructionSize);

  masm.FinalizeCode();
}


TEST(mte_addg_subg) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);

  START();
  __ Mov(x0, 0x5555000055555555);

  // Add/subtract an address offset, changing tag each time.
  __ Addg(x1, x0, 16, 2);
  __ Subg(x2, x1, 16, 1);

  // Add/subtract address offsets, keep tag.
  __ Addg(x3, x0, 1008, 0);
  __ Subg(x4, x3, 1008, 0);

  // Change tag only. Check wraparound.
  __ Addg(x5, x0, 0, 15);
  __ Subg(x6, x0, 0, 14);

  // Do nothing.
  __ Addg(x7, x0, 0, 0);
  __ Subg(x8, x0, 0, 0);

  // Use stack pointer as source/destination.
  __ Mov(x20, sp);  // Store original sp.

  __ Subg(sp, sp, 32, 0);  // Claim 32 bytes.
  __ Sub(x9, sp, x20);     // Subtract original sp and store difference.

  __ Mov(sp, x20);  // Restore original sp.
  __ Claim(32);
  __ Addg(sp, sp, 32, 0);  // Drop 32 bytes.
  __ Sub(x10, sp, x20);    // Subtract original sp and store difference.

  __ Mov(sp, x20);        // Restore sp (should be no-op)
  __ Addg(sp, sp, 0, 1);  // Tag the sp.
  __ Sub(x11, sp, x20);  // Subtract original sp and store for later comparison.
  __ Mov(sp, x20);       // Restore sp.

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0x5755000055555565, x1);
    ASSERT_EQUAL_64(0x5855000055555555, x2);
    ASSERT_EQUAL_64(0x5555000055555945, x3);
    ASSERT_EQUAL_64(0x5555000055555555, x4);
    ASSERT_EQUAL_64(0x5455000055555555, x5);
    ASSERT_EQUAL_64(0x5355000055555555, x6);
    ASSERT_EQUAL_64(0x5555000055555555, x7);
    ASSERT_EQUAL_64(0x5555000055555555, x8);
    ASSERT_EQUAL_64(-32, x9);
    ASSERT_EQUAL_64(0, x10);
    ASSERT_EQUAL_64(UINT64_C(1) << 56, x11);
  }
}

TEST(mte_subp) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);

  START();
  __ Mov(x0, 0x5555555555555555);
  __ Mov(x1, -42);

  // Test subp with equivalent sbfx/sub(s) operations.
  __ Sbfx(x10, x0, 0, 56);
  __ Sbfx(x11, x1, 0, 56);

  __ Subp(x4, x0, x1);
  __ Sub(x5, x10, x11);

  __ Subp(x6, x1, x0);
  __ Sub(x7, x11, x10);

  __ Subps(x8, x0, x1);
  __ Mrs(x18, NZCV);
  __ Subs(x9, x10, x11);
  __ Mrs(x19, NZCV);

  __ Cmpp(x1, x0);
  __ Mrs(x20, NZCV);
  __ Cmp(x11, x10);
  __ Mrs(x21, NZCV);

  // Test equal pointers with mismatched tags compare equal and produce a zero
  // difference with subps.
  __ Mov(x2, 0x20);  // Exclude tag 5.
  __ Irg(x3, x0, x2);
  __ Subps(x22, x0, x3);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(x5, x4);
    ASSERT_EQUAL_64(x7, x6);
    ASSERT_EQUAL_64(x9, x8);
    ASSERT_EQUAL_64(x19, x18);
    ASSERT_EQUAL_64(x20, x21);
    ASSERT_EQUAL_64(0, x22);
    ASSERT_EQUAL_NZCV(ZCFlag);
  }
}

TEST(mte_gmi) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);

  START();
  __ Mov(x0, 0xaaaa);
  __ Mov(x20, 0x12345678);

  __ Gmi(x0, x20, x0);  // Add mask bit 0.
  __ Addg(x20, x20, 0, 1);
  __ Gmi(x1, x20, x0);  // No effect.
  __ Addg(x20, x20, 0, 1);
  __ Gmi(x2, x20, x1);  // Add mask bit 2.
  __ Addg(x20, x20, 0, 1);
  __ Gmi(x3, x20, x2);  // No effect.
  __ Addg(x20, x20, 0, 1);
  __ Gmi(x4, x20, x3);  // Add mask bit 4.
  __ Addg(x20, x20, 0, 1);
  __ Gmi(x5, x20, x4);  // No effect.
  __ Addg(x20, x20, 0, 9);
  __ Gmi(x6, x20, x5);   // Add mask bit 14.
  __ Gmi(x7, x20, xzr);  // Only mask bit 14.
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0xaaab, x0);
    ASSERT_EQUAL_64(0xaaab, x1);
    ASSERT_EQUAL_64(0xaaaf, x2);
    ASSERT_EQUAL_64(0xaaaf, x3);
    ASSERT_EQUAL_64(0xaabf, x4);
    ASSERT_EQUAL_64(0xaabf, x5);
    ASSERT_EQUAL_64(0xeabf, x6);
    ASSERT_EQUAL_64(0x4000, x7);
  }
}

TEST(mte_irg) {
  SETUP_WITH_FEATURES(CPUFeatures::kMTE);

  START();
  __ Mov(x10, 8);
  __ Mov(x0, 0x5555555555555555);
  // Insert a random tag repeatedly. If the loop doesn't exit in the expected
  // way, it's statistically likely that a random tag was never inserted.
  Label loop, failed, done;
  __ Bind(&loop);
  __ Irg(x1, x0);
  __ Sub(x10, x10, 1);
  __ Cbz(x10, &failed);  // Exit if loop count exceeded.
  __ Cmp(x1, 0x5555555555555555);
  __ B(eq, &loop);  // Loop if the tag hasn't changed.

  // Check non-tag bits have not changed.
  __ Bic(x1, x1, 0x0f00000000000000);
  __ Subs(x1, x1, 0x5055555555555555);
  __ B(&done);

  __ Bind(&failed);
  __ Mov(x1, 1);

  __ Bind(&done);

  // Insert random tags, excluding oddly-numbered tags, and set a bit in a
  // result register for each tag used.
  // After 128 rounds, it's statistically likely that all even bits in the
  // least-significant half word will be set.
  __ Mov(x3, 0);
  __ Mov(x4, 1);
  __ Mov(x10, 128);
  __ Mov(x11, 0xaaaa);

  Label loop2;
  __ Bind(&loop2);
  __ Irg(x2, x1, x11);
  __ Lsr(x2, x2, 56);
  __ Lsl(x2, x4, x2);
  __ Orr(x3, x3, x2);
  __ Subs(x10, x10, 1);
  __ B(ne, &loop2);
  __ Mov(x2, x3);

  // Check that excluding all tags results in zero tag insertion.
  __ Mov(x3, 0xffffffffffffffff);
  __ Irg(x3, x3, x3);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x1);
    ASSERT_EQUAL_64(0x5555, x2);
    ASSERT_EQUAL_64(0xf0ffffffffffffff, x3);
  }
}

TEST(mops_set) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS);

  uint8_t dst[16];
  memset(dst, 0x55, ArrayLength(dst));
  uintptr_t dst_addr = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x0, dst_addr);
  __ Add(x1, x0, 1);
  __ Mov(x2, 13);
  __ Mov(x3, 0x1234aa);

  // Set 13 bytes dst[1] onwards to 0xaa.
  __ Setp(x1, x2, x3);
  __ Setm(x1, x2, x3);
  __ Sete(x1, x2, x3);
  __ Mrs(x20, NZCV);

  // x2 is now zero, so this should do nothing.
  __ Setp(x1, x2, x3);
  __ Setm(x1, x2, x3);
  __ Sete(x1, x2, x3);
  __ Mrs(x21, NZCV);

  // Set dst[15] to zero using the masm helper.
  __ Add(x1, x0, 15);
  __ Mov(x2, 1);
  __ Set(x1, x2, xzr);
  __ Mrs(x22, NZCV);

  // Load dst for comparison.
  __ Ldp(x10, x11, MemOperand(x0));
  END();

  if (CAN_RUN()) {
    // Permitted results:
    //            NZCV    Xd                Xn
    //  Option A: ....    end of buffer     0
    //  Option B: ..C.    end of buffer     0

    std::vector<uint64_t> allowed_flags = {NoFlag, CFlag};

    RUN();
    ASSERT_EQUAL_64(allowed_flags, x20);
    ASSERT_EQUAL_64(allowed_flags, x21);
    ASSERT_EQUAL_64(allowed_flags, x22);
    ASSERT_EQUAL_64(dst_addr + 16, x1);
    ASSERT_EQUAL_64(0, x2);
    ASSERT_EQUAL_64(0x1234aa, x3);
    ASSERT_EQUAL_64(0xaaaa'aaaa'aaaa'aa55, x10);
    ASSERT_EQUAL_64(0x0055'aaaa'aaaa'aaaa, x11);
  }
}

TEST(mops_setn) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS);

  // In simulation, non-temporal set is handled by the same code as normal set,
  // so only a basic test is required beyond that already provided above.

  uint8_t dst[16] = {0x55};
  uintptr_t dst_addr = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x0, dst_addr);
  __ Mov(x1, x0);
  __ Mov(x2, 16);
  __ Mov(x3, 0x42);
  __ Setn(x1, x2, x3);
  __ Mrs(x20, NZCV);
  __ Ldp(x10, x11, MemOperand(x0));
  END();

  if (CAN_RUN()) {
    // Permitted results:
    //            NZCV    Xd                Xn
    //  Option A: ....    end of buffer     0
    //  Option B: ..C.    end of buffer     0

    std::vector<uint64_t> allowed_flags = {NoFlag, CFlag};

    RUN();
    ASSERT_EQUAL_64(allowed_flags, x20);
    ASSERT_EQUAL_64(dst_addr + 16, x1);
    ASSERT_EQUAL_64(0, x2);
    ASSERT_EQUAL_64(0x42, x3);
    ASSERT_EQUAL_64(0x4242'4242'4242'4242, x10);
    ASSERT_EQUAL_64(0x4242'4242'4242'4242, x11);
  }
}

TEST(mops_setg) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS, CPUFeatures::kMTE);

  uint8_t* dst = nullptr;
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  const int dst_size = 32;
  dst = reinterpret_cast<uint8_t*>(
      simulator.Mmap(NULL,
                     dst_size * sizeof(uint8_t),
                     PROT_READ | PROT_WRITE | PROT_MTE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1,
                     0));

  VIXL_ASSERT(dst != nullptr);
  uint8_t* untagged_ptr = AddressUntag(dst);
  memset(untagged_ptr, 0xc9, dst_size);
#else
// TODO: Port the memory allocation to work on MTE supported platform natively.
// Note that `CAN_RUN` prevents running in MTE-unsupported environments.
#endif

  uintptr_t dst_addr = reinterpret_cast<uintptr_t>(dst);
  uint64_t tag_mask = 0xf0ff'ffff'ffff'ffff;

  START();
  __ Mov(x0, dst_addr);
  __ Gmi(x2, x0, xzr);
  __ Irg(x1, x0, x2);  // Choose new tag for setg destination.
  __ Mov(x2, 16);
  __ Mov(x3, 0x42);
  __ Setg(x1, x2, x3);
  __ Mrs(x20, NZCV);

  __ Ubfx(x4, x1, 56, 4);  // Extract new tag.
  __ Bfi(x0, x4, 56, 4);   // Tag dst_addr so set region can be loaded.
  __ Ldp(x10, x11, MemOperand(x0));

  __ Mov(x0, dst_addr);
  __ Ldp(x12, x13, MemOperand(x0, 16));  // Unset region has original tag.

  __ And(x1, x1, tag_mask);  // Strip tag for repeatable checks.
  END();

  if (CAN_RUN()) {
    // Permitted results:
    //            NZCV    Xd                Xn
    //  Option A: ....    end of buffer     0
    //  Option B: ..C.    end of buffer     0

    std::vector<uint64_t> allowed_flags = {NoFlag, CFlag};

    RUN();
    ASSERT_EQUAL_64(allowed_flags, x20);
    ASSERT_EQUAL_64((dst_addr & tag_mask) + 16, x1);
    ASSERT_EQUAL_64(0, x2);
    ASSERT_EQUAL_64(0x42, x3);
    ASSERT_EQUAL_64(0x4242'4242'4242'4242, x10);
    ASSERT_EQUAL_64(0x4242'4242'4242'4242, x11);
    ASSERT_EQUAL_64(0xc9c9'c9c9'c9c9'c9c9, x12);
    ASSERT_EQUAL_64(0xc9c9'c9c9'c9c9'c9c9, x13);
  }

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  simulator.Munmap(dst, dst_size, PROT_MTE);
#endif
}

TEST(mops_cpy) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS);

  uint8_t buf[16];
  uintptr_t buf_addr = reinterpret_cast<uintptr_t>(buf);

  for (unsigned i = 0; i < ArrayLength(buf); i++) {
    buf[i] = i;
  }

  START();
  __ Mov(x0, buf_addr);

  // Copy first eight bytes into second eight.
  __ Mov(x1, x0);     // src = &buf[0]
  __ Add(x2, x0, 8);  // dst = &buf[8]
  __ Mov(x3, 8);      // count = 8
  __ Cpyp(x2, x1, x3);
  __ Cpym(x2, x1, x3);
  __ Cpye(x2, x1, x3);
  __ Ldp(x10, x11, MemOperand(x0));
  __ Mrs(x20, NZCV);

  // Copy first eight bytes to overlapping offset, forcing backwards copy.
  __ Mov(x4, x0);     // src = &buf[0]
  __ Add(x5, x0, 4);  // dst = &buf[4]
  __ Mov(x6, 8);      // count = 8
  __ Cpy(x5, x4, x6);
  __ Ldp(x12, x13, MemOperand(x0));
  __ Mrs(x21, NZCV);

  // Copy last eight bytes to overlapping offset, forcing forwards copy.
  __ Add(x7, x0, 8);  // src = &buf[8]
  __ Add(x8, x0, 6);  // dst = &buf[6]
  __ Mov(x9, 8);      // count = 8
  __ Cpy(x8, x7, x9);
  __ Ldp(x14, x15, MemOperand(x0));
  __ Mrs(x22, NZCV);
  END();

  if (CAN_RUN()) {
    // Permitted results:
    //                        NZCV    Xs/Xd               Xn
    //  Option A (forwards) : ....    ends of buffers     0
    //  Option A (backwards): ....    starts of buffers   0
    //  Option B (forwards) : ..C.    ends of buffers     0
    //  Option B (backwards): N.C.    starts of buffers   0

    std::vector<uint64_t> allowed_backwards_flags = {NoFlag, NCFlag};
    std::vector<uint64_t> allowed_forwards_flags = {NoFlag, CFlag};

    RUN();
    // IMPLEMENTATION DEFINED direction
    if (static_cast<uintptr_t>(core.xreg(2)) > buf_addr) {
      // Forwards
      ASSERT_EQUAL_64(buf_addr + 8, x1);
      ASSERT_EQUAL_64(buf_addr + 16, x2);
      ASSERT_EQUAL_64(allowed_forwards_flags, x20);
    } else {
      // Backwards
      ASSERT_EQUAL_64(buf_addr, x1);
      ASSERT_EQUAL_64(buf_addr + 8, x2);
      ASSERT_EQUAL_64(allowed_backwards_flags, x20);
    }
    ASSERT_EQUAL_64(0, x3);  // Xn
    ASSERT_EQUAL_64(0x0706'0504'0302'0100, x10);
    ASSERT_EQUAL_64(0x0706'0504'0302'0100, x11);

    ASSERT_EQUAL_64(buf_addr, x4);      // Xs
    ASSERT_EQUAL_64(buf_addr + 4, x5);  // Xd
    ASSERT_EQUAL_64(0, x6);             // Xn
    ASSERT_EQUAL_64(0x0302'0100'0302'0100, x12);
    ASSERT_EQUAL_64(0x0706'0504'0706'0504, x13);
    ASSERT_EQUAL_64(allowed_backwards_flags, x21);

    ASSERT_EQUAL_64(buf_addr + 16, x7);  // Xs
    ASSERT_EQUAL_64(buf_addr + 14, x8);  // Xd
    ASSERT_EQUAL_64(0, x9);              // Xn
    ASSERT_EQUAL_64(0x0504'0100'0302'0100, x14);
    ASSERT_EQUAL_64(0x0706'0706'0504'0706, x15);
    ASSERT_EQUAL_64(allowed_forwards_flags, x22);
  }
}

TEST(mops_cpyn) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS);

  // In simulation, non-temporal cpy is handled by the same code as normal cpy,
  // so only a basic test is required beyond that already provided above.

  uint8_t buf[16];
  uintptr_t buf_addr = reinterpret_cast<uintptr_t>(buf);

  for (unsigned i = 0; i < ArrayLength(buf); i++) {
    buf[i] = i;
  }

  START();
  __ Mov(x0, buf_addr);

  __ Add(x1, x0, 1);  // src = &buf[1]
  __ Mov(x2, x0);     // dst = &buf[0]
  __ Mov(x3, 15);     // count = 15
  __ Cpyn(x2, x1, x3);
  __ Ldp(x10, x11, MemOperand(x0));
  __ Mrs(x20, NZCV);

  __ Add(x4, x0, 1);  // src = &buf[1]
  __ Mov(x5, x0);     // dst = &buf[0]
  __ Mov(x6, 15);     // count = 15
  __ Cpyrn(x5, x4, x6);
  __ Ldp(x12, x13, MemOperand(x0));
  __ Mrs(x21, NZCV);

  __ Add(x7, x0, 1);  // src = &buf[1]
  __ Mov(x8, x0);     // dst = &buf[0]
  __ Mov(x9, 15);     // count = 15
  __ Cpywn(x8, x7, x9);
  __ Ldp(x14, x15, MemOperand(x0));
  __ Mrs(x22, NZCV);
  END();

  if (CAN_RUN()) {
    // Permitted results:
    //                        NZCV    Xs/Xd               Xn
    //  Option A (forwards) : ....    ends of buffers     0
    //  Option A (backwards): ....    starts of buffers   0
    //  Option B (forwards) : ..C.    ends of buffers     0
    //  Option B (backwards): N.C.    starts of buffers   0
    //
    // All cases overlap to force a forwards copy.

    std::vector<uint64_t> allowed_forwards_flags = {NoFlag, CFlag};

    RUN();
    ASSERT_EQUAL_64(buf_addr + 16, x1);  // Xs
    ASSERT_EQUAL_64(buf_addr + 15, x2);  // Xd
    ASSERT_EQUAL_64(0, x3);              // Xn
    ASSERT_EQUAL_64(allowed_forwards_flags, x20);
    ASSERT_EQUAL_64(0x0807'0605'0403'0201, x10);
    ASSERT_EQUAL_64(0x0f0f'0e0d'0c0b'0a09, x11);

    ASSERT_EQUAL_64(buf_addr + 16, x4);  // Xs
    ASSERT_EQUAL_64(buf_addr + 15, x5);  // Xd
    ASSERT_EQUAL_64(0, x6);              // Xn
    ASSERT_EQUAL_64(allowed_forwards_flags, x21);
    ASSERT_EQUAL_64(0x0908'0706'0504'0302, x12);
    ASSERT_EQUAL_64(0x0f0f'0f0e'0d0c'0b0a, x13);

    ASSERT_EQUAL_64(buf_addr + 16, x7);  // Xs
    ASSERT_EQUAL_64(buf_addr + 15, x8);  // Xd
    ASSERT_EQUAL_64(0, x9);              // Xn
    ASSERT_EQUAL_64(allowed_forwards_flags, x22);
    ASSERT_EQUAL_64(0x0a09'0807'0605'0403, x14);
    ASSERT_EQUAL_64(0x0f0f'0f0f'0e0d'0c0b, x15);
  }
}

TEST(mops_cpyf) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS);

  uint8_t buf[16];
  uintptr_t buf_addr = reinterpret_cast<uintptr_t>(buf);

  for (unsigned i = 0; i < ArrayLength(buf); i++) {
    buf[i] = i;
  }

  // As `mops_cpy`, but `cpyf` always copies forwards, so is only useful for
  // non-overlapping buffers, or those where the source address is greater than
  // the destination address.

  START();
  __ Mov(x0, buf_addr);

  // Copy first eight bytes into second eight, without overlap.
  __ Mov(x1, x0);     // src = &buf[0]
  __ Add(x2, x0, 8);  // dst = &buf[8]
  __ Mov(x3, 8);      // count = 8
  __ Cpyfp(x2, x1, x3);
  __ Cpyfm(x2, x1, x3);
  __ Cpyfe(x2, x1, x3);
  __ Ldp(x10, x11, MemOperand(x0));
  __ Mrs(x20, NZCV);

  // Copy last eight bytes to overlapping offset where src < dst.
  __ Add(x4, x0, 8);  // src = &buf[8]
  __ Add(x5, x0, 6);  // dst = &buf[6]
  __ Mov(x6, 8);      // count = 8
  __ Cpyf(x5, x4, x6);
  __ Ldp(x12, x13, MemOperand(x0));
  __ Mrs(x21, NZCV);

  // Copy first eight bytes to overlapping offset where src > dst.
  __ Mov(x7, x0);     // src = &buf[0]
  __ Add(x8, x0, 4);  // dst = &buf[4]
  __ Mov(x9, 8);      // count = 8
  __ Cpyf(x8, x7, x9);
  // The only testable result is the first and last four bytes, which are not
  // written at all.
  __ Ldr(w14, MemOperand(x0));
  __ Ldr(w15, MemOperand(x0, 12));
  __ Mrs(x22, NZCV);

  END();

  if (CAN_RUN()) {
    // Permitted results:
    //            NZCV    Xs/Xd               Xn
    //  Option A: ....    ends of buffers     0
    //  Option B: ..C.    ends of buffers     0

    std::vector<uint64_t> allowed_forwards_flags = {NoFlag, CFlag};

    RUN();

    // No overlap.
    ASSERT_EQUAL_64(buf_addr + 8, x1);   // Xs
    ASSERT_EQUAL_64(buf_addr + 16, x2);  // Xd
    ASSERT_EQUAL_64(0, x3);              // Xn
    ASSERT_EQUAL_64(allowed_forwards_flags, x20);
    ASSERT_EQUAL_64(0x0706'0504'0302'0100, x10);
    ASSERT_EQUAL_64(0x0706'0504'0302'0100, x11);

    // Overlap, src > dst.
    ASSERT_EQUAL_64(buf_addr + 16, x4);  // Xs
    ASSERT_EQUAL_64(buf_addr + 14, x5);  // Xd
    ASSERT_EQUAL_64(0, x6);              // Xn
    ASSERT_EQUAL_64(0x0100'0504'0302'0100, x12);
    ASSERT_EQUAL_64(0x0706'0706'0504'0302, x13);
    ASSERT_EQUAL_64(allowed_forwards_flags, x21);

    // Overlap, src < dst.
    ASSERT_EQUAL_64(buf_addr + 8, x7);   // Xs
    ASSERT_EQUAL_64(buf_addr + 12, x8);  // Xd
    ASSERT_EQUAL_64(0, x9);              // Xn
    // We can only reliably test that the operation didn't write outside the
    // specified region.
    ASSERT_EQUAL_32(0x0302'0100, w14);
    ASSERT_EQUAL_32(0x0706'0706, w15);
    ASSERT_EQUAL_64(allowed_forwards_flags, x22);
  }
}

TEST(mops_cpyfn) {
  SETUP_WITH_FEATURES(CPUFeatures::kMOPS);

  // In simulation, non-temporal cpy is handled by the same code as normal cpy,
  // so only a basic test is required beyond that already provided above.

  uint8_t buf[16];
  uintptr_t buf_addr = reinterpret_cast<uintptr_t>(buf);

  for (unsigned i = 0; i < ArrayLength(buf); i++) {
    buf[i] = i;
  }

  START();
  __ Mov(x0, buf_addr);

  __ Add(x1, x0, 1);  // src = &buf[1]
  __ Mov(x2, x0);     // dst = &buf[0]
  __ Mov(x3, 15);     // count = 15
  __ Cpyfn(x2, x1, x3);
  __ Ldp(x10, x11, MemOperand(x0));
  __ Mrs(x20, NZCV);

  __ Add(x4, x0, 1);  // src = &buf[1]
  __ Mov(x5, x0);     // dst = &buf[0]
  __ Mov(x6, 15);     // count = 15
  __ Cpyfrn(x5, x4, x6);
  __ Ldp(x12, x13, MemOperand(x0));
  __ Mrs(x21, NZCV);

  __ Add(x7, x0, 1);  // src = &buf[1]
  __ Mov(x8, x0);     // dst = &buf[0]
  __ Mov(x9, 15);     // count = 15
  __ Cpyfwn(x8, x7, x9);
  __ Ldp(x14, x15, MemOperand(x0));
  __ Mrs(x22, NZCV);
  END();

  if (CAN_RUN()) {
    // Permitted results:
    //            NZCV    Xs/Xd               Xn
    //  Option A: ....    ends of buffers     0
    //  Option B: ..C.    ends of buffers     0

    std::vector<uint64_t> allowed_flags = {NoFlag, CFlag};

    RUN();
    ASSERT_EQUAL_64(buf_addr + 16, x1);  // Xs
    ASSERT_EQUAL_64(buf_addr + 15, x2);  // Xd
    ASSERT_EQUAL_64(0, x3);              // Xn
    ASSERT_EQUAL_64(allowed_flags, x20);
    ASSERT_EQUAL_64(0x0807'0605'0403'0201, x10);
    ASSERT_EQUAL_64(0x0f0f'0e0d'0c0b'0a09, x11);

    ASSERT_EQUAL_64(buf_addr + 16, x4);  // Xs
    ASSERT_EQUAL_64(buf_addr + 15, x5);  // Xd
    ASSERT_EQUAL_64(0, x6);              // Xn
    ASSERT_EQUAL_64(allowed_flags, x21);
    ASSERT_EQUAL_64(0x0908'0706'0504'0302, x12);
    ASSERT_EQUAL_64(0x0f0f'0f0e'0d0c'0b0a, x13);

    ASSERT_EQUAL_64(buf_addr + 16, x7);  // Xs
    ASSERT_EQUAL_64(buf_addr + 15, x8);  // Xd
    ASSERT_EQUAL_64(0, x9);              // Xn
    ASSERT_EQUAL_64(allowed_flags, x22);
    ASSERT_EQUAL_64(0x0a09'0807'0605'0403, x14);
    ASSERT_EQUAL_64(0x0f0f'0f0f'0e0d'0c0b, x15);
  }
}

TEST(cssc_abs) {
  SETUP_WITH_FEATURES(CPUFeatures::kCSSC);

  START();
  __ Mov(x0, -1);
  __ Mov(x1, 1);
  __ Mov(x2, 0);
  __ Mov(x3, 0x7fff'ffff);
  __ Mov(x4, 0x8000'0000);
  __ Mov(x5, 0x8000'0001);
  __ Mov(x6, 0x7fff'ffff'ffff'ffff);
  __ Mov(x7, 0x8000'0000'0000'0000);
  __ Mov(x8, 0x8000'0000'0000'0001);

  __ Abs(w10, w0);
  __ Abs(x11, x0);
  __ Abs(w12, w1);
  __ Abs(x13, x1);
  __ Abs(w14, w2);
  __ Abs(x15, x2);

  __ Abs(w19, w3);
  __ Abs(x20, x3);
  __ Abs(w21, w4);
  __ Abs(x22, x4);
  __ Abs(w23, w5);
  __ Abs(x24, x5);
  __ Abs(w25, w6);
  __ Abs(x26, x6);
  __ Abs(w27, w7);
  __ Abs(x28, x7);
  __ Abs(w29, w8);
  __ Abs(x30, x8);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(1, x10);
    ASSERT_EQUAL_64(1, x11);
    ASSERT_EQUAL_64(1, x12);
    ASSERT_EQUAL_64(1, x13);
    ASSERT_EQUAL_64(0, x14);
    ASSERT_EQUAL_64(0, x15);
    ASSERT_EQUAL_64(0x7fff'ffff, x19);
    ASSERT_EQUAL_64(0x7fff'ffff, x20);
    ASSERT_EQUAL_64(0x8000'0000, x21);
    ASSERT_EQUAL_64(0x8000'0000, x22);
    ASSERT_EQUAL_64(0x7fff'ffff, x23);
    ASSERT_EQUAL_64(0x8000'0001, x24);
    ASSERT_EQUAL_64(1, x25);
    ASSERT_EQUAL_64(0x7fff'ffff'ffff'ffff, x26);
    ASSERT_EQUAL_64(0, x27);
    ASSERT_EQUAL_64(0x8000'0000'0000'0000, x28);
    ASSERT_EQUAL_64(1, x29);
    ASSERT_EQUAL_64(0x7fff'ffff'ffff'ffff, x30);
  }
}

TEST(cssc_cnt) {
  SETUP_WITH_FEATURES(CPUFeatures::kCSSC);

  START();
  __ Mov(x0, -1);
  __ Mov(x1, 1);
  __ Mov(x2, 0);
  __ Mov(x3, 0x7fff'ffff);
  __ Mov(x4, 0x8000'0000);
  __ Mov(x5, 0x8000'0001);
  __ Mov(x6, 0x7fff'ffff'ffff'ffff);
  __ Mov(x7, 0x4242'4242'aaaa'aaaa);

  __ Cnt(w10, w0);
  __ Cnt(x11, x0);
  __ Cnt(w12, w1);
  __ Cnt(x13, x1);
  __ Cnt(w14, w2);
  __ Cnt(x15, x2);
  __ Cnt(w19, w3);
  __ Cnt(x20, x3);
  __ Cnt(w21, w4);
  __ Cnt(x22, x4);
  __ Cnt(w23, w5);
  __ Cnt(x24, x5);
  __ Cnt(w25, w6);
  __ Cnt(x26, x6);
  __ Cnt(w27, w7);
  __ Cnt(x28, x7);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(32, x10);
    ASSERT_EQUAL_64(64, x11);
    ASSERT_EQUAL_64(1, x12);
    ASSERT_EQUAL_64(1, x13);
    ASSERT_EQUAL_64(0, x14);
    ASSERT_EQUAL_64(0, x15);
    ASSERT_EQUAL_64(31, x19);
    ASSERT_EQUAL_64(31, x20);
    ASSERT_EQUAL_64(1, x21);
    ASSERT_EQUAL_64(1, x22);
    ASSERT_EQUAL_64(2, x23);
    ASSERT_EQUAL_64(2, x24);
    ASSERT_EQUAL_64(32, x25);
    ASSERT_EQUAL_64(63, x26);
    ASSERT_EQUAL_64(16, x27);
    ASSERT_EQUAL_64(24, x28);
  }
}

TEST(cssc_ctz) {
  SETUP_WITH_FEATURES(CPUFeatures::kCSSC);

  START();
  __ Mov(x0, -1);
  __ Mov(x1, 1);
  __ Mov(x2, 2);
  __ Mov(x3, 0x7fff'ff00);
  __ Mov(x4, 0x8000'4000);
  __ Mov(x5, 0x4000'0001);
  __ Mov(x6, 0x0000'0001'0000'0000);
  __ Mov(x7, 0x4200'0000'0000'0000);

  __ Ctz(w10, w0);
  __ Ctz(x11, x0);
  __ Ctz(w12, w1);
  __ Ctz(x13, x1);
  __ Ctz(w14, w2);
  __ Ctz(x15, x2);
  __ Ctz(w19, w3);
  __ Ctz(x20, x3);
  __ Ctz(w21, w4);
  __ Ctz(x22, x4);
  __ Ctz(w23, w5);
  __ Ctz(x24, x5);
  __ Ctz(w25, w6);
  __ Ctz(x26, x6);
  __ Ctz(w27, w7);
  __ Ctz(x28, x7);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x10);
    ASSERT_EQUAL_64(0, x11);
    ASSERT_EQUAL_64(0, x12);
    ASSERT_EQUAL_64(0, x13);
    ASSERT_EQUAL_64(1, x14);
    ASSERT_EQUAL_64(1, x15);
    ASSERT_EQUAL_64(8, x19);
    ASSERT_EQUAL_64(8, x20);
    ASSERT_EQUAL_64(14, x21);
    ASSERT_EQUAL_64(14, x22);
    ASSERT_EQUAL_64(0, x23);
    ASSERT_EQUAL_64(0, x24);
    ASSERT_EQUAL_64(32, x25);
    ASSERT_EQUAL_64(32, x26);
    ASSERT_EQUAL_64(32, x27);
    ASSERT_EQUAL_64(57, x28);
  }
}

using MinMaxOp = void (MacroAssembler::*)(const Register&,
                                          const Register&,
                                          const Operand&);

static void MinMaxHelper(MinMaxOp op,
                         bool is_signed,
                         uint64_t a,
                         uint64_t b,
                         uint32_t wexp,
                         uint64_t xexp) {
  SETUP_WITH_FEATURES(CPUFeatures::kCSSC);

  START();
  __ Mov(x0, a);
  __ Mov(x1, b);
  if ((is_signed && IsInt8(b)) || (!is_signed && IsUint8(b))) {
    (masm.*op)(w10, w0, b);
    (masm.*op)(x11, x0, b);
  } else {
    (masm.*op)(w10, w0, w1);
    (masm.*op)(x11, x0, x1);
  }
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(wexp, x10);
    ASSERT_EQUAL_64(xexp, x11);
  }
}

TEST(cssc_umin) {
  MinMaxOp op = &MacroAssembler::Umin;
  uint32_t s32min = 0x8000'0000;
  uint32_t s32max = 0x7fff'ffff;
  uint64_t s64min = 0x8000'0000'0000'0000;
  uint64_t s64max = 0x7fff'ffff'ffff'ffff;

  MinMaxHelper(op, false, 0, 0, 0, 0);
  MinMaxHelper(op, false, 128, 255, 128, 128);
  MinMaxHelper(op, false, 0, 0xffff'ffff'ffff'ffff, 0, 0);
  MinMaxHelper(op, false, s32max, s32min, s32max, s32max);
  MinMaxHelper(op, false, s32min, s32max, s32max, s32max);
  MinMaxHelper(op, false, s64max, s32min, s32min, s32min);
  MinMaxHelper(op, false, s64min, s64max, 0, s64max);
}

TEST(cssc_umax) {
  MinMaxOp op = &MacroAssembler::Umax;
  uint32_t s32min = 0x8000'0000;
  uint32_t s32max = 0x7fff'ffff;
  uint64_t s64min = 0x8000'0000'0000'0000;
  uint64_t s64max = 0x7fff'ffff'ffff'ffff;

  MinMaxHelper(op, false, 0, 0, 0, 0);
  MinMaxHelper(op, false, 128, 255, 255, 255);
  MinMaxHelper(op,
               false,
               0,
               0xffff'ffff'ffff'ffff,
               0xffff'ffff,
               0xffff'ffff'ffff'ffff);
  MinMaxHelper(op, false, s32max, s32min, s32min, s32min);
  MinMaxHelper(op, false, s32min, s32max, s32min, s32min);
  MinMaxHelper(op, false, s64max, s32min, 0xffff'ffff, s64max);
  MinMaxHelper(op, false, s64min, s64max, 0xffff'ffff, s64min);
}

TEST(cssc_smin) {
  MinMaxOp op = &MacroAssembler::Smin;
  uint32_t s32min = 0x8000'0000;
  uint32_t s32max = 0x7fff'ffff;
  uint64_t s64min = 0x8000'0000'0000'0000;
  uint64_t s64max = 0x7fff'ffff'ffff'ffff;

  MinMaxHelper(op, true, 0, 0, 0, 0);
  MinMaxHelper(op, true, 128, 255, 128, 128);
  MinMaxHelper(op,
               true,
               0,
               0xffff'ffff'ffff'ffff,
               0xffff'ffff,
               0xffff'ffff'ffff'ffff);
  MinMaxHelper(op, true, s32max, s32min, s32min, s32max);
  MinMaxHelper(op, true, s32min, s32max, s32min, s32max);
  MinMaxHelper(op, true, s64max, s32min, s32min, s32min);
  MinMaxHelper(op, true, s64min, s64max, 0xffff'ffff, s64min);
}

TEST(cssc_smax) {
  MinMaxOp op = &MacroAssembler::Smax;
  uint32_t s32min = 0x8000'0000;
  uint32_t s32max = 0x7fff'ffff;
  uint64_t s64min = 0x8000'0000'0000'0000;
  uint64_t s64max = 0x7fff'ffff'ffff'ffff;

  MinMaxHelper(op, true, 0, 0, 0, 0);
  MinMaxHelper(op, true, 128, 255, 255, 255);
  MinMaxHelper(op, true, 0, 0xffff'ffff'ffff'ffff, 0, 0);
  MinMaxHelper(op, true, s32max, s32min, s32max, s32min);
  MinMaxHelper(op, true, s32min, s32max, s32max, s32min);
  MinMaxHelper(op, true, s64max, s32min, 0xffff'ffff, s64max);
  MinMaxHelper(op, true, s64min, s64max, 0, s64max);
}

static void ChkfeatHelper(uint64_t initial,
                          uint64_t chkfeat,
                          CPUFeatures require) {
  SETUP_WITH_FEATURES(require);

  START();
  __ Mov(x16, initial);
  __ Chkfeat(x16);
  __ Mov(x0, x16);

  __ Mov(x1, initial);
  __ Chkfeat(x1);
  END();

  if (CAN_RUN()) {
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
    ASSERT_EQUAL_64(chkfeat, x0);
    ASSERT_EQUAL_64(x0, x1);
  }
}

TEST(chkfeat) { ChkfeatHelper(0x0, 0x0, CPUFeatures::None()); }

TEST(chkfeat_gcs) { ChkfeatHelper(0x1, 0x0, CPUFeatures::kGCS); }

TEST(chkfeat_unused) {
  // Bits 1-63 are reserved. This test ensures that they are unmodified by
  // `chkfeat`, but it will need to be updated if these bits are assigned in the
  // future.
  ChkfeatHelper(0xffff'ffff'ffff'fffe,
                0xffff'ffff'ffff'fffe,
                CPUFeatures::None());
}

TEST(gcs_feature_off) {
  SETUP();

  START();
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  simulator.DisableGCSCheck();
#else
// TODO: Disable GCS via operating system for this test, here and in the
// gcs_off_pac_on test below.
#endif
  __ Mov(x16, 0x0123'4567'89ab'cdef);
  __ Chkfeat(x16);

  // This sequence would fail with GCS enabled.
  Label lab, end;
  __ Bl(&lab);
  __ B(&end);

  __ Bind(&lab);
  __ Adr(lr, &end);
  __ Ret();

  __ Bind(&end);
  END();

  if (CAN_RUN()) {
    // TODO: This will currently fail on GCS-supporting hardware.
    RUN();
    ASSERT_EQUAL_64(0x0123'4567'89ab'cdef, x16);
  }
}

TEST(gcs_gcspushm) {
  SETUP_WITH_FEATURES(CPUFeatures::kGCS);

  Label ret;
  START();
  __ Adr(x0, &ret);
  __ Gcspushm(x0);
  __ Ret(x0);
  __ Nop();
  __ Bind(&ret);
  END();

  if (CAN_RUN()) {
    RUN();
  }
}

TEST(gcs_gcspopm) {
  SETUP_WITH_FEATURES(CPUFeatures::kGCS);

  Label lab, ret;
  START();
  __ Adr(x0, &ret);
  __ Bl(&lab);
  __ Bind(&ret);
  __ Nop();
  __ Bind(&lab);
  __ Gcspopm(x1);
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(x0, x1);
  }
}

TEST(gcs_gcsss1) {
  SETUP_WITH_FEATURES(CPUFeatures::kGCS);

  START();
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  uint64_t new_gcs = simulator.GetGCSManager().AllocateStack();
  __ Mov(x0, new_gcs);
#else
// TODO: Request new GCS from the operating system.
#endif

  // Partial stack swap to check GCS has changed, and a token is at the top
  // of the new stack.
  __ Gcsss1(x0);
  __ Gcspopm(x1);

  __ Bic(x0, x0, 7);  // Clear LSB of new GCS.
  __ Bic(x2, x1, 7);  // Clear LSB of old GCS.
  __ Cmp(x0, x2);
  __ Cset(x0, eq);
  __ And(x1, x1, 7);  // In progress token.
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(0, x0);  // GCS must not be equal.
    ASSERT_EQUAL_64(5, x1);  // In progress token must be present.
  }
}

// TODO: Add extra tests for combinations of PAC and GCS enabled.
TEST(gcs_stack_swap) {
  SETUP_WITH_FEATURES(CPUFeatures::kGCS);

  START();
  Label stack_swap, sub_fn, end;
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  uint64_t new_gcs = simulator.GetGCSManager().AllocateStack();
  __ Mov(x0, new_gcs);
#else
// TODO: Request new GCS from the operating system.
#endif
  __ Bl(&stack_swap);
  __ B(&end);

  __ Bind(&stack_swap);
  __ Gcsss1(x0);  // x0 = new GCS.
  __ Gcsss2(x1);  // x1 = old GCS.
  __ Mov(x29, lr);
  __ Bl(&sub_fn);
  __ Mov(lr, x29);
  __ Gcsss1(x1);  // Restore old GCS.
  __ Gcsss2(x0);
  __ Ret();

  __ Bind(&sub_fn);
  __ Mov(x2, 42);
  __ Ret();

  __ Bind(&end);
  END();

  if (CAN_RUN()) {
    RUN();
    ASSERT_EQUAL_64(42, x2);
  }
}

TEST(gcs_off_pac_on) {
  SETUP_WITH_FEATURES(CPUFeatures::kPAuth);

  START();
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
  simulator.DisableGCSCheck();
#else
// TODO: Disable GCS via operating system for this test, and enable for native.
#endif
  __ Mov(x16, 1);
  __ Chkfeat(x16);
  __ Mov(x1, x16);

  Label fn1, after_fn1;

  __ Mov(x28, sp);
  __ Mov(x29, lr);
  __ Mov(sp, 0x477d469dec0b8760);

  __ Mov(x0, 0);
  __ B(&after_fn1);

  __ Bind(&fn1);
  __ Mov(x0, 42);
  __ Paciasp();
  __ Retaa();

  __ Bind(&after_fn1);
  __ Bl(&fn1);

  __ Mov(sp, x28);
  __ Mov(lr, x29);
  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(42, x0);
    ASSERT_EQUAL_64(1, x1);
  }
}

#ifdef VIXL_NEGATIVE_TESTING
TEST(gcs_negative_test) {
  SETUP_WITH_FEATURES(CPUFeatures::kGCS);

  Label fn, bad_return_addr, done;
  START();
  __ Bl(&fn);
  __ Nop();  // GCS enforces that fn() returns here...

  __ Bind(&bad_return_addr);
  __ B(&done);  // ... but this test attempts to return here.

  __ Bind(&fn);
  __ Adr(lr, &bad_return_addr);
  __ Ret();

  __ Bind(&done);
  END();

  if (CAN_RUN()) {
    MUST_FAIL_WITH_MESSAGE(RUN(), "GCS failed");
  }
}
#endif  // VIXL_NEGATIVE_TESTING

TEST(dc_zva) {
  SETUP_WITH_FEATURES(CPUFeatures::kNEON);

  const int zva_blocksize = 64;  // Assumed blocksize.
  uint8_t buf[2 * zva_blocksize];
  uintptr_t buf_addr = reinterpret_cast<uintptr_t>(buf);
  uintptr_t aligned_addr = AlignUp(buf_addr, zva_blocksize);

  START();
  // Skip this test if the ZVA blocksize is not 64 bytes.
  // Set up initial register values to allow the test to pass when skipped.
  Label skip;
  __ Movi(q0.V16B(), 0);
  __ Movi(q1.V16B(), 0);
  __ Movi(q2.V16B(), 0);
  __ Movi(q3.V16B(), 0);

  __ Mrs(x1, DCZID_EL0);
  __ Cmp(x1, 4);  // 4 => DC ZVA enabled with 64-byte blocks.
  __ B(ne, &skip);

  // Fill aligned region with a pattern.
  __ Mov(x0, aligned_addr);
  __ Movi(q0.V16B(), 0x55);
  __ Movi(q1.V16B(), 0xaa);
  __ Movi(q2.V16B(), 0x55);
  __ Movi(q3.V16B(), 0xaa);
  __ St4(q0.V16B(), q1.V16B(), q2.V16B(), q3.V16B(), MemOperand(x0));

  // Misalign the address to check DC ZVA re-aligns.
  __ Add(x0, x0, 42);

  // Clear the aligned region.
  __ Dc(ZVA, x0);

  // Reload the aligned region to check contents.
  __ Mov(x0, aligned_addr);
  __ Ld1(q0.V16B(), q1.V16B(), q2.V16B(), q3.V16B(), MemOperand(x0));

  __ Bind(&skip);
  END();

  if (CAN_RUN()) {
    RUN();
    if (core.xreg(1) == 4) {
      ASSERT_EQUAL_128(0, 0, q0);
      ASSERT_EQUAL_128(0, 0, q1);
      ASSERT_EQUAL_128(0, 0, q2);
      ASSERT_EQUAL_128(0, 0, q3);
    } else {
      printf("SKIPPED: DC ZVA chunksize not 64-bytes");
    }
  }
}

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
// Test the pseudo-instructions that control CPUFeatures dynamically in the
// Simulator. These are used by the test infrastructure itself, but in a fairly
// limited way.

static void RunHelperWithFeatureCombinations(
    void (*helper)(const CPUFeatures& base, const CPUFeatures& f)) {
  // Iterate, testing the first n features in this list.
  CPUFeatures::Feature features[] = {
      // Put kNone first, so that the first iteration uses an empty feature set.
      CPUFeatures::kNone,
      // The remaining features used are arbitrary.
      CPUFeatures::kIDRegisterEmulation,
      CPUFeatures::kDCPoP,
      CPUFeatures::kPAuth,
      CPUFeatures::kFcma,
      CPUFeatures::kAES,
      CPUFeatures::kNEON,
      CPUFeatures::kCRC32,
      CPUFeatures::kFP,
      CPUFeatures::kPmull1Q,
      CPUFeatures::kSM4,
      CPUFeatures::kSM3,
      CPUFeatures::kDotProduct,
  };
  VIXL_ASSERT(CPUFeatures(CPUFeatures::kNone) == CPUFeatures::None());
  // The features are not necessarily encoded in kInstructionSize-sized slots,
  // so the MacroAssembler must pad the list to align the following instruction.
  // Ensure that we have enough features in the list to cover all interesting
  // alignment cases, even if the highest common factor of kInstructionSize and
  // an encoded feature is one.
  VIXL_STATIC_ASSERT(ARRAY_SIZE(features) > kInstructionSize);

  CPUFeatures base = CPUFeatures::None();
  for (size_t i = 0; i < ARRAY_SIZE(features); i++) {
    base.Combine(features[i]);
    CPUFeatures f = CPUFeatures::None();
    for (size_t j = 0; j < ARRAY_SIZE(features); j++) {
      f.Combine(features[j]);
      helper(base, f);
    }
  }
}

static void SetSimulatorCPUFeaturesHelper(const CPUFeatures& base,
                                          const CPUFeatures& f) {
  SETUP_WITH_FEATURES(base);
  START();

  __ SetSimulatorCPUFeatures(f);

  END();
  if (CAN_RUN()) {
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
    VIXL_CHECK(*(simulator.GetCPUFeatures()) == f);
  }
}

TEST(configure_cpu_features_set) {
  RunHelperWithFeatureCombinations(SetSimulatorCPUFeaturesHelper);
}

static void EnableSimulatorCPUFeaturesHelper(const CPUFeatures& base,
                                             const CPUFeatures& f) {
  SETUP_WITH_FEATURES(base);
  START();

  __ EnableSimulatorCPUFeatures(f);

  END();
  if (CAN_RUN()) {
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
    VIXL_CHECK(*(simulator.GetCPUFeatures()) == base.With(f));
  }
}

TEST(configure_cpu_features_enable) {
  RunHelperWithFeatureCombinations(EnableSimulatorCPUFeaturesHelper);
}

static void DisableSimulatorCPUFeaturesHelper(const CPUFeatures& base,
                                              const CPUFeatures& f) {
  SETUP_WITH_FEATURES(base);
  START();

  __ DisableSimulatorCPUFeatures(f);

  END();
  if (CAN_RUN()) {
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
    VIXL_CHECK(*(simulator.GetCPUFeatures()) == base.Without(f));
  }
}

TEST(configure_cpu_features_disable) {
  RunHelperWithFeatureCombinations(DisableSimulatorCPUFeaturesHelper);
}

static void SaveRestoreSimulatorCPUFeaturesHelper(const CPUFeatures& base,
                                                  const CPUFeatures& f) {
  SETUP_WITH_FEATURES(base);
  START();

  {
    __ SaveSimulatorCPUFeatures();
    __ SetSimulatorCPUFeatures(f);
    {
      __ SaveSimulatorCPUFeatures();
      __ SetSimulatorCPUFeatures(CPUFeatures::All());
      __ RestoreSimulatorCPUFeatures();
    }
    __ RestoreSimulatorCPUFeatures();
  }

  END();
  if (CAN_RUN()) {
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
    VIXL_CHECK(*(simulator.GetCPUFeatures()) == base);
  }
}

TEST(configure_cpu_features_save_restore) {
  RunHelperWithFeatureCombinations(SaveRestoreSimulatorCPUFeaturesHelper);
}

static void SimulationCPUFeaturesScopeHelper(const CPUFeatures& base,
                                             const CPUFeatures& f) {
  SETUP_WITH_FEATURES(base);
  START();

  {
    SimulationCPUFeaturesScope scope_a(&masm, f);
    {
      SimulationCPUFeaturesScope scope_b(&masm, CPUFeatures::All());
      {
        SimulationCPUFeaturesScope scope_c(&masm, CPUFeatures::None());
        // The scope arguments should combine with 'Enable', so we should be
        // able to use any CPUFeatures here.
        __ Fadd(v0.V4S(), v1.V4S(), v2.V4S());  // Requires {FP, NEON}.
      }
    }
  }

  END();
  if (CAN_RUN()) {
    RUN_WITHOUT_SEEN_FEATURE_CHECK();
    VIXL_CHECK(*(simulator.GetCPUFeatures()) == base);
  }
}

TEST(configure_cpu_features_scope) {
  RunHelperWithFeatureCombinations(SimulationCPUFeaturesScopeHelper);
}
#endif


#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
TEST(large_sim_stack) {
  SimStack builder;
  builder.SetUsableSize(16 * 1024);  // The default is 8kB.
  SimStack::Allocated stack = builder.Allocate();
  uintptr_t base = reinterpret_cast<uintptr_t>(stack.GetBase());
  uintptr_t limit = reinterpret_cast<uintptr_t>(stack.GetLimit());
  SETUP_CUSTOM_SIM(std::move(stack));
  START();

  // Check that we can access the extremes of the stack.
  __ Mov(x0, base);
  __ Mov(x1, limit);
  __ Mov(x2, sp);
  __ Add(sp, x1, 1);  // Avoid accessing memory below `sp`.

  __ Mov(x10, 42);
  __ Poke(x10, 0);
  __ Peek(x10, base - limit - kXRegSizeInBytes - 1);

  __ Mov(sp, x2);

  END();
  if (CAN_RUN()) {
    RUN();
  }
}

#ifdef VIXL_NEGATIVE_TESTING
TEST(sim_stack_limit_guard_read) {
  SimStack builder;
  SimStack::Allocated stack = builder.Allocate();
  uintptr_t limit = reinterpret_cast<uintptr_t>(stack.GetLimit());
  SETUP_CUSTOM_SIM(std::move(stack));
  START();

  __ Mov(x1, limit);
  __ Mov(x2, sp);
  __ Add(sp, x1, 1);  // Avoid accessing memory below `sp`.

  // `sp` points to the lowest usable byte of the stack.
  __ Mov(w10, 42);
  __ Ldrb(w10, MemOperand(sp, -1));

  __ Mov(sp, x2);

  END();
  if (CAN_RUN()) {
    MUST_FAIL_WITH_MESSAGE(RUN(), "Attempt to read from stack guard region");
  }
}

TEST(sim_stack_limit_guard_write) {
  SimStack builder;
  SimStack::Allocated stack = builder.Allocate();
  uintptr_t limit = reinterpret_cast<uintptr_t>(stack.GetLimit());
  SETUP_CUSTOM_SIM(std::move(stack));
  START();

  __ Mov(x1, limit);
  __ Mov(x2, sp);
  __ Add(sp, x1, 1);  // Avoid accessing memory below `sp`.

  // `sp` points to the lowest usable byte of the stack.
  __ Mov(w10, 42);
  __ Strb(w10, MemOperand(sp, -1));

  __ Mov(sp, x2);

  END();
  if (CAN_RUN()) {
    MUST_FAIL_WITH_MESSAGE(RUN(), "Attempt to write to stack guard region");
  }
}

TEST(sim_stack_base_guard_read) {
  SimStack builder;
  SimStack::Allocated stack = builder.Allocate();
  uintptr_t base = reinterpret_cast<uintptr_t>(stack.GetBase());
  SETUP_CUSTOM_SIM(std::move(stack));
  START();

  __ Mov(x0, base);
  // `base` (x0) is the byte after the highest usable byte of the stack.
  // The last byte of this access will hit the guard region.
  __ Mov(x10, 42);
  __ Ldr(x10, MemOperand(x0, -static_cast<int64_t>(kXRegSizeInBytes) + 1));

  END();
  if (CAN_RUN()) {
    MUST_FAIL_WITH_MESSAGE(RUN(), "Attempt to read from stack guard region");
  }
}

TEST(sim_stack_base_guard_write) {
  SimStack builder;
  SimStack::Allocated stack = builder.Allocate();
  uintptr_t base = reinterpret_cast<uintptr_t>(stack.GetBase());
  SETUP_CUSTOM_SIM(std::move(stack));
  START();

  __ Mov(x0, base);
  // `base` (x0) is the byte after the highest usable byte of the stack.
  // The last byte of this access will hit the guard region.
  __ Mov(x10, 42);
  __ Str(x10, MemOperand(x0, -static_cast<int64_t>(kXRegSizeInBytes) + 1));

  END();
  if (CAN_RUN()) {
    MUST_FAIL_WITH_MESSAGE(RUN(), "Attempt to write to stack guard region");
  }
}
#endif
#endif

TEST(scalar_movi) {
  SETUP_WITH_FEATURES(CPUFeatures::kFP, CPUFeatures::kNEON);
  START();

  // Make sure that V0 is initialized to a non-zero value.
  __ Movi(v0.V16B(), 0xFF);
  // This constant value can't be encoded in a MOVI instruction,
  // so the program would use a fallback path that must set the
  // upper 64 bits of the destination vector to 0.
  __ Movi(v0.V1D(), 0xDECAFC0FFEE);
  __ Mov(x0, v0.V2D(), 1);

  END();

  if (CAN_RUN()) {
    RUN();

    ASSERT_EQUAL_64(0, x0);
  }
}

}  // namespace aarch64
}  // namespace vixl
