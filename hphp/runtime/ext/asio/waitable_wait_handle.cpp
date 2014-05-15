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

#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_array_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_map_wait_handle.h"
#include "hphp/runtime/ext/asio/gen_vector_wait_handle.h"
#include "hphp/runtime/ext/asio/reschedule_wait_handle.h"
#include "hphp/runtime/ext/asio/sleep_wait_handle.h"
#include "hphp/runtime/ext/asio/external_thread_event_wait_handle.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_WaitableWaitHandle::c_WaitableWaitHandle(Class* cb)
    : c_WaitHandle(cb)
    , m_firstParent(nullptr) {
  setContextIdx(AsioSession::Get()->getCurrentContextIdx());
}

c_WaitableWaitHandle::~c_WaitableWaitHandle() {
  switch (getState()) {
    case STATE_SUCCEEDED:
      tvRefcountedDecRefCell(&m_resultOrException);
      break;

    case STATE_FAILED:
      tvDecRefObj(&m_resultOrException);
      break;
  }
}

int c_WaitableWaitHandle::t_getcontextidx() {
  return getContextIdx();
}

Object c_WaitableWaitHandle::t_getcreator() {
  return Object();
}

Array c_WaitableWaitHandle::t_getparents() {
  // no parent data available if finished
  if (isFinished()) {
    return empty_array;
  }

  Array result = Array::Create();
  c_BlockableWaitHandle* curr = m_firstParent;

  while (curr) {
    result.append(curr);
    curr = curr->getNextParent();
  }

  return result;
}

void c_WaitableWaitHandle::done() {
  assert(isFinished());
  assert(cellIsPlausible(m_resultOrException));

  // unblock parents
  while (m_firstParent) {
    m_firstParent = m_firstParent->unblock();
  }
}

// throws on context depth level overflows and cross-context cycles
void c_WaitableWaitHandle::join() {
  JIT::EagerVMRegAnchor _;

  AsioSession* session = AsioSession::Get();

  assert(!isFinished());
  assert(!session->isInContext() || session->getCurrentContext()->isRunning());

  if (UNLIKELY(session->hasOnJoinCallback())) {
    session->onJoin(this);
  }

  // enter new asio context and set up guard that will exit once we are done
  session->enterContext();
  auto exit_guard = folly::makeGuard([&] { session->exitContext(); });

  assert(session->isInContext());
  assert(!session->getCurrentContext()->isRunning());

  // import this wait handle to the newly created context
  // throws if cross-context cycle found
  enterContext(session->getCurrentContextIdx());

  // run queues until we are finished
  session->getCurrentContext()->runUntil(this);
  assert(isFinished());
}

String c_WaitableWaitHandle::getName() {
  switch (getKind()) {
    case Kind::Static:
      not_reached();
    case Kind::AsyncFunction:
      return static_cast<c_AsyncFunctionWaitHandle*>(this)->getName();
    case Kind::GenArray:
      return static_cast<c_GenArrayWaitHandle*>(this)->getName();
    case Kind::GenMap:
      return static_cast<c_GenMapWaitHandle*>(this)->getName();
    case Kind::GenVector:
      return static_cast<c_GenVectorWaitHandle*>(this)->getName();
    case Kind::Reschedule:
      return static_cast<c_RescheduleWaitHandle*>(this)->getName();
    case Kind::Sleep:
      return static_cast<c_SleepWaitHandle*>(this)->getName();
    case Kind::ExternalThreadEvent:
      return static_cast<c_ExternalThreadEventWaitHandle*>(this)->getName();
  }
  not_reached();
}

c_WaitableWaitHandle* c_WaitableWaitHandle::getChild() {
  assert(!isFinished());

  switch (getKind()) {
    case Kind::Static:
      not_reached();
    case Kind::AsyncFunction:
      return static_cast<c_AsyncFunctionWaitHandle*>(this)->getChild();
    case Kind::GenArray:
      return static_cast<c_GenArrayWaitHandle*>(this)->getChild();
    case Kind::GenMap:
      return static_cast<c_GenMapWaitHandle*>(this)->getChild();
    case Kind::GenVector:
      return static_cast<c_GenVectorWaitHandle*>(this)->getChild();
    case Kind::Reschedule:
    case Kind::Sleep:
    case Kind::ExternalThreadEvent:
      return nullptr;
  }
  not_reached();
}

void c_WaitableWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  switch (getKind()) {
    case Kind::Static:
      not_reached();
    case Kind::AsyncFunction:
      static_cast<c_AsyncFunctionWaitHandle*>(this)->enterContextImpl(ctx_idx);
      return;
    case Kind::GenArray:
      static_cast<c_GenArrayWaitHandle*>(this)->enterContextImpl(ctx_idx);
      return;
    case Kind::GenMap:
      static_cast<c_GenMapWaitHandle*>(this)->enterContextImpl(ctx_idx);
      return;
    case Kind::GenVector:
      static_cast<c_GenVectorWaitHandle*>(this)->enterContextImpl(ctx_idx);
      return;
    case Kind::Reschedule:
      static_cast<c_RescheduleWaitHandle*>(this)->enterContextImpl(ctx_idx);
      return;
    case Kind::Sleep:
      static_cast<c_SleepWaitHandle*>(this)->enterContextImpl(ctx_idx);
      return;
    case Kind::ExternalThreadEvent:
      static_cast<c_ExternalThreadEventWaitHandle*>(this)->enterContextImpl(
        ctx_idx);
      return;
  }
  not_reached();
}

bool
c_WaitableWaitHandle::isDescendantOf(c_WaitableWaitHandle* wait_handle) const {
  assert(wait_handle);

  while (wait_handle != this && wait_handle && !wait_handle->isFinished()) {
    wait_handle = wait_handle->getChild();
  }

  return wait_handle == this;
}

Array c_WaitableWaitHandle::t_getdependencystack() {
  g_context->nullOutReturningActRecs();
  Array result = Array::Create();
  if (isFinished()) return result;
  hphp_hash_set<int64_t> visited;
  auto wait_handle = this;
  while (wait_handle != nullptr) {
    result.append(wait_handle);
    visited.insert(wait_handle->t_getid());
    auto context_idx = wait_handle->getContextIdx();

    // 1. find parent in the same context
    auto p = wait_handle->getFirstParent();
    while (p) {
      if ((p->getContextIdx() == context_idx) &&
          visited.find(p->t_getid()) == visited.end()) {
        wait_handle = p;
        break;
      }
      p = p->getNextParent();
    }
    if (p) continue;

    // 2. cross the context boundary
    result.append(null_object);
    wait_handle = (context_idx > 1)
      ? AsioSession::Get()->getContext(context_idx - 1)->getCurrent()
      : nullptr;
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
