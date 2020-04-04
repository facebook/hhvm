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

#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-structure-helpers.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/zend-functions.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/hh/ext_hh.h"
#include "hphp/runtime/ext/std/ext_std_function.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/portability.h"
#include "hphp/util/string-vsnprintf.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

namespace jit {

//////////////////////////////////////////////////////////////////////

ArrayData* addNewElemHelper(ArrayData* a, TypedValue value) {
  assertx(a->isPHPArrayType());

  auto r = a->append(*tvAssertPlausible(&value));
  if (UNLIKELY(r != a)) {
    decRefArr(a);
  }
  return r;
}

ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2) {
  assertx(a1->isPHPArrayType());
  assertx(a2->isPHPArrayType());

  if (checkHACArrayPlus()) raiseHackArrCompatAdd();

  if (!a2->empty()) {
    if (a1->empty()) {
      // We consume refs on a2 and also produce references, so there's
      // no need to inc/dec a2.
      decRefArr(a1);
      return a2;
    }
    if (a1 != a2) {
      auto const escalated = a1->plusEq(a2);
      if (escalated != a1) {
        decRefArr(a2);
        decRefArr(a1);
        return escalated;
      }
    }
  }
  decRefArr(a2);
  return a1;
}

ArrayData* convTVToArrHelper(TypedValue tv) {
  // Note: the call sites of this function all assume that
  // no user code will run and no recoverable exceptions will
  // occur while running this code. This seems trivially true
  // in all cases but converting objects to arrays. It also
  // seems true for that case as well, since the resulting array
  // is essentially metadata for the object. If that is not true,
  // you might end up looking at this code in a debugger and now
  // you know why.
  tvCastToArrayInPlace(&tv); // consumes a ref on counted values
  return tv.m_data.parr;
}

ArrayData* convArrToNonDVArrHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArrayType());
  if (adIn->isNotDVArray()) return adIn;
  auto a = adIn->toPHPArray(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convVecToArrHelper(ArrayData* adIn) {
  assertx(adIn->isVecArrayKind());
  auto a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isPHPArrayType());
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convDictToArrHelper(ArrayData* adIn) {
  assertx(adIn->isDictKind());
  auto a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isPHPArrayType());
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convKeysetToArrHelper(ArrayData* adIn) {
  assertx(adIn->isKeysetKind());
  auto a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isPHPArrayType());
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convArrToVecHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArrayType());
  auto a = adIn->toVec(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convDictToVecHelper(ArrayData* adIn) {
  assertx(adIn->isDictKind());
  auto a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convKeysetToVecHelper(ArrayData* adIn) {
  assertx(adIn->isKeysetKind());
  auto a = SetArray::ToVec(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convObjToVecHelper(ObjectData* obj) {
  auto a = castObjToVec(obj);
  assertx(a->isVecArrayType());
  decRefObj(obj);
  return a;
}

ArrayData* convArrToDictHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArrayType());
  auto a = adIn->toDict(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convVecToDictHelper(ArrayData* adIn) {
  assertx(adIn->isVecArrayKind());
  auto a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convKeysetToDictHelper(ArrayData* adIn) {
  assertx(adIn->isKeysetKind());
  auto a = SetArray::ToDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToDictHelper(ObjectData* obj) {
  auto a = castObjToDict(obj);
  assertx(a->isDictType());
  decRefObj(obj);
  return a;
}

ArrayData* convArrToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArrayType());
  auto a = adIn->toKeyset(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convVecToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isVecArrayKind());
  auto a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convDictToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isDictKind());
  auto a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToKeysetHelper(ObjectData* obj) {
  auto a = castObjToKeyset(obj);
  assertx(a->isKeysetType());
  decRefObj(obj);
  return a;
}

ArrayData* convClsMethToArrHelper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("array");
  auto a = make_varray(clsmeth->getClsStr(), clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToVArrHelper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("varray");
  auto a = make_varray(clsmeth->getClsStr(), clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToVecHelper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("vec");
  auto a = make_vec_array(clsmeth->getClsStr(), clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToDArrHelper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("darray");
  auto a = make_darray(
    0, clsmeth->getClsStr(), 1, clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToDictHelper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("dict");
  auto a = make_dict_array(
    0, clsmeth->getClsStr(), 1, clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToKeysetHelper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("keyset");
  auto a = make_keyset_array(
    clsmeth->getClsStr(), clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

double convObjToDblHelper(const ObjectData* o) {
  return o->toDouble();
}

double convArrToDblHelper(ArrayData* a) {
  return a->empty() ? 0 : 1;
}

double convStrToDblHelper(const StringData* s) {
  return s->toDouble();
}

double convResToDblHelper(const ResourceHdr* r) {
  return r->getId();
}

double convTVToDblHelper(TypedValue tv) {
  return tvCastToDouble(tv);
}

StringData* convDblToStrHelper(double d) {
  return buildStringData(d);
}

StringData* convIntToStrHelper(int64_t i) {
  return buildStringData(i);
}

StringData* convObjToStrHelper(ObjectData* o) {
  // toString() returns a counted String; detach() it to move ownership
  // of the count to the caller
  return o->invokeToString().detach();
}

StringData* convResToStrHelper(ResourceHdr* r) {
  // toString() returns a counted String; detach() it to move ownership
  // of the count to the caller
  return r->data()->o_toString().detach();
}

void raiseUndefProp(ObjectData* base, const StringData* name) {
  base->raiseUndefProp(name);
}

void raiseClsMethPropConvertNotice(const TypeConstraint* tc,
                                   bool isSProp,
                                   const Class* cls,
                                   const StringData* name) {
  raise_notice(
    "class_meth Compat: %s '%s::%s' declared as type %s, clsmeth "
    "assigned",
    isSProp ? "Static property" : "Property",
    cls->name()->data(),
    name->data(),
    tc->displayName().c_str()
  );
}

void raiseUndefVariable(StringData* nm) {
  raise_notice(Strings::UNDEFINED_VARIABLE, nm->data());
  decRefStr(nm);
}

void raise_error_sd(const StringData *msg) {
  raise_error("%s", msg->data());
}

ALWAYS_INLINE
static bool VerifyTypeSlowImpl(const Class* cls,
                               const Class* constraint,
                               const TypeConstraint* expected) {
  // This helper should only be called for the Object, This, Self, and Parent
  // cases
  assertx(expected->isObject() || expected->isSelf() || expected->isParent()
          || expected->isThis());
  // For the This, Self and Parent cases, we must always have a resolved class
  // for the constraint
  assertx(IMPLIES(
    expected->isSelf() || expected->isParent() || expected->isThis(),
    constraint != nullptr
  ));
  // If we have a resolved class for the constraint, all we have to do is
  // check if the value's class is compatible with it
  if (LIKELY(constraint != nullptr)) {
    if (expected->isThis()) {
      return cls == constraint;
    }
    return cls->classof(constraint);
  }
  // The Self and Parent cases should never reach here because they were
  // handled above
  assertx(expected->isObject());
  // Handle the case where the constraint is a type alias
  return expected->checkTypeAliasObj(cls);
}

ALWAYS_INLINE
static bool verifyRecDescImpl(const RecordDesc* rec,
                              const RecordDesc* constraint,
                              const TypeConstraint* tc) {
  if (constraint) return rec->recordDescOf(constraint);
  return tc->checkTypeAliasRecord(rec);
}

void VerifyParamRecDescImpl(const RecordDesc* rec,
                            const RecordDesc* constraint,
                            const TypeConstraint* tc,
                            int param) {
  if (UNLIKELY(!verifyRecDescImpl(rec, constraint, tc))) {
    VerifyParamTypeFail(param, tc);
  }
}

void VerifyRetRecDescImpl(int32_t id,
                          const RecordDesc* rec,
                          const RecordDesc* constraint,
                          const TypeConstraint* tc,
                          TypedValue tv) {
  if (UNLIKELY(!verifyRecDescImpl(rec, constraint, tc))) {
    VerifyRetTypeFail(id, &tv, tc);
  }
}

void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         const TypeConstraint* expected,
                         int param) {
  if (!VerifyTypeSlowImpl(cls, constraint, expected)) {
    VerifyParamTypeFail(param, expected);
  }
}

void VerifyParamTypeCallable(TypedValue value, int param) {
  if (UNLIKELY(!is_callable(tvAsCVarRef(&value)))) {
    VerifyParamTypeFail(param, nullptr);
  }
}


void VerifyParamTypeFail(int paramNum, const TypeConstraint* tc) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  if (!tc) {
    tc = &func->params()[paramNum].typeConstraint;
  }
  TypedValue* tv = frame_local(ar, paramNum);
  assertx(!tc->check(tv, func->cls()));
  tc->verifyParamFail(func, tv, paramNum);
}

void VerifyRetTypeSlow(int32_t id,
                       const Class* cls,
                       const Class* constraint,
                       const TypeConstraint* expected,
                       TypedValue tv) {
  if (!VerifyTypeSlowImpl(cls, constraint, expected)) {
    VerifyRetTypeFail(id, &tv, expected);
  }
}

void VerifyRetTypeCallable(int32_t id, TypedValue value) {
  if (UNLIKELY(!is_callable(tvAsCVarRef(&value)))) {
    VerifyRetTypeFail(id, &value, nullptr);
  }
}

void VerifyRetTypeFail(int32_t id, TypedValue* tv, const TypeConstraint* tc) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  if (id == TypeConstraint::ReturnId) {
    if (!tc) tc = &func->returnTypeConstraint();
    assertx(!tc->check(tv, func->cls()));
    tc->verifyReturnFail(func, tv);
  } else {
    if (!tc) tc = &func->params()[id].typeConstraint;
    assertx(!tc->check(tv, func->cls()));
    tc->verifyOutParamFail(func, tv, id);
  }
}

void VerifyReifiedLocalTypeImpl(int32_t id, ArrayData* ts) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  TypedValue* param = frame_local(ar, id);
  bool warn = false;
  if (verifyReifiedLocalType(ts, param, tcCouldBeReified(func, id), warn)) {
    return;
  }
  raise_reified_typehint_error(
    folly::sformat(
      "Argument {} passed to {}() must be an instance of {}, {} given",
      id + 1,
      func->fullName()->data(),
      TypeStructure::toString(ArrNR(ts),
        TypeStructure::TSDisplayType::TSDisplayTypeUser).c_str(),
      describe_actual_type(param)
    ), warn
  );
}

void VerifyReifiedReturnTypeImpl(TypedValue cell, ArrayData* ts) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  bool warn = false;
  if (verifyReifiedLocalType(ts, &cell,
        tcCouldBeReified(func, TypeConstraint::ReturnId), warn)) {
    return;
  }
  raise_reified_typehint_error(
    folly::sformat(
      "Value returned from function {}() must be of type {}, {} given",
      func->fullName()->data(),
      TypeStructure::toString(ArrNR(ts),
        TypeStructure::TSDisplayType::TSDisplayTypeUser).c_str(),
      describe_actual_type(&cell)
    ), warn
  );
}

namespace {

ALWAYS_INLINE
TypedValue getDefaultIfNullTV(tv_rval rval, const TypedValue& def) {
  return UNLIKELY(!rval) ? def : rval.tv();
}

NEVER_INLINE
TypedValue arrayIdxSSlow(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArrayType());
  return getDefaultIfNullTV(a->rval(key), def);
}

ALWAYS_INLINE
TypedValue doScan(const MixedArray* arr, StringData* key, TypedValue def) {
  assertx(key->isStatic());
  assertx(arr->keyTypes().mustBeStaticStrs());
  auto used = arr->iterLimit();
  for (auto elm = arr->data(); used; used--, elm++) {
    assertx(elm->hasStrKey());
    assertx(elm->strKey()->isStatic());
    if (key == elm->strKey()) return *elm->datatv();
  }
  return def;
}

}

TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isPHPArrayType());
  return getDefaultIfNullTV(a->rval(key), def);
}

TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArrayType());
  if (!a->isMixedKind()) return arrayIdxSSlow(a, key, def);
  return dictIdxS(a, key, def);
}

TypedValue arrayIdxScan(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArrayType());
  if (!a->isMixedKind()) return arrayIdxSSlow(a, key, def);
  return dictIdxScan(a, key, def);
}

TypedValue dictIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->hasVanillaMixedLayout());
  static_assert(MixedArray::NvGetInt == MixedArray::NvGetIntDict, "");
  return getDefaultIfNullTV(MixedArray::NvGetIntDict(a, key), def);
}

NEVER_INLINE
TypedValue dictIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->hasVanillaMixedLayout());
  static_assert(MixedArray::NvGetStr == MixedArray::NvGetStrDict, "");
  return getDefaultIfNullTV(MixedArray::NvGetStrDict(a, key), def);
}

NEVER_INLINE
TypedValue dictIdxScan(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->hasVanillaMixedLayout());
  auto const ad = MixedArray::asMixed(a);
  if (!ad->keyTypes().mustBeStaticStrs()) return dictIdxS(a, key, def);
  return doScan(ad, key, def);
}

TypedValue keysetIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isKeysetKind());
  return getDefaultIfNullTV(SetArray::NvGetInt(a, key), def);
}

TypedValue keysetIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isKeysetKind());
  return getDefaultIfNullTV(SetArray::NvGetStr(a, key), def);
}

template <bool isFirst>
TypedValue vecFirstLast(ArrayData* a) {
  assertx(a->isVecArrayKind() || a->isPackedKind());
  int64_t idx = isFirst ? 0 : a->size() - 1;
  auto rval = a->rval(idx);
  return UNLIKELY(!rval) ? make_tv<KindOfNull>() : rval.tv();
}

template TypedValue vecFirstLast<true>(ArrayData*);
template TypedValue vecFirstLast<false>(ArrayData*);

template <bool isFirst, bool isKey>
TypedValue arrFirstLast(ArrayData* a) {
  if (UNLIKELY(a->empty())) {
    return make_tv<KindOfNull>();
  }
  auto pos = isFirst ? a->iter_begin() : a->iter_last();
  return isKey ? a->nvGetKey(pos) : a->nvGetVal(pos);
}

template TypedValue arrFirstLast<true, false>(ArrayData*);
template TypedValue arrFirstLast<false, false>(ArrayData*);
template TypedValue arrFirstLast<true, true>(ArrayData*);
template TypedValue arrFirstLast<false, true>(ArrayData*);

TypedValue* getSPropOrNull(const Class* cls,
                           const StringData* name,
                           Class* ctx,
                           bool ignoreLateInit,
                           bool disallowConst) {
  auto const lookup = ignoreLateInit
    ? cls->getSPropIgnoreLateInit(ctx, name)
    : cls->getSProp(ctx, name);
  if (disallowConst && UNLIKELY(lookup.constant)) {
    throw_cannot_modify_static_const_prop(cls->name()->data(), name->data());
  }
  if (UNLIKELY(!lookup.val || !lookup.accessible)) return nullptr;

  return lookup.val;
}

TypedValue* getSPropOrRaise(const Class* cls,
                            const StringData* name,
                            Class* ctx,
                            bool ignoreLateInit,
                            bool disallowConst) {
  auto sprop = getSPropOrNull(cls, name, ctx, ignoreLateInit, disallowConst);
  if (UNLIKELY(!sprop)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(), name->data());
  }
  return sprop;
}

TypedValue* ldGblAddrDefHelper(StringData* name) {
  return g_context->m_globalVarEnv->lookupAdd(name);
}

template <typename T>
static int64_t switchBoundsCheck(T v, int64_t base, int64_t nTargets) {
  // I'm relying on gcc to be smart enough to optimize away the next
  // two lines when T is int64.
  if (int64_t(v) == v) {
    int64_t ival = v;
    if (ival >= base && ival < (base + nTargets)) {
      return ival - base;
    }
  }
  return nTargets + 1;
}

int64_t switchDoubleHelper(double val, int64_t base, int64_t nTargets) {
  return switchBoundsCheck(val, base, nTargets);
}

int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets) {
  int64_t ival;
  double dval;

  [&] {
    switch (s->isNumericWithVal(ival, dval, 1)) {
      case KindOfNull:
        ival = switchBoundsCheck(0, base, nTargets);
        return;
      case KindOfInt64:
        ival = switchBoundsCheck(ival, base, nTargets);
        return;
      case KindOfDouble:
        ival = switchBoundsCheck(dval, base, nTargets);
        return;

      case KindOfUninit:
      case KindOfBoolean:
      case KindOfPersistentString:
      case KindOfString:
      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset:
      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
      case KindOfFunc:
      case KindOfClass:
      case KindOfClsMeth:
      case KindOfRecord:
        break;
    }
    not_reached();
  }();

  decRefStr(s);
  return ival;
}

int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets) {
  auto const ival = o->toInt64();
  decRefObj(o);
  return switchBoundsCheck(ival, base, nTargets);
}

//////////////////////////////////////////////////////////////////////

void checkFrame(ActRec* fp, TypedValue* sp, bool fullCheck) {
  const Func* func = fp->m_func;
  func->validate();
  if (func->cls()) {
    assertx(!func->cls()->isZombie());
  }
  if ((func->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
    assertx(fp->getVarEnv()->getFP() == fp);
  }
  int numLocals = func->numLocals();
  assertx(sp <= (TypedValue*)fp - func->numSlotsInFrame() || isResumed(fp));

  if (!fullCheck) return;

  int numParams = func->numParams();
  for (int i = 0; i < numLocals; i++) {
    if (i >= numParams && isResumed(fp) && i < func->numNamedLocals() &&
        func->localVarName(i)) {
      continue;
    }
    assertx(tvIsPlausible(*frame_local(fp, i)));
  }

  visitStackElems(
    fp, sp,
    [](const TypedValue* tv) {
      assertx(tvIsPlausible(*tv));
    }
  );
}

const Func* loadClassCtor(Class* cls, ActRec* fp) {
  const Func* f = cls->getCtor();
  if (UNLIKELY(!(f->attrs() & AttrPublic))) {
    auto const ctx = arGetContextClass(fp);
    UNUSED auto func =
      lookupMethodCtx(cls, nullptr, ctx, CallType::CtorMethod, true);
    assertx(func == f);
  }
  return f;
}

const Func* lookupClsMethodHelper(const Class* cls, const StringData* methName,
                                  ObjectData* obj, const Class* ctx) {
  const Func* f;
  auto const res = lookupClsMethod(f, cls, methName, obj, ctx, true);

  if (res == LookupResult::MethodFoundWithThis ||
      res == LookupResult::MagicCallFound) {
    // Handled by interpreter.
    return nullptr;
  }

  assertx(res == LookupResult::MethodFoundNoThis);
  if (!f->isStaticInPrologue()) {
    throw_missing_this(f);
  }

  return f;
}

//////////////////////////////////////////////////////////////////////

namespace {

std::string formatArgumentErrMsg(const Func* func, const char* amount,
                                 uint32_t expected, uint32_t got) {
  return folly::sformat(
    "{}() expects {} {} parameter{}, {} given",
    func->fullName()->data(),
    amount,
    expected,
    expected == 1 ? "" : "s",
    got
  );
}

}

void throwMissingArgument(const Func* func, int got) {
  auto const expected = func->numRequiredParams();
  assertx(got < expected);
  auto const amount = expected < func->numParams() ? "at least" : "exactly";
  auto const errMsg = formatArgumentErrMsg(func, amount, expected, got);
  SystemLib::throwRuntimeExceptionObject(Variant(errMsg));
}

void raiseTooManyArguments(const Func* func, int got) {
  assertx(!func->hasVariadicCaptureParam());

  if (!RuntimeOption::EvalWarnOnTooManyArguments && !func->isCPPBuiltin()) {
    return;
  }

  auto const total = func->numNonVariadicParams();
  assertx(got > total);
  auto const amount = func->numRequiredParams() < total ? "at most" : "exactly";
  auto const errMsg = formatArgumentErrMsg(func, amount, total, got);

  if (RuntimeOption::EvalWarnOnTooManyArguments > 1 || func->isCPPBuiltin()) {
    SystemLib::throwRuntimeExceptionObject(Variant(errMsg));
  } else {
    raise_warning(errMsg);
  }
}

void raiseTooManyArgumentsPrologue(const Func* func, ArrayData* unpackArgs) {
  SCOPE_EXIT { decRefArr(unpackArgs); };
  if (unpackArgs->empty()) return;
  auto const got = func->numNonVariadicParams() + unpackArgs->size();
  raiseTooManyArguments(func, got);
}

//////////////////////////////////////////////////////////////////////

Class* lookupClsRDS(const StringData* name) {
  return NamedEntity::get(name)->getCachedClass();
}

bool methodExistsHelper(Class* cls, StringData* meth) {
  assertx(isNormalClass(cls) && !isAbstract(cls));
  return cls->lookupMethod(meth) != nullptr;
}

ArrayData* resolveTypeStructHelper(
  uint32_t n,
  const TypedValue* values,
  const Class* declaringCls,
  const Class* calledCls,
  bool suppress,
  bool isOrAsOp
) {
  assertx(n != 0);
  auto const v = *values;
  isValidTSType(v, true);
  auto const ts = v.m_data.parr;
  req::vector<Array> tsList;
  for (int i = 0; i < n - 1; ++i) {
    auto const a = values[n - i - 1];
    isValidTSType(a, true);
    tsList.emplace_back(Array::attach(a.m_data.parr));
  }
  auto resolved = [&] {
    if (isOrAsOp) {
      return resolveAndVerifyTypeStructure<true>(
               ArrNR(ts), declaringCls, calledCls, tsList, suppress);
    }
    return resolveAndVerifyTypeStructure<false>(
             ArrNR(ts), declaringCls, calledCls, tsList, suppress);
  }();
  return resolved.detach();
}

bool isTypeStructHelper(ArrayData* a, TypedValue c) {
  auto const ts = ArrNR(a);
  return checkTypeStructureMatchesTV(ts, c);
}

void throwAsTypeStructExceptionHelper(ArrayData* a, TypedValue c) {
  std::string givenType, expectedType, errorKey;
  auto const ts = ArrNR(a);
  if (!checkTypeStructureMatchesTV(ts, c, givenType, expectedType,
                                     errorKey)) {
    throwTypeStructureDoesNotMatchTVException(
      givenType, expectedType, errorKey);
  }
  raise_error("Invalid bytecode sequence: Instruction must throw");
}

ArrayData* errorOnIsAsExpressionInvalidTypesHelper(ArrayData* a) {
  errorOnIsAsExpressionInvalidTypes(ArrNR(a), false);
  return a;
}

ArrayData* recordReifiedGenericsAndGetTSList(ArrayData* tsList) {
  auto const mangledName = makeStaticString(mangleReifiedGenericsName(tsList));
  auto result = addToReifiedGenericsTable(mangledName, tsList);
  return result;
}

void throwOOBException(TypedValue base, TypedValue key) {
  if (isArrayLikeType(base.m_type)) {
    throwOOBArrayKeyException(key, base.m_data.parr);
  } else if (base.m_type == KindOfObject) {
    assertx(isIntType(key.m_type));
    collections::throwOOB(key.m_data.num);
  }
  not_reached();
}

void invalidArrayKeyHelper(const ArrayData* ad, TypedValue key) {
  throwInvalidArrayKeyException(&key, ad);
}

namespace MInstrHelpers {

void setNewElemArray(tv_lval base, TypedValue val) {
  HPHP::SetNewElemArray(base, &val);
}

TypedValue setOpElem(tv_lval base, TypedValue key,
                     TypedValue val, SetOpOp op, const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpElem(localTvRef, op, base, key, &val, pState);
  return cGetRefShuffle(localTvRef, result);
}

StringData* stringGetI(StringData* base, uint64_t x) {
  if (LIKELY(x < base->size())) {
    return base->getChar(x);
  }
  raise_notice("Uninitialized string offset: %" PRId64,
               static_cast<int64_t>(x));
  return staticEmptyString();
}

uint64_t pairIsset(c_Pair* pair, int64_t index) {
  auto result = pair->get(index);
  return result ? !tvIsNull(result) : false;
}

uint64_t vectorIsset(c_Vector* vec, int64_t index) {
  auto result = vec->get(index);
  return result ? !tvIsNull(*result) : false;
}

TypedValue incDecElem(tv_lval base, TypedValue key,
                      IncDecOp op, const MInstrPropState* pState) {
  auto const result = HPHP::IncDecElem(op, base, key, pState);
  return result;
}

tv_lval elemVecIU(tv_lval base, int64_t key) {
  assertx(isVecType(type(base)));
  return ElemUVec<KeyType::Int>(base, key);
}

}

//////////////////////////////////////////////////////////////////////

uintptr_t tlsBaseNoInline() {
  return tlsBase();
}

//////////////////////////////////////////////////////////////////////

}}
