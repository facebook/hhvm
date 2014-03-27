/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
  , m_index(-1LL)
  , m_key(make_tv<KindOfInt64>(-1LL))
  , m_value(make_tv<KindOfNull>())
{
  o_subclassData.u16 = 0;
}

c_Continuation::~c_Continuation() {
  tvRefcountedDecRef(m_key);
  tvRefcountedDecRef(m_value);

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

void c_Continuation::suspend(Offset offset, const Cell& value) {
  assert(actRec()->func()->contains(offset));
  m_offset = offset;
  cellSet(make_tv<KindOfInt64>(++m_index), m_key);
  cellSet(value, m_value);
}

void c_Continuation::suspend(Offset offset, const Cell& key,
                             const Cell& value) {
  assert(actRec()->func()->contains(offset));
  m_offset = offset;
  cellSet(key, m_key);
  cellSet(value, m_value);
  if (m_key.m_type == KindOfInt64) {
    int64_t new_index = m_key.m_data.num;
    m_index = new_index > m_index ? new_index : m_index;
  }
}

Variant c_Continuation::t_current() {
  const_assert(false);
  return tvAsCVarRef(&m_value);
}

Variant c_Continuation::t_key() {
  startedCheck();
  return tvAsCVarRef(&m_key);
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

void c_Continuation::t_send(const Variant& v) {
  const_assert(false);
}

void c_Continuation::t_raise(const Variant& v) {
  const_assert(false);
}

String c_Continuation::t_getorigfuncname() {
  const Func* origFunc = actRec()->func();
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
      fp->setVarEnv(VarEnv::createLocal(fp));
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
    const Func *func = actRec()->m_func;
    skipThis = func->lookupVarId(thisStr) != kInvalidId;
    for (Id i = 0; i < func->numNamedLocals(); ++i) {
      dupContVar(func->localVarName(i), frame_local(fp, i));
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
    ? CreateMeth(fp->func(), fp->getThisOrClass(), thiz->m_offset)
    : CreateFunc(fp->func(), thiz->m_offset));

  cont->copyContinuationVars(fp);

  cont->o_subclassData.u16 = thiz->o_subclassData.u16;
  cont->m_index  = thiz->m_index;
  cellSet(thiz->m_key, cont->m_key);
  cellSet(thiz->m_value, cont->m_value);

  return cont;
}

///////////////////////////////////////////////////////////////////////////////
}
