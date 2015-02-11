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

#include "hphp/runtime/ext/asio/condition_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_condition("<condition>");

  ObjectData* getNotNotifiedException() {
    return SystemLib::AllocInvalidArgumentExceptionObject(
      "ConditionWaitHandle not notified by its child");
  }

  NEVER_INLINE __attribute__((__noreturn__))
  void failAlreadyFinished() {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to notify ConditionWaitHandle that has already finished"));
    throw e;
  }
}

void c_ConditionWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnConditionCreateCallback(callback);
}

Object c_ConditionWaitHandle::ti_create(const Variant& child) {
  // Child not a WaitHandle?
  auto const child_wh = c_WaitHandle::fromCell(child.asCell());
  if (UNLIKELY(!child_wh)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected child to be an instance of WaitHandle"));
    throw e;
  }

  // Child finished before notification?
  if (UNLIKELY(child_wh->isFinished())) {
    Object e(getNotNotifiedException());
    throw e;
  }

  assert(child_wh->instanceof(c_WaitableWaitHandle::classof()));
  auto const child_wwh = static_cast<c_WaitableWaitHandle*>(child_wh);

  // Import child into the current context, detect cross-context cycles.
  auto const session = AsioSession::Get();
  if (session->isInContext()) {
    child_wwh->enterContext(session->getCurrentContextIdx());
  }

  c_ConditionWaitHandle* wh = newobj<c_ConditionWaitHandle>();
  wh->initialize(child_wwh);
  return wh;
}

void c_ConditionWaitHandle::t_succeed(const Variant& result) {
  if (isFinished()) {
    failAlreadyFinished();
  }

  assert(getState() == STATE_BLOCKED);
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  cellDup(*result.asCell(), m_resultOrException);
  parentChain.unblock();
}

void c_ConditionWaitHandle::t_fail(const Variant& exception) {
  auto const cell = exception.asCell();
  if (UNLIKELY(
    cell->m_type != KindOfObject ||
    !cell->m_data.pobj->instanceof(SystemLib::s_ExceptionClass)
  )) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected exception to be an instance of Exception"));
    throw e;
  }

  if (isFinished()) {
    failAlreadyFinished();
  }

  assert(getState() == STATE_BLOCKED);
  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  cellDup(make_tv<KindOfObject>(cell->m_data.pobj), m_resultOrException);
  parentChain.unblock();
}

void c_ConditionWaitHandle::initialize(c_WaitableWaitHandle* child) {
  assert(!child->isFinished());

  setState(STATE_BLOCKED);
  m_child = child;
  m_child->incRefCount();

  auto& parentChain = child->getParentChain();
  parentChain.addParent(m_blockable, AsioBlockable::Kind::ConditionWaitHandle);
  incRefCount();

  auto const session = AsioSession::Get();
  if (UNLIKELY(session->hasOnConditionCreateCallback())) {
    session->onConditionCreate(this, child);
  }
}

void c_ConditionWaitHandle::onUnblocked() {
  decRefObj(m_child);
  m_child = nullptr;

  // Unblocked after notification.
  if (LIKELY(isFinished())) {
    decRefObj(this);
    return;
  }

  auto parentChain = getParentChain();
  setState(STATE_FAILED);
  tvWriteObject(getNotNotifiedException(), &m_resultOrException);
  parentChain.unblock();
  decRefObj(this);
}

String c_ConditionWaitHandle::getName() {
  return s_condition;
}

c_WaitableWaitHandle* c_ConditionWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return m_child;
}

void c_ConditionWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);

  m_child->enterContext(ctx_idx);
  setContextIdx(ctx_idx);
}

void c_ConditionWaitHandle::exitContextBlocked(context_idx_t ctx_idx) {
  if (isFinished()) {
    return;
  }

  assert(getState() == STATE_BLOCKED);
  assert(AsioSession::Get()->getContext(ctx_idx));

  // Not in a context being exited.
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    return;
  }

  // Move us to the parent context.
  setContextIdx(getContextIdx() - 1);

  // Recursively move all wait handles blocked by us.
  getParentChain().exitContext(ctx_idx);
}

///////////////////////////////////////////////////////////////////////////////
}
