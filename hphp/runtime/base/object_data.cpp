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

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/util/lock.h"
#include "hphp/runtime/base/class_info.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_simplexml.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

// current maximum object identifier
IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(int, ObjectData::os_max_id);

int ObjectData::GetMaxId() {
  return *(ObjectData::os_max_id.getCheck());
}

static StaticString s_offsetGet("offsetGet");
static StaticString s___call("__call");
static StaticString s___callStatic("__callStatic");
static StaticString s_serialize("serialize");

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor

ObjectData::~ObjectData() {
  if (ArrayData* a = o_properties.get()) decRefArr(a);
  int &pmax = *os_max_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
}

bool ObjectData::instanceof(const HPHP::Class* c) const {
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
  if (isResource()) return o_getClassNameHook();
  return *(const String*)(&m_cls->m_preClass->nameRef());
}

CStrRef ObjectData::o_getParentName() const {
  if (isResource()) return empty_string;
  return *(const String*)(&m_cls->m_preClass->parentRef());
}

CStrRef ObjectData::o_getClassNameHook() const {
  throw FatalErrorException("Class didnt provide a name");
  return empty_string;
}

HOT_FUNC
bool ObjectData::o_instanceof(CStrRef s) const {
  HPHP::Class* cls = Unit::lookupClass(s.get());
  if (!cls) return false;
  return m_cls->classof(cls);
}

bool ObjectData::o_toBooleanImpl() const noexcept {
  not_reached();
}
int64_t ObjectData::o_toInt64Impl() const noexcept {
  not_reached();
}
double ObjectData::o_toDoubleImpl() const noexcept {
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

static StaticString s_getIterator("getIterator");

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

MutableArrayIter ObjectData::begin(Variant *key, Variant &val,
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
  ArrayData *arr = properties.detach();
  return MutableArrayIter(arr, key, val);
}

void ObjectData::initProperties(int nProp) {
  if (!o_properties.get()) ((HPHP::Instance*)this)->initDynProps(nProp);
}

Variant* ObjectData::o_realProp(CStrRef propName, int flags,
                                CStrRef context /* = null_string */) const {
  /*
   * Returns a pointer to a place for a property value. This should never
   * call the magic methods __get or __set. The flags argument describes the
   * behavior in cases where the named property is nonexistent or
   * inaccessible.
   */
  HPHP::Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = Unit::lookupClass(context.get());
  }

  HPHP::Instance* thiz = (HPHP::Instance*)(this);  // sigh
  bool visible, accessible, unset;
  TypedValue* ret = (flags & RealPropNoDynamic)
                    ? thiz->getDeclProp(ctx, propName.get(), visible,
                                        accessible, unset)
                    : thiz->getProp(ctx, propName.get(), visible,
                                    accessible, unset);
  if (!ret) {
    // Property is not declared, and not dynamically created yet.
    if (!(flags & RealPropCreate)) {
      return nullptr;
    }
    assert(!(flags & RealPropNoDynamic));
    if (!o_properties.get()) {
      thiz->initDynProps();
    }
    o_properties.get()->createLvalPtr(propName, *(Variant**)(&ret), false);
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

  if (Variant *t = o_realProp(propName, flags, context)) {
    if (t->isInitialized())
      return *t;
  }

  if (getAttribute(UseGet)) {
    AttributeClearer a(UseGet, this);
    return t___get(propName);
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
                                                   bool forInit,
                                                   CStrRef context) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  bool useSet = !forInit && getAttribute(UseSet);
  int flags = useSet ? 0 : RealPropCreate;
  if (forInit) flags |= RealPropUnchecked;

  if (Variant *t = o_realProp(propName, flags, context)) {
    if (!useSet || t->isInitialized()) {
      *t = v;
      return variant(v);
    }
  }

  if (useSet) {
    AttributeClearer a(UseSet, this);
    t___set(propName, variant(v));
    return variant(v);
  }

  return variant(v);
}

Variant ObjectData::o_set(CStrRef propName, CVarRef v) {
  return o_setImpl<CVarRef>(propName, v, false, null_string);
}

Variant ObjectData::o_set(CStrRef propName, RefResult v) {
  return o_setRef(propName, variant(v), null_string);
}

Variant ObjectData::o_setRef(CStrRef propName, CVarRef v) {
  return o_setImpl<RefResult>(propName, ref(v), false, null_string);
}

Variant ObjectData::o_set(CStrRef propName, CVarRef v, CStrRef context) {
  return o_setImpl<CVarRef>(propName, v, false, context);
}

Variant ObjectData::o_set(CStrRef propName, RefResult v, CStrRef context) {
  return o_setRef(propName, variant(v), context);
}

Variant ObjectData::o_setRef(CStrRef propName, CVarRef v, CStrRef context) {
  return o_setImpl<RefResult>(propName, ref(v), false, context);
}

HOT_FUNC
void ObjectData::o_setArray(CArrRef properties) {
  auto thiz = static_cast<Instance*>(this);
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
    thiz->setProp(ctx, k.get(), (TypedValue*)(&secondRef),
                  secondRef.isReferenced());
  }
}

void ObjectData::o_getArray(Array &props, bool pubOnly /* = false */) const {
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
  auto thiz = static_cast<const Instance*>(this);
  do {
    thiz->getProps(cls, pubOnly, cls->m_preClass.get(), props, inserted);
    const std::vector<ClassPtr> &usedTraits = cls->m_usedTraits;
    for (unsigned t = 0; t < usedTraits.size(); t++) {
      const ClassPtr& trait = usedTraits[t];
      thiz->getProps(cls, pubOnly, trait->m_preClass.get(), props, inserted);
    }
    cls = cls->m_parent.get();
  } while (cls);

  // Iterate over dynamic properties and insert {name --> prop} pairs.
  if (o_properties.get() && !o_properties.get()->empty()) {
    for (ArrayIter it(o_properties.get()); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      props.addLval(key, true).setWithRef(value);
    }
  }
}

Object ObjectData::FromArray(ArrayData *properties) {
  ObjectData *ret = SystemLib::AllocStdClassObject();
  if (!properties->empty()) {
    ret->o_properties.asArray() = properties;
  }
  return ret;
}

Array ObjectData::o_toArray() const {
  Array ret(ArrayData::Create());
  o_getArray(ret, false);
  return ret;
}

Array ObjectData::o_toIterArray(CStrRef context,
                                bool getRef /* = false */) {
  size_t size = m_cls->m_declPropNumAccessible +
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
    const PreClass::Prop* props = klass->m_preClass->properties();
    const size_t numProps = klass->m_preClass->numProperties();

    for (size_t i = 0; i < numProps; ++i) {
      auto key = const_cast<StringData*>(props[i].name());
      bool visible, accessible, unset;
      TypedValue* val = ((Instance*)this)->getProp(
        ctx, key, visible, accessible, unset);
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
    klass = klass->m_parent.get();
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
  // TODO This duplicates some logic from vm_decode_function and
  // vm_call_user_func, we should refactor this in the near future
  ctx.this_ = obj;
  ctx.cls = obj->getVMClass();
  ctx.invName = nullptr;

  // XXX The lookup below doesn't take context into account, so it will lead
  // to incorrect behavior in some corner cases. o_invoke is gradually being
  // removed from the HPHP runtime this should be ok for the short term.
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
    case 10: tvDup(*a9.asTypedValue(), args[9]);
    case  9: tvDup(*a8.asTypedValue(), args[8]);
    case  8: tvDup(*a7.asTypedValue(), args[7]);
    case  7: tvDup(*a6.asTypedValue(), args[6]);
#endif
#if INVOKE_FEW_ARGS_COUNT > 3
    case  6: tvDup(*a5.asTypedValue(), args[5]);
    case  5: tvDup(*a4.asTypedValue(), args[4]);
    case  4: tvDup(*a3.asTypedValue(), args[3]);
#endif
    case  3: tvDup(*a2.asTypedValue(), args[2]);
    case  2: tvDup(*a1.asTypedValue(), args[1]);
    case  1: tvDup(*a0.asTypedValue(), args[0]);
    case  0: break;
  }

  Variant ret;
  g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, count, args);
  return ret;
}

bool ObjectData::php_sleep(Variant &ret) {
  setAttribute(HasSleep);
  ret = t___sleep();
  return getAttribute(HasSleep);
}

StaticString s_zero("\0", 1);

void ObjectData::serialize(VariableSerializer *serializer) const {
  if (UNLIKELY(serializer->incNestedLevel((void*)this, true))) {
    serializer->writeOverflow((void*)this, true);
  } else {
    serializeImpl(serializer);
  }
  serializer->decNestedLevel((void*)this);
}

static StaticString s_PHP_Incomplete_Class("__PHP_Incomplete_Class");
static StaticString s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name");

void ObjectData::serializeImpl(VariableSerializer *serializer) const {
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
    handleSleep = const_cast<ObjectData*>(this)->php_sleep(ret);
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
    try {
      handleSleep = const_cast<ObjectData*>(this)->php_sleep(ret);
    } catch (...) {
      raise_warning("%s::sleep() throws exception", o_getClassName().data());
      ret = uninit_null();
      handleSleep = true;
    }
  }
  if (UNLIKELY(handleSleep)) {
    assert(!isCollection());
    if (ret.isArray()) {
      auto thiz = (Instance*)(this);
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
            if (m_cls->m_declProperties[propInd].m_attrs & AttrPrivate) {
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
      if (instanceof(c_Closure::s_cls)) {
        if (serializer->getType() == VariableSerializer::Type::APCSerialize) {
          p_DummyClosure dummy(NEWOBJ(c_DummyClosure));
          serializer->write(dummy);
        } else if (serializer->getType() ==
                   VariableSerializer::Type::DebuggerSerialize) {
          serializer->write("Closure");
        } else {
          throw_fatal("Serialization of Closure is not allowed");
        }
      } else if (instanceof(c_Continuation::s_cls)) {
        if (serializer->getType() == VariableSerializer::Type::APCSerialize) {
          p_DummyContinuation dummy(NEWOBJ(c_DummyContinuation));
          serializer->write(dummy);
        } else if (serializer->getType() ==
                   VariableSerializer::Type::DebuggerSerialize) {
          serializer->write("Continuation");
        } else {
          throw_fatal("Serialization of Continuation is not allowed");
        }
      } else {
        raise_warning("serialize(): __sleep should return an array only "
                      "containing the names of instance-variables to "
                      "serialize");
        uninit_null().serialize(serializer);
      }
    }
  } else {
    if (isCollection()) {
      collectionSerialize(const_cast<ObjectData*>(this), serializer);
    } else {
      CStrRef className = o_getClassName();
      Array properties = o_toArray();
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

bool ObjectData::hasInternalReference(PointerSet &vars,
                                      bool ds /* = false */) const {
  if (isCollection()) {
    return true;
  }
  return o_toArray().get()->hasInternalReference(vars, ds);
}

void ObjectData::dump() const {
  o_toArray().dump();
}

ObjectData *ObjectData::clone() {
  HPHP::Instance* instance = static_cast<HPHP::Instance*>(this);
  return instance->cloneImpl();
}

///////////////////////////////////////////////////////////////////////////////
// magic methods that user classes can override, and these are default handlers
// or actions to take:

Variant ObjectData::t___destruct() {
  // do nothing
  return uninit_null();
}

Variant ObjectData::t___call(Variant v_name, Variant v_arguments) {
  // do nothing
  return uninit_null();
}

Variant ObjectData::t___set(Variant v_name, Variant v_value) {
  // not called
  return uninit_null();
}

Variant ObjectData::t___get(Variant v_name) {
  // not called
  return uninit_null();
}

Variant ObjectData::offsetGet(Variant key) {
  assert(instanceof(SystemLib::s_ArrayAccessClass));
  const Func* method = m_cls->lookupMethod(s_offsetGet.get());
  assert(method);
  if (!method) {
    return uninit_null();
  }
  Variant v;
  TypedValue args[1];
  tvDup(*key.asTypedValue(), args[0]);
  g_vmContext->invokeFuncFew(v.asTypedValue(), method,
                             this, nullptr, 1, args);
  return v;
}

bool ObjectData::t___isset(Variant v_name) {
  return false;
}

Variant ObjectData::t___unset(Variant v_name) {
  // not called
  return uninit_null();
}

Variant ObjectData::t___sleep() {
  clearAttribute(HasSleep);
  return uninit_null();
}

Variant ObjectData::t___wakeup() {
  // do nothing
  return uninit_null();
}

String ObjectData::t___tostring() {
  string msg = o_getClassName().data();
  msg += "::__toString() was not defined";
  throw BadTypeConversionException(msg.c_str());
}

Variant ObjectData::t___clone() {
  // do nothing
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template<int Idx>
struct FindIndex {
  static int run(int size) {
    if (size <= ObjectSizeTable<Idx>::value) {
      return Idx;
    }
    return FindIndex<Idx + 1>::run(size);
  }
};

template<>
struct FindIndex<NumObjectSizeClasses> {
  static int run(int) {
    return -1;
  }
};

template<int Idx>
struct FindSize {
  static int run(int idx) {
    if (idx == Idx) {
      return ObjectSizeTable<Idx>::value;
    }
    return FindSize<Idx + 1>::run(idx);
  }
};

template<>
struct FindSize<NumObjectSizeClasses> {
  static int run(int) {
    not_reached();
  }
};

}

int object_alloc_size_to_index(size_t size) {
  return FindIndex<0>::run(size);
}

// This returns the maximum size for the size class
size_t object_alloc_index_to_size(int idx) {
  return FindSize<0>::run(idx);
}

///////////////////////////////////////////////////////////////////////////////
}
