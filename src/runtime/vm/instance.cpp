/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/base/base_includes.h"
#include "runtime/base/variable_serializer.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/hhbc.h"
#include "runtime/vm/class.h"
#include "runtime/vm/instance.h"
#include "runtime/vm/object_allocator_sizes.h"
#include "runtime/vm/translator/translator-inline.h"
#include "system/lib/systemlib.h"

namespace HPHP {
namespace VM {

static StaticString s___get(LITSTR_INIT("__get"));
static StaticString s___set(LITSTR_INIT("__set"));
static StaticString s___isset(LITSTR_INIT("__isset"));
static StaticString s___unset(LITSTR_INIT("__unset"));
static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

TRACE_SET_MOD(runtime);

int HPHP::VM::Instance::ObjAllocatorSizeClassCount =
  HPHP::VM::InitializeAllocators();

TypedValue* Instance::propVec() {
  uintptr_t ret = (uintptr_t)this + sizeof(ObjectData) + builtinPropSize();
  // TODO(#1432007): some builtins still do not have TypedValue-aligned sizes.
  ASSERT(ret % sizeof(TypedValue) == builtinPropSize() % sizeof(TypedValue));
  return (TypedValue*) ret;
}

const TypedValue* Instance::propVec() const {
  return const_cast<Instance*>(this)->propVec();
}

Instance* Instance::callCustomInstanceInit() {
  static StringData* sd_init = StringData::GetStaticString("__init__");
  const Func* init = m_cls->lookupMethod(sd_init);
  if (init != NULL) {
    TypedValue tv;
    // We need to incRef/decRef here because we're still a new (_count
    // == 0) object and invokeFunc is going to expect us to have a
    // reasonable refcount.
    incRefCount();
    g_vmContext->invokeFunc(&tv, init, Array::Create(), this);
    decRefCount();
    ASSERT(!IS_REFCOUNTED_TYPE(tv.m_type));
  }
  return this;
}

HOT_FUNC_VM
Instance* Instance::newInstanceRaw(Class* cls, int idx) {
  Instance* obj = (Instance*)ALLOCOBJIDX(idx);
  new (obj) Instance(cls, noinit);
  return obj;
}

void Instance::destructHard(const Func* meth) {
  static ArrayData* args =
    ArrayData::GetScalarArray(HphpArray::GetStaticEmptyArray());
  TypedValue retval;
  tvWriteNull(&retval);
  try {
    // Call the destructor method
    g_vmContext->invokeFunc(&retval, meth, CArrRef(args), this);
  } catch (...) {
    // Swallow any exceptions that escape the __destruct method
    handle_destructor_exception();
  }
  tvRefcountedDecRef(&retval);
}

void Instance::forgetSweepable() {
  ASSERT(RuntimeOption::EnableObjDestructCall);
  g_vmContext->m_liveBCObjs.erase(this);
}

void Instance::invokeUserMethod(TypedValue* retval, const Func* method,
                                CArrRef params) {
  g_vmContext->invokeFunc(retval, method, params, this);
}

Object Instance::FromArray(ArrayData *properties) {
  ASSERT(hhvm);
  Instance* retval = Instance::newInstance(SystemLib::s_stdclassClass);
  retval->initDynProps();
  HphpArray* props = static_cast<HphpArray*>(retval->o_properties.get());
  for (ssize_t pos = properties->iter_begin(); pos != ArrayData::invalid_index;
       pos = properties->iter_advance(pos)) {
    TypedValue* value = properties->nvGetValueRef(pos);
    TypedValue key;
    properties->nvGetKey(&key, pos);
    if (key.m_type == KindOfInt64) {
      props->nvSet(key.m_data.num, value, false);
    } else {
      ASSERT(IS_STRING_TYPE(key.m_type));
      StringData* strKey = key.m_data.pstr;
      props->nvSet(strKey, value, false);
      decRefStr(strKey);
    }
  }
  return retval;
}

void Instance::initDynProps(int numDynamic /* = 0 */) {
  // Create o_properties with room for numDynamic
  o_properties.asArray() = NEW(HphpArray)(numDynamic);
}

Slot Instance::declPropInd(TypedValue* prop) const {
  // Do an address range check to determine whether prop physically resides
  // in propVec.
  const TypedValue* pv = propVec();
  if (prop >= pv && prop < &pv[m_cls->numDeclProperties()]) {
    return prop - pv;
  } else {
    return kInvalidSlot;
  }
}

template <bool declOnly>
TypedValue* Instance::getPropImpl(Class* ctx, const StringData* key,
                                  bool& visible, bool& accessible,
                                  bool& unset) {
  TypedValue* prop = NULL;
  unset = false;
  Slot propInd = m_cls->getDeclPropIndex(ctx, key, accessible);
  visible = (propInd != kInvalidSlot);
  if (propInd != kInvalidSlot) {
    // We found a visible property, but it might not be accessible.
    // No need to check if there is a dynamic property with this name.
    prop = &propVec()[propInd];
    if (prop->m_type == KindOfUninit) {
      unset = true;
    }
  } else {
    ASSERT(!visible && !accessible);
    // We could not find a visible property. We need to check for a
    // dynamic property with this name if declOnly = false.
    if (!declOnly && o_properties.get()) {
      prop = static_cast<HphpArray*>(o_properties.get())->nvGet(key);
      if (prop) {
        // o_properties.get()->nvGet() returned a non-declared property,
        // we know that it is visible and accessible (since all
        // dynamic properties are), and we know it is not unset
        // (since unset dynamic properties don't appear in o_properties.get()).
        visible = true;
        accessible = true;
      }
    }
  }
  return prop;
}

TypedValue* Instance::getProp(Class* ctx, const StringData* key,
                              bool& visible, bool& accessible, bool& unset) {
  return getPropImpl<false>(ctx, key, visible, accessible, unset);
}

TypedValue* Instance::getDeclProp(Class* ctx, const StringData* key,
                                  bool& visible, bool& accessible,
                                  bool& unset) {
  return getPropImpl<true>(ctx, key, visible, accessible, unset);
}

void Instance::invokeSet(TypedValue* retval, const StringData* key,
                         TypedValue* val) {
  AttributeClearer a(UseSet, this);
  const Func* meth = m_cls->lookupMethod(s___set.get());
  ASSERT(meth);
  invokeUserMethod(retval, meth,
                   CREATE_VECTOR2(CStrRef(key), tvAsVariant(val)));
}

#define MAGIC_PROP_BODY(name, attr) \
  AttributeClearer a((attr), this); \
  const Func* meth = m_cls->lookupMethod(name); \
  ASSERT(meth); \
  invokeUserMethod(retval, meth, CREATE_VECTOR1(CStrRef(key))); \

void Instance::invokeGet(TypedValue* retval, const StringData* key) {
  MAGIC_PROP_BODY(s___get.get(), UseGet);
}

void Instance::invokeIsset(TypedValue* retval, const StringData* key) {
  MAGIC_PROP_BODY(s___isset.get(), UseIsset);
}

void Instance::invokeUnset(TypedValue* retval, const StringData* key) {
  MAGIC_PROP_BODY(s___unset.get(), UseUnset);
}

void Instance::invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                             const StringData* key) {
  invokeGet(&tvRef, key);
  retval = &tvRef;
}

template <bool warn, bool define>
void Instance::propImpl(TypedValue*& retval, TypedValue& tvRef,
                        Class* ctx,
                        const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);

  if (visible) {
    if (accessible) {
      if (unset) {
        if (getAttribute(UseGet)) {
          invokeGetProp(retval, tvRef, key);
        } else {
          if (warn) {
            raiseUndefProp(key);
          }
          if (define) {
            retval = propVal;
          } else {
            retval = (TypedValue*)&init_null_variant;
          }
        }
      } else {
        retval = propVal;
      }
    } else {
      if (getAttribute(UseGet)) {
        invokeGetProp(retval, tvRef, key);
      } else {
        raise_error("Inaccessible property: %s::$%s",
                    m_cls->m_preClass->name()->data(), key->data());
      }
    }
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else {
    if (getAttribute(UseGet)) {
      invokeGetProp(retval, tvRef, key);
    } else {
      if (warn) {
        raiseUndefProp(key);
      }
      if (define) {
        if (o_properties.get() == NULL) {
          initDynProps();
        }
        o_properties.get()->lvalPtr(*(const String*)&key,
                           *(Variant**)(&retval), false, true);
      } else {
        retval = (TypedValue*)&init_null_variant;
      }
    }
  }
}

void Instance::prop(TypedValue*& retval, TypedValue& tvRef,
                    Class* ctx, const StringData* key) {
  propImpl<false, false>(retval, tvRef, ctx, key);
}

void Instance::propD(TypedValue*& retval, TypedValue& tvRef,
                     Class* ctx, const StringData* key) {
  propImpl<false, true>(retval, tvRef, ctx, key);
}

void Instance::propW(TypedValue*& retval, TypedValue& tvRef,
                     Class* ctx, const StringData* key) {
  propImpl<true, false>(retval, tvRef, ctx, key);
}

void Instance::propWD(TypedValue*& retval, TypedValue& tvRef,
                      Class* ctx, const StringData* key) {
  propImpl<true, true>(retval, tvRef, ctx, key);
}

bool Instance::propIsset(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible && !unset) {
    return isset(tvAsCVarRef(propVal));
  }
  if (!getAttribute(UseIsset)) {
    return false;
  }
  TypedValue tv;
  tvWriteUninit(&tv);
  invokeIsset(&tv, key);
  tvCastToBooleanInPlace(&tv);
  return tv.m_data.num;
}

bool Instance::propEmpty(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible && !unset) {
    return empty(tvAsCVarRef(propVal));
  }
  if (!getAttribute(UseIsset)) {
    return true;
  }
  TypedValue tv;
  tvWriteUninit(&tv);
  invokeIsset(&tv, key);
  tvCastToBooleanInPlace(&tv);
  if (!tv.m_data.num) {
    return true;
  }
  if (getAttribute(UseGet)) {
    invokeGet(&tv, key);
    bool emptyResult = empty(tvAsCVarRef(&tv));
    if (IS_REFCOUNTED_TYPE(tv.m_type)) {
      tvDecRef(&tv);
    }
    return emptyResult;
  }
  return false;
}

TypedValue* Instance::setProp(Class* ctx, const StringData* key,
                              TypedValue* val,
                              bool bindingAssignment /* = false */) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    ASSERT(propVal);
    if (unset && getAttribute(UseSet)) {
      TypedValue ignored;
      invokeSet(&ignored, key, val);
      tvRefcountedDecRef(&ignored);
    } else {
      if (UNLIKELY(bindingAssignment)) {
        tvBind(val, propVal);
      } else {
        tvSet(val, propVal);
      }
    }
    // Return a pointer to the property if it's a declared property
    return declPropInd(propVal) != kInvalidSlot ? propVal : NULL;
  }
  ASSERT(!accessible);
  if (visible) {
    ASSERT(propVal);
    if (!getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else if (!getAttribute(UseSet)) {
    if (o_properties.get() == NULL) {
      initDynProps();
    }
    // when seting a dynamic property, do not write
    // directly to the TypedValue in the HphpArray, since
    // its _count field is used to store the string hash of
    // the property name. Instead, call the appropriate
    // setters (set() or setRef()).
    if (UNLIKELY(bindingAssignment)) {
      o_properties.get()->setRef(const_cast<StringData*>(key),
                                 tvAsCVarRef(val), false);
    } else {
      o_properties.get()->set(const_cast<StringData*>(key),
                              tvAsCVarRef(val), false);
    }
    return NULL;
  }
  ASSERT(!accessible);
  ASSERT(getAttribute(UseSet));
  TypedValue ignored;
  invokeSet(&ignored, key, val);
  tvRefcountedDecRef(&ignored);
  return NULL;
}

TypedValue* Instance::setOpProp(TypedValue& tvRef, Class* ctx,
                                unsigned char op, const StringData* key,
                                Cell* val) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    ASSERT(propVal);
    if (unset && getAttribute(UseGet)) {
      TypedValue tvResult;
      tvWriteUninit(&tvResult);
      invokeGet(&tvResult, key);
      SETOP_BODY(&tvResult, op, val);
      if (getAttribute(UseSet)) {
        TypedValue ignored;
        invokeSet(&ignored, key, &tvResult);
        tvRefcountedDecRef(&ignored);
        propVal = &tvResult;
      } else {
        memcpy((void *)propVal, (void *)&tvResult, sizeof(TypedValue));
      }
    } else {
      SETOP_BODY(propVal, op, val);
    }
    return propVal;
  }
  ASSERT(!accessible);
  if (visible) {
    ASSERT(propVal);
    if (!getAttribute(UseGet) || !getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else if (!getAttribute(UseGet)) {
    if (o_properties.get() == NULL) {
      initDynProps();
    }
    o_properties.get()->lvalPtr(*(const String*)&key,
                        *(Variant**)(&propVal), false, true);
    // don't write propVal->_count because it holds data
    // owned by the HphpArray
    propVal->m_type = KindOfNull;
    SETOP_BODY(propVal, op, val);
    return propVal;
  } else if (!getAttribute(UseSet)) {
    TypedValue tvResult;
    tvWriteUninit(&tvResult);
    invokeGet(&tvResult, key);
    SETOP_BODY(&tvResult, op, val);
    if (o_properties.get() == NULL) {
      initDynProps();
    }
    o_properties.get()->lvalPtr(*(const String*)&key, *(Variant**)(&propVal),
                       false, true);
    // don't write propVal->_count because it holds data
    // owned by the HphpArray
    propVal->m_data.num = tvResult.m_data.num;
    propVal->m_type = tvResult.m_type;
    return propVal;
  }
  ASSERT(!accessible);
  ASSERT(getAttribute(UseGet) && getAttribute(UseSet));
  invokeGet(&tvRef, key);
  SETOP_BODY(&tvRef, op, val);
  TypedValue ignored;
  invokeSet(&ignored, key, &tvRef);
  tvRefcountedDecRef(&ignored);
  propVal = &tvRef;
  return propVal;
}

template <bool setResult>
void Instance::incDecPropImpl(TypedValue& tvRef, Class* ctx,
                              unsigned char op, const StringData* key,
                              TypedValue& dest) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    ASSERT(propVal);
    if (unset && getAttribute(UseGet)) {
      TypedValue tvResult;
      tvWriteUninit(&tvResult);
      invokeGet(&tvResult, key);
      IncDecBody<setResult>(op, &tvResult, &dest);
      if (getAttribute(UseSet)) {
        TypedValue ignored;
        invokeSet(&ignored, key, &tvResult);
        tvRefcountedDecRef(&ignored);
        propVal = &tvResult;
      } else {
        memcpy((void *)propVal, (void *)&tvResult, sizeof(TypedValue));
      }
    } else {
      IncDecBody<setResult>(op, propVal, &dest);
    }
    return;
  }
  ASSERT(!accessible);
  if (visible) {
    ASSERT(propVal);
    if (!getAttribute(UseGet) || !getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else if (!getAttribute(UseGet)) {
    if (o_properties.get() == NULL) {
      initDynProps();
    }
    o_properties.get()->lvalPtr(*(const String*)&key,
                       *(Variant**)(&propVal), false, true);
    // don't write propVal->_count because it holds data
    // owned by the HphpArray
    propVal->m_type = KindOfNull;
    IncDecBody<setResult>(op, propVal, &dest);
    return;
  } else if (!getAttribute(UseSet)) {
    TypedValue tvResult;
    tvWriteUninit(&tvResult);
    invokeGet(&tvResult, key);
    IncDecBody<setResult>(op, &tvResult, &dest);
    if (o_properties.get() == NULL) {
      initDynProps();
    }
    o_properties.get()->lvalPtr(*(const String*)&key, *(Variant**)(&propVal),
                       false, true);
    // don't write propVal->_count because it holds data
    // owned by the HphpArray
    propVal->m_data.num = tvResult.m_data.num;
    propVal->m_type = tvResult.m_type;
    return;
  }
  ASSERT(!accessible);
  ASSERT(getAttribute(UseGet) && getAttribute(UseSet));
  invokeGet(&tvRef, key);
  IncDecBody<setResult>(op, &tvRef, &dest);
  TypedValue ignored;
  invokeSet(&ignored, key, &tvRef);
  tvRefcountedDecRef(&ignored);
  propVal = &tvRef;
}

// Actualize template method so that the method can be defined in instance.cpp
// (rather than instance.h), but still be invoked elsewhere.
template <>
void Instance::incDecProp<false>(TypedValue& tvRef, Class* ctx,
                                 unsigned char op, const StringData* key,
                                 TypedValue& dest) {
  incDecPropImpl<false>(tvRef, ctx, op, key, dest);
}

template <>
void Instance::incDecProp<true>(TypedValue& tvRef, Class* ctx,
                                unsigned char op, const StringData* key,
                                TypedValue& dest) {
  incDecPropImpl<true>(tvRef, ctx, op, key, dest);
}

void Instance::unsetProp(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    Slot propInd = declPropInd(propVal);
    if (propInd != kInvalidSlot) {
      // Declared property.
      tvSetIgnoreRef((TypedValue*)&null_variant, propVal);
    } else {
      // Dynamic property.
      ASSERT(o_properties.get() != NULL);
      o_properties.get()->remove(CStrRef(key), false);
    }
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else {
    ASSERT(!accessible);
    if (getAttribute(UseUnset)) {
      TypedValue ignored;
      invokeUnset(&ignored, key);
      tvRefcountedDecRef(&ignored);
    } else if (visible) {
      raise_error("Cannot unset inaccessible property");
    }
  }
}

void Instance::raiseUndefProp(const StringData* key) {
  raise_notice("Undefined property: %s::$%s",
               m_cls->name()->data(), key->data());
}

Array Instance::o_toIterArray(CStrRef context, bool getRef /* = false */) {
  int size = m_cls->m_declPropNumAccessible +
               (o_properties.get() != NULL ? o_properties.get()->size() : 0);
  HphpArray* retval = NEW(HphpArray)(size);
  Class* ctx = NULL;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  // Get all declared properties first, bottom-to-top in the inheritance
  // hierarchy, in declaration order.
  const Class* klass = m_cls;
  while (klass != NULL) {
    const PreClass::Prop* props = klass->m_preClass->properties();
    const size_t numProps = klass->m_preClass->numProperties();

    for (size_t i = 0; i < numProps; ++i) {
      StringData* key = const_cast<StringData*>(props[i].name());
      bool visible, accessible, unset;
      TypedValue* val = getProp(ctx, key, visible, accessible, unset);
      if (accessible && val->m_type != KindOfUninit && !unset) {
        if (getRef) {
          if (val->m_type != KindOfRef) {
            tvBox(val);
          }
          retval->nvBind(key, val);
        } else {
          retval->nvSet(key, val, false);
        }
      }
    }
    klass = klass->m_parent.get();
  }

  // Now get dynamic properties.
  if (o_properties.get() != NULL) {
    ssize_t iter = o_properties.get()->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      TypedValue key;
      static_cast<HphpArray*>(o_properties.get())->nvGetKey(&key, iter);
      iter = o_properties.get()->iter_advance(iter);

      // You can get this if you cast an array to object. These properties must
      // be dynamic because you can't declare a property with a non-string name.
      if (UNLIKELY(!IS_STRING_TYPE(key.m_type))) {
        ASSERT(key.m_type == KindOfInt64);
        TypedValue* val =
          static_cast<HphpArray*>(o_properties.get())->nvGet(key.m_data.num);
        if (getRef) {
          if (val->m_type != KindOfRef) {
            tvBox(val);
          }
          retval->nvBind(key.m_data.num, val);
        } else {
          retval->nvSet(key.m_data.num, val, false);
        }
        continue;
      }

      StringData* strKey = key.m_data.pstr;
      TypedValue* val =
        static_cast<HphpArray*>(o_properties.get())->nvGet(strKey);
      if (getRef) {
        if (val->m_type != KindOfRef) {
          tvBox(val);
        }
        retval->nvBind(strKey, val);
      } else {
        retval->nvSet(strKey, val, false);
      }
      decRefStr(strKey);
    }
  }

  return Array(retval);
}

void Instance::o_setArray(CArrRef properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String k = iter.first().toString();
    Class* ctx = NULL;

    // If the key begins with a NUL, it's a private or protected property. Read
    // the class name from between the two NUL bytes.
    if (!k.empty() && k.charAt(0) == '\0') {
      int subLen = k.find('\0', 1) + 1;
      String cls = k.substr(1, subLen - 2);
      if (cls == "*") {
        // Protected.
        ctx = m_cls;
      } else {
        // Private.
        ctx = Unit::lookupClass(cls.get());
        if (!ctx) continue;
      }
      k = k.substr(subLen);
    }

    CVarRef secondRef = iter.secondRef();
    setProp(ctx, k.get(), (TypedValue*)(&secondRef),
            secondRef.isReferenced());
  }
  // set public properties
  ObjectData::o_setArray(properties);
}

void Instance::getProp(const Class* klass, bool pubOnly,
                       const PreClass::Prop* prop,
                       Array& props,
                       std::vector<bool>& inserted) const {
  if (prop->attrs() & AttrStatic) {
    return;
  }

  Slot propInd = klass->lookupDeclProp(prop->name());
  ASSERT(propInd != kInvalidSlot);
  const TypedValue* propVal = &propVec()[propInd];

  if ((!pubOnly || (prop->attrs() & AttrPublic)) &&
      propVal->m_type != KindOfUninit &&
      !inserted[propInd]) {
    inserted[propInd] = true;
    props.lvalAt(CStrRef(klass->declProperties()[propInd].m_mangledName))
      .setWithRef(tvAsCVarRef(propVal));
  }
}

void Instance::getProps(const Class* klass, bool pubOnly,
                        const PreClass* pc,
                        Array& props,
                        std::vector<bool>& inserted) const {
  PreClass::Prop const* propVec = pc->properties();
  size_t count = pc->numProperties();
  for (size_t i = 0; i < count; ++i) {
    getProp(klass, pubOnly, &propVec[i], props, inserted);
  }
}

void Instance::o_getArray(Array& props, bool pubOnly /* = false */) const {
  // The declared properties in the resultant array should be a permutation of
  // propVec. They appear in the following order: go most-to-least-derived in
  // the inheritance hierarchy, inserting properties in declaration order (with
  // the wrinkle that overridden properties should appear only once, with the
  // access level given to it in its most-derived declaration).

  // This is needed to keep track of which elements have been inserted. This is
  // the smoothest way to get overridden properties right.
  std::vector<bool> inserted(m_cls->numDeclProperties(), false);

  // Iterate over declared properties and insert {mangled name --> prop} pairs.
  const Class* klass = m_cls;
  while (klass != NULL) {
    getProps(klass, pubOnly, klass->m_preClass.get(), props, inserted);

    const std::vector<ClassPtr> &usedTraits = klass->m_usedTraits;
    for (unsigned t = 0; t < usedTraits.size(); t++) {
      const ClassPtr& trait = usedTraits[t];
      getProps(klass, pubOnly, trait->m_preClass.get(), props, inserted);
    }

    klass = klass->m_parent.get();
  }

  // Iterate over dynamic properties and insert {name --> prop} pairs.
  if (o_properties.get() != NULL && !o_properties.get()->empty()) {
    for (ArrayIter it(o_properties.get()); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      props.addLval(key, true).setWithRef(value);
    }
  }
}

bool Instance::o_get_call_info_hook(const char *clsname,
                                    MethodCallPackage &mcp,
                                    strhash_t hash /* = -1 */) {
  return false;
}

Variant Instance::t___destruct() {
  static StringData* sd__destruct = StringData::GetStaticString("__destruct");
  const Func* method = m_cls->lookupMethod(sd__destruct);
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method, Array::Create(), this);
    return v;
  } else {
    return null;
  }
}

Variant Instance::t___call(Variant v_name, Variant v_arguments) {
  static StringData* sd__call = StringData::GetStaticString("__call");
  const Func* method = m_cls->lookupMethod(sd__call);
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR2(v_name, v_arguments), this);
    return v;
  } else {
    return null;
  }
}

Variant Instance::t___set(Variant v_name, Variant v_value) {
  const Func* method = m_cls->lookupMethod(s___set.get());
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method,
      Array(ArrayInit(2).set(v_name).set(withRefBind(v_value)).create()),
      this);
    return v;
  } else {
    return null;
  }
}

Variant Instance::t___get(Variant v_name) {
  const Func* method = m_cls->lookupMethod(s___get.get());
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR1(v_name), this);
    return v;
  } else {
    return null;
  }
}

bool Instance::t___isset(Variant v_name) {
  const Func* method = m_cls->lookupMethod(s___isset.get());
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR1(v_name), this);
    return v;
  } else {
    return null;
  }
}

Variant Instance::t___unset(Variant v_name) {
  const Func* method = m_cls->lookupMethod(s___unset.get());
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR1(v_name), this);
    return v;
  } else {
    return null;
  }
}

DECLARE_THREAD_LOCAL(Variant, __lvalProxy);
DECLARE_THREAD_LOCAL(Variant, nullProxy);

Variant& Instance::___offsetget_lval(Variant key) {
  if (isCollection()) {
    return collectionOffsetGet(this, key);
  } else {
    if (!o_instanceof("ArrayAccess")) {
      throw InvalidOperandException("not ArrayAccess objects");
    }
    static StringData* sd__offsetGet = StringData::GetStaticString("offsetGet");
    const Func* method = m_cls->lookupMethod(sd__offsetGet);
    ASSERT(method);
    if (method) {
      Variant *v = __lvalProxy.get();
      g_vmContext->invokeFunc((TypedValue*)v, method,
                              CREATE_VECTOR1(key), this);
      return *v;
    } else {
      return *nullProxy.get();
    }
  }
}

Variant Instance::t___sleep() {
  static StringData* sd__sleep = StringData::GetStaticString("__sleep");
  const Func *method = m_cls->lookupMethod(sd__sleep);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFunc(&tv, method, Array::Create(), this);
    return tvAsVariant(&tv);
  } else {
    clearAttribute(HasSleep);
    return null;
  }
}

Variant Instance::t___wakeup() {
  static StringData* sd__wakeup = StringData::GetStaticString("__wakeup");
  const Func *method = m_cls->lookupMethod(sd__wakeup);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFunc(&tv, method, Array::Create(), this);
    return tvAsVariant(&tv);
  } else {
    return null;
  }
}

Variant Instance::t___set_state(Variant v_properties) {
  static StringData* sd__set_state = StringData::GetStaticString("__set_state");
  const Func* method = m_cls->lookupMethod(sd__set_state);
  if (method) {
    Variant v;
    g_vmContext->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR1(v_properties), this);
    return v;
  } else {
    return false;
  }
}

String Instance::t___tostring() {
  const Func *method = m_cls->getToString();
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFunc(&tv, method, Array::Create(), this);
    if (!IS_STRING_TYPE(tv.m_type)) {
      void (*notify_user)(const char *, ...) = &raise_error;
      if (hphpiCompat) {
        tvCastToStringInPlace(&tv);
        notify_user = &raise_warning;
      }
      notify_user("Method %s::__toString() must return a string value",
                  m_cls->m_preClass->name()->data());
    }
    return tv.m_data.pstr;
  } else {
    std::string msg = m_cls->m_preClass->name()->data();
    msg += "::__toString() was not defined";
    throw BadTypeConversionException(msg.c_str());
  }
}

Variant Instance::t___clone() {
  static StringData* sd__clone = StringData::GetStaticString("__clone");
  const Func *method = m_cls->lookupMethod(sd__clone);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFunc(&tv, method, Array::Create(), this);
    return false;
  } else {
    return false;
  }
}

void Instance::cloneSet(ObjectData* clone) {
  Instance* iclone = static_cast<Instance*>(clone);
  Slot nProps = m_cls->numDeclProperties();
  TypedValue* iclonePropVec = (TypedValue *)((uintptr_t)iclone +
                               sizeof(ObjectData) + builtinPropSize());
  for (Slot i = 0; i < nProps; i++) {
    tvRefcountedDecRef(&iclonePropVec[i]);
    tvDupFlattenVars(&propVec()[i], &iclonePropVec[i], NULL);
  }
  iclone->initDynProps();
  if (o_properties.get() != NULL) {
    ssize_t iter = o_properties.get()->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      auto props = static_cast<HphpArray*>(o_properties.get());
      TypedValue key;
      props->nvGetKey(&key, iter);
      ASSERT(tvIsString(&key));
      StringData* strKey = key.m_data.pstr;
      TypedValue *val = props->nvGet(strKey);
      TypedValue *retval;
      auto cloneProps = iclone->o_properties.get();
      cloneProps->lvalPtr(strKey, *(Variant**)&retval, false, true);
      tvDupFlattenVars(val, retval, cloneProps);
      iter = o_properties.get()->iter_advance(iter);
      decRefStr(strKey);
    }
  }
}

ObjectData* Instance::cloneImpl() {
  Instance* obj = Instance::newInstance(m_cls);
  cloneSet(obj);
  obj->incRefCount();
  obj->t___clone();
  return obj;
}

} } // HPHP::VM
