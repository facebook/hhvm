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

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <folly/portability/GTest.h>

namespace HPHP { namespace jit {

namespace x64 { struct ImmFolder; }

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

//
// Template used to generate code fragment for binary instructions (Ins)
// with one argument being a constant.  argPos controls the position of the
// constant argument.
//
// Two variants of each instruction are generated.  One that references a
// status register that is used elsewhere and one that references an otherwise
// unused status register.  This is to check that instruction substitutions
// will not clobber any potential uses of status flags.
//
// The return value is a string representation of the code.
//
template <typename Ins, typename Arg>
std::string genTestCode(int argPos, Arg constArg) {
  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout out(unit, unit.entry);

  auto v0 = unit.makeReg();
  auto v1 = unit.makeReg();
  auto v2 = unit.makeReg();
  auto sf_used = unit.makeReg();
  auto sf = unit.makeReg();

  out << movl{unit.makeConst(42u), v0};    // initialize v0
  if (argPos == 0) {
    out << Ins{unit.makeConst(constArg), v0, v1, sf_used};
    out << Ins{unit.makeConst(constArg), v0, v2, sf};
  } else {
    out << Ins{v0, unit.makeConst(constArg), v1, sf_used};
    out << Ins{v0, unit.makeConst(constArg), v2, sf};
  }
  out << movl{sf_used, unit.makeReg()};    // force use of sf_used.
  out << ret{};

  foldImms<x64::ImmFolder>(unit);

  return stripWhitespace(show(unit));
}

}

TEST(Vasm, FoldImms) {
  //
  // add
  //
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "addqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(0, 0u) // addq 0,r
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "addqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(1, 0u) // addq r,0
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "addqi 1, %128 => %129, %131\n"
    "incq %128 => %130, %132\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(0, 1u) // addq 1,r
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "addqi 1, %128 => %129, %131\n"
    "incq %128 => %130, %132\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(1, 1u) // addq r,1
  );

  //
  // sub
  //
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "subqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<subq>(0, 0u) // subq 0,r
  );

  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "neg %128 => %129, %131\n"
    "neg %128 => %130, %132\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<subq>(1, 0u) // subq r,0
  );

  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "subqi 1, %128 => %129, %131\n"
    "decq %128 => %130, %132\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<subq>(0, 1u) // subq 1,r
  );

  //
  // xorb
  //
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorbi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(0, 0u) // xorb 0,r
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorbi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(1, 0u) // xorb r,0
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorbi -1, %128 => %129, %131\n"
    "notb %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(0, -1) // xorb -1,r
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorbi -1, %128 => %129, %131\n"
    "notb %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(1, -1) // xorb r,-1
  );

  //
  // xor
  //
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(0, 0u) // xorq 0,r
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(1, 0u) // xorq r,0
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorqi -1, %128 => %129, %131\n"
    "not %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(0, -1) // xorq -1,r
  );
  EXPECT_EQ(
    "B0 main\n"
    "movl %133(42l) => %128\n"
    "xorqi -1, %128 => %129, %131\n"
    "not %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(1, -1) // xorq r,-1
  );
}

}}
