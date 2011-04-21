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

#include <runtime/base/dynamic_object_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/externals.h>
#include <runtime/base/builtin_functions.h>
#include <util/util.h>

namespace HPHP {

/////////////////////////////////////////////////////////////////////////////
// constructor/destructor

DynamicObjectData::DynamicObjectData(const char* pname,
                                     ObjectData* r /* = NULL */) :
  root(r ? r : this) {
  if (pname) {
    CountableHelper h(root);
    parent = create_object(pname, Array(), false, root);
    setAttributes(parent.get());
  }
}

void DynamicObjectData::init() {
  if (!parent.isNull()) {
    parent->init();
  }
}

void DynamicObjectData::dynConstruct(CArrRef params) {
  if (!parent.isNull()) {
    parent->dynConstruct(params);
  }
}

void DynamicObjectData::getConstructor(MethodCallPackage &mcp) {
  if (!parent.isNull()) {
    parent->getConstructor(mcp);
  } else {
    ObjectData::getConstructor(mcp);
  }
}

void DynamicObjectData::destruct() {
  if (!parent.isNull()) {
    if (inCtorDtor()) {
      parent->setInDtor();
    }
    this->incRefCount();
    parent.reset();
    this->decRefCount();
  }
}

void DynamicObjectData::setRoot(ObjectData *r) {
  root = r;
  if (!parent.isNull()) {
    parent->setRoot(r);
  }
}
ObjectData *DynamicObjectData::getRoot() {
  return root;
}

void DynamicObjectData::cloneSet(ObjectData *clone) {
  if (!parent.isNull()) {
    parent->cloneSet(static_cast<DynamicObjectData*>(clone)->parent.get());
  }
  ObjectData::cloneSet(clone);
}

///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

Variant *DynamicObjectData::o_realProp(
  CStrRef propName, int flags, CStrRef context /* = null_string */) const {
  if (!parent.isNull()) {
    return parent->o_realProp(propName, flags, context);
  } else {
    return ObjectData::o_realProp(propName, flags, context);
  }
}

Variant *DynamicObjectData::o_realPropPublic(CStrRef propName,
                                             int flags) const {
  if (!parent.isNull()) {
    return parent->o_realPropPublic(propName, flags);
  } else {
    return ObjectData::o_realPropPublic(propName, flags);
  }
}

void DynamicObjectData::o_getArray(Array &props, bool pubOnly /* = false */)
const {
  if (!parent.isNull()) {
    return parent->o_getArray(props, pubOnly);
  } else {
    return ObjectData::o_getArray(props, pubOnly);
  }
}

void DynamicObjectData::o_setArray(CArrRef props) {
  if (!parent.isNull()) {
    return parent->o_setArray(props);
  } else {
    return ObjectData::o_setArray(props);
  }
}

Array DynamicObjectData::o_toArray() const {
  if (!parent.isNull()) {
    return parent->o_toArray();
  } else {
    return ObjectData::o_toArray();
  }
}

Array DynamicObjectData::o_getDynamicProperties() const {
  if (!parent.isNull()) {
    return parent->o_getDynamicProperties();
  } else {
    return ObjectData::o_getDynamicProperties();
  }
}

bool DynamicObjectData::o_get_call_info(MethodCallPackage &info,
    int64 hash /* = -1*/) {
  if (!parent.isNull()) {
    return parent->o_get_call_info(info, hash);
  } else {
    info.obj = this;
    return ObjectData::o_get_call_info(info, hash);
  }
}

bool DynamicObjectData::o_get_call_info_ex(const char *clsname,
      MethodCallPackage &info, int64 hash /* = -1 */) {
  if (strcasecmp(o_getClassName(), clsname) == 0) {
    return o_get_call_info(info, hash);
  } else if(!parent.isNull()) {
    return parent->o_get_call_info_ex(clsname, info, hash);
  } else {
    return ObjectData::o_get_call_info_ex(clsname, info, hash);
  }
}

Variant DynamicObjectData::doCall(Variant v_name, Variant v_arguments,
                                  bool fatal) {
  if (!parent.isNull()) {
    return parent->doCall(v_name, v_arguments, fatal);
  } else {
    return ObjectData::doCall(v_name, v_arguments, fatal);
  }
}

Variant DynamicObjectData::doRootCall(Variant v_name, Variant v_arguments,
                                      bool fatal) {
  return root->doCall(v_name, v_arguments, fatal);
}

///////////////////////////////////////////////////////////////////////////////
// magic methods that user classes can override, and these are default handlers
// or actions to take:

Variant DynamicObjectData::t___destruct() {
  if (!parent.isNull()) {
    return parent->t___destruct();
  } else {
    return ObjectData::t___destruct();
  }
}
Variant DynamicObjectData::t___set(Variant v_name, Variant v_value) {
  if (!parent.isNull()) {
    return parent->t___set(v_name, withRefBind(v_value));
  } else {
    return ObjectData::t___set(v_name, withRefBind(v_value));
  }
}

Variant DynamicObjectData::t___get(Variant v_name) {
  if (!parent.isNull()) {
    return parent->t___get(v_name);
  } else {
    return ObjectData::t___get(v_name);
  }
}

bool DynamicObjectData::t___isset(Variant v_name) {
  if (!parent.isNull()) {
    return parent->t___isset(v_name);
  } else {
    return ObjectData::t___isset(v_name);
  }
}

Variant DynamicObjectData::t___unset(Variant v_name) {
  if (!parent.isNull()) {
    return parent->t___unset(v_name);
  } else {
    return ObjectData::t___unset(v_name);
  }
}

Variant DynamicObjectData::t___sleep() {
  if (!parent.isNull()) {
    Variant ret = parent->t___sleep();
    if (!parent->getAttribute(HasSleep)) {
      // When the parent also has the default implementation in ObjectData,
      // the attribute HasSleep should be cleared both in the parent and the
      // current object.
      clearAttribute(HasSleep);
    }
    return ret;
  } else {
    return ObjectData::t___sleep();
  }
}

Variant DynamicObjectData::t___wakeup() {
  if (!parent.isNull()) {
    return parent->t___wakeup();
  } else {
    return ObjectData::t___wakeup();
  }
}

String DynamicObjectData::t___tostring() {
  if (!parent.isNull()) {
    return parent->t___tostring();
  } else {
    return ObjectData::t___tostring();
  }
}

Variant DynamicObjectData::t___clone() {
  if (!parent.isNull()) {
    return parent->t___clone();
  } else {
    return ObjectData::t___clone();
  }
}

Variant &DynamicObjectData::___offsetget_lval(Variant v_name) {
  if (!parent.isNull()) {
    return parent->___offsetget_lval(v_name);
  } else {
    return ObjectData::___offsetget_lval(v_name);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
