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

#include "util/base.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace std { namespace tr1 {
  template<> struct hash<HPHP::VM::JIT::Type> {
    size_t operator()(HPHP::VM::JIT::Type t) const { return t.hash(); }
  };
} }

namespace HPHP { namespace VM { namespace JIT {

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

TEST(Type, StaticallyKnown) {
  auto trueTypes = {
    Type::Int, Type::BoxedCell, Type::StaticStr, Type::Str,
    Type::Null, Type::Uninit, Type::InitNull
  };
  for (auto t : trueTypes) {
    EXPECT_TRUE(t.isStaticallyKnown())
      << t.toString() << ".isStaticallyKnown()";
  }
  auto falseTypes = {
    Type::FuncCtx, Type::Cell, Type::Gen, Type::Ctx,
    Type::Int | Type::Dbl
  };
  for (auto t : falseTypes) {
    EXPECT_FALSE(t.isStaticallyKnown())
      << "!" << t.toString() << ".isStaticallyKnown()";
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
  EXPECT_EQ(Type::BoxedCell, Type::Cell.box());

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

TEST(Type, CanRunDtor) {
  TypeSet types = allTypes();
  auto expectTrue = [&](Type t) {
    EXPECT_TRUE(t.canRunDtor()) << t.toString() << ".canRunDtor() == true";
    types.erase(t);
  };
  expectTrue(Type::Arr);
  expectTrue(Type::Obj);
  expectTrue(Type::BoxedArr);
  expectTrue(Type::BoxedObj);
  expectTrue(Type::BoxedCell);
  expectTrue(Type::Cell);
  expectTrue(Type::Gen);
  expectTrue(Type::Ctx);
  expectTrue(Type::Obj | Type::Func);

  for (Type t : types) {
    EXPECT_FALSE(t.canRunDtor()) << t.toString() << ".canRunDtor == false";
  }
}

} } }
