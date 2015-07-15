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

#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/proxy-array.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

req::ptr<c_AwaitAllWaitHandle> c_AwaitAllWaitHandle::Alloc(int32_t cnt) {
  auto size = c_AwaitAllWaitHandle::heapSize(cnt);
  auto mem = MM().objMalloc(size);
  auto handle = new (mem) c_AwaitAllWaitHandle(cnt);
  assert(handle->hasExactlyOneRef());
  return req::ptr<c_AwaitAllWaitHandle>::attach(handle);
}

void delete_AwaitAllWaitHandle(ObjectData* od, const Class*) {
  auto const waitHandle = static_cast<c_AwaitAllWaitHandle*>(od);
  auto bytes = waitHandle->heapSize();
  waitHandle->~c_AwaitAllWaitHandle();
  MM().objFree(waitHandle, bytes);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_awaitAll("<await-all>");

  NEVER_INLINE __attribute__((__noreturn__))
  void failArray() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be an array");
  }

  NEVER_INLINE __attribute__((__noreturn__))
  void failMap() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a Map");
  }

  NEVER_INLINE __attribute__((__noreturn__))
  void failVector() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a Vector");
  }

  NEVER_INLINE __attribute__((__noreturn__))
  void failWaitHandle() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a collection of WaitHandle instances");
  }

  c_StaticWaitHandle* returnEmpty() {
    return c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfNull>());
  }

  void prepareChild(const Cell* src, context_idx_t& ctx_idx, int32_t& cnt) {
    auto const waitHandle = c_WaitHandle::fromCell(src);
    if (UNLIKELY(!waitHandle)) failWaitHandle();
    if (waitHandle->isFinished()) return;
    auto const child = static_cast<c_WaitableWaitHandle*>(waitHandle);
    ctx_idx = std::min(ctx_idx, child->getContextIdx());
    ++cnt;
  }

  void addChild(const Cell* src, c_WaitableWaitHandle**& dst) {
    auto const waitHandle = c_WaitHandle::fromCellAssert(src);
    if (waitHandle->isFinished()) return;
    waitHandle->incRefCount();
    *(--dst) = static_cast<c_WaitableWaitHandle*>(waitHandle);
  }
}

void c_AwaitAllWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnAwaitAllCreate(callback);
}

Object c_AwaitAllWaitHandle::ti_fromarray(const Array& dependencies) {
  auto ad = dependencies.get();
  assert(ad);
  if (!ad->size()) return returnEmpty();

retry:
  switch (ad->kind()) {
    case ArrayData::kPackedKind:
      return FromPackedArray(ad);

    case ArrayData::kMixedKind:
    case ArrayData::kStructKind:
      return FromMixedArray(MixedArray::asMixed(ad));

    case ArrayData::kProxyKind:
      ad = ProxyArray::innerArr(ad);
      goto retry;

    case ArrayData::kApcKind:
    case ArrayData::kGlobalsKind:
      // APC can't store WaitHandles, GlobalsArray is used only for
      // $GLOBALS, which contain non-WaitHandles.
      failArray();

    case ArrayData::kEmptyKind:
      // Handled by dependencies->size() check.
      not_reached();

    case ArrayData::kNumKinds:
      not_reached();
  }

  not_reached();
}

Object c_AwaitAllWaitHandle::ti_frommap(const Variant& dependencies) {
  if (LIKELY(dependencies.isObject())) {
    auto obj = dependencies.getObjectData();
    if (LIKELY(obj->isCollection() && isMapCollection(obj->collectionType()))) {
      assertx(collections::isType(obj->getVMClass(), CollectionType::Map,
                                                     CollectionType::ImmMap));
      return FromMap(static_cast<BaseMap*>(obj));
    }
  }
  failMap();
}

Object c_AwaitAllWaitHandle::ti_fromvector(const Variant& dependencies) {
  if (LIKELY(dependencies.isObject())) {
    auto obj = dependencies.getObjectData();
    if (LIKELY(obj->isCollection() &&
               isVectorCollection(obj->collectionType()))) {
      assertx(collections::isType(obj->getVMClass(), CollectionType::Vector,
                                                  CollectionType::ImmVector));
      return FromVector(static_cast<BaseVector*>(obj));
    }
  }
  failVector();
}

Object c_AwaitAllWaitHandle::FromPackedArray(const ArrayData* dependencies) {
  auto const start = reinterpret_cast<const TypedValue*>(dependencies + 1);
  auto const stop = start + dependencies->getSize();
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  int32_t cnt = 0;

  for (auto iter = start; iter < stop; ++iter) {
    prepareChild(tvToCell(iter), ctx_idx, cnt);
  }

  if (!cnt) return returnEmpty();

  auto result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter < stop; ++iter) {
    addChild(tvToCell(iter), next);
  }

  assert(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return Object{std::move(result)};
}

Object c_AwaitAllWaitHandle::FromMixedArray(const MixedArray* dependencies) {
  auto const start = dependencies->data();
  auto const stop = start + dependencies->iterLimit();
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  int32_t cnt = 0;

  for (auto iter = start; iter < stop; ++iter) {
    if (MixedArray::isTombstone(iter->data.m_type)) continue;
    prepareChild(tvToCell(&iter->data), ctx_idx, cnt);
  }

  if (!cnt) return returnEmpty();

  auto result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter < stop; ++iter) {
    if (MixedArray::isTombstone(iter->data.m_type)) continue;
    addChild(tvToCell(&iter->data), next);
  }

  assert(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return Object{std::move(result)};
}

Object c_AwaitAllWaitHandle::FromMap(const BaseMap* dependencies) {
  auto const start = dependencies->firstElm();
  auto const stop = dependencies->elmLimit();
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  int32_t cnt = 0;

  for (auto iter = start; iter != stop; iter = BaseMap::nextElm(iter, stop)) {
    prepareChild(tvAssertCell(&iter->data), ctx_idx, cnt);
  }

  if (!cnt) return returnEmpty();

  auto result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter != stop; iter = BaseMap::nextElm(iter, stop)) {
    addChild(tvAssertCell(&iter->data), next);
  }

  assert(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return Object{std::move(result)};
}

Object c_AwaitAllWaitHandle::FromVector(const BaseVector* dependencies) {
  auto const start = dependencies->data();
  auto const stop = start + dependencies->size();
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  int32_t cnt = 0;

  for (auto iter = start; iter < stop; ++iter) {
    prepareChild(tvAssertCell(iter), ctx_idx, cnt);
  }

  if (!cnt) return returnEmpty();

  auto result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter < stop; ++iter) {
    addChild(tvAssertCell(iter), next);
  }

  assert(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return Object{std::move(result)};
}

void c_AwaitAllWaitHandle::initialize(context_idx_t ctx_idx) {
  setState(STATE_BLOCKED);
  setContextIdx(ctx_idx);
  assert(m_cur >= 0);

  if (UNLIKELY(AsioSession::Get()->hasOnAwaitAllCreate())) {
    auto vector = req::make<c_Vector>();
    for (int32_t idx = m_cur; idx >= 0; --idx) {
      TypedValue child = make_tv<KindOfObject>(m_children[idx]);
      vector->add(&child);
    }
    AsioSession::Get()->onAwaitAllCreate(this, Variant(std::move(vector)));
  }

  blockOnCurrent<false>();
  incRefCount();
}

void c_AwaitAllWaitHandle::onUnblocked() {
  assert(m_cur >= 0);
  assert(m_children[m_cur]->isFinished());
  auto child = &m_children[m_cur];

  do {
    decRefObj(*child);

    if (m_cur == 0) {
      auto parentChain = getParentChain();
      setState(STATE_SUCCEEDED);
      tvWriteNull(&m_resultOrException);
      parentChain.unblock();
      decRefObj(this);
      return;
    }

    --m_cur;
    --child;
  } while ((*child)->isFinished());

  assert(child == &m_children[m_cur]);
  blockOnCurrent<true>();
}

template<bool checkCycle>
void c_AwaitAllWaitHandle::blockOnCurrent() {
  auto child = m_children[m_cur];
  assert(!child->isFinished());

  if (checkCycle) {
    try {
      detectCycle(child);
    } catch (const Object& cycle_exception) {
      markAsFailed(cycle_exception);
      return;
    }
  }

  child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::AwaitAllWaitHandle);
}

void c_AwaitAllWaitHandle::markAsFailed(const Object& exception) {
  auto child = &m_children[m_cur];
  while (m_cur-- >= 0) {
    decRefObj(*(child--));
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
  assert(getState() == STATE_BLOCKED);
  assert(m_cur >= 0);
  return m_children[m_cur];
}

///////////////////////////////////////////////////////////////////////////////
}
