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

#include <runtime/eval/ast/function_call_expression.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

// current maximum object identifier
static IMPLEMENT_THREAD_LOCAL(int, os_max_id);

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor

ObjectData::ObjectData() : o_properties(NULL), o_attribute(0) {
  o_id = ++(*os_max_id.get());
}

ObjectData::~ObjectData() {
  if (o_properties) {
    o_properties->release();
  }
  int *pmax = os_max_id.get();
  if (o_id == *pmax) {
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

Variant ObjectData::os_invoke(const char *c, const char *s,
                              CArrRef params, int64 hash,
                              bool fatal /* = true */) {
  Object obj = create_object(c, Array::Create(), false);
  return obj->o_invoke(s, params, hash, fatal);
}

Variant ObjectData::os_constant(const char *s) {
  ostringstream msg;
  msg << "unknown class constant " << s;
  throw FatalErrorException(msg.str().c_str());
}

Variant
ObjectData::os_invoke_from_eval(const char *c, const char *s,
                                Eval::VariableEnvironment &env,
                                const Eval::FunctionCallExpression *call,
                                int64 hash, bool fatal /* = true */) {
  return os_invoke(c, s, call->getParams(env), hash, fatal);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

bool ObjectData::o_exists(CStrRef propName, int64 hash,
    const char *context /* = NULL */) const {
  return o_existsPublic(propName, hash);
}

bool ObjectData::o_existsPublic(CStrRef propName, int64 hash) const {
  return propName.size() > 0 && o_properties &&
         // object properties are always strings
         o_properties->exists(propName, hash, true);
}

Variant ObjectData::o_get(CStrRef propName, int64 hash,
    bool error /* = true */, const char *context /* = NULL */) {
  return o_getPublic(propName, hash, error);
}
Variant ObjectData::o_getPublic(CStrRef propName, int64 hash,
    bool error /* = true */) {
  if (propName.size() == 0) {
    return null;
  }
  if (o_properties && o_properties->exists(propName, hash, true)) {
    return o_properties->rvalAt(propName, hash, false, true);
  }
  return doGet(propName, error);
}

Variant ObjectData::o_getUnchecked(CStrRef propName, int64 hash,
    const char *context /* = NULL */) {
  return o_get(propName, hash, true, context);
}

Variant ObjectData::o_set(CStrRef propName, int64 hash, CVarRef v,
    bool forInit /* = false */, const char *context /* = NULL */) {
  return o_setPublic(propName, hash, v, forInit);
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

void ObjectData::o_set(const Array properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    o_set(iter.first().toString(), -1, iter.second());
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
    const char *context /* = NULL */) {
  return o_lvalPublic(propName, hash);
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
  o_get(ret);
  if (o_properties && !o_properties->empty()) {
    ret += (*o_properties);
    return ret;
  }
  return ret;
}

Array ObjectData::o_toIterArray(const char *context,
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
  if (context == NULL || !*context) {
    category = 1;
  } else {
    contextClassInfo = ClassInfo::FindClass(context);
    ASSERT(contextClassInfo);
    if (strcasecmp(object_class, context) == 0) {
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
  for (ClassInfo::PropertyVec::const_iterator iter = properties.begin();
       iter != properties.end(); ++iter) {
    if ((*iter)->attribute & ClassInfo::IsStatic) continue;

    bool visible = false;
    switch (category) {
    case 1:
      visible = ((*iter)->attribute & ClassInfo::IsPublic);
      break;
    case 2:
      if (((*iter)->attribute & ClassInfo::IsPrivate) == 0) {
        visible = true;
      } else {
        ClassInfo::PropertyMap::const_iterator iterProp =
          contextProperties.find((*iter)->name);
        if (iterProp != contextProperties.end() &&
            iterProp->second->owner == contextClassInfo) {
          visible = true;
        } else {
          visible = false;
        }
      }
      break;
    case 3:
      if (((*iter)->attribute & ClassInfo::IsPrivate) == 0 ||
          (*iter)->owner == classInfo) {
        visible = true;
      }
      break;
    default:
      ASSERT(false);
    }
    if (visible) {
      if (getRef) {
        Variant &ov = o_lval((*iter)->name, -1, context);
        Variant &av = ret.lvalAt((*iter)->name, -1, false, true);
        av = ref(ov);
      } else {
        ret.set((*iter)->name, o_getUnchecked((*iter)->name, -1,
                (*iter)->owner->getName()));
      }
    }
    dynamics.remove((*iter)->name);
  }
  if (dynamics.size()) {
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

Variant ObjectData::o_invoke(const char *s, CArrRef params, int64 hash,
                             bool fatal /* = true */) {
  return doRootCall(s, params, fatal);
}

Variant ObjectData::o_invoke_ex(const char *clsname, const char *s,
                                CArrRef params, int64 hash,
                                bool fatal /* = true */) {
  if (fatal) {
    throw InvalidClassException(clsname);
  } else {
    return false;
  }
}

Variant ObjectData::o_invoke_few_args(const char *s, int64 hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  switch (count) {
  case 0: {
    return ObjectData::o_invoke(s, Array(), hash);
  }
  case 1: {
    Array params(ArrayInit(1, true).set(0, a0).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 2: {
    Array params(ArrayInit(2, true).set(0, a0).set(1, a1).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 3: {
    Array params(ArrayInit(3, true).set(0, a0).set(1, a1).set(2, a2).create());
    return ObjectData::o_invoke(s, params, hash);
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    Array params(ArrayInit(4, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 5: {
    Array params(ArrayInit(5, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 6: {
    Array params(ArrayInit(6, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).create());
    return ObjectData::o_invoke(s, params, hash);
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    Array params(ArrayInit(7, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 8: {
    Array params(ArrayInit(8, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 9: {
    Array params(ArrayInit(9, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).set(8, a8).create());
    return ObjectData::o_invoke(s, params, hash);
  }
  case 10: {
    Array params(ArrayInit(10, true).set(0, a0).set(1, a1).set(2, a2).
                                     set(3, a3).set(4, a4).set(5, a5).
                                     set(6, a6).set(7, a7).set(8, a8).
                                     set(9, a9).create());
    return ObjectData::o_invoke(s, params, hash);
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

Variant ObjectData::o_invoke_from_eval(const char *s,
                                       Eval::VariableEnvironment &env,
                                       const Eval::FunctionCallExpression *call,
                                       int64 hash,
                                       bool fatal /* = true */) {
  return o_invoke(s, call->getParams(env), hash, fatal);
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

void ObjectData::serialize(VariableSerializer *serializer) const {
  if (serializer->incNestedLevel((void*)this, true)) {
    serializer->writeOverflow((void*)this, true);
  } else if (serializer->getType() == VariableSerializer::Serialize &&
             o_instanceof("serializable")) {
    Variant ret =
      const_cast<ObjectData*>(this)->o_invoke("serialize", Array(), -1);
    if (ret.isString()) {
      serializer->writeSerializableObject(o_getClassName(), ret.toString());
    } else if (ret.isNull()) {
      serializer->writeNull();
    } else {
      raise_error("%s::serialize() must return a string or NULL",
                  o_getClassName());
    }
  } else {
    Variant ret;
    if (serializer->getType() == VariableSerializer::Serialize &&
        const_cast<ObjectData*>(this)->php_sleep(ret)) {
      if (ret.isArray()) {
        Array wanted = Array::Create();
        Array props = ret.toArray();
        for (ArrayIter iter(props); iter; ++iter) {
          String name = iter.second().toString();
          if (o_exists(name, -1, o_getClassName())) {
            wanted.set(name, const_cast<ObjectData*>(this)->
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

/**
 * Currently the o_fast_invoke methods simply forward the parameters to
 * the o_invoke methods. If fast_invoke* is adopted in the future, we
 * will provide better implementations for o_fast_invoke that do the use
 * the o_invoke methods as a crutch.
 */
Variant ObjectData::o_fast_invoke(const char *s, int64 hash, int count,
                                  const Variant ** args,
                                  bool fatal /* = true */) {
  if (count >= 0) {
    Array params = Array::Create();
    for (int i = 0; i < count; ++i) {
      params.append(ref(*args[i]));
    }
    return ref(o_invoke(s, params, hash, fatal));
  }
  ASSERT(count == -1);
  return ref(o_invoke(s, *args[0], hash, fatal));
}

Variant ObjectData::o_fast_invoke(const char *s, int64 hash, int count,
                                  CArrRef args,
                                  bool fatal /* = true */) {
  ASSERT(count == -1);
  return ref(o_invoke(s, args, hash, fatal));
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

Variant ObjectData::doRootCall(Variant v_name, Variant v_arguments, bool fatal) {
  return doCall(v_name, v_arguments, fatal);
}

Variant ObjectData::doGet(Variant v_name, bool error) {
  if (error) {
    raise_notice("Undefined property: %s::$%s", o_getClassName(),
                 v_name.toString().data());
  }
  return null_variant;
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
  if (!o_exists(v_name.toString(), -1)) return false;
  Variant v = o_get(v_name.toString(), -1, false);
  return isset(v);
}

Variant ObjectData::t___unset(Variant v_name) {
  unset(o_lval(v_name.toString(), -1));
  if (o_properties) o_properties->weakRemove(v_name);
  return null;
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
  string msg = o_getClassName();
  msg += "::__toString() was not defined";
  throw BadTypeConversionException(msg.c_str());
}

Variant ObjectData::t___clone() {
  // do nothing
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
