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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static StaticString s___cont__("__cont__");

c_Continuation::c_Continuation() : 
  m_callInfo(NULL), m_extra(NULL), m_isMethod(false) {}
c_Continuation::~c_Continuation() {}

void c_Continuation::t___construct(
    int64 func, int64 extra, bool isMethod, 
    CArrRef vars, CVarRef obj, CArrRef args) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::__construct);
  // initialize the member variables
  m_label   = 0LL;
  m_done    = false;
  m_index   = -1LL;
  m_running = false;
  m_received.assignVal(null);

  m_callInfo = (const CallInfo*) func;
  m_extra    = (void*) extra;
  m_isMethod = isMethod;
  m_vars     = vars;
  if (!obj.isNull()) {
    ASSERT(obj.isObject());
    m_obj = obj.toObject();
    ASSERT(!m_obj.isNull());
  } else {
    ASSERT(m_obj.isNull());
  }
  m_args = args;

  ASSERT(m_callInfo);
}

void c_Continuation::t_update(int64 label, CVarRef value, CArrRef vars) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::update);
  m_label = label;
  m_value.assignVal(value);
  m_vars = vars;
  m_vars.weakRemove(s___cont__, true);
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
      p_Exception(
        ((c_Exception*)coo_Exception())->create("Need to call next() first")));
  }
  return m_value;
}

int64 c_Continuation::t_key() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::key);
  if (m_index < 0LL) {
    throw_exception(
      p_Exception(
        ((c_Exception*)coo_Exception())->create("Need to call next() first")));
  }
  return m_index;
}

#define NEXT_IMPL \
  if (m_done) { \
    throw_exception( \
      p_Exception( \
        ((c_Exception*)coo_Exception())->create( \
          "Continuation is already finished"))); \
  } \
  if (m_running) { \
    throw_exception( \
      p_Exception( \
        ((c_Exception*)coo_Exception())->create( \
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
  m_received.assignVal(null);
  NEXT_IMPL;
}

void c_Continuation::t_rewind() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::rewind);
  throw_exception(
    p_Exception(
      ((c_Exception*)coo_Exception())->create(
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
      p_Exception(
        ((c_Exception*)coo_Exception())->create("Need to call next() first")));
  }
  m_received.assignVal(v);
  NEXT_IMPL;
}

Variant c_Continuation::t_receive() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::receive);
  return m_received;
}

Array c_Continuation::t_getvars() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::getvars);
  return m_vars;
}

Variant c_Continuation::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Continuation, Continuation::__destruct);
  return null;
}

#undef NEXT_IMPL

///////////////////////////////////////////////////////////////////////////////
}
