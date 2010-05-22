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

#include <runtime/eval/runtime/eval_object_data.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/runtime/eval_state.h>

namespace HPHP {
namespace Eval {

/////////////////////////////////////////////////////////////////////////////
// constructor/destructor

IMPLEMENT_OBJECT_ALLOCATION_CLS(HPHP::Eval,EvalObjectData)

EvalObjectData::EvalObjectData(ClassEvalState &cls, const char* pname,
                               ObjectData* r /* = NULL */)
: DynamicObjectData(pname, r ? r : this), m_cls(cls) {
  if (pname) setRoot(root); // For ext classes
  if (r == NULL) {
    RequestEvalState::registerObject(this);
  }
}
// Only used for cloning and so should not register object
EvalObjectData::EvalObjectData(ClassEvalState &cls) :
  DynamicObjectData(NULL, this), m_cls(cls) {
}

ObjectData *EvalObjectData::dynCreate(CArrRef params, bool ini /* = true */) {
  init();
  if (ini) {
    dynConstruct(params);
  }
  return this;
}

void EvalObjectData::init() {
  m_cls.getClass()->initializeObject(this);
  DynamicObjectData::init();
}

void EvalObjectData::dynConstruct(CArrRef params) {
  const MethodStatement *ms = m_cls.getConstructor();
  if (ms) {
    ms->invokeInstance(Object(root), params);
  } else {
    DynamicObjectData::dynConstruct(params);
  }
}

void EvalObjectData::dynConstructFromEval(VariableEnvironment &env,
                                          const FunctionCallExpression *call) {
  const MethodStatement *ms = m_cls.getConstructor();
  if (ms) {
    ms->invokeInstanceDirect(Object(root), env, call);
  } else {
    DynamicObjectData::dynConstructFromEval(env, call);
  }
}

void EvalObjectData::destruct() {
  const MethodStatement *ms;
  incRefCount();
  if (!inCtorDtor() && (ms = getMethodStatement("__destruct"))) {
    setInDtor();
    try {
      ms->invokeInstance(Object(root), Array(), false);
    } catch (...) {
      handle_destructor_exception();
    }
  }
  DynamicObjectData::destruct();
  if (root == this) {
    RequestEvalState::deregisterObject(this);
  }
}

Array EvalObjectData::o_toArray() const {
  Array values(DynamicObjectData::o_toArray());
  Array props(Array::Create());
  m_cls.getClass()->toArray(props, values);
  if (!values.empty()) {
    props += values;
  }
  return props;
}

bool EvalObjectData::o_exists(CStrRef s, int64 hash,
    const char *context /* = NULL */) const {
  if (!context) {
    context = FrameInjection::GetClassName(false);
  }
  return (m_privates.exists(context) &&
      m_privates.rvalAt(context).getArrayData()->exists(s, hash)) ||
    DynamicObjectData::o_exists(s, hash, context);
}

void EvalObjectData::o_get(Array &props) const {
  String zero("\0", 1, AttachLiteral);
  for (ArrayIter it(m_privates); !it.end(); it.next()) {
    String prefix(zero);
    prefix += it.first();
    prefix += zero;
    for (ArrayIter it2(it.second()); !it2.end(); it2.next()) {
      CVarRef v = it2.secondRef();
      props.set(prefix + it2.first(), v.isReferenced() ? ref(v) : v);
    }
  }
  DynamicObjectData::o_get(props);
}

Variant EvalObjectData::o_get(CStrRef s, int64 hash, bool error /* = true */,
    const char *context /* = NULL */) {
  if (!context) {
    context = FrameInjection::GetClassName(false);
  }
  Variant priv = m_privates.rvalAt(context);
  if (priv.is(KindOfArray) && priv.getArrayData()->exists(s, hash)) {
    return priv.rvalAt(s);
  }
  m_cls.getClass()->attemptPropertyAccess(this, s, context);
  return DynamicObjectData::o_get(s, hash, error);
}

Variant EvalObjectData::o_getUnchecked(CStrRef s, int64 hash,
    const char *context /* = NULL */) {
  Variant priv = m_privates.rvalAt(context);
  if (priv.is(KindOfArray) && priv.getArrayData()->exists(s, hash)) {
    return priv.rvalAt(s);
  }
  return DynamicObjectData::o_get(s, hash, true, context);
}


Variant &EvalObjectData::o_lval(CStrRef s, int64 hash,
    const char *context /* = NULL */) {
  if (!context) {
    context = FrameInjection::GetClassName(false);
  }
  if (m_privates.exists(context)) {
    Variant &priv = m_privates.lvalAt(context);
    if (priv.getArrayData()->exists(s, hash)) {
      return priv.lvalAt(s, hash);
    }
  }
  m_cls.getClass()->attemptPropertyAccess(this, s, context);
  return DynamicObjectData::o_lval(s, hash, context);
}

Variant EvalObjectData::o_set(CStrRef s, int64 hash, CVarRef v,
    bool forInit /*  = false */, const char *context /* = NULL */) {
  if (!context) {
    context = FrameInjection::GetClassName(false);
  }
  if (m_privates.exists(context)) {
    Variant &priv = m_privates.lvalAt(context);
    if (priv.is(KindOfArray) && priv.getArrayData()->exists(s, hash)) {
      return priv.set(s, v, hash);
    }
  }
  if (!forInit) m_cls.getClass()->
    attemptPropertyAccess(this, s, FrameInjection::GetClassName(false));
  return DynamicObjectData::o_set(s, hash, v, forInit, context);
}

void EvalObjectData::o_setPrivate(const char *cls, const char *s, int64 hash,
    CVarRef v) {
  m_privates.lvalAt(cls).set(s, v, hash);
}

const char *EvalObjectData::o_getClassName() const {
  return m_cls.getClass()->name().c_str();
}

const MethodStatement
*EvalObjectData::getMethodStatement(const char* name) const {
  const hphp_const_char_imap<const MethodStatement*> &meths =
    m_cls.getMethodTable();
  hphp_const_char_imap<const MethodStatement*>::const_iterator it =
    meths.find(name);
  if (it != meths.end()) {
    return it->second;
  }
  return NULL;
}

bool EvalObjectData::o_instanceof(const char *s) const {
  return m_cls.getClass()->subclassOf(s) ||
    (!parent.isNull() && parent->o_instanceof(s));
}

Variant EvalObjectData::o_invoke(const char *s, CArrRef params, int64 hash,
                                 bool fatal /* = true */) {
  const hphp_const_char_imap<const MethodStatement*> &meths =
    m_cls.getMethodTable();
  hphp_const_char_imap<const MethodStatement*>::const_iterator it =
    meths.find(s);
  if (it != meths.end()) {
    if (it->second) {
      return it->second->invokeInstance(Object(root), params);
    } else {
      return DynamicObjectData::o_invoke(s, params, hash, fatal);
    }
  } else {
    return doCall(s, params, fatal);
  }
}

Variant EvalObjectData::o_invoke_ex(const char *clsname, const char *s,
                                       CArrRef params, int64 hash,
                                       bool fatal /* = false */) {
  if (m_cls.getClass()->subclassOf(clsname)) {
    bool foundClass;
    const MethodStatement *ms = RequestEvalState::findMethod(clsname, s,
                                                             foundClass);
    if (ms) {
      return ms->invokeInstance(Object(root), params);
    } else {
      // Possibly builtin class has this method
      const hphp_const_char_imap<const MethodStatement*> &meths =
        m_cls.getMethodTable();
      if (meths.find(s) == meths.end()) {
        // Absolutely nothing in the hierarchy has this method
        return doCall(s, params, fatal);
      }
    }
    // Know it's a builtin parent so no need for _ex
    return DynamicObjectData::o_invoke(s, params, hash, fatal);
  }
  return DynamicObjectData::o_invoke_ex(clsname, s, params, hash, fatal);
}

Variant EvalObjectData::o_invoke_few_args(const char *s, int64 hash, int count,
    INVOKE_FEW_ARGS_IMPL_ARGS) {
  switch (count) {
  case 0: {
    return o_invoke(s, Array(), hash);
  }
  case 1: {
    Array params(ArrayInit(1, true).set(0, a0).create());
    return o_invoke(s, params, hash);
  }
  case 2: {
    Array params(ArrayInit(2, true).set(0, a0).set(1, a1).create());
    return o_invoke(s, params, hash);
  }
  case 3: {
    Array params(ArrayInit(3, true).set(0, a0).set(1, a1).set(2, a2).create());
    return o_invoke(s, params, hash);
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    Array params(ArrayInit(4, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).create());
    return o_invoke(s, params, hash);
  }
  case 5: {
    Array params(ArrayInit(5, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).create());
    return o_invoke(s, params, hash);
  }
  case 6: {
    Array params(ArrayInit(6, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).create());
    return o_invoke(s, params, hash);
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    Array params(ArrayInit(7, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).create());
    return o_invoke(s, params, hash);
  }
  case 8: {
    Array params(ArrayInit(8, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).create());
    return o_invoke(s, params, hash);
  }
  case 9: {
    Array params(ArrayInit(9, true).set(0, a0).set(1, a1).set(2, a2).
                                    set(3, a3).set(4, a4).set(5, a5).
                                    set(6, a6).set(7, a7).set(8, a8).create());
    return o_invoke(s, params, hash);
  }
  case 10: {
    Array params(ArrayInit(10, true).set(0, a0).set(1, a1).set(2, a2).
                                     set(3, a3).set(4, a4).set(5, a5).
                                     set(6, a6).set(7, a7).set(8, a8).
                                     set(9, a9).create());
    return o_invoke(s, params, hash);
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

Variant EvalObjectData::doCall(Variant v_name, Variant v_arguments,
                               bool fatal) {
  const MethodStatement *ms = getMethodStatement("__call");
  if (ms) {
    return ms->invokeInstance(Object(root),
                              CREATE_VECTOR2(v_name, v_arguments), false);
  } else {
    return DynamicObjectData::doCall(v_name, v_arguments, fatal);
  }
}

Variant EvalObjectData::doGet(Variant v_name, bool error) {
  const MethodStatement *ms = getMethodStatement("__get");
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::doGet(v_name, error);
  }
}

Variant EvalObjectData::t___destruct() {
  const MethodStatement *ms = getMethodStatement("__destruct");
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___destruct();
  }
}
Variant EvalObjectData::t___set(Variant v_name, Variant v_value) {
  if (v_value.isReferenced()) {
    v_value.setContagious();
  }
  const MethodStatement *ms = getMethodStatement("__set");
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR2(v_name, v_value),
        false);
  } else {
    return DynamicObjectData::t___set(v_name, v_value);
  }
}
Variant EvalObjectData::t___get(Variant v_name) {
  const MethodStatement *ms = getMethodStatement("__get");
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::t___get(v_name);
  }
}
bool EvalObjectData::t___isset(Variant v_name) {
  const MethodStatement *ms = getMethodStatement("__isset");
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::t___isset(v_name);
  }
}
Variant EvalObjectData::t___unset(Variant v_name) {
  const MethodStatement *ms = getMethodStatement("__unset");
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::t___unset(v_name);
  }
}

bool EvalObjectData::php_sleep(Variant &ret) {
  ret = t___sleep();
  return getMethodStatement("__sleep");
}

Variant EvalObjectData::t___sleep() {
  const MethodStatement *ms = getMethodStatement("__sleep");
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___sleep();
  }
}
Variant EvalObjectData::t___wakeup() {
  const MethodStatement *ms = getMethodStatement("__wakeup");
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___wakeup();
  }
}
Variant EvalObjectData::t___set_state(Variant v_properties) {
  const MethodStatement *ms = getMethodStatement("__set_state");
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_properties),
        false);
  } else {
    return DynamicObjectData::t___set_state(v_properties);
  }
}
String EvalObjectData::t___tostring() {
  const MethodStatement *ms = getMethodStatement("__tostring");
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___tostring();
  }
}
Variant EvalObjectData::t___clone() {
  const MethodStatement *ms = getMethodStatement("__clone");
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___clone();
  }
}

ObjectData* EvalObjectData::cloneImpl() {
  EvalObjectData *e = NEW(EvalObjectData)(m_cls);
  if (!parent.isNull()) {
    e->setParent(parent->clone());
  } else {
    cloneSet(e);
  }
  e->m_privates = m_privates;
  // Registration is done here because the clone constructor is not
  // passed root.
  if (root == this) {
    RequestEvalState::registerObject(e);
  }
  return e;
}

Variant &EvalObjectData::___lval(Variant v_name) {
  const MethodStatement *ms = getMethodStatement("__get");
  if (ms) {
    Variant &v = get_globals()->__lvalProxy;
    v = ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
    return v;
  } else {
    return DynamicObjectData::___lval(v_name);
  }
}
Variant &EvalObjectData::___offsetget_lval(Variant v_name) {
  const MethodStatement *ms = getMethodStatement("offsetget");
  if (ms) {
    Variant &v = get_globals()->__lvalProxy;
    v = ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
    return v;
  } else {
    return DynamicObjectData::___offsetget_lval(v_name);
  }
}


///////////////////////////////////////////////////////////////////////////////
}
}
