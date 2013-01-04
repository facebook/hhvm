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

static CallInfoWithConstructor s_ObjectData_call_handler(
  (void*)ObjectData::callHandler,
  (void*)ObjectData::callHandlerFewArgs, 0,
  CallInfo::VarArgs | CallInfo::Method | CallInfo::CallMagicMethod, 0);

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
  const_assert(hhvm);
  return m_cls->classof(pc);
}

bool ObjectData::instanceof(const HPHP::VM::Class* c) const {
  return m_cls->classof(c);
}

CallInfo *ObjectData::GetCallHandler() {
  return &s_ObjectData_call_handler;
}

static CallInfoWithConstructor s_ObjectData_null_constructor(
  (void*)ObjectData::NullConstructor,
  (void*)ObjectData::NullConstructorFewArgs, 0, CallInfo::Method, 0);

static inline ALWAYS_INLINE void GetConstructorHelper(ObjectData *obj,
                                                      MethodCallPackage &mcp) {
  const ObjectStaticCallbacks *osc = obj->o_get_callbacks();
  if (LIKELY(osc != 0)) {
    do {
      if (LIKELY(osc->constructor != 0)) {
        mcp.ci = osc->constructor;
        mcp.obj = obj;
        return;
      }
      if (LIKELY(!osc->redeclaredParent)) break;
      osc = *(ObjectStaticCallbacks**)((char*)get_global_variables() +
                                       osc->redeclaredParent);
      obj = static_cast<DynamicObjectData*>(obj)->getRedeclaredParent();
    } while (osc);
  } else if (ObjectData *parent = obj->getRedeclaredParent()) {
    parent->getConstructor(mcp);
    return;
  }
  mcp.ci = &s_ObjectData_null_constructor;
  mcp.obj = obj;
}

void ObjectData::getConstructor(MethodCallPackage &mcp) {
  GetConstructorHelper(this, mcp);
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
  if (hhvm) {
    if (isResource()) return o_getClassNameHook();
    return *(const String*)(&m_cls->m_preClass->nameRef());
  }
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  if (UNLIKELY(!osc)) {
    return o_getClassNameHook();
  }
  return *osc->cls;
}

CStrRef ObjectData::o_getParentName() const {
  if (hhvm) {
    if (isResource()) return empty_string;
    return *(const String*)(&m_cls->m_preClass->parentRef());
  }
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  if (UNLIKELY(!osc)) {
    return GetParentName(o_getClassNameHook());
  }
  return osc->parent ? *osc->parent->cls : empty_string;
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
  if (hhvm) {
    return 0;
  }
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  if (UNLIKELY(!osc)) {
    return 0;
  }
  return osc->cpt;
}

CStrRef ObjectData::o_getClassNameHook() const {
  throw FatalErrorException("Class didnt provide a name");
  return empty_string;
}

inline ALWAYS_INLINE bool InstanceOfHelper(CStrRef s,
                                           const ObjectData *obj,
                                           const ObjectStaticCallbacks *osc) {
  const char *globals = 0;
  strhash_t hash = s->hash();

  if (UNLIKELY(!osc)) {
    return obj->o_instanceof_hook(s);
  } else {
    do {
      if (const int *ix = osc->instanceof_index) {
        const InstanceOfInfo *info = osc->instanceof_table;

        int h = hash & ix[0];
        int o = ix[h + 1];
        if (o >= 0) {
          info += o;
          do {
            if (hash == info->hash &&
                LIKELY((info->name == s->data()) ||
                  !strcasecmp(info->name, s->data()))) {
              return true;
            }
          } while (!info++->flags);
        }
      }
      if (!osc->redeclaredParent) break;
      if (LIKELY(!globals)) {
        globals = (char*)get_global_variables();
      }
      osc = *(ObjectStaticCallbacks**)(globals + osc->redeclaredParent);
    } while (osc);
  }

  return false;
}

HOT_FUNC
bool ObjectData::o_instanceof(CStrRef s) const {
  if (hhvm) {
    HPHP::VM::Class* cls = VM::Unit::lookupClass(s.get());
    if (!cls) return false;
    return m_cls->classof(cls);
  }
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  return InstanceOfHelper(s, this, osc);
}

bool ObjectData::o_instanceof_hook(CStrRef s) const {
  return false;
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
  ASSERT(mcp.obj == NULL);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = ifa(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
  mcp.obj = NULL;
  return v;
}

Variant ObjectData::i_dummy(MethodCallPackage &mcp, CArrRef params,
                            Variant (*i)(MethodCallPackage &mcp,
                                         CArrRef params),
                            ObjectData *(*coo)(ObjectData*)) {
  ASSERT(mcp.obj == NULL);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = i(mcp, params);
  mcp.obj = NULL;
  return v;
}

Variant ObjectData::ifa_dummy(MethodCallPackage &mcp, int count,
                              INVOKE_FEW_ARGS_IMPL_ARGS,
                              Variant (*ifa)(MethodCallPackage &mcp, int count,
                                             INVOKE_FEW_ARGS_IMPL_ARGS),
                              ObjectData *(*coo)()) {
  ASSERT(mcp.obj == NULL);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = ifa(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
  mcp.obj = NULL;
  return v;
}

Variant ObjectData::i_dummy(MethodCallPackage &mcp, CArrRef params,
                            Variant (*i)(MethodCallPackage &mcp,
                                         CArrRef params),
                            ObjectData *(*coo)()) {
  ASSERT(mcp.obj == NULL);
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  Variant v = i(mcp, params);
  mcp.obj = NULL;
  return v;
}

///////////////////////////////////////////////////////////////////////////////
// static methods and properties

ObjectData *coo_ObjectData(ObjectData *) {
  throw FatalErrorException("unknown class");
}

inline void checkRedeclaredClass(const RedeclaredObjectStaticCallbacks *r) {
  if (UNLIKELY(r->id < 0)) {
    throw FatalErrorException(0, "unknown class %s", r->oscb.cls);
  }
}

Variant RedeclaredObjectStaticCallbacks::os_getInit(CStrRef s) const {
  return oscb.os_getInit(s);
}
Variant RedeclaredObjectStaticCallbacks::os_get(CStrRef s) const {
  return oscb.os_get(s);
}
Variant &RedeclaredObjectStaticCallbacks::os_lval(CStrRef s) const {
  return oscb.os_lval(s);
}
Variant RedeclaredObjectStaticCallbacks::os_constant(const char *s) const {
  return oscb.os_constant(s);
}
bool RedeclaredObjectStaticCallbacks::os_get_call_info(
  MethodCallPackage &info, strhash_t hash) const {
  return oscb.os_get_call_info(info, hash);
}
ObjectData *RedeclaredObjectStaticCallbacks::createOnlyNoInit(
  ObjectData* root) const {
  return oscb.createOnlyNoInit(root);
}
Object RedeclaredObjectStaticCallbacks::create(CArrRef params, bool init,
                                                      ObjectData* root) const {
  return oscb.create(params, init, root);
}
Object RedeclaredObjectStaticCallbacks::createOnly(ObjectData *root) const {
  return oscb.createOnly(root);
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
          default:            ASSERT(false);          break;
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
  do {
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
    if (LIKELY(!osc->redeclaredParent)) break;
    if (LIKELY(!globals)) {
      globals = (char*)get_global_variables();
    }
    osc = *(ObjectStaticCallbacks**)(globals + osc->redeclaredParent);
  } while (osc);

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

  GlobalVariables *g = get_global_variables();
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
    GlobalVariables *g = get_global_variables();
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
  ASSERT(cpt);
  ASSERT(cpt->m_lazy_init_offset);
  LazyInitializer(cpt, (const char*)g);
  return g;
}

Object ObjectStaticCallbacks::create(CArrRef params, bool init /* = true */,
                            ObjectData* root /* = NULL */) const {
  Object o(createOnlyNoInit(root));
  o.get()->init();
  if (init) {
    MethodCallPackage mcp;
    mcp.construct(o);
    if (mcp.ci) {
      (mcp.ci->getMeth())(mcp, params);
      o.get()->clearNoDestruct();
    }
  }
  return o;
}

Object ObjectStaticCallbacks::createOnly(ObjectData* root /* = NULL */) const {
  Object o(createOnlyNoInit(root));
  o.get()->init();
  return o;
}

inline ALWAYS_INLINE bool GetCallInfoHelper(bool ex, const char *cls,
                                            const ObjectStaticCallbacks *osc,
                                            MethodCallPackage &mcp,
                                            strhash_t hash) {
  const char *globals = 0;
  CStrRef s = *mcp.name;
  if (hash < 0) hash = s->hash();
  bool found = false;

  if (UNLIKELY(!osc)) {
    if (mcp.obj && mcp.obj->o_get_call_info_hook(cls, mcp, hash)) {
      return true;
    }
  } else {
    const ObjectStaticCallbacks *cur = osc;
    do {
      if (!ex || found || !strcasecmp(cls, cur->cls->data())) {
        if (ex) found = true;
        if (const int *ix = cur->mcit_ix) {
          const MethodCallInfoTable *info = cur->mcit;

          int h = hash & ix[0];
          int o = ix[h + 1];
          if (o >= 0) {
            info += o;
            do {
              if (info->name == s->data() ||
                    (LIKELY(hash == info->hash) &&
                     LIKELY(!strcasecmp(info->name, s->data())))) {
                mcp.ci = info->ci;
                return true;
              }
            } while (!(info++->flags & 1));
          }
        }
      }
      if (!cur->parent) {
        if (LIKELY(!cur->redeclaredParent)) break;
        if (LIKELY(!globals)) {
          globals = (char*)get_global_variables();
        }
        cur = *(ObjectStaticCallbacks**)(globals + cur->redeclaredParent);
        if (mcp.obj) {
          mcp.obj = static_cast<DynamicObjectData*>(mcp.obj)->
            getRedeclaredParent();
        }
      } else {
        cur = cur->parent;
      }
    } while (cur);
  }

  if (mcp.obj) {
    mcp.ci = &s_ObjectData_call_handler;
    if (mcp.obj->hasCall() || mcp.obj->hasCallStatic()) {
      return true;
    }
  } else {
    ObjectData *obj = FrameInjection::GetThis();
    ASSERT(!mcp.isObj);
    StrNR cls(mcp.rootCls);
    bool ok = false;
    if (!obj || !obj->o_instanceof(cls)) {
      if (LIKELY(osc != 0)) {
        ok = osc->checkAttribute(ObjectData::HasCallStatic);
      } else {
        const ClassInfo *info = ClassInfo::FindClassInterfaceOrTrait(cls);
        if (info) {
          ok = info->getAttribute() & ClassInfo::HasCallStatic;
        } else if (mcp.m_fatal) {
          throw_missing_class(cls.data());
        }
      }
      obj = 0;
    } else {
      ok = obj->hasCallStatic() || obj->hasCall();
    }
    if (ok) {
      mcp.obj = obj;
      mcp.ci = &s_ObjectData_call_handler;
      return true;
    }
  }

  mcp.fail();
  return false;
}

HOT_FUNC_HPHP
bool ObjectStaticCallbacks::GetCallInfo(const ObjectStaticCallbacks *osc,
                                        MethodCallPackage &mcp,
                                        strhash_t hash) {
  return GetCallInfoHelper(false, 0, osc, mcp, hash);
}

bool ObjectStaticCallbacks::GetCallInfoEx(const char *cls,
                                          const ObjectStaticCallbacks *osc,
                                          MethodCallPackage &mcp,
                                          strhash_t hash) {
  return GetCallInfoHelper(true, cls, osc, mcp, hash);
}

bool ObjectStaticCallbacks::checkAttribute(int attrs) const {
  if (attributes & attrs) return true;
  if (LIKELY(!redeclaredParent)) return false;

  const ObjectStaticCallbacks *osc =
    *(ObjectStaticCallbacks**)((char*)get_global_variables() +
                               redeclaredParent);
  return osc->checkAttribute(attrs);
}

Variant ObjectData::os_invoke(CStrRef c, CStrRef s,
                              CArrRef params, strhash_t hash,
                              bool fatal /* = true */) {
  Object obj = FrameInjection::GetThis();
  if (!obj.instanceof(c)) {
    ObjectData *o = create_object_only_no_init(c);
    if (UNLIKELY(!o)) throw_missing_class(c);
    o->setDummy();
    obj = o;
  }
  return obj->o_invoke_ex(c, s, params, fatal);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

StaticString ssIterator("Iterator");
static StaticString s_IteratorAggregate("IteratorAggregate");
static StaticString s_getIterator("getIterator");

Object ObjectData::iterableObject(bool& isIterable,
                                  bool mayImplementIterator /* = true */) {
  ASSERT(mayImplementIterator || !implementsIterator());
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

HOT_FUNC_HPHP
Variant *ObjectData::RealPropPublicHelper(
  CStrRef propName, strhash_t hash, int flags, const ObjectData *obj,
  const ObjectStaticCallbacks *osc) {
  const char *globals = 0;
  do {
    if (const ClassPropTable *cpt = osc->cpt) {
      do {
        if (cpt->m_size_mask >= 0) {
          const int *ix = cpt->m_hash_entries;
          int h = hash & cpt->m_size_mask;
          int o = ix[h];
          if (o >= 0) {
            const ClassPropTableEntry *prop = cpt->m_entries + o;
            do {
              if (hash == prop->hash &&
                  ((!prop->isPrivate() && !prop->isOverride()) ||
                   (flags & RealPropExist)) &&
                  LIKELY(!strcmp(prop->keyName->data() + prop->prop_offset,
                                 propName->data()))) {
                const char *addr = ((const char *)obj) + prop->offset;
                if (LIKELY(prop->type == KindOfUnknown) ||
                    (flags & RealPropExist)) {
                  return (Variant*)addr;
                }
                if (flags & (RealPropCreate|RealPropWrite)) break;
                if (LIKELY(!globals)) globals = (char*)get_global_variables();
                Variant *res = &((Globals*)globals)->__realPropProxy;
                *res = prop->getVariant(addr);
                return res;
              }
            } while (!prop++->isLast());
          }
        }
        cpt = cpt->m_parent;
      } while (cpt);
    }
    if (LIKELY(!osc->redeclaredParent)) break;
    if (LIKELY(!globals)) {
      globals = (char*)get_global_variables();
    }
    osc = *(ObjectStaticCallbacks**)(globals + osc->redeclaredParent);
    obj = obj->getRedeclaredParent();
  } while (osc);

  if (propName.size() > 0 &&
      !(flags & RealPropNoDynamic) &&
      (obj->o_properties.get() || (flags & RealPropCreate))) {
    Array& arr = const_cast<ObjectData*>(obj)->o_properties.asArray();
    return arr.lvalPtr(propName, flags & RealPropWrite,
      flags & RealPropCreate);
  }

  return NULL;
}

void ObjectData::initProperties(int nProp) {
  if (hhvm) {
    if (!o_properties.get()) ((HPHP::VM::Instance*)this)->initDynProps(nProp);
  } else {
    ASSERT(hhvm || (enable_hphp_array && RuntimeOption::UseHphpArray));
    if (!o_properties.get()) {
      o_properties.asArray() = NEW(HphpArray)(nProp); // addref
    }
  }
}

void *ObjectData::o_realPropTyped(CStrRef propName, int flags,
                                  CStrRef context, DataType *type) const {
  *type = KindOfUnknown;
  if (hhvm) {
    /*
     * Returns a pointer to a place for a property value. This should never
     * call the magic methods __get or __set. The flags argument describes the
     * behavior in cases where the named property is nonexistent or
     * inaccessible.
     */
    HPHP::VM::Class* ctx = NULL;
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
    if (ret == NULL) {
      // Property is not declared, and not dynamically created yet.
      if (flags & RealPropCreate) {
        ASSERT(!(flags & RealPropNoDynamic));
        if (o_properties.get() == NULL) {
          thiz->initDynProps();
        }
        o_properties.get()->lvalPtr(propName,
                                    *(Variant**)(&ret), false, true);
        return (Variant*)ret;
      } else {
        return NULL;
      }
    }

    // ret is non-NULL if we reach here
    ASSERT(visible);
    if ((accessible && !unset) ||
        (flags & (RealPropUnchecked|RealPropExist))) {
      return (Variant*)ret;
    } else {
      return NULL;
    }
  }

  const ObjectStaticCallbacks *orig = o_get_callbacks();
  if (UNLIKELY(!orig)) {
    return o_realPropHook(propName, flags, context);
  }

  const char *globals = 0;

  strhash_t hash = propName->hash();

  const StringData *sdctx = context.get();
  if (!sdctx) {
    if (hhvm) {
      HPHP::VM::ActRec* ar = g_vmContext->getFP();
      if (ar && ar->m_func->cls()) {
        sdctx = ar->m_func->cls()->name();
      } else {
        sdctx = empty_string.get();
      }
    } else {
      sdctx = FrameInjection::GetClassName(false).get();
    }
  }
  if (!(flags & RealPropExist) && sdctx->size()) {
    const ObjectStaticCallbacks *osc = orig;
    const ObjectData *obj = this;
    strhash_t c_hash = sdctx->hash();
    do {
      if (const int *ix = osc->instanceof_index) {
        const InstanceOfInfo *info = osc->instanceof_table;

        int h = c_hash & ix[0];
        int o = ix[h + 1];
        if (o >= 0) {
          info += o;
          do {
            if (c_hash == info->hash &&
                LIKELY(!strcasecmp(info->name, sdctx->data()))) {
              osc = info->cb;
              if (UNLIKELY(int64(osc) & 1)) {
                if (LIKELY(!globals)) {
                  globals = (char*)get_global_variables();
                }
                osc = *(ObjectStaticCallbacks**)(globals - 1 + int64(osc));
              }
              if (osc->parent && osc->parent->cpt == osc->cpt) goto do_public;
              goto found_private_class;
            }
          } while (!info++->flags);
        }
      }
      if (!osc->redeclaredParent) break;
      if (LIKELY(!globals)) {
        globals = (char*)get_global_variables();
      }
      osc = *(ObjectStaticCallbacks**)(globals + osc->redeclaredParent);
      obj = obj->getRedeclaredParent();
    } while (osc);
    goto do_public;

    found_private_class:
    const ClassPropTable *cpt = osc->cpt;
    if (cpt && *cpt->privates() >= 0) {
      const int *ix = cpt->m_hash_entries;
      int h = hash & cpt->m_size_mask;
      int o = ix[h];
      if (o >= 0) {
        const ClassPropTableEntry *prop = cpt->m_entries + o;
        do {
          if (prop->isPrivate() &&
              hash == prop->hash &&
              LIKELY(!strcmp(prop->keyName->data() + prop->prop_offset,
                             propName->data()))) {
            char *addr = ((char *)obj) + prop->offset;
            *type = DataType(prop->type);
            return addr;
          }
        } while (!prop++->isLast());
      }
    }
  }

  do_public:
  return RealPropPublicHelper(propName, hash, flags, this, orig);
}

Variant *ObjectData::o_realProp(CStrRef propName, int flags,
                                CStrRef context /* = null_string */) const {
  DataType type;
  if (void *p = o_realPropTyped(propName, flags, context, &type)) {
    if (LIKELY(type == KindOfUnknown)) return (Variant*)p;
    if (flags & (RealPropCreate|RealPropWrite)) return NULL;
    char *globals = (char*)get_global_variables();
    Variant *res = &((Globals*)globals)->__realPropProxy;
    *res = ClassPropTableEntry::GetVariant(type, p);
    return res;
  }

  return NULL;
}

Variant *ObjectData::o_realPropPublic(CStrRef propName, int flags) const {
  if (hhvm) {
    return o_realProp(propName, flags, empty_string);
  }
  const ObjectStaticCallbacks *orig = o_get_callbacks();
  if (UNLIKELY(!orig)) {
    return o_realPropHook(propName, flags, empty_string);
  }
  return RealPropPublicHelper(propName, propName->hash(), flags, this, orig);
}

Variant *ObjectData::o_realPropHook(CStrRef propName, int flags,
                                    CStrRef context /* = null_string */) const {
  if (propName.size() > 0 &&
      !(flags & RealPropNoDynamic) &&
      (o_properties.get() || (flags & RealPropCreate))) {
    Array& arr = const_cast<ObjectData*>(this)->o_properties.asArray();
    return arr.lvalPtr(propName, flags & RealPropWrite,
      flags & RealPropCreate);
  }
  return NULL;
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
  return NULL;
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
  const ClassInfo *contextClassInfo = NULL;
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
    ASSERT(contextClassInfo);
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
      ASSERT(false);
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
  if (hhvm) {
    // TODO This duplicates some logic from vm_decode_function and
    // vm_call_user_func, we should refactor this in the near future
    ObjectData* this_ = this;
    HPHP::VM::Class* cls = getVMClass();
    StringData* invName = NULL;
    // XXX The lookup below doesn't take context into account, so it will lead
    // to incorrect behavior in some corner cases. o_invoke is gradually being
    // removed from the HPHP runtime this should be ok for the short term.
    const HPHP::VM::Func* f = cls->lookupMethod(s.get());
    if (f && (f->attrs() & HPHP::VM::AttrStatic)) {
      // If we found a method and its static, null out this_
      this_ = NULL;
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
      ASSERT(!(f->attrs() & HPHP::VM::AttrStatic));
      invName = s.get();
      invName->incRefCount();
    }
    ASSERT(f);
    Variant ret;
    g_vmContext->invokeFunc((TypedValue*)&ret, f, params, this_, cls,
                            NULL, invName);
    return ret;
  }
  MethodCallPackage mcp;
  if (!fatal) mcp.noFatal();
  mcp.methodCall(this, s, hash);
  return (mcp.ci->getMeth())(mcp, params);
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
  if (hhvm) {
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
  MethodCallPackage mcp;
  mcp.methodCall(this, s, hash);
  return (mcp.ci->getMethFewArgs())(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_root_invoke_few_args(CStrRef s, strhash_t hash, int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  return getRoot()->o_invoke_few_args(s, hash, count,
                                      INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_invoke_ex(CStrRef clsname, CStrRef s,
                                CArrRef params, bool fatal /* = true */) {
  if (hhvm) {
    // TODO This duplicates some logic from vm_decode_function and
    // vm_call_user_func, we should refactor this in the near future
    ObjectData* this_ = this;
    HPHP::VM::Class* cls = VM::Unit::lookupClass(clsname.get());
    if (!cls || !getVMClass()->classof(cls)) {
      o_invoke_failed(clsname.data(), s.data(), fatal);
      return null;
    }
    StringData* invName = NULL;
    // XXX The lookup below doesn't take context into account, so it will lead
    // to incorrect behavior in some corner cases. o_invoke is gradually being
    // removed from the HPHP runtime this should be ok for the short term.
    const HPHP::VM::Func* f = cls->lookupMethod(s.get());
    if (f && (f->attrs() & HPHP::VM::AttrStatic)) {
      // If we found a method and its static, null out this_
      this_ = NULL;
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
      ASSERT(!(f->attrs() & HPHP::VM::AttrStatic));
      invName = s.get();
      invName->incRefCount();
    }
    ASSERT(f);
    Variant ret;
    g_vmContext->invokeFunc((TypedValue*)&ret, f, params, this_, cls,
                            NULL, invName);
    return ret;
  }
  MethodCallPackage mcp;
  if (!fatal) mcp.noFatal();
  String str(s);
  mcp.methodCallEx(this, str);
  if (o_get_call_info_ex(clsname, mcp)) {
    return (mcp.ci->getMeth())(mcp, params);
  } else {
    o_invoke_failed(clsname.data(), s.data(), fatal);
  }
  return null;
}

HOT_FUNC_HPHP
bool ObjectData::o_get_call_info(MethodCallPackage &mcp,
                                 strhash_t hash /* = -1 */) {
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  mcp.obj = this;
  return ObjectStaticCallbacks::GetCallInfo(osc, mcp, hash);
}

bool ObjectData::o_get_call_info_ex(const char *clsname,
                                    MethodCallPackage &mcp,
                                    strhash_t hash /* = -1 */) {
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  mcp.obj = this;
  return ObjectStaticCallbacks::GetCallInfoEx(clsname, osc, mcp, hash);
}

bool ObjectData::o_get_call_info_hook(const char *clsname,
                                      MethodCallPackage &mcp,
                                      strhash_t hash /* = -1 */) {
  return false;
}

Variant ObjectData::o_throw_fatal(const char *msg) {
  throw_fatal(msg);
  return null;
}

bool ObjectData::hasCall() {
  if (hhvm) {
    return m_cls->lookupMethod(s___call.get()) != NULL;
  }
  return getRoot()->getAttribute(HasCall);
}

bool ObjectData::hasCallStatic() {
  if (hhvm) {
    return m_cls->lookupMethod(s___callStatic.get()) != NULL;
  }
  return getRoot()->getAttribute(HasCallStatic);
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
      ASSERT(!isCollection());
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
      ASSERT(!isCollection());
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
    ASSERT(!isCollection());
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
  if (hhvm) {
    HPHP::VM::Instance* instance = static_cast<HPHP::VM::Instance*>(this);
    return instance->cloneImpl();
  }
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  if (UNLIKELY(!osc)) {
    raise_error("Cannot clone non-object");
    return 0;
  }

  ObjectData *clone = osc->createOnlyNoInit(NULL), *orig = clone;
  CountableHelper h(clone);
  clone->init();
  ObjectData *obj = this;

  while (true) {
    const ClassPropTable *ct = osc->cpt;
    while (ct) {
      const ClassPropTableEntry *p = ct->m_entries;
      int off = ct->m_offset;
      if (off >= 0) {
        do {
          p += off;
          if (UNLIKELY(p->isOverride())) continue;
          const char *a1 = (const char*)obj + p->offset;
          const char *a2 = (const char*)clone + p->offset;
          switch (p->type) {
            case KindOfBoolean:
              *(bool*)a2 = *(bool*)a1;
              break;
            case KindOfInt64:
              *(int64*)a2 = *(int64*)a1;
              break;
            case KindOfDouble:
              *(double*)a2 = *(double*)a1;
              break;
            case KindOfString:
              *(String*)a2 = *(String*)a1;
              break;
            case KindOfArray:
              *(Array*)a2 = *(Array*)a1;
              break;
            case KindOfObject:
              *(Object*)a2 = *(Object*)a1;
              break;
            case KindOfUnknown:
              ((Variant*)a2)->setWithRef(*(Variant*)a1);
              break;
            default:
              not_reached();
          }
        } while ((off = p->next) != 0);
      }
      ct = ct->m_parent;
    }
    if (LIKELY(!osc->redeclaredParent)) break;
    osc = *(ObjectStaticCallbacks**)((char*)get_global_variables() +
                                     osc->redeclaredParent);
    obj = obj->getRedeclaredParent();
    clone = clone->getRedeclaredParent();
  }

  clone->cloneDynamic(obj);
  return orig->clearNoDestruct();
}

void ObjectData::cloneDynamic(ObjectData *orig) {
  o_properties.asArray() = orig->o_properties;
}

ObjectData *ObjectData::getRoot() { return this; }

Variant ObjectData::doCall(Variant v_name, Variant v_arguments, bool fatal) {
  return o_invoke_failed(o_getClassName(), v_name.toString().data(), fatal);
}

Variant ObjectData::doRootCall(Variant v_name,
                               Variant v_arguments, bool fatal) {
  return doCall(v_name, v_arguments, fatal);
}

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
  return NULL;
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
  extra = NULL;
  return NULL;
}

Variant ObjectData::callHandler(MethodCallPackage &info, CArrRef params) {
  if (info.obj && info.obj->o_getId() && info.obj->hasCall()) {
    return info.obj->doRootCall(*info.name, params, true);
  }
  String clsname;
  if (!info.obj) {
    ASSERT(!info.isObj);
    clsname = info.rootCls;
  } else {
    clsname = info.obj->o_getClassName();
  }
  return invoke_static_method(clsname, s___callStatic,
                              CREATE_VECTOR2(*info.name, params), info.m_fatal);
}

Variant ObjectData::callHandlerFewArgs(MethodCallPackage &info, int count,
                                       INVOKE_FEW_ARGS_IMPL_ARGS) {
  return callHandler(info, collect_few_args(count, INVOKE_FEW_ARGS_PASS_ARGS));
}

Variant ObjectData::NullConstructor(MethodCallPackage &info, CArrRef params) {
  return null_variant;
}
Variant ObjectData::NullConstructorFewArgs(MethodCallPackage &info, int count,
      INVOKE_FEW_ARGS_IMPL_ARGS) {
  return null_variant;
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
