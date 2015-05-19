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

#include <unordered_set>

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

std::unordered_set<Type> allTypes() {
  std::unordered_set<Type> r;
# define IRT(name, ...) r.insert(T##name);
# define IRTP(name, ...) IRT(name)
  IR_TYPES
# undef IRT
# undef IRTP
  return r;
}

}

TEST(Type, Equality) {
  EXPECT_NE(TCls, TPtrToBoxedObj);
}

TEST(Type, Null) {
  EXPECT_TRUE(TUninit <= TNull);
  EXPECT_TRUE(TInitNull <= TNull);
  EXPECT_FALSE(TBool <= TNull);
  EXPECT_FALSE(TNull <= TInitNull);
  EXPECT_NE(TNull, TUninit);
  EXPECT_NE(TNull, TInitNull);

  EXPECT_TRUE(TNull.needsReg());
  EXPECT_FALSE(TUninit.needsReg());
  EXPECT_FALSE(TInitNull.needsReg());
}

TEST(Type, KnownDataType) {
  auto trueTypes = {
    TInt, TBoxedCell, TStaticStr,
    TStr, // TODO(#3390819): this should return false...
    TObj,
    TDbl,
    TArr,
    TStaticArr,
    TCountedArr,
    TRes,
    TBool,
    TUninit,
    TInitNull
  };
  for (auto t : trueTypes) {
    EXPECT_TRUE(t.isKnownDataType())
      << t.toString() << ".isKnownDataType()";
  }
  auto falseTypes = {
    // TNull, // TODO(#3390819)
    TCell,
    TGen,
    TInt | TDbl
  };
  for (auto t : falseTypes) {
    EXPECT_FALSE(t.isKnownDataType())
      << "!" << t.toString() << ".isKnownDataType()";
  }
}

TEST(Type, ToString) {
  EXPECT_EQ("Int", TInt.toString());
  EXPECT_EQ("Cell", TCell.toString());
  EXPECT_EQ("BoxedDbl", TBoxedDbl.toString());

  auto const sub = Type::SubObj(SystemLib::s_IteratorClass);
  auto const exact = Type::ExactObj(SystemLib::s_IteratorClass);

  EXPECT_EQ("Obj<=Iterator", sub.toString());
  EXPECT_EQ("Obj=Iterator", exact.toString());
}

TEST(Type, Boxes) {
  EXPECT_EQ(TBoxedDbl, TDbl.box());
  EXPECT_TRUE(TBoxedDbl <= TBoxedCell);
  EXPECT_EQ(TDbl, TBoxedDbl.unbox());
  EXPECT_FALSE(TDbl <= TBoxedCell);
  EXPECT_EQ(TCell, TGen.unbox());
  EXPECT_EQ((TBoxedCell - TBoxedUninit),
            (TCell - TUninit).box());

  EXPECT_EQ(TBottom, TBoxedCell & TPtrToGen);

  EXPECT_EQ(TInt | TDbl, (TInt | TBoxedDbl).unbox());
}

TEST(Type, Ptr) {
  EXPECT_TRUE(TPtrToInt <= TPtrToGen);
  EXPECT_TRUE(TPtrToBoxedInt <= TPtrToGen);
  EXPECT_TRUE(TPtrToBoxedCell <= TPtrToGen);
  EXPECT_TRUE(TPtrToInt <= TPtrToCell);

  EXPECT_EQ(TPtrToInt, TInt.ptr(Ptr::Unk));
  EXPECT_EQ(TPtrToCell, TCell.ptr(Ptr::Unk));
  EXPECT_EQ(TInt, TPtrToInt.deref());
  EXPECT_EQ(TBoxedCell, TPtrToBoxedCell.deref());
}

TEST(Type, Subtypes) {
  Type numbers = TDbl | TInt;
  EXPECT_EQ("{Int|Dbl}", numbers.toString());
  EXPECT_TRUE(TDbl <= numbers);
  EXPECT_TRUE(TInt <= numbers);
  EXPECT_FALSE(TBool <= numbers);

  EXPECT_FALSE(TFunc <= TCell);
  EXPECT_FALSE(TTCA <= TGen);

  EXPECT_TRUE(TPtrToCell < TPtrToGen);
}

TEST(Type, Top) {
  for (auto t : allTypes()) {
    EXPECT_TRUE(t <= TTop);
  }
  for (auto t : allTypes()) {
    if (t == TTop) continue;
    EXPECT_FALSE(TTop <= t);
  }
}

namespace {
inline bool fits(Type t, TypeConstraint tc) {
  return typeFitsConstraint(t, tc);
}
}

TEST(Type, TypeConstraints) {
  EXPECT_TRUE(fits(TGen, DataTypeGeneric));
  EXPECT_FALSE(fits(TGen, DataTypeCountness));
  EXPECT_FALSE(fits(TGen, DataTypeCountnessInit));
  EXPECT_FALSE(fits(TGen, DataTypeSpecific));
  EXPECT_FALSE(fits(TGen,
                    TypeConstraint(DataTypeSpecialized).setWantArrayKind()));

  EXPECT_TRUE(fits(TCell,
                   {DataTypeGeneric}));
  EXPECT_TRUE(fits(TGen,
                    {DataTypeGeneric}));

  EXPECT_FALSE(fits(TArr,
                    TypeConstraint(DataTypeSpecialized).setWantArrayKind()));
  EXPECT_TRUE(fits(Type::Array(ArrayData::kPackedKind),
                   TypeConstraint(DataTypeSpecialized).setWantArrayKind()));
}

TEST(Type, RelaxType) {
  EXPECT_EQ(TGen, relaxType(TBoxedStr, {DataTypeGeneric}));
  EXPECT_EQ(TBoxedInitCell | TUncounted,
            relaxType(TBoxedObj | TInitNull,
                      {DataTypeCountness}));


  auto tc = TypeConstraint{DataTypeSpecialized};
  tc.setDesiredClass(SystemLib::s_IteratorClass);
  tc.category = DataTypeSpecialized;
  auto type = Type::SubObj(SystemLib::s_IteratorClass);
  EXPECT_EQ("Obj<=Iterator", type.toString());
  EXPECT_EQ(type, relaxType(type, tc));

  EXPECT_EQ(TBoxedInitCell,
            relaxType(TBoxedInitCell, DataTypeCountnessInit));
  EXPECT_EQ(TBoxedInitCell,
            relaxType(TBoxedInitCell, DataTypeCountness));
}

TEST(Type, RelaxConstraint) {
  EXPECT_EQ(TypeConstraint(DataTypeCountness),
            relaxConstraint(TypeConstraint{DataTypeSpecific},
                            TCell,
                            TArr));

  EXPECT_EQ(TypeConstraint(DataTypeGeneric),
            relaxConstraint(TypeConstraint{DataTypeCountness},
                            TArr,
                            TCell));
}

TEST(Type, Specialized) {
  auto packed = Type::Array(ArrayData::kPackedKind);
  EXPECT_LE(packed, TArr);
  EXPECT_LT(packed, TArr);
  EXPECT_FALSE(TArr <= packed);
  EXPECT_LT(packed, TArr | TObj);
  EXPECT_EQ(packed, packed & (TArr | TCounted));
  EXPECT_GE(packed, TBottom);
  EXPECT_GT(packed, TBottom);

  EXPECT_TRUE(TInt <= (packed | TInt));

  EXPECT_EQ(TBottom, packed & Type::Array(ArrayData::kMixedKind));
  EXPECT_EQ(TBottom, packed - TArr);

  EXPECT_EQ(TPtrToSPropCell, TPtrToSPropGen - TPtrToBoxedCell);

  auto const array = make_packed_array(1, 2, 3, 4);
  auto const mixed = make_map_array(1, 1, 2, 2);
  auto const arrData = ArrayData::GetScalarArray(array.get());
  auto const arrDataMixed = ArrayData::GetScalarArray(mixed.get());
  auto constArray = Type::cns(arrData);
  auto constArrayMixed = Type::cns(arrDataMixed);
  auto const spacked = Type::StaticArray(ArrayData::kPackedKind);
  EXPECT_EQ(spacked, spacked - constArray); // conservative
  EXPECT_EQ(TBottom, constArray - spacked);

  // Implemented conservatively right now, but the following better not return
  // bottom:
  EXPECT_EQ(constArrayMixed, constArrayMixed - spacked);

  // Checking specialization dropping.
  EXPECT_EQ(TArr | TBoxedInitCell, packed | TBoxedInitCell);
  auto specializedObj = Type::SubObj(SystemLib::s_IteratorClass);
  EXPECT_EQ(TArr | TObj, packed | specializedObj);
}

TEST(Type, SpecializedObjects) {
  auto const A = SystemLib::s_IteratorClass;
  auto const B = SystemLib::s_TraversableClass;
  EXPECT_TRUE(A->classof(B));

  auto const obj = TObj;
  auto const exactA = Type::ExactObj(A);
  auto const exactB = Type::ExactObj(B);
  auto const subA = Type::SubObj(A);
  auto const subB = Type::SubObj(B);

  EXPECT_EQ(exactA.clsSpec().cls(), A);
  EXPECT_EQ(subA.clsSpec().cls(), A);

  EXPECT_EQ(exactA.clsSpec().exactCls(), A);
  EXPECT_EQ(subA.clsSpec().exactCls(), nullptr);

  EXPECT_LE(exactA, exactA);
  EXPECT_LE(subA, subA);

  EXPECT_LT(exactA, obj);
  EXPECT_LT(subA, obj);

  EXPECT_LE(TBottom, subA);
  EXPECT_LE(TBottom, exactA);

  EXPECT_LT(exactA, subA);

  EXPECT_LT(exactA, subB);
  EXPECT_LT(subA, subB);

  EXPECT_FALSE(exactA <= exactB);
  EXPECT_FALSE(subA <= exactB);

  EXPECT_EQ(exactA & subA, exactA);
  EXPECT_EQ(subA & exactA, exactA);
  EXPECT_EQ(exactB & subB, exactB);
  EXPECT_EQ(subB & exactB, exactB);

  EXPECT_EQ(TObj, TObj - subA);  // conservative
  EXPECT_EQ(subA, subA - exactA);  // conservative
}

TEST(Type, Const) {
  auto five = Type::cns(5);
  EXPECT_LT(five, TInt);
  EXPECT_NE(five, TInt);
  EXPECT_TRUE(five.hasConstVal());
  EXPECT_EQ(5, five.intVal());
  EXPECT_TRUE(five.hasConstVal(TInt));
  EXPECT_TRUE(five.hasConstVal(5));
  EXPECT_FALSE(five.hasConstVal(5.0));
  EXPECT_TRUE(TGen.maybe(five));
  EXPECT_EQ(TInt, five | TInt);
  EXPECT_EQ(TInt, five | Type::cns(10));
  EXPECT_EQ(five, five | Type::cns(5));
  EXPECT_EQ(five, Type::cns(5) & five);
  EXPECT_EQ(five, five & TInt);
  EXPECT_EQ(five, TGen & five);
  EXPECT_EQ("Int<5>", five.toString());
  EXPECT_EQ(five, five - TArr);
  EXPECT_EQ(five, five - Type::cns(1));
  EXPECT_EQ(TInt, TInt - five); // conservative
  EXPECT_EQ(TBottom, five - TInt);
  EXPECT_EQ(TBottom, five - five);
  EXPECT_EQ(TPtrToGen,
            (TPtrToGen|TNullptr) - TNullptr);
  EXPECT_EQ(TInt, five.dropConstVal());
  EXPECT_TRUE(!five.maybe(Type::cns(2)));

  auto True = Type::cns(true);
  EXPECT_EQ("Bool<true>", True.toString());
  EXPECT_LT(True, TBool);
  EXPECT_NE(True, TBool);
  EXPECT_TRUE(True.hasConstVal());
  EXPECT_EQ(true, True.boolVal());
  EXPECT_TRUE(TUncounted.maybe(True));
  EXPECT_FALSE(five <= True);
  EXPECT_FALSE(five > True);

  EXPECT_TRUE(!five.maybe(True));
  EXPECT_EQ(TInt | TBool, five | True);
  EXPECT_EQ(TBottom, five & True);
  EXPECT_EQ(Type::cns(false), TBool - True);

  auto array = make_packed_array(1, 2, 3, 4);
  auto arrData = ArrayData::GetScalarArray(array.get());
  auto constArray = Type::cns(arrData);
  auto packedArray = Type::Array(ArrayData::kPackedKind);
  auto mixedArray = Type::Array(ArrayData::kMixedKind);

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
  auto ratArray1 = Type::Array(rat1);
  auto rat2 = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                 RepoAuthType(RepoAuthType::Tag::Int));
  auto ratArray2 = Type::Array(rat2);
  EXPECT_EQ(TArr, ratArray1 & ratArray2);
  EXPECT_TRUE(ratArray1 < TArr);
  EXPECT_TRUE(ratArray1 <= ratArray1);
  EXPECT_TRUE(ratArray1 < (TArr|TObj));
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
  auto const frameGen    = TGen.ptr(Ptr::Frame);
  auto const frameUninit = TUninit.ptr(Ptr::Frame);
  auto const frameBool   = TBool.ptr(Ptr::Frame);
  auto const unknownBool = TBool.ptr(Ptr::Unk);
  auto const unknownGen  = TGen.ptr(Ptr::Unk);
  auto const stackObj    = TObj.ptr(Ptr::Stk);
  auto const stackBool    = TBool.ptr(Ptr::Stk);

  EXPECT_EQ("PtrToFrameGen", frameGen.toString());
  EXPECT_EQ("PtrToFrameBool", frameBool.toString());
  EXPECT_EQ("PtrToBool", unknownBool.toString());
  EXPECT_EQ("PtrToStkObj", stackObj.toString());
  EXPECT_EQ("Nullptr|PtrToPropCell",
    (TPtrToPropCell|TNullptr).toString());

  EXPECT_EQ(Ptr::Frame, (frameUninit|frameBool).ptrKind());

  EXPECT_TRUE(frameBool <= unknownBool);
  EXPECT_TRUE(frameBool <= frameGen);
  EXPECT_FALSE(frameBool <= frameUninit);
  EXPECT_TRUE(frameBool.maybe(frameGen));
  EXPECT_TRUE(frameBool.maybe(unknownBool));
  EXPECT_TRUE(!frameUninit.maybe(frameBool));
  EXPECT_TRUE(frameUninit.maybe(frameGen));
  EXPECT_TRUE(!frameUninit.maybe(unknownBool));
  EXPECT_TRUE(!TPtrToUninit.maybe(TPtrToBool));
  EXPECT_FALSE(unknownBool <= frameBool);
  EXPECT_EQ(unknownBool, frameBool | unknownBool);

  EXPECT_EQ(unknownGen, frameGen | unknownBool);

  EXPECT_EQ(TBottom, frameBool & stackBool);
  EXPECT_EQ(frameBool, frameBool & unknownBool);

  EXPECT_EQ(TPtrToCell, TPtrToPropCell|TPtrToFrameCell);

  EXPECT_EQ(Ptr::Prop,
            (TPtrToPropCell|TNullptr).ptrKind());
  EXPECT_EQ(Ptr::RProp,
            (TPtrToRPropCell|TNullptr).ptrKind());
  EXPECT_EQ(TPtrToPropCell,
            (TPtrToPropCell|TNullptr) - TNullptr);

  auto const frameGenOrCell = frameGen | TCell;
  auto const frameOrRefGenOrCell = frameGenOrCell | TGen.ptr(Ptr::Ref);
  auto const stackOrRefArrOrInt = TArr.ptr(Ptr::RStk) | TInt;
  EXPECT_EQ(frameGenOrCell & stackOrRefArrOrInt, TInt);
  EXPECT_EQ(frameOrRefGenOrCell & stackOrRefArrOrInt,
            TArr.ptr(Ptr::Ref) | TInt);
}

TEST(Type, PtrRefs) {
  EXPECT_EQ(TBottom, TPtrToStkCell & TPtrToRefCell);
  EXPECT_EQ(TPtrToRefCell, TPtrToRStkCell & TPtrToRFrameCell);
  EXPECT_EQ(TPtrToPropCell, TPtrToRPropCell & TPtrToPropCell);
  EXPECT_EQ(TBottom, TPtrToRPropCell & TPtrToFrameBool);
  EXPECT_FALSE(TPtrToRPropCell.maybe(TPtrToFrameBool));

  EXPECT_EQ(TPtrToPropCell, TPtrToPropGen - TPtrToBoxedCell);
  EXPECT_EQ(TPtrToPropInt, TPtrToRPropInt - TPtrToRefCell);
  EXPECT_EQ(TPtrToPropInt, TPtrToRPropInt - TPtrToRStkCell);

  EXPECT_EQ(
    Ptr::Ref,
    ((TPtrToRPropCell|TNullptr) & TPtrToRFrameCell).ptrKind()
  );

  EXPECT_TRUE(TPtrToPropCell < TPtrToRPropCell);
  EXPECT_TRUE(TPtrToPropCell <= TPtrToRPropCell);
  EXPECT_TRUE(TPtrToPropCell < TPtrToMembCell);
  EXPECT_TRUE(TPtrToPropCell < TPtrToRMembCell);
  EXPECT_FALSE(TPtrToPropCell < TPtrToRMembInt);
  EXPECT_TRUE(TPtrToPropCell.maybe(TPtrToRMembInt));
  EXPECT_TRUE(!TPtrToPropInt.maybe(TPtrToRefInt));
  EXPECT_TRUE(!TPtrToPropInt.maybe(TPtrToRefBool));
  EXPECT_TRUE(!TPtrToPropInt.maybe(TPtrToPropBool));

  EXPECT_EQ(TBottom, TPtrToRefInt & TPtrToClsInitCell);
  EXPECT_EQ(TBottom, TPtrToMembCell & TPtrToClsInitCell);
}

} }
