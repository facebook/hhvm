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
  setId(r);
  if (pname) setRoot(root); // For ext classes
  if (r == NULL) {
    RequestEvalState::registerObject(this);
  }
  setAttributes(m_ce.getAttributes() | NoDestructor);

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

EvalObjectData::~EvalObjectData() {
  if (root == this) {
    RequestEvalState::deregisterObject(this);
  }
}

void EvalObjectData::init() {
  m_ce.getClass()->initializeObject(m_ce, this);
  DynamicObjectData::init();
}

void EvalObjectData::getConstructor(MethodCallPackage &mcp) {
  const MethodStatementWrapper *msw = &m_ce.getConstructorWrapper();
  if (msw->m_methodStatement) {
    mcp.extra = (void*)msw;
    mcp.obj = this;
    mcp.ci = msw->m_methodStatement->getCallInfo();
  } else {
    DynamicObjectData::getConstructor(mcp);
  }
}

Variant *EvalObjectData::o_realPropHook(
  CStrRef s, int flags, CStrRef context /* = null_string */) const {
  CStrRef c = context.isNull() ? FrameInjection::GetClassName(false) : context;
  if (UNLIKELY(flags & RealPropExist)) {
    Variant *t = EvalObjectData::o_realPropHook(
      s, (flags & ~RealPropExist) | RealPropUnchecked, c);
    if (t) return t;
    if (m_ce.getClass()->checkPropExist(s)) {
      return (Variant*)&null_variant;
    }
    return NULL;
  }

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
  Array dyn_props = (parent.get() ? parent.get() : this)->getProperties();
  m_ce.getClass()->getArray(props, dyn_props, &m_ce, pubOnly ? 0 : &m_privates);

  if (parent.get()) {
    ClassInfo::GetArray(parent.get(), parent.get()->o_getClassPropTable(),
                        props,
                        pubOnly ?
                        ClassInfo::GetArrayNone :
                        ClassInfo::GetArrayPrivate);
  }
  if (!dyn_props.empty()) {
    for (ArrayIter it(dyn_props); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      props.lvalAt(key, AccessFlags::Key).setWithRef(value);
    }
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

const MethodStatementWrapper *EvalObjectData::getMethodStatementWrapper(
  CStrRef name) const {
  return m_ce.getMethod(name);
}

const MethodStatementWrapper*
EvalObjectData::getConstructorStatementWrapper() const {
  return &m_ce.getConstructorWrapper();
}

bool EvalObjectData::o_instanceof_hook(CStrRef s) const {
  return m_ce.getClass()->subclassOf(s) ||
    (!parent.isNull() && parent->o_instanceof(s));
}

const CallInfo *EvalObjectData::t___invokeCallInfoHelper(void *&extra) {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___invoke);
  if (LIKELY(msw != NULL)) {
    extra = (void*) &m_invokeMcp;
    m_invokeMcp.extra = (void*) msw;
    return msw->m_methodStatement->getCallInfo();
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
      if (it->second.m_methodStatement) {
        mcp.extra = (void*)&it->second;
        mcp.ci = it->second.m_methodStatement->getCallInfo();
        return true;
      }
    }
    if (ObjectData *p = parent.get()) {
      return p->o_get_call_info(mcp, hash);
    }
  } else {
    if (m_ce.getClass()->subclassOf(clsname)) {
      ClassEvalState *ce;
      const MethodStatementWrapper *msw =
        RequestEvalState::findMethod(clsname, *(mcp.name), ce);
      if (msw && msw->m_methodStatement) {
        mcp.extra = (void*)msw;
        mcp.obj = this;
        mcp.ci = msw->m_methodStatement->getCallInfo();
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
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___call);
  if (msw && msw->m_methodStatement) {
    if (v_arguments.isNull()) {
      v_arguments = Array::Create();
    }
    return msw->m_methodStatement->invokeInstance(Object(root),
      CREATE_VECTOR2(v_name, v_arguments), msw, false);
  } else {
    return DynamicObjectData::doCall(v_name, v_arguments, fatal);
  }
}

Variant EvalObjectData::t___destruct() {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___destruct);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root), Array(),
      msw, false);
  } else {
    return DynamicObjectData::t___destruct();
  }
}
Variant EvalObjectData::t___set(Variant v_name, Variant v_value) {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___set);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      CREATE_VECTOR2(v_name, withRefBind(v_value)), msw, false);
  } else {
    return DynamicObjectData::t___set(v_name, withRefBind(v_value));
  }
}
Variant EvalObjectData::t___get(Variant v_name) {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___get);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      CREATE_VECTOR1(v_name), msw, false);
  } else {
    return DynamicObjectData::t___get(v_name);
  }
}
bool EvalObjectData::t___isset(Variant v_name) {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___isset);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      CREATE_VECTOR1(v_name), msw, false);
  } else {
    return DynamicObjectData::t___isset(v_name);
  }
}
Variant EvalObjectData::t___unset(Variant v_name) {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___unset);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      CREATE_VECTOR1(v_name), msw, false);
  } else {
    return DynamicObjectData::t___unset(v_name);
  }
}

bool EvalObjectData::php_sleep(Variant &ret) {
  ret = t___sleep();
  return getMethodStatementWrapper(s___sleep);
}

Variant EvalObjectData::t___sleep() {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___sleep);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      Array(), msw, false);
  } else {
    return DynamicObjectData::t___sleep();
  }
}

Variant EvalObjectData::t___wakeup() {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___wakeup);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      Array(), msw, false);
  } else {
    return DynamicObjectData::t___wakeup();
  }
}

String EvalObjectData::t___tostring() {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___tostring);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      Array(), msw, false);
  } else {
    return DynamicObjectData::t___tostring();
  }
}
Variant EvalObjectData::t___clone() {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s___clone);
  if (msw && msw->m_methodStatement) {
    return msw->m_methodStatement->invokeInstance(Object(root),
      Array(), msw, false);
  } else {
    return DynamicObjectData::t___clone();
  }
}

ObjectData* EvalObjectData::clone() {
  return NEWOBJ(EvalObjectData)(this);
}

Variant &EvalObjectData::___offsetget_lval(Variant v_name) {
  const MethodStatementWrapper *msw = getMethodStatementWrapper(s_offsetget);
  if (msw && msw->m_methodStatement) {
    Variant &v = get_globals()->__lvalProxy;
    v = msw->m_methodStatement->invokeInstance(Object(root),
      CREATE_VECTOR1(v_name), msw, false);
    return v;
  } else {
    return DynamicObjectData::___offsetget_lval(v_name);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
