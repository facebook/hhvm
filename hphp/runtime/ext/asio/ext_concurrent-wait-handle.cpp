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

#include "hphp/runtime/ext/asio/ext_concurrent-wait-handle.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

req::ptr<c_ConcurrentWaitHandle> c_ConcurrentWaitHandle::Alloc(int32_t cnt) {
  auto size = c_ConcurrentWaitHandle::heapSize(cnt);
  auto mem = tl_heap->objMalloc(size);
  auto handle = new (mem) c_ConcurrentWaitHandle(cnt);
  assertx(handle->hasExactlyOneRef());
  return req::ptr<c_ConcurrentWaitHandle>::attach(handle);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_concurrent("<concurrent>");
}

void HHVM_STATIC_METHOD(ConcurrentWaitHandle, setOnCreateCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnConcurrentCreate(callback);
}

ObjectData* c_ConcurrentWaitHandle::fromFrameNoCheck(
  const ActRec* fp, uint32_t first, uint32_t last, uint32_t cnt
) {
  assertx(cnt);
  assertx(first < last);

  auto result = Alloc(cnt);
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  auto next = &result->m_children[cnt];
  uint32_t idx = cnt;

  for (int64_t i = first; i < last; i++) {
    auto const local = frame_local(fp, i);
    if (tvIsNull(local)) continue;
    auto const waitHandle = c_Awaitable::fromTVAssert(*local);
    if (waitHandle->isFinished()) continue;

    auto const child = static_cast<c_WaitableWaitHandle*>(waitHandle);
    ctx_idx = std::min(ctx_idx, child->getContextIdx());

    child->incRefCount();
    (--next)->m_child = child;
    next->m_index = --idx;
    next->m_child->getParentChain().addParent(
      next->m_blockable,
      AsioBlockable::Kind::ConcurrentWaitHandleNode
    );

    if (!idx) break;
  }

  assertx(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return result.detach();
}

void c_ConcurrentWaitHandle::initialize(context_idx_t ctx_idx) {
  setState(STATE_BLOCKED);
  setContextIdx(ctx_idx);

  // For CCWH not being finished, accounts for edges from all children.
  incRefCount();

  if (UNLIKELY(AsioSession::Get()->hasOnConcurrentCreate())) {
    VecInit dependencies(m_cap);
    for (int32_t idx = m_cap - 1; idx >= 0; --idx) {
      auto const child = make_tv<KindOfObject>(m_children[idx].m_child);
      dependencies.append(child);
    }
    AsioSession::Get()->onConcurrentCreate(this, dependencies.toArray());
  }
}

void c_ConcurrentWaitHandle::onUnblocked(uint32_t idx) {
  assertx(idx <= m_unfinished);
  assertx(getState() == STATE_BLOCKED);

  if (idx == m_unfinished) {
    for (uint32_t next = idx - 1; next < idx; --next) {
      auto const child = m_children[next].m_child;
      if (!child->isFinished()) {
        // Found the next unfinished child.
        m_unfinished = next;

        // Make sure there's no cyclic dependencies.
        try {
          detectCycle(child);
        } catch (const Object& cycle_exception) {
          assertx(cycle_exception->instanceof(SystemLib::getThrowableClass()));
          throwable_recompute_backtrace_from_wh(cycle_exception.get(), this);
          markAsFailed(cycle_exception);
        }
        return;
      }
    }
    // All children finished.
    markAsFinished();
  }
}

void c_ConcurrentWaitHandle::markAsFinished() {
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  tvWriteNull(m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

void c_ConcurrentWaitHandle::markAsFailed(const Object& exception) {
  for (uint32_t idx = 0; idx < m_cap; idx++) {
    auto const child = m_children[idx].m_child;
    if (!child->isFinished()) {
      // Remove the current CCWH from the parent chain of all children.
      child->getParentChain().removeFromChain(&m_children[idx].m_blockable);
    }
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(exception.get(), &m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

String c_ConcurrentWaitHandle::getName() {
  return s_concurrent;
}

c_WaitableWaitHandle* c_ConcurrentWaitHandle::getChild() {
  assertx(getState() == STATE_BLOCKED);
  assertx(m_unfinished < m_cap);
  return m_children[m_unfinished].m_child;
}

///////////////////////////////////////////////////////////////////////////////

void AsioExtension::initConcurrentWaitHandle() {
#define CCWH_SME(meth) \
  HHVM_STATIC_MALIAS(HH\\ConcurrentWaitHandle, meth, ConcurrentWaitHandle, meth)
  CCWH_SME(setOnCreateCallback);
#undef CCWH_SME

  Native::registerClassExtraDataHandler(
    c_ConcurrentWaitHandle::className(), finish_class<c_ConcurrentWaitHandle>);
}

///////////////////////////////////////////////////////////////////////////////
}
