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
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

p_Continuation f_hphp_create_continuation(CStrRef clsname,
                                          CStrRef funcname,
                                          CStrRef origFuncName,
                                          CArrRef args /* = null_array */) {
  Eval::VariableEnvironment *env =
    FrameInjection::GetVariableEnvironment(true);
  if (UNLIKELY(!env)) {
    throw_fatal("Cannot call hphp_create_continuation in non-eval context");
  }
  bool isMethod = !clsname.isNull() && !clsname.empty();
  int64 callInfo = f_hphp_get_call_info(clsname, funcname);
  int64 extra = f_hphp_get_call_info_extra(clsname, funcname);
  CObjRef obj = FrameInjection::GetThis(true);
  p_GenericContinuation cont(
      ((c_GenericContinuation*)coo_GenericContinuation())->
        create(callInfo, extra, isMethod, origFuncName,
               env->getDefinedVariables(), obj, args));
  if (isMethod) {
    CStrRef cls = f_get_called_class();
    cont->setCalledClass(cls);
  }
  return cont;
}

void f_hphp_pack_continuation(CObjRef continuation,
                              int64 label, CVarRef value) {
  Eval::VariableEnvironment *env =
    FrameInjection::GetVariableEnvironment(true);
  if (UNLIKELY(!env)) {
    throw_fatal("Cannot call hphp_pack_continuation in non-eval context");
  }
  if (UNLIKELY(!continuation->o_instanceof("GenericContinuation"))) {
    throw_fatal(
        "Cannot call hphp_pack_continuation with a "
        "non-GenericContinuation object");
  }
  p_GenericContinuation c(
      static_cast<c_GenericContinuation*>(continuation.get()));
  c->t_update(label, value, env->getDefinedVariables());
}

void f_hphp_unpack_continuation(CObjRef continuation) {
  Eval::VariableEnvironment *env =
    FrameInjection::GetVariableEnvironment(true);
  if (UNLIKELY(!env)) {
    throw_fatal("Cannot call hphp_unpack_continuation in non-eval context");
  }
  if (UNLIKELY(!continuation->o_instanceof("GenericContinuation"))) {
    throw_fatal(
        "Cannot call hphp_pack_continuation with a "
        "non-GenericContinuation object");
  }
  p_GenericContinuation c(
      static_cast<c_GenericContinuation*>(continuation.get()));
  extract(env, c->t_getvars(), 256 /* EXTR_REFS */);
}

///////////////////////////////////////////////////////////////////////////////

static StaticString s___cont__("__cont__");

c_Continuation::c_Continuation(const ObjectStaticCallbacks *cb) :
    ExtObjectData(cb),
    m_label(0LL), m_index(-1LL),
    m_value(Variant::nullInit), m_received(Variant::nullInit),
    m_done(false), m_running(false), m_should_throw(false),
    m_isMethod(false), m_callInfo(NULL), m_extra(NULL) {}

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
  if (m_index < 0LL) {
    throw_exception(
      Object(SystemLib::AllocExceptionObject("Need to call next() first")));
  }
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

#define NEXT_IMPL \
  if (m_done) { \
    throw_exception(Object(SystemLib::AllocExceptionObject( \
      "Continuation is already finished"))); \
  } \
  if (m_running) { \
    throw_exception(Object(SystemLib::AllocExceptionObject( \
      "Continuation is already running"))); \
  } \
  m_running = true; \
  ++m_index; \
  try { \
    if (m_isMethod) { \
      MethodCallPackage mcp; \
      mcp.isObj = true; \
      mcp.obj = mcp.rootObj = m_obj.get(); \
      mcp.extra = m_extra; \
      fi.setStaticClassName(m_called_class); \
      (m_callInfo->getMeth1Args())(mcp, 1, GET_THIS_TYPED(Continuation)); \
    } else { \
      (m_callInfo->getFunc1Args())(m_extra, 1, GET_THIS_TYPED(Continuation)); \
    } \
  } catch (Object e) { \
    if (e.instanceof("exception")) { \
      m_running = false; \
      m_done = true; \
      throw_exception(e); \
    } else { \
      throw; \
    } \
  } \
  m_running = false;

void c_Continuation::t_next() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::next);
  m_received.setNull();
  NEXT_IMPL;
}

void c_Continuation::t_rewind() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::rewind);
  throw_exception(Object(SystemLib::AllocExceptionObject(
    "Cannot rewind on a Continuation object")));
}

bool c_Continuation::t_valid() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::valid);
  return !m_done;
}

void c_Continuation::t_send(CVarRef v) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::send);
  if (m_index < 0LL) {
    throw_exception(
      Object(SystemLib::AllocExceptionObject("Need to call next() first")));
  }

  m_received.assignVal(v);
  NEXT_IMPL;
}

void c_Continuation::t_raise(CVarRef v) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::raise);
  if (m_index < 0LL) {
    throw_exception(
      Object(SystemLib::AllocExceptionObject("Need to call next() first")));
  }

  m_received.assignVal(v);
  m_should_throw = true;
  NEXT_IMPL;
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
  return m_origFuncName;
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

#undef NEXT_IMPL

c_GenericContinuation::c_GenericContinuation(const ObjectStaticCallbacks *cb) :
    c_Continuation(cb)  {}
c_GenericContinuation::~c_GenericContinuation() {}

void
c_GenericContinuation::t___construct(int64 func, int64 extra, bool isMethod,
                                     CStrRef origFuncName, CArrRef vars,
                                     CVarRef obj, CArrRef args) {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::__construct);
  c_Continuation::t___construct(func, extra, isMethod,
                                origFuncName, obj, args);
  m_vars = vars;
}

void c_GenericContinuation::t_update(int64 label, CVarRef value, CArrRef vars) {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::update);
  c_Continuation::t_update(label, value);
  m_vars = vars;
  m_vars.weakRemove(s___cont__, true);
}

Array c_GenericContinuation::t_getvars() {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::getvars);
  return m_vars;
}

Variant c_GenericContinuation::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(GenericContinuation, GenericContinuation::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
