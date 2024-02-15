/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

AsyncGenerator::~AsyncGenerator() {
  if (LIKELY(getState() == State::Done)) {
    return;
  }

  assertx(!isRunning());

  // Free locals, but don't trigger the EventHook for FunctionReturn since
  // the generator has already been exited. We don't want redundant calls.
  ActRec* ar = actRec();
  frame_free_locals_inl_no_hook(ar, ar->func()->numLocals());
}

ObjectData*
AsyncGenerator::Create(const ActRec* fp, size_t numSlots,
                       jit::TCA resumeAddr, Offset suspendOffset) {
  assertx(fp);
  assertx(!isResumed(fp));
  assertx(fp->func()->isAsyncGenerator());
  const size_t frameSz = Resumable::getFrameSize(numSlots);
  const size_t genSz = genSize(sizeof(AsyncGenerator), frameSz);
  auto const obj = BaseGenerator::Alloc<AsyncGenerator>(classof(), genSz);
  auto const genData = new (Native::data<AsyncGenerator>(obj)) AsyncGenerator();
  genData->resumable()->initialize<false>(fp,
                                          resumeAddr,
                                          suspendOffset,
                                          frameSz,
                                          genSz);
  genData->setState(State::Created);
  return obj;
}

c_StaticWaitHandle*
AsyncGenerator::yield(Offset suspendOffset,
                      const TypedValue* key, const TypedValue value) {
  assertx(isRunning());
  resumable()->setResumeAddr(nullptr, suspendOffset);
  setState(State::Started);

  auto keyValueTuple = make_vec_array(
    key ? Variant(tvAsCVarRef(key), Variant::TVCopy()) : init_null_variant,
    Variant(tvAsCVarRef(&value), Variant::TVCopy()));
  auto keyValueTupleTV = make_array_like_tv(keyValueTuple.detach());

  if (m_waitHandle) {
    // Resumed execution.
    req::ptr<c_AsyncGeneratorWaitHandle> wh(std::move(m_waitHandle));
    wh->ret(*tvAssertPlausible(&keyValueTupleTV));
    return nullptr;
  }
  // Eager execution.
  return c_StaticWaitHandle::CreateSucceeded(keyValueTupleTV);
}

c_StaticWaitHandle*
AsyncGenerator::ret() {
  assertx(isRunning());
  setState(State::Done);

  auto nullTV = make_tv<KindOfNull>();

  if (m_waitHandle) {
    // Resumed execution. Take wh out of `this` as ret() may free `this`.
    req::ptr<c_AsyncGeneratorWaitHandle> wh(std::move(m_waitHandle));
    wh->ret(nullTV);
    return nullptr;
  }
  return c_StaticWaitHandle::CreateSucceeded(nullTV);
}

c_StaticWaitHandle*
AsyncGenerator::fail(ObjectData* exception) {
  assertx(isRunning());
  setState(State::Done);

  if (m_waitHandle) {
    // Resumed execution. Take wh out of `this` as fail() may free `this`.
    req::ptr<c_AsyncGeneratorWaitHandle> wh(std::move(m_waitHandle));
    wh->fail(exception);
    return nullptr;
  }
  return c_StaticWaitHandle::CreateFailed(exception);
}

void AsyncGenerator::failCpp() {
  assertx(isRunning());
  setState(State::Done);

  if (m_waitHandle) {
    // Resumed execution. Take wh out of `this` as failCpp() may free `this`.
    req::ptr<c_AsyncGeneratorWaitHandle> wh(std::move(m_waitHandle));
    wh->failCpp();
  }
}
///////////////////////////////////////////////////////////////////////////////

void AsioExtension::registerNativeAsyncGenerator() {
  Native::registerNativeDataInfo<AsyncGenerator>(
    Native::NDIFlags::NO_SWEEP | Native::NDIFlags::NO_COPY |
      Native::NDIFlags::CTOR_THROWS
  );
}

///////////////////////////////////////////////////////////////////////////////
}
