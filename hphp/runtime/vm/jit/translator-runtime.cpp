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
#include "hphp/runtime/base/array-iterator.h"
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
  assertx(a->isPHPArray());

  auto r = a->append(*tvAssertCell(&value));
  if (UNLIKELY(r != a)) {
    decRefArr(a);
  }
  return r;
}

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64_t key,
                               TypedValue value) {
  assertx(ad->isPHPArray());
  assertx(cellIsPlausible(value));
  // this does not re-enter
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval = ad->set(key, tvAsCVarRef(&value));
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  tvDecRefGen(&value);
  return arrayRefShuffle<false, KindOfArray>(ad, retval, nullptr);
}

ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  TypedValue value) {
  assertx(ad->isPHPArray());
  assertx(cellIsPlausible(value));
  // set will decRef any old value that may have been overwritten
  // if appropriate
  auto const retval = ad->set(key, *tvToCell(&value));
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  decRefStr(key);
  tvDecRefGen(&value);
  return arrayRefShuffle<false, KindOfArray>(ad, retval, nullptr);
}

ArrayData* dictAddElemIntKeyHelper(ArrayData* ad,
                                   int64_t key,
                                   TypedValue value) {
  assertx(ad->isDict());
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval = MixedArray::SetIntDict(ad, key, *tvAssertCell(&value));
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  tvDecRefGen(&value);
  return arrayRefShuffle<false, KindOfDict>(ad, retval, nullptr);
}

ArrayData* dictAddElemStringKeyHelper(ArrayData* ad,
                                      StringData* key,
                                      TypedValue value) {
  assertx(ad->isDict());
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval = MixedArray::SetStrDict(ad, key, *tvAssertCell(&value));
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  decRefStr(key);
  tvDecRefGen(&value);
  return arrayRefShuffle<false, KindOfDict>(ad, retval, nullptr);
}

ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2) {
  assertx(a1->isPHPArray());
  assertx(a2->isPHPArray());

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

RefData* boxValue(TypedValue tv) {
  assertx(!isRefType(tv.m_type));
  if (tv.m_type == KindOfUninit) tv = make_tv<KindOfNull>();
  return RefData::Make(tv);
}

ArrayData* convCellToArrHelper(TypedValue tv) {
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
  assertx(adIn->isPHPArray());
  if (adIn->isNotDVArray()) return adIn;
  auto a = adIn->toPHPArray(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convVecToArrHelper(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isPHPArray());
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convDictToArrHelper(ArrayData* adIn) {
  assertx(adIn->isDict());
  auto a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isPHPArray());
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convKeysetToArrHelper(ArrayData* adIn) {
  assertx(adIn->isKeyset());
  auto a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  assertx(a->isPHPArray());
  assertx(a->isNotDVArray());
  return a;
}

ArrayData* convArrToVecHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArray());
  auto a = adIn->toVec(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convDictToVecHelper(ArrayData* adIn) {
  assertx(adIn->isDict());
  auto a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convKeysetToVecHelper(ArrayData* adIn) {
  assertx(adIn->isKeyset());
  auto a = SetArray::ToVec(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convObjToVecHelper(ObjectData* obj) {
  auto a = castObjToVec(obj);
  assertx(a->isVecArray());
  decRefObj(obj);
  return a;
}

ArrayData* convArrToDictHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArray());
  auto a = adIn->toDict(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convVecToDictHelper(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convKeysetToDictHelper(ArrayData* adIn) {
  assertx(adIn->isKeyset());
  auto a = SetArray::ToDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToDictHelper(ObjectData* obj) {
  auto a = castObjToDict(obj);
  assertx(a->isDict());
  decRefObj(obj);
  return a;
}

ArrayData* convArrToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isPHPArray());
  auto a = adIn->toKeyset(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convVecToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
  assertx(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convDictToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isDict());
  auto a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToKeysetHelper(ObjectData* obj) {
  auto a = castObjToKeyset(obj);
  assertx(a->isKeyset());
  decRefObj(obj);
  return a;
}

ArrayData* convClsMethToArrHealper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("array");
  auto a = make_varray(clsmeth->getCls(), clsmeth->getFunc()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToVArrHealper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("varray");
  auto a = make_varray(clsmeth->getCls(), clsmeth->getFunc()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToVecHealper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("vec");
  auto a = make_vec_array(clsmeth->getCls(), clsmeth->getFunc()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToDArrHealper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("darray");
  auto a = make_darray(0, clsmeth->getCls(), 1, clsmeth->getFunc()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToDictHealper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("dict");
  auto a = make_dict_array(
    0, clsmeth->getCls(), 1, clsmeth->getFunc()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToKeysetHealper(ClsMethDataRef clsmeth) {
  raiseClsMethConvertWarningHelper("keyset");
  auto a = make_keyset_array(
    const_cast<StringData*>(classToStringHelper(clsmeth->getCls())),
    const_cast<StringData*>(funcToStringHelper(clsmeth->getFunc()))).detach();
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

double convCellToDblHelper(TypedValue tv) {
  return tvCastToDouble(tv);
}

ObjectData* convCellToObjHelper(TypedValue tv) {
  // Note: the call sites of this function all assume that
  // no user code will run and no recoverable exceptions will
  // occur while running this code. This seems trivially true
  // in all cases but converting arrays to objects. It also
  // seems true for that case as well, since the source array
  // is essentially metadata for the object. If that is not true,
  // you might end up looking at this code in a debugger and now
  // you know why.
  tvCastToObjectInPlace(&tv); // consumes a ref on counted values
  return tv.m_data.pobj;
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
    if (expected->isThis() && RuntimeOption::EvalThisTypeHintLevel >= 2) {
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
    VerifyParamTypeFail(param);
  }
}

void VerifyRetRecDescImpl(int32_t id,
                          const RecordDesc* rec,
                          const RecordDesc* constraint,
                          const TypeConstraint* tc,
                          TypedValue tv) {
  if (UNLIKELY(!verifyRecDescImpl(rec, constraint, tc))) {
    VerifyRetTypeFail(id, &tv);
  }
}

void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         const TypeConstraint* expected,
                         int param) {
  if (!VerifyTypeSlowImpl(cls, constraint, expected)) {
    VerifyParamTypeFail(param);
  }
}

void VerifyParamTypeCallable(TypedValue value, int param) {
  if (UNLIKELY(!is_callable(tvAsCVarRef(&value)))) {
    VerifyParamTypeFail(param);
  }
}


void VerifyParamTypeFail(int paramNum) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  auto const& tc = func->params()[paramNum].typeConstraint;
  TypedValue* tv = frame_local(ar, paramNum);
  assertx(!tc.check(tv, func->cls()));
  tc.verifyParamFail(func, tv, paramNum);
}

void VerifyRetTypeSlow(int32_t id,
                       const Class* cls,
                       const Class* constraint,
                       const TypeConstraint* expected,
                       TypedValue tv) {
  if (!VerifyTypeSlowImpl(cls, constraint, expected)) {
    VerifyRetTypeFail(id, &tv);
  }
}

void VerifyRetTypeCallable(int32_t id, TypedValue value) {
  if (UNLIKELY(!is_callable(tvAsCVarRef(&value)))) {
    VerifyRetTypeFail(id, &value);
  }
}

void VerifyRetTypeFail(int32_t id, TypedValue* tv) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  if (id == TypeConstraint::ReturnId) {
    auto const& tc = func->returnTypeConstraint();
    assertx(!tc.check(tv, func->cls()));
    tc.verifyReturnFail(func, tv);
  } else {
    auto const& tc = func->params()[id].typeConstraint;
    assertx(!tc.check(tv, func->cls()));
    tc.verifyOutParamFail(func, tv, id);
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
      TypeStructure::toStringForDisplay(ArrNR(ts)).c_str(),
      describe_actual_type(param, true)
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
      TypeStructure::toStringForDisplay(ArrNR(ts)).c_str(),
      describe_actual_type(&cell, true)
    ), warn
  );
}

namespace {

ALWAYS_INLINE
TypedValue getDefaultIfNullCell(tv_rval rval, const TypedValue& def) {
  return UNLIKELY(!rval) ? def : rval.tv();
}

NEVER_INLINE
TypedValue arrayIdxSSlow(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  return getDefaultIfNullCell(a->rval(key), def);
}

ALWAYS_INLINE
bool isMixedArrayWithStaticKeys(const ArrayData* arr) {
  // In one comparison we check both the kind and the fact that the array only
  // has static string keys (no tombstones, int keys, or counted str keys).
  auto const test = static_cast<uint32_t>(HeaderKind::Mixed);
  auto const mask = static_cast<uint32_t>(~MixedArray::kStaticStrKey) << 24 |
                    static_cast<uint32_t>(0xff);
  return (*(reinterpret_cast<const uint32_t*>(arr) + 1) & mask) == test;
}

ALWAYS_INLINE
TypedValue doScan(const MixedArray* arr, StringData* key, TypedValue def) {
  assertx(key->isStatic());
  assertx((arr->keyTypes() & ~MixedArray::kStaticStrKey) == 0x00);
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
  assertx(a->isPHPArray());
  return getDefaultIfNullCell(a->rval(key), def);
}

TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  if (UNLIKELY(!a->isMixed())) return arrayIdxSSlow(a, key, def);
  return getDefaultIfNullCell(MixedArray::RvalStr(a, key), def);
}

TypedValue arrayIdxScan(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  return LIKELY(isMixedArrayWithStaticKeys(a))
    ? doScan(MixedArray::asMixed(a), key, def)
    : arrayIdxSSlow(a, key, def);
}

TypedValue dictIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isDict());
  return getDefaultIfNullCell(MixedArray::RvalIntDict(a, key), def);
}

NEVER_INLINE
TypedValue dictIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isDict());
  return getDefaultIfNullCell(MixedArray::RvalStrDict(a, key), def);
}

TypedValue dictIdxScan(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isDict());
  auto ad = MixedArray::asMixed(a);
  return LIKELY((ad->keyTypes() & ~MixedArray::kStaticStrKey) == 0)
    ? doScan(ad, key, def)
    : dictIdxS(a, key, def);
}

TypedValue keysetIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isKeyset());
  return getDefaultIfNullCell(SetArray::RvalInt(a, key), def);
}

TypedValue keysetIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isKeyset());
  return getDefaultIfNullCell(SetArray::RvalStr(a, key), def);
}

template <bool isFirst>
TypedValue vecFirstLast(ArrayData* a) {
  assertx(a->isVecArray() || a->isPacked());
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
  return isKey ? a->nvGetKey(pos) : a->atPos(pos);
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
      case KindOfPersistentArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
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

void checkFrame(ActRec* fp, Cell* sp, bool fullCheck) {
  const Func* func = fp->m_func;
  func->validate();
  if (func->cls()) {
    assertx(!func->cls()->isZombie());
  }
  if ((func->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
    assertx(fp->getVarEnv()->getFP() == fp);
  }
  int numLocals = func->numLocals();
  assertx(sp <= (Cell*)fp - func->numSlotsInFrame() || fp->resumed());

  if (!fullCheck) return;

  int numParams = func->numParams();
  for (int i = 0; i < numLocals; i++) {
    if (i >= numParams && fp->resumed() && i < func->numNamedLocals()) {
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

void raiseArgumentImpl(const Func* func, int got, bool missing) {
  if (!missing && !RuntimeOption::EvalWarnOnTooManyArguments &&
      !func->isCPPBuiltin()) {
    return;
  }
  const auto total = func->numNonVariadicParams();
  const auto variadic = func->hasVariadicCaptureParam();
  const Func::ParamInfoVec& params = func->params();
  if (variadic && !missing) return;
  int expected = 0;
  bool atmost = false;
  // We subtract the number of parameters with default value at the end
  for (size_t i = total; i--; ) {
    if (!params[i].hasDefaultValue()) {
      expected = i + 1;
      break;
    }
  }
  auto const amount = [&] {
    if (!missing) {
      atmost = expected < total;
      return atmost ? "at most" : "exactly";
    }
    return variadic || expected < total ? "at least" : "exactly";
  }();

  auto const value = atmost ? total : expected;
  auto const msg = folly::sformat("{}() expects {} {} parameter{}, {} given",
                                  func->fullDisplayName()->data(),
                                  amount,
                                  value,
                                  value == 1 ? "" : "s",
                                  got);

  if (missing || RuntimeOption::EvalWarnOnTooManyArguments > 1 ||
      func->isCPPBuiltin()) {
    SystemLib::throwRuntimeExceptionObject(Variant(msg));
  } else {
    raise_warning(msg);
  }
}

void raiseMissingArgument(const Func* func, int got) {
  raiseArgumentImpl(func, got, true);
}

void raiseTooManyArguments(const Func* func, int got) {
  raiseArgumentImpl(func, got, false);
}

//////////////////////////////////////////////////////////////////////

Class* lookupClsRDS(const StringData* name) {
  return NamedEntity::get(name)->getCachedClass();
}

void throwSwitchMode() {
  // This is only called right after dispatchBB, so the VM regs really are
  // clean.
  tl_regState = VMRegState::CLEAN;
  throw VMSwitchMode();
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

bool isTypeStructHelper(ArrayData* a, Cell c) {
  auto const ts = ArrNR(a);
  return checkTypeStructureMatchesCell(ts, c);
}

void throwAsTypeStructExceptionHelper(ArrayData* a, Cell c) {
  std::string givenType, expectedType, errorKey;
  auto const ts = ArrNR(a);
  if (!checkTypeStructureMatchesCell(ts, c, givenType, expectedType,
                                     errorKey)) {
    throwTypeStructureDoesNotMatchCellException(
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

void setNewElemArray(tv_lval base, Cell val) {
  HPHP::SetNewElemArray(base, &val);
}

TypedValue setOpElem(tv_lval base, TypedValue key,
                     Cell val, SetOpOp op, const MInstrPropState* pState) {
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
  return result ? !cellIsNull(result) : false;
}

uint64_t vectorIsset(c_Vector* vec, int64_t index) {
  auto result = vec->get(index);
  return result ? !cellIsNull(*result) : false;
}

TypedValue incDecElem(tv_lval base, TypedValue key,
                      IncDecOp op, const MInstrPropState* pState) {
  auto const result = HPHP::IncDecElem(op, base, key, pState);
  assertx(!isRefType(result.m_type));
  return result;
}

tv_lval elemVecIU(tv_lval base, int64_t key) {
  auto cbase = tvToCell(base);
  assertx(isVecType(type(cbase)));
  return ElemUVec<KeyType::Int>(cbase, key);
}

}

//////////////////////////////////////////////////////////////////////

uintptr_t tlsBaseNoInline() {
  return tlsBase();
}

//////////////////////////////////////////////////////////////////////

}}
