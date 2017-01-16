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
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <gtest/gtest.h>

namespace HPHP { namespace jit {
using namespace reg;

template<class T> uint64_t test_const(T val) {
  using testfunc = double (*)();
  static const Abi test_abi = {
    .gpUnreserved = RegSet{},
    .gpReserved = x64::abi().gp(),
    .simdUnreserved = RegSet{xmm0},
    .simdReserved = x64::abi().simd() - RegSet{xmm0},
    .calleeSaved = x64::abi().calleeSaved,
    .sf = x64::abi().sf
  };
  static uint8_t code[1000];
  // None of these tests should use much data.
  static uint8_t data_buffer[100];

  CodeBlock main;
  main.init(code, sizeof(code), "test");
  DataBlock data;
  data.init(data_buffer, sizeof(data_buffer), "data");

  Vunit unit;
  Vasm vasm{unit};
  Vtext text { main, data };

  auto& v = vasm.main();
  unit.entry = v;

  v << copy{v.cns(val), Vreg{xmm0}};
  v << ret{RegSet{xmm0}};

  optimizeX64(vasm.unit(), test_abi, true /* regalloc */);
  CGMeta fixups;

  emitX64(unit, text, fixups, nullptr);
  // The above code might use fixups.literals but shouldn't use anything else.
  fixups.literals.clear();
  EXPECT_TRUE(fixups.empty());

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
  EXPECT_EQ(test_const(KindOfArray), KindOfArray);
}

TEST(Vasm, XlsIntXmm) {
  EXPECT_EQ(test_const(uint32_t(1234)), 1234);
  EXPECT_EQ(test_const(uint32_t(0)), 0);
  // 32-bit constants are unsigned. make sure not sign-extended
  EXPECT_EQ(test_const(uint32_t(0xffffffff)), 0xffffffffl);
  EXPECT_EQ(test_const(uint32_t(0x80000000)), 0x80000000l);
}

}}
