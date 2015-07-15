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

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/bytecode-defs.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void delete_AsyncFunctionWaitHandle(ObjectData* od, const Class*) {
  auto wh = static_cast<c_AsyncFunctionWaitHandle*>(od);
  Resumable::Destroy(wh->resumable()->size(), wh);
}

///////////////////////////////////////////////////////////////////////////////

c_AsyncFunctionWaitHandle::~c_AsyncFunctionWaitHandle() {
  if (LIKELY(isFinished())) {
    return;
  }

  assert(!isRunning());
  frame_free_locals_inl_no_hook<false>(actRec(), actRec()->func()->numLocals());
  decRefObj(m_children[0].getChild());
}

void c_AsyncFunctionWaitHandle::t___construct() {
  // gen-ext-hhvm requires at least one declared method in the class to work
  not_reached();
}

namespace {
  const StaticString s__closure_("{closure}");
}

template <bool mayUseVV>
c_AsyncFunctionWaitHandle*
c_AsyncFunctionWaitHandle::Create(const ActRec* fp,
                                  size_t numSlots,
                                  jit::TCA resumeAddr,
                                  Offset resumeOffset,
                                  c_WaitableWaitHandle* child) {
  assert(fp);
  assert(!fp->resumed());
  assert(fp->func()->isAsyncFunction());
  assert(child);
  assert(child->instanceof(c_WaitableWaitHandle::classof()));
  assert(!child->isFinished());

  const size_t frameSize = Resumable::getFrameSize(numSlots);
  const size_t totalSize = sizeof(ResumableNode) + frameSize +
                           sizeof(Resumable) +
                           sizeof(c_AsyncFunctionWaitHandle);
  auto const resumable = Resumable::Create(frameSize, totalSize);
  resumable->initialize<false, mayUseVV>(fp,
                                         resumeAddr,
                                         resumeOffset,
                                         frameSize,
                                         totalSize);
  auto const waitHandle = new (resumable + 1) c_AsyncFunctionWaitHandle();
  assert(waitHandle->hasExactlyOneRef());
  waitHandle->actRec()->setReturnVMExit();
  assert(waitHandle->noDestruct());
  waitHandle->initialize(child);
  return waitHandle;
}

template c_AsyncFunctionWaitHandle*
c_AsyncFunctionWaitHandle::Create<true>(const ActRec* fp,
                                        size_t numSlots,
                                        jit::TCA resumeAddr,
                                        Offset resumeOffset,
                                        c_WaitableWaitHandle* child);

template c_AsyncFunctionWaitHandle*
c_AsyncFunctionWaitHandle::Create<false>(const ActRec* fp,
                                        size_t numSlots,
                                        jit::TCA resumeAddr,
                                        Offset resumeOffset,
                                        c_WaitableWaitHandle* child);

void c_AsyncFunctionWaitHandle::PrepareChild(const ActRec* fp,
                                             c_WaitableWaitHandle* child) {
  frame_afwh(fp)->prepareChild(child);
}

void c_AsyncFunctionWaitHandle::initialize(c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);
  setContextIdx(child->getContextIdx());
  m_children[0].setChild(child);
  incRefCount();
}

void c_AsyncFunctionWaitHandle::resume() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    decRefObj(this);
    return;
  }

  auto const child = m_children[0].getChild();
  assert(getState() == STATE_SCHEDULED);
  assert(child->isFinished());
  setState(STATE_RUNNING);

  if (LIKELY(child->isSucceeded())) {
    // child succeeded, pass the result to the async function
    g_context->resumeAsyncFunc(resumable(), child, child->getResult());
  } else {
    // child failed, raise the exception inside the async function
    g_context->resumeAsyncFuncThrow(resumable(), child,
                                    child->getException());
  }
}

void c_AsyncFunctionWaitHandle::prepareChild(c_WaitableWaitHandle* child) {
  assert(!child->isFinished());

  // import child into the current context, throw on cross-context cycles
  asio::enter_context(child, getContextIdx());

  // detect cycles
  detectCycle(child);
}

void c_AsyncFunctionWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  } else {
    decRefObj(this);
  }
}

void c_AsyncFunctionWaitHandle::await(Offset resumeOffset,
                                      c_WaitableWaitHandle* child) {
  // Prepare child for establishing dependency. May throw.
  prepareChild(child);

  // Suspend the async function.
  resumable()->setResumeAddr(nullptr, resumeOffset);

  // Set up the dependency.
  setState(STATE_BLOCKED);
  m_children[0].setChild(child);
}

void c_AsyncFunctionWaitHandle::ret(Cell& result) {
  assert(isRunning());
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  cellCopy(result, m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

/**
 * Mark the wait handle as failed due to PHP exception.
 *
 * - consumes reference of the given Exception object
 */
void c_AsyncFunctionWaitHandle::fail(ObjectData* exception) {
  assert(isRunning());
  assert(exception);
  assert(exception->instanceof(SystemLib::s_ExceptionClass));

  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnResumableFail())) {
    try {
      session->onResumableFail(this, exception);
    } catch (...) {
      // TODO(#4557954) Make unwinder able to deal with new exceptions better.
      handle_destructor_exception("AsyncFunctionWaitHandle fail callback");
    }
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  cellCopy(make_tv<KindOfObject>(exception), m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

/**
 * Mark the wait handle as failed due to unexpected abrupt interrupt.
 */
void c_AsyncFunctionWaitHandle::failCpp() {
  assert(isRunning());
  auto const exception = AsioSession::Get()->getAbruptInterruptException();
  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

String c_AsyncFunctionWaitHandle::getName() {
  switch (getState()) {
    case STATE_BLOCKED:
    case STATE_SCHEDULED:
    case STATE_RUNNING: {
      auto func = actRec()->func();
      if (!actRec()->getThisOrClass() ||
          func->cls()->attrs() & AttrNoOverride) {
        auto name = func->fullName();
        if (func->isClosureBody()) {
          const char* p = strchr(name->data(), ':');
          if (p) {
            return
              concat(String(name->data(), p + 1 - name->data(), CopyString),
                     s__closure_);
          } else {
            return s__closure_;
          }
        }
        return const_cast<StringData*>(name);
      }
      String funcName;
      if (actRec()->func()->isClosureBody()) {
        // Can we do better than this?
        funcName = s__closure_;
      } else {
        funcName = const_cast<StringData*>(actRec()->func()->name());
      }

      String clsName;
      if (actRec()->hasThis()) {
        clsName = const_cast<StringData*>(actRec()->getThis()->
                                          getVMClass()->name());
      } else if (actRec()->hasClass()) {
        clsName = const_cast<StringData*>(actRec()->getClass()->name());
      } else {
        return funcName;
      }

      return concat3(clsName, "::", funcName);
    }

    default:
      throw FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

c_WaitableWaitHandle* c_AsyncFunctionWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    return m_children[0].getChild();
  } else {
    assert(getState() == STATE_SCHEDULED || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_AsyncFunctionWaitHandle::exitContext(context_idx_t ctx_idx) {
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

// Get the filename in which execution will proceed when execution resumes.
String c_AsyncFunctionWaitHandle::getFileName() {
  if (actRec()->func()->originalFilename()) {
    return actRec()->func()->originalFilename()->data();
  } else {
    return actRec()->func()->unit()->filepath()->data();
  }
}

// Get the next execution offset
Offset c_AsyncFunctionWaitHandle::getNextExecutionOffset() {
  if (isFinished()) {
    return InvalidAbsoluteOffset;
  }

  always_assert(!isRunning());
  return resumable()->resumeOffset();
}

// Get the line number on which execution will proceed when execution resumes.
int c_AsyncFunctionWaitHandle::getLineNumber() {
  if (isFinished()) {
    return -1;
  }

  always_assert(!isRunning());
  return actRec()->func()->unit()->getLineNumber(resumable()->resumeOffset());
}

///////////////////////////////////////////////////////////////////////////////
}
