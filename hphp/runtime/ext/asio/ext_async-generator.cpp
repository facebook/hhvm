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
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

Class* AsyncGenerator::s_class = nullptr;
const StaticString AsyncGenerator::s_className("HH\\AsyncGenerator");

AsyncGenerator::~AsyncGenerator() {
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
AsyncGenerator::Create(const ActRec* fp, size_t numSlots,
                       jit::TCA resumeAddr, Offset resumeOffset) {
  assert(fp);
  assert(!fp->resumed());
  assert(fp->func()->isAsyncGenerator());
  const size_t frameSz = Resumable::getFrameSize(numSlots);
  const size_t genSz = genSize(sizeof(AsyncGenerator), frameSz);
  auto const obj = BaseGenerator::Alloc(s_class, genSz);
  auto const genData = new (Native::data<AsyncGenerator>(obj)) AsyncGenerator();
  genData->resumable()->initialize<false>(fp,
                                          resumeAddr,
                                          resumeOffset,
                                          frameSz,
                                          genSz);
  genData->setState(State::Created);
  return obj;
}

c_AsyncGeneratorWaitHandle*
AsyncGenerator::await(Offset resumeOffset, c_WaitableWaitHandle* child) {
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
AsyncGenerator::yield(Offset resumeOffset,
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
AsyncGenerator::ret() {
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
AsyncGenerator::fail(ObjectData* exception) {
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

void AsyncGenerator::failCpp() {
  assert(getState() == State::Running);
  setState(State::Done);

  if (m_waitHandle) {
    // Resumed execution.
    m_waitHandle->failCpp();
    m_waitHandle = nullptr;
  }
}
///////////////////////////////////////////////////////////////////////////////

void AsioExtension::initAsyncGenerator() {
  Native::registerNativeDataInfo<AsyncGenerator>(
    AsyncGenerator::s_className.get(),
    Native::NDIFlags::NO_SWEEP | Native::NDIFlags::NO_COPY);
  loadSystemlib("async-generator");
  AsyncGenerator::s_class =
    Unit::lookupClass(AsyncGenerator::s_className.get());
  assert(AsyncGenerator::s_class);
}

///////////////////////////////////////////////////////////////////////////////
}
