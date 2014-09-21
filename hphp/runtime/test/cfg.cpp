/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <gtest/gtest.h>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"

#include "hphp/runtime/test/match.h"
#include "hphp/runtime/test/test-context.h"

namespace HPHP { namespace jit {

TEST(CFG, InsertPreHeaders_Simple) {
  IRUnit unit{test_context};
  auto const dummy = BCMarker::Dummy();

  // Three forward edges into loop, only one back-edge.

  /*
    digraph G {
    B0 -> B1; B0 -> B2
    B1 -> B3; B1 -> B4
    B2 -> B5
    B3 -> B5
    B4 -> B5
    B5 -> B5
    }
  */

  auto b0 = unit.entry();
  auto b1 = unit.defBlock();
  auto b2 = unit.defBlock();
  auto b3 = unit.defBlock();
  auto b4 = unit.defBlock();
  auto b5 = unit.defBlock();

  auto val1 = unit.gen(Conjure, dummy, Type::Bool);
  auto val2 = unit.gen(Conjure, dummy, Type::Bool);

  b0->push_back(unit.gen(JmpZero, dummy, b1, val1->dst()));
  b0->back().setNext(b2);

  b1->push_back(unit.gen(JmpNZero, dummy, b3, val2->dst()));
  b1->back().setNext(b4);

  b2->push_back(unit.gen(Jmp, dummy, b5));
  b3->push_back(unit.gen(Jmp, dummy, b5));
  b4->push_back(unit.gen(Jmp, dummy, b5));
  b5->push_back(unit.gen(Jmp, dummy, b5));

  auto oldSize = unit.numBlocks();
  auto res = insertLoopPreHeaders(unit);
  auto newSize = unit.numBlocks();

  EXPECT_TRUE(res);
  EXPECT_EQ(newSize, oldSize + 1);

  // b6 is the new pre-header.
  auto b6 = b2->taken();

  EXPECT_NE(b6, b5);
  EXPECT_EQ(b6->taken(), b5);

  EXPECT_EQ(b3->taken(), b6);
  EXPECT_EQ(b4->taken(), b6);

  EXPECT_EQ(b5->taken(), b5);
}

TEST(CFG, InsertPreHeaders_MultiBackEdge) {
  IRUnit unit{test_context};
  auto const dummy = BCMarker::Dummy();

  // Multiple back-edges to the same block.

  /*
    digraph G {
    B0 -> B1; B0 -> B2
    B1 -> B3
    B2 -> B3
    B3 -> B3; B3 -> B4
    B4 -> B3
    }
  */

  auto b0 = unit.entry();
  auto b1 = unit.defBlock();
  auto b2 = unit.defBlock();
  auto b3 = unit.defBlock();
  auto b4 = unit.defBlock();

  auto val1 = unit.gen(Conjure, dummy, Type::Bool);
  auto val2 = unit.gen(Conjure, dummy, Type::Bool);

  b0->push_back(unit.gen(JmpZero, dummy, b1, val1->dst()));
  b0->back().setNext(b2);

  b1->push_back(unit.gen(Jmp, dummy, b3));
  b2->push_back(unit.gen(Jmp, dummy, b3));

  b3->push_back(unit.gen(JmpNZero, dummy, b3, val2->dst()));
  b3->back().setNext(b4);

  b4->push_back(unit.gen(Jmp, dummy, b3));

  auto oldSize = unit.numBlocks();
  auto res = insertLoopPreHeaders(unit);
  auto newSize = unit.numBlocks();

  EXPECT_TRUE(res);
  EXPECT_EQ(newSize, oldSize + 1);

  // b5 is the new pre-header.
  auto b5 = b1->taken();

  EXPECT_NE(b3, b4);
  EXPECT_EQ(b5->numSuccs(), 1);
  EXPECT_EQ(b2->taken(), b5);
  EXPECT_EQ(b5->taken(), b3);

  EXPECT_EQ(b3->taken(), b3);
  EXPECT_EQ(b4->taken(), b3);
}

TEST(CFG, InsertPreHeaders_NoChange) {
  IRUnit unit{test_context};
  auto const dummy = BCMarker::Dummy();

  /*
    digraph G {
    B0 -> B1
    B1 -> B2
    B2 -> B2
    }
  */

  auto b0 = unit.entry();
  auto b1 = unit.defBlock();
  auto b2 = unit.defBlock();

  b0->push_back(unit.gen(Jmp, dummy, b1));
  b1->push_back(unit.gen(Jmp, dummy, b2));
  b2->push_back(unit.gen(Jmp, dummy, b2));

  auto oldSize = unit.numBlocks();
  auto res = insertLoopPreHeaders(unit);
  auto newSize = unit.numBlocks();

  EXPECT_FALSE(res);
  EXPECT_EQ(newSize, oldSize);
}

}}
