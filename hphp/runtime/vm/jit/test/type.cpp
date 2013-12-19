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

#include <gtest/gtest.h>

#include "folly/ScopeGuard.h"

#include "hphp/util/base.h"
#include "hphp/runtime/vm/jit/ir.h"
// for specialized object tests to get some real VM::Class
#include "hphp/system/systemlib.h"

namespace HPHP {  namespace JIT {

namespace {

typedef hphp_hash_set<Type> TypeSet;

TypeSet allTypes() {
  TypeSet r;
# define IRT(name, ...) r.insert(Type::name);
  IR_TYPES
# undef IRT
  return r;
}

}

TEST(Type, Equality) {
  EXPECT_NE(Type::Cls, Type::PtrToBoxedObj);
}

TEST(Type, Null) {
  EXPECT_TRUE(Type::Null.isNull());
  EXPECT_TRUE(Type::Uninit.isNull());
  EXPECT_TRUE(Type::InitNull.isNull());
  EXPECT_FALSE(Type::Bool.isNull());
  EXPECT_FALSE(Type::Null.subtypeOf(Type::InitNull));
  EXPECT_NE(Type::Null, Type::Uninit);
  EXPECT_NE(Type::Null, Type::InitNull);
}

TEST(Type, KnownDataType) {
  auto trueTypes = {
    Type::Int, Type::BoxedCell, Type::StaticStr,
    Type::Str, // TODO(#3390819): this should return false...
    Type::Obj,
    Type::Dbl,
    Type::Arr,
    Type::StaticArr,
    Type::CountedArr,
    Type::Res,
    Type::Bool,
    Type::Uninit,
    Type::InitNull
  };
  for (auto t : trueTypes) {
    EXPECT_TRUE(t.isKnownDataType())
      << t.toString() << ".isKnownDataType()";
  }
  auto falseTypes = {
    // Type::Null, // TODO(#3390819)
    Type::Cell,
    Type::Gen,
    Type::Int | Type::Dbl
  };
  for (auto t : falseTypes) {
    EXPECT_FALSE(t.isKnownDataType())
      << "!" << t.toString() << ".isKnownDataType()";
  }
}

TEST(Type, ToString) {
  EXPECT_EQ("Int", Type::Int.toString());
  EXPECT_EQ("Cell", Type::Cell.toString());
  EXPECT_EQ("BoxedDbl", Type::BoxedDbl.toString());
}

TEST(Type, FromString) {
  EXPECT_EQ(Type::Int, Type::fromString("Int"));
  EXPECT_EQ(Type::None, Type::fromString("Blah"));
  EXPECT_EQ(Type::PtrToBoxedInt, Type::fromString("PtrToBoxedInt"));
}

TEST(Type, Boxes) {
  EXPECT_EQ(Type::BoxedDbl, Type::Dbl.box());
  EXPECT_TRUE(Type::BoxedDbl.isBoxed());
  EXPECT_EQ(Type::Dbl, Type::BoxedDbl.unbox());
  EXPECT_FALSE(Type::Dbl.isBoxed());
  EXPECT_EQ(Type::Cell, Type::Gen.unbox());
  EXPECT_EQ((Type::BoxedCell - Type::BoxedUninit),
            (Type::Cell - Type::Uninit).box());

  EXPECT_EQ(Type::Bottom, Type::BoxedCell & Type::PtrToGen);

  EXPECT_FALSE(Type::Func.isBoxed());
  EXPECT_FALSE(Type::Func.notBoxed());

  EXPECT_EQ(Type::Int | Type::Dbl, (Type::Int | Type::BoxedDbl).unbox());
}

TEST(Type, Ptr) {
  EXPECT_TRUE(Type::PtrToInt.isPtr());
  EXPECT_TRUE(Type::PtrToBoxedInt.isPtr());
  EXPECT_TRUE(Type::PtrToBoxedCell.isPtr());
  EXPECT_TRUE(Type::PtrToInt.subtypeOf(Type::PtrToCell));

  EXPECT_EQ(Type::PtrToInt, Type::Int.ptr());
  EXPECT_EQ(Type::PtrToCell, Type::Cell.ptr());
  EXPECT_EQ(Type::Int, Type::PtrToInt.deref());
  EXPECT_EQ(Type::BoxedCell, Type::PtrToBoxedCell.deref());
}

TEST(Type, Subtypes) {
  Type numbers = Type::Dbl | Type::Int;
  EXPECT_EQ("{Int|Dbl}", numbers.toString());
  EXPECT_TRUE(Type::Dbl.subtypeOf(numbers));
  EXPECT_TRUE(Type::Int.subtypeOf(numbers));
  EXPECT_FALSE(Type::Bool.subtypeOf(numbers));

  EXPECT_FALSE(Type::Func.subtypeOf(Type::Cell));
  EXPECT_FALSE(Type::TCA.subtypeOf(Type::Gen));

  EXPECT_TRUE(Type::PtrToCell.strictSubtypeOf(Type::PtrToGen));
}

TEST(Type, RuntimeType) {
  auto sd = StringData::MakeMalloced("", 0);
  SCOPE_EXIT { sd->destruct(); };

  HPHP::JIT::RuntimeType rt(sd);
  Type t = Type(rt);
  EXPECT_TRUE(t.subtypeOf(Type::Str));
  EXPECT_FALSE(t.subtypeOf(Type::Int));

  rt = HPHP::JIT::RuntimeType(HphpArray::GetStaticEmptyArray());
  t = Type(rt);
  EXPECT_TRUE(t.subtypeOf(Type::Arr));
  EXPECT_FALSE(t.subtypeOf(Type::Str));

  rt = HPHP::JIT::RuntimeType(true);
  t = Type(rt);
  EXPECT_TRUE(t.subtypeOf(Type::Bool));
  EXPECT_FALSE(t.subtypeOf(Type::Obj));

  rt = HPHP::JIT::RuntimeType((int64_t) 1);
  t = Type(rt);
  EXPECT_TRUE(t.subtypeOf(Type::Int));
  EXPECT_FALSE(t.subtypeOf(Type::Dbl));

  rt = HPHP::JIT::RuntimeType(DataType::KindOfObject,
                                 DataType::KindOfInvalid);
  rt = rt.setKnownClass(SystemLib::s_TraversableClass);
  t = Type(rt);
  EXPECT_TRUE(t.subtypeOf(Type::Obj));
  EXPECT_FALSE(Type::Obj.subtypeOf(t));
  EXPECT_FALSE(Type::Int.subtypeOf(t));
  HPHP::JIT::RuntimeType rt1 =
    HPHP::JIT::RuntimeType(DataType::KindOfObject,
                              DataType::KindOfInvalid);
  rt1 = rt1.setKnownClass(SystemLib::s_IteratorClass);
  Type t1 = Type(rt1);
  EXPECT_TRUE(t1.subtypeOf(Type::Obj));
  EXPECT_TRUE(t1.subtypeOf(t));
  EXPECT_FALSE(Type::Obj.subtypeOf(t1));
  EXPECT_FALSE(t.subtypeOf(t1));
  EXPECT_FALSE(t.subtypeOf(Type::Str));
  EXPECT_FALSE(Type::Int.subtypeOf(t));
}

TEST(Type, CanRunDtor) {
  TypeSet types = allTypes();
  auto expectTrue = [&](Type t) {
    EXPECT_TRUE(t.canRunDtor()) << t.toString() << ".canRunDtor() == true";
    types.erase(t);
  };
  expectTrue(Type::Arr);
  expectTrue(Type::CountedArr);
  expectTrue(Type::Obj);
  expectTrue(Type::Res);
  expectTrue(Type::Counted);
  expectTrue(Type::BoxedArr);
  expectTrue(Type::BoxedCountedArr);
  expectTrue(Type::BoxedObj);
  expectTrue(Type::BoxedRes);
  expectTrue(Type::BoxedInitCell);
  expectTrue(Type::BoxedCell);
  expectTrue(Type::InitCell);
  expectTrue(Type::Cell);
  expectTrue(Type::Gen);
  expectTrue(Type::Ctx);
  expectTrue(Type::Obj | Type::Func);
  expectTrue(Type::Init);
  expectTrue(Type::Top);
  expectTrue(Type::StackElem);
  expectTrue(Type::AnyObj);
  expectTrue(Type::AnyRes);
  expectTrue(Type::AnyArr);
  expectTrue(Type::AnyCountedArr);
  expectTrue(Type::AnyInitCell);
  expectTrue(Type::AnyCell);

  for (Type t : types) {
    EXPECT_FALSE(t.canRunDtor()) << t.toString() << ".canRunDtor == false";
  }
}

TEST(Type, UnionOf) {
  EXPECT_EQ(Type::PtrToGen, Type::unionOf(Type::PtrToCell, Type::PtrToGen));
  EXPECT_EQ(Type::UncountedInit, Type::unionOf(Type::Int, Type::Dbl));
  EXPECT_EQ(Type::Str, Type::unionOf(Type::StaticStr, Type::Str));
  EXPECT_EQ(Type::Gen, Type::unionOf(Type::Cell, Type::BoxedInt));
}

TEST(Type, Top) {
  for (auto t : allTypes()) {
    EXPECT_TRUE(t.subtypeOf(Type::Top));
  }
  for (auto t : allTypes()) {
    if (t.equals(Type::Top)) continue;
    EXPECT_FALSE(Type::Top.subtypeOf(t));
  }
}

} }
