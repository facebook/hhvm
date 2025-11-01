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

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_concurrent-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_priority-bridge-wait-handle.h"
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

inline c_ConcurrentWaitHandle::Node* getConcurrentWaitHandleNode(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::ConcurrentWaitHandleNode);
  return getContainingObject<c_ConcurrentWaitHandle::Node>(blockable);
}

inline c_ConditionWaitHandle* getConditionWaitHandle(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::ConditionWaitHandle);
  return getContainingObject<c_ConditionWaitHandle>(blockable);
}

inline c_PriorityBridgeWaitHandle* getPriorityBridgeWaitHandle(
  const AsioBlockable* blockable
) {
  assertx(blockable->getKind() == Kind::PriorityBridgeWaitHandle);
  return getContainingObject<c_PriorityBridgeWaitHandle>(blockable);
}

inline void exitContextImpl(
  std::vector<AsioBlockableChain>& worklist,
  c_WaitableWaitHandle* waitHandle,
  ContextIndex contextIdx
) {
  assertx(AsioSession::Get()->getContext(contextIdx));
  assertx(!waitHandle->isFinished());
  assertx(waitHandle->getContextIndex() <= contextIdx);

  // Not in a context being exited.
  if (waitHandle->getContextIndex() != contextIdx) {
    return;
  }

  // Move the wait handle to the parent context.
  waitHandle->setContextStateIndex(contextIdx.parent().toRegular());

  // Request exit to the parent context for all parents.
  worklist.emplace_back(waitHandle->getParentChain());
}

inline void exitContextImpl(
  std::vector<AsioBlockableChain>& worklist,
  c_AsyncFunctionWaitHandle::Node* node,
  ContextIndex contextIdx
) {
  if (node->isFirstUnfinishedChild()) {
    exitContextImpl(worklist, node->getWaitHandle(), contextIdx);
  }
}

inline void exitContextImpl(
  std::vector<AsioBlockableChain>& worklist,
  c_AwaitAllWaitHandle::Node* node,
  ContextIndex contextIdx
) {
  if (node->isFirstUnfinishedChild()) {
    exitContextImpl(worklist, node->getWaitHandle(), contextIdx);
  }
}

inline void exitContextImpl(
  std::vector<AsioBlockableChain>& worklist,
  c_ConcurrentWaitHandle::Node* node,
  ContextIndex contextIdx
) {
  if (node->isFirstUnfinishedChild()) {
    exitContextImpl(worklist, node->getWaitHandle(), contextIdx);
  }
}

inline void exitContextImpl(
  std::vector<AsioBlockableChain>& worklist,
  c_ConditionWaitHandle* waitHandle,
  ContextIndex contextIdx
) {
  // ConditionWaitHandle may finish before its children do.
  if (waitHandle->isFinished()) {
    return;
  }

  exitContextImpl(
    worklist, static_cast<c_WaitableWaitHandle*>(waitHandle), contextIdx);
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
    case Kind::ConcurrentWaitHandleNode:
      return getConcurrentWaitHandleNode(this)->getWaitHandle();
    case Kind::ConditionWaitHandle:
      return getConditionWaitHandle(this);
    case Kind::PriorityBridgeWaitHandle:
      return getPriorityBridgeWaitHandle(this);
  }
  not_reached();
}

void AsioBlockableChain::unblock() {
  std::vector<AsioBlockableChain> worklist = { *this };
  while (!worklist.empty()) {
    auto const lastParent = worklist.back().m_lastParent;
    worklist.pop_back();

    for (AsioBlockable* cur = lastParent, *next; cur; cur = next) {
      next = cur->getPrevParent();
      cur->updatePrevParent(nullptr);
      // the onUnblocked handler may free cur
      switch (cur->getKind()) {
        case Kind::AsyncFunctionWaitHandleNode:
          getAsyncFunctionWaitHandleNode(cur)->onUnblocked();
          break;
        case Kind::AsyncGeneratorWaitHandle:
          getAsyncGeneratorWaitHandle(cur)->onUnblocked();
          break;
        case Kind::AwaitAllWaitHandleNode:
          getAwaitAllWaitHandleNode(cur)->onUnblocked(worklist);
          break;
        case Kind::ConcurrentWaitHandleNode:
          getConcurrentWaitHandleNode(cur)->onUnblocked(worklist);
          break;
        case Kind::ConditionWaitHandle:
          getConditionWaitHandle(cur)->onUnblocked(worklist);
          break;
        case Kind::PriorityBridgeWaitHandle:
          getPriorityBridgeWaitHandle(cur)->onUnblocked(worklist);
          break;
      }
    }
  }
}

void AsioBlockableChain::exitContext(ContextIndex contextIdx) {
  std::vector<AsioBlockableChain> worklist = { *this };
  while (!worklist.empty()) {
    auto const lastParent = worklist.back().m_lastParent;
    worklist.pop_back();

    for (auto cur = lastParent; cur; cur = cur->getPrevParent()) {
      switch (cur->getKind()) {
        case Kind::AsyncFunctionWaitHandleNode:
          exitContextImpl(
            worklist, getAsyncFunctionWaitHandleNode(cur), contextIdx);
          break;
        case Kind::AsyncGeneratorWaitHandle:
          exitContextImpl(worklist, getAsyncGeneratorWaitHandle(cur), contextIdx);
          break;
        case Kind::AwaitAllWaitHandleNode:
          exitContextImpl(worklist, getAwaitAllWaitHandleNode(cur), contextIdx);
          break;
        case Kind::ConcurrentWaitHandleNode:
          exitContextImpl(worklist, getConcurrentWaitHandleNode(cur), contextIdx);
          break;
        case Kind::ConditionWaitHandle:
          exitContextImpl(worklist, getConditionWaitHandle(cur), contextIdx);
          break;
        case Kind::PriorityBridgeWaitHandle:
          exitContextImpl(worklist, getPriorityBridgeWaitHandle(cur), contextIdx);
          break;
      }
    }
  }
}

// Currently only AAWH and CCWH utilizes this to handle failures.
void AsioBlockableChain::removeFromChain(AsioBlockable* ab) {
  AsioBlockable* next = nullptr;
  for (AsioBlockable* cur = m_lastParent, *prev; cur; cur = prev) {
    prev = cur->getPrevParent();
    if (ab == cur) {
      // Found the AAWH or CCWH we need to remove
      assertx(cur->getKind() == Kind::AwaitAllWaitHandleNode ||
              cur->getKind() == Kind::ConcurrentWaitHandleNode);
      if (!next) {
        m_lastParent = prev;
      } else {
        next->updatePrevParent(prev);
      }
      cur->updatePrevParent(nullptr);
      return;
    }
    next = cur;
  }
  // We should always be able to find the parent.
  assertx(false);
}

c_WaitableWaitHandle*
AsioBlockableChain::firstInContext(ContextStateIndex ctxStateIdx) {
  // Returns the first wait handle in the parent chain that matches the given
  // ContextStateIndex. If no match is found, returns the first wait handle with
  // a matching ContextIndex (if there is one).
  c_WaitableWaitHandle* result = nullptr;
  c_WaitableWaitHandle* ctxResult = nullptr;

  // Iterate through parents looking for a match. Continue until the last
  // matching parent is found (i.e. first one that got enqueued) in order to
  // maintain stacktrace consistency as more parents are added.
  for (auto cur = m_lastParent; cur; cur = cur->getPrevParent()) {
    auto const wh = cur->getWaitHandle();
    if (!wh->isFinished() &&
        wh->getContextStateIndex() == ctxStateIdx) {
      result = wh;
    }
    if (!wh->isFinished() &&
        wh->getContextIndex() == ctxStateIdx.contextIndex()) {
      ctxResult = wh;
    }
  }

  return result ? result : ctxResult;
}

void AsioBlockableChain::Unblock(AsioBlockableChain chain) {
  chain.unblock();
}

///////////////////////////////////////////////////////////////////////////////
}
