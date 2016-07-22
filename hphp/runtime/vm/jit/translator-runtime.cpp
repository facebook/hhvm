/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/zend-functions.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/hh/ext_hh.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/std/ext_std_function.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/minstr-state.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"

#include "hphp/util/portability.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

const StaticString s_staticPrefix("86static_");

// Defined here so it can be inlined below.
RefData* lookupStaticFromClosure(ObjectData* closure,
                                 const StringData* name,
                                 bool& inited) {
  assertx(closure->instanceof(c_Closure::classof()));
  auto str = String::attach(
    StringData::Make(s_staticPrefix.slice(), name->slice())
  );
  auto const cls = closure->getVMClass();
  auto const slot = cls->lookupDeclProp(str.get());
  assertx(slot != kInvalidSlot);
  auto const val = c_Closure::fromObject(closure)->getStaticVar(slot);

  if (val->m_type == KindOfUninit) {
    inited = false;
    val->m_type = KindOfNull;
    tvBox(val);
    return val->m_data.pref;
  }
  inited = true;
  assertx(val->m_type == KindOfRef);
  return val->m_data.pref;
}

namespace jit {

//////////////////////////////////////////////////////////////////////

ArrayData* addNewElemHelper(ArrayData* a, TypedValue value) {
  auto r = a->append(*tvAssertCell(&value), a->hasMultipleRefs());
  if (UNLIKELY(r != a)) {
    decRefArr(a);
  }
  tvRefcountedDecRef(value);
  return r;
}

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64_t key,
                               TypedValue value) {
  // this does not re-enter
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval = ad->set(key, tvAsCVarRef(&value),
                              ad->cowCheck());
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  tvRefcountedDecRef(&value);
  return arrayRefShuffle<false>(ad, retval, nullptr);
}

ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  TypedValue value) {
  // this does not re-enter
  bool copy = ad->cowCheck();
  // set will decRef any old value that may have been overwritten
  // if appropriate
  int64_t intkey;
  ArrayData* retval = UNLIKELY(ad->convertKey(key, intkey)) ?
                  ad->set(intkey, tvAsCVarRef(&value), copy) :
                  ad->set(key, tvAsCVarRef(&value), copy);
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  decRefStr(key);
  tvRefcountedDecRef(&value);
  return arrayRefShuffle<false>(ad, retval, nullptr);
}

ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2) {
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
  return reinterpretDblAsInt(tvCastToDouble(&tv));
}

int64_t convArrToIntHelper(ArrayData* a) {
  return a->empty() ? 0 : 1;
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

TypedValue getMemoKeyHelper(TypedValue tv) {
  auto var = HHVM_FN(serialize_memoize_param)(tvAsCVarRef(&tv));
  auto res = var.asTypedValue();
  tvRefcountedIncRef(res);
  return *res;
}

inline void coerceCellFail(DataType expected, DataType actual, int64_t argNum,
                           const Func* func) {
  raise_param_type_warning(func->name()->data(), argNum, expected, actual);

  throw TVCoercionException(func, argNum, actual, expected);
}

bool coerceCellToBoolHelper(TypedValue tv, int64_t argNum, const Func* func) {
  assertx(cellIsPlausible(tv));

  tvCoerceIfStrict(tv, argNum, func);

  DataType type = tv.m_type;
  if (isArrayType(type) || type == KindOfObject || type == KindOfResource) {
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      coerceCellFail(KindOfDouble, tv.m_type, argNum, func);
      break;

    case KindOfRef:
    case KindOfClass:
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      coerceCellFail(KindOfInt64, tv.m_type, argNum, func);
      break;

    case KindOfRef:
    case KindOfClass:
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
    case KindOfPersistentArray:
    case KindOfArray:         raise_notice("Array to string conversion");
                              return array_string.get();
    case KindOfObject:        return convObjToStrHelper(tv.m_data.pobj);
    case KindOfResource:      return convResToStrHelper(tv.m_data.pres);
    case KindOfRef:
    case KindOfClass:         break;
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

RefData* ldClosureStaticLoc(StringData* name, ActRec* fp) {
  auto const func = fp->m_func;
  assertx(func->isClosureBody());
  auto const closureLoc = frame_local(fp, func->numParams());

  bool inited;
  auto const refData = lookupStaticFromClosure(
    closureLoc->m_data.pobj, name, inited);
  if (!inited) tvWriteUninit(refData->tv());
  return refData;
}

ALWAYS_INLINE
static bool ak_exist_string_impl(ArrayData* arr, StringData* key) {
  int64_t n;
  if (arr->convertKey(key, n)) {
    return arr->exists(n);
  }
  return arr->exists(key);
}

bool ak_exist_string(ArrayData* arr, StringData* key) {
  return ak_exist_string_impl(arr, key);
}

bool ak_exist_string_obj(ObjectData* obj, StringData* key) {
  if (obj->isCollection()) {
    return collections::contains(obj, Variant{key});
  }
  auto arr = obj->toArray();
  return ak_exist_string_impl(arr.get(), key);
}

bool ak_exist_int_obj(ObjectData* obj, int64_t key) {
  if (obj->isCollection()) {
    return collections::contains(obj, key);
  }
  auto arr = obj->toArray();
  return arr.get()->exists(key);
}

namespace {
ALWAYS_INLINE
TypedValue getDefaultIfNullCell(const TypedValue* tv, TypedValue& def) {
  return UNLIKELY(tv == nullptr) ? def : *tv;
}
}

TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

TypedValue arrayIdxSi(ArrayData* a, StringData* key, TypedValue def) {
  int64_t i;
  return UNLIKELY(a->convertKey(key, i)) ?
         getDefaultIfNullCell(a->nvGet(i), def) :
         getDefaultIfNullCell(a->nvGet(key), def);
}

TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

TypedValue arrayIdxIc(ArrayData* a, int64_t key, TypedValue def) {
  return arrayIdxI(a, key, def);
}

TypedValue mapIdx(ObjectData* mapOD, StringData* key, TypedValue def) {
  assert(collections::isType(mapOD->getVMClass(), CollectionType::Map) ||
         collections::isType(mapOD->getVMClass(), CollectionType::ImmMap));
  return getDefaultIfNullCell(static_cast<BaseMap*>(mapOD)->get(key), def);
}

int32_t arrayVsize(ArrayData* ad) {
  return ad->vsize();
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
      case KindOfPersistentArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
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

void profileArrayKindHelper(ArrayKindProfile* profile, ArrayData* arr) {
  profile->report(arr->kind());
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
      assertx(tv->m_type == KindOfClass || tvIsPlausible(*tv));
    }
  );
}

enum class OnFail { Warn, Fatal };

template<OnFail FailBehavior, class FooNR>
void loadFuncContextImpl(FooNR callableNR, ActRec* preLiveAR, ActRec* fp) {
  static_assert(
    std::is_same<FooNR,ArrNR>::value ||
      std::is_same<FooNR,StrNR>::value,
    "check loadFuncContextImpl for a new FooNR"
  );

  ObjectData* inst = nullptr;
  Class* cls = nullptr;
  StringData* invName = nullptr;

  auto func = vm_decode_function(
    VarNR(callableNR),
    fp,
    false, // forward
    inst,
    cls,
    invName,
    FailBehavior == OnFail::Warn
  );
  if (UNLIKELY(func == nullptr)) {
    if (FailBehavior == OnFail::Fatal) {
      raise_error("Invalid callable (array)");
    }
    func = SystemLib::s_nullFunc;
  }

  preLiveAR->m_func = func;
  if (inst) {
    inst->incRefCount();
    preLiveAR->setThis(inst);
  } else {
    preLiveAR->setClass(cls);
  }
  if (UNLIKELY(invName != nullptr)) {
    preLiveAR->setMagicDispatch(invName);
  }
}

void loadArrayFunctionContext(ArrayData* arr, ActRec* preLiveAR, ActRec* fp) {
  try {
    loadFuncContextImpl<OnFail::Fatal>(ArrNR(arr), preLiveAR, fp);
  } catch (...) {
    *arPreliveOverwriteCells(preLiveAR) = make_tv<KindOfArray>(arr);
    throw;
  }
}

NEVER_INLINE
static void fpushCufHelperArraySlowPath(ArrayData* arr,
                                        ActRec* preLiveAR,
                                        ActRec* fp) {
  loadFuncContextImpl<OnFail::Warn>(ArrNR(arr), preLiveAR, fp);
}

ALWAYS_INLINE
static bool strHasColon(StringData* sd) {
  auto const sl = sd->slice();
  auto const e = sl.end();
  for (auto p = sl.begin(); p != e; ++p) {
    if (*p == ':') return true;
  }
  return false;
}

void fpushCufHelperArray(ArrayData* arr, ActRec* preLiveAR, ActRec* fp) {
  try {
    if (UNLIKELY(!arr->isPacked() || arr->getSize() != 2)) {
      return fpushCufHelperArraySlowPath(arr, preLiveAR, fp);
    }

    auto const elem0 = tvToCell(PackedArray::NvGetInt(arr, 0));
    auto const elem1 = tvToCell(PackedArray::NvGetInt(arr, 1));

    if (UNLIKELY(elem0->m_type != KindOfObject ||
                 !isStringType(elem1->m_type))) {
      return fpushCufHelperArraySlowPath(arr, preLiveAR, fp);
    }

    // If the string contains a class name (e.g. Foo::bar), all kinds
    // of weird junk happens (wrt forwarding class contexts and
    // things).  We just do a quick loop to try to bail out of this
    // case.
    if (UNLIKELY(strHasColon(elem1->m_data.pstr))) {
      return fpushCufHelperArraySlowPath(arr, preLiveAR, fp);
    }

    auto const inst = elem0->m_data.pobj;
    auto const func = g_context->lookupMethodCtx(
      inst->getVMClass(),
      elem1->m_data.pstr,
      fp->m_func->cls(),
      CallType::ObjMethod
    );
    if (UNLIKELY(!func || (func->attrs() & AttrStatic))) {
      return fpushCufHelperArraySlowPath(arr, preLiveAR, fp);
    }

    preLiveAR->m_func = func;
    inst->incRefCount();
    preLiveAR->setThis(inst);
  } catch (...) {
    *arPreliveOverwriteCells(preLiveAR) = make_tv<KindOfArray>(arr);
    throw;
  }
}

NEVER_INLINE
static void fpushCufHelperStringSlowPath(StringData* sd,
                                         ActRec* preLiveAR,
                                         ActRec* fp) {
  loadFuncContextImpl<OnFail::Warn>(StrNR(sd), preLiveAR, fp);
}

NEVER_INLINE
static void fpushStringFail(const StringData* sd, ActRec* preLiveAR) {
  throw_invalid_argument("function: method '%s' not found", sd->data());
  preLiveAR->m_func = SystemLib::s_nullFunc;
}

void fpushCufHelperString(StringData* sd, ActRec* preLiveAR, ActRec* fp) {
  try {
    if (UNLIKELY(strHasColon(sd))) {
      return fpushCufHelperStringSlowPath(sd, preLiveAR, fp);
    }

    auto const func = Unit::loadFunc(sd);
    preLiveAR->m_func = func;
    if (UNLIKELY(!func)) {
      return fpushStringFail(sd, preLiveAR);
    }
  } catch (...) {
    *arPreliveOverwriteCells(preLiveAR) = make_tv<KindOfString>(sd);
    throw;
  }
}

const Func* loadClassCtor(Class* cls, ActRec* fp) {
  const Func* f = cls->getCtor();
  if (UNLIKELY(!(f->attrs() & AttrPublic))) {
    auto const ctx = arGetContextClass(fp);
    UNUSED auto func =
      g_context->lookupMethodCtx(cls, nullptr, ctx, CallType::CtorMethod, true);
    assertx(func == f);
  }
  return f;
}

//////////////////////////////////////////////////////////////////////

ObjectData* colAddNewElemCHelper(ObjectData* coll, TypedValue value) {
  collections::initElem(coll, &value);
  // If we specialized this on Vector we could use a DecRefNZ here (since we
  // could assume that initElem has incref'd the value).  Right now, HH\Set
  // goes through this code path also, though, and it might fail to add the new
  // element.
  tvRefcountedDecRef(value);
  return coll;
}

ObjectData* colAddElemCHelper(ObjectData* coll, TypedValue key,
                              TypedValue value) {
  collections::initMapElem(coll, &key, &value);
  // consume the input value. the collection setter either threw or created a
  // reference to value, so we can use a cheaper decref.
  tvRefcountedDecRefNZ(value);
  return coll;
}

//////////////////////////////////////////////////////////////////////

/*
 * The standard VMRegAnchor treatment won't work for some cases called
 * during function prologues.
 *
 * The fp sync machinery is fundamentally based on the notion that
 * instruction pointers in the TC are uniquely associated with source
 * HHBC instructions, and that source HHBC instructions are in turn
 * uniquely associated with SP->FP deltas.
 *
 * trimExtraArgs is called from the prologue of the callee.
 * The prologue is 1) still in the caller frame for now,
 * and 2) shared across multiple call sites. 1 means that we have the
 * fp from the caller's frame, and 2 means that this fp is not enough
 * to figure out sp.
 *
 * However, the prologue passes us the callee actRec, whose predecessor
 * has to be the caller. So we can sync sp and fp by ourselves here.
 * Geronimo!
 */
static void sync_regstate_to_caller(ActRec* preLive) {
  assertx(tl_regState == VMRegState::DIRTY);
  auto const ec = g_context.getNoCheck();
  auto& regs = vmRegsUnsafe();
  regs.stack.top() = (TypedValue*)preLive - preLive->numArgs();
  ActRec* fp = preLive == vmFirstAR() ?
    ec->m_nestedVMs.back().fp : preLive->m_sfp;
  regs.fp = fp;
  regs.pc = fp->m_func->unit()->at(fp->m_func->base() + preLive->m_soff);
  tl_regState = VMRegState::CLEAN;
}

#define SHUFFLE_EXTRA_ARGS_PRELUDE()                                    \
  const Func* f = ar->m_func;                                           \
  int numParams = f->numNonVariadicParams();                            \
  int numArgs = ar->numArgs();                                          \
  assertx(numArgs > numParams);                                         \
  int numExtra = numArgs - numParams;                                   \
  TRACE(1, "extra args: %d args, function %s takes only %d, ar %p\n",   \
        numArgs, f->name()->data(), numParams, ar);                     \
  auto tvArgs = reinterpret_cast<TypedValue*>(ar) - numArgs;            \
  /* end SHUFFLE_EXTRA_ARGS_PRELUDE */

NEVER_INLINE
static void trimExtraArgsMayReenter(ActRec* ar,
                                    TypedValue* tvArgs,
                                    TypedValue* limit) {
  sync_regstate_to_caller(ar);
  do {
    tvRefcountedDecRef(tvArgs); // may reenter for __destruct
    ++tvArgs;
  } while (tvArgs != limit);
  ar->setNumArgs(ar->m_func->numParams());

  // go back to dirty (see the comments of sync_regstate_to_caller)
  tl_regState = VMRegState::DIRTY;
}

void trimExtraArgs(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(!f->hasVariadicCaptureParam());
  assertx(!(f->attrs() & AttrMayUseVV));

  TypedValue* limit = tvArgs + numExtra;
  do {
    if (UNLIKELY(tvDecRefWillCallHelper(tvArgs))) {
      trimExtraArgsMayReenter(ar, tvArgs, limit);
      return;
    }
    tvDecRefOnly(tvArgs);
    ++tvArgs;
  } while (tvArgs != limit);

  assertx(f->numParams() == (numArgs - numExtra));
  assertx(f->numParams() == numParams);
  ar->setNumArgs(numParams);
}

void shuffleExtraArgsMayUseVV(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(!f->hasVariadicCaptureParam());
  assertx(f->attrs() & AttrMayUseVV);

  ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numExtra));
}

void shuffleExtraArgsVariadic(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(f->hasVariadicCaptureParam());
  assertx(!(f->attrs() & AttrMayUseVV));

  auto varArgsArray = Array::attach(PackedArray::MakePacked(numExtra, tvArgs));
  // write into the last (variadic) param
  auto tv = reinterpret_cast<TypedValue*>(ar) - numParams - 1;
  tv->m_type = KindOfArray;
  tv->m_data.parr = varArgsArray.detach();
  assertx(tv->m_data.parr->hasExactlyOneRef());

  // no incref is needed, since extra values are being transferred
  // from the stack to the last local
  assertx(f->numParams() == (numArgs - numExtra + 1));
  assertx(f->numParams() == (numParams + 1));
  ar->setNumArgs(numParams + 1);
}

void shuffleExtraArgsVariadicAndVV(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(f->hasVariadicCaptureParam());
  assertx(f->attrs() & AttrMayUseVV);

  ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numExtra));

  auto varArgsArray = Array::attach(PackedArray::MakePacked(numExtra, tvArgs));
  auto tvIncr = tvArgs; uint32_t i = 0;
  // an incref is needed to compensate for discarding from the stack
  for (; i < numExtra; ++i, ++tvIncr) { tvRefcountedIncRef(tvIncr); }
  // write into the last (variadic) param
  auto tv = reinterpret_cast<TypedValue*>(ar) - numParams - 1;
  tv->m_type = KindOfArray;
  tv->m_data.parr = varArgsArray.detach();
  assertx(tv->m_data.parr->hasExactlyOneRef());
  // Before, for each arg: refcount = n + 1 (stack)
  // After, for each arg: refcount = n + 2 (ExtraArgs, varArgsArray)
}

#undef SHUFFLE_EXTRA_ARGS_PRELUDE

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
  if (expected == 1) {
    raise_warning(Strings::MISSING_ARGUMENT, func->name()->data(),
                  lessNeeded ? "at least" : "exactly", got);
  } else {
    raise_warning(Strings::MISSING_ARGUMENTS, func->name()->data(),
                  lessNeeded ? "at least" : "exactly", expected, got);
  }
}

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

int64_t decodeCufIterHelper(Iter* it, TypedValue func) {
  DECLARE_FRAME_POINTER(fp);

  ObjectData* obj = nullptr;
  Class* cls = nullptr;
  StringData* invName = nullptr;

  auto ar = fp->m_sfp;
  if (LIKELY(ar->func()->isBuiltin())) {
    ar = g_context->getOuterVMFrame(ar);
  }
  auto const f = vm_decode_function(tvAsVariant(&func),
                                    ar, false,
                                    obj, cls, invName,
                                    false);
  if (UNLIKELY(!f)) return false;

  auto& cit = it->cuf();
  cit.setFunc(f);
  if (obj) {
    cit.setCtx(obj);
    obj->incRefCount();
  } else {
    cit.setCtx(cls);
  }
  cit.setName(invName);
  return true;
}

namespace MInstrHelpers {

TypedValue setOpElem(TypedValue* base, TypedValue key,
                     Cell val, SetOpOp op) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpElem(localTvRef, op, base, key, &val);

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
  return result ? !cellIsNull(result) : false;
}

void bindElemC(TypedValue* base, TypedValue key, RefData* val) {
  TypedValue localTvRef;
  auto elem = HPHP::ElemD<MOpFlags::DefineReffy>(localTvRef, base, key);

  if (UNLIKELY(elem == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvRefcountedDecRef(localTvRef);
    return;
  }

  tvBindRef(val, elem);
}

void setWithRefElem(TypedValue* base, TypedValue keyTV, TypedValue val) {
  TypedValue localTvRef;
  auto const keyC = tvToCell(&keyTV);
  auto elem = UNLIKELY(val.m_type == KindOfRef)
    ? HPHP::ElemD<MOpFlags::DefineReffy>(localTvRef, base, *keyC)
    : HPHP::ElemD<MOpFlags::Define>(localTvRef, base, *keyC);
  // Intentionally leak the old value pointed to by elem, including from magic
  // methods.
  tvDup(val, *elem);
}

TypedValue incDecElem(TypedValue* base, TypedValue key, IncDecOp op) {
  TypedValue result;
  HPHP::IncDecElem(op, base, key, result);
  assertx(result.m_type != KindOfRef);
  return result;
}

void bindNewElem(TypedValue* base, RefData* val) {
  TypedValue localTvRef;
  auto elem = HPHP::NewElem<true>(localTvRef, base);

  if (UNLIKELY(elem == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvRefcountedDecRef(localTvRef);
    return;
  }

  tvBindRef(val, elem);
}

}

//////////////////////////////////////////////////////////////////////

uintptr_t tlsBaseNoInline() {
  return tlsBase();
}

//////////////////////////////////////////////////////////////////////

}}
