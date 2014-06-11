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

#include "hphp/runtime/ext/asio/async_generator_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/async_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  const StaticString s_asyncGenerator("<async-generator>");

  void checkCreateErrors(c_WaitableWaitHandle* child) {
    AsioSession* session = AsioSession::Get();
    if (session->isInContext()) {
      child->enterContext(session->getCurrentContextIdx());
    }
  }
}

c_AsyncGeneratorWaitHandle::~c_AsyncGeneratorWaitHandle() {
  if (LIKELY(isFinished())) {
    return;
  }

  assert(!isRunning());
  decRefObj(m_generator);
  decRefObj(m_child);
}

void c_AsyncGeneratorWaitHandle::t___construct() {
  // gen-ext-hhvm requires at least one declared method in the class to work
  not_reached();
}

c_AsyncGeneratorWaitHandle*
c_AsyncGeneratorWaitHandle::Create(c_AsyncGenerator* gen,
                                   c_WaitableWaitHandle* child) {
  assert(child);
  assert(child->instanceof(c_WaitableWaitHandle::classof()));
  assert(!child->isFinished());

  checkCreateErrors(child);
  auto const waitHandle = NEWOBJ(c_AsyncGeneratorWaitHandle)();
  waitHandle->incRefCount();
  waitHandle->initialize(gen, child);
  return waitHandle;
}

void c_AsyncGeneratorWaitHandle::initialize(c_AsyncGenerator* gen,
                                            c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);
  m_generator = gen;
  m_generator->incRefCount();
  m_child = child;

  blockOn(child);
  incRefCount();
}

void c_AsyncGeneratorWaitHandle::resume() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    decRefObj(this);
    return;
  }

  assert(getState() == STATE_SCHEDULED);
  assert(m_child->isFinished());
  setState(STATE_RUNNING);

  auto const resumable = m_generator->resumable();
  resumable->actRec()->setReturnVMExit();

  if (LIKELY(m_child->isSucceeded())) {
    // child succeeded, pass the result to the async generator
    g_context->resumeAsyncFunc(resumable, m_child, m_child->getResult());
  } else {
    // child failed, raise the exception inside the async generator
    g_context->resumeAsyncFuncThrow(resumable, m_child,
                                    m_child->getException());
  }
}

void c_AsyncGeneratorWaitHandle::prepareChild(c_WaitableWaitHandle* child) {
  assert(!child->isFinished());

  // import child into the current context, throw on cross-context cycles
  child->enterContext(getContextIdx());

  // detect cycles
  if (UNLIKELY(isDescendantOf(child))) {
    Object e(createCycleException(child));
    throw e;
  }
}

void c_AsyncGeneratorWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  } else {
    decRefObj(this);
  }
}

void c_AsyncGeneratorWaitHandle::await(c_WaitableWaitHandle* child) {
  // Prepare child for establishing dependency. May throw.
  prepareChild(child);

  // Set up the dependency.
  m_child = child;
  setState(STATE_BLOCKED);
  blockOn(m_child);
}

void c_AsyncGeneratorWaitHandle::ret(Cell& result) {
  auto const parentChain = getFirstParent();
  setState(STATE_SUCCEEDED);
  cellCopy(result, m_resultOrException);
  UnblockChain(parentChain);
  decRefObj(m_generator);
  decRefObj(this);
}

void c_AsyncGeneratorWaitHandle::fail(ObjectData* exception) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnResumableFailCallback())) {
    session->onResumableFail(this, exception);
  }

  auto const parentChain = getFirstParent();
  setState(STATE_FAILED);
  cellCopy(make_tv<KindOfObject>(exception), m_resultOrException);
  UnblockChain(parentChain);
  decRefObj(m_generator);
  decRefObj(this);
}

void c_AsyncGeneratorWaitHandle::failCpp() {
  auto const exception = AsioSession::Get()->getAbruptInterruptException();
  auto const parentChain = getFirstParent();
  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);
  UnblockChain(parentChain);
  decRefObj(m_generator);
  decRefObj(this);
}

String c_AsyncGeneratorWaitHandle::getName() {
  return s_asyncGenerator;
}

c_WaitableWaitHandle* c_AsyncGeneratorWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    assert(m_child);
    return m_child;
  } else {
    assert(getState() == STATE_SCHEDULED || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_AsyncGeneratorWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  switch (getState()) {
    case STATE_BLOCKED:
      // enter child into new context recursively
      assert(m_child);
      m_child->enterContext(ctx_idx);
      setContextIdx(ctx_idx);
      break;

    case STATE_SCHEDULED:
      // reschedule so that we get run
      setContextIdx(ctx_idx);
      getContext()->schedule(this);
      incRefCount();
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

void c_AsyncGeneratorWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    decRefObj(this);
    return;
  }

  // not in a context being exited
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    decRefObj(this);
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // we were already ran due to duplicit scheduling; the context will be
      // updated thru exitContext() call on the non-blocked wait handle we
      // recursively depend on
      decRefObj(this);
      break;

    case STATE_SCHEDULED:
      // Recursively move all wait handles blocked by us.
      for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
        pwh->exitContextBlocked(ctx_idx);
      }

      // Move us to the parent context.
      setContextIdx(getContextIdx() - 1);

      // Reschedule if still in a context.
      if (isInContext()) {
        getContext()->schedule(this);
      } else {
        decRefObj(this);
      }

      break;

    default:
      assert(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
