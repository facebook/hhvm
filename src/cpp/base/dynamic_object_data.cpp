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

#include <cpp/base/dynamic_object_data.h>
#include <cpp/base/externals.h>
#include <util/util.h>

namespace HPHP {

/////////////////////////////////////////////////////////////////////////////
// constructor/destructor

DynamicObjectData::DynamicObjectData(const char* pname,
                                     ObjectData* r /* = NULL */) :
  root(r ? r : this) {
  if (pname) parent = create_object(pname, Array(), false, root);
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

ObjectData* DynamicObjectData::clone() {
  ObjectData *clone = cloneImpl();
  clone->setRoot(clone);
  return clone;
}


///////////////////////////////////////////////////////////////////////////////
// instance methods and properties

bool DynamicObjectData::o_exists(CStrRef propName, int64 hash) const {
  if (!parent.isNull()) {
    return parent->o_exists(propName, hash);
  } else {
    return ObjectData::o_exists(propName, hash);
  }
}

Variant DynamicObjectData::o_get(CStrRef propName, int64 hash) {
  if (!parent.isNull()) {
    return parent->o_get(propName, hash);
  } else {
    if (propName.size() == 0) {
      return null;
    }
    if (o_properties && o_properties->exists(propName)) {
      return o_properties->rvalAt(propName);
    }
    return root->t___get(propName);
  }
}

void DynamicObjectData::o_get(std::vector<ArrayElement *> &props) const {
  if (!parent.isNull()) {
    return parent->o_get(props);
  } else {
    return ObjectData::o_get(props);
  }
}

Variant DynamicObjectData::o_set(CStrRef propName, int64 hash, CVarRef v,
                                 bool forInit /* = false */) {
  if (!parent.isNull()) {
    return parent->o_set(propName, hash, v, forInit);
  } else {
    if (propName.size() == 0) {
      throw EmptyObjectPropertyException();
    }
    if (o_properties && o_properties->exists(propName)) {
      o_properties->set(propName, v);
      return v;
    }
    if (forInit) {
      return ObjectData::t___set(propName, v);
    } else {
      return root->t___set(propName, v);
    }
  }
}

Variant &DynamicObjectData::o_lval(CStrRef propName, int64 hash) {
  if (!parent.isNull()) {
    return parent->o_lval(propName, hash);
  } else {
    if (o_properties && o_properties->exists(propName)) {
      return o_properties->lvalAt(propName, hash);
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

Variant DynamicObjectData::o_invoke(const char *s, CArrRef params, int64 hash,
                                    bool fatal /* = false */) {
  if (!parent.isNull()) {
    return parent->o_invoke(s, params, hash, fatal);
  } else {
    return root->doCall(s, params, fatal);
  }
}

Variant DynamicObjectData::o_invoke_ex(const char *clsname, const char *s,
                                       CArrRef params, int64 hash,
                                       bool fatal /* = false */) {
  if (strcasecmp(o_getClassName(), clsname) == 0) {
    return o_invoke(s, params, hash);
  } else if(!parent.isNull()) {
    return parent->o_invoke_ex(clsname, s, params, hash, fatal);
  } else {
    return ObjectData::o_invoke_ex(clsname, s, params, hash, fatal);
  }
}

Variant DynamicObjectData::o_invoke_few_args(const char *s, int64 hash, int count,
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
  if (!parent.isNull()) {
    return parent->o_invoke_few_args(s, hash, count, a0, a1, a2
#if INVOKE_FEW_ARGS_COUNT > 3
                                     ,a3, a4, a5
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                                     ,a6, a7, a8, a9
#endif
                                     );
  } else {
    switch (count) {
    case 0: {
      return DynamicObjectData::o_invoke(s, Array(), hash);
    }
    case 1: {
      Array params(NEW(ArrayElement)(a0), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 2: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 3: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
#if INVOKE_FEW_ARGS_COUNT > 3
    case 4: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 5: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                   NEW(ArrayElement)(a4), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 6: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                   NEW(ArrayElement)(a4), NEW(ArrayElement)(a5), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
    case 7: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                   NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                   NEW(ArrayElement)(a6), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 8: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                   NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                   NEW(ArrayElement)(a6), NEW(ArrayElement)(a7), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 9: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                   NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                   NEW(ArrayElement)(a6), NEW(ArrayElement)(a7),
                   NEW(ArrayElement)(a8), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
    case 10: {
      Array params(NEW(ArrayElement)(a0), NEW(ArrayElement)(a1),
                   NEW(ArrayElement)(a2), NEW(ArrayElement)(a3),
                   NEW(ArrayElement)(a4), NEW(ArrayElement)(a5),
                   NEW(ArrayElement)(a6), NEW(ArrayElement)(a7),
                   NEW(ArrayElement)(a8), NEW(ArrayElement)(a9), NULL);
      return DynamicObjectData::o_invoke(s, params, hash);
    }
#endif
    default:
      ASSERT(false);
    }
    return null;
  }
}

Variant DynamicObjectData::o_root_invoke(const char *s, CArrRef params,
                                         int64 hash, bool fatal /* = false */) {
  if (root != this) {
    return root->o_root_invoke(s, params, hash, fatal);
  } else {
    return o_invoke(s, params, hash, fatal);
  }
}
Variant
DynamicObjectData::o_root_invoke_few_args(const char *s, int64 hash, int count,
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
  if (root != this) {
    return root->o_invoke_few_args(s, hash, count, a0, a1, a2
#if INVOKE_FEW_ARGS_COUNT > 3
                                   ,a3, a4, a5
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                                   ,a6, a7, a8, a9
#endif
                                   );
  } else {
    return o_invoke_few_args(s, hash, count, a0, a1, a2
#if INVOKE_FEW_ARGS_COUNT > 3
                             ,a3, a4, a5
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                             ,a6, a7, a8, a9
#endif
                             );
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
    return parent->t___sleep();
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
