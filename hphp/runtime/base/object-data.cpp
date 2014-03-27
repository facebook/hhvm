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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_datetime.h"
#include "hphp/runtime/ext/ext_domdocument.h"
#include "hphp/runtime/ext/ext_simplexml.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/system/systemlib.h"

#include "folly/Hash.h"
#include "folly/ScopeGuard.h"

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

static Array ArrayObject_toArray(const ObjectData* obj) {
  bool visible, accessible, unset;
  auto prop = obj->getProp(
    SystemLib::s_ArrayObjectClass, s_storage.get(),
    visible, accessible, unset
  );
  assert(visible && accessible && !unset);
  return tvAsCVarRef(prop).toArray();
}

static_assert(sizeof(ObjectData) == 32, "Change this only on purpose");

//////////////////////////////////////////////////////////////////////

bool ObjectData::destruct() {
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    g_context->m_liveBCObjs.erase(this);
  }
  if (!noDestruct()) {
    setNoDestruct();
    if (auto meth = m_cls->getDtor()) {
      // We don't run PHP destructors while we're unwinding for a C++ exception.
      // We want to minimize the PHP code we run while propagating fatals, so
      // we do this check here on a very common path, in the relativley slower
      // case.
      auto& faults = g_context->m_faults;
      if (!faults.empty()) {
        if (faults.back().m_faultType == Fault::Type::CppException) return true;
      }
      // We raise the refcount around the call to __destruct(). This is to
      // prevent the refcount from going to zero when the destructor returns.
      CountableHelper h(this);
      RefCount c = this->getCount();
      TypedValue retval;
      tvWriteNull(&retval);
      try {
        // Call the destructor method
        g_context->invokeFuncFew(&retval, meth, this);
      } catch (...) {
        // Swallow any exceptions that escape the __destruct method
        handle_destructor_exception();
      }
      tvRefcountedDecRef(&retval);
      return c == this->getCount();
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// class info

const String& ObjectData::o_getClassName() const {
  return *(const String*)(&m_cls->preClass()->nameRef());
}

bool ObjectData::o_instanceof(const String& s) const {
  Class* cls = Unit::lookupClass(s.get());
  if (!cls) return false;
  return m_cls->classof(cls);
}

bool ObjectData::o_toBooleanImpl() const noexcept {
  // Note: if you add more cases here, hhbbc/class-util.cpp also needs
  // to be changed.
  if (isCollection()) {
    if (m_cls == c_Vector::classof()) {
      return c_Vector::ToBool(this);
    } else if (m_cls == c_Map::classof()) {
      return c_Map::ToBool(this);
    } else if (m_cls == c_ImmMap::classof()) {
      return c_ImmMap::ToBool(this);
    } else if (m_cls == c_Set::classof()) {
      return c_Set::ToBool(this);
    } else if (m_cls == c_ImmVector::classof()) {
      return c_ImmVector::ToBool(this);
    } else if (m_cls == c_ImmSet::classof()) {
      return c_ImmSet::ToBool(this);
    } else {
      always_assert(false);
    }
  } else if (instanceof(c_SimpleXMLElement::classof())) {
    // SimpleXMLElement is the only non-collection class that has custom
    // bool casting.
    return c_SimpleXMLElement::ToBool(this);
  }
  always_assert(false);
  return false;
}

int64_t ObjectData::o_toInt64Impl() const noexcept {
  // SimpleXMLElement is the only class that has proper custom int casting.
  // If others are added in future, just turn this assert into an if and
  // add cases.
  assert(instanceof(c_SimpleXMLElement::classof()));
  return c_SimpleXMLElement::ToInt64(this);
}

double ObjectData::o_toDoubleImpl() const noexcept {
  // SimpleXMLElement is the only non-collection class that has custom
  // double casting. If others are added in future, just turn this assert
  // into an if and add cases.
  assert(instanceof(c_SimpleXMLElement::classof()));
  return c_SimpleXMLElement::ToDouble(this);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

const StaticString s_getIterator("getIterator");

Object ObjectData::iterableObject(bool& isIterable,
                                  bool mayImplementIterator /* = true */) {
  assert(mayImplementIterator || !implementsIterator());
  if (mayImplementIterator && implementsIterator()) {
    isIterable = true;
    return Object(this);
  }
  Object obj(this);
  while (obj->instanceof(SystemLib::s_IteratorAggregateClass)) {
    Variant iterator = obj->o_invoke_few_args(s_getIterator, 0);
    if (!iterator.isObject()) break;
    ObjectData* o = iterator.getObjectData();
    if (o->instanceof(SystemLib::s_IteratorClass)) {
      isIterable = true;
      return o;
    }
    obj = o;
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
  if (getAttribute(HasDynPropArr)) return dynPropArray();

  assert(!g_context->dynPropTable.count(this));
  auto& arr = g_context->dynPropTable[this].arr();
  arr = Array::attach(HphpArray::MakeReserve(numDynamic));
  setAttribute(HasDynPropArr);
  return arr;
}

Variant* ObjectData::o_realProp(const String& propName, int flags,
                                const String& context /* = null_string */) {
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

  bool visible, accessible, unset;
  auto ret = getProp(ctx, propName.get(), visible, accessible, unset);
  if (!ret) {
    // Property is not declared, and not dynamically created yet.
    if (!(flags & RealPropCreate)) {
      return nullptr;
    }
    return &reserveProperties().lvalAt(propName, AccessFlags::Key);
  }

  // ret is non-NULL if we reach here
  assert(visible);
  if ((accessible && !unset) ||
      (flags & (RealPropUnchecked|RealPropExist))) {
    return reinterpret_cast<Variant*>(ret);
  } else {
    return nullptr;
  }
}

inline Variant ObjectData::o_getImpl(const String& propName, int flags,
                                     bool error /* = true */,
                                     const String& context /*= null_string*/) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  if (Variant* t = o_realProp(propName, flags, context)) {
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
    raise_notice("Undefined property: %s::$%s", o_getClassName().data(),
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

Variant ObjectData::o_set(const String& propName, RefResult v) {
  return o_setRef(propName, variant(v), null_string);
}

Variant ObjectData::o_setRef(const String& propName, const Variant& v) {
  return o_setImpl<RefResult>(propName, ref(v), null_string);
}

Variant ObjectData::o_set(const String& propName, const Variant& v,
                          const String& context) {
  return o_setImpl<const Variant&>(propName, v, context);
}

Variant ObjectData::o_set(const String& propName, RefResult v,
                          const String& context) {
  return o_setRef(propName, variant(v), context);
}

Variant ObjectData::o_setRef(const String& propName, const Variant& v,
                             const String& context) {
  return o_setImpl<RefResult>(propName, ref(v), context);
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
    setProp(ctx, k.get(), (TypedValue*)(&secondRef),
            secondRef.isReferenced());
  }
}

void ObjectData::o_getArray(Array& props, bool pubOnly /* = false */) const {
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
      getProps(cls, pubOnly, traitCls->preClass(), props, inserted);
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

Array ObjectData::o_toArray(bool pubOnly /* = false */) const {
  // We can quickly tell if this object is a collection, which lets us avoid
  // checking for each class in turn if it's not one.
  if (isCollection()) {
    if (m_cls == c_Vector::classof()) {
      return c_Vector::ToArray(this);
    } else if (m_cls == c_Map::classof()) {
      return c_Map::ToArray(this);
    } else if (m_cls == c_Set::classof()) {
      return c_Set::ToArray(this);
    } else if (m_cls == c_Pair::classof()) {
      return c_Pair::ToArray(this);
    } else if (m_cls == c_ImmVector::classof()) {
      return c_ImmVector::ToArray(this);
    } else if (m_cls == c_ImmMap::classof()) {
      return c_ImmMap::ToArray(this);
    } else if (m_cls == c_ImmSet::classof()) {
      return c_ImmSet::ToArray(this);
    }
    // It's undefined what happens if you reach not_reached. We want to be sure
    // to hard fail if we get here.
    always_assert(false);
  } else if (UNLIKELY(getAttribute(CallToImpl))) {
    // If we end up with other classes that need special behavior, turn the
    // assert into an if and add cases.
    assert(instanceof(c_SimpleXMLElement::classof()));
    return c_SimpleXMLElement::ToArray(this);
  } else if (UNLIKELY(instanceof(SystemLib::s_ArrayObjectClass))) {
    return ArrayObject_toArray(this);
  } else {
    Array ret(ArrayData::Create());
    o_getArray(ret, pubOnly);
    return ret;
  }
}

Array ObjectData::o_toIterArray(const String& context,
                                bool getRef /* = false */) {
  Array* dynProps = nullptr;
  size_t size = m_cls->declPropNumAccessible();
  if (getAttribute(HasDynPropArr)) {
    dynProps = &dynPropArray();
    size += dynProps->size();
  }
  Array retArray { Array::attach(HphpArray::MakeReserve(size)) };

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
      bool visible, accessible, unset;
      auto val = getProp(ctx, key, visible, accessible, unset);
      if (accessible && val->m_type != KindOfUninit && !unset) {
        if (getRef) {
          if (val->m_type != KindOfRef) {
            tvBox(val);
          }
          retArray.setRef(StrNR(key), tvAsCVarRef(val), true /* isKey */);
        } else {
          retArray.set(StrNR(key), tvAsCVarRef(val), true /* isKey */);
        }
      }
    }
    klass = klass->parent();
  }

  // Now get dynamic properties.
  if (dynProps) {
    ssize_t iter = dynProps->get()->iter_begin();
    while (iter != ArrayData::invalid_index) {
      TypedValue key;
      dynProps->get()->nvGetKey(&key, iter);
      iter = dynProps->get()->iter_advance(iter);

      // You can get this if you cast an array to object. These
      // properties must be dynamic because you can't declare a
      // property with a non-string name.
      if (UNLIKELY(!IS_STRING_TYPE(key.m_type))) {
        assert(key.m_type == KindOfInt64);
        TypedValue* val = dynProps->get()->nvGet(key.m_data.num);
        if (getRef) {
          if (val->m_type != KindOfRef) {
            tvBox(val);
          }
          retArray.setRef(key.m_data.num, tvAsCVarRef(val));
        } else {
          retArray.set(key.m_data.num, tvAsCVarRef(val));
        }
        continue;
      }

      StringData* strKey = key.m_data.pstr;
      TypedValue* val = dynProps->get()->nvGet(strKey);
      if (getRef) {
        if (val->m_type != KindOfRef) {
          tvBox(val);
        }
        retArray.setRef(StrNR(strKey), tvAsCVarRef(val), true /* isKey */);
      } else {
        retArray.set(StrNR(strKey), tvAsCVarRef(val), true /* isKey */);
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
    if (ctx.func->attrs() & AttrStatic) {
      // If we found a method and its static, null out this_
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
  s_PHP_Unserializable_Class_Name("__PHP_Unserializable_Class_Name"),
  s_debugInfo("__debugInfo");

/* Get properties from the actual object unless we're
 * serializing for var_dump()/print_r() and the object
 * exports a __debugInfo() magic method.
 * In which case, call that and use the array it returns.
 */
inline Array getSerializeProps(const ObjectData* obj,
                               VariableSerializer* serializer) {
  if ((serializer->getType() != VariableSerializer::Type::PrintR) &&
      (serializer->getType() != VariableSerializer::Type::VarDump)) {
    return obj->o_toArray();
  }
  auto cls = obj->getVMClass();
  auto debuginfo = cls->lookupMethod(s_debugInfo.get());
  if (!debuginfo) {
    return obj->o_toArray();
  }
  if (debuginfo->attrs() & (AttrPrivate|AttrProtected|
                            AttrAbstract|AttrStatic)) {
    raise_warning("%s::__debugInfo() must be public and non-static",
                  cls->name()->data());
    return obj->o_toArray();
  }
  Variant ret = const_cast<ObjectData*>(obj)->o_invoke_few_args(s_debugInfo, 0);
  if (ret.isArray()) {
    return ret.toArray();
  }
  if (ret.isNull()) {
    return Array::Create();
  }
  raise_error("__debugInfo() must return an array");
  not_reached();
}

void ObjectData::serializeImpl(VariableSerializer* serializer) const {
  bool handleSleep = false;
  Variant ret;

  if (LIKELY(serializer->getType() == VariableSerializer::Type::Serialize ||
             serializer->getType() == VariableSerializer::Type::APCSerialize)) {
    if (instanceof(SystemLib::s_SerializableClass)) {
      assert(!isCollection());
      Variant ret =
        const_cast<ObjectData*>(this)->o_invoke_few_args(s_serialize, 0);
      if (ret.isString()) {
        serializer->writeSerializableObject(o_getClassName(), ret.toString());
      } else if (ret.isNull()) {
        serializer->writeNull();
      } else {
        raise_error("%s::serialize() must return a string or NULL",
                    o_getClassName().data());
      }
      return;
    }
    // Only serialize CPP extension type instances which can actually
    // be deserialized.
    auto cls = getVMClass();
    if (cls->instanceCtor() && !cls->isCppSerializable()) {
      Object placeholder = ObjectData::newInstance(
        SystemLib::s___PHP_Unserializable_ClassClass);
      placeholder->o_set(s_PHP_Unserializable_Class_Name, o_getClassName());
      placeholder->serialize(serializer);
      return;
    }
    if (getAttribute(HasSleep)) {
      handleSleep = true;
      ret = const_cast<ObjectData*>(this)->invokeSleep();
    }
  } else if (UNLIKELY(serializer->getType() ==
                      VariableSerializer::Type::DebuggerSerialize)) {
    if (instanceof(SystemLib::s_SerializableClass)) {
      assert(!isCollection());
      try {
        Variant ret =
          const_cast<ObjectData*>(this)->o_invoke_few_args(s_serialize, 0);
        if (ret.isString()) {
          serializer->writeSerializableObject(o_getClassName(), ret.toString());
        } else if (ret.isNull()) {
          serializer->writeNull();
        } else {
          raise_warning("%s::serialize() must return a string or NULL",
                        o_getClassName().data());
          serializer->writeNull();
        }
      } catch (...) {
        // serialize() throws exception
        raise_warning("%s::serialize() throws exception",
                      o_getClassName().data());
        serializer->writeNull();
      }
      return;
    }
    // Don't try to serialize a CPP extension class which doesn't
    // support serialization. Just send the class name instead.
    if (getAttribute(IsCppBuiltin) && !getVMClass()->isCppSerializable()) {
      serializer->write(o_getClassName());
      return;
    }
    if (getAttribute(HasSleep)) {
      try {
        handleSleep = true;
        ret = const_cast<ObjectData*>(this)->invokeSleep();
      } catch (...) {
        raise_warning("%s::sleep() throws exception", o_getClassName().data());
        serializer->writeNull();
        return;
      }
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

        bool accessible;
        Slot propInd = m_cls->getDeclPropIndex(ctx, memberName.get(),
                                               accessible);
        if (propInd != kInvalidSlot) {
          if (accessible) {
            const TypedValue* prop = &propVec()[propInd];
            if (prop->m_type != KindOfUninit) {
              auto attrs = m_cls->declProperties()[propInd].m_attrs;
              if (attrs & AttrPrivate) {
                memberName = concat4(s_zero, ctx->nameRef(),
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
      serializer->setObjectInfo(o_getClassName(), o_getId(), 'O');
      wanted.serialize(serializer, true);
    } else {
      raise_notice("serialize(): __sleep should return an array only "
                   "containing the names of instance-variables to "
                   "serialize");
      uninit_null().serialize(serializer);
    }
  } else {
    if (isCollection()) {
      collectionSerialize(const_cast<ObjectData*>(this), serializer);
    } else {
      const String& className = o_getClassName();
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
            o_getClassName().data());
        }
      }
      if (serializer->getType() == VariableSerializer::Type::DebuggerDump) {
        Variant* debugDispVal = const_cast<ObjectData*>(this)->  // XXX
          o_realProp(s_PHP_DebugDisplay, 0);
        if (debugDispVal) {
          debugDispVal->serialize(serializer);
          return;
        }
      }
      if (serializer->getType() != VariableSerializer::Type::VarDump &&
          className == s_PHP_Incomplete_Class) {
        Variant* cname = const_cast<ObjectData*>(this)-> // XXX
          o_realProp(s_PHP_Incomplete_Class_Name, 0);
        if (cname && cname->isString()) {
          serializer->setObjectInfo(cname->toCStrRef(), o_getId(), 'O');
          properties.remove(s_PHP_Incomplete_Class_Name, true);
          properties.serialize(serializer, true);
          return;
        }
      }
      serializer->setObjectInfo(className, o_getId(), 'O');
      properties.serialize(serializer, true);
    }
  }
}

ObjectData* ObjectData::clone() {
  if (getAttribute(HasClone) && getAttribute(IsCppBuiltin)) {
    if (isCollection()) {
      if (m_cls == c_Vector::classof()) {
        return c_Vector::Clone(this);
      } else if (m_cls == c_Map::classof()) {
        return c_Map::Clone(this);
      } else if (m_cls == c_ImmMap::classof()) {
        return c_ImmMap::Clone(this);
      } else if (m_cls == c_Set::classof()) {
        return c_Set::Clone(this);
      } else if (m_cls == c_Pair::classof()) {
        return c_Pair::Clone(this);
      } else if (m_cls == c_ImmVector::classof()) {
        return c_ImmVector::Clone(this);
      } else if (m_cls == c_ImmSet::classof()) {
        return c_ImmSet::Clone(this);
      } else {
        always_assert(false);
      }
    } else if (instanceof(c_Closure::classof())) {
      return c_Closure::Clone(this);
    } else if (instanceof(c_Continuation::classof())) {
      return c_Continuation::Clone(this);
    } else if (instanceof(c_DateTime::classof())) {
      return c_DateTime::Clone(this);
    } else if (instanceof(c_DateTimeZone::classof())) {
      return c_DateTimeZone::Clone(this);
    } else if (instanceof(c_DateInterval::classof())) {
      return c_DateInterval::Clone(this);
    } else if (instanceof(c_DOMNode::classof())) {
      return c_DOMNode::Clone(this);
    } else if (instanceof(c_SimpleXMLElement::classof())) {
      return c_SimpleXMLElement::Clone(this);
    }
    always_assert(false);
  }

  return cloneImpl();
}

Variant ObjectData::offsetGet(Variant key) {
  assert(instanceof(SystemLib::s_ArrayAccessClass));
  const Func* method = m_cls->lookupMethod(s_offsetGet.get());
  assert(method);
  if (!method) {
    return uninit_null();
  }
  Variant v;
  g_context->invokeFuncFew(v.asTypedValue(), method,
                             this, nullptr, 1, key.asCell());
  return v;
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
      tvIncRef(dst);
      collectionDeepCopyTV(dst);
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
  const Func* init = m_cls->lookupMethod(s___init__.get());
  assert(init);
  TypedValue tv;
  // We need to incRef/decRef here because we're still a new (m_count
  // == 0) object and invokeFunc is going to expect us to have a
  // reasonable refcount.
  try {
    incRefCount();
    g_context->invokeFuncFew(&tv, init, this);
    decRefCount();
    assert(!IS_REFCOUNTED_TYPE(tv.m_type));
  } catch (...) {
    this->setNoDestruct();
    decRefObj(this);
    throw;
  }
  return this;
}

ObjectData* ObjectData::newInstanceRaw(Class* cls, uint32_t size) {
  return new (MM().smartMallocSizeLogged(size))
    ObjectData(cls, NoInit::noinit);
}

ObjectData* ObjectData::newInstanceRawBig(Class* cls, size_t size) {
  return new (MM().smartMallocSizeBigLogged<false>(size).first)
    ObjectData(cls, NoInit::noinit);
}

NEVER_INLINE
static void freeDynPropArray(ObjectData* inst) {
  auto& table = g_context->dynPropTable;
  auto it = table.find(inst);
  assert(it != end(table));
  it->second.destroy();
  table.erase(it);
}

ObjectData::~ObjectData() {
  int& pmax = os_max_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
  if (UNLIKELY(getAttribute(HasDynPropArr))) freeDynPropArray(this);
}

void ObjectData::DeleteObject(ObjectData* objectData) {
  auto const cls = objectData->getVMClass();

  if (UNLIKELY(objectData->getAttribute(InstanceDtor))) {
    return cls->instanceDtor()(objectData, cls);
  }

  assert(!cls->preClass()->builtinObjSize());
  assert(!cls->preClass()->builtinODOffset());
  objectData->~ObjectData();

  // ObjectData subobject is logically destructed now---don't access
  // objectData->foo for anything.

  auto const nProps = size_t{cls->numDeclProperties()};
  auto prop = reinterpret_cast<TypedValue*>(objectData + 1);
  auto const stop = prop + nProps;
  for (; prop != stop; ++prop) {
    tvRefcountedDecRef(prop);
  }

  auto const size = sizeForNProps(nProps);
  if (LIKELY(size <= kMaxSmartSize)) {
    return MM().smartFreeSizeLogged(objectData, size);
  }
  MM().smartFreeSizeBigLogged(objectData, size);
}

Object ObjectData::FromArray(ArrayData* properties) {
  ObjectData* retval = ObjectData::newInstance(SystemLib::s_stdclassClass);
  auto& dynArr = retval->reserveProperties(properties->size());
  for (ssize_t pos = properties->iter_begin(); pos != ArrayData::invalid_index;
       pos = properties->iter_advance(pos)) {
    TypedValue* value = properties->nvGetValueRef(pos);
    TypedValue key;
    properties->nvGetKey(&key, pos);
    if (key.m_type == KindOfInt64) {
      dynArr.set(key.m_data.num, tvAsCVarRef(value));
    } else {
      assert(IS_STRING_TYPE(key.m_type));
      StringData* strKey = key.m_data.pstr;
      dynArr.set(StrNR(strKey), tvAsCVarRef(value), true /* isKey */);
      decRefStr(strKey);
    }
  }
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

TypedValue* ObjectData::getProp(Class* ctx, const StringData* key,
                                bool& visible, bool& accessible,
                                bool& unset) {
  unset = false;

  Slot propInd = m_cls->getDeclPropIndex(ctx, key, accessible);
  visible = (propInd != kInvalidSlot);
  if (LIKELY(propInd != kInvalidSlot)) {
    // We found a visible property, but it might not be accessible.
    // No need to check if there is a dynamic property with this name.
    auto const prop = &propVec()[propInd];
    if (prop->m_type == KindOfUninit) {
      unset = true;
    }

    if (debug) {
      if (RuntimeOption::RepoAuthoritative && Repo::get().global().UsedHHBBC) {
        auto const repoTy = m_cls->declPropRepoAuthType(propInd);
        always_assert(tvMatchesRepoAuthType(*prop, repoTy));
      }
    }

    return prop;
  }

  // We could not find a visible declared property. We need to check
  // for a dynamic property with this name.
  assert(!visible && !accessible);
  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    if (auto const prop = dynPropArray()->nvGet(key)) {
      // Returned a non-declared property, we know that it is
      // visible and accessible (since all dynamic properties are),
      // and we know it is not unset (since unset dynamic properties
      // don't appear in the dynamic property array).
      visible = true;
      accessible = true;
      return prop;
    }
  }

  return nullptr;
}

const TypedValue* ObjectData::getProp(Class* ctx, const StringData* key,
                                      bool& visible, bool& accessible,
                                      bool& unset) const {
  return const_cast<ObjectData*>(this)->getProp(
    ctx, key, visible, accessible, unset
  );
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
  typedef smart::hash_set<PropAccessInfo,PropAccessInfo::Hash> RecurSet;

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
      propRecurInfo.activeSet = smart_new<PropRecurInfo::RecurSet>();
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
      smart_delete(propRecurInfo.activeSet);
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
    g_context->invokeFuncFew(retval, meth, info.obj, nullptr, 1, args);
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
      g_context->invokeFuncFew(retval, meth, this, nullptr, 2, args);
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

bool ObjectData::invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                               const StringData* key) {
  if (!invokeGet(&tvRef, key)) return false;
  retval = &tvRef;
  return true;
}

//////////////////////////////////////////////////////////////////////

template <bool warn, bool define>
void ObjectData::propImpl(TypedValue*& retval, TypedValue& tvRef,
                          Class* ctx,
                          const StringData* key) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);

  if (visible) {
    if (accessible) {
      if (unset) {
        if (!getAttribute(UseGet) || !invokeGetProp(retval, tvRef, key)) {
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
      if (!getAttribute(UseGet) || !invokeGetProp(retval, tvRef, key)) {
        // No need to check hasProp since visible is true
        // Visibility is either protected or private since accessible is false
        Slot propInd = m_cls->lookupDeclProp(key);
        bool priv = m_cls->declProperties()[propInd].m_attrs & AttrPrivate;

        raise_error("Cannot access %s property %s::$%s",
                    priv ? "private" : "protected",
                    m_cls->preClass()->name()->data(),
                    key->data());
      }
    }
  } else {
    if (getAttribute(UseGet) && invokeGetProp(retval, tvRef, key)) {
      return;
    }

    if (UNLIKELY(!*key->data())) {
      throw_invalid_property_name(StrNR(key));
    } else {
      if (warn) {
        raiseUndefProp(key);
      }
      if (define) {
        retval = reinterpret_cast<TypedValue*>(
          &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
        );
      } else {
        retval = const_cast<TypedValue*>(
          reinterpret_cast<const TypedValue*>(&init_null_variant)
        );
      }
    }
  }
}

void ObjectData::prop(TypedValue*& retval, TypedValue& tvRef,
                      Class* ctx, const StringData* key) {
  propImpl<false, false>(retval, tvRef, ctx, key);
}

void ObjectData::propD(TypedValue*& retval, TypedValue& tvRef,
                       Class* ctx, const StringData* key) {
  propImpl<false, true>(retval, tvRef, ctx, key);
}

void ObjectData::propW(TypedValue*& retval, TypedValue& tvRef,
                       Class* ctx, const StringData* key) {
  propImpl<true, false>(retval, tvRef, ctx, key);
}

void ObjectData::propWD(TypedValue*& retval, TypedValue& tvRef,
                        Class* ctx, const StringData* key) {
  propImpl<true, true>(retval, tvRef, ctx, key);
}

bool ObjectData::propIsset(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible && !unset) {
    return !cellIsNull(tvToCell(propVal));
  }

  auto tv = make_tv<KindOfUninit>();
  if (!getAttribute(UseIsset) || !invokeIsset(&tv, key)) {
    return false;
  }
  tvCastToBooleanInPlace(&tv);
  return tv.m_data.num;
}

bool ObjectData::propEmpty(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible && !unset) {
    return !cellToBool(*tvToCell(propVal));
  }

  auto tv = make_tv<KindOfUninit>();
  if (!getAttribute(UseIsset) || !invokeIsset(&tv, key)) {
    return true;
  }
  tvCastToBooleanInPlace(&tv);
  if (!tv.m_data.num) {
    return true;
  }
  if (getAttribute(UseGet)) {
    if (invokeGet(&tv, key)) {
      bool emptyResult = !cellToBool(*tvToCell(&tv));
      tvRefcountedDecRef(&tv);
      return emptyResult;
    }
  }
  return false;
}

void ObjectData::setProp(Class* ctx,
                         const StringData* key,
                         TypedValue* val,
                         bool bindingAssignment /* = false */) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    assert(propVal);

    TypedValue ignored;
    if (!unset || !getAttribute(UseSet) || !invokeSet(&ignored, key, val)) {
      if (UNLIKELY(bindingAssignment)) {
        tvBind(val, propVal);
      } else {
        tvSet(*val, *propVal);
      }
      return;
    }
    tvRefcountedDecRef(&ignored);
    return;
  }

  TypedValue ignored;
  if (!getAttribute(UseSet) || !invokeSet(&ignored, key, val)) {
    if (visible) {
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
    // directly to the TypedValue in the HphpArray, since
    // its m_aux field is used to store the string hash of
    // the property name. Instead, call the appropriate
    // setters (set() or setRef()).
    if (UNLIKELY(bindingAssignment)) {
      reserveProperties().setRef(
        StrNR(key), tvAsCVarRef(val), true /* isKey */);
    } else {
      reserveProperties().set(
        StrNR(key), tvAsCVarRef(val), true /* isKey */);
    }
    return;
  }

  tvRefcountedDecRef(&ignored);
}

TypedValue* ObjectData::setOpProp(TypedValue& tvRef, Class* ctx,
                                  SetOpOp op, const StringData* key,
                                  Cell* val) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);

  if (visible && accessible) {
    if (unset && getAttribute(UseGet)) {
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
        cellDup(*tvToCell(&tvResult), *propVal);
        return propVal;
      }
    }

    propVal = tvToCell(propVal);
    SETOP_BODY_CELL(propVal, op, val);
    return propVal;
  }

  if (UNLIKELY(!*key->data())) throw_invalid_property_name(StrNR(key));

  auto const useSet = getAttribute(UseSet);
  auto const useGet = getAttribute(UseGet);

  if (useGet && !useSet) {
    auto tvResult = make_tv<KindOfNull>();
    // If invokeGet fails due to recursion, it leaves the KindOfNull.
    invokeGet(&tvResult, key);

    // Note: the tvUnboxIfNeeded comes *after* the setop on purpose
    // here, even though it comes before the IncDecOp in the analagous
    // situation in incDecProp.  This is to match zend 5.5 behavior.
    SETOP_BODY(&tvResult, op, val);
    tvUnboxIfNeeded(&tvResult);

    if (visible) raise_error("Cannot access protected property");
    propVal = reinterpret_cast<TypedValue*>(
      &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
    );

    // Normally this code path is defining a new dynamic property, but
    // unlike the non-magic case below, we may have already created it
    // under the recursion into invokeGet above, so we need to do a
    // tvSet here.
    tvSet(tvResult, *propVal);
    return propVal;
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

  if (visible) raise_error("Cannot access protected property");

  // No visible/accessible property, and no applicable magic method:
  // create a new dynamic property.  (We know this is a new property,
  // or it would've hit the visible && accessible case above.)
  propVal = reinterpret_cast<TypedValue*>(
    &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
  );
  assert(propVal->m_type == KindOfNull); // cannot exist yet
  SETOP_BODY_CELL(propVal, op, val);
  return propVal;
}

template <bool setResult>
void ObjectData::incDecProp(TypedValue& tvRef,
                            Class* ctx,
                            IncDecOp op,
                            const StringData* key,
                            TypedValue& dest) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);

  if (visible && accessible) {
    auto tvResult = make_tv<KindOfUninit>();
    if (unset && getAttribute(UseGet) && invokeGet(&tvResult, key)) {
      IncDecBody<setResult>(op, &tvResult, &dest);
      TypedValue ignored;
      if (getAttribute(UseSet) && invokeSet(&ignored, key, &tvResult)) {
        tvRefcountedDecRef(&ignored);
        propVal = &tvResult;
      } else {
        memcpy(propVal, &tvResult, sizeof(TypedValue));
      }
      return;
    }

    IncDecBody<setResult>(op, propVal, &dest);
    return;
  }

  if (UNLIKELY(!*key->data())) throw_invalid_property_name(StrNR(key));

  auto const useSet = getAttribute(UseSet);
  auto const useGet = getAttribute(UseGet);

  if (useGet && !useSet) {
    auto tvResult = make_tv<KindOfNull>();
    // If invokeGet fails due to recursion, it leaves the KindOfNull
    // in tvResult.
    invokeGet(&tvResult, key);
    tvUnboxIfNeeded(&tvResult);
    IncDecBody<setResult>(op, &tvResult, &dest);
    if (visible) raise_error("Cannot access protected property");
    propVal = reinterpret_cast<TypedValue*>(
      &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
    );

    // Normally this code path is defining a new dynamic property, but
    // unlike the non-magic case below, we may have already created it
    // under the recursion into invokeGet above, so we need to do a
    // tvSet here.
    tvSet(tvResult, *propVal);
    return;
  }

  if (useGet && useSet) {
    if (invokeGet(&tvRef, key)) {
      tvUnboxIfNeeded(&tvRef);
      IncDecBody<setResult>(op, &tvRef, &dest);
      TypedValue ignored;
      if (invokeSet(&ignored, key, &tvRef)) {
        tvRefcountedDecRef(&ignored);
      }
      return;
    }
  }

  if (visible) raise_error("Cannot access protected property");

  // No visible/accessible property, and no applicable magic method:
  // create a new dynamic property.  (We know this is a new property,
  // or it would've hit the visible && accessible case above.)
  propVal = reinterpret_cast<TypedValue*>(
    &reserveProperties().lvalAt(StrNR(key), AccessFlags::Key)
  );
  assert(propVal->m_type == KindOfNull); // cannot exist yet
  IncDecBody<setResult>(op, propVal, &dest);
}

template void ObjectData::incDecProp<true>(TypedValue&,
                                           Class*,
                                           IncDecOp,
                                           const StringData*,
                                           TypedValue&);
template void ObjectData::incDecProp<false>(TypedValue&,
                                            Class*,
                                            IncDecOp,
                                            const StringData*,
                                            TypedValue&);

void ObjectData::unsetProp(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  auto propVal = getProp(ctx, key, visible, accessible, unset);
  Slot propInd = declPropInd(propVal);

  if (visible && accessible && !unset) {
    if (propInd != kInvalidSlot) {
      // Declared property.
      tvSetIgnoreRef(*null_variant.asTypedValue(), *propVal);
    } else {
      // Dynamic property.
      dynPropArray().remove(StrNR(key).asString(),
                            true /* isString */);
    }
    return;
  }

  bool tryUnset = getAttribute(UseUnset);

  if (propInd != kInvalidSlot && !accessible && !tryUnset) {
    // defined property that is not accessible
    raise_error("Cannot unset inaccessible property");
  }

  TypedValue ignored;
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
              (attrs & AttrTrait)     ? "trait" : "abstract class",
              cls->preClass()->name()->data());
}

void ObjectData::raiseUndefProp(const StringData* key) {
  raise_notice("Undefined property: %s::$%s",
               m_cls->name()->data(), key->data());
}

void ObjectData::getProp(const Class* klass, bool pubOnly,
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

Variant ObjectData::invokeSleep() {
  const Func* method = m_cls->lookupMethod(s___sleep.get());
  if (method) {
    TypedValue tv;
    g_context->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

Variant ObjectData::invokeToDebugDisplay() {
  const Func* method = m_cls->lookupMethod(s___toDebugDisplay.get());
  if (method) {
    TypedValue tv;
    g_context->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

Variant ObjectData::invokeWakeup() {
  const Func* method = m_cls->lookupMethod(s___wakeup.get());
  if (method) {
    TypedValue tv;
    g_context->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

String ObjectData::invokeToString() {
  const Func* method = m_cls->getToString();
  if (!method) {
    // If the object does not define a __toString() method, raise a
    // recoverable error
    raise_recoverable_error(
      "Object of class %s could not be converted to string",
      m_cls->preClass()->name()->data()
    );
    // If the user error handler decides to allow execution to continue,
    // we return the empty string.
    return empty_string;
  }
  TypedValue tv;
  g_context->invokeFuncFew(&tv, method, this);
  if (!IS_STRING_TYPE(tv.m_type)) {
    // Discard the value returned by the __toString() method and raise
    // a recoverable error
    tvRefcountedDecRef(tv);
    raise_recoverable_error(
      "Method %s::__toString() must return a string value",
      m_cls->preClass()->name()->data());
    // If the user error handler decides to allow execution to continue,
    // we return the empty string.
    return empty_string;
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
    auto& dynProps = dynPropArray();
    auto& cloneProps = clone->reserveProperties(dynProps.size());

    ssize_t iter = dynProps.get()->iter_begin();
    while (iter != ArrayData::invalid_index) {
      auto props = static_cast<HphpArray*>(dynProps.get());
      TypedValue key;
      props->nvGetKey(&key, iter);
      assert(tvIsString(&key));
      StringData* strKey = key.m_data.pstr;
      TypedValue* val = props->nvGet(strKey);

      auto const retval = reinterpret_cast<TypedValue*>(
        &cloneProps.lvalAt(String(strKey), AccessFlags::Key)
      );
      tvDupFlattenVars(val, retval, cloneProps.get());
      iter = dynProps.get()->iter_advance(iter);
      decRefStr(strKey);
    }
  }
}

ObjectData* ObjectData::cloneImpl() {
  ObjectData* obj;
  Object o = obj = ObjectData::newInstance(m_cls);
  cloneSet(obj);
  if (UNLIKELY(getAttribute(HasNativeData))) {
    Native::nativeDataInstanceCopy(obj, this);
  }

  auto const hasCloneBit = getAttribute(HasClone);

  if (!hasCloneBit) return o.detach();

  auto const method = obj->m_cls->lookupMethod(s_clone.get());

  // PHP classes that inherit from cpp builtins that have special clone
  // functionality *may* also define a __clone method, but it's totally
  // fine if a __clone doesn't exist.
  if (!method && getAttribute(IsCppBuiltin)) return o.detach();
  assert(method);

  TypedValue tv;
  tvWriteNull(&tv);
  g_context->invokeFuncFew(&tv, method, obj);
  tvRefcountedDecRef(&tv);

  return o.detach();
}

RefData* ObjectData::zGetProp(Class* ctx, const StringData* key,
                              bool& visible, bool& accessible,
                              bool& unset) {
  auto tv = getProp(ctx, key, visible, accessible, unset);
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
  return tv->m_data.pref;
}

bool ObjectData::hasDynProps() const {
  return getAttribute(HasDynPropArr) && dynPropArray().size() != 0;
}

void ObjectData::getChildren(std::vector<TypedValue*>& out) {
  Slot nProps = m_cls->numDeclProperties();
  for (Slot i = 0; i < nProps; ++i) {
    out.push_back(&propVec()[i]);
  }
  if (UNLIKELY(getAttribute(HasDynPropArr))) {
    dynPropArray()->getChildren(out);
  }
}

const char* ObjectData::classname_cstr() const {
  return o_getClassName().data();
}

void ObjectData::compileTimeAssertions() {
  static_assert(offsetof(ObjectData, m_count) == FAST_REFCOUNT_OFFSET, "");
}

} // HPHP
