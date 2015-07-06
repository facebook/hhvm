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

#include <hphp/runtime/ext/asio/ext_gen-map-wait-handle.h>

#include <hphp/runtime/ext/collections/ext_collections-idl.h>
#include <hphp/runtime/ext/ext_closure.h>
#include <hphp/runtime/ext/asio/asio-blockable.h>
#include <hphp/runtime/ext/asio/asio-context.h>
#include <hphp/runtime/ext/asio/asio-session.h>
#include <hphp/runtime/ext/asio/ext_static-wait-handle.h>
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
  AsioSession::Get()->setOnGenMapCreate(callback);
}

Object c_GenMapWaitHandle::ti_create(const Variant& dependencies) {
  ObjectData* obj;
  if (UNLIKELY(!dependencies.isObject() ||
      !(obj = dependencies.getObjectData())->isCollection() ||
      obj->collectionType() != CollectionType::Map)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected dependencies to be an instance of Map");
  }
  assertx(obj->collectionType() == CollectionType::Map);
  auto deps = req::ptr<c_Map>::attach(c_Map::Clone(obj));
  auto ctx_idx = std::numeric_limits<context_idx_t>::max();
  for (ssize_t iter_pos = deps->iter_begin();
       deps->iter_valid(iter_pos);
       iter_pos = deps->iter_next(iter_pos)) {

    auto* current = tvAssertCell(deps->iter_value(iter_pos));
    auto const child = c_WaitHandle::fromCell(current);
    if (UNLIKELY(!child)) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Expected dependencies to be a map of WaitHandle instances");
    }

    if (!child->isFinished()) {
      ctx_idx = std::min(
        ctx_idx,
        static_cast<c_WaitableWaitHandle*>(child)->getContextIdx()
      );
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
      auto result = child->getResult();
      deps->set(k.asCell(), &result);
    } else if (child->isFailed()) {
      putException(exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitableWaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      auto my_wh = req::make<c_GenMapWaitHandle>();
      my_wh->initialize(exception, deps.get(), iter_pos, ctx_idx, child_wh);

      AsioSession* session = AsioSession::Get();
      if (UNLIKELY(session->hasOnGenMapCreate())) {
        session->onGenMapCreate(my_wh.get(), dependencies);
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

void c_GenMapWaitHandle::initialize(
  const Object& exception,
  c_Map* deps,
  ssize_t iter_pos,
  context_idx_t ctx_idx,
  c_WaitableWaitHandle* child
) {
  setState(STATE_BLOCKED);
  setContextIdx(ctx_idx);
  m_exception = exception;
  m_deps = deps;
  m_iterPos = iter_pos;

  child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::GenMapWaitHandle);
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
      auto result = child->getResult();
      m_deps->set(k.asCell(), &result);
    } else if (child->isFailed()) {
      putException(m_exception, child->getException());
    } else {
      assert(child->instanceof(c_WaitHandle::classof()));
      auto child_wh = static_cast<c_WaitableWaitHandle*>(child);

      try {
        detectCycle(child_wh);
        child_wh->getParentChain()
          .addParent(m_blockable, AsioBlockable::Kind::GenMapWaitHandle);
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

///////////////////////////////////////////////////////////////////////////////
}
