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

#ifdef __aarch64__

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <folly/portability/GTest.h>

namespace HPHP::jit {

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

TEST(Vasm, FoldImmsArmMemIndex) {
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
}

}

#endif
