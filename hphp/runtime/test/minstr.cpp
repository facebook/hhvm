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

#include <gtest/gtest.h>

#include <folly/Format.h>

#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

namespace HPHP { namespace jit {

#define EXPECT_TEQ(exp, act)                                            \
  EXPECT_EQ(exp, act)                                                   \
  << folly::format("Expected {}, got {}", (exp).toString(), (act).toString()) \

TEST(MInstrEffects, Basic) {
  MInstrEffects elem(SetElem, TDict);
  EXPECT_TEQ(TDict, elem.baseType);
  EXPECT_FALSE(elem.baseTypeChanged);
  EXPECT_TRUE(elem.baseValChanged);

  MInstrEffects prop(SetProp, TObj);
  EXPECT_TEQ(TObj, prop.baseType);
  EXPECT_FALSE(prop.baseTypeChanged);
  EXPECT_FALSE(prop.baseValChanged);
}

TEST(MInstrEffects, AddCounted) {
  MInstrEffects elem(SetElem, TStaticDict);
  EXPECT_TEQ(TCountedDict|TStaticDict, elem.baseType);
  EXPECT_TRUE(elem.baseTypeChanged);
  EXPECT_TRUE(elem.baseValChanged);
}

TEST(MInstrEffects, NonObjProp) {
  MInstrEffects effects(SetProp, TInt);
  EXPECT_TEQ(TInt, effects.baseType);
  EXPECT_FALSE(effects.baseTypeChanged);
  EXPECT_FALSE(effects.baseValChanged);
}

TEST(MInstrEffects, NonArrElem) {
  MInstrEffects effects(SetElem, TDbl);
  EXPECT_TEQ(TDbl, effects.baseType);
  EXPECT_FALSE(effects.baseTypeChanged);
  EXPECT_FALSE(effects.baseValChanged);
}

TEST(MInstrEffects, PromoteNull) {
  MInstrEffects elem(SetElem, TNull);
  EXPECT_TEQ(TBottom, elem.baseType);
  EXPECT_TRUE(elem.baseTypeChanged);
  EXPECT_FALSE(elem.baseValChanged);

  MInstrEffects prop(SetProp, TUninit);
  EXPECT_TEQ(TBottom, prop.baseType);
  EXPECT_TRUE(prop.baseTypeChanged);
  EXPECT_FALSE(prop.baseValChanged);
}

TEST(MInstrEffects, UnknownBase) {
  MInstrEffects effects(SetElem, TCell);
  EXPECT_TEQ(TCell - TNull, effects.baseType);
  EXPECT_TRUE(effects.baseTypeChanged);
  EXPECT_TRUE(effects.baseValChanged);
}

} }
