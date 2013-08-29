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

#include <hphp/runtime/ext/ext_collections.h>
#include <hphp/runtime/ext/ext_asio.h>
#include <hphp/runtime/ext/ext_closure.h>
#include <hphp/runtime/ext/asio/asio_context.h>
#include <hphp/runtime/ext/asio/asio_session.h>
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

c_GenMapWaitHandle::c_GenMapWaitHandle(Class* cb)
    : c_BlockableWaitHandle(cb), m_exception() {
}

c_GenMapWaitHandle::~c_GenMapWaitHandle() {
}

void c_GenMapWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use GenMapWaitHandle::create() instead of constructor"));
  throw e;
}

void c_GenMapWaitHandle::ti_setoncreatecallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set GenMapWaitHandle::onCreate: on_create_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnGenMapCreateCallback(callback.getObjectDataOrNull());
}

Object c_GenMapWaitHandle::ti_create(CVarRef dependencies) {
  if (UNLIKELY(!dependencies.instanceof(c_Map::s_cls))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected dependencies to be an instance of Map"));
    throw e;
  }
  assert(dependencies.getObjectData()->instanceof(c_Map::s_cls));
  p_Map deps = static_cast<c_Map*>(dependencies.getObjectData())->clone();
  for (ssize_t iter_pos = deps->iter_begin();
       iter_pos;
       iter_pos = deps->iter_next(iter_pos)) {

    Cell* current = tvAssertCell(deps->iter_value(iter_pos));
    if (UNLIKELY(!c_WaitHandle::fromCell(current))) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected dependencies to be a map of WaitHandle instances"));
      throw e;
    }
  }

  Object exception;
  for (ssize_t iter_pos = deps->iter_begin();
       iter_pos;
       iter_pos = deps->iter_next(iter_pos)) {

    Cell* current = tvAssertCell(deps->iter_value(iter_pos));
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::s_cls));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      cellSet(child->getResult(), *current);
    } else if (child->isFailed()) {
      putException(exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitableWaitHandle::s_cls));
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
    return c_StaticResultWaitHandle::Create(make_tv<KindOfObject>(deps.get()));
  } else {
    return c_StaticExceptionWaitHandle::Create(exception.get());
  }
}

void c_GenMapWaitHandle::initialize(CObjRef exception, c_Map* deps, ssize_t iter_pos, c_WaitableWaitHandle* child) {
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;
  try {
    blockOn(child);
  } catch (const Object& cycle_exception) {
    putException(m_exception, cycle_exception.get());
    m_iterPos = m_deps->iter_next(m_iterPos);
    onUnblocked();
  }
}

void c_GenMapWaitHandle::onUnblocked() {
  for (;
       m_iterPos;
       m_iterPos = m_deps->iter_next(m_iterPos)) {

    Cell* current = tvAssertCell(m_deps->iter_value(m_iterPos));
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::s_cls));
    auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

    if (child->isSucceeded()) {
      cellSet(child->getResult(), *current);
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitHandle::s_cls));
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
    setResult(make_tv<KindOfObject>(m_deps.get()));
    m_deps = nullptr;
  } else {
    setException(m_exception.get());
    m_exception = nullptr;
    m_deps = nullptr;
  }
}

String c_GenMapWaitHandle::getName() {
  return s_genMap;
}

c_WaitableWaitHandle* c_GenMapWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return static_cast<c_WaitableWaitHandle*>(
      m_deps->iter_value(m_iterPos)->m_data.pobj);
}

void c_GenMapWaitHandle::enterContext(context_idx_t ctx_idx) {
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
    assert(m_iterPos);
    Cell* current = tvAssertCell(m_deps->iter_value(m_iterPos));

    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitableWaitHandle::s_cls));
    auto child_wh = static_cast<c_WaitableWaitHandle*>(current->m_data.pobj);
    child_wh->enterContext(ctx_idx);
  }

  // import ourselves
  setContextIdx(ctx_idx);

  // try to import other children
  try {
    for (ssize_t iter_pos = m_deps->iter_next(m_iterPos);
         iter_pos;
         iter_pos = m_deps->iter_next(iter_pos)) {

      Cell* current = tvAssertCell(m_deps->iter_value(iter_pos));
      assert(current->m_type == KindOfObject);
      assert(current->m_data.pobj->instanceof(c_WaitHandle::s_cls));
      auto child = static_cast<c_WaitHandle*>(current->m_data.pobj);

      if (child->isFinished()) {
        continue;
      }

      assert(child->instanceof(c_WaitableWaitHandle::s_cls));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);
      child_wh->enterContext(ctx_idx);
    }
  } catch (const Object& cycle_exception) {
    // exception will be eventually processed by onUnblocked()
  }
}

///////////////////////////////////////////////////////////////////////////////
}
