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
#include <runtime/ext/ext_collection.h>

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
  if (!noDestruct()) {
    setNoDestruct();
    CountableHelper h(this);
    try {
      t___destruct();
    } catch (...) {
      handle_destructor_exception();
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

const ClassPropTable *ObjectData::o_getClassPropTable() const {
  return 0;
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

int64 ObjectData::o_toInt64() const {
  raise_notice("Object of class %s could not be converted to int",
               o_getClassName().data());
  return 1;
}

void ObjectData::bindThis(ThreadInfo *info) {
  FrameInjection::SetStaticClassName(info, getRoot()->o_getClassName());
}

void ObjectData::setDummy() {
  int *pmax = os_max_id.getNoCheck();
  if (o_id == *pmax) --(*pmax);
  o_id = 0; // for isset($this) to tell whether this is a fake obj
}

Variant ObjectData::ifa_dummy(MethodCallPackage &mcp, int count,
                              INVOKE_FEW_ARGS_IMPL_ARGS,
                              Variant (*ifa)(MethodCallPackage &mcp, int count,
                                             INVOKE_FEW_ARGS_IMPL_ARGS),
                              ObjectData *(*coo)(ObjectData*)) {
  assert(mcp.obj == nullptr);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = ifa(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
  mcp.obj = nullptr;
  return v;
}

Variant ObjectData::i_dummy(MethodCallPackage &mcp, CArrRef params,
                            Variant (*i)(MethodCallPackage &mcp,
                                         CArrRef params),
                            ObjectData *(*coo)(ObjectData*)) {
  assert(mcp.obj == nullptr);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = i(mcp, params);
  mcp.obj = nullptr;
  return v;
}

Variant ObjectData::ifa_dummy(MethodCallPackage &mcp, int count,
                              INVOKE_FEW_ARGS_IMPL_ARGS,
                              Variant (*ifa)(MethodCallPackage &mcp, int count,
                                             INVOKE_FEW_ARGS_IMPL_ARGS),
                              ObjectData *(*coo)()) {
  assert(mcp.obj == nullptr);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = ifa(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
  mcp.obj = nullptr;
  return v;
}

Variant ObjectData::i_dummy(MethodCallPackage &mcp, CArrRef params,
                            Variant (*i)(MethodCallPackage &mcp,
                                         CArrRef params),
                            ObjectData *(*coo)()) {
  assert(mcp.obj == nullptr);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = i(mcp, params);
  mcp.obj = nullptr;
  return v;
}

///////////////////////////////////////////////////////////////////////////////
// static methods and properties

ObjectData *coo_ObjectData(ObjectData *) {
  throw FatalErrorException("unknown class");
}

static void LazyInitializer(const ClassPropTable *cpt, const char *globals) {
  const char *addr = globals + cpt->m_lazy_init_offset;
  if (!*(bool*)addr) {
    *(bool*)addr = true;
    int i = 0;
    for (const int *p = cpt->lazy_inits(); ; p++) {
      if (*p < 0) {
        if (i++) break;
        continue;
      }
      const ClassPropTableEntry *ce = cpt->m_entries + *p;
      CVarRef init = cpt->getInitVal(ce);
      addr = globals + ce->offset;
      if (LIKELY(ce->type == KindOfUnknown)) {
        *(Variant*)addr = init;
      } else {
        switch (ce->type) {
          case KindOfBoolean: *(bool*)addr = init;   break;
          case KindOfInt64:   *(int64*)addr = init;  break;
          case KindOfDouble:  *(double*)addr = init; break;
          case KindOfString:  *(String*)addr = init; break;
          case KindOfArray:   *(Array*)addr = init;  break;
          case KindOfObject:  *(Object*)addr = init; break;
          default:            assert(false);          break;
        }
      }
    }
  }
}

inline ALWAYS_INLINE
const ClassPropTableEntry *PropertyFinder(
  const ClassPropTable **resTable,
  CStrRef propName, strhash_t hash, int flagsMask, int flagsVal,
  const ObjectStaticCallbacks *osc) {
  const char *globals = 0;

  if (const ClassPropTable *cpt = osc->cpt) {
    if (cpt->m_lazy_init_offset) {
      if (LIKELY(!globals)) {
        globals = (char*)get_global_variables();
      }
      LazyInitializer(cpt, globals);
    }
    do {
      if ((!(flagsMask & ClassPropTableEntry::Static) ||
           (flagsVal & ClassPropTableEntry::Static)) &&
          (!(flagsMask & ClassPropTableEntry::Constant) ||
           !(flagsVal & ClassPropTableEntry::Constant))) {
        if (cpt->m_static_size_mask >= 0) {
          const int *ix = cpt->m_hash_entries;
          int h = hash & cpt->m_static_size_mask;
          int o = ix[-h-1];
          if (o >= 0) {
            const ClassPropTableEntry *prop = cpt->m_entries + o;
            do {
              if ((prop->flags & flagsMask) == flagsVal &&
                  hash == prop->hash &&
                  LIKELY(!strcmp(prop->keyName->data() + prop->prop_offset,
                                 propName->data()))) {
                if (resTable) *resTable = cpt;
                return prop;
              }
            } while (!prop++->isLast());
          }
        }
      }
      if ((!(flagsMask & ClassPropTableEntry::Static) ||
           !(flagsVal & ClassPropTableEntry::Static)) &&
          (!(flagsMask & ClassPropTableEntry::Constant) ||
           !(flagsVal & ClassPropTableEntry::Constant))) {
        if (cpt->m_size_mask >= 0) {
          const int *ix = cpt->m_hash_entries;
          int h = hash & cpt->m_size_mask;
          int o = ix[h];
          if (o >= 0) {
            const ClassPropTableEntry *prop = cpt->m_entries + o;
            do {
              if ((prop->flags & flagsMask) == flagsVal &&
                  hash == prop->hash &&
                  LIKELY(!strcmp(prop->keyName->data() + prop->prop_offset,
                                 propName->data()))) {
                if (resTable) *resTable = cpt;
                return prop;
              }
            } while (!prop++->isLast());
          }
        }
      }
      if (!(flagsMask & ClassPropTableEntry::Constant) ||
          (flagsVal & ClassPropTableEntry::Constant)) {
        if (cpt->m_const_size_mask >= 0) {
          const int *ix = cpt->m_hash_entries;
          int h = hash & cpt->m_const_size_mask;
          int o = ix[-cpt->m_static_size_mask-h-2];
          if (o >= 0) {
            const ClassPropTableEntry *prop = cpt->m_entries + o;
            do {
              if ((prop->flags & flagsMask) == flagsVal &&
                  hash == prop->hash &&
                  LIKELY(!strcmp(prop->keyName->data() + prop->prop_offset,
                                 propName->data()))) {
                if (resTable) *resTable = cpt;
                return prop;
              }
            } while (!prop++->isLast());
          }
        }
      }
      cpt = cpt->m_parent;
    } while (cpt);
  }

  return 0;
}

Variant ObjectStaticCallbacks::os_getInit(CStrRef s) const {
  const ClassPropTable *cpt;
  const ClassPropTableEntry *prop = PropertyFinder(
    &cpt, s, s->hash(),
    ClassPropTableEntry::Constant, 0, this);
  if (UNLIKELY(!prop)) {
    throw FatalErrorException(0, "unknown property %s", s.c_str());
  }
  return cpt->getInitVal(prop);
}

Variant ObjectStaticCallbacks::os_get(CStrRef s) const {
  const ClassPropTable *cpt;
  const ClassPropTableEntry *prop = PropertyFinder(
    &cpt, s, s->hash(),
    ClassPropTableEntry::Static,
    ClassPropTableEntry::Static, this);

  if (UNLIKELY(!prop)) {
    throw FatalErrorException(0, "unknown static property %s", s.c_str());
  }

  SystemGlobals *g = get_global_variables();
  char *addr = (char*)g + prop->offset;
  if (LIKELY(prop->type == KindOfUnknown)) {
    return *(Variant*)addr;
  }

  return prop->getVariant(addr);
}

Variant &ObjectStaticCallbacks::os_lval(CStrRef s) const {
  const ClassPropTable *cpt;
  const ClassPropTableEntry *prop = PropertyFinder(
    &cpt, s, s->hash(),
    ClassPropTableEntry::Static,
    ClassPropTableEntry::Static, this);

  if (LIKELY(prop != 0) &&
      LIKELY(prop->type == KindOfUnknown)) {
    SystemGlobals *g = get_global_variables();
    char *addr = (char*)g + prop->offset;
    return *(Variant*)addr;
  }

  throw FatalErrorException(0, "unknown static property %s", s.c_str());
}

Variant ObjectStaticCallbacks::os_constant(const char *s) const {
  const ClassPropTable *cpt;
  const ClassPropTableEntry *prop = PropertyFinder(
    &cpt, s, hash_string_i(s),
    ClassPropTableEntry::Constant,
    ClassPropTableEntry::Constant, this);

  if (UNLIKELY(!prop)) {
    throw FatalErrorException(0, "unknown class constant %s::%s",
                              (*cls)->data(), s);
  }

  return cpt->getInitVal(prop);
}

GlobalVariables *ObjectStaticCallbacks::lazy_initializer(
  GlobalVariables *g) const {
  assert(cpt);
  assert(cpt->m_lazy_init_offset);
  LazyInitializer(cpt, (const char*)g);
  return g;
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

StaticString ssIterator("Iterator");
static StaticString s_IteratorAggregate("IteratorAggregate");
static StaticString s_getIterator("getIterator");

Object ObjectData::iterableObject(bool& isIterable,
                                  bool mayImplementIterator /* = true */) {
  assert(mayImplementIterator || !implementsIterator());
  if (mayImplementIterator && implementsIterator()) {
    isIterable = true;
    return Object(this);
  }
  Object obj(this);
  while (obj->o_instanceof(s_IteratorAggregate)) {
    Variant iterator = obj->o_invoke(s_getIterator, Array());
    if (!iterator.isObject()) break;
    if (iterator.instanceof(ssIterator)) {
      isIterable = true;
      return iterator.getObjectData();
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
    *res = ClassPropTableEntry::GetVariant(type, p);
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

  return null;
}

Variant ObjectData::o_get(CStrRef propName, bool error /* = true */,
                          CStrRef context /* = null_string */) {
  return o_getImpl(propName, 0, error, context);
}

HOT_FUNC_HPHP
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

  return null;
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

Variant ObjectData::o_setPublicWithRef(CStrRef propName, CVarRef v) {
  return o_setPublicImpl<CVarWithRefBind>(propName, withRefBind(v), false);
}

Variant ObjectData::o_i_set(CStrRef propName, CVarRef v) {
  return o_setPublicImpl<CVarRef>(propName, v, true);
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

Variant ObjectData::o_argval(bool byRef, CStrRef s,
    bool error /* = true */, CStrRef context /* = null_string */) {
  if (byRef) {
    return strongBind(o_lval(s, context));
  } else {
    return o_get(s, error, context);
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
  ObjectData *root = const_cast<ObjectData*>(this)->getRoot();
  ClassInfo::GetArray(root, root->o_getClassPropTable(), ret,
                      ClassInfo::GetArrayAll);
  return ret;
}

Array ObjectData::o_toIterArray(CStrRef context,
                                bool getRef /* = false */) {
  CStrRef object_class = o_getClassName();
  const ClassInfo *classInfo = ClassInfo::FindClass(object_class);
  const ClassInfo *contextClassInfo = nullptr;
  int category;

  if (!classInfo) {
    return Array::Create();
  }

  Array ret(Array::Create());

  // There are 3 cases:
  // (1) called from standalone function (this_object == null) or
  //  the object class is not related to the context class;
  // (2) the object class is a subclass of the context class or vice versa.
  // (3) the object class is the same as the context class
  // For (1), only public properties of the object are accessible.
  // For (2) and (3), the public/protected properties of the object are
  // accessible;
  // any property of the context class is also accessible unless it is
  // overriden by the object class. (3) is really just an optimization.
  if (context.empty()) { // null_string is also empty
    category = 1;
  } else {
    contextClassInfo = ClassInfo::FindClass(context);
    assert(contextClassInfo);
    if (object_class->isame(context.get())) {
      category = 3;
    } else if (classInfo->derivesFrom(context, false) ||
               contextClassInfo->derivesFrom(object_class, false)) {
      category = 2;
    } else {
      category = 1;
    }
  }

  ClassInfo::PropertyVec properties;
  classInfo->getAllProperties(properties);
  ClassInfo::PropertyMap contextProperties;
  if (category == 2) {
    contextClassInfo->getAllProperties(contextProperties);
  }
  Array dynamics = o_getDynamicProperties();
  for (unsigned int i = 0; i < properties.size(); i++) {
    ClassInfo::PropertyInfo *prop = properties[i];
    if (prop->attribute & ClassInfo::IsStatic) continue;

    bool visible = false;
    switch (category) {
    case 1:
      visible = (prop->attribute & ClassInfo::IsPublic);
      break;
    case 2:
      if ((prop->attribute & ClassInfo::IsPrivate) == 0) {
        visible = true;
      } else {
        ClassInfo::PropertyMap::const_iterator iterProp =
          contextProperties.find(prop->name);
        if (iterProp != contextProperties.end() &&
            iterProp->second->owner == contextClassInfo) {
          visible = true;
        } else {
          visible = false;
        }
      }
      break;
    case 3:
      if ((prop->attribute & ClassInfo::IsPrivate) == 0 ||
          prop->owner == classInfo) {
        visible = true;
      }
      break;
    default:
      assert(false);
    }
    if (visible && o_propForIteration(prop->name, context)) {
      if (getRef) {
        Variant tmp;
        Variant &ov = o_lval(prop->name, tmp, context);
        Variant &av = ret.lvalAt(prop->name, AccessFlags::Key);
        av.assignRef(ov);
      } else {
        ret.set(prop->name, o_getUnchecked(prop->name,
                                           prop->owner->getName()));
      }
    }
    dynamics.remove(prop->name);
  }
  if (!dynamics.empty()) {
    if (getRef) {
      for (ArrayIter iter(o_getDynamicProperties()); iter; ++iter) {
        // Object property names are always strings.
        String key = iter.first().toString();
        if (dynamics->exists(key)) {
          CVarRef value = iter.secondRef();
          Variant &av = ret.lvalAt(key, AccessFlags::Key);
          av.assignRef(value);
        }
      }
    } else {
      ret += dynamics;
    }
  }
  return ret;
}

Array ObjectData::o_getDynamicProperties() const {
  return o_properties;
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
      return null;
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

Variant ObjectData::o_root_invoke(CStrRef s, CArrRef params,
                                  strhash_t hash /* = -1 */,
                                  bool fatal /* = true */) {
  return getRoot()->o_invoke(s, params, hash, fatal);
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

Variant ObjectData::o_root_invoke_few_args(CStrRef s, strhash_t hash, int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  return getRoot()->o_invoke_few_args(s, hash, count,
                                      INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_invoke_ex(CStrRef clsname, CStrRef s,
                                CArrRef params, bool fatal /* = true */) {
  // TODO This duplicates some logic from vm_decode_function and
  // vm_call_user_func, we should refactor this in the near future
  ObjectData* this_ = this;
  HPHP::VM::Class* cls = VM::Unit::lookupClass(clsname.get());
  if (!cls || !getVMClass()->classof(cls)) {
    o_invoke_failed(clsname.data(), s.data(), fatal);
    return null;
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
      return null;
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

Variant ObjectData::o_throw_fatal(const char *msg) {
  throw_fatal(msg);
  return null;
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
    if (o_instanceof("Serializable")) {
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
    if (o_instanceof("Serializable")) {
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
      ret = null;
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
          wanted.set(name, null);
        }
      }
      serializer->setObjectInfo(o_getClassName(), o_getId(), 'O');
      wanted.serialize(serializer, true);
    } else {
      if (o_instanceof("Closure")) {
        if (serializer->getType() == VariableSerializer::APCSerialize) {
          p_DummyClosure dummy(NEWOBJ(c_DummyClosure));
          serializer->write(dummy);
        } else if (serializer->getType() ==
                   VariableSerializer::DebuggerSerialize) {
          serializer->write("Closure");
        } else {
          throw_fatal("Serialization of Closure is not allowed");
        }
      } else if (o_instanceof("Continuation")) {
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
        null.serialize(serializer);
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

void ObjectData::cloneDynamic(ObjectData *orig) {
  o_properties.asArray() = orig->o_properties;
}

ObjectData *ObjectData::getRoot() { return this; }

Variant ObjectData::o_getError(CStrRef prop, CStrRef context) {
  raise_notice("Undefined property: %s::$%s", o_getClassName().data(),
               prop.data());
  return null;
}

Variant ObjectData::o_setError(CStrRef prop, CStrRef context) {
  return null;
}

bool ObjectData::o_isset(CStrRef prop, CStrRef context) {
  if (Variant *t = o_realProp(prop, 0, context)) {
    if (t->isInitialized()) {
      return !t->isNull();
    }
  }
  if (getAttribute(UseIsset)) {
    AttributeClearer a(UseIsset, this);
    return t___isset(prop);
  }
  return false;
}

bool ObjectData::o_empty(CStrRef prop, CStrRef context) {
  if (Variant *t = o_realProp(prop, 0, context)) {
    if (t->isInitialized()) {
      return empty(*t);
    }
  }
  if (getAttribute(UseIsset)) {
    {
      AttributeClearer a(UseIsset, this);
      if (!t___isset(prop) || !getAttribute(UseGet)) {
        return true;
      }
    }
    AttributeClearer a(UseGet, this);
    return empty(t___get(prop));
  }
  return true;
}

void ObjectData::o_unset(CStrRef prop, CStrRef context) {
  if (Variant *t = o_realProp(prop,
                              RealPropWrite|RealPropNoDynamic, context)) {
    unset(*t);
  } else if (o_properties.asArray().exists(prop, true)) {
    o_properties.asArray().weakRemove(prop, true);
  } else if (UNLIKELY(!*prop.data())) {
    throw_invalid_property_name(prop);
  } else if (getAttribute(UseUnset)) {
    AttributeClearer a(UseUnset, this);
    t___unset(prop);
  }
}

///////////////////////////////////////////////////////////////////////////////
// magic methods that user classes can override, and these are default handlers
// or actions to take:

Variant ObjectData::t___destruct() {
  // do nothing
  return null;
}

Variant ObjectData::t___call(Variant v_name, Variant v_arguments) {
  // do nothing
  return null;
}

Variant ObjectData::t___set(Variant v_name, Variant v_value) {
  // not called
  return null;
}

Variant ObjectData::t___get(Variant v_name) {
  // not called
  return null;
}

Variant *ObjectData::___lval(Variant v_name) {
  return nullptr;
}

Variant& ObjectData::___offsetget_lval(Variant key) {
  if (isCollection()) {
    return collectionOffsetGet(this, key);
  } else {
    if (!o_instanceof("ArrayAccess")) {
      throw InvalidOperandException("not ArrayAccess objects");
    }
    Variant &v = get_system_globals()->__lvalProxy;
    v = o_invoke_few_args(s_offsetGet, -1, 1, key);
    return v;
  }
}

bool ObjectData::t___isset(Variant v_name) {
  return false;
}

Variant ObjectData::t___unset(Variant v_name) {
  // not called
  return null;
}

bool ObjectData::o_propExists(CStrRef s, CStrRef context /* = null_string */) {
  Variant *t = o_realProp(s, RealPropExist, context);
  return t;
}

bool ObjectData::o_propForIteration(CStrRef s,
                                    CStrRef context /* = null_string */) {
  Variant *t = o_realProp(s, 0, context);
  return t && t->isInitialized();
}

Variant ObjectData::t___sleep() {
  clearAttribute(HasSleep);
  return null;
}

Variant ObjectData::t___wakeup() {
  // do nothing
  return null;
}

String ObjectData::t___tostring() {
  string msg = o_getClassName().data();
  msg += "::__toString() was not defined";
  throw BadTypeConversionException(msg.c_str());
}

Variant ObjectData::t___clone() {
  // do nothing
  return null;
}

const CallInfo *ObjectData::t___invokeCallInfoHelper(void *&extra) {
  extra = nullptr;
  return nullptr;
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
