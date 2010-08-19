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

#include <runtime/base/dynamic_object_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/externals.h>
#include <runtime/base/runtime_option.h>
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

void DynamicObjectData::
dynConstructFromEval(Eval::VariableEnvironment &env,
                    const Eval::FunctionCallExpression *call) {
  if (!parent.isNull()) {
    parent->dynConstructFromEval(env, call);
  }
}

void DynamicObjectData::destruct() {
  if (!parent.isNull()) {
    if (inCtorDtor()) {
      parent->setInDtor();
    }
    parent = Object();
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

ObjectData* DynamicObjectData::clone() {
  ObjectData *clone = cloneImpl();
  clone->setRoot(clone);
  return clone;
}


///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

bool DynamicObjectData::o_exists(CStrRef propName,
                                 CStrRef context /* = null_string */) const {
  if (!parent.isNull()) {
    return parent->o_exists(propName, context);
  } else {
    return ObjectData::o_exists(propName, context);
  }
}

Variant DynamicObjectData::o_get(CStrRef propName, bool error /* = true */,
                                 CStrRef context /* = null_string */) {
  if (!parent.isNull()) {
    return parent->o_get(propName, error, context);
  } else {
    if (propName.size() == 0) {
      return null;
    }
    // property names are definitely strings
    if (o_properties && o_properties->exists(propName, -1, true)) {
      return o_properties->rvalAt(propName, -1, false, true);
    }
    if (root->getAttribute(InGet)) {
      return ObjectData::doGet(propName, error);
    } else {
      AttributeSetter a(InGet, root);
      return root->doGet(propName, error);
    }
  }
}

void DynamicObjectData::o_getArray(Array &props) const {
  if (!parent.isNull()) {
    return parent->o_getArray(props);
  } else {
    return ObjectData::o_getArray(props);
  }
}

void DynamicObjectData::o_setArray(CArrRef props) {
  if (!parent.isNull()) {
    return parent->o_setArray(props);
  } else {
    return ObjectData::o_setArray(props);
  }
}

Variant DynamicObjectData::o_set(CStrRef propName, CVarRef v,
    bool forInit /* = false */, CStrRef context /* = null_string */) {
  if (!parent.isNull()) {
    return parent->o_set(propName, v, forInit, context);
  } else {
    if (propName.size() == 0) {
      throw EmptyObjectPropertyException();
    }
    if (o_properties && o_properties->exists(propName, -1, true)) {
      o_properties->set(propName, v, -1, true);
      return v;
    }
    if (forInit || root->getAttribute(InSet)) {
      return ObjectData::t___set(propName, v);
    } else {
      AttributeSetter a(InSet, root);
      return root->t___set(propName, v);
    }
  }
}

Variant &DynamicObjectData::o_lval(CStrRef propName,
                                   CStrRef context /* = null_string */) {
  if (!parent.isNull()) {
    return parent->o_lval(propName, context);
  } else {
    if (o_properties && o_properties->exists(propName, -1, true)) {
      return o_properties->lvalAt(propName, -1, false, true);
    }
    return root->___lval(propName);
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

Variant DynamicObjectData::o_invoke(MethodIndex methodIndex,
                                    const char *s, CArrRef params, int64 hash,
                                    bool fatal /* = false */) {
  if (!parent.isNull()) {
    return parent->o_invoke(methodIndex, s, params, hash, fatal);
  } else {
    // FMC need test case
    if (RuntimeOption::FastMethodCall) {
      s = g_bypassMILR ? s : methodIndexLookupReverse(methodIndex);
    }
    return root->doCall(s, params, fatal);
  }
}
Variant DynamicObjectData::o_invoke_mil(const char *s, CArrRef params,
                                        int64 hash,
                                        bool fatal /* = false */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return root->doCall(s, params, fatal);
    }
  }
  return o_invoke( methodIndex, s, params, hash, fatal);
}

Variant DynamicObjectData::o_invoke_ex(const char *clsname,
                                       MethodIndex methodIndex,
                                       const char *s,
                                       CArrRef params, int64 hash,
                                       bool fatal /* = false */) {
  if (strcasecmp(o_getClassName(), clsname) == 0) {
    return o_invoke(methodIndex, s, params, hash );
  } else if(!parent.isNull()) {
    return parent->o_invoke_ex(clsname, methodIndex, s, params, hash, fatal);
  } else {
    return ObjectData::o_invoke_ex(clsname, methodIndex, s, params, hash,
                                   fatal);
  }
}

Variant DynamicObjectData::o_invoke_ex_mil(const char *clsname,
                                           const char *s,
                                           CArrRef params, int64 hash,
                                           bool fatal /* = false */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return root->doCall(s, params, fatal);
    }
  }
  return o_invoke_ex(clsname, methodIndex, s, params, hash, fatal);
}

Variant DynamicObjectData::o_invoke_few_args(MethodIndex methodIndex,
                                             const char *s, int64 hash,
                                             int count,
                                             INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (!parent.isNull()) {
    return parent->o_invoke_few_args(methodIndex, s, hash, count,
        INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    switch (count) {
    case 0: {
      return DynamicObjectData::o_invoke(methodIndex, s, Array(), hash);
    }
    case 1: {
      Array params(ArrayInit(1, true).set(0, a0).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash);
    }
    case 2: {
      Array params(ArrayInit(2, true).set(0, a0).set(1, a1).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash );
    }
    case 3: {
      Array params(ArrayInit(3, true).set(0, a0).set(1, a1).set(2, a2).
                                      create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash );
    }
#if INVOKE_FEW_ARGS_COUNT > 3
    case 4: {
      Array params(ArrayInit(4, true).set(0, a0).set(1, a1).set(2, a2).
                                      set(3, a3).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash );
    }
    case 5: {
      Array params(ArrayInit(5, true).set(0, a0).set(1, a1).set(2, a2).
                                      set(3, a3).set(4, a4).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash );
    }
    case 6: {
      Array params(ArrayInit(6, true).set(0, a0).set(1, a1).set(2, a2).
                                      set(3, a3).set(4, a4).set(5, a5).
                                      create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash);
    }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
    case 7: {
      Array params(ArrayInit(7, true).set(0, a0).set(1, a1).set(2, a2).
                                      set(3, a3).set(4, a4).set(5, a5).
                                      set(6, a6).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash);
    }
    case 8: {
      Array params(ArrayInit(8, true).set(0, a0).set(1, a1).set(2, a2).
                                      set(3, a3).set(4, a4).set(5, a5).
                                      set(6, a6).set(7, a7).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash);
    }
    case 9: {
      Array params(ArrayInit(9, true).set(0, a0).set(1, a1).set(2, a2).
                                      set(3, a3).set(4, a4).set(5, a5).
                                      set(6, a6).set(7, a7).set(8, a8).
                                      create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash);
    }
    case 10: {
      Array params(ArrayInit(10, true).set(0, a0).set(1, a1).set(2, a2).
                                       set(3, a3).set(4, a4).set(5, a5).
                                       set(6, a6).set(7, a7).set(8, a8).
                                       set(9, a9).create());
      return DynamicObjectData::o_invoke(methodIndex, s, params, hash);
    }
#endif
    default:
      ASSERT(false);
    }
    return null;
  }
}

Variant DynamicObjectData::o_invoke_few_args_mil(const char *s, int64 hash,
                                                 int count,
                                                 INVOKE_FEW_ARGS_IMPL_ARGS) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return root->doCall(s, collectArgs(count, INVOKE_FEW_ARGS_PASS_ARGS),
                          true);
    }
  }
  return o_invoke_few_args(methodIndex, s, hash, count,
                           INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant
DynamicObjectData::o_root_invoke_few_args(MethodIndex methodIndex,
                                          const char *s, int64 hash, int count,
                                          INVOKE_FEW_ARGS_IMPL_ARGS) {
  if (root != this) {
    return root->o_invoke_few_args(methodIndex, s, hash, count,
      INVOKE_FEW_ARGS_PASS_ARGS);
  } else {
    return o_invoke_few_args(methodIndex, s, hash, count,
      INVOKE_FEW_ARGS_PASS_ARGS);
  }
}

Variant
DynamicObjectData::o_root_invoke_few_args_mil(const char *s, int64 hash,
                                              int count,
                                              INVOKE_FEW_ARGS_IMPL_ARGS) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return root->doCall(s, collectArgs(count, INVOKE_FEW_ARGS_PASS_ARGS),
                          true);
    }
  }
  return o_root_invoke_few_args(methodIndex, s, hash, count,
                                INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant DynamicObjectData::o_root_invoke(MethodIndex methodIndex,
                                         const char *s, CArrRef params,
                                         int64 hash, bool fatal /* = false */) {
  if (root != this) {
    return root->o_root_invoke(methodIndex, s, params, hash, fatal);
  } else {
    return o_invoke(methodIndex, s, params, hash, fatal);
  }
}

Variant DynamicObjectData::o_root_invoke_mil(const char *s, CArrRef params,
                                             int64 hash,
                                             bool fatal /* = false */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      return root->doCall(s, params, fatal);
    }
  }
  return o_root_invoke(methodIndex, s, params, hash, fatal) ;
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


Variant DynamicObjectData::doGet(Variant v_name, bool error) {
  if (!parent.isNull()) {
    return parent->doGet(v_name, error);
  } else {
    return ObjectData::doGet(v_name, error);
  }
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
  if (v_value.isReferenced()) {
    v_value.setContagious();
  }
  if (!parent.isNull()) {
    return parent->t___set(v_name, v_value);
  } else {
    return ObjectData::t___set(v_name, v_value);
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

Variant DynamicObjectData::t___set_state(Variant v_properties) {
  if (!parent.isNull()) {
    return parent->t___set_state(v_properties);
  } else {
    return ObjectData::t___set_state(v_properties);
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

Variant &DynamicObjectData::___lval(Variant v_name) {
  if (!parent.isNull()) {
    return parent->___lval(v_name);
  } else {
    return ObjectData::___lval(v_name);
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
