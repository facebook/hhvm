/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <gtest/gtest.h>

namespace HPHP { namespace jit {

namespace {

// Strip extraneous whitespace from vasm code strings.
std::string stripWhitespace(std::string str) {
  if (str.length() > 1 && str[0] == '\n' && str[1] == ' ') {
    str.erase(0, 2);
  }
  size_t spc, pos = 0;
  while((spc = str.find("  ", pos)) != std::string::npos) {
    str.erase(spc, 1);
    pos = spc;
  }
  pos = 0;
  while((spc = str.find(" \n", pos)) != std::string::npos) {
    str.erase(spc, 1);
    pos = spc;
  }
  pos = 0;
  while((spc = str.find("\n ", pos)) != std::string::npos) {
    str.erase(spc+1, 1);
    pos = spc;
  }
  return str;
}

void testSetccXor() {
  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_Z, sf, dst};
    v << xorbi{1, dst, xdst, v.makeReg()};

    simplify(unit);

    // Test that setcc/xor pair is collapsed.
    EXPECT_EQ(
      "B0 main\n"
      "movl %131(42l) => %128\n"
      "setcc NE, %128 => %130\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_Z, sf, dst};
    v << xorbi{1, dst, xdst, v.makeReg()};
    v << movl{dst, v.makeReg()};

    simplify(unit);

    // Test that setcc/xor pair is not collapsed when setcc result
    // has more than one use.
    EXPECT_EQ(
      "B0 main\n"
      "movl %131(42l) => %128\n"
      "setcc E, %128 => %129\n"
      "xorbi 1, %129 => %130, %132\n"
      "movl %129 => %133\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};
    v << xorbi{1, dst, xdst, v.makeReg()};

    simplify(unit);

    // Check that setcc/xor pair is collapsed with different condition.
    EXPECT_EQ(
      "B0 main\n"
      "movl %131(42l) => %128\n"
      "setcc E, %128 => %130\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};

    simplify(unit);

    // Make sure that setcc with no xor doesn't cause a buffer overrun.
    EXPECT_EQ(
      "B0 main\n"
      "movl %130(42l) => %128\n"
      "setcc NE, %128 => %129\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};
    v << xorbi{2, dst, xdst, v.makeReg()};

    simplify(unit);

    // Make sure that setcc/xor with an non-1 xor constant is skipped.
    EXPECT_EQ(
      "B0 main\n"
      "movl %131(42l) => %128\n"
      "setcc NE, %128 => %129\n"
      "xorbi 2, %129 => %130, %132\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    auto xsf = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};
    v << xorbi{1, dst, xdst, xsf};
    v << movl{xsf, v.makeReg()};

    simplify(unit);

    // Make sure that setcc/xor with xor status flags being used is skipped.
    EXPECT_EQ(
      "B0 main\n"
      "movl %132(42l) => %128\n"
      "setcc NE, %128 => %129\n"
      "xorbi 1, %129 => %130, %131\n"
      "movl %131 => %133\n",
      stripWhitespace(show(unit))
    );
  }
}

}

TEST(Vasm, Simplifier) {
  testSetccXor();
}

}}
