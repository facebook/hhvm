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

#include <runtime/ext/ext_asio.h>
#include <runtime/ext/asio/asio_context.h>
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_setResultToRef("<set-result-to-ref>");
}

c_SetResultToRefWaitHandle::c_SetResultToRefWaitHandle(
    const ObjectStaticCallbacks *cb)
    : c_BlockableWaitHandle(cb), m_child(), m_ref() {
}

c_SetResultToRefWaitHandle::~c_SetResultToRefWaitHandle() {
  if (m_ref) {
    decRefRef(m_ref);
  }
}

void c_SetResultToRefWaitHandle::t___construct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(SetResultToRefWaitHandle, SetResultToRefWaitHandle::__construct);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use SetResultToRefWaitHandle::create() instead of constructor"));
  throw e;
}

Object c_SetResultToRefWaitHandle::ti_create(const char* cls, CObjRef wait_handle, VRefParam ref) {
  TypedValue* var_or_cell = ref->asTypedValue();
  c_WaitHandle* wh = wait_handle.getTyped<c_WaitHandle>(true, true);
  if (!wh) {
    if (wait_handle.isNull()) {
      tvSetNull(var_or_cell);
      return wait_handle;
    }

    STATIC_METHOD_INJECTION_BUILTIN(SetResultToRefWaitHandle, SetResultToRefWaitHandle::create);
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected wait_handle to be an instance of WaitHandle or null"));
    throw e;
  }

  // succeeded? set result to ref and give back succeeded wait handle
  if (wh->isSucceeded()) {
    tvSet(wh->getResult(), var_or_cell);
    return wh;
  }

  // failed? reset ref and give back failed wait handle
  if (wh->isFailed()) {
    tvSetNull(var_or_cell);
    return wh;
  }

  // it's still running so it must be WaitableWaitHandle
  c_WaitableWaitHandle* child_wh = static_cast<c_WaitableWaitHandle*>(wh);

  // make sure the reference is properly boxed so that we can store cell pointer
  if (UNLIKELY(var_or_cell->m_type != KindOfRef)) {
    tvBox(var_or_cell);
  }

  c_SetResultToRefWaitHandle* my_wh = NEWOBJ(c_SetResultToRefWaitHandle)();
  my_wh->initialize(child_wh, var_or_cell->m_data.pref);
  return my_wh;
}

void c_SetResultToRefWaitHandle::initialize(c_WaitableWaitHandle* child, RefData* ref) {
  setContext(AsioSession::GetCurrentContext());
  m_child = child;
  m_ref = ref;
  m_ref->incRefCount();
  blockOn(child);
}

c_WaitableWaitHandle* c_SetResultToRefWaitHandle::getBlockedOn() {
  assert(getState() == STATE_BLOCKED);
  return m_child.get();
}

void c_SetResultToRefWaitHandle::onUnblocked() {
  // succeeded
  if (m_child->isSucceeded()) {
    TypedValue* result = m_child->getResult();
    RefData* ref = m_ref;

    m_ref = nullptr;
    tvSetIgnoreRef(result, ref->tv());
    decRefRef(ref);

    setResult(result);
    m_child = nullptr;
    return;
  }

  // failed
  if (m_child->isFailed()) {
    RefData* ref = m_ref;

    m_ref = nullptr;
    tvSetNullIgnoreRef(ref->tv());
    decRefRef(ref);

    setException(m_child->getException());
    m_child = nullptr;
    return;
  }

  throw FatalErrorException(
      "Invariant violation: child neither succeeded nor failed");
}

void c_SetResultToRefWaitHandle::failBlock(CObjRef exception) {
  RefData* ref = m_ref;

  m_ref = nullptr;
  tvSetNullIgnoreRef(ref->tv());
  decRefRef(ref);

  setException(exception.get());
  m_child = nullptr;
}

String c_SetResultToRefWaitHandle::getName() {
  return s_setResultToRef;
}

void c_SetResultToRefWaitHandle::enterContext(AsioContext* ctx) {
  // stop before corrupting unioned data
  if (getState() == STATE_SUCCEEDED || getState() == STATE_FAILED) {
    return;
  }

  // already in the more specific context?
  if (LIKELY(ctx->includes(getContext()))) {
    return;
  }

  if (getState() != STATE_BLOCKED) {
    throw new FatalErrorException(
        "Invariant violation: encountered unexpected state");
  }

  setContext(ctx);
  m_child->enterContext(ctx);
}

///////////////////////////////////////////////////////////////////////////////
}
