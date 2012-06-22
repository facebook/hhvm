/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_continuation.h>
#include <runtime/base/builtin_functions.h>

#include <runtime/ext/ext_spl.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_function.h>

#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/func.h>
#include <runtime/vm/stats.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

p_Continuation f_hphp_create_continuation(CStrRef clsname,
                                          CStrRef funcname,
                                          CStrRef origFuncName,
                                          CArrRef args /* = null_array */) {
  if (hhvm) {
    throw_fatal("hphp_create_continuation is not supported under hhvm");
  }
  Array definedVariables;
  Object obj;
  Eval::VariableEnvironment *env =
    FrameInjection::GetVariableEnvironment(true);
  if (UNLIKELY(!env)) {
    throw_fatal("Invalid call hphp_create_continuation");
  }
  definedVariables = env->getDefinedVariables();
  obj = FrameInjection::GetThis(true);
  bool isMethod = !clsname.isNull() && !clsname.empty();
  int64 callInfo = f_hphp_get_call_info(clsname, funcname);
  int64 extra = f_hphp_get_call_info_extra(clsname, funcname);
  if (has_eval_support && isMethod &&
      Eval::MethodStatementWrapper::isTraitMethod(
        (const Eval::MethodStatementWrapper *)extra)) {
    extra = f_hphp_get_call_info_extra(FrameInjection::GetClassName(false),
                                       funcname);
  }
  p_GenericContinuation cont(
      ((c_GenericContinuation*)coo_GenericContinuation())->
        create(callInfo, extra, isMethod, origFuncName,
               definedVariables, obj, args));
  if (isMethod) {
    CStrRef cls = f_get_called_class();
    cont->setCalledClass(cls);
  }

  return cont;
}

void f_hphp_pack_continuation(CObjRef continuation,
                              int64 label, CVarRef value) {
  if (hhvm) {
    throw_fatal("hphp_pack_continuation is not supported under hhvm");
  }
  if (UNLIKELY(!continuation->o_instanceof("GenericContinuation"))) {
    throw_fatal(
        "Cannot call hphp_pack_continuation with a "
        "non-GenericContinuation object");
  }
  Array definedVariables;
  Eval::VariableEnvironment *env =
    FrameInjection::GetVariableEnvironment(true);
  if (UNLIKELY(!env)) {
    throw_fatal("Invalid call hphp_pack_continuation");
  }
  definedVariables = env->getDefinedVariables();
  p_GenericContinuation c(
      static_cast<c_GenericContinuation*>(continuation.get()));
  c->t_update(label, value, definedVariables);
}

void f_hphp_unpack_continuation(CObjRef continuation) {
  if (hhvm) {
    throw_fatal("hphp_unpack_continuation is not supported under hhvm");
  }
  if (UNLIKELY(!continuation->o_instanceof("GenericContinuation"))) {
    throw_fatal(
        "Cannot call hphp_unpack_continuation with a "
        "non-GenericContinuation object");
  }
  Eval::VariableEnvironment *env =
    FrameInjection::GetVariableEnvironment(true);
  if (UNLIKELY(!env)) {
    throw_fatal("Invalid call hphp_unpack_continuation");
  }
  p_GenericContinuation c(
    static_cast<c_GenericContinuation*>(continuation.get()));
  extract(env, c->t_getvars(), 256 /* EXTR_REFS */);
}

///////////////////////////////////////////////////////////////////////////////

static StaticString s___cont__("__cont__");

#define LABEL_INIT m_label(0ll)

c_Continuation::c_Continuation(const ObjectStaticCallbacks *cb) :
    ExtObjectData(cb),
#ifndef HHVM
    LABEL_INIT,
#endif
    m_index(-1LL),
    m_value(Variant::nullInit), m_received(Variant::nullInit),
    m_done(false), m_running(false), m_should_throw(false),
    m_isMethod(false), m_callInfo(NULL), m_extra(NULL)
#ifdef HHVM
    ,LABEL_INIT
#endif
{
  CPP_BUILTIN_CLASS_INIT(Continuation);
}
#undef LABEL_INIT

c_Continuation::~c_Continuation() {}

void c_Continuation::t___construct(
    int64 func, int64 extra, bool isMethod,
    CStrRef origFuncName, CVarRef obj, CArrRef args) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::__construct);
  m_callInfo     = (const CallInfo*) func;
  m_extra        = (void*) extra;
  m_isMethod     = isMethod;
  m_origFuncName = origFuncName;

  if (!obj.isNull()) {
    m_obj = obj.toObject();
    ASSERT(!m_obj.isNull());
  } else {
    ASSERT(m_obj.isNull());
  }
  m_args = args;
  ASSERT(m_callInfo);
}

void c_Continuation::t_update(int64 label, CVarRef value) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::update);
  m_label = label;
  m_value.assignVal(value);
}

void c_Continuation::t_done() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::done);
  m_done = true;
}

int64 c_Continuation::t_getlabel() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::getlabel);
  return m_label;
}

int64 c_Continuation::t_num_args() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::num_args);
  return m_args.size();
}

Array c_Continuation::t_get_args() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::get_args);
  return m_args;
}

Variant c_Continuation::t_get_arg(int64 id) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::get_arg);
  if (id < 0LL || id >= m_args.size()) return false;
  return m_args.rvalAt(id, AccessFlags::Error);
}

Variant c_Continuation::t_current() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::current);
  const_assert(!hhvm);
  nextCheck();
  return m_value;
}

int64 c_Continuation::t_key() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::key);
  if (m_index < 0LL) {
    throw_exception(
      Object(SystemLib::AllocExceptionObject("Need to call next() first")));
  }
  return m_index;
}

bool c_Continuation::php_sleep(Variant &ret) {
  ret = false;
  return true;
}

template<typename FI>
inline void c_Continuation::nextImpl(FI& fi) {
  const_assert(!hhvm);
  preNext();
  try {
    if (m_isMethod) {
      MethodCallPackage mcp;
      mcp.isObj = hhvm || m_obj.get();
      if (mcp.isObj) {
        mcp.obj = mcp.rootObj = m_obj.get();
      } else {
        mcp.rootCls = m_called_class.get();
      }
      mcp.extra = m_extra;
      if (!hhvm) {
        fi.setStaticClassName(m_called_class);
      }
      (m_callInfo->getMeth1Args())(mcp, 1, this);
    } else {
      if (hhvm) {
        MethodCallPackage mcp;
        mcp.isObj = false;
        mcp.obj = mcp.rootObj = NULL;
        mcp.extra = m_extra;
        (m_callInfo->getMeth1Args())(mcp, 1, this);
      } else {
        (m_callInfo->getFunc1Args())(m_extra, 1, this);
      }
    }
  } catch (Object e) {
    if (e.instanceof("exception")) {
      m_running = false;
      m_done = true;
      throw_exception(e);
    } else {
      throw;
    }
  }
  m_running = false;
}

void c_Continuation::t_next() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::next);
  const_assert(!hhvm);
  m_received.setNull();
  nextImpl(fi);
}

void c_Continuation::t_rewind() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::rewind);
  throw_exception(Object(SystemLib::AllocExceptionObject(
    "Cannot rewind on a Continuation object")));
}

bool c_Continuation::t_valid() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::valid);
  const_assert(!hhvm);
  return !m_done;
}

void c_Continuation::t_send(CVarRef v) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::send);
  const_assert(!hhvm);
  nextCheck();
  m_received.assignVal(v);
  nextImpl(fi);
}

void c_Continuation::t_raise(CVarRef v) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::raise);
  const_assert(!hhvm);
  nextCheck();
  m_received.assignVal(v);
  m_should_throw = true;
  nextImpl(fi);
}

void c_Continuation::t_raised() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::raised);
  if (m_should_throw) {
    m_should_throw = false;
    throw_exception(m_received);
  }
}

Variant c_Continuation::t_receive() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::receive);
  if (m_should_throw) {
    m_should_throw = false;
    throw_exception(m_received);
  }
  return m_received;
}

String c_Continuation::t_getorigfuncname() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::getorigfuncname);
  if (hhvm) {
    c_GenericContinuation* self = static_cast<c_GenericContinuation*>(this);
    VM::Class* vmClass = self->getVMCalledClass();
    if (vmClass != NULL && m_called_class.size() == 0) {
      m_called_class = vmClass->name()->data();
    }
  }
  if (m_called_class.size() == 0) {
    return m_origFuncName;
  }

  /*
    Replace the class name in m_origFuncName with the LSB class.  This
    produces more useful traces.
   */
  size_t method_pos = m_origFuncName.find("::");
  if (method_pos != std::string::npos) {
    return concat3(m_called_class, "::", m_origFuncName.substr(method_pos+2));
  } else {
    return m_origFuncName;
  }
}

Variant c_Continuation::t___clone() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::__clone);
  throw_fatal(
      "Trying to clone an uncloneable object of class Continuation");
  return null;
}

Variant c_Continuation::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::__destruct);
  return null;
}

c_GenericContinuation::c_GenericContinuation(const ObjectStaticCallbacks *cb) :
    c_Continuation(cb), m_locals(NULL), m_hasExtraVars(false), m_nLocals(0),
    m_vmCalledClass(0ll) {}
c_GenericContinuation::~c_GenericContinuation() {
  if (hhvm && m_locals != NULL) {
    for (int i = 0; i < m_nLocals; ++i) {
      tvRefcountedDecRef(&m_locals[i]);
    }
  }
  c_GenericContinuation::sweep();
}

void c_GenericContinuation::sweep() {
  if (hhvm && m_locals != NULL) {
    free(m_locals);
    m_locals = NULL;
  } else {
    ASSERT(m_locals == NULL);
  }
}

void
c_GenericContinuation::t___construct(int64 func, int64 extra, bool isMethod,
                                     CStrRef origFuncName, CArrRef vars,
                                     CVarRef obj, CArrRef args) {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::__construct);
  c_Continuation::t___construct(func, extra, isMethod,
                                origFuncName, obj, args);
  if (!hhvm) {
    m_vars = vars;
  }
}

void c_GenericContinuation::t_update(int64 label, CVarRef value, CArrRef vars) {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::update);
  c_Continuation::t_update(label, value);
  if (!hhvm) {
    m_vars = vars;
    m_vars.weakRemove(s___cont__, true);
  }

}

Array c_GenericContinuation::t_getvars() {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::getvars);
  return m_vars;
}

Variant c_GenericContinuation::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::__destruct);
  return null;
}

HphpArray* c_GenericContinuation::getStaticLocals() {
  const_assert(hhvm);
  if (m_VMStatics.get() == NULL) {
    m_VMStatics = NEW(HphpArray)(1);
  }

  return m_VMStatics.get();
}
///////////////////////////////////////////////////////////////////////////////

c_DummyContinuation::c_DummyContinuation(const ObjectStaticCallbacks *cb) :
  ExtObjectData(cb) {
  CPP_BUILTIN_CLASS_INIT(DummyContinuation);
}

c_DummyContinuation::~c_DummyContinuation() {}

void c_DummyContinuation::t___construct() {
}

Variant c_DummyContinuation::t_current() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DummyContinuation, DummyContinuation::current);
  throw_fatal("Tring to use a DummyContinuation");
  return null;
}

int64 c_DummyContinuation::t_key() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DummyContinuation, DummyContinuation::key);
  throw_fatal("Tring to use a DummyContinuation");
  return 0;
}

void c_DummyContinuation::t_next() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DummyContinuation, DummyContinuation::next);
  throw_fatal("Tring to use a DummyContinuation");
}

void c_DummyContinuation::t_rewind() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DummyContinuation, DummyContinuation::rewind);
  throw_fatal("Tring to use a DummyContinuation");
}

bool c_DummyContinuation::t_valid() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DummyContinuation, DummyContinuation::valid);
  throw_fatal("Tring to use a DummyContinuation");
  return false;
}

Variant c_DummyContinuation::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DummyContinuation, DummyContinuation::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
