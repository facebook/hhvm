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

#include "hphp/runtime/ext/asio/ext_priority-bridge-wait-handle.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

namespace {
StaticString s_priorityBridge("<priority_bridge>");
}

void HHVM_STATIC_METHOD(PriorityBridgeWaitHandle, setOnCreateCallback,
                        const Variant &callback) {
  AsioSession::Get()->setOnPriorityBridgeCreate(callback);
}

Object HHVM_STATIC_METHOD(PriorityBridgeWaitHandle, create,
                          const Object& childObj) {
  if (!childObj->instanceof(c_Awaitable::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected child to be an instance of Awaitable");
  }

  auto childWh = wait_handle<c_Awaitable>(childObj.get());
  if (UNLIKELY(childWh->isFinished())) {
    SystemLib::throwInvalidArgumentExceptionObject(
        "Expected child to be in an unfinished state");
  }

  assertx(childWh->instanceof(c_WaitableWaitHandle::classof()));
  auto const childWwh = static_cast<c_WaitableWaitHandle*>(childWh);

  auto wh = req::make<c_PriorityBridgeWaitHandle>();
  wh->initialize(childWwh);
  return Object(std::move(wh));
}

void c_PriorityBridgeWaitHandle::initialize(c_WaitableWaitHandle* child) {
  assertx(!child->isFinished());
  setState(STATE_BLOCKED);

  // If the child is in a different context, use the child's context state,
  // otherwise use the current context state.
  auto const session = AsioSession::Get();
  auto const ctxStateIdx = session->getCurrentContextStateIndex();
  setContextStateIndex(child->getContextIndex() != ctxStateIdx.contextIndex()
    ? child->getContextStateIndex()
    : ctxStateIdx
  );

  m_child = child;
  m_child->incRefCount();
  m_child->getParentChain().addParent(
      m_blockable, AsioBlockable::Kind::PriorityBridgeWaitHandle);
  incRefCount();


  if (UNLIKELY(session->hasOnPriorityBridgeCreate())) {
    session->onPriorityBridgeCreate(this, child);
  }
}

void c_PriorityBridgeWaitHandle::onUnblocked() {
  auto parentChain = getParentChain();

  // Propagate the child's result.
  setState(m_child->getState());
  assertx(isFinished());
  tvCopy(m_child->m_resultOrException, m_resultOrException);
  decRefObj(m_child);
  m_child = nullptr;

  parentChain.unblock();
  decRefObj(this);
}

String c_PriorityBridgeWaitHandle::getName() { return s_priorityBridge; }

c_WaitableWaitHandle* c_PriorityBridgeWaitHandle::getChild() {
  assertx(getState() == STATE_BLOCKED);
  return m_child;
}

void c_PriorityBridgeWaitHandle::prioritize() {
  if (isFinished()) {
    return;
  }

  auto const child = getChild();
  assertx(!child->isFinished());

  m_prioritized = true;
  asio::enter_context_state(child, getContextStateIndex());
}

void HHVM_METHOD(PriorityBridgeWaitHandle, prioritize) {
  auto obj = wait_handle<c_PriorityBridgeWaitHandle>(this_);
  obj->prioritize();
}

void AsioExtension::registerNativePriorityBridgeWaitHandle() {
  HHVM_STATIC_MALIAS(HH\\PriorityBridgeWaitHandle, create,
                     PriorityBridgeWaitHandle, create);
  HHVM_STATIC_MALIAS(HH\\PriorityBridgeWaitHandle, setOnCreateCallback,
                     PriorityBridgeWaitHandle, setOnCreateCallback);
  HHVM_MALIAS(HH\\PriorityBridgeWaitHandle, prioritize,
              PriorityBridgeWaitHandle, prioritize);

  Native::registerClassExtraDataHandler(
      c_PriorityBridgeWaitHandle::className(),
      finish_class<c_PriorityBridgeWaitHandle>);
}

} // namespace HPHP
