/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/base/runtime_error.h"
#include "hphp/util/lock.h"
#include "hphp/runtime/base/class_info.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_simplexml.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/member_operations.h"
#include "hphp/runtime/vm/object_allocator_sizes.h"
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

static StaticString s_offsetGet("offsetGet");
static StaticString s___call("__call");
static StaticString s___callStatic("__callStatic");
static StaticString s_serialize("serialize");

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
  Class* cls = Unit::lookupClass(s.get());
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
                                                   bool forInit,
                                                   CStrRef context) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  bool useSet = !forInit && getAttribute(UseSet);
  auto flags = useSet ? 0 : RealPropCreate;
  if (forInit) flags |= RealPropUnchecked;

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
    getProps(cls, pubOnly, cls->m_preClass.get(), props, inserted);
    for (auto const& trait : cls->m_usedTraits) {
      getProps(cls, pubOnly, trait->m_preClass.get(), props, inserted);
    }
    cls = cls->parent();
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

bool ObjectData::php_sleep(Variant& ret) {
  setAttribute(HasSleep);
  ret = t___sleep();
  return getAttribute(HasSleep);
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

static StaticString s_PHP_Incomplete_Class("__PHP_Incomplete_Class");
static StaticString s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name");
static StaticString s_PHP_Unserializable_Class_Name(
                      "__PHP_Unserializable_Class_Name");

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
    // Don't try to serialize a CPP extension class which doesn't
    // support serialization. Just send the class name instead.
    if ((builtinPropSize() > 0) && !getVMClass()->isCppSerializable()) {
      serializer->write(o_getClassName());
      return;
    }
    try {
      handleSleep = const_cast<ObjectData*>(this)->php_sleep(ret);
    } catch (...) {
      raise_warning("%s::sleep() throws exception", o_getClassName().data());
      serializer->writeNull();
      return;
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
  TypedValue args[1];
  tvDup(*key.asTypedValue(), args[0]);
  g_vmContext->invokeFuncFew(v.asTypedValue(), method,
                             this, nullptr, 1, args);
  return v;
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

static StaticString s___get(LITSTR_INIT("__get"));
static StaticString s___set(LITSTR_INIT("__set"));
static StaticString s___isset(LITSTR_INIT("__isset"));
static StaticString s___unset(LITSTR_INIT("__unset"));

TRACE_SET_MOD(runtime);

int ObjectData::ObjAllocatorSizeClassCount = InitializeAllocators();

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
  static StringData* sd_init = StringData::GetStaticString("__init__");
  const Func* init = m_cls->lookupMethod(sd_init);
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
ObjectData* ObjectData::newInstanceRaw(Class* cls, int idx) {
  ObjectData* obj = (ObjectData*)ALLOCOBJIDX(idx);
  new (obj) ObjectData(cls, NoInit::noinit);
  return obj;
}

void ObjectData::operator delete(void* p) {
  ObjectData* this_ = (ObjectData*)p;
  Class* cls = this_->getVMClass();
  size_t nProps = cls->numDeclProperties();
  // cppext classes have their own implementation of delete
  assert(this_->builtinPropSize() == 0);
  TypedValue* propVec = (TypedValue*)((uintptr_t)this_ + sizeof(ObjectData));
  for (unsigned i = 0; i < nProps; ++i) {
    TypedValue* prop = &propVec[i];
    tvRefcountedDecRef(prop);
  }
  DELETEOBJSZ(sizeForNProps(nProps))(this_);
}

void ObjectData::invokeUserMethod(TypedValue* retval, const Func* method,
                                  CArrRef params) {
  g_vmContext->invokeFunc(retval, method, params, this);
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

template <bool declOnly>
TypedValue* ObjectData::getPropImpl(Class* ctx, const StringData* key,
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

TypedValue* ObjectData::getProp(Class* ctx, const StringData* key,
                                bool& visible, bool& accessible, bool& unset) {
  return getPropImpl<false>(ctx, key, visible, accessible, unset);
}

TypedValue* ObjectData::getDeclProp(Class* ctx, const StringData* key,
                                    bool& visible, bool& accessible,
                                    bool& unset) {
  return getPropImpl<true>(ctx, key, visible, accessible, unset);
}

void ObjectData::invokeSet(TypedValue* retval, const StringData* key,
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

bool ObjectData::propEmpty(Class* ctx, const StringData* key) {
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

TypedValue* ObjectData::setProp(Class* ctx, const StringData* key,
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
    return nullptr;
  }
  assert(!accessible);
  assert(getAttribute(UseSet));
  TypedValue ignored;
  invokeSet(&ignored, key, val);
  tvRefcountedDecRef(&ignored);
  return nullptr;
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
    o_properties.get()->lval(*(const String*)&key,
                             *(Variant**)(&propVal), false);
    // don't write propVal->m_aux because it holds data
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

Variant ObjectData::t___sleep() {
  static StringData* sd__sleep = StringData::GetStaticString("__sleep");
  const Func* method = m_cls->lookupMethod(sd__sleep);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    clearAttribute(HasSleep);
    return uninit_null();
  }
}

Variant ObjectData::t___wakeup() {
  static StringData* sd__wakeup = StringData::GetStaticString("__wakeup");
  const Func* method = m_cls->lookupMethod(sd__wakeup);
  if (method) {
    TypedValue tv;
    g_vmContext->invokeFuncFew(&tv, method, this);
    return tvAsVariant(&tv);
  } else {
    return uninit_null();
  }
}

String ObjectData::t___tostring() {
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
                  m_cls->m_preClass->name()->data());
    }
    return tv.m_data.pstr;
  } else {
    std::string msg = m_cls->m_preClass->name()->data();
    msg += "::__toString() was not defined";
    throw BadTypeConversionException(msg.c_str());
  }
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


