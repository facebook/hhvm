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

#include "hphp/runtime/ext/asio/ext_gen-vector-wait-handle.h"

#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include <hphp/runtime/ext/asio/ext_static-wait-handle.h>
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
  AsioSession::Get()->setOnGenVectorCreate(callback);
}

Object c_GenVectorWaitHandle::ti_create(const Variant& dependencies) {
  ObjectData* obj;
  if (UNLIKELY(!dependencies.isObject() ||
      !(obj = dependencies.getObjectData())->isCollection() ||
      obj->collectionType() != CollectionType::Vector)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be an instance of Vector");
  }
  assertx(collections::isType(obj->getVMClass(), CollectionType::Vector));
  auto deps = req::ptr<c_Vector>::attach(c_Vector::Clone(obj));
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  for (int64_t iter_pos = 0; iter_pos < deps->size(); ++iter_pos) {
    Cell* current = deps->at(iter_pos);

    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Expected dependencies to be a vector of WaitHandle instances");
    }

    if (!child->isFinished()) {
      ctx_idx = std::min(
        ctx_idx,
        static_cast<c_WaitableWaitHandle*>(child)->getContextIdx()
      );
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

      auto my_wh = req::make<c_GenVectorWaitHandle>();
      my_wh->initialize(exception, deps.get(), iter_pos, ctx_idx, child_wh);
      AsioSession* session = AsioSession::Get();
      if (UNLIKELY(session->hasOnGenVectorCreate())) {
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

void c_GenVectorWaitHandle::initialize(
  const Object& exception,
  c_Vector* deps,
  int64_t iter_pos,
  context_idx_t ctx_idx,
  c_WaitableWaitHandle* child
) {
  setState(STATE_BLOCKED);
  setContextIdx(ctx_idx);
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;

  child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::GenVectorWaitHandle);
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
        detectCycle(child_wh);
        child_wh->getParentChain()
          .addParent(m_blockable, AsioBlockable::Kind::GenVectorWaitHandle);
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

///////////////////////////////////////////////////////////////////////////////
}
