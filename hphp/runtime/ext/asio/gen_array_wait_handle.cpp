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

#include "hphp/runtime/ext/asio/gen_array_wait_handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include <hphp/runtime/ext/asio/static_exception_wait_handle.h>
#include <hphp/runtime/ext/asio/static_result_wait_handle.h>
#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/hphp-array-defs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_genArray("<gen-array>");

  void putException(Object& exception_field, ObjectData* new_exception) {
    assert(new_exception);
    assert(new_exception->instanceof(SystemLib::s_ExceptionClass));

    if (exception_field.isNull()) {
      exception_field = new_exception;
    }
  }
}

void c_GenArrayWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use GenArrayWaitHandle::create() instead of constructor"));
  throw e;
}

void c_GenArrayWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set GenArrayWaitHandle::onCreate: on_create_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnGenArrayCreateCallback(callback.getObjectDataOrNull());
}

NEVER_INLINE __attribute__((noreturn))
static void fail() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Expected dependencies to be an array of WaitHandle instances"));
  throw e;
}

Object c_GenArrayWaitHandle::ti_create(const Array& inputDependencies) {
  Array depCopy = inputDependencies->copy();
  if (UNLIKELY(depCopy->kind() > ArrayData::kMixedKind)) {
    // The only array kind that can return a non-kPackedKind or
    // non-kMixedKind from ->copy() is NameValueTableWrapper, which
    // returns itself.  This is only for $GLOBALS, which is about to
    // throw anyway since it will fail the WaitHandle checks below.
    fail();
  }
  assert(depCopy->kind() == ArrayData::kPackedKind ||
         depCopy->kind() == ArrayData::kMixedKind);

  Object exception;

  HphpArray::ValIter arrIter(static_cast<HphpArray*>(depCopy.get()));
  for (; !arrIter.empty(); arrIter.advance()) {
    auto const current = &arrIter.current()->data;
    if (UNLIKELY(current->m_type == KindOfRef)) {
      tvUnbox(current);
    }
    if (IS_NULL_TYPE(current->m_type)) continue;

    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) fail();

    if (child->isSucceeded()) {
      cellSet(child->getResult(), *current);
      continue;
    }
    if (UNLIKELY(child->isFailed())) {
      putException(exception, child->getException());
      continue;
    }

    /*
     * We found a handle that isn't finished yet.
     *
     * We need to finish unboxing any refs in our dependency array,
     * and then create a GenArrayWaitHandle to wrap it.
     */
    auto const current_pos = arrIter.currentPos();
    arrIter.advance();
    for (; !arrIter.empty(); arrIter.advance()) {
      auto const future = &arrIter.current()->data;
      if (UNLIKELY(future->m_type == KindOfRef)) {
        tvUnbox(future);
      }
      if (IS_NULL_TYPE(future->m_type)) continue;
      if (UNLIKELY(!c_WaitHandle::fromCell(future))) fail();
    }

    assert(child->instanceof(c_WaitableWaitHandle::classof()));
    auto const child_wh = static_cast<c_WaitableWaitHandle*>(child);
    p_GenArrayWaitHandle my_wh = NEWOBJ(c_GenArrayWaitHandle)();
    my_wh->initialize(exception, depCopy, current_pos, child_wh);

    auto const session = AsioSession::Get();
    if (UNLIKELY(session->hasOnGenArrayCreateCallback())) {
      session->onGenArrayCreate(my_wh.get(), inputDependencies);
    }

    return my_wh;
  }

  // Down here, everything was finished.  If there's an exception,
  // that's all we give back.  Otherwise give back the array of
  // results.
  if (exception.isNull()) {
    return c_StaticResultWaitHandle::Create(
      make_tv<KindOfArray>(depCopy.get()));
  } else {
    return c_StaticExceptionWaitHandle::Create(exception.get());
  }
}

void c_GenArrayWaitHandle::initialize(const Object& exception, const Array& deps, ssize_t iter_pos, c_WaitableWaitHandle* child) {
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;

  if (isInContext()) {
    try {
      child->enterContext(getContextIdx());
    } catch (const Object& cycle_exception) {
      putException(m_exception, cycle_exception.get());
      m_iterPos = m_deps->iter_advance(m_iterPos);
      onUnblocked();
      return;
    }
  }

  blockOn(child);
}

void c_GenArrayWaitHandle::onUnblocked() {
  HphpArray::ValIter arrIter(static_cast<HphpArray*>(m_deps.get()), m_iterPos);

  for (; !arrIter.empty(); arrIter.advance()) {
    auto const current = tvAssertCell(&arrIter.current()->data);

    if (IS_NULL_TYPE(current->m_type)) continue;
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));

    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      cellSet(child->getResult(), *current);
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitableWaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      try {
        m_iterPos = arrIter.currentPos();
        if (isInContext()) {
          child_wh->enterContext(getContextIdx());
        }
        detectCycle(child_wh);
        blockOn(child_wh);
        return;
      } catch (const Object& cycle_exception) {
        putException(m_exception, cycle_exception.get());
      }
    }
  }

  m_iterPos = arrIter.currentPos();

  if (m_exception.isNull()) {
    setResult(make_tv<KindOfArray>(m_deps.get()));
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

void c_GenArrayWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);

  // recursively import current child
  {
    assert(m_iterPos != ArrayData::invalid_index);
    Cell* current = tvAssertCell(m_deps->nvGetValueRef(m_iterPos));

    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));
    auto child_wh = static_cast<c_WaitableWaitHandle*>(current->m_data.pobj);
    child_wh->enterContext(ctx_idx);
  }

  // import ourselves
  setContextIdx(ctx_idx);

  // try to import other children
  try {
    for (ssize_t iter_pos = m_deps->iter_advance(m_iterPos);
         iter_pos != ArrayData::invalid_index;
         iter_pos = m_deps->iter_advance(iter_pos)) {

      Cell* current = tvAssertCell(m_deps->nvGetValueRef(iter_pos));
      if (IS_NULL_TYPE(current->m_type)) {
        continue;
      }

      assert(current->m_type == KindOfObject);
      assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));
      auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

      if (child->isFinished()) {
        continue;
      }

      assert(child->instanceof(c_WaitableWaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);
      child_wh->enterContext(ctx_idx);
    }
  } catch (const Object& cycle_exception) {
    // exception will be eventually processed by onUnblocked()
  }
}

///////////////////////////////////////////////////////////////////////////////
}
