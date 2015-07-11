/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/object-data.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/mixed-array-defs.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"

#include "hphp/system/systemlib.h"

#include <folly/Hash.h>
#include <folly/ScopeGuard.h>

#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// current maximum object identifier
__thread int ObjectData::os_max_id;

TRACE_SET_MOD(runtime);

const StaticString
  s_offsetGet("offsetGet"),
  s_call("__call"),
  s_serialize("serialize"),
  s_clone("__clone");

const StaticString
  ObjectData::s_serializedNativeDataKey(std::string("\0native", 7));

static Array convert_to_array(const ObjectData* obj, Class* cls) {
  auto const lookup = obj->getProp(cls, s_storage.get());
  auto const prop = lookup.prop;

  // We currently do not special case ArrayObjects / ArrayIterators in
  // reflectionClass. Until, either ArrayObject moves to HNI or a special
  // case is added to reflection unset should be turned off.
  assert(prop && lookup.accessible /* && prop.m_type != KindOfUninit */);
  return tvAsCVarRef(prop).toArray();
}

static_assert(sizeof(ObjectData) == (use_lowptr ? 16 : 24),
              "Change this only on purpose");

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
static void invoke_destructor(ObjectData* obj, const Func* dtor) {
  try {
    // Call the destructor method
    g_context->invokeMethodV(obj, dtor);
  } catch (...) {
    // Swallow any exceptions that escape the __destruct method
    handle_destructor_exception();
  }
}

NEVER_INLINE bool ObjectData::destructImpl() {
  setNoDestruct();
  auto const dtor = m_cls->getDtor();
  if (!dtor) return true;

  // We don't run PHP destructors while we're unwinding for a C++
  // exception.  We want to minimize the PHP code we run while propagating
  // fatals, so we do this check here on a very common path, in the
  // relatively slower case.
  if (g_context->m_unwindingCppException) return true;

  // Some decref paths call release() when --count == 0 and some call it
  // when count == 1. This difference only matters for objects that
  // resurrect themselves in their destructors, so make sure count is
  // consistent here.
  assert(!hasMultipleRefs());
  m_hdr.count = 0;

  // We raise the refcount around the call to __destruct(). This is to
  // prevent the refcount from going to zero when the destructor returns.
  CountableHelper h(this);
  invoke_destructor(this, dtor);
  return getCount() == 1;
}

void ObjectData::destructForExit() {
  assert(RuntimeOption::EnableObjDestructCall);
  auto const dtor = m_cls->getDtor();
  if (dtor) {
    g_context->m_liveBCObjs.erase(this);
  }

  if (noDestruct()) return;
  setNoDestruct();

  // We're exiting, so there should not be any live faults.
  assert(g_context->m_faults.empty());
  assert(!g_context->m_unwindingCppException);

  CountableHelper h(this);
  invoke_destructor(this, dtor);
}

NEVER_INLINE
static void freeDynPropArray(ObjectData* inst) {
  auto& table = g_context->dynPropTable;
  auto it = table.find(inst);
  assert(it != end(table));
  it->second.destroy();
  table.erase(it);
}

NEVER_INLINE
void ObjectData::releaseNoObjDestructCheck() noexcept {
  assert(!hasMultipleRefs());

  auto const attrs = getAttributes();

  if (UNLIKELY(!(attrs & Attribute::NoDestructor))) {
    if (UNLIKELY(!destructImpl())) return;
  }

  auto const cls = getVMClass();

  if (UNLIKELY(attrs & InstanceDtor))  return cls->instanceDtor()(this, cls);

  assert(!cls->preClass()->builtinObjSize());
  assert(!cls->preClass()->builtinODOffset());

  // `this' is being torn down now---be careful about where/how you dereference
  // this from here on.

  auto const nProps = size_t{cls->numDeclProperties()};
  auto prop = reinterpret_cast<TypedValue*>(this + 1);
  auto const stop = prop + nProps;
  for (; prop != stop; ++prop) {
    tvRefcountedDecRef(prop);
  }

  // Deliberately reload `attrs' to check for dynamic properties.  This made
  // gcc generate better code at the time it was done (saving a spill).
  if (UNLIKELY(getAttributes() & HasDynPropArr)) freeDynPropArray(this);

  auto& pmax = os_max_id;
  if (o_id && o_id == pmax) --pmax;

  auto const size =
    reinterpret_cast<char*>(stop) - reinterpret_cast<char*>(this);
  assert(size == sizeForNProps(nProps));
  if (LIKELY(size <= kMaxSmallSize)) {
    return MM().freeSmallSize(this, size);
  }
  MM().freeBigSize(this, size);
}

NEVER_INLINE
static void tail_call_remove_live_bc_obj(ObjectData* obj) {
  g_context->m_liveBCObjs.erase(obj);
  return obj->releaseNoObjDestructCheck();
}

void ObjectData::release() noexcept {
  assert(!hasMultipleRefs());
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall && m_cls->getDtor())) {
    return tail_call_remove_live_bc_obj(this);
  }
  releaseNoObjDestructCheck();
}

///////////////////////////////////////////////////////////////////////////////
// class info

StrNR ObjectData::getClassName() const {
  return m_cls->preClass()->nameStr();
}

bool ObjectData::instanceof(const String& s) const {
  auto const cls = Unit::lookupClass(s.get());
  return cls && instanceof(cls);
}

bool ObjectData::toBooleanImpl() const noexcept {
  // Note: if you add more cases here, hhbbc/class-util.cpp also needs
  // to be changed.
  if (isCollection()) {
    return collections::toBool(this);
  }

  if (instanceof(c_SimpleXMLElement::classof())) {
    // SimpleXMLElement is the only non-collection class that has custom bool
    // casting.
    return c_SimpleXMLElement::ToBool(this);
  }

  always_assert(false);
  return false;
}

int64_t ObjectData::toInt64Impl() const noexcept {
  // SimpleXMLElement is the only class that has proper custom int casting.
  assert(instanceof(c_SimpleXMLElement::classof()));
  return c_SimpleXMLElement::ToInt64(this);
}

double ObjectData::toDoubleImpl() const noexcept {
  // SimpleXMLElement is the only class that has custom double casting.
  assert(instanceof(c_SimpleXMLElement::classof()));
  return c_SimpleXMLElement::ToDouble(this);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

const StaticString s_getIterator("getIterator");

Object ObjectData::iterableObject(bool& isIterable,
                                  bool mayImplementIterator /* = true */) {
  assert(mayImplementIterator || !isIterator());
  if (mayImplementIterator && isIterator()) {
    isIterable = true;
    return Object(this);
  }
  Object obj(this);
  while (obj->instanceof(SystemLib::s_IteratorAggregateClass)) {
    auto iterator = obj->o_invoke_few_args(s_getIterator, 0);
    if (!iterator.isObject()) break;
    auto o = iterator.getObjectData();
    if (o->isIterator()) {
      isIterable = true;
      return o;
    }
    obj = o;
  }
  if (!isIterator() && obj->instanceof(c_SimpleXMLElement::classof())) {
    auto iterator = cast<c_SimpleXMLElement>(obj)
      ->t_getiterator()
      .toObject();
    isIterable = true;
    return iterator;
  }
  isIterable = false;
  return obj;
}

Array& ObjectData::dynPropArray() const {
  assert(getAttribute(HasDynPropArr));
  assert(g_context->dynPropTable.count(this));
  return g_context->dynPropTable[this].arr();
}

Array& ObjectData::reserveProperties(int numDynamic /* = 2 */) {
  if (getAttribute(HasDynPropArr)) {
    return dynPropArray();
  }

  assert(!g_context->dynPropTable.count(this));
  auto& arr = g_context->dynPropTable[this].arr();
  arr = Array::attach(MixedArray::MakeReserveMixed(numDynamic));
  setAttribute(HasDynPropArr);
  return arr;
}

Variant* ObjectData::realPropImpl(const String& propName, int flags,
                                  const String& context,
                                  bool copyDynArray) {
  /*
   * Returns a pointer to a place for a property value. This should never
   * call the magic methods __get or __set. The flags argument describes the
   * behavior in cases where the named property is nonexistent or
   * inaccessible.
   */
  Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  auto const lookup = getPropImpl(ctx, propName.get(), copyDynArray);
  auto const prop = lookup.prop;

  if (!prop) {
    // Property is not declared, and not dynamically created yet.
    if (!(flags & RealPropCreate)) return nullptr;

    return &reserveProperties().lvalAt(propName, AccessFlags::Key);
  }

  // Property is non-NULL if we reach here.
  if ((lookup.accessible && prop->m_type != KindOfUninit) ||
      (flags & (RealPropUnchecked|RealPropExist))) {
    return reinterpret_cast<Variant*>(prop);
  } else {
    return nullptr;
  }
}

Variant* ObjectData::o_realProp(const String& propName, int flags,
                                const String& context /* = null_string */) {
  return realPropImpl(propName, flags, context, true);
}

const Variant* ObjectData::o_realProp(const String& propName, int flags,
                                      const String& context) const {
  return const_cast<ObjectData*>(this)->realPropImpl(propName, flags, context,
                                                     false);
}

inline Variant ObjectData::o_getImpl(const String& propName,
                                     int flags,
                                     bool error /* = true */,
                                     const String& context /*= null_string*/) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  if (const Variant* t = o_realProp(propName, flags, context)) {
    if (t->isInitialized())
      return *t;
  }

  if (getAttribute(UseGet)) {
    TypedValue tvResult;
    tvWriteNull(&tvResult);
    if (invokeGet(&tvResult, propName.get())) {
      return tvAsCVarRef(&tvResult);
    }
  }

  if (error) {
    raise_notice("Undefined property: %s::$%s", getClassName().data(),
                 propName.data());
  }

  return uninit_null();
}

Variant ObjectData::o_get(const String& propName, bool error /* = true */,
                          const String& context /* = null_string */) {
  return o_getImpl(propName, 0, error, context);
}

template <class T>
ALWAYS_INLINE Variant ObjectData::o_setImpl(const String& propName, T v,
                                            const String& context) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  bool useSet = getAttribute(UseSet);
  auto flags = useSet ? 0 : RealPropCreate;

  if (Variant* t = o_realProp(propName, flags, context)) {
    if (!useSet || t->isInitialized()) {
      *t = v;
      return variant(v);
    }
  }

  TypedValue ignored;
  if (useSet &&
      invokeSet(&ignored, propName.get(), (TypedValue*)(&variant(v)))) {
    tvRefcountedDecRef(&ignored);
  }

  return variant(v);
}

Variant ObjectData::o_set(const String& propName, const Variant& v) {
  return o_setImpl<const Variant&>(propName, v, null_string);
}

Variant ObjectData::o_set(const String& propName, const Variant& v,
                          const String& context) {
  return o_setImpl<const Variant&>(propName, v, context);
}

void ObjectData::o_setArray(const Array& properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String k = iter.first().toString();
    Class* ctx = nullptr;
    // If the key begins with a NUL, it's a private or protected property. Read
    // the class name from between the two NUL bytes.
    //
    // Note: if you change this, you need to change similar logic in
    // apc-object.
    if (!k.empty() && k[0] == '\0') {
      int subLen = k.find('\0', 1) + 1;
      String cls = k.substr(1, subLen - 2);
      if (cls.size() == 1 && cls[0] == '*') {
        // Protected.
        ctx = m_cls;
      } else {
        // Private.
        ctx = Unit::lookupClass(cls.get());
        if (!ctx) continue;
      }
      k = k.substr(subLen);
    }

    const Variant& secondRef = iter.secondRef();
    setProp(ctx,
            k.get(),
            // Set prop is happening with WithRef semantics---we only
            // use a binding assignment if it was already KindOfRef,
            // so despite the const_cast here we're safely not
            // modifying the original Variant.
            const_cast<TypedValue*>(secondRef.asTypedValue()),
            secondRef.isReferenced());
  }
}

void ObjectData::o_getArray(Array& props, bool pubOnly /* = false */) const {
  // Fast path for classes with no declared properties
  if (!m_cls->numDeclProperties() && getAttribute(HasDynPropArr)) {
    props = dynPropArray();
    return;
  }
  // The declared properties in the resultant array should be a permutation of
  // propVec. They appear in the following order: go most-to-least-derived in
  // the inheritance hierarchy, inserting properties in declaration order (with
  // the wrinkle that overridden properties should appear only once, with the
  // access level given to it in its most-derived declaration).

  // This is needed to keep track of which elements have been inserted. This is
  // the smoothest way to get overridden properties right.
  std::vector<bool> inserted(m_cls->numDeclProperties(), false);

  // Iterate over declared properties and insert {mangled name --> prop} pairs.
  const Class* cls = m_cls;
  do {
    getProps(cls, pubOnly, cls->preClass(), props, inserted);
    for (auto const& traitCls : cls->usedTraitClasses()) {
      getTraitProps(cls, pubOnly, traitCls.get(), props, inserted);
    }
    cls = cls->parent();
  } while (cls);

  // Iterate over dynamic properties and insert {name --> prop} pairs.
  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    auto& dynProps = dynPropArray();
    if (!dynProps.empty()) {
      for (ArrayIter it(dynProps.get()); !it.end(); it.next()) {
        props.setWithRef(it.first(), it.secondRef(), true);
      }
    }
  }
}

// a constant for arrayobjects that changes the way the array is
// converted to an object
const int64_t ARRAYOBJ_STD_PROP_LIST = 1;

const StaticString s_flags("flags");

Array ObjectData::toArray(bool pubOnly /* = false */) const {
  // We can quickly tell if this object is a collection, which lets us avoid
  // checking for each class in turn if it's not one.
  if (isCollection()) {
    return collections::toArray(this);
  } else if (UNLIKELY(getAttribute(CallToImpl))) {
    // If we end up with other classes that need special behavior, turn the
    // assert into an if and add cases.
    assert(instanceof(c_SimpleXMLElement::classof()));
    return c_SimpleXMLElement::ToArray(this);
  } else if (UNLIKELY(instanceof(SystemLib::s_ArrayObjectClass))) {
    auto const lookup = getProp(SystemLib::s_ArrayObjectClass, s_flags.get());
    auto const flags = lookup.prop;

    if (UNLIKELY(flags->m_type == KindOfInt64 &&
                 flags->m_data.num == ARRAYOBJ_STD_PROP_LIST)) {
      auto ret = Array::Create();
      o_getArray(ret, true);
      return ret;
    }
    return convert_to_array(this, SystemLib::s_ArrayObjectClass);
  } else if (UNLIKELY(instanceof(SystemLib::s_ArrayIteratorClass))) {
    return convert_to_array(this, SystemLib::s_ArrayIteratorClass);
  } else if (UNLIKELY(instanceof(SystemLib::s_ClosureClass))) {
    return Array::Create(Object(const_cast<ObjectData*>(this)));
  } else if (UNLIKELY(instanceof(DateTimeData::getClass()))) {
    return Native::data<DateTimeData>(this)->getDebugInfo();
  } else {
    auto ret = Array::Create();
    o_getArray(ret, pubOnly);
    return ret;
  }
}

namespace {

size_t getPropertyIfAccessible(ObjectData* obj,
                               const Class* ctx,
                               const StringData* key,
                               ObjectData::IterMode mode,
                               Array& properties,
                               size_t propLeft) {
  auto const lookup = obj->getPropImpl(ctx, key, false);
  auto const val = lookup.prop;

  if (lookup.accessible && val->m_type != KindOfUninit) {
    --propLeft;
    if (mode == ObjectData::CreateRefs) {
      if (val->m_type != KindOfRef) tvBox(val);

      properties.setRef(StrNR(key), tvAsVariant(val), true /* isKey */);
    } else if (mode == ObjectData::EraseRefs) {
      properties.set(StrNR(key), tvAsCVarRef(val), true /* isKey */);
    } else {
      properties.setWithRef(VarNR(key), tvAsCVarRef(val), true /* isKey */);
    }
  }
  return propLeft;
}

}

Array ObjectData::o_toIterArray(const String& context, IterMode mode) {
  if (mode == PreserveRefs && !m_cls->numDeclProperties()) {
    if (getAttribute(HasDynPropArr)) return dynPropArray();
    return Array::Create();
  }

  Array* dynProps = nullptr;
  size_t accessibleProps = m_cls->declPropNumAccessible();
  size_t size = accessibleProps;
  if (getAttribute(HasDynPropArr)) {
    dynProps = &dynPropArray();
    size += dynProps->size();
  }
  Array retArray { Array::attach(MixedArray::MakeReserveMixed(size)) };

  Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  // Get all declared properties first, bottom-to-top in the inheritance
  // hierarchy, in declaration order.
  const Class* klass = m_cls;
  while (klass) {
    const PreClass::Prop* props = klass->preClass()->properties();
    const size_t numProps = klass->preClass()->numProperties();

    for (size_t i = 0; i < numProps; ++i) {
      auto key = const_cast<StringData*>(props[i].name());
      accessibleProps = getPropertyIfAccessible(
          this, ctx, key, mode, retArray, accessibleProps);
    }
    klass = klass->parent();
  }
  if (!RuntimeOption::RepoAuthoritative && accessibleProps > 0) {
    // we may have properties from traits
    const auto* props = m_cls->declProperties();
    auto numDeclProp = m_cls-> numDeclProperties();
    for (size_t i = 0; i < numDeclProp; i++) {
      const auto* key = props[i].m_name.get();
      if (!retArray.get()->exists(key)) {
        accessibleProps = getPropertyIfAccessible(
            this, ctx, key, mode, retArray, accessibleProps);
        if (accessibleProps == 0) break;
      }
    }
  }

  // Now get dynamic properties.
  if (dynProps) {
    auto ad = dynProps->get();
    ssize_t iter = ad->iter_begin();
    auto pos_limit = ad->iter_end();
    while (iter != pos_limit) {
      TypedValue key;
      dynProps->get()->nvGetKey(&key, iter);
      iter = dynProps->get()->iter_advance(iter);

      // You can get this if you cast an array to object. These
      // properties must be dynamic because you can't declare a
      // property with a non-string name.
      if (UNLIKELY(!IS_STRING_TYPE(key.m_type))) {
        assert(key.m_type == KindOfInt64);
        switch (mode) {
        case CreateRefs: {
          auto& lval = dynProps->lvalAt(key.m_data.num);
          retArray.setRef(key.m_data.num, lval);
        }
        case EraseRefs: {
          auto const val = dynProps->get()->nvGet(key.m_data.num);
          retArray.set(key.m_data.num, tvAsCVarRef(val));
        }
        case PreserveRefs: {
          auto const val = dynProps->get()->nvGet(key.m_data.num);
          retArray.setWithRef(key.m_data.num, tvAsCVarRef(val));
        }
        }
        continue;
      }

      auto const strKey = key.m_data.pstr;
      switch (mode) {
      case CreateRefs: {
        auto& lval = dynProps->lvalAt(StrNR(strKey), AccessFlags::Key);
        retArray.setRef(StrNR(strKey), lval, true /* isKey */);
      }
      case EraseRefs: {
        auto const val = dynProps->get()->nvGet(strKey);
        retArray.set(StrNR(strKey), tvAsCVarRef(val), true /* isKey */);
      }
      case PreserveRefs: {
        auto const val = dynProps->get()->nvGet(strKey);
        retArray.setWithRef(VarNR(strKey), tvAsCVarRef(val), true /* isKey */);
      }
      }
      decRefStr(strKey);
    }
  }

  return retArray;
}

static bool decode_invoke(const String& s, ObjectData* obj, bool fatal,
                          CallCtx& ctx) {
  ctx.this_ = obj;
  ctx.cls = obj->getVMClass();
  ctx.invName = nullptr;

  ctx.func = ctx.cls->lookupMethod(s.get());
  if (ctx.func) {
    // Null out this_ for static methods, unless it's a closure.
    //
    // Closures will sort out $this for themselves downstream, and
    // they need this one because it's the closure object.
    if ((ctx.func->attrs() & AttrStatic) &&
        !ctx.func->isClosureBody()) {
      ctx.this_ = nullptr;
    }
  } else {
    // If this_ is non-null AND we could not find a method, try
    // looking up __call in cls's method table
    ctx.func = ctx.cls->lookupMethod(s_call.get());

    if (!ctx.func) {
      // Bail if we couldn't find the method or __call
      o_invoke_failed(ctx.cls->name()->data(), s.data(), fatal);
      return false;
    }
    // We found __call! Stash the original name into invName.
    assert(!(ctx.func->attrs() & AttrStatic));
    ctx.invName = s.get();
    ctx.invName->incRefCount();
  }
  return true;
}

Variant ObjectData::o_invoke(const String& s, const Variant& params,
                             bool fatal /* = true */) {
  CallCtx ctx;
  if (!decode_invoke(s, this, fatal, ctx) ||
      (!isContainer(params) && !params.isNull())) {
    return Variant(Variant::NullInit());
  }
  Variant ret;
  g_context->invokeFunc((TypedValue*)&ret, ctx, params);
  return ret;
}

#define INVOKE_FEW_ARGS_IMPL3                        \
  const Variant& a0, const Variant& a1, const Variant& a2
#define INVOKE_FEW_ARGS_IMPL6                        \
  INVOKE_FEW_ARGS_IMPL3,                             \
  const Variant& a3, const Variant& a4, const Variant& a5
#define INVOKE_FEW_ARGS_IMPL10                       \
  INVOKE_FEW_ARGS_IMPL6,                             \
  const Variant& a6, const Variant& a7, const Variant& a8, const Variant& a9
#define INVOKE_FEW_ARGS_IMPL_ARGS INVOKE_FEW_ARGS(IMPL,INVOKE_FEW_ARGS_COUNT)

Variant ObjectData::o_invoke_few_args(const String& s, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {

  CallCtx ctx;
  if (!decode_invoke(s, this, true, ctx)) {
    return Variant(Variant::NullInit());
  }

  TypedValue args[INVOKE_FEW_ARGS_COUNT];
  switch(count) {
    default: not_implemented();
#if INVOKE_FEW_ARGS_COUNT > 6
    case 10: tvCopy(*a9.asTypedValue(), args[9]);
    case  9: tvCopy(*a8.asTypedValue(), args[8]);
    case  8: tvCopy(*a7.asTypedValue(), args[7]);
    case  7: tvCopy(*a6.asTypedValue(), args[6]);
#endif
#if INVOKE_FEW_ARGS_COUNT > 3
    case  6: tvCopy(*a5.asTypedValue(), args[5]);
    case  5: tvCopy(*a4.asTypedValue(), args[4]);
    case  4: tvCopy(*a3.asTypedValue(), args[3]);
#endif
    case  3: tvCopy(*a2.asTypedValue(), args[2]);
    case  2: tvCopy(*a1.asTypedValue(), args[1]);
    case  1: tvCopy(*a0.asTypedValue(), args[0]);
    case  0: break;
  }

  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), ctx, count, args);
  return ret;
}

const StaticString
  s_zero("\0", 1),
  s_protected_prefix("\0*\0", 3);

void ObjectData::serialize(VariableSerializer* serializer) const {
  if (UNLIKELY(serializer->incNestedLevel((void*)this, true))) {
    serializer->writeOverflow((void*)this, true);
  } else {
    serializeImpl(serializer);
  }
  serializer->decNestedLevel((void*)this);
}

const StaticString
  s_PHP_DebugDisplay("__PHP_DebugDisplay"),
  s_PHP_Incomplete_Class("__PHP_Incomplete_Class"),
  s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name"),
  s_debugInfo("__debugInfo");

/* Get properties from the actual object unless we're
 * serializing for var_dump()/print_r() and the object
 * exports a __debugInfo() magic method.
 * In which case, call that and use the array it returns.
 */
inline Array getSerializeProps(const ObjectData* obj,
                               VariableSerializer* serializer) {
  if (serializer->getType() == VariableSerializer::Type::VarExport) {
    Array props = Array::Create();
    for (ArrayIter iter(obj->toArray()); iter; ++iter) {
      auto key = iter.first().toString();
      // Jump over any class attribute mangling
      if (key[0] == '\0' && key.size() > 0) {
        int sizeToCut = 0;
        do {
          sizeToCut++;
        } while (key[sizeToCut] != '\0');
        key = key.substr(sizeToCut+1);
      }
      props.setWithRef(key, iter.secondRef());
    }
    return props;
  }
  if ((serializer->getType() != VariableSerializer::Type::PrintR) &&
      (serializer->getType() != VariableSerializer::Type::VarDump)) {
    return obj->toArray();
  }
  auto cls = obj->getVMClass();
  auto debuginfo = cls->lookupMethod(s_debugInfo.get());
  if (!debuginfo) {
    // When ArrayIterator is cast to an array, it returns its array object,
    // however when it's being var_dump'd or print_r'd, it shows its properties
    if (UNLIKELY(obj->instanceof(SystemLib::s_ArrayIteratorClass))) {
      auto ret = Array::Create();
      obj->o_getArray(ret);
      return ret;
    }

    // Same with Closure, since it's a dynamic object but still has its own
    // different behavior for var_dump and cast to array
    if (UNLIKELY(obj->instanceof(SystemLib::s_ClosureClass))) {
      auto ret = Array::Create();
      obj->o_getArray(ret);
      return ret;
    }

    return obj->toArray();
  }
  if (debuginfo->attrs() & (AttrPrivate|AttrProtected|
                            AttrAbstract|AttrStatic)) {
    raise_warning("%s::__debugInfo() must be public and non-static",
                  cls->name()->data());
    return obj->toArray();
  }
  Variant ret = const_cast<ObjectData*>(obj)->o_invoke_few_args(s_debugInfo, 0);
  if (ret.isArray()) {
    return ret.toArray();
  }
  if (ret.isNull()) {
    return empty_array();
  }
  raise_error("__debugInfo() must return an array");
  not_reached();
}

void ObjectData::serializeImpl(VariableSerializer* serializer) const {
  bool handleSleep = false;
  Variant serializableNativeData = init_null();
  Variant ret;

  if (LIKELY(serializer->getType() == VariableSerializer::Type::Serialize ||
             serializer->getType() == VariableSerializer::Type::APCSerialize)) {
    if (instanceof(SystemLib::s_SerializableClass)) {
      assert(!isCollection());
      Variant ret =
        const_cast<ObjectData*>(this)->o_invoke_few_args(s_serialize, 0);
      if (ret.isString()) {
        serializer->writeSerializableObject(getClassName(), ret.toString());
      } else if (ret.isNull()) {
        serializer->writeNull();
      } else {
        raise_error("%s::serialize() must return a string or NULL",
                    getClassName().data());
      }
      return;
    }
    // Only serialize CPP extension type instances which can actually
    // be deserialized.  Otherwise, raise a warning and serialize
    // null.
    auto cls = getVMClass();
    if (cls->instanceCtor() && !cls->isCppSerializable()) {
      raise_warning("Attempted to serialize unserializable builtin class %s",
        getVMClass()->preClass()->name()->data());
      Variant placeholder = init_null();
      serializeVariant(placeholder, serializer);
      return;
    }
    if (getAttribute(HasSleep)) {
      handleSleep = true;
      ret = const_cast<ObjectData*>(this)->invokeSleep();
    }
    if (getAttribute(HasNativeData)) {
      auto* ndi = cls->getNativeDataInfo();
      if (ndi->isSerializable()) {
        serializableNativeData = Native::nativeDataSleep(this);
      }
    }
  } else if (UNLIKELY(serializer->getType() ==
                      VariableSerializer::Type::DebuggerSerialize)) {
    // Don't try to serialize a CPP extension class which doesn't
    // support serialization. Just send the class name instead.
    if (getAttribute(IsCppBuiltin) && !getVMClass()->isCppSerializable()) {
      serializer->write(getClassName());
      return;
    }
  }

  if (UNLIKELY(handleSleep)) {
    assert(!isCollection());
    if (ret.isArray()) {
      Array wanted = Array::Create();
      assert(ret.getRawType() == KindOfArray); // can't be KindOfRef
      const Array &props = ret.asCArrRef();
      for (ArrayIter iter(props); iter; ++iter) {
        String memberName = iter.second().toString();
        String propName = memberName;
        Class* ctx = m_cls;
        auto attrMask = AttrNone;
        if (memberName.data()[0] == 0) {
          int subLen = memberName.find('\0', 1) + 1;
          if (subLen > 2) {
            if (subLen == 3 && memberName.data()[1] == '*') {
              attrMask = AttrProtected;
              memberName = memberName.substr(subLen);
            } else {
              attrMask = AttrPrivate;
              String cls = memberName.substr(1, subLen - 2);
              ctx = Unit::lookupClass(cls.get());
              if (ctx) {
                memberName = memberName.substr(subLen);
              } else {
                ctx = m_cls;
              }
            }
          }
        }

        auto const lookup = m_cls->getDeclPropIndex(ctx, memberName.get());
        auto const propIdx = lookup.prop;

        if (propIdx != kInvalidSlot) {
          if (lookup.accessible) {
            auto const prop = &propVec()[propIdx];
            if (prop->m_type != KindOfUninit) {
              auto const attrs = m_cls->declProperties()[propIdx].m_attrs;
              if (attrs & AttrPrivate) {
                memberName = concat4(s_zero, ctx->nameStr(),
                                     s_zero, memberName);
              } else if (attrs & AttrProtected) {
                memberName = concat(s_protected_prefix, memberName);
              }
              if (!attrMask || (attrMask & attrs) == attrMask) {
                wanted.set(memberName, tvAsCVarRef(prop));
                continue;
              }
            }
          }
        }
        if (!attrMask && UNLIKELY(getAttribute(HasDynPropArr))) {
          const TypedValue* prop = dynPropArray()->nvGet(propName.get());
          if (prop) {
            wanted.set(propName, tvAsCVarRef(prop));
            continue;
          }
        }
        raise_notice("serialize(): \"%s\" returned as member variable from "
                     "__sleep() but does not exist", propName.data());
        wanted.set(propName, init_null());
      }
      serializer->pushObjectInfo(getClassName(), getId(), 'O');
      if (!serializableNativeData.isNull()) {
        wanted.set(s_serializedNativeDataKey, serializableNativeData);
      }
      wanted.serialize(serializer, true);
      serializer->popObjectInfo();
    } else {
      raise_notice("serialize(): __sleep should return an array only "
                   "containing the names of instance-variables to "
                   "serialize");
      serializeVariant(uninit_null(), serializer);
    }
  } else {
    if (isCollection()) {
      collections::serialize(const_cast<ObjectData*>(this), serializer);
    } else if (serializer->getType() == VariableSerializer::Type::VarExport &&
               instanceof(c_Closure::classof())) {
      serializer->write(getClassName());
    } else {
      auto className = getClassName();
      Array properties = getSerializeProps(this, serializer);
      if (serializer->getType() ==
        VariableSerializer::Type::DebuggerSerialize) {
        try {
           auto val = const_cast<ObjectData*>(this)->invokeToDebugDisplay();
           if (val.isInitialized()) {
             properties.lvalAt(s_PHP_DebugDisplay).assign(val);
           }
        } catch (...) {
          raise_warning("%s::__toDebugDisplay() throws exception",
            getClassName().data());
        }
      }
      if (serializer->getType() == VariableSerializer::Type::DebuggerDump) {
        const Variant* debugDispVal = o_realProp(s_PHP_DebugDisplay, 0);
        if (debugDispVal) {
          serializeVariant(*debugDispVal, serializer, false, false, true);
          return;
        }
      }
      if (serializer->getType() != VariableSerializer::Type::VarDump &&
          className.asString() == s_PHP_Incomplete_Class) {
        const Variant* cname = o_realProp(s_PHP_Incomplete_Class_Name, 0);
        if (cname && cname->isString()) {
          serializer->pushObjectInfo(cname->toCStrRef(), getId(), 'O');
          properties.remove(s_PHP_Incomplete_Class_Name, true);
          properties.serialize(serializer, true);
          serializer->popObjectInfo();
          return;
        }
      }
      serializer->pushObjectInfo(className, getId(), 'O');
      if (!serializableNativeData.isNull()) {
        properties.set(s_serializedNativeDataKey, serializableNativeData);
      }
      properties.serialize(serializer, true);
      serializer->popObjectInfo();
    }
  }
}

ObjectData* ObjectData::clone() {
  if (getAttribute(HasClone) && getAttribute(IsCppBuiltin)) {
    if (isCollection()) {
      return collections::clone(this);
    } else if (instanceof(c_Closure::classof())) {
      return c_Closure::Clone(this);
    } else if (instanceof(c_SimpleXMLElement::classof())) {
      return c_SimpleXMLElement::Clone(this);
    }
    always_assert(false);
  }
  return cloneImpl();
}

Variant ObjectData::offsetGet(Variant key) {
  assert(instanceof(SystemLib::s_ArrayAccessClass));

  auto const method = m_cls->lookupMethod(s_offsetGet.get());
  assert(method);

  return g_context->invokeMethodV(this, method, InvokeArgs(key.asCell(), 1));
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s___get(LITSTR_INIT("__get")),
  s___set(LITSTR_INIT("__set")),
  s___isset(LITSTR_INIT("__isset")),
  s___unset(LITSTR_INIT("__unset")),
  s___init__(LITSTR_INIT("__init__")),
  s___sleep(LITSTR_INIT("__sleep")),
  s___toDebugDisplay(LITSTR_INIT("__toDebugDisplay")),
  s___wakeup(LITSTR_INIT("__wakeup"));

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps) {
  auto* dst = propVec;
  auto* src = propData;
  for (; src != propData + nProps; ++src, ++dst) {
    *dst = *src;
    // m_aux.u_deepInit is true for properties that need "deep" initialization
    if (src->deepInit()) {
      tvRefcountedIncRef(dst);
      collections::deepCopy(dst);
    }
  }
}

TypedValue* ObjectData::propVec() {
  auto const ret = reinterpret_cast<uintptr_t>(this + 1);
  if (UNLIKELY(getAttribute(IsCppBuiltin))) {
    return reinterpret_cast<TypedValue*>(ret + m_cls->builtinODTailSize());
  }
  return reinterpret_cast<TypedValue*>(ret);
}

const TypedValue* ObjectData::propVec() const {
  return const_cast<ObjectData*>(this)->propVec();
}

/**
 * Only call this if cls->callsCustomInstanceInit() is true
 */
ObjectData* ObjectData::callCustomInstanceInit() {
  auto const init = m_cls->lookupMethod(s___init__.get());
  assert(init);

  // No need to inc-ref here because we're a newly created object and our
  // ref-count starts at 1.
  try {
    DEBUG_ONLY auto const tv = g_context->invokeMethod(this, init);
    assert(!IS_REFCOUNTED_TYPE(tv.m_type));
  } catch (...) {
    this->setNoDestruct();
    decRefObj(this);
    throw;
  }

  return this;
}

// called from jit code
ObjectData* ObjectData::newInstanceRaw(Class* cls, uint32_t size) {
  auto o = new (MM().mallocSmallSize(size)) ObjectData(cls, NoInit{});
  assert(o->hasExactlyOneRef());
  return o;
}

// called from jit code
ObjectData* ObjectData::newInstanceRawBig(Class* cls, size_t size) {
  auto o = new (MM().mallocBigSize<false>(size).ptr)
    ObjectData(cls, NoInit{});
  assert(o->hasExactlyOneRef());
  return o;
}

// Note: the normal object destruction path does not actually call this
// destructor.  See ObjectData::release.
ObjectData::~ObjectData() {
  int& pmax = os_max_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
  if (UNLIKELY(getAttribute(HasDynPropArr))) freeDynPropArray(this);
}

Object ObjectData::FromArray(ArrayData* properties) {
  Object retval{SystemLib::s_stdclassClass};
  retval->setAttribute(HasDynPropArr);
  g_context->dynPropTable.emplace(retval.get(), properties);
  return retval;
}

Slot ObjectData::declPropInd(TypedValue* prop) const {
  // Do an address range check to determine whether prop physically resides
  // in propVec.
  const TypedValue* pv = propVec();
  if (prop >= pv && prop < &pv[m_cls->numDeclProperties()]) {
    return prop - pv;
  } else {
    return kInvalidSlot;
  }
}

ObjectData::PropLookup<TypedValue*> ObjectData::getPropImpl(
  const Class* ctx,
  const StringData* key,
  bool copyDynArray
) {
  auto const lookup = m_cls->getDeclPropIndex(ctx, key);
  auto const propIdx = lookup.prop;

  if (LIKELY(propIdx != kInvalidSlot)) {
    // We found a visible property, but it might not be accessible.  No need to
    // check if there is a dynamic property with this name.
    auto const prop = &propVec()[propIdx];

    if (debug) {
      if (RuntimeOption::RepoAuthoritative) {
        auto const repoTy = m_cls->declPropRepoAuthType(propIdx);
        always_assert(tvMatchesRepoAuthType(*prop, repoTy));
      }
    }

    return PropLookup<TypedValue*> { prop, lookup.accessible };
  }

  // We could not find a visible declared property. We need to check for a
  // dynamic property with this name.
  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    if (auto prop = dynPropArray()->nvGet(key)) {
      // If we may write to the property we need to allow the array to escalate
      if (copyDynArray) {
        prop = dynPropArray().lvalAt(StrNR(key),
                                     AccessFlags::Key).asTypedValue();
      }

      // Returning a non-declared property, we know that it is accessible since
      // all dynamic properties are.
      return PropLookup<TypedValue*> { const_cast<TypedValue*>(prop), true };
    }
  }

  return PropLookup<TypedValue*> { nullptr, false };
}

ObjectData::PropLookup<TypedValue*> ObjectData::getProp(
  const Class* ctx,
  const StringData* key
) {
  return getPropImpl(ctx, key, true);
}

ObjectData::PropLookup<const TypedValue*> ObjectData::getProp(
  const Class* ctx,
  const StringData* key
) const {
  auto const lookup = const_cast<ObjectData*>(this)->
    getPropImpl(ctx, key, false);
  return PropLookup<const TypedValue*> { lookup.prop, lookup.accessible };
}

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Recursion of magic property accessors is allowed, but if you
 * recurse on the same object, for the same property, for the same
 * kind of magic method, it doesn't actually enter the magic method
 * anymore.  This matches zend behavior.
 *
 * This means we need to track all active property getters and ensure
 * we aren't recursing for the same one.  Since most accesses to magic
 * property getters aren't going to recurse, we optimize for the case
 * where only a single getter is active.  If it recurses again, we
 * promote to a hash set to track all the information needed.
 *
 * The various invokeFoo functions are the entry points here.  They
 * require that the appropriate ObjectData::Attribute has been checked
 * first, and return false if they refused to run the magic method due
 * to a recursion error.
 */

struct PropAccessInfo {
  struct Hash;

  bool operator==(const PropAccessInfo& o) const {
    return obj == o.obj && attr == o.attr && key->same(o.key);
  }

  ObjectData* obj;
  const StringData* key;      // note: not necessarily static
  ObjectData::Attribute attr;
};

struct PropAccessInfo::Hash {
  size_t operator()(PropAccessInfo const& info) const {
    return folly::hash::hash_combine(
      hash_int64(reinterpret_cast<intptr_t>(info.obj)),
      info.key->hash(),
      static_cast<uint32_t>(info.attr)
    );
  }
};

struct PropRecurInfo {
  typedef req::hash_set<PropAccessInfo,PropAccessInfo::Hash> RecurSet;
  const PropAccessInfo* activePropInfo;
  RecurSet* activeSet;
};

__thread PropRecurInfo propRecurInfo;

template<class Invoker>
bool magic_prop_impl(TypedValue* retval,
                     const StringData* key,
                     const PropAccessInfo& info,
                     Invoker invoker) {
  if (UNLIKELY(propRecurInfo.activePropInfo != nullptr)) {
    if (!propRecurInfo.activeSet) {
      propRecurInfo.activeSet = req::make_raw<PropRecurInfo::RecurSet>();
      propRecurInfo.activeSet->insert(*propRecurInfo.activePropInfo);
    }
    if (!propRecurInfo.activeSet->insert(info).second) {
      // We're already running a magic method on the same type here.
      return false;
    }
    SCOPE_EXIT {
      propRecurInfo.activeSet->erase(info);
    };

    invoker();
    return true;
  }

  propRecurInfo.activePropInfo = &info;
  SCOPE_EXIT {
    propRecurInfo.activePropInfo = nullptr;
    if (UNLIKELY(propRecurInfo.activeSet != nullptr)) {
      req::destroy_raw(propRecurInfo.activeSet);
      propRecurInfo.activeSet = nullptr;
    }
  };

  invoker();
  return true;
}

// Helper for making invokers for the single-argument magic property
// methods.  __set takes 2 args, so it uses its own function.
struct MagicInvoker {
  TypedValue* retval;
  const StringData* magicFuncName;
  const PropAccessInfo& info;

  void operator()() const {
    auto const meth = info.obj->getVMClass()->lookupMethod(magicFuncName);
    TypedValue args[1] = {
      make_tv<KindOfString>(const_cast<StringData*>(info.key))
    };
    *retval = g_context->invokeMethod(info.obj, meth, folly::range(args));
  }
};

}

bool ObjectData::invokeSet(TypedValue* retval, const StringData* key,
                           TypedValue* val) {
  auto const info = PropAccessInfo { this, key, UseSet };
  return magic_prop_impl(
    retval,
    key,
    info,
    [&] {
      auto const meth = m_cls->lookupMethod(s___set.get());
      TypedValue args[2] = {
        make_tv<KindOfString>(const_cast<StringData*>(key)),
        *tvToCell(val)
      };
      *retval = g_context->invokeMethod(this, meth, folly::range(args));
    }
  );
}

bool ObjectData::invokeGet(TypedValue* retval, const StringData* key) {
  auto const info = PropAccessInfo { this, key, UseGet };
  return magic_prop_impl(
    retval,
    key,
    info,
    MagicInvoker { retval, s___get.get(), info }
  );
}

bool ObjectData::invokeIsset(TypedValue* retval, const StringData* key) {
  auto const info = PropAccessInfo { this, key, UseIsset };
  return magic_prop_impl(
    retval,
    key,
    info,
    MagicInvoker { retval, s___isset.get(), info }
  );
}

bool ObjectData::invokeUnset(TypedValue* retval, const StringData* key) {
  auto const info = PropAccessInfo { this, key, UseUnset };
  return magic_prop_impl(
    retval,
    key,
    info,
    MagicInvoker { retval, s___unset.get(), info }
  );
}

static bool guardedNativePropResult(TypedValue* retval, Variant result) {
  if (!Native::isPropHandled(result)) {
    return false;
  }
  tvDup(*result.asTypedValue(), *retval);
  return true;
}

bool ObjectData::invokeNativeGetProp(TypedValue* retval,
                                     const StringData* key) {
  return guardedNativePropResult(retval, Native::getProp(this, StrNR(key)));
}

bool ObjectData::invokeNativeSetProp(TypedValue* retval,
                                     const StringData* key,
                                     TypedValue* val) {
  return guardedNativePropResult(
    retval,
    Native::setProp(this, StrNR(key), tvAsVariant(val))
  );
}

bool ObjectData::invokeNativeIssetProp(TypedValue* retval,
                                       const StringData* key) {
  return guardedNativePropResult(retval, Native::issetProp(this, StrNR(key)));
}

bool ObjectData::invokeNativeUnsetProp(TypedValue* retval,
                                       const StringData* key) {
  return guardedNativePropResult(retval, Native::unsetProp(this, StrNR(key)));
}

//////////////////////////////////////////////////////////////////////

template <bool warn, bool define>
TypedValue* ObjectData::propImpl(
  TypedValue* tvScratch,
  TypedValue* tvRef,
  Class* ctx,
  const StringData* key
) {
  auto const lookup = getProp(ctx, key);
  auto const prop = lookup.prop;

  if (prop) {
    if (lookup.accessible) {
      // Property exists, is accessible, and is not unset.
      if (prop->m_type != KindOfUninit) return prop;

      // Property is unset, try __get.
      if (getAttribute(UseGet) && invokeGet(tvRef, key)) return tvRef;

      if (warn) raiseUndefProp(key);

      if (define) return prop;
      return const_cast<TypedValue*>(init_null_variant.asTypedValue());
    }

    // Property is not accessible, try __get.
    if (getAttribute(UseGet) && invokeGet(tvRef, key)) {
      return tvRef;
    }

    // Property exists, but it is either protected or private since accessible
    // is false.
    auto const propInd = m_cls->lookupDeclProp(key);
    auto const attrs = m_cls->declProperties()[propInd].m_attrs;
    auto const priv = (attrs & AttrPrivate) ? "private" : "protected";

    raise_error(
      "Cannot access %s property %s::$%s",
      priv,
      m_cls->preClass()->name()->data(),
      key->data()
    );

    *tvScratch = make_tv<KindOfUninit>();
    return tvScratch;
  }

  // First see if native getter is implemented.
  if (getAttribute(HasNativePropHandler) && invokeNativeGetProp(tvRef, key)) {
    return tvRef;
  }

  // Next try calling user-level `__get` if it's used.
  if (getAttribute(UseGet) && invokeGet(tvRef, key)) {
    return tvRef;
  }

  if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
    *tvScratch = make_tv<KindOfUninit>();
    return tvScratch;
  }

  if (warn) raiseUndefProp(key);

  if (define) {
    auto& var = reserveProperties().lvalAt(StrNR(key), AccessFlags::Key);
    return var.asTypedValue();
  }

  return const_cast<TypedValue*>(init_null_variant.asTypedValue());
}

TypedValue* ObjectData::prop(
  TypedValue* tvScratch,
  TypedValue* tvRef,
  Class* ctx,
  const StringData* key
) {
  return propImpl<false, false>(tvScratch, tvRef, ctx, key);
}

TypedValue* ObjectData::propD(
  TypedValue* tvScratch,
  TypedValue* tvRef,
  Class* ctx,
  const StringData* key
) {
  return propImpl<false, true>(tvScratch, tvRef, ctx, key);
}

TypedValue* ObjectData::propW(
  TypedValue* tvScratch,
  TypedValue* tvRef,
  Class* ctx,
  const StringData* key
) {
  return propImpl<true, false>(tvScratch, tvRef, ctx, key);
}

TypedValue* ObjectData::propWD(
  TypedValue* tvScratch,
  TypedValue* tvRef,
  Class* ctx,
  const StringData* key
) {
  return propImpl<true, true>(tvScratch, tvRef, ctx, key);
}

bool ObjectData::propIsset(const Class* ctx, const StringData* key) {
  auto const lookup = getPropImpl(ctx, key, false);
  auto const prop = lookup.prop;

  if (prop && lookup.accessible && prop->m_type != KindOfUninit) {
    return !cellIsNull(tvToCell(prop));
  }

  auto tv = make_tv<KindOfUninit>();

  if (getAttribute(HasNativePropHandler) && invokeNativeIssetProp(&tv, key)) {
    tvCastToBooleanInPlace(&tv);
    return tv.m_data.num;
  }

  if (!getAttribute(UseIsset) || !invokeIsset(&tv, key)) return false;

  tvCastToBooleanInPlace(&tv);
  return tv.m_data.num;
}

bool ObjectData::propEmptyImpl(const Class* ctx, const StringData* key) {
  auto const lookup = getPropImpl(ctx, key, false);
  auto const prop = lookup.prop;

  if (prop && lookup.accessible && prop->m_type != KindOfUninit) {
    return !cellToBool(*tvToCell(prop));
  }

  auto tv = make_tv<KindOfUninit>();

  if (getAttribute(HasNativePropHandler) && invokeNativeIssetProp(&tv, key)) {
    tvCastToBooleanInPlace(&tv);
    if (!tv.m_data.num) {
      return true;
    }
    if (invokeNativeGetProp(&tv, key)) {
      auto const emptyResult = !cellToBool(*tvToCell(&tv));
      tvRefcountedDecRef(&tv);
      return emptyResult;
    }
    return false;
  }

  if (!getAttribute(UseIsset) || !invokeIsset(&tv, key)) return true;

  tvCastToBooleanInPlace(&tv);
  if (!tv.m_data.num) return true;

  if (getAttribute(UseGet)) {
    if (invokeGet(&tv, key)) {
      auto const emptyResult = !cellToBool(*tvToCell(&tv));
      tvRefcountedDecRef(&tv);
      return emptyResult;
    }
  }
  return false;
}

bool ObjectData::propEmpty(const Class* ctx, const StringData* key) {
  if (UNLIKELY(getAttribute(HasPropEmpty))) {
    if (instanceof(c_SimpleXMLElement::classof())) {
      return c_SimpleXMLElement::PropEmpty(this, key);
    }
  }
  return propEmptyImpl(ctx, key);
}

void ObjectData::setProp(Class* ctx,
                         const StringData* key,
                         TypedValue* val,
                         bool bindingAssignment /* = false */) {
  auto const lookup = getProp(ctx, key);
  auto const prop = lookup.prop;

  if (prop && lookup.accessible) {
    TypedValue ignored;
    if (prop->m_type != KindOfUninit ||
        !getAttribute(UseSet) ||
        !invokeSet(&ignored, key, val)) {
      if (UNLIKELY(bindingAssignment)) {
        tvBind(val, prop);
      } else {
        tvSet(*val, *prop);
      }
      return;
    }
    tvRefcountedDecRef(&ignored);
    return;
  }

  TypedValue ignored;

  // First see if native setter is implemented.
  if (getAttribute(HasNativePropHandler) &&
    invokeNativeSetProp(&ignored, key, val)) {
    tvRefcountedDecRef(&ignored);
    return;
  }

  // Then go to user-level `__set`.
  if (!getAttribute(UseSet) || !invokeSet(&ignored, key, val)) {
    if (prop) {
      /*
       * Note: this differs from Zend right now in the case of a
       * failed recursive __set.  In Zend, the __set is silently
       * dropped, and the protected property is not modified.
       */
      raise_error("Cannot access protected property");
    }
    if (UNLIKELY(!*key->data())) {
      throw_invalid_property_name(StrNR(key));
    }
    // when seting a dynamic property, do not write
    // directly to the TypedValue in the MixedArray, since
    // its m_aux field is used to store the string hash of
    // the property name. Instead, call the appropriate
    // setters (set() or setRef()).
    if (UNLIKELY(bindingAssignment)) {
      reserveProperties().setRef(
        StrNR(key), tvAsVariant(val), true /* isKey */);
    } else {
      reserveProperties().set(
        StrNR(key), tvAsCVarRef(val), true /* isKey */);
    }
    return;
  }

  tvRefcountedDecRef(&ignored);
}

TypedValue* ObjectData::setOpProp(TypedValue& tvRef,
                                  Class* ctx,
                                  SetOpOp op,
                                  const StringData* key,
                                  Cell* val) {
  auto const lookup = getProp(ctx, key);
  auto prop = lookup.prop;

  if (prop && lookup.accessible) {
    if (prop->m_type == KindOfUninit && getAttribute(UseGet)) {
      auto tvResult = make_tv<KindOfUninit>();
      if (invokeGet(&tvResult, key)) {
        SETOP_BODY(&tvResult, op, val);
        if (getAttribute(UseSet)) {
          assert(tvRef.m_type == KindOfUninit);
          cellDup(*tvToCell(&tvResult), tvRef);
          TypedValue ignored;
          if (invokeSet(&ignored, key, &tvRef)) {
            tvRefcountedDecRef(&ignored);
            return &tvRef;
          }
          tvRef.m_type = KindOfUninit;
        }
        cellDup(*tvToCell(&tvResult), *prop);
        return prop;
      }
    }

    prop = tvToCell(prop);
    SETOP_BODY_CELL(prop, op, val);
    return prop;
  }

  if (UNLIKELY(!*key->data())) throw_invalid_property_name(StrNR(key));

  // Native accessors.
  if (getAttribute(HasNativePropHandler)) {
    if (invokeNativeGetProp(&tvRef, key)) {
      SETOP_BODY(&tvRef, op, val);
      TypedValue ignored;
      if (invokeNativeSetProp(&ignored, key, &tvRef)) {
        tvRefcountedDecRef(&ignored);
        return &tvRef;
      }
    }
  }

  auto const useSet = getAttribute(UseSet);
  auto const useGet = getAttribute(UseGet);

  if (useGet && !useSet) {
    auto tvResult = make_tv<KindOfNull>();
    // If invokeGet fails due to recursion, it leaves the KindOfNull.
    invokeGet(&tvResult, key);

    // Note: the tvUnboxIfNeeded comes *after* the setop on purpose
    // here, even though it comes before the IncDecOp in the analogous
    // situation in incDecProp.  This is to match zend 5.5 behavior.
    SETOP_BODY(&tvResult, op, val);
    tvUnboxIfNeeded(&tvResult);

    if (prop) raise_error("Cannot access protected property");
    prop = reinterpret_cast<TypedValue*>(
      &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
    );

    // Normally this code path is defining a new dynamic property, but
    // unlike the non-magic case below, we may have already created it
    // under the recursion into invokeGet above, so we need to do a
    // tvSet here.
    tvSet(tvResult, *prop);
    return prop;
  }

  if (useGet && useSet) {
    if (invokeGet(&tvRef, key)) {
      // Matching zend again: incDecProp does an unbox before the
      // operation, but setop doesn't need to here.  (We'll unbox the
      // value that gets passed to the magic setter, though, since
      // __set functions can't take parameters by reference.)
      SETOP_BODY(&tvRef, op, val);
      TypedValue ignored;
      if (invokeSet(&ignored, key, &tvRef)) {
        tvRefcountedDecRef(&ignored);
      }
      return &tvRef;
    }
  }

  if (prop) raise_error("Cannot access protected property");

  // No visible/accessible property, and no applicable magic method:
  // create a new dynamic property.  (We know this is a new property,
  // or it would've hit the visible && accessible case above.)
  prop = reinterpret_cast<TypedValue*>(
    &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
  );
  assert(prop->m_type == KindOfNull); // cannot exist yet
  SETOP_BODY_CELL(prop, op, val);
  return prop;
}

template <bool setResult>
void ObjectData::incDecProp(
  Class* ctx,
  IncDecOp op,
  const StringData* key,
  TypedValue& dest
) {
  auto const lookup = getProp(ctx, key);
  auto prop = lookup.prop;

  auto tv = make_tv<KindOfUninit>();

  if (prop && lookup.accessible) {
    auto tvResult = make_tv<KindOfNull>();
    if (prop->m_type == KindOfUninit &&
        getAttribute(UseGet) &&
        invokeGet(&tvResult, key)) {
      IncDecBody<setResult>(op, &tvResult, &dest);
      TypedValue ignored;
      if (getAttribute(UseSet) && invokeSet(&ignored, key, &tvResult)) {
        tvRefcountedDecRef(&ignored);
        prop = &tvResult;
      } else {
        memcpy(prop, &tvResult, sizeof(TypedValue));
      }
      return;
    }

    if (prop->m_type == KindOfUninit) {
      tvWriteNull(prop);
    } else {
      prop = tvToCell(prop);
    }
    IncDecBody<setResult>(op, prop, &dest);
    return;
  }

  if (UNLIKELY(!*key->data())) throw_invalid_property_name(StrNR(key));

  // Native accessors.
  if (getAttribute(HasNativePropHandler)) {
    if (invokeNativeGetProp(&tv, key)) {
      tvUnboxIfNeeded(&tv);
      IncDecBody<setResult>(op, &tv, &dest);
      TypedValue ignored;
      if (invokeNativeSetProp(&ignored, key, &tv)) {
        tvRefcountedDecRef(&ignored);
        return;
      }
    }
  }

  auto const useSet = getAttribute(UseSet);
  auto const useGet = getAttribute(UseGet);

  if (useGet && !useSet) {
    auto tvResult = make_tv<KindOfNull>();
    // If invokeGet fails due to recursion, it leaves the KindOfNull
    // in tvResult.
    invokeGet(&tvResult, key);
    tvUnboxIfNeeded(&tvResult);
    IncDecBody<setResult>(op, &tvResult, &dest);
    if (prop) raise_error("Cannot access protected property");
    prop = reinterpret_cast<TypedValue*>(
      &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
    );

    // Normally this code path is defining a new dynamic property, but
    // unlike the non-magic case below, we may have already created it
    // under the recursion into invokeGet above, so we need to do a
    // tvSet here.
    tvSet(tvResult, *prop);
    return;
  }

  if (useGet && useSet) {
    if (invokeGet(&tv, key)) {
      tvUnboxIfNeeded(&tv);
      IncDecBody<setResult>(op, &tv, &dest);
      TypedValue ignored;
      if (invokeSet(&ignored, key, &tv)) {
        tvRefcountedDecRef(&ignored);
      }
      return;
    }
  }

  if (prop) raise_error("Cannot access protected property");

  // No visible/accessible property, and no applicable magic method:
  // create a new dynamic property.  (We know this is a new property,
  // or it would've hit the visible && accessible case above.)
  prop = reinterpret_cast<TypedValue*>(
    &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
  );
  assert(prop->m_type == KindOfNull); // cannot exist yet
  IncDecBody<setResult>(op, prop, &dest);
}

template void ObjectData::incDecProp<true>(Class*,
                                           IncDecOp,
                                           const StringData*,
                                           TypedValue&);
template void ObjectData::incDecProp<false>(Class*,
                                            IncDecOp,
                                            const StringData*,
                                            TypedValue&);

void ObjectData::unsetProp(Class* ctx, const StringData* key) {
  auto const lookup = getProp(ctx, key);
  auto const prop = lookup.prop;
  auto const propInd = declPropInd(prop);

  if (prop && lookup.accessible && prop->m_type != KindOfUninit) {
    if (propInd != kInvalidSlot) {
      // Declared property.
      tvSetIgnoreRef(*null_variant.asTypedValue(), *prop);
    } else {
      // Dynamic property.
      dynPropArray().remove(StrNR(key).asString(), true /* isString */);
    }
    return;
  }

  TypedValue ignored;

  // Native unset first.
  if (getAttribute(HasNativePropHandler) &&
      invokeNativeUnsetProp(&ignored, key)) {
    tvRefcountedDecRef(&ignored);
    return;
  }

  auto const tryUnset = getAttribute(UseUnset);

  if (propInd != kInvalidSlot && !lookup.accessible && !tryUnset) {
    // Defined property that is not accessible.
    raise_error("Cannot unset inaccessible property");
  }

  if (!tryUnset || !invokeUnset(&ignored, key)) {
    if (UNLIKELY(!*key->data())) {
      throw_invalid_property_name(StrNR(key));
    }

    return;
  }

  tvRefcountedDecRef(&ignored);
}

void ObjectData::raiseObjToIntNotice(const char* clsName) {
  raise_notice("Object of class %s could not be converted to int", clsName);
}

void ObjectData::raiseAbstractClassError(Class* cls) {
  Attr attrs = cls->attrs();
  raise_error("Cannot instantiate %s %s",
              (attrs & AttrInterface) ? "interface" :
              (attrs & AttrTrait)     ? "trait" :
              (attrs & AttrEnum)      ? "enum" : "abstract class",
              cls->preClass()->name()->data());
}

void ObjectData::raiseUndefProp(const StringData* key) {
  raise_notice("Undefined property: %s::$%s",
               m_cls->name()->data(), key->data());
}

void ObjectData::getProp(const Class* klass,
                         bool pubOnly,
                         const PreClass::Prop* prop,
                         Array& props,
                         std::vector<bool>& inserted) const {
  if (prop->attrs()
      & (AttrStatic | // statics aren't part of individual instances
         AttrBuiltin  // runtime-internal attrs, such as the <<Memoize>> cache
        )) {
    return;
  }

  Slot propInd = klass->lookupDeclProp(prop->name());
  assert(propInd != kInvalidSlot);
  const TypedValue* propVal = &propVec()[propInd];

  if ((!pubOnly || (prop->attrs() & AttrPublic)) &&
      propVal->m_type != KindOfUninit &&
      !inserted[propInd]) {
    inserted[propInd] = true;
    props.lvalAt(
      StrNR(klass->declProperties()[propInd].m_mangledName).asString())
      .setWithRef(tvAsCVarRef(propVal));
  }
}

void ObjectData::getProps(const Class* klass, bool pubOnly,
                          const PreClass* pc,
                          Array& props,
                          std::vector<bool>& inserted) const {
  PreClass::Prop const* propVec = pc->properties();
  size_t count = pc->numProperties();
  for (size_t i = 0; i < count; ++i) {
    getProp(klass, pubOnly, &propVec[i], props, inserted);
  }
}

void ObjectData::getTraitProps(const Class* klass, bool pubOnly,
                               const Class* trait, Array& props,
                               std::vector<bool>& inserted) const {
  assert(isNormalClass(klass));
  assert(isTrait(trait));

  getProps(klass, pubOnly, trait->preClass(), props, inserted);
  for (auto const& traitCls : trait->usedTraitClasses()) {
    getProps(klass, pubOnly, traitCls->preClass(), props, inserted);
    getTraitProps(klass, pubOnly, traitCls.get(), props, inserted);
  }
}

static Variant invokeSimple(ObjectData* obj, const StaticString& name) {
  auto const meth = obj->methodNamed(name.get());
  return meth ? g_context->invokeMethodV(obj, meth) : uninit_null();
}

Variant ObjectData::invokeSleep() {
  return invokeSimple(this, s___sleep);
}

Variant ObjectData::invokeToDebugDisplay() {
  return invokeSimple(this, s___toDebugDisplay);
}

Variant ObjectData::invokeWakeup() {
  return invokeSimple(this, s___wakeup);
}

String ObjectData::invokeToString() {
  const Func* method = m_cls->getToString();
  if (!method) {
    // If the object does not define a __toString() method, raise a
    // recoverable error
    raise_recoverable_error(
      "Object of class %s could not be converted to string",
      classname_cstr()
    );
    // If the user error handler decides to allow execution to continue,
    // we return the empty string.
    return empty_string();
  }
  auto const tv = g_context->invokeMethod(this, method);
  if (!IS_STRING_TYPE(tv.m_type)) {
    // Discard the value returned by the __toString() method and raise
    // a recoverable error
    tvRefcountedDecRef(tv);
    raise_recoverable_error(
      "Method %s::__toString() must return a string value",
      m_cls->preClass()->name()->data());
    // If the user error handler decides to allow execution to continue,
    // we return the empty string.
    return empty_string();
  }
  String ret = tv.m_data.pstr;
  decRefStr(tv.m_data.pstr);
  return ret;
}

bool ObjectData::hasToString() {
  return (m_cls->getToString() != nullptr);
}

void ObjectData::cloneSet(ObjectData* clone) {
  auto const nProps = m_cls->numDeclProperties();
  auto const clonePropVec = clone->propVec();
  for (auto i = Slot{0}; i < nProps; i++) {
    tvRefcountedDecRef(&clonePropVec[i]);
    tvDupFlattenVars(&propVec()[i], &clonePropVec[i]);
  }
  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    clone->setAttribute(HasDynPropArr);
    g_context->dynPropTable.emplace(clone, dynPropArray().get());
  }
}

ObjectData* ObjectData::cloneImpl() {
  ObjectData* obj = instanceof(Generator::getClass())
                    ? Generator::allocClone(this)
                    : ObjectData::newInstance(m_cls);
  Object o = Object::attach(obj);
  cloneSet(o.get());
  if (UNLIKELY(getAttribute(HasNativeData))) {
    Native::nativeDataInstanceCopy(o.get(), this);
  }

  auto const hasCloneBit = getAttribute(HasClone);

  if (!hasCloneBit) return o.detach();

  auto const method = o->m_cls->lookupMethod(s_clone.get());

  // PHP classes that inherit from cpp builtins that have special clone
  // functionality *may* also define a __clone method, but it's totally
  // fine if a __clone doesn't exist.
  if (!method && getAttribute(IsCppBuiltin)) return o.detach();
  assert(method);

  g_context->invokeMethodV(o.get(), method);

  return o.detach();
}

bool ObjectData::hasDynProps() const {
  return getAttribute(HasDynPropArr) && dynPropArray().size() != 0;
}

const char* ObjectData::classname_cstr() const {
  return getClassName().data();
}

void ObjectData::compileTimeAssertions() {
  static_assert(offsetof(ObjectData, m_hdr) == HeaderOffset, "");
}

} // HPHP
