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

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

#include <gtest/gtest.h>

namespace HPHP { namespace jit {

namespace {
template<typename T, typename Lcodegen, typename Ltest>
void test_function(Lcodegen lcodegen, Ltest ltest) {
  auto blockSize = 4096;
  auto code = static_cast<uint8_t*>(mmap(nullptr, blockSize,
                                         PROT_READ | PROT_WRITE | PROT_EXEC,
                                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  SCOPE_EXIT { munmap(code, blockSize); };

  auto const dataSize = 100;
  auto const codeSize = blockSize - dataSize;
  // None of these tests should use much data.
  auto data_buffer = code + codeSize;

  CodeBlock main;
  main.init(code, codeSize, "test");
  DataBlock data;
  data.init(data_buffer, dataSize, "data");

  Vunit unit;
  Vasm vasm{unit};
  Vtext text { main, data };

  auto& v = vasm.main();
  unit.entry = v;

  lcodegen(v);

  CGMeta meta;
  if (arch() == Arch::ARM) {
    emitARM(unit, text, meta, nullptr);
  } else if (arch() == Arch::X64) {
    emitX64(unit, text, meta, nullptr);
  }

  ltest((T)code);
}
}

TEST(Vasm, Move128ARM) {
#if !defined(__powerpc64__)
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
#endif
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

} }
