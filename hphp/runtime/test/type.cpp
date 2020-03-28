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

std::unique_ptr<RecordDesc> testRecordDesc(const char* name,
                                           const char* parentName = nullptr) {
  auto const parentStr = parentName ? makeStaticString(name) : nullptr;
  auto const preRec = new PreRecordDesc(nullptr, -1, -1, makeStaticString(name),
                                        Attr{}, parentStr, nullptr, 1);
  auto const parentPreRec = parentStr ?
    new PreRecordDesc(nullptr, -1, -1, parentStr, Attr{}, nullptr, nullptr, 1) :
    nullptr;
  return std::make_unique<RecordDesc>(
    preRec,
    parentPreRec ? RecordDesc::newRecordDesc(parentPreRec, nullptr) : nullptr
  );
}

}

TEST(Type, Equality) {
  EXPECT_NE(TCls, TPtrToObj);
  EXPECT_NE(TCls, TLvalToObj);
  EXPECT_NE(TCls, TMemToObj);
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
    TStaticStr,
    TCountedStr,
    TStr,
    TObj,
    TRecord,
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

  EXPECT_EQ("Vec", TVec.toString());
  EXPECT_EQ("Dict", TDict.toString());
  EXPECT_EQ("Keyset", TKeyset.toString());

  auto const sub = Type::SubObj(SystemLib::s_HH_IteratorClass);
  auto const exact = Type::ExactObj(SystemLib::s_HH_IteratorClass);

  EXPECT_EQ("Obj<=HH\\Iterator", sub.toString());
  EXPECT_EQ("Obj=HH\\Iterator", exact.toString());
  EXPECT_EQ("PtrToStr", TPtrToStr.toString());
  EXPECT_EQ("LvalToStr", TLvalToStr.toString());

  auto const recA = testRecordDesc("A");
  auto const subRec = Type::SubRecord(recA.get());
  auto const exactRec = Type::ExactRecord(recA.get());
  EXPECT_EQ("Record<=A", subRec.toString());
  EXPECT_EQ("Record=A", exactRec.toString());

  EXPECT_EQ("PtrTo{Prop|MIS|MMisc|Other|Field}Cell",
            (TPtrToMembCell - TPtrToElemCell).toString());
  EXPECT_EQ("LvalTo{Prop|MIS|MMisc|Other|Field}Cell",
            (TLvalToMembCell - TLvalToElemCell).toString());
  EXPECT_EQ("PtrToMembCell", TPtrToMembCell.toString());
  EXPECT_EQ("LvalToMembCell", TLvalToMembCell.toString());
  EXPECT_EQ("MemToInt", TMemToInt.toString());
  EXPECT_EQ("PtrTo{Str|Int}|LvalTo{Str|Int}",
            (TMemToInt | TMemToStr).toString());

  EXPECT_EQ("PtrTo{Int|StaticStr}|{Int|StaticStr}",
            (TInt | TPtrToStaticStr).toString());
  EXPECT_EQ("LvalTo{Int|StaticStr}|{Int|StaticStr}",
            (TInt | TLvalToStaticStr).toString());
  EXPECT_EQ("{Obj<=HH\\Iterator|Int}", (TInt | sub).toString());
  EXPECT_EQ("{Record<=A|Int}", (TInt | subRec).toString());

  EXPECT_EQ("Cls<=HH\\Iterator",
            Type::SubCls(SystemLib::s_HH_IteratorClass).toString());
  EXPECT_EQ("Cls=HH\\Iterator",
            Type::ExactCls(SystemLib::s_HH_IteratorClass).toString());

  EXPECT_EQ("{ABC|Func}", (TABC | TFunc).toString());

  EXPECT_EQ("InitNull", TInitNull.constValString());

  EXPECT_EQ("InitCell", TInitCell.toString());
  EXPECT_EQ("PtrToInitCell", TInitCell.ptr(Ptr::Ptr).toString());
  EXPECT_EQ("PtrToFrameInitCell", TPtrToFrameInitCell.toString());
  EXPECT_EQ("LvalToFrameInitCell", TLvalToFrameInitCell.toString());

  auto const ptrCns = Type::cns((TypedValue*)0xba5eba11, TPtrToMembInitNull);
  EXPECT_EQ("PtrToMembInitNull<TV: 0xba5eba11>", ptrCns.toString());
  EXPECT_EQ("TV: 0xba5eba11", ptrCns.constValString());
}

TEST(Type, Ptr) {
  EXPECT_TRUE(TPtrToInt <= TPtrToCell);

  EXPECT_EQ(TPtrToInt, TInt.ptr(Ptr::Ptr));
  EXPECT_EQ(TPtrToCell, TCell.ptr(Ptr::Ptr));
  EXPECT_EQ(TInt, TPtrToInt.deref());

  EXPECT_EQ(TPtrToInt, TPtrToInt - TInt);
  EXPECT_EQ(TInt, (TPtrToInt | TInt) - TPtrToInt);
  EXPECT_EQ(TPtrToUncountedInit, TPtrToUncounted - TPtrToUninit);

  auto const t = TPtrToInt | TPtrToStr | TInt | TStr;
  EXPECT_EQ(t, t - TPtrToInt);
  EXPECT_EQ(t, t - TInt);
  EXPECT_EQ(TPtrToInt | TPtrToStr, t - (TInt | TStr));
  EXPECT_EQ(TInt | TStr, t - (TPtrToInt | TPtrToStr));

  EXPECT_EQ(TBottom, TPtrToInt & TInt);
  auto const a1 = TPackedArr.ptr(Ptr::Frame);
  auto const a2 = TMixedArr.ptr(Ptr::Frame);
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

  auto ptrToPackedArray = TPackedArr.ptr(Ptr::Ptr);
  EXPECT_FALSE(ptrToPackedArray.hasConstVal());
  EXPECT_TRUE(ptrToPackedArray.isSpecialized());
  EXPECT_EQ(TPtrToArr, ptrToPackedArray.unspecialize());
  EXPECT_EQ(packedSpec, ptrToPackedArray.arrSpec());

  auto ptrToExactObj =
    Type::ExactObj(SystemLib::s_HH_IteratorClass).ptr(Ptr::Ptr);
  auto exactClassSpec =
    ClassSpec(SystemLib::s_HH_IteratorClass, ClassSpec::ExactTag{});
  EXPECT_FALSE(ptrToExactObj.hasConstVal());
  EXPECT_TRUE(ptrToExactObj.isSpecialized());
  EXPECT_EQ(TPtrToObj, ptrToExactObj.unspecialize());
  EXPECT_EQ(exactClassSpec, ptrToExactObj.clsSpec());

  auto ptrToSubObj = Type::SubObj(SystemLib::s_HH_IteratorClass).ptr(Ptr::Ptr);
  auto subClassSpec =
    ClassSpec(SystemLib::s_HH_IteratorClass, ClassSpec::SubTag{});
  EXPECT_FALSE(ptrToSubObj.hasConstVal());
  EXPECT_TRUE(ptrToSubObj.isSpecialized());
  EXPECT_EQ(TPtrToObj, ptrToSubObj.unspecialize());
  EXPECT_EQ(subClassSpec, ptrToSubObj.clsSpec());

  auto const recA = testRecordDesc("A");
  auto const ptrToExactRec = Type::ExactRecord(recA.get()).ptr(Ptr::Ptr);
  auto const exactRecSpec = RecordSpec(recA.get(), RecordSpec::ExactTag{});
  EXPECT_FALSE(ptrToExactRec.hasConstVal());
  EXPECT_TRUE(ptrToExactRec.isSpecialized());
  EXPECT_EQ(TPtrToRecord, ptrToExactRec.unspecialize());
  EXPECT_EQ(exactRecSpec, ptrToExactRec.recSpec());

  auto const ptrToSubRec = Type::SubRecord(recA.get()).ptr(Ptr::Ptr);
  auto const subRecSpec = RecordSpec(recA.get(), RecordSpec::SubTag{});
  EXPECT_FALSE(ptrToSubRec.hasConstVal());
  EXPECT_TRUE(ptrToSubRec.isSpecialized());
  EXPECT_EQ(TPtrToRecord, ptrToSubRec.unspecialize());
  EXPECT_EQ(subRecSpec, ptrToSubRec.recSpec());
}

TEST(Type, Lval) {
  EXPECT_TRUE(TLvalToInt <= TLvalToCell);

  EXPECT_EQ(TInt, TLvalToInt.deref());

  EXPECT_EQ(TLvalToInt, TLvalToInt - TInt);
  EXPECT_EQ(TInt, (TLvalToInt | TInt) - TLvalToInt);
  EXPECT_EQ(TLvalToUncountedInit, TLvalToUncounted - TLvalToUninit);

  auto const t = TLvalToInt | TLvalToStr | TInt | TStr;
  EXPECT_EQ(t, t - TLvalToInt);
  EXPECT_EQ(t, t - TInt);
  EXPECT_EQ(TLvalToInt | TLvalToStr, t - (TInt | TStr));
  EXPECT_EQ(TInt | TStr, t - (TLvalToInt | TLvalToStr));

  EXPECT_EQ(TBottom, TLvalToInt & TInt);
}

TEST(Type, Mem) {
  EXPECT_TRUE(TMemToInt <= TMemToCell);

  EXPECT_EQ(TInt, TMemToInt.deref());

  EXPECT_EQ(TMemToInt, TMemToInt - TInt);
  EXPECT_EQ(TInt, (TMemToInt | TInt) - TMemToInt);
  EXPECT_EQ(TMemToUncountedInit, TMemToUncounted - TMemToUninit);

  auto const t = TMemToInt | TMemToStr | TInt | TStr;
  EXPECT_EQ(t, t - TMemToInt);
  EXPECT_EQ(t, t - TInt);
  EXPECT_EQ(TMemToInt | TMemToStr, t - (TInt | TStr));
  EXPECT_EQ(TInt | TStr, t - (TMemToInt | TMemToStr));

  EXPECT_EQ(TBottom, TMemToInt & TInt);
}

TEST(Type, MemPtrLval) {
  EXPECT_TRUE(TPtrToInt <= TMemToCell);
  EXPECT_TRUE(TLvalToInt <= TMemToCell);
  EXPECT_FALSE(TInt <= TMemToCell);

  EXPECT_EQ(TBottom, TPtrToInt & TLvalToInt);
  EXPECT_EQ(TBottom, TPtrToCell & TLvalToCell);
  EXPECT_EQ(TPtrToInt, TPtrToInt & TMemToCell);

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
  EXPECT_FALSE(TTCA <= TCell);

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
  EXPECT_TRUE(fits(TCell, DataTypeGeneric));
  EXPECT_FALSE(fits(TCell, DataTypeCountness));
  EXPECT_FALSE(fits(TCell, DataTypeCountnessInit));
  EXPECT_FALSE(fits(TCell, DataTypeSpecific));

  EXPECT_TRUE(fits(TCell, {DataTypeGeneric}));

  auto const kindConstraint =
    GuardConstraint(DataTypeSpecialized).setWantArrayKind();
  EXPECT_FALSE(fits(TCell, kindConstraint));
  EXPECT_FALSE(fits(TArr, kindConstraint));
  EXPECT_FALSE(fits(TVanillaArr, kindConstraint));
  EXPECT_TRUE(fits(TPackedArr, kindConstraint));

  auto const vanillaConstraint =
    GuardConstraint(DataTypeSpecialized).setWantVanillaArray();
  EXPECT_FALSE(fits(TCell, vanillaConstraint));
  EXPECT_FALSE(fits(TArr, vanillaConstraint));
  EXPECT_TRUE(fits(TVanillaArr, vanillaConstraint));
  EXPECT_TRUE(fits(TPackedArr, vanillaConstraint));
}

TEST(Type, RelaxType) {
  auto gc = GuardConstraint{DataTypeSpecialized};
  gc.setDesiredClass(SystemLib::s_HH_IteratorClass);
  gc.category = DataTypeSpecialized;
  auto subIter = Type::SubObj(SystemLib::s_HH_IteratorClass);
  EXPECT_EQ("Obj<=HH\\Iterator", subIter.toString());
  EXPECT_EQ(subIter, relaxType(subIter, gc.category));

  auto const rec = testRecordDesc("A");
  gc = GuardConstraint{DataTypeSpecialized};
  gc.setDesiredRecord(rec.get());
  gc.category = DataTypeSpecialized;
  auto subRec = Type::SubRecord(rec.get());
  EXPECT_EQ("Record<=A", subRec.toString());
  EXPECT_EQ(subRec, relaxType(subRec, gc.category));
}

TEST(Type, RelaxConstraint) {
  EXPECT_EQ(GuardConstraint(DataTypeCountnessInit),
            relaxConstraint(GuardConstraint{DataTypeSpecific},
                            TCell,
                            TArr));

  EXPECT_EQ(GuardConstraint(DataTypeGeneric),
            relaxConstraint(GuardConstraint{DataTypeCountness},
                            TArr,
                            TCell));
}

TEST(Type, Specialized) {
  EXPECT_LE(TPackedArr, TArr);
  EXPECT_LT(TPackedArr, TArr);
  EXPECT_FALSE(TArr <= TPackedArr);
  EXPECT_LT(TPackedArr, TArr | TObj);
  EXPECT_LT(TPackedArr, TArr | TRecord);
  EXPECT_EQ(TPackedArr, TPackedArr & (TArr | TCounted));
  EXPECT_GE(TPackedArr, TBottom);
  EXPECT_GT(TPackedArr, TBottom);

  EXPECT_TRUE(TInt <= (TPackedArr | TInt));

  EXPECT_EQ(TBottom, TPackedArr & TMixedArr);
  EXPECT_EQ(TBottom, TPackedArr - TArr);

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
  // We cannot specialize on more than one type.
  auto subIter = Type::SubObj(SystemLib::s_HH_IteratorClass);
  EXPECT_EQ(TArr | TObj, TPackedArr | subIter);

  auto const recA = testRecordDesc("A");
  auto const subRec = Type::SubRecord(recA.get());
  EXPECT_EQ(TArr | TRecord, TPackedArr | subRec);

  auto const packedOrInt = spacked | TInt;
  EXPECT_EQ(TInt, packedOrInt - TArr);
  EXPECT_EQ(TInt, packedOrInt - spacked);
  EXPECT_EQ(spacked, packedOrInt - TInt);
  EXPECT_EQ(TPtrToArr, TPtrToArr - TPackedArr.ptr(Ptr::Ptr));

  auto const iterOrStr = subIter | TStr;
  EXPECT_EQ(TStr, iterOrStr - TObj);
  EXPECT_EQ(TStr, iterOrStr - subIter);
  EXPECT_EQ(subIter, iterOrStr - TStr);
  EXPECT_EQ(TPtrToObj, TPtrToObj - subIter.ptr(Ptr::Ptr));

  auto const recOrStr = subRec | TStr;
  EXPECT_EQ(TStr, recOrStr - TRecord);
  EXPECT_EQ(TStr, recOrStr - subRec);
  EXPECT_EQ(subRec, recOrStr - TStr);
  EXPECT_EQ(TPtrToRecord, TPtrToRecord - subRec.ptr(Ptr::Ptr));

  EXPECT_EQ(TObj | TRecord, subRec | subIter);

  auto const subCls = Type::SubCls(SystemLib::s_HH_IteratorClass);
  EXPECT_EQ(TCls, TCls - subCls);
}

TEST(Type, ArrayFitsSpec) {
  auto const nonempty = RepoAuthType::Array::Empty::No;
  auto const maybe_empty = RepoAuthType::Array::Empty::Maybe;
  auto const str_rat = RepoAuthType(RepoAuthType::Tag::Str);
  auto const int_rat = RepoAuthType(RepoAuthType::Tag::Int);

  ArrayTypeTable::Builder builder;
  auto const rat1 = builder.packed(nonempty, {int_rat, int_rat});
  auto const rat2 = builder.packed(maybe_empty, {int_rat, str_rat});
  auto const rat3 = builder.packedn(nonempty, int_rat);

  auto const ratType1 = Type::Array(rat1);
  auto const ratType2 = Type::Array(rat2);
  auto const ratType3 = Type::Array(rat3);
  auto const packedRatType1 = Type::Array(ArrayData::kPackedKind, rat1);
  auto const packedRatType2 = Type::Array(ArrayData::kPackedKind, rat2);
  auto const packedRatType3 = Type::Array(ArrayData::kPackedKind, rat3);

  auto const varr1 = ArrayData::GetScalarArray(make_varray(2, 3));
  auto const varr2 = ArrayData::GetScalarArray(make_varray(2, ""));
  auto const varr3 = ArrayData::GetScalarArray(make_varray(2, 3, 5));

  DArrayInit darr_init1{2};
  darr_init1.set(make_tv<KindOfInt64>(0), 2);
  darr_init1.set(make_tv<KindOfInt64>(1), 3);
  auto const darr1 = ArrayData::GetScalarArray(darr_init1.toArray());

  DArrayInit darr_init2{2};
  darr_init2.set(make_tv<KindOfInt64>(17), 2);
  darr_init2.set(make_tv<KindOfInt64>(19), 3);
  auto const darr2 = ArrayData::GetScalarArray(darr_init2.toArray());

  EXPECT_FALSE(Type::cns(staticEmptyVArray()) <= ratType1);
  EXPECT_TRUE(Type::cns(staticEmptyVArray()) <= ratType2);
  EXPECT_FALSE(Type::cns(staticEmptyVArray()) <= ratType3);
  EXPECT_FALSE(Type::cns(staticEmptyVArray()) <= packedRatType1);
  EXPECT_TRUE(Type::cns(staticEmptyVArray()) <= packedRatType2);
  EXPECT_FALSE(Type::cns(staticEmptyVArray()) <= packedRatType3);

  EXPECT_TRUE(Type::cns(varr1) <= ratType1);
  EXPECT_FALSE(Type::cns(varr1) <= ratType2);
  EXPECT_TRUE(Type::cns(varr1) <= ratType3);
  EXPECT_TRUE(Type::cns(varr1) <= packedRatType1);
  EXPECT_FALSE(Type::cns(varr1) <= packedRatType2);
  EXPECT_TRUE(Type::cns(varr1) <= packedRatType3);

  EXPECT_FALSE(Type::cns(varr2) <= ratType1);
  EXPECT_TRUE(Type::cns(varr2) <= ratType2);
  EXPECT_FALSE(Type::cns(varr2) <= ratType3);
  EXPECT_FALSE(Type::cns(varr2) <= packedRatType1);
  EXPECT_TRUE(Type::cns(varr2) <= packedRatType2);
  EXPECT_FALSE(Type::cns(varr2) <= packedRatType3);

  EXPECT_FALSE(Type::cns(varr3) <= ratType1);
  EXPECT_FALSE(Type::cns(varr3) <= ratType2);
  EXPECT_TRUE(Type::cns(varr3) <= ratType3);
  EXPECT_FALSE(Type::cns(varr3) <= packedRatType1);
  EXPECT_FALSE(Type::cns(varr3) <= packedRatType2);
  EXPECT_TRUE(Type::cns(varr3) <= packedRatType3);

  EXPECT_TRUE(Type::cns(darr1) <= ratType1);
  EXPECT_FALSE(Type::cns(darr1) <= ratType2);
  EXPECT_TRUE(Type::cns(darr1) <= ratType3);
  EXPECT_FALSE(Type::cns(darr1) <= packedRatType1);
  EXPECT_FALSE(Type::cns(darr1) <= packedRatType2);
  EXPECT_FALSE(Type::cns(darr1) <= packedRatType3);

  EXPECT_FALSE(Type::cns(darr2) <= ratType1);
  EXPECT_FALSE(Type::cns(darr2) <= ratType2);
  EXPECT_FALSE(Type::cns(darr2) <= ratType3);
  EXPECT_FALSE(Type::cns(darr2) <= packedRatType1);
  EXPECT_FALSE(Type::cns(darr2) <= packedRatType2);
  EXPECT_FALSE(Type::cns(darr2) <= packedRatType3);
}

TEST(Type, SpecializedArrays) {
  EXPECT_FALSE(TArr.isSpecialized());
  EXPECT_FALSE(TArr.arrSpec());
  EXPECT_FALSE(TArr.arrSpec().kind());
  EXPECT_FALSE(TArr.arrSpec().vanilla());

  EXPECT_TRUE(TPackedArr.isSpecialized());
  EXPECT_TRUE(TPackedArr.arrSpec());
  EXPECT_TRUE(TPackedArr.arrSpec().kind());
  EXPECT_TRUE(TPackedArr.arrSpec().vanilla());
  EXPECT_EQ(TPackedArr.arrSpec().kind(), ArrayData::kPackedKind);

  auto const const_array = Type::cns(staticEmptyVArray());
  EXPECT_TRUE(const_array.isSpecialized());
  EXPECT_TRUE(const_array.arrSpec());
  EXPECT_TRUE(const_array.arrSpec().kind());
  EXPECT_TRUE(const_array.arrSpec().vanilla());
  EXPECT_EQ(*const_array.arrSpec().kind(), ArrayData::kPackedKind);

  EXPECT_FALSE(TDict.isSpecialized());
  EXPECT_FALSE(TDict.arrSpec());
  EXPECT_FALSE(TDict.arrSpec().kind());
  EXPECT_FALSE(TDict.arrSpec().vanilla());

  auto const const_dict = Type::cns(staticEmptyDictArray());
  EXPECT_TRUE(const_dict.isSpecialized());
  EXPECT_TRUE(const_dict.arrSpec());
  EXPECT_FALSE(const_dict.arrSpec().kind());
  EXPECT_TRUE(const_dict.arrSpec().vanilla());
}

TEST(Type, SpecializedObjects) {
  auto const A = SystemLib::s_HH_IteratorClass;
  auto const B = SystemLib::s_HH_TraversableClass;
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

TEST(Type, SpecializedRecords) {
  auto const rA = testRecordDesc("A", "B");
  auto const A = rA.get();
  auto const B = A->parent();
  EXPECT_TRUE(A->recordDescOf(B));

  auto const exactA = Type::ExactRecord(A);
  auto const exactB = Type::ExactRecord(B);
  auto const subA = Type::SubRecord(A);
  auto const subB = Type::SubRecord(B);

  EXPECT_EQ(exactA.recSpec().rec(), A);
  EXPECT_EQ(subA.recSpec().rec(), A);

  EXPECT_EQ(exactA.recSpec().exactRec(), A);
  EXPECT_EQ(subA.recSpec().exactRec(), nullptr);

  EXPECT_LE(exactA, exactA);
  EXPECT_LE(subA, subA);

  EXPECT_LT(exactA, TRecord);
  EXPECT_LT(subA, TRecord);

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

  EXPECT_EQ(TRecord, TRecord - subA);  // conservative
  EXPECT_EQ(subA, subA - exactA);  // conservative
}

TEST(Type, SpecializedClass) {
  auto const A = SystemLib::s_HH_IteratorClass;
  auto const B = SystemLib::s_HH_TraversableClass;

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
  EXPECT_TRUE(TCell.maybe(five));
  EXPECT_EQ(TInt, five | TInt);
  EXPECT_EQ(TInt, five | Type::cns(10));
  EXPECT_EQ(five, five | Type::cns(5));
  EXPECT_EQ(five, Type::cns(5) & five);
  EXPECT_EQ(five, five & TInt);
  EXPECT_EQ(five, TCell & five);
  EXPECT_EQ("Int<5>", five.toString());
  EXPECT_EQ(five, five - TArr);
  EXPECT_EQ(five, five - Type::cns(1));
  EXPECT_EQ(TInt, TInt - five); // conservative
  EXPECT_EQ(TInt, fiveArr - TArr);
  EXPECT_EQ(fiveArr, fiveArr - five);
  EXPECT_EQ(TArr, fiveArr - TInt);
  EXPECT_EQ(TBottom, five - TInt);
  EXPECT_EQ(TBottom, five - five);
  EXPECT_EQ(TPtrToCell,
            (TPtrToCell|TNullptr) - TNullptr);
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

  auto const arrData = ArrayData::GetScalarArray(make_packed_array(1, 2, 3, 4));
  auto const constArray = Type::cns(arrData);

  EXPECT_TRUE(constArray <= TPackedArr);
  EXPECT_TRUE(constArray < TPackedArr);
  EXPECT_FALSE(TPackedArr <= constArray);
  EXPECT_TRUE(constArray <= constArray);
  EXPECT_FALSE(TPackedArr <= TMixedArr);
  EXPECT_FALSE(TMixedArr <= TPackedArr);
  EXPECT_FALSE(constArray <= TMixedArr);
  EXPECT_EQ(constArray, constArray & TPackedArr);

  ArrayTypeTable::Builder ratBuilder;
  auto const rat1 = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                       RepoAuthType(RepoAuthType::Tag::Str));
  auto const ratArray1 = Type::Array(rat1);
  auto const rat2 = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                       RepoAuthType(RepoAuthType::Tag::Int));
  auto const ratArray2 = Type::Array(rat2);
  EXPECT_EQ(ratArray1, ratArray1 & ratArray2);
  EXPECT_EQ(ratArray1, ratArray2 & ratArray1);
  EXPECT_TRUE(ratArray1 < TArr);
  EXPECT_TRUE(ratArray1 <= ratArray1);
  EXPECT_TRUE(ratArray1 < (TArr|TObj));
  EXPECT_TRUE(ratArray1 < (TArr|TRecord));
  EXPECT_FALSE(ratArray1 < ratArray2);
  EXPECT_NE(ratArray1, ratArray2);

  auto const packedRat = ratArray1 & TPackedArr;
  EXPECT_EQ("Arr=PackedKind", TPackedArr.toString());
  EXPECT_EQ("Arr=PackedKind:N([Str])", packedRat.toString());
  EXPECT_TRUE(packedRat <= TPackedArr);
  EXPECT_TRUE(packedRat < TPackedArr);
  EXPECT_TRUE(packedRat <= ratArray1);
  EXPECT_TRUE(packedRat < ratArray1);
  EXPECT_TRUE(packedRat.arrSpec().vanilla());

  auto const widenedRat = ratArray1.widenToBespoke();
  EXPECT_EQ("Arr=N([Str])", ratArray1.toString());
  EXPECT_EQ("Arr={N([Str])|Bespoke}", widenedRat.toString());
  EXPECT_TRUE(ratArray1 < widenedRat);
  EXPECT_TRUE(ratArray1 <= widenedRat);
  EXPECT_FALSE(widenedRat < ratArray1);
  EXPECT_FALSE(widenedRat <= ratArray1);
  EXPECT_EQ(ratArray1, widenedRat & TVanillaArr);
  EXPECT_EQ(ratArray1, widenedRat.narrowToVanilla());
  EXPECT_FALSE(widenedRat.arrSpec().vanilla());

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

TEST(Type, DVArray) {
  EXPECT_EQ(TDArr, TMixedArr.narrowToDVArray());
  EXPECT_EQ("Arr=MixedKind:DV", TDArr.toString());
  EXPECT_TRUE(TDArr <= TArr);
  EXPECT_TRUE(TDArr <= TMixedArr);
  EXPECT_FALSE(TDArr <= TPackedArr);
  EXPECT_TRUE(TDArr.arrSpec().kind());
  EXPECT_FALSE(TDArr.arrSpec().type());
  EXPECT_TRUE(TDArr.arrSpec().dvarray());
  EXPECT_TRUE(TDArr.arrSpec().vanilla());

  EXPECT_EQ(TVArr, TPackedArr.narrowToDVArray());
  EXPECT_EQ("Arr=PackedKind:DV", TVArr.toString());
  EXPECT_TRUE(TVArr <= TArr);
  EXPECT_FALSE(TVArr <= TMixedArr);
  EXPECT_TRUE(TVArr <= TPackedArr);
  EXPECT_TRUE(TVArr.arrSpec().kind());
  EXPECT_FALSE(TVArr.arrSpec().type());
  EXPECT_TRUE(TVArr.arrSpec().dvarray());
  EXPECT_TRUE(TVArr.arrSpec().vanilla());

  auto const dvarr = TDArr | TVArr;
  EXPECT_TRUE(dvarr <= TArr);
  EXPECT_FALSE(dvarr <= TDArr);
  EXPECT_FALSE(dvarr <= TVArr);
  EXPECT_FALSE(dvarr.arrSpec().kind());
  EXPECT_FALSE(dvarr.arrSpec().type());
  EXPECT_TRUE(dvarr.arrSpec().dvarray());
  EXPECT_TRUE(dvarr.arrSpec().vanilla());

  EXPECT_EQ(TDArr, dvarr & TMixedArr);
  EXPECT_EQ(TVArr, dvarr & TPackedArr);
}

TEST(Type, NarrowToVanilla) {
  EXPECT_EQ("Arr=Vanilla", TArr.narrowToVanilla().toString());
  EXPECT_EQ("{Vec=Vanilla|Int}", (TVec|TInt).narrowToVanilla().toString());
  EXPECT_EQ("{Vec|Obj}", (TVec|TObj).narrowToVanilla().toString());
  EXPECT_EQ("Arr=PackedKind", TPackedArr.narrowToVanilla().toString());
}

TEST(Type, WidenToBespoke) {
  EXPECT_EQ("Arr", TVanillaArr.widenToBespoke().toString());
  EXPECT_EQ("{Vec|Int}", (TVanillaVec|TInt).widenToBespoke().toString());
  EXPECT_EQ("Arr={PackedKind|Bespoke}", TPackedArr.widenToBespoke().toString());
}

TEST(Type, VanillaArray) {
  EXPECT_EQ("Arr=Vanilla", TVanillaArr.toString());
  EXPECT_EQ("ArrLike=Vanilla", TVanillaArrLike.toString());
  EXPECT_TRUE(TVanillaArr <= TArr);
  EXPECT_TRUE(TVanillaArr < TArr);
  EXPECT_FALSE(TVanillaArr.arrSpec().kind());
  EXPECT_FALSE(TVanillaArr.arrSpec().type());
  EXPECT_TRUE(TVanillaArr.arrSpec().vanilla());

  EXPECT_EQ("Arr=PackedKind", TPackedArr.toString());
  EXPECT_TRUE(TPackedArr.arrSpec().kind());
  EXPECT_FALSE(TPackedArr.arrSpec().type());
  EXPECT_TRUE(TPackedArr.arrSpec().vanilla());

  ArrayTypeTable::Builder ratBuilder;
  auto const rat = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                      RepoAuthType(RepoAuthType::Tag::Str));
  auto const packedRat = Type::Array(ArrayData::kPackedKind, rat);
  EXPECT_EQ("Arr=PackedKind:N([Str])", packedRat.toString());
  EXPECT_TRUE(packedRat.arrSpec().kind());
  EXPECT_TRUE(packedRat.arrSpec().type());
  EXPECT_TRUE(packedRat.arrSpec().vanilla());

  auto const widenedRat = packedRat.widenToBespoke();
  EXPECT_EQ("Arr={PackedKind:N([Str])|Bespoke}", widenedRat.toString());
  EXPECT_FALSE(widenedRat.arrSpec().kind());
  EXPECT_FALSE(widenedRat.arrSpec().type());
  EXPECT_FALSE(widenedRat.arrSpec().vanilla());
  EXPECT_EQ(packedRat, widenedRat & TVanillaArr);
  EXPECT_EQ(packedRat, widenedRat & TPackedArr);
  EXPECT_EQ(packedRat, widenedRat.narrowToVanilla());

  EXPECT_TRUE(packedRat <= TPackedArr);
  EXPECT_TRUE(packedRat < TPackedArr);
  EXPECT_TRUE(packedRat <= packedRat);
  EXPECT_FALSE(packedRat < packedRat);
  EXPECT_TRUE(packedRat <= TVanillaArr);
  EXPECT_TRUE(packedRat < TVanillaArr);

  EXPECT_FALSE(TPackedArr <= widenedRat);
  EXPECT_FALSE(widenedRat <= TPackedArr);
  EXPECT_FALSE(TPackedArr < widenedRat);
  EXPECT_FALSE(widenedRat < TPackedArr);
  EXPECT_TRUE(TPackedArr <= TVanillaArr);
  EXPECT_FALSE(widenedRat <= TVanillaArr);
  EXPECT_TRUE(TPackedArr < TVanillaArr);
  EXPECT_FALSE(widenedRat < TVanillaArr);
}

TEST(Type, PtrKinds) {
  auto const frameCell    = TCell.ptr(Ptr::Frame);
  auto const frameUninit = TUninit.ptr(Ptr::Frame);
  auto const frameBool   = TBool.ptr(Ptr::Frame);
  auto const unknownBool = TBool.ptr(Ptr::Ptr);
  auto const unknownCell  = TCell.ptr(Ptr::Ptr);
  auto const stackObj    = TObj.ptr(Ptr::Stk);
  auto const stackRec    = TRecord.ptr(Ptr::Stk);
  auto const stackBool    = TBool.ptr(Ptr::Stk);

  EXPECT_EQ("PtrToFrameCell", frameCell.toString());
  EXPECT_EQ("PtrToFrameBool", frameBool.toString());
  EXPECT_EQ("PtrToBool", unknownBool.toString());
  EXPECT_EQ("PtrToStkObj", stackObj.toString());
  EXPECT_EQ("PtrToStkRecord", stackRec.toString());
  EXPECT_EQ("Nullptr|PtrToPropCell",
    (TPtrToPropCell|TNullptr).toString());
  EXPECT_EQ("Nullptr|PtrToFieldCell",
    (TPtrToFieldCell|TNullptr).toString());

  EXPECT_EQ(Ptr::Frame, (frameUninit|frameBool).ptrKind());

  EXPECT_TRUE(frameBool <= unknownBool);
  EXPECT_TRUE(frameBool <= frameCell);
  EXPECT_FALSE(frameBool <= frameUninit);
  EXPECT_TRUE(frameBool.maybe(frameCell));
  EXPECT_TRUE(frameBool.maybe(unknownBool));
  EXPECT_TRUE(!frameUninit.maybe(frameBool));
  EXPECT_TRUE(frameUninit.maybe(frameCell));
  EXPECT_TRUE(!frameUninit.maybe(unknownBool));
  EXPECT_TRUE(!TPtrToUninit.maybe(TPtrToBool));
  EXPECT_FALSE(unknownBool <= frameBool);
  EXPECT_EQ(unknownBool, frameBool | unknownBool);

  EXPECT_EQ(unknownCell, frameCell | unknownBool);

  EXPECT_EQ(TBottom, frameBool & stackBool);
  EXPECT_EQ(frameBool, frameBool & unknownBool);

  EXPECT_EQ(Ptr::Prop,
            (TPtrToPropCell|TNullptr).ptrKind());
  EXPECT_EQ(TPtrToPropCell,
            (TPtrToPropCell|TNullptr) - TNullptr);
  EXPECT_EQ(Ptr::Field,
            (TPtrToFieldCell|TNullptr).ptrKind());
  EXPECT_EQ(TPtrToFieldCell,
            (TPtrToFieldCell|TNullptr) - TNullptr);

  auto const frameCellOrCell = frameCell | TCell;
  auto const stackOrArrOrInt = TArr.ptr(Ptr::Stk) | TInt;
  EXPECT_EQ(TInt | TArr, frameCellOrCell & stackOrArrOrInt);
}

TEST(Type, ConstantPtrTypes) {
  std::vector<TypedValue> darrays;
  for (auto const key : {"foo", "bar"}) {
    DArrayInit dinit{1};
    dinit.set(key, make_tv<KindOfBoolean>(true));
    auto const darray = dinit.toArray();
    MixedArray::asMixed(darray.get())->onSetEvalScalar();
    auto const static_darray = MixedArray::CopyStatic(darray.get());
    darrays.push_back(make_persistent_array_like_tv(static_darray));
  }

  // In typical iterator usage, the constant pointer may point to an invalid
  // TypedValue that's off the end of the array being iterated over.
  auto const arr_type1 = Type::cns(darrays[0]);
  auto const arr_type2 = Type::cns(darrays[1]);
  auto const tv = &darrays[2];
  auto const spec_ptr_type = (arr_type1 | arr_type2).ptr(Ptr::Elem);
  auto const base_ptr_type = spec_ptr_type.unspecialize();
  auto const cons_ptr_type = Type::cns(tv, spec_ptr_type);

  EXPECT_EQ("PtrToElemStaticArr=MixedKind:DV", spec_ptr_type.toString());
  EXPECT_EQ("PtrToElemStaticArr", base_ptr_type.toString());
  auto const str = folly::format("PtrToElemStaticArr<TV: {}>", tv).str();
  EXPECT_EQ(str, cons_ptr_type.toString());

  // The specialized ptr type is not constant. The constant ptr type is not
  // specialized, because we can't represent both.
  EXPECT_TRUE(spec_ptr_type.isSpecialized());
  EXPECT_FALSE(spec_ptr_type.hasConstVal());
  EXPECT_FALSE(base_ptr_type.isSpecialized());
  EXPECT_FALSE(base_ptr_type.hasConstVal());
  EXPECT_FALSE(cons_ptr_type.isSpecialized());
  EXPECT_TRUE(cons_ptr_type.hasConstVal());

  // Because of these representation issues, the intersection of these two
  // types should just return the constant pointer type alone.
  EXPECT_FALSE(cons_ptr_type <= spec_ptr_type);
  EXPECT_TRUE(cons_ptr_type <= base_ptr_type);
  EXPECT_EQ(cons_ptr_type, cons_ptr_type & spec_ptr_type);
  EXPECT_EQ(cons_ptr_type, spec_ptr_type & cons_ptr_type);
  EXPECT_EQ(base_ptr_type, cons_ptr_type | spec_ptr_type);
  EXPECT_EQ(base_ptr_type, spec_ptr_type | cons_ptr_type);
}

} }
