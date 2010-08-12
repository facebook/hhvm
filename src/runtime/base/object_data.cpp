/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/object_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/externals.h>
#include <runtime/base/variable_serializer.h>
#include <util/lock.h>
#include <runtime/base/class_info.h>
#include <runtime/base/fiber_reference_map.h>

#include <runtime/eval/ast/function_call_expression.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

// current maximum object identifier
static IMPLEMENT_THREAD_LOCAL(int, os_max_id);

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor

ObjectData::ObjectData(bool isResource /* = false */)
    : o_properties(NULL), o_attribute(0) {
  if (!isResource) {
    o_id = ++(*os_max_id.get());
  }
}

ObjectData::~ObjectData() {
  if (o_properties) {
    o_properties->release();
  }
  int *pmax = os_max_id.get();
  if (o_id && o_id == *pmax) {
    --(*pmax);
  }
}

void ObjectData::
dynConstructFromEval(Eval::VariableEnvironment &env,
                     const Eval::FunctionCallExpression *call) {}

///////////////////////////////////////////////////////////////////////////////
// class info

bool ObjectData::o_isClass(const char *s) const {
  return strcasecmp(s, o_getClassName()) == 0;
}

const Eval::MethodStatement *ObjectData::getMethodStatement(const char* name)
  const {
  return NULL;
}

void ObjectData::bindThis(ThreadInfo *info) {
  FrameInjection::SetCallingObject(info, this);
}

///////////////////////////////////////////////////////////////////////////////
// static methods and properties

Variant ObjectData::os_getInit(const char *s, int64 hash) {
  throw FatalErrorException("unknown property %s", s);
}

Variant ObjectData::os_get(const char *s, int64 hash) {
  throw FatalErrorException("unknown static property %s", s);
}

Variant &ObjectData::os_lval(const char *s, int64 hash) {
  throw FatalErrorException("unknown static property %s", s);
}

Variant ObjectData::os_invoke(const char *c, MethodIndex methodIndex,
                              const char *s, CArrRef params, int64 hash,
                              bool fatal /* = true */) {
  Object obj = FrameInjection::GetThis();
  if (obj.isNull() || !obj->o_instanceof(c)) {
    obj = create_object(c, Array::Create(), false);
    int *pmax = os_max_id.get();
    int &id = obj.get()->o_id;
    if (id == *pmax) --(*pmax);
    id = 0; // for isset($this) to tell whether this is a fake obj
  }
  return obj->o_invoke_ex(c, methodIndex, s, params, hash, fatal);
}

Variant ObjectData::os_invoke_mil(const char *c,
                                  const char *s, CArrRef params, int64 hash,
                                  bool fatal /* = true */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    // no __call for static calls, give up now
    if (methodIndex.isFail()) {
      return o_invoke_failed(c, s, fatal);
    }
  }
  // redispatch, not necessarily call the above method
  return os_invoke(c, methodIndex, s, params, hash, fatal);
}

Variant ObjectData::os_constant(const char *s) {
  ostringstream msg;
  msg << "unknown class constant " << s;
  throw FatalErrorException(msg.str().c_str());
}

// FMC need test
Variant
ObjectData::os_invoke_from_eval(const char *c, const char *s,
                                Eval::VariableEnvironment &env,
                                const Eval::FunctionCallExpression *call,
                                int64 hash, bool fatal /* = true */) {
  return os_invoke_mil(c, s, call->getParams(env), hash, fatal);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

bool ObjectData::o_exists(CStrRef propName, int64 hash,
                          CStrRef context /* = null_string */) const {
  StringData *sd;
  if (context.isNull()) {
    sd = FrameInjection::GetClassName(false).get();
  } else {
    sd = context.get();
  }
  ASSERT(sd && sd->data());
  return o_exists(propName, hash, sd->data(), sd->hash());
}

bool ObjectData::o_exists(CStrRef propName, int64 phash,
                          const char *context, int64 hash) const {
  return o_existsPublic(propName, phash);
}

bool ObjectData::o_existsPublic(CStrRef propName, int64 hash) const {
  return propName.size() > 0 && o_properties &&
         // object properties are always strings
         o_properties->exists(propName, hash, true);
}

Variant ObjectData::o_get(CStrRef propName, int64 hash,
    bool error /* = true */, CStrRef context /* = null_string */) {
  StringData *sd;
  if (context.isNull()) {
    sd = FrameInjection::GetClassName(false).get();
  } else {
    sd = context.get();
  }
  ASSERT(sd && sd->data());
  return o_get(propName, hash, error, sd->data(), sd->hash());
}

Variant ObjectData::o_get(CStrRef propName, int64 phash, bool error,
    const char *context, int64 hash) {
  return o_getPublic(propName, phash, error);
}

Variant ObjectData::o_getPublic(CStrRef propName, int64 hash,
    bool error /* = true */) {
  if (propName.size() == 0) {
    return null;
  }
  if (o_properties && o_properties->exists(propName, hash, true)) {
    return o_properties->rvalAt(propName, hash, false, true);
  }
  if (getAttribute(InGet)) {
    return ObjectData::doGet(propName, error);
  } else {
    AttributeSetter a(InGet, this);
    return doGet(propName, error);
  }
}

Variant ObjectData::o_getUnchecked(CStrRef propName, int64 hash,
    CStrRef context /* = null_string */) {
  StringData *sd;
  if (context.isNull()) {
    sd = FrameInjection::GetClassName(false).get();
  } else {
    sd = context.get();
  }
  ASSERT(sd && sd->data());
  return o_getUnchecked(propName, hash, sd->data(), sd->hash());
}

Variant ObjectData::o_getUnchecked(CStrRef propName, int64 phash,
    const char *context, int64 hash) {
  return o_get(propName, phash, true, context, hash);
}

Variant ObjectData::o_set(CStrRef propName, int64 hash, CVarRef v,
    bool forInit /* = false */, CStrRef context /* = null_string */) {
  StringData *sd;
  if (context.isNull()) {
    sd = FrameInjection::GetClassName(false).get();
  } else {
    sd = context.get();
  }
  ASSERT(sd && sd->data());
  return o_set(propName, hash, v, forInit, sd->data(), sd->hash());
}

Variant ObjectData::o_set(CStrRef propName, int64 phash, CVarRef v,
    bool forInit, const char *context, int64 hash) {
  return o_setPublic(propName, phash, v, forInit);
}

Variant ObjectData::o_setPublic(CStrRef propName, int64 hash, CVarRef v,
    bool forInit /* = false */) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }
  if (forInit || getAttribute(InSet)) {
    return ObjectData::t___set(propName, v);
  } else {
    AttributeSetter a(InSet, this);
    return t___set(propName, v);
  }
}

void ObjectData::o_setArray(CArrRef properties) {
  bool valueRef = properties->supportValueRef();
  for (ArrayIter iter(properties); iter; ++iter) {
    String key = iter.first().toString();
    if (key.empty() || key.charAt(0) != '\0') {
      // public property
      if (valueRef) {
        CVarRef secondRef = iter.secondRef();
        if (secondRef.isReferenced()) {
          o_setPublic(key, -1, ref(secondRef), false);
          continue;
        }
      }
      o_setPublic(key, -1, iter.second(), false);
    }
  }
}

Object ObjectData::FromArray(ArrayData *properties) {
  ObjectData *ret = NEW(c_stdclass)();
  if (!properties->empty()) {
    ret->o_properties = NEW(Array)(properties);
  }
  return ret;
}

CVarRef ObjectData::set(CStrRef s, CVarRef v) {
  o_set(s, -1, v);
  return v;
}

Variant &ObjectData::o_lval(CStrRef propName, int64 hash,
    CStrRef context /* = null_string */) {
  StringData *sd;
  if (context.isNull()) {
    sd = FrameInjection::GetClassName(false).get();
  } else {
    sd = context.get();
  }
  ASSERT(sd && sd->data());
  return o_lval(propName, hash, sd->data(), sd->hash());
}

Variant &ObjectData::o_lval(CStrRef propName, int64 phash,
    const char *context, int64 hash) {
  return o_lvalPublic(propName, phash);
}

Variant &ObjectData::o_lvalPublic(CStrRef propName, int64 hash) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }
  if (o_properties) {
    return o_properties->lvalAt(propName, hash, false, true);
  }
  return ___lval(propName);
}

Array ObjectData::o_toArray() const {
  Array ret(ArrayData::Create());
  o_getArray(ret);
  if (o_properties && !o_properties->empty()) {
    ret += (*o_properties);
    return ret;
  }
  return ret;
}

Array ObjectData::o_toIterArray(CStrRef context,
                                bool getRef /* = false */) {
  const char *object_class = o_getClassName();
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
    if (strcasecmp(object_class, context.data()) == 0) {
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
    if (visible && o_propExists(prop->name, -1, context)) {
      if (getRef) {
        Variant &ov = o_lval(prop->name, -1, context);
        Variant &av = ret.lvalAt(prop->name, -1, false, true);
        av = ref(ov);
      } else {
        ret.set(prop->name, o_getUnchecked(prop->name, -1,
                                           prop->owner->getName(), -1));
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
          Variant &av = ret.lvalAt(key, -1, false, true);
          av = ref(value);
        }
      }
    } else {
      ret += dynamics;
    }
  }
  return ret;
}

Array ObjectData::o_getDynamicProperties() const {
  if (o_properties) return *o_properties;
  return Array();
}

Variant ObjectData::o_invoke(CStrRef s, CArrRef params, int64 hash /* = -1 */,
                             bool fatal /* = true */) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_invoke_mil(sd->data(), params, hash < 0 ? sd->hash() : hash,
                      fatal);
}

Variant ObjectData::o_root_invoke(CStrRef s, CArrRef params,
                                  int64 hash /* = -1 */,
                                  bool fatal /* = true */) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_root_invoke_mil(sd->data(), params,
                           hash < 0 ? sd->hash() : hash, fatal);
}

Variant ObjectData::o_invoke_few_args(CStrRef s, int64 hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_invoke_few_args_mil(sd->data(), hash < 0 ? sd->hash() : hash,
                               count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_root_invoke_few_args_mil(sd->data(), hash < 0 ? sd->hash() : hash,
                                    count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_invoke(MethodIndex methodIndex, const char *s,
                             CArrRef params, int64 hash,
                             bool fatal /* = true */) {
  if (RuntimeOption::FastMethodCall) {
    // can only be called with a valid methodIndex
    s = methodIndexLookupReverse(methodIndex);
  }
  return doRootCall(s, params, fatal);
}

// This is not the base o_invoke, it's a redispatch
// to handle a dynamic method name.
Variant ObjectData::o_invoke_mil(const char *s,
                                 CArrRef params, int64 hash,
                                 bool fatal /* = true */) {
  MethodIndex methodIndex (MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    // if the method doesn't exist anywhere, __call is the only
    // choice no matter where we are in the hierarchy
    if (methodIndex.isFail()) return doRootCall(s, params, fatal);
  }
  // redispatch, not necessarily ObjectData::o_invoke
  return o_invoke(methodIndex, s, params, hash, fatal);
}

Variant ObjectData::o_root_invoke(MethodIndex methodIndex,
                                  const char *s, CArrRef params, int64 hash,
                                  bool fatal /* = true */) {
  return o_invoke(methodIndex, s, params, hash, fatal);
}

Variant ObjectData::o_root_invoke_mil(const char *s, CArrRef params, int64 hash,
                                      bool fatal /* = true */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return doRootCall(s, params, fatal);
    }
  }
  return o_root_invoke(methodIndex, s, params, hash, fatal);
}

Variant ObjectData::o_invoke_ex(const char *clsname, MethodIndex methodIndex ,
                                const char *s, CArrRef params, int64 hash,
                                bool fatal /* = true */) {
  if (fatal) {
    throw InvalidClassException(clsname);
  } else {
    return false;
  }
}

Variant ObjectData::o_invoke_ex_mil(const char *clsname,
                                    const char *s, CArrRef params, int64 hash,
                                    bool fatal /* = true */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      if (fatal) {
        throw InvalidClassException(clsname);
      } else {
        return false;
      }
    }
  }
  return o_invoke_ex(clsname, methodIndex, s, params, hash,
                     fatal);
}

Variant ObjectData::o_invoke_few_args(MethodIndex methodIndex, const char *s,
                                      int64 hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  switch (count) {
  case 0: {
    return ObjectData::o_invoke(methodIndex, s, Array(), hash);
  }
  case 1: {
    Array params(ArrayInit(1, true).set(0, a0).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 2: {
    Array params(ArrayInit(2, true).set(0, a0).set(1, a1).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 3: {
    Array params(ArrayInit(3, true).set(0, a0).set(1, a1).set(2, a2).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    Array params(ArrayInit(4, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 5: {
    Array params(ArrayInit(5, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 6: {
    Array params(ArrayInit(6, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    Array params(ArrayInit(7, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 8: {
    Array params(ArrayInit(8, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 9: {
    Array params(ArrayInit(9, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).set(8, a8).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
  case 10: {
    Array params(ArrayInit(10, true).set(0, a0).set(1, a1).set(2, a2).
                                     set(3, a3).set(4, a4).set(5, a5).
                                     set(6, a6).set(7, a7).set(8, a8).
                                     set(9, a9).create());
    return ObjectData::o_invoke(methodIndex, s, params, hash);
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

Array ObjectData::collectArgs(int count, INVOKE_FEW_ARGS_IMPL_ARGS) {
  switch (count) {
  case 0: {
    return Array();
  }
  case 1: {
    return Array (ArrayInit(1, true).set(0, a0).create());
  }
  case 2: {
    return Array (ArrayInit(2, true).set(0, a0).set(1, a1).create());
  }
  case 3: {
    return Array (ArrayInit(3, true).set(0, a0).set(1, a1).set(2, a2).create());
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    return Array (ArrayInit(4, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).create());
  }
  case 5: {
    return Array(ArrayInit(5, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).create());
  }
  case 6: {
    return Array (ArrayInit(6, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).create());
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    return Array (ArrayInit(7, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).create());
  }
  case 8: {
    return Array (ArrayInit(8, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).create());
  }
  case 9: {
    return Array (ArrayInit(9, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).set(8, a8).create());
  }
  case 10: {
    return Array (ArrayInit(10, true).set(0, a0).set(1, a1).set(2, a2).
                                     set(3, a3).set(4, a4).set(5, a5).
                                     set(6, a6).set(7, a7).set(8, a8).
                                     set(9, a9).create());
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

Variant ObjectData::o_invoke_few_args_mil(const char *s,
                                          int64 hash, int count,
                                          INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (RuntimeOption::FastMethodCall) {
    MethodIndex methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      // FMC: must test this
      return doRootCall(s, collectArgs(count, INVOKE_FEW_ARGS_PASS_ARGS),
                        true);
    }
    return o_invoke_few_args(methodIndex, s, hash, count,
                             INVOKE_FEW_ARGS_PASS_ARGS);
  }
  else
    return o_invoke_few_args(MethodIndex::fail(), s, hash, count,
                             INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_root_invoke_few_args(MethodIndex methodIndex,
                                           const char *s,
                                           int64 hash, int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  return o_invoke_few_args(methodIndex, s, hash, count,
                           INVOKE_FEW_ARGS_PASS_ARGS);
}
Variant ObjectData::o_root_invoke_few_args_mil(const char *s,
                                               int64 hash, int count,
                                               INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (RuntimeOption::FastMethodCall) {
    MethodIndex methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      // FMC: broken, don't know what happens here.
      return doRootCall(s, collectArgs(count, INVOKE_FEW_ARGS_PASS_ARGS),
                        true);
    }
    return o_root_invoke_few_args(methodIndex, s, hash, count,
                                  INVOKE_FEW_ARGS_PASS_ARGS);
  }
  return o_root_invoke_few_args(MethodIndex::fail(), s, hash, count,
                                INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_invoke_from_eval(const char *s,
                                       Eval::VariableEnvironment &env,
                                       const Eval::FunctionCallExpression *call,
                                       int64 hash,
                                       bool fatal /* = true */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return doRootCall(s, call->getParams(env), fatal);
    }
  }
  return o_invoke(methodIndex, s, call->getParams(env), hash,
                  fatal);
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
  if (serializer->incNestedLevel((void*)this, true)) {
    serializer->writeOverflow((void*)this, true);
  } else if ((serializer->getType() == VariableSerializer::Serialize ||
              serializer->getType() == VariableSerializer::APCSerialize) &&
             o_instanceof("Serializable")) {
    Variant ret =
      const_cast<ObjectData*>(this)->o_invoke_mil(
                                              "serialize", Array(), -1);
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
         serializer->getType() == VariableSerializer::APCSerialize) &&
        const_cast<ObjectData*>(this)->php_sleep(ret)) {
      if (ret.isArray()) {
        const ClassInfo *cls = ClassInfo::FindClass(o_getClassName());
        Array wanted = Array::Create();
        Array props = ret.toArray();
        for (ArrayIter iter(props); iter; ++iter) {
          String name = iter.second().toString();
          if (o_exists(name, -1, o_getClassName())) {
            ClassInfo::PropertyInfo *p = cls->getPropertyInfo(name);
            String propName = name;
            if (p && (p->attribute & ClassInfo::IsPrivate)) {
              propName = concat4(s_zero, o_getClassName(), s_zero, name);
            }
            wanted.set(propName, const_cast<ObjectData*>(this)->
                       o_getUnchecked(name, -1, o_getClassName()));
          } else {
            raise_warning("\"%s\" returned as member variable from "
                          "__sleep() but does not exist", name.data());
            wanted.set(name, null);
          }
        }
        serializer->setObjectInfo(o_getClassName(), o_getId());
        wanted.serialize(serializer);
      } else {
        raise_warning("serialize(): __sleep should return an array only "
                      "containing the names of instance-variables to "
                      "serialize");
        null.serialize(serializer);
      }
    } else {
      serializer->setObjectInfo(o_getClassName(), o_getId());
      o_toArray().serialize(serializer);
    }
  }
  serializer->decNestedLevel((void*)this);
}

void ObjectData::dump() const {
  o_toArray().dump();
}

ObjectData *ObjectData::clone() {
  ObjectData *clone = cloneImpl();
  return clone;
}

void ObjectData::cloneSet(ObjectData *clone) {
  if (o_properties) {
    clone->o_properties = NEW(Array)(*o_properties);
  }
}

ObjectData *ObjectData::getRoot() { return this; }

Variant ObjectData::doCall(Variant v_name, Variant v_arguments, bool fatal) {
  return o_invoke_failed(o_getClassName(), v_name.toString().data(), fatal);
}

Variant ObjectData::doRootCall(Variant v_name,
                               Variant v_arguments, bool fatal) {
  return doCall(v_name, v_arguments, fatal);
}

Variant ObjectData::doGet(Variant v_name, bool error) {
  if (error) {
    raise_notice("Undefined property: %s::$%s", o_getClassName().data(),
                 v_name.toString().data());
  }
  return null_variant;
}

bool ObjectData::doIsSet(CStrRef prop, int64 phash,
                         CStrRef context) {
  if (o_exists(prop, phash, context)) {
    return !o_get(prop, phash, false, context).isNull();
  }
  return t___isset(prop);
}

bool ObjectData::doEmpty(CStrRef prop, int64 phash,
                         CStrRef context) {
  if (o_exists(prop, phash, context)) {
    return empty(o_get(prop, phash, false, context));
  }
  return !t___isset(prop) || empty(t___get(prop));
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
  if (!o_properties) {
    o_properties = NEW(Array)();
  }
  if (v_value.isReferenced()) {
    o_properties->set(v_name, ref(v_value), -1, true);
  } else {
    o_properties->set(v_name, v_value, -1, true);
  }
  return null;
}

Variant ObjectData::t___get(Variant v_name) {
  // Not called
  return null;
}

Variant &ObjectData::___lval(Variant v_name) {
  if (!o_properties) {
    // this is needed, since a lval() is actually going to create a null
    // element in properties array
    o_properties = NEW(Array)();
  }
  return o_properties->lvalAt(v_name, -1, false, true);
}
Variant &ObjectData::___offsetget_lval(Variant v_name) {
  return ___lval(v_name);
}
bool ObjectData::t___isset(Variant v_name) {
  return false;
}

Variant ObjectData::t___unset(Variant v_name) {
  String sname = v_name.toString();
  unset(o_lval(sname, -1));
  if (o_properties && o_properties->exists(sname, -1, true)) {
    o_properties->weakRemove(sname, -1, true);
  }
  return null;
}

bool ObjectData::o_propExists(CStrRef s, int64 hash /* = -1 */,
                              CStrRef context /* = null_string */) {
  // Exists and the value is not null or it is null but also initialized.
  // Can't just do isInitialized because type inferred properties may not
  // be in the o_lval table.
  return o_exists(s, hash, context) && (!o_get(s, hash, context).isNull() ||
      o_lval(s, hash, context).isInitialized());
}

Variant ObjectData::t___sleep() {
  clearAttribute(HasSleep);
  return null;
}

Variant ObjectData::t___wakeup() {
  // do nothing
  return null;
}

Variant ObjectData::t___set_state(Variant v_properties) {
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

Variant ExtObjectData::o_root_invoke(MethodIndex methodIndex, const char *s,
                                     CArrRef ps, int64 h,
    bool f /* = true */) {
  return root->o_invoke(methodIndex, s, ps, h, f);
}

Variant ExtObjectData::o_root_invoke_few_args(MethodIndex methodIndex,
                          const char *s, int64 h, int count,
                          INVOKE_FEW_ARGS_IMPL_ARGS) {
  return root->o_invoke_few_args(methodIndex, s, h, count,
                                 INVOKE_FEW_ARGS_PASS_ARGS);
}

Object ObjectData::fiberMarshal(FiberReferenceMap &refMap) const {
  ObjectData *px = (ObjectData*)refMap.lookup((void*)this);
  if (px == NULL) {
    Object copy = create_object(o_getClassName(), null_array);
    // ahead of deep copy
    refMap.insert(const_cast<ObjectData*>(this), copy.get());
    Array props;
    o_getArray(props);
    copy->o_setArray(props.fiberMarshal(refMap));
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
      copy = create_object(o_getClassName(), null_array);
      px = copy.get();
    }
    // ahead of deep copy
    refMap.insert(const_cast<ObjectData*>(this), px);
    Array props;
    o_getArray(props);
    px->o_setArray(props.fiberUnmarshal(refMap));
  }
  return Object(px);
}

///////////////////////////////////////////////////////////////////////////////
}
