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
#include "hphp/runtime/ext/asio/asio-context-enter.h"

#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_concurrent-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_priority-bridge-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP::asio {
///////////////////////////////////////////////////////////////////////////////

namespace {

// Struct to hold metadate for tracking throughout discover phase.
struct WHInfo {
  c_WaitableWaitHandle* waitHandle;
  ContextStateIndex newStateIdx;
};

struct EnterContextState final {
  using Kind = c_Awaitable::Kind;

  EnterContextState(c_WaitableWaitHandle* root, ContextStateIndex newStateIdx)
  : m_importMap({{root, newStateIdx}}), m_pending({{root, newStateIdx}}) {}

  void enqueue(c_WaitableWaitHandle* node, ContextStateIndex newStateIdx) {
    // Do not need to import this WH if it's already finished or in a higher
    // priority context state.
    if (node->isFinished() || node->getContextStateIndex() >= newStateIdx) {
      return;
    }

    auto it = m_importMap.find(node);
    if (it == m_importMap.end()) {
      m_importMap.insert({node, newStateIdx});
      m_pending.push_back({node, newStateIdx});
    } else {
      // Already imported this WH, but reprocess if the parent is in a higher
      // priority context state.
      auto const currentIdx = it->second;
      if (newStateIdx > currentIdx) {
        m_importMap[node] = newStateIdx;
        m_pending.push_back({node, newStateIdx});
      }
    }
  }

  bool discoverResumable(c_ResumableWaitHandle* node) {
    if (node->getState() == c_ResumableWaitHandle::STATE_RUNNING) {
      SystemLib::throwInvalidOperationExceptionObject(
        "Detected cross-context dependency cycle. You are trying to depend "
        "on something that is running you serially.");
    }

    return node->getState() == c_ResumableWaitHandle::STATE_BLOCKED;
  }

  void discover(c_AsyncFunctionWaitHandle *node,
                ContextStateIndex newStateIdx) {
    if (discoverResumable(node)) {
      if (auto const child = node->getChild()) enqueue(child, newStateIdx);
    }
  }

  void discover(c_AsyncGeneratorWaitHandle* node,
                ContextStateIndex newStateIdx) {
    if (discoverResumable(node)) enqueue(node->getChild(), newStateIdx);
  }

  void discover(c_AwaitAllWaitHandle* node, ContextStateIndex newStateIdx) {
    assertx(node->getState() == c_AwaitAllWaitHandle::STATE_BLOCKED);
    node->forEachChild([this, newStateIdx] (c_WaitableWaitHandle* child) {
      enqueue(child, newStateIdx);
    });
  }

  void discover(c_ConcurrentWaitHandle* node, ContextStateIndex newStateIdx) {
    assertx(node->getState() == c_ConcurrentWaitHandle::STATE_BLOCKED);
    node->forEachChild([this, newStateIdx] (c_WaitableWaitHandle* child) {
      enqueue(child, newStateIdx);
    });
  }

  void discover(c_ConditionWaitHandle* node, ContextStateIndex newStateIdx) {
    assertx(node->getState() == c_ConditionWaitHandle::STATE_BLOCKED);
    enqueue(node->getChild(), newStateIdx);
  }

  // Allow children to run in a lower priority context, unless they've been
  // prioritized, in which case maintain the new state index.
  void discover(c_PriorityBridgeWaitHandle* node,
                ContextStateIndex newStateIdx) {
    assertx(node->getState() == c_PriorityBridgeWaitHandle::STATE_BLOCKED);
    auto child = node->getChild();

    newStateIdx = !node->isPrioritized() && child->getContextStateIndex().isLow()
      ? newStateIdx.toLow()
      : newStateIdx;
    enqueue(child, newStateIdx);
  }

  void discover() {
    while (!m_pending.empty()) {
      auto whInfo = m_pending.back();
      m_pending.pop_back();

      switch (whInfo.waitHandle->getKind()) {
        case Kind::AsyncFunction:
          discover(whInfo.waitHandle->asAsyncFunction(), whInfo.newStateIdx);
          break;
        case Kind::AsyncGenerator:
          discover(whInfo.waitHandle->asAsyncGenerator(), whInfo.newStateIdx);
          break;
        case Kind::AwaitAll:
          discover(whInfo.waitHandle->asAwaitAll(), whInfo.newStateIdx);
          break;
        case Kind::Concurrent:
          discover(whInfo.waitHandle->asConcurrent(), whInfo.newStateIdx);
          break;
        case Kind::Condition:
          discover(whInfo.waitHandle->asCondition(), whInfo.newStateIdx);
          break;
        case Kind::PriorityBridge:
          discover(whInfo.waitHandle->asPriorityBridge(), whInfo.newStateIdx);
          break;
        case Kind::Static:
        case Kind::Reschedule:
        case Kind::Sleep:
        case Kind::ExternalThreadEvent:
          break;
      }
    }
  }

  void enter(c_ResumableWaitHandle* node) {
    if (node->getState() == c_ResumableWaitHandle::STATE_READY) {
      node->getContext()->schedule(node);
      node->incRefCount();
    }
  }

  void enter(c_RescheduleWaitHandle* node) {
    assertx(node->getState() == c_RescheduleWaitHandle::STATE_SCHEDULED);
    node->scheduleInContext();
  }

  void preEnter(c_SleepWaitHandle* node) {
    assertx(node->getState() == c_SleepWaitHandle::STATE_WAITING);
    if (node->isInContext()) {
      node->unregisterFromContext();
    }
  }

  void enter(c_SleepWaitHandle* node) {
    assertx(node->getState() == c_SleepWaitHandle::STATE_WAITING);
    node->registerToContext();
  }

  void preEnter(c_ExternalThreadEventWaitHandle* node) {
    assertx(node->getState() ==
            c_ExternalThreadEventWaitHandle::STATE_WAITING);
    if (node->isInContext()) {
      node->unregisterFromContext();
    }
  }

  void enter(c_ExternalThreadEventWaitHandle* node) {
    assertx(node->getState() ==
            c_ExternalThreadEventWaitHandle::STATE_WAITING);
    node->registerToContext();
  }

  void enter() {
    for (auto const& [handle, newStateIdx] : m_importMap) {
      switch (handle->getKind()) {
        case Kind::Sleep:
          preEnter(handle->asSleep());
          break;
        case Kind::ExternalThreadEvent:
          preEnter(handle->asExternalThreadEvent());
          break;
        case Kind::Static:
        case Kind::AsyncFunction:
        case Kind::AsyncGenerator:
        case Kind::AwaitAll:
        case Kind::Concurrent:
        case Kind::Condition:
        case Kind::Reschedule:
        case Kind::PriorityBridge:
          break;
      }

      handle->setContextStateIndex(newStateIdx);

      switch (handle->getKind()) {
        case Kind::AsyncFunction:
        case Kind::AsyncGenerator:
          enter(handle->asResumable());
          break;
        case Kind::Reschedule:
          enter(handle->asReschedule());
          break;
        case Kind::Sleep:
          enter(handle->asSleep());
          break;
        case Kind::ExternalThreadEvent:
          enter(handle->asExternalThreadEvent());
          break;
        case Kind::Static:
        case Kind::AwaitAll:
        case Kind::Concurrent:
        case Kind::Condition:
        case Kind::PriorityBridge:
          break;
      }
    }
  }

  hphp_fast_map<c_WaitableWaitHandle*, ContextStateIndex> m_importMap;
  std::vector<WHInfo> m_pending;
};
}

void enter_context_impl(c_WaitableWaitHandle *root,
                        ContextStateIndex newStateIdx) {
  assertx(!root->isFinished());
  assertx(root->getContextStateIndex() < newStateIdx);

  EnterContextState ctx(root, newStateIdx);
  ctx.discover();
  ctx.enter();
}

///////////////////////////////////////////////////////////////////////////////
}
