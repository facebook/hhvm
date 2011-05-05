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

#include <runtime/base/object_data.h>
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
IMPLEMENT_THREAD_LOCAL_NO_CHECK(int, ObjectData::os_max_id);

int ObjectData::GetMaxId() {
  return *(ObjectData::os_max_id.getCheck());
}

static CallInfo s_ObjectData_call_handler((void*)ObjectData::callHandler,
    (void*)ObjectData::callHandlerFewArgs, 0,
    CallInfo::VarArgs | CallInfo::Method | CallInfo::CallMagicMethod, 0);

static StaticString s___callStatic("__callStatic");

///////////////////////////////////////////////////////////////////////////////
// constructor/destructor
ObjectData::~ObjectData() {
  if (o_properties) {
    o_properties->release();
  }
  int &pmax = *os_max_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
}

void ObjectData::dynConstruct(CArrRef params) {}

static CallInfo s_ObjectData_null_constructor(
    (void*)ObjectData::NullConstructor,
    (void*)ObjectData::NullConstructorFewArgs, 0, CallInfo::Method, 0);
void ObjectData::getConstructor(MethodCallPackage &mcp) {
  mcp.ci = &s_ObjectData_null_constructor;
  mcp.obj = this;
}

void ObjectData:: dynConstructUnchecked(CArrRef params) {
  return dynConstruct(params);
}

///////////////////////////////////////////////////////////////////////////////
// class info

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

#ifdef ENABLE_LATE_STATIC_BINDING
void ObjectData::bindThis(ThreadInfo *info) {
  FrameInjection::SetStaticClassName(info, getRoot()->o_getClassName());
}
#endif

void ObjectData::setDummy() {
  int *pmax = os_max_id.getNoCheck();
  if (o_id == *pmax) --(*pmax);
  o_id = 0; // for isset($this) to tell whether this is a fake obj
}

///////////////////////////////////////////////////////////////////////////////
// static methods and properties

Variant ObjectData::os_getInit(CStrRef s) {
  throw FatalErrorException(0, "unknown property %s", s.c_str());
}

Variant ObjectData::os_get(CStrRef s) {
  throw FatalErrorException(0, "unknown static property %s", s.c_str());
}

Variant &ObjectData::os_lval(CStrRef s) {
  throw FatalErrorException(0, "unknown static property %s", s.c_str());
}

Variant ObjectData::os_invoke(const char *c, const char *s,
                              CArrRef params, int64 hash,
                              bool fatal /* = true */) {
  Object obj = FrameInjection::GetThis();
  if (!obj.instanceof(c)) {
    obj = create_object(c, Array::Create(), false);
    obj->setDummy();
  }
  return obj->o_invoke_ex(c, s, params, fatal);
}

Variant ObjectData::os_constant(const char *s) {
  ostringstream msg;
  msg << "unknown class constant " << s;
  throw FatalErrorException(msg.str().c_str());
}

bool ObjectData::os_get_call_info(MethodCallPackage &info,
                                  int64 hash /* = -1 */) {
  if (info.obj) {
    // For classes that do not inherit from redeclared classes,
    // o_get_call_info() delegates to os_get_call_info() with info.obj set
    // beforehand.
    return info.obj->ObjectData::o_get_call_info(info, hash);
  }
  Object obj = FrameInjection::GetThis();
  ASSERT(!info.isObj);
  String cls = info.rootCls;
  if (!obj.instanceof(cls)) {
    obj = create_object(cls, Array::Create(), false);
    obj->setDummy();
  } else {
    return obj->o_get_call_info(info, hash);
  }
  if (!obj->hasCallStatic()) {
    info.fail();
    return false;
  }

  info.ci = &s_ObjectData_call_handler;
  info.obj = NULL;
  return true;
}

bool ObjectData::os_get_call_info_with_index(MethodCallPackage &info,
                                             MethodIndex mi,
                                             int64 hash /* = -1 */) {
  return os_get_call_info(info, hash);
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
  return MutableArrayIter(arr, key, val);
}

Variant *ObjectData::o_realProp(CStrRef propName, int flags,
                                CStrRef context /* = null_string */) const {
  return o_realPropPublic(propName, flags);
}

Variant *ObjectData::o_realPropPublic(CStrRef propName, int flags) const {
  if (propName.size() > 0 &&
      !(flags & RealPropNoDynamic) &&
      (o_properties ||
       ((flags & RealPropCreate) && (o_properties = NEW(Array)(), true)))) {
    return o_properties->lvalPtr(propName,
                                 flags & RealPropWrite, flags & RealPropCreate);
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

Variant ObjectData::o_set(CStrRef propName, CVarRef v,
                          bool forInit /* = false */,
                          CStrRef context /* = null_string */) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }

  bool useSet = !forInit && getAttribute(UseSet);
  int flags = useSet ? RealPropWrite : RealPropCreate | RealPropWrite;
  if (forInit) flags |= RealPropUnchecked;

  if (Variant *t = o_realProp(propName, flags, context)) {
    if (!useSet || t->isInitialized()) {
      *t = v;
      return v;
    }
  }

  if (useSet) {
    AttributeClearer a(UseSet, this);
    t___set(propName, v);
    return v;
  }

  o_setError(propName, context);
  return v;
}

Variant ObjectData::o_setPublic(CStrRef propName, CVarRef v,
                                bool forInit /* = false */) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }

  bool useSet = !forInit && getAttribute(UseSet);
  int flags = useSet ? RealPropWrite : RealPropCreate | RealPropWrite;
  if (forInit) flags |= RealPropUnchecked;

  if (Variant *t = o_realPropPublic(propName, flags)) {
    if (!useSet || t->isInitialized()) {
      *t = v;
      return v;
    }
  }

  if (useSet) {
    AttributeClearer a(UseSet, this);
    t___set(propName, v);
    return v;
  }

  o_setError(propName, null_string);
  return v;
}

Variant ObjectData::o_setPublicWithRef(CStrRef propName, CVarRef v,
                                       bool forInit /* = false */) {
  if (propName.size() == 0) {
    throw EmptyObjectPropertyException();
  }

  bool useSet = !forInit && getAttribute(UseSet);
  int flags = useSet ? RealPropWrite : RealPropCreate | RealPropWrite;
  if (forInit) flags |= RealPropUnchecked;

  if (Variant *t = o_realPropPublic(propName, flags)) {
    if (!useSet || t->isInitialized()) {
      t->setWithRef(v);
      return v;
    }
  }

  if (useSet) {
    AttributeClearer a(UseSet, this);
    t___set(propName, v);
    return v;
  }

  o_setError(propName, null_string);
  return v;
}

void ObjectData::o_setArray(CArrRef properties) {
  for (ArrayIter iter(properties); iter; ++iter) {
    String key = iter.first().toString();
    if (key.empty() || key.charAt(0) != '\0') {
      // non-private property
      CVarRef secondRef = iter.secondRef();
      o_setPublicWithRef(key, secondRef, false);
    }
  }
}

void ObjectData::o_getArray(Array &props, bool pubOnly /* = false */) const {
  if (o_properties && !o_properties->empty()) {
    for (ArrayIter it(*o_properties); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      props.addLval(key, true).setWithRef(value);
    }
  }
}

Variant ObjectData::o_argval(bool byRef, CStrRef s,
    bool error /* = true */, CStrRef context /* = null_string */) {
  if (byRef) {
    return ref(o_lval(s, context));
  } else {
    return o_get(s, error, context);
  }
}

Object ObjectData::FromArray(ArrayData *properties) {
  ObjectData *ret = SystemLib::AllocStdClassObject();
  if (!properties->empty()) {
    ret->o_properties = NEW(Array)(properties);
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

  ASSERT(useGet);

  AttributeClearer a(UseGet, this);
  if (getAttribute(HasLval)) {
    return *___lval(propName);
  }

  Variant &ret = const_cast<Variant&>(tmpForGet);
  ret = t___get(propName);
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
  const_cast<ObjectData*>(this)->getRoot()->o_getArray(ret);
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
  if (o_properties) return *o_properties;
  return Array();
}

Variant ObjectData::o_invoke(CStrRef s, CArrRef params, int64 hash /* = -1 */,
                             bool fatal /* = true */) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_invoke(sd->data(), params, hash < 0 ? sd->hash() : hash,
                  fatal);
}

Variant ObjectData::o_root_invoke(CStrRef s, CArrRef params,
                                  int64 hash /* = -1 */,
                                  bool fatal /* = true */) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_root_invoke(sd->data(), params,
      hash < 0 ? sd->hash() : hash, fatal);
}

Variant ObjectData::o_invoke_few_args(CStrRef s, int64 hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_invoke_few_args(sd->data(), hash < 0 ? sd->hash() : hash,
      count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  StringData *sd = s.get();
  ASSERT(sd && sd->data());
  return o_root_invoke_few_args(sd->data(),
      hash < 0 ? sd->hash() : hash, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_invoke(const char *s, CArrRef params, int64 hash,
                             bool fatal /* = true */) {
  MethodCallPackage mcp;
  if (!fatal) mcp.noFatal();
  String str(s);
  mcp.methodCall(this, str, hash);
  return (mcp.ci->getMeth())(mcp, params);
}

Variant ObjectData::o_root_invoke(const char *s, CArrRef params, int64 hash,
                                  bool fatal /* = true */) {
  return o_invoke(s, params, hash, fatal);
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

Variant ObjectData::o_invoke_few_args(const char *s, int64 hash, int count,
                                      INVOKE_FEW_ARGS_IMPL_ARGS) {
  MethodCallPackage mcp;
  String str(s);
  mcp.methodCall(this, str, hash);
  return (mcp.ci->getMethFewArgs())(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant ObjectData::o_root_invoke_few_args(const char *s, int64 hash,
                                           int count,
                                           INVOKE_FEW_ARGS_IMPL_ARGS) {
  return o_invoke_few_args(s, hash, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

bool ObjectData::o_get_call_info(MethodCallPackage &mcp,
                                 int64 hash /* = -1 */) {
  // If UseMethodIndex is on, classes will define o_get_call_info_with_index
  // and this will be a virtual call to the class' version.
  // If it's off, this will go to ObjectData's version.
  return o_get_call_info_with_index(mcp, methodIndexExists(*mcp.name), hash);
}

bool ObjectData::o_get_call_info_ex(const char *clsname,
                                    MethodCallPackage &mcp,
                                    int64 hash /* = -1 */) {
  mcp.obj = this;
  return ObjectData::o_get_call_info(mcp, hash);
}

bool ObjectData::o_get_call_info_with_index(MethodCallPackage &mcp,
                                            MethodIndex mi,
                                            int64 hash /* = -1 */) {
  mcp.ci = &s_ObjectData_call_handler;
  if (!hasCall() && !hasCallStatic()) {
    mcp.fail();
    return false;
  }
  return true;
}

bool ObjectData::o_get_call_info_with_index_ex(const char *clsname,
                                               MethodCallPackage &mcp,
                                               MethodIndex mi,
                                               int64 hash /* = -1 */) {
  mcp.obj = this;
  return ObjectData::o_get_call_info_with_index(mcp, mi, hash);
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
              serializer->getType() == VariableSerializer::APCSerialize) &&
             o_instanceof("Serializable")) {
    Variant ret =
      const_cast<ObjectData*>(this)->o_invoke("serialize", Array(), -1);
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
  return t___isset(prop);
}

bool ObjectData::o_empty(CStrRef prop, CStrRef context) {
  if (Variant *t = o_realProp(prop, 0, context)) {
    if (t->isInitialized()) {
      return empty(*t);
    }
  }
  return !t___isset(prop) || !getAttribute(UseGet) ||
    (AttributeClearer(UseGet, this),empty(t___get(prop)));
}

void ObjectData::o_unset(CStrRef prop, CStrRef context) {
  if (getAttribute(UseUnset)) {
    AttributeClearer a(UseUnset, this);
    t___unset(prop);
  } else {
    if (Variant *t = o_realProp(prop,
                                RealPropWrite|RealPropNoDynamic, context)) {
      unset(*t);
    } else if (o_properties && o_properties->exists(prop, true)) {
      o_properties->weakRemove(prop, true);
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
  if (!o_properties) {
    // this is needed, since a lval() is actually going to create a null
    // element in properties array
    o_properties = NEW(Array)();
  }
  return o_properties->lvalAt(v_name, AccessFlags::Key);
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

Variant ExtObjectData::o_root_invoke(const char *s, CArrRef ps, int64 h,
    bool f /* = true */) {
  return root->o_invoke(s, ps, h, f);
}
Variant ExtObjectData::o_root_invoke_few_args(const char *s, int64 h, int count,
                          INVOKE_FEW_ARGS_IMPL_ARGS) {
  return root->o_invoke_few_args(s, h, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Object ObjectData::fiberMarshal(FiberReferenceMap &refMap) const {
  ObjectData *px = (ObjectData*)refMap.lookup((void*)this);
  if (px == NULL) {
    Object copy = create_object(o_getClassName().fiberCopy(),
                                null_array, false);
    // ahead of deep copy
    refMap.insert(const_cast<ObjectData*>(this), copy.get());
    Array props;
    o_getArray(props);
    if (!props.empty()) {
      copy->o_setArray(props.fiberMarshal(refMap));
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
    o_getArray(props);
    if (!props.empty()) {
      px->o_setArray(props.fiberUnmarshal(refMap));
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
