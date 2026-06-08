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

namespace HPHP::jit {

#ifdef __aarch64__
namespace arm { struct ImmFolder; }
#endif

#ifdef __x86_64__
namespace x64 { struct ImmFolder; }
#endif

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
#ifndef __aarch64__
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
#endif

} // namespace

TEST(Vasm, FoldImms) {
#ifndef __x86_64__
  GTEST_SKIP() << "x86-specific foldImms test";
#else
  //
  // add
  //
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "addqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(0, 0u) // addq 0,r
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "addqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(1, 0u) // addq r,0
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "addqi 1, %128 => %129, %131\n"
    "incq %128 => %130, %132\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<addq>(0, 1u) // addq 1,r
  );
  EXPECT_EQ(
    "B0 main (1)\n"
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
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "subqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<subq>(0, 0u) // subq 0,r
  );

  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "neg %128 => %129, %131\n"
    "neg %128 => %130, %132\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<subq>(1, 0u) // subq r,0
  );

  EXPECT_EQ(
    "B0 main (1)\n"
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
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorbi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(0, 0u) // xorb 0,r
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorbi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(1, 0u) // xorb r,0
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorbi -1, %128 => %129, %131\n"
    "notb %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorb>(0, -1) // xorb -1,r
  );
  EXPECT_EQ(
    "B0 main (1)\n"
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
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(0, 0u) // xorq 0,r
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorqi 0, %128 => %129, %131\n"
    "copy %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(1, 0u) // xorq r,0
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorqi -1, %128 => %129, %131\n"
    "not_ %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(0, -1) // xorq -1,r
  );
  EXPECT_EQ(
    "B0 main (1)\n"
    "movl %133(42l) => %128\n"
    "xorqi -1, %128 => %129, %131\n"
    "not_ %128 => %130\n"
    "movl %131 => %135\n"
    "ret {}\n",
    genTestCode<xorq>(1, -1) // xorq r,-1
  );
#endif
}

#ifdef __aarch64__
namespace arm { struct ImmFolder; }

namespace {

template <typename EmitFn>
Vunit genArmFoldCode(uint64_t idxVal, EmitFn emit) {
  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout out(unit, unit.entry);
  auto base = unit.makeReg();
  auto idx = unit.makeReg();
  out << ldimmq{Immed64{1}, base};
  out << ldimmq{Immed64{idxVal}, idx};
  emit(unit, out, base, idx);
  out << ret{};
  foldImms<arm::ImmFolder>(unit);
  return unit;
}

} // namespace

TEST(Vasm, FoldImmsArmLimitsSignedZeroToCmpsd) {
  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const base = v.makeReg();
  auto const positiveZero = unit.makeConst(0.0);
  auto const negativeZero = unit.makeConst(-0.0);
  auto const nonZero = unit.makeConst(1.0);
  auto const positiveCmp = VregDbl{v.makeReg()};
  auto const negativeCmp = VregDbl{v.makeReg()};
  auto const neqCmp = VregDbl{v.makeReg()};

  v << ldimmq{0x1000, base};
  v << store{positiveZero, base[0]};
  v << store{negativeZero, base[8]};
  v << cmpsd{
    ComparisonPred::eq_ord,
    VregDbl{nonZero},
    VregDbl{positiveZero},
    positiveCmp
  };
  v << cmpsd{
    ComparisonPred::eq_ord,
    VregDbl{negativeZero},
    VregDbl{nonZero},
    negativeCmp
  };
  v << cmpsd{
    ComparisonPred::ne_unord,
    VregDbl{nonZero},
    VregDbl{positiveZero},
    neqCmp
  };
  v << ret{};

  foldImms<arm::ImmFolder>(unit);

  auto const& code = unit.blocks[unit.entry].code;
  ASSERT_EQ(code.size(), 7);
  ASSERT_EQ(code[1].op, Vinstr::store);
  ASSERT_EQ(code[2].op, Vinstr::store);
  ASSERT_EQ(code[3].op, Vinstr::cmpsdz);
  ASSERT_EQ(code[4].op, Vinstr::cmpsdz);
  ASSERT_EQ(code[5].op, Vinstr::cmpsdz);
  EXPECT_EQ(code[1].store_.s, Vreg64{PhysReg(vixl::xzr)}) << show(unit);
  EXPECT_EQ(code[2].store_.s, negativeZero) << show(unit);
  EXPECT_EQ(code[3].cmpsdz_.pred, ComparisonPred::eq_ord) << show(unit);
  EXPECT_EQ(code[3].cmpsdz_.s, VregDbl{nonZero}) << show(unit);
  EXPECT_EQ(code[4].cmpsdz_.pred, ComparisonPred::eq_ord) << show(unit);
  EXPECT_EQ(code[4].cmpsdz_.s, VregDbl{nonZero}) << show(unit);
  EXPECT_EQ(code[5].cmpsdz_.pred, ComparisonPred::ne_unord) << show(unit);
  EXPECT_EQ(code[5].cmpsdz_.s, VregDbl{nonZero}) << show(unit);
}

TEST(Vasm, FoldImmsArmLimitsSignedZeroToUcomisd) {
  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const base = v.makeReg();
  auto const positiveZero = unit.makeConst(0.0);
  auto const negativeZero = unit.makeConst(-0.0);
  auto const nonZero = unit.makeConst(1.0);
  auto const positiveSf = VregSF{v.makeReg()};
  auto const negativeSf = VregSF{v.makeReg()};
  auto const reverseSf = VregSF{v.makeReg()};

  v << ldimmq{0x1000, base};
  v << store{positiveZero, base[0]};
  v << store{negativeZero, base[8]};
  v << ucomisd{VregDbl{nonZero}, VregDbl{positiveZero}, positiveSf};
  v << ucomisd{VregDbl{nonZero}, VregDbl{negativeZero}, negativeSf};
  v << ucomisd{VregDbl{positiveZero}, VregDbl{nonZero}, reverseSf};
  v << ret{};

  foldImms<arm::ImmFolder>(unit);

  auto const& code = unit.blocks[unit.entry].code;
  ASSERT_EQ(code.size(), 7);
  ASSERT_EQ(code[1].op, Vinstr::store);
  ASSERT_EQ(code[2].op, Vinstr::store);
  ASSERT_EQ(code[3].op, Vinstr::ucomisdz);
  ASSERT_EQ(code[4].op, Vinstr::ucomisdz);
  ASSERT_EQ(code[5].op, Vinstr::ucomisd);
  EXPECT_EQ(code[1].store_.s, Vreg64{PhysReg(vixl::xzr)}) << show(unit);
  EXPECT_EQ(code[2].store_.s, negativeZero) << show(unit);
  EXPECT_EQ(code[3].ucomisdz_.s, VregDbl{nonZero}) << show(unit);
  EXPECT_EQ(code[4].ucomisdz_.s, VregDbl{nonZero}) << show(unit);
  EXPECT_EQ(code[5].ucomisd_.s0, VregDbl{positiveZero}) << show(unit);
  EXPECT_EQ(code[5].ucomisd_.s1, VregDbl{nonZero}) << show(unit);
}
#endif

TEST(Vasm, FoldImmsArmMemIndex) {
#ifndef __aarch64__
  GTEST_SKIP() << "ARM-specific fold_mem_index test";
#else
  // ldimmq constant index folded into load displacement
  {
    auto unit = genArmFoldCode(100,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << load{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::load, code[2].op);
    EXPECT_FALSE(code[2].load_.s.index.isValid());
    EXPECT_EQ(100, code[2].load_.s.disp);
    EXPECT_EQ(1, code[2].load_.s.scale);
  }

  // constToReg constant index folded into load displacement
  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout out(unit, unit.entry);
    auto base = unit.makeReg();
    auto dst = unit.makeReg();
    auto cst = unit.makeConst(uint64_t{200});
    out << ldimmq{Immed64{1}, base};
    out << load{Vptr{base, Vreg{cst}, 1, 0}, dst};
    out << ret{};
    foldImms<arm::ImmFolder>(unit);
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(3, code.size());
    EXPECT_EQ(Vinstr::load, code[1].op);
    EXPECT_FALSE(code[1].load_.s.index.isValid());
    EXPECT_EQ(200, code[1].load_.s.disp);
    EXPECT_EQ(1, code[1].load_.s.scale);
  }

  // Scaled index with existing displacement: 10 * 4 + 16 = 56
  {
    auto unit = genArmFoldCode(10,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << load{Vptr{base, idx, 4, 16}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::load, code[2].op);
    EXPECT_FALSE(code[2].load_.s.index.isValid());
    EXPECT_EQ(56, code[2].load_.s.disp);
    EXPECT_EQ(1, code[2].load_.s.scale);
  }

  // Store: constant index folded into displacement
  {
    auto unit = genArmFoldCode(100,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      auto src = u.makeReg();
      out << ldimmq{Immed64{1}, src};
      out << store{src, Vptr{base, idx, 1, 0}};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(5, code.size());
    EXPECT_EQ(Vinstr::store, code[3].op);
    EXPECT_FALSE(code[3].store_.d.index.isValid());
    EXPECT_EQ(100, code[3].store_.d.disp);
    EXPECT_EQ(1, code[3].store_.d.scale);
  }

  // Lea: constant index folded into displacement
  {
    auto unit = genArmFoldCode(100,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << lea{Vptr{base, idx, 1, 0}, Vreg64{u.makeReg()}};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::lea, code[2].op);
    EXPECT_FALSE(code[2].lea_.s.index.isValid());
    EXPECT_EQ(100, code[2].lea_.s.disp);
    EXPECT_EQ(1, code[2].lea_.s.scale);
  }

  // Boundary: max scaled quad load (32760 = 4095 * 8) folds
  {
    auto unit = genArmFoldCode(32760,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << load{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::load, code[2].op);
    EXPECT_FALSE(code[2].load_.s.index.isValid());
    EXPECT_EQ(32760, code[2].load_.s.disp);
  }

  // No fold: quad load displacement exceeds ARM64 scaled range
  {
    auto unit = genArmFoldCode(32768,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << load{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::load, code[2].op);
    EXPECT_TRUE(code[2].load_.s.index.isValid());
  }

  // Byte load: max scaled range is 4095
  {
    auto unit = genArmFoldCode(4095,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << loadb{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::loadb, code[2].op);
    EXPECT_FALSE(code[2].loadb_.s.index.isValid());
    EXPECT_EQ(4095, code[2].loadb_.s.disp);
  }

  // No fold: byte load displacement exceeds byte scaled range
  {
    auto unit = genArmFoldCode(4096,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << loadb{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::loadb, code[2].op);
    EXPECT_TRUE(code[2].loadb_.s.index.isValid());
  }

  // Small displacement folds for byte load (fits unscaled range)
  {
    auto unit = genArmFoldCode(100,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << loadb{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::loadb, code[2].op);
    EXPECT_FALSE(code[2].loadb_.s.index.isValid());
    EXPECT_EQ(100, code[2].loadb_.s.disp);
  }

  // No fold: quad load with unaligned displacement outside unscaled range
  {
    auto unit = genArmFoldCode(257,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << load{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::load, code[2].op);
    EXPECT_TRUE(code[2].load_.s.index.isValid());
  }

  // No fold: constant exceeds INT32_MAX
  {
    auto unit = genArmFoldCode(uint64_t{INT32_MAX} + 1,
        [](Vunit& u, Vout& out, Vreg base, Vreg idx) {
      out << load{Vptr{base, idx, 1, 0}, u.makeReg()};
    });
    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(4, code.size());
    EXPECT_EQ(Vinstr::load, code[2].op);
    EXPECT_TRUE(code[2].load_.s.index.isValid());
    EXPECT_EQ(0, code[2].load_.s.disp);
  }
#endif
}

}
