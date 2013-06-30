/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
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

c_Continuation::c_Continuation(Class* cb)
    : ExtObjectData(cb)
    , m_label(0)
    , m_index(-1LL)
    , m_key(-1LL)
    , m_value(Variant::NullInit())
    , m_origFunc(nullptr) {
  o_subclassData.u16 = 0;
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
    frame_free_locals_inl(ar, ar->m_func->numLocals());
  }
}

void c_Continuation::t___construct() {}

void c_Continuation::t_update(int64_t label, CVarRef value) {
  m_label = label;
  assert(m_label == label); // check m_label for truncation
  m_value.assignVal(value);
  m_key = ++m_index;
}

void c_Continuation::t_update_key(int64_t label, CVarRef key, CVarRef value) {
  m_label = label;
  assert(m_label == label); // check m_label for truncation
  m_key.assignVal(key);
  m_value.assignVal(value);
  if (m_key.isInteger()) {
    int64_t new_index = m_key.toInt64Val();
    m_index = new_index > m_index ? new_index : m_index;
  }
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

Variant c_Continuation::t_current() {
  const_assert(false);
  return m_value;
}

Variant c_Continuation::t_key() {
  startedCheck();
  return m_key;
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
  return !done();
}

void c_Continuation::t_send(CVarRef v) {
  const_assert(false);
}

void c_Continuation::t_raise(CVarRef v) {
  const_assert(false);
}

String c_Continuation::t_getorigfuncname() {
  static auto const closureName = StringData::GetStaticString("{closure}");
  auto const origName = m_origFunc->isClosureBody() ? closureName
                                                    : m_origFunc->name();
  assert(origName->isStatic());
  return String(const_cast<StringData*>(origName));
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

void c_Continuation::dupContVar(const StringData* name, TypedValue* src) {
  ActRec *fp = actRec();
  Id destId = fp->m_func->lookupVarId(name);
  if (destId != kInvalidId) {
    // Copy the value of the local to the cont object.
    tvDupFlattenVars(src, frame_local(fp, destId), nullptr);
  } else {
    if (!fp->hasVarEnv()) {
      // This VarEnv may potentially outlive the most recently stack-allocated
      // VarEnv, so we need to heap allocate it.
      fp->setVarEnv(VarEnv::createLocalOnHeap(fp));
    }
    fp->getVarEnv()->setWithRef(name, src);
  }
}

static const StaticString s_this("this");

void c_Continuation::copyContinuationVars(ActRec* fp) {
  // For functions that contain only named locals, we can copy TVs
  // right to the local space.
  static const StringData* thisStr = s_this.get();
  bool skipThis;
  if (fp->hasVarEnv()) {
    Stats::inc(Stats::Cont_CreateVerySlow);
    Array definedVariables = fp->getVarEnv()->getDefinedVariables();
    skipThis = definedVariables.exists(s_this, true);

    for (ArrayIter iter(definedVariables); !iter.end(); iter.next()) {
      if (iter.first().getStringData()->same(s___cont__.get())) {
        continue;
      }
      dupContVar(iter.first().getStringData(),
                 const_cast<TypedValue *>(iter.secondRef().asTypedValue()));
    }
  } else {
    const Func *genFunc = actRec()->m_func;
    skipThis = genFunc->lookupVarId(thisStr) != kInvalidId;
    // skip local 0 because that's the old continuation
    for (Id i = 1; i < genFunc->numNamedLocals(); ++i) {
      dupContVar(genFunc->localVarName(i), frame_local(fp, i));
    }
  }

  // If $this is used as a local inside the body and is not provided
  // by our containing environment, just prefill it here instead of
  // using InitThisLoc inside the body
  if (!skipThis && fp->hasThis()) {
    Id id = actRec()->m_func->lookupVarId(thisStr);
    if (id != kInvalidId) {
      tvAsVariant(frame_local(actRec(), id)) = fp->getThis();
    }
  }
}

c_Continuation *c_Continuation::clone() {
  const Func *origFunc = m_origFunc;
  const Func *genFunc = actRec()->m_func;

  ActRec *fp = g_vmContext->getFP();
  c_Continuation* cont = origFunc->isMethod()
    ? g_vmContext->createContMeth(origFunc, genFunc, fp->getThisOrClass())
    : g_vmContext->createContFunc(origFunc, genFunc);

  cont->copyContinuationVars(actRec());

  cont->o_subclassData.u16 = o_subclassData.u16;
  cont->m_label = m_label;
  cont->m_index = m_index;
  cont->m_key   = m_key;
  cont->m_value = m_value;

  return cont;
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

Variant c_DummyContinuation::t_key() {
  throw_fatal("Tring to use a DummyContinuation");
  return uninit_null();
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
