/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"

#include <gtest/gtest.h>

namespace HPHP { namespace JIT {

TEST(RegAlgorithms, twoCycle) {
  PhysReg::Map<PhysReg> moves;

  using namespace reg;
  moves[rax] = rdx;
  moves[rdx] = rax;

  auto howTo = JIT::doRegMoves(moves, r10);
  EXPECT_EQ(howTo.size(), 1);
  EXPECT_EQ(howTo[0].m_kind, JIT::MoveInfo::Kind::Xchg);
  EXPECT_TRUE((howTo[0].m_reg1 == rax && howTo[0].m_reg2 == rdx) ||
              (howTo[0].m_reg1 == rdx && howTo[0].m_reg2 == rax));
}

TEST(RegAlgorithms, threeCycle) {
  PhysReg::Map<PhysReg> moves;

  using namespace reg;
  moves[rax] = rdx;
  moves[rdx] = rcx;
  moves[rcx] = rax;

  auto howTo = JIT::doRegMoves(moves, r10);
  EXPECT_EQ(howTo.size(), 2);
  EXPECT_EQ(howTo[0].m_kind, JIT::MoveInfo::Kind::Xchg);
  EXPECT_TRUE((howTo[0].m_reg1 == rax && howTo[0].m_reg2 == rdx) ||
              (howTo[0].m_reg1 == rdx && howTo[0].m_reg2 == rax));
  EXPECT_TRUE((howTo[1].m_reg1 == rdx && howTo[1].m_reg2 == rcx) ||
              (howTo[1].m_reg1 == rcx && howTo[1].m_reg2 == rdx));
}

TEST(RegAlgorithms, noCycle) {
  PhysReg::Map<PhysReg> moves;

  using namespace reg;
  moves[rax] = rdx;
  moves[rdx] = r13;
  moves[r13] = r10;

  auto howTo = JIT::doRegMoves(moves, r11);
  EXPECT_EQ(howTo.size(), 3);
  EXPECT_EQ(howTo[0].m_kind, JIT::MoveInfo::Kind::Move);
  EXPECT_EQ(howTo[0].m_reg1, rdx);
  EXPECT_EQ(howTo[0].m_reg2, rax);
  EXPECT_EQ(howTo[1].m_kind, JIT::MoveInfo::Kind::Move);
  EXPECT_EQ(howTo[1].m_reg1, r13);
  EXPECT_EQ(howTo[1].m_reg2, rdx);
  EXPECT_EQ(howTo[2].m_kind, JIT::MoveInfo::Kind::Move);
  EXPECT_EQ(howTo[2].m_reg1, r10);
  EXPECT_EQ(howTo[2].m_reg2, r13);
}

TEST(RegAlgorithms, outdegTwo) {
  PhysReg::Map<PhysReg> moves;

  using namespace reg;
  moves[rax] = rcx;
  moves[rdx] = rcx;

  auto howTo = JIT::doRegMoves(moves, r11);
  EXPECT_EQ(howTo.size(), 2);
  EXPECT_EQ(howTo[0].m_reg1, rcx);
  EXPECT_EQ(howTo[1].m_reg1, rcx);
  EXPECT_TRUE((howTo[0].m_reg2 == rax && howTo[1].m_reg2 == rdx) ||
              (howTo[0].m_reg2 == rdx && howTo[1].m_reg2 == rax));
}

TEST(RegAlgorithms, simdCycle) {
  PhysReg::Map<PhysReg> moves;

  using namespace reg;
  moves[rax] = xmm0;
  moves[xmm0] = rax;

  auto howTo = JIT::doRegMoves(moves, r11);
  EXPECT_EQ(howTo.size(), 3);
  EXPECT_EQ(howTo[0].m_kind, JIT::MoveInfo::Kind::Move);
  EXPECT_EQ(howTo[0].m_reg1, rax);
  EXPECT_EQ(howTo[0].m_reg2, r11);
  EXPECT_EQ(howTo[1].m_kind, JIT::MoveInfo::Kind::Move);
  EXPECT_EQ(howTo[1].m_reg1, xmm0);
  EXPECT_EQ(howTo[1].m_reg2, rax);
  EXPECT_EQ(howTo[2].m_kind, JIT::MoveInfo::Kind::Move);
  EXPECT_EQ(howTo[2].m_reg1, r11);
  EXPECT_EQ(howTo[2].m_reg2, xmm0);
}

} }
