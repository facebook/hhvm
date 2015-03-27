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
#include "hphp/runtime/ext/asio/asio_blockable.h"

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/async_generator_wait_handle.h"
#include "hphp/runtime/ext/asio/await_all_wait_handle.h"
#include "hphp/runtime/ext/asio/condition_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_array_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_map_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_vector_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  typedef AsioBlockable::Kind Kind;

  template<class T>
  inline T* getWH(const AsioBlockable* blockable) {
    return reinterpret_cast<T*>(
      const_cast<char*>(
        reinterpret_cast<const char*>(blockable) -
        T::blockableOff()));
  }

  inline c_AsyncFunctionWaitHandle* getAsyncFunctionWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::AsyncFunctionWaitHandle);
    return getWH<c_AsyncFunctionWaitHandle>(blockable);
  }

  inline c_AsyncGeneratorWaitHandle* getAsyncGeneratorWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::AsyncGeneratorWaitHandle);
    return getWH<c_AsyncGeneratorWaitHandle>(blockable);
  }

  inline c_AwaitAllWaitHandle* getAwaitAllWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::AwaitAllWaitHandle);
    return getWH<c_AwaitAllWaitHandle>(blockable);
  }

  inline c_ConditionWaitHandle* getConditionWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::ConditionWaitHandle);
    return getWH<c_ConditionWaitHandle>(blockable);
  }

  inline c_GenArrayWaitHandle* getGenArrayWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::GenArrayWaitHandle);
    return getWH<c_GenArrayWaitHandle>(blockable);
  }

  inline c_GenMapWaitHandle* getGenMapWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::GenMapWaitHandle);
    return getWH<c_GenMapWaitHandle>(blockable);
  }

  inline c_GenVectorWaitHandle* getGenVectorWaitHandle(
    const AsioBlockable* blockable
  ) {
    assert(blockable->getKind() == Kind::GenVectorWaitHandle);
    return getWH<c_GenVectorWaitHandle>(blockable);
  }

  inline void exitContextImpl(
    c_WaitableWaitHandle* waitHandle,
    context_idx_t ctx_idx
  ) {
    assert(AsioSession::Get()->getContext(ctx_idx));
    assert(!waitHandle->isFinished());
    assert(waitHandle->getContextIdx() <= ctx_idx);

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
    c_ConditionWaitHandle* waitHandle,
    context_idx_t ctx_idx
  ) {
    // ConditionWaitHandle may finish before its children do.
    if (waitHandle->isFinished()) {
      return;
    }

    exitContextImpl(static_cast<c_WaitableWaitHandle*>(waitHandle), ctx_idx);
  }
}

c_WaitableWaitHandle* AsioBlockable::getWaitHandle() const {
  switch (getKind()) {
    case Kind::AsyncFunctionWaitHandle:
      return getAsyncFunctionWaitHandle(this);
    case Kind::AsyncGeneratorWaitHandle:
      return getAsyncGeneratorWaitHandle(this);
    case Kind::AwaitAllWaitHandle:
      return getAwaitAllWaitHandle(this);
    case Kind::ConditionWaitHandle:
      return getConditionWaitHandle(this);
    case Kind::GenArrayWaitHandle:
      return getGenArrayWaitHandle(this);
    case Kind::GenMapWaitHandle:
      return getGenMapWaitHandle(this);
    case Kind::GenVectorWaitHandle:
      return getGenVectorWaitHandle(this);
  }
  not_reached();
}

void AsioBlockableChain::unblock() {
  auto cur = m_firstParent;
  while (cur) {
    auto const next = cur->getNextParent();

    // May free cur.
    switch (cur->getKind()) {
      case Kind::AsyncFunctionWaitHandle:
        getAsyncFunctionWaitHandle(cur)->onUnblocked();
        break;
      case Kind::AsyncGeneratorWaitHandle:
        getAsyncGeneratorWaitHandle(cur)->onUnblocked();
        break;
      case Kind::AwaitAllWaitHandle:
        getAwaitAllWaitHandle(cur)->onUnblocked();
        break;
      case Kind::ConditionWaitHandle:
        getConditionWaitHandle(cur)->onUnblocked();
        break;
      case Kind::GenArrayWaitHandle:
        getGenArrayWaitHandle(cur)->onUnblocked();
        break;
      case Kind::GenMapWaitHandle:
        getGenMapWaitHandle(cur)->onUnblocked();
        break;
      case Kind::GenVectorWaitHandle:
        getGenVectorWaitHandle(cur)->onUnblocked();
        break;
    }

    cur = next;
  }
}

void AsioBlockableChain::exitContext(context_idx_t ctx_idx) {
  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    switch (cur->getKind()) {
      case Kind::AsyncFunctionWaitHandle:
        exitContextImpl(getAsyncFunctionWaitHandle(cur), ctx_idx);
        break;
      case Kind::AsyncGeneratorWaitHandle:
        exitContextImpl(getAsyncGeneratorWaitHandle(cur), ctx_idx);
        break;
      case Kind::AwaitAllWaitHandle:
        exitContextImpl(getAwaitAllWaitHandle(cur), ctx_idx);
        break;
      case Kind::ConditionWaitHandle:
        exitContextImpl(getConditionWaitHandle(cur), ctx_idx);
        break;
      case Kind::GenArrayWaitHandle:
        exitContextImpl(getGenArrayWaitHandle(cur), ctx_idx);
        break;
      case Kind::GenMapWaitHandle:
        exitContextImpl(getGenMapWaitHandle(cur), ctx_idx);
        break;
      case Kind::GenVectorWaitHandle:
        exitContextImpl(getGenVectorWaitHandle(cur), ctx_idx);
        break;
    }
  }
}

Array AsioBlockableChain::toArray() {
  Array result = Array::Create();
  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    auto const wh = cur->getWaitHandle();
    if (!wh->isFinished()) {
      result.append(wh);
    }
  }
  return result;
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

///////////////////////////////////////////////////////////////////////////////
}
