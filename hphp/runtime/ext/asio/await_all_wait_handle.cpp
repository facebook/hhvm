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

#include "hphp/runtime/ext/asio/await_all_wait_handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/static_wait_handle.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/proxy-array.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void delete_AwaitAllWaitHandle(ObjectData* od, const Class*) {
  auto const waitHandle = static_cast<c_AwaitAllWaitHandle*>(od);
  auto const size = waitHandle->m_size;
  waitHandle->~c_AwaitAllWaitHandle();
  MM().objFreeLogged(waitHandle, size);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_awaitAll("<await-all>");

  NEVER_INLINE __attribute__((__noreturn__))
  void failArray() {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be an array of WaitHandle instances"));
    throw e;
  }

  NEVER_INLINE __attribute__((__noreturn__))
  void failMap() {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be a Map of WaitHandle instances"));
    throw e;
  }

  NEVER_INLINE __attribute__((__noreturn__))
  void failVector() {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be a Vector of WaitHandle instances"));
    throw e;
  }

  c_StaticWaitHandle* returnEmpty() {
    return c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfNull>());
  }
}

void c_AwaitAllWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnAwaitAllCreateCallback(callback);
}

Object c_AwaitAllWaitHandle::ti_fromarray(const Array& dependencies) {
  auto ad = dependencies.get();
  assert(ad);
  if (!ad->size()) return returnEmpty();

retry:
  switch (ad->kind()) {
    case ArrayData::kPackedKind:
    case ArrayData::kVPackedKind:
      return FromPackedArray(ad);

    case ArrayData::kMixedKind:
    case ArrayData::kIntMapKind:
    case ArrayData::kStrMapKind:
      return FromMixedArray(MixedArray::asMixed(ad));

    case ArrayData::kProxyKind:
      ad = ProxyArray::innerArr(ad);
      goto retry;

    case ArrayData::kSharedKind:
    case ArrayData::kNvtwKind:
      // APC can't store WaitHandles, NameValueTableWrapper is used only for
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
  if (UNLIKELY(!dependencies.isObject() ||
               !Collection::isMapType(
                 dependencies.getObjectData()->getCollectionType()))) {
    failMap();
  }
  assert(
    dependencies.getObjectData()->instanceof(c_Map::classof()) ||
    dependencies.getObjectData()->instanceof(c_ImmMap::classof())
  );

  return FromMap(static_cast<BaseMap*>(dependencies.getObjectData()));
}

Object c_AwaitAllWaitHandle::ti_fromvector(const Variant& dependencies) {
  if (UNLIKELY(!dependencies.isObject() ||
               !Collection::isVectorType(
                 dependencies.getObjectData()->getCollectionType()))) {
    failVector();
  }
  assert(
    dependencies.getObjectData()->instanceof(c_Vector::classof()) ||
    dependencies.getObjectData()->instanceof(c_ImmVector::classof())
  );

  return FromVector(static_cast<BaseVector*>(dependencies.getObjectData()));
}


Object c_AwaitAllWaitHandle::FromPackedArray(const ArrayData* dependencies) {
  auto const start = reinterpret_cast<const TypedValue*>(dependencies + 1);
  auto const stop = start + dependencies->getSize();
  int32_t cnt = 0;

  for (auto iter = start; iter < stop; ++iter) {
    auto const current = tvToCell(iter);
    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) failArray();
    cnt += !child->isFinished();
  }

  if (!cnt) return returnEmpty();

  p_AwaitAllWaitHandle result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter < stop; ++iter) {
    auto const current = tvToCell(iter);
    auto const child = c_WaitHandle::fromCell(current);
    if (child->isFinished()) continue;
    child->incRefCount();
    *(--next) = static_cast<c_WaitableWaitHandle*>(child);
  }

  assert(next == &result->m_children[0]);
  result->initialize();
  return result;
}

Object c_AwaitAllWaitHandle::FromMixedArray(const MixedArray* dependencies) {
  auto const start = dependencies->data();
  auto const stop = start + dependencies->iterLimit();
  int32_t cnt = 0;

  for (auto iter = start; iter < stop; ++iter) {
    if (MixedArray::isTombstone(iter->data.m_type)) continue;
    auto const current = tvToCell(&iter->data);
    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) failArray();
    cnt += !child->isFinished();
  }

  if (!cnt) return returnEmpty();

  p_AwaitAllWaitHandle result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter < stop; ++iter) {
    if (MixedArray::isTombstone(iter->data.m_type)) continue;
    auto const current = tvToCell(&iter->data);
    auto const child = c_WaitHandle::fromCell(current);
    if (child->isFinished()) continue;
    child->incRefCount();
    *(--next) = static_cast<c_WaitableWaitHandle*>(child);
  }

  assert(next == &result->m_children[0]);
  result->initialize();
  return result;
}

Object c_AwaitAllWaitHandle::FromMap(const BaseMap* dependencies) {
  auto const start = dependencies->firstElm();
  auto const stop = dependencies->elmLimit();
  int32_t cnt = 0;

  for (auto iter = start; iter != stop; iter = BaseMap::nextElm(iter, stop)) {
    auto const current = tvAssertCell(&iter->data);
    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) failMap();
    cnt += !child->isFinished();
  }

  if (!cnt) return returnEmpty();

  p_AwaitAllWaitHandle result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter != stop; iter = BaseMap::nextElm(iter, stop)) {
    auto const current = tvAssertCell(&iter->data);
    auto const child = c_WaitHandle::fromCell(current);
    if (child->isFinished()) continue;
    child->incRefCount();
    *(--next) = static_cast<c_WaitableWaitHandle*>(child);
  }

  assert(next == &result->m_children[0]);
  result->initialize();
  return result;
}

Object c_AwaitAllWaitHandle::FromVector(const BaseVector* dependencies) {
  auto const start = dependencies->data();
  auto const stop = start + dependencies->size();
  int32_t cnt = 0;

  for (auto iter = start; iter < stop; ++iter) {
    auto const current = tvAssertCell(iter);
    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) failVector();
    cnt += !child->isFinished();
  }

  if (!cnt) return returnEmpty();

  p_AwaitAllWaitHandle result = Alloc(cnt);
  auto next = &result->m_children[cnt];

  for (auto iter = start; iter < stop; ++iter) {
    auto const current = tvAssertCell(iter);
    auto const child = c_WaitHandle::fromCell(current);
    if (child->isFinished()) continue;
    child->incRefCount();
    *(--next) = static_cast<c_WaitableWaitHandle*>(child);
  }

  assert(next == &result->m_children[0]);
  result->initialize();
  return result;
}

c_AwaitAllWaitHandle* c_AwaitAllWaitHandle::Alloc(int32_t cnt) {
  size_t size = sizeof(c_AwaitAllWaitHandle) +
                cnt * sizeof(c_WaitableWaitHandle*);
  void* mem = MM().objMallocLogged(size);
  auto const waitHandle = new (mem) c_AwaitAllWaitHandle();
  waitHandle->m_cur = cnt - 1;
  waitHandle->m_size = size;
  return waitHandle;
}

void c_AwaitAllWaitHandle::initialize() {
  setState(STATE_BLOCKED);
  assert(m_cur >= 0);

  if (UNLIKELY(AsioSession::Get()->hasOnAwaitAllCreateCallback())) {
    p_Vector vector = NEWOBJ(c_Vector)();
    for (int32_t idx = m_cur; idx >= 0; --idx) {
      TypedValue child = make_tv<KindOfObject>(m_children[idx]);
      vector->add(&child);
    }
    AsioSession::Get()->onAwaitAllCreate(this, vector);
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

  try {
    if (isInContext()) {
      child->enterContext(getContextIdx());
    }
    if (checkCycle) {
      detectCycle(child);
    }
  } catch (const Object& cycle_exception) {
    markAsFailed(cycle_exception);
    return;
  }

  blockOn(child);
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

void c_AwaitAllWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);
  assert(m_cur >= 0);

  // recursively import current child
  m_children[m_cur]->enterContext(ctx_idx);

  // import ourselves
  setContextIdx(ctx_idx);

  // try to import other children
  auto cur = m_cur;
  auto child = &m_children[cur];

  while (cur > 0) {
    --cur;
    --child;

    if ((*child)->isFinished()) {
      continue;
    }

    try {
      (*child)->enterContext(ctx_idx);
    } catch (const Object& cycle_exception) {
      // exception will be eventually processed by onUnblocked()
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
