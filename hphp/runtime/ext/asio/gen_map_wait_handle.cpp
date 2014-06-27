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

#include <hphp/runtime/ext/asio/gen_map_wait_handle.h>

#include <hphp/runtime/ext/ext_collections.h>
#include <hphp/runtime/ext/ext_closure.h>
#include <hphp/runtime/ext/asio/asio_blockable.h>
#include <hphp/runtime/ext/asio/asio_context.h>
#include <hphp/runtime/ext/asio/asio_session.h>
#include <hphp/runtime/ext/asio/static_wait_handle.h>
#include <hphp/system/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_genMap("<gen-map>");

  void putException(Object& exception_field, ObjectData* new_exception) {
    assert(new_exception);
    assert(new_exception->instanceof(SystemLib::s_ExceptionClass));

    if (exception_field.isNull()) {
      exception_field = new_exception;
    }
  }
}

void c_GenMapWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnGenMapCreateCallback(callback);
}

Object c_GenMapWaitHandle::ti_create(const Variant& dependencies) {
  if (UNLIKELY(!dependencies.isObject() ||
      dependencies.getObjectData()->getCollectionType() !=
        Collection::MapType)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be an instance of Map"));
    throw e;
  }
  assert(dependencies.getObjectData()->instanceof(c_Map::classof()));
  auto deps = p_Map::attach(c_Map::Clone(dependencies.getObjectData()));
  for (ssize_t iter_pos = deps->iter_begin();
       deps->iter_valid(iter_pos);
       iter_pos = deps->iter_next(iter_pos)) {

    auto* current = tvAssertCell(deps->iter_value(iter_pos));
    if (UNLIKELY(!c_WaitHandle::fromCell(current))) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected dependencies to be a map of WaitHandle instances"));
      throw e;
    }
  }

  Object exception;
  for (ssize_t iter_pos = deps->iter_begin();
       deps->iter_valid(iter_pos);
       iter_pos = deps->iter_next(iter_pos)) {

    auto* current = tvAssertCell(deps->iter_value(iter_pos));
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      auto k = deps->iter_key(iter_pos);
      deps->set(k.asCell(), &child->getResult());
    } else if (child->isFailed()) {
      putException(exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitableWaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      p_GenMapWaitHandle my_wh = NEWOBJ(c_GenMapWaitHandle)();
      my_wh->initialize(exception, deps.get(), iter_pos, child_wh);

      AsioSession* session = AsioSession::Get();
      if (UNLIKELY(session->hasOnGenMapCreateCallback())) {
        session->onGenMapCreate(my_wh.get(), dependencies);
      }

      return my_wh;
    }
  }

  if (exception.isNull()) {
    return Object::attach(c_StaticWaitHandle::CreateSucceeded(
      make_tv<KindOfObject>(deps.detach())));
  } else {
    return Object::attach(c_StaticWaitHandle::CreateFailed(exception.detach()));
  }
}

void c_GenMapWaitHandle::initialize(const Object& exception, c_Map* deps, ssize_t iter_pos, c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;

  if (isInContext()) {
    try {
      child->enterContext(getContextIdx());
    } catch (const Object& cycle_exception) {
      putException(m_exception, cycle_exception.get());
      m_iterPos = m_deps->iter_next(m_iterPos);
      incRefCount();
      onUnblocked();
      return;
    }
  }

  blockOn(child);
  incRefCount();
}

void c_GenMapWaitHandle::onUnblocked() {
  assert(getState() == STATE_BLOCKED);

  for (;
       m_deps->iter_valid(m_iterPos);
       m_iterPos = m_deps->iter_next(m_iterPos)) {

    auto* current = tvAssertCell(m_deps->iter_value(m_iterPos));
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      auto k = m_deps->iter_key(m_iterPos);
      m_deps->set(k.asCell(), &child->getResult());
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      try {
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

String c_GenMapWaitHandle::getName() {
  return s_genMap;
}

c_WaitableWaitHandle* c_GenMapWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return static_cast<c_WaitableWaitHandle*>(
      m_deps->iter_value(m_iterPos)->m_data.pobj);
}

void c_GenMapWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);

  // recursively import current child
  {
    auto* current = tvAssertCell(m_deps->iter_value(m_iterPos));

    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitableWaitHandle::classof()));
    auto child_wh = static_cast<c_WaitableWaitHandle*>(current->m_data.pobj);
    child_wh->enterContext(ctx_idx);
  }

  // import ourselves
  setContextIdx(ctx_idx);

  // try to import other children
  try {
    for (ssize_t iter_pos = m_deps->iter_next(m_iterPos);
         m_deps->iter_valid(iter_pos);
         iter_pos = m_deps->iter_next(iter_pos)) {

      auto* current = tvAssertCell(m_deps->iter_value(iter_pos));
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
