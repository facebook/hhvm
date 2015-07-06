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

#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_condition("<condition>");

  Object getNotNotifiedException() {
    return SystemLib::AllocInvalidArgumentExceptionObject(
      "ConditionWaitHandle not notified by its child");
  }

  NEVER_INLINE ATTRIBUTE_NORETURN
  Object throwNotNotifiedException() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "ConditionWaitHandle not notified by its child");
  }

  NEVER_INLINE ATTRIBUTE_NORETURN
  void failAlreadyFinished() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Unable to notify ConditionWaitHandle that has already finished");
  }
}

void c_ConditionWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnConditionCreate(callback);
}

Object c_ConditionWaitHandle::ti_create(const Variant& child) {
  // Child not a WaitHandle?
  auto const child_wh = c_WaitHandle::fromCell(child.asCell());
  if (UNLIKELY(!child_wh)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected child to be an instance of WaitHandle");
  }

  // Child finished before notification?
  if (UNLIKELY(child_wh->isFinished())) {
    throwNotNotifiedException();
  }

  assert(child_wh->instanceof(c_WaitableWaitHandle::classof()));
  auto const child_wwh = static_cast<c_WaitableWaitHandle*>(child_wh);

  auto wh = req::make<c_ConditionWaitHandle>();
  wh->initialize(child_wwh);
  return Object(std::move(wh));
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
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected exception to be an instance of Exception");
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
  setContextIdx(child->getContextIdx());
  m_child = child;
  m_child->incRefCount();
  m_child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::ConditionWaitHandle);
  incRefCount();

  auto const session = AsioSession::Get();
  if (UNLIKELY(session->hasOnConditionCreate())) {
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
  tvCopy(
    make_tv<KindOfObject>(getNotNotifiedException().detach()),
    m_resultOrException
  );
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

///////////////////////////////////////////////////////////////////////////////
}
