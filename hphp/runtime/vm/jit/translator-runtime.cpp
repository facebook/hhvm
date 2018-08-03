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
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/method-lookup.h"
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

  auto r = a->append(*tvAssertCell(&value), a->hasMultipleRefs());
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
  ArrayData* retval = ad->set(key, tvAsCVarRef(&value),
                              ad->cowCheck());
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  tvDecRefGen(&value);
  return arrayRefShuffle<false, KindOfArray>(ad, retval, nullptr);
}

template <bool intishWarn>
ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  TypedValue value) {
  assertx(ad->isPHPArray());
  assertx(cellIsPlausible(value));
  // this does not re-enter
  bool copy = ad->cowCheck();
  // set will decRef any old value that may have been overwritten
  // if appropriate
  int64_t intkey;
  ArrayData* retval;
  if (UNLIKELY(key->isStrictlyInteger(intkey))) {
    if (intishWarn) raise_intish_index_cast();
    retval = ad->set(intkey, *tvToCell(&value), copy);
  } else {
    retval = ad->set(key, *tvToCell(&value), copy);
  }
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  decRefStr(key);
  tvDecRefGen(&value);
  return arrayRefShuffle<false, KindOfArray>(ad, retval, nullptr);
}

template ArrayData*
addElemStringKeyHelper<true>(ArrayData*, StringData*, TypedValue);
template ArrayData*
addElemStringKeyHelper<false>(ArrayData*, StringData*, TypedValue);

ArrayData* dictAddElemIntKeyHelper(ArrayData* ad,
                                   int64_t key,
                                   TypedValue value) {
  assertx(ad->isDict());
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval =
    MixedArray::SetIntDict(ad, key, *tvAssertCell(&value), ad->cowCheck());
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
  ArrayData* retval =
    MixedArray::SetStrDict(ad, key, *tvAssertCell(&value), ad->cowCheck());
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

  if (checkHACMisc()) raiseHackArrCompatAdd();

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

inline void coerceCellFail(DataType expected, DataType actual, int64_t argNum,
                           const Func* func) {
  raise_param_type_warning(func->displayName()->data(),
                           argNum, expected, actual);

  throw TVCoercionException(func, argNum, actual, expected);
}

void builtinCoercionWarningHelper(DataType ty, DataType expKind,
                                  const Func* callee, int64_t arg_num) {
  if (RuntimeOption::EvalWarnOnCoerceBuiltinParams &&
      !equivDataTypes(ty, expKind)) {
    raise_warning(
      "Argument %ld of type %s was passed to %s, "
      "it was coerced to %s",
      arg_num,
      getDataTypeString(ty).data(),
      callee->fullDisplayName()->data(),
      getDataTypeString(expKind).data()
    );
  }
}

bool coerceCellToBoolHelper(TypedValue tv, int64_t argNum, const Func* func) {
  assertx(cellIsPlausible(tv));

  tvCoerceIfStrict(tv, argNum, func);

  DataType type = tv.m_type;
  if (isArrayLikeType(type) || type == KindOfObject || type == KindOfResource) {
    coerceCellFail(KindOfBoolean, type, argNum, func);
    not_reached();
  }

  builtinCoercionWarningHelper(tv.m_type, KindOfBoolean, func, argNum);

  return cellToBool(tv);
}

double coerceStrToDblHelper(StringData* sd, int64_t argNum, const Func* func) {
  DataType type = is_numeric_string(sd->data(), sd->size(), nullptr, nullptr);

  if (UNLIKELY(RuntimeOption::PHP7_ScalarTypes)) {
    auto tv = make_tv<KindOfString>(sd);

    // In strict mode this will always fail, in weak mode it will be a noop
    tvCoerceIfStrict(tv, argNum, func);
  }
  if (type != KindOfDouble && type != KindOfInt64) {
    coerceCellFail(KindOfDouble, KindOfString, argNum, func);
    not_reached();
  }

  builtinCoercionWarningHelper(KindOfString, KindOfDouble, func, argNum);

  return sd->toDouble();
}

double coerceCellToDblHelper(Cell tv, int64_t argNum, const Func* func) {
  assertx(cellIsPlausible(tv));

  tvCoerceIfStrict(tv, argNum, func);

  switch (tv.m_type) {
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      builtinCoercionWarningHelper(tv.m_type, KindOfDouble, func, argNum);
      return convCellToDblHelper(tv);

    case KindOfFunc: {
      auto s = funcToStringHelper(tv.m_data.pfunc);
      return coerceStrToDblHelper(const_cast<StringData*>(s), argNum, func);
    }
    case KindOfClass: {
      auto s = classToStringHelper(tv.m_data.pclass);
      return coerceStrToDblHelper(const_cast<StringData*>(s), argNum, func);
    }
    case KindOfPersistentString:
    case KindOfString:
      return coerceStrToDblHelper(tv.m_data.pstr, argNum, func);

    case KindOfUninit:
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
      coerceCellFail(KindOfDouble, tv.m_type, argNum, func);
      break;

    case KindOfRef:
      break;
  }
  not_reached();
}

int64_t coerceStrToIntHelper(StringData* sd, int64_t argNum, const Func* func) {
  DataType type = is_numeric_string(sd->data(), sd->size(), nullptr, nullptr);

  if (UNLIKELY(RuntimeOption::PHP7_ScalarTypes)) {
    auto tv = make_tv<KindOfString>(sd);

    // In strict mode this will always fail, in weak mode it will be a noop
    tvCoerceIfStrict(tv, argNum, func);
  }
  if (type != KindOfDouble && type != KindOfInt64) {
    coerceCellFail(KindOfInt64, KindOfString, argNum, func);
    not_reached();
  }

  builtinCoercionWarningHelper(KindOfString, KindOfInt64, func, argNum);

  return sd->toInt64();
}

int64_t coerceCellToIntHelper(TypedValue tv, int64_t argNum, const Func* func) {
  assertx(cellIsPlausible(tv));

  tvCoerceIfStrict(tv, argNum, func);

  switch (tv.m_type) {
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      builtinCoercionWarningHelper(tv.m_type, KindOfInt64, func, argNum);
      return cellToInt(tv);

    case KindOfFunc: {
      auto s = funcToStringHelper(tv.m_data.pfunc);
      return coerceStrToIntHelper(const_cast<StringData*>(s), argNum, func);
    }

    case KindOfClass: {
      auto s = classToStringHelper(tv.m_data.pclass);
      return coerceStrToIntHelper(const_cast<StringData*>(s), argNum, func);
    }

    case KindOfPersistentString:
    case KindOfString:
      return coerceStrToIntHelper(tv.m_data.pstr, argNum, func);

    case KindOfUninit:
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
      coerceCellFail(KindOfInt64, tv.m_type, argNum, func);
      break;

    case KindOfRef:
      break;
  }
  not_reached();
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

namespace {
ALWAYS_INLINE
TypedValue getDefaultIfNullCell(tv_rval rval, const TypedValue& def) {
  return UNLIKELY(!rval) ? def : rval.tv();
}

template <bool intishWarn>
NEVER_INLINE
TypedValue arrayIdxSiSlow(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  int64_t i;
  if (UNLIKELY(key->isStrictlyInteger(i))) {
    if (intishWarn) raise_intish_index_cast();
    return getDefaultIfNullCell(a->rval(i), def);
  } else {
    return getDefaultIfNullCell(a->rval(key), def);
  }
}

NEVER_INLINE
TypedValue arrayIdxSSlow(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  return getDefaultIfNullCell(a->rval(key), def);
}

}

TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  if (UNLIKELY(!a->isMixed())) return arrayIdxSSlow(a, key, def);
  return getDefaultIfNullCell(MixedArray::RvalStr(a, key), def);
}

template <bool intishWarn>
TypedValue arrayIdxSi(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isPHPArray());
  if (UNLIKELY(!a->isMixed())) return arrayIdxSiSlow<intishWarn>(a, key, def);
  int64_t i;
  if (UNLIKELY(key->isStrictlyInteger(i))) {
    if (intishWarn) raise_intish_index_cast();
    return getDefaultIfNullCell(MixedArray::RvalInt(a, i), def);
  } else {
    return getDefaultIfNullCell(MixedArray::RvalStr(a, key), def);
  }
}

template TypedValue arrayIdxSi<false>(ArrayData*, StringData*, TypedValue);
template TypedValue arrayIdxSi<true>(ArrayData*, StringData*, TypedValue);

TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isPHPArray());
  return getDefaultIfNullCell(a->rval(key), def);
}

TypedValue dictIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isDict());
  return getDefaultIfNullCell(MixedArray::RvalIntDict(a, key), def);
}

TypedValue dictIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isDict());
  return getDefaultIfNullCell(MixedArray::RvalStrDict(a, key), def);
}

TypedValue keysetIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isKeyset());
  return getDefaultIfNullCell(SetArray::RvalInt(a, key), def);
}

TypedValue keysetIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isKeyset());
  return getDefaultIfNullCell(SetArray::RvalStr(a, key), def);
}

TypedValue* getSPropOrNull(const Class* cls,
                           const StringData* name,
                           Class* ctx) {
  auto const lookup = cls->getSProp(ctx, name);

  if (UNLIKELY(!lookup.val || !lookup.accessible)) return nullptr;

  return lookup.val;
}

TypedValue* getSPropOrRaise(const Class* cls,
                            const StringData* name,
                            Class* ctx) {
  auto sprop = getSPropOrNull(cls, name, ctx);
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

void checkFrame(ActRec* fp, Cell* sp, bool fullCheck, Offset bcOff) {
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
    fp, sp, bcOff,
    [](const ActRec* ar, Offset) {
      ar->func()->validate();
    },
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

//////////////////////////////////////////////////////////////////////

[[noreturn]] void throwMissingArgument(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  SystemLib::throwArgumentCountErrorObject(Variant(msg));
}

void raiseMissingArgument(const Func* func, int got) {
  const auto total = func->numNonVariadicParams();
  const auto variadic = func->hasVariadicCaptureParam();
  const Func::ParamInfoVec& params = func->params();
  int expected = 0;
  // We subtract the number of parameters with default value at the end
  for (size_t i = total; i--; ) {
    if (!params[i].hasDefaultValue()) {
      expected = i + 1;
      break;
    }
  }
  bool lessNeeded = (variadic || expected < total);

  if (RuntimeOption::PHP7_EngineExceptions) {
    throwMissingArgument(
      Strings::MISSING_ARGUMENT_EXCEPTION,
      func->displayName()->data(),
      got,
      lessNeeded ? "at least" : "exactly",
      expected
    );
  }
  if (expected == 1) {
    raise_warning(Strings::MISSING_ARGUMENT, func->displayName()->data(),
                  lessNeeded ? "at least" : "exactly", got);
  } else {
    raise_warning(Strings::MISSING_ARGUMENTS, func->displayName()->data(),
                  lessNeeded ? "at least" : "exactly", expected, got);
  }
}

//////////////////////////////////////////////////////////////////////

Class* lookupClsRDS(const StringData* name) {
  auto const handle = NamedEntity::get(name)->getClassHandle();
  assertx(rds::isHandleBound(handle));
  return rds::isHandleInit(handle)
    ? &*rds::handleToRef<LowPtr<Class>, rds::Mode::NonLocal>(handle)
    : nullptr;
}

void registerLiveObj(ObjectData* obj) {
  assertx(RuntimeOption::EnableObjDestructCall && obj->getVMClass()->getDtor());
  g_context->m_liveBCObjs.insert(obj);
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
  const ArrayData* a,
  const Class* declaringCls,
  const Class* calledCls,
  bool suppress
) {
  auto const ts = ArrNR(a);
  auto resolved =
    resolveAndVerifyTypeStructure(ts, declaringCls, calledCls, suppress);
  return resolved.detach();
}

bool isTypeStructHelper(ArrayData* a, Cell c) {
  auto const ts = ArrNR(a);
  return checkTypeStructureMatchesCell(ts, c);
}

void asTypeStructHelper(ArrayData* a, Cell c) {
  std::string givenType, expectedType, errorKey;
  auto const ts = ArrNR(a);
  if (!checkTypeStructureMatchesCell(
    ts, c, givenType, expectedType, errorKey)) {
    throwTypeStructureDoesNotMatchCellException(
      givenType, expectedType, errorKey);
  }
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
void setNewElem(tv_lval base, Cell val, const MInstrPropState* pState) {
  HPHP::SetNewElem<false>(base, &val, pState);
}

void setNewElemArray(tv_lval base, Cell val) {
  HPHP::SetNewElemArray(base, &val);
}

void setNewElemVec(tv_lval base, Cell val) {
  HPHP::SetNewElemVec(base, &val);
}

template <bool intishWarn>
TypedValue setOpElem(tv_lval base, TypedValue key,
                     Cell val, SetOpOp op, const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto result =
    HPHP::SetOpElem<intishWarn>(localTvRef, op, base, key, &val, pState);

  return cGetRefShuffle(localTvRef, result);
}

template TypedValue setOpElem<true>(tv_lval, TypedValue, Cell, SetOpOp,
                                    const MInstrPropState*);
template TypedValue setOpElem<false>(tv_lval, TypedValue, Cell, SetOpOp,
                                     const MInstrPropState*);

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
  return result ? !cellIsNull(result) : false;
}

template <bool intishWarn>
void bindElemC(tv_lval base, TypedValue key, RefData* val,
               const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto elem = HPHP::ElemD<MOpMode::Define, true, intishWarn>(
    localTvRef, base, key, pState
  );

  if (UNLIKELY(elem == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvDecRefGen(localTvRef);
    return;
  }

  tvBindRef(val, elem);
}

template void bindElemC<true>(tv_lval, TypedValue, RefData*,
                              const MInstrPropState*);
template void bindElemC<false>(tv_lval, TypedValue, RefData*,
                               const MInstrPropState*);

template <bool intishWarn>
void setWithRefElem(tv_lval base, TypedValue keyTV, TypedValue val,
                    const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto const keyC = tvToCell(keyTV);

  if (UNLIKELY(isRefType(val.m_type))) {
    HPHP::SetWithRefMLElem<MOpMode::Define, true, intishWarn>(
      localTvRef, base, keyC, val, pState);
  } else {
    HPHP::SetWithRefMLElem<MOpMode::Define, false, intishWarn>(
      localTvRef, base, keyC, val, pState);
  }
}

template void setWithRefElem<true>(tv_lval, TypedValue, TypedValue,
                                   const MInstrPropState*);
template void setWithRefElem<false>(tv_lval, TypedValue, TypedValue,
                                    const MInstrPropState*);

template <bool intishWarn>
TypedValue incDecElem(tv_lval base, TypedValue key,
                      IncDecOp op, const MInstrPropState* pState) {
  auto const result = HPHP::IncDecElem<intishWarn>(op, base, key, pState);
  assertx(!isRefType(result.m_type));
  return result;
}

template TypedValue incDecElem<true>(tv_lval, TypedValue, IncDecOp,
                                     const MInstrPropState* pState);
template TypedValue incDecElem<false>(tv_lval, TypedValue, IncDecOp,
                                      const MInstrPropState* pState);

void bindNewElem(tv_lval base,
                 RefData* val,
                 const MInstrPropState* pState) {
  if (UNLIKELY(tvIsHackArray(base))) {
    throwRefInvalidArrayValueException(HPHP::val(base).parr);
  }

  TypedValue localTvRef;
  auto elem = HPHP::NewElem<true>(localTvRef, base, pState);

  if (UNLIKELY(elem == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvDecRefGen(localTvRef);
    return;
  }

  tvBindRef(val, elem);
}

tv_lval elemVecID(tv_lval base, int64_t key) {
  auto cbase = tvToCell(base);
  assertx(isVecType(type(cbase)));
  return ElemDVec<false, KeyType::Int>(cbase, key);
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

void tvCoerceIfStrict(TypedValue& tv, int64_t argNum, const Func* func) {
  if (LIKELY(!RuntimeOption::PHP7_ScalarTypes ||
             RuntimeOption::EnableHipHopSyntax)) {
    return;
  }

  VMRegAnchor _;
  if (!call_uses_strict_types(func)) {
    return;
  }

  auto const& tc = func->params()[argNum - 1].typeConstraint;
  tc.verifyParam(&tv, func, argNum - 1);
}

TVCoercionException::TVCoercionException(const Func* func,
                                         int arg_num,
                                         DataType actual,
                                         DataType expected)
    : std::runtime_error(
        folly::format("Unable to coerce param {} to {}() "
                      "from {} to {}",
                      arg_num,
                      func->name(),
                      actual,
                      expected).str())
{
  if (func->attrs() & AttrParamCoerceModeFalse) {
    m_tv = make_tv<KindOfBoolean>(false);
  } else {
    m_tv = make_tv<KindOfNull>();
  }
}

//////////////////////////////////////////////////////////////////////

}}
