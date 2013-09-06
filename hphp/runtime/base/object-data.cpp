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

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/memory-profile.h"
#include "hphp/util/lock.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_datetime.h"
#include "hphp/runtime/ext/ext_domdocument.h"
#include "hphp/runtime/ext/ext_simplexml.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

// current maximum object identifier
IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(int, ObjectData::os_max_id);

int ObjectData::GetMaxId() {
  return *(ObjectData::os_max_id.getCheck());
}

const StaticString
  s_offsetGet("offsetGet"),
  s___call("__call"),
  s___callStatic("__callStatic"),
  s_serialize("serialize");

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor

ObjectData::~ObjectData() {
  if (ArrayData* a = o_properties.get()) decRefArr(a);
  int& pmax = *os_max_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
}

bool ObjectData::instanceof(const Class* c) const {
  return m_cls->classof(c);
}

HOT_FUNC
void ObjectData::destruct() {
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    assert(RuntimeOption::EnableObjDestructCall);
    g_vmContext->m_liveBCObjs.erase(this);
  }
  if (!noDestruct()) {
    setNoDestruct();
    if (auto meth = m_cls->getDtor()) {
      // We don't run PHP destructors while we're unwinding for a C++ exception.
      // We want to minimize the PHP code we run while propagating fatals, so
      // we do this check here on a very common path, in the relativley slower
      // case.
      auto& faults = g_vmContext->m_faults;
      if (!faults.empty()) {
        if (faults.back().m_faultType == Fault::Type::CppException) return;
      }
      // We raise the refcount around the call to __destruct(). This is to
      // prevent the refcount from going to zero when the destructor returns.
      CountableHelper h(this);
      TypedValue retval;
      tvWriteNull(&retval);
      try {
        // Call the destructor method
        g_vmContext->invokeFuncFew(&retval, meth, this);
      } catch (...) {
        // Swallow any exceptions that escape the __destruct method
        handle_destructor_exception();
      }
      tvRefcountedDecRef(&retval);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// class info

CStrRef ObjectData::o_getClassName() const {
  return *(const String*)(&m_cls->preClass()->nameRef());
}

HOT_FUNC
bool ObjectData::o_instanceof(CStrRef s) const {
  Class* cls = Unit::lookupClass(s.get());
  if (!cls) return false;
  return m_cls->classof(cls);
}

bool ObjectData::o_toBooleanImpl() const noexcept {
  // SimpleXMLElement is the only class that has custom bool casting. If others
  // are added in future, just turn this assert into an if and add cases.
  assert(instanceof(c_SimpleXMLElement::s_cls));
  return c_SimpleXMLElement::ToBoolean(this);
}
int64_t ObjectData::o_toInt64Impl() const noexcept {
  // SimpleXMLElement is the only class that has custom int casting. If others
  // are added in future, just turn this assert into an if and add cases.
  assert(instanceof(c_SimpleXMLElement::s_cls));
  return c_SimpleXMLElement::ToInt64(this);
}
double ObjectData::o_toDoubleImpl() const noexcept {
  // SimpleXMLElement is the only class that has custom double casting. If
  // others are added in future, just turn this assert into an if and add cases.
  assert(instanceof(c_SimpleXMLElement::s_cls));
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
    obj = iterator.getObjectData();
  }
  isIterable = false;
  return obj;
}

ArrayIter ObjectData::begin(CStrRef context /* = null_string */) {
  bool isIterable;
  if (isCollection()) {
    return ArrayIter(this);
  }
  Object iterable = iterableObject(isIterable);
  if (isIterable) {
    return ArrayIter(iterable, ArrayIter::transferOwner);
  } else {
    return ArrayIter(iterable->o_toIterArray(context));
  }
}

MutableArrayIter ObjectData::begin(Variant* key, Variant& val,
                                   CStrRef context /* = null_string */) {
  bool isIterable;
  if (isCollection()) {
    raise_error("Collection elements cannot be taken by reference");
  }
  Object iterable = iterableObject(isIterable);
  if (isIterable) {
    throw FatalErrorException("An iterator cannot be used with "
                              "foreach by reference");
  }
  Array properties = iterable->o_toIterArray(context, true);
  ArrayData* arr = properties.detach();
  return MutableArrayIter(arr, key, val);
}

void ObjectData::initProperties(int nProp) {
  if (!o_properties.get()) initDynProps(nProp);
}

Variant* ObjectData::o_realProp(CStrRef propName, int flags,
                                CStrRef context /* = null_string */) const {
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

  auto thiz = const_cast<ObjectData*>(this);
  bool visible, accessible, unset;
  TypedValue* ret = thiz->getProp(ctx, propName.get(), visible,
                                  accessible, unset);
  if (!ret) {
    // Property is not declared, and not dynamically created yet.
    if (!(flags & RealPropCreate)) {
      return nullptr;
    }
    if (!o_properties.get()) {
      thiz->initDynProps();
    }
    o_properties.get()->lval(propName, *(Variant**)(&ret), false);
    return (Variant*)ret;
  }

  // ret is non-NULL if we reach here
  assert(visible);
  if ((accessible && !unset) ||
      (flags & (RealPropUnchecked|RealPropExist))) {
    return (Variant*)ret;
  } else {
    return nullptr;
  }
}

inline Variant ObjectData::o_getImpl(CStrRef propName, int flags,
                                     bool error /* = true */,
                                     CStrRef context /* = null_string */) {
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
    invokeGet(&tvResult, propName.get());
    return tvAsCVarRef(&tvResult);
  }

  if (error) {
    raise_notice("Undefined property: %s::$%s", o_getClassName().data(),
                 propName.data());
  }

  return uninit_null();
}

Variant ObjectData::o_get(CStrRef propName, bool error /* = true */,
                          CStrRef context /* = null_string */) {
  return o_getImpl(propName, 0, error, context);
}

template <class T>
inline ALWAYS_INLINE Variant ObjectData::o_setImpl(CStrRef propName, T v,
                                                   CStrRef context) {
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

  if (useSet) {
    TypedValue ignored;
    invokeSet(&ignored, propName.get(), (TypedValue*)(&variant(v)));
    tvRefcountedDecRef(&ignored);
    return variant(v);
  }

  return variant(v);
}

Variant ObjectData::o_set(CStrRef propName, CVarRef v) {
  return o_setImpl<CVarRef>(propName, v, null_string);
}

Variant ObjectData::o_set(CStrRef propName, RefResult v) {
  return o_setRef(propName, variant(v), null_string);
}

Variant ObjectData::o_setRef(CStrRef propName, CVarRef v) {
  return o_setImpl<RefResult>(propName, ref(v), null_string);
}

Variant ObjectData::o_set(CStrRef propName, CVarRef v, CStrRef context) {
  return o_setImpl<CVarRef>(propName, v, context);
}

Variant ObjectData::o_set(CStrRef propName, RefResult v, CStrRef context) {
  return o_setRef(propName, variant(v), context);
}

Variant ObjectData::o_setRef(CStrRef propName, CVarRef v, CStrRef context) {
  return o_setImpl<RefResult>(propName, ref(v), context);
}

HOT_FUNC
void ObjectData::o_setArray(CArrRef properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String k = iter.first().toString();
    Class* ctx = nullptr;
    // If the key begins with a NUL, it's a private or protected property. Read
    // the class name from between the two NUL bytes.
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

    CVarRef secondRef = iter.secondRef();
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
    for (auto const& trait : cls->usedTraits()) {
      getProps(cls, pubOnly, trait->preClass(), props, inserted);
    }
    cls = cls->parent();
  } while (cls);

  // Iterate over dynamic properties and insert {name --> prop} pairs.
  if (o_properties.get() && !o_properties.get()->empty()) {
    for (ArrayIter it(o_properties.get()); !it.end(); it.next()) {
      props.setWithRef(it.first(), it.secondRef());
    }
  }
}

Array ObjectData::o_toArray() const {
  // We can quickly tell if this object is a collection, which lets us avoid
  // checking for each class in turn if it's not one.
  if (isCollection()) {
    if (instanceof(c_Vector::s_cls)) {
      return c_Vector::ToArray(this);
    } else if (instanceof(c_Map::s_cls)) {
      return c_Map::ToArray(this);
    } else if (instanceof(c_StableMap::s_cls)) {
      return c_StableMap::ToArray(this);
    } else if (instanceof(c_Set::s_cls)) {
      return c_Set::ToArray(this);
    } else if (instanceof(c_Pair::s_cls)) {
      return c_Pair::ToArray(this);
    }
    // It's undefined what happens if you reach not_reached. We want to be sure
    // to hard fail if we get here.
    always_assert(false);
  } else if (UNLIKELY(getAttribute(CallToImpl))) {
    // If we end up with other classes that need special behavior, turn the
    // assert into an if and add cases.
    assert(instanceof(c_SimpleXMLElement::s_cls));
    return c_SimpleXMLElement::ToArray(this);
  } else {
    Array ret(ArrayData::Create());
    o_getArray(ret, false);
    return ret;
  }
}

Array ObjectData::o_toIterArray(CStrRef context,
                                bool getRef /* = false */) {
  size_t size = m_cls->declPropNumAccessible() +
                (o_properties.get() ? o_properties.get()->size() : 0);
  auto retval = ArrayData::Make(size);
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
      TypedValue* val = getProp(ctx, key, visible, accessible, unset);
      if (accessible && val->m_type != KindOfUninit && !unset) {
        if (getRef) {
          if (val->m_type != KindOfRef) {
            tvBox(val);
          }
          retval->nvBind(key, val);
        } else {
          retval->set(key, tvAsCVarRef(val), false);
        }
      }
    }
    klass = klass->parent();
  }

  // Now get dynamic properties.
  if (o_properties.get()) {
    ssize_t iter = o_properties.get()->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      TypedValue key;
      static_cast<HphpArray*>(o_properties.get())->nvGetKey(&key, iter);
      iter = o_properties.get()->iter_advance(iter);

      // You can get this if you cast an array to object. These properties must
      // be dynamic because you can't declare a property with a non-string name.
      if (UNLIKELY(!IS_STRING_TYPE(key.m_type))) {
        assert(key.m_type == KindOfInt64);
        TypedValue* val =
          static_cast<HphpArray*>(o_properties.get())->nvGet(key.m_data.num);
        if (getRef) {
          if (val->m_type != KindOfRef) {
            tvBox(val);
          }
          retval->nvBind(key.m_data.num, val);
        } else {
          retval->set(key.m_data.num, tvAsCVarRef(val), false);
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
        retval->set(strKey, tvAsCVarRef(val), false);
      }
      decRefStr(strKey);
    }
  }

  return Array(retval);
}

static bool decode_invoke(CStrRef s, ObjectData* obj, bool fatal,
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
    ctx.func = ctx.cls->lookupMethod(s___call.get());

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

Variant ObjectData::o_invoke(CStrRef s, CArrRef params,
                             bool fatal /* = true */) {
  CallCtx ctx;
  if (!decode_invoke(s, this, fatal, ctx)) {
    return Variant(Variant::NullInit());
  }
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, ctx, params);
  return ret;
}

Variant ObjectData::o_invoke_few_args(CStrRef s, int count,
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
  g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, count, args);
  return ret;
}

StaticString s_zero("\0", 1);

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
  s_PHP_Unserializable_Class_Name("__PHP_Unserializable_Class_Name");

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
    if ((builtinPropSize() > 0) && !getVMClass()->isCppSerializable()) {
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
    if ((builtinPropSize() > 0) && !getVMClass()->isCppSerializable()) {
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
      auto thiz = const_cast<ObjectData*>(this);
      Array wanted = Array::Create();
      Array props = ret.toArray();
      for (ArrayIter iter(props); iter; ++iter) {
        String name = iter.second().toString();
        bool visible, accessible, unset;
        thiz->getProp(m_cls, name.get(), visible, accessible, unset);
        if (accessible && !unset) {
          String propName = name;
          Slot propInd = m_cls->getDeclPropIndex(m_cls, name.get(), accessible);
          if (accessible && propInd != kInvalidSlot) {
            if (m_cls->declProperties()[propInd].m_attrs & AttrPrivate) {
              propName = concat4(s_zero, o_getClassName(), s_zero, name);
            }
          }
          wanted.set(propName, const_cast<ObjectData*>(this)->
              o_getImpl(name, RealPropUnchecked, true, o_getClassName()));
        } else {
          raise_warning("\"%s\" returned as member variable from "
              "__sleep() but does not exist", name.data());
          wanted.set(name, uninit_null());
        }
      }
      serializer->setObjectInfo(o_getClassName(), o_getId(), 'O');
      wanted.serialize(serializer, true);
    } else {
      raise_warning("serialize(): __sleep should return an array only "
                    "containing the names of instance-variables to "
                    "serialize");
      uninit_null().serialize(serializer);
    }
  } else {
    if (isCollection()) {
      collectionSerialize(const_cast<ObjectData*>(this), serializer);
    } else {
      CStrRef className = o_getClassName();
      Array properties = o_toArray();
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
        Variant* debugDispVal = o_realProp(s_PHP_DebugDisplay, 0);
        if (debugDispVal) {
          debugDispVal->serialize(serializer);
          return;
        }
      }
      if (serializer->getType() != VariableSerializer::Type::VarDump &&
          className == s_PHP_Incomplete_Class) {
        Variant* cname = o_realProp(s_PHP_Incomplete_Class_Name, 0);
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

bool ObjectData::hasInternalReference(PointerSet& vars,
                                      bool ds /* = false */) const {
  if (isCollection()) {
    return true;
  }
  return o_toArray().get()->hasInternalReference(vars, ds);
}

void ObjectData::dump() const {
  o_toArray().dump();
}

ObjectData* ObjectData::clone() {
  if (getAttribute(HasClone)) {
    if (isCollection()) {
      if (instanceof(c_Vector::s_cls)) {
        return c_Vector::Clone(this);
      } else if (instanceof(c_Map::s_cls)) {
        return c_Map::Clone(this);
      } else if (instanceof(c_StableMap::s_cls)) {
        return c_StableMap::Clone(this);
      } else if (instanceof(c_Set::s_cls)) {
        return c_Set::Clone(this);
      } else if (instanceof(c_Pair::s_cls)) {
        return c_Pair::Clone(this);
      }
    } else if (instanceof(c_Closure::s_cls)) {
      return c_Closure::Clone(this);
    } else if (instanceof(c_Continuation::s_cls)) {
      return c_Continuation::Clone(this);
    } else if (instanceof(c_DateTime::s_cls)) {
      return c_DateTime::Clone(this);
    } else if (instanceof(c_DateTimeZone::s_cls)) {
      return c_DateTimeZone::Clone(this);
    } else if (instanceof(c_DateInterval::s_cls)) {
      return c_DateInterval::Clone(this);
    } else if (instanceof(c_DOMNode::s_cls)) {
      return c_DOMNode::Clone(this);
    } else if (instanceof(c_SimpleXMLElement::s_cls)) {
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
  g_vmContext->invokeFuncFew(v.asTypedValue(), method,
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

TRACE_SET_MOD(runtime);

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
  uintptr_t ret = (uintptr_t)this + sizeof(ObjectData) + builtinPropSize();
  // TODO(#1432007): some builtins still do not have TypedValue-aligned sizes.
  assert(ret % sizeof(TypedValue) == builtinPropSize() % sizeof(TypedValue));
  return (TypedValue*) ret;
}

const TypedValue* ObjectData::propVec() const {
  return const_cast<ObjectData*>(this)->propVec();
}

ObjectData* ObjectData::callCustomInstanceInit() {
  const Func* init = m_cls->lookupMethod(s___init__.get());
  if (init != nullptr) {
    TypedValue tv;
    // We need to incRef/decRef here because we're still a new (m_count
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
ObjectData* ObjectData::newInstanceRaw(Class* cls, uint32_t size) {
  return new (MM().smartMallocSize(size)) ObjectData(cls, NoInit::noinit);
}

ObjectData* ObjectData::newInstanceRawBig(Class* cls, size_t size) {
  return new (MM().smartMallocSizeBig(size)) ObjectData(cls, NoInit::noinit);
}

void ObjectData::operator delete(void* p) {
  ObjectData* this_ = (ObjectData*)p;
  Class* cls = this_->getVMClass();
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = cls->builtinPropSize();
  TypedValue* propVec = (TypedValue*)((uintptr_t)this_ + sizeof(ObjectData) +
                                      builtinPropSize);
  for (unsigned i = 0; i < nProps; ++i) {
    TypedValue* prop = &propVec[i];
    tvRefcountedDecRef(prop);
  }

  auto const size = sizeForNProps(nProps) + builtinPropSize;
  if (LIKELY(size <= MemoryManager::kMaxSmartSize)) {
    return MM().smartFreeSize(this_, size);
  }
  MM().smartFreeSizeBig(this_, size);
}

Object ObjectData::FromArray(ArrayData* properties) {
  ObjectData* retval = ObjectData::newInstance(SystemLib::s_stdclassClass);
  retval->initDynProps();
  HphpArray* props = static_cast<HphpArray*>(retval->o_properties.get());
  for (ssize_t pos = properties->iter_begin(); pos != ArrayData::invalid_index;
       pos = properties->iter_advance(pos)) {
    TypedValue* value = properties->nvGetValueRef(pos);
    TypedValue key;
    properties->nvGetKey(&key, pos);
    if (key.m_type == KindOfInt64) {
      props->set(key.m_data.num, tvAsCVarRef(value), false);
    } else {
      assert(IS_STRING_TYPE(key.m_type));
      StringData* strKey = key.m_data.pstr;
      props->set(strKey, tvAsCVarRef(value), false);
      decRefStr(strKey);
    }
  }
  return retval;
}

void ObjectData::initDynProps(int numDynamic /* = 0 */) {
  // Create o_properties with room for numDynamic
  o_properties.asArray() = ArrayData::Make(numDynamic);
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
    // We could not find a visible declared property. We need to check
    // for a dynamic property with this name.
    if (o_properties.get()) {
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

void ObjectData::invokeSet(TypedValue* retval, const StringData* key,
                           TypedValue* val) {
  AttributeClearer a(UseSet, this);
  const Func* meth = m_cls->lookupMethod(s___set.get());
  assert(meth);
  TypedValue args[2] = {
    make_tv<KindOfString>(const_cast<StringData*>(key)),
    *tvToCell(val)
  };
  g_vmContext->invokeFuncFew(retval, meth, this, nullptr, 2, args);
}

#define MAGIC_PROP_BODY(name, attr) \
  AttributeClearer a((attr), this); \
  const Func* meth = m_cls->lookupMethod(name); \
  assert(meth); \
  TypedValue args[1] = { \
    make_tv<KindOfString>(const_cast<StringData*>(key)) \
  }; \
  g_vmContext->invokeFuncFew(retval, meth, this, nullptr, 1, args);

void ObjectData::invokeGet(TypedValue* retval, const StringData* key) {
  MAGIC_PROP_BODY(s___get.get(), UseGet);
}

void ObjectData::invokeIsset(TypedValue* retval, const StringData* key) {
  MAGIC_PROP_BODY(s___isset.get(), UseIsset);
}

void ObjectData::invokeUnset(TypedValue* retval, const StringData* key) {
  MAGIC_PROP_BODY(s___unset.get(), UseUnset);
}

void ObjectData::invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                               const StringData* key) {
  invokeGet(&tvRef, key);
  retval = &tvRef;
}

template <bool warn, bool define>
void ObjectData::propImpl(TypedValue*& retval, TypedValue& tvRef,
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
                    m_cls->preClass()->name()->data(),
                    key->data());
      }
    }
  } else {
    if (getAttribute(UseGet)) {
      invokeGetProp(retval, tvRef, key);
    } else if (UNLIKELY(!*key->data())) {
      throw_invalid_property_name(StrNR(key));
    } else {
      if (warn) {
        raiseUndefProp(key);
      }
      if (define) {
        if (o_properties.get() == nullptr) {
          initDynProps();
        }
        o_properties.get()->lval(*(const String*)&key,
                                 *(Variant**)(&retval), false);
      } else {
        retval = (TypedValue*)&init_null_variant;
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
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible && !unset) {
    return !tvIsNull(tvToCell(propVal));
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

bool ObjectData::propEmpty(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible && !unset) {
    return !cellToBool(*tvToCell(propVal));
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
    bool emptyResult = !cellToBool(*tvToCell(&tv));
    tvRefcountedDecRef(&tv);
    return emptyResult;
  }
  return false;
}

void ObjectData::setProp(Class* ctx,
                         const StringData* key,
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
        tvSet(*val, *propVal);
      }
    }
    return;
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
    // its m_aux field is used to store the string hash of
    // the property name. Instead, call the appropriate
    // setters (set() or setRef()).
    if (UNLIKELY(bindingAssignment)) {
      o_properties.get()->setRef(const_cast<StringData*>(key),
                                 tvAsCVarRef(val), false);
    } else {
      o_properties.get()->set(const_cast<StringData*>(key),
                              tvAsCVarRef(val), false);
    }
    return;
  }

  assert(!accessible);
  assert(getAttribute(UseSet));
  TypedValue ignored;
  invokeSet(&ignored, key, val);
  tvRefcountedDecRef(&ignored);
}

TypedValue* ObjectData::setOpProp(TypedValue& tvRef, Class* ctx,
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
        cellDup(*tvToCell(&tvResult), tvRef);
        TypedValue ignored;
        invokeSet(&ignored, key, &tvRef);
        tvRefcountedDecRef(&ignored);
        propVal = &tvRef;
      } else {
        cellDup(*tvToCell(&tvResult), *propVal);
      }
    } else {
      propVal = tvToCell(propVal);
      SETOP_BODY_CELL(propVal, op, val);
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
    o_properties.get()->lval(*(const String*)&key,
                             *(Variant**)(&propVal), false);
    // don't write propVal->m_aux because it holds data
    // owned by the HphpArray
    propVal->m_type = KindOfNull;
    SETOP_BODY_CELL(propVal, op, val);
    return propVal;
  } else if (!getAttribute(UseSet)) {
    TypedValue tvResult;
    tvWriteUninit(&tvResult);
    invokeGet(&tvResult, key);
    SETOP_BODY(&tvResult, op, val);
    if (o_properties.get() == nullptr) {
      initDynProps();
    }
    o_properties.get()->lval(*(const String*)&key,
                             *(Variant**)(&propVal), false);
    // don't write propVal->m_aux because it holds data
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
void ObjectData::incDecPropImpl(TypedValue& tvRef, Class* ctx,
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
        memcpy((void*)propVal, (void*)&tvResult, sizeof(TypedValue));
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
    o_properties.get()->lval(*(const String*)&key,
                             *(Variant**)(&propVal), false);
    // don't write propVal->m_aux because it holds data
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
    o_properties.get()->lval(*(const String*)&key,
                             *(Variant**)(&propVal), false);
    // don't write propVal->m_aux because it holds data
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

template <>
void ObjectData::incDecProp<false>(TypedValue& tvRef, Class* ctx,
                                   unsigned char op, const StringData* key,
                                   TypedValue& dest) {
  incDecPropImpl<false>(tvRef, ctx, op, key, dest);
}

template <>
void ObjectData::incDecProp<true>(TypedValue& tvRef, Class* ctx,
                                  unsigned char op, const StringData* key,
                                  TypedValue& dest) {
  incDecPropImpl<true>(tvRef, ctx, op, key, dest);
}

void ObjectData::unsetProp(Class* ctx, const StringData* key) {
  bool visible, accessible, unset;
  TypedValue* propVal = getProp(ctx, key, visible, accessible, unset);
  if (visible && accessible) {
    Slot propInd = declPropInd(propVal);
    if (propInd != kInvalidSlot) {
      // Declared property.
      tvSetIgnoreRef(*null_variant.asTypedValue(), *propVal);
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
    props.lvalAt(CStrRef(klass->declProperties()[propInd].m_mangledName))
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
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

Variant ObjectData::invokeToDebugDisplay() {
  const Func* method = m_cls->lookupMethod(s___toDebugDisplay.get());
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

Variant ObjectData::invokeWakeup() {
  const Func* method = m_cls->lookupMethod(s___wakeup.get());
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

String ObjectData::invokeToString() {
  const Func* method = m_cls->getToString();
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    if (!IS_STRING_TYPE(tv.m_type)) {
      void (*notify_user)(const char*, ...) = &raise_error;
      if (hphpiCompat) {
        tvCastToStringInPlace(&tv);
        notify_user = &raise_warning;
      }
      notify_user("Method %s::__toString() must return a string value",
                  m_cls->preClass()->name()->data());
    }
    return tv.m_data.pstr;
  }
  raise_recoverable_error(
    "Object of class %s could not be converted to string",
    m_cls->preClass()->name()->data()
  );
  // If the user error handler decides to allow execution to continue,
  // we return the empty string.
  return empty_string;
}

bool ObjectData::hasToString() {
  return (m_cls->getToString() != nullptr);
}

void ObjectData::cloneSet(ObjectData* clone) {
  Slot nProps = m_cls->numDeclProperties();
  TypedValue* clonePropVec = (TypedValue*)((uintptr_t)clone +
                               sizeof(ObjectData) + builtinPropSize());
  for (Slot i = 0; i < nProps; i++) {
    tvRefcountedDecRef(&clonePropVec[i]);
    tvDupFlattenVars(&propVec()[i], &clonePropVec[i], nullptr);
  }
  if (o_properties.get()) {
    clone->initDynProps();
    ssize_t iter = o_properties.get()->iter_begin();
    while (iter != HphpArray::ElmIndEmpty) {
      auto props = static_cast<HphpArray*>(o_properties.get());
      TypedValue key;
      props->nvGetKey(&key, iter);
      assert(tvIsString(&key));
      StringData* strKey = key.m_data.pstr;
      TypedValue* val = props->nvGet(strKey);
      TypedValue* retval;
      auto cloneProps = clone->o_properties.get();
      cloneProps->lval(strKey, *(Variant**)&retval, false);
      tvDupFlattenVars(val, retval, cloneProps);
      iter = o_properties.get()->iter_advance(iter);
      decRefStr(strKey);
    }
  }
}

ObjectData* ObjectData::cloneImpl() {
  ObjectData* obj;
  Object o = obj = ObjectData::newInstance(m_cls);
  cloneSet(obj);
  static StringData* sd__clone = StringData::GetStaticString("__clone");
  const Func* method = obj->m_cls->lookupMethod(sd__clone);
  if (method) {
    TypedValue tv;
    tvWriteNull(&tv);
    g_vmContext->invokeFuncFew(&tv, method, obj);
    tvRefcountedDecRef(&tv);
  }
  return o.detach();
}

} // HPHP


