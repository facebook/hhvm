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

#include <runtime/base/complex_types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/externals.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/execution_context.h>
#include <util/lock.h>
#include <runtime/base/class_info.h>
#include <runtime/ext/ext_closure.h>
#include <runtime/ext/ext_continuation.h>
#include <runtime/ext/ext_collections.h>
#include <runtime/vm/class.h>

#include <system/lib/systemlib.h>

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

HPHP::VM::Class*
ObjectData::instanceof(const HPHP::VM::PreClass* pc) const {
  return m_cls->classof(pc);
}

bool ObjectData::instanceof(const HPHP::VM::Class* c) const {
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
      // We raise the refcount around the call to __destruct(). This is to
      // prevent the refcount from going to zero when the destructor returns.
      CountableHelper h(this);
      TypedValue retval;
      tvWriteNull(&retval);
      try {
        // Call the destructor method
        g_vmContext->invokeFunc(&retval, meth, null_array, this);
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

CStrRef ObjectData::GetParentName(CStrRef cls) {
  const ClassInfo *classInfo = ClassInfo::FindClass(cls);
  if (classInfo) {
    CStrRef parentClass = classInfo->getParentClass();
    if (!parentClass.isNull()) {
      return parentClass;
    }
  }
  return empty_string;
}

CStrRef ObjectData::o_getClassNameHook() const {
  throw FatalErrorException("Class didnt provide a name");
  return empty_string;
}

HOT_FUNC
bool ObjectData::o_instanceof(CStrRef s) const {
  HPHP::VM::Class* cls = VM::Unit::lookupClass(s.get());
  if (!cls) return false;
  return m_cls->classof(cls);
}

bool ObjectData::o_isClass(const char *s) const {
  return strcasecmp(s, o_getClassName()) == 0;
}

int64_t ObjectData::o_toInt64() const {
  raise_notice("Object of class %s could not be converted to int",
               o_getClassName().data());
  return 1;
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
    Variant iterator = obj->o_invoke(s_getIterator, Array());
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
  if (!o_properties.get()) ((HPHP::VM::Instance*)this)->initDynProps(nProp);
}

void *ObjectData::o_realPropTyped(CStrRef propName, int flags,
                                  CStrRef context, DataType *type) const {
  *type = KindOfUnknown;

  /*
   * Returns a pointer to a place for a property value. This should never
   * call the magic methods __get or __set. The flags argument describes the
   * behavior in cases where the named property is nonexistent or
   * inaccessible.
   */
  HPHP::VM::Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = VM::Unit::lookupClass(context.get());
  }

  HPHP::VM::Instance* thiz = (HPHP::VM::Instance*)(this);  // sigh
  bool visible, accessible, unset;
  TypedValue* ret = (flags & RealPropNoDynamic)
                    ? thiz->getDeclProp(ctx, propName.get(), visible,
                                        accessible, unset)
                    : thiz->getProp(ctx, propName.get(), visible,
                                    accessible, unset);
  if (ret == nullptr) {
    // Property is not declared, and not dynamically created yet.
    if (flags & RealPropCreate) {
      assert(!(flags & RealPropNoDynamic));
      if (o_properties.get() == nullptr) {
        thiz->initDynProps();
      }
      o_properties.get()->lvalPtr(propName,
                                  *(Variant**)(&ret), false, true);
      return (Variant*)ret;
    } else {
      return nullptr;
    }
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

Variant *ObjectData::o_realProp(CStrRef propName, int flags,
                                CStrRef context /* = null_string */) const {
  DataType type;
  if (void *p = o_realPropTyped(propName, flags, context, &type)) {
    if (LIKELY(type == KindOfUnknown)) return (Variant*)p;
    if (flags & (RealPropCreate|RealPropWrite)) return nullptr;
    SystemGlobals* globals = get_system_globals();
    Variant *res = &globals->__realPropProxy;
    *res = ClassInfo::GetVariant(type, p);
    return res;
  }

  return nullptr;
}

Variant *ObjectData::o_realPropPublic(CStrRef propName, int flags) const {
  return o_realProp(propName, flags, empty_string);
}

bool ObjectData::o_exists(CStrRef propName,
                          CStrRef context /* = null_string */) const {
  const Variant *t = o_realProp(propName, RealPropUnchecked, context);
  return t && t->isInitialized();
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
    return o_getError(propName, context);
  }

  return uninit_null();
}

Variant ObjectData::o_get(CStrRef propName, bool error /* = true */,
                          CStrRef context /* = null_string */) {
  return o_getImpl(propName, 0, error, context);
}

Variant ObjectData::o_getPublic(CStrRef propName, bool error /* = true */) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  if (Variant *t = o_realPropPublic(propName, 0)) {
    if (t->isInitialized())
      return *t;
  }

  if (getAttribute(UseGet)) {
    AttributeClearer a(UseGet, this);
    return t___get(propName);
  }

  if (error) {
    return o_getError(propName, null_string);
  }

  return uninit_null();
}

Variant ObjectData::o_getUnchecked(CStrRef propName,
                                   CStrRef context /* = null_string */) {
  return o_getImpl(propName, RealPropUnchecked, true, context);
}

template <class T>
inline ALWAYS_INLINE Variant ObjectData::o_setImpl(CStrRef propName, T v,
                                                   bool forInit,
                                                   CStrRef context) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  bool useSet = !forInit && getAttribute(UseSet);
  int flags = useSet ? RealPropWrite : RealPropCreate | RealPropWrite;
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

  o_setError(propName, context);
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

template<typename T>
inline ALWAYS_INLINE Variant ObjectData::o_setPublicImpl(CStrRef propName,
                                                         T v,
                                                         bool forInit) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  bool useSet = !forInit && getAttribute(UseSet);
  int flags = useSet ? RealPropWrite : RealPropCreate | RealPropWrite;
  if (forInit) flags |= RealPropUnchecked;

  if (Variant *t = o_realPropPublic(propName, flags)) {
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

  o_setError(propName, null_string);
  return variant(v);
}

Variant ObjectData::o_setPublic(CStrRef propName, CVarRef v) {
  return o_setPublicImpl<CVarRef>(propName, v, false);
}

Variant ObjectData::o_setPublic(CStrRef propName, RefResult v) {
  return o_setPublicRef(propName, variant(v));
}

Variant ObjectData::o_setPublicRef(CStrRef propName, CVarRef v) {
  return o_setPublicImpl<CVarStrongBind>(propName, strongBind(v), false);
}

Variant ObjectData::o_i_setPublicWithRef(CStrRef propName, CVarRef v) {
  return o_setPublicImpl<CVarWithRefBind>(propName, withRefBind(v), true);
}

HOT_FUNC
void ObjectData::o_setArray(CArrRef properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String key = iter.first().toString();
    if (key.empty() || key.charAt(0) != '\0') {
      // non-private property
      CVarRef secondRef = iter.secondRef();
      o_i_setPublicWithRef(key, secondRef);
    }
  }
}

void ObjectData::o_getArray(Array &props, bool pubOnly /* = false */) const {
  const Array& arr = o_properties.asArray();
  if (!arr.empty()) {
    for (ArrayIter it(arr); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      props.lvalAt(key, AccessFlags::Key).setWithRef(value);
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

CVarRef ObjectData::set(CStrRef s, CVarRef v) {
  o_set(s, v);
  return v;
}

Variant &ObjectData::o_lval(CStrRef propName, CVarRef tmpForGet,
                            CStrRef context /* = null_string */) {
  if (UNLIKELY(!*propName.data())) {
    throw_invalid_property_name(propName);
  }

  bool useGet = getAttribute(UseGet);
  int flags = useGet ? RealPropWrite : RealPropCreate | RealPropWrite;
  if (Variant *t = o_realProp(propName, flags, context)) {
    if (!useGet || t->isInitialized()) {
      return *t;
    }
  }

  Variant &ret = const_cast<Variant&>(tmpForGet);
  if (LIKELY(useGet)) {
    AttributeClearer a(UseGet, this);
    if (getAttribute(HasLval)) {
      return *___lval(propName);
    }

    ret = t___get(propName);
    return ret;
  }

  /* we only get here if its a protected property
     under hphpi - and then o_getError fatals
     with a suitable message
  */
  ret = o_getError(propName, context);
  return ret;
}

Variant *ObjectData::o_weakLval(CStrRef propName,
                                CStrRef context /* = null_string */) {
  if (Variant *t = o_realProp(propName, RealPropWrite|RealPropUnchecked,
                              context)) {
    if (t->isInitialized()) {
      return t;
    }
  }
  return nullptr;
}

Array ObjectData::o_toArray() const {
  Array ret(ArrayData::Create());
  ClassInfo::GetArray(this, ret, ClassInfo::GetArrayAll);
  return ret;
}

Array ObjectData::o_toIterArray(CStrRef context,
                                bool getRef /* = false */) {
  size_t size = m_cls->m_declPropNumAccessible +
                (o_properties.get() ? o_properties.get()->size() : 0);
  HphpArray* retval = NEW(HphpArray)(size);
  VM::Class* ctx = nullptr;
  if (!context.empty()) {
    ctx = VM::Unit::lookupClass(context.get());
  }

  // Get all declared properties first, bottom-to-top in the inheritance
  // hierarchy, in declaration order.
  const VM::Class* klass = m_cls;
  while (klass != nullptr) {
    const VM::PreClass::Prop* props = klass->m_preClass->properties();
    const size_t numProps = klass->m_preClass->numProperties();

    for (size_t i = 0; i < numProps; ++i) {
      auto key = const_cast<StringData*>(props[i].name());
      bool visible, accessible, unset;
      TypedValue* val = ((VM::Instance*)this)->getProp(
        ctx, key, visible, accessible, unset);
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

Variant ObjectData::o_invoke(CStrRef s, CArrRef params,
                             strhash_t hash /* = -1 */,
                             bool fatal /* = true */) {
  // TODO This duplicates some logic from vm_decode_function and
  // vm_call_user_func, we should refactor this in the near future
  ObjectData* this_ = this;
  HPHP::VM::Class* cls = getVMClass();
  StringData* invName = nullptr;
  // XXX The lookup below doesn't take context into account, so it will lead
  // to incorrect behavior in some corner cases. o_invoke is gradually being
  // removed from the HPHP runtime this should be ok for the short term.
  const HPHP::VM::Func* f = cls->lookupMethod(s.get());
  if (f && (f->attrs() & HPHP::VM::AttrStatic)) {
    // If we found a method and its static, null out this_
    this_ = nullptr;
  } else if (!f) {
    if (this_) {
      // If this_ is non-null AND we could not find a method, try
      // looking up __call in cls's method table
      f = cls->lookupMethod(s___call.get());
    }
    if (!f) {
      // Bail if we couldn't find the method or __call
      o_invoke_failed(o_getClassName().data(), s.data(), fatal);
      return uninit_null();
    }
    // We found __call! Stash the original name into invName.
    assert(!(f->attrs() & HPHP::VM::AttrStatic));
    invName = s.get();
    invName->incRefCount();
  }
  assert(f);
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, this_, cls,
                          nullptr, invName);
  return ret;
}

#define APPEND_1_ARGS(params) params.append(a0);
#define APPEND_2_ARGS(params) APPEND_1_ARGS(params); params.append(a1)
#define APPEND_3_ARGS(params) APPEND_2_ARGS(params); params.append(a2)
#define APPEND_4_ARGS(params) APPEND_3_ARGS(params); params.append(a3)
#define APPEND_5_ARGS(params) APPEND_4_ARGS(params); params.append(a4)
#define APPEND_6_ARGS(params) APPEND_5_ARGS(params); params.append(a5)
#define APPEND_7_ARGS(params) APPEND_6_ARGS(params); params.append(a6)
#define APPEND_8_ARGS(params) APPEND_7_ARGS(params); params.append(a7)
#define APPEND_9_ARGS(params) APPEND_8_ARGS(params); params.append(a8)
#define APPEND_10_ARGS(params)  APPEND_9_ARGS(params); params.append(a9)

Variant ObjectData::o_invoke_few_args(CStrRef s, strhash_t hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  Array params = Array::Create();
  switch(count) {
    case 1: APPEND_1_ARGS(params);
            break;
    case 2: APPEND_2_ARGS(params);
            break;
    case 3: APPEND_3_ARGS(params);
            break;
#if INVOKE_FEW_ARGS_COUNT > 3
    case 4: APPEND_4_ARGS(params);
            break;
    case 5: APPEND_5_ARGS(params);
            break;
    case 6: APPEND_6_ARGS(params);
            break;
#if INVOKE_FEW_ARGS_COUNT > 6
    case 7: APPEND_7_ARGS(params);
            break;
    case 8: APPEND_8_ARGS(params);
            break;
    case 9: APPEND_9_ARGS(params);
            break;
    case 10: APPEND_10_ARGS(params);
            break;
#endif
#endif
    default: not_implemented();
  }
  return o_invoke(s, params, hash);
}

Variant ObjectData::o_invoke_ex(CStrRef clsname, CStrRef s,
                                CArrRef params, bool fatal /* = true */) {
  // TODO This duplicates some logic from vm_decode_function and
  // vm_call_user_func, we should refactor this in the near future
  ObjectData* this_ = this;
  HPHP::VM::Class* cls = VM::Unit::lookupClass(clsname.get());
  if (!cls || !getVMClass()->classof(cls)) {
    o_invoke_failed(clsname.data(), s.data(), fatal);
    return uninit_null();
  }
  StringData* invName = nullptr;
  // XXX The lookup below doesn't take context into account, so it will lead
  // to incorrect behavior in some corner cases. o_invoke is gradually being
  // removed from the HPHP runtime this should be ok for the short term.
  const HPHP::VM::Func* f = cls->lookupMethod(s.get());
  if (f && (f->attrs() & HPHP::VM::AttrStatic)) {
    // If we found a method and its static, null out this_
    this_ = nullptr;
  } else if (!f) {
    if (this_) {
      // If this_ is non-null AND we could not find a method, try
      // looking up __call in cls's method table
      f = cls->lookupMethod(s___call.get());
    }
    if (!f) {
      // Bail if we couldn't find the method or __call
      o_invoke_failed(clsname.data(), s.data(), fatal);
      return uninit_null();
    }
    // We found __call! Stash the original name into invName.
    assert(!(f->attrs() & HPHP::VM::AttrStatic));
    invName = s.get();
    invName->incRefCount();
  }
  assert(f);
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, this_, cls,
                          nullptr, invName);
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

void ObjectData::serializeImpl(VariableSerializer *serializer) const {
  bool handleSleep = false;
  Variant ret;
  if (LIKELY(serializer->getType() == VariableSerializer::Serialize ||
             serializer->getType() == VariableSerializer::APCSerialize)) {
    if (instanceof(SystemLib::s_SerializableClass)) {
      assert(!isCollection());
      Variant ret =
        const_cast<ObjectData*>(this)->o_invoke(s_serialize, Array(), -1);
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
                      VariableSerializer::DebuggerSerialize)) {
    if (instanceof(SystemLib::s_SerializableClass)) {
      assert(!isCollection());
      try {
        Variant ret =
          const_cast<ObjectData*>(this)->o_invoke(s_serialize, Array(), -1);
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
      const ClassInfo *cls = ClassInfo::FindClass(o_getClassName());
      Array wanted = Array::Create();
      Array props = ret.toArray();
      for (ArrayIter iter(props); iter; ++iter) {
        String name = iter.second().toString();
        if (o_exists(name, o_getClassName())) {
          ClassInfo::PropertyInfo *p = cls->getPropertyInfo(name);
          String propName = name;
          if (p && (p->attribute & ClassInfo::IsPrivate)) {
            propName = concat4(s_zero, o_getClassName(), s_zero, name);
          }
          wanted.set(propName, const_cast<ObjectData*>(this)->
              o_getUnchecked(name, o_getClassName()));
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
        if (serializer->getType() == VariableSerializer::APCSerialize) {
          p_DummyClosure dummy(NEWOBJ(c_DummyClosure));
          serializer->write(dummy);
        } else if (serializer->getType() ==
                   VariableSerializer::DebuggerSerialize) {
          serializer->write("Closure");
        } else {
          throw_fatal("Serialization of Closure is not allowed");
        }
      } else if (instanceof(c_Continuation::s_cls)) {
        if (serializer->getType() == VariableSerializer::APCSerialize) {
          p_DummyContinuation dummy(NEWOBJ(c_DummyContinuation));
          serializer->write(dummy);
        } else if (serializer->getType() ==
                   VariableSerializer::DebuggerSerialize) {
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
      serializer->setObjectInfo(o_getClassName(), o_getId(), 'O');
      o_toArray().serialize(serializer, true);
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
  HPHP::VM::Instance* instance = static_cast<HPHP::VM::Instance*>(this);
  return instance->cloneImpl();
}

Variant ObjectData::o_getError(CStrRef prop, CStrRef context) {
  raise_notice("Undefined property: %s::$%s", o_getClassName().data(),
               prop.data());
  return uninit_null();
}

Variant ObjectData::o_setError(CStrRef prop, CStrRef context) {
  return uninit_null();
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

Variant *ObjectData::___lval(Variant v_name) {
  return nullptr;
}

Variant ObjectData::offsetGet(Variant key) {
  assert(instanceof(SystemLib::s_ArrayAccessClass));
  const VM::Func* method = m_cls->lookupMethod(s_offsetGet.get());
  assert(method);
  if (!method) {
    return uninit_null();
  }
  Variant v;
  g_vmContext->invokeFunc((TypedValue*)(&v), method,
                          CREATE_VECTOR1(key), this);
  return v;
}

bool ObjectData::t___isset(Variant v_name) {
  return false;
}

Variant ObjectData::t___unset(Variant v_name) {
  // not called
  return uninit_null();
}

bool ObjectData::o_propExists(CStrRef s, CStrRef context /* = null_string */) {
  Variant *t = o_realProp(s, RealPropExist, context);
  return t;
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
