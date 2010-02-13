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

#include <cpp/base/object_data.h>
#include <cpp/base/type_array.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/externals.h>
#include <cpp/base/variable_serializer.h>
#include <util/lock.h>
#include <cpp/base/class_info.h>

#include <cpp/eval/ast/function_call_expression.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

static ThreadLocal<int> os_max_id; // current maximum object identifier

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor

ObjectData::ObjectData() : o_properties(NULL), o_attribute(0), o_inCall(0) {
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

///////////////////////////////////////////////////////////////////////////////
// class info

bool ObjectData::o_isClass(const char *s) const {
  return strcasecmp(s, o_getClassName()) == 0;
}

const Eval::MethodStatement *ObjectData::getMethodStatement(const char* name)
  const {
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// static methods and properties

Variant ObjectData::os_get(const char *s, int64 hash) {
  throw FatalErrorException((string("unknown static property ") + s).c_str());
}

Variant &ObjectData::os_lval(const char *s, int64 hash) {
  throw FatalErrorException((string("unknown static property ") + s).c_str());
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

bool ObjectData::o_exists(CStrRef propName, int64 hash) const {
  return propName.size() > 0 && o_properties && o_properties->exists(propName);
}

Variant ObjectData::o_get(CStrRef propName, int64 hash) {
  if (propName.size() == 0) {
    return null;
  }
  if (o_properties && o_properties->exists(propName)) {
    return o_properties->rvalAt(propName);
  }
  return t___get(propName);
}

Variant ObjectData::o_getUnchecked(CStrRef propName, int64 hash) {
  return o_get(propName, hash);
}

Variant ObjectData::o_set(CStrRef propName, int64 hash, CVarRef v,
                          bool forInit /* = false */) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }
  if (forInit) {
    return ObjectData::t___set(propName, v);
  } else {
    return t___set(propName, v);
  }
}

void ObjectData::o_set(const Array properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    o_set(iter.first().toString(), -1, iter.second());
  }
}

CVarRef ObjectData::set(CStrRef s, CVarRef v) {
  o_set(s, -1, v);
  return v;
}

Variant &ObjectData::o_lval(CStrRef propName, int64 hash) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }
  if (o_properties) {
    return o_properties->lvalAt(propName, hash);
  }
  return ___lval(propName);
}

Array ObjectData::o_toArray() const {
  vector<ArrayElement *> props;
  o_get(props);
  Array ret(ArrayData::Create(props, false));
  if (o_properties && !o_properties->empty()) {
    return ret.merge(*o_properties);
  }
  return ret;
}

Array ObjectData::o_toIterArray(const char *context) {
  const char *object_class = o_getClassName();
  const ClassInfo *classInfo = ClassInfo::FindClass(object_class);
  const ClassInfo *contextClassInfo = NULL;
  int category;

  if (!classInfo) return Array::Create();

  Array ret = Array::Create();

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
      ret.set((*iter)->name, o_getUnchecked((*iter)->name, -1));
    }
    dynamics.remove((*iter)->name);
  }
  if (dynamics.size()) {
    ret.merge(dynamics);
  }
  return ret;
}

Array ObjectData::o_getDynamicProperties() const {
  if (o_properties) return *o_properties;
  return Array();
}

Variant ObjectData::o_invoke(const char *s, CArrRef params, int64 hash,
                             bool fatal /* = true */) {
  return doCall(s, params, fatal);
}

Variant ObjectData::o_root_invoke(const char *s, CArrRef params, int64 hash,
                                  bool fatal /* = true */) {
  return o_invoke(s, params, hash, fatal);
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
                                      CVarRef a0 /* = null_variant */,
                                      CVarRef a1 /* = null_variant */,
                                      CVarRef a2 /* = null_variant */
#if INVOKE_FEW_ARGS_COUNT > 3
                                      ,CVarRef a3 /* = null_variant */,
                                      CVarRef a4 /* = null_variant */,
                                      CVarRef a5 /* = null_variant */
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                                      ,CVarRef a6 /* = null_variant */,
                                      CVarRef a7 /* = null_variant */,
                                      CVarRef a8 /* = null_variant */,
                                      CVarRef a9 /* = null_variant */
#endif
) {
  switch (count) {
  case 0: {
    return ObjectData::o_invoke(s, Array(), hash);
  }
  case 1: {
    Array params(NEW(ArrayElement)(a0), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 2: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 3: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 5: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                 NEW(ArrayElement)(a4), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 6: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                 NEW(ArrayElement)(a4), NEW(ArrayElement)(a5), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                 NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                 NEW(ArrayElement)(a6), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 8: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                 NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                 NEW(ArrayElement)(a6), NEW(ArrayElement)(a7), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 9: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                 NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                 NEW(ArrayElement)(a6), NEW(ArrayElement)(a7),
                 NEW(ArrayElement)(a8), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
  case 10: {
    Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                 NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                 NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                 NEW(ArrayElement)(a6), NEW(ArrayElement)(a7),
                 NEW(ArrayElement)(a8), NEW(ArrayElement)(a9), NULL);
    return ObjectData::o_invoke(s, params, hash);
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

Variant ObjectData::o_root_invoke_few_args(const char *s, int64 hash, int count,
                                           CVarRef a0 /* = null_variant */,
                                           CVarRef a1 /* = null_variant */,
                                           CVarRef a2 /* = null_variant */
#if INVOKE_FEW_ARGS_COUNT > 3
                                           ,CVarRef a3 /* = null_variant */,
                                           CVarRef a4 /* = null_variant */,
                                           CVarRef a5 /* = null_variant */
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                                           ,CVarRef a6 /* = null_variant */,
                                           CVarRef a7 /* = null_variant */,
                                           CVarRef a8 /* = null_variant */,
                                           CVarRef a9 /* = null_variant */
#endif
) {
  return o_invoke_few_args(s, hash, count, a0, a1, a2
#if INVOKE_FEW_ARGS_COUNT > 3
                           ,a3, a4, a5
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                           ,a6, a7, a8, a9
#endif
                           );
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

void ObjectData::serialize(VariableSerializer *serializer) const {
  if (serializer->incNestedLevel((void*)this, true)) {
    serializer->writeOverflow((void*)this, true);
  } else {
    setAttribute(HasSleep);
    Variant ret = const_cast<ObjectData*>(this)->t___sleep();
    if (getAttribute(HasSleep)) {
      if (ret.isArray()) {
        Array wanted = Array::Create();
        Array props = ret.toArray();
        for (ArrayIter iter(props); iter; ++iter) {
          String name = iter.second().toString();
          if (o_exists(name, -1)) {
            wanted.set(name, const_cast<ObjectData*>(this)->o_get(name, -1));
          } else {
            Logger::Warning("\"%s\" returned as member variable from "
                            "__sleep() but does not exist", name.data());
            wanted.set(name, null);
          }
        }
        serializer->setObjectInfo(o_getClassName(), o_getId());
        wanted.serialize(serializer);
      } else {
        Logger::Warning("serialize(): __sleep should return an array only "
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

Variant ObjectData::doCall(Variant v_name, Variant v_arguments, bool fatal) {
  return o_invoke_failed(o_getClassName(), v_name.toString().data(), fatal);
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

void ObjectData::setInCall(CStrRef name) {
  if (++o_inCall > 10) {
    string msg = "Too many levels of recursion in __call() while calling ";
    msg += (const char *)name;
    throw FatalErrorException(msg.c_str());
  }
}

Variant ObjectData::t___set(Variant v_name, Variant v_value) {
  if (!o_properties) {
    o_properties = NEW(Array)();
  }
  if (v_value.isReferenced()) {
    o_properties->set(v_name, ref(v_value));
  } else {
    o_properties->set(v_name, v_value);
  }
  return null;
}

Variant ObjectData::t___get(Variant v_name) {
  if (!o_properties) {
    // this is needed, since a get() is actually going to create a null
    // element in properties array
    o_properties = NEW(Array)();
  }
  return o_properties->rvalAt(v_name);
}

Variant &ObjectData::___lval(Variant v_name) {
  if (!o_properties) {
    // this is needed, since a lval() is actually going to create a null
    // element in properties array
    o_properties = NEW(Array)();
  }
  return o_properties->lvalAt(v_name);
}
Variant &ObjectData::___offsetget_lval(Variant v_name) {
  return ___lval(v_name);
}
bool ObjectData::t___isset(Variant v_name) {
  if (!o_exists(v_name.toString(), -1)) return false;
  Variant v = o_get(v_name.toString(), -1);
  return isset(v);
}

Variant ObjectData::t___unset(Variant v_name) {
  unset(o_lval(v_name.toString(), -1));
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
