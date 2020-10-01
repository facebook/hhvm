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
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/test/bespoke-layout-mock.h"
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
  EXPECT_NE(TLazyCls, TPtrToObj);
  EXPECT_NE(TCls, TLvalToObj);
  EXPECT_NE(TLazyCls, TLvalToObj);
  EXPECT_NE(TCls, TMemToObj);
  EXPECT_NE(TLazyCls, TMemToObj);
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
    TVArr,
    TPersistentVArr,
    TStaticVArr,
    TCountedVArr,
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
    TLazyCls,
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
    TArr,
    TPersistentArr,
    TStaticArr,
    TCountedArr,
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

  EXPECT_EQ("LazyCls", TLazyCls.toString());

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
  auto const a1 = TVArr.ptr(Ptr::Frame);
  auto const a2 = TDArr.ptr(Ptr::Frame);
  EXPECT_EQ(TBottom, a1 & a2);
  EXPECT_EQ(a1, a1 - a2);

  EXPECT_EQ(TPtrToLazyCls, TLazyCls.ptr(Ptr::Ptr));
  EXPECT_EQ(TLazyCls, TPtrToLazyCls.deref());
  EXPECT_EQ(TBottom, TPtrToLazyCls & TLazyCls);

  EXPECT_EQ(TBottom, TBottom.deref());

  auto const vanillaSpec = ArraySpec::Top().narrowToVanilla();
  auto const vecData = ArrayData::GetScalarArray(make_vec_array(1, 2, 3, 4));
  auto const ptrToConstVec  = Type::cns(vecData).ptr(Ptr::Ptr);
  EXPECT_TRUE(ptrToConstVec < TPtrToStaticVec);
  EXPECT_FALSE(ptrToConstVec.hasConstVal());
  EXPECT_TRUE(ptrToConstVec.isSpecialized());
  EXPECT_EQ(TPtrToStaticVec, ptrToConstVec.unspecialize());
  EXPECT_EQ(vanillaSpec, ptrToConstVec.arrSpec());

  auto const ptrToStaticVanillaVec = TStaticVec.narrowToVanilla().ptr(Ptr::Ptr);
  EXPECT_TRUE(ptrToStaticVanillaVec < TPtrToStaticVec);
  EXPECT_FALSE(ptrToStaticVanillaVec.hasConstVal());
  EXPECT_TRUE(ptrToStaticVanillaVec.isSpecialized());
  EXPECT_EQ(TPtrToStaticVec, ptrToStaticVanillaVec.unspecialize());
  EXPECT_EQ(vanillaSpec, ptrToStaticVanillaVec.arrSpec());

  auto const ptrToStaticVec = TStaticVec.ptr(Ptr::Ptr);
  EXPECT_EQ(TPtrToStaticVec, ptrToStaticVec);
  EXPECT_FALSE(ptrToStaticVec.hasConstVal());
  EXPECT_FALSE(ptrToStaticVec.isSpecialized());
  EXPECT_EQ(TPtrToStaticVec, ptrToStaticVec.unspecialize());
  EXPECT_EQ(ArraySpec::Top(), ptrToStaticVec.arrSpec());

  auto const ptrToVanillaVec = TVanillaVec.ptr(Ptr::Ptr);
  EXPECT_TRUE(ptrToVanillaVec < TPtrToVec);
  EXPECT_FALSE(ptrToVanillaVec.hasConstVal());
  EXPECT_TRUE(ptrToVanillaVec.isSpecialized());
  EXPECT_EQ(TPtrToVec, ptrToVanillaVec.unspecialize());
  EXPECT_EQ(vanillaSpec, ptrToVanillaVec.arrSpec());

  auto const ptrToVec = TVec.ptr(Ptr::Ptr);
  EXPECT_EQ(TPtrToVec, ptrToVec);
  EXPECT_FALSE(ptrToVec.hasConstVal());
  EXPECT_FALSE(ptrToVec.isSpecialized());
  EXPECT_EQ(TPtrToVec, ptrToVec.unspecialize());
  EXPECT_EQ(ArraySpec::Top(), ptrToVec.arrSpec());

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

  EXPECT_TRUE(TVArr <= TArr);
  EXPECT_TRUE(TDArr <= TArr);

  EXPECT_TRUE(TVec <= TArrLike);
  EXPECT_TRUE(TDict <= TArrLike);
  EXPECT_TRUE(TKeyset <= TArrLike);
  EXPECT_TRUE(TVArr <= TArrLike);
  EXPECT_TRUE(TDArr <= TArrLike);
  EXPECT_TRUE(TArr <= TArrLike);

  Type funcOrlcls = TFunc | TLazyCls;
  EXPECT_EQ("{Func|LazyCls}", funcOrlcls.toString());
  EXPECT_TRUE(TLazyCls <= funcOrlcls);
  EXPECT_FALSE(TCls <= funcOrlcls);
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
  EXPECT_FALSE(fits(TCell, DataTypeCountnessInit));
  EXPECT_FALSE(fits(TCell, DataTypeSpecific));

  EXPECT_TRUE(fits(TCell, {DataTypeGeneric}));

  auto const vanillaConstraint =
    GuardConstraint(DataTypeSpecialized).setArrayLayoutSensitive();
  EXPECT_FALSE(fits(TCell, vanillaConstraint));
  EXPECT_FALSE(fits(TArr, vanillaConstraint));
  EXPECT_FALSE(fits(TVec, vanillaConstraint));
  EXPECT_FALSE(fits(TVanillaArr, vanillaConstraint));
  EXPECT_TRUE(fits(TVanillaVec, vanillaConstraint));
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
            relaxConstraint(GuardConstraint{DataTypeSpecific}, TCell, TDict));
}

TEST(Type, Specialized) {
  EXPECT_LE(TVanillaArr, TArr);
  EXPECT_LT(TVanillaArr, TArr);
  EXPECT_FALSE(TArr <= TVanillaArr);
  EXPECT_LT(TVanillaArr, TArr | TObj);
  EXPECT_LT(TVanillaArr, TArr | TRecord);
  EXPECT_EQ(TVanillaArr, TVanillaArr & (TArr | TCounted));
  EXPECT_GE(TVanillaArr, TBottom);
  EXPECT_GT(TVanillaArr, TBottom);

  EXPECT_TRUE(TInt <= (TVanillaArr | TInt));

  EXPECT_EQ(TBottom, TVanillaArr & TVec);
  EXPECT_EQ(TBottom, TVanillaArr - TArr);

  auto const varrData = ArrayData::GetScalarArray(make_varray(1, 2, 3, 4));
  auto const darrData = ArrayData::GetScalarArray(make_darray(1, 1, 2, 2));
  auto const constVArr = Type::cns(varrData);
  auto const constDArr = Type::cns(darrData);

  // For some difference types, we are pessimistic: we had better not narrow
  // these differences to TBottom, but we can't represent them finely.
  EXPECT_EQ(TStaticVArr, TStaticVArr - constVArr);
  EXPECT_EQ(TBottom, constVArr - TStaticVArr);
  EXPECT_EQ(constDArr, constDArr - TStaticVArr);
  EXPECT_EQ(TBottom, constDArr - TDArr);

  // Checking specialization dropping.  We cannot specialize on two dimensions
  // (e.g. array-like and object, or array-like and record) at the same time.
  EXPECT_EQ(TStaticVArr | TObj, constVArr | TObj);
  auto const subIter = Type::SubObj(SystemLib::s_HH_IteratorClass);
  EXPECT_EQ(TVArr | TObj, TVArr | subIter);
  auto const recA = testRecordDesc("A");
  auto const subRec = Type::SubRecord(recA.get());
  EXPECT_EQ(TVArr | TRecord, TVArr | subRec);

  auto const varrOrInt = TVArr | TInt;
  EXPECT_EQ(TInt, varrOrInt - TArr);
  EXPECT_EQ(TInt, varrOrInt - TVArr);
  EXPECT_EQ(TVArr, varrOrInt - TInt);
  EXPECT_EQ(TPtrToVArr, TPtrToVArr - constVArr.ptr(Ptr::Ptr));

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

  auto const lclsOrStr = TLazyCls | TStr;
  EXPECT_EQ(TStr, lclsOrStr - TLazyCls);
  EXPECT_EQ(TLazyCls, lclsOrStr - TStr);
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

  EXPECT_TRUE(Type::cns(varr1) <= ratType1);
  EXPECT_FALSE(Type::cns(varr1) <= ratType2);
  EXPECT_TRUE(Type::cns(varr1) <= ratType3);

  EXPECT_FALSE(Type::cns(varr2) <= ratType1);
  EXPECT_TRUE(Type::cns(varr2) <= ratType2);
  EXPECT_FALSE(Type::cns(varr2) <= ratType3);

  EXPECT_FALSE(Type::cns(varr3) <= ratType1);
  EXPECT_FALSE(Type::cns(varr3) <= ratType2);
  EXPECT_TRUE(Type::cns(varr3) <= ratType3);

  EXPECT_TRUE(Type::cns(darr1) <= ratType1);
  EXPECT_FALSE(Type::cns(darr1) <= ratType2);
  EXPECT_TRUE(Type::cns(darr1) <= ratType3);

  EXPECT_FALSE(Type::cns(darr2) <= ratType1);
  EXPECT_FALSE(Type::cns(darr2) <= ratType2);
  EXPECT_FALSE(Type::cns(darr2) <= ratType3);
}

TEST(Type, SpecializedArrays) {
  EXPECT_FALSE(TArr.isSpecialized());
  EXPECT_FALSE(TArr.arrSpec());
  EXPECT_FALSE(TArr.arrSpec().vanilla());

  auto const const_array = Type::cns(staticEmptyVArray());
  EXPECT_TRUE(const_array.isSpecialized());
  EXPECT_TRUE(const_array.arrSpec());
  EXPECT_TRUE(const_array.arrSpec().vanilla());

  EXPECT_FALSE(TDict.isSpecialized());
  EXPECT_FALSE(TDict.arrSpec());
  EXPECT_FALSE(TDict.arrSpec().vanilla());

  auto const const_dict = Type::cns(staticEmptyDictArray());
  EXPECT_TRUE(const_dict.isSpecialized());
  EXPECT_TRUE(const_dict.arrSpec());
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

  auto const arrData = ArrayData::GetScalarArray(make_map_array(1, 2, 3, 4));
  auto const constArray = Type::cns(arrData);

  EXPECT_EQ(constArray, constArray & TDArr);
  EXPECT_TRUE(constArray <= constArray);
  EXPECT_TRUE(constArray <= TDArr);
  EXPECT_TRUE(constArray < TDArr);
  EXPECT_FALSE(TDArr <= constArray);
  EXPECT_FALSE(TDArr <= TVArr);

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

  auto const vanillaRat = ratArray1 & TVanillaArr;
  EXPECT_EQ("Arr=Vanilla", TVanillaArr.toString());
  EXPECT_EQ("Arr=Vanilla:N([Str])", vanillaRat.toString());
  EXPECT_TRUE(vanillaRat <= TVanillaArr);
  EXPECT_TRUE(vanillaRat < TVanillaArr);
  EXPECT_TRUE(vanillaRat <= ratArray1);
  EXPECT_TRUE(vanillaRat < ratArray1);
  EXPECT_TRUE(vanillaRat.arrSpec().vanilla());

  auto const narrowedRat = ratArray1.narrowToVanilla();
  EXPECT_EQ("Arr=N([Str])", ratArray1.toString());
  EXPECT_EQ("Arr=Vanilla:N([Str])", narrowedRat.toString());
  EXPECT_TRUE(narrowedRat < ratArray1);
  EXPECT_TRUE(narrowedRat <= ratArray1);
  EXPECT_FALSE(ratArray1 < narrowedRat);
  EXPECT_FALSE(ratArray1 <= narrowedRat);
  EXPECT_EQ(narrowedRat, ratArray1 & TVanillaArr);
  EXPECT_FALSE(ratArray1.arrSpec().vanilla());

  auto darray = ArrayData::GetScalarArray(make_darray(1, 1, 10, 10));
  auto const constDArray = Type::cns(darray);
  EXPECT_TRUE(constDArray.hasConstVal());
  EXPECT_TRUE(constDArray <= TArr);
  EXPECT_TRUE(constDArray <= TDArr);
  EXPECT_TRUE(constDArray < TDArr);
  EXPECT_FALSE(constDArray <= TVArr);

  auto varray = ArrayData::GetScalarArray(make_varray(1, 1, 10, 10));
  auto const constVArray = Type::cns(varray);
  EXPECT_TRUE(constVArray.hasConstVal());
  EXPECT_TRUE(constVArray <= TArr);
  EXPECT_TRUE(constVArray <= TVArr);
  EXPECT_TRUE(constVArray < TVArr);
  EXPECT_FALSE(constVArray <= TDArr);

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

  auto constLazyCls = Type::cns(LazyClassData::create(makeStaticString("Foo")));
  EXPECT_TRUE(constLazyCls < TLazyCls);
  EXPECT_TRUE(constLazyCls.hasConstVal());
  EXPECT_FALSE(TLazyCls.hasConstVal());
  EXPECT_EQ(TLazyCls | TBool, constLazyCls | True);
  EXPECT_EQ(TBottom, constLazyCls & True);
}

TEST(Type, DVArray) {
  EXPECT_EQ("DArr", TDArr.toString());
  EXPECT_TRUE(TDArr <= TArr);
  EXPECT_FALSE(TDArr <= TVArr);
  EXPECT_FALSE(TArr <= TDArr);
  EXPECT_FALSE(TDArr.arrSpec().type());

  EXPECT_EQ("VArr", TVArr.toString());
  EXPECT_TRUE(TVArr <= TArr);
  EXPECT_FALSE(TVArr <= TDArr);
  EXPECT_FALSE(TArr <= TVArr);
  EXPECT_FALSE(TVArr.arrSpec().type());

  EXPECT_EQ(TArr, TVArr | TDArr);
  EXPECT_EQ(TBottom, TVArr & TDArr);
}

TEST(Type, NarrowToVanilla) {
  EXPECT_EQ("Arr=Vanilla", TArr.narrowToVanilla().toString());
  EXPECT_EQ("Vec=Vanilla", TVec.narrowToVanilla().toString());
  EXPECT_EQ("{Vec=Vanilla|Int}", (TVec|TInt).narrowToVanilla().toString());
  EXPECT_EQ("{Vec|Obj}", (TVec|TObj).narrowToVanilla().toString());
}

TEST(Type, VanillaArray) {
  EXPECT_EQ("Arr=Vanilla", TVanillaArr.toString());
  EXPECT_EQ("ArrLike=Vanilla", TVanillaArrLike.toString());
  EXPECT_TRUE(TVanillaArr <= TArr);
  EXPECT_TRUE(TVanillaArr < TArr);
  EXPECT_FALSE(TVanillaArr.arrSpec().type());
  EXPECT_TRUE(TVanillaArr.arrSpec().vanilla());
}

TEST(Type, VanillaVec) {
  EXPECT_EQ("Vec", TVec.toString());
  EXPECT_FALSE(TVec.arrSpec().type());
  EXPECT_FALSE(TVec.arrSpec().vanilla());

  EXPECT_EQ("Vec=Vanilla", TVanillaVec.toString());
  EXPECT_FALSE(TVanillaVec.arrSpec().type());
  EXPECT_TRUE(TVanillaVec.arrSpec().vanilla());
  EXPECT_EQ(TVanillaVec, TVec & TVanillaVec);
  EXPECT_EQ(TVanillaVec, TVec.narrowToVanilla());

  EXPECT_FALSE(TVec <= TVanillaVec);
  EXPECT_TRUE(TVanillaVec <= TVec);
  EXPECT_FALSE(TVec < TVanillaVec);
  EXPECT_TRUE(TVanillaVec < TVec);
}

TEST(Type, BespokeVec) {
  auto const foo_layout = BespokeLayout{bespoke::testing::makeDummyLayout("foo")};
  auto const bar_layout = BespokeLayout{bespoke::testing::makeDummyLayout("bar")};

  auto const vecFoo = TVec.narrowToBespokeLayout(foo_layout);
  EXPECT_EQ("Vec=Bespoke(foo)", vecFoo.toString());
  EXPECT_FALSE(TVec <= vecFoo);
  EXPECT_FALSE(TVanillaVec <= vecFoo);
  EXPECT_FALSE(vecFoo <= TVanillaVec);
  EXPECT_EQ(vecFoo | TVanillaVec, TVec);
  EXPECT_EQ(TVanillaVec & vecFoo, TBottom);
  EXPECT_EQ(vecFoo | TVec, TVec);
  EXPECT_EQ(vecFoo & TVec, vecFoo);

  auto const vecBar = TVec.narrowToBespokeLayout(bar_layout);
  EXPECT_EQ("Vec=Bespoke(bar)", vecBar.toString());
  EXPECT_FALSE(vecFoo <= vecBar);
  EXPECT_FALSE(vecBar <= vecFoo);
  EXPECT_EQ(TBottom, vecFoo & vecBar);
  EXPECT_EQ(TVec, vecFoo | vecBar);

  auto const vecVanillaBar = TVanillaVec.narrowToBespokeLayout(bar_layout);
  EXPECT_EQ(TBottom, vecVanillaBar);
}

TEST(Type, BespokeVecRAT) {
  auto const foo_layout = BespokeLayout{bespoke::testing::makeDummyLayout("foo")};
  auto const bar_layout = BespokeLayout{bespoke::testing::makeDummyLayout("bar")};

  ArrayTypeTable::Builder ratBuilder;
  auto const rat = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                      RepoAuthType(RepoAuthType::Tag::Str));
  auto const vecRat = Type::Vec(rat);
  EXPECT_EQ("Vec=N([Str])", vecRat.toString());
  auto const vecRatBespoke = vecRat.narrowToBespokeLayout(foo_layout);
  EXPECT_EQ("Vec=Bespoke(foo):N([Str])", vecRatBespoke.toString());
  auto const vecRatVanilla = vecRat.narrowToVanilla();
  EXPECT_EQ("Vec=Vanilla:N([Str])", vecRatVanilla.toString());

  EXPECT_EQ(TBottom, vecRatBespoke.narrowToVanilla());
  EXPECT_EQ(TBottom, vecRatVanilla.narrowToBespokeLayout(foo_layout));
  EXPECT_EQ(TBottom, vecRatBespoke.narrowToBespokeLayout(bar_layout));
}

TEST(Type, VanillaVecRAT) {
  ArrayTypeTable::Builder ratBuilder;
  auto const rat = ratBuilder.packedn(RepoAuthType::Array::Empty::No,
                                      RepoAuthType(RepoAuthType::Tag::Str));
  auto const vecRat = Type::Vec(rat);
  EXPECT_EQ("Vec=N([Str])", vecRat.toString());
  EXPECT_TRUE(vecRat.arrSpec().type());
  EXPECT_FALSE(vecRat.arrSpec().vanilla());

  auto const vanillaVecRat = vecRat.narrowToVanilla();
  EXPECT_EQ("Vec=Vanilla:N([Str])", vanillaVecRat.toString());
  EXPECT_TRUE(vanillaVecRat.arrSpec().type());
  EXPECT_TRUE(vanillaVecRat.arrSpec().vanilla());
  EXPECT_EQ(vanillaVecRat, vecRat & TVanillaVec);

  EXPECT_FALSE(TVec <= vecRat);
  EXPECT_TRUE(vecRat <= TVec);
  EXPECT_FALSE(TVec < vecRat);
  EXPECT_TRUE(vecRat < TVec);

  EXPECT_TRUE(vanillaVecRat <= TVec);
  EXPECT_TRUE(vanillaVecRat < TVec);
  EXPECT_TRUE(vanillaVecRat <= vecRat);
  EXPECT_FALSE(vecRat < vanillaVecRat);
  EXPECT_TRUE(vanillaVecRat <= TVanillaVec);
  EXPECT_TRUE(vanillaVecRat < TVanillaVec);

  EXPECT_TRUE(vanillaVecRat <= TVanillaVec);
  EXPECT_FALSE(vecRat <= TVanillaVec);
  EXPECT_TRUE(vanillaVecRat < TVanillaVec);
  EXPECT_FALSE(vecRat < TVanillaVec);
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

  EXPECT_EQ("PtrToElemStaticDArr=Vanilla", spec_ptr_type.toString());
  EXPECT_EQ("PtrToElemStaticDArr", base_ptr_type.toString());
  auto const str = folly::format("PtrToElemStaticDArr<TV: {}>", tv).str();
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
