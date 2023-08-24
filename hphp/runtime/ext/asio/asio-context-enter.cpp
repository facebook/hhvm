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
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP::asio {
///////////////////////////////////////////////////////////////////////////////

namespace {
  struct EnterContext final {
    typedef c_Awaitable::Kind Kind;

    EnterContext(c_WaitableWaitHandle* root, context_idx_t ctx_idx)
      : m_importSet({root}), m_pending({root}), m_contextIdx(ctx_idx) {
    }

    void enqueue(c_WaitableWaitHandle* node) {
      if (!node->isFinished() && node->getContextIdx() < m_contextIdx) {
        if (m_importSet.insert(node).second) {
          m_pending.push_back(node);
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

    void discover(c_AsyncFunctionWaitHandle* node) {
      if (discoverResumable(node)) enqueue(node->getChild());
    }

    void discover(c_AsyncGeneratorWaitHandle* node) {
      if (discoverResumable(node)) enqueue(node->getChild());
    }

    void discover(c_AwaitAllWaitHandle* node) {
      assertx(node->getState() == c_AwaitAllWaitHandle::STATE_BLOCKED);
      node->forEachChild([this] (c_WaitableWaitHandle* child) {
        enqueue(child);
      });
    }

    void discover(c_ConcurrentWaitHandle* node) {
      assertx(node->getState() == c_ConcurrentWaitHandle::STATE_BLOCKED);
      node->forEachChild([this] (c_WaitableWaitHandle* child) {
        enqueue(child);
      });
    }

    void discover(c_ConditionWaitHandle* node) {
      assertx(node->getState() == c_ConditionWaitHandle::STATE_BLOCKED);
      enqueue(node->getChild());
    }

    void discover() {
      while (!m_pending.empty()) {
        auto node = m_pending.back();
        m_pending.pop_back();

        switch (node->getKind()) {
          case Kind::AsyncFunction:
            discover(node->asAsyncFunction());
            break;
          case Kind::AsyncGenerator:
            discover(node->asAsyncGenerator());
            break;
          case Kind::AwaitAll:
            discover(node->asAwaitAll());
            break;
          case Kind::Concurrent:
            discover(node->asConcurrent());
            break;
          case Kind::Condition:
            discover(node->asCondition());
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
      for (auto node : m_importSet) {
        switch (node->getKind()) {
          case Kind::Sleep:
            preEnter(node->asSleep());
            break;
          case Kind::ExternalThreadEvent:
            preEnter(node->asExternalThreadEvent());
            break;
          case Kind::Static:
          case Kind::AsyncFunction:
          case Kind::AsyncGenerator:
          case Kind::AwaitAll:
          case Kind::Concurrent:
          case Kind::Condition:
          case Kind::Reschedule:
            break;
        }

        node->setContextIdx(m_contextIdx);

        switch (node->getKind()) {
          case Kind::AsyncFunction:
          case Kind::AsyncGenerator:
            enter(node->asResumable());
            break;
          case Kind::Reschedule:
            enter(node->asReschedule());
            break;
          case Kind::Sleep:
            enter(node->asSleep());
            break;
          case Kind::ExternalThreadEvent:
            enter(node->asExternalThreadEvent());
            break;
          case Kind::Static:
          case Kind::AwaitAll:
          case Kind::Concurrent:
          case Kind::Condition:
            break;
        }
      }
    }

    hphp_fast_set<c_WaitableWaitHandle*> m_importSet;
    std::vector<c_WaitableWaitHandle*> m_pending;
    context_idx_t const m_contextIdx;
  };
}

void enter_context_impl(c_WaitableWaitHandle* root, context_idx_t ctx_idx) {
  assertx(!root->isFinished());
  assertx(root->getContextIdx() < ctx_idx);

  EnterContext ctx(root, ctx_idx);
  ctx.discover();
  ctx.enter();
}

///////////////////////////////////////////////////////////////////////////////
}
