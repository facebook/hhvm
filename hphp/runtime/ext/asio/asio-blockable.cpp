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
#include "hphp/runtime/ext/asio/asio-blockable.h"

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
using Kind = AsioBlockable::Kind;

template<class T>
inline T* getContainingObject(const AsioBlockable* blockable) {
  return reinterpret_cast<T*>(
    const_cast<char*>(
      reinterpret_cast<const char*>(blockable) -
      T::blockableOff()));
}

inline c_AsyncFunctionWaitHandle::Node* getAsyncFunctionWaitHandleNode(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::AsyncFunctionWaitHandleNode);
  return getContainingObject<c_AsyncFunctionWaitHandle::Node>(blockable);
}

inline c_AsyncGeneratorWaitHandle* getAsyncGeneratorWaitHandle(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::AsyncGeneratorWaitHandle);
  return getContainingObject<c_AsyncGeneratorWaitHandle>(blockable);
}

inline c_AwaitAllWaitHandle::Node* getAwaitAllWaitHandleNode(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::AwaitAllWaitHandleNode);
  return getContainingObject<c_AwaitAllWaitHandle::Node>(blockable);
}

inline c_ConditionWaitHandle* getConditionWaitHandle(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::ConditionWaitHandle);
  return getContainingObject<c_ConditionWaitHandle>(blockable);
}

inline void exitContextImpl(
  c_WaitableWaitHandle* waitHandle,
  context_idx_t ctx_idx
) {
  assertx(AsioSession::Get()->getContext(ctx_idx));
  assertx(!waitHandle->isFinished());
  assertx(waitHandle->getContextIdx() <= ctx_idx);

  // Not in a context being exited.
  if (waitHandle->getContextIdx() != ctx_idx) {
    return;
  }

  // Move the wait handle to the parent context.
  waitHandle->setContextIdx(ctx_idx - 1);

  // Recursively move all the parents to the parent context.
  waitHandle->getParentChain().exitContext(ctx_idx);
}

inline void exitContextImpl(
  c_AsyncFunctionWaitHandle::Node* node,
  context_idx_t ctx_idx
) {
  if (node->isFirstUnfinishedChild()) {
    exitContextImpl(node->getWaitHandle(), ctx_idx);
  }
}

inline void exitContextImpl(
  c_AwaitAllWaitHandle::Node* node,
  context_idx_t ctx_idx
) {
  if (node->isFirstUnfinishedChild()) {
    exitContextImpl(node->getWaitHandle(), ctx_idx);
  }
}

inline void exitContextImpl(
  c_ConditionWaitHandle* waitHandle,
  context_idx_t ctx_idx
) {
  // ConditionWaitHandle may finish before its children do.
  if (waitHandle->isFinished()) {
    return;
  }

  exitContextImpl(static_cast<c_WaitableWaitHandle*>(waitHandle), ctx_idx);
}

} // anon namespace

c_WaitableWaitHandle* AsioBlockable::getWaitHandle() const {
  switch (getKind()) {
    case Kind::AsyncFunctionWaitHandleNode:
      return getAsyncFunctionWaitHandleNode(this)->getWaitHandle();
    case Kind::AsyncGeneratorWaitHandle:
      return getAsyncGeneratorWaitHandle(this);
    case Kind::AwaitAllWaitHandleNode:
      return getAwaitAllWaitHandleNode(this)->getWaitHandle();
    case Kind::ConditionWaitHandle:
      return getConditionWaitHandle(this);
  }
  not_reached();
}

void AsioBlockableChain::unblock() {
  while (auto cur = m_firstParent) {
    m_firstParent = cur->getNextParent();
    cur->updateNextParent(nullptr);
    // the onUnblocked handler may free cur
    switch (cur->getKind()) {
      case Kind::AsyncFunctionWaitHandleNode:
        getAsyncFunctionWaitHandleNode(cur)->onUnblocked();
        break;
      case Kind::AsyncGeneratorWaitHandle:
        getAsyncGeneratorWaitHandle(cur)->onUnblocked();
        break;
      case Kind::AwaitAllWaitHandleNode:
        getAwaitAllWaitHandleNode(cur)->onUnblocked();
        break;
      case Kind::ConditionWaitHandle:
        getConditionWaitHandle(cur)->onUnblocked();
        break;
    }
  }
}

void AsioBlockableChain::exitContext(context_idx_t ctx_idx) {
  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    switch (cur->getKind()) {
      case Kind::AsyncFunctionWaitHandleNode:
        exitContextImpl(getAsyncFunctionWaitHandleNode(cur), ctx_idx);
        break;
      case Kind::AsyncGeneratorWaitHandle:
        exitContextImpl(getAsyncGeneratorWaitHandle(cur), ctx_idx);
        break;
      case Kind::AwaitAllWaitHandleNode:
        exitContextImpl(getAwaitAllWaitHandleNode(cur), ctx_idx);
        break;
      case Kind::ConditionWaitHandle:
        exitContextImpl(getConditionWaitHandle(cur), ctx_idx);
        break;
    }
  }
}

// Currently only AAWH utilizes this to handle failures.
void AsioBlockableChain::removeFromChain(AsioBlockable* ab) {
  AsioBlockable* prev = nullptr;
  for (AsioBlockable* cur = m_firstParent, *next; cur; cur = next) {
    next = cur->getNextParent();
    if (ab == cur) {
      // Found the AAWH we need to remove
      assertx(cur->getKind() == Kind::AwaitAllWaitHandleNode);
      if (!prev) {
        m_firstParent = next;
      } else {
        prev->updateNextParent(next);
      }
      cur->updateNextParent(nullptr);
      return;
    }
    prev = cur;
  }
  // We should always be able to find the parent.
  assertx(false);
}

c_WaitableWaitHandle*
AsioBlockableChain::firstInContext(context_idx_t ctx_idx) {
  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    auto const wh = cur->getWaitHandle();
    if (!wh->isFinished() && wh->getContextIdx() == ctx_idx) {
      return wh;
    }
  }
  return nullptr;
}

void AsioBlockableChain::UnblockJitHelper(ActRec* ar,
                                          TypedValue* sp,
                                          AsioBlockableChain chain) {
  assertx(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  SCOPE_EXIT { tl_regState = VMRegState::DIRTY; };

  auto prevAr = g_context->getOuterVMFrame(ar);
  const Func* prevF = prevAr->m_func;
  auto& regs = vmRegs();
  regs.stack.top() = sp;
  assertx(vmStack().isValidAddress((uintptr_t)vmsp()));
  regs.pc = prevF->unit()->at(prevF->base() + ar->callOffset());
  regs.fp = prevAr;
  regs.jitReturnAddr = nullptr;

  chain.unblock();
}

///////////////////////////////////////////////////////////////////////////////
}
