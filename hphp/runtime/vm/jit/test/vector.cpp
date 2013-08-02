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

#include "gtest/gtest.h"

#include "folly/Format.h"

#include "hphp/util/base.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP {  namespace JIT {

#define EXPECT_TEQ(exp, act)                                            \
  EXPECT_EQ(exp, act)                                                   \
  << folly::format("Expected {}, got {}", (exp).toString(), (act).toString()) \

TEST(VectorEffects, Basic) {
  VectorEffects elem(SetElem, Type::PtrToArr);
  EXPECT_TEQ(Type::PtrToArr, elem.baseType);
  EXPECT_FALSE(elem.baseTypeChanged);
  EXPECT_TRUE(elem.baseValChanged);

  VectorEffects prop(SetProp, Type::Obj);
  EXPECT_TEQ(Type::Obj, prop.baseType);
  EXPECT_FALSE(prop.baseTypeChanged);
  EXPECT_FALSE(prop.baseValChanged);
}

TEST(VectorEffects, BadArrayKey) {
  VectorEffects ve(SetElem, Type::PtrToArr);
  EXPECT_TEQ(Type::PtrToArr, ve.baseType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_TRUE(ve.baseValChanged);
}

TEST(VectorEffects, NonObjProp) {
  VectorEffects ve(SetProp, Type::PtrToInt);
  EXPECT_TEQ(Type::PtrToInt, ve.baseType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_FALSE(ve.baseValChanged);
}

TEST(VectorEffects, NonArrElem) {
  VectorEffects ve(SetElem, Type::PtrToDbl);
  EXPECT_TEQ(Type::PtrToDbl, ve.baseType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_FALSE(ve.baseValChanged);
}

TEST(VectorEffects, PromoteNull) {
  VectorEffects elem(SetElem, Type::PtrToNull);
  EXPECT_TEQ(Type::PtrToArr, elem.baseType);
  EXPECT_TRUE(elem.baseTypeChanged);
  EXPECT_TRUE(elem.baseValChanged);

  VectorEffects prop(SetProp, Type::PtrToUninit);
  EXPECT_TEQ(Type::PtrToObj, prop.baseType);
  EXPECT_TRUE(prop.baseTypeChanged);
  EXPECT_TRUE(prop.baseValChanged);
}

TEST(VectorEffects, UnknownBase) {
  VectorEffects ve(SetElem, Type::PtrToCell);
  EXPECT_TEQ(Type::PtrToCell - Type::PtrToNull, ve.baseType);
  EXPECT_TRUE(ve.baseTypeChanged);
  EXPECT_TRUE(ve.baseValChanged);
}

} }
