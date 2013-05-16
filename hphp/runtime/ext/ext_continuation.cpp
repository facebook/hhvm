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

#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_asio.h"
#include "hphp/runtime/base/builtin_functions.h"

#include "hphp/runtime/ext/ext_spl.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/ext/ext_function.h"

#include "hphp/runtime/vm/translator/translator.h"
#include "hphp/runtime/vm/translator/translator-inline.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

p_Continuation f_hphp_create_continuation(CStrRef clsname,
                                          CStrRef funcname,
                                          CStrRef origFuncName,
                                          CArrRef args /* = null_array */) {
  throw_fatal("Invalid call hphp_create_continuation");
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////

static StaticString s___cont__("__cont__");

c_Continuation::c_Continuation(Class* cb) :
    ExtObjectData(cb),
    m_index(-1LL),
    m_value(Variant::nullInit), m_received(Variant::nullInit),
    m_done(false), m_running(false), m_should_throw(false),
    m_vmFunc(nullptr), m_label(0ll) {
}

c_Continuation::~c_Continuation() {
  ActRec* ar = actRec();

  // The first local is the object itself, and it wasn't increffed at creation
  // time (see createContinuation()). Overwrite its type to exempt it from
  // refcounting here.
  TypedValue* contLocal = frame_local(ar, 0);
  assert(contLocal->m_data.pobj == this);
  contLocal->m_type = KindOfNull;

  if (ar->hasVarEnv()) {
    ar->getVarEnv()->detach(ar);
  } else {
    frame_free_locals_inl(ar, m_vmFunc->numLocals());
  }
}

void c_Continuation::t___construct() {}

void c_Continuation::t_update(int64_t label, CVarRef value) {
  m_label = label;
  m_value.assignVal(value);
}

Object c_Continuation::t_getwaithandle() {
  if (m_waitHandle.isNull()) {
    c_ContinuationWaitHandle::Create(this);
    assert(!m_waitHandle.isNull());
  }
  return m_waitHandle;
}

int64_t c_Continuation::t_getlabel() {
  return m_label;
}

int64_t c_Continuation::t_num_args() {
  return m_args.size();
}

Array c_Continuation::t_get_args() {
  return m_args;
}

Variant c_Continuation::t_get_arg(int64_t id) {
  if (id < 0LL || id >= m_args.size()) return false;
  return m_args.rvalAt(id, AccessFlags::Error);
}

Variant c_Continuation::t_current() {
  const_assert(false);
  return m_value;
}

int64_t c_Continuation::t_key() {
  startedCheck();
  return m_index;
}

bool c_Continuation::php_sleep(Variant &ret) {
  ret = false;
  return true;
}

void c_Continuation::t_next() {
  const_assert(false);
}

static StaticString s_next("next");
void c_Continuation::t_rewind() {
  this->o_invoke_few_args(s_next, 0);
}

bool c_Continuation::t_valid() {
  const_assert(false);
  return !m_done;
}

void c_Continuation::t_send(CVarRef v) {
  const_assert(false);
}

void c_Continuation::t_raise(CVarRef v) {
  const_assert(false);
}

void c_Continuation::t_raised() {
  if (m_should_throw) {
    m_should_throw = false;
    throw_exception(m_received);
  }
}

String c_Continuation::t_getorigfuncname() {
  return String(const_cast<StringData*>(m_origFuncName));
}

String c_Continuation::t_getcalledclass() {
  String called_class;

  if (actRec()->hasThis()) {
    called_class = actRec()->getThis()->getVMClass()->name()->data();
  } else if (actRec()->hasClass()) {
    called_class = actRec()->getClass()->name()->data();
  } else {
    called_class = empty_string;
  }

  return called_class;
}

Variant c_Continuation::t___clone() {
  throw_fatal(
      "Trying to clone an uncloneable object of class Continuation");
  return uninit_null();
}

HphpArray* c_Continuation::getStaticLocals() {
  if (m_VMStatics.get() == NULL) {
    m_VMStatics = NEW(HphpArray)(1);
  }
  return m_VMStatics.get();
}

namespace {
  StaticString s_send("send");
  StaticString s_raise("raise");
}

void c_Continuation::call_next() {
  const HPHP::Func* func = m_cls->lookupMethod(s_next.get());
  g_vmContext->invokeContFunc(func, this);
}

void c_Continuation::call_send(TypedValue* v) {
  const HPHP::Func* func = m_cls->lookupMethod(s_send.get());
  g_vmContext->invokeContFunc(func, this, v);
}

void c_Continuation::call_raise(ObjectData* e) {
  assert(e);
  assert(e->instanceof(SystemLib::s_ExceptionClass));

  const HPHP::Func* func = m_cls->lookupMethod(s_raise.get());

  TypedValue arg;
  arg.m_type = KindOfObject;
  arg.m_data.pobj = e;

  g_vmContext->invokeContFunc(func, this, &arg);
}

///////////////////////////////////////////////////////////////////////////////

c_DummyContinuation::c_DummyContinuation(Class* cb) :
  ExtObjectData(cb) {
}

c_DummyContinuation::~c_DummyContinuation() {}

void c_DummyContinuation::t___construct() {
}

Variant c_DummyContinuation::t_current() {
  throw_fatal("Tring to use a DummyContinuation");
  return uninit_null();
}

int64_t c_DummyContinuation::t_key() {
  throw_fatal("Tring to use a DummyContinuation");
  return 0;
}

void c_DummyContinuation::t_next() {
  throw_fatal("Tring to use a DummyContinuation");
}

void c_DummyContinuation::t_rewind() {
  throw_fatal("Tring to use a DummyContinuation");
}

bool c_DummyContinuation::t_valid() {
  throw_fatal("Tring to use a DummyContinuation");
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
