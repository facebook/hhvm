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

#include "hphp/runtime/ext/asio/async_generator.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/async_generator_wait_handle.h"
#include "hphp/runtime/ext/asio/static_wait_handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void delete_AsyncGenerator(ObjectData* od, const Class*) {
  auto const gen = static_cast<c_AsyncGenerator*>(od);
  auto const size = gen->resumable()->size();
  auto const base = (char*)(gen + 1) - size;
  gen->~c_AsyncGenerator();
  MM().objFreeLogged(base, size);
}

///////////////////////////////////////////////////////////////////////////////

c_AsyncGenerator::~c_AsyncGenerator() {
  if (LIKELY(getState() == State::Done)) {
    return;
  }

  assert(getState() != State::Running);

  // Free locals, but don't trigger the EventHook for FunctionReturn since
  // the generator has already been exited. We don't want redundant calls.
  ActRec* ar = actRec();
  frame_free_locals_inl_no_hook<false>(ar, ar->m_func->numLocals());
}

void c_AsyncGenerator::t___construct() {}

// Functions with native implementation.
void c_AsyncGenerator::t_next() {always_assert(false);}
void c_AsyncGenerator::t_send(const Variant& value) {always_assert(false);}
void c_AsyncGenerator::t_raise(const Object& exception) {always_assert(false);}

c_AsyncGenerator* c_AsyncGenerator::Create(const ActRec* fp,
                                           size_t numSlots,
                                           jit::TCA resumeAddr,
                                           Offset resumeOffset) {
  assert(fp);
  assert(!fp->resumed());
  assert(fp->func()->isAsyncGenerator());
  void* obj = Resumable::Create<false>(fp, numSlots, resumeAddr, resumeOffset,
                                       sizeof(c_AsyncGenerator));
  auto const gen = new (obj) c_AsyncGenerator();
  gen->incRefCount();
  gen->setNoDestruct();
  gen->setState(State::Created);
  gen->m_waitHandle = nullptr;
  return gen;
}

c_AsyncGeneratorWaitHandle*
c_AsyncGenerator::await(Offset resumeOffset, c_WaitableWaitHandle* child) {
  assert(getState() == State::Running);
  resumable()->setResumeAddr(nullptr, resumeOffset);

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->await(child);
    return nullptr;
  } else {
    // Eager executon.
    m_waitHandle = c_AsyncGeneratorWaitHandle::Create(this, child);
    return m_waitHandle;
  }
}

c_StaticWaitHandle*
c_AsyncGenerator::yield(Offset resumeOffset,
                        const Cell* key, const Cell& value) {
  assert(getState() == State::Running);
  resumable()->setResumeAddr(nullptr, resumeOffset);
  setState(State::Started);

  auto keyValueTuple = make_packed_array(
    key ? Variant(tvAsCVarRef(key), Variant::CellCopy()) : init_null_variant,
    Variant(tvAsCVarRef(&value), Variant::CellCopy()));
  auto keyValueTupleTV = make_tv<KindOfArray>(keyValueTuple.detach());

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->ret(*tvAssertCell(&keyValueTupleTV));
    m_waitHandle = nullptr;
    return nullptr;
  } else {
    // Eager execution.
    return c_StaticWaitHandle::CreateSucceeded(keyValueTupleTV);
  }
}

c_StaticWaitHandle*
c_AsyncGenerator::ret() {
  assert(getState() == State::Running);
  setState(State::Done);

  auto nullTV = make_tv<KindOfNull>();

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->ret(nullTV);
    m_waitHandle = nullptr;
    return nullptr;
  } else {
    return c_StaticWaitHandle::CreateSucceeded(nullTV);
  }
}

c_StaticWaitHandle*
c_AsyncGenerator::fail(ObjectData* exception) {
  assert(getState() == State::Running);
  setState(State::Done);

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->fail(exception);
    m_waitHandle = nullptr;
    return nullptr;
  } else {
    return c_StaticWaitHandle::CreateFailed(exception);
  }
}

void c_AsyncGenerator::failCpp() {
  assert(getState() == State::Running);
  setState(State::Done);

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->failCpp();
    m_waitHandle = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
