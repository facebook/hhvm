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

#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/spl/ext_spl.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/base/stats.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void delete_Generator(ObjectData* od, const Class*) {
  auto gen = static_cast<c_Generator*>(od);
  Resumable::Destroy(gen->data()->resumable()->size(), gen);
}

///////////////////////////////////////////////////////////////////////////////

GeneratorData::GeneratorData()
  : m_index(-1LL)
  , m_key(make_tv<KindOfInt64>(-1LL))
  , m_value(make_tv<KindOfNull>())
{
}

GeneratorData::~GeneratorData() {
  if (LIKELY(getState() == State::Done)) {
    return;
  }

  assert(getState() != State::Running);
  tvRefcountedDecRef(m_key);
  tvRefcountedDecRef(m_value);

  // Free locals, but don't trigger the EventHook for FunctionReturn since
  // the generator has already been exited. We don't want redundant calls.
  ActRec* ar = actRec();
  frame_free_locals_inl_no_hook<false>(ar, ar->func()->numLocals());
}

void GeneratorData::copyVars(const ActRec* srcFp) {
  const auto dstFp = actRec();
  const auto func = dstFp->func();
  assert(srcFp->func() == dstFp->func());

  for (Id i = 0; i < func->numLocals(); ++i) {
    tvDupFlattenVars(frame_local(srcFp, i), frame_local(dstFp, i));
  }

  if (dstFp->hasThis()) {
    dstFp->getThis()->incRefCount();
  }

  if (LIKELY(!(srcFp->func()->attrs() & AttrMayUseVV))) return;
  if (LIKELY(srcFp->m_varEnv == nullptr)) return;

  if (srcFp->hasExtraArgs()) {
    dstFp->setExtraArgs(srcFp->getExtraArgs()->clone(dstFp));
  } else {
    assert(srcFp->hasVarEnv());
    dstFp->setVarEnv(srcFp->getVarEnv()->clone(dstFp));
  }
}

void GeneratorData::yield(Offset resumeOffset,
                          const Cell* key, const Cell value) {
  assert(getState() == State::Running);
  resumable()->setResumeAddr(nullptr, resumeOffset);

  if (key) {
    cellSet(*key, m_key);
    tvRefcountedDecRefNZ(*key);
    if (m_key.m_type == KindOfInt64) {
      int64_t new_index = m_key.m_data.num;
      m_index = new_index > m_index ? new_index : m_index;
    }
  } else {
    cellSet(make_tv<KindOfInt64>(++m_index), m_key);
  }
  cellSet(value, m_value);
  tvRefcountedDecRefNZ(value);

  setState(State::Started);
}

void GeneratorData::done() {
  assert(getState() == State::Running);
  cellSetNull(m_key);
  cellSetNull(m_value);
  setState(State::Done);
}

///////////////////////////////////////////////////////////////////////////////

void c_Generator::t___construct() {}

// Functions with native implementation.
void c_Generator::t_next() { always_assert(false); }
void c_Generator::t_send(const Variant& v) { always_assert(false); }
void c_Generator::t_raise(const Variant& v) { always_assert(false); }
bool c_Generator::t_valid() { always_assert(false); }
Variant c_Generator::t_current() { always_assert(false); }
Variant c_Generator::t_key() { always_assert(false); }

const StaticString s_next("next");
void c_Generator::t_rewind() {
  this->o_invoke_few_args(s_next, 0);
}

const StaticString s__closure_("{closure}");
String c_Generator::t_getorigfuncname() {
  const Func* origFunc = data()->actRec()->func();
  auto const origName = origFunc->isClosureBody() ? s__closure_.get()
                                                  : origFunc->name();
  assert(origName->isStatic());
  return String(const_cast<StringData*>(origName));
}

String c_Generator::t_getcalledclass() {
  String called_class;

  if (data()->actRec()->hasThis()) {
    called_class = data()->actRec()->getThis()->getVMClass()->name()->data();
  } else if (data()->actRec()->hasClass()) {
    called_class = data()->actRec()->getClass()->name()->data();
  } else {
    called_class = empty_string();
  }

  return called_class;
}

GeneratorData *GeneratorData::Clone(ObjectData* obj) {
  auto thiz = GeneratorData::fromObject(obj);
  auto fp = thiz->actRec();

  auto objClone = Create<true>(fp, fp->func()->numSlotsInFrame(),
                               thiz->resumable()->resumeAddr(),
                               thiz->resumable()->resumeOffset());
  auto cont = GeneratorData::fromObject(objClone);
  cont->copyVars(fp);
  cont->setState(thiz->getState());
  cont->m_index  = thiz->m_index;
  cellSet(thiz->m_key, cont->m_key);
  cellSet(thiz->m_value, cont->m_value);

  return cont;
}

///////////////////////////////////////////////////////////////////////////////
}
