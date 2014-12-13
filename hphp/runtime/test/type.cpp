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

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/type.h"

// for specialized object tests to get some real VM::Class
#include "hphp/system/systemlib.h"

namespace HPHP { namespace jit {

namespace {

typedef hphp_hash_set<Type> TypeSet;

TypeSet allTypes() {
  TypeSet r;
# define IRT(name, ...) r.insert(Type::name);
# define IRTP(name, ...) IRT(name)
  IR_TYPES
# undef IRT
# undef IRTP
  return r;
}

}

TEST(Type, Equality) {
  EXPECT_NE(Type::Cls, Type::PtrToBoxedObj);
}

TEST(Type, Null) {
  EXPECT_TRUE(Type::Uninit <= Type::Null);
  EXPECT_TRUE(Type::InitNull <= Type::Null);
  EXPECT_FALSE(Type::Bool <= Type::Null);
  EXPECT_FALSE(Type::Null.subtypeOf(Type::InitNull));
  EXPECT_NE(Type::Null, Type::Uninit);
  EXPECT_NE(Type::Null, Type::InitNull);

  EXPECT_TRUE(Type::Null.needsReg());
  EXPECT_FALSE(Type::Uninit.needsReg());
  EXPECT_FALSE(Type::InitNull.needsReg());
  EXPECT_FALSE(Type::Null.needsValueReg());
  EXPECT_FALSE(Type::Uninit.needsValueReg());
  EXPECT_FALSE(Type::InitNull.needsValueReg());
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


  auto const sub = Type::Obj.specialize(SystemLib::s_IteratorClass);
  auto const exact = Type::Obj.specializeExact(SystemLib::s_IteratorClass);

  EXPECT_EQ("Obj<=Iterator", sub.toString());
  EXPECT_EQ("Obj=Iterator", exact.toString());
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

  EXPECT_EQ(Type::Int | Type::Dbl, (Type::Int | Type::BoxedDbl).unbox());
}

TEST(Type, Ptr) {
  EXPECT_TRUE(Type::PtrToInt.isPtr());
  EXPECT_TRUE(Type::PtrToBoxedInt.isPtr());
  EXPECT_TRUE(Type::PtrToBoxedCell.isPtr());
  EXPECT_TRUE(Type::PtrToInt.subtypeOf(Type::PtrToCell));

  EXPECT_EQ(Type::PtrToInt, Type::Int.ptr(Ptr::Unk));
  EXPECT_EQ(Type::PtrToCell, Type::Cell.ptr(Ptr::Unk));
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
  expectTrue(Type::CountedArr);
  expectTrue(Type::Obj);
  expectTrue(Type::NullableObj);
  expectTrue(Type::Res);
  expectTrue(Type::Counted);
  expectTrue(Type::BoxedArr);
  expectTrue(Type::BoxedCountedArr);
  expectTrue(Type::BoxedObj);
  expectTrue(Type::BoxedNullableObj);
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
  expectTrue(Type::AnyNullableObj);
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
  EXPECT_EQ(Type::Bool, Type::unionOf(Type::cns(true), Type::cns(false)));
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

namespace {
inline bool fits(Type t, TypeConstraint tc) {
  return typeFitsConstraint(t, tc);
}
}

TEST(Type, TypeConstraints) {
  EXPECT_TRUE(fits(Type::Gen, DataTypeGeneric));
  EXPECT_FALSE(fits(Type::Gen, DataTypeCountness));
  EXPECT_FALSE(fits(Type::Gen, DataTypeCountnessInit));
  EXPECT_FALSE(fits(Type::Gen, DataTypeSpecific));
  EXPECT_FALSE(fits(Type::Gen,
                    TypeConstraint(DataTypeSpecialized).setWantArrayKind()));

  EXPECT_TRUE(fits(Type::Cell,
                   {DataTypeGeneric}));
  EXPECT_TRUE(fits(Type::Gen,
                    {DataTypeGeneric}));

  EXPECT_FALSE(fits(Type::Arr,
                    TypeConstraint(DataTypeSpecialized).setWantArrayKind()));
  EXPECT_TRUE(fits(Type::Arr.specialize(ArrayData::kPackedKind),
                   TypeConstraint(DataTypeSpecialized).setWantArrayKind()));
}

TEST(Type, RelaxType) {
  EXPECT_EQ(Type::Gen, relaxType(Type::BoxedStr, {DataTypeGeneric}));
  EXPECT_EQ(Type::BoxedInitCell | Type::Uncounted,
            relaxType(Type::BoxedObj | Type::InitNull,
                      {DataTypeCountness}));


  auto tc = TypeConstraint{DataTypeSpecialized};
  tc.setDesiredClass(SystemLib::s_IteratorClass);
  tc.category = DataTypeSpecialized;
  auto type = Type::Obj.specialize(SystemLib::s_IteratorClass);
  EXPECT_EQ("Obj<=Iterator", type.toString());
  EXPECT_EQ(type, relaxType(type, tc));

  EXPECT_EQ(Type::BoxedInitCell,
            relaxType(Type::BoxedInitCell, DataTypeCountnessInit));
  EXPECT_EQ(Type::BoxedInitCell,
            relaxType(Type::BoxedInitCell, DataTypeCountness));
}

TEST(Type, RelaxConstraint) {
  EXPECT_EQ(TypeConstraint(DataTypeCountness),
            relaxConstraint(TypeConstraint{DataTypeSpecific},
                            Type::Cell,
                            Type::Arr));

  EXPECT_EQ(TypeConstraint(DataTypeGeneric),
            relaxConstraint(TypeConstraint{DataTypeCountness},
                            Type::Arr,
                            Type::Cell));
}

TEST(Type, Specialized) {
  auto packed = Type::Arr.specialize(ArrayData::kPackedKind);
  EXPECT_LE(packed, Type::Arr);
  EXPECT_LT(packed, Type::Arr);
  EXPECT_FALSE(Type::Arr <= packed);
  EXPECT_LT(packed, Type::Arr | Type::Obj);
  EXPECT_EQ(packed, packed & (Type::Arr | Type::Counted));
  EXPECT_GE(packed, Type::Bottom);
  EXPECT_GT(packed, Type::Bottom);

  EXPECT_TRUE(Type::Int <= (packed | Type::Int));

  EXPECT_EQ(Type::Bottom, packed - Type::Arr);

  auto const array = make_packed_array(1, 2, 3, 4);
  auto const mixed = make_map_array(1, 1, 2, 2);
  auto const arrData = ArrayData::GetScalarArray(array.get());
  auto const arrDataMixed = ArrayData::GetScalarArray(mixed.get());
  auto constArray = Type::cns(arrData);
  auto constArrayMixed = Type::cns(arrDataMixed);
  auto const spacked = Type::StaticArr.specialize(ArrayData::kPackedKind);
  EXPECT_EQ(spacked, spacked - constArray); // conservative
  EXPECT_EQ(constArray, constArray - spacked); // conservative (could be
                                               // bottom if we did better)

  // Implemented conservatively right now, but the following better not return
  // bottom:
  EXPECT_EQ(constArrayMixed, constArrayMixed - spacked);

  // Checking specialization dropping.
  EXPECT_EQ(Type::Arr | Type::BoxedInitCell, packed | Type::BoxedInitCell);
  auto specializedObj = Type::Obj.specialize(SystemLib::s_IteratorClass);
  EXPECT_EQ(Type::Arr | Type::Obj, packed | specializedObj);
}

TEST(Type, SpecializedObjects) {
  auto const A = SystemLib::s_IteratorClass;
  auto const B = SystemLib::s_TraversableClass;
  EXPECT_TRUE(A->classof(B));

  auto const obj = Type::Obj;
  auto const exactA = obj.specializeExact(A);
  auto const exactB = obj.specializeExact(B);
  auto const subA = obj.specialize(A);
  auto const subB = obj.specialize(B);

  EXPECT_EQ(exactA.getClass(), A);
  EXPECT_EQ(subA.getClass(), A);

  EXPECT_EQ(exactA.getExactClass(), A);
  EXPECT_EQ(subA.getExactClass(), nullptr);

  EXPECT_LE(exactA, exactA);
  EXPECT_LE(subA, subA);

  EXPECT_LT(exactA, obj);
  EXPECT_LT(subA, obj);

  EXPECT_LE(Type::Bottom, subA);
  EXPECT_LE(Type::Bottom, exactA);

  EXPECT_LT(exactA, subA);

  EXPECT_LT(exactA, subB);
  EXPECT_LT(subA, subB);

  EXPECT_FALSE(exactA <= exactB);
  EXPECT_FALSE(subA <= exactB);

  EXPECT_EQ(exactA & subA, exactA);
  EXPECT_EQ(subA & exactA, exactA);
  EXPECT_EQ(exactB & subB, exactB);
  EXPECT_EQ(subB & exactB, exactB);

  EXPECT_EQ(Type::Obj, Type::Obj - subA);  // conservative
  EXPECT_EQ(subA, subA - exactA);  // conservative
}

TEST(Type, Const) {
  auto five = Type::cns(5);
  EXPECT_LT(five, Type::Int);
  EXPECT_NE(five, Type::Int);
  EXPECT_TRUE(five.isConst());
  EXPECT_EQ(5, five.intVal());
  EXPECT_TRUE(five.isConst(Type::Int));
  EXPECT_TRUE(five.isConst(5));
  EXPECT_FALSE(five.isConst(5.0));
  EXPECT_TRUE(Type::Gen.maybe(five));
  EXPECT_EQ(Type::Int, five | Type::Int);
  EXPECT_EQ(Type::Int, five | Type::cns(10));
  EXPECT_EQ(five, five | Type::cns(5));
  EXPECT_EQ(five, Type::cns(5) & five);
  EXPECT_EQ(five, five & Type::Int);
  EXPECT_EQ(five, Type::Gen & five);
  EXPECT_EQ("Int<5>", five.toString());
  EXPECT_EQ(five, five - Type::Arr);
  EXPECT_EQ(five, five - Type::cns(1));
  EXPECT_EQ(Type::Int, Type::Int - five); // conservative
  EXPECT_EQ(Type::Bottom, five - Type::Int);
  EXPECT_EQ(Type::Bottom, five - five);
  EXPECT_EQ(Type::PtrToGen,
            (Type::PtrToGen|Type::Nullptr) - Type::Nullptr);
  EXPECT_EQ(Type::Int, five.dropConstVal());
  EXPECT_TRUE(five.not(Type::cns(2)));

  auto True = Type::cns(true);
  EXPECT_EQ("Bool<true>", True.toString());
  EXPECT_LT(True, Type::Bool);
  EXPECT_NE(True, Type::Bool);
  EXPECT_TRUE(True.isConst());
  EXPECT_EQ(true, True.boolVal());
  EXPECT_TRUE(Type::Uncounted.maybe(True));
  EXPECT_FALSE(five <= True);
  EXPECT_FALSE(five > True);

  EXPECT_TRUE(five.not(True));
  EXPECT_EQ(Type::Int | Type::Bool, five | True);
  EXPECT_EQ(Type::Bottom, five & True);

  EXPECT_TRUE(Type::Uninit.isConst());
  EXPECT_TRUE(Type::InitNull.isConst());
  EXPECT_FALSE(Type::Null.isConst());
  EXPECT_FALSE((Type::Uninit | Type::Bool).isConst());
  EXPECT_FALSE(Type::Int.isConst());

  auto array = make_packed_array(1, 2, 3, 4);
  auto arrData = ArrayData::GetScalarArray(array.get());
  auto constArray = Type::cns(arrData);
  auto packedArray = Type::Arr.specialize(ArrayData::kPackedKind);
  auto mixedArray = Type::Arr.specialize(ArrayData::kMixedKind);

  EXPECT_TRUE(constArray <= packedArray);
  EXPECT_TRUE(constArray < packedArray);
  EXPECT_FALSE(packedArray <= constArray);
  EXPECT_TRUE(constArray <= constArray);
  EXPECT_FALSE(packedArray <= mixedArray);
  EXPECT_FALSE(mixedArray <= packedArray);
  EXPECT_FALSE(constArray <= mixedArray);
  EXPECT_EQ(constArray, constArray & packedArray);

  ArrayTypeTable::Builder ratBuilder;
  auto rat1 = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                 RepoAuthType(RepoAuthType::Tag::Str));
  auto ratArray1 = Type::Arr.specialize(rat1);
  auto rat2 = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                 RepoAuthType(RepoAuthType::Tag::Int));
  auto ratArray2 = Type::Arr.specialize(rat2);
  EXPECT_EQ(Type::Arr, ratArray1 & ratArray2);
  EXPECT_TRUE(ratArray1 < Type::Arr);
  EXPECT_TRUE(ratArray1 <= ratArray1);
  EXPECT_TRUE(ratArray1 < (Type::Arr|Type::Obj));
  EXPECT_FALSE(ratArray1 < ratArray2);
  EXPECT_NE(ratArray1, ratArray2);

  auto packedRat = packedArray & ratArray1;
  EXPECT_EQ("Arr=PackedKind:N([Str])", packedRat.toString());
  EXPECT_TRUE(packedRat <= packedArray);
  EXPECT_TRUE(packedRat < packedArray);
  EXPECT_TRUE(packedRat <= ratArray1);
  EXPECT_TRUE(packedRat < ratArray1);
  EXPECT_EQ(packedRat, packedRat & packedArray);
  EXPECT_EQ(packedRat, packedRat & ratArray1);
}

TEST(Type, PtrKinds) {
  auto const frameGen    = Type::Gen.ptr(Ptr::Frame);
  auto const frameUninit = Type::Uninit.ptr(Ptr::Frame);
  auto const frameBool   = Type::Bool.ptr(Ptr::Frame);
  auto const unknownBool = Type::Bool.ptr(Ptr::Unk);
  auto const unknownGen  = Type::Gen.ptr(Ptr::Unk);
  auto const stackObj    = Type::Obj.ptr(Ptr::Stk);
  auto const stackBool    = Type::Bool.ptr(Ptr::Stk);

  EXPECT_EQ("PtrToFrameGen", frameGen.toString());
  EXPECT_EQ("PtrToFrameBool", frameBool.toString());
  EXPECT_EQ("PtrToBool", unknownBool.toString());
  EXPECT_EQ("PtrToStkObj", stackObj.toString());
  EXPECT_EQ("Nullptr|PtrToPropCell",
    (Type::PtrToPropCell|Type::Nullptr).toString());

  EXPECT_EQ(Ptr::Frame, (frameUninit|frameBool).ptrKind());

  EXPECT_TRUE(frameBool.subtypeOf(unknownBool));
  EXPECT_TRUE(frameBool.subtypeOf(frameGen));
  EXPECT_TRUE(!frameBool.subtypeOf(frameUninit));
  EXPECT_TRUE(frameBool.maybe(frameGen));
  EXPECT_TRUE(frameBool.maybe(unknownBool));
  EXPECT_TRUE(!frameUninit.maybe(frameBool));
  EXPECT_TRUE(frameUninit.maybe(frameGen));
  EXPECT_TRUE(!frameUninit.maybe(unknownBool));
  EXPECT_TRUE(!Type::PtrToUninit.maybe(Type::PtrToBool));
  EXPECT_TRUE(!unknownBool.subtypeOf(frameBool));
  EXPECT_EQ(unknownBool, frameBool | unknownBool);

  EXPECT_EQ(unknownGen, Type::unionOf(frameGen, unknownBool));

  EXPECT_EQ(Type::Bottom, frameBool & stackBool);
  EXPECT_EQ(frameBool, frameBool & unknownBool);

  EXPECT_EQ(Type::PtrToCell, Type::PtrToPropCell|Type::PtrToFrameCell);

  EXPECT_EQ(Ptr::Prop,
            (Type::PtrToPropCell|Type::Nullptr).ptrKind());
  EXPECT_EQ(Ptr::RProp,
            (Type::PtrToRPropCell|Type::Nullptr).ptrKind());
  EXPECT_EQ(Type::PtrToPropCell,
            (Type::PtrToPropCell|Type::Nullptr) - Type::Nullptr);
}

TEST(Type, PtrRefs) {
  EXPECT_EQ(Type::Bottom, Type::PtrToStkCell & Type::PtrToRefCell);
  EXPECT_EQ(Type::PtrToRefCell, Type::PtrToRStkCell & Type::PtrToRFrameCell);
  EXPECT_EQ(Type::PtrToPropCell, Type::PtrToRPropCell & Type::PtrToPropCell);
  EXPECT_EQ(Type::Bottom, Type::PtrToRPropCell & Type::PtrToFrameBool);
  EXPECT_FALSE(Type::PtrToRPropCell.maybe(Type::PtrToFrameBool));

  EXPECT_EQ(Type::PtrToPropCell, Type::PtrToPropGen - Type::PtrToBoxedCell);

  EXPECT_EQ(
    Ptr::Ref,
    ((Type::PtrToRPropCell|Type::Nullptr) & Type::PtrToRFrameCell).ptrKind()
  );

  EXPECT_TRUE(Type::PtrToPropCell < Type::PtrToRPropCell);
  EXPECT_TRUE(Type::PtrToPropCell <= Type::PtrToRPropCell);
  EXPECT_TRUE(Type::PtrToPropCell < Type::PtrToMembCell);
  EXPECT_TRUE(Type::PtrToPropCell < Type::PtrToRMembCell);
  EXPECT_FALSE(Type::PtrToPropCell < Type::PtrToRMembInt);
  EXPECT_TRUE(Type::PtrToPropCell.maybe(Type::PtrToRMembInt));
  EXPECT_TRUE(!Type::PtrToPropInt.maybe(Type::PtrToRefInt));
  EXPECT_TRUE(!Type::PtrToPropInt.maybe(Type::PtrToRefBool));
  EXPECT_TRUE(!Type::PtrToPropInt.maybe(Type::PtrToPropBool));

  EXPECT_EQ(Type::Bottom, Type::PtrToRefInt & Type::PtrToClsInitCell);
  EXPECT_EQ(Type::Bottom, Type::PtrToMembCell & Type::PtrToClsInitCell);
}

} }
