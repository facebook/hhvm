/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "folly/Format.h"

#include "util/base.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace VM { namespace JIT {

#define EXPECT_TEQ(exp, act)                                            \
  EXPECT_EQ(exp, act)                                                   \
  << folly::format("Expected {}, got {}", (exp).toString(), (act).toString()) \

TEST(VectorEffects, Basic) {
  VectorEffects elem(SetElem, Type::PtrToArr, Type::Int, Type::Str);
  EXPECT_TEQ(Type::PtrToArr, elem.baseType);
  EXPECT_TEQ(Type::Str, elem.valType);
  EXPECT_FALSE(elem.baseTypeChanged);
  EXPECT_TRUE(elem.baseValChanged);
  EXPECT_FALSE(elem.valTypeChanged);

  VectorEffects prop(SetProp, Type::Obj, Type::StaticStr, Type::Str);
  EXPECT_TEQ(Type::Obj, prop.baseType);
  EXPECT_TEQ(Type::Str, prop.valType);
  EXPECT_FALSE(prop.baseTypeChanged);
  EXPECT_FALSE(prop.baseValChanged);
  EXPECT_FALSE(prop.valTypeChanged);
}

TEST(VectorEffects, BadArrayKey) {
  VectorEffects ve(SetElem, Type::PtrToArr, Type::Arr, Type::Int);
  EXPECT_TEQ(Type::PtrToArr, ve.baseType);
  EXPECT_TEQ(Type::InitNull, ve.valType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_TRUE(ve.baseValChanged);
  EXPECT_TRUE(ve.valTypeChanged);
}

TEST(VectorEffects, NonObjProp) {
  VectorEffects ve(SetProp, Type::PtrToInt, Type::Str, Type::Dbl);
  EXPECT_TEQ(Type::PtrToInt, ve.baseType);
  EXPECT_TEQ(Type::InitNull, ve.valType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_FALSE(ve.baseValChanged);
  EXPECT_TRUE(ve.valTypeChanged);
}

TEST(VectorEffects, NonArrElem) {
  VectorEffects ve(SetElem, Type::PtrToDbl, Type::Int, Type::Obj);
  EXPECT_TEQ(Type::PtrToDbl, ve.baseType);
  EXPECT_TEQ(Type::InitNull, ve.valType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_FALSE(ve.baseValChanged);
  EXPECT_TRUE(ve.valTypeChanged);
}

TEST(VectorEffects, PromoteNull) {
  VectorEffects elem(SetElem, Type::PtrToNull, Type::Int, Type::Str);
  EXPECT_TEQ(Type::PtrToArr, elem.baseType);
  EXPECT_TEQ(Type::Str, elem.valType);
  EXPECT_TRUE(elem.baseTypeChanged);
  EXPECT_TRUE(elem.baseValChanged);
  EXPECT_FALSE(elem.valTypeChanged);

  VectorEffects prop(SetProp, Type::PtrToUninit, Type::StaticStr, Type::Str);
  EXPECT_TEQ(Type::PtrToObj, prop.baseType);
  EXPECT_TEQ(Type::Str, prop.valType);
  EXPECT_TRUE(prop.baseTypeChanged);
  EXPECT_TRUE(prop.baseValChanged);
  EXPECT_FALSE(prop.valTypeChanged);
}

TEST(VectorEffects, UnknownBase) {
  VectorEffects ve(SetElem, Type::PtrToCell, Type::Int, Type::Obj);
  EXPECT_TEQ(Type::PtrToCell, ve.baseType);
  EXPECT_TEQ(Type::Obj|Type::InitNull, ve.valType);
  EXPECT_FALSE(ve.baseTypeChanged);
  EXPECT_TRUE(ve.baseValChanged);
  EXPECT_TRUE(ve.valTypeChanged);
}

} } }
