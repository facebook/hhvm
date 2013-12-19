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

#include "hphp/runtime/ext/asio/set_result_to_ref_wait_handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_setResultToRef("<set-result-to-ref>");
}

void c_SetResultToRefWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use SetResultToRefWaitHandle::create() instead of constructor"));
  throw e;
}

void c_SetResultToRefWaitHandle::ti_setoncreatecallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set SetResultToRefWaitHandle::onCreate: on_create_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnSetResultToRefCreateCallback(callback.getObjectDataOrNull());
}

Object c_SetResultToRefWaitHandle::ti_create(CObjRef wait_handle, VRefParam ref) {
  TypedValue* var_or_cell = ref->asTypedValue();
  if (wait_handle.isNull()) {
    tvSetNull(*var_or_cell);
    return wait_handle;
  }

  if (!wait_handle.get()->getAttribute(ObjectData::IsWaitHandle)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected wait_handle to be an instance of WaitHandle or null"));
    throw e;
  }

  auto wh = static_cast<c_WaitHandle*>(wait_handle.get());

  // succeeded? set result to ref and give back succeeded wait handle
  if (wh->isSucceeded()) {
    tvSet(wh->getResult(), *var_or_cell);
    return wh;
  }

  // failed? reset ref and give back failed wait handle
  if (wh->isFailed()) {
    tvSetNull(*var_or_cell);
    return wh;
  }

  // it's still running so it must be WaitableWaitHandle
  auto child = static_cast<c_WaitableWaitHandle*>(wh);

  // import child into the current context, detect cross-context cycles
  auto session = AsioSession::Get();
  if (session->isInContext()) {
    child->enterContext(session->getCurrentContextIdx());
  }

  // make sure the reference is properly boxed so that we can store cell pointer
  if (UNLIKELY(var_or_cell->m_type != KindOfRef)) {
    tvBox(var_or_cell);
  }

  p_SetResultToRefWaitHandle my_wh = NEWOBJ(c_SetResultToRefWaitHandle)();
  my_wh->initialize(child, var_or_cell->m_data.pref);

  if (UNLIKELY(session->hasOnSetResultToRefCreateCallback())) {
    session->onSetResultToRefCreate(my_wh.get(), child);
  }

  return my_wh;
}

void c_SetResultToRefWaitHandle::initialize(c_WaitableWaitHandle* child, RefData* ref) {
  m_child = child;
  m_ref = ref;
  m_ref->incRefCount();
  blockOn(child);
}

void c_SetResultToRefWaitHandle::onUnblocked() {
  if (m_child->isSucceeded()) {
    // succeeded
    markAsSucceeded(m_child->getResult());
  } else if (m_child->isFailed()) {
    // failed
    markAsFailed(m_child->getException());
  } else {
    assert(false);
    throw FatalErrorException(
        "Invariant violation: child neither succeeded nor failed");
  }
}

void c_SetResultToRefWaitHandle::markAsSucceeded(const Cell& result) {
  RefData* ref = m_ref;

  m_ref = nullptr;
  cellSet(result, *ref->tv());
  decRefRef(ref);

  setResult(result);
  m_child = nullptr;
}

void c_SetResultToRefWaitHandle::markAsFailed(CObjRef exception) {
  RefData* ref = m_ref;

  m_ref = nullptr;
  tvSetIgnoreRef(make_tv<KindOfNull>(), *ref->tv());
  decRefRef(ref);

  setException(exception.get());
  m_child = nullptr;
}

String c_SetResultToRefWaitHandle::getName() {
  return s_setResultToRef;
}

c_WaitableWaitHandle* c_SetResultToRefWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return m_child.get();
}

void c_SetResultToRefWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);

  m_child->enterContext(ctx_idx);
  setContextIdx(ctx_idx);
}

///////////////////////////////////////////////////////////////////////////////
}
