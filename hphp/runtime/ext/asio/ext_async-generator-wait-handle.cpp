/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"

#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  const StaticString s_asyncGenerator("<async-generator>");
}

c_AsyncGeneratorWaitHandle::~c_AsyncGeneratorWaitHandle() {
  if (LIKELY(isFinished())) return;
  assertx(!isRunning());
  decRefObj(m_child);
}

c_AsyncGeneratorWaitHandle*
c_AsyncGeneratorWaitHandle::Create(const ActRec* fp,
                                   jit::TCA resumeAddr,
                                   Offset suspendOffset,
                                   c_WaitableWaitHandle* child) {
  assertx(fp);
  assertx(isResumed(fp));
  assertx(fp->func()->isAsyncGenerator());
  assertx(child);
  assertx(child->instanceof(c_WaitableWaitHandle::classof()));
  assertx(!child->isFinished());

  auto const gen = frame_async_generator(fp);
  auto wh = req::make<c_AsyncGeneratorWaitHandle>(gen, child);
  child->getParentChain().addParent(
      wh->m_blockable,
      AsioBlockable::Kind::AsyncGeneratorWaitHandle
  );
  // Implied reference from child via its AsioBlockableChain.
  wh->incRefCount();

  // Set resume address and link the AGWH to the async generator.
  gen->resumable()->setResumeAddr(resumeAddr, suspendOffset);
  gen->attachWaitHandle(req::ptr<c_AsyncGeneratorWaitHandle>(wh));

  wh.get()->m_implicitContext = *ImplicitContext::activeCtx;
  return wh.detach();
}

c_AsyncGeneratorWaitHandle::c_AsyncGeneratorWaitHandle(AsyncGenerator* gen,
                                            c_WaitableWaitHandle* child)
  : c_ResumableWaitHandle(classof(), HeaderKind::WaitHandle,
                type_scan::getIndexForMalloc<c_AsyncGeneratorWaitHandle>())
  , m_generator(gen->toObject())
{
  setState(STATE_BLOCKED);
  setContextIdx(child->getContextIdx());
  m_child = child; // no incref, to avoid leaking parent<-->child cycle
}

void c_AsyncGeneratorWaitHandle::resume() {
  // No refcnt: incref by being executed, decref by no longer in runnable queue.
  assertx(getState() == STATE_READY);
  assertx(m_child->isFinished());
  setState(STATE_RUNNING);

  auto generator = Native::data<AsyncGenerator>(m_generator);
  auto const resumable = generator->resumable();
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
  assertx(!child->isFinished());

  // import child into the current context, throw on cross-context cycles
  asio::enter_context(child, getContextIdx());

  // detect cycles
  detectCycle(child);
}

void c_AsyncGeneratorWaitHandle::onUnblocked() {
  setState(STATE_READY);
  if (isInContext()) {
    // No refcnt: incref by runnable queue, decref by no longer refd by child.
    getContext()->schedule(this);
  } else {
    // Drop implied reference from child.
    decRefObj(this);
  }
}

void c_AsyncGeneratorWaitHandle::await(req::ptr<c_WaitableWaitHandle>&& child) {
  // Prepare child for establishing dependency. May throw.
  prepareChild(child.get());
  this->m_implicitContext = *ImplicitContext::activeCtx;

  // Set up the dependency.
  // No refcnt: incref by ref from child, decref by no longer being executed.
  setState(STATE_BLOCKED);
  m_child = child.detach();
  m_child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::AsyncGeneratorWaitHandle);
}

void c_AsyncGeneratorWaitHandle::ret(TypedValue& result) {
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  tvCopy(result, m_resultOrException);
  parentChain.unblock();
  m_generator.reset();

  // Drop implied reference by being executed.
  decRefObj(this);
}

void c_AsyncGeneratorWaitHandle::fail(ObjectData* exception) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnResumableFail())) {
    session->onResumableFail(this, Object{exception});
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvCopy(make_tv<KindOfObject>(exception), m_resultOrException);
  parentChain.unblock();
  m_generator.reset();

  // Drop implied reference by being executed.
  decRefObj(this);
}

void c_AsyncGeneratorWaitHandle::failCpp() {
  auto const exception = AsioSession::Get()->getAbruptInterruptException();
  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);
  parentChain.unblock();
  m_generator.reset();

  // Drop implied reference by being executed.
  decRefObj(this);
}

String c_AsyncGeneratorWaitHandle::getName() {
  return s_asyncGenerator;
}

c_WaitableWaitHandle* c_AsyncGeneratorWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    assertx(m_child);
    return m_child;
  } else {
    assertx(getState() == STATE_READY || getState() == STATE_RUNNING);
    return nullptr;
  }
}

Resumable* c_AsyncGeneratorWaitHandle::resumable() const {
  auto generator = Native::data<AsyncGenerator>(m_generator);
  return generator->resumable();
}

void c_AsyncGeneratorWaitHandle::exitContext(context_idx_t ctx_idx) {
  assertx(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    decRefObj(this);
    return;
  }

  // not in a context being exited
  assertx(getContextIdx() <= ctx_idx);
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

    case STATE_READY:
      // Recursively move all wait handles blocked by us.
      getParentChain().exitContext(ctx_idx);

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
      assertx(false);
  }
}

void AsioExtension::registerNativeAsyncGeneratorWaitHandle() {
  Native::registerClassExtraDataHandler(
    c_AsyncGeneratorWaitHandle::className(),
    finish_class<c_AsyncGeneratorWaitHandle>);
}

///////////////////////////////////////////////////////////////////////////////
}
