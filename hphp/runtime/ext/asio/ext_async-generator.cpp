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

#include "hphp/runtime/ext/asio/ext_async-generator.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/static-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void delete_AsyncGenerator(ObjectData* od, const Class*) {
  auto gen = static_cast<c_AsyncGenerator*>(od);
  Resumable::Destroy(gen->data()->resumable()->size(), gen);
}

///////////////////////////////////////////////////////////////////////////////

AsyncGeneratorData::~AsyncGeneratorData() {
  if (LIKELY(getState() == State::Done)) {
    return;
  }

  assert(getState() != State::Running);

  // Free locals, but don't trigger the EventHook for FunctionReturn since
  // the generator has already been exited. We don't want redundant calls.
  ActRec* ar = actRec();
  frame_free_locals_inl_no_hook<false>(ar, ar->m_func->numLocals());
}

ObjectData*
AsyncGeneratorData::Create(const ActRec* fp, size_t numSlots,
                           jit::TCA resumeAddr, Offset resumeOffset) {
  assert(fp);
  assert(!fp->resumed());
  assert(fp->func()->isAsyncGenerator());
  void* genDataPtr = Resumable::Create<false,
                       sizeof(AsyncGeneratorData) + sizeof(c_AsyncGenerator)>(
                       fp, numSlots, resumeAddr, resumeOffset);
  AsyncGeneratorData* genData = new (genDataPtr) AsyncGeneratorData();
  auto const gen = new (genData + 1) c_AsyncGenerator();
  assert(gen->hasExactlyOneRef());
  assert(gen->noDestruct());
  genData->setState(State::Created);
  genData->m_waitHandle = nullptr;
  return static_cast<ObjectData*>(gen);
}

c_AsyncGeneratorWaitHandle*
AsyncGeneratorData::await(Offset resumeOffset, c_WaitableWaitHandle* child) {
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
AsyncGeneratorData::yield(Offset resumeOffset,
                        const Cell* key, const Cell value) {
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
AsyncGeneratorData::ret() {
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
AsyncGeneratorData::fail(ObjectData* exception) {
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

void AsyncGeneratorData::failCpp() {
  assert(getState() == State::Running);
  setState(State::Done);

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->failCpp();
    m_waitHandle = nullptr;
  }
}

void c_AsyncGenerator::t___construct() {}

// Functions with native implementation.
void c_AsyncGenerator::t_next() {always_assert(false);}
void c_AsyncGenerator::t_send(const Variant& value) {always_assert(false);}
void c_AsyncGenerator::t_raise(const Object& exception) {always_assert(false);}

///////////////////////////////////////////////////////////////////////////////
}
