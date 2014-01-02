/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-x64-internal.h"
#include "hphp/runtime/base/stats.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

const StaticString s_staticPrefix("86static_");

// Defined here so it can be inlined below.
RefData* lookupStaticFromClosure(ObjectData* closure,
                                 StringData* name,
                                 bool& inited) {
  assert(closure->instanceof(c_Closure::classof()));
  String str(StringData::Make(s_staticPrefix->slice(), name->slice()));
  auto const cls = closure->getVMClass();
  auto const slot = cls->lookupDeclProp(str.get());
  assert(slot != kInvalidSlot);
  auto const val = static_cast<c_Closure*>(closure)->getStaticVar(slot);

  if (val->m_type == KindOfUninit) {
    inited = false;
    val->m_type = KindOfNull;
    tvBox(val);
    return val->m_data.pref;
  }
  inited = true;
  assert(val->m_type == KindOfRef);
  return val->m_data.pref;
}

namespace JIT {

//////////////////////////////////////////////////////////////////////

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64_t key,
                               TypedValue value) {
  // this does not re-enter
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval = ad->set(key, tvAsCVarRef(&value),
                              ad->hasMultipleRefs());
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
  bool copy = ad->hasMultipleRefs();
  // set will decRef any old value that may have been overwritten
  // if appropriate
  int64_t intkey;
  ArrayData* retval = UNLIKELY(key->isStrictlyInteger(intkey)) ?
                      ad->set(intkey, tvAsCVarRef(&value), copy) :
                      ad->set(key, tvAsCVarRef(&value), copy);
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
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

void bindNewElemIR(TypedValue* base, RefData* val, MInstrState* mis) {
  base = HPHP::NewElem(mis->tvScratch, mis->tvRef, base);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBindRef(val, base);
  }
}

RefData* boxValue(TypedValue tv) {
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

int64_t convArrToDblHelper(ArrayData* a) {
  return reinterpretDblAsInt(a->empty() ? 0 : 1);
}

int64_t convStrToDblHelper(const StringData* s) {
  return reinterpretDblAsInt(s->toDouble());
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
  auto r = buildStringData(d);
  r->incRefCount();
  return r;
}

StringData* convIntToStrHelper(int64_t i) {
  auto r = buildStringData(i);
  r->incRefCount();
  return r;
}

StringData* convObjToStrHelper(ObjectData* o) {
  auto s = o->invokeToString();
  auto r = s.get();
  if (!r->isStatic()) r->incRefCount();
  return r;
}

StringData* convResToStrHelper(ResourceData* o) {
  auto s = o->o_toString();
  auto r = s.get();
  if (!r->isStatic()) r->incRefCount();
  return r;
}

const StaticString
  s_empty(""),
  s_1("1"),
  s_Array("Array");

StringData* convCellToStrHelper(TypedValue tv) {
  switch (tv.m_type) {
  case KindOfUninit:
  case KindOfNull:     return s_empty.get();
  case KindOfBoolean:  return tv.m_data.num ? s_1.get() : s_empty.get();
  case KindOfInt64:    return convIntToStrHelper(tv.m_data.num);
  case KindOfDouble:   return convDblToStrHelper(tv.m_data.num);
  case KindOfString:   tv.m_data.pstr->incRefCount();
                       /* fallthrough */
  case KindOfStaticString:
                       return tv.m_data.pstr;
  case KindOfArray:    raise_notice("Array to string conversion");
                       return s_Array.get();
  case KindOfObject:   return convObjToStrHelper(tv.m_data.pobj);
  case KindOfResource: return convResToStrHelper(tv.m_data.pres);
  default:             not_reached();
  }
}

void raisePropertyOnNonObject() {
  raise_warning("Cannot access property on non-object");
}

void raiseUndefProp(ObjectData* base, const StringData* name) {
  base->raiseUndefProp(name);
}

void raise_error_sd(const StringData *msg) {
  raise_error("%s", msg->data());
}

void VerifyParamTypeFail(int paramNum) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  auto const& tc = func->params()[paramNum].typeConstraint();
  TypedValue* tv = frame_local(ar, paramNum);
  assert(!tc.check(tv, func));
  tc.verifyFail(func, paramNum, tv);
}

void VerifyParamTypeCallable(TypedValue value, int param) {
  if (UNLIKELY(!f_is_callable(tvAsCVarRef(&value)))) {
    VerifyParamTypeFail(param);
  }
}

void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         int param,
                         const HPHP::TypeConstraint* expected) {
  if (LIKELY(constraint && cls->classof(constraint))) {
    return;
  }

  // Check a typedef for a class.  We interp'd if the param wasn't an
  // object, so if it's a typedef for something non-objecty we're
  // failing anyway.
  if (auto namedEntity = expected->namedEntity()) {
    auto def = namedEntity->getCachedTypeAlias();
    if (UNLIKELY(!def)) {
      VMRegAnchor _;
      String nameStr(const_cast<StringData*>(expected->typeName()));
      if (AutoloadHandler::s_instance->autoloadType(nameStr)) {
        def = namedEntity->getCachedTypeAlias();
      }
    }
    if (def) {
      // There's no need to handle nullable typedefs specially here:
      // we already know we're checking a non-null object with the
      // class `cls'.  We do however need to check for typedefs to
      // mixed.
      if (def->kind == KindOfObject) {
        constraint = def->klass;
        if (constraint && cls->classof(constraint)) return;
      } else if (def->kind == KindOfAny) {
        return;
      }
    }
  }

  VerifyParamTypeFail(param);
}

RefData* closureStaticLocInit(StringData* name, ActRec* fp, TypedValue val) {
  auto const func = fp->m_func;
  assert(func->isClosureBody() || func->isGeneratorFromClosure());
  auto const closureLoc =
    LIKELY(func->isClosureBody())
      ? frame_local(fp, func->numParams())
      : frame_local(fp, func->getGeneratorOrigFunc()->numParams());

  bool inited;
  auto const refData = lookupStaticFromClosure(
    closureLoc->m_data.pobj, name, inited);
  if (!inited) {
    cellCopy(val, *refData->tv());
  }
  return refData;
}

ALWAYS_INLINE
static int64_t ak_exist_string_impl(ArrayData* arr, StringData* key) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return arr->exists(n);
  }
  return arr->exists(key);
}

int64_t ak_exist_string(ArrayData* arr, StringData* key) {
  return ak_exist_string_impl(arr, key);
}

int64_t ak_exist_int(ArrayData* arr, int64_t key) {
  bool res = arr->exists(key);
  return res;
}

int64_t ak_exist_string_obj(ObjectData* obj, StringData* key) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  int64_t res = ak_exist_string_impl(arr.get(), key);
  return res;
}

int64_t ak_exist_int_obj(ObjectData* obj, int64_t key) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  bool res = arr.get()->exists(key);
  return res;
}

ALWAYS_INLINE
TypedValue& getDefaultIfNullCell(TypedValue* tv, TypedValue& def) {
  if (UNLIKELY(nullptr == tv)) {
    // DecRef of def is done unconditionally by the IR, since there's
    // a good chance it will be paired with an IncRef and optimized
    // away.  So we need to IncRef here if it is being returned.
    tvRefcountedIncRef(&def);
    return def;
  }
  TypedValue* ret = tvToCell(tv);
  tvRefcountedIncRef(ret);
  return *ret;
}

TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

TypedValue arrayIdxSi(ArrayData* a, StringData* key, TypedValue def) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ?
         getDefaultIfNullCell(a->nvGet(i), def) :
         getDefaultIfNullCell(a->nvGet(key), def);
}

TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

const StaticString s_idx("idx");

TypedValue genericIdx(TypedValue obj, TypedValue key, TypedValue def) {
  static auto func = Unit::loadFunc(s_idx.get());
  assert(func != nullptr);
  Array args = PackedArrayInit(3)
                         .append(tvAsVariant(&obj))
                         .append(tvAsVariant(&key))
                         .append(tvAsVariant(&def))
                         .toArray();
  TypedValue ret;
  g_vmContext->invokeFunc(&ret, func, args);
  return ret;
}

int32_t arrayVsize(ArrayData* ad) {
  return ad->vsize();
}

TypedValue* ldGblAddrHelper(StringData* name) {
  return g_vmContext->m_globalVarEnv->lookup(name);
}

TypedValue* ldGblAddrDefHelper(StringData* name) {
  return g_vmContext->m_globalVarEnv->lookupAdd(name);
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
  switch (s->isNumericWithVal(ival, dval, 1)) {
    case KindOfNull:
      ival = switchBoundsCheck(0, base, nTargets);
      break;

    case KindOfDouble:
      ival = switchBoundsCheck(dval, base, nTargets);
      break;

    case KindOfInt64:
      ival = switchBoundsCheck(ival, base, nTargets);
      break;

    default:
      not_reached();
  }
  decRefStr(s);
  return ival;
}

int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets) {
  int64_t ival = o->o_toInt64();
  decRefObj(o);
  return switchBoundsCheck(ival, base, nTargets);
}

TCA sswitchHelperFast(const StringData* val,
                      const SSwitchMap* table,
                      TCA* def) {
  TCA* dest = table->find(val);
  return dest ? *dest : *def;
}

// TODO(#2031980): clear these out
void tv_release_generic(TypedValue* tv) {
  assert(JIT::tx64->stateIsDirty());
  assert(tv->m_type == KindOfString || tv->m_type == KindOfArray ||
         tv->m_type == KindOfObject || tv->m_type == KindOfResource ||
         tv->m_type == KindOfRef);
  g_destructors[typeToDestrIndex(tv->m_type)](tv->m_data.pref);
}

void tv_release_typed(RefData* pv, DataType dt) {
  assert(JIT::tx64->stateIsDirty());
  assert(dt == KindOfString || dt == KindOfArray ||
         dt == KindOfObject || dt == KindOfResource ||
         dt == KindOfRef);
  g_destructors[typeToDestrIndex(dt)](pv);
}

Cell lookupCnsHelper(const TypedValue* tv,
                     StringData* nm,
                     bool error) {
  assert(tv->m_type == KindOfUninit);

  // Deferred constants such as SID
  if (UNLIKELY(tv->m_data.pref != nullptr)) {
    ClassInfo::ConstantInfo* ci =
      (ClassInfo::ConstantInfo*)(void*)tv->m_data.pref;
    Cell *cns = const_cast<Variant&>(ci->getDeferredValue()).asTypedValue();
    if (LIKELY(cns->m_type != KindOfUninit)) {
      Cell c1;
      cellDup(*cns, c1);
      return c1;
    }
  }

  Cell *cns = nullptr;
  if (UNLIKELY(RDS::s_constants().get() != nullptr)) {
    cns = RDS::s_constants()->nvGet(nm);
  }
  if (!cns) {
    cns = Unit::loadCns(const_cast<StringData*>(nm));
  }
  if (LIKELY(cns != nullptr)) {
    Cell c1;
    c1.m_type = cns->m_type;
    c1.m_data = cns->m_data;
    return c1;
  }

  // Undefined constants
  if (error) {
    raise_error("Undefined constant '%s'", nm->data());
  } else {
    raise_notice(Strings::UNDEFINED_CONSTANT, nm->data(), nm->data());
    Cell c1;
    c1.m_data.pstr = const_cast<StringData*>(nm);
    c1.m_type = KindOfStaticString;
    return c1;
  }
  not_reached();
}

void lookupClsMethodHelper(Class* cls,
                           StringData* meth,
                           ActRec* ar,
                           ActRec* fp) {
  try {
    using namespace MethodLookup;
    const Func* f;
    ObjectData* obj = fp->hasThis() ? fp->getThis() : nullptr;
    Class* ctx = fp->m_func->cls();
    LookupResult res =
      g_vmContext->lookupClsMethod(f, cls, meth, obj, ctx, true);
    if (res == LookupResult::MethodFoundNoThis ||
        res == LookupResult::MagicCallStaticFound) {
      ar->setClass(cls);
    } else {
      assert(obj);
      assert(res == LookupResult::MethodFoundWithThis ||
             res == LookupResult::MagicCallFound);
      obj->incRefCount();
      ar->setThis(obj);
    }
    ar->m_func = f;
    if (res == LookupResult::MagicCallFound ||
        res == LookupResult::MagicCallStaticFound) {
      ar->setInvName(meth);
      meth->incRefCount();
    }
  } catch (...) {
    *arPreliveOverwriteCells(ar) = make_tv<KindOfString>(meth);
    throw;
  }
}

Cell lookupCnsUHelper(const TypedValue* tv,
                      StringData* nm,
                      StringData* fallback) {
  Cell *cns = nullptr;
  Cell c1;

  // lookup qualified name in thread-local constants
  bool cacheConsts = RDS::s_constants().get() != nullptr;
  if (UNLIKELY(cacheConsts)) {
    cns = RDS::s_constants()->nvGet(nm);
  }
  if (!cns) {
    cns = Unit::loadCns(const_cast<StringData*>(nm));
  }

  // try cache handle for unqualified name
  if (UNLIKELY(!cns && tv->m_type != KindOfUninit)) {
    cns = const_cast<Cell*>(tv);
  }

  // lookup unqualified name in thread-local constants
  if (UNLIKELY(!cns)) {
    if (UNLIKELY(cacheConsts)) {
      cns = RDS::s_constants()->nvGet(fallback);
    }
    if (!cns) {
      cns = Unit::loadCns(const_cast<StringData*>(fallback));
    }
    if (UNLIKELY(!cns)) {
      raise_notice(Strings::UNDEFINED_CONSTANT,
                   fallback->data(), fallback->data());
      c1.m_data.pstr = const_cast<StringData*>(fallback);
      c1.m_type = KindOfStaticString;
    }
  } else {
    c1.m_type = cns->m_type;
    c1.m_data = cns->m_data;
  }
  return c1;
}

//////////////////////////////////////////////////////////////////////

void checkFrame(ActRec* fp, Cell* sp, bool checkLocals) {
  const Func* func = fp->m_func;
  func->validate();
  if (func->cls()) {
    assert(!func->cls()->isZombie());
  }
  if (fp->hasVarEnv()) {
    assert(fp->getVarEnv()->getCfp() == fp);
  }
  // TODO: validate this pointer from actrec
  int numLocals = func->numLocals();
  assert(sp <= (Cell*)fp - func->numSlotsInFrame()
         || func->isGenerator());
  if (checkLocals) {
    int numParams = func->numParams();
    for (int i=0; i < numLocals; i++) {
      if (i >= numParams && func->isGenerator() && i < func->numNamedLocals()) {
        continue;
      }
      assert(tvIsPlausible(*frame_local(fp, i)));
    }
  }
  // We unfortunately can't do the same kind of check for the stack
  // without knowing about FPI regions, because it may contain
  // ActRecs.
}

void traceCallback(ActRec* fp, Cell* sp, int64_t pcOff, void* rip) {
  if (HPHP::Trace::moduleEnabled(HPHP::Trace::hhirTracelets)) {
    FTRACE(0, "{} {} {}\n", fp->m_func->fullName()->data(), pcOff, rip);
  }
  checkFrame(fp, sp, /*checkLocals*/true);
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
    preLiveAR->setInvName(invName);
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
  auto const e = sl.ptr + sl.len;
  for (auto p = sl.ptr; p != e; ++p) {
    if (*p == ':') return true;
  }
  return false;
}

void fpushCufHelperArray(ArrayData* arr, ActRec* preLiveAR, ActRec* fp) {
  try {
    if (UNLIKELY(!arr->isPacked() || arr->getSize() != 2)) {
      return fpushCufHelperArraySlowPath(arr, preLiveAR, fp);
    }

    auto const elem0 = tvToCell(HphpArray::NvGetIntPacked(arr, 0));
    auto const elem1 = tvToCell(HphpArray::NvGetIntPacked(arr, 1));

    if (UNLIKELY(elem0->m_type != KindOfObject ||
                 !IS_STRING_TYPE(elem1->m_type))) {
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
    auto const func = g_vmContext->lookupMethodCtx(
      inst->getVMClass(),
      elem1->m_data.pstr,
      fp->m_func->cls(),
      MethodLookup::CallType::ObjMethod
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

const Func* lookupUnknownFunc(const StringData* name) {
  JIT::VMRegAnchor _;
  auto const func = Unit::loadFunc(name);
  if (UNLIKELY(!func)) {
    raise_error("Undefined function: %s", name->data());
  }
  return func;
}

Class* lookupKnownClass(Class** cache, const StringData* clsName) {
  Class* cls = *cache;
  assert(!cls); // the caller should already have checked

  AutoloadHandler::s_instance->invokeHandler(
    StrNR(const_cast<StringData*>(clsName)));
  cls = *cache;

  if (UNLIKELY(!cls)) raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  return cls;
}

Cell lookupClassConstantTv(TypedValue* cache,
                           const NamedEntity* ne,
                           const StringData* cls,
                           const StringData* cns) {
  Cell clsCns = g_vmContext->lookupClsCns(ne, cls, cns);
  assert(isUncounted(clsCns));
  cellDup(clsCns, *cache);
  return clsCns;
}

//////////////////////////////////////////////////////////////////////

ObjectData* newColHelper(uint32_t type, uint32_t size) {
  ObjectData* obj = newCollectionHelper(type, size);
  obj->incRefCount();
  return obj;
}

ObjectData* colAddNewElemCHelper(ObjectData* coll, TypedValue value) {
  if (coll->isCollection()) {
    collectionInitAppend(coll, &value);
    // consume the input value. the collection setter either threw or created a
    // reference to value, so we can use a cheaper decref.
    tvRefcountedDecRefNZ(value);
  } else {
    raise_error("ColAddNewElemC: $2 must be a collection");
  }
  return coll;
}

ObjectData* colAddElemCHelper(ObjectData* coll, TypedValue key,
                              TypedValue value) {
  if (coll->isCollection()) {
    collectionSet(coll, &key, &value);
    // consume the input value. the collection setter either threw or created a
    // reference to value, so we can use a cheaper decref.
    tvRefcountedDecRefNZ(value);
  } else {
    raise_error("ColAddNewElemC: $2 must be a collection");
  }
  return coll;
}

//////////////////////////////////////////////////////////////////////

/*
 * The standard VMRegAnchor treatment won't work for some cases called
 * during function preludes.
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
  assert(tl_regState == VMRegState::DIRTY);
  VMExecutionContext* ec = g_vmContext;
  ec->m_stack.top() = (TypedValue*)preLive - preLive->numArgs();
  ActRec* fp = preLive == ec->m_firstAR ?
    ec->m_nestedVMs.back().m_savedState.fp : (ActRec*)preLive->m_savedRbp;
  ec->m_fp = fp;
  ec->m_pc = fp->m_func->unit()->at(fp->m_func->base() + preLive->m_soff);
  tl_regState = VMRegState::CLEAN;
}

void trimExtraArgs(ActRec* ar) {
  assert(!ar->hasInvName());

  sync_regstate_to_caller(ar);
  const Func* f = ar->m_func;
  int numParams = f->numParams();
  int numArgs = ar->numArgs();
  assert(numArgs > numParams);
  int numExtra = numArgs - numParams;

  TRACE(1, "trimExtraArgs: %d args, function %s takes only %d, ar %p\n",
        numArgs, f->name()->data(), numParams, ar);

  if (f->attrs() & AttrMayUseVV) {
    assert(!ar->hasExtraArgs());
    ar->setExtraArgs(ExtraArgs::allocateCopy(
      (TypedValue*)(uintptr_t(ar) - numArgs * sizeof(TypedValue)),
      numArgs - numParams));
  } else {
    // Function is not marked as "MayUseVV", so discard the extra arguments
    TypedValue* tv = (TypedValue*)(uintptr_t(ar) - numArgs*sizeof(TypedValue));
    for (int i = 0; i < numExtra; ++i) {
      tvRefcountedDecRef(tv);
      ++tv;
    }
    ar->setNumArgs(numParams);
  }

  // Only go back to dirty in a non-exception case.  (Same reason as
  // above.)
  tl_regState = VMRegState::DIRTY;
}

void raiseMissingArgument(const char* name, int expected, int got) {
  if (expected == 1) {
    raise_warning(Strings::MISSING_ARGUMENT, name, got);
  } else {
    raise_warning(Strings::MISSING_ARGUMENTS, name, expected, got);
  }
}

RDS::Handle lookupClsRDSHandle(const StringData* name) {
  return Unit::GetNamedEntity(name)->getClassHandle();
}

//////////////////////////////////////////////////////////////////////

uintptr_t tlsBaseNoInline() {
  return tlsBase();
}

//////////////////////////////////////////////////////////////////////

}}
