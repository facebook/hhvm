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

#include "hphp/runtime/ext/asio/ext_wait-handle.h"

#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_result("<result>");
const StaticString s_exception("<exception>");

void HHVM_STATIC_METHOD(Awaitable, setOnIoWaitEnterCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnIOWaitEnter(callback);
}

void HHVM_STATIC_METHOD(Awaitable, setOnIoWaitExitCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnIOWaitExit(callback);
}

void HHVM_STATIC_METHOD(Awaitable, setOnJoinCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnJoin(callback);
}

bool HHVM_METHOD(Awaitable, isFinished) {
  return wait_handle<c_Awaitable>(this_)->isFinished();
}

bool HHVM_METHOD(Awaitable, isSucceeded) {
  return wait_handle<c_Awaitable>(this_)->isSucceeded();
}

bool HHVM_METHOD(Awaitable, isFailed) {
  return wait_handle<c_Awaitable>(this_)->isFailed();
}

String HHVM_METHOD(Awaitable, getName) {
  auto obj = wait_handle<c_Awaitable>(this_);
  if (obj->isSucceeded()) {
    return s_result;
  } else if (obj->isFailed()) {
    return s_exception;
  }

  assertx(obj->instanceof(c_WaitableWaitHandle::classof()));
  return static_cast<c_WaitableWaitHandle*>(obj)->getName();
}

///////////////////////////////////////////////////////////////////////////////

size_t asio_object_size(const ObjectData* od) {
  auto obj = wait_handle<c_Awaitable>(od);
  switch (obj->getKind()) {
#define X(knd) \
    case c_Awaitable::Kind::knd: \
      return sizeof(c_ ## knd ## WaitHandle);
    X(Static)
    X(Condition)
    X(Reschedule)
    X(Sleep)
    X(ExternalThreadEvent)
    X(AsyncGenerator)
#undef X
    case c_Awaitable::Kind::AwaitAll:
      return obj->asAwaitAll()->heapSize();
    case c_Awaitable::Kind::AsyncFunction:
      return obj->asAsyncFunction()->resumable()->size();
  }
  always_assert(false);
}

void AsioExtension::initWaitHandle() {
  // ensure AsioBlockable* fields are pointer-followable
  (void)type_scan::getIndexForMalloc<AsioBlockable>();
#define WH_SME(meth) \
  HHVM_STATIC_MALIAS(HH\\Awaitable, meth, Awaitable, meth)
  WH_SME(setOnIoWaitEnterCallback);
  WH_SME(setOnIoWaitExitCallback);
  WH_SME(setOnJoinCallback);
#undef WH_SME

#define WH_ME(meth) \
  HHVM_MALIAS(HH\\Awaitable, meth, Awaitable, meth)
  WH_ME(isFinished);
  WH_ME(isSucceeded);
  WH_ME(isFailed);
  WH_ME(getName);
#undef WH_ME
}

const StaticString
  s_DoNotNewInstance("Awaitables may not be directly instantiated");

static ObjectData* asioInstanceCtor(Class*) {
  SystemLib::throwExceptionObject(s_DoNotNewInstance);
}

// Asio's memory layout relies on the following invariants:
//   * Inextensible: private final (do-nothing) constructor in base class
//   * No declared properties
// This guarantees that there will be no overlap between internal asio state
// and declared property slots, and that instance methods can only be called
// on the official, systemlib base classes.
template<class T> typename
  std::enable_if<std::is_base_of<c_Awaitable, T>::value, void>::type
finish_class() {
  DEBUG_ONLY auto const wh = c_Awaitable::classof();
  auto cls = const_cast<Class*>(T::classof());
  assertx(wh && cls);
  assertx((cls == wh) || (cls->classof(wh)));
  assertx(cls->numDeclProperties() == 0);
  assertx(cls->numStaticProperties() == 0);
  assertx(!cls->hasMemoSlots());
  DEBUG_ONLY auto const ctor = cls->getCtor();
  assertx(ctor == wh->getCtor());
  assertx(ctor->attrs() & AttrPrivate);
  assertx(ctor->attrs() & AttrFinal);

  cls->allocExtraData();
  assertx(!cls->m_extra->m_nativeDataInfo);
  assertx(!cls->m_extra->m_instanceCtor);
  assertx(!cls->m_extra->m_instanceCtorUnlocked);
  assertx(!cls->m_extra->m_instanceDtor);
  cls->m_extra.raw()->m_instanceCtor = asioInstanceCtor;
  cls->m_extra.raw()->m_instanceCtorUnlocked = asioInstanceCtor;
  cls->m_extra.raw()->m_instanceDtor = T::instanceDtor;
  cls->m_releaseFunc = T::instanceDtor;
}

void AsioExtension::finishClasses() {
  finish_class<c_Awaitable>();
  finish_class<c_WaitableWaitHandle>();
  finish_class<c_AwaitAllWaitHandle>();
  finish_class<c_ResumableWaitHandle>();
  finish_class<c_AsyncFunctionWaitHandle>();
  finish_class<c_AsyncGeneratorWaitHandle>();
  finish_class<c_StaticWaitHandle>();
  finish_class<c_ConditionWaitHandle>();
  finish_class<c_SleepWaitHandle>();
  finish_class<c_RescheduleWaitHandle>();
  finish_class<c_ExternalThreadEventWaitHandle>();
}

///////////////////////////////////////////////////////////////////////////////
}
