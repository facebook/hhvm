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

#include "hphp/runtime/ext/asio/waitable-wait-handle.h"

#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/condition-wait-handle.h"
#include "hphp/runtime/ext/asio/gen-array-wait-handle.h"
#include "hphp/runtime/ext/asio/gen-map-wait-handle.h"
#include "hphp/runtime/ext/asio/gen-vector-wait-handle.h"
#include "hphp/runtime/ext/asio/reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/external-thread-event-wait-handle.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int c_WaitableWaitHandle::t_getcontextidx() {
  if (isFinished()) {
    return 0;
  }

  return getContextIdx();
}

Object c_WaitableWaitHandle::t_getcreator() {
  return Object();
}

Array c_WaitableWaitHandle::t_getparents() {
  // no parent data available if finished
  if (isFinished()) {
    return empty_array();
  }

  return getParentChain().toArray();
}

// throws on context depth level overflows and cross-context cycles
void c_WaitableWaitHandle::join() {
  EagerVMRegAnchor _;
  auto const savedFP = vmfp();

  assert(!isFinished());

  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnJoinCallback())) {
    session->onJoin(this);
  }

  // enter new asio context and set up guard that will exit once we are done
  session->enterContext(savedFP);
  auto exit_guard = folly::makeGuard([&] { session->exitContext(); });

  // import this wait handle to the newly created context
  // throws if cross-context cycle found
  enterContext(session->getCurrentContextIdx());

  // run queues until we are finished
  session->getCurrentContext()->runUntil(this);
  assert(isFinished());
}

String c_WaitableWaitHandle::getName() {
  switch (getKind()) {
    case Kind::Static:              not_reached();
    case Kind::AsyncFunction:       return asAsyncFunction()->getName();
    case Kind::AsyncGenerator:      return asAsyncGenerator()->getName();
    case Kind::AwaitAll:            return asAwaitAll()->getName();
    case Kind::GenArray:            return asGenArray()->getName();
    case Kind::GenMap:              return asGenMap()->getName();
    case Kind::GenVector:           return asGenVector()->getName();
    case Kind::Condition:           return asCondition()->getName();
    case Kind::Reschedule:          return asReschedule()->getName();
    case Kind::Sleep:               return asSleep()->getName();
    case Kind::ExternalThreadEvent: return asExternalThreadEvent()->getName();
  }
  not_reached();
}

c_WaitableWaitHandle* c_WaitableWaitHandle::getChild() {
  assert(!isFinished());

  switch (getKind()) {
    case Kind::Static:              not_reached();
    case Kind::AsyncFunction:       return asAsyncFunction()->getChild();
    case Kind::AsyncGenerator:      return asAsyncGenerator()->getChild();
    case Kind::AwaitAll:            return asAwaitAll()->getChild();
    case Kind::GenArray:            return asGenArray()->getChild();
    case Kind::GenMap:              return asGenMap()->getChild();
    case Kind::GenVector:           return asGenVector()->getChild();
    case Kind::Condition:           return asCondition()->getChild();
    case Kind::Reschedule:          return nullptr;
    case Kind::Sleep:               return nullptr;
    case Kind::ExternalThreadEvent: return nullptr;
  }
  not_reached();
}

void c_WaitableWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  switch (getKind()) {
    case Kind::Static:
      not_reached();
    case Kind::AsyncFunction:
      return asAsyncFunction()->enterContextImpl(ctx_idx);
    case Kind::AsyncGenerator:
      return asAsyncGenerator()->enterContextImpl(ctx_idx);
    case Kind::AwaitAll:
      return asAwaitAll()->enterContextImpl(ctx_idx);
    case Kind::GenArray:
      return asGenArray()->enterContextImpl(ctx_idx);
    case Kind::GenMap:
      return asGenMap()->enterContextImpl(ctx_idx);
    case Kind::GenVector:
      return asGenVector()->enterContextImpl(ctx_idx);
    case Kind::Condition:
      return asCondition()->enterContextImpl(ctx_idx);
    case Kind::Reschedule:
      return asReschedule()->enterContextImpl(ctx_idx);
    case Kind::Sleep:
      return asSleep()->enterContextImpl(ctx_idx);
    case Kind::ExternalThreadEvent:
      return asExternalThreadEvent()->enterContextImpl(ctx_idx);
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

ObjectData*
c_WaitableWaitHandle::createCycleException(c_WaitableWaitHandle* child) const {
  assert(isDescendantOf(child));

  smart::vector<std::string> exception_msg_items;
  exception_msg_items.push_back("Encountered dependency cycle.\n");
  exception_msg_items.push_back("Existing stack:\n");

  exception_msg_items.push_back(folly::stringPrintf(
    "  %s (%" PRId64 ")\n", child->getName().data(), child->t_getid()));

  auto current = child;

  while (current != this) {
    current = current->getChild();
    assert(current);

    exception_msg_items.push_back(folly::stringPrintf(
      "  %s (%" PRId64 ")\n", current->getName().data(), current->t_getid()));
  }

  exception_msg_items.push_back("Trying to introduce dependency on:\n");
  exception_msg_items.push_back(folly::stringPrintf(
    "  %s (%" PRId64 ") (dupe)\n", child->getName().data(), child->t_getid()));
  return SystemLib::AllocInvalidOperationExceptionObject(
      folly::join("", exception_msg_items));
}

Array c_WaitableWaitHandle::t_getdependencystack() {
  if (isFinished()) return empty_array();
  Array result = Array::Create();
  hphp_hash_set<int64_t> visited;
  auto wait_handle = this;
  auto session = AsioSession::Get();
  while (wait_handle != nullptr) {
    result.append(wait_handle);
    visited.insert(wait_handle->t_getid());
    auto context_idx = wait_handle->getContextIdx();

    // 1. find parent in the same context
    auto p = wait_handle->getParentChain().firstInContext(context_idx);
    if (p && visited.find(p->t_getid()) == visited.end()) {
      wait_handle = p;
      continue;
    }

    // 2. cross the context boundary
    auto context = session->getContext(context_idx);
    if (!context) {
      break;
    }
    wait_handle = c_ResumableWaitHandle::getRunning(context->getSavedFP());
    auto target_context_idx = wait_handle ? wait_handle->getContextIdx() : 0;
    while (context_idx > target_context_idx) {
      --context_idx;
      result.append(null_object);
    }
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
