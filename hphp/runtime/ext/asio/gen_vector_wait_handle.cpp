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

#include "hphp/runtime/ext/asio/gen_vector_wait_handle.h"

#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include <hphp/runtime/ext/asio/static_wait_handle.h>
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

void c_GenVectorWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnGenVectorCreateCallback(callback);
}

Object c_GenVectorWaitHandle::ti_create(const Variant& dependencies) {
  if (UNLIKELY(!dependencies.isObject() ||
      dependencies.getObjectData()->getCollectionType() !=
        Collection::VectorType)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be an instance of Vector"));
    throw e;
  }
  assert(dependencies.getObjectData()->instanceof(c_Vector::classof()));
  auto deps = SmartPtr<c_Vector>::attach(
    c_Vector::Clone(dependencies.getObjectData()));
  for (int64_t iter_pos = 0; iter_pos < deps->size(); ++iter_pos) {
    Cell* current = deps->at(iter_pos);

    if (UNLIKELY(!c_WaitHandle::fromCell(current))) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected dependencies to be a vector of WaitHandle instances"));
      throw e;
    }
  }

  Object exception;
  for (int64_t iter_pos = 0; iter_pos < deps->size(); ++iter_pos) {

    auto current = tvAssertCell(deps->at(iter_pos));
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      auto result = child->getResult();
      deps->set(iter_pos, &result);
    } else if (child->isFailed()) {
      putException(exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitableWaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      auto my_wh = makeSmartPtr<c_GenVectorWaitHandle>();
      my_wh->initialize(exception, deps.get(), iter_pos, child_wh);
      AsioSession* session = AsioSession::Get();
      if (UNLIKELY(session->hasOnGenVectorCreateCallback())) {
        session->onGenVectorCreate(my_wh.get(), dependencies);
      }
      return Object(std::move(my_wh));
    }
  }

  if (exception.isNull()) {
    return Object::attach(c_StaticWaitHandle::CreateSucceeded(
      make_tv<KindOfObject>(deps.detach())));
  } else {
    return Object::attach(c_StaticWaitHandle::CreateFailed(exception.detach()));
  }
}

void c_GenVectorWaitHandle::initialize(const Object& exception, c_Vector* deps, int64_t iter_pos, c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;

  if (isInContext()) {
    try {
      child->enterContext(getContextIdx());
    } catch (const Object& cycle_exception) {
      putException(m_exception, cycle_exception.get());
      ++m_iterPos;
      incRefCount();
      onUnblocked();
      return;
    }
  }

  blockOn(child);
  incRefCount();
}

void c_GenVectorWaitHandle::onUnblocked() {
  assert(getState() == STATE_BLOCKED);

  for (; m_iterPos < m_deps->size(); ++m_iterPos) {

    Cell* current = tvAssertCell(m_deps->at(m_iterPos));
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      auto result = child->getResult();
      m_deps->set(m_iterPos, &result);
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitableWaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      try {
        child_wh->enterContext(getContextIdx());
        detectCycle(child_wh);
        blockOn(child_wh);
        return;
      } catch (const Object& cycle_exception) {
        putException(m_exception, cycle_exception.get());
      }
    }
  }

  auto parentChain = getParentChain();
  if (m_exception.isNull()) {
    setState(STATE_SUCCEEDED);
    tvWriteObject(m_deps.get(), &m_resultOrException);
  } else {
    setState(STATE_FAILED);
    tvWriteObject(m_exception.get(), &m_resultOrException);
    m_exception = nullptr;
  }

  m_deps = nullptr;
  parentChain.unblock();
  decRefObj(this);
}

String c_GenVectorWaitHandle::getName() {
  return s_genVector;
}

c_WaitableWaitHandle* c_GenVectorWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return static_cast<c_WaitableWaitHandle*>(m_deps->at(m_iterPos)->m_data.pobj);
}

void c_GenVectorWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);

  // recursively import current child
  {
    assert(m_iterPos < m_deps->size());
    Cell* current = tvAssertCell(m_deps->at(m_iterPos));

    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));
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

      Cell* current = tvAssertCell(m_deps->at(iter_pos));
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
