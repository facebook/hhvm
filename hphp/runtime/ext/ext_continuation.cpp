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
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/base/builtin-functions.h"

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

void delete_Continuation(ObjectData* od, const Class*) {
  auto const cont = static_cast<c_Continuation*>(od);
  auto const size = cont->getObjectSize();
  auto const base = cont->getMallocBase();
  cont->~c_Continuation();
  if (LIKELY(size <= kMaxSmartSize)) {
    return MM().smartFreeSizeLogged(base, size);
  }
  MM().smartFreeSizeBigLogged(base, size);
}

///////////////////////////////////////////////////////////////////////////////

c_Continuation::c_Continuation(Class* cb)
  : ExtObjectDataFlags(cb)
  , m_label(0)
  , m_index(-1LL)
  , m_key(-1LL)
  , m_value(Variant::NullInit())
{
  o_subclassData.u16 = 0;
}

c_Continuation::~c_Continuation() {
  ActRec* ar = actRec();

  if (ar->hasVarEnv()) {
    ar->getVarEnv()->detach(ar);
  } else {
    // Free locals, but don't trigger the EventHook for FunctionExit
    // since the continuation function has already been exited. We
    // don't want redundant calls.
    frame_free_locals_inl_no_hook<false>(ar, ar->m_func->numLocals());
  }
}

//////////////////////////////////////////////////////////////////////

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

void c_Continuation::t_next() {
  const_assert(false);
}

const StaticString
  s_next("next"),
  s__closure_("{closure}"),
  s_this("this");

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
  const Func* origFunc = actRec()->func()->getGeneratorOrigFunc();
  auto const origName = origFunc->isClosureBody() ? s__closure_.get()
                                                  : origFunc->name();
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
    tvDupFlattenVars(src, frame_local(fp, destId));
  } else {
    if (!fp->hasVarEnv()) {
      // This VarEnv may potentially outlive the most recently stack-allocated
      // VarEnv, so we need to heap allocate it.
      fp->setVarEnv(VarEnv::createLocalOnHeap(fp));
    }
    fp->getVarEnv()->setWithRef(name, src);
  }
}

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
      dupContVar(iter.first().getStringData(),
                 const_cast<TypedValue *>(iter.secondRef().asTypedValue()));
    }
  } else {
    const Func *genFunc = actRec()->m_func;
    skipThis = genFunc->lookupVarId(thisStr) != kInvalidId;
    for (Id i = 0; i < genFunc->numNamedLocals(); ++i) {
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

c_Continuation *c_Continuation::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_Continuation*>(obj);
  auto fp = thiz->actRec();

  c_Continuation* cont = static_cast<c_Continuation*>(fp->getThisOrClass()
    ? CreateMeth(fp->func(), fp->getThisOrClass())
    : CreateFunc(fp->func()));

  cont->copyContinuationVars(fp);

  cont->o_subclassData.u16 = thiz->o_subclassData.u16;
  cont->m_label = thiz->m_label;
  cont->m_index = thiz->m_index;
  cont->m_key   = thiz->m_key;
  cont->m_value = thiz->m_value;

  return cont;
}

namespace {
  DEBUG_ONLY StaticString s_send("send");
  DEBUG_ONLY StaticString s_raise("raise");
}

void c_Continuation::call_send(Cell& v) {
  assert(SystemLib::s_continuationSendFunc ==
         getVMClass()->lookupMethod(s_send.get()));
  g_vmContext->invokeContFunc(SystemLib::s_continuationSendFunc, this, &v);
}

void c_Continuation::call_raise(ObjectData* e) {
  assert(SystemLib::s_continuationRaiseFunc ==
         getVMClass()->lookupMethod(s_raise.get()));
  assert(e);
  assert(e->instanceof(SystemLib::s_ExceptionClass));
  Cell arg;
  arg.m_type = KindOfObject;
  arg.m_data.pobj = e;
  g_vmContext->invokeContFunc(SystemLib::s_continuationRaiseFunc, this, &arg);
}

// Compute the bytecode offset at which execution will resume assuming
// the given label.
Offset c_Continuation::getExecutionOffset(int32_t label) const {
  auto func = actRec()->m_func;
  PC funcBase = func->unit()->entry() + func->base();
  assert(toOp(*funcBase) == OpUnpackCont); // One byte
  PC switchOffset = funcBase + 1;
  assert(toOp(*switchOffset) == OpSwitch);
  // The Switch opcode is one byte for the opcode itself, plus four
  // bytes for the jmp table size, then the jump table.
  if (label >= *(int32_t*)(switchOffset + 1)) {
    return InvalidAbsoluteOffset;
  }
  Offset* jmpTable = (Offset*)(switchOffset + 5);
  Offset relOff = jmpTable[label];
  return func->base() + relOff + 1;
}

// Compute the bytecode offset at which execution will resume when
// this continuation resumes. Only valid on started but not actually
// running continuations.
Offset c_Continuation::getNextExecutionOffset() const {
  assert(!running());
  return getExecutionOffset(m_label);
}

///////////////////////////////////////////////////////////////////////////////
}
