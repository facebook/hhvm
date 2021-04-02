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
#include "hphp/runtime/base/array-init.h"
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
#include "hphp/runtime/vm/jit/target-cache.h"
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

void setNewElem(tv_lval base, TypedValue val) {
  HPHP::SetNewElem<false>(base, &val);
}

void setNewElemVec(tv_lval base, TypedValue val) {
  HPHP::SetNewElemVec(base, &val);
}

void setNewElemDict(tv_lval base, TypedValue val) {
  HPHP::SetNewElemDict(base, &val);
}

//////////////////////////////////////////////////////////////////////

ArrayData* addNewElemVec(ArrayData* vec, TypedValue v) {
  assertx(vec->isVanillaVec());
  tvIncRefGen(v);
  return PackedArray::AppendMove(vec, v);
}

ArrayData* addNewElemKeyset(ArrayData* keyset, TypedValue v) {
  assertx(keyset->isVanillaKeyset());
  tvIncRefGen(v);
  return SetArray::AppendMove(keyset, v);
}

//////////////////////////////////////////////////////////////////////

ArrayData* convArrLikeToVecHelper(ArrayData* adIn) {
  auto a = adIn->toVec(adIn->cowCheck());
  assertx(a->isVecType());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToVecHelper(ObjectData* obj) {
  auto a = castObjToVec(obj);
  assertx(a->isVecType());
  decRefObj(obj);
  return a;
}

ArrayData* convArrLikeToDictHelper(ArrayData* adIn) {
  auto a = adIn->toDict(adIn->cowCheck());
  assertx(a->isDictType());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToDictHelper(ObjectData* obj) {
  auto a = castObjToDict(obj);
  assertx(a->isDictType());
  decRefObj(obj);
  return a;
}

ArrayData* convArrLikeToKeysetHelper(ArrayData* adIn) {
  auto a = adIn->toKeyset(adIn->cowCheck());
  assertx(a->isKeysetType());
  if (a != adIn) decRefArr(adIn);
  return a;
}

ArrayData* convObjToKeysetHelper(ObjectData* obj) {
  auto a = castObjToKeyset(obj);
  assertx(a->isKeysetType());
  decRefObj(obj);
  return a;
}

ArrayData* convClsMethToVecHelper(ClsMethDataRef clsmeth) {
  assertx(RO::EvalIsCompatibleClsMethType);
  raiseClsMethConvertWarningHelper("vec");
  auto a = make_vec_array(clsmeth->getClsStr(), clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToDictHelper(ClsMethDataRef clsmeth) {
  assertx(RO::EvalIsCompatibleClsMethType);
  raiseClsMethConvertWarningHelper("dict");
  auto a = make_dict_array(
    0, clsmeth->getClsStr(), 1, clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

ArrayData* convClsMethToKeysetHelper(ClsMethDataRef clsmeth) {
  assertx(RO::EvalIsCompatibleClsMethType);
  raiseClsMethConvertWarningHelper("keyset");
  auto a = make_keyset_array(
    clsmeth->getClsStr(), clsmeth->getFuncStr()).detach();
  decRefClsMeth(clsmeth);
  return a;
}

double convObjToDblHelper(const ObjectData* o) {
  return o->toDouble();
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

void raiseUndefProp(ObjectData* base, const StringData* name) {
  base->raiseUndefProp(name);
}

void raiseClsMethPropConvertNotice(const TypeConstraint* tc,
                                   bool isSProp,
                                   const Class* cls,
                                   const StringData* name) {
  assertx(RO::EvalIsCompatibleClsMethType);
  raise_notice(
    "class_meth Compat: %s '%s::%s' declared as type %s, clsmeth "
    "assigned",
    isSProp ? "Static property" : "Property",
    cls->name()->data(),
    name->data(),
    tc->displayName().c_str()
  );
}

void throwUndefVariable(StringData* nm) {
  SCOPE_EXIT { decRefStr(nm); };
  SystemLib::throwUndefinedVariableExceptionObject(
    folly::sformat("Undefined variable: {}", nm->data()));
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
  const Func* func = ar->func();
  if (!tc) {
    tc = &func->params()[paramNum].typeConstraint;
  }
  auto const param = frame_local(ar, paramNum);
  assertx(!tc->check(param, func->cls()));
  tc->verifyParamFail(func, param, paramNum);
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
  const Func* func = ar->func();
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
  const Func* func = ar->func();
  auto const param = frame_local(ar, id);
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
  const Func* func = ar->func();
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
TypedValue getDefaultIfMissing(TypedValue tv, TypedValue def) {
  return tv.is_init() ? tv : def;
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

// This helper may also be used when we know we have a MixedArray in the JIT.
TypedValue dictIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isVanillaDict());
  return getDefaultIfMissing(MixedArray::NvGetInt(a, key), def);
}

// This helper is also used for MixedArrays.
NEVER_INLINE
TypedValue dictIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isVanillaDict());
  return getDefaultIfMissing(MixedArray::NvGetStr(a, key), def);
}

// This helper is also used for MixedArrays.
NEVER_INLINE
TypedValue dictIdxScan(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isVanillaDict());
  auto const ad = MixedArray::asMixed(a);
  if (!ad->keyTypes().mustBeStaticStrs()) return dictIdxS(a, key, def);
  return doScan(ad, key, def);
}

TypedValue keysetIdxI(ArrayData* a, int64_t key, TypedValue def) {
  assertx(a->isVanillaKeyset());
  return getDefaultIfMissing(SetArray::NvGetInt(a, key), def);
}

TypedValue keysetIdxS(ArrayData* a, StringData* key, TypedValue def) {
  assertx(a->isVanillaKeyset());
  return getDefaultIfMissing(SetArray::NvGetStr(a, key), def);
}

template <bool isFirst>
TypedValue vecFirstLast(ArrayData* a) {
  assertx(a->isVanillaVec());
  auto const size = a->size();
  if (UNLIKELY(size == 0)) return make_tv<KindOfNull>();
  return PackedArray::NvGetInt(a, isFirst ? 0 : size - 1);
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
                           bool disallowConst,
                           bool mustBeMutable,
                           bool mustBeReadOnly) {
  auto const lookup = ignoreLateInit
    ? cls->getSPropIgnoreLateInit(ctx, name)
    : cls->getSProp(ctx, name);
  if (disallowConst && UNLIKELY(lookup.constant)) {
    throw_cannot_modify_static_const_prop(cls->name()->data(), name->data());
  }
  if (mustBeMutable && UNLIKELY(lookup.readonly)) {
    throw_must_be_mutable(cls->name()->data(), name->data());
  }
  if (mustBeReadOnly && UNLIKELY(!lookup.readonly)) {
    throw_cannot_write_non_readonly_prop(cls->name()->data(), name->data());
  }
  if (UNLIKELY(!lookup.val || !lookup.accessible)) return nullptr;

  return lookup.val;
}

TypedValue* getSPropOrRaise(const Class* cls,
                            const StringData* name,
                            Class* ctx,
                            bool ignoreLateInit,
                            bool disallowConst,
                            bool mustBeMutable,
                            bool mustBeReadOnly) {
  auto sprop = getSPropOrNull(cls, name, ctx, ignoreLateInit, disallowConst,
                              mustBeMutable, mustBeReadOnly);
  if (UNLIKELY(!sprop)) {
    raise_error("Invalid static property access: %s::%s",
                cls->name()->data(), name->data());
  }
  return sprop;
}

tv_lval ldGblAddrDefHelper(StringData* name) {
  return g_context->m_globalNVTable->lookupAdd(name);
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
      case KindOfObject:
      case KindOfResource:
      case KindOfRFunc:
      case KindOfFunc:
      case KindOfClass:
      case KindOfLazyClass:
      case KindOfClsMeth:
      case KindOfRClsMeth:
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
  const Func* func = fp->func();
  func->validate();
  if (func->cls()) {
    assertx(!func->cls()->isZombie());
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

const Func* loadClassCtor(Class* cls, Class* ctx) {
  const Func* f = cls->getCtor();
  if (UNLIKELY(!(f->attrs() & AttrPublic))) {
    UNUSED auto func =
      lookupMethodCtx(cls, nullptr, ctx, CallType::CtorMethod,
                      MethodLookupErrorOptions::RaiseOnNotFound);
    assertx(func == f);
  }
  return f;
}

const Func* lookupClsMethodHelper(const Class* cls, const StringData* methName,
                                  ObjectData* obj, const Class* ctx) {
  const Func* f;
  auto const res = lookupClsMethod(f, cls, methName, obj, ctx,
                                   MethodLookupErrorOptions::RaiseOnNotFound);

  if (res == LookupResult::MethodFoundWithThis) {
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

Class* lookupClsRDS(const StringData* name) {
  return NamedEntity::get(name)->getCachedClass();
}

TypedValue lookupClsCns(const Class* cls, const StringData* cnsName) {
  return cls->clsCnsGet(cnsName);
}

int lookupClsCtxCns(const Class* cls, const StringData* cnsName) {
  return cls->clsCtxCnsGet(cnsName, true)->value();
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

bool isTypeStructHelper(ArrayData* a, TypedValue c, rds::Handle h) {
  auto const cls = TSClassCache::write(h, a);
  if (!cls) return checkTypeStructureMatchesTV(ArrNR(a), c);
  if (!tvIsObject(c)) return false;
  return c.m_data.pobj->getVMClass()->classofNonIFace(cls);
}

void profileIsTypeStructHelper(ArrayData* a, IsTypeStructProfile* prof) {
  prof->update(a);
}

void throwAsTypeStructExceptionHelper(ArrayData* a, TypedValue c) {
  std::string givenType, expectedType, errorKey;
  auto const ts = ArrNR(a);
  if (!checkTypeStructureMatchesTV(ts, c, givenType, expectedType,
                                     errorKey)) {
    throwTypeStructureDoesNotMatchTVException(
      givenType, expectedType, errorKey);
  }
  always_assert(false && "Invalid bytecode sequence: Instruction must throw");
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

const StaticString s_classname("classname");
const StaticString s_kind("kind");
const StaticString s_type_structure_non_existant_class(
  "hh\\__internal\\type_structure_non_existant_class");

ArrayData* loadClsTypeCnsHelper(
  const Class* cls, const StringData* name, bool no_throw_on_undefined
) {
  auto const getFake = [&] {
    DArrayInit arr(2);
    arr.add(s_kind,
            Variant(static_cast<uint8_t>(TypeStructure::Kind::T_class)));
    arr.add(s_classname,
            Variant(s_type_structure_non_existant_class));
    auto result = arr.create();
    ArrayData::GetScalarArray(&result);
    return result;
  };
  TypedValue typeCns;
  if (no_throw_on_undefined) {
    try {
      typeCns = cls->clsCnsGet(name, ConstModifiers::Kind::Type);
    } catch (Exception& e) {
      return getFake();
    } catch (Object& e) {
      return getFake();
    }
  } else {
    typeCns = cls->clsCnsGet(name, ConstModifiers::Kind::Type);
  }
  if (typeCns.m_type == KindOfUninit) {
    if (no_throw_on_undefined) {
      return getFake();
    } else {
      if (cls->hasTypeConstant(name, true)) {
        raise_error("Type constant %s::%s is abstract",
                    cls->name()->data(), name->data());
      } else {
        raise_error("Non-existent type constant %s::%s",
                    cls->name()->data(), name->data());
      }
    }
  }

  assertx(isArrayLikeType(typeCns.m_type));
  assertx(typeCns.m_data.parr->isDictType());
  assertx(typeCns.m_data.parr->isStatic());
  return typeCns.m_data.parr;
}

void raiseCoeffectsCallViolationHelper(const Func* callee,
                                       uint64_t providedCoeffects,
                                       uint64_t requiredCoeffects) {
  raiseCoeffectsCallViolation(callee,
                              RuntimeCoeffects::fromValue(providedCoeffects),
                              RuntimeCoeffects::fromValue(requiredCoeffects));
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

TypedValue setOpElem(tv_lval base, TypedValue key,
                     TypedValue val, SetOpOp op) {
  auto const result = HPHP::SetOpElem(op, base, key, &val);
  tvIncRefGen(result);
  return result;
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

TypedValue incDecElem(tv_lval base, TypedValue key, IncDecOp op) {
  auto const result = HPHP::IncDecElem(op, base, key);
  return result;
}

tv_lval elemVecIU(tv_lval base, int64_t key) {
  assertx(tvIsVec(base));
  return ElemUVec<KeyType::Int>(base, key);
}

}

//////////////////////////////////////////////////////////////////////

}}
