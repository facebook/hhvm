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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_
#define incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_

#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
struct c_AsyncGeneratorWaitHandle;
struct c_StaticWaitHandle;
struct c_WaitableWaitHandle;
struct c_Awaitable;

///////////////////////////////////////////////////////////////////////////////
// class AsyncGenerator

struct AsyncGenerator final : BaseGenerator,
                              SystemLib::ClassLoader<"HH\\AsyncGenerator"> {
   AsyncGenerator() : m_waitHandle() {}
  ~AsyncGenerator();

  static ObjectData* Create(const ActRec* fp, size_t numSlots,
                            jit::TCA resumeAddr, Offset suspendOffset);
  static constexpr ptrdiff_t objectOff() {
    return -(Native::dataOffset<AsyncGenerator>());
  }
  static constexpr ptrdiff_t waitHandleOff() {
    return offsetof(AsyncGenerator, m_waitHandle);
  }

  static AsyncGenerator* fromObject(ObjectData *obj) {
    return Native::data<AsyncGenerator>(obj);
  }

  c_StaticWaitHandle* yield(Offset suspendOffset,
                            const TypedValue* key, TypedValue value);
  c_StaticWaitHandle* ret();
  c_StaticWaitHandle* fail(ObjectData* exception);
  void failCpp();

  ObjectData* toObject() {
    return Native::object<AsyncGenerator>(this);
  }

  bool isEagerlyExecuted() const {
    assertx(isRunning());
    return !m_waitHandle;
  }

  c_AsyncGeneratorWaitHandle* getWaitHandle() const {
    assertx(!isEagerlyExecuted());
    return m_waitHandle.get();
  }

  req::ptr<c_AsyncGeneratorWaitHandle> detachWaitHandle() {
    return std::move(m_waitHandle);
  }

  void attachWaitHandle(req::ptr<c_AsyncGeneratorWaitHandle>&& waitHandle) {
    assertx(isEagerlyExecuted());
    m_waitHandle = std::move(waitHandle);
  }

private:
  // valid only in Running state; null during eager execution
  req::ptr<c_AsyncGeneratorWaitHandle> m_waitHandle;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_H_
