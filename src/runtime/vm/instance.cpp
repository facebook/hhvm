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
#include "runtime/base/tv_macros.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/hhbc.h"
#include "runtime/vm/class.h"
#include "runtime/vm/instance.h"
#include "runtime/vm/object_allocator_sizes.h"

namespace HPHP {
namespace VM {

static StaticString s___get(LITSTR_INIT("__get"));
static StaticString s___set(LITSTR_INIT("__set"));
static StaticString s___isset(LITSTR_INIT("__isset"));
static StaticString s___unset(LITSTR_INIT("__unset"));
static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

//=============================================================================
// Instance.

int HPHP::VM::Instance::ObjAllocatorSizeClassCount =
  HPHP::VM::InitializeAllocators();

void Instance::initialize(unsigned nProps) {
  Class::PropInitVec* propInitVec = g_context->getPropData(m_cls);
  ASSERT(propInitVec != NULL);
  ASSERT(nProps == propInitVec->size());
  memcpy(m_propVec, &(*propInitVec)[0], nProps * sizeof(TypedValue));
}

template <>
void Instance::destructHardImpl<true>(const Func* meth) {
  static ArrayData* args = Unit::mergeAnonArray(StaticEmptyHphpArray::Get());
  // XXX Swallow exceptions that occur in __destruct().
  TypedValue retval;
  g_context->invokeFunc(&retval, meth, CArrRef(args), this);
  tvRefcountedDecRef(&retval);
}

template <>
void Instance::destructHardImpl<false>(const Func* meth) {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  // call destructor only if stack won't overflow.
  if (stack_in_bounds(info) &&
      !g_context->m_stack.wouldOverflow(meth->maxStackCells()
                                         + MAX_STACK_LIMIT)) {
    static ArrayData* args = Unit::mergeAnonArray(StaticEmptyHphpArray::Get());
    TypedValue retval;
    g_context->invokeFunc(&retval, meth, CArrRef(args), this);
    tvRefcountedDecRef(&retval);
  }
}

ALWAYS_INLINE void Instance::destructHard(const Func* method) {
  Instance::destructHardImpl<true>(method);
}

void Instance::forgetSweepable() {
  ASSERT(RuntimeOption::EnableObjDestructCall);
  g_context->m_liveBCObjs.erase(this);
}

void Instance::invokeUserMethod(TypedValue* retval, const Func* method,
                                CArrRef params) {
  g_context->invokeFunc(retval, method, params, this);
}

Object Instance::FromArray(ArrayData *properties) {
  ASSERT(hhvm);
  static const StringData* s_stdclass = StringData::GetStaticString("stdclass");
  Instance* retval = Instance::newInstance(g_context->lookupClass(s_stdclass));
  retval->initPropMap();
  HphpArray* props = retval->m_propMap;
  if (LIKELY(HphpArray::isHphpArray(properties))) {
    HphpArray* oldProps = static_cast<HphpArray*>(properties);
    for (ssize_t pos = oldProps->iter_begin(); pos != ArrayData::invalid_index;
         pos = oldProps->iter_advance(pos)) {
      TypedValue* value = oldProps->nvGetValueRef(pos);
      TypedValue key;
      oldProps->nvGetKey(&key, pos);
      if (key.m_type == KindOfInt64) {
        props->nvSet(key.m_data.num, value, false);
      } else {
        ASSERT(IS_STRING_TYPE(key.m_type));
        props->nvSet(key.m_data.pstr, value, false);
      }
    }
  } else {
    for (ssize_t pos = properties->iter_begin();
         pos != ArrayData::invalid_index; pos = properties->iter_advance(pos)) {
      Variant key(properties->getKey(pos));
      CVarRef value = properties->getValueRef(pos);
      if (key.m_type == KindOfInt64) {
        props->nvSet(key.toInt64(), (TypedValue*)&value, false);
      } else {
        ASSERT(IS_STRING_TYPE(key.m_type));
        props->nvSet(key.asStrRef().get(), (TypedValue*)&value, false);
      }
    }
  }
  return retval;
}

void Instance::initPropMap() {
  // Create m_propMap with at least enough room for all the accessible declared
  // properties, plus one which will soon be inserted.
  m_propMap = NEW(HphpArray)(m_cls->m_declPropNumAccessible + 1);
  m_propMap->incRefCount();
  const Class::PropInfoVec* declProps = &m_cls->m_declPropInfo;
  for (int i = 0, nProps = declProps->size(); i < nProps; ++i) {
    const Class::Prop& prop = (*declProps)[i];
    if (prop.m_name->size() != 0) {
      m_propMap->migrateAndSet(const_cast<StringData*>(prop.m_name), &m_propVec[i]);
    }
  }
}

int Instance::declPropInd(TypedValue* prop) const {
  // Do an address range check to determine whether prop physically resides in
  // m_propVec.
  if (uintptr_t(prop) >= uintptr_t(m_propVec) && uintptr_t(prop) <
      uintptr_t(&m_propVec[m_cls->m_declPropInfo.size()])) {
    return (uintptr_t(prop) - uintptr_t(m_propVec)) / sizeof(TypedValue);
  } else {
    return -1;
  }
}

TypedValue* Instance::getProp(PreClass* ctx, const StringData* key,
                              bool& visible, bool& accessible, bool& unset) {
  unset = false;
  if (ctx == m_cls->m_preClass.get()) {
    // Property access is from within a method of this type's class, so if the
    // property exists, it is accessible.
    TypedValue* prop;
    if (m_propMap == NULL) {
      // There are no dynamically created properties, so if this property
      // exists, it must have been declared.
      int propInd = m_cls->lookupDeclProp(key);
      prop = (propInd < 0) ? NULL : &m_propVec[propInd];
    } else {
      prop = m_propMap->nvGet(key, false);
    }
    visible = accessible = (prop != NULL);
    if (prop != NULL && prop->m_type == KindOfUninit) {
      unset = true;
    }
    return prop;
  }

  if (ctx != NULL) {
    Class* ctxClass = instanceof(ctx);
    bool instanceOfCtxClass = ctxClass != NULL;
    if (ctxClass == NULL) {
      // m_cls isn't a descendant of ctx. Try the other way.
      Class* lookedUp = g_context->lookupClass(ctx->m_name);
      if (lookedUp->classof(m_cls->m_preClass.get()) != NULL) {
        // ctx is a descendant of m_cls.
        ctxClass = lookedUp;
      }
    }
    if (ctxClass != NULL) {
      // One of m_cls and ctx inherits from the other, but they aren't the same
      // class. They can access each other's protected properties, but not
      // private. Always start by looking up the property as if it's declared,
      // and then fall back to looking for a dynamically created property.
      TypedValue* prop;
      int propInd = ctxClass->lookupDeclProp(key);
      // The index we got from the context class is only valid if this
      // object is an instance of the context class.
      if (propInd >= 0 && instanceOfCtxClass) {
        prop = &m_propVec[propInd];
      } else {
        // It's not a declared property in the context class. It's inaccessible
        // if it's a declared private property in the instance's class.
        // Otherwise, it's accessible.
        int instancePropInd = m_cls->lookupDeclProp(key);
        if (instancePropInd >= 0) {
          prop = &m_propVec[instancePropInd];
          if (prop->m_type == KindOfUninit) {
            unset = true;
          }
          visible = true;
          Attr propAttrs = m_cls->m_declPropInfo[instancePropInd].m_attrs;
          switch (propAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
            case AttrPublic:
            case AttrProtected: accessible = true; break;
            case AttrPrivate:
              accessible = g_context->getDebuggerBypassCheck(); break;
            default:            not_reached();
          }
          return prop;
        } else {
          if (m_propMap != NULL) {
            prop = m_propMap->nvGet(key, false);
          } else {
            prop = NULL;
          }
        }
      }
      if (prop == NULL) {
        visible = false;
        accessible = false;
        return NULL;
      } else if (prop->m_type == KindOfUninit) {
        unset = true;
      }
      visible = true;
      accessible = true;
      return prop;
    }

    // ctx and m_cls do not have parent/child relationship. Check
    // if the property is declared in a common ancestor of ctx and  m_cls.
    Class* baseClass = NULL;
    int instancePropInd = m_cls->lookupDeclProp(key);
    if (instancePropInd >= 0) {
      TypedValue* prop = &m_propVec[instancePropInd];
      baseClass = m_cls->m_declPropInfo[instancePropInd].m_class;
      ASSERT(baseClass != NULL);
      if (baseClass != m_cls) {
        ctxClass = g_context->lookupClass(ctx->m_name);
        ASSERT(ctxClass != NULL);
        if (ctxClass->classof(baseClass->m_preClass.get())) {
          // ctx and m_cls have a common ancestor that declares this property,
          // and this property is protected or public.
          if (prop->m_type == KindOfUninit) {
            unset = true;
          }
          visible = true;
          accessible = true;
          return prop;
        }
      }
    }
  } // if (ctx != NULL)

  // Property access is in an effectively anonymous context, so only public
  // properties are accessible.
  if (m_propMap == NULL) {
    // There are no dynamically created properties, so if this property
    // exists, it must have been declared.
    int propInd = m_cls->lookupDeclProp(key);
    if (propInd < 0) {
      Class *cls = m_cls;
      if (g_context->getDebuggerBypassCheck()) {
        // Do lookup bottom-up here to find property on the chain
        while (cls = cls->m_parent.get()) {
          propInd = cls->lookupDeclProp(key);
          if (propInd >= 0) {
            TypedValue* prop = &m_propVec[propInd];
            visible = true;
            accessible = true;
            if (prop->m_type == KindOfUninit) {
              unset = true;
            }
            return prop;
          }
        }
      }
      visible = false;
      accessible = false;
      return NULL;
    }
    TypedValue* prop = &m_propVec[propInd];
    if (prop->m_type == KindOfUninit) {
      unset = true;
    }
    visible = true;
    Attr propAttrs = m_cls->m_declPropInfo[propInd].m_attrs;
    switch (propAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
    case AttrPublic:    accessible = true; break;
    case AttrProtected:
    case AttrPrivate:
      accessible = g_context->getDebuggerBypassCheck(); break;
    default:            not_reached();
    }
    return prop;
  } else {
    TypedValue* prop = m_propMap->nvGet(key, false);
    if (prop == NULL) {
      visible = false;
      accessible = false;
      return NULL;
    } else if (prop->m_type == KindOfUninit) {
      unset = true;
    }
    visible = true;
    // This property may have been declared.
    int propInd = declPropInd(prop);
    if (propInd >= 0) {
      Attr propAttrs = m_cls->m_declPropInfo[propInd].m_attrs;
      switch (propAttrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:    accessible = true; break;
      case AttrProtected:
      case AttrPrivate:
        accessible = g_context->getDebuggerBypassCheck(); break;
      default:            not_reached();
      }
    } else {
      accessible = true;
    }
    return prop;
  }
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

template <bool warn, bool define>
void Instance::propImpl(TypedValue*& retval, PreClass* ctx,
                        const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);

  if (visible) {
    if (accessible) {
      if (unset) {
        if (getAttribute(UseGet)) {
          invokeGet(retval, key);
        } else {
          if (warn) {
            raise_notice("Undefined property: %s::$%s",
                       m_cls->m_preClass->m_name->data(), key->data());
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
        invokeGet(retval, key);
      } else {
        raise_error("Inaccessible property: %s::$%s",
                    m_cls->m_preClass->m_name->data(), key->data());
      }
    }
  } else {
    if (getAttribute(UseGet)) {
      invokeGet(retval, key);
    } else {
      if (warn) {
        raise_notice("Undefined property: %s::$%s",
                     m_cls->m_preClass->m_name->data(), key->data());
      }
      if (define) {
        if (m_propMap == NULL) {
          initPropMap();
        }
        m_propMap->lvalPtr(*(const String*)&key, *(Variant**)(&retval), false,
                           true);
      } else {
        retval = (TypedValue*)&init_null_variant;
      }
    }
  }
}

void Instance::prop(TypedValue*& retval, PreClass* ctx,
                    const StringData* key) {
  propImpl<false, false>(retval, ctx, key);
}

void Instance::propD(TypedValue*& retval, PreClass* ctx,
                     const StringData* key) {
  propImpl<false, true>(retval, ctx, key);
}

void Instance::propW(TypedValue*& retval, PreClass* ctx,
                     const StringData* key) {
  propImpl<true, false>(retval, ctx, key);
}

void Instance::propWD(TypedValue*& retval, PreClass* ctx,
                      const StringData* key) {
  propImpl<true, true>(retval, ctx, key);
}

bool Instance::propIsset(PreClass* ctx, const StringData* key) {
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

bool Instance::propEmpty(PreClass* ctx, const StringData* key) {
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

void Instance::setProp(PreClass* ctx, const StringData* key, TypedValue* val,
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
    return;
  }
  ASSERT(!accessible);
  if (visible) {
    ASSERT(propVal);
    if (!getAttribute(UseSet)) {
      raise_error("Cannot access protected property");
    }
    // Fall through to the last case below
  } else if (!getAttribute(UseSet)) {
    if (m_propMap == NULL) {
      initPropMap();
    }
    m_propMap->lvalPtr(*(const String*)&key, *(Variant**)(&propVal), false,
                       true);
    propImpl<false, true>(propVal, ctx, key);
    if (UNLIKELY(bindingAssignment)) {
      tvBind(val, propVal);
    } else {
      tvSet(val, propVal);
    }
    return;
  }
  ASSERT(!accessible);
  ASSERT(getAttribute(UseSet));
  TypedValue ignored;
  invokeSet(&ignored, key, val);
  tvRefcountedDecRef(&ignored);
}

TypedValue* Instance::setOpProp(TypedValue& tvRef, PreClass* ctx,
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
  } else if (!getAttribute(UseGet)) {
    if (m_propMap == NULL) {
      initPropMap();
    }
    m_propMap->lvalPtr(*(const String*)&key, *(Variant**)(&propVal), false,
                       true);
    tvWriteNull(propVal);
    SETOP_BODY(propVal, op, val);
    return propVal;
  } else if (!getAttribute(UseSet)) {
    TypedValue tvResult;
    tvWriteUninit(&tvResult);
    invokeGet(&tvResult, key);
    SETOP_BODY(&tvResult, op, val);
    if (m_propMap == NULL) {
      initPropMap();
    }
    m_propMap->lvalPtr(*(const String*)&key, *(Variant**)(&propVal),
                       false, true);
    memcpy((void*)propVal, (void*)&tvResult, sizeof(TypedValue));
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

void Instance::incDecProp(TypedValue& tvRef, PreClass* ctx,
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
      IncDecBody(op, &tvResult, &dest);
      if (getAttribute(UseSet)) {
        TypedValue ignored;
        invokeSet(&ignored, key, &tvResult);
        tvRefcountedDecRef(&ignored);
        propVal = &tvResult;
      } else {
        memcpy((void *)propVal, (void *)&tvResult, sizeof(TypedValue));
      }
    } else {
      IncDecBody(op, propVal, &dest);
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
  } else if (!getAttribute(UseGet)) {
    if (m_propMap == NULL) {
      initPropMap();
    }
    m_propMap->lvalPtr(*(const String*)&key, *(Variant**)(&propVal), false,
                       true);
    tvWriteNull(propVal);
    IncDecBody(op, propVal, &dest);
    return;
  } else if (!getAttribute(UseSet)) {
    TypedValue tvResult;
    tvWriteUninit(&tvResult);
    invokeGet(&tvResult, key);
    IncDecBody(op, &tvResult, &dest);
    if (m_propMap == NULL) {
      initPropMap();
    }
    m_propMap->lvalPtr(*(const String*)&key, *(Variant**)(&propVal),
                       false, true);
    memcpy((void*)propVal, (void*)&tvResult, sizeof(TypedValue));
    return;
  }
  ASSERT(!accessible);
  ASSERT(getAttribute(UseGet) && getAttribute(UseSet));
  invokeGet(&tvRef, key);
  IncDecBody(op, &tvRef, &dest);
  TypedValue ignored;
  invokeSet(&ignored, key, &tvRef);
  tvRefcountedDecRef(&ignored);
  propVal = &tvRef;
}

void Instance::unsetProp(PreClass* ctx, const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    int propInd = declPropInd(propVal);
    if (propInd >= 0) {
      // Declared property.
      tvRefcountedDecRef(propVal);
      TV_WRITE_UNINIT(propVal);
    } else {
      // Dynamic property.
      ASSERT(m_propMap != NULL);
      m_propMap->remove(CStrRef(key), false);
    }
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

const String& Instance::o_getClassName() const {
  return *(String*)(&m_cls->m_preClass->m_name);
}

const String& Instance::o_getParentName() const {
  return *(String*)(&m_cls->m_preClass->m_parent);
}

Array Instance::o_toIterArray(CStrRef context, bool getRef /* = false */) {
  int size = (m_propMap != NULL ?
              m_propMap->size() : m_cls->m_declPropNumAccessible);
  HphpArray* retval = NEW(HphpArray)(size);
  PreClass* ctx = NULL;
  if (!context.empty()) {
    ctx = g_context->lookupClass(context.get())->m_preClass.get();
  }

  // Get all declared properties first, bottom-to-top in the inheritance
  // hierarchy, in declaration order.
  const Class* klass = m_cls;
  while (klass != NULL) {
    const PreClass::PropertyVec& props = klass->m_preClass.get()->m_propertyVec;
    for (PreClass::PropertyVec::const_iterator it = props.begin();
         it != props.end(); ++it) {
      StringData* key = const_cast<StringData*>((*it)->m_name);
      bool visible, accessible, unset;
      TypedValue* val = getProp(ctx, key, visible, accessible, unset);
      if (accessible && val->m_type != KindOfUninit && !unset) {
        if (getRef) {
          if (val->m_type != KindOfVariant) {
            tvBox(val);
          }
          retval->migrateAndSet(key, val);
        } else {
          retval->nvSet(key, val, false);
        }
      }
    }
    klass = klass->m_parent.get();
  }

  // Now get dynamic properties.
  if (m_propMap != NULL) {
    ssize_t iter = m_propMap->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      TypedValue key;
      m_propMap->nvGetKey(&key, iter);
      iter = m_propMap->iter_advance(iter);

      // You can get this if you cast an array to object. These properties must
      // be dynamic because you can't declare a property with a non-string name.
      if (UNLIKELY(!IS_STRING_TYPE(key.m_type))) {
        ASSERT(key.m_type == KindOfInt64);
        TypedValue* val = m_propMap->nvGet(key.m_data.num);
        if (getRef) {
          not_implemented();
        } else {
          retval->nvSet(key.m_data.num, val, false);
        }
        continue;
      }

      if (m_cls->lookupDeclProp(key.m_data.pstr) < 0) {
        TypedValue* val = m_propMap->nvGet(key.m_data.pstr);
        if (getRef) {
          if (val->m_type != KindOfVariant) {
            tvBox(val);
          }
          retval->migrateAndSet(key.m_data.pstr, val);
        } else {
          retval->nvSet(key.m_data.pstr, val, false);
        }
      }
    }
  }

  return Array(retval);
}

void Instance::o_setArray(CArrRef properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String k = iter.first().toString();
    PreClass* ctx = NULL;

    // If the key begins with a NUL, it's a private or protected property. Read
    // the class name from between the two NUL bytes.
    if (!k.empty() && k.charAt(0) == '\0') {
      int subLen = k.find('\0', 1) + 1;
      String cls = k.substr(1, subLen - 2);
      if (cls == "*") {
        // Protected.
        ctx = m_cls->m_preClass.get();
      } else {
        // Private.
        ctx = g_context->lookupClass(cls.get())->m_preClass.get();
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

void Instance::o_getArray(Array& props, bool pubOnly /* = false */) const {
  // The declared properties in the resultant array should be a permutation of
  // m_propVec. They appear in the following order: go most-to-least-derived in
  // the inheritance hierarchy, inserting properties in declaration order (with
  // the wrinkle that overridden properties should appear only once, with the
  // access level given to it in its most-derived declaration).

  // This is needed to keep track of which elements have been inserted. This is
  // the smoothest way to get overridden properties right.
  std::vector<bool> inserted;
  for (unsigned i = 0; i < m_cls->m_declPropInfo.size(); ++i) {
    inserted.push_back(false);
  }

  // Iterate over declared properties and insert {mangled name --> prop} pairs.
  const Class* klass = m_cls;
  while (klass != NULL) {
    for (PreClass::PropertyVec::const_iterator it =
           klass->m_preClass->m_propertyVec.begin();
         it != klass->m_preClass->m_propertyVec.end(); ++it) {
      PreClass::Prop* prop = *it;
      if (prop->m_attrs & AttrStatic) {
        continue;
      }

      int propInd = klass->lookupDeclProp(prop->m_name);
      ASSERT(propInd >= 0);
      TypedValue* propVal = &m_propVec[propInd];

      if ((!pubOnly || (prop->m_attrs & AttrPublic)) &&
          propVal->m_type != KindOfUninit &&
          !inserted[propInd]) {
        inserted[propInd] = true;
        props.lvalAt(CStrRef(prop->m_mangledName))
             .setWithRef(tvAsCVarRef(propVal));
      }
    }
    klass = klass->m_parent.get();
  }

  // Iterate over dynamic properties and insert {name --> prop} pairs.
  if (m_propMap != NULL && !m_propMap->empty()) {
    for (ArrayIter it(m_propMap); !it.end(); it.next()) {
      CVarRef val = it.secondRef();
      if (declPropInd((TypedValue*)&val) < 0) {
        // Not a declared property; insert.
        Variant key = it.first();
        CVarRef value = it.secondRef();
        props.addLval(key, true).setWithRef(value);
      }
    }
  }
}

bool Instance::o_get_call_info_hook(const char *clsname,
                                    MethodCallPackage &mcp,
                                    int64 hash /* = -1 */) {
  return false;
}

Variant Instance::t___destruct() {
  static StringData* sd__destruct = StringData::GetStaticString("__destruct");
  const Func* method = m_cls->lookupMethod(sd__destruct);
  if (method) {
    Variant v;
    g_context->invokeFunc((TypedValue*)&v, method, Array::Create(), this);
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
    g_context->invokeFunc((TypedValue*)&v, method,
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
    g_context->invokeFunc((TypedValue*)&v, method,
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
    g_context->invokeFunc((TypedValue*)&v, method,
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
    g_context->invokeFunc((TypedValue*)&v, method,
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
    g_context->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR1(v_name), this);
    return v;
  } else {
    return null;
  }
}

DECLARE_THREAD_LOCAL(Variant, __lvalProxy);
DECLARE_THREAD_LOCAL(Variant, nullProxy);

Variant& Instance::___offsetget_lval(Variant v_name) {
  static StringData* sd__offsetGet = StringData::GetStaticString("offsetGet");
  const Func* method = m_cls->lookupMethod(sd__offsetGet);
  if (method) {
    Variant *v = __lvalProxy.get();
    g_context->invokeFunc((TypedValue*)v, method,
                          CREATE_VECTOR1(v_name), this);
    return *v;
  } else {
    return *nullProxy.get();
  }
}

Variant Instance::t___sleep() {
  static StringData* sd__sleep = StringData::GetStaticString("__sleep");
  const Func *method = m_cls->lookupMethod(sd__sleep);
  if (method) {
    TypedValue tv;
    g_context->invokeFunc(&tv, method, Array::Create(), this);
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
    g_context->invokeFunc(&tv, method, Array::Create(), this);
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
    g_context->invokeFunc((TypedValue*)&v, method,
                          CREATE_VECTOR1(v_properties), this);
    return v;
  } else {
    return false;
  }
}

String Instance::t___tostring() {
  static StringData* sd__tostring = StringData::GetStaticString("__toString");
  const Func *method = m_cls->lookupMethod(sd__tostring);
  if (method) {
    TypedValue tv;
    g_context->invokeFunc(&tv, method, Array::Create(), this);
    if (!IS_STRING_TYPE(tv.m_type)) {
      raise_error("Method %s::__toString() must return a string value",
                  m_cls->m_preClass->m_name->data());
    }
    return tv.m_data.pstr;
  } else {
    std::string msg = m_cls->m_preClass->m_name->data();
    msg += "::__toString() was not defined";
    throw BadTypeConversionException(msg.c_str());
  }
}

Variant Instance::t___clone() {
  static StringData* sd__clone = StringData::GetStaticString("__clone");
  const Func *method = m_cls->lookupMethod(sd__clone);
  if (method) {
    TypedValue tv;
    g_context->invokeFunc(&tv, method, Array::Create(), this);
    return false;
  } else {
    return false;
  }
}

void Instance::cloneSet(ObjectData* clone) {
  Instance* iclone = static_cast<Instance*>(clone);
  unsigned nProps = m_cls->m_declPropInfo.size();
  for (unsigned int i = 0; i < nProps; i++) {
    tvRefcountedDecRef(&iclone->m_propVec[i]);
    TypedValue* fr = &m_propVec[i];
    TypedValue* to = &iclone->m_propVec[i];
    TV_DUP_FLATTEN_VARS(fr, to, NULL);
  }
  iclone->initPropMap();
  if (m_propMap != NULL) {
    ssize_t iter = m_propMap->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      TypedValue key;
      m_propMap->nvGetKey(&key, iter);
      if (m_cls->lookupDeclProp(key.m_data.pstr) < 0) {
        // not a declared property. insert.
        TypedValue* retval;
        TypedValue *val = m_propMap->nvGet(key.m_data.pstr);
        iclone->m_propMap->lvalPtr(*(const String *)&key.m_data.pstr,
                                   *(Variant**)&retval, false, true);
        TV_DUP_FLATTEN_VARS(val, retval, iclone->m_propMap);
      }
      iter = m_propMap->iter_advance(iter);
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

bool Instance::hasCall() {
  return m_cls->lookupMethod(s___call.get()) != NULL;
}

bool Instance::hasCallStatic() {
  return m_cls->lookupMethod(s___callStatic.get()) != NULL;
}

} } // HPHP::VM
