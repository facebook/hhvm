/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/asio/ext_gen-array-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_gen-map-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_gen-vector-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_result("<result>");
const StaticString s_exception("<exception>");

void HHVM_STATIC_METHOD(WaitHandle, setOnIoWaitEnterCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnIOWaitEnter(callback);
}

void HHVM_STATIC_METHOD(WaitHandle, setOnIoWaitExitCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnIOWaitExit(callback);
}

void HHVM_STATIC_METHOD(WaitHandle, setOnJoinCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnJoin(callback);
}

// throws if cross-context cycle found
void HHVM_METHOD(WaitHandle, import) {
  auto obj = wait_handle<c_WaitHandle>(this_);
  if (obj->isFinished()) {
    return;
  }

  assert(obj->instanceof(c_WaitableWaitHandle::classof()));
  auto const ctx_idx = AsioSession::Get()->getCurrentContextIdx();
  asio::enter_context(static_cast<c_WaitableWaitHandle*>(this_), ctx_idx);
}

Variant HHVM_METHOD(WaitHandle, join) {
  auto obj = wait_handle<c_WaitHandle>(this_);
  if (!obj->isFinished()) {
    // run the full blown machinery
    assert(obj->instanceof(c_WaitableWaitHandle::classof()));
    static_cast<c_WaitableWaitHandle*>(obj)->join();
  }
  assert(obj->isFinished());

  if (LIKELY(obj->isSucceeded())) {
    // succeeded? return result
    return cellAsCVarRef(obj->getResult());
  } else {
    // failed? throw exception
    throw_object(Object{obj->getException()});
  }
}

bool HHVM_METHOD(WaitHandle, isFinished) {
  return wait_handle<c_WaitHandle>(this_)->isFinished();
}

bool HHVM_METHOD(WaitHandle, isSucceeded) {
  return wait_handle<c_WaitHandle>(this_)->isSucceeded();
}

bool HHVM_METHOD(WaitHandle, isFailed) {
  return wait_handle<c_WaitHandle>(this_)->isFailed();
}

int64_t HHVM_METHOD(WaitHandle, getId) {
  return wait_handle<c_WaitHandle>(this_)->getId();
}

String HHVM_METHOD(WaitHandle, getName) {
  auto obj = wait_handle<c_WaitHandle>(this_);
  if (obj->isSucceeded()) {
    return s_result;
  } else if (obj->isFailed()) {
    return s_exception;
  }

  assert(obj->instanceof(c_WaitableWaitHandle::classof()));
  return static_cast<c_WaitableWaitHandle*>(obj)->getName();
}

///////////////////////////////////////////////////////////////////////////////

size_t asio_object_size(const ObjectData* od) {
  auto obj = wait_handle<c_WaitHandle>(od);
  switch (obj->getKind()) {
#define X(knd) \
    case c_WaitHandle::Kind::knd: \
      return sizeof(c_ ## knd ## WaitHandle);
    X(Static)
    X(GenArray)
    X(GenMap)
    X(GenVector)
    X(Condition)
    X(Reschedule)
    X(Sleep)
    X(ExternalThreadEvent)
    X(AsyncGenerator)
#undef X
    case c_WaitHandle::Kind::AwaitAll:
      return obj->asAwaitAll()->heapSize();
    case c_WaitHandle::Kind::AsyncFunction:
      return obj->asAsyncFunction()->resumable()->size();
  }
  always_assert(false);
}

void AsioExtension::initWaitHandle() {
#define WH_SME(meth) \
  HHVM_STATIC_MALIAS(HH\\WaitHandle, meth, WaitHandle, meth)
  WH_SME(setOnIoWaitEnterCallback);
  WH_SME(setOnIoWaitExitCallback);
  WH_SME(setOnJoinCallback);
#undef WH_SME

#define WH_ME(meth) \
  HHVM_MALIAS(HH\\WaitHandle, meth, WaitHandle, meth)
  WH_ME(import);
  WH_ME(join);
  WH_ME(isFinished);
  WH_ME(isSucceeded);
  WH_ME(isFailed);
  WH_ME(getId);
  WH_ME(getName);
#undef WH_ME
}

const StaticString
  s_DoNotNewInstance("WaitHandles may not be directly instantiated");

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
  std::enable_if<std::is_base_of<c_WaitHandle, T>::value, void>::type
finish_class() {
  DEBUG_ONLY auto const wh = c_WaitHandle::classof();
  auto cls = const_cast<Class*>(T::classof());
  assert(wh && cls);
  assert((cls == wh) || (cls->classof(wh)));
  assert(cls->numDeclProperties() == 0);
  assert(cls->numStaticProperties() == 0);
  DEBUG_ONLY auto const ctor = cls->getCtor();
  assert(ctor == wh->getCtor());
  assert(ctor->attrs() & AttrPrivate);
  assert(ctor->attrs() & AttrFinal);

  cls->allocExtraData();
  assert(!cls->m_extra->m_nativeDataInfo);
  assert(!cls->m_extra->m_instanceCtor);
  assert(!cls->m_extra->m_instanceDtor);
  cls->m_extra.raw()->m_instanceCtor = asioInstanceCtor;
  cls->m_extra.raw()->m_instanceDtor = T::instanceDtor;
}

void AsioExtension::finishClasses() {
  finish_class<c_WaitHandle>();
  finish_class<c_WaitableWaitHandle>();
  finish_class<c_AwaitAllWaitHandle>();
  finish_class<c_GenArrayWaitHandle>();
  finish_class<c_GenMapWaitHandle>();
  finish_class<c_GenVectorWaitHandle>();
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
