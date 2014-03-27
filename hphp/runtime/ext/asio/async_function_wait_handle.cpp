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

#include "hphp/runtime/ext/asio/async_function_wait_handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// max depth of continuation
const uint16_t MAX_DEPTH = 512;

c_AsyncFunctionWaitHandle::c_AsyncFunctionWaitHandle(Class* cb)
    : c_BlockableWaitHandle(cb), m_continuation(), m_child(), m_privData(),
      m_depth(0) {
}

void c_AsyncFunctionWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use $continuation->getWaitHandle() instead of constructor"));
  throw e;
}

void c_AsyncFunctionWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onStart: on_start_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionCreateCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonawaitcallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onAwait: on_await_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionAwaitCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonsuccesscallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onSuccess: on_success_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionSuccessCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonfailcallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onFail: on_fail_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionFailCallback(callback.getObjectDataOrNull());
}

Object c_AsyncFunctionWaitHandle::t_getprivdata() {
  return m_privData;
}

void c_AsyncFunctionWaitHandle::t_setprivdata(const Object& data) {
  m_privData = data;
}

namespace {

void checkCreateErrors(c_WaitableWaitHandle* child) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->getCurrentWaitHandleDepth() >= MAX_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
          "Asio stack overflow"));
    throw e;
  }

  if (session->isInContext()) {
    child->enterContext(session->getCurrentContextIdx());
  }
}

}

ObjectData*
c_AsyncFunctionWaitHandle::CreateFunc(const Func* genFunc,
                                      Offset offset,
                                      ObjectData* child) {
  assert(child->instanceof(c_WaitableWaitHandle::classof()));
  auto child_wh = static_cast<c_WaitableWaitHandle*>(child);
  assert(!child_wh->isFinished());

  checkCreateErrors(child_wh);

  auto cont = c_Continuation::CreateFunc(genFunc, offset);
  auto wait_handle = NEWOBJ(c_AsyncFunctionWaitHandle)();
  wait_handle->incRefCount();
  wait_handle->initialize(static_cast<c_Continuation*>(cont), child_wh);
  return wait_handle;
}

ObjectData*
c_AsyncFunctionWaitHandle::CreateMeth(const Func* genFunc,
                                      void* objOrCls,
                                      Offset offset,
                                      ObjectData* child) {
  assert(child->instanceof(c_WaitableWaitHandle::classof()));
  auto child_wh = static_cast<c_WaitableWaitHandle*>(child);
  assert(!child_wh->isFinished());

  checkCreateErrors(child_wh);

  auto cont = c_Continuation::CreateMeth(genFunc, objOrCls, offset);
  auto wait_handle = NEWOBJ(c_AsyncFunctionWaitHandle)();
  wait_handle->incRefCount();
  wait_handle->initialize(static_cast<c_Continuation*>(cont), child_wh);
  return wait_handle;
}

void c_AsyncFunctionWaitHandle::initialize(c_Continuation* continuation,
                                           c_WaitableWaitHandle* child) {
  auto session = AsioSession::Get();

  continuation->start();

  m_continuation = continuation;
  m_child = child;
  m_privData = nullptr;
  m_depth = session->getCurrentWaitHandleDepth() + 1;
  blockOn(m_child.get());
  decRefObj(continuation);
  decRefObj(child);

  // needs to be called with non-zero refcnt
  if (UNLIKELY(session->hasOnAsyncFunctionCreateCallback())) {
    session->onAsyncFunctionCreate(this, child);
  }
}

void c_AsyncFunctionWaitHandle::run() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    return;
  }

  try {
    setState(STATE_RUNNING);

    // iterate continuation
    if (LIKELY(m_child->isSucceeded())) {
      // child succeeded, pass the result to the async function
      g_context->resumeAsyncFunc(*m_continuation.get(), m_child->getResult());
    } else if (m_child->isFailed()) {
      // child failed, raise the exception inside continuation
      g_context->resumeAsyncFuncThrow(*m_continuation.get(),
                                      m_child->getException());
    } else {
      throw FatalErrorException(
          "Invariant violation: child neither succeeded nor failed");
    }

  retry:
    // continuation finished, retrieve result from its m_value
    if (m_continuation->done()) {
      markAsSucceeded(m_continuation->m_value);
      return;
    }

    // save child
    Cell& value = m_continuation->m_value;
    assert(value.m_type == KindOfObject);
    assert(value.m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));

    m_child = static_cast<c_WaitableWaitHandle*>(value.m_data.pobj);
    assert(!m_child->isFinished());

    // import child into the current context, detect cross-context cycles
    try {
      m_child->enterContext(getContextIdx());
    } catch (Object& e) {
      g_context->resumeAsyncFuncThrow(*m_continuation.get(), e.get());
      goto retry;
    }

    // detect cycles
    if (UNLIKELY(isDescendantOf(m_child.get()))) {
      Object e(createCycleException(m_child.get()));
      g_context->resumeAsyncFuncThrow(*m_continuation.get(), e.get());
      goto retry;
    }

    // on await callback
    AsioSession* session = AsioSession::Get();
    if (UNLIKELY(session->hasOnAsyncFunctionAwaitCallback())) {
      session->onAsyncFunctionAwait(this, m_child.get());
    }

    // set up dependency
    blockOn(m_child.get());
  } catch (const Object& exception) {
    // process exception thrown by the async function
    markAsFailed(exception);
  } catch (...) {
    // process C++ exception
    markAsFailed(AsioSession::Get()->getAbruptInterruptException());
    throw;
  }
}

void c_AsyncFunctionWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  }
}

void c_AsyncFunctionWaitHandle::markAsSucceeded(const Cell& result) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnAsyncFunctionSuccessCallback())) {
    session->onAsyncFunctionSuccess(this, cellAsCVarRef(result));
  }

  setResult(result);

  // free m_continuation / m_child later, result may be stored there
  m_continuation = nullptr;
  m_child = nullptr;
}

void c_AsyncFunctionWaitHandle::markAsFailed(const Object& exception) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnAsyncFunctionFailCallback())) {
    session->onAsyncFunctionFail(this, exception);
  }

  setException(exception.get());

  m_continuation = nullptr;
  m_child = nullptr;
}

String c_AsyncFunctionWaitHandle::getName() {
  switch (getState()) {
    case STATE_BLOCKED:
    case STATE_SCHEDULED:
    case STATE_RUNNING:
      if (m_continuation->t_getcalledclass().empty()) {
        return m_continuation->t_getorigfuncname();
      } else {
        return concat3(m_continuation->t_getcalledclass(),
                       "::",
                       m_continuation->t_getorigfuncname());
      }

    default:
      throw FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

c_WaitableWaitHandle* c_AsyncFunctionWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    return m_child.get();
  } else {
    assert(getState() == STATE_SCHEDULED || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_AsyncFunctionWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  switch (getState()) {
    case STATE_BLOCKED:
      // enter child into new context recursively
      m_child->enterContext(ctx_idx);
      setContextIdx(ctx_idx);
      break;

    case STATE_SCHEDULED:
      // reschedule so that we get run
      setContextIdx(ctx_idx);
      getContext()->schedule(this);
      break;

    case STATE_RUNNING: {
      Object e(SystemLib::AllocInvalidOperationExceptionObject(
          "Detected cross-context dependency cycle. You are trying to depend "
          "on something that is running you serially."));
      throw e;
    }

    default:
      assert(false);
  }
}

void c_AsyncFunctionWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // not in a context being exited
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // we were already ran due to duplicit scheduling; the context will be
      // updated thru exitContext() call on the non-blocked wait handle we
      // recursively depend on
      break;

    case STATE_SCHEDULED:
      // move us to the parent context
      setContextIdx(getContextIdx() - 1);

      // reschedule if still in a context
      if (isInContext()) {
        getContext()->schedule(this);
      }

      // recursively move all wait handles blocked by us
      for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
        pwh->exitContextBlocked(ctx_idx);
      }

      break;

    default:
      assert(false);
  }
}

// Get the filename in which execution will proceed when execution resumes.
String c_AsyncFunctionWaitHandle::getFileName() {
  if (m_continuation.isNull()) return empty_string;
  auto ar = m_continuation->actRec();
  auto file = ar->m_func->unit()->filepath()->data();
  if (ar->m_func->originalFilename()) {
    file = ar->m_func->originalFilename()->data();
  }
  return file;
}

// Get the next execution offset
Offset c_AsyncFunctionWaitHandle::getNextExecutionOffset() {
  if (m_continuation.isNull()) return InvalidAbsoluteOffset;
  return m_continuation->offset();
}

// Get the line number on which execution will proceed when execution resumes.
int c_AsyncFunctionWaitHandle::getLineNumber() {
  if (m_continuation.isNull()) return -1;
  auto const unit = m_continuation->actRec()->m_func->unit();
  return unit->getLineNumber(m_continuation->offset());
}

ActRec* c_AsyncFunctionWaitHandle::getActRec() {
  return m_continuation->actRec();
}

///////////////////////////////////////////////////////////////////////////////
}
