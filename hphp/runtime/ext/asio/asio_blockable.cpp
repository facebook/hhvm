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
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_array_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_map_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_vector_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  inline void dispatchUnblock(c_BlockableWaitHandle* wh) {
    typedef c_WaitHandle::Kind Kind;
    switch (wh->getKind()) {
      case Kind::AsyncFunction:  wh->asAsyncFunction()->onUnblocked(); break;
      case Kind::AsyncGenerator: wh->asAsyncGenerator()->onUnblocked(); break;
      case Kind::AwaitAll:       wh->asAwaitAll()->onUnblocked(); break;
      case Kind::GenArray:       wh->asGenArray()->onUnblocked(); break;
      case Kind::GenMap:         wh->asGenMap()->onUnblocked(); break;
      case Kind::GenVector:      wh->asGenVector()->onUnblocked(); break;
      case Kind::Static:
      case Kind::Reschedule:
      case Kind::Sleep:
      case Kind::ExternalThreadEvent:
        not_reached();
    }
  }
}

c_BlockableWaitHandle* AsioBlockable::getBlockableWaitHandle() const {
  assert(getKind() == Kind::BlockableWaitHandle);
  return reinterpret_cast<c_BlockableWaitHandle*>(
    const_cast<char*>(
      reinterpret_cast<const char*>(this) -
      c_BlockableWaitHandle::blockableOff()));
}

void AsioBlockableChain::unblock() {
  auto cur = m_firstParent;
  while (cur) {
    auto const next = cur->getNextParent();

    // May free cur.
    switch (cur->getKind()) {
      case AsioBlockable::Kind::BlockableWaitHandle:
        dispatchUnblock(cur->getBlockableWaitHandle());
        break;
    }

    cur = next;
  }
}

void AsioBlockableChain::exitContext(context_idx_t ctx_idx) {
  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    switch (cur->getKind()) {
      case AsioBlockable::Kind::BlockableWaitHandle:
        cur->getBlockableWaitHandle()->exitContextBlocked(ctx_idx);
        break;
    }
  }
}

Array AsioBlockableChain::toArray() {
  Array result = Array::Create();

  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    switch (cur->getKind()) {
      case AsioBlockable::Kind::BlockableWaitHandle:
        result.append(cur->getBlockableWaitHandle());
        break;
    }
  }

  return result;
}

c_BlockableWaitHandle*
AsioBlockableChain::firstInContext(context_idx_t ctx_idx) {
  for (auto cur = m_firstParent; cur; cur = cur->getNextParent()) {
    switch (cur->getKind()) {
      case AsioBlockable::Kind::BlockableWaitHandle:
        auto const wh = cur->getBlockableWaitHandle();
        if (wh->getContextIdx() == ctx_idx) return wh;
        break;
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
