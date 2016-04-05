/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
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
  assert(!isRunning());
  decRefObj(m_child);
}

req::ptr<c_AsyncGeneratorWaitHandle>
c_AsyncGeneratorWaitHandle::Create(AsyncGenerator* gen,
                                   c_WaitableWaitHandle* child) {
  assert(child->instanceof(c_WaitableWaitHandle::classof()));
  assert(!child->isFinished());
  auto wh = req::make<c_AsyncGeneratorWaitHandle>(gen, child);
  child->getParentChain().addParent(
      wh->m_blockable,
      AsioBlockable::Kind::AsyncGeneratorWaitHandle
  );
  wh->incRefCount(); // on behalf of child->parent ptr in AsioBlockableChain
  return wh;
}

c_AsyncGeneratorWaitHandle::c_AsyncGeneratorWaitHandle(AsyncGenerator* gen,
                                            c_WaitableWaitHandle* child)
  : c_ResumableWaitHandle(classof())
  , m_generator(gen->toObject())
{
  setState(STATE_BLOCKED);
  setContextIdx(child->getContextIdx());
  m_child = child; // no incref, to avoid leaking parent<-->child cycle
}

void c_AsyncGeneratorWaitHandle::resume() {
  assert(getState() == STATE_READY);
  assert(m_child->isFinished());
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
  assert(!child->isFinished());

  // import child into the current context, throw on cross-context cycles
  asio::enter_context(child, getContextIdx());

  // detect cycles
  detectCycle(child);
}

void c_AsyncGeneratorWaitHandle::onUnblocked() {
  setState(STATE_READY);
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
  setState(STATE_BLOCKED);
  m_child = child;
  m_child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::AsyncGeneratorWaitHandle);
}

void c_AsyncGeneratorWaitHandle::ret(Cell& result) {
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  cellCopy(result, m_resultOrException);
  parentChain.unblock();
  m_generator.reset();
}

void c_AsyncGeneratorWaitHandle::fail(ObjectData* exception) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnResumableFail())) {
    session->onResumableFail(this, Object{exception});
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  cellCopy(make_tv<KindOfObject>(exception), m_resultOrException);
  parentChain.unblock();
  m_generator.reset();
}

void c_AsyncGeneratorWaitHandle::failCpp() {
  auto const exception = AsioSession::Get()->getAbruptInterruptException();
  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);
  parentChain.unblock();
  m_generator.reset();
}

String c_AsyncGeneratorWaitHandle::getName() {
  return s_asyncGenerator;
}

c_WaitableWaitHandle* c_AsyncGeneratorWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    assert(m_child);
    return m_child;
  } else {
    assert(getState() == STATE_READY || getState() == STATE_RUNNING);
    return nullptr;
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
      assert(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
