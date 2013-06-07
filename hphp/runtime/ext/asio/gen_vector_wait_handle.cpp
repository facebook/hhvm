/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_asio.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_genVector("<gen-vector>");

  void putException(Object& exception_field, ObjectData* new_exception) {
    assert(new_exception);
    assert(new_exception->instanceof(SystemLib::s_ExceptionClass));

    if (exception_field.isNull()) {
      exception_field = new_exception;
    }
  }
}

c_GenVectorWaitHandle::c_GenVectorWaitHandle(Class* cb)
    : c_BlockableWaitHandle(cb), m_exception() {
}

c_GenVectorWaitHandle::~c_GenVectorWaitHandle() {
}

void c_GenVectorWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use GenVectorWaitHandle::create() instead of constructor"));
  throw e;
}

void c_GenVectorWaitHandle::ti_setoncreatecallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set GenVectorWaitHandle::onCreate: on_create_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnGenVectorCreateCallback(callback.getObjectDataOrNull());
}

Object c_GenVectorWaitHandle::ti_create(CVarRef dependencies) {
  if (UNLIKELY(!dependencies.instanceof(c_Vector::s_cls))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be an instance of Vector"));
    throw e;
  }
  assert(dynamic_cast<c_Vector*>(dependencies.getObjectData()));
  p_Vector deps = static_cast<c_Vector*>(dependencies.getObjectData())->clone();
  for (int64_t iter_pos = 0; iter_pos < deps->size(); ++iter_pos) {
    TypedValue* current = deps->at(iter_pos);

    if (!c_WaitHandle::fromTypedValue(current)) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected dependencies to be a vector of WaitHandle instances"));
      throw e;
    }
  }

  Object exception;
  for (int64_t iter_pos = 0; iter_pos < deps->size(); ++iter_pos) {

    TypedValue* current = deps->at(iter_pos);
    assert(current->m_type == KindOfObject);
    assert(dynamic_cast<c_WaitHandle*>(current->m_data.pobj));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      tvSetIgnoreRef(child->getResult(), current);
    } else if (child->isFailed()) {
      putException(exception, child->getException());
    } else {
      assert(dynamic_cast<c_WaitableWaitHandle*>(child));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      p_GenVectorWaitHandle my_wh = NEWOBJ(c_GenVectorWaitHandle)();
      my_wh->initialize(exception, deps.get(), iter_pos, child_wh);
      AsioSession* session = AsioSession::Get();
      if (UNLIKELY(session->hasOnGenVectorCreateCallback())) {
        session->onGenVectorCreate(my_wh.get(), dependencies);
      }
      return my_wh;
    }
  }

  if (exception.isNull()) {
    TypedValue tv;
    tv.m_type = KindOfObject;
    tv.m_data.pobj = deps.get();
    return c_StaticResultWaitHandle::Create(&tv);
  } else {
    return c_StaticExceptionWaitHandle::Create(exception.get());
  }
}

void c_GenVectorWaitHandle::initialize(CObjRef exception, c_Vector* deps, int64_t iter_pos, c_WaitableWaitHandle* child) {
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;
  try {
    blockOn(child);
  } catch (const Object& cycle_exception) {
    putException(m_exception, cycle_exception.get());
    ++m_iterPos;
    onUnblocked();
  }
}

void c_GenVectorWaitHandle::onUnblocked() {
  for (; m_iterPos < m_deps->size(); ++m_iterPos) {

    TypedValue* current = m_deps->at(m_iterPos);
    assert(current->m_type == KindOfObject);
    assert(dynamic_cast<c_WaitHandle*>(current->m_data.pobj));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      tvSetIgnoreRef(child->getResult(), current);
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      assert(dynamic_cast<c_WaitableWaitHandle*>(child));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      try {
        blockOn(child_wh);
        return;
      } catch (const Object& cycle_exception) {
        putException(m_exception, cycle_exception.get());
      }
    }
  }

  if (m_exception.isNull()) {
    TypedValue result;
    result.m_type = KindOfObject;
    result.m_data.pobj = m_deps.get();
    setResult(&result);
    m_deps = nullptr;
  } else {
    setException(m_exception.get());
    m_exception = nullptr;
    m_deps = nullptr;
  }
}

String c_GenVectorWaitHandle::getName() {
  return s_genVector;
}

c_WaitableWaitHandle* c_GenVectorWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return static_cast<c_WaitableWaitHandle*>(m_deps->at(m_iterPos)->m_data.pobj);
}

void c_GenVectorWaitHandle::enterContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // already in the more specific context?
  if (LIKELY(getContextIdx() >= ctx_idx)) {
    return;
  }

  assert(getState() == STATE_BLOCKED);

  // recursively import current child
  {
    assert(m_iterPos < m_deps->size());
    TypedValue* current = m_deps->at(m_iterPos);

    assert(current->m_type == KindOfObject);
    assert(dynamic_cast<c_WaitableWaitHandle*>(current->m_data.pobj));
    auto child_wh = static_cast<c_WaitableWaitHandle*>(current->m_data.pobj);
    child_wh->enterContext(ctx_idx);
  }

  // import ourselves
  setContextIdx(ctx_idx);

  // try to import other children
  try {
    for (int64_t iter_pos = m_iterPos + 1;
         iter_pos < m_deps->size();
         ++iter_pos) {

      TypedValue* current = m_deps->at(iter_pos);
      assert(current->m_type == KindOfObject);
      assert(dynamic_cast<c_WaitHandle*>(current->m_data.pobj));
      auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

      if (child->isFinished()) {
        continue;
      }

      assert(dynamic_cast<c_WaitableWaitHandle*>(child));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);
      child_wh->enterContext(ctx_idx);
    }
  } catch (const Object& cycle_exception) {
    // exception will be eventually processed by onUnblocked()
  }
}

///////////////////////////////////////////////////////////////////////////////
}
