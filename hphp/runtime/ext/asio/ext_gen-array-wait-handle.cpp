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

#include "hphp/runtime/ext/asio/ext_gen-array-wait-handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include <hphp/runtime/ext/asio/ext_static-wait-handle.h>
#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"

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

void c_GenArrayWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnGenArrayCreate(callback);
}

NEVER_INLINE __attribute__((__noreturn__))
static void fail() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Expected dependencies to be an array of WaitHandle instances");
}

Object c_GenArrayWaitHandle::ti_create(const Array& inputDependencies) {
  auto depCopy = inputDependencies.copy();
  if (UNLIKELY(depCopy->kind() > ArrayData::kEmptyKind)) {
    // The only array kind that can return a non-k{Packed,Mixed,Empty}Kind
    // from ->copy() is GlobalsArray, which returns itself.
    // This is only for $GLOBALS, which is about to throw anyway since it
    // will fail the WaitHandle checks below.
    fail();
  }
  assert(depCopy->isPacked() || depCopy->isMixed() || depCopy->isStruct() ||
         depCopy->kind() == ArrayData::kEmptyKind);

  if (depCopy->kind() == ArrayData::kEmptyKind) {
    auto const empty = make_tv<KindOfArray>(depCopy.detach());
    return Object::attach(c_StaticWaitHandle::CreateSucceeded(empty));
  }

  Object exception;

  MixedArray::ValIter arrIter(depCopy.get());
  for (; !arrIter.empty(); arrIter.advance()) {
    auto const current = arrIter.current();
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
    assert(child->instanceof(c_WaitableWaitHandle::classof()));
    auto const child_wh = static_cast<c_WaitableWaitHandle*>(child);
    auto const current_pos = arrIter.currentPos();
    auto ctx_idx = child_wh->getContextIdx();
    arrIter.advance();
    for (; !arrIter.empty(); arrIter.advance()) {
      auto const future = arrIter.current();
      if (UNLIKELY(future->m_type == KindOfRef)) {
        tvUnbox(future);
      }
      if (IS_NULL_TYPE(future->m_type)) continue;
      auto const future_wh = c_WaitHandle::fromCell(future);
      if (UNLIKELY(!future_wh)) fail();
      if (!future_wh->isFinished()) {
        ctx_idx = std::min(
          ctx_idx,
          static_cast<c_WaitableWaitHandle*>(future_wh)->getContextIdx()
        );
      }
    }

    auto my_wh = req::make<c_GenArrayWaitHandle>();
    my_wh->initialize(exception, depCopy, current_pos, ctx_idx, child_wh);

    auto const session = AsioSession::Get();
    if (UNLIKELY(session->hasOnGenArrayCreate())) {
      session->onGenArrayCreate(my_wh.get(), inputDependencies);
    }

    return Object(std::move(my_wh));
  }

  // Down here, everything was finished.  If there's an exception,
  // that's all we give back.  Otherwise give back the array of
  // results.
  if (exception.isNull()) {
    return Object::attach(c_StaticWaitHandle::CreateSucceeded(
      make_tv<KindOfArray>(depCopy.detach())));
  } else {
    return Object::attach(c_StaticWaitHandle::CreateFailed(exception.detach()));
  }
}

void c_GenArrayWaitHandle::initialize(
  const Object& exception,
  const Array& deps,
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
    .addParent(m_blockable, AsioBlockable::Kind::GenArrayWaitHandle);
  incRefCount();
}

void c_GenArrayWaitHandle::onUnblocked() {
  assert(getState() == STATE_BLOCKED);
  MixedArray::ValIter arrIter(m_deps.get(), m_iterPos);

  for (; !arrIter.empty(); arrIter.advance()) {
    auto const current = tvAssertCell(arrIter.current());

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
        detectCycle(child_wh);
        child_wh->getParentChain()
          .addParent(m_blockable, AsioBlockable::Kind::GenArrayWaitHandle);
        return;
      } catch (const Object& cycle_exception) {
        putException(m_exception, cycle_exception.get());
      }
    }
  }

  m_iterPos = arrIter.currentPos();

  auto parentChain = getParentChain();
  if (m_exception.isNull()) {
    setState(STATE_SUCCEEDED);
    cellDup(make_tv<KindOfArray>(m_deps.get()), m_resultOrException);
  } else {
    setState(STATE_FAILED);
    tvWriteObject(m_exception.get(), &m_resultOrException);
    m_exception = nullptr;
  }

  m_deps = nullptr;
  parentChain.unblock();
  decRefObj(this);
}

String c_GenArrayWaitHandle::getName() {
  return s_genArray;
}

c_WaitableWaitHandle* c_GenArrayWaitHandle::getChild() {
  assert(getState() == STATE_BLOCKED);
  return static_cast<c_WaitableWaitHandle*>(
    m_deps->getValueRef(m_iterPos).asTypedValue()->m_data.pobj);
}

///////////////////////////////////////////////////////////////////////////////
}
