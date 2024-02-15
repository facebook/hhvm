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

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"

#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

#include <folly/SharedMutex.h>
#include <folly/container/F14Map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
size_t s_numAsyncFrameIds = 1;
SrcKey s_asyncFrames[kMaxAsyncFrameId + 1];
folly::F14FastMap<SrcKey, AsyncFrameId, SrcKey::Hasher> s_asyncFrameMap;
folly::SharedMutex s_asyncFrameLock;
}

AsyncFrameId getAsyncFrameId(SrcKey sk) {
  {
    std::shared_lock lock(s_asyncFrameLock);
    auto const it = s_asyncFrameMap.find(sk);
    if (it != s_asyncFrameMap.end()) return it->second;
    if (s_numAsyncFrameIds > kMaxAsyncFrameId) return kInvalidAsyncFrameId;
  }
  std::unique_lock lock(s_asyncFrameLock);
  auto const it = s_asyncFrameMap.insert({sk, s_numAsyncFrameIds});
  if (!it.second) return it.first->second;
  if (s_numAsyncFrameIds > kMaxAsyncFrameId) {
    s_asyncFrameMap.erase(sk);
    return kInvalidAsyncFrameId;
  }
  s_asyncFrames[s_numAsyncFrameIds++] = sk;
  return s_numAsyncFrameIds - 1;
}

SrcKey getAsyncFrame(AsyncFrameId id) {
  assertx(0 < id && id <= kMaxAsyncFrameId);
  assertx(id != kInvalidAsyncFrameId);
  return s_asyncFrames[id];
}

///////////////////////////////////////////////////////////////////////////////

bool c_AsyncFunctionWaitHandle::hasTailFrames() const {
  return tailFrame(kNumTailFrames - 1) != kInvalidAsyncFrameId;
}

size_t c_AsyncFunctionWaitHandle::firstTailFrameIndex() const {
  assertx(hasTailFrames());
  for (auto i = 0; i < kNumTailFrames; i++) {
    if (tailFrame(i) != kInvalidAsyncFrameId) return i;
  }
  assertx(false);
  not_reached();
}

size_t c_AsyncFunctionWaitHandle::lastTailFrameIndex() const {
  return kNumTailFrames;
}

AsyncFrameId c_AsyncFunctionWaitHandle::tailFrame(size_t index) const {
  assertx(0 <= index && index < kNumTailFrames);
  return m_tailFrameIds[kNumTailFrames - index - 1];
}

///////////////////////////////////////////////////////////////////////////////

c_AsyncFunctionWaitHandle::~c_AsyncFunctionWaitHandle() {
  if (LIKELY(isFinished())) {
    return;
  }

  assertx(!isRunning());
  frame_free_locals_inl_no_hook(actRec(), actRec()->func()->numLocals());
  decRefObj(m_children[0].getChild());
}

namespace {
  const StaticString s__closure_("{closure}");
}

c_AsyncFunctionWaitHandle*
c_AsyncFunctionWaitHandle::Create(const ActRec* fp,
                                  size_t numSlots,
                                  jit::TCA resumeAddr,
                                  Offset suspendOffset,
                                  c_WaitableWaitHandle* child) {
  assertx(fp);
  assertx(!isResumed(fp));
  assertx(fp->func()->isAsyncFunction());
  assertx(child);
  assertx(child->instanceof(c_WaitableWaitHandle::classof()));
  assertx(!child->isFinished());

  const size_t frameSize = Resumable::getFrameSize(numSlots);
  const size_t totalSize = sizeof(NativeNode) + frameSize + sizeof(Resumable) +
                           sizeof(c_AsyncFunctionWaitHandle);
  auto const resumable = Resumable::Create(frameSize, totalSize);
  resumable->initialize<false>(fp,
                               resumeAddr,
                               suspendOffset,
                               frameSize,
                               totalSize);
  auto const waitHandle = new (resumable + 1) c_AsyncFunctionWaitHandle();
  assertx(waitHandle->hasExactlyOneRef());
  waitHandle->actRec()->setReturnVMExit();
  waitHandle->m_packedTailFrameIds = -1;
  assertx(!waitHandle->hasTailFrames());
  waitHandle->initialize(child);
  waitHandle->m_implicitContext = *ImplicitContext::activeCtx;
  return waitHandle;
}

void c_AsyncFunctionWaitHandle::PrepareChild(const ActRec* fp,
                                             c_WaitableWaitHandle* child) {
  frame_afwh(fp)->prepareChild(child);
}

void c_AsyncFunctionWaitHandle::initialize(c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);
  setContextIdx(child->getContextIdx());
  m_children[0].setChild(child);
  incRefCount(); // account for child->this back-reference
}

void c_AsyncFunctionWaitHandle::resume() {
  auto const child = m_children[0].getChild();
  assertx(getState() == STATE_READY);
  assertx(child->isFinished());
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
  assertx(!child->isFinished());

  // import child into the current context, throw on cross-context cycles
  asio::enter_context(child, getContextIdx());

  // detect cycles
  detectCycle(child);
}

void c_AsyncFunctionWaitHandle::onUnblocked() {
  setState(STATE_READY);
  if (isInContext()) {
    if (isFastResumable()) {
      getContext()->scheduleFast(this);
    } else {
      getContext()->schedule(this);
    }
  } else {
    decRefObj(this);
  }
}

void c_AsyncFunctionWaitHandle::await(Offset suspendOffset,
                                      req::ptr<c_WaitableWaitHandle>&& child) {
  // Prepare child for establishing dependency. May throw.
  prepareChild(child.get());
  this->m_implicitContext = *ImplicitContext::activeCtx;

  // Suspend the async function.
  resumable()->setResumeAddr(nullptr, suspendOffset);

  // Set up the dependency.
  setState(STATE_BLOCKED);
  m_children[0].setChild(child.detach());
}

void c_AsyncFunctionWaitHandle::ret(TypedValue& result) {
  assertx(isRunning());
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  tvCopy(result, m_resultOrException);
  parentChain.unblock();
}

/**
 * Mark the wait handle as failed due to PHP exception.
 *
 * - consumes reference of the given Exception object
 */
void c_AsyncFunctionWaitHandle::fail(ObjectData* exception) {
  assertx(isRunning());
  assertx(exception);
  assertx(exception->instanceof(SystemLib::getThrowableClass()));

  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnResumableFail())) {
    try {
      session->onResumableFail(this, Object{exception});
    } catch (...) {
      // TODO(#4557954) Make unwinder able to deal with new exceptions better.
      handle_destructor_exception("AsyncFunctionWaitHandle fail callback");
    }
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvCopy(make_tv<KindOfObject>(exception), m_resultOrException);
  parentChain.unblock();
}

/**
 * Mark the wait handle as failed due to unexpected abrupt interrupt.
 */
void c_AsyncFunctionWaitHandle::failCpp() {
  assertx(isRunning());
  auto const exception = AsioSession::Get()->getAbruptInterruptException();
  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);
  parentChain.unblock();
}

String c_AsyncFunctionWaitHandle::getName() {
  switch (getState()) {
    case STATE_BLOCKED:
    case STATE_READY:
    case STATE_RUNNING: {
      auto func = actRec()->func();
      if (!func->cls() ||
          func->cls()->attrs() & AttrNoOverride ||
          actRec()->localsDecRefd()) {
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
        return String{const_cast<StringData*>(name)};
      }

      auto const cls = actRec()->hasThis() ?
        actRec()->getThis()->getVMClass() :
        actRec()->getClass();

      if (cls == func->cls() && !func->isClosureBody()) {
        return String{const_cast<StringData*>(func->fullName())};
      }

      StrNR funcName(func->isClosureBody() ? s__closure_.get() : func->name());

      return concat3(cls->nameStr(), "::", funcName);
    }

    default:
      raise_fatal_error(
          "Invariant violation: encountered unexpected state");
  }
}

c_WaitableWaitHandle* c_AsyncFunctionWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    return m_children[0].getChild();
  } else {
    assertx(getState() == STATE_READY || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_AsyncFunctionWaitHandle::exitContext(context_idx_t ctx_idx) {
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
        if (isFastResumable()) {
          getContext()->scheduleFast(this);
        } else {
          getContext()->schedule(this);
        }
      } else {
        decRefObj(this);
      }
      break;

    default:
      assertx(false);
  }
}

// Get the filename in which execution will proceed when execution resumes.
String c_AsyncFunctionWaitHandle::getFilename() {
  if (actRec()->func()->originalFilename()) {
    return actRec()->func()->originalFilename()->data();
  } else {
    return actRec()->func()->unit()->filepath()->data();
  }
}

// Get the next execution offset
Offset c_AsyncFunctionWaitHandle::getNextExecutionOffset() {
  assertx(!isFinished());
  always_assert(!isRunning());
  return resumable()->resumeFromAwaitOffset();
}

void AsioExtension::registerNativeAsyncFunctionWaitHandle() {
  Native::registerClassExtraDataHandler(
    c_AsyncFunctionWaitHandle::className(),
    finish_class<c_AsyncFunctionWaitHandle>);
}

///////////////////////////////////////////////////////////////////////////////
}
