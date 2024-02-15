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

#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

req::ptr<c_AwaitAllWaitHandle> c_AwaitAllWaitHandle::Alloc(int32_t cnt) {
  auto size = c_AwaitAllWaitHandle::heapSize(cnt);
  auto mem = tl_heap->objMalloc(size);
  auto handle = new (mem) c_AwaitAllWaitHandle(cnt);
  assertx(handle->hasExactlyOneRef());
  return req::ptr<c_AwaitAllWaitHandle>::attach(handle);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_awaitAll("<await-all>");

  [[noreturn]] NEVER_INLINE
  void failWaitHandle() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a collection of WaitHandle instances");
  }

  c_StaticWaitHandle* returnEmpty() {
    return c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfNull>());
  }

  void prepareChild(TypedValue src, context_idx_t& ctx_idx, uint32_t& cnt) {
    auto const waitHandle = c_Awaitable::fromTV(src);
    if (UNLIKELY(!waitHandle)) failWaitHandle();
    if (waitHandle->isFinished()) return;
    assertx(isa<c_WaitableWaitHandle>(waitHandle));
    auto const child = static_cast<c_WaitableWaitHandle*>(waitHandle);
    ctx_idx = std::min(ctx_idx, child->getContextIdx());
    ++cnt;
  }

  bool addChild(TypedValue src, c_AwaitAllWaitHandle::Node*& dst, uint32_t& idx) {
    auto const waitHandle = c_Awaitable::fromTVAssert(src);
    if (waitHandle->isFinished()) return false;

    waitHandle->incRefCount();
    (--dst)->m_child = static_cast<c_WaitableWaitHandle*>(waitHandle);
    dst->m_index = idx--;
    dst->m_child->getParentChain().addParent(dst->m_blockable,
      AsioBlockable::Kind::AwaitAllWaitHandleNode);
    return true;
  }
}

void HHVM_STATIC_METHOD(AwaitAllWaitHandle, setOnCreateCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnAwaitAllCreate(callback);
}

template<typename Iter>
Object c_AwaitAllWaitHandle::Create(Iter iter) {
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  uint32_t cnt = 0;

  iter([&](TypedValue v) { prepareChild(v, ctx_idx, cnt); });

  if (!cnt) {
    return Object{returnEmpty()};
  }

  auto result = Alloc(cnt);
  auto next = &result->m_children[cnt];
  uint32_t idx = cnt - 1;

  iter([&](TypedValue v) { addChild(v, next, idx); });

  assertx(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return Object{std::move(result)};
}

Object c_AwaitAllWaitHandle::fromArrLike(const ArrayData* ad) {
  return c_AwaitAllWaitHandle::Create([=](auto fn) { IterateV(ad, fn); });
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromVec,
                          const Array& dependencies) {
  auto ad = dependencies.get();
  assertx(ad);
  if (!ad->size()) return Object{returnEmpty()};
  if (!ad->isVanilla()) return c_AwaitAllWaitHandle::fromArrLike(ad);
  assertx(ad->isVanillaVec());
  return c_AwaitAllWaitHandle::Create([=](auto fn) {
    VanillaVec::IterateV(ad, fn);
  });
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromDict,
                          const Array& dependencies) {
  auto ad = dependencies.get();
  assertx(ad);
  if (!ad->size()) return Object{returnEmpty()};
  if (!ad->isVanilla()) return c_AwaitAllWaitHandle::fromArrLike(ad);
  assertx(ad->isVanillaDict());
  return c_AwaitAllWaitHandle::Create([=](auto fn) {
    VanillaDict::IterateV(VanillaDict::as(ad), fn);
  });
}

void c_AwaitAllWaitHandle::initialize(context_idx_t ctx_idx) {
  setState(STATE_BLOCKED);
  setContextIdx(ctx_idx);

  // For AAWH not being finished, accounts for edges from all children.
  incRefCount();

  if (UNLIKELY(AsioSession::Get()->hasOnAwaitAllCreate())) {
    VecInit dependencies(m_cap);
    for (int32_t idx = m_cap - 1; idx >= 0; --idx) {
      auto const child = make_tv<KindOfObject>(m_children[idx].m_child);
      dependencies.append(child);
    }
    AsioSession::Get()->onAwaitAllCreate(this, dependencies.toArray());
  }
}

void c_AwaitAllWaitHandle::onUnblocked(uint32_t idx) {
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

void c_AwaitAllWaitHandle::markAsFinished() {
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  tvWriteNull(m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

void c_AwaitAllWaitHandle::markAsFailed(const Object& exception) {
  for (uint32_t idx = 0; idx < m_cap; idx++) {
    auto const child = m_children[idx].m_child;
    if (!child->isFinished()) {
      // Remove the current AAWH from the parent chain of all children.
      child->getParentChain().removeFromChain(&m_children[idx].m_blockable);
    }
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(exception.get(), &m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

String c_AwaitAllWaitHandle::getName() {
  return s_awaitAll;
}

c_WaitableWaitHandle* c_AwaitAllWaitHandle::getChild() {
  assertx(getState() == STATE_BLOCKED);
  assertx(m_unfinished < m_cap);
  return m_children[m_unfinished].m_child;
}

///////////////////////////////////////////////////////////////////////////////

void AsioExtension::registerNativeAwaitAllWaitHandle() {
#define AAWH_SME(meth) \
  HHVM_STATIC_MALIAS(HH\\AwaitAllWaitHandle, meth, AwaitAllWaitHandle, meth)
  AAWH_SME(fromVec);
  AAWH_SME(fromDict);
  AAWH_SME(setOnCreateCallback);
#undef AAWH_SME

  Native::registerClassExtraDataHandler(
    c_AwaitAllWaitHandle::className(), finish_class<c_AwaitAllWaitHandle>);
}

///////////////////////////////////////////////////////////////////////////////
}
