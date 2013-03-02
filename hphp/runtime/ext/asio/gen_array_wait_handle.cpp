/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_asio.h>
#include <runtime/ext/asio/asio_context.h>
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_genArray("<gen-array>");

  void putException(Object& exception_field, ObjectData* new_exception) {
    assert(new_exception);
    assert(new_exception->o_instanceof("Exception"));

    if (exception_field.isNull()) {
      exception_field = new_exception;
    }
  }

  void putInvalidArgumentException(Object& exception_field) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected dependencies to be an array of WaitHandle instances"));
    putException(exception_field, e.get());
  }
}

c_GenArrayWaitHandle::c_GenArrayWaitHandle(VM::Class* cb)
    : c_BlockableWaitHandle(cb), m_exception() {
}

c_GenArrayWaitHandle::~c_GenArrayWaitHandle() {
}

void c_GenArrayWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use GenArrayWaitHandle::create() instead of constructor"));
  throw e;
}

Object c_GenArrayWaitHandle::ti_create(const char* cls, CArrRef dependencies) {
  Array deps = dependencies->copy();
  Object exception;
  for (ssize_t iter_pos = deps->iter_begin();
       iter_pos != ArrayData::invalid_index;
       iter_pos = deps->iter_advance(iter_pos)) {

    TypedValue* current = deps->nvGetValueRef(iter_pos);
    if (UNLIKELY(current->m_type == KindOfRef)) {
      tvUnbox(current);
    }

    if (IS_NULL_TYPE(current->m_type)) {
      // {uninit,null} yields null
      tvWriteNull(current);
      continue;
    }

    c_WaitHandle* child = c_WaitHandle::fromTypedValue(current);
    if (UNLIKELY(!child)) {
      putInvalidArgumentException(exception);
    } else if (child->isSucceeded()) {
      tvSetIgnoreRef(child->getResult(), current);
    } else if (child->isFailed()) {
      putException(exception, child->getException());
    } else {
      c_WaitableWaitHandle* child_wh =
        static_cast<c_WaitableWaitHandle*>(child);

      c_GenArrayWaitHandle* my_wh = NEWOBJ(c_GenArrayWaitHandle)();
      my_wh->initialize(exception, deps, iter_pos, child_wh);
      return my_wh;
    }
  }

  if (exception.isNull()) {
    return c_StaticResultWaitHandle::t_create(deps);
  } else {
    return c_StaticExceptionWaitHandle::t_create(exception);
  }
}

void c_GenArrayWaitHandle::initialize(CObjRef exception, CArrRef deps, ssize_t iter_pos, c_WaitableWaitHandle* child) {
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;
  try {
    blockOn(child);
  } catch (Object cycle_exception) {
    putException(m_exception, cycle_exception.get());
    m_iterPos = m_deps->iter_advance(m_iterPos);
    onUnblocked();
  }
}

void c_GenArrayWaitHandle::onUnblocked() {
  for (;
       m_iterPos != ArrayData::invalid_index;
       m_iterPos = m_deps->iter_advance(m_iterPos)) {

    TypedValue* current = m_deps->nvGetValueRef(m_iterPos);
    if (UNLIKELY(current->m_type == KindOfRef)) {
      tvUnbox(current);
    }

    if (IS_NULL_TYPE(current->m_type)) {
      // {uninit,null} yields null
      tvWriteNull(current);
      continue;
    }

    c_WaitHandle* child = c_WaitHandle::fromTypedValue(current);
    if (UNLIKELY(!child)) {
      putInvalidArgumentException(m_exception);
    } else if (child->isSucceeded()) {
      tvSetIgnoreRef(child->getResult(), current);
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      c_WaitableWaitHandle* child_wh =
        static_cast<c_WaitableWaitHandle*>(child);

      try {
        blockOn(child_wh);
        return;
      } catch (Object cycle_exception) {
        putException(m_exception, cycle_exception.get());
      }
    }
  }

  if (m_exception.isNull()) {
    TypedValue result;
    result.m_type = KindOfArray;
    result.m_data.parr = m_deps.get();
    setResult(&result);
    m_deps = nullptr;
  } else {
    setException(m_exception.get());
    m_exception = nullptr;
    m_deps = nullptr;
  }
}

String c_GenArrayWaitHandle::getName() {
  return s_genArray;
}

c_WaitableWaitHandle* c_GenArrayWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return static_cast<c_WaitableWaitHandle*>(
      m_deps->nvGetValueRef(m_iterPos)->m_data.pobj);
}

void c_GenArrayWaitHandle::enterContext(context_idx_t ctx_idx) {
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

  // TODO: enterContext() for all remaining dependencies
  c_WaitableWaitHandle* wait_handle = static_cast<c_WaitableWaitHandle*>(
      m_deps->nvGetValueRef(m_iterPos)->m_data.pobj);
  wait_handle->enterContext(ctx_idx);
  setContextIdx(ctx_idx);
}

///////////////////////////////////////////////////////////////////////////////
}
