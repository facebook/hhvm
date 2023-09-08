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
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/test/bespoke-layout-mock.h"
#include "hphp/runtime/vm/jit/array-layout.h"
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
  EXPECT_NE(TCls, TPtr);
  EXPECT_NE(TLazyCls, TPtr);
  EXPECT_NE(TCls, TLval);
  EXPECT_NE(TLazyCls, TLval);
  EXPECT_NE(TCls, TMem);
  EXPECT_NE(TLazyCls, TMem);
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
    TDbl,
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
    TEnumClassLabel,
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

TEST(Type, ArrayLayout) {
  auto const top     = ArrayLayout::Top();
  auto const vanilla = ArrayLayout::Vanilla();
  auto const bespoke = ArrayLayout::Bespoke();
  auto const bottom  = ArrayLayout::Bottom();
  auto const foo = ArrayLayout{
    bespoke::testing::makeDummyLayout("foo", {ArrayLayout::Bespoke()})
  };

  EXPECT_EQ("Top",     top.describe());
  EXPECT_EQ("Vanilla", vanilla.describe());
  EXPECT_EQ("Bespoke", bespoke.describe());
  EXPECT_EQ("Bottom",  bottom.describe());
  EXPECT_EQ("foo",     foo.describe());

  EXPECT_EQ(top, top);
  EXPECT_EQ(vanilla, vanilla);
  EXPECT_EQ(bespoke, bespoke);
  EXPECT_EQ(bottom, bottom);
  EXPECT_EQ(foo, foo);

  EXPECT_NE(top, vanilla);
  EXPECT_NE(top, bespoke);
  EXPECT_NE(top, bottom);
  EXPECT_NE(top, foo);
  EXPECT_NE(vanilla, bespoke);
  EXPECT_NE(vanilla, bottom);
  EXPECT_NE(vanilla, foo);
  EXPECT_NE(bespoke, bottom);
  EXPECT_NE(bespoke, foo);
  EXPECT_NE(bottom, foo);

  EXPECT_TRUE(top <= top);
  EXPECT_FALSE(top <= vanilla);
  EXPECT_FALSE(top <= bespoke);
  EXPECT_FALSE(top <= bottom);
  EXPECT_FALSE(top <= foo);
  EXPECT_TRUE(vanilla <= top);
  EXPECT_TRUE(vanilla <= vanilla);
  EXPECT_FALSE(vanilla <= bespoke);
  EXPECT_FALSE(vanilla <= bottom);
  EXPECT_FALSE(vanilla <= foo);
  EXPECT_TRUE(bespoke <= top);
  EXPECT_FALSE(bespoke <= vanilla);
  EXPECT_TRUE(bespoke <= bespoke);
  EXPECT_FALSE(bespoke <= bottom);
  EXPECT_FALSE(bespoke <= foo);
  EXPECT_TRUE(bottom <= top);
  EXPECT_TRUE(bottom <= vanilla);
  EXPECT_TRUE(bottom <= bespoke);
  EXPECT_TRUE(bottom <= bottom);
  EXPECT_TRUE(bottom <= foo);
  EXPECT_TRUE(foo <= top);
  EXPECT_FALSE(foo <= vanilla);
  EXPECT_TRUE(foo <= bespoke);
  EXPECT_FALSE(foo <= bottom);
  EXPECT_TRUE(foo <= foo);

  EXPECT_EQ(top | top,         top);
  EXPECT_EQ(top | vanilla,     top);
  EXPECT_EQ(top | bespoke,     top);
  EXPECT_EQ(top | bottom,      top);
  EXPECT_EQ(top | foo,         top);
  EXPECT_EQ(vanilla | top,     top);
  EXPECT_EQ(vanilla | vanilla, vanilla);
  EXPECT_EQ(vanilla | bespoke, top);
  EXPECT_EQ(vanilla | bottom,  vanilla);
  EXPECT_EQ(vanilla | foo,     top);
  EXPECT_EQ(bespoke | top,     top);
  EXPECT_EQ(bespoke | vanilla, top);
  EXPECT_EQ(bespoke | bespoke, bespoke);
  EXPECT_EQ(bespoke | bottom,  bespoke);
  EXPECT_EQ(bespoke | foo,     bespoke);
  EXPECT_EQ(bottom | top,      top);
  EXPECT_EQ(bottom | vanilla,  vanilla);
  EXPECT_EQ(bottom | bespoke,  bespoke);
  EXPECT_EQ(bottom | bottom,   bottom);
  EXPECT_EQ(bottom | foo,      foo);
  EXPECT_EQ(foo | top,         top);
  EXPECT_EQ(foo | vanilla,     top);
  EXPECT_EQ(foo | bespoke,     bespoke);
  EXPECT_EQ(foo | bottom,      foo);
  EXPECT_EQ(foo | foo,         foo);

  EXPECT_EQ(top & top,         top);
  EXPECT_EQ(top & vanilla,     vanilla);
  EXPECT_EQ(top & bespoke,     bespoke);
  EXPECT_EQ(top & bottom,      bottom);
  EXPECT_EQ(top & foo,         foo);
  EXPECT_EQ(vanilla & top,     vanilla);
  EXPECT_EQ(vanilla & vanilla, vanilla);
  EXPECT_EQ(vanilla & bespoke, bottom);
  EXPECT_EQ(vanilla & bottom,  bottom);
  EXPECT_EQ(vanilla & foo,     bottom);
  EXPECT_EQ(bespoke & top,     bespoke);
  EXPECT_EQ(bespoke & vanilla, bottom);
  EXPECT_EQ(bespoke & bespoke, bespoke);
  EXPECT_EQ(bespoke & bottom,  bottom);
  EXPECT_EQ(bespoke & foo,     foo);
  EXPECT_EQ(bottom & top,      bottom);
  EXPECT_EQ(bottom & vanilla,  bottom);
  EXPECT_EQ(bottom & bespoke,  bottom);
  EXPECT_EQ(bottom & bottom,   bottom);
  EXPECT_EQ(bottom & foo,      bottom);
  EXPECT_EQ(foo & top,         foo);
  EXPECT_EQ(foo & vanilla,     bottom);
  EXPECT_EQ(foo & bespoke,     foo);
  EXPECT_EQ(foo & bottom,      bottom);
  EXPECT_EQ(foo & foo,         foo);
}

TEST(Type, ToString) {
  EXPECT_EQ("Int", TInt.toString());
  EXPECT_EQ("Cell", TCell.toString());

  EXPECT_EQ("Vec", TVec.toString());
  EXPECT_EQ("Dict", TDict.toString());
  EXPECT_EQ("Keyset", TKeyset.toString());

  EXPECT_EQ("LazyCls", TLazyCls.toString());
  EXPECT_EQ("EnumClassLabel", TEnumClassLabel.toString());

  auto const sub = Type::SubObj(SystemLib::getHH_IteratorClass());
  auto const exact = Type::ExactObj(SystemLib::getHH_IteratorClass());

  EXPECT_EQ("Obj<=HH\\Iterator", sub.toString());
  EXPECT_EQ("Obj=HH\\Iterator", exact.toString());
  EXPECT_EQ("Ptr", TPtr.toString());
  EXPECT_EQ("Lval", TLval.toString());
  EXPECT_EQ("Mem", TMem.toString());

  EXPECT_EQ("PtrToConst",
            (TPtrToElemOrConst - TPtrToElem).toString());
  EXPECT_EQ("LvalToConst",
            (TLvalToElemOrConst - TLvalToElem).toString());
  EXPECT_EQ("MemToConst",
            (TMemToElemOrConst - TMemToElem).toString());
  EXPECT_EQ("PtrToElemOrConst", TPtrToElemOrConst.toString());
  EXPECT_EQ("LvalToElemOrConst", TLvalToElemOrConst.toString());
  EXPECT_EQ("MemToElemOrConst", TMemToElemOrConst.toString());
  EXPECT_EQ("MemToElem", TMemToElem.toString());
  EXPECT_EQ("LvalToElem", TLvalToElem.toString());
  EXPECT_EQ("LvalTo{Frame|Gbl}", (TLvalToFrame | TLvalToGbl).toString());
  EXPECT_EQ("MemTo{Frame|Gbl}", (TLvalToFrame | TPtrToGbl).toString());

  EXPECT_EQ("Ptr|Int", (TInt | TPtr).toString());
  EXPECT_EQ("LvalToFrame|Int", (TInt | TLvalToFrame).toString());
  EXPECT_EQ("LvalTo{Frame|Elem}|Int",
            (TLvalToElem | TInt | TLvalToFrame).toString());
  EXPECT_EQ("{Obj<=HH\\Iterator|Int}", (TInt | sub).toString());

  EXPECT_EQ("Cls<=HH\\Iterator",
            Type::SubCls(SystemLib::getHH_IteratorClass()).toString());
  EXPECT_EQ("Cls=HH\\Iterator",
            Type::ExactCls(SystemLib::getHH_IteratorClass()).toString());

  EXPECT_EQ("{ABC|Func}", (TABC | TFunc).toString());

  EXPECT_EQ("InitNull", TInitNull.constValString());

  EXPECT_EQ("InitCell", TInitCell.toString());
  EXPECT_EQ("PtrToFrame", TPtrToFrame.toString());
  EXPECT_EQ("LvalToFrame", TLvalToFrame.toString());

  auto const ptrCns = Type::cns((TypedValue*)0xba5eba11, TPtrToConst);
  EXPECT_EQ("PtrToConst<TV: 0xba5eba11>", ptrCns.toString());
  EXPECT_EQ("TV: 0xba5eba11", ptrCns.constValString());
}

TEST(Type, Ptr) {
  EXPECT_TRUE(TPtrToFrame <= TPtr);

  EXPECT_EQ(TPtrToElem, TPtrToElem - TInt);
  EXPECT_EQ(TInt, (TPtrToElem | TInt) - TPtrToElem);

  auto const t = TPtrToFrame | TPtrToElem | TInt | TStr;
  EXPECT_EQ(TInt | TStr, t - TPtr);
  EXPECT_EQ(TPtrToFrame | TPtrToElem | TStr, t - TInt);
  EXPECT_EQ(TPtrToFrame | TPtrToElem, t - (TInt | TStr));

  EXPECT_EQ(TBottom, TPtrToFrame & TInt);
  EXPECT_EQ(TBottom, TPtrToFrame & TPtrToElem);
  EXPECT_EQ(TPtrToFrame, TPtrToFrame - TPtrToElem);
  EXPECT_EQ(TBottom, TPtrToElem & TLazyCls);

  auto const ptrCns = Type::cns((TypedValue*)0xba5eba11, TPtrToConst);
  EXPECT_TRUE(ptrCns.hasConstVal());
  EXPECT_TRUE(ptrCns <= TPtrToConst);
  EXPECT_FALSE(ptrCns <= TPtrToElem);
}

TEST(Type, Lval) {
  EXPECT_TRUE(TLvalToFrame <= TLval);

  EXPECT_EQ(TLvalToFrame, TLvalToFrame - TInt);
  EXPECT_EQ(TInt, (TLvalToFrame | TInt) - TLvalToFrame);

  auto const t = TLvalToFrame | TLvalToElem | TInt | TStr;
  EXPECT_EQ(TInt | TStr, t - TLval);
  EXPECT_EQ(TLvalToFrame | TLvalToElem | TStr, t - TInt);
  EXPECT_EQ(TLvalToFrame | TLvalToElem, t - (TInt | TStr));

  EXPECT_EQ(TBottom, TLvalToFrame & TInt);
}

TEST(Type, Mem) {
  EXPECT_TRUE(TMemToFrame <= TMem);

  EXPECT_EQ(TMemToFrame, TMemToFrame - TInt);
  EXPECT_EQ(TInt, (TMemToFrame | TInt) - TMemToFrame);

  auto const t = TMemToFrame | TMemToElem | TInt | TStr;
  EXPECT_EQ(TInt | TStr, t - TMem);
  EXPECT_EQ(TMemToFrame | TMemToElem | TStr, t - TInt);
  EXPECT_EQ(TMemToFrame | TMemToElem, t - (TInt | TStr));

  EXPECT_EQ(TBottom, TMemToFrame & TInt);
}

TEST(Type, MemPtrLval) {
  EXPECT_TRUE(TPtr <= TMem);
  EXPECT_TRUE(TLval <= TMem);
  EXPECT_FALSE(TInt <= TMem);

  EXPECT_EQ(TBottom, TPtr & TLval);
  EXPECT_EQ(TBottom, TPtrToFrame & TLvalToFrame);
  EXPECT_EQ(TPtrToFrame, TPtrToFrame & TMem);

  EXPECT_EQ(TPtrToFrame, TMemToFrame - TLvalToFrame);
  EXPECT_EQ(TLvalToFrame, TMemToFrame - TPtrToFrame);

  auto const t = TInt | TPtrToFrame | TLvalToFrame;
  EXPECT_EQ(TInt, t - TMemToFrame);
  EXPECT_EQ(TPtrToFrame, t - (TInt | TLvalToFrame));
  EXPECT_EQ(TLvalToFrame, t - (TInt | TPtrToFrame));
  EXPECT_EQ(TMemToFrame, t - TInt);
  EXPECT_EQ(TInt | TLvalToFrame, t - TPtrToFrame);
  EXPECT_EQ(TInt | TPtrToFrame, t - TLvalToFrame);

  EXPECT_EQ(TLval, TMem - TPtr);
  EXPECT_EQ(TPtr, TMem - TLval);
  EXPECT_EQ(TBottom, TPtr - TMem);
  EXPECT_EQ(TBottom, TLval - TMem);
  EXPECT_EQ(TInt, (TInt | TLval) - TMem);
  EXPECT_EQ(TLval, TLval - TPtr);
  EXPECT_EQ(TPtr, TPtr - TLval);
  EXPECT_EQ(TInt, TInt - TMem);

  EXPECT_EQ(TBottom, TPtrToFrame - TMemToFrame);
  EXPECT_EQ(TPtrToFrame, TPtrToFrame - TLvalToFrame);
  EXPECT_EQ(TLvalToFrame, TLvalToFrame - TPtrToFrame);
  EXPECT_EQ(TPtrToFrame, TPtrToFrame - TPtrToStk);
  EXPECT_EQ(TMemToFrame, (TMemToFrame | TMemToStk) - TPtrToStk);
  EXPECT_EQ(TLvalToFrame, TMemToFrame - TPtrToFrame);
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
  EXPECT_FALSE(fits(TArrLike, vanillaConstraint));
  EXPECT_FALSE(fits(TVec, vanillaConstraint));
  EXPECT_FALSE(fits(TVanillaArrLike, vanillaConstraint));
  EXPECT_TRUE(fits(TVanillaVec, vanillaConstraint));
}

TEST(Type, RelaxType) {
  auto gc = GuardConstraint{DataTypeSpecialized};
  gc.setDesiredClass(SystemLib::getHH_IteratorClass());
  gc.category = DataTypeSpecialized;
  auto subIter = Type::SubObj(SystemLib::getHH_IteratorClass());
  EXPECT_EQ("Obj<=HH\\Iterator", subIter.toString());
  EXPECT_EQ(subIter, relaxType(subIter, gc.category));
}

TEST(Type, RelaxConstraint) {
  EXPECT_EQ(GuardConstraint(DataTypeCountnessInit),
            relaxConstraint(GuardConstraint{DataTypeSpecific}, TCell, TDict));
}

TEST(Type, Specialized) {
  EXPECT_LE(TVanillaArrLike, TArrLike);
  EXPECT_LT(TVanillaArrLike, TArrLike);
  EXPECT_FALSE(TArrLike <= TVanillaArrLike);
  EXPECT_LT(TVanillaArrLike, TArrLike | TObj);
  EXPECT_EQ(TVanillaArrLike, TVanillaArrLike & (TArrLike | TCounted));
  EXPECT_GE(TVanillaArrLike, TBottom);
  EXPECT_GT(TVanillaArrLike, TBottom);

  EXPECT_TRUE(TInt <= (TVanillaArrLike | TInt));

  EXPECT_EQ(TBottom, TVanillaArrLike & TObj);
  EXPECT_EQ(TBottom, TVanillaArrLike - TArrLike);

  auto const vecData = ArrayData::GetScalarArray(make_vec_array(1, 2, 3, 4));
  auto const dictData = ArrayData::GetScalarArray(make_dict_array(1, 1, 2, 2));
  auto const constVec = Type::cns(vecData);
  auto const constDict = Type::cns(dictData);

  // Basic checks on constant array types.

  EXPECT_TRUE(constDict.hasConstVal());
  EXPECT_TRUE(constDict <= TArrLike);
  EXPECT_TRUE(constDict <= TDict);
  EXPECT_TRUE(constDict < TDict);
  EXPECT_TRUE(constDict <= TStaticDict);
  EXPECT_TRUE(constDict < TStaticDict);
  EXPECT_FALSE(constDict <= TVec);

  EXPECT_TRUE(constVec.hasConstVal());
  EXPECT_TRUE(constVec <= TArrLike);
  EXPECT_TRUE(constVec <= TVec);
  EXPECT_TRUE(constVec < TVec);
  EXPECT_TRUE(constVec <= TStaticVec);
  EXPECT_TRUE(constVec < TStaticVec);
  EXPECT_FALSE(constVec <= TDict);

  // For some difference types, we are pessimistic: we had better not narrow
  // these differences to TBottom, but we can't represent them finely.
  EXPECT_EQ(TStaticVec, TStaticVec - constVec);
  EXPECT_EQ(TBottom, constVec - TStaticVec);
  EXPECT_EQ(constDict, constDict - TStaticVec);
  EXPECT_EQ(TBottom, constDict - TDict);

  // Checking specialization dropping.  We cannot specialize on two dimensions
  // (e.g. array-like and object) at the same time.
  EXPECT_EQ(TStaticVec | TObj, constVec | TObj);
  auto const subIter = Type::SubObj(SystemLib::getHH_IteratorClass());
  EXPECT_EQ(TVec | TObj, TVec | subIter);

  auto const vecOrInt = TVec | TInt;
  EXPECT_EQ(TInt, vecOrInt - TArrLike);
  EXPECT_EQ(TInt, vecOrInt - TVec);
  EXPECT_EQ(TVec, vecOrInt - TInt);

  auto const iterOrStr = subIter | TStr;
  EXPECT_EQ(TStr, iterOrStr - TObj);
  EXPECT_EQ(TStr, iterOrStr - subIter);
  EXPECT_EQ(subIter, iterOrStr - TStr);

  auto const subCls = Type::SubCls(SystemLib::getHH_IteratorClass());
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

  auto const rat1 = RepoAuthType::Array::tuple(nonempty, {int_rat, int_rat});
  auto const rat2 = RepoAuthType::Array::tuple(maybe_empty, {int_rat, str_rat});
  auto const rat3 = RepoAuthType::Array::packed(nonempty, int_rat);

  auto const ratType1 = Type::Vec(rat1);
  auto const ratType2 = Type::Vec(rat2);
  auto const ratType3 = Type::Vec(rat3);

  auto const vec1 = ArrayData::GetScalarArray(make_vec_array(2, 3));
  auto const vec2 = ArrayData::GetScalarArray(make_vec_array(2, ""));
  auto const vec3 = ArrayData::GetScalarArray(make_vec_array(2, 3, 5));

  DictInit dict_init1{2};
  dict_init1.set((int64_t)0, 2);
  dict_init1.set((int64_t)1, 3);
  auto const dict1 = ArrayData::GetScalarArray(dict_init1.toArray());

  DictInit dict_init2{2};
  dict_init2.set(17, 2);
  dict_init2.set(19, 3);
  auto const dict2 = ArrayData::GetScalarArray(dict_init2.toArray());

  EXPECT_FALSE(Type::cns(staticEmptyVec()) <= ratType1);
  EXPECT_TRUE(Type::cns(staticEmptyVec()) <= ratType2);
  EXPECT_FALSE(Type::cns(staticEmptyVec()) <= ratType3);

  EXPECT_TRUE(Type::cns(vec1) <= ratType1);
  EXPECT_FALSE(Type::cns(vec1) <= ratType2);
  EXPECT_TRUE(Type::cns(vec1) <= ratType3);

  EXPECT_FALSE(Type::cns(vec2) <= ratType1);
  EXPECT_TRUE(Type::cns(vec2) <= ratType2);
  EXPECT_FALSE(Type::cns(vec2) <= ratType3);

  EXPECT_FALSE(Type::cns(vec3) <= ratType1);
  EXPECT_FALSE(Type::cns(vec3) <= ratType2);
  EXPECT_TRUE(Type::cns(vec3) <= ratType3);

  EXPECT_FALSE(Type::cns(dict1) <= ratType1);
  EXPECT_FALSE(Type::cns(dict1) <= ratType2);
  EXPECT_FALSE(Type::cns(dict1) <= ratType3);

  EXPECT_FALSE(Type::cns(dict2) <= ratType1);
  EXPECT_FALSE(Type::cns(dict2) <= ratType2);
  EXPECT_FALSE(Type::cns(dict2) <= ratType3);

  EXPECT_LE(TUninit, ratType1 | TNull);
  EXPECT_FALSE((ratType1 | TNull) <= TUninit);

  EXPECT_LE(TUninit, ratType3 | TNull);
  EXPECT_FALSE((ratType3 | TNull) <= TUninit);
}

TEST(Type, SpecializedArrays) {
  EXPECT_FALSE(TArrLike.isSpecialized());
  EXPECT_FALSE(TArrLike.arrSpec());
  EXPECT_FALSE(TArrLike.arrSpec().vanilla());

  auto const const_array = Type::cns(staticEmptyVec());
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
  auto const A = SystemLib::getHH_IteratorClass();
  auto const B = SystemLib::getHH_TraversableClass();
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

TEST(Type, SpecializedClass) {
  auto const A = SystemLib::getHH_IteratorClass();
  auto const B = SystemLib::getHH_TraversableClass();

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

  EXPECT_LE(TUninit, exactA | TNull);
  EXPECT_FALSE((exactA | TNull) <= TUninit);

  EXPECT_LE(TUninit, subA | TNull);
  EXPECT_FALSE((subA | TNull) <= TUninit);
}

TEST(Type, Const) {
  auto five = Type::cns(5);
  auto fiveArr = five | TArrLike;
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
  EXPECT_EQ(five, five - TArrLike);
  EXPECT_EQ(five, five - Type::cns(1));
  EXPECT_EQ(TInt, TInt - five); // conservative
  EXPECT_EQ(TInt, fiveArr - TArrLike);
  EXPECT_EQ(fiveArr, fiveArr - five);
  EXPECT_EQ(TArrLike, fiveArr - TInt);
  EXPECT_EQ(TBottom, five - TInt);
  EXPECT_EQ(TBottom, five - five);
  EXPECT_EQ(TPtr, (TPtr|TNullptr) - TNullptr);
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

  auto const arrData = ArrayData::GetScalarArray(make_dict_array(1, 2, 3, 4));
  auto const constArray = Type::cns(arrData);

  EXPECT_EQ(constArray, constArray & TDict);
  EXPECT_TRUE(constArray <= constArray);
  EXPECT_TRUE(constArray <= TDict);
  EXPECT_TRUE(constArray < TDict);
  EXPECT_FALSE(TDict <= constArray);
  EXPECT_FALSE(TDict <= TVec);

  auto const rat1 = RepoAuthType::Array::packed(RepoAuthType::Array::Empty::No,
                                                RepoAuthType(RepoAuthType::Tag::Str));
  auto const ratArray1 = Type::Dict(rat1);
  auto const rat2 = RepoAuthType::Array::packed(RepoAuthType::Array::Empty::No,
                                                RepoAuthType(RepoAuthType::Tag::Int));
  auto const ratArray2 = Type::Dict(rat2);

  // Read function chooseRATArray in hphp/runtime/vm/jit/type-specialization.cpp
  // to understand the behavior of this intersection
  EXPECT_EQ(ratArray2, ratArray1 & ratArray2);
  EXPECT_EQ(ratArray2, ratArray2 & ratArray1);
  EXPECT_TRUE(ratArray1 <= TArrLike);
  EXPECT_TRUE(ratArray1 < TArrLike);
  EXPECT_TRUE(ratArray1 <= TDict);
  EXPECT_TRUE(ratArray1 < TDict);
  EXPECT_TRUE(ratArray1 <= ratArray1);
  EXPECT_TRUE(ratArray1 < (TDict|TObj));
  EXPECT_FALSE(ratArray1 < ratArray2);
  EXPECT_NE(ratArray1, ratArray2);

  auto const vanillaRat = ratArray1 & TVanillaArrLike;
  EXPECT_EQ("ArrLike=Vanilla", TVanillaArrLike.toString());
  EXPECT_EQ("Dict=Vanilla:N([Str])", vanillaRat.toString());
  EXPECT_TRUE(vanillaRat <= TVanillaArrLike);
  EXPECT_TRUE(vanillaRat < TVanillaArrLike);
  EXPECT_TRUE(vanillaRat <= TVanillaDict);
  EXPECT_TRUE(vanillaRat < TVanillaDict);
  EXPECT_TRUE(vanillaRat <= ratArray1);
  EXPECT_TRUE(vanillaRat < ratArray1);
  EXPECT_TRUE(vanillaRat.arrSpec().vanilla());

  auto const narrowedRat = ratArray1.narrowToVanilla();
  EXPECT_EQ("Dict=N([Str])", ratArray1.toString());
  EXPECT_EQ("Dict=Vanilla:N([Str])", narrowedRat.toString());
  EXPECT_TRUE(narrowedRat < ratArray1);
  EXPECT_TRUE(narrowedRat <= ratArray1);
  EXPECT_FALSE(ratArray1 < narrowedRat);
  EXPECT_FALSE(ratArray1 <= narrowedRat);
  EXPECT_EQ(narrowedRat, ratArray1 & TVanillaArrLike);
  EXPECT_FALSE(ratArray1.arrSpec().vanilla());

  auto const vec = make_vec_array(1, 2, 3, 4);
  auto const vecData = ArrayData::GetScalarArray(std::move(vec));
  auto const constVec = Type::cns(vecData);
  EXPECT_TRUE(constVec < TVec);

  auto const dict = make_dict_array(1, 1, 2, 2, 3, 3, 4, 4);
  auto const dictData = ArrayData::GetScalarArray(std::move(dict));
  auto const constDict = Type::cns(dictData);
  EXPECT_TRUE(constDict < TDict);

  auto const ratArray3 = Type::StaticDict(rat2);
  auto const dict2 = make_dict_array(0, 0, 1, 1, 2, 2, 3, 3, 4, 4);
  auto const dictData2 = ArrayData::GetScalarArray(std::move(dict2));
  auto const constDict2 = Type::cns(dictData2);
  EXPECT_TRUE(constDict2 < ratArray3);
  EXPECT_EQ(constDict2 | ratArray3, ratArray3);
  EXPECT_EQ(ratArray3 | constDict2, ratArray3);
  EXPECT_EQ(constDict2 & ratArray3, constDict2);
  EXPECT_EQ(ratArray3 & constDict2, constDict2);

  auto const keyset = make_keyset_array(1, 2, 3, 4);
  auto const keysetData = ArrayData::GetScalarArray(std::move(keyset));
  auto const constKeyset = Type::cns(keysetData);
  EXPECT_TRUE(constKeyset < TKeyset);

  auto constLazyCls = Type::cns(LazyClassData::create(makeStaticString("Foo")));
  EXPECT_TRUE(constLazyCls < TLazyCls);
  EXPECT_TRUE(constLazyCls.hasConstVal());
  EXPECT_FALSE(TLazyCls.hasConstVal());
  EXPECT_EQ(TLazyCls | TBool, constLazyCls | True);
  EXPECT_EQ(TBottom, constLazyCls & True);
}

TEST(Type, NarrowToVanilla) {
  EXPECT_EQ("Vec=Vanilla", TVec.narrowToVanilla().toString());
  EXPECT_EQ("{Dict|Vec}=Vanilla", (TVec|TDict).narrowToVanilla().toString());
  EXPECT_EQ("{Vec=Vanilla|Int}", (TVec|TInt).narrowToVanilla().toString());
  EXPECT_EQ("{Vec|Obj}", (TVec|TObj).narrowToVanilla().toString());
}

TEST(Type, VanillaArray) {
  EXPECT_EQ("ArrLike=Vanilla", TVanillaArrLike.toString());
  EXPECT_TRUE(TVanillaArrLike <= TArrLike);
  EXPECT_TRUE(TVanillaArrLike < TArrLike);
  EXPECT_FALSE(TVanillaArrLike.arrSpec().type());
  EXPECT_TRUE(TVanillaArrLike.arrSpec().vanilla());
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
  auto const foo_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("foo", {ArrayLayout::Bespoke()})
  };
  auto const bar_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("bar", {ArrayLayout::Bespoke()})
  };

  auto const vecFoo = TVec.narrowToLayout(foo_layout);
  EXPECT_EQ("Vec=foo", vecFoo.toString());
  EXPECT_FALSE(TVec <= vecFoo);
  EXPECT_FALSE(TVanillaVec <= vecFoo);
  EXPECT_FALSE(vecFoo <= TVanillaVec);
  EXPECT_EQ(vecFoo | TVanillaVec, TVec);
  EXPECT_EQ(TVanillaVec & vecFoo, TBottom);
  EXPECT_EQ(vecFoo | TVec, TVec);
  EXPECT_EQ(vecFoo & TVec, vecFoo);

  auto const vecBar = TVec.narrowToLayout(bar_layout);
  EXPECT_EQ("Vec=bar", vecBar.toString());

  auto const vecVanillaBar = TVanillaVec.narrowToLayout(bar_layout);
  EXPECT_EQ(TBottom, vecVanillaBar);
}

TEST(Type, BespokeVecRAT) {
  RO::EvalBespokeArrayLikeMode = 2;
  auto const foo_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("foo", {ArrayLayout::Bespoke()})
  };
  auto const bar_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("bar", {ArrayLayout::Bespoke()})
  };
  bespoke::selectBespokeLayouts();

  auto const rat = RepoAuthType::Array::packed(RepoAuthType::Array::Empty::No,
                                               RepoAuthType(RepoAuthType::Tag::Str));
  auto const vecRat = Type::Vec(rat);
  EXPECT_EQ("Vec=N([Str])", vecRat.toString());
  auto const vecRatBespoke = vecRat.narrowToLayout(foo_layout);
  EXPECT_EQ("Vec=foo:N([Str])", vecRatBespoke.toString());
  auto const vecRatVanilla = vecRat.narrowToVanilla();
  EXPECT_EQ("Vec=Vanilla:N([Str])", vecRatVanilla.toString());

  EXPECT_EQ(TBottom, vecRatBespoke.narrowToVanilla());
  EXPECT_EQ(TBottom, vecRatVanilla.narrowToLayout(foo_layout));
  EXPECT_EQ(TBottom, vecRatBespoke.narrowToLayout(bar_layout));
}

TEST(Type, VanillaVecRAT) {
  auto const rat = RepoAuthType::Array::packed(RepoAuthType::Array::Empty::No,
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

TEST(Type, BespokeHierarchy) {
  /*
   *    top(L)
   *   /     \
   *  foo(L)  baz
   *  |   \   / \
   * bar  bat(L) ter
   */
  RO::EvalBespokeArrayLikeMode = 2;
  bespoke::Layout::ClearHierarchy();
  auto const foo_layout = ArrayLayout{
    bespoke::testing::makeDummyAbstractLayout("foo", {ArrayLayout::Bespoke()})
  };
  auto const baz_layout = ArrayLayout{
    bespoke::testing::makeDummyAbstractLayout("baz", {ArrayLayout::Bespoke()})
  };
  auto const bar_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("bar", {foo_layout})
  };
  auto const bat_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("bat", {foo_layout, baz_layout})
  };
  auto const ter_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("ter", {baz_layout})
  };
  bespoke::selectBespokeLayouts();

  auto const top = TVec.narrowToLayout(ArrayLayout::Bespoke());
  auto const foo = TVec.narrowToLayout(foo_layout);
  auto const baz = TVec.narrowToLayout(baz_layout);
  auto const bar = TVec.narrowToLayout(bar_layout);
  auto const bat = TVec.narrowToLayout(bat_layout);
  auto const ter = TVec.narrowToLayout(ter_layout);
  auto const bot = TBottom;

  // Subtypes
  EXPECT_TRUE(top <= top);
  EXPECT_TRUE(foo <= top);
  EXPECT_TRUE(baz <= top);
  EXPECT_TRUE(bar <= top);
  EXPECT_TRUE(bat <= top);
  EXPECT_TRUE(ter <= top);

  EXPECT_FALSE(top <= foo);
  EXPECT_TRUE(foo <= foo);
  EXPECT_FALSE(baz <= foo);
  EXPECT_TRUE(bar <= foo);
  EXPECT_TRUE(bat <= foo);
  EXPECT_FALSE(ter <= foo);

  EXPECT_FALSE(top <= bar);
  EXPECT_FALSE(foo <= bar);
  EXPECT_FALSE(baz <= bar);
  EXPECT_TRUE(bar <= bar);
  EXPECT_FALSE(bat <= bar);
  EXPECT_FALSE(ter <= bar);

  EXPECT_FALSE(top <= bat);
  EXPECT_FALSE(foo <= bat);
  EXPECT_FALSE(baz <= bat);
  EXPECT_FALSE(bar <= bat);
  EXPECT_TRUE(bat <= bat);
  EXPECT_FALSE(ter <= bat);

  EXPECT_FALSE(top <= baz);
  EXPECT_FALSE(foo <= baz);
  EXPECT_TRUE(baz <= baz);
  EXPECT_FALSE(bar <= baz);
  EXPECT_TRUE(bat <= baz);
  EXPECT_TRUE(ter <= baz);

  EXPECT_FALSE(top <= ter);
  EXPECT_FALSE(foo <= ter);
  EXPECT_FALSE(baz <= ter);
  EXPECT_FALSE(bar <= ter);
  EXPECT_FALSE(bat <= ter);
  EXPECT_TRUE(ter <= ter);

  EXPECT_TRUE(bot <= top);
  EXPECT_TRUE(bot <= foo);
  EXPECT_TRUE(bot <= baz);
  EXPECT_TRUE(bot <= bar);
  EXPECT_TRUE(bot <= bat);
  EXPECT_TRUE(bot <= ter);

  // Joins
  EXPECT_EQ(top, top | top);
  EXPECT_EQ(top, top | foo);
  EXPECT_EQ(top, top | baz);
  EXPECT_EQ(top, top | bar);
  EXPECT_EQ(top, top | bat);
  EXPECT_EQ(top, top | ter);

  EXPECT_EQ(top, foo | top);
  EXPECT_EQ(foo, foo | foo);
  EXPECT_EQ(top, foo | baz);
  EXPECT_EQ(foo, foo | bar);
  EXPECT_EQ(foo, foo | bat);
  EXPECT_EQ(top, foo | ter);

  EXPECT_EQ(top, baz | top);
  EXPECT_EQ(top, baz | foo);
  EXPECT_EQ(baz, baz | baz);
  EXPECT_EQ(top, baz | bar);
  EXPECT_EQ(baz, baz | bat);
  EXPECT_EQ(baz, baz | ter);

  EXPECT_EQ(top, bar | top);
  EXPECT_EQ(foo, bar | foo);
  EXPECT_EQ(top, bar | baz);
  EXPECT_EQ(bar, bar | bar);
  EXPECT_EQ(foo, bar | bat);
  EXPECT_EQ(top, bar | ter);

  EXPECT_EQ(top, bat | top);
  EXPECT_EQ(foo, bat | foo);
  EXPECT_EQ(baz, bat | baz);
  EXPECT_EQ(foo, bat | bar);
  EXPECT_EQ(bat, bat | bat);
  EXPECT_EQ(baz, bat | ter);

  EXPECT_EQ(top, ter | top);
  EXPECT_EQ(top, ter | foo);
  EXPECT_EQ(baz, ter | baz);
  EXPECT_EQ(top, ter | bar);
  EXPECT_EQ(baz, ter | bat);
  EXPECT_EQ(ter, ter | ter);

  // Meets
  EXPECT_EQ(top, top & top);
  EXPECT_EQ(foo, top & foo);
  EXPECT_EQ(baz, top & baz);
  EXPECT_EQ(bar, top & bar);
  EXPECT_EQ(bat, top & bat);
  EXPECT_EQ(ter, top & ter);

  EXPECT_EQ(foo, foo & top);
  EXPECT_EQ(foo, foo & foo);
  EXPECT_EQ(bat, foo & baz);
  EXPECT_EQ(bar, foo & bar);
  EXPECT_EQ(bat, foo & bat);
  EXPECT_EQ(bot, foo & ter);

  EXPECT_EQ(baz, baz & top);
  EXPECT_EQ(bat, baz & foo);
  EXPECT_EQ(baz, baz & baz);
  EXPECT_EQ(bot, baz & bar);
  EXPECT_EQ(bat, baz & bat);
  EXPECT_EQ(ter, baz & ter);

  EXPECT_EQ(bar, bar & top);
  EXPECT_EQ(bar, bar & foo);
  EXPECT_EQ(bot, bar & baz);
  EXPECT_EQ(bar, bar & bar);
  EXPECT_EQ(bot, bar & bat);
  EXPECT_EQ(bot, bar & ter);

  EXPECT_EQ(bat, bat & top);
  EXPECT_EQ(bat, bat & foo);
  EXPECT_EQ(bat, bat & baz);
  EXPECT_EQ(bot, bat & bar);
  EXPECT_EQ(bat, bat & bat);
  EXPECT_EQ(bot, bat & ter);

  EXPECT_EQ(ter, ter & top);
  EXPECT_EQ(bot, ter & foo);
  EXPECT_EQ(ter, ter & baz);
  EXPECT_EQ(bot, ter & bar);
  EXPECT_EQ(bot, ter & bat);
  EXPECT_EQ(ter, ter & ter);
}

TEST(Type, BespokeRanges) {
  /*
   *    top(L)
   *   /     \
   *  foo(L)  baz
   *  |   \   / \
   * bar  bat(L) ter
   *  |
   * qop
   */
  RO::EvalBespokeArrayLikeMode = 2;
  bespoke::Layout::ClearHierarchy();
  auto const foo_layout = ArrayLayout{
    bespoke::testing::makeDummyAbstractLayout("foo", {ArrayLayout::Bespoke()})
  };
  auto const baz_layout = ArrayLayout{
    bespoke::testing::makeDummyAbstractLayout("baz", {ArrayLayout::Bespoke()})
  };
  auto const bar_layout = ArrayLayout{
    bespoke::testing::makeDummyAbstractLayout("bar", {foo_layout})
  };
  auto const bat_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("bat", {foo_layout, baz_layout})
  };
  auto const ter_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("ter", {baz_layout})
  };
  auto const qop_layout = ArrayLayout{
    bespoke::testing::makeDummyLayout("qup", {bar_layout})
  };
  bespoke::selectBespokeLayouts();

  foo_layout.bespokeLayoutTest();
  baz_layout.bespokeLayoutTest();
  bar_layout.bespokeLayoutTest();
  bat_layout.bespokeLayoutTest();
  ter_layout.bespokeLayoutTest();
  qop_layout.bespokeLayoutTest();
}

TEST(Type, PtrKinds) {
  EXPECT_EQ("PtrToFrame", TPtrToFrame.toString());
  EXPECT_EQ("Ptr", TPtr.toString());
  EXPECT_EQ("PtrToStk", TPtrToStk.toString());
  EXPECT_EQ("Nullptr|PtrToProp",
    (TPtrToProp|TNullptr).toString());
  EXPECT_EQ("Nullptr|PtrToElem",
    (TPtrToElem|TNullptr).toString());

  EXPECT_EQ(PtrLocation::Elem, TPtrToElem.ptrLocation());

  EXPECT_TRUE(TPtrToFrame <= TPtr);
  EXPECT_TRUE(TPtrToFrame.maybe(TPtr));
  EXPECT_TRUE(TPtr.maybe(TPtrToFrame));
  EXPECT_EQ(TPtr, TPtrToFrame | TPtr);

  EXPECT_EQ(TBottom, TPtrToFrame & TPtrToStk);
  EXPECT_EQ(TPtrToFrame, TPtrToFrame & TPtr);

  EXPECT_EQ(PtrLocation::Prop,
            (TPtrToProp|TNullptr).ptrLocation());
  EXPECT_EQ(TPtrToProp,
            (TPtrToProp|TNullptr) - TNullptr);
  EXPECT_EQ(PtrLocation::Elem,
            (TPtrToElem|TNullptr).ptrLocation());
  EXPECT_EQ(TPtrToElem,
            (TPtrToElem|TNullptr) - TNullptr);

  auto const a1 = TPtrToFrame | TInt | TStr;
  auto const a2 = TPtrToStk | TInt | TStr;
  EXPECT_EQ(TInt | TStr, a1 & a2);
}

TEST(Type, PtrToLval) {
  EXPECT_EQ(TPtr.ptrToLval(), TLval);
  EXPECT_EQ(TLval.ptrToLval(), TLval);
  EXPECT_EQ(TMem.ptrToLval(), TLval);

  EXPECT_EQ(TPtrToFrame.ptrToLval(), TLvalToFrame);
  EXPECT_EQ(TLvalToFrame.ptrToLval(), TLvalToFrame);
  EXPECT_EQ(TMemToFrame.ptrToLval(), TLvalToFrame);

  EXPECT_EQ(TInt.ptrToLval(), TInt);
  EXPECT_EQ((TPtr | TInt).ptrToLval(), TLval | TInt);
  EXPECT_EQ((TLval | TInt).ptrToLval(), TLval | TInt);
  EXPECT_EQ((TMem | TInt).ptrToLval(), TLval | TInt);

  EXPECT_EQ((TPtrToFrame | TPtrToStk).ptrToLval(),
            TLvalToFrame | TLvalToStk);
  EXPECT_EQ((TLvalToFrame | TLvalToStk).ptrToLval(),
            TLvalToFrame | TLvalToStk);
  EXPECT_EQ((TMemToFrame | TMemToStk).ptrToLval(),
            TLvalToFrame | TLvalToStk);
}

} }
