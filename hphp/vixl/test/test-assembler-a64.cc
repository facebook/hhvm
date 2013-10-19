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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <gtest/gtest.h>

#include "hphp/vixl/test/test-utils-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/vixl/a64/simulator-a64.h"
#include "hphp/vixl/a64/debugger-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/vixl/a64/cpu-a64.h"

namespace vixl {

// Test infrastructure.
//
// Tests are functions which accept no parameters and have no return values.
// The testing code should not perform an explicit return once completed. For
// example to test the mov immediate instruction a very simple test would be:
//
//   TEST(Assembler, mov_x0_one) {
//     SETUP();
//
//     START();
//     __ mov(x0, Operand(1));
//     END();
//
//     RUN();
//
//     ASSERT_EQUAL_64(1, x0);
//
//     TEARDOWN();
//   }
//
// Within a START ... END block all registers but sp can be modified. sp has to
// be explicitly saved/restored. The END() macro replaces the function return
// so it may appear multiple times in a test if the test has multiple exit
// points.
//
// Once the test has been run all integer and floating point registers as well
// as flags are accessible through a RegisterDump instance, see
// utils-a64.cc for more info on RegisterDump.
//
// We provide some helper assert to handle common cases:
//
//   ASSERT_EQUAL_32(int32_t, int_32t)
//   ASSERT_EQUAL_FP32(float, float)
//   ASSERT_EQUAL_32(int32_t, W register)
//   ASSERT_EQUAL_FP32(float, S register)
//   ASSERT_EQUAL_64(int64_t, int_64t)
//   ASSERT_EQUAL_FP64(double, double)
//   ASSERT_EQUAL_64(int64_t, X register)
//   ASSERT_EQUAL_64(X register, X register)
//   ASSERT_EQUAL_FP64(double, D register)
//
// e.g. ASSERT_EQUAL_64(0.5, d30);
//
// If more advanced computation is required before the assert then access the
// RegisterDump named core directly:
//
//   ASSERT_EQUAL_64(0x1234, core->reg_x0() & 0xffff);


#define __ masm.

#define BUF_SIZE (4096)

#define SETUP() SETUP_SIZE(BUF_SIZE)

#ifdef USE_SIMULATOR

// Run tests with the simulator.
#define SETUP_SIZE(buf_size)                                                   \
  byte* buf = new byte[buf_size];                                              \
  HPHP::CodeBlock cb;                                                          \
  cb.init(buf, buf_size);                                                      \
  Decoder decoder;                                                             \
  Simulator* simulator = nullptr;                                              \
  simulator = new Simulator(&decoder);                                         \
  RegisterDump core;                                                           \
  { /* masm needs to be destroyed before buf is deleted */                     \
    MacroAssembler masm(cb)                                                    \

#define START()                                                                \
  masm.Reset();                                                                \
  simulator->ResetState();                                                     \
  __ PushCalleeSavedRegisters();

#define END()                                                                  \
  core.Dump(&masm);                                                            \
  __ PopCalleeSavedRegisters();                                                \
  __ Ret();                                                                    \
  masm.FinalizeCode()

#define RUN()                                                                  \
  simulator->RunFrom(reinterpret_cast<Instruction*>(buf))

#define TEARDOWN()                                                             \
  } /* closing scope for masm in SETUP / SETUP_SIZE */                         \
  delete simulator;                                                            \
  delete[] buf;

#else  // ifdef USE_SIMULATOR.
// Run the test on real hardware or models.
#define SETUP_SIZE(buf_size)                                                   \
  byte* buf = new byte[buf_size];                                              \
  HPHP::CodeBlock cb;                                                          \
  cb.init(buf, buf_size);                                                      \
  MacroAssembler masm(cb);                                                     \
  RegisterDump core;                                                           \
  CPU::SetUp()

#define START()                                                                \
  masm.Reset();                                                                \
  __ PushCalleeSavedRegisters()

#define END()                                                                  \
  core.Dump(&masm);                                                            \
  __ PopCalleeSavedRegisters();                                                \
  __ Ret();                                                                    \
  masm.FinalizeCode()

#define RUN()                                                                  \
  CPU::EnsureIAndDCacheCoherency(&buf, sizeof(buf));                           \
  {                                                                            \
    void (*test_function)(void);                                               \
    memcpy(&test_function, &buf, sizeof(buf));                                 \
    test_function();                                                           \
  }

#define TEARDOWN()                                                             \
  delete[] buf;

#endif  // ifdef USE_SIMULATOR.

#define ASSERT_EQUAL_NZCV(expected)                                            \
  EXPECT_TRUE(EqualNzcv(expected, core.flags_nzcv()))

#define ASSERT_EQUAL_REGISTERS(expected)                                       \
  EXPECT_TRUE(EqualRegisters(&expected, &core))

#define ASSERT_EQUAL_32(expected, result)                                      \
  EXPECT_TRUE(Equal32(static_cast<uint32_t>(expected), &core, result))

#define ASSERT_EQUAL_FP32(expected, result)                                    \
  EXPECT_TRUE(EqualFP32(expected, &core, result))

#define ASSERT_EQUAL_64(expected, result)                                      \
  EXPECT_TRUE(Equal64(expected, &core, result))

#define ASSERT_EQUAL_FP64(expected, result)                                    \
  EXPECT_TRUE(EqualFP64(expected, &core, result))

#define ASSERT_LITERAL_POOL_SIZE(expected)                                     \
  EXPECT_EQ((expected), (__ LiteralPoolSize()))


TEST(Assembler, stack_ops) {
  SETUP();

  START();
  // save sp.
  __ Mov(x29, sp);

  // Set the sp to a known value.
  __ Mov(sp, 0x1004);
  __ Mov(x0, sp);

  // Add immediate to the sp, and move the result to a normal register.
  __ Add(sp, sp, Operand(0x50));
  __ Mov(x1, sp);

  // Add extended to the sp, and move the result to a normal register.
  __ Mov(x17, 0xfff);
  __ Add(sp, sp, Operand(x17, SXTB));
  __ Mov(x2, sp);

  // Create an sp using a logical instruction, and move to normal register.
  __ Orr(sp, xzr, Operand(0x1fff));
  __ Mov(x3, sp);

  // Write wsp using a logical instruction.
  __ Orr(wsp, wzr, Operand(0xfffffff8L));
  __ Mov(x4, sp);

  // Write sp, and read back wsp.
  __ Orr(sp, xzr, Operand(0xfffffff8L));
  __ Mov(w5, wsp);

  //  restore sp.
  __ Mov(sp, x29);
  END();

  RUN();

  ASSERT_EQUAL_64(0x1004, x0);
  ASSERT_EQUAL_64(0x1054, x1);
  ASSERT_EQUAL_64(0x1053, x2);
  ASSERT_EQUAL_64(0x1fff, x3);
  ASSERT_EQUAL_64(0xfffffff8, x4);
  ASSERT_EQUAL_64(0xfffffff8, x5);

  TEARDOWN();
}


TEST(Assembler, mvn) {
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

  RUN();

  ASSERT_EQUAL_64(0xfffff000, x0);
  ASSERT_EQUAL_64(0xfffffffffffff000UL, x1);
  ASSERT_EQUAL_64(0x00001fff, x2);
  ASSERT_EQUAL_64(0x0000000000003fffUL, x3);
  ASSERT_EQUAL_64(0xe00001ff, x4);
  ASSERT_EQUAL_64(0xf0000000000000ffUL, x5);
  ASSERT_EQUAL_64(0x00000001, x6);
  ASSERT_EQUAL_64(0x0, x7);
  ASSERT_EQUAL_64(0x7ff80000, x8);
  ASSERT_EQUAL_64(0x3ffc000000000000UL, x9);
  ASSERT_EQUAL_64(0xffffff00, x10);
  ASSERT_EQUAL_64(0x0000000000000001UL, x11);
  ASSERT_EQUAL_64(0xffff8003, x12);
  ASSERT_EQUAL_64(0xffffffffffff0007UL, x13);
  ASSERT_EQUAL_64(0xfffffffffffe000fUL, x14);
  ASSERT_EQUAL_64(0xfffffffffffe000fUL, x15);

  TEARDOWN();
}


TEST(Assembler, mov) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffffL);
  __ Mov(x1, 0xffffffffffffffffL);
  __ Mov(x2, 0xffffffffffffffffL);
  __ Mov(x3, 0xffffffffffffffffL);

  __ Mov(x0, 0x0123456789abcdefL);

  __ movz(x1, 0xabcdL << 16);
  __ movk(x2, 0xabcdL << 32);
  __ movn(x3, 0xabcdL << 48);

  __ Mov(x4, 0x0123456789abcdefL);
  __ Mov(x5, x4);

  __ Mov(w6, -1);

  // Test that moves back to the same register have the desired effect. This
  // is a no-op for X registers, and a truncation for W registers.
  __ Mov(x7, 0x0123456789abcdefL);
  __ Mov(x7, x7);
  __ Mov(x8, 0x0123456789abcdefL);
  __ Mov(w8, w8);
  __ Mov(x9, 0x0123456789abcdefL);
  __ Mov(x9, Operand(x9));
  __ Mov(x10, 0x0123456789abcdefL);
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
  END();

  RUN();

  ASSERT_EQUAL_64(0x0123456789abcdefL, x0);
  ASSERT_EQUAL_64(0x00000000abcd0000L, x1);
  ASSERT_EQUAL_64(0xffffabcdffffffffL, x2);
  ASSERT_EQUAL_64(0x5432ffffffffffffL, x3);
  ASSERT_EQUAL_64(x4, x5);
  ASSERT_EQUAL_32(-1, w6);
  ASSERT_EQUAL_64(0x0123456789abcdefL, x7);
  ASSERT_EQUAL_32(0x89abcdefL, w8);
  ASSERT_EQUAL_64(0x0123456789abcdefL, x9);
  ASSERT_EQUAL_32(0x89abcdefL, w10);
  ASSERT_EQUAL_64(0x00000fff, x11);
  ASSERT_EQUAL_64(0x0000000000000fffUL, x12);
  ASSERT_EQUAL_64(0x00001ffe, x13);
  ASSERT_EQUAL_64(0x0000000000003ffcUL, x14);
  ASSERT_EQUAL_64(0x000001ff, x15);
  ASSERT_EQUAL_64(0x00000000000000ffUL, x18);
  ASSERT_EQUAL_64(0x00000001, x19);
  ASSERT_EQUAL_64(0x0, x20);
  ASSERT_EQUAL_64(0x7ff80000, x21);
  ASSERT_EQUAL_64(0x3ffc000000000000UL, x22);
  ASSERT_EQUAL_64(0x000000fe, x23);
  ASSERT_EQUAL_64(0xfffffffffffffffcUL, x24);
  ASSERT_EQUAL_64(0x00007ff8, x25);
  ASSERT_EQUAL_64(0x000000000000fff0UL, x26);
  ASSERT_EQUAL_64(0x000000000001ffe0UL, x27);

  TEARDOWN();
}


TEST(Assembler, orr) {
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
  __ Orr(w10, w0, Operand(0xf));
  __ Orr(x11, x0, Operand(0xf0000000f0000000L));
  END();

  RUN();

  ASSERT_EQUAL_64(0xf000f0ff, x2);
  ASSERT_EQUAL_64(0xf000f0f0, x3);
  ASSERT_EQUAL_64(0xf00000ff0000f0f0L, x4);
  ASSERT_EQUAL_64(0x0f00f0ff, x5);
  ASSERT_EQUAL_64(0xff00f0ff, x6);
  ASSERT_EQUAL_64(0x0f00f0ff, x7);
  ASSERT_EQUAL_64(0x0ffff0f0, x8);
  ASSERT_EQUAL_64(0x0ff00000000ff0f0L, x9);
  ASSERT_EQUAL_64(0xf0ff, x10);
  ASSERT_EQUAL_64(0xf0000000f000f0f0L, x11);

  TEARDOWN();
}


TEST(Assembler, orr_extend) {
  SETUP();

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x8000000080008080UL);
  __ Orr(w6, w0, Operand(w1, UXTB));
  __ Orr(x7, x0, Operand(x1, UXTH, 1));
  __ Orr(w8, w0, Operand(w1, UXTW, 2));
  __ Orr(x9, x0, Operand(x1, UXTX, 3));
  __ Orr(w10, w0, Operand(w1, SXTB));
  __ Orr(x11, x0, Operand(x1, SXTH, 1));
  __ Orr(x12, x0, Operand(x1, SXTW, 2));
  __ Orr(x13, x0, Operand(x1, SXTX, 3));
  END();

  RUN();

  ASSERT_EQUAL_64(0x00000081, x6);
  ASSERT_EQUAL_64(0x00010101, x7);
  ASSERT_EQUAL_64(0x00020201, x8);
  ASSERT_EQUAL_64(0x0000000400040401UL, x9);
  ASSERT_EQUAL_64(0x00000000ffffff81UL, x10);
  ASSERT_EQUAL_64(0xffffffffffff0101UL, x11);
  ASSERT_EQUAL_64(0xfffffffe00020201UL, x12);
  ASSERT_EQUAL_64(0x0000000400040401UL, x13);

  TEARDOWN();
}


TEST(Assembler, bitwise_wide_imm) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0xf0f0f0f0f0f0f0f0UL);

  __ Orr(x10, x0, Operand(0x1234567890abcdefUL));
  __ Orr(w11, w1, Operand(0x90abcdef));
  END();

  RUN();

  ASSERT_EQUAL_64(0, x0);
  ASSERT_EQUAL_64(0xf0f0f0f0f0f0f0f0UL, x1);
  ASSERT_EQUAL_64(0x1234567890abcdefUL, x10);
  ASSERT_EQUAL_64(0xf0fbfdffUL, x11);

  TEARDOWN();
}


TEST(Assembler, orn) {
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
  __ Orn(w10, w0, Operand(0xffff));
  __ Orn(x11, x0, Operand(0xffff0000ffffL));
  END();

  RUN();

  ASSERT_EQUAL_64(0xffffffff0ffffff0L, x2);
  ASSERT_EQUAL_64(0xfffff0ff, x3);
  ASSERT_EQUAL_64(0xfffffff0fffff0ffL, x4);
  ASSERT_EQUAL_64(0xffffffff87fffff0L, x5);
  ASSERT_EQUAL_64(0x07fffff0, x6);
  ASSERT_EQUAL_64(0xffffffff87fffff0L, x7);
  ASSERT_EQUAL_64(0xff00ffff, x8);
  ASSERT_EQUAL_64(0xff00ffffffffffffL, x9);
  ASSERT_EQUAL_64(0xfffff0f0, x10);
  ASSERT_EQUAL_64(0xffff0000fffff0f0L, x11);

  TEARDOWN();
}


TEST(Assembler, orn_extend) {
  SETUP();

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x8000000080008081UL);
  __ Orn(w6, w0, Operand(w1, UXTB));
  __ Orn(x7, x0, Operand(x1, UXTH, 1));
  __ Orn(w8, w0, Operand(w1, UXTW, 2));
  __ Orn(x9, x0, Operand(x1, UXTX, 3));
  __ Orn(w10, w0, Operand(w1, SXTB));
  __ Orn(x11, x0, Operand(x1, SXTH, 1));
  __ Orn(x12, x0, Operand(x1, SXTW, 2));
  __ Orn(x13, x0, Operand(x1, SXTX, 3));
  END();

  RUN();

  ASSERT_EQUAL_64(0xffffff7f, x6);
  ASSERT_EQUAL_64(0xfffffffffffefefdUL, x7);
  ASSERT_EQUAL_64(0xfffdfdfb, x8);
  ASSERT_EQUAL_64(0xfffffffbfffbfbf7UL, x9);
  ASSERT_EQUAL_64(0x0000007f, x10);
  ASSERT_EQUAL_64(0x0000fefd, x11);
  ASSERT_EQUAL_64(0x00000001fffdfdfbUL, x12);
  ASSERT_EQUAL_64(0xfffffffbfffbfbf7UL, x13);

  TEARDOWN();
}


TEST(Assembler, and_) {
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

  TEARDOWN();
}


TEST(Assembler, and_extend) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffffUL);
  __ Mov(x1, 0x8000000080008081UL);
  __ And(w6, w0, Operand(w1, UXTB));
  __ And(x7, x0, Operand(x1, UXTH, 1));
  __ And(w8, w0, Operand(w1, UXTW, 2));
  __ And(x9, x0, Operand(x1, UXTX, 3));
  __ And(w10, w0, Operand(w1, SXTB));
  __ And(x11, x0, Operand(x1, SXTH, 1));
  __ And(x12, x0, Operand(x1, SXTW, 2));
  __ And(x13, x0, Operand(x1, SXTX, 3));
  END();

  RUN();

  ASSERT_EQUAL_64(0x00000081, x6);
  ASSERT_EQUAL_64(0x00010102, x7);
  ASSERT_EQUAL_64(0x00020204, x8);
  ASSERT_EQUAL_64(0x0000000400040408UL, x9);
  ASSERT_EQUAL_64(0xffffff81, x10);
  ASSERT_EQUAL_64(0xffffffffffff0102UL, x11);
  ASSERT_EQUAL_64(0xfffffffe00020204UL, x12);
  ASSERT_EQUAL_64(0x0000000400040408UL, x13);

  TEARDOWN();
}


TEST(Assembler, ands) {
  SETUP();

  START();
  __ Mov(x1, 0xf00000ff);
  __ And(w0, w1, Operand(w1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);
  ASSERT_EQUAL_64(0xf00000ff, x0);

  START();
  __ Mov(x0, 0xfff0);
  __ Mov(x1, 0xf00000ff);
  __ And(w0, w0, Operand(w1, LSR, 4), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZFlag);
  ASSERT_EQUAL_64(0x00000000, x0);

  START();
  __ Mov(x0, 0x8000000000000000L);
  __ Mov(x1, 0x00000001);
  __ And(x0, x0, Operand(x1, ROR, 1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);
  ASSERT_EQUAL_64(0x8000000000000000L, x0);

  START();
  __ Mov(x0, 0xfff0);
  __ And(w0, w0, Operand(0xf), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZFlag);
  ASSERT_EQUAL_64(0x00000000, x0);

  START();
  __ Mov(x0, 0xff000000);
  __ And(w0, w0, Operand(0x80000000), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);
  ASSERT_EQUAL_64(0x80000000, x0);

  TEARDOWN();
}


TEST(Assembler, bic) {
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

  TEARDOWN();
}


TEST(Assembler, bic_extend) {
  SETUP();

  START();
  __ Mov(x0, 0xffffffffffffffffUL);
  __ Mov(x1, 0x8000000080008081UL);
  __ Bic(w6, w0, Operand(w1, UXTB));
  __ Bic(x7, x0, Operand(x1, UXTH, 1));
  __ Bic(w8, w0, Operand(w1, UXTW, 2));
  __ Bic(x9, x0, Operand(x1, UXTX, 3));
  __ Bic(w10, w0, Operand(w1, SXTB));
  __ Bic(x11, x0, Operand(x1, SXTH, 1));
  __ Bic(x12, x0, Operand(x1, SXTW, 2));
  __ Bic(x13, x0, Operand(x1, SXTX, 3));
  END();

  RUN();

  ASSERT_EQUAL_64(0xffffff7e, x6);
  ASSERT_EQUAL_64(0xfffffffffffefefdUL, x7);
  ASSERT_EQUAL_64(0xfffdfdfb, x8);
  ASSERT_EQUAL_64(0xfffffffbfffbfbf7UL, x9);
  ASSERT_EQUAL_64(0x0000007e, x10);
  ASSERT_EQUAL_64(0x0000fefd, x11);
  ASSERT_EQUAL_64(0x00000001fffdfdfbUL, x12);
  ASSERT_EQUAL_64(0xfffffffbfffbfbf7UL, x13);

  TEARDOWN();
}


TEST(Assembler, bics) {
  SETUP();

  START();
  __ Mov(x1, 0xffff);
  __ Bic(w0, w1, Operand(w1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZFlag);
  ASSERT_EQUAL_64(0x00000000, x0);

  START();
  __ Mov(x0, 0xffffffff);
  __ Bic(w0, w0, Operand(w0, LSR, 1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);
  ASSERT_EQUAL_64(0x80000000, x0);

  START();
  __ Mov(x0, 0x8000000000000000L);
  __ Mov(x1, 0x00000001);
  __ Bic(x0, x0, Operand(x1, ROR, 1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZFlag);
  ASSERT_EQUAL_64(0x00000000, x0);

  START();
  __ Mov(x0, 0xffffffffffffffffL);
  __ Bic(x0, x0, Operand(0x7fffffffffffffffL), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);
  ASSERT_EQUAL_64(0x8000000000000000L, x0);

  START();
  __ Mov(w0, 0xffff0000);
  __ Bic(w0, w0, Operand(0xfffffff0), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZFlag);
  ASSERT_EQUAL_64(0x00000000, x0);

  TEARDOWN();
}


TEST(Assembler, eor) {
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
  __ Eor(w10, w0, Operand(0xff00ff00));
  __ Eor(x11, x0, Operand(0xff00ff00ff00ff00L));
  END();

  RUN();

  ASSERT_EQUAL_64(0xf000ff0f, x2);
  ASSERT_EQUAL_64(0x0000f000, x3);
  ASSERT_EQUAL_64(0x0000000f0000f000L, x4);
  ASSERT_EQUAL_64(0x7800ff8f, x5);
  ASSERT_EQUAL_64(0xffff00f0, x6);
  ASSERT_EQUAL_64(0x0000f0f0, x7);
  ASSERT_EQUAL_64(0x0000f00f, x8);
  ASSERT_EQUAL_64(0x00000ff00000ffffL, x9);
  ASSERT_EQUAL_64(0xff0000f0, x10);
  ASSERT_EQUAL_64(0xff00ff00ff0000f0L, x11);

  TEARDOWN();
}

TEST(Assembler, eor_extend) {
  SETUP();

  START();
  __ Mov(x0, 0x1111111111111111UL);
  __ Mov(x1, 0x8000000080008081UL);
  __ Eor(w6, w0, Operand(w1, UXTB));
  __ Eor(x7, x0, Operand(x1, UXTH, 1));
  __ Eor(w8, w0, Operand(w1, UXTW, 2));
  __ Eor(x9, x0, Operand(x1, UXTX, 3));
  __ Eor(w10, w0, Operand(w1, SXTB));
  __ Eor(x11, x0, Operand(x1, SXTH, 1));
  __ Eor(x12, x0, Operand(x1, SXTW, 2));
  __ Eor(x13, x0, Operand(x1, SXTX, 3));
  END();

  RUN();

  ASSERT_EQUAL_64(0x11111190, x6);
  ASSERT_EQUAL_64(0x1111111111101013UL, x7);
  ASSERT_EQUAL_64(0x11131315, x8);
  ASSERT_EQUAL_64(0x1111111511151519UL, x9);
  ASSERT_EQUAL_64(0xeeeeee90, x10);
  ASSERT_EQUAL_64(0xeeeeeeeeeeee1013UL, x11);
  ASSERT_EQUAL_64(0xeeeeeeef11131315UL, x12);
  ASSERT_EQUAL_64(0x1111111511151519UL, x13);

  TEARDOWN();
}


TEST(Assembler, eon) {
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
  __ Eon(w10, w0, Operand(0x03c003c0));
  __ Eon(x11, x0, Operand(0x0000100000001000L));
  END();

  RUN();

  ASSERT_EQUAL_64(0xffffffff0fff00f0L, x2);
  ASSERT_EQUAL_64(0xffff0fff, x3);
  ASSERT_EQUAL_64(0xfffffff0ffff0fffL, x4);
  ASSERT_EQUAL_64(0xffffffff87ff0070L, x5);
  ASSERT_EQUAL_64(0x0000ff0f, x6);
  ASSERT_EQUAL_64(0xffffffffffff0f0fL, x7);
  ASSERT_EQUAL_64(0xffff0ff0, x8);
  ASSERT_EQUAL_64(0xfffff00fffff0000L, x9);
  ASSERT_EQUAL_64(0xfc3f03cf, x10);
  ASSERT_EQUAL_64(0xffffefffffff100fL, x11);

  TEARDOWN();
}


TEST(Assembler, eon_extend) {
  SETUP();

  START();
  __ Mov(x0, 0x1111111111111111UL);
  __ Mov(x1, 0x8000000080008081UL);
  __ Eon(w6, w0, Operand(w1, UXTB));
  __ Eon(x7, x0, Operand(x1, UXTH, 1));
  __ Eon(w8, w0, Operand(w1, UXTW, 2));
  __ Eon(x9, x0, Operand(x1, UXTX, 3));
  __ Eon(w10, w0, Operand(w1, SXTB));
  __ Eon(x11, x0, Operand(x1, SXTH, 1));
  __ Eon(x12, x0, Operand(x1, SXTW, 2));
  __ Eon(x13, x0, Operand(x1, SXTX, 3));
  END();

  RUN();

  ASSERT_EQUAL_64(0xeeeeee6f, x6);
  ASSERT_EQUAL_64(0xeeeeeeeeeeefefecUL, x7);
  ASSERT_EQUAL_64(0xeeececea, x8);
  ASSERT_EQUAL_64(0xeeeeeeeaeeeaeae6UL, x9);
  ASSERT_EQUAL_64(0x1111116f, x10);
  ASSERT_EQUAL_64(0x111111111111efecUL, x11);
  ASSERT_EQUAL_64(0x11111110eeececeaUL, x12);
  ASSERT_EQUAL_64(0xeeeeeeeaeeeaeae6UL, x13);

  TEARDOWN();
}


TEST(Assembler, mul) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffffUL);

  __ Mul(w0, w16, w16);
  __ Mul(w1, w16, w17);
  __ Mul(w2, w17, w18);
  __ Mul(w3, w18, w19);
  __ Mul(x4, x16, x16);
  __ Mul(x5, x17, x18);
  __ Mul(x6, x18, x19);
  __ Mul(x7, x19, x19);
  __ Smull(x8, w17, w18);
  __ Smull(x9, w18, w18);
  __ Smull(x10, w19, w19);
  __ Mneg(w11, w16, w16);
  __ Mneg(w12, w16, w17);
  __ Mneg(w13, w17, w18);
  __ Mneg(w14, w18, w19);
  __ Mneg(x20, x16, x16);
  __ Mneg(x21, x17, x18);
  __ Mneg(x22, x18, x19);
  __ Mneg(x23, x19, x19);
  END();

  RUN();

  ASSERT_EQUAL_64(0, x0);
  ASSERT_EQUAL_64(0, x1);
  ASSERT_EQUAL_64(0xffffffff, x2);
  ASSERT_EQUAL_64(1, x3);
  ASSERT_EQUAL_64(0, x4);
  ASSERT_EQUAL_64(0xffffffff, x5);
  ASSERT_EQUAL_64(0xffffffff00000001UL, x6);
  ASSERT_EQUAL_64(1, x7);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(1, x10);
  ASSERT_EQUAL_64(0, x11);
  ASSERT_EQUAL_64(0, x12);
  ASSERT_EQUAL_64(1, x13);
  ASSERT_EQUAL_64(0xffffffff, x14);
  ASSERT_EQUAL_64(0, x20);
  ASSERT_EQUAL_64(0xffffffff00000001UL, x21);
  ASSERT_EQUAL_64(0xffffffff, x22);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x23);

  TEARDOWN();
}


TEST(Assembler, madd) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffffUL);

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
  ASSERT_EQUAL_64(0xffffffff, x14);
  ASSERT_EQUAL_64(0xffffffffffffffff, x15);
  ASSERT_EQUAL_64(1, x20);
  ASSERT_EQUAL_64(0x100000000UL, x21);
  ASSERT_EQUAL_64(0, x22);
  ASSERT_EQUAL_64(0xffffffff, x23);
  ASSERT_EQUAL_64(0x1fffffffe, x24);
  ASSERT_EQUAL_64(0xfffffffe00000002UL, x25);
  ASSERT_EQUAL_64(0, x26);
  ASSERT_EQUAL_64(0, x27);

  TEARDOWN();
}


TEST(Assembler, msub) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffffUL);

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
  ASSERT_EQUAL_64(0xffffffff, x14);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x15);
  ASSERT_EQUAL_64(1, x20);
  ASSERT_EQUAL_64(0xfffffffeUL, x21);
  ASSERT_EQUAL_64(0xfffffffffffffffeUL, x22);
  ASSERT_EQUAL_64(0xffffffff00000001UL, x23);
  ASSERT_EQUAL_64(0, x24);
  ASSERT_EQUAL_64(0x200000000UL, x25);
  ASSERT_EQUAL_64(0x1fffffffeUL, x26);
  ASSERT_EQUAL_64(0xfffffffffffffffeUL, x27);

  TEARDOWN();
}


TEST(Assembler, smulh) {
  SETUP();

  START();
  __ Mov(x20, 0);
  __ Mov(x21, 1);
  __ Mov(x22, 0x0000000100000000L);
  __ Mov(x23, 0x12345678);
  __ Mov(x24, 0x0123456789abcdefL);
  __ Mov(x25, 0x0000000200000000L);
  __ Mov(x26, 0x8000000000000000UL);
  __ Mov(x27, 0xffffffffffffffffUL);
  __ Mov(x28, 0x5555555555555555UL);
  __ Mov(x29, 0xaaaaaaaaaaaaaaaaUL);

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

  RUN();

  ASSERT_EQUAL_64(0, x0);
  ASSERT_EQUAL_64(0, x1);
  ASSERT_EQUAL_64(0, x2);
  ASSERT_EQUAL_64(0x01234567, x3);
  ASSERT_EQUAL_64(0x02468acf, x4);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x5);
  ASSERT_EQUAL_64(0x4000000000000000UL, x6);
  ASSERT_EQUAL_64(0, x7);
  ASSERT_EQUAL_64(0, x8);
  ASSERT_EQUAL_64(0x1c71c71c71c71c71UL, x9);
  ASSERT_EQUAL_64(0xe38e38e38e38e38eUL, x10);
  ASSERT_EQUAL_64(0x1c71c71c71c71c72UL, x11);

  TEARDOWN();
}


TEST(Assembler, smaddl_umaddl) {
  SETUP();

  START();
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffffUL);
  __ Mov(x20, 4);
  __ Mov(x21, 0x200000000UL);

  __ Smaddl(x9, w17, w18, x20);
  __ Smaddl(x10, w18, w18, x20);
  __ Smaddl(x11, w19, w19, x20);
  __ Smaddl(x12, w19, w19, x21);
  __ Umaddl(x13, w17, w18, x20);
  __ Umaddl(x14, w18, w18, x20);
  __ Umaddl(x15, w19, w19, x20);
  __ Umaddl(x22, w19, w19, x21);
  END();

  RUN();

  ASSERT_EQUAL_64(3, x9);
  ASSERT_EQUAL_64(5, x10);
  ASSERT_EQUAL_64(5, x11);
  ASSERT_EQUAL_64(0x200000001UL, x12);
  ASSERT_EQUAL_64(0x100000003UL, x13);
  ASSERT_EQUAL_64(0xfffffffe00000005UL, x14);
  ASSERT_EQUAL_64(0xfffffffe00000005UL, x15);
  ASSERT_EQUAL_64(0x1, x22);

  TEARDOWN();
}


TEST(Assembler, smsubl_umsubl) {
  SETUP();

  START();
  __ Mov(x17, 1);
  __ Mov(x18, 0xffffffff);
  __ Mov(x19, 0xffffffffffffffffUL);
  __ Mov(x20, 4);
  __ Mov(x21, 0x200000000UL);

  __ Smsubl(x9, w17, w18, x20);
  __ Smsubl(x10, w18, w18, x20);
  __ Smsubl(x11, w19, w19, x20);
  __ Smsubl(x12, w19, w19, x21);
  __ Umsubl(x13, w17, w18, x20);
  __ Umsubl(x14, w18, w18, x20);
  __ Umsubl(x15, w19, w19, x20);
  __ Umsubl(x22, w19, w19, x21);
  END();

  RUN();

  ASSERT_EQUAL_64(5, x9);
  ASSERT_EQUAL_64(3, x10);
  ASSERT_EQUAL_64(3, x11);
  ASSERT_EQUAL_64(0x1ffffffffUL, x12);
  ASSERT_EQUAL_64(0xffffffff00000005UL, x13);
  ASSERT_EQUAL_64(0x200000003UL, x14);
  ASSERT_EQUAL_64(0x200000003UL, x15);
  ASSERT_EQUAL_64(0x3ffffffffUL, x22);

  TEARDOWN();
}


TEST(Assembler, div) {
  SETUP();

  START();
  __ Mov(x16, 1);
  __ Mov(x17, 0xffffffff);
  __ Mov(x18, 0xffffffffffffffffUL);
  __ Mov(x19, 0x80000000);
  __ Mov(x20, 0x8000000000000000UL);
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
  END();

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
  ASSERT_EQUAL_64(0xffffffff00000001UL, x9);
  ASSERT_EQUAL_64(0x40000000, x10);
  ASSERT_EQUAL_64(0xC0000000, x11);
  ASSERT_EQUAL_64(0x40000000, x12);
  ASSERT_EQUAL_64(0x40000000, x13);
  ASSERT_EQUAL_64(0x4000000000000000UL, x14);
  ASSERT_EQUAL_64(0xC000000000000000UL, x15);

  TEARDOWN();
}


TEST(Assembler, rbit_rev) {
  SETUP();

  START();
  __ Mov(x24, 0xfedcba9876543210UL);
  __ Rbit(w0, w24);
  __ Rbit(x1, x24);
  __ Rev16(w2, w24);
  __ Rev16(x3, x24);
  __ Rev(w4, w24);
  __ Rev32(x5, x24);
  __ Rev(x6, x24);
  END();

  RUN();

  ASSERT_EQUAL_64(0x084c2a6e, x0);
  ASSERT_EQUAL_64(0x084c2a6e195d3b7fUL, x1);
  ASSERT_EQUAL_64(0x54761032, x2);
  ASSERT_EQUAL_64(0xdcfe98ba54761032UL, x3);
  ASSERT_EQUAL_64(0x10325476, x4);
  ASSERT_EQUAL_64(0x98badcfe10325476UL, x5);
  ASSERT_EQUAL_64(0x1032547698badcfeUL, x6);

  TEARDOWN();
}


TEST(Assembler, clz_cls) {
  SETUP();

  START();
  __ Mov(x24, 0x0008000000800000UL);
  __ Mov(x25, 0xff800000fff80000UL);
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

  TEARDOWN();
}


TEST(Assembler, label) {
  SETUP();

  Label label_1, label_2, label_3, label_4;

  START();
  __ Mov(x0, 0x1);
  __ Mov(x1, 0x0);
  __ Mov(x22, lr);    // Save lr.

  __ B(&label_1);
  __ B(&label_1);
  __ B(&label_1);     // Multiple branches to the same label.
  __ Mov(x0, 0x0);
  __ Bind(&label_2);
  __ B(&label_3);     // Forward branch.
  __ Mov(x0, 0x0);
  __ Bind(&label_1);
  __ B(&label_2);     // Backward branch.
  __ Mov(x0, 0x0);
  __ Bind(&label_3);
  __ Bl(&label_4);
  END();

  __ Bind(&label_4);
  __ Mov(x1, 0x1);
  __ Mov(lr, x22);
  END();

  RUN();

  ASSERT_EQUAL_64(0x1, x0);
  ASSERT_EQUAL_64(0x1, x1);

  TEARDOWN();
}


TEST(Assembler, adr) {
  SETUP();

  Label label_1, label_2, label_3, label_4;

  START();
  __ Mov(x0, 0x0);        // Set to non-zero to indicate failure.
  __ Adr(x1, &label_3);   // Set to zero to indicate success.

  __ Adr(x2, &label_1);   // Multiple forward references to the same label.
  __ Adr(x3, &label_1);
  __ Adr(x4, &label_1);

  __ Bind(&label_2);
  __ Eor(x5, x2, Operand(x3));  // Ensure that x2,x3 and x4 are identical.
  __ Eor(x6, x2, Operand(x4));
  __ Orr(x0, x0, Operand(x5));
  __ Orr(x0, x0, Operand(x6));
  __ Br(x2);  // label_1, label_3

  __ Bind(&label_3);
  __ Adr(x2, &label_3);   // Self-reference (offset 0).
  __ Eor(x1, x1, Operand(x2));
  __ Adr(x2, &label_4);   // Simple forward reference.
  __ Br(x2);  // label_4

  __ Bind(&label_1);
  __ Adr(x2, &label_3);   // Multiple reverse references to the same label.
  __ Adr(x3, &label_3);
  __ Adr(x4, &label_3);
  __ Adr(x5, &label_2);   // Simple reverse reference.
  __ Br(x5);  // label_2

  __ Bind(&label_4);
  END();

  RUN();

  ASSERT_EQUAL_64(0x0, x0);
  ASSERT_EQUAL_64(0x0, x1);

  TEARDOWN();
}


TEST(Assembler, branch_cond) {
  SETUP();

  Label wrong;

  START();
  __ Mov(x0, 0x1);
  __ Mov(x1, 0x1);
  __ Mov(x2, 0x8000000000000000L);

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

  Label ok_5;
  __ b(&ok_5, al);
  __ Mov(x0, 0x0);
  __ Bind(&ok_5);

  Label ok_6;
  __ b(&ok_6, nv);
  __ Mov(x0, 0x0);
  __ Bind(&ok_6);

  END();

  __ Bind(&wrong);
  __ Mov(x0, 0x0);
  END();

  RUN();

  ASSERT_EQUAL_64(0x1, x0);

  TEARDOWN();
}


TEST(Assembler, branch_to_reg) {
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
  Label fn2, after_fn2;

  __ Mov(x2, 0);
  __ B(&after_fn2);

  __ Bind(&fn2);
  __ Mov(x0, lr);
  __ Mov(x2, 84);
  __ Blr(x0);

  __ Bind(&after_fn2);
  __ Bl(&fn2);
  __ Mov(x3, lr);

  __ Mov(lr, x29);
  END();

  RUN();

  ASSERT_EQUAL_64(core.xreg(3) + kInstructionSize, x0);
  ASSERT_EQUAL_64(42, x1);
  ASSERT_EQUAL_64(84, x2);

  TEARDOWN();
}


TEST(Assembler, compare_branch) {
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

  __ Mov(x18, 0xffffffff00000000UL);

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

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(0, x1);
  ASSERT_EQUAL_64(1, x2);
  ASSERT_EQUAL_64(0, x3);
  ASSERT_EQUAL_64(1, x4);
  ASSERT_EQUAL_64(0, x5);

  TEARDOWN();
}


TEST(Assembler, test_branch) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0);
  __ Mov(x2, 0);
  __ Mov(x3, 0);
  __ Mov(x16, 0xaaaaaaaaaaaaaaaaUL);

  Label bz, bz_end;
  __ Tbz(x16, 0, &bz);
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
  __ Tbnz(x16, 2, &nbo);
  __ B(&nbo_end);
  __ Bind(&nbo);
  __ Mov(x3, 1);
  __ Bind(&nbo_end);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(0, x1);
  ASSERT_EQUAL_64(1, x2);
  ASSERT_EQUAL_64(0, x3);

  TEARDOWN();
}


TEST(Assembler, ldr_str_offset) {
  SETUP();

  uint64_t src[2] = {0xfedcba9876543210UL, 0x0123456789abcdefUL};
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

  RUN();

  ASSERT_EQUAL_64(0x76543210, x0);
  ASSERT_EQUAL_64(0x76543210, dst[0]);
  ASSERT_EQUAL_64(0xfedcba98, x1);
  ASSERT_EQUAL_64(0xfedcba9800000000UL, dst[1]);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, x2);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, dst[2]);
  ASSERT_EQUAL_64(0x32, x3);
  ASSERT_EQUAL_64(0x3200, dst[3]);
  ASSERT_EQUAL_64(0x7654, x4);
  ASSERT_EQUAL_64(0x765400, dst[4]);
  ASSERT_EQUAL_64(src_base, x17);
  ASSERT_EQUAL_64(dst_base, x18);

  TEARDOWN();
}


TEST(Assembler, ldr_str_wide) {
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

  TEARDOWN();
}


TEST(Assembler, ldr_str_preindex) {
  SETUP();

  uint64_t src[2] = {0xfedcba9876543210UL, 0x0123456789abcdefUL};
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

  RUN();

  ASSERT_EQUAL_64(0xfedcba98, x0);
  ASSERT_EQUAL_64(0xfedcba9800000000UL, dst[1]);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, x1);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, dst[2]);
  ASSERT_EQUAL_64(0x01234567, x2);
  ASSERT_EQUAL_64(0x0123456700000000UL, dst[4]);
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

  TEARDOWN();
}


TEST(Assembler, ldr_str_postindex) {
  SETUP();

  uint64_t src[2] = {0xfedcba9876543210UL, 0x0123456789abcdefUL};
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

  RUN();

  ASSERT_EQUAL_64(0xfedcba98, x0);
  ASSERT_EQUAL_64(0xfedcba9800000000UL, dst[1]);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, x1);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, dst[2]);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, x2);
  ASSERT_EQUAL_64(0x0123456789abcdefUL, dst[4]);
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

  TEARDOWN();
}


TEST(Assembler, ldr_str_largeindex) {
  SETUP();

  // This value won't fit in the immediate offset field of ldr/str instructions.
  int largeoffset = 0xabcdef;

  int64_t data[3] = { 0x1122334455667788, 0, 0 };
  uintptr_t base_addr = reinterpret_cast<uintptr_t>(data);
  uintptr_t drifted_addr = base_addr - largeoffset;

  // This test checks that we we can use large immediate offsets when
  // using PreIndex or PostIndex addressing mode of the MacroAssembler
  // Ldr/Str instructions.

  START();
  __ Mov(x17, drifted_addr);
  __ Ldr(x0, MemOperand(x17, largeoffset, PreIndex));

  __ Mov(x18, base_addr);
  __ Ldr(x1, MemOperand(x18, largeoffset, PostIndex));

  __ Mov(x19, drifted_addr);
  __ Str(x0, MemOperand(x19, largeoffset + 8, PreIndex));

  __ Mov(x20, base_addr + 16);
  __ Str(x0, MemOperand(x20, largeoffset, PostIndex));
  END();

  RUN();

  ASSERT_EQUAL_64(0x1122334455667788, data[0]);
  ASSERT_EQUAL_64(0x1122334455667788, data[1]);
  ASSERT_EQUAL_64(0x1122334455667788, data[2]);
  ASSERT_EQUAL_64(0x1122334455667788, x0);
  ASSERT_EQUAL_64(0x1122334455667788, x1);

  ASSERT_EQUAL_64(base_addr, x17);
  ASSERT_EQUAL_64(base_addr + largeoffset, x18);
  ASSERT_EQUAL_64(base_addr + 8, x19);
  ASSERT_EQUAL_64(base_addr + 16 + largeoffset, x20);

  TEARDOWN();
}


TEST(Assembler, load_signed) {
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

  RUN();

  ASSERT_EQUAL_64(0xffffff80, x0);
  ASSERT_EQUAL_64(0x0000007f, x1);
  ASSERT_EQUAL_64(0xffff8080, x2);
  ASSERT_EQUAL_64(0x00007f7f, x3);
  ASSERT_EQUAL_64(0xffffffffffffff80UL, x4);
  ASSERT_EQUAL_64(0x000000000000007fUL, x5);
  ASSERT_EQUAL_64(0xffffffffffff8080UL, x6);
  ASSERT_EQUAL_64(0x0000000000007f7fUL, x7);
  ASSERT_EQUAL_64(0xffffffff80008080UL, x8);
  ASSERT_EQUAL_64(0x000000007fff7f7fUL, x9);

  TEARDOWN();
}


TEST(Assembler, load_store_regoffset) {
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

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(0x0000000300000002UL, x1);
  ASSERT_EQUAL_64(3, x2);
  ASSERT_EQUAL_64(3, x3);
  ASSERT_EQUAL_64(2, x4);
  ASSERT_EQUAL_32(1, dst[0]);
  ASSERT_EQUAL_32(2, dst[1]);
  ASSERT_EQUAL_32(3, dst[2]);
  ASSERT_EQUAL_32(3, dst[3]);

  TEARDOWN();
}


TEST(Assembler, load_store_float) {
  SETUP();

  float src[3] = {1.0, 2.0, 3.0};
  float dst[3] = {0.0, 0.0, 0.0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base);
  __ Mov(x18, dst_base);
  __ Mov(x19, src_base);
  __ Mov(x20, dst_base);
  __ Mov(x21, src_base);
  __ Mov(x22, dst_base);
  __ Ldr(s0, MemOperand(x17, sizeof(src[0])));
  __ Str(s0, MemOperand(x18, sizeof(dst[0]), PostIndex));
  __ Ldr(s1, MemOperand(x19, sizeof(src[0]), PostIndex));
  __ Str(s1, MemOperand(x20, 2 * sizeof(dst[0]), PreIndex));
  __ Ldr(s2, MemOperand(x21, 2 * sizeof(src[0]), PreIndex));
  __ Str(s2, MemOperand(x22, sizeof(dst[0])));
  END();

  RUN();

  ASSERT_EQUAL_FP32(2.0, s0);
  ASSERT_EQUAL_FP32(2.0, dst[0]);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(1.0, dst[2]);
  ASSERT_EQUAL_FP32(3.0, s2);
  ASSERT_EQUAL_FP32(3.0, dst[1]);
  ASSERT_EQUAL_64(src_base, x17);
  ASSERT_EQUAL_64(dst_base + sizeof(dst[0]), x18);
  ASSERT_EQUAL_64(src_base + sizeof(src[0]), x19);
  ASSERT_EQUAL_64(dst_base + 2 * sizeof(dst[0]), x20);
  ASSERT_EQUAL_64(src_base + 2 * sizeof(src[0]), x21);
  ASSERT_EQUAL_64(dst_base, x22);

  TEARDOWN();
}


TEST(Assembler, load_store_double) {
  SETUP();

  double src[3] = {1.0, 2.0, 3.0};
  double dst[3] = {0.0, 0.0, 0.0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x17, src_base);
  __ Mov(x18, dst_base);
  __ Mov(x19, src_base);
  __ Mov(x20, dst_base);
  __ Mov(x21, src_base);
  __ Mov(x22, dst_base);
  __ Ldr(d0, MemOperand(x17, sizeof(src[0])));
  __ Str(d0, MemOperand(x18, sizeof(dst[0]), PostIndex));
  __ Ldr(d1, MemOperand(x19, sizeof(src[0]), PostIndex));
  __ Str(d1, MemOperand(x20, 2 * sizeof(dst[0]), PreIndex));
  __ Ldr(d2, MemOperand(x21, 2 * sizeof(src[0]), PreIndex));
  __ Str(d2, MemOperand(x22, sizeof(dst[0])));
  END();

  RUN();

  ASSERT_EQUAL_FP64(2.0, d0);
  ASSERT_EQUAL_FP64(2.0, dst[0]);
  ASSERT_EQUAL_FP64(1.0, d1);
  ASSERT_EQUAL_FP64(1.0, dst[2]);
  ASSERT_EQUAL_FP64(3.0, d2);
  ASSERT_EQUAL_FP64(3.0, dst[1]);
  ASSERT_EQUAL_64(src_base, x17);
  ASSERT_EQUAL_64(dst_base + sizeof(dst[0]), x18);
  ASSERT_EQUAL_64(src_base + sizeof(src[0]), x19);
  ASSERT_EQUAL_64(dst_base + 2 * sizeof(dst[0]), x20);
  ASSERT_EQUAL_64(src_base + 2 * sizeof(src[0]), x21);
  ASSERT_EQUAL_64(dst_base, x22);

  TEARDOWN();
}


TEST(Assembler, load_pc_relative) {
  SETUP();

  constexpr auto beforeValue = 0xdeadbeeffeedface;
  constexpr auto afterValue  = 0xf00dcafebeadf00c;

  START();
  Label dataBefore;
  Label dataAfter;
  Label codeStart;
  Label codeEnd;
  __ B   (&codeStart);
  __ bind(&dataBefore);
  __ dc64(beforeValue);
  __ bind(&codeStart);
  __ Ldr (x0, &dataBefore);
  __ Ldr (x1, &dataAfter);
  __ B   (&codeEnd);
  __ bind(&dataAfter);
  __ dc64(afterValue);
  __ bind(&codeEnd);
  END();

  RUN();

  ASSERT_EQUAL_64(beforeValue, x0);
  ASSERT_EQUAL_64(afterValue, x1);

  TEARDOWN();
}


TEST(Assembler, ldp_stp_float) {
  SETUP();

  float src[2] = {1.0, 2.0};
  float dst[3] = {0.0, 0.0, 0.0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Ldp(s31, s0, MemOperand(x16, 2 * sizeof(src[0]), PostIndex));
  __ Stp(s0, s31, MemOperand(x17, sizeof(dst[1]), PreIndex));
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.0, s31);
  ASSERT_EQUAL_FP32(2.0, s0);
  ASSERT_EQUAL_FP32(0.0, dst[0]);
  ASSERT_EQUAL_FP32(2.0, dst[1]);
  ASSERT_EQUAL_FP32(1.0, dst[2]);
  ASSERT_EQUAL_64(src_base + 2 * sizeof(src[0]), x16);
  ASSERT_EQUAL_64(dst_base + sizeof(dst[1]), x17);

  TEARDOWN();
}


TEST(Assembler, ldp_stp_double) {
  SETUP();

  double src[2] = {1.0, 2.0};
  double dst[3] = {0.0, 0.0, 0.0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Ldp(d31, d0, MemOperand(x16, 2 * sizeof(src[0]), PostIndex));
  __ Stp(d0, d31, MemOperand(x17, sizeof(dst[1]), PreIndex));
  END();

  RUN();

  ASSERT_EQUAL_FP64(1.0, d31);
  ASSERT_EQUAL_FP64(2.0, d0);
  ASSERT_EQUAL_FP64(0.0, dst[0]);
  ASSERT_EQUAL_FP64(2.0, dst[1]);
  ASSERT_EQUAL_FP64(1.0, dst[2]);
  ASSERT_EQUAL_64(src_base + 2 * sizeof(src[0]), x16);
  ASSERT_EQUAL_64(dst_base + sizeof(dst[1]), x17);

  TEARDOWN();
}


TEST(Assembler, ldp_stp_offset) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677UL, 0x8899aabbccddeeffUL,
                     0xffeeddccbbaa9988UL};
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

  RUN();

  ASSERT_EQUAL_64(0x44556677, x0);
  ASSERT_EQUAL_64(0x00112233, x1);
  ASSERT_EQUAL_64(0x0011223344556677UL, dst[0]);
  ASSERT_EQUAL_64(0x00112233, x2);
  ASSERT_EQUAL_64(0xccddeeff, x3);
  ASSERT_EQUAL_64(0xccddeeff00112233UL, dst[1]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x4);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, dst[2]);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, x5);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, dst[3]);
  ASSERT_EQUAL_64(0x8899aabb, x6);
  ASSERT_EQUAL_64(0xbbaa9988, x7);
  ASSERT_EQUAL_64(0xbbaa99888899aabbUL, dst[4]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x8);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, dst[5]);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, x9);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, dst[6]);
  ASSERT_EQUAL_64(src_base, x16);
  ASSERT_EQUAL_64(dst_base, x17);
  ASSERT_EQUAL_64(src_base + 24, x18);
  ASSERT_EQUAL_64(dst_base + 56, x19);

  TEARDOWN();
}


TEST(Assembler, ldnp_stnp_offset) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677UL, 0x8899aabbccddeeffUL,
                     0xffeeddccbbaa9988UL};
  uint64_t dst[7] = {0, 0, 0, 0, 0, 0, 0};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);
  uintptr_t dst_base = reinterpret_cast<uintptr_t>(dst);

  START();
  __ Mov(x16, src_base);
  __ Mov(x17, dst_base);
  __ Mov(x18, src_base + 24);
  __ Mov(x19, dst_base + 56);
  __ Ldnp(w0, w1, MemOperand(x16));
  __ Ldnp(w2, w3, MemOperand(x16, 4));
  __ Ldnp(x4, x5, MemOperand(x16, 8));
  __ Ldnp(w6, w7, MemOperand(x18, -12));
  __ Ldnp(x8, x9, MemOperand(x18, -16));
  __ Stnp(w0, w1, MemOperand(x17));
  __ Stnp(w2, w3, MemOperand(x17, 8));
  __ Stnp(x4, x5, MemOperand(x17, 16));
  __ Stnp(w6, w7, MemOperand(x19, -24));
  __ Stnp(x8, x9, MemOperand(x19, -16));
  END();

  RUN();

  ASSERT_EQUAL_64(0x44556677, x0);
  ASSERT_EQUAL_64(0x00112233, x1);
  ASSERT_EQUAL_64(0x0011223344556677UL, dst[0]);
  ASSERT_EQUAL_64(0x00112233, x2);
  ASSERT_EQUAL_64(0xccddeeff, x3);
  ASSERT_EQUAL_64(0xccddeeff00112233UL, dst[1]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x4);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, dst[2]);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, x5);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, dst[3]);
  ASSERT_EQUAL_64(0x8899aabb, x6);
  ASSERT_EQUAL_64(0xbbaa9988, x7);
  ASSERT_EQUAL_64(0xbbaa99888899aabbUL, dst[4]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x8);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, dst[5]);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, x9);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, dst[6]);
  ASSERT_EQUAL_64(src_base, x16);
  ASSERT_EQUAL_64(dst_base, x17);
  ASSERT_EQUAL_64(src_base + 24, x18);
  ASSERT_EQUAL_64(dst_base + 56, x19);

  TEARDOWN();
}


TEST(Assembler, ldp_stp_preindex) {
  SETUP();

  uint64_t src[3] = {0x0011223344556677UL, 0x8899aabbccddeeffUL,
                     0xffeeddccbbaa9988UL};
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

  RUN();

  ASSERT_EQUAL_64(0x00112233, x0);
  ASSERT_EQUAL_64(0xccddeeff, x1);
  ASSERT_EQUAL_64(0x44556677, x2);
  ASSERT_EQUAL_64(0x00112233, x3);
  ASSERT_EQUAL_64(0xccddeeff00112233UL, dst[0]);
  ASSERT_EQUAL_64(0x0000000000112233UL, dst[1]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x4);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, x5);
  ASSERT_EQUAL_64(0x0011223344556677UL, x6);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x7);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, dst[2]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, dst[3]);
  ASSERT_EQUAL_64(0x0011223344556677UL, dst[4]);
  ASSERT_EQUAL_64(src_base, x16);
  ASSERT_EQUAL_64(dst_base, x17);
  ASSERT_EQUAL_64(dst_base + 16, x18);
  ASSERT_EQUAL_64(src_base + 4, x19);
  ASSERT_EQUAL_64(dst_base + 4, x20);
  ASSERT_EQUAL_64(src_base + 8, x21);
  ASSERT_EQUAL_64(dst_base + 24, x22);

  TEARDOWN();
}


TEST(Assembler, ldp_stp_postindex) {
  SETUP();

  uint64_t src[4] = {0x0011223344556677UL, 0x8899aabbccddeeffUL,
                     0xffeeddccbbaa9988UL, 0x7766554433221100UL};
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

  RUN();

  ASSERT_EQUAL_64(0x44556677, x0);
  ASSERT_EQUAL_64(0x00112233, x1);
  ASSERT_EQUAL_64(0x00112233, x2);
  ASSERT_EQUAL_64(0xccddeeff, x3);
  ASSERT_EQUAL_64(0x4455667700112233UL, dst[0]);
  ASSERT_EQUAL_64(0x0000000000112233UL, dst[1]);
  ASSERT_EQUAL_64(0x0011223344556677UL, x4);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x5);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, x6);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, x7);
  ASSERT_EQUAL_64(0xffeeddccbbaa9988UL, dst[2]);
  ASSERT_EQUAL_64(0x8899aabbccddeeffUL, dst[3]);
  ASSERT_EQUAL_64(0x0011223344556677UL, dst[4]);
  ASSERT_EQUAL_64(src_base, x16);
  ASSERT_EQUAL_64(dst_base, x17);
  ASSERT_EQUAL_64(dst_base + 16, x18);
  ASSERT_EQUAL_64(src_base + 4, x19);
  ASSERT_EQUAL_64(dst_base + 4, x20);
  ASSERT_EQUAL_64(src_base + 8, x21);
  ASSERT_EQUAL_64(dst_base + 24, x22);

  TEARDOWN();
}


TEST(Assembler, ldp_sign_extend) {
  SETUP();

  uint32_t src[2] = {0x80000000, 0x7fffffff};
  uintptr_t src_base = reinterpret_cast<uintptr_t>(src);

  START();
  __ Mov(x24, src_base);
  __ Ldpsw(x0, x1, MemOperand(x24));
  END();

  RUN();

  ASSERT_EQUAL_64(0xffffffff80000000UL, x0);
  ASSERT_EQUAL_64(0x000000007fffffffUL, x1);

  TEARDOWN();
}


TEST(Assembler, ldur_stur) {
  SETUP();

  int64_t src[2] = {0x0123456789abcdefUL, 0x0123456789abcdefUL};
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

  RUN();

  ASSERT_EQUAL_64(0x6789abcd, x0);
  ASSERT_EQUAL_64(0x6789abcd0000L, dst[0]);
  ASSERT_EQUAL_64(0xabcdef0123456789L, x1);
  ASSERT_EQUAL_64(0xcdef012345678900L, dst[1]);
  ASSERT_EQUAL_64(0x000000ab, dst[2]);
  ASSERT_EQUAL_64(0xabcdef01, x2);
  ASSERT_EQUAL_64(0x00abcdef01000000L, dst[3]);
  ASSERT_EQUAL_64(0x00000001, x3);
  ASSERT_EQUAL_64(0x0100000000000000L, dst[4]);
  ASSERT_EQUAL_64(src_base, x17);
  ASSERT_EQUAL_64(dst_base, x18);
  ASSERT_EQUAL_64(src_base + 16, x19);
  ASSERT_EQUAL_64(dst_base + 32, x20);

  TEARDOWN();
}


TEST(Assembler, ldr_literal) {
  SETUP();

  START();
  __ Ldr(x2, 0x1234567890abcdefUL);
  __ Ldr(w3, 0xfedcba09);
  __ Ldr(d13, 1.234);
  __ Ldr(s25, 2.5);
  END();

  RUN();

  ASSERT_EQUAL_64(0x1234567890abcdefUL, x2);
  ASSERT_EQUAL_64(0xfedcba09, x3);
  ASSERT_EQUAL_FP64(1.234, d13);
  ASSERT_EQUAL_FP32(2.5, s25);

  TEARDOWN();
}


static void LdrLiteralRangeHelper(ptrdiff_t range_,
                                  LiteralPoolEmitOption option,
                                  bool expect_dump) {
  assert(range_ > 0);
  SETUP_SIZE(range_ + 1024);

  Label label_1, label_2;

  size_t range = static_cast<size_t>(range_);
  size_t code_size = 0;
  size_t pool_guard_size;

  if (option == NoJumpRequired) {
    // Space for an explicit branch.
    pool_guard_size = sizeof(Instr);
  } else {
    pool_guard_size = 0;
  }

  START();
  // Force a pool dump so the pool starts off empty.
  __ EmitLiteralPool(JumpRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  __ Ldr(x0, 0x1234567890abcdefUL);
  __ Ldr(w1, 0xfedcba09);
  __ Ldr(d0, 1.234);
  __ Ldr(s1, 2.5);
  ASSERT_LITERAL_POOL_SIZE(24);

  code_size += 4 * sizeof(Instr);

  // Check that the requested range (allowing space for a branch over the pool)
  // can be handled by this test.
  assert((code_size + pool_guard_size) <= range);

  // Emit NOPs up to 'range', leaving space for the pool guard.
  while ((code_size + pool_guard_size) < range) {
    __ Nop();
    code_size += sizeof(Instr);
  }

  // Emit the guard sequence before the literal pool.
  if (option == NoJumpRequired) {
    __ B(&label_1);
    code_size += sizeof(Instr);
  }

  assert(code_size == range);
  ASSERT_LITERAL_POOL_SIZE(24);

  // Possibly generate a literal pool.
  __ CheckLiteralPool(option);
  __ Bind(&label_1);
  if (expect_dump) {
    ASSERT_LITERAL_POOL_SIZE(0);
  } else {
    ASSERT_LITERAL_POOL_SIZE(24);
  }

  // Force a pool flush to check that a second pool functions correctly.
  __ EmitLiteralPool(JumpRequired);
  ASSERT_LITERAL_POOL_SIZE(0);

  // These loads should be after the pool (and will require a new one).
  __ Ldr(x4, 0x34567890abcdef12UL);
  __ Ldr(w5, 0xdcba09fe);
  __ Ldr(d4, 123.4);
  __ Ldr(s5, 250.0);
  ASSERT_LITERAL_POOL_SIZE(24);
  END();

  RUN();

  // Check that the literals loaded correctly.
  ASSERT_EQUAL_64(0x1234567890abcdefUL, x0);
  ASSERT_EQUAL_64(0xfedcba09, x1);
  ASSERT_EQUAL_FP64(1.234, d0);
  ASSERT_EQUAL_FP32(2.5, s1);
  ASSERT_EQUAL_64(0x34567890abcdef12UL, x4);
  ASSERT_EQUAL_64(0xdcba09fe, x5);
  ASSERT_EQUAL_FP64(123.4, d4);
  ASSERT_EQUAL_FP32(250.0, s5);

  TEARDOWN();
}


TEST(Assembler, ldr_literal_range_1) {
  LdrLiteralRangeHelper(kRecommendedLiteralPoolRange,
                        NoJumpRequired,
                        true);
}


TEST(Assembler, ldr_literal_range_2) {
  LdrLiteralRangeHelper(kRecommendedLiteralPoolRange-sizeof(Instr),
                        NoJumpRequired,
                        false);
}


TEST(Assembler, ldr_literal_range_3) {
  LdrLiteralRangeHelper(2 * kRecommendedLiteralPoolRange,
                        JumpRequired,
                        true);
}


TEST(Assembler, ldr_literal_range_4) {
  LdrLiteralRangeHelper(2 * kRecommendedLiteralPoolRange-sizeof(Instr),
                        JumpRequired,
                        false);
}


TEST(Assembler, ldr_literal_range_5) {
  LdrLiteralRangeHelper(kLiteralPoolCheckInterval,
                        JumpRequired,
                        false);
}


TEST(Assembler, ldr_literal_range_6) {
  LdrLiteralRangeHelper(kLiteralPoolCheckInterval-sizeof(Instr),
                        JumpRequired,
                        false);
}


TEST(Assembler, add_sub_imm) {
  SETUP();

  START();
  __ Mov(x0, 0x0);
  __ Mov(x1, 0x1111);
  __ Mov(x2, 0xffffffffffffffffL);
  __ Mov(x3, 0x8000000000000000L);

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

  RUN();

  ASSERT_EQUAL_64(0x123, x10);
  ASSERT_EQUAL_64(0x123111, x11);
  ASSERT_EQUAL_64(0xabc000, x12);
  ASSERT_EQUAL_64(0x0, x13);

  ASSERT_EQUAL_32(0x123, w14);
  ASSERT_EQUAL_32(0x123111, w15);
  ASSERT_EQUAL_32(0xabc000, w16);
  ASSERT_EQUAL_32(0x0, w17);

  ASSERT_EQUAL_64(0xffffffffffffffffL, x20);
  ASSERT_EQUAL_64(0x1000, x21);
  ASSERT_EQUAL_64(0x111, x22);
  ASSERT_EQUAL_64(0x7fffffffffffffffL, x23);

  ASSERT_EQUAL_32(0xffffffff, w24);
  ASSERT_EQUAL_32(0x1000, w25);
  ASSERT_EQUAL_32(0x111, w26);
  ASSERT_EQUAL_32(0xffffffff, w27);

  TEARDOWN();
}


TEST(Assembler, add_sub_wide_imm) {
  SETUP();

  START();
  __ Mov(x0, 0x0);
  __ Mov(x1, 0x1);

  __ Add(x10, x0, Operand(0x1234567890abcdefUL));
  __ Add(x11, x1, Operand(0xffffffff));

  __ Add(w12, w0, Operand(0x12345678));
  __ Add(w13, w1, Operand(0xffffffff));

  __ Sub(x20, x0, Operand(0x1234567890abcdefUL));

  __ Sub(w21, w0, Operand(0x12345678));
  END();

  RUN();

  ASSERT_EQUAL_64(0x1234567890abcdefUL, x10);
  ASSERT_EQUAL_64(0x100000000UL, x11);

  ASSERT_EQUAL_32(0x12345678, w12);
  ASSERT_EQUAL_64(0x0, x13);

  ASSERT_EQUAL_64(-0x1234567890abcdefUL, x20);

  ASSERT_EQUAL_32(-0x12345678, w21);

  TEARDOWN();
}


TEST(Assembler, add_sub_shifted) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x0123456789abcdefL);
  __ Mov(x2, 0xfedcba9876543210L);
  __ Mov(x3, 0xffffffffffffffffL);

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

  RUN();

  ASSERT_EQUAL_64(0xffffffffffffffffL, x10);
  ASSERT_EQUAL_64(0x23456789abcdef00L, x11);
  ASSERT_EQUAL_64(0x000123456789abcdL, x12);
  ASSERT_EQUAL_64(0x000123456789abcdL, x13);
  ASSERT_EQUAL_64(0xfffedcba98765432L, x14);
  ASSERT_EQUAL_64(0xff89abcd, x15);
  ASSERT_EQUAL_64(0xef89abcc, x18);
  ASSERT_EQUAL_64(0xef0123456789abccL, x19);

  ASSERT_EQUAL_64(0x0123456789abcdefL, x20);
  ASSERT_EQUAL_64(0xdcba9876543210ffL, x21);
  ASSERT_EQUAL_64(0xfffedcba98765432L, x22);
  ASSERT_EQUAL_64(0xfffedcba98765432L, x23);
  ASSERT_EQUAL_64(0x000123456789abcdL, x24);
  ASSERT_EQUAL_64(0x00765432, x25);
  ASSERT_EQUAL_64(0x10765432, x26);
  ASSERT_EQUAL_64(0x10fedcba98765432L, x27);

  TEARDOWN();
}


TEST(Assembler, add_sub_extended) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x0123456789abcdefL);
  __ Mov(x2, 0xfedcba9876543210L);
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

  RUN();

  ASSERT_EQUAL_64(0xefL, x10);
  ASSERT_EQUAL_64(0x1deL, x11);
  ASSERT_EQUAL_64(0x337bcL, x12);
  ASSERT_EQUAL_64(0x89abcdef0L, x13);

  ASSERT_EQUAL_64(0xffffffffffffffefL, x14);
  ASSERT_EQUAL_64(0xffffffffffffffdeL, x15);
  ASSERT_EQUAL_64(0xffffffffffff37bcL, x16);
  ASSERT_EQUAL_64(0xfffffffc4d5e6f78L, x17);
  ASSERT_EQUAL_64(0x10L, x18);
  ASSERT_EQUAL_64(0x20L, x19);
  ASSERT_EQUAL_64(0xc840L, x20);
  ASSERT_EQUAL_64(0x3b2a19080L, x21);

  ASSERT_EQUAL_64(0x0123456789abce0fL, x22);
  ASSERT_EQUAL_64(0x0123456789abcdcfL, x23);

  ASSERT_EQUAL_32(0x89abce2f, w24);
  ASSERT_EQUAL_32(0xffffffef, w25);
  ASSERT_EQUAL_32(0xffffffde, w26);
  ASSERT_EQUAL_32(0xc3b2a188, w27);

  ASSERT_EQUAL_32(0x4d5e6f78, w28);
  ASSERT_EQUAL_64(0xfffffffc4d5e6f78L, x29);

  ASSERT_EQUAL_64(256, x30);

  TEARDOWN();
}


TEST(Assembler, add_sub_negative) {
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

  TEARDOWN();
}


TEST(Assembler, neg) {
  SETUP();

  START();
  __ Mov(x0, 0xf123456789abcdefL);

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

  RUN();

  ASSERT_EQUAL_64(0xfffffffffffffeddUL, x1);
  ASSERT_EQUAL_64(0xfffffedd, x2);
  ASSERT_EQUAL_64(0x1db97530eca86422UL, x3);
  ASSERT_EQUAL_64(0xd950c844, x4);
  ASSERT_EQUAL_64(0xe1db97530eca8643UL, x5);
  ASSERT_EQUAL_64(0xf7654322, x6);
  ASSERT_EQUAL_64(0x0076e5d4c3b2a191UL, x7);
  ASSERT_EQUAL_64(0x01d950c9, x8);
  ASSERT_EQUAL_64(0xffffff11, x9);
  ASSERT_EQUAL_64(0x0000000000000022UL, x10);
  ASSERT_EQUAL_64(0xfffcc844, x11);
  ASSERT_EQUAL_64(0x0000000000019088UL, x12);
  ASSERT_EQUAL_64(0x65432110, x13);
  ASSERT_EQUAL_64(0x0000000765432110UL, x14);

  TEARDOWN();
}


TEST(Assembler, adc_sbc_shift) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x2, 0x0123456789abcdefL);
  __ Mov(x3, 0xfedcba9876543210L);
  __ Mov(x4, 0xffffffffffffffffL);

  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);

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

  RUN();

  ASSERT_EQUAL_64(0xffffffffffffffffL, x5);
  ASSERT_EQUAL_64(1L << 60, x6);
  ASSERT_EQUAL_64(0xf0123456789abcddL, x7);
  ASSERT_EQUAL_64(0x0111111111111110L, x8);
  ASSERT_EQUAL_64(0x1222222222222221L, x9);

  ASSERT_EQUAL_32(0xffffffff, w10);
  ASSERT_EQUAL_32(1 << 30, w11);
  ASSERT_EQUAL_32(0xf89abcdd, w12);
  ASSERT_EQUAL_32(0x91111110, w13);
  ASSERT_EQUAL_32(0x9a222221, w14);

  ASSERT_EQUAL_64(0xffffffffffffffffL + 1, x18);
  ASSERT_EQUAL_64((1L << 60) + 1, x19);
  ASSERT_EQUAL_64(0xf0123456789abcddL + 1, x20);
  ASSERT_EQUAL_64(0x0111111111111110L + 1, x21);
  ASSERT_EQUAL_64(0x1222222222222221L + 1, x22);

  ASSERT_EQUAL_32(0xffffffff + 1, w23);
  ASSERT_EQUAL_32((1 << 30) + 1, w24);
  ASSERT_EQUAL_32(0xf89abcdd + 1, w25);
  ASSERT_EQUAL_32(0x91111110 + 1, w26);
  ASSERT_EQUAL_32(0x9a222221 + 1, w27);

  // Check that adc correctly sets the condition flags.
  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0xffffffffffffffffL);
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Adc(x10, x0, Operand(x1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x8000000000000000L);
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Adc(x10, x0, Operand(x1, ASR, 63), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  START();
  __ Mov(x0, 0x10);
  __ Mov(x1, 0x07ffffffffffffffL);
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Adc(x10, x0, Operand(x1, LSL, 4), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NVFlag);

  TEARDOWN();
}


TEST(Assembler, adc_sbc_extend) {
  SETUP();

  START();
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);

  __ Mov(x0, 0);
  __ Mov(x1, 1);
  __ Mov(x2, 0x0123456789abcdefL);

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

  RUN();

  ASSERT_EQUAL_64(0x1df, x10);
  ASSERT_EQUAL_64(0xffffffffffff37bdL, x11);
  ASSERT_EQUAL_64(0xfffffff765432110L, x12);
  ASSERT_EQUAL_64(0x123456789abcdef1L, x13);

  ASSERT_EQUAL_32(0x1df, w14);
  ASSERT_EQUAL_32(0xffff37bd, w15);
  ASSERT_EQUAL_32(0x9abcdef1, w9);

  ASSERT_EQUAL_64(0x1df + 1, x20);
  ASSERT_EQUAL_64(0xffffffffffff37bdL + 1, x21);
  ASSERT_EQUAL_64(0xfffffff765432110L + 1, x22);
  ASSERT_EQUAL_64(0x123456789abcdef1L + 1, x23);

  ASSERT_EQUAL_32(0x1df + 1, w24);
  ASSERT_EQUAL_32(0xffff37bd + 1, w25);
  ASSERT_EQUAL_32(0x9abcdef1 + 1, w26);

  // Check that adc correctly sets the condition flags.
  START();
  __ Mov(x0, 0xff);
  __ Mov(x1, 0xffffffffffffffffL);
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Adc(x10, x0, Operand(x1, SXTX, 1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(CFlag);

  START();
  __ Mov(x0, 0x7fffffffffffffffL);
  __ Mov(x1, 1);
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Adc(x10, x0, Operand(x1, UXTB, 2), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NVFlag);

  START();
  __ Mov(x0, 0x7fffffffffffffffL);
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Adc(x10, x0, Operand(1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NVFlag);

  TEARDOWN();
}


TEST(Assembler, adc_sbc_wide_imm) {
  SETUP();

  START();
  __ Mov(x0, 0);

  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);

  __ Adc(x7, x0, Operand(0x1234567890abcdefUL));
  __ Adc(w8, w0, Operand(0xffffffff));

  // Set the C flag.
  __ Cmp(w0, Operand(w0));

  __ Adc(x27, x0, Operand(0x1234567890abcdefUL));
  __ Adc(w28, w0, Operand(0xffffffff));
  END();

  RUN();

  ASSERT_EQUAL_64(0x1234567890abcdefUL, x7);
  ASSERT_EQUAL_64(0xffffffff, x8);
  ASSERT_EQUAL_64(0x1234567890abcdefUL + 1, x27);
  ASSERT_EQUAL_64(0, x28);

  TEARDOWN();
}

TEST(Assembler, flags) {
  SETUP();

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x1111111111111111L);
  __ Neg(x10, Operand(x0));
  __ Neg(x11, Operand(x1));
  __ Neg(w12, Operand(w1));
  // Clear the C flag.
  __ Add(x0, x0, Operand(0), SetFlags);
  __ Ngc(x13, Operand(x0));
  // Set the C flag.
  __ Cmp(x0, Operand(x0));
  __ Ngc(w14, Operand(w0));
  END();

  RUN();

  ASSERT_EQUAL_64(0, x10);
  ASSERT_EQUAL_64(-0x1111111111111111L, x11);
  ASSERT_EQUAL_32(-0x11111111, w12);
  ASSERT_EQUAL_64(-1L, x13);
  ASSERT_EQUAL_32(0, w14);

  START();
  __ Mov(x0, 0);
  __ Cmp(x0, Operand(x0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  START();
  __ Mov(w0, 0);
  __ Cmp(w0, Operand(w0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  START();
  __ Mov(x0, 0);
  __ Mov(x1, 0x1111111111111111L);
  __ Cmp(x0, Operand(x1));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 0x11111111);
  __ Cmp(w0, Operand(w1));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);

  START();
  __ Mov(x1, 0x1111111111111111L);
  __ Cmp(x1, Operand(0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(CFlag);

  START();
  __ Mov(w1, 0x11111111);
  __ Cmp(w1, Operand(0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(CFlag);

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0x7fffffffffffffffL);
  __ Cmn(x1, Operand(x0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NVFlag);

  START();
  __ Mov(w0, 1);
  __ Mov(w1, 0x7fffffff);
  __ Cmn(w1, Operand(w0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NVFlag);

  START();
  __ Mov(x0, 1);
  __ Mov(x1, 0xffffffffffffffffL);
  __ Cmn(x1, Operand(x0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  START();
  __ Mov(w0, 1);
  __ Mov(w1, 0xffffffff);
  __ Cmn(w1, Operand(w0));
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 1);
  // Clear the C flag.
  __ Add(w0, w0, Operand(0), SetFlags);
  __ Ngc(w0, Operand(w1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(NFlag);

  START();
  __ Mov(w0, 0);
  __ Mov(w1, 0);
  // Set the C flag.
  __ Cmp(w0, Operand(w0));
  __ Ngc(w0, Operand(w1), SetFlags);
  END();

  RUN();

  ASSERT_EQUAL_NZCV(ZCFlag);

  TEARDOWN();
}


TEST(Assembler, cmp_shift) {
  SETUP();

  START();
  __ Mov(x18, 0xf0000000);
  __ Mov(x19, 0xf000000010000000UL);
  __ Mov(x20, 0xf0000000f0000000UL);
  __ Mov(x21, 0x7800000078000000UL);
  __ Mov(x22, 0x3c0000003c000000UL);
  __ Mov(x23, 0x8000000780000000UL);
  __ Mov(x24, 0x0000000f00000000UL);
  __ Mov(x25, 0x00000003c0000000UL);
  __ Mov(x26, 0x8000000780000000UL);
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

  RUN();

  ASSERT_EQUAL_32(ZCFlag, w0);
  ASSERT_EQUAL_32(ZCFlag, w1);
  ASSERT_EQUAL_32(ZCFlag, w2);
  ASSERT_EQUAL_32(ZCFlag, w3);
  ASSERT_EQUAL_32(ZCFlag, w4);
  ASSERT_EQUAL_32(ZCFlag, w5);
  ASSERT_EQUAL_32(ZCFlag, w6);
  ASSERT_EQUAL_32(ZCFlag, w7);

  TEARDOWN();
}


TEST(Assembler, cmp_extend) {
  SETUP();

  START();
  __ Mov(w20, 0x2);
  __ Mov(w21, 0x1);
  __ Mov(x22, 0xffffffffffffffffUL);
  __ Mov(x23, 0xff);
  __ Mov(x24, 0xfffffffffffffffeUL);
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

  RUN();

  ASSERT_EQUAL_32(ZCFlag, w0);
  ASSERT_EQUAL_32(ZCFlag, w1);
  ASSERT_EQUAL_32(ZCFlag, w2);
  ASSERT_EQUAL_32(NCFlag, w3);
  ASSERT_EQUAL_32(NCFlag, w4);
  ASSERT_EQUAL_32(ZCFlag, w5);
  ASSERT_EQUAL_32(NCFlag, w6);
  ASSERT_EQUAL_32(ZCFlag, w7);

  TEARDOWN();
}


TEST(Assembler, ccmp) {
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

  __ ccmp(x16, x16, NZCVFlag, al);
  __ Mrs(x4, NZCV);

  __ ccmp(x16, x16, NZCVFlag, nv);
  __ Mrs(x5, NZCV);

  END();

  RUN();

  ASSERT_EQUAL_32(NFlag, w0);
  ASSERT_EQUAL_32(NCFlag, w1);
  ASSERT_EQUAL_32(NoFlag, w2);
  ASSERT_EQUAL_32(NZCVFlag, w3);
  ASSERT_EQUAL_32(ZCFlag, w4);
  ASSERT_EQUAL_32(ZCFlag, w5);

  TEARDOWN();
}


TEST(Assembler, ccmp_wide_imm) {
  SETUP();

  START();
  __ Mov(w20, 0);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(w20, Operand(0x12345678), NZCVFlag, eq);
  __ Mrs(x0, NZCV);

  __ Cmp(w20, Operand(w20));
  __ Ccmp(x20, Operand(0xffffffffffffffffUL), NZCVFlag, eq);
  __ Mrs(x1, NZCV);
  END();

  RUN();

  ASSERT_EQUAL_32(NFlag, w0);
  ASSERT_EQUAL_32(NoFlag, w1);

  TEARDOWN();
}


TEST(Assembler, ccmp_shift_extend) {
  SETUP();

  START();
  __ Mov(w20, 0x2);
  __ Mov(w21, 0x1);
  __ Mov(x22, 0xffffffffffffffffUL);
  __ Mov(x23, 0xff);
  __ Mov(x24, 0xfffffffffffffffeUL);

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

  RUN();

  ASSERT_EQUAL_32(ZCFlag, w0);
  ASSERT_EQUAL_32(ZCFlag, w1);
  ASSERT_EQUAL_32(ZCFlag, w2);
  ASSERT_EQUAL_32(NCFlag, w3);
  ASSERT_EQUAL_32(NZCVFlag, w4);

  TEARDOWN();
}


TEST(Assembler, csel) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Mov(x24, 0x0000000f0000000fUL);
  __ Mov(x25, 0x0000001f0000001fUL);

  __ Cmp(w16, Operand(0));
  __ Csel(w0, w24, w25, eq);
  __ Csel(w1, w24, w25, ne);
  __ Csinc(w2, w24, w25, mi);
  __ Csinc(w3, w24, w25, pl);

  __ csel(w13, w24, w25, al);
  __ csel(x14, x24, x25, nv);

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

  __ csel(w15, w24, w25, al);
  __ csel(x17, x24, x25, nv);

  END();

  RUN();

  ASSERT_EQUAL_64(0x0000000f, x0);
  ASSERT_EQUAL_64(0x0000001f, x1);
  ASSERT_EQUAL_64(0x00000020, x2);
  ASSERT_EQUAL_64(0x0000000f, x3);
  ASSERT_EQUAL_64(0xffffffe0ffffffe0UL, x4);
  ASSERT_EQUAL_64(0x0000000f0000000fUL, x5);
  ASSERT_EQUAL_64(0xffffffe0ffffffe1UL, x6);
  ASSERT_EQUAL_64(0x0000000f0000000fUL, x7);
  ASSERT_EQUAL_64(0x00000001, x8);
  ASSERT_EQUAL_64(0xffffffff, x9);
  ASSERT_EQUAL_64(0x0000001f00000020UL, x10);
  ASSERT_EQUAL_64(0xfffffff0fffffff0UL, x11);
  ASSERT_EQUAL_64(0xfffffff0fffffff1UL, x12);
  ASSERT_EQUAL_64(0x0000000f, x13);
  ASSERT_EQUAL_64(0x0000000f0000000fUL, x14);
  ASSERT_EQUAL_64(0x0000000f, x15);
  ASSERT_EQUAL_64(0x0000000f0000000fUL, x17);

  TEARDOWN();
}


TEST(Assembler, lslv) {
  SETUP();

  uint64_t value = 0x0123456789abcdefUL;
  int shift[] = {1, 3, 5, 9, 17, 33};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  __ lslv(x0, x0, xzr);

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

  TEARDOWN();
}


TEST(Assembler, lsrv) {
  SETUP();

  uint64_t value = 0x0123456789abcdefUL;
  int shift[] = {1, 3, 5, 9, 17, 33};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  __ lsrv(x0, x0, xzr);

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

  RUN();

  ASSERT_EQUAL_64(value, x0);
  ASSERT_EQUAL_64(value >> (shift[0] & 63), x16);
  ASSERT_EQUAL_64(value >> (shift[1] & 63), x17);
  ASSERT_EQUAL_64(value >> (shift[2] & 63), x18);
  ASSERT_EQUAL_64(value >> (shift[3] & 63), x19);
  ASSERT_EQUAL_64(value >> (shift[4] & 63), x20);
  ASSERT_EQUAL_64(value >> (shift[5] & 63), x21);

  value &= 0xffffffffUL;
  ASSERT_EQUAL_32(value >> (shift[0] & 31), w22);
  ASSERT_EQUAL_32(value >> (shift[1] & 31), w23);
  ASSERT_EQUAL_32(value >> (shift[2] & 31), w24);
  ASSERT_EQUAL_32(value >> (shift[3] & 31), w25);
  ASSERT_EQUAL_32(value >> (shift[4] & 31), w26);
  ASSERT_EQUAL_32(value >> (shift[5] & 31), w27);

  TEARDOWN();
}


TEST(Assembler, asrv) {
  SETUP();

  int64_t value = 0xfedcba98fedcba98UL;
  int shift[] = {1, 3, 5, 9, 17, 33};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  __ asrv(x0, x0, xzr);

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

  RUN();

  ASSERT_EQUAL_64(value, x0);
  ASSERT_EQUAL_64(value >> (shift[0] & 63), x16);
  ASSERT_EQUAL_64(value >> (shift[1] & 63), x17);
  ASSERT_EQUAL_64(value >> (shift[2] & 63), x18);
  ASSERT_EQUAL_64(value >> (shift[3] & 63), x19);
  ASSERT_EQUAL_64(value >> (shift[4] & 63), x20);
  ASSERT_EQUAL_64(value >> (shift[5] & 63), x21);

  int32_t value32 = static_cast<int32_t>(value & 0xffffffffUL);
  ASSERT_EQUAL_32(value32 >> (shift[0] & 31), w22);
  ASSERT_EQUAL_32(value32 >> (shift[1] & 31), w23);
  ASSERT_EQUAL_32(value32 >> (shift[2] & 31), w24);
  ASSERT_EQUAL_32(value32 >> (shift[3] & 31), w25);
  ASSERT_EQUAL_32(value32 >> (shift[4] & 31), w26);
  ASSERT_EQUAL_32(value32 >> (shift[5] & 31), w27);

  TEARDOWN();
}


TEST(Assembler, rorv) {
  SETUP();

  uint64_t value = 0x0123456789abcdefUL;
  int shift[] = {4, 8, 12, 16, 24, 36};

  START();
  __ Mov(x0, value);
  __ Mov(w1, shift[0]);
  __ Mov(w2, shift[1]);
  __ Mov(w3, shift[2]);
  __ Mov(w4, shift[3]);
  __ Mov(w5, shift[4]);
  __ Mov(w6, shift[5]);

  __ rorv(x0, x0, xzr);

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

  RUN();

  ASSERT_EQUAL_64(value, x0);
  ASSERT_EQUAL_64(0xf0123456789abcdeUL, x16);
  ASSERT_EQUAL_64(0xef0123456789abcdUL, x17);
  ASSERT_EQUAL_64(0xdef0123456789abcUL, x18);
  ASSERT_EQUAL_64(0xcdef0123456789abUL, x19);
  ASSERT_EQUAL_64(0xabcdef0123456789UL, x20);
  ASSERT_EQUAL_64(0x789abcdef0123456UL, x21);
  ASSERT_EQUAL_32(0xf89abcde, w22);
  ASSERT_EQUAL_32(0xef89abcd, w23);
  ASSERT_EQUAL_32(0xdef89abc, w24);
  ASSERT_EQUAL_32(0xcdef89ab, w25);
  ASSERT_EQUAL_32(0xabcdef89, w26);
  ASSERT_EQUAL_32(0xf89abcde, w27);

  TEARDOWN();
}


TEST(Assembler, bfm) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdefL);

  __ Mov(x10, 0x8888888888888888L);
  __ Mov(x11, 0x8888888888888888L);
  __ Mov(x12, 0x8888888888888888L);
  __ Mov(x13, 0x8888888888888888L);
  __ Mov(w20, 0x88888888);
  __ Mov(w21, 0x88888888);

  __ bfm(x10, x1, 16, 31);
  __ bfm(x11, x1, 32, 15);

  __ bfm(w20, w1, 16, 23);
  __ bfm(w21, w1, 24, 15);

  // Aliases.
  __ Bfi(x12, x1, 16, 8);
  __ Bfxil(x13, x1, 16, 8);
  END();

  RUN();


  ASSERT_EQUAL_64(0x88888888888889abL, x10);
  ASSERT_EQUAL_64(0x8888cdef88888888L, x11);

  ASSERT_EQUAL_32(0x888888ab, w20);
  ASSERT_EQUAL_32(0x88cdef88, w21);

  ASSERT_EQUAL_64(0x8888888888ef8888L, x12);
  ASSERT_EQUAL_64(0x88888888888888abL, x13);

  TEARDOWN();
}


TEST(Assembler, sbfm) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdefL);
  __ Mov(x2, 0xfedcba9876543210L);

  __ sbfm(x10, x1, 16, 31);
  __ sbfm(x11, x1, 32, 15);
  __ sbfm(x12, x1, 32, 47);
  __ sbfm(x13, x1, 48, 35);

  __ sbfm(w14, w1, 16, 23);
  __ sbfm(w15, w1, 24, 15);
  __ sbfm(w16, w2, 16, 23);
  __ sbfm(w17, w2, 24, 15);

  // Aliases.
  __ Asr(x18, x1, 32);
  __ Asr(x19, x2, 32);
  __ Sbfiz(x20, x1, 8, 16);
  __ Sbfiz(x21, x2, 8, 16);
  __ Sbfx(x22, x1, 8, 16);
  __ Sbfx(x23, x2, 8, 16);
  __ Sxtb(x24, x1);
  __ Sxtb(x25, x2);
  __ Sxth(x26, x1);
  __ Sxth(x27, x2);
  __ Sxtw(x28, x1);
  __ Sxtw(x29, x2);
  END();

  RUN();


  ASSERT_EQUAL_64(0xffffffffffff89abL, x10);
  ASSERT_EQUAL_64(0xffffcdef00000000L, x11);
  ASSERT_EQUAL_64(0x4567L, x12);
  ASSERT_EQUAL_64(0x789abcdef0000L, x13);

  ASSERT_EQUAL_32(0xffffffab, w14);
  ASSERT_EQUAL_32(0xffcdef00, w15);
  ASSERT_EQUAL_32(0x54, w16);
  ASSERT_EQUAL_32(0x00321000, w17);

  ASSERT_EQUAL_64(0x01234567L, x18);
  ASSERT_EQUAL_64(0xfffffffffedcba98L, x19);
  ASSERT_EQUAL_64(0xffffffffffcdef00L, x20);
  ASSERT_EQUAL_64(0x321000L, x21);
  ASSERT_EQUAL_64(0xffffffffffffabcdL, x22);
  ASSERT_EQUAL_64(0x5432L, x23);
  ASSERT_EQUAL_64(0xffffffffffffffefL, x24);
  ASSERT_EQUAL_64(0x10, x25);
  ASSERT_EQUAL_64(0xffffffffffffcdefL, x26);
  ASSERT_EQUAL_64(0x3210, x27);
  ASSERT_EQUAL_64(0xffffffff89abcdefL, x28);
  ASSERT_EQUAL_64(0x76543210, x29);

  TEARDOWN();
}


TEST(Assembler, ubfm) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdefL);
  __ Mov(x2, 0xfedcba9876543210L);

  __ Mov(x10, 0x8888888888888888L);
  __ Mov(x11, 0x8888888888888888L);

  __ ubfm(x10, x1, 16, 31);
  __ ubfm(x11, x1, 32, 15);
  __ ubfm(x12, x1, 32, 47);
  __ ubfm(x13, x1, 48, 35);

  __ ubfm(w25, w1, 16, 23);
  __ ubfm(w26, w1, 24, 15);
  __ ubfm(w27, w2, 16, 23);
  __ ubfm(w28, w2, 24, 15);

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

  RUN();

  ASSERT_EQUAL_64(0x00000000000089abL, x10);
  ASSERT_EQUAL_64(0x0000cdef00000000L, x11);
  ASSERT_EQUAL_64(0x4567L, x12);
  ASSERT_EQUAL_64(0x789abcdef0000L, x13);

  ASSERT_EQUAL_32(0x000000ab, w25);
  ASSERT_EQUAL_32(0x00cdef00, w26);
  ASSERT_EQUAL_32(0x54, w27);
  ASSERT_EQUAL_32(0x00321000, w28);

  ASSERT_EQUAL_64(0x8000000000000000L, x15);
  ASSERT_EQUAL_64(0x0123456789abcdefL, x16);
  ASSERT_EQUAL_64(0x01234567L, x17);
  ASSERT_EQUAL_64(0xcdef00L, x18);
  ASSERT_EQUAL_64(0xabcdL, x19);
  ASSERT_EQUAL_64(0xefL, x20);
  ASSERT_EQUAL_64(0xcdefL, x21);
  ASSERT_EQUAL_64(0x89abcdefL, x22);

  TEARDOWN();
}


TEST(Assembler, extr) {
  SETUP();

  START();
  __ Mov(x1, 0x0123456789abcdefL);
  __ Mov(x2, 0xfedcba9876543210L);

  __ Extr(w10, w1, w2, 0);
  __ Extr(w11, w1, w2, 1);
  __ Extr(x12, x2, x1, 2);

  __ Ror(w13, w1, 0);
  __ Ror(w14, w2, 17);
  __ Ror(w15, w1, 31);
  __ Ror(x18, x2, 1);
  __ Ror(x19, x1, 63);
  END();

  RUN();

  ASSERT_EQUAL_64(0x76543210, x10);
  ASSERT_EQUAL_64(0xbb2a1908, x11);
  ASSERT_EQUAL_64(0x0048d159e26af37bUL, x12);
  ASSERT_EQUAL_64(0x89abcdef, x13);
  ASSERT_EQUAL_64(0x19083b2a, x14);
  ASSERT_EQUAL_64(0x13579bdf, x15);
  ASSERT_EQUAL_64(0x7f6e5d4c3b2a1908UL, x18);
  ASSERT_EQUAL_64(0x02468acf13579bdeUL, x19);

  TEARDOWN();
}


TEST(Assembler, fmov_imm) {
  SETUP();

  START();
  __ Fmov(s11, 1.0);
  __ Fmov(d22, -13.0);
  __ Fmov(s1, 255.0);
  __ Fmov(d2, 12.34567);
  __ Fmov(s3, 0.0);
  __ Fmov(d4, 0.0);
  __ Fmov(s5, kFP32PositiveInfinity);
  __ Fmov(d6, kFP64NegativeInfinity);
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.0, s11);
  ASSERT_EQUAL_FP64(-13.0, d22);
  ASSERT_EQUAL_FP32(255.0, s1);
  ASSERT_EQUAL_FP64(12.34567, d2);
  ASSERT_EQUAL_FP32(0.0, s3);
  ASSERT_EQUAL_FP64(0.0, d4);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s5);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d6);

  TEARDOWN();
}


TEST(Assembler, fmov_reg) {
  SETUP();

  START();
  __ Fmov(s20, 1.0);
  __ Fmov(w10, s20);
  __ Fmov(s30, w10);
  __ Fmov(s5, s20);
  __ Fmov(d1, -13.0);
  __ Fmov(x1, d1);
  __ Fmov(d2, x1);
  __ Fmov(d4, d1);
  __ Fmov(d6, rawbits_to_double(0x0123456789abcdefL));
  __ Fmov(s6, s6);
  END();

  RUN();

  ASSERT_EQUAL_32(float_to_rawbits(1.0), w10);
  ASSERT_EQUAL_FP32(1.0, s30);
  ASSERT_EQUAL_FP32(1.0, s5);
  ASSERT_EQUAL_64(double_to_rawbits(-13.0), x1);
  ASSERT_EQUAL_FP64(-13.0, d2);
  ASSERT_EQUAL_FP64(-13.0, d4);
  ASSERT_EQUAL_FP32(rawbits_to_float(0x89abcdef), s6);

  TEARDOWN();
}


TEST(Assembler, fadd) {
  SETUP();

  START();
  __ Fmov(s13, -0.0);
  __ Fmov(s14, kFP32PositiveInfinity);
  __ Fmov(s15, kFP32NegativeInfinity);
  __ Fmov(s16, 3.25);
  __ Fmov(s17, 1.0);
  __ Fmov(s18, 0);

  __ Fmov(d26, -0.0);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0);
  __ Fmov(d30, -2.0);
  __ Fmov(d31, 2.25);

  __ Fadd(s0, s16, s17);
  __ Fadd(s1, s17, s18);
  __ Fadd(s2, s13, s17);
  __ Fadd(s3, s14, s17);
  __ Fadd(s4, s15, s17);

  __ Fadd(d5, d30, d31);
  __ Fadd(d6, d29, d31);
  __ Fadd(d7, d26, d31);
  __ Fadd(d8, d27, d31);
  __ Fadd(d9, d28, d31);
  END();

  RUN();

  ASSERT_EQUAL_FP32(4.25, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(1.0, s2);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s3);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s4);
  ASSERT_EQUAL_FP64(0.25, d5);
  ASSERT_EQUAL_FP64(2.25, d6);
  ASSERT_EQUAL_FP64(2.25, d7);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d8);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d9);

  TEARDOWN();
}


TEST(Assembler, fsub) {
  SETUP();

  START();
  __ Fmov(s13, -0.0);
  __ Fmov(s14, kFP32PositiveInfinity);
  __ Fmov(s15, kFP32NegativeInfinity);
  __ Fmov(s16, 3.25);
  __ Fmov(s17, 1.0);
  __ Fmov(s18, 0);

  __ Fmov(d26, -0.0);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0);
  __ Fmov(d30, -2.0);
  __ Fmov(d31, 2.25);

  __ Fsub(s0, s16, s17);
  __ Fsub(s1, s17, s18);
  __ Fsub(s2, s13, s17);
  __ Fsub(s3, s17, s14);
  __ Fsub(s4, s17, s15);

  __ Fsub(d5, d30, d31);
  __ Fsub(d6, d29, d31);
  __ Fsub(d7, d26, d31);
  __ Fsub(d8, d31, d27);
  __ Fsub(d9, d31, d28);
  END();

  RUN();

  ASSERT_EQUAL_FP32(2.25, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(-1.0, s2);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s3);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s4);
  ASSERT_EQUAL_FP64(-4.25, d5);
  ASSERT_EQUAL_FP64(-2.25, d6);
  ASSERT_EQUAL_FP64(-2.25, d7);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d8);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d9);

  TEARDOWN();
}


TEST(Assembler, fmul) {
  SETUP();

  START();
  __ Fmov(s13, -0.0);
  __ Fmov(s14, kFP32PositiveInfinity);
  __ Fmov(s15, kFP32NegativeInfinity);
  __ Fmov(s16, 3.25);
  __ Fmov(s17, 2.0);
  __ Fmov(s18, 0);
  __ Fmov(s19, -2.0);

  __ Fmov(d26, -0.0);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0);
  __ Fmov(d30, -2.0);
  __ Fmov(d31, 2.25);

  __ Fmul(s0, s16, s17);
  __ Fmul(s1, s17, s18);
  __ Fmul(s2, s13, s13);
  __ Fmul(s3, s14, s19);
  __ Fmul(s4, s15, s19);

  __ Fmul(d5, d30, d31);
  __ Fmul(d6, d29, d31);
  __ Fmul(d7, d26, d26);
  __ Fmul(d8, d27, d30);
  __ Fmul(d9, d28, d30);
  END();

  RUN();

  ASSERT_EQUAL_FP32(6.5, s0);
  ASSERT_EQUAL_FP32(0.0, s1);
  ASSERT_EQUAL_FP32(0.0, s2);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s3);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s4);
  ASSERT_EQUAL_FP64(-4.5, d5);
  ASSERT_EQUAL_FP64(0.0, d6);
  ASSERT_EQUAL_FP64(0.0, d7);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d8);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d9);

  TEARDOWN();
}


TEST(Assembler, fmsub) {
  SETUP();

  START();
  __ Fmov(s16, 3.25);
  __ Fmov(s17, 2.0);
  __ Fmov(s18, 0);
  __ Fmov(s19, -0.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, -0);

  __ Fmov(d29, 0);
  __ Fmov(d30, -2.0);
  __ Fmov(d31, 2.25);
  __ Fmov(d28, 4);
  __ Fmov(d24, kFP64PositiveInfinity);
  __ Fmov(d25, kFP64NegativeInfinity);
  __ Fmov(d26, -0);

     // Normal combinations
  __ Fmsub(s0, s16, s17, s18);
  __ Fmsub(s1, s17, s18, s16);
  __ Fmsub(s2, s17, s16, s19);
     // Pos/Neg Infinity
  __ Fmsub(s3, s16, s21, s19);
  __ Fmsub(s4, s17, s16, s20);
  __ Fmsub(s5, s20, s16, s19);
  __ Fmsub(s6, s21, s16, s19);
     // -0
  __ Fmsub(s7, s22, s16, s19);
  __ Fmsub(s8, s19, s16, s22);

     // Normal combinations
  __ Fmsub(d9, d30, d31, d29);
  __ Fmsub(d10, d29, d31, d30);
  __ Fmsub(d11, d30, d31, d28);
     // Pos/Neg Infinity
  __ Fmsub(d12, d30, d24, d28);
  __ Fmsub(d13, d24, d31, d25);
  __ Fmsub(d14, d24, d31, d28);
  __ Fmsub(d15, d25, d31, d28);
     // -0
  __ Fmsub(d16, d26, d31, d28);
  __ Fmsub(d17, d30, d26, d28);
  END();

  RUN();

  // Normal combinations
  ASSERT_EQUAL_FP32(-6.5, s0);
  ASSERT_EQUAL_FP32(3.25, s1);
  ASSERT_EQUAL_FP32(-7, s2);
  // Pos/Neg Infinity
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s3);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s4);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s5);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s6);
  // -0
  ASSERT_EQUAL_FP32(-0.5, s7);
  ASSERT_EQUAL_FP32(1.625, s8);

  // Normal combinations
  ASSERT_EQUAL_FP64(4.5, d9);
  ASSERT_EQUAL_FP64(-2.0, d10);
  ASSERT_EQUAL_FP64(8.5, d11);
  // Pos/Neg Infinity
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d12);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d13);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d14);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d15);
  // -0
  ASSERT_EQUAL_FP64(4.0, d16);
  ASSERT_EQUAL_FP64(4.0, d17);

  TEARDOWN();
}


TEST(Assembler, fdiv) {
  SETUP();

  START();
  __ Fmov(s13, -0.0);
  __ Fmov(s14, kFP32PositiveInfinity);
  __ Fmov(s15, kFP32NegativeInfinity);
  __ Fmov(s16, 3.25);
  __ Fmov(s17, 2.0);
  __ Fmov(s18, 2.0);
  __ Fmov(s19, -2.0);

  __ Fmov(d26, -0.0);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0);
  __ Fmov(d30, -2.0);
  __ Fmov(d31, 2.25);

  __ Fdiv(s0, s16, s17);
  __ Fdiv(s1, s17, s18);
  __ Fdiv(s2, s13, s17);
  __ Fdiv(s3, s17, s14);
  __ Fdiv(s4, s17, s15);
  __ Fdiv(d5, d31, d30);
  __ Fdiv(d6, d29, d31);
  __ Fdiv(d7, d26, d31);
  __ Fdiv(d8, d31, d27);
  __ Fdiv(d9, d31, d28);
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.625, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(-0.0, s2);
  ASSERT_EQUAL_FP32(0.0, s3);
  ASSERT_EQUAL_FP32(-0.0, s4);
  ASSERT_EQUAL_FP64(-1.125, d5);
  ASSERT_EQUAL_FP64(0.0, d6);
  ASSERT_EQUAL_FP64(-0.0, d7);
  ASSERT_EQUAL_FP64(0.0, d8);
  ASSERT_EQUAL_FP64(-0.0, d9);

  TEARDOWN();
}


TEST(Assembler, fmin_s) {
  SETUP();

  START();
  __ Fmov(s25, 0.0);
  __ Fneg(s26, s25);
  __ Fmov(s27, kFP32PositiveInfinity);
  __ Fmov(s28, 1.0);
  __ Fmin(s0, s25, s26);
  __ Fmin(s1, s27, s28);
  __ Fmin(s2, s28, s26);
  END();

  RUN();

  ASSERT_EQUAL_FP32(-0.0, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(-0.0, s2);

  TEARDOWN();
}


TEST(Assembler, fmin_d) {
  SETUP();

  START();
  __ Fmov(d25, 0.0);
  __ Fneg(d26, d25);
  __ Fmov(d27, kFP32PositiveInfinity);
  __ Fneg(d28, d27);
  __ Fmov(d29, 1.0);

  for (unsigned j = 0; j < 5; j++) {
    for (unsigned i = 0; i < 5; i++) {
      // Test all combinations, writing results into d0 - d24.
      __ Fmin(FPRegister::DRegFromCode(i + 5*j),
              FPRegister::DRegFromCode(i + 25),
              FPRegister::DRegFromCode(j + 25));
    }
  }
  END();

  RUN();

  // Second register is 0.0.
  ASSERT_EQUAL_FP64(0.0, d0);
  ASSERT_EQUAL_FP64(-0.0, d1);
  ASSERT_EQUAL_FP64(0.0, d2);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d3);
  ASSERT_EQUAL_FP64(0.0, d4);

  // Second register is -0.0.
  ASSERT_EQUAL_FP64(-0.0, d5);
  ASSERT_EQUAL_FP64(-0.0, d6);
  ASSERT_EQUAL_FP64(-0.0, d7);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d8);
  ASSERT_EQUAL_FP64(-0.0, d9);

  // Second register is +Inf.
  ASSERT_EQUAL_FP64(0.0, d10);
  ASSERT_EQUAL_FP64(-0.0, d11);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d12);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d13);
  ASSERT_EQUAL_FP64(1.0, d14);

  // Second register is -Inf.
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d15);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d16);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d17);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d18);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d19);

  // Second register is 1.0.
  ASSERT_EQUAL_FP64(0.0, d20);
  ASSERT_EQUAL_FP64(-0.0, d21);
  ASSERT_EQUAL_FP64(1.0, d22);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d23);
  ASSERT_EQUAL_FP64(1.0, d24);

  TEARDOWN();
}


TEST(Assembler, fmax_s) {
  SETUP();

  START();
  __ Fmov(s25, 0.0);
  __ Fneg(s26, s25);
  __ Fmov(s27, kFP32PositiveInfinity);
  __ Fmov(s28, 1.0);
  __ Fmax(s0, s25, s26);
  __ Fmax(s1, s27, s28);
  __ Fmax(s2, s28, s26);
  END();

  RUN();

  ASSERT_EQUAL_FP32(0.0, s0);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s1);
  ASSERT_EQUAL_FP32(1.0, s2);

  TEARDOWN();
}


TEST(Assembler, fmax_d) {
  SETUP();

  START();
  __ Fmov(d25, 0.0);
  __ Fneg(d26, d25);
  __ Fmov(d27, kFP32PositiveInfinity);
  __ Fneg(d28, d27);
  __ Fmov(d29, 1.0);

  for (unsigned j = 0; j < 5; j++) {
    for (unsigned i = 0; i < 5; i++) {
      // Test all combinations, writing results into d0 - d24.
      __ Fmax(FPRegister::DRegFromCode(i + 5*j),
              FPRegister::DRegFromCode(i + 25),
              FPRegister::DRegFromCode(j + 25));
    }
  }
  END();

  RUN();

  // Second register is 0.0.
  ASSERT_EQUAL_FP64(0.0, d0);
  ASSERT_EQUAL_FP64(0.0, d1);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d2);
  ASSERT_EQUAL_FP64(0.0, d3);
  ASSERT_EQUAL_FP64(1.0, d4);

  // Second register is -0.0.
  ASSERT_EQUAL_FP64(0.0, d5);
  ASSERT_EQUAL_FP64(-0.0, d6);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d7);
  ASSERT_EQUAL_FP64(-0.0, d8);
  ASSERT_EQUAL_FP64(1.0, d9);

  // Second register is +Inf.
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d10);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d11);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d12);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d13);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d14);

  // Second register is -Inf.
  ASSERT_EQUAL_FP64(0.0, d15);
  ASSERT_EQUAL_FP64(-0.0, d16);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d17);
  ASSERT_EQUAL_FP64(kFP32NegativeInfinity, d18);
  ASSERT_EQUAL_FP64(1.0, d19);

  // Second register is 1.0.
  ASSERT_EQUAL_FP64(1.0, d20);
  ASSERT_EQUAL_FP64(1.0, d21);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d22);
  ASSERT_EQUAL_FP64(1.0, d23);
  ASSERT_EQUAL_FP64(1.0, d24);

  TEARDOWN();
}


TEST(Assembler, fccmp) {
  SETUP();

  START();
  __ Fmov(s16, 0.0);
  __ Fmov(s17, 0.5);
  __ Fmov(d18, -0.5);
  __ Fmov(d19, -1.0);
  __ Mov(x20, 0);

  __ Cmp(x20, 0);
  __ Fccmp(s16, s16, NoFlag, eq);
  __ Mrs(x0, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(s16, s16, VFlag, ne);
  __ Mrs(x1, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(s16, s17, CFlag, ge);
  __ Mrs(x2, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(s16, s17, CVFlag, lt);
  __ Mrs(x3, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(d18, d18, ZFlag, le);
  __ Mrs(x4, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(d18, d18, ZVFlag, gt);
  __ Mrs(x5, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(d18, d19, ZCVFlag, ls);
  __ Mrs(x6, NZCV);

  __ Cmp(x20, 0);
  __ Fccmp(d18, d19, NFlag, hi);
  __ Mrs(x7, NZCV);

  __ fccmp(s16, s16, NFlag, al);
  __ Mrs(x8, NZCV);

  __ fccmp(d18, d18, NFlag, nv);
  __ Mrs(x9, NZCV);
  END();

  RUN();

  ASSERT_EQUAL_32(ZCFlag, w0);
  ASSERT_EQUAL_32(VFlag, w1);
  ASSERT_EQUAL_32(NFlag, w2);
  ASSERT_EQUAL_32(CVFlag, w3);
  ASSERT_EQUAL_32(ZCFlag, w4);
  ASSERT_EQUAL_32(ZVFlag, w5);
  ASSERT_EQUAL_32(CFlag, w6);
  ASSERT_EQUAL_32(NFlag, w7);
  ASSERT_EQUAL_32(ZCFlag, w8);
  ASSERT_EQUAL_32(ZCFlag, w9);

  TEARDOWN();
}


TEST(Assembler, fcmp) {
  SETUP();

  START();
  __ Fmov(s8, 0.0);
  __ Fmov(s9, 0.5);
  __ Mov(w18, 0x7f800001);  // Single precision NaN.
  __ Fmov(s18, w18);

  __ Fcmp(s8, s8);
  __ Mrs(x0, NZCV);
  __ Fcmp(s8, s9);
  __ Mrs(x1, NZCV);
  __ Fcmp(s9, s8);
  __ Mrs(x2, NZCV);
  __ Fcmp(s8, s18);
  __ Mrs(x3, NZCV);
  __ Fcmp(s18, s18);
  __ Mrs(x4, NZCV);
  __ Fcmp(s8, 0.0);
  __ Mrs(x5, NZCV);
  __ Fcmp(s8, 255.0);
  __ Mrs(x6, NZCV);

  __ Fmov(d19, 0.0);
  __ Fmov(d20, 0.5);
  __ Mov(x21, 0x7ff0000000000001UL);  // Double precision NaN.
  __ Fmov(d21, x21);

  __ Fcmp(d19, d19);
  __ Mrs(x10, NZCV);
  __ Fcmp(d19, d20);
  __ Mrs(x11, NZCV);
  __ Fcmp(d20, d19);
  __ Mrs(x12, NZCV);
  __ Fcmp(d19, d21);
  __ Mrs(x13, NZCV);
  __ Fcmp(d21, d21);
  __ Mrs(x14, NZCV);
  __ Fcmp(d19, 0.0);
  __ Mrs(x15, NZCV);
  __ Fcmp(d19, 12.3456);
  __ Mrs(x16, NZCV);
  END();

  RUN();

  ASSERT_EQUAL_32(ZCFlag, w0);
  ASSERT_EQUAL_32(NFlag, w1);
  ASSERT_EQUAL_32(CFlag, w2);
  ASSERT_EQUAL_32(CVFlag, w3);
  ASSERT_EQUAL_32(CVFlag, w4);
  ASSERT_EQUAL_32(ZCFlag, w5);
  ASSERT_EQUAL_32(NFlag, w6);
  ASSERT_EQUAL_32(ZCFlag, w10);
  ASSERT_EQUAL_32(NFlag, w11);
  ASSERT_EQUAL_32(CFlag, w12);
  ASSERT_EQUAL_32(CVFlag, w13);
  ASSERT_EQUAL_32(CVFlag, w14);
  ASSERT_EQUAL_32(ZCFlag, w15);
  ASSERT_EQUAL_32(NFlag, w16);

  TEARDOWN();
}


TEST(Assembler, fcsel) {
  SETUP();

  START();
  __ Mov(x16, 0);
  __ Fmov(s16, 1.0);
  __ Fmov(s17, 2.0);
  __ Fmov(d18, 3.0);
  __ Fmov(d19, 4.0);

  __ Cmp(x16, 0);
  __ Fcsel(s0, s16, s17, eq);
  __ Fcsel(s1, s16, s17, ne);
  __ Fcsel(d2, d18, d19, eq);
  __ Fcsel(d3, d18, d19, ne);
  __ fcsel(s4, s16, s17, al);
  __ fcsel(d5, d18, d19, nv);
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.0, s0);
  ASSERT_EQUAL_FP32(2.0, s1);
  ASSERT_EQUAL_FP64(3.0, d2);
  ASSERT_EQUAL_FP64(4.0, d3);
  ASSERT_EQUAL_FP32(1.0, s4);
  ASSERT_EQUAL_FP64(3.0, d5);

  TEARDOWN();
}


TEST(Assembler, fneg) {
  SETUP();

  START();
  __ Fmov(s16, 1.0);
  __ Fmov(s17, 0.0);
  __ Fmov(s18, kFP32PositiveInfinity);
  __ Fmov(d19, 1.0);
  __ Fmov(d20, 0.0);
  __ Fmov(d21, kFP64PositiveInfinity);

  __ Fneg(s0, s16);
  __ Fneg(s1, s0);
  __ Fneg(s2, s17);
  __ Fneg(s3, s2);
  __ Fneg(s4, s18);
  __ Fneg(s5, s4);
  __ Fneg(d6, d19);
  __ Fneg(d7, d6);
  __ Fneg(d8, d20);
  __ Fneg(d9, d8);
  __ Fneg(d10, d21);
  __ Fneg(d11, d10);
  END();

  RUN();

  ASSERT_EQUAL_FP32(-1.0, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(-0.0, s2);
  ASSERT_EQUAL_FP32(0.0, s3);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s4);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s5);
  ASSERT_EQUAL_FP64(-1.0, d6);
  ASSERT_EQUAL_FP64(1.0, d7);
  ASSERT_EQUAL_FP64(-0.0, d8);
  ASSERT_EQUAL_FP64(0.0, d9);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d10);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d11);

  TEARDOWN();
}


TEST(Assembler, fabs) {
  SETUP();

  START();
  __ Fmov(s16, -1.0);
  __ Fmov(s17, -0.0);
  __ Fmov(s18, kFP32NegativeInfinity);
  __ Fmov(d19, -1.0);
  __ Fmov(d20, -0.0);
  __ Fmov(d21, kFP64NegativeInfinity);

  __ Fabs(s0, s16);
  __ Fabs(s1, s0);
  __ Fabs(s2, s17);
  __ Fabs(s3, s18);
  __ Fabs(d4, d19);
  __ Fabs(d5, d4);
  __ Fabs(d6, d20);
  __ Fabs(d7, d21);
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.0, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(0.0, s2);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s3);
  ASSERT_EQUAL_FP64(1.0, d4);
  ASSERT_EQUAL_FP64(1.0, d5);
  ASSERT_EQUAL_FP64(0.0, d6);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d7);

  TEARDOWN();
}


TEST(Assembler, fsqrt) {
  SETUP();

  START();
  __ Fmov(s16, 0.0);
  __ Fmov(s17, 1.0);
  __ Fmov(s18, 0.25);
  __ Fmov(s19, 65536.0);
  __ Fmov(s20, -0.0);
  __ Fmov(s21, kFP32PositiveInfinity);
  __ Fmov(d22, 0.0);
  __ Fmov(d23, 1.0);
  __ Fmov(d24, 0.25);
  __ Fmov(d25, 4294967296.0);
  __ Fmov(d26, -0.0);
  __ Fmov(d27, kFP64PositiveInfinity);

  __ Fsqrt(s0, s16);
  __ Fsqrt(s1, s17);
  __ Fsqrt(s2, s18);
  __ Fsqrt(s3, s19);
  __ Fsqrt(s4, s20);
  __ Fsqrt(s5, s21);
  __ Fsqrt(d6, d22);
  __ Fsqrt(d7, d23);
  __ Fsqrt(d8, d24);
  __ Fsqrt(d9, d25);
  __ Fsqrt(d10, d26);
  __ Fsqrt(d11, d27);
  END();

  RUN();

  ASSERT_EQUAL_FP32(0.0, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(0.5, s2);
  ASSERT_EQUAL_FP32(256.0, s3);
  ASSERT_EQUAL_FP32(-0.0, s4);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s5);
  ASSERT_EQUAL_FP64(0.0, d6);
  ASSERT_EQUAL_FP64(1.0, d7);
  ASSERT_EQUAL_FP64(0.5, d8);
  ASSERT_EQUAL_FP64(65536.0, d9);
  ASSERT_EQUAL_FP64(-0.0, d10);
  ASSERT_EQUAL_FP64(kFP32PositiveInfinity, d11);

  TEARDOWN();
}


TEST(Assembler, frintn) {
  SETUP();

  START();
  __ Fmov(s16, 1.0);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, 1.9);
  __ Fmov(s20, 2.5);
  __ Fmov(s21, -1.5);
  __ Fmov(s22, -2.5);
  __ Fmov(s23, kFP32PositiveInfinity);
  __ Fmov(s24, kFP32NegativeInfinity);
  __ Fmov(s25, 0.0);
  __ Fmov(s26, -0.0);

  __ Frintn(s0, s16);
  __ Frintn(s1, s17);
  __ Frintn(s2, s18);
  __ Frintn(s3, s19);
  __ Frintn(s4, s20);
  __ Frintn(s5, s21);
  __ Frintn(s6, s22);
  __ Frintn(s7, s23);
  __ Frintn(s8, s24);
  __ Frintn(s9, s25);
  __ Frintn(s10, s26);

  __ Fmov(d16, 1.0);
  __ Fmov(d17, 1.1);
  __ Fmov(d18, 1.5);
  __ Fmov(d19, 1.9);
  __ Fmov(d20, 2.5);
  __ Fmov(d21, -1.5);
  __ Fmov(d22, -2.5);
  __ Fmov(d23, kFP32PositiveInfinity);
  __ Fmov(d24, kFP32NegativeInfinity);
  __ Fmov(d25, 0.0);
  __ Fmov(d26, -0.0);

  __ Frintn(d11, d16);
  __ Frintn(d12, d17);
  __ Frintn(d13, d18);
  __ Frintn(d14, d19);
  __ Frintn(d15, d20);
  __ Frintn(d16, d21);
  __ Frintn(d17, d22);
  __ Frintn(d18, d23);
  __ Frintn(d19, d24);
  __ Frintn(d20, d25);
  __ Frintn(d21, d26);
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.0, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(2.0, s2);
  ASSERT_EQUAL_FP32(2.0, s3);
  ASSERT_EQUAL_FP32(2.0, s4);
  ASSERT_EQUAL_FP32(-2.0, s5);
  ASSERT_EQUAL_FP32(-2.0, s6);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s7);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s8);
  ASSERT_EQUAL_FP32(0.0, s9);
  ASSERT_EQUAL_FP32(-0.0, s10);
  ASSERT_EQUAL_FP64(1.0, d11);
  ASSERT_EQUAL_FP64(1.0, d12);
  ASSERT_EQUAL_FP64(2.0, d13);
  ASSERT_EQUAL_FP64(2.0, d14);
  ASSERT_EQUAL_FP64(2.0, d15);
  ASSERT_EQUAL_FP64(-2.0, d16);
  ASSERT_EQUAL_FP64(-2.0, d17);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d18);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d19);
  ASSERT_EQUAL_FP64(0.0, d20);
  ASSERT_EQUAL_FP64(-0.0, d21);

  TEARDOWN();
}


TEST(Assembler, frintz) {
  SETUP();

  START();
  __ Fmov(s16, 1.0);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, 1.9);
  __ Fmov(s20, 2.5);
  __ Fmov(s21, -1.5);
  __ Fmov(s22, -2.5);
  __ Fmov(s23, kFP32PositiveInfinity);
  __ Fmov(s24, kFP32NegativeInfinity);
  __ Fmov(s25, 0.0);
  __ Fmov(s26, -0.0);

  __ Frintz(s0, s16);
  __ Frintz(s1, s17);
  __ Frintz(s2, s18);
  __ Frintz(s3, s19);
  __ Frintz(s4, s20);
  __ Frintz(s5, s21);
  __ Frintz(s6, s22);
  __ Frintz(s7, s23);
  __ Frintz(s8, s24);
  __ Frintz(s9, s25);
  __ Frintz(s10, s26);

  __ Fmov(d16, 1.0);
  __ Fmov(d17, 1.1);
  __ Fmov(d18, 1.5);
  __ Fmov(d19, 1.9);
  __ Fmov(d20, 2.5);
  __ Fmov(d21, -1.5);
  __ Fmov(d22, -2.5);
  __ Fmov(d23, kFP32PositiveInfinity);
  __ Fmov(d24, kFP32NegativeInfinity);
  __ Fmov(d25, 0.0);
  __ Fmov(d26, -0.0);

  __ Frintz(d11, d16);
  __ Frintz(d12, d17);
  __ Frintz(d13, d18);
  __ Frintz(d14, d19);
  __ Frintz(d15, d20);
  __ Frintz(d16, d21);
  __ Frintz(d17, d22);
  __ Frintz(d18, d23);
  __ Frintz(d19, d24);
  __ Frintz(d20, d25);
  __ Frintz(d21, d26);
  END();

  RUN();

  ASSERT_EQUAL_FP32(1.0, s0);
  ASSERT_EQUAL_FP32(1.0, s1);
  ASSERT_EQUAL_FP32(1.0, s2);
  ASSERT_EQUAL_FP32(1.0, s3);
  ASSERT_EQUAL_FP32(2.0, s4);
  ASSERT_EQUAL_FP32(-1.0, s5);
  ASSERT_EQUAL_FP32(-2.0, s6);
  ASSERT_EQUAL_FP32(kFP32PositiveInfinity, s7);
  ASSERT_EQUAL_FP32(kFP32NegativeInfinity, s8);
  ASSERT_EQUAL_FP32(0.0, s9);
  ASSERT_EQUAL_FP32(-0.0, s10);
  ASSERT_EQUAL_FP64(1.0, d11);
  ASSERT_EQUAL_FP64(1.0, d12);
  ASSERT_EQUAL_FP64(1.0, d13);
  ASSERT_EQUAL_FP64(1.0, d14);
  ASSERT_EQUAL_FP64(2.0, d15);
  ASSERT_EQUAL_FP64(-1.0, d16);
  ASSERT_EQUAL_FP64(-2.0, d17);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d18);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d19);
  ASSERT_EQUAL_FP64(0.0, d20);
  ASSERT_EQUAL_FP64(-0.0, d21);

  TEARDOWN();
}


TEST(Assembler, fcvt_ds) {
  SETUP();

  START();
  __ Fmov(s16, 1.0);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, 1.9);
  __ Fmov(s20, 2.5);
  __ Fmov(s21, -1.5);
  __ Fmov(s22, -2.5);
  __ Fmov(s23, kFP32PositiveInfinity);
  __ Fmov(s24, kFP32NegativeInfinity);
  __ Fmov(s25, 0.0);
  __ Fmov(s26, -0.0);
  __ Fmov(s27, FLT_MAX);
  __ Fmov(s28, FLT_MIN);
  __ Fmov(s29, rawbits_to_float(0x7fc12345));   // Quiet NaN.
  __ Fmov(s30, rawbits_to_float(0x7f812345));   // Signalling NaN.

  __ Fcvt(d0, s16);
  __ Fcvt(d1, s17);
  __ Fcvt(d2, s18);
  __ Fcvt(d3, s19);
  __ Fcvt(d4, s20);
  __ Fcvt(d5, s21);
  __ Fcvt(d6, s22);
  __ Fcvt(d7, s23);
  __ Fcvt(d8, s24);
  __ Fcvt(d9, s25);
  __ Fcvt(d10, s26);
  __ Fcvt(d11, s27);
  __ Fcvt(d12, s28);
  __ Fcvt(d13, s29);
  __ Fcvt(d14, s30);
  END();

  RUN();

  ASSERT_EQUAL_FP64(1.0f, d0);
  ASSERT_EQUAL_FP64(1.1f, d1);
  ASSERT_EQUAL_FP64(1.5f, d2);
  ASSERT_EQUAL_FP64(1.9f, d3);
  ASSERT_EQUAL_FP64(2.5f, d4);
  ASSERT_EQUAL_FP64(-1.5f, d5);
  ASSERT_EQUAL_FP64(-2.5f, d6);
  ASSERT_EQUAL_FP64(kFP64PositiveInfinity, d7);
  ASSERT_EQUAL_FP64(kFP64NegativeInfinity, d8);
  ASSERT_EQUAL_FP64(0.0f, d9);
  ASSERT_EQUAL_FP64(-0.0f, d10);
  ASSERT_EQUAL_FP64(FLT_MAX, d11);
  ASSERT_EQUAL_FP64(FLT_MIN, d12);

  // Check that the NaN payload is preserved according to A64 conversion rules:
  //  - The sign bit is preserved.
  //  - The top bit of the mantissa is forced to 1 (making it a quiet NaN).
  //  - The remaining mantissa bits are copied until they run out.
  //  - The low-order bits that haven't already been assigned are set to 0.
  ASSERT_EQUAL_FP64(rawbits_to_double(0x7ff82468a0000000), d13);
  ASSERT_EQUAL_FP64(rawbits_to_double(0x7ff82468a0000000), d14);

  TEARDOWN();
}


TEST(Assembler, fcvt_sd) {
  // There are a huge number of corner-cases to check, so this test iterates
  // through a list. The list is then negated and checked again (since the sign
  // is irrelevant in ties-to-even rounding), so the list shouldn't include any
  // negative values.
  //
  // Note that this test only checks ties-to-even rounding, because that is all
  // that the simulator supports.
  struct {double in; float expected;} test[] = {
    // Check some simple conversions.
    {0.0, 0.0f},
    {1.0, 1.0f},
    {1.5, 1.5f},
    {2.0, 2.0f},
    {FLT_MAX, FLT_MAX},
    //  - The smallest normalized float.
    {pow(2, -126), pow(2, -126)},
    //  - Normal floats that need (ties-to-even) rounding.
    //    For normalized numbers:
    //         bit 29 (0x0000000020000000) is the lowest-order bit which will
    //                                     fit in the float's mantissa.
    {rawbits_to_double(0x3ff0000000000000), rawbits_to_float(0x3f800000)},
    {rawbits_to_double(0x3ff0000000000001), rawbits_to_float(0x3f800000)},
    {rawbits_to_double(0x3ff0000010000000), rawbits_to_float(0x3f800000)},
    {rawbits_to_double(0x3ff0000010000001), rawbits_to_float(0x3f800001)},
    {rawbits_to_double(0x3ff0000020000000), rawbits_to_float(0x3f800001)},
    {rawbits_to_double(0x3ff0000020000001), rawbits_to_float(0x3f800001)},
    {rawbits_to_double(0x3ff0000030000000), rawbits_to_float(0x3f800002)},
    {rawbits_to_double(0x3ff0000030000001), rawbits_to_float(0x3f800002)},
    {rawbits_to_double(0x3ff0000040000000), rawbits_to_float(0x3f800002)},
    {rawbits_to_double(0x3ff0000040000001), rawbits_to_float(0x3f800002)},
    {rawbits_to_double(0x3ff0000050000000), rawbits_to_float(0x3f800002)},
    {rawbits_to_double(0x3ff0000050000001), rawbits_to_float(0x3f800003)},
    {rawbits_to_double(0x3ff0000060000000), rawbits_to_float(0x3f800003)},
    //  - A mantissa that overflows into the exponent during rounding.
    {rawbits_to_double(0x3feffffff0000000), rawbits_to_float(0x3f800000)},
    //  - The largest double that rounds to a normal float.
    {rawbits_to_double(0x47efffffefffffff), rawbits_to_float(0x7f7fffff)},

    // Doubles that are too big for a float.
    {kFP64PositiveInfinity, kFP32PositiveInfinity},
    {DBL_MAX, kFP32PositiveInfinity},
    //  - The smallest exponent that's too big for a float.
    {pow(2, 128), kFP32PositiveInfinity},
    //  - This exponent is in range, but the value rounds to infinity.
    {rawbits_to_double(0x47effffff0000000), kFP32PositiveInfinity},

    // Doubles that are too small for a float.
    //  - The smallest (subnormal) double.
    {DBL_MIN, 0.0},
    //  - The largest double which is too small for a subnormal float.
    {rawbits_to_double(0x3690000000000000), rawbits_to_float(0x00000000)},

    // Normal doubles that become subnormal floats.
    //  - The largest subnormal float.
    {rawbits_to_double(0x380fffffc0000000), rawbits_to_float(0x007fffff)},
    //  - The smallest subnormal float.
    {rawbits_to_double(0x36a0000000000000), rawbits_to_float(0x00000001)},
    //  - Subnormal floats that need (ties-to-even) rounding.
    //    For these subnormals:
    //         bit 34 (0x0000000400000000) is the lowest-order bit which will
    //                                     fit in the float's mantissa.
    {rawbits_to_double(0x37c159e000000000), rawbits_to_float(0x00045678)},
    {rawbits_to_double(0x37c159e000000001), rawbits_to_float(0x00045678)},
    {rawbits_to_double(0x37c159e200000000), rawbits_to_float(0x00045678)},
    {rawbits_to_double(0x37c159e200000001), rawbits_to_float(0x00045679)},
    {rawbits_to_double(0x37c159e400000000), rawbits_to_float(0x00045679)},
    {rawbits_to_double(0x37c159e400000001), rawbits_to_float(0x00045679)},
    {rawbits_to_double(0x37c159e600000000), rawbits_to_float(0x0004567a)},
    {rawbits_to_double(0x37c159e600000001), rawbits_to_float(0x0004567a)},
    {rawbits_to_double(0x37c159e800000000), rawbits_to_float(0x0004567a)},
    {rawbits_to_double(0x37c159e800000001), rawbits_to_float(0x0004567a)},
    {rawbits_to_double(0x37c159ea00000000), rawbits_to_float(0x0004567a)},
    {rawbits_to_double(0x37c159ea00000001), rawbits_to_float(0x0004567b)},
    {rawbits_to_double(0x37c159ec00000000), rawbits_to_float(0x0004567b)},
    //  - The smallest double which rounds up to become a subnormal float.
    {rawbits_to_double(0x3690000000000001), rawbits_to_float(0x00000001)},

    // Check NaN payload preservation.
    {rawbits_to_double(0x7ff82468a0000000), rawbits_to_float(0x7fc12345)},
    {rawbits_to_double(0x7ff82468bfffffff), rawbits_to_float(0x7fc12345)},
    //  - Signalling NaNs become quiet NaNs.
    {rawbits_to_double(0x7ff02468a0000000), rawbits_to_float(0x7fc12345)},
    {rawbits_to_double(0x7ff02468bfffffff), rawbits_to_float(0x7fc12345)},
    {rawbits_to_double(0x7ff000001fffffff), rawbits_to_float(0x7fc00000)},
  };
  int count = sizeof(test) / sizeof(test[0]);

  for (int i = 0; i < count; i++) {
    double in = test[i].in;
    float expected = test[i].expected;

    // We only expect positive input.
    assert(std::signbit(in) == 0);
    assert(std::signbit(expected) == 0);

    SETUP();
    START();

    __ Fmov(d10, in);
    __ Fcvt(s20, d10);

    __ Fmov(d11, -in);
    __ Fcvt(s21, d11);

    END();
    RUN();
    ASSERT_EQUAL_FP32(expected, s20);
    ASSERT_EQUAL_FP32(-expected, s21);
    TEARDOWN();
  }
}


TEST(Assembler, fcvtms) {
  SETUP();

  START();
  __ Fmov(s0, 1.0);
  __ Fmov(s1, 1.1);
  __ Fmov(s2, 1.5);
  __ Fmov(s3, -1.5);
  __ Fmov(s4, kFP32PositiveInfinity);
  __ Fmov(s5, kFP32NegativeInfinity);
  __ Fmov(s6, 0x7fffff80);  // Largest float < INT32_MAX.
  __ Fneg(s7, s6);          // Smallest float > INT32_MIN.
  __ Fmov(d8, 1.0);
  __ Fmov(d9, 1.1);
  __ Fmov(d10, 1.5);
  __ Fmov(d11, -1.5);
  __ Fmov(d12, kFP64PositiveInfinity);
  __ Fmov(d13, kFP64NegativeInfinity);
  __ Fmov(d14, kWMaxInt - 1);
  __ Fmov(d15, kWMinInt + 1);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, -1.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, 0x7fffff8000000000UL);  // Largest float < INT64_MAX.
  __ Fneg(s23, s22);                    // Smallest float > INT64_MIN.
  __ Fmov(d24, 1.1);
  __ Fmov(d25, 1.5);
  __ Fmov(d26, -1.5);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0x7ffffffffffffc00UL);  // Largest double < INT64_MAX.
  __ Fneg(d30, d29);                    // Smallest double > INT64_MIN.

  __ Fcvtms(w0, s0);
  __ Fcvtms(w1, s1);
  __ Fcvtms(w2, s2);
  __ Fcvtms(w3, s3);
  __ Fcvtms(w4, s4);
  __ Fcvtms(w5, s5);
  __ Fcvtms(w6, s6);
  __ Fcvtms(w7, s7);
  __ Fcvtms(w8, d8);
  __ Fcvtms(w9, d9);
  __ Fcvtms(w10, d10);
  __ Fcvtms(w11, d11);
  __ Fcvtms(w12, d12);
  __ Fcvtms(w13, d13);
  __ Fcvtms(w14, d14);
  __ Fcvtms(w15, d15);
  __ Fcvtms(x17, s17);
  __ Fcvtms(x18, s18);
  __ Fcvtms(x19, s19);
  __ Fcvtms(x20, s20);
  __ Fcvtms(x21, s21);
  __ Fcvtms(x22, s22);
  __ Fcvtms(x23, s23);
  __ Fcvtms(x24, d24);
  __ Fcvtms(x25, d25);
  __ Fcvtms(x26, d26);
  __ Fcvtms(x27, d27);
  __ Fcvtms(x28, d28);
  __ Fcvtms(x29, d29);
  __ Fcvtms(x30, d30);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(1, x1);
  ASSERT_EQUAL_64(1, x2);
  ASSERT_EQUAL_64(0xfffffffe, x3);
  ASSERT_EQUAL_64(0x7fffffff, x4);
  ASSERT_EQUAL_64(0x80000000, x5);
  ASSERT_EQUAL_64(0x7fffff80, x6);
  ASSERT_EQUAL_64(0x80000080, x7);
  ASSERT_EQUAL_64(1, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(1, x10);
  ASSERT_EQUAL_64(0xfffffffe, x11);
  ASSERT_EQUAL_64(0x7fffffff, x12);
  ASSERT_EQUAL_64(0x80000000, x13);
  ASSERT_EQUAL_64(0x7ffffffe, x14);
  ASSERT_EQUAL_64(0x80000001, x15);
  ASSERT_EQUAL_64(1, x17);
  ASSERT_EQUAL_64(1, x18);
  ASSERT_EQUAL_64(0xfffffffffffffffeUL, x19);
  ASSERT_EQUAL_64(0x7fffffffffffffffUL, x20);
  ASSERT_EQUAL_64(0x8000000000000000UL, x21);
  ASSERT_EQUAL_64(0x7fffff8000000000UL, x22);
  ASSERT_EQUAL_64(0x8000008000000000UL, x23);
  ASSERT_EQUAL_64(1, x24);
  ASSERT_EQUAL_64(1, x25);
  ASSERT_EQUAL_64(0xfffffffffffffffeUL, x26);
  ASSERT_EQUAL_64(0x7fffffffffffffffUL, x27);
  ASSERT_EQUAL_64(0x8000000000000000UL, x28);
  ASSERT_EQUAL_64(0x7ffffffffffffc00UL, x29);
  ASSERT_EQUAL_64(0x8000000000000400UL, x30);

  TEARDOWN();
}


TEST(Assembler, fcvtmu) {
  SETUP();

  START();
  __ Fmov(s0, 1.0);
  __ Fmov(s1, 1.1);
  __ Fmov(s2, 1.5);
  __ Fmov(s3, -1.5);
  __ Fmov(s4, kFP32PositiveInfinity);
  __ Fmov(s5, kFP32NegativeInfinity);
  __ Fmov(s6, 0x7fffff80);  // Largest float < INT32_MAX.
  __ Fneg(s7, s6);          // Smallest float > INT32_MIN.
  __ Fmov(d8, 1.0);
  __ Fmov(d9, 1.1);
  __ Fmov(d10, 1.5);
  __ Fmov(d11, -1.5);
  __ Fmov(d12, kFP64PositiveInfinity);
  __ Fmov(d13, kFP64NegativeInfinity);
  __ Fmov(d14, kWMaxInt - 1);
  __ Fmov(d15, kWMinInt + 1);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, -1.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, 0x7fffff8000000000UL);  // Largest float < INT64_MAX.
  __ Fneg(s23, s22);                    // Smallest float > INT64_MIN.
  __ Fmov(d24, 1.1);
  __ Fmov(d25, 1.5);
  __ Fmov(d26, -1.5);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0x7ffffffffffffc00UL);  // Largest double < INT64_MAX.
  __ Fneg(d30, d29);                    // Smallest double > INT64_MIN.

  __ Fcvtmu(w0, s0);
  __ Fcvtmu(w1, s1);
  __ Fcvtmu(w2, s2);
  __ Fcvtmu(w3, s3);
  __ Fcvtmu(w4, s4);
  __ Fcvtmu(w5, s5);
  __ Fcvtmu(w6, s6);
  __ Fcvtmu(w7, s7);
  __ Fcvtmu(w8, d8);
  __ Fcvtmu(w9, d9);
  __ Fcvtmu(w10, d10);
  __ Fcvtmu(w11, d11);
  __ Fcvtmu(w12, d12);
  __ Fcvtmu(w13, d13);
  __ Fcvtmu(w14, d14);
  __ Fcvtmu(x17, s17);
  __ Fcvtmu(x18, s18);
  __ Fcvtmu(x19, s19);
  __ Fcvtmu(x20, s20);
  __ Fcvtmu(x21, s21);
  __ Fcvtmu(x22, s22);
  __ Fcvtmu(x23, s23);
  __ Fcvtmu(x24, d24);
  __ Fcvtmu(x25, d25);
  __ Fcvtmu(x26, d26);
  __ Fcvtmu(x27, d27);
  __ Fcvtmu(x28, d28);
  __ Fcvtmu(x29, d29);
  __ Fcvtmu(x30, d30);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(1, x1);
  ASSERT_EQUAL_64(1, x2);
  ASSERT_EQUAL_64(0, x3);
  ASSERT_EQUAL_64(0xffffffff, x4);
  ASSERT_EQUAL_64(0, x5);
  ASSERT_EQUAL_64(0x7fffff80, x6);
  ASSERT_EQUAL_64(0, x7);
  ASSERT_EQUAL_64(1, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(1, x10);
  ASSERT_EQUAL_64(0, x11);
  ASSERT_EQUAL_64(0xffffffff, x12);
  ASSERT_EQUAL_64(0, x13);
  ASSERT_EQUAL_64(0x7ffffffe, x14);
  ASSERT_EQUAL_64(1, x17);
  ASSERT_EQUAL_64(1, x18);
  ASSERT_EQUAL_64(0x0UL, x19);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x20);
  ASSERT_EQUAL_64(0x0UL, x21);
  ASSERT_EQUAL_64(0x7fffff8000000000UL, x22);
  ASSERT_EQUAL_64(0x0UL, x23);
  ASSERT_EQUAL_64(1, x24);
  ASSERT_EQUAL_64(1, x25);
  ASSERT_EQUAL_64(0x0UL, x26);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x27);
  ASSERT_EQUAL_64(0x0UL, x28);
  ASSERT_EQUAL_64(0x7ffffffffffffc00UL, x29);
  ASSERT_EQUAL_64(0x0UL, x30);

  TEARDOWN();
}


TEST(Assembler, fcvtns) {
  SETUP();

  START();
  __ Fmov(s0, 1.0);
  __ Fmov(s1, 1.1);
  __ Fmov(s2, 1.5);
  __ Fmov(s3, -1.5);
  __ Fmov(s4, kFP32PositiveInfinity);
  __ Fmov(s5, kFP32NegativeInfinity);
  __ Fmov(s6, 0x7fffff80);  // Largest float < INT32_MAX.
  __ Fneg(s7, s6);          // Smallest float > INT32_MIN.
  __ Fmov(d8, 1.0);
  __ Fmov(d9, 1.1);
  __ Fmov(d10, 1.5);
  __ Fmov(d11, -1.5);
  __ Fmov(d12, kFP64PositiveInfinity);
  __ Fmov(d13, kFP64NegativeInfinity);
  __ Fmov(d14, kWMaxInt - 1);
  __ Fmov(d15, kWMinInt + 1);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, -1.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, 0x7fffff8000000000UL);   // Largest float < INT64_MAX.
  __ Fneg(s23, s22);                    // Smallest float > INT64_MIN.
  __ Fmov(d24, 1.1);
  __ Fmov(d25, 1.5);
  __ Fmov(d26, -1.5);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0x7ffffffffffffc00UL);   // Largest double < INT64_MAX.
  __ Fneg(d30, d29);                    // Smallest double > INT64_MIN.

  __ Fcvtns(w0, s0);
  __ Fcvtns(w1, s1);
  __ Fcvtns(w2, s2);
  __ Fcvtns(w3, s3);
  __ Fcvtns(w4, s4);
  __ Fcvtns(w5, s5);
  __ Fcvtns(w6, s6);
  __ Fcvtns(w7, s7);
  __ Fcvtns(w8, d8);
  __ Fcvtns(w9, d9);
  __ Fcvtns(w10, d10);
  __ Fcvtns(w11, d11);
  __ Fcvtns(w12, d12);
  __ Fcvtns(w13, d13);
  __ Fcvtns(w14, d14);
  __ Fcvtns(w15, d15);
  __ Fcvtns(x17, s17);
  __ Fcvtns(x18, s18);
  __ Fcvtns(x19, s19);
  __ Fcvtns(x20, s20);
  __ Fcvtns(x21, s21);
  __ Fcvtns(x22, s22);
  __ Fcvtns(x23, s23);
  __ Fcvtns(x24, d24);
  __ Fcvtns(x25, d25);
  __ Fcvtns(x26, d26);
  __ Fcvtns(x27, d27);
  __ Fcvtns(x28, d28);
  __ Fcvtns(x29, d29);
  __ Fcvtns(x30, d30);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(1, x1);
  ASSERT_EQUAL_64(2, x2);
  ASSERT_EQUAL_64(0xfffffffe, x3);
  ASSERT_EQUAL_64(0x7fffffff, x4);
  ASSERT_EQUAL_64(0x80000000, x5);
  ASSERT_EQUAL_64(0x7fffff80, x6);
  ASSERT_EQUAL_64(0x80000080, x7);
  ASSERT_EQUAL_64(1, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(2, x10);
  ASSERT_EQUAL_64(0xfffffffe, x11);
  ASSERT_EQUAL_64(0x7fffffff, x12);
  ASSERT_EQUAL_64(0x80000000, x13);
  ASSERT_EQUAL_64(0x7ffffffe, x14);
  ASSERT_EQUAL_64(0x80000001, x15);
  ASSERT_EQUAL_64(1, x17);
  ASSERT_EQUAL_64(2, x18);
  ASSERT_EQUAL_64(0xfffffffffffffffeUL, x19);
  ASSERT_EQUAL_64(0x7fffffffffffffffUL, x20);
  ASSERT_EQUAL_64(0x8000000000000000UL, x21);
  ASSERT_EQUAL_64(0x7fffff8000000000UL, x22);
  ASSERT_EQUAL_64(0x8000008000000000UL, x23);
  ASSERT_EQUAL_64(1, x24);
  ASSERT_EQUAL_64(2, x25);
  ASSERT_EQUAL_64(0xfffffffffffffffeUL, x26);
  ASSERT_EQUAL_64(0x7fffffffffffffffUL, x27);
  ASSERT_EQUAL_64(0x8000000000000000UL, x28);
  ASSERT_EQUAL_64(0x7ffffffffffffc00UL, x29);
  ASSERT_EQUAL_64(0x8000000000000400UL, x30);

  TEARDOWN();
}


TEST(Assembler, fcvtnu) {
  SETUP();

  START();
  __ Fmov(s0, 1.0);
  __ Fmov(s1, 1.1);
  __ Fmov(s2, 1.5);
  __ Fmov(s3, -1.5);
  __ Fmov(s4, kFP32PositiveInfinity);
  __ Fmov(s5, kFP32NegativeInfinity);
  __ Fmov(s6, 0xffffff00);  // Largest float < UINT32_MAX.
  __ Fmov(d8, 1.0);
  __ Fmov(d9, 1.1);
  __ Fmov(d10, 1.5);
  __ Fmov(d11, -1.5);
  __ Fmov(d12, kFP64PositiveInfinity);
  __ Fmov(d13, kFP64NegativeInfinity);
  __ Fmov(d14, 0xfffffffe);
  __ Fmov(s16, 1.0);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, -1.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, 0xffffff0000000000UL);  // Largest float < UINT64_MAX.
  __ Fmov(d24, 1.1);
  __ Fmov(d25, 1.5);
  __ Fmov(d26, -1.5);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0xfffffffffffff800UL);  // Largest double < UINT64_MAX.
  __ Fmov(s30, 0x100000000UL);

  __ Fcvtnu(w0, s0);
  __ Fcvtnu(w1, s1);
  __ Fcvtnu(w2, s2);
  __ Fcvtnu(w3, s3);
  __ Fcvtnu(w4, s4);
  __ Fcvtnu(w5, s5);
  __ Fcvtnu(w6, s6);
  __ Fcvtnu(w8, d8);
  __ Fcvtnu(w9, d9);
  __ Fcvtnu(w10, d10);
  __ Fcvtnu(w11, d11);
  __ Fcvtnu(w12, d12);
  __ Fcvtnu(w13, d13);
  __ Fcvtnu(w14, d14);
  __ Fcvtnu(w15, d15);
  __ Fcvtnu(x16, s16);
  __ Fcvtnu(x17, s17);
  __ Fcvtnu(x18, s18);
  __ Fcvtnu(x19, s19);
  __ Fcvtnu(x20, s20);
  __ Fcvtnu(x21, s21);
  __ Fcvtnu(x22, s22);
  __ Fcvtnu(x24, d24);
  __ Fcvtnu(x25, d25);
  __ Fcvtnu(x26, d26);
  __ Fcvtnu(x27, d27);
  __ Fcvtnu(x28, d28);
  __ Fcvtnu(x29, d29);
  __ Fcvtnu(w30, s30);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(1, x1);
  ASSERT_EQUAL_64(2, x2);
  ASSERT_EQUAL_64(0, x3);
  ASSERT_EQUAL_64(0xffffffff, x4);
  ASSERT_EQUAL_64(0, x5);
  ASSERT_EQUAL_64(0xffffff00, x6);
  ASSERT_EQUAL_64(1, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(2, x10);
  ASSERT_EQUAL_64(0, x11);
  ASSERT_EQUAL_64(0xffffffff, x12);
  ASSERT_EQUAL_64(0, x13);
  ASSERT_EQUAL_64(0xfffffffe, x14);
  ASSERT_EQUAL_64(1, x16);
  ASSERT_EQUAL_64(1, x17);
  ASSERT_EQUAL_64(2, x18);
  ASSERT_EQUAL_64(0, x19);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x20);
  ASSERT_EQUAL_64(0, x21);
  ASSERT_EQUAL_64(0xffffff0000000000UL, x22);
  ASSERT_EQUAL_64(1, x24);
  ASSERT_EQUAL_64(2, x25);
  ASSERT_EQUAL_64(0, x26);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x27);
  ASSERT_EQUAL_64(0, x28);
  ASSERT_EQUAL_64(0xfffffffffffff800UL, x29);
  ASSERT_EQUAL_64(0xffffffff, x30);

  TEARDOWN();
}


TEST(Assembler, fcvtzs) {
  SETUP();

  START();
  __ Fmov(s0, 1.0);
  __ Fmov(s1, 1.1);
  __ Fmov(s2, 1.5);
  __ Fmov(s3, -1.5);
  __ Fmov(s4, kFP32PositiveInfinity);
  __ Fmov(s5, kFP32NegativeInfinity);
  __ Fmov(s6, 0x7fffff80);  // Largest float < INT32_MAX.
  __ Fneg(s7, s6);          // Smallest float > INT32_MIN.
  __ Fmov(d8, 1.0);
  __ Fmov(d9, 1.1);
  __ Fmov(d10, 1.5);
  __ Fmov(d11, -1.5);
  __ Fmov(d12, kFP64PositiveInfinity);
  __ Fmov(d13, kFP64NegativeInfinity);
  __ Fmov(d14, kWMaxInt - 1);
  __ Fmov(d15, kWMinInt + 1);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, -1.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, 0x7fffff8000000000UL);   // Largest float < INT64_MAX.
  __ Fneg(s23, s22);                    // Smallest float > INT64_MIN.
  __ Fmov(d24, 1.1);
  __ Fmov(d25, 1.5);
  __ Fmov(d26, -1.5);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0x7ffffffffffffc00UL);   // Largest double < INT64_MAX.
  __ Fneg(d30, d29);                    // Smallest double > INT64_MIN.

  __ Fcvtzs(w0, s0);
  __ Fcvtzs(w1, s1);
  __ Fcvtzs(w2, s2);
  __ Fcvtzs(w3, s3);
  __ Fcvtzs(w4, s4);
  __ Fcvtzs(w5, s5);
  __ Fcvtzs(w6, s6);
  __ Fcvtzs(w7, s7);
  __ Fcvtzs(w8, d8);
  __ Fcvtzs(w9, d9);
  __ Fcvtzs(w10, d10);
  __ Fcvtzs(w11, d11);
  __ Fcvtzs(w12, d12);
  __ Fcvtzs(w13, d13);
  __ Fcvtzs(w14, d14);
  __ Fcvtzs(w15, d15);
  __ Fcvtzs(x17, s17);
  __ Fcvtzs(x18, s18);
  __ Fcvtzs(x19, s19);
  __ Fcvtzs(x20, s20);
  __ Fcvtzs(x21, s21);
  __ Fcvtzs(x22, s22);
  __ Fcvtzs(x23, s23);
  __ Fcvtzs(x24, d24);
  __ Fcvtzs(x25, d25);
  __ Fcvtzs(x26, d26);
  __ Fcvtzs(x27, d27);
  __ Fcvtzs(x28, d28);
  __ Fcvtzs(x29, d29);
  __ Fcvtzs(x30, d30);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(1, x1);
  ASSERT_EQUAL_64(1, x2);
  ASSERT_EQUAL_64(0xffffffff, x3);
  ASSERT_EQUAL_64(0x7fffffff, x4);
  ASSERT_EQUAL_64(0x80000000, x5);
  ASSERT_EQUAL_64(0x7fffff80, x6);
  ASSERT_EQUAL_64(0x80000080, x7);
  ASSERT_EQUAL_64(1, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(1, x10);
  ASSERT_EQUAL_64(0xffffffff, x11);
  ASSERT_EQUAL_64(0x7fffffff, x12);
  ASSERT_EQUAL_64(0x80000000, x13);
  ASSERT_EQUAL_64(0x7ffffffe, x14);
  ASSERT_EQUAL_64(0x80000001, x15);
  ASSERT_EQUAL_64(1, x17);
  ASSERT_EQUAL_64(1, x18);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x19);
  ASSERT_EQUAL_64(0x7fffffffffffffffUL, x20);
  ASSERT_EQUAL_64(0x8000000000000000UL, x21);
  ASSERT_EQUAL_64(0x7fffff8000000000UL, x22);
  ASSERT_EQUAL_64(0x8000008000000000UL, x23);
  ASSERT_EQUAL_64(1, x24);
  ASSERT_EQUAL_64(1, x25);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x26);
  ASSERT_EQUAL_64(0x7fffffffffffffffUL, x27);
  ASSERT_EQUAL_64(0x8000000000000000UL, x28);
  ASSERT_EQUAL_64(0x7ffffffffffffc00UL, x29);
  ASSERT_EQUAL_64(0x8000000000000400UL, x30);

  TEARDOWN();
}

TEST(Assembler, fcvtzu) {
  SETUP();

  START();
  __ Fmov(s0, 1.0);
  __ Fmov(s1, 1.1);
  __ Fmov(s2, 1.5);
  __ Fmov(s3, -1.5);
  __ Fmov(s4, kFP32PositiveInfinity);
  __ Fmov(s5, kFP32NegativeInfinity);
  __ Fmov(s6, 0x7fffff80);  // Largest float < INT32_MAX.
  __ Fneg(s7, s6);          // Smallest float > INT32_MIN.
  __ Fmov(d8, 1.0);
  __ Fmov(d9, 1.1);
  __ Fmov(d10, 1.5);
  __ Fmov(d11, -1.5);
  __ Fmov(d12, kFP64PositiveInfinity);
  __ Fmov(d13, kFP64NegativeInfinity);
  __ Fmov(d14, kWMaxInt - 1);
  __ Fmov(d15, kWMinInt + 1);
  __ Fmov(s17, 1.1);
  __ Fmov(s18, 1.5);
  __ Fmov(s19, -1.5);
  __ Fmov(s20, kFP32PositiveInfinity);
  __ Fmov(s21, kFP32NegativeInfinity);
  __ Fmov(s22, 0x7fffff8000000000UL);  // Largest float < INT64_MAX.
  __ Fneg(s23, s22);                    // Smallest float > INT64_MIN.
  __ Fmov(d24, 1.1);
  __ Fmov(d25, 1.5);
  __ Fmov(d26, -1.5);
  __ Fmov(d27, kFP64PositiveInfinity);
  __ Fmov(d28, kFP64NegativeInfinity);
  __ Fmov(d29, 0x7ffffffffffffc00UL);  // Largest double < INT64_MAX.
  __ Fneg(d30, d29);                    // Smallest double > INT64_MIN.

  __ Fcvtzu(w0, s0);
  __ Fcvtzu(w1, s1);
  __ Fcvtzu(w2, s2);
  __ Fcvtzu(w3, s3);
  __ Fcvtzu(w4, s4);
  __ Fcvtzu(w5, s5);
  __ Fcvtzu(w6, s6);
  __ Fcvtzu(w7, s7);
  __ Fcvtzu(w8, d8);
  __ Fcvtzu(w9, d9);
  __ Fcvtzu(w10, d10);
  __ Fcvtzu(w11, d11);
  __ Fcvtzu(w12, d12);
  __ Fcvtzu(w13, d13);
  __ Fcvtzu(w14, d14);
  __ Fcvtzu(x17, s17);
  __ Fcvtzu(x18, s18);
  __ Fcvtzu(x19, s19);
  __ Fcvtzu(x20, s20);
  __ Fcvtzu(x21, s21);
  __ Fcvtzu(x22, s22);
  __ Fcvtzu(x23, s23);
  __ Fcvtzu(x24, d24);
  __ Fcvtzu(x25, d25);
  __ Fcvtzu(x26, d26);
  __ Fcvtzu(x27, d27);
  __ Fcvtzu(x28, d28);
  __ Fcvtzu(x29, d29);
  __ Fcvtzu(x30, d30);
  END();

  RUN();

  ASSERT_EQUAL_64(1, x0);
  ASSERT_EQUAL_64(1, x1);
  ASSERT_EQUAL_64(1, x2);
  ASSERT_EQUAL_64(0, x3);
  ASSERT_EQUAL_64(0xffffffff, x4);
  ASSERT_EQUAL_64(0, x5);
  ASSERT_EQUAL_64(0x7fffff80, x6);
  ASSERT_EQUAL_64(0, x7);
  ASSERT_EQUAL_64(1, x8);
  ASSERT_EQUAL_64(1, x9);
  ASSERT_EQUAL_64(1, x10);
  ASSERT_EQUAL_64(0, x11);
  ASSERT_EQUAL_64(0xffffffff, x12);
  ASSERT_EQUAL_64(0, x13);
  ASSERT_EQUAL_64(0x7ffffffe, x14);
  ASSERT_EQUAL_64(1, x17);
  ASSERT_EQUAL_64(1, x18);
  ASSERT_EQUAL_64(0x0UL, x19);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x20);
  ASSERT_EQUAL_64(0x0UL, x21);
  ASSERT_EQUAL_64(0x7fffff8000000000UL, x22);
  ASSERT_EQUAL_64(0x0UL, x23);
  ASSERT_EQUAL_64(1, x24);
  ASSERT_EQUAL_64(1, x25);
  ASSERT_EQUAL_64(0x0UL, x26);
  ASSERT_EQUAL_64(0xffffffffffffffffUL, x27);
  ASSERT_EQUAL_64(0x0UL, x28);
  ASSERT_EQUAL_64(0x7ffffffffffffc00UL, x29);
  ASSERT_EQUAL_64(0x0UL, x30);

  TEARDOWN();
}


// Test that scvtf and ucvtf can convert the 64-bit input into the expected
// value. All possible values of 'fbits' are tested. The expected value is
// modified accordingly in each case.
//
// The expected value is specified as the bit encoding of the expected double
// produced by scvtf (expected_scvtf_bits) as well as ucvtf
// (expected_ucvtf_bits).
//
// Where the input value is representable by int32_t or uint32_t, conversions
// from W registers will also be tested.
static void TestUScvtfHelper(uint64_t in,
                             uint64_t expected_scvtf_bits,
                             uint64_t expected_ucvtf_bits) {
  uint64_t u64 = in;
  uint32_t u32 = u64 & 0xffffffff;
  int64_t s64 = static_cast<int64_t>(in);
  int32_t s32 = s64 & 0x7fffffff;

  bool cvtf_s32 = (s64 == s32);
  bool cvtf_u32 = (u64 == u32);

  double results_scvtf_x[65];
  double results_ucvtf_x[65];
  double results_scvtf_w[33];
  double results_ucvtf_w[33];

  SETUP();
  START();

  __ Mov(x0, reinterpret_cast<int64_t>(results_scvtf_x));
  __ Mov(x1, reinterpret_cast<int64_t>(results_ucvtf_x));
  __ Mov(x2, reinterpret_cast<int64_t>(results_scvtf_w));
  __ Mov(x3, reinterpret_cast<int64_t>(results_ucvtf_w));

  __ Mov(x10, s64);

  // Corrupt the top word, in case it is accidentally used during W-register
  // conversions.
  __ Mov(x11, 0x5555555555555555);
  __ Bfi(x11, x10, 0, kWRegSize);

  // Test integer conversions.
  __ Scvtf(d0, x10);
  __ Ucvtf(d1, x10);
  __ Scvtf(d2, w11);
  __ Ucvtf(d3, w11);
  __ Str(d0, MemOperand(x0));
  __ Str(d1, MemOperand(x1));
  __ Str(d2, MemOperand(x2));
  __ Str(d3, MemOperand(x3));

  // Test all possible values of fbits.
  for (int fbits = 1; fbits <= 32; fbits++) {
    __ Scvtf(d0, x10, fbits);
    __ Ucvtf(d1, x10, fbits);
    __ Scvtf(d2, w11, fbits);
    __ Ucvtf(d3, w11, fbits);
    __ Str(d0, MemOperand(x0, fbits * kDRegSizeInBytes));
    __ Str(d1, MemOperand(x1, fbits * kDRegSizeInBytes));
    __ Str(d2, MemOperand(x2, fbits * kDRegSizeInBytes));
    __ Str(d3, MemOperand(x3, fbits * kDRegSizeInBytes));
  }

  // Conversions from W registers can only handle fbits values <= 32, so just
  // test conversions from X registers for 32 < fbits <= 64.
  for (int fbits = 33; fbits <= 64; fbits++) {
    __ Scvtf(d0, x10, fbits);
    __ Ucvtf(d1, x10, fbits);
    __ Str(d0, MemOperand(x0, fbits * kDRegSizeInBytes));
    __ Str(d1, MemOperand(x1, fbits * kDRegSizeInBytes));
  }

  END();
  RUN();

  // Check the results.
  double expected_scvtf_base = rawbits_to_double(expected_scvtf_bits);
  double expected_ucvtf_base = rawbits_to_double(expected_ucvtf_bits);

  for (int fbits = 0; fbits <= 32; fbits++) {
    double expected_scvtf = expected_scvtf_base / pow(2, fbits);
    double expected_ucvtf = expected_ucvtf_base / pow(2, fbits);
    ASSERT_EQUAL_FP64(expected_scvtf, results_scvtf_x[fbits]);
    ASSERT_EQUAL_FP64(expected_ucvtf, results_ucvtf_x[fbits]);
    if (cvtf_s32) ASSERT_EQUAL_FP64(expected_scvtf, results_scvtf_w[fbits]);
    if (cvtf_u32) ASSERT_EQUAL_FP64(expected_ucvtf, results_ucvtf_w[fbits]);
  }
  for (int fbits = 33; fbits <= 64; fbits++) {
    double expected_scvtf = expected_scvtf_base / pow(2, fbits);
    double expected_ucvtf = expected_ucvtf_base / pow(2, fbits);
    ASSERT_EQUAL_FP64(expected_scvtf, results_scvtf_x[fbits]);
    ASSERT_EQUAL_FP64(expected_ucvtf, results_ucvtf_x[fbits]);
  }

  TEARDOWN();
}


TEST(Assembler, scvtf_ucvtf_double) {
  // Simple conversions of positive numbers which require no rounding; the
  // results should not depened on the rounding mode, and ucvtf and scvtf should
  // produce the same result.
  TestUScvtfHelper(0x0000000000000000, 0x0000000000000000, 0x0000000000000000);
  TestUScvtfHelper(0x0000000000000001, 0x3ff0000000000000, 0x3ff0000000000000);
  TestUScvtfHelper(0x0000000040000000, 0x41d0000000000000, 0x41d0000000000000);
  TestUScvtfHelper(0x0000000100000000, 0x41f0000000000000, 0x41f0000000000000);
  TestUScvtfHelper(0x4000000000000000, 0x43d0000000000000, 0x43d0000000000000);
  // Test mantissa extremities.
  TestUScvtfHelper(0x4000000000000400, 0x43d0000000000001, 0x43d0000000000001);
  // The largest int32_t that fits in a double.
  TestUScvtfHelper(0x000000007fffffff, 0x41dfffffffc00000, 0x41dfffffffc00000);
  // Values that would be negative if treated as an int32_t.
  TestUScvtfHelper(0x00000000ffffffff, 0x41efffffffe00000, 0x41efffffffe00000);
  TestUScvtfHelper(0x0000000080000000, 0x41e0000000000000, 0x41e0000000000000);
  TestUScvtfHelper(0x0000000080000001, 0x41e0000000200000, 0x41e0000000200000);
  // The largest int64_t that fits in a double.
  TestUScvtfHelper(0x7ffffffffffffc00, 0x43dfffffffffffff, 0x43dfffffffffffff);
  // Check for bit pattern reproduction.
  TestUScvtfHelper(0x0123456789abcde0, 0x43723456789abcde, 0x43723456789abcde);
  TestUScvtfHelper(0x0000000012345678, 0x41b2345678000000, 0x41b2345678000000);

  // Simple conversions of negative int64_t values. These require no rounding,
  // and the results should not depend on the rounding mode.
  TestUScvtfHelper(0xffffffffc0000000, 0xc1d0000000000000, 0x43effffffff80000);
  TestUScvtfHelper(0xffffffff00000000, 0xc1f0000000000000, 0x43efffffffe00000);
  TestUScvtfHelper(0xc000000000000000, 0xc3d0000000000000, 0x43e8000000000000);

  // Conversions which require rounding.
  TestUScvtfHelper(0x1000000000000000, 0x43b0000000000000, 0x43b0000000000000);
  TestUScvtfHelper(0x1000000000000001, 0x43b0000000000000, 0x43b0000000000000);
  TestUScvtfHelper(0x1000000000000080, 0x43b0000000000000, 0x43b0000000000000);
  TestUScvtfHelper(0x1000000000000081, 0x43b0000000000001, 0x43b0000000000001);
  TestUScvtfHelper(0x1000000000000100, 0x43b0000000000001, 0x43b0000000000001);
  TestUScvtfHelper(0x1000000000000101, 0x43b0000000000001, 0x43b0000000000001);
  TestUScvtfHelper(0x1000000000000180, 0x43b0000000000002, 0x43b0000000000002);
  TestUScvtfHelper(0x1000000000000181, 0x43b0000000000002, 0x43b0000000000002);
  TestUScvtfHelper(0x1000000000000200, 0x43b0000000000002, 0x43b0000000000002);
  TestUScvtfHelper(0x1000000000000201, 0x43b0000000000002, 0x43b0000000000002);
  TestUScvtfHelper(0x1000000000000280, 0x43b0000000000002, 0x43b0000000000002);
  TestUScvtfHelper(0x1000000000000281, 0x43b0000000000003, 0x43b0000000000003);
  TestUScvtfHelper(0x1000000000000300, 0x43b0000000000003, 0x43b0000000000003);
  // Check rounding of negative int64_t values (and large uint64_t values).
  TestUScvtfHelper(0x8000000000000000, 0xc3e0000000000000, 0x43e0000000000000);
  TestUScvtfHelper(0x8000000000000001, 0xc3e0000000000000, 0x43e0000000000000);
  TestUScvtfHelper(0x8000000000000200, 0xc3e0000000000000, 0x43e0000000000000);
  TestUScvtfHelper(0x8000000000000201, 0xc3dfffffffffffff, 0x43e0000000000000);
  TestUScvtfHelper(0x8000000000000400, 0xc3dfffffffffffff, 0x43e0000000000000);
  TestUScvtfHelper(0x8000000000000401, 0xc3dfffffffffffff, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000600, 0xc3dffffffffffffe, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000601, 0xc3dffffffffffffe, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000800, 0xc3dffffffffffffe, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000801, 0xc3dffffffffffffe, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000a00, 0xc3dffffffffffffe, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000a01, 0xc3dffffffffffffd, 0x43e0000000000001);
  TestUScvtfHelper(0x8000000000000c00, 0xc3dffffffffffffd, 0x43e0000000000002);
  // Round up to produce a result that's too big for the input to represent.
  TestUScvtfHelper(0x7ffffffffffffe00, 0x43e0000000000000, 0x43e0000000000000);
  TestUScvtfHelper(0x7fffffffffffffff, 0x43e0000000000000, 0x43e0000000000000);
  TestUScvtfHelper(0xfffffffffffffc00, 0xc090000000000000, 0x43f0000000000000);
  TestUScvtfHelper(0xffffffffffffffff, 0xbff0000000000000, 0x43f0000000000000);
}


// The same as TestUScvtfHelper, but convert to floats.
static void TestUScvtf32Helper(uint64_t in,
                               uint32_t expected_scvtf_bits,
                               uint32_t expected_ucvtf_bits) {
  uint64_t u64 = in;
  uint32_t u32 = u64 & 0xffffffff;
  int64_t s64 = static_cast<int64_t>(in);
  int32_t s32 = s64 & 0x7fffffff;

  bool cvtf_s32 = (s64 == s32);
  bool cvtf_u32 = (u64 == u32);

  float results_scvtf_x[65];
  float results_ucvtf_x[65];
  float results_scvtf_w[33];
  float results_ucvtf_w[33];

  SETUP();
  START();

  __ Mov(x0, reinterpret_cast<int64_t>(results_scvtf_x));
  __ Mov(x1, reinterpret_cast<int64_t>(results_ucvtf_x));
  __ Mov(x2, reinterpret_cast<int64_t>(results_scvtf_w));
  __ Mov(x3, reinterpret_cast<int64_t>(results_ucvtf_w));

  __ Mov(x10, s64);

  // Corrupt the top word, in case it is accidentally used during W-register
  // conversions.
  __ Mov(x11, 0x5555555555555555);
  __ Bfi(x11, x10, 0, kWRegSize);

  // Test integer conversions.
  __ Scvtf(s0, x10);
  __ Ucvtf(s1, x10);
  __ Scvtf(s2, w11);
  __ Ucvtf(s3, w11);
  __ Str(s0, MemOperand(x0));
  __ Str(s1, MemOperand(x1));
  __ Str(s2, MemOperand(x2));
  __ Str(s3, MemOperand(x3));

  // Test all possible values of fbits.
  for (int fbits = 1; fbits <= 32; fbits++) {
    __ Scvtf(s0, x10, fbits);
    __ Ucvtf(s1, x10, fbits);
    __ Scvtf(s2, w11, fbits);
    __ Ucvtf(s3, w11, fbits);
    __ Str(s0, MemOperand(x0, fbits * kSRegSizeInBytes));
    __ Str(s1, MemOperand(x1, fbits * kSRegSizeInBytes));
    __ Str(s2, MemOperand(x2, fbits * kSRegSizeInBytes));
    __ Str(s3, MemOperand(x3, fbits * kSRegSizeInBytes));
  }

  // Conversions from W registers can only handle fbits values <= 32, so just
  // test conversions from X registers for 32 < fbits <= 64.
  for (int fbits = 33; fbits <= 64; fbits++) {
    __ Scvtf(s0, x10, fbits);
    __ Ucvtf(s1, x10, fbits);
    __ Str(s0, MemOperand(x0, fbits * kSRegSizeInBytes));
    __ Str(s1, MemOperand(x1, fbits * kSRegSizeInBytes));
  }

  END();
  RUN();

  // Check the results.
  float expected_scvtf_base = rawbits_to_float(expected_scvtf_bits);
  float expected_ucvtf_base = rawbits_to_float(expected_ucvtf_bits);

  for (int fbits = 0; fbits <= 32; fbits++) {
    float expected_scvtf = expected_scvtf_base / pow(2, fbits);
    float expected_ucvtf = expected_ucvtf_base / pow(2, fbits);
    ASSERT_EQUAL_FP32(expected_scvtf, results_scvtf_x[fbits]);
    ASSERT_EQUAL_FP32(expected_ucvtf, results_ucvtf_x[fbits]);
    if (cvtf_s32) ASSERT_EQUAL_FP32(expected_scvtf, results_scvtf_w[fbits]);
    if (cvtf_u32) ASSERT_EQUAL_FP32(expected_ucvtf, results_ucvtf_w[fbits]);
    break;
  }
  for (int fbits = 33; fbits <= 64; fbits++) {
    break;
    float expected_scvtf = expected_scvtf_base / pow(2, fbits);
    float expected_ucvtf = expected_ucvtf_base / pow(2, fbits);
    ASSERT_EQUAL_FP32(expected_scvtf, results_scvtf_x[fbits]);
    ASSERT_EQUAL_FP32(expected_ucvtf, results_ucvtf_x[fbits]);
  }

  TEARDOWN();
}


TEST(Assembler, scvtf_ucvtf_float) {
  // Simple conversions of positive numbers which require no rounding; the
  // results should not depened on the rounding mode, and ucvtf and scvtf should
  // produce the same result.
  TestUScvtf32Helper(0x0000000000000000, 0x00000000, 0x00000000);
  TestUScvtf32Helper(0x0000000000000001, 0x3f800000, 0x3f800000);
  TestUScvtf32Helper(0x0000000040000000, 0x4e800000, 0x4e800000);
  TestUScvtf32Helper(0x0000000100000000, 0x4f800000, 0x4f800000);
  TestUScvtf32Helper(0x4000000000000000, 0x5e800000, 0x5e800000);
  // Test mantissa extremities.
  TestUScvtf32Helper(0x0000000000800001, 0x4b000001, 0x4b000001);
  TestUScvtf32Helper(0x4000008000000000, 0x5e800001, 0x5e800001);
  // The largest int32_t that fits in a float.
  TestUScvtf32Helper(0x000000007fffff80, 0x4effffff, 0x4effffff);
  // Values that would be negative if treated as an int32_t.
  TestUScvtf32Helper(0x00000000ffffff00, 0x4f7fffff, 0x4f7fffff);
  TestUScvtf32Helper(0x0000000080000000, 0x4f000000, 0x4f000000);
  TestUScvtf32Helper(0x0000000080000100, 0x4f000001, 0x4f000001);
  // The largest int64_t that fits in a float.
  TestUScvtf32Helper(0x7fffff8000000000, 0x5effffff, 0x5effffff);
  // Check for bit pattern reproduction.
  TestUScvtf32Helper(0x0000000000876543, 0x4b076543, 0x4b076543);

  // Simple conversions of negative int64_t values. These require no rounding,
  // and the results should not depend on the rounding mode.
  TestUScvtf32Helper(0xfffffc0000000000, 0xd4800000, 0x5f7ffffc);
  TestUScvtf32Helper(0xc000000000000000, 0xde800000, 0x5f400000);

  // Conversions which require rounding.
  TestUScvtf32Helper(0x0000800000000000, 0x57000000, 0x57000000);
  TestUScvtf32Helper(0x0000800000000001, 0x57000000, 0x57000000);
  TestUScvtf32Helper(0x0000800000800000, 0x57000000, 0x57000000);
  TestUScvtf32Helper(0x0000800000800001, 0x57000001, 0x57000001);
  TestUScvtf32Helper(0x0000800001000000, 0x57000001, 0x57000001);
  TestUScvtf32Helper(0x0000800001000001, 0x57000001, 0x57000001);
  TestUScvtf32Helper(0x0000800001800000, 0x57000002, 0x57000002);
  TestUScvtf32Helper(0x0000800001800001, 0x57000002, 0x57000002);
  TestUScvtf32Helper(0x0000800002000000, 0x57000002, 0x57000002);
  TestUScvtf32Helper(0x0000800002000001, 0x57000002, 0x57000002);
  TestUScvtf32Helper(0x0000800002800000, 0x57000002, 0x57000002);
  TestUScvtf32Helper(0x0000800002800001, 0x57000003, 0x57000003);
  TestUScvtf32Helper(0x0000800003000000, 0x57000003, 0x57000003);
  // Check rounding of negative int64_t values (and large uint64_t values).
  TestUScvtf32Helper(0x8000000000000000, 0xdf000000, 0x5f000000);
  TestUScvtf32Helper(0x8000000000000001, 0xdf000000, 0x5f000000);
  TestUScvtf32Helper(0x8000004000000000, 0xdf000000, 0x5f000000);
  TestUScvtf32Helper(0x8000004000000001, 0xdeffffff, 0x5f000000);
  TestUScvtf32Helper(0x8000008000000000, 0xdeffffff, 0x5f000000);
  TestUScvtf32Helper(0x8000008000000001, 0xdeffffff, 0x5f000001);
  TestUScvtf32Helper(0x800000c000000000, 0xdefffffe, 0x5f000001);
  TestUScvtf32Helper(0x800000c000000001, 0xdefffffe, 0x5f000001);
  TestUScvtf32Helper(0x8000010000000000, 0xdefffffe, 0x5f000001);
  TestUScvtf32Helper(0x8000010000000001, 0xdefffffe, 0x5f000001);
  TestUScvtf32Helper(0x8000014000000000, 0xdefffffe, 0x5f000001);
  TestUScvtf32Helper(0x8000014000000001, 0xdefffffd, 0x5f000001);
  TestUScvtf32Helper(0x8000018000000000, 0xdefffffd, 0x5f000002);
  // Round up to produce a result that's too big for the input to represent.
  TestUScvtf32Helper(0x000000007fffffc0, 0x4f000000, 0x4f000000);
  TestUScvtf32Helper(0x000000007fffffff, 0x4f000000, 0x4f000000);
  TestUScvtf32Helper(0x00000000ffffff80, 0x4f800000, 0x4f800000);
  TestUScvtf32Helper(0x00000000ffffffff, 0x4f800000, 0x4f800000);
  TestUScvtf32Helper(0x7fffffc000000000, 0x5f000000, 0x5f000000);
  TestUScvtf32Helper(0x7fffffffffffffff, 0x5f000000, 0x5f000000);
  TestUScvtf32Helper(0xffffff8000000000, 0xd3000000, 0x5f800000);
  TestUScvtf32Helper(0xffffffffffffffff, 0xbf800000, 0x5f800000);
}


TEST(Assembler, system_mrs) {
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
  __ Add(w0, w2, w2, SetFlags);
  __ Mrs(x5, NZCV);

  // Read the default FPCR.
  __ Mrs(x6, FPCR);
  END();

  RUN();

  // NZCV
  ASSERT_EQUAL_32(ZCFlag, w3);
  ASSERT_EQUAL_32(NFlag, w4);
  ASSERT_EQUAL_32(ZCVFlag, w5);

  // FPCR
  // The default FPCR on Linux-based platforms is 0.
  ASSERT_EQUAL_32(0, w6);

  TEARDOWN();
}


TEST(Assembler, system_msr) {
  // All FPCR fields that must be implemented: AHP, DN, FZ, RMode
  const uint64_t fpcr_core = 0x07c00000;

  // All FPCR fields (including fields which may be read-as-zero):
  //  Stride, Len
  //  IDE, IXE, UFE, OFE, DZE, IOE
  const uint64_t fpcr_all = fpcr_core | 0x00379f00;

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

  // All core FPCR fields must be writable.
  __ Mov(x8, fpcr_core);
  __ Msr(FPCR, x8);
  __ Mrs(x8, FPCR);

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

  END();

  RUN();

  // We should have incremented x7 (from 0) exactly 8 times.
  ASSERT_EQUAL_64(8, x7);

  ASSERT_EQUAL_64(fpcr_core, x8);
  ASSERT_EQUAL_64(fpcr_core, x9);
  ASSERT_EQUAL_64(0, x10);

  TEARDOWN();
}


TEST(Assembler, system_nop) {
  SETUP();
  RegisterDump before;

  START();
  before.Dump(&masm);
  __ Nop();
  END();

  RUN();

  ASSERT_EQUAL_REGISTERS(before);
  ASSERT_EQUAL_NZCV(before.flags_nzcv());

  TEARDOWN();
}


TEST(Assembler, zero_dest) {
  SETUP();
  RegisterDump before;

  START();
  // Preserve the stack pointer, in case we clobber it.
  __ Mov(x30, sp);
  // Initialize the other registers used in this test.
  uint64_t literal_base = 0x0100001000100101UL;
  __ Mov(x0, 0);
  __ Mov(x1, literal_base);
  for (unsigned i = 2; i < x30.code(); i++) {
    __ Add(Register::XRegFromCode(i), Register::XRegFromCode(i-1), x1);
  }
  before.Dump(&masm);

  // All of these instructions should be NOPs in these forms, but have
  // alternate forms which can write into the stack pointer.
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

  RUN();

  ASSERT_EQUAL_REGISTERS(before);
  ASSERT_EQUAL_NZCV(before.flags_nzcv());

  TEARDOWN();
}


TEST(Assembler, zero_dest_setflags) {
  SETUP();
  RegisterDump before;

  START();
  // Preserve the stack pointer, in case we clobber it.
  __ Mov(x30, sp);
  // Initialize the other registers used in this test.
  uint64_t literal_base = 0x0100001000100101UL;
  __ Mov(x0, 0);
  __ Mov(x1, literal_base);
  for (int i = 2; i < 30; i++) {
    __ Add(Register::XRegFromCode(i), Register::XRegFromCode(i-1), x1);
  }
  before.Dump(&masm);

  // All of these instructions should only write to the flags in these forms,
  // but have alternate forms which can write into the stack pointer.
  __ add(xzr, x0, Operand(x1, UXTX), SetFlags);
  __ add(xzr, x1, Operand(xzr, UXTX), SetFlags);
  __ add(xzr, x1, 1234, SetFlags);
  __ add(xzr, x0, x1, SetFlags);
  __ add(xzr, x1, xzr, SetFlags);
  __ add(xzr, xzr, x1, SetFlags);

  __ and_(xzr, x2, ~0xf, SetFlags);
  __ and_(xzr, xzr, ~0xf, SetFlags);
  __ and_(xzr, x0, x2, SetFlags);
  __ and_(xzr, x2, xzr, SetFlags);
  __ and_(xzr, xzr, x2, SetFlags);

  __ bic(xzr, x3, ~0xf, SetFlags);
  __ bic(xzr, xzr, ~0xf, SetFlags);
  __ bic(xzr, x0, x3, SetFlags);
  __ bic(xzr, x3, xzr, SetFlags);
  __ bic(xzr, xzr, x3, SetFlags);

  __ sub(xzr, x0, Operand(x3, UXTX), SetFlags);
  __ sub(xzr, x3, Operand(xzr, UXTX), SetFlags);
  __ sub(xzr, x3, 1234, SetFlags);
  __ sub(xzr, x0, x3, SetFlags);
  __ sub(xzr, x3, xzr, SetFlags);
  __ sub(xzr, xzr, x3, SetFlags);

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

  RUN();

  ASSERT_EQUAL_REGISTERS(before);

  TEARDOWN();
}


TEST(Assembler, register_bit) {
  // No code generation takes place in this test, so no need to setup and
  // teardown.

  // Simple tests.
  assert(x0.Bit() == (1UL << 0));
  assert(x1.Bit() == (1UL << 1));
  assert(x10.Bit() == (1UL << 10));

  // AAPCS64 definitions.
  assert(lr.Bit() == (1UL << kLinkRegCode));

  // Fixed (hardware) definitions.
  assert(xzr.Bit() == (1UL << kZeroRegCode));

  // Internal ABI definitions.
  assert(sp.Bit() == (1UL << kSPRegInternalCode));
  assert(sp.Bit() != xzr.Bit());

  // xn.Bit() == wn.Bit() at all times, for the same n.
  assert(x0.Bit() == w0.Bit());
  assert(x1.Bit() == w1.Bit());
  assert(x10.Bit() == w10.Bit());
  assert(xzr.Bit() == wzr.Bit());
  assert(sp.Bit() == wsp.Bit());
}


TEST(Assembler, stack_pointer_override) {
  // This test generates some stack maintenance code, but the test only checks
  // the reported state.
  SETUP();
  START();

  // The default stack pointer in VIXL is sp.
  assert(sp.Is(__ StackPointer()));
  __ SetStackPointer(x0);
  assert(x0.Is(__ StackPointer()));
  __ SetStackPointer(x28);
  assert(x28.Is(__ StackPointer()));
  __ SetStackPointer(sp);
  assert(sp.Is(__ StackPointer()));

  END();
  RUN();
  TEARDOWN();
}


TEST(Assembler, peek_poke_simple) {
  SETUP();
  START();

  static const RegList x0_to_x3 = x0.Bit() | x1.Bit() | x2.Bit() | x3.Bit();
  static const RegList x10_to_x13 = x10.Bit() | x11.Bit() |
                                    x12.Bit() | x13.Bit();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101UL;

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
  RUN();

  ASSERT_EQUAL_64(literal_base * 1, x0);
  ASSERT_EQUAL_64(literal_base * 2, x1);
  ASSERT_EQUAL_64(literal_base * 3, x2);
  ASSERT_EQUAL_64(literal_base * 4, x3);

  ASSERT_EQUAL_64((literal_base * 1) & 0xffffffff, x10);
  ASSERT_EQUAL_64((literal_base * 2) & 0xffffffff, x11);
  ASSERT_EQUAL_64((literal_base * 3) & 0xffffffff, x12);
  ASSERT_EQUAL_64((literal_base * 4) & 0xffffffff, x13);

  TEARDOWN();
}


TEST(Assembler, peek_poke_unaligned) {
  SETUP();
  START();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101UL;

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
  Clobber(&masm, x0.Bit());
  __ Peek(x0, 1);
  __ Poke(x1, 2);
  Clobber(&masm, x1.Bit());
  __ Peek(x1, 2);
  __ Poke(x2, 3);
  Clobber(&masm, x2.Bit());
  __ Peek(x2, 3);
  __ Poke(x3, 4);
  Clobber(&masm, x3.Bit());
  __ Peek(x3, 4);
  __ Poke(x4, 5);
  Clobber(&masm, x4.Bit());
  __ Peek(x4, 5);
  __ Poke(x5, 6);
  Clobber(&masm, x5.Bit());
  __ Peek(x5, 6);
  __ Poke(x6, 7);
  Clobber(&masm, x6.Bit());
  __ Peek(x6, 7);

  __ Poke(w0, 1);
  Clobber(&masm, w10.Bit());
  __ Peek(w10, 1);
  __ Poke(w1, 2);
  Clobber(&masm, w11.Bit());
  __ Peek(w11, 2);
  __ Poke(w2, 3);
  Clobber(&masm, w12.Bit());
  __ Peek(w12, 3);

  __ Drop(32);

  END();
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

  TEARDOWN();
}


TEST(Assembler, peek_poke_endianness) {
  SETUP();
  START();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101UL;

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
  RUN();

  uint64_t x0_expected = literal_base * 1;
  uint64_t x1_expected = literal_base * 2;
  uint64_t x4_expected = (x0_expected << 32) | (x0_expected >> 32);
  uint64_t x5_expected = ((x1_expected << 16) & 0xffff0000) |
                         ((x1_expected >> 16) & 0x0000ffff);

  ASSERT_EQUAL_64(x0_expected, x0);
  ASSERT_EQUAL_64(x1_expected, x1);
  ASSERT_EQUAL_64(x4_expected, x4);
  ASSERT_EQUAL_64(x5_expected, x5);

  TEARDOWN();
}


TEST(Assembler, peek_poke_mixed) {
  SETUP();
  START();

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101UL;

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
    assert(__ StackPointer().Is(sp));
    __ Mov(x4, __ StackPointer());
    __ SetStackPointer(x4);

    __ Poke(wzr, 0);    // Clobber the space we're about to drop.
    __ Drop(4);
    __ Peek(x6, 0);
    __ Claim(8);
    __ Peek(w7, 10);
    __ Poke(x3, 28);
    __ Poke(xzr, 0);    // Clobber the space we're about to drop.
    __ Drop(8);
    __ Poke(x2, 12);
    __ Push(w0);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  __ Pop(x0, x1, x2, x3);

  END();
  RUN();

  uint64_t x0_expected = literal_base * 1;
  uint64_t x1_expected = literal_base * 2;
  uint64_t x2_expected = literal_base * 3;
  uint64_t x3_expected = literal_base * 4;
  uint64_t x6_expected = (x1_expected << 32) | (x0_expected >> 32);
  uint64_t x7_expected = ((x1_expected << 16) & 0xffff0000) |
                         ((x0_expected >> 48) & 0x0000ffff);

  ASSERT_EQUAL_64(x0_expected, x0);
  ASSERT_EQUAL_64(x1_expected, x1);
  ASSERT_EQUAL_64(x2_expected, x2);
  ASSERT_EQUAL_64(x3_expected, x3);
  ASSERT_EQUAL_64(x6_expected, x6);
  ASSERT_EQUAL_64(x7_expected, x7);

  TEARDOWN();
}


// This enum is used only as an argument to the push-pop test helpers.
enum PushPopMethod {
  // Push or Pop using the Push and Pop methods, with blocks of up to four
  // registers. (Smaller blocks will be used if necessary.)
  PushPopByFour,

  // Use Push<Size>RegList and Pop<Size>RegList to transfer the registers.
  PushPopRegList
};


// The maximum number of registers that can be used by the PushPopXReg* tests,
// where a reg_count field is provided.
static int const kPushPopXRegMaxRegCount = -1;

// Test a simple push-pop pattern:
//  * Claim <claim> bytes to set the stack alignment.
//  * Push <reg_count> registers with size <reg_size>.
//  * Clobber the register contents.
//  * Pop <reg_count> registers to restore the original contents.
//  * Drop <claim> bytes to restore the original stack pointer.
//
// Different push and pop methods can be specified independently to test for
// proper word-endian behaviour.
static void PushPopXRegSimpleHelper(int reg_count,
                                    int claim,
                                    int reg_size,
                                    PushPopMethod push_method,
                                    PushPopMethod pop_method) {
  SETUP();

  START();

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x20;
  const RegList allowed = ~stack_pointer.Bit();
  if (reg_count == kPushPopXRegMaxRegCount) {
    reg_count = CountSetBits(allowed, kNumberOfRegisters);
  }
  // Work out which registers to use, based on reg_size.
  Register r[kNumberOfRegisters];
  Register x[kNumberOfRegisters];
  RegList list = PopulateRegisterArray(nullptr, x, r, reg_size, reg_count,
                                       allowed);

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101UL;

  {
    assert(__ StackPointer().Is(sp));
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
          __ Push(r[i-1], r[i-2], r[i-3], r[i-4]);
        }
        // Finish off the leftovers.
        switch (i) {
          case 3:  __ Push(r[2], r[1], r[0]); break;
          case 2:  __ Push(r[1], r[0]);       break;
          case 1:  __ Push(r[0]);             break;
          default: assert(i == 0);            break;
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
        for (i = 0; i <= (reg_count-4); i += 4) {
          __ Pop(r[i], r[i+1], r[i+2], r[i+3]);
        }
        // Finish off the leftovers.
        switch (reg_count - i) {
          case 3:  __ Pop(r[i], r[i+1], r[i+2]); break;
          case 2:  __ Pop(r[i], r[i+1]);         break;
          case 1:  __ Pop(r[i]);                 break;
          default: assert(i == reg_count);       break;
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

  RUN();

  // Check that the register contents were preserved.
  // Always use ASSERT_EQUAL_64, even when testing W registers, so we can test
  // that the upper word was properly cleared by Pop.
  literal_base &= (0xffffffffffffffffUL >> (64-reg_size));
  for (int i = 0; i < reg_count; i++) {
    if (x[i].Is(xzr)) {
      ASSERT_EQUAL_64(0, x[i]);
    } else {
      ASSERT_EQUAL_64(literal_base * i, x[i]);
    }
  }

  TEARDOWN();
}


TEST(Assembler, push_pop_xreg_simple_32) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopXRegSimpleHelper(count, claim, kWRegSize,
                              PushPopByFour, PushPopByFour);
      PushPopXRegSimpleHelper(count, claim, kWRegSize,
                              PushPopByFour, PushPopRegList);
      PushPopXRegSimpleHelper(count, claim, kWRegSize,
                              PushPopRegList, PushPopByFour);
      PushPopXRegSimpleHelper(count, claim, kWRegSize,
                              PushPopRegList, PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kWRegSize, PushPopByFour, PushPopByFour);
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kWRegSize, PushPopByFour, PushPopRegList);
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kWRegSize, PushPopRegList, PushPopByFour);
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kWRegSize, PushPopRegList, PushPopRegList);
  }
}


TEST(Assembler, push_pop_xreg_simple_64) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopXRegSimpleHelper(count, claim, kXRegSize,
                              PushPopByFour, PushPopByFour);
      PushPopXRegSimpleHelper(count, claim, kXRegSize,
                              PushPopByFour, PushPopRegList);
      PushPopXRegSimpleHelper(count, claim, kXRegSize,
                              PushPopRegList, PushPopByFour);
      PushPopXRegSimpleHelper(count, claim, kXRegSize,
                              PushPopRegList, PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kXRegSize, PushPopByFour, PushPopByFour);
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kXRegSize, PushPopByFour, PushPopRegList);
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kXRegSize, PushPopRegList, PushPopByFour);
    PushPopXRegSimpleHelper(kPushPopXRegMaxRegCount,
                            claim, kXRegSize, PushPopRegList, PushPopRegList);
  }
}


// The maximum number of registers that can be used by the PushPopFPXReg* tests,
// where a reg_count field is provided.
static int const kPushPopFPXRegMaxRegCount = -1;

// Test a simple push-pop pattern:
//  * Claim <claim> bytes to set the stack alignment.
//  * Push <reg_count> FP registers with size <reg_size>.
//  * Clobber the register contents.
//  * Pop <reg_count> FP registers to restore the original contents.
//  * Drop <claim> bytes to restore the original stack pointer.
//
// Different push and pop methods can be specified independently to test for
// proper word-endian behaviour.
static void PushPopFPXRegSimpleHelper(int reg_count,
                                      int claim,
                                      int reg_size,
                                      PushPopMethod push_method,
                                      PushPopMethod pop_method) {
  SETUP();

  START();

  // We can use any floating-point register. None of them are reserved for
  // debug code, for example.
  static RegList const allowed = ~0;
  if (reg_count == kPushPopFPXRegMaxRegCount) {
    reg_count = CountSetBits(allowed, kNumberOfFPRegisters);
  }
  // Work out which registers to use, based on reg_size.
  FPRegister v[kNumberOfRegisters];
  FPRegister d[kNumberOfRegisters];
  RegList list = PopulateFPRegisterArray(nullptr, d, v, reg_size, reg_count,
                                         allowed);

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x10;

  // The literal base is chosen to have two useful properties:
  //  * When multiplied (using an integer) by small values (such as a register
  //    index), this value is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  //  * It is never a floating-point NaN, and will therefore always compare
  //    equal to itself.
  uint64_t literal_base = 0x0100001000100101UL;

  {
    assert(__ StackPointer().Is(sp));
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
          __ Push(v[i-1], v[i-2], v[i-3], v[i-4]);
        }
        // Finish off the leftovers.
        switch (i) {
          case 3:  __ Push(v[2], v[1], v[0]); break;
          case 2:  __ Push(v[1], v[0]);       break;
          case 1:  __ Push(v[0]);             break;
          default: assert(i == 0);            break;
        }
        break;
      case PushPopRegList:
        __ PushSizeRegList(list, reg_size, CPURegister::kFPRegister);
        break;
    }

    // Clobber all the registers, to ensure that they get repopulated by Pop.
    ClobberFP(&masm, list);

    switch (pop_method) {
      case PushPopByFour:
        // Pop low-numbered registers first (from the lowest addresses).
        for (i = 0; i <= (reg_count-4); i += 4) {
          __ Pop(v[i], v[i+1], v[i+2], v[i+3]);
        }
        // Finish off the leftovers.
        switch (reg_count - i) {
          case 3:  __ Pop(v[i], v[i+1], v[i+2]); break;
          case 2:  __ Pop(v[i], v[i+1]);         break;
          case 1:  __ Pop(v[i]);                 break;
          default: assert(i == reg_count);       break;
        }
        break;
      case PushPopRegList:
        __ PopSizeRegList(list, reg_size, CPURegister::kFPRegister);
        break;
    }

    // Drop memory to restore the stack pointer.
    __ Drop(claim);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  END();

  RUN();

  // Check that the register contents were preserved.
  // Always use ASSERT_EQUAL_FP64, even when testing S registers, so we can
  // test that the upper word was properly cleared by Pop.
  literal_base &= (0xffffffffffffffffUL >> (64-reg_size));
  for (int i = 0; i < reg_count; i++) {
    uint64_t literal = literal_base * i;
    double expected;
    memcpy(&expected, &literal, sizeof(expected));
    ASSERT_EQUAL_FP64(expected, d[i]);
  }

  TEARDOWN();
}


TEST(Assembler, push_pop_fp_xreg_simple_32) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopFPXRegSimpleHelper(count, claim, kSRegSize,
                                PushPopByFour, PushPopByFour);
      PushPopFPXRegSimpleHelper(count, claim, kSRegSize,
                                PushPopByFour, PushPopRegList);
      PushPopFPXRegSimpleHelper(count, claim, kSRegSize,
                                PushPopRegList, PushPopByFour);
      PushPopFPXRegSimpleHelper(count, claim, kSRegSize,
                                PushPopRegList, PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kSRegSize,
                              PushPopByFour, PushPopByFour);
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kSRegSize,
                              PushPopByFour, PushPopRegList);
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kSRegSize,
                              PushPopRegList, PushPopByFour);
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kSRegSize,
                              PushPopRegList, PushPopRegList);
  }
}


TEST(Assembler, push_pop_fp_xreg_simple_64) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 0; count <= 8; count++) {
      PushPopFPXRegSimpleHelper(count, claim, kDRegSize,
                                PushPopByFour, PushPopByFour);
      PushPopFPXRegSimpleHelper(count, claim, kDRegSize,
                                PushPopByFour, PushPopRegList);
      PushPopFPXRegSimpleHelper(count, claim, kDRegSize,
                                PushPopRegList, PushPopByFour);
      PushPopFPXRegSimpleHelper(count, claim, kDRegSize,
                                PushPopRegList, PushPopRegList);
    }
    // Test with the maximum number of registers.
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kDRegSize,
                              PushPopByFour, PushPopByFour);
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kDRegSize,
                              PushPopByFour, PushPopRegList);
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kDRegSize,
                              PushPopRegList, PushPopByFour);
    PushPopFPXRegSimpleHelper(kPushPopFPXRegMaxRegCount, claim, kDRegSize,
                              PushPopRegList, PushPopRegList);
  }
}


// Push and pop data using an overlapping combination of Push/Pop and
// RegList-based methods.
static void PushPopXRegMixedMethodsHelper(int claim, int reg_size) {
  SETUP();

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x5;
  const RegList allowed = ~stack_pointer.Bit();
  // Work out which registers to use, based on reg_size.
  Register r[10];
  Register x[10];
  PopulateRegisterArray(nullptr, x, r, reg_size, 10, allowed);

  // Calculate some handy register lists.
  RegList r0_to_r3 = 0;
  for (int i = 0; i <= 3; i++) {
    r0_to_r3 |= x[i].Bit();
  }
  RegList r4_to_r5 = 0;
  for (int i = 4; i <= 5; i++) {
    r4_to_r5 |= x[i].Bit();
  }
  RegList r6_to_r9 = 0;
  for (int i = 6; i <= 9; i++) {
    r6_to_r9 |= x[i].Bit();
  }

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  uint64_t literal_base = 0x0100001000100101UL;

  START();
  {
    assert(__ StackPointer().Is(sp));
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

  RUN();

  // Always use ASSERT_EQUAL_64, even when testing W registers, so we can test
  // that the upper word was properly cleared by Pop.
  literal_base &= (0xffffffffffffffffUL >> (64-reg_size));

  ASSERT_EQUAL_64(literal_base * 3, x[9]);
  ASSERT_EQUAL_64(literal_base * 2, x[8]);
  ASSERT_EQUAL_64(literal_base * 0, x[7]);
  ASSERT_EQUAL_64(literal_base * 3, x[6]);
  ASSERT_EQUAL_64(literal_base * 1, x[5]);
  ASSERT_EQUAL_64(literal_base * 2, x[4]);

  TEARDOWN();
}


TEST(Assembler, push_pop_xreg_mixed_methods_64) {
  for (int claim = 0; claim <= 8; claim++) {
    PushPopXRegMixedMethodsHelper(claim, kXRegSize);
  }
}


TEST(Assembler, push_pop_xreg_mixed_methods_32) {
  for (int claim = 0; claim <= 8; claim++) {
    PushPopXRegMixedMethodsHelper(claim, kWRegSize);
  }
}


// Push and pop data using overlapping X- and W-sized quantities.
static void PushPopXRegWXOverlapHelper(int reg_count, int claim) {
  SETUP();

  // Arbitrarily pick a register to use as a stack pointer.
  const Register& stack_pointer = x10;
  const RegList allowed = ~stack_pointer.Bit();
  if (reg_count == kPushPopXRegMaxRegCount) {
    reg_count = CountSetBits(allowed, kNumberOfRegisters);
  }
  // Work out which registers to use, based on reg_size.
  Register w[kNumberOfRegisters];
  Register x[kNumberOfRegisters];
  RegList list = PopulateRegisterArray(w, x, nullptr, 0, reg_count, allowed);

  // The number of W-sized slots we expect to pop. When we pop, we alternate
  // between W and X registers, so we need reg_count*1.5 W-sized slots.
  int const requested_w_slots = reg_count + reg_count / 2;

  // Track what _should_ be on the stack, using W-sized slots.
  static int const kMaxWSlots = kNumberOfRegisters + kNumberOfRegisters / 2;
  uint32_t stack[kMaxWSlots];
  for (int i = 0; i < kMaxWSlots; i++) {
    stack[i] = 0xdeadbeef;
  }

  // The literal base is chosen to have two useful properties:
  //  * When multiplied by small values (such as a register index), this value
  //    is clearly readable in the result.
  //  * The value is not formed from repeating fixed-size smaller values, so it
  //    can be used to detect endianness-related errors.
  static uint64_t const literal_base = 0x0100001000100101UL;
  static uint64_t const literal_base_hi = literal_base >> 32;
  static uint64_t const literal_base_lo = literal_base & 0xffffffff;
  static uint64_t const literal_base_w = literal_base & 0xffffffff;

  START();
  {
    assert(__ StackPointer().Is(sp));
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
      assert(i < reg_count);
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
      // and fill the empty space with a dummy value.
      do {
        stack[active_w_slots--] = 0xdeadbeef;
      } while (active_w_slots > requested_w_slots);
    }

    // ---- Pop ----

    Clobber(&masm, list);

    // If popping an even number of registers, the first one will be X-sized.
    // Otherwise, the first one will be W-sized.
    bool next_is_64 = !(reg_count & 1);
    for (int i = reg_count-1; i >= 0; i--) {
      if (next_is_64) {
        __ Pop(x[i]);
        active_w_slots -= 2;
      } else {
        __ Pop(w[i]);
        active_w_slots -= 1;
      }
      next_is_64 = !next_is_64;
    }
    assert(active_w_slots == 0);

    // Drop memory to restore stack_pointer.
    __ Drop(claim);

    __ Mov(sp, __ StackPointer());
    __ SetStackPointer(sp);
  }

  END();

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
  assert(slot == requested_w_slots);

  TEARDOWN();
}


TEST(Assembler, push_pop_xreg_wx_overlap) {
  for (int claim = 0; claim <= 8; claim++) {
    for (int count = 1; count <= 8; count++) {
      PushPopXRegWXOverlapHelper(count, claim);
    }
    // Test with the maximum number of registers.
    PushPopXRegWXOverlapHelper(kPushPopXRegMaxRegCount, claim);
  }
}


TEST(Assembler, push_pop_sp) {
  SETUP();

  START();

  assert(sp.Is(__ StackPointer()));

  __ Mov(x3, 0x3333333333333333UL);
  __ Mov(x2, 0x2222222222222222UL);
  __ Mov(x1, 0x1111111111111111UL);
  __ Mov(x0, 0x0000000000000000UL);
  __ Claim(2 * kXRegSizeInBytes);
  __ PushXRegList(x0.Bit() | x1.Bit() | x2.Bit() | x3.Bit());
  __ Push(x3, x2);
  __ PopXRegList(x0.Bit() | x1.Bit() | x2.Bit() | x3.Bit());
  __ Push(x2, x1, x3, x0);
  __ Pop(x4, x5);
  __ Pop(x6, x7, x8, x9);

  __ Claim(2 * kXRegSizeInBytes);
  __ PushWRegList(w0.Bit() | w1.Bit() | w2.Bit() | w3.Bit());
  __ Push(w3, w1, w2, w0);
  __ PopWRegList(w10.Bit() | w11.Bit() | w12.Bit() | w13.Bit());
  __ Pop(w14, w15, w16, w17);

  __ Claim(2 * kXRegSizeInBytes);
  __ Push(w2, w2, w1, w1);
  __ Push(x3, x3);
  __ Pop(w18, w19, w20, w21);
  __ Pop(x22, x23);

  __ Claim(2 * kXRegSizeInBytes);
  __ PushXRegList(x1.Bit() | x22.Bit());
  __ PopXRegList(x24.Bit() | x26.Bit());

  __ Claim(2 * kXRegSizeInBytes);
  __ PushWRegList(w1.Bit() | w2.Bit() | w4.Bit() | w22.Bit());
  __ PopWRegList(w25.Bit() | w27.Bit() | w28.Bit() | w29.Bit());

  __ Claim(2 * kXRegSizeInBytes);
  __ PushXRegList(0);
  __ PopXRegList(0);
  __ PushXRegList(0xffffffff);
  __ PopXRegList(0xffffffff);
  __ Drop(12 * kXRegSizeInBytes);
  END();

  RUN();

  ASSERT_EQUAL_64(0x1111111111111111UL, x3);
  ASSERT_EQUAL_64(0x0000000000000000UL, x2);
  ASSERT_EQUAL_64(0x3333333333333333UL, x1);
  ASSERT_EQUAL_64(0x2222222222222222UL, x0);
  ASSERT_EQUAL_64(0x3333333333333333UL, x9);
  ASSERT_EQUAL_64(0x2222222222222222UL, x8);
  ASSERT_EQUAL_64(0x0000000000000000UL, x7);
  ASSERT_EQUAL_64(0x3333333333333333UL, x6);
  ASSERT_EQUAL_64(0x1111111111111111UL, x5);
  ASSERT_EQUAL_64(0x2222222222222222UL, x4);

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
  ASSERT_EQUAL_64(0x3333333333333333UL, x22);
  ASSERT_EQUAL_64(0x0000000000000000UL, x23);

  ASSERT_EQUAL_64(0x3333333333333333UL, x24);
  ASSERT_EQUAL_64(0x3333333333333333UL, x26);

  ASSERT_EQUAL_32(0x33333333U, w25);
  ASSERT_EQUAL_32(0x00000000U, w27);
  ASSERT_EQUAL_32(0x22222222U, w28);
  ASSERT_EQUAL_32(0x33333333U, w29);
  TEARDOWN();
}


TEST(Assembler, noreg) {
  // This test doesn't generate any code, but it verifies some invariants
  // related to NoReg.
  assert(NoReg.Is(NoFPReg));
  assert(NoFPReg.Is(NoReg));
  assert(NoReg.Is(NoCPUReg));
  assert(NoCPUReg.Is(NoReg));
  assert(NoFPReg.Is(NoCPUReg));
  assert(NoCPUReg.Is(NoFPReg));

  assert(NoReg.IsNone());
  assert(NoFPReg.IsNone());
  assert(NoCPUReg.IsNone());
}


TEST(Assembler, isvalid) {
  // This test doesn't generate any code, but it verifies some invariants
  // related to IsValid().
  assert(!NoReg.IsValid());
  assert(!NoFPReg.IsValid());
  assert(!NoCPUReg.IsValid());

  assert(x0.IsValid());
  assert(w0.IsValid());
  assert(x30.IsValid());
  assert(w30.IsValid());
  assert(xzr.IsValid());
  assert(wzr.IsValid());

  assert(sp.IsValid());
  assert(wsp.IsValid());

  assert(d0.IsValid());
  assert(s0.IsValid());
  assert(d31.IsValid());
  assert(s31.IsValid());

  assert(x0.IsValidRegister());
  assert(w0.IsValidRegister());
  assert(xzr.IsValidRegister());
  assert(wzr.IsValidRegister());
  assert(sp.IsValidRegister());
  assert(wsp.IsValidRegister());
  assert(!x0.IsValidFPRegister());
  assert(!w0.IsValidFPRegister());
  assert(!xzr.IsValidFPRegister());
  assert(!wzr.IsValidFPRegister());
  assert(!sp.IsValidFPRegister());
  assert(!wsp.IsValidFPRegister());

  assert(d0.IsValidFPRegister());
  assert(s0.IsValidFPRegister());
  assert(!d0.IsValidRegister());
  assert(!s0.IsValidRegister());

  // Test the same as before, but using CPURegister types. This shouldn't make
  // any difference.
  assert(static_cast<CPURegister>(x0).IsValid());
  assert(static_cast<CPURegister>(w0).IsValid());
  assert(static_cast<CPURegister>(x30).IsValid());
  assert(static_cast<CPURegister>(w30).IsValid());
  assert(static_cast<CPURegister>(xzr).IsValid());
  assert(static_cast<CPURegister>(wzr).IsValid());

  assert(static_cast<CPURegister>(sp).IsValid());
  assert(static_cast<CPURegister>(wsp).IsValid());

  assert(static_cast<CPURegister>(d0).IsValid());
  assert(static_cast<CPURegister>(s0).IsValid());
  assert(static_cast<CPURegister>(d31).IsValid());
  assert(static_cast<CPURegister>(s31).IsValid());

  assert(static_cast<CPURegister>(x0).IsValidRegister());
  assert(static_cast<CPURegister>(w0).IsValidRegister());
  assert(static_cast<CPURegister>(xzr).IsValidRegister());
  assert(static_cast<CPURegister>(wzr).IsValidRegister());
  assert(static_cast<CPURegister>(sp).IsValidRegister());
  assert(static_cast<CPURegister>(wsp).IsValidRegister());
  assert(!static_cast<CPURegister>(x0).IsValidFPRegister());
  assert(!static_cast<CPURegister>(w0).IsValidFPRegister());
  assert(!static_cast<CPURegister>(xzr).IsValidFPRegister());
  assert(!static_cast<CPURegister>(wzr).IsValidFPRegister());
  assert(!static_cast<CPURegister>(sp).IsValidFPRegister());
  assert(!static_cast<CPURegister>(wsp).IsValidFPRegister());

  assert(static_cast<CPURegister>(d0).IsValidFPRegister());
  assert(static_cast<CPURegister>(s0).IsValidFPRegister());
  assert(!static_cast<CPURegister>(d0).IsValidRegister());
  assert(!static_cast<CPURegister>(s0).IsValidRegister());
}


TEST(Assembler, printf) {
  SETUP();
  START();

  char const * test_plain_string = "Printf with no arguments.\n";
  char const * test_substring = "'This is a substring.'";
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

  // Check that we don't clobber any registers, except those that we explicitly
  // write results into.
  before.Dump(&masm);

  __ Printf(test_plain_string);   // NOLINT(runtime/printf)
  __ Printf("x0: %" PRId64", x1: 0x%08" PRIx64 "\n", x0, x1);
  __ Printf("d0: %f\n", d0);
  __ Printf("Test %%s: %s\n", x2);
  __ Printf("w3(uint32): %" PRIu32 "\nw4(int32): %" PRId32 "\n"
            "x5(uint64): %" PRIu64 "\nx6(int64): %" PRId64 "\n",
            w3, w4, x5, x6);
  __ Printf("%%f: %f\n%%g: %g\n%%e: %e\n%%E: %E\n", s1, s2, d3, d4);
  __ Printf("0x%08" PRIx32 ", 0x%016" PRIx64 "\n", x28, x28);
  __ Printf("%g\n", d10);

  // Test with a different stack pointer.
  const Register old_stack_pointer = __ StackPointer();
  __ mov(x29, old_stack_pointer);
  __ SetStackPointer(x29);
  __ Printf("old_stack_pointer: 0x%016" PRIx64 "\n", old_stack_pointer);
  __ mov(old_stack_pointer, __ StackPointer());
  __ SetStackPointer(old_stack_pointer);

  __ Printf("3=%u, 4=%u, 5=%u\n", x10, x11, x12);

  END();
  RUN();

  // We cannot easily test the output of the Printf sequences, and because
  // Printf preserves all registers by default, we can't look at the number of
  // bytes that were printed. However, the printf_no_preserve test should check
  // that, and here we just test that we didn't clobber any registers.
  ASSERT_EQUAL_REGISTERS(before);

  TEARDOWN();
}


TEST(Assembler, printf_no_preserve) {
  SETUP();
  START();

  char const * test_plain_string = "Printf with no arguments.\n";
  char const * test_substring = "'This is a substring.'";

  __ PrintfNoPreserve(test_plain_string);
  __ Mov(x19, x0);

  // Test simple integer arguments.
  __ Mov(x0, 1234);
  __ Mov(x1, 0x1234);
  __ PrintfNoPreserve("x0: %" PRId64", x1: 0x%08" PRIx64 "\n", x0, x1);
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
  __ PrintfNoPreserve("w3(uint32): %" PRIu32 "\nw4(int32): %" PRId32 "\n"
                      "x5(uint64): %" PRIu64 "\nx6(int64): %" PRId64 "\n",
                      w3, w4, x5, x6);
  __ Mov(x23, x0);

  __ Fmov(s1, 1.234);
  __ Fmov(s2, 2.345);
  __ Fmov(d3, 3.456);
  __ Fmov(d4, 4.567);
  __ PrintfNoPreserve("%%f: %f\n%%g: %g\n%%e: %e\n%%E: %E\n", s1, s2, d3, d4);
  __ Mov(x24, x0);

  // Test printing callee-saved registers.
  __ Mov(x28, 0x123456789abcdef);
  __ PrintfNoPreserve("0x%08" PRIx32 ", 0x%016" PRIx64 "\n", x28, x28);
  __ Mov(x25, x0);

  __ Fmov(d10, 42.0);
  __ PrintfNoPreserve("%g\n", d10);
  __ Mov(x26, x0);

  // Test with a different stack pointer.
  const Register old_stack_pointer = __ StackPointer();
  __ Mov(x29, old_stack_pointer);
  __ SetStackPointer(x29);

  __ PrintfNoPreserve("old_stack_pointer: 0x%016" PRIx64 "\n",
                      old_stack_pointer);
  __ Mov(x27, x0);

  __ Mov(old_stack_pointer, __ StackPointer());
  __ SetStackPointer(old_stack_pointer);

  // Test with three arguments.
  __ Mov(x3, 3);
  __ Mov(x4, 40);
  __ Mov(x5, 500);
  __ PrintfNoPreserve("3=%u, 4=%u, 5=%u\n", x3, x4, x5);
  __ Mov(x28, x0);

  END();
  RUN();

  // We cannot easily test the exact output of the Printf sequences, but we can
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
  // 0x89abcdef, 0x0123456789abcdef
  ASSERT_EQUAL_64(31, x25);
  // 42
  ASSERT_EQUAL_64(3, x26);
  // old_stack_pointer: 0x00007fb037ae2370
  // Note: This is an example value, but the field width is fixed here so the
  // string length is still predictable.
  ASSERT_EQUAL_64(38, x27);
  // 3=3, 4=40, 5=500
  ASSERT_EQUAL_64(17, x28);

  TEARDOWN();
}

// Functions for testing the HostCall pseudo-opcode.
extern "C" {

int64_t minusOne() {
  return -1;
}

char secondChar(const char* str) {
  return str[1];
}

int addTogether(int a, int b) {
  // Make sure the arguments are in the right order
  return (3 * a) + b;
}

} // extern "C"

TEST(Assembler, hostcall) {
  SETUP();
  START();

  __ Push (x29, x30);  // save frame pointer and return address

  // No arguments
  __ Mov  (x16, reinterpret_cast<intptr_t>(&minusOne));
  __ HostCall(0);
  __ Mov  (x19, x0);

  // One argument
  __ Mov  (x16, reinterpret_cast<intptr_t>(&secondChar));
  __ Mov  (x0, reinterpret_cast<intptr_t>("hi"));
  __ HostCall(1);
  __ Mov  (x20, x0);   // stash in a callee-saved reg, to check later

  // Two arguments
  __ Mov  (x16, reinterpret_cast<intptr_t>(&addTogether));
  __ Mov  (x0, 1030);
  __ Mov  (x1,  307);
  __ HostCall(2);
  __ Mov  (x21, x0);

  __ Pop  (x30, x29);  // restore frame pointer and return address

  END();
  RUN();

  ASSERT_EQUAL_64(minusOne(), x19);
  ASSERT_EQUAL_64(secondChar("hi"), x20);
  ASSERT_EQUAL_64(addTogether(1030, 307), x21);

  TEARDOWN();
}


#ifndef USE_SIMULATOR
TEST(Assembler, trace) {
  // The Trace helper should not generate any code unless the simulator (or
  // debugger) is being used.
  SETUP();
  START();

  Label start;
  __ Bind(&start);
  __ Trace(LOG_ALL, TRACE_ENABLE);
  __ Trace(LOG_ALL, TRACE_DISABLE);
  assert(__ SizeOfCodeGeneratedSince(&start) == 0);

  END();
  TEARDOWN();
}
#endif


#ifndef USE_SIMULATOR
TEST(Assembler, log) {
  // The Log helper should not generate any code unless the simulator (or
  // debugger) is being used.
  SETUP();
  START();

  Label start;
  __ Bind(&start);
  __ Log(LOG_ALL);
  assert(__ SizeOfCodeGeneratedSince(&start) == 0);

  END();
  TEARDOWN();
}
#endif


TEST(Assembler, instruction_accurate_scope) {
  SETUP();
  START();

  // By default macro instructions are allowed.
  assert(masm.AllowMacroInstructions());
  {
    InstructionAccurateScope scope1(&masm);
    assert(!masm.AllowMacroInstructions());
    {
      InstructionAccurateScope scope2(&masm);
      assert(!masm.AllowMacroInstructions());
    }
    assert(!masm.AllowMacroInstructions());
  }
  assert(masm.AllowMacroInstructions());

  {
    InstructionAccurateScope scope(&masm, 2);
    __ add(x0, x0, x0);
    __ sub(x0, x0, x0);
  }

  END();
  RUN();
  TEARDOWN();
}


TEST(Assembler, blr_lr) {
  // A simple test to check that the simulator correcty handle "blr lr".
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

  RUN();

  ASSERT_EQUAL_64(0xc001c0de, x0);

  TEARDOWN();
}

}  // namespace vixl
