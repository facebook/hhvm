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

#include <unordered_set>

#include <folly/portability/GTest.h>
#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/type.h"

// for specialized object tests to get some real VM::Class
#include "hphp/system/systemlib.h"

namespace HPHP { namespace jit {

namespace {

std::unordered_set<Type> allTypes() {
  std::unordered_set<Type> r;
#define IRT(name, ...) r.insert(T##name);
#define IRTP(name, ...) IRT(name)
#define IRTL(name, ...) IRT(name)
#define IRTM(name, ...) IRT(name)
#define IRTX(name, ...) IRT(name)
  IR_TYPES
#undef IRT
#undef IRTP
#undef IRTL
#undef IRTM
#undef IRTX
  return r;
}

}

TEST(Type, Equality) {
  EXPECT_NE(TCls, TPtrToBoxedObj);
  EXPECT_NE(TCls, TLvalToBoxedObj);
  EXPECT_NE(TCls, TMemToBoxedObj);
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
    TInt,
    TBoxedCell,
    TStaticStr,
    TCountedStr,
    TStr,
    TObj,
    TDbl,
    TArr,
    TPersistentArr,
    TStaticArr,
    TCountedArr,
    TVec,
    TPersistentVec,
    TStaticVec,
    TCountedVec,
    TDict,
    TPersistentDict,
    TStaticDict,
    TCountedDict,
    TShape,
    TPersistentShape,
    TCountedShape,
    TKeyset,
    TPersistentKeyset,
    TStaticKeyset,
    TCountedKeyset,
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
    TNull,
    TCell,
    TGen,
    TInt | TDbl,
    TArrLike,
    TPersistentArrLike
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
  EXPECT_EQ("Boxed{Dbl|Int}", (TBoxedInt | TBoxedDbl).toString());

  EXPECT_EQ("Vec", TVec.toString());
  EXPECT_EQ("Dict", TDict.toString());
  EXPECT_EQ("Keyset", TKeyset.toString());

  auto const sub = Type::SubObj(SystemLib::s_IteratorClass);
  auto const exact = Type::ExactObj(SystemLib::s_IteratorClass);

  EXPECT_EQ("Obj<=Iterator", sub.toString());
  EXPECT_EQ("Obj=Iterator", exact.toString());
  EXPECT_EQ("PtrToStr", TPtrToStr.toString());
  EXPECT_EQ("LvalToStr", TLvalToStr.toString());

  EXPECT_EQ("PtrTo{Prop|MIS|MMisc|Other}Gen",
            (TPtrToMembGen - TPtrToElemGen).toString());
  EXPECT_EQ("LvalTo{Prop|MIS|MMisc|Other}Gen",
            (TLvalToMembGen - TLvalToElemGen).toString());
  EXPECT_EQ("PtrToMembGen", TPtrToMembGen.toString());
  EXPECT_EQ("LvalToMembGen", TLvalToMembGen.toString());
  EXPECT_EQ(
    "PtrTo{ClsInit|ClsCns|Frame|Stk|Gbl|Prop|Elem|SProp|MIS|MMisc|Other}Gen",
    (TPtrToGen - TPtrToRefGen).toString()
  );
  EXPECT_EQ(
    "LvalTo{ClsInit|ClsCns|Frame|Stk|Gbl|Prop|Elem|SProp|MIS|MMisc|Other}Gen",
    (TLvalToGen - TLvalToRefGen).toString()
  );
  EXPECT_EQ("MemToInt", TMemToInt.toString());
  EXPECT_EQ("PtrTo{Str|Int}|LvalTo{Str|Int}",
            (TMemToInt | TMemToStr).toString());

  EXPECT_EQ("PtrTo{Int|StaticStr}|{Int|StaticStr}",
            (TInt | TPtrToStaticStr).toString());
  EXPECT_EQ("LvalTo{Int|StaticStr}|{Int|StaticStr}",
            (TInt | TLvalToStaticStr).toString());
  EXPECT_EQ("{Obj<=Iterator|Int}", (TInt | sub).toString());

  EXPECT_EQ("Cls<=Iterator",
            Type::SubCls(SystemLib::s_IteratorClass).toString());
  EXPECT_EQ("Cls=Iterator",
            Type::ExactCls(SystemLib::s_IteratorClass).toString());

  EXPECT_EQ("{ABC|Func}", (TABC | TFunc).toString());

  EXPECT_EQ("InitNull", TInitNull.constValString());

  EXPECT_EQ("InitGen", TInitGen.toString());
  EXPECT_EQ("PtrToInitGen", TInitGen.ptr(Ptr::Ptr).toString());
  EXPECT_EQ("PtrToFrameInitGen", TPtrToFrameInitGen.toString());
  EXPECT_EQ("LvalToFrameInitGen", TLvalToFrameInitGen.toString());

  auto const ptrCns = Type::cns((TypedValue*)0xba5eba11, TPtrToMembInitNull);
  EXPECT_EQ("PtrToMembInitNull<TV: 0xba5eba11>", ptrCns.toString());
  EXPECT_EQ("TV: 0xba5eba11", ptrCns.constValString());
}

TEST(Type, Boxes) {
  EXPECT_EQ(TBoxedDbl, TDbl.box());
  EXPECT_TRUE(TBoxedDbl <= TBoxedCell);
  EXPECT_EQ(TDbl, TBoxedDbl.unbox());
  EXPECT_FALSE(TDbl <= TBoxedCell);
  EXPECT_EQ(TCell, TGen.unbox());
  EXPECT_EQ(TBoxedVec, TVec.box());
  EXPECT_EQ(TBoxedDict, TDict.box());
  EXPECT_EQ(TBoxedKeyset, TKeyset.box());

  EXPECT_EQ((TBoxedCell - TBoxedUninit),
            (TCell - TUninit).box());

  EXPECT_EQ(TBottom, TBoxedCell & TPtrToGen);
  EXPECT_EQ(TBottom, TBoxedCell & TLvalToGen);
  EXPECT_EQ(TBottom, TBoxedCell & TMemToGen);

  EXPECT_EQ(TInt | TDbl, (TInt | TBoxedDbl).unbox());

  auto const packedSpec = ArraySpec(ArrayData::kPackedKind);
  auto const arrData = ArrayData::GetScalarArray(make_packed_array(1, 2, 3, 4));
  auto boxedConstPackedArray = Type::cns(arrData).box();
  EXPECT_FALSE(boxedConstPackedArray.hasConstVal());
  EXPECT_TRUE(boxedConstPackedArray.isSpecialized());
  EXPECT_EQ(TBoxedStaticArr, boxedConstPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, boxedConstPackedArray.arrSpec());

  auto boxedStaticPackedArray = Type::StaticArray(ArrayData::kPackedKind).box();
  EXPECT_FALSE(boxedStaticPackedArray.hasConstVal());
  EXPECT_TRUE(boxedStaticPackedArray.isSpecialized());
  EXPECT_EQ(TBoxedStaticArr, boxedStaticPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, boxedStaticPackedArray.arrSpec());

  auto boxedPackedArray = Type::Array(ArrayData::kPackedKind).box();
  EXPECT_FALSE(boxedPackedArray.hasConstVal());
  EXPECT_TRUE(boxedPackedArray.isSpecialized());
  EXPECT_EQ(TBoxedArr, boxedPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, boxedPackedArray.arrSpec());

  auto boxedExactObj = Type::ExactObj(SystemLib::s_IteratorClass).box();
  auto exactClassSpec =
    ClassSpec(SystemLib::s_IteratorClass, ClassSpec::ExactTag{});
  EXPECT_FALSE(boxedExactObj.hasConstVal());
  EXPECT_TRUE(boxedExactObj.isSpecialized());
  EXPECT_EQ(TBoxedObj, boxedExactObj.unspecialize());
  EXPECT_EQ(exactClassSpec, boxedExactObj.clsSpec());

  auto boxedSubObj = Type::SubObj(SystemLib::s_IteratorClass).box();
  auto subClassSpec =
    ClassSpec(SystemLib::s_IteratorClass, ClassSpec::SubTag{});
  EXPECT_FALSE(boxedSubObj.hasConstVal());
  EXPECT_TRUE(boxedSubObj.isSpecialized());
  EXPECT_EQ(TBoxedObj, boxedSubObj.unspecialize());
  EXPECT_EQ(subClassSpec, boxedSubObj.clsSpec());
}

TEST(Type, Ptr) {
  EXPECT_TRUE(TPtrToInt <= TPtrToGen);
  EXPECT_TRUE(TPtrToBoxedInt <= TPtrToGen);
  EXPECT_TRUE(TPtrToBoxedCell <= TPtrToGen);
  EXPECT_TRUE(TPtrToInt <= TPtrToCell);

  EXPECT_EQ(TPtrToInt, TInt.ptr(Ptr::Ptr));
  EXPECT_EQ(TPtrToCell, TCell.ptr(Ptr::Ptr));
  EXPECT_EQ(TInt, TPtrToInt.deref());
  EXPECT_EQ(TBoxedCell, TPtrToBoxedCell.deref());

  EXPECT_EQ(TPtrToInt, TPtrToInt - TInt);
  EXPECT_EQ(TInt, (TPtrToInt | TInt) - TPtrToInt);
  EXPECT_EQ(TPtrToUncountedInit, TPtrToUncounted - TPtrToUninit);

  auto const t = TPtrToInt | TPtrToStr | TInt | TStr;
  EXPECT_EQ(t, t - TPtrToInt);
  EXPECT_EQ(t, t - TInt);
  EXPECT_EQ(TPtrToInt | TPtrToStr, t - (TInt | TStr));
  EXPECT_EQ(TInt | TStr, t - (TPtrToInt | TPtrToStr));
  EXPECT_EQ(TPtrToFrameGen, TPtrToRFrameGen - TPtrToRefGen);

  EXPECT_EQ(TBottom, TPtrToInt & TInt);
  auto const a1 = Type::Array(ArrayData::kPackedKind).ptr(Ptr::Frame);
  auto const a2 = Type::Array(ArrayData::kMixedKind).ptr(Ptr::Frame);
  EXPECT_EQ(TBottom, a1 & a2);
  EXPECT_EQ(a1, a1 - a2);
  EXPECT_EQ(TVarEnv, (TVarEnv | a1) - a1);

  EXPECT_EQ(TBottom, TBottom.deref());

  auto const packedSpec = ArraySpec(ArrayData::kPackedKind);
  auto const arrData = ArrayData::GetScalarArray(make_packed_array(1, 2, 3, 4));
  auto ptrToConstPackedArray = Type::cns(arrData).ptr(Ptr::Ptr);
  EXPECT_FALSE(ptrToConstPackedArray.hasConstVal());
  EXPECT_TRUE(ptrToConstPackedArray.isSpecialized());
  EXPECT_EQ(TPtrToStaticArr, ptrToConstPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, ptrToConstPackedArray.arrSpec());

  auto ptrToStaticPackedArray =
    Type::StaticArray(ArrayData::kPackedKind).ptr(Ptr::Ptr);
  EXPECT_FALSE(ptrToStaticPackedArray.hasConstVal());
  EXPECT_TRUE(ptrToStaticPackedArray.isSpecialized());
  EXPECT_EQ(TPtrToStaticArr, ptrToStaticPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, ptrToStaticPackedArray.arrSpec());

  auto ptrToPackedArray = Type::Array(ArrayData::kPackedKind).ptr(Ptr::Ptr);
  EXPECT_FALSE(ptrToPackedArray.hasConstVal());
  EXPECT_TRUE(ptrToPackedArray.isSpecialized());
  EXPECT_EQ(TPtrToArr, ptrToPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, ptrToPackedArray.arrSpec());

  auto ptrToExactObj = Type::ExactObj(SystemLib::s_IteratorClass).ptr(Ptr::Ptr);
  auto exactClassSpec =
    ClassSpec(SystemLib::s_IteratorClass, ClassSpec::ExactTag{});
  EXPECT_FALSE(ptrToExactObj.hasConstVal());
  EXPECT_TRUE(ptrToExactObj.isSpecialized());
  EXPECT_EQ(TPtrToObj, ptrToExactObj.unspecialize());
  EXPECT_EQ(exactClassSpec, ptrToExactObj.clsSpec());

  auto ptrToSubObj = Type::SubObj(SystemLib::s_IteratorClass).ptr(Ptr::Ptr);
  auto subClassSpec =
    ClassSpec(SystemLib::s_IteratorClass, ClassSpec::SubTag{});
  EXPECT_FALSE(ptrToSubObj.hasConstVal());
  EXPECT_TRUE(ptrToSubObj.isSpecialized());
  EXPECT_EQ(TPtrToObj, ptrToSubObj.unspecialize());
  EXPECT_EQ(subClassSpec, ptrToSubObj.clsSpec());
}

TEST(Type, Lval) {
  EXPECT_TRUE(TLvalToInt <= TLvalToGen);
  EXPECT_TRUE(TLvalToBoxedInt <= TLvalToGen);
  EXPECT_TRUE(TLvalToBoxedCell <= TLvalToGen);
  EXPECT_TRUE(TLvalToInt <= TLvalToCell);

  EXPECT_EQ(TInt, TLvalToInt.deref());
  EXPECT_EQ(TBoxedCell, TLvalToBoxedCell.deref());

  EXPECT_EQ(TLvalToInt, TLvalToInt - TInt);
  EXPECT_EQ(TInt, (TLvalToInt | TInt) - TLvalToInt);
  EXPECT_EQ(TLvalToUncountedInit, TLvalToUncounted - TLvalToUninit);

  auto const t = TLvalToInt | TLvalToStr | TInt | TStr;
  EXPECT_EQ(t, t - TLvalToInt);
  EXPECT_EQ(t, t - TInt);
  EXPECT_EQ(TLvalToInt | TLvalToStr, t - (TInt | TStr));
  EXPECT_EQ(TInt | TStr, t - (TLvalToInt | TLvalToStr));
  EXPECT_EQ(TLvalToFrameGen, TLvalToRFrameGen - TLvalToRefGen);

  EXPECT_EQ(TBottom, TLvalToInt & TInt);
}

TEST(Type, Mem) {
  EXPECT_TRUE(TMemToInt <= TMemToGen);
  EXPECT_TRUE(TMemToBoxedInt <= TMemToGen);
  EXPECT_TRUE(TMemToBoxedCell <= TMemToGen);
  EXPECT_TRUE(TMemToInt <= TMemToCell);

  EXPECT_EQ(TInt, TMemToInt.deref());
  EXPECT_EQ(TBoxedCell, TMemToBoxedCell.deref());

  EXPECT_EQ(TMemToInt, TMemToInt - TInt);
  EXPECT_EQ(TInt, (TMemToInt | TInt) - TMemToInt);
  EXPECT_EQ(TMemToUncountedInit, TMemToUncounted - TMemToUninit);

  auto const t = TMemToInt | TMemToStr | TInt | TStr;
  EXPECT_EQ(t, t - TMemToInt);
  EXPECT_EQ(t, t - TInt);
  EXPECT_EQ(TMemToInt | TMemToStr, t - (TInt | TStr));
  EXPECT_EQ(TInt | TStr, t - (TMemToInt | TMemToStr));
  EXPECT_EQ(TMemToFrameGen, TMemToRFrameGen - TMemToRefGen);

  EXPECT_EQ(TBottom, TMemToInt & TInt);
}

TEST(Type, MemPtrLval) {
  EXPECT_TRUE(TPtrToInt <= TMemToGen);
  EXPECT_TRUE(TLvalToInt <= TMemToGen);
  EXPECT_FALSE(TInt <= TMemToGen);

  EXPECT_TRUE(TPtrToBoxedInt <= TMemToGen);
  EXPECT_TRUE(TLvalToBoxedInt <= TMemToGen);
  EXPECT_TRUE(TPtrToBoxedCell <= TMemToGen);
  EXPECT_TRUE(TLvalToBoxedCell <= TMemToGen);
  EXPECT_TRUE(TPtrToInt <= TMemToCell);
  EXPECT_TRUE(TLvalToInt <= TMemToCell);

  EXPECT_EQ(TBottom, TPtrToInt & TLvalToInt);
  EXPECT_EQ(TBottom, TPtrToGen & TLvalToGen);
  EXPECT_EQ(TPtrToInt, TPtrToInt & TMemToGen);

  EXPECT_EQ(TPtrToInt, TMemToInt - TLvalToInt);
  EXPECT_EQ(TLvalToInt, TMemToInt - TPtrToInt);

  auto const t = TInt | TPtrToInt | TLvalToInt;
  EXPECT_EQ(TInt, t - (TPtrToInt | TLvalToInt));
  EXPECT_EQ(TPtrToInt, t - (TInt | TLvalToInt));
  EXPECT_EQ(TLvalToInt, t - (TInt | TPtrToInt));
  EXPECT_EQ(TPtrToInt | TLvalToInt, t - TInt);
  EXPECT_EQ(TInt | TLvalToInt, t - TPtrToInt);
  EXPECT_EQ(TInt | TPtrToInt, t - TLvalToInt);

  EXPECT_EQ(t | TStr | TMemToStr, t | TStr);
  EXPECT_EQ(t | TStr | TMemToStr, (t | TStr) - TLvalToInt);

  EXPECT_EQ(TLvalToUncounted, TLvalToUncounted - TPtrToUninit);
}

TEST(Type, Subtypes) {
  Type numbers = TDbl | TInt;
  EXPECT_EQ("{Dbl|Int}", numbers.toString());
  EXPECT_TRUE(TDbl <= numbers);
  EXPECT_TRUE(TInt <= numbers);
  EXPECT_FALSE(TBool <= numbers);

  EXPECT_TRUE(TFunc <= TCell);
  EXPECT_FALSE(TTCA <= TGen);

  EXPECT_TRUE(TPtrToCell < TPtrToGen);

  EXPECT_TRUE(TVec <= TArrLike);
  EXPECT_TRUE(TDict <= TArrLike);
  EXPECT_TRUE(TKeyset <= TArrLike);
  EXPECT_TRUE(TArr <= TArrLike);
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
inline bool fits(Type t, GuardConstraint gc) {
  return typeFitsConstraint(t, gc);
}
}

TEST(Type, GuardConstraints) {
  EXPECT_TRUE(fits(TGen, DataTypeGeneric));
  EXPECT_FALSE(fits(TGen, DataTypeBoxAndCountness));
  EXPECT_FALSE(fits(TGen, DataTypeBoxAndCountnessInit));
  EXPECT_FALSE(fits(TGen, DataTypeSpecific));
  EXPECT_FALSE(fits(TGen,
                    GuardConstraint(DataTypeSpecialized).setWantArrayKind()));

  EXPECT_TRUE(fits(TCell,
                   {DataTypeGeneric}));
  EXPECT_TRUE(fits(TGen,
                    {DataTypeGeneric}));

  EXPECT_FALSE(fits(TArr,
                    GuardConstraint(DataTypeSpecialized).setWantArrayKind()));
  EXPECT_TRUE(fits(Type::Array(ArrayData::kPackedKind),
                   GuardConstraint(DataTypeSpecialized).setWantArrayKind()));
}

TEST(Type, RelaxType) {
  EXPECT_EQ(TGen, relaxType(TBoxedStr, DataTypeGeneric));
  EXPECT_EQ(TBoxedInitCell | TUncounted,
            relaxType(TBoxedObj | TInitNull,
                      DataTypeBoxAndCountness));


  auto gc = GuardConstraint{DataTypeSpecialized};
  gc.setDesiredClass(SystemLib::s_IteratorClass);
  gc.category = DataTypeSpecialized;
  auto subIter = Type::SubObj(SystemLib::s_IteratorClass);
  EXPECT_EQ("Obj<=Iterator", subIter.toString());
  EXPECT_EQ(subIter, relaxType(subIter, gc.category));

  EXPECT_EQ(TBoxedInitCell,
            relaxType(TBoxedInitCell, DataTypeBoxAndCountnessInit));
  EXPECT_EQ(TBoxedInitCell,
            relaxType(TBoxedInitCell, DataTypeBoxAndCountness));
}

TEST(Type, RelaxConstraint) {
  EXPECT_EQ(GuardConstraint(DataTypeBoxAndCountness),
            relaxConstraint(GuardConstraint{DataTypeSpecific},
                            TCell,
                            TArr));

  EXPECT_EQ(GuardConstraint(DataTypeGeneric),
            relaxConstraint(GuardConstraint{DataTypeBoxAndCountness},
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



  auto const arrData = ArrayData::GetScalarArray(make_packed_array(1, 2, 3, 4));
  auto const arrDataMixed = ArrayData::GetScalarArray(make_map_array(1, 1,
                                                                     2, 2));
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
  auto subIter = Type::SubObj(SystemLib::s_IteratorClass);
  EXPECT_EQ(TArr | TObj, packed | subIter);

  auto const packedOrInt = spacked | TInt;
  EXPECT_EQ(TInt, packedOrInt - TArr);
  EXPECT_EQ(TInt, packedOrInt - spacked);
  EXPECT_EQ(spacked, packedOrInt - TInt);
  EXPECT_EQ(TPtrToArr,
            TPtrToArr - Type::Array(ArrayData::kPackedKind).ptr(Ptr::Ptr));

  auto const iterOrStr = subIter | TStr;
  EXPECT_EQ(TStr, iterOrStr - TObj);
  EXPECT_EQ(TStr, iterOrStr - subIter);
  EXPECT_EQ(subIter, iterOrStr - TStr);
  EXPECT_EQ(TPtrToObj, TPtrToObj - subIter.ptr(Ptr::Ptr));

  auto const subCls = Type::SubCls(SystemLib::s_IteratorClass);
  EXPECT_EQ(TCls, TCls - subCls);
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

  EXPECT_LT(subA | TCctx, TCtx);
}

TEST(Type, SpecializedClass) {
  auto const A = SystemLib::s_IteratorClass;
  auto const B = SystemLib::s_TraversableClass;

  EXPECT_TRUE(A->classof(B));

  auto const cls = TCls;
  auto const exactA = Type::ExactCls(A);
  auto const exactB = Type::ExactCls(B);
  auto const subA = Type::SubCls(A);
  auto const subB = Type::SubCls(B);

  EXPECT_EQ(exactA.clsSpec().exactCls(), A);
  EXPECT_EQ(subA.clsSpec().cls(), A);
  EXPECT_EQ(subA.clsSpec().exactCls(), nullptr);

  EXPECT_LE(exactA, exactA);
  EXPECT_LE(subA, subA);

  EXPECT_LT(exactA, cls);
  EXPECT_LT(subA, cls);

  EXPECT_LE(TBottom, exactA);
  EXPECT_LE(TBottom, subA);

  EXPECT_LT(exactA, subA);

  EXPECT_LT(exactA, subB);
  EXPECT_LT(subA, subB);

  EXPECT_FALSE(exactA <= exactB);
  EXPECT_FALSE(subA <= exactB);

  EXPECT_EQ(exactA & subA, exactA);
  EXPECT_EQ(subA & exactA, exactA);
  EXPECT_EQ(exactB & subB, exactB);
  EXPECT_EQ(subB & exactB, exactB);

  EXPECT_EQ(cls, cls - subA);  // conservative
  EXPECT_EQ(subA, subA - exactA);  // conservative
}

TEST(Type, Const) {
  auto five = Type::cns(5);
  auto fiveArr = five | TArr;
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
  EXPECT_EQ(TInt, fiveArr - TArr);
  EXPECT_EQ(fiveArr, fiveArr - five);
  EXPECT_EQ(TArr, fiveArr - TInt);
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
  EXPECT_TRUE(True.boolVal());
  EXPECT_TRUE(TUncounted.maybe(True));
  EXPECT_FALSE(five <= True);
  EXPECT_FALSE(five > True);

  EXPECT_TRUE(!five.maybe(True));
  EXPECT_EQ(TInt | TBool, five | True);
  EXPECT_EQ(TBottom, five & True);
  EXPECT_EQ(Type::cns(false), TBool - True);

  auto arrData = ArrayData::GetScalarArray(make_packed_array(1, 2, 3, 4));
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

  auto vec = make_vec_array(1, 2, 3, 4);
  auto vecData = ArrayData::GetScalarArray(std::move(vec));
  auto constVec = Type::cns(vecData);
  EXPECT_TRUE(constVec < TVec);

  auto dict = make_dict_array(1, 1, 2, 2, 3, 3, 4, 4);
  auto dictData = ArrayData::GetScalarArray(std::move(dict));
  auto constDict = Type::cns(dictData);
  EXPECT_TRUE(constDict < TDict);

  auto keyset = make_keyset_array(1, 2, 3, 4);
  auto keysetData = ArrayData::GetScalarArray(std::move(keyset));
  auto constKeyset = Type::cns(keysetData);
  EXPECT_TRUE(constKeyset < TKeyset);
}

TEST(Type, PtrKinds) {
  auto const frameGen    = TGen.ptr(Ptr::Frame);
  auto const frameUninit = TUninit.ptr(Ptr::Frame);
  auto const frameBool   = TBool.ptr(Ptr::Frame);
  auto const unknownBool = TBool.ptr(Ptr::Ptr);
  auto const unknownGen  = TGen.ptr(Ptr::Ptr);
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

  EXPECT_EQ(Ptr::Prop,
            (TPtrToPropCell|TNullptr).ptrKind());
  EXPECT_EQ(Ptr::RProp,
            (TPtrToRPropCell|TNullptr).ptrKind());
  EXPECT_EQ(TPtrToPropCell,
            (TPtrToPropCell|TNullptr) - TNullptr);

  auto const frameGenOrCell = frameGen | TCell;
  auto const frameOrRefGenOrCell = frameGenOrCell | TGen.ptr(Ptr::Ref);
  auto const stackOrRefArrOrInt = TArr.ptr(Ptr::RStk) | TInt;
  EXPECT_EQ(TInt | TArr, frameGenOrCell & stackOrRefArrOrInt);
  EXPECT_EQ(TArr.ptr(Ptr::Ref) | TInt,
            frameOrRefGenOrCell & stackOrRefArrOrInt);
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
