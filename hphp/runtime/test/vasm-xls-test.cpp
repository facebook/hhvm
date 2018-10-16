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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <folly/portability/GTest.h>

namespace HPHP { namespace jit {
using namespace reg;

template<class T> uint64_t test_const(T val) {
  using testfunc = double (*)();
  static const Abi test_abi_arm = {
    .gpUnreserved = RegSet{},
    .gpReserved = arm::abi().gp(),
    .simdUnreserved = RegSet{vixl::d0},
    .simdReserved = arm::abi().simd() - RegSet{vixl::d0},
    .calleeSaved = arm::abi().calleeSaved,
    .sf = arm::abi().sf
  };
  static const Abi test_abi_x64 = {
    .gpUnreserved = RegSet{},
    .gpReserved = x64::abi().gp(),
    .simdUnreserved = RegSet{xmm0},
    .simdReserved = x64::abi().simd() - RegSet{xmm0},
    .calleeSaved = x64::abi().calleeSaved,
    .sf = x64::abi().sf
  };

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

  v << copy{v.cns(val), Vreg{xmm0}};
  v << ret{RegSet{xmm0}};

  CGMeta meta;
  if (arch() == Arch::ARM) {
    optimizeARM(vasm.unit(), test_abi_arm, true /* regalloc */);
    emitARM(unit, text, meta, nullptr);
  } else if (arch() == Arch::X64) {
    optimizeX64(vasm.unit(), test_abi_x64, true /* regalloc */);
    emitX64(unit, text, meta, nullptr);
  }
  // The above code might use meta.literalAddrs but shouldn't use anything else.
  meta.literalAddrs.clear();
  EXPECT_TRUE(meta.empty());

  union { double d; uint64_t c; } u;
  u.d = ((testfunc)code)();
  return u.c;
}

TEST(Vasm, XlsByteXmm) {
  EXPECT_EQ(test_const(false), 0);
  EXPECT_EQ(test_const(true), 1);
  // DataType is actually mapped to uint64_t constants, for some reason,
  // but if that changes we still want to test them as bytes here.
  EXPECT_EQ(test_const(KindOfUninit), 0);
  EXPECT_EQ(static_cast<DataType>(test_const(KindOfArray)), KindOfArray);
}

TEST(Vasm, XlsIntXmm) {
  EXPECT_EQ(test_const(uint32_t(1234)), 1234);
  EXPECT_EQ(test_const(uint32_t(0)), 0);
  // 32-bit constants are unsigned. make sure not sign-extended
  EXPECT_EQ(test_const(uint32_t(0xffffffff)), 0xffffffffl);
  EXPECT_EQ(test_const(uint32_t(0x80000000)), 0x80000000l);
}

}}
