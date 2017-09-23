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

  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatAdd();

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

void setNewElem(TypedValue* base, Cell val) {
  HPHP::SetNewElem<false>(base, &val);
}

void setNewElemArray(TypedValue* base, Cell val) {
  HPHP::SetNewElemArray(base, &val);
}

void setNewElemVec(TypedValue* base, Cell val) {
  HPHP::SetNewElemVec(base, &val);
}

RefData* boxValue(TypedValue tv) {
  assertx(tv.m_type != KindOfRef);
  if (tv.m_type == KindOfUninit) tv = make_tv<KindOfNull>();
  return RefData::Make(tv);
}

inline int64_t reinterpretDblAsInt(double d) {
  union {
    int64_t intval;
    double dblval;
  } u;
  u.dblval = d;
  return u.intval;
}

inline double reinterpretIntAsDbl(int64_t i) {
  union {
    int64_t intval;
    double dblval;
  } u;
  u.intval = i;
  return u.dblval;
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

ArrayData* convVecToArrHelper(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToPHPArrayVec(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convDictToArrHelper(ArrayData* adIn) {
  assertx(adIn->isDict());
  auto a = MixedArray::ToPHPArrayDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convKeysetToArrHelper(ArrayData* adIn) {
  assertx(adIn->isKeyset());
  auto a = SetArray::ToPHPArray(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convArrToVecHelper(ArrayData* adIn) {
  assert(adIn->isPHPArray());
  auto a = adIn->toVec(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convDictToVecHelper(ArrayData* adIn) {
  assert(adIn->isDict());
  auto a = MixedArray::ToVecDict(adIn, adIn->cowCheck());
  assert(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convKeysetToVecHelper(ArrayData* adIn) {
  assert(adIn->isKeyset());
  auto a = SetArray::ToVec(adIn, adIn->cowCheck());
  assert(a != adIn);
  decRefArr(adIn);
  return a;
}

static Array arrayFromCollection(ObjectData* obj) {
  if (auto ad = collections::asArray(obj)) {
    return ArrNR{ad}.asArray();
  }
  return collections::toArray(obj);
}

ArrayData* convObjToVecHelper(ObjectData* obj) {
  if (obj->isCollection()) {
    auto a = arrayFromCollection(obj).toVec();
    decRefObj(obj);
    return a.detach();
  }

  if (obj->instanceof(SystemLib::s_IteratorClass)) {
    auto arr = Array::CreateVec();
    for (ArrayIter iter(obj); iter; ++iter) {
      arr.append(iter.second());
    }
    decRefObj(obj);
    return arr.detach();
  }

  SystemLib::throwInvalidOperationExceptionObject(
    "Non-iterable object to vec conversion"
  );
}

ArrayData* convArrToDictHelper(ArrayData* adIn) {
  assert(adIn->isPHPArray());
  auto a = adIn->toDict(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convVecToDictHelper(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToDictVec(adIn, adIn->cowCheck());
  assert(a != adIn);
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
  if (obj->isCollection()) {
    auto a = arrayFromCollection(obj).toDict();
    decRefObj(obj);
    return a.detach();
  }

  if (obj->instanceof(SystemLib::s_IteratorClass)) {
    auto arr = Array::CreateDict();
    for (ArrayIter iter(obj); iter; ++iter) {
      arr.set(iter.first(), iter.second());
    }
    decRefObj(obj);
    return arr.detach();
  }

  SystemLib::throwInvalidOperationExceptionObject(
    "Non-iterable object to dict conversion"
  );
}

ArrayData* convArrToKeysetHelper(ArrayData* adIn) {
  assert(adIn->isPHPArray());
  auto a = adIn->toKeyset(adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convVecToKeysetHelper(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToKeysetVec(adIn, adIn->cowCheck());
  assert(a != adIn);
  decRefArr(adIn);
  return a;
}

ArrayData* convDictToKeysetHelper(ArrayData* adIn) {
  assert(adIn->isDict());
  auto a = MixedArray::ToKeysetDict(adIn, adIn->cowCheck());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToKeysetHelper(ObjectData* obj) {
  if (obj->isCollection()) {
    auto a = arrayFromCollection(obj).toKeyset();
    decRefObj(obj);
    return a.detach();
  }

  if (obj->instanceof(SystemLib::s_IteratorClass)) {
    auto arr = Array::CreateKeyset();
    for (ArrayIter iter(obj); iter; ++iter) {
      arr.append(iter.second());
    }
    decRefObj(obj);
    return arr.detach();
  }

  SystemLib::throwInvalidOperationExceptionObject(
    "Non-iterable object to keyset conversion"
  );
}

int64_t convObjToDblHelper(const ObjectData* o) {
  return reinterpretDblAsInt(o->toDouble());
}

int64_t convArrToDblHelper(ArrayData* a) {
  return reinterpretDblAsInt(a->empty() ? 0 : 1);
}

int64_t convStrToDblHelper(const StringData* s) {
  return reinterpretDblAsInt(s->toDouble());
}

int64_t convResToDblHelper(const ResourceHdr* r) {
  return reinterpretDblAsInt(r->getId());
}

int64_t convCellToDblHelper(TypedValue tv) {
  return reinterpretDblAsInt(tvCastToDouble(tv));
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

StringData* convDblToStrHelper(int64_t i) {
  double d = reinterpretIntAsDbl(i);
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

bool coerceCellToBoolHelper(TypedValue tv, int64_t argNum, const Func* func) {
  assertx(cellIsPlausible(tv));

  tvCoerceIfStrict(tv, argNum, func);

  DataType type = tv.m_type;
  if (isArrayLikeType(type) || type == KindOfObject || type == KindOfResource) {
    coerceCellFail(KindOfBoolean, type, argNum, func);
    not_reached();
  }

  return cellToBool(tv);
}

int64_t coerceStrToDblHelper(StringData* sd, int64_t argNum, const Func* func) {
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

  return reinterpretDblAsInt(sd->toDouble());
}

int64_t coerceCellToDblHelper(Cell tv, int64_t argNum, const Func* func) {
  assertx(cellIsPlausible(tv));

  tvCoerceIfStrict(tv, argNum, func);

  switch (tv.m_type) {
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
      return convCellToDblHelper(tv);

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
      return cellToInt(tv);

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

const StaticString
  s_empty(""),
  s_1("1");

StringData* convCellToStrHelper(TypedValue tv) {
  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:          return s_empty.get();
    case KindOfBoolean:       return tv.m_data.num ? s_1.get() : s_empty.get();
    case KindOfInt64:         return convIntToStrHelper(tv.m_data.num);
    case KindOfDouble:        return convDblToStrHelper(tv.m_data.num);
    case KindOfString:        tv.m_data.pstr->incRefCount();
                              /* fallthrough */
    case KindOfPersistentString:
                              return tv.m_data.pstr;
    case KindOfPersistentVec:
    case KindOfVec:           raise_notice("Vec to string conversion");
                              return vec_string.get();
    case KindOfPersistentDict:
    case KindOfDict:          raise_notice("Dict to string conversion");
                              return dict_string.get();
    case KindOfPersistentKeyset:
    case KindOfKeyset:        raise_notice("Keyset to string conversion");
                              return keyset_string.get();
    case KindOfPersistentArray:
    case KindOfArray:         raise_notice("Array to string conversion");
                              return array_string.get();
    case KindOfObject:        return convObjToStrHelper(tv.m_data.pobj);
    case KindOfResource:      return convResToStrHelper(tv.m_data.pres);
    case KindOfRef:           break;
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
                               const HPHP::TypeConstraint* expected) {
  // This helper should only be called for the Object, Self, and Parent cases
  assertx(expected->isObject() || expected->isSelf() || expected->isParent());
  // For the Self and Parent cases, we must always have a resolved class for
  // the constraint
  assertx(IMPLIES(
    expected->isSelf() || expected->isParent(), constraint != nullptr));
  // If we have a resolved class for the constraint, all we have to do is
  // check if the value's class is compatible with it
  if (LIKELY(constraint != nullptr)) {
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
                         const HPHP::TypeConstraint* expected,
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
  auto unit = func->unit();
  bool useStrictTypes =
    unit->isHHFile() || RuntimeOption::EnableHipHopSyntax ||
    !ar->useWeakTypes();
  assertx(!tc.check(tv, func));
  tc.verifyParamFail(func, tv, paramNum, useStrictTypes);
}

void VerifyRetTypeSlow(const Class* cls,
                       const Class* constraint,
                       const HPHP::TypeConstraint* expected,
                       TypedValue tv) {
  if (!VerifyTypeSlowImpl(cls, constraint, expected)) {
    VerifyRetTypeFail(&tv);
  }
}

void VerifyRetTypeCallable(TypedValue value) {
  if (UNLIKELY(!is_callable(tvAsCVarRef(&value)))) {
    VerifyRetTypeFail(&value);
  }
}

void VerifyRetTypeFail(TypedValue* tv) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  const HPHP::TypeConstraint& tc = func->returnTypeConstraint();
  auto unit = func->unit();
  bool useStrictTypes =
    RuntimeOption::EnableHipHopSyntax || func->isBuiltin() ||
    unit->useStrictTypes();
  assertx(!tc.check(tv, func));
  tc.verifyReturnFail(func, tv, useStrictTypes);
}

namespace {
ALWAYS_INLINE
TypedValue getDefaultIfNullCell(member_rval rval, const TypedValue& def) {
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

  if (UNLIKELY(!lookup.prop || !lookup.accessible)) return nullptr;

  return lookup.prop;
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

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets) {
  union {
    int64_t intbits;
    double dblval;
  } u;
  u.intbits = val;
  return switchBoundsCheck(u.dblval, base, nTargets);
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
  assertx(handle != rds::kInvalidHandle);
  return rds::isHandleInit(handle)
    ? &*rds::handleToRef<LowPtr<Class>>(handle)
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

template <bool intishWarn>
TypedValue setOpElem(TypedValue* base, TypedValue key,
                     Cell val, SetOpOp op) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpElem<intishWarn>(localTvRef, op, base, key, &val);

  return cGetRefShuffle(localTvRef, result);
}

template TypedValue setOpElem<true>(TypedValue*, TypedValue, Cell, SetOpOp);
template TypedValue setOpElem<false>(TypedValue*, TypedValue, Cell, SetOpOp);

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
void bindElemC(TypedValue* base, TypedValue key, RefData* val) {
  TypedValue localTvRef;
  auto elem =
    HPHP::ElemD<MOpMode::Define, true, intishWarn>(localTvRef, base, key);

  if (UNLIKELY(elem == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvDecRefGen(localTvRef);
    return;
  }

  tvBindRef(val, elem);
}

template void bindElemC<true>(TypedValue*, TypedValue, RefData*);
template void bindElemC<false>(TypedValue*, TypedValue, RefData*);

template <bool intishWarn>
void setWithRefElem(TypedValue* base, TypedValue keyTV, TypedValue val) {
  TypedValue localTvRef;
  auto const keyC = tvToCell(keyTV);

  if (UNLIKELY(val.m_type == KindOfRef)) {
    HPHP::SetWithRefMLElem<MOpMode::Define, true, intishWarn>(
      localTvRef, base, keyC, val);
  } else {
    HPHP::SetWithRefMLElem<MOpMode::Define, false, intishWarn>(
      localTvRef, base, keyC, val);
  }
}

template void setWithRefElem<true>(TypedValue*, TypedValue, TypedValue);
template void setWithRefElem<false>(TypedValue*, TypedValue, TypedValue);

template <bool intishWarn>
TypedValue incDecElem(TypedValue* base, TypedValue key, IncDecOp op) {
  auto const result = HPHP::IncDecElem<intishWarn>(op, base, key);
  assertx(result.m_type != KindOfRef);
  return result;
}

template TypedValue incDecElem<true>(TypedValue*, TypedValue, IncDecOp);
template TypedValue incDecElem<false>(TypedValue*, TypedValue, IncDecOp);

void bindNewElem(TypedValue* base, RefData* val) {
  if (UNLIKELY(isHackArrayType(base->m_type))) {
    throwRefInvalidArrayValueException(base->m_data.parr);
  }

  TypedValue localTvRef;
  auto elem = HPHP::NewElem<true>(localTvRef, base);

  if (UNLIKELY(elem == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvDecRefGen(localTvRef);
    return;
  }

  tvBindRef(val, elem);
}

TypedValue* elemVecID(TypedValue* base, int64_t key) {
  auto cbase = tvToCell(base);
  assertx(isVecType(cbase->m_type));
  return ElemDVec<false, KeyType::Int>(cbase, key);
}

TypedValue* elemVecIU(TypedValue* base, int64_t key) {
  auto cbase = tvToCell(base);
  assertx(isVecType(cbase->m_type));
  return ElemUVec<KeyType::Int>(cbase, key);
}

}

//////////////////////////////////////////////////////////////////////

uintptr_t tlsBaseNoInline() {
  return tlsBase();
}

//////////////////////////////////////////////////////////////////////

/*
 * Sometimes calls to builtin functions are inlined so that the call itself can
 * occur via CallBuiltin rather than NativeImpl.  In these instances it's
 * possible that no ActRec was pushed for the builtin call, in which case the
 * liveFunc() will be the caller rather than the callee.
 *
 * If no ActRec was pushed for the builtin function, inspect the caller to
 * determine if the call used strict types.
 */
bool useStrictTypesHelper(const Func* callee) {
  return liveFunc() == callee
    ? !liveFrame()->useWeakTypes()
    : liveUnit()->useStrictTypes() && !liveUnit()->isHHFile();
}

void tvCoerceIfStrict(TypedValue& tv, int64_t argNum, const Func* func) {
  if (LIKELY(!RuntimeOption::PHP7_ScalarTypes ||
             RuntimeOption::EnableHipHopSyntax)) {
    return;
  }

  VMRegAnchor _;
  if (!useStrictTypesHelper(func)) return;

  auto const& tc = func->params()[argNum - 1].typeConstraint;
  tc.verifyParam(&tv, func, argNum - 1, true);
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
