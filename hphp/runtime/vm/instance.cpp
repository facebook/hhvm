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

#include "hphp/runtime/vm/instance.h"
#include "hphp/runtime/base/base_includes.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/vm/core_types.h"
#include "hphp/runtime/vm/member_operations.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/object_allocator_sizes.h"
#include "hphp/runtime/vm/translator/translator-inline.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/system/lib/systemlib.h"

namespace HPHP {

static StaticString s___get(LITSTR_INIT("__get"));
static StaticString s___set(LITSTR_INIT("__set"));
static StaticString s___isset(LITSTR_INIT("__isset"));
static StaticString s___unset(LITSTR_INIT("__unset"));
static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

TRACE_SET_MOD(runtime);

int HPHP::Instance::ObjAllocatorSizeClassCount =
  HPHP::InitializeAllocators();

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps) {
  auto* dst = propVec;
  auto* src = propData;
  for (; src != propData + nProps; ++src, ++dst) {
    *dst = *src;
    // m_aux.u_deepInit is true for properties that need "deep" initialization
    if (src->deepInit()) {
      tvIncRef(dst);
      collectionDeepCopyTV(dst);
    }
  }
}

TypedValue* Instance::propVec() {
  uintptr_t ret = (uintptr_t)this + sizeof(ObjectData) + builtinPropSize();
  // TODO(#1432007): some builtins still do not have TypedValue-aligned sizes.
  assert(ret % sizeof(TypedValue) == builtinPropSize() % sizeof(TypedValue));
  return (TypedValue*) ret;
}

const TypedValue* Instance::propVec() const {
  return const_cast<Instance*>(this)->propVec();
}

Instance* Instance::callCustomInstanceInit() {
  static StringData* sd_init = StringData::GetStaticString("__init__");
  const Func* init = m_cls->lookupMethod(sd_init);
  if (init != nullptr) {
    TypedValue tv;
    // We need to incRef/decRef here because we're still a new (_count
    // == 0) object and invokeFunc is going to expect us to have a
    // reasonable refcount.
    try {
      incRefCount();
      g_vmContext->invokeFuncFew(&tv, init, this);
      decRefCount();
      assert(!IS_REFCOUNTED_TYPE(tv.m_type));
    } catch (...) {
      this->setNoDestruct();
      decRefObj(this);
      throw;
    }
  }
  return this;
}

HOT_FUNC_VM
Instance* Instance::newInstanceRaw(Class* cls, int idx) {
  Instance* obj = (Instance*)ALLOCOBJIDX(idx);
  new (obj) Instance(cls, noinit);
  return obj;
}

void Instance::invokeUserMethod(TypedValue* retval, const Func* method,
                                CArrRef params) {
  g_vmContext->invokeFunc(retval, method, params, this);
}

Object Instance::FromArray(ArrayData *properties) {
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
      assert(IS_STRING_TYPE(key.m_type));
      StringData* strKey = key.m_data.pstr;
      props->nvSet(strKey, value, false);
      decRefStr(strKey);
    }
  }
  return retval;
}

void Instance::initDynProps(int numDynamic /* = 0 */) {
  // Create o_properties with room for numDynamic
  o_properties.asArray() = ArrayData::Make(numDynamic);
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
  TypedValue* prop = nullptr;
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
    assert(!visible && !accessible);
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
  assert(meth);
  invokeUserMethod(retval, meth,
                   CREATE_VECTOR2(CStrRef(key), tvAsVariant(val)));
}

#define MAGIC_PROP_BODY(name, attr) \
  AttributeClearer a((attr), this); \
  const Func* meth = m_cls->lookupMethod(name); \
  assert(meth); \
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
        // No need to check hasProp since visible is true
        // Visibility is either protected or private since accessible is false
        Slot propInd = m_cls->lookupDeclProp(key);
        bool priv = m_cls->declProperties()[propInd].m_attrs & AttrPrivate;

        raise_error("Cannot access %s property %s::$%s",
                    priv ? "private" : "protected",
                    m_cls->m_preClass->name()->data(),
                    key->data());
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
        if (o_properties.get() == nullptr) {
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
    tvRefcountedDecRef(&tv);
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
    assert(propVal);
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
    return declPropInd(propVal) != kInvalidSlot ? propVal : nullptr;
  }
  assert(!accessible);
  if (visible) {
    assert(propVal);
    if (!getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else if (!getAttribute(UseSet)) {
    if (o_properties.get() == nullptr) {
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
    return nullptr;
  }
  assert(!accessible);
  assert(getAttribute(UseSet));
  TypedValue ignored;
  invokeSet(&ignored, key, val);
  tvRefcountedDecRef(&ignored);
  return nullptr;
}

TypedValue* Instance::setOpProp(TypedValue& tvRef, Class* ctx,
                                unsigned char op, const StringData* key,
                                Cell* val) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    assert(propVal);
    if (unset && getAttribute(UseGet)) {
      TypedValue tvResult;
      tvWriteUninit(&tvResult);
      invokeGet(&tvResult, key);
      SETOP_BODY(&tvResult, op, val);
      if (getAttribute(UseSet)) {
        assert(tvRef.m_type == KindOfUninit);
        memcpy(&tvRef, &tvResult, sizeof(TypedValue));
        TypedValue ignored;
        invokeSet(&ignored, key, &tvRef);
        tvRefcountedDecRef(&ignored);
        propVal = &tvRef;
      } else {
        memcpy(propVal, &tvResult, sizeof(TypedValue));
      }
    } else {
      SETOP_BODY(propVal, op, val);
    }
    return propVal;
  }
  assert(!accessible);
  if (visible) {
    assert(propVal);
    if (!getAttribute(UseGet) || !getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else if (!getAttribute(UseGet)) {
    if (o_properties.get() == nullptr) {
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
    if (o_properties.get() == nullptr) {
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
  assert(!accessible);
  assert(getAttribute(UseGet) && getAttribute(UseSet));
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
    assert(propVal);
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
  assert(!accessible);
  if (visible) {
    assert(propVal);
    if (!getAttribute(UseGet) || !getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else if (!getAttribute(UseGet)) {
    if (o_properties.get() == nullptr) {
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
    if (o_properties.get() == nullptr) {
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
  assert(!accessible);
  assert(getAttribute(UseGet) && getAttribute(UseSet));
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
      assert(o_properties.get() != nullptr);
      o_properties.get()->remove(CStrRef(key), false);
    }
  } else if (UNLIKELY(!*key->data())) {
    throw_invalid_property_name(StrNR(key));
  } else {
    assert(!accessible);
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

void Instance::getProp(const Class* klass, bool pubOnly,
                       const PreClass::Prop* prop,
                       Array& props,
                       std::vector<bool>& inserted) const {
  if (prop->attrs() & AttrStatic) {
    return;
  }

  Slot propInd = klass->lookupDeclProp(prop->name());
  assert(propInd != kInvalidSlot);
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

Variant Instance::t___destruct() {
  static StringData* sd__destruct = StringData::GetStaticString("__destruct");
  const Func* method = m_cls->lookupMethod(sd__destruct);
  if (method) {
    Variant v;
    g_vmContext->invokeFuncFew((TypedValue*)&v, method, this);
    return v;
  } else {
    return uninit_null();
  }
}

Variant Instance::t___call(Variant v_name, Variant v_arguments) {
  static StringData* sd__call = StringData::GetStaticString("__call");
  const Func* method = m_cls->lookupMethod(sd__call);
  if (method) {
    Variant v;
    TypedValue args[2];
    tvDup(v_name.asTypedValue(), args + 0);
    tvDup(v_arguments.asTypedValue(), args + 1);
    g_vmContext->invokeFuncFew((TypedValue*)&v, method, this, nullptr, 2, args);
    return v;
  } else {
    return uninit_null();
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
    return uninit_null();
  }
}

Variant Instance::t___get(Variant v_name) {
  const Func* method = m_cls->lookupMethod(s___get.get());
  if (method) {
    Variant v;
    TypedValue args[1];
    tvDup(v_name.asTypedValue(), args + 0);
    g_vmContext->invokeFuncFew((TypedValue*)&v, method, this, nullptr, 1, args);
    return v;
  } else {
    return uninit_null();
  }
}

bool Instance::t___isset(Variant v_name) {
  const Func* method = m_cls->lookupMethod(s___isset.get());
  if (method) {
    Variant v;
    TypedValue args[1];
    tvDup(v_name.asTypedValue(), args + 0);
    g_vmContext->invokeFuncFew((TypedValue*)&v, method, this, nullptr, 1, args);
    return v;
  } else {
    return uninit_null();
  }
}

Variant Instance::t___unset(Variant v_name) {
  const Func* method = m_cls->lookupMethod(s___unset.get());
  if (method) {
    Variant v;
    TypedValue args[1];
    tvDup(v_name.asTypedValue(), args + 0);
    g_vmContext->invokeFuncFew((TypedValue*)&v, method, this, nullptr, 1, args);
    return v;
  } else {
    return uninit_null();
  }
}

Variant Instance::t___sleep() {
  static StringData* sd__sleep = StringData::GetStaticString("__sleep");
  const Func *method = m_cls->lookupMethod(sd__sleep);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    clearAttribute(HasSleep);
    return uninit_null();
  }
}

Variant Instance::t___wakeup() {
  static StringData* sd__wakeup = StringData::GetStaticString("__wakeup");
  const Func *method = m_cls->lookupMethod(sd__wakeup);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

Variant Instance::t___set_state(Variant v_properties) {
  static StringData* sd__set_state = StringData::GetStaticString("__set_state");
  const Func* method = m_cls->lookupMethod(sd__set_state);
  if (method) {
    Variant v;
    TypedValue args[1];
    tvDup(v_properties.asTypedValue(), args + 0);
    g_vmContext->invokeFuncFew((TypedValue*)&v, method, this, nullptr, 1, args);
    return v;
  } else {
    return false;
  }
}

String Instance::t___tostring() {
  const Func *method = m_cls->getToString();
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
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
    g_vmContext->invokeFuncFew(&tv, method, this);
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
    tvDupFlattenVars(&propVec()[i], &iclonePropVec[i], nullptr);
  }
  if (o_properties.get()) {
    iclone->initDynProps();
    ssize_t iter = o_properties.get()->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      auto props = static_cast<HphpArray*>(o_properties.get());
      TypedValue key;
      props->nvGetKey(&key, iter);
      assert(tvIsString(&key));
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

 } // HPHP::VM
