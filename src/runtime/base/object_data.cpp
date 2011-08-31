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
#include <util/lock.h>
#include <runtime/base/class_info.h>
#include <runtime/base/fiber_reference_map.h>

#include <runtime/eval/ast/function_call_expression.h>

#include <system/lib/systemlib.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

// current maximum object identifier
IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(int, ObjectData::os_max_id);

int ObjectData::GetMaxId() {
  return *(ObjectData::os_max_id.getCheck());
}

static CallInfo s_ObjectData_call_handler((void*)ObjectData::callHandler,
    (void*)ObjectData::callHandlerFewArgs, 0,
    CallInfo::VarArgs | CallInfo::Method | CallInfo::CallMagicMethod, 0);

static StaticString s___callStatic("__callStatic");
static StaticString s_serialize("serialize");

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor
ObjectData::~ObjectData() {
  int &pmax = *os_max_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
}


static CallInfo s_ObjectData_null_constructor(
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

void ObjectData::release() {
  ASSERT(getCount() == 0);
  destruct();
  if (LIKELY(getCount() == 0)) {
    delete this;
  }
}

void ObjectData::destruct() {
  if (!inCtorDtor()) {
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
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  if (UNLIKELY(!osc)) {
    return o_getClassNameHook();
  }
  return *osc->cls;
}

const ClassPropTable *ObjectData::o_getClassPropTable() const {
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
  int64 hash = s->hash();

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
                LIKELY(!strcasecmp(info->name, s->data()))) {
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

bool ObjectData::o_instanceof(CStrRef s) const {
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

const Eval::MethodStatement* ObjectData::getMethodStatement(const char* name)
  const {
  return NULL;
}
const Eval::MethodStatement* ObjectData::getConstructorStatement() const {
  return NULL;
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
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  return ifa(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::i_dummy(MethodCallPackage &mcp, CArrRef params,
                            Variant (*i)(MethodCallPackage &mcp,
                                         CArrRef params),
                            ObjectData *(*coo)(ObjectData*)) {
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  return i(mcp, params);
}

Variant ObjectData::ifa_dummy(MethodCallPackage &mcp, int count,
                              INVOKE_FEW_ARGS_IMPL_ARGS,
                              Variant (*ifa)(MethodCallPackage &mcp, int count,
                                             INVOKE_FEW_ARGS_IMPL_ARGS),
                              ObjectData *(*coo)()) {
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  return ifa(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::i_dummy(MethodCallPackage &mcp, CArrRef params,
                            Variant (*i)(MethodCallPackage &mcp,
                                         CArrRef params),
                            ObjectData *(*coo)()) {
  Object obj(Object::CreateDummy(coo));
  mcp.obj = obj.get();
  return i(mcp, params);
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
  MethodCallPackage &info, int64 hash) const {
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
    for (const int *p = cpt->lazy_inits(); ; *p++) {
      if (*p < 0) {
        if (i++) break;
        continue;
      }
      const ClassPropTableEntry *ce = cpt->m_entries + *p;
      CVarRef init = cpt->getInitVal(ce);
      addr = globals + ce->offset;
      if (LIKELY(ce->type == KindOfVariant)) {
        *(Variant*)addr = init;
      } else {
        switch (ce->type) {
          case KindOfBoolean: *(bool*)addr = init;   break;
          case KindOfInt32:   *(int*)addr = init;    break;
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
  CStrRef propName, int64 hash, int flagsMask, int flagsVal,
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
    ClassPropTableEntry::Static|ClassPropTableEntry::Private,
    ClassPropTableEntry::Static, this);

  if (UNLIKELY(!prop)) {
    throw FatalErrorException(0, "unknown static property %s", s.c_str());
  }

  GlobalVariables *g = get_global_variables();
  char *addr = (char*)g + prop->offset;
  if (LIKELY(prop->type == KindOfVariant)) {
    return *(Variant*)addr;
  }

  return prop->getVariant(addr);
}

Variant &ObjectStaticCallbacks::os_lval(CStrRef s) const {
  const ClassPropTable *cpt;
  const ClassPropTableEntry *prop = PropertyFinder(
    &cpt, s, s->hash(),
    ClassPropTableEntry::Static|ClassPropTableEntry::Private,
    ClassPropTableEntry::Static, this);

  if (LIKELY(prop != 0) && LIKELY(prop->type == KindOfVariant)) {
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
                                            int64 hash) {
  const char *globals = 0;
  CStrRef s = *mcp.name;
  if (hash < 0) hash = s->hash();
  bool found = false;

  if (UNLIKELY(!osc)) {
    if (mcp.obj && mcp.obj->o_get_call_info_hook(cls, mcp, hash)) {
      return true;
    }
  } else {
    do {
      if (!ex || found || !strcasecmp(cls, osc->cls->data())) {
        if (ex) found = true;
        if (const int *ix = osc->mcit_ix) {
          const MethodCallInfoTable *info = osc->mcit;

          int h = hash & ix[0];
          int o = ix[h + 1];
          if (o >= 0) {
            info += o;
            do {
              if (LIKELY(hash == info->hash) &&
                  LIKELY(info->len == s->size()) &&
                  LIKELY(!strcasecmp(info->name, s->data()))) {
                mcp.ci = info->ci;
                return true;
              }
              info++;
            } while (!(info->flags & 1));
          }
        }
      }
      if (!osc->parent) {
        if (LIKELY(!osc->redeclaredParent)) break;
        if (LIKELY(!globals)) {
          globals = (char*)get_global_variables();
        }
        osc = *(ObjectStaticCallbacks**)(globals + osc->redeclaredParent);
        if (mcp.obj) {
          mcp.obj = static_cast<DynamicObjectData*>(mcp.obj)->
            getRedeclaredParent();
        }
      } else {
        osc = osc->parent;
      }
    } while (osc);
  }

  if (mcp.obj) {
    mcp.ci = &s_ObjectData_call_handler;
    if (mcp.obj->hasCall() || mcp.obj->hasCallStatic()) {
      return true;
    }
  } else {
    ObjectData *obj = FrameInjection::GetThis();
    ASSERT(!mcp.isObj);
    StrNR cls = mcp.rootCls;
    bool ok = false;
    if (!obj || !obj->o_instanceof(cls)) {
      obj = create_object_only_no_init(cls);
      obj->setDummy();
      ok = obj->hasCallStatic();
      obj->release();
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

bool ObjectStaticCallbacks::GetCallInfo(const ObjectStaticCallbacks *osc,
                                        MethodCallPackage &mcp,
                                        int64 hash) {
  return GetCallInfoHelper(false, 0, osc, mcp, hash);
}

bool ObjectStaticCallbacks::GetCallInfoEx(const char *cls,
                                          const ObjectStaticCallbacks *osc,
                                          MethodCallPackage &mcp,
                                          int64 hash) {
  return GetCallInfoHelper(true, cls, osc, mcp, hash);
}

Variant ObjectData::os_invoke(CStrRef c, CStrRef s,
                              CArrRef params, int64 hash,
                              bool fatal /* = true */) {
  Object obj = FrameInjection::GetThis();
  if (!obj.instanceof(c)) {
    obj = create_object_only_no_init(c);
    obj->setDummy();
  }
  return obj->o_invoke_ex(c, s, params, fatal);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

static StaticString s_Iterator("Iterator");
static StaticString s_IteratorAggregate("IteratorAggregate");
static StaticString s_getIterator("getIterator");

ArrayIter ObjectData::begin(CStrRef context /* = null_string */) {
  if (o_instanceof(s_Iterator)) {
    return ArrayIter(this);
  }
  ObjectData *obj = this;
  while (obj->o_instanceof(s_IteratorAggregate)) {
    Variant iterator = obj->o_invoke(s_getIterator, Array());
    if (!iterator.isObject()) break;
    if (iterator.instanceof(s_Iterator)) {
      return ArrayIter(iterator.getObjectData(), false);
    }
    obj = iterator.getObjectData();
  }
  return ArrayIter(obj->o_toIterArray(context));
}

MutableArrayIter ObjectData::begin(Variant *key, Variant &val,
                                   CStrRef context /* = null_string */) {
  if (o_instanceof(s_Iterator)) {
    throw FatalErrorException("An iterator cannot be used with "
                              "foreach by reference");
  }
  ObjectData *obj = this;
  while (obj->o_instanceof(s_IteratorAggregate)) {
    Variant iterator = obj->o_invoke(s_getIterator, Array());
    if (!iterator.isObject()) break;
    if (iterator.instanceof(s_Iterator)) {
      throw FatalErrorException("An iterator cannot be used with "
                                "foreach by reference");
    }
    obj = iterator.getObjectData();
  }
  Array properties = obj->o_toIterArray(context, true);
  properties.escalate(true);
  ArrayData *arr = properties.getArrayData();
  if (arr->getCount() > 1) {
    properties = arr = arr->copy();
  }
  return MutableArrayIter(arr, key, val);
}

Variant *ObjectData::RealPropPublicHelper(
  CStrRef propName, int64 hash, int flags, const ObjectData *obj,
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
              if (!prop->isPrivate() &&
                  !prop->isOverride() &&
                  hash == prop->hash &&
                  LIKELY(!strcmp(prop->keyName->data(),
                                 propName->data()))) {
                const char *addr = ((const char *)obj) + prop->offset;
                if (LIKELY(prop->type == KindOfVariant)) {
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
    return const_cast<ObjectData*>(obj)->o_properties.lvalPtr(
      propName, flags & RealPropWrite, flags & RealPropCreate);
  }

  return NULL;
}

Variant *ObjectData::o_realProp(CStrRef propName, int flags,
                                CStrRef context /* = null_string */) const {
  const ObjectStaticCallbacks *orig = o_get_callbacks();
  if (UNLIKELY(!orig)) {
    return o_realPropHook(propName, flags, context);
  }

  const char *globals = 0;

  int64 hash = propName->hash();

  const StringData *ctx = context.get();
  if (!ctx) {
    ctx = FrameInjection::GetClassName(false).get();
  }
  if (ctx->size()) {
    const ObjectStaticCallbacks *osc = orig;
    const ObjectData *obj = this;
    int64 c_hash = ctx->hash();
    do {
      if (const int *ix = osc->instanceof_index) {
        const InstanceOfInfo *info = osc->instanceof_table;

        int h = c_hash & ix[0];
        int o = ix[h + 1];
        if (o >= 0) {
          info += o;
          do {
            if (c_hash == info->hash &&
                LIKELY(!strcasecmp(info->name, ctx->data()))) {
              osc = info->cb;
              if (UNLIKELY(int64(osc) & 1)) {
                if (LIKELY(!globals)) {
                  globals = (char*)get_global_variables();
                }
                osc = *(ObjectStaticCallbacks**)(globals - 1 + int64(osc));
              }
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
            const char *addr = ((const char *)obj) + prop->offset;
            if (LIKELY(prop->type == KindOfVariant)) {
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
  }

  do_public:
  return RealPropPublicHelper(propName, hash, flags, this, orig);
}

Variant *ObjectData::o_realPropPublic(CStrRef propName, int flags) const {
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
    return const_cast<ObjectData*>(this)->o_properties.lvalPtr(
      propName, flags & RealPropWrite, flags & RealPropCreate);
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
  if (propName.size() == 0) {
    return null;
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

Variant ObjectData::o_getPublic(CStrRef propName, bool error /* = true */) {
  if (propName.size() == 0) {
    return null;
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
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
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

Variant ObjectData::o_set(CStrRef propName, CVarRef v,
                          bool forInit /* = false */,
                          CStrRef context /* = null_string */) {
  return o_setImpl<CVarRef>(propName, v, forInit, context);
}

Variant ObjectData::o_setRef(CStrRef propName, CVarRef v,
                             bool forInit /* = false */,
                             CStrRef context /* = null_string */) {
  return o_setImpl<RefResult>(propName, ref(v), forInit, context);
}

Variant ObjectData::o_set(CStrRef propName, RefResult v,
                          bool forInit /* = false */,
                          CStrRef context /* = null_string */) {
  return o_setRef(propName, variant(v), forInit, context);
}

template<typename T>
inline ALWAYS_INLINE Variant ObjectData::o_setPublicImpl(CStrRef propName,
                                                         T v,
                                                         bool forInit) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
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

Variant ObjectData::o_setPublic(CStrRef propName, CVarRef v,
                                bool forInit /* = false */) {
  return o_setPublicImpl<CVarRef>(propName, v, forInit);
}

Variant ObjectData::o_setPublic(CStrRef propName, RefResult v,
                                bool forInit /* = false */) {
  return o_setPublicRef(propName, variant(v), forInit);
}

Variant ObjectData::o_setPublicRef(CStrRef propName, CVarRef v,
                                   bool forInit /* = false */) {
  return o_setPublicImpl<CVarStrongBind>(propName, strongBind(v), forInit);
}

Variant ObjectData::o_setPublicWithRef(CStrRef propName, CVarRef v,
                                       bool forInit /* = false */) {
  return o_setPublicImpl<CVarWithRefBind>(propName, withRefBind(v), forInit);
}

void ObjectData::o_setArray(CArrRef properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String key = iter.first().toString();
    if (key.empty() || key.charAt(0) != '\0') {
      // non-private property
      CVarRef secondRef = iter.secondRef();
      o_setPublicWithRef(key, secondRef, true);
    }
  }
}

void ObjectData::o_getArray(Array &props, bool pubOnly /* = false */) const {
  if (!o_properties.empty()) {
    for (ArrayIter it(o_properties); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      props.addLval(key, true).setWithRef(value);
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
    ret->o_properties = properties;
  }
  return ret;
}

CVarRef ObjectData::set(CStrRef s, CVarRef v) {
  o_set(s, v);
  return v;
}

Variant &ObjectData::o_lval(CStrRef propName, CVarRef tmpForGet,
                            CStrRef context /* = null_string */) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
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
  ClassInfo::GetArray(root, root->o_getClassPropTable(), ret, false);
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
  } else {
    // Resolve redeclared class info
    classInfo = classInfo->getCurrent();
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
    if (visible && o_propExists(prop->name, context)) {
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

Variant ObjectData::o_invoke(CStrRef s, CArrRef params, int64 hash /* = -1 */,
                             bool fatal /* = true */) {
  MethodCallPackage mcp;
  if (!fatal) mcp.noFatal();
  mcp.methodCall(this, s, hash);
  return (mcp.ci->getMeth())(mcp, params);
}

Variant ObjectData::o_root_invoke(CStrRef s, CArrRef params,
                                  int64 hash /* = -1 */,
                                  bool fatal /* = true */) {
  return getRoot()->o_invoke(s, params, hash, fatal);
}

Variant ObjectData::o_invoke_few_args(CStrRef s, int64 hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  MethodCallPackage mcp;
  mcp.methodCall(this, s, hash);
  return (mcp.ci->getMethFewArgs())(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  return getRoot()->o_invoke_few_args(s, hash, count,
                                      INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_invoke_ex(CStrRef clsname, CStrRef s,
                                CArrRef params, bool fatal /* = true */) {
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

bool ObjectData::o_get_call_info(MethodCallPackage &mcp,
                                 int64 hash /* = -1 */) {
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  mcp.obj = this;
  return ObjectStaticCallbacks::GetCallInfo(osc, mcp, hash);
}

bool ObjectData::o_get_call_info_ex(const char *clsname,
                                    MethodCallPackage &mcp,
                                    int64 hash /* = -1 */) {
  const ObjectStaticCallbacks *osc = o_get_callbacks();
  mcp.obj = this;
  return ObjectStaticCallbacks::GetCallInfoEx(clsname, osc, mcp, hash);
}

bool ObjectData::o_get_call_info_hook(const char *clsname,
                                      MethodCallPackage &mcp,
                                      int64 hash /* = -1 */) {
  return false;
}

Variant ObjectData::o_throw_fatal(const char *msg) {
  throw_fatal(msg);
  return null;
}

bool ObjectData::hasCall() {
  return getRoot()->getAttribute(HasCall);
}

bool ObjectData::hasCallStatic() {
  return getRoot()->getAttribute(HasCallStatic);
}

bool ObjectData::php_sleep(Variant &ret) {
  setAttribute(HasSleep);
  ret = t___sleep();
  return getAttribute(HasSleep);
}

StaticString s_zero("\0", 1);

void ObjectData::serialize(VariableSerializer *serializer) const {
  if (serializer->incNestedLevel((void*)this, true)) {
    serializer->writeOverflow((void*)this, true);
  } else if ((serializer->getType() == VariableSerializer::Serialize ||
              serializer->getType() == VariableSerializer::APCSerialize ||
              serializer->getType() == VariableSerializer::DebuggerSerialize) &&
             o_instanceof("Serializable")) {
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
  } else {
    Variant ret;
    if ((serializer->getType() == VariableSerializer::Serialize ||
         serializer->getType() == VariableSerializer::APCSerialize ||
         serializer->getType() == VariableSerializer::DebuggerSerialize) &&
        const_cast<ObjectData*>(this)->php_sleep(ret)) {
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
        serializer->setObjectInfo(o_getClassName(), o_getId());
        wanted.serialize(serializer, true);
      } else {
        if (o_instanceof("Closure")) {
          if (serializer->getType() == VariableSerializer::DebuggerSerialize) {
            serializer->write("Closure");
          } else {
            throw_fatal("Serialization of Closure is not allowed");
          }
        } else if (o_instanceof("Continuation")) {
          if (serializer->getType() == VariableSerializer::DebuggerSerialize) {
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
      serializer->setObjectInfo(o_getClassName(), o_getId());
      o_toArray().serialize(serializer, true);
    }
  }
  serializer->decNestedLevel((void*)this);
}

void ObjectData::dump() const {
  o_toArray().dump();
}

ObjectData *ObjectData::clone() {
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
            case KindOfInt32:
              *(int*)a2 = *(int*)a1;
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
            case KindOfVariant:
              ((Variant*)a2)->setWithRef(*(Variant*)a1);
              break;
            default:
              assert(false);
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
  return orig;
}

void ObjectData::cloneDynamic(ObjectData *orig) {
  o_properties = orig->o_properties;
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
  if (getAttribute(UseUnset)) {
    AttributeClearer a(UseUnset, this);
    t___unset(prop);
  } else {
    if (Variant *t = o_realProp(prop,
                                RealPropWrite|RealPropNoDynamic, context)) {
      unset(*t);
    } else if (o_properties.exists(prop, true)) {
      o_properties.weakRemove(prop, true);
    }
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
Variant &ObjectData::___offsetget_lval(Variant v_name) {
  return o_properties.lvalAt(v_name, AccessFlags::Key);
}
bool ObjectData::t___isset(Variant v_name) {
  return false;
}

Variant ObjectData::t___unset(Variant v_name) {
  // not called
  return null;
}

bool ObjectData::o_propExists(CStrRef s, CStrRef context /* = null_string */) {
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

Object ObjectData::fiberMarshal(FiberReferenceMap &refMap) const {
  ObjectData *px = (ObjectData*)refMap.lookup((void*)this);
  if (px == NULL) {
    Object copy = create_object(o_getClassName().fiberCopy(),
                                null_array, false);
    // ahead of deep copy
    refMap.insert(const_cast<ObjectData*>(this), copy.get());
    Array props;
    ClassInfo::GetArray(this, this->o_getClassPropTable(), props, false);
    if (!props.empty()) {
      ClassInfo::SetArray(copy.get(), copy->o_getClassPropTable(),
                          props.fiberMarshal(refMap));
    }
    FiberLocal *src = dynamic_cast<FiberLocal*>(const_cast<ObjectData*>(this));
    if (src) {
      FiberLocal *dest = dynamic_cast<FiberLocal*>(copy.get());
      ASSERT(dest);
      if (dest) {
        dest->fiberInit(src, refMap);
      }
    }
    return copy;
  }
  return px;
}

Object ObjectData::fiberUnmarshal(FiberReferenceMap &refMap) const {
  // marshaling back to original thread
  ObjectData *px = (ObjectData*)refMap.lookup((void*)this);
  Object copy;
  if (px == NULL) {
    // was i in original thread?
    px = (ObjectData*)refMap.reverseLookup((void*)this);
    if (px == NULL) {
      copy = create_object(o_getClassName().fiberCopy(), null_array, false);
      px = copy.get();
    }
    // ahead of deep copy
    refMap.insert(const_cast<ObjectData*>(this), px);
    Array props;
    ClassInfo::GetArray(this, this->o_getClassPropTable(), props, false);
    if (!props.empty()) {
      ClassInfo::SetArray(px, px->o_getClassPropTable(),
                          props.fiberMarshal(refMap));
    }
    FiberLocal *src = dynamic_cast<FiberLocal*>(const_cast<ObjectData*>(this));
    if (src) {
      FiberLocal *dest = dynamic_cast<FiberLocal*>(px);
      ASSERT(dest);
      if (dest) {
        dest->fiberExit(src, refMap);
      }
    }
  }
  return Object(px);
}

///////////////////////////////////////////////////////////////////////////////
}
