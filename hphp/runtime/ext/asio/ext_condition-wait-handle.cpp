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

#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"

#include "hphp/runtime/ext/asio/ext_asio.h"
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

  [[noreturn]] NEVER_INLINE
  void throwNotNotifiedException() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "ConditionWaitHandle not notified by its child");
  }

  [[noreturn]] NEVER_INLINE
  void failAlreadyFinished() {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Unable to notify ConditionWaitHandle that has already finished");
  }
}

void HHVM_STATIC_METHOD(ConditionWaitHandle, setOnCreateCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnConditionCreate(callback);
}

Object HHVM_STATIC_METHOD(ConditionWaitHandle, create,
                          const Variant& child) {
  // Child not an Awaitable?
  auto const child_wh = c_Awaitable::fromTV(*child.asTypedValue());
  if (UNLIKELY(!child_wh)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected child to be an instance of Awaitable");
  }

  // Child finished before notification?
  if (UNLIKELY(child_wh->isFinished())) {
    throwNotNotifiedException();
  }

  assertx(child_wh->instanceof(c_WaitableWaitHandle::classof()));
  auto const child_wwh = static_cast<c_WaitableWaitHandle*>(child_wh);

  auto wh = req::make<c_ConditionWaitHandle>();
  wh->initialize(child_wwh);
  return Object(std::move(wh));
}

void HHVM_METHOD(ConditionWaitHandle, succeed, const Variant& result) {
  auto obj = wait_handle<c_ConditionWaitHandle>(this_);
  if (obj->isFinished()) {
    failAlreadyFinished();
  }

  assertx(obj->getState() == c_ConditionWaitHandle::STATE_BLOCKED);
  auto parentChain = obj->getParentChain();
  obj->setState(c_ConditionWaitHandle::STATE_SUCCEEDED);
  tvDup(*result.asTypedValue(), obj->m_resultOrException);
  parentChain.unblock();
}

void HHVM_METHOD(ConditionWaitHandle, fail, const Object& exception) {
  if (!exception->instanceof(SystemLib::getThrowableClass())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected exception to be an instance of Throwable");
  }
  auto obj = wait_handle<c_ConditionWaitHandle>(this_);

  if (obj->isFinished()) {
    failAlreadyFinished();
  }

  assertx(obj->getState() == c_ConditionWaitHandle::STATE_BLOCKED);
  auto parentChain = obj->getParentChain();
  obj->setState(c_ConditionWaitHandle::STATE_FAILED);
  tvDup(make_tv<KindOfObject>(exception.get()), obj->m_resultOrException);
  parentChain.unblock();
}

void c_ConditionWaitHandle::initialize(c_WaitableWaitHandle* child) {
  assertx(!child->isFinished());

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
  assertx(getState() == STATE_BLOCKED);
  return m_child;
}

///////////////////////////////////////////////////////////////////////////////

void AsioExtension::initConditionWaitHandle() {
  HHVM_STATIC_MALIAS(HH\\ConditionWaitHandle, create,
                     ConditionWaitHandle, create);
  HHVM_STATIC_MALIAS(HH\\ConditionWaitHandle, setOnCreateCallback,
                     ConditionWaitHandle, setOnCreateCallback);
  HHVM_MALIAS(HH\\ConditionWaitHandle, succeed, ConditionWaitHandle, succeed);
  HHVM_MALIAS(HH\\ConditionWaitHandle, fail, ConditionWaitHandle, fail);

  Native::registerClassExtraDataHandler(
    c_ConditionWaitHandle::className(), finish_class<c_ConditionWaitHandle>);
}

///////////////////////////////////////////////////////////////////////////////
}
