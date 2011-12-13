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
#include <runtime/eval/runtime/eval_object_data.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/runtime/eval_state.h>

namespace HPHP {
namespace Eval {

static StaticString s___destruct("__destruct");
static StaticString s___invoke("__invoke");
static StaticString s___call("__call");
static StaticString s___set("__set");
static StaticString s___get("__get");
static StaticString s___isset("__isset");
static StaticString s___unset("__unset");
static StaticString s___sleep("__sleep");
static StaticString s___wakeup("__wakeup");
static StaticString s___tostring("__tostring");
static StaticString s___clone("__clone");
static StaticString s_offsetget("offsetget");

/////////////////////////////////////////////////////////////////////////////
// constructor/destructor

IMPLEMENT_OBJECT_ALLOCATION_CLS(HPHP::Eval,EvalObjectData)

EvalObjectData::EvalObjectData(ClassEvalState &ce, const char* pname,
                               ObjectData* r /* = NULL */)
: DynamicObjectData(0, pname, r ? r : this), m_ce(ce) {
  if (pname) setRoot(root); // For ext classes
  if (r == NULL) {
    RequestEvalState::registerObject(this);
  }
  setAttributes(m_ce.getAttributes());

  // an object can never live longer than its class
  m_class_name = m_ce.getClass()->name();

  // seems to require both
  m_invokeMcp.obj = m_invokeMcp.rootObj = this;
  // so that getClassName() will work
  m_invokeMcp.isObj = true;
}

EvalObjectData::EvalObjectData(EvalObjectData *original) :
    DynamicObjectData(0, 0, this), m_ce(original->m_ce) {

  RequestEvalState::registerObject(this);

  setAttributes(m_ce.getAttributes());

  // an object can never live longer than its class
  m_class_name = m_ce.getClass()->name();

  // seems to require both
  m_invokeMcp.obj = m_invokeMcp.rootObj = this;
  // so that getClassName() will work
  m_invokeMcp.isObj = true;

  if (ObjectData *p = original->parent.get()) {
    CountableHelper h(this);
    parent = p->clone();
    parent->setRoot(this);
    setAttributes(parent.get());
  }
  m_privates = original->m_privates;
  cloneDynamic(original);
}

void EvalObjectData::init() {
  m_ce.getClass()->initializeObject(m_ce, this);
  DynamicObjectData::init();
}

void EvalObjectData::getConstructor(MethodCallPackage &mcp) {
  const MethodStatement *ms = m_ce.getConstructor();
  if (ms) {
    mcp.extra = (void*)ms;
    mcp.obj = this;
    mcp.ci = ms->getCallInfo();
  } else {
    DynamicObjectData::getConstructor(mcp);
  }
}

void EvalObjectData::destruct() {
  const MethodStatement *ms;
  incRefCount();
  int access = 0;
  if (!inCtorDtor() && (ms = getMethodStatement(s___destruct, access))) {
    setInDtor();
    try {
      ms->invokeInstance(Object(root), Array(), false);
    } catch (...) {
      handle_destructor_exception();
    }
  }
  decRefCount();
  if (root == this) {
    if (LIKELY(getCount() == 0)) {
      RequestEvalState::deregisterObject(this);
    }
  }
}

Array EvalObjectData::o_toArray() const {
  Array values(DynamicObjectData::o_toArray());
  Array props(Array::Create());
  m_ce.getClass()->toArray(props, values);
  if (!values.empty()) {
    props += values;
  }
  return props;
}

Variant *EvalObjectData::o_realPropHook(
  CStrRef s, int flags, CStrRef context /* = null_string */) const {
  CStrRef c = context.isNull() ? FrameInjection::GetClassName(false) : context;
  if (Variant *priv =
      const_cast<Array&>(m_privates).lvalPtr(c, flags & RealPropWrite, false)) {
    if (Variant *ret = priv->lvalPtr(s, flags & RealPropWrite, false)) {
      return ret;
    }
  }
  int mods;
  if (!(flags & RealPropUnchecked) &&
      !m_ce.getClass()->attemptPropertyAccess(s, c, mods)) {
    return NULL;
  }
  if (parent.get()) return parent->o_realProp(s, flags);
  if (Variant *ret = ObjectData::o_realPropHook(s, flags, context)) {
    return ret;
  }

  if (g_context->getDebuggerBypassCheck()) {
    for (ArrayIter it(m_privates); !it.end(); it.next()) {
      // iterating up to the base class chain
      Variant *ret = it.second().lvalPtr(s, flags & RealPropWrite, false);
      if (ret) {
        return ret;
      }
    }
  }
  return NULL;
}

Variant EvalObjectData::o_getError(CStrRef prop, CStrRef context) {
  CStrRef c = context.isNull() ? FrameInjection::GetClassName(false) : context;
  int mods;
  if (!m_ce.getClass()->attemptPropertyAccess(prop, c, mods)) {
    m_ce.getClass()->failPropertyAccess(prop, c, mods);
  } else {
    DynamicObjectData::o_getError(prop, context);
  }
  return null;
}

Variant EvalObjectData::o_setError(CStrRef prop, CStrRef context) {
  CStrRef c = context.isNull() ? FrameInjection::GetClassName(false) : context;
  int mods;
  if (!m_ce.getClass()->attemptPropertyAccess(prop, c, mods)) {
    m_ce.getClass()->failPropertyAccess(prop, c, mods);
  }
  return null;
}

void EvalObjectData::o_getArray(Array &props, bool pubOnly /* = false */)
const {
  if (!pubOnly) {
    String zero("\0", 1, AttachLiteral);
    for (ArrayIter it(m_privates); !it.end(); it.next()) {
      String prefix(zero);
      prefix += it.first();
      prefix += zero;
      for (ArrayIter it2(it.second()); !it2.end(); it2.next()) {
        CVarRef v = it2.secondRef();
        if (v.isInitialized()) {
          props.lvalAt(prefix + it2.first()).setWithRef(v);
        }
      }
    }
  }
  DynamicObjectData::o_getArray(props, pubOnly);
  if (pubOnly) {
    const ClassInfo *info = ClassInfo::FindClass(o_getClassName());
    info->filterProperties(props, ClassInfo::IsProtected);
  }
}

void EvalObjectData::o_setArray(CArrRef props) {
  for (ArrayIter it(props); !it.end(); it.next()) {
    String k = it.first();
    if (!k.empty() && k.charAt(0) == '\0') {
      int subLen = k.find('\0', 1) + 1;
      String cls = k.substr(1, subLen - 2);
      String key = k.substr(subLen);
      if (cls == "*") cls = o_getClassName();
      props->load(k, o_lval(key, Variant(), cls));
    }
  }
  DynamicObjectData::o_setArray(props);
}

void EvalObjectData::o_setPrivate(CStrRef cls, CStrRef s, CVarRef v) {
  m_privates.lvalAt(cls).set(s, v, true);
}

CStrRef EvalObjectData::o_getClassNameHook() const {
  return m_class_name;
}

const MethodStatement
*EvalObjectData::getMethodStatement(CStrRef name, int &access) const {
  return m_ce.getMethod(name, access);
}

const MethodStatement* EvalObjectData::getConstructorStatement() const {
  return m_ce.getConstructor();
}

bool EvalObjectData::o_instanceof_hook(CStrRef s) const {
  return m_ce.getClass()->subclassOf(s) ||
    (!parent.isNull() && parent->o_instanceof(s));
}

const CallInfo *EvalObjectData::t___invokeCallInfoHelper(void *&extra) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___invoke, access);
  if (LIKELY(ms != NULL)) {
    extra = (void*) &m_invokeMcp;
    m_invokeMcp.extra = (void*) ms;
    return ms->getCallInfo();
  }
  return DynamicObjectData::t___invokeCallInfoHelper(extra);
}

bool EvalObjectData::o_get_call_info_hook(const char *clsname,
                                          MethodCallPackage &mcp,
                                          int64 hash /* = -1 */) {
  if (!clsname) {
    const ClassEvalState::MethodTable &meths = m_ce.getMethodTable();
    ClassEvalState::MethodTable::const_iterator it =
      meths.find(*mcp.name);
    if (it != meths.end()) {
      if (it->second.first) {
        mcp.extra = (void*)it->second.first;
        mcp.ci = it->second.first->getCallInfo();
        return true;
      }
    }
    if (ObjectData *p = parent.get()) {
      return p->o_get_call_info(mcp, hash);
    }
  } else {
    if (m_ce.getClass()->subclassOf(clsname)) {
      ClassEvalState *ce;
      const MethodStatement *ms =
        RequestEvalState::findMethod(clsname, *(mcp.name), ce);
      if (ms) {
        mcp.extra = (void*)ms;
        mcp.obj = this;
        mcp.ci = ms->getCallInfo();
        return true;
      }
    }
    if (ObjectData *p = parent.get()) {
      return p->o_get_call_info_ex(clsname, mcp, hash);
    }
  }
  return false;
}

Variant EvalObjectData::doCall(Variant v_name, Variant v_arguments,
                               bool fatal) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___call, access);
  if (ms) {
    if (v_arguments.isNull()) {
      v_arguments = Array::Create();
    }
    return ms->invokeInstance(Object(root),
                              CREATE_VECTOR2(v_name, v_arguments), false);
  } else {
    return DynamicObjectData::doCall(v_name, v_arguments, fatal);
  }
}

Variant EvalObjectData::t___destruct() {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___destruct, access);
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___destruct();
  }
}
Variant EvalObjectData::t___set(Variant v_name, Variant v_value) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___set, access);
  if (ms) {
    return ms->invokeInstance(Object(root),
                              CREATE_VECTOR2(v_name, withRefBind(v_value)),
        false);
  } else {
    return DynamicObjectData::t___set(v_name, withRefBind(v_value));
  }
}
Variant EvalObjectData::t___get(Variant v_name) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___get, access);
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::t___get(v_name);
  }
}
bool EvalObjectData::t___isset(Variant v_name) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___isset, access);
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::t___isset(v_name);
  }
}
Variant EvalObjectData::t___unset(Variant v_name) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___unset, access);
  if (ms) {
    return ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
  } else {
    return DynamicObjectData::t___unset(v_name);
  }
}

bool EvalObjectData::php_sleep(Variant &ret) {
  ret = t___sleep();
  int access = 0;
  return getMethodStatement(s___sleep, access);
}

Variant EvalObjectData::t___sleep() {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___sleep, access);
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___sleep();
  }
}

Variant EvalObjectData::t___wakeup() {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___wakeup, access);
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___wakeup();
  }
}

String EvalObjectData::t___tostring() {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___tostring, access);
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___tostring();
  }
}
Variant EvalObjectData::t___clone() {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s___clone, access);
  if (ms) {
    return ms->invokeInstance(Object(root), Array(), false);
  } else {
    return DynamicObjectData::t___clone();
  }
}

ObjectData* EvalObjectData::clone() {
  return NEWOBJ(EvalObjectData)(this);
}

Variant &EvalObjectData::___offsetget_lval(Variant v_name) {
  int access = 0;
  const MethodStatement *ms = getMethodStatement(s_offsetget, access);
  if (ms) {
    Variant &v = get_globals()->__lvalProxy;
    v = ms->invokeInstance(Object(root), CREATE_VECTOR1(v_name), false);
    return v;
  } else {
    return DynamicObjectData::___offsetget_lval(v_name);
  }
}

Object EvalObjectData::fiberMarshal(FiberReferenceMap &refMap) const {
  ObjectData *px = (ObjectData *)refMap.lookup((void *)this);
  if (px) return px;
  Object ret = ObjectData::fiberMarshal(refMap);
  EvalObjectData *obj = dynamic_cast<EvalObjectData*>(ret.get());
  ASSERT(obj);
  obj->m_privates = m_privates.fiberMarshal(refMap);
  return ret;
}

Object EvalObjectData::fiberUnmarshal(FiberReferenceMap &refMap) const {
  ObjectData *px = (ObjectData *)refMap.lookup((void *)this);
  if (px) return px;
  Object ret = ObjectData::fiberUnmarshal(refMap);
  EvalObjectData *obj = dynamic_cast<EvalObjectData*>(ret.get());
  ASSERT(obj);
  obj->m_privates = m_privates.fiberUnmarshal(refMap);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
}
