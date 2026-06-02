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

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#ifdef __aarch64__
#include "hphp/runtime/vm/jit/vasm-util-arm.h"
#endif

#include "hphp/util/arch.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/data-block.h"
#include "hphp/vixl/hphp-compat.h"

#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

#include <gtest/gtest.h>

namespace HPHP { namespace jit {

namespace {
constexpr size_t kDefaultBlockSize = 4096;
constexpr size_t kDataSize = 100;

template<typename Lcodegen, typename Ltest>
void test_vasm(size_t blockSize,
               Lcodegen lcodegen,
               Ltest ltest,
               bool forceFarLiteral = false) {
  auto code = static_cast<uint8_t*>(mmap(nullptr, blockSize,
                                         PROT_READ | PROT_WRITE | PROT_EXEC,
                                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  SCOPE_EXIT { munmap(code, blockSize); };

  auto const codeSize = blockSize - kDataSize;
  // None of these tests should use much data.
  auto data_buffer = code + codeSize;

  CodeBlock main;
  main.init(code, codeSize, "test");
  DataBlock data;
  data.init(data_buffer, kDataSize, "data");

  Vunit unit;
  if (forceFarLiteral) unit.enableFarLiteral();
  Vasm vasm{unit};
  Vtext text { main, data };

  auto& v = vasm.main();
  unit.entry = v;

  lcodegen(v);

  CGMeta meta;
  emit(unit, text, meta, nullptr);

  ltest(code, main, meta);
}

template<typename T, typename Lcodegen, typename Ltest>
void test_function(Lcodegen lcodegen, Ltest ltest) {
  test_vasm(kDefaultBlockSize, lcodegen, [&] (uint8_t* code,
                                              CodeBlock&,
                                              const CGMeta&) {
    ltest((T)code);
  });
}

template<typename Lcodegen, typename Ltest>
void test_emission(size_t blockSize,
                   Lcodegen lcodegen,
                   Ltest ltest,
                   bool forceFarLiteral = false) {
  test_vasm(blockSize, lcodegen, [&] (uint8_t*,
                                      CodeBlock& main,
                                      const CGMeta& meta) {
    ltest(main, meta);
  }, forceFarLiteral);
}
}

TEST(Vasm, Move128ARM) {
  // A long double will use simd 128 bit registers for arguments and return
  // types on ARM.  __m128 will do the same on x86.
  // This code checks that we actually copy a full 128 bit value when copying
  // simd registers.
#if defined(__aarch64__)
  using xmmType = long double;
#else
  using xmmType = __m128;
#endif
  using testfunc = xmmType (*)(xmmType, xmmType);
  using namespace reg;

  auto const l = [](Vout& v) {
    v << copy{Vreg{xmm1}, Vreg{xmm0}};
    v << ret{RegSet{xmm0}};
  };

  test_function<testfunc>(l, [](testfunc f) {
    union { xmmType x; struct { uint64_t t; uint64_t v; }; } a0, a1, r;

    a0.t = 0x88a6b5e2c10d3f9f; a0.v = 0x98713e0ad31c6b55;
    a1.t = 0x256b36bd8ef79236; a1.v = 0x63f4b85d224d9dfb;
    r.t = 0; r.v = 0;

    r.x = f(a0.x, a1.x);

    EXPECT_EQ(r.t, 0x256b36bd8ef79236);
    EXPECT_EQ(r.v, 0x63f4b85d224d9dfb);
  });
}

TEST(Vasm, PrintVptr) {
  auto v0 = Vreg{Vreg::V0};
  auto v1 = Vreg{Vreg::V0 + 1};
  Vptr p{Vreg{v0}, Vreg{}, 1, 0};
  EXPECT_EQ("[%128]", show(p));
  p.index = v1;
  EXPECT_EQ("[%128 + %129]", show(p));
  p.scale = 4;
  EXPECT_EQ("[%128 + %129 * 4]", show(p));
  p.disp = 0xbeef;
  EXPECT_EQ("[%128 + 0xbeef + %129 * 4]", show(p));
  p.disp = -16;
  p.index = Vreg{};
  EXPECT_EQ("[%128 - 0x10]", show(p));
  p.seg = Segment::FS;
  EXPECT_EQ("[%fs + %128 - 0x10]", show(p));
  p.base = Vreg{};
  EXPECT_EQ("[%fs - 0x10]", show(p));
}

#ifdef __aarch64__

TEST(Vasm, ArmNearLiteralLoadStaysLiteralLdr) {
  test_emission(4096, [] (Vout& v) {
    v << leap{reg::rip[(intptr_t)0x12340000], rarg(0)};
    v << ret{RegSet{}};
  }, [] (CodeBlock& main, const CGMeta&) {
    auto const start = vixl::Instruction::Cast(main.base());
    auto const load = jit::arm::LoadLiteral::at(start);

    ASSERT_TRUE(load);
    EXPECT_FALSE(load.isFar());

    EXPECT_EQ(start->Mask(vixl::LoadLiteralMask), vixl::LDR_w_lit);
  });
}

TEST(Vasm, ArmFarLiteralLoadPatchesToAdrpLdr) {
  constexpr size_t kBlockSize = 2 * 1024 * 1024;
  constexpr size_t kFillerNops = (1024 * 1024) / vixl::kInstructionSize + 1024;

  test_emission(kBlockSize, [] (Vout& v) {
    v << leap{reg::rip[(intptr_t)0x12340000], rarg(0)};
    for (size_t i = 0; i < kFillerNops; ++i) {
      v << nop{};
    }
    v << ret{RegSet{}};
  }, [] (CodeBlock& main, const CGMeta&) {
    auto const start = vixl::Instruction::Cast(main.base());
    auto const load = jit::arm::LoadLiteral::at(start);

    ASSERT_TRUE(load && load.isFar());
    auto const ldr = load.ldr();
    EXPECT_EQ(ldr->Mask(vixl::LoadStoreUnsignedOffsetMask), vixl::LDR_w_unsigned);
    EXPECT_EQ(start->Rd(), ldr->Rn());
    EXPECT_EQ(start->Rd(), ldr->Rt());
  },
  // Direct code emission no longer retries on far literals. Force the
  // ADRP form explicitly so this test still exercises the far-literal rewrite.
  true);
}

TEST(Vasm, ArmMcprepSeedsTaggedMovqLiteral) {
  test_emission(4096, [] (Vout& v) {
    v << mcprep{rarg(0)};
    v << ret{RegSet{}};
  }, [] (CodeBlock& main, const CGMeta&) {
    auto const start = vixl::Instruction::Cast(main.base());
    auto const load = jit::arm::LoadLiteral::at(start);
    ASSERT_TRUE(load);
    auto const literal =
      *reinterpret_cast<uint64_t*>(load.literalAddress());
    EXPECT_EQ(literal, (reinterpret_cast<uint64_t>(main.base()) << 1) | 1);
  });
}

#endif

} }
