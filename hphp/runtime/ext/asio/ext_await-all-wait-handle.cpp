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

#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
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

///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_awaitAll("<await-all>");

  [[noreturn]] NEVER_INLINE
  void failArray() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be an array");
  }

  [[noreturn]] NEVER_INLINE
  void failMap() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a Map");
  }

  [[noreturn]] NEVER_INLINE
  void failVector() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a Vector");
  }

  [[noreturn]] NEVER_INLINE
  void failWaitHandle() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be a collection of WaitHandle instances");
  }

  c_StaticWaitHandle* returnEmpty() {
    return c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfNull>());
  }

  void prepareChild(Cell src, context_idx_t& ctx_idx, uint32_t& cnt) {
    auto const waitHandle = c_WaitHandle::fromCell(src);
    if (UNLIKELY(!waitHandle)) failWaitHandle();
    if (waitHandle->isFinished()) return;
    assert(isa<c_WaitableWaitHandle>(waitHandle));
    auto const child = static_cast<c_WaitableWaitHandle*>(waitHandle);
    ctx_idx = std::min(ctx_idx, child->getContextIdx());
    ++cnt;
  }

  bool addChild(Cell src, c_AwaitAllWaitHandle::Node*& dst, uint32_t& idx) {
    auto const waitHandle = c_WaitHandle::fromCellAssert(src);
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

template<bool convert, typename Iter>
Object c_AwaitAllWaitHandle::Create(Iter iter) {
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  uint32_t cnt = 0;

  auto toCell = convert
    ? [](TypedValue tv) { return tvToCell(tv); }
    : [](TypedValue tv) { return tvAssertCell(tv); };

  iter([&](TypedValue v) { prepareChild(toCell(v), ctx_idx, cnt); });

  if (!cnt) {
    return Object{returnEmpty()};
  }

  auto result = Alloc(cnt);
  auto next = &result->m_children[cnt];
  uint32_t idx = cnt - 1;

  iter([&](TypedValue v) { addChild(toCell(v), next, idx); });

  assert(next == &result->m_children[0]);
  result->initialize(ctx_idx);
  return Object{std::move(result)};
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromArray,
                          const Array& dependencies) {
  auto ad = dependencies.get();
  assertx(ad);
  assertx(ad->isPHPArray());
  if (!ad->size()) return Object{returnEmpty()};

retry:
  switch (ad->kind()) {
    case ArrayData::kPackedKind:
      return c_AwaitAllWaitHandle::Create<true>([=](auto fn) {
        PackedArray::IterateV(ad, fn);
      });

    case ArrayData::kMixedKind:
      return c_AwaitAllWaitHandle::Create<true>([=](auto fn) {
        MixedArray::IterateV(MixedArray::asMixed(ad), fn);
      });

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

    case ArrayData::kVecKind:
    case ArrayData::kDictKind:
    case ArrayData::kKeysetKind:
      // Shouldn't get Hack arrays
      not_reached();

    case ArrayData::kNumKinds:
      not_reached();
  }

  not_reached();
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromVec,
                          const Array& dependencies) {
  auto ad = dependencies.get();
  assertx(ad);
  assertx(ad->isVecArray());
  if (!ad->size()) return Object{returnEmpty()};
  return c_AwaitAllWaitHandle::Create<false>([=](auto fn) {
    PackedArray::IterateV(ad, fn);
  });
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromDict,
                          const Array& dependencies) {
  auto ad = dependencies.get();
  assertx(ad);
  assertx(ad->isDict());
  if (!ad->size()) return Object{returnEmpty()};
  return c_AwaitAllWaitHandle::Create<false>([=](auto fn) {
    MixedArray::IterateV(MixedArray::asMixed(ad), fn);
  });
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromMap,
                          const Variant& dependencies) {
  if (LIKELY(dependencies.isObject())) {
    auto obj = dependencies.getObjectData();
    if (LIKELY(obj->isCollection() && isMapCollection(obj->collectionType()))) {
      assertx(collections::isType(obj->getVMClass(), CollectionType::Map,
                                                     CollectionType::ImmMap));
      return c_AwaitAllWaitHandle::Create<false>([=](auto fn) {
        MixedArray::IterateV(static_cast<BaseMap*>(obj)->arrayData(), fn);
      });
    }
  }
  failMap();
}

Object HHVM_STATIC_METHOD(AwaitAllWaitHandle, fromVector,
                          const Variant& dependencies) {
  if (LIKELY(dependencies.isObject())) {
    auto obj = dependencies.getObjectData();
    if (LIKELY(obj->isCollection() &&
               isVectorCollection(obj->collectionType()))) {
      assertx(collections::isType(obj->getVMClass(), CollectionType::Vector,
                                                  CollectionType::ImmVector));
      return c_AwaitAllWaitHandle::Create<false>([=](auto fn) {
        PackedArray::IterateV(static_cast<BaseVector*>(obj)->arrayData(), fn);
      });
    }
  }
  failVector();
}

void c_AwaitAllWaitHandle::initialize(context_idx_t ctx_idx) {
  setState(STATE_BLOCKED);
  setContextIdx(ctx_idx);

  if (UNLIKELY(AsioSession::Get()->hasOnAwaitAllCreate())) {
    auto vector = req::make<c_Vector>();
    for (int32_t idx = m_cap - 1; idx >= 0; --idx) {
      auto const child = make_tv<KindOfObject>(m_children[idx].m_child);
      vector->add(child);
    }
    AsioSession::Get()->onAwaitAllCreate(this, Variant(std::move(vector)));
  }

  incRefCount();
}

void c_AwaitAllWaitHandle::onUnblocked(uint32_t idx) {
  assert(idx <= m_unfinished);
  assert(getState() == STATE_BLOCKED);

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
  tvWriteNull(&m_resultOrException);
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
  assert(getState() == STATE_BLOCKED);
  assert(m_unfinished < m_cap);
  return m_children[m_unfinished].m_child;
}

///////////////////////////////////////////////////////////////////////////////

void AsioExtension::initAwaitAllWaitHandle() {
#define AAWH_SME(meth) \
  HHVM_STATIC_MALIAS(HH\\AwaitAllWaitHandle, meth, AwaitAllWaitHandle, meth)
  AAWH_SME(fromArray);
  AAWH_SME(fromVec);
  AAWH_SME(fromDict);
  AAWH_SME(fromMap);
  AAWH_SME(fromVector);
  AAWH_SME(setOnCreateCallback);
#undef AAWH_SME
}

///////////////////////////////////////////////////////////////////////////////
}
