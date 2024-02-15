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

#include "hphp/runtime/ext/asio/ext_asio.h"

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {

c_ResumableWaitHandle* GetResumedWaitHandle() {
  c_ResumableWaitHandle* ret = nullptr;
  walkStack([&] (const BTFrame& frm) {
    if (frm.isRegularResumed() && frm.func()->isAsync()) {
      if (frm.func()->isGenerator()) {
        // async generator
        auto generator = frame_async_generator(frm.fpInternal());
        if (generator->isRunning() && !generator->isEagerlyExecuted()) {
          ret = generator->getWaitHandle();
          return true;
        }
      } else {
        // async function
        ret = frame_afwh(frm.fpInternal());
        return true;
      }
    }
    return false;
  }, true);
  return ret;
}

int64_t HHVM_FUNCTION(asio_get_current_context_idx) {
  return AsioSession::Get()->getCurrentContextIdx();
}

Object HHVM_FUNCTION(asio_get_running_in_context, int64_t ctx_idx) {
  auto session = AsioSession::Get();

  if (ctx_idx <= 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected ctx_idx to be a positive integer");
  }
  if (ctx_idx > session->getCurrentContextIdx()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected ctx_idx to be less than or equal to the current context index");
  }

  if (ctx_idx < session->getCurrentContextIdx()) {
    auto fp = session->getContext(ctx_idx + 1)->getSavedFP();
    return Object{c_ResumableWaitHandle::getRunning(fp)};
  } else {
    return Object{GetResumedWaitHandle()};
  }
}

}

Object HHVM_FUNCTION(asio_get_running) {
  return Object{GetResumedWaitHandle()};
}

Variant HHVM_FUNCTION(join, const Object& obj) {
  if (!obj->instanceof(c_Awaitable::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Joining unsupported for user-land Awaitable");
  }
  auto handle = wait_handle<c_Awaitable>(obj.get());
  if (!handle->isFinished()) {
    // run the full blown machinery
    assertx(handle->instanceof(c_WaitableWaitHandle::classof()));
    static_cast<c_WaitableWaitHandle*>(handle)->join();
    assertx(handle->isFinished());
  }

  if (LIKELY(handle->isSucceeded())) {
    return tvAsCVarRef(handle->getResult());
  } else {
    throw_object(Object{handle->getException()});
  }
}

bool HHVM_FUNCTION(cancel, const Object& obj, const Object& exception) {
  if (!obj->instanceof(c_Awaitable::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Cancellation unsupported for user-land Awaitable");
  }
  auto handle = wait_handle<c_Awaitable>(obj.get());

  switch(handle->getKind()) {
    case c_Awaitable::Kind::ExternalThreadEvent:
      return handle->asExternalThreadEvent()->cancel(exception);
    case c_Awaitable::Kind::Sleep:
      return handle->asSleep()->cancel(exception);
    default:
      SystemLib::throwInvalidArgumentExceptionObject(
        "Cancellation unsupported for " +
        HHVM_MN(Awaitable, getName) (handle)
      );
  }
}

Array HHVM_FUNCTION(backtrace,
                    const Object& obj,
                    int64_t options,
                    int64_t limit) {
  bool provide_object = options & k_DEBUG_BACKTRACE_PROVIDE_OBJECT;
  bool provide_metadata = options & k_DEBUG_BACKTRACE_PROVIDE_METADATA;
  bool ignore_args = options & k_DEBUG_BACKTRACE_IGNORE_ARGS;

  if (!obj->instanceof(c_Awaitable::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Backtrace unsupported for user-land Awaitable");
  }

  // it's not possible to backtrace finished wait handle,
  // because it doesn't keep parent chain
  if (wait_handle<c_Awaitable>(obj.get())->isFinished()) {
    return Array();
  }

  // only descendants of c_WaitableWaitHandle can be in non-finished state
  auto handle = wait_handle<c_WaitableWaitHandle>(obj.get());

  return createBacktrace(BacktraceArgs()
                         .fromWaitHandle(handle)
                         .withSelf()
                         .withThis(provide_object)
                         .withMetadata(provide_metadata)
                         .ignoreArgs(ignore_args)
                         .setLimit(limit));
}

static AsioExtension s_asio_extension;

void AsioExtension::moduleInit() {
  initNativeWaitHandle();
}

void AsioExtension::moduleRegisterNative() {
  registerNativeFunctions();

  registerNativeWaitHandle();
  registerNativeResumableWaitHandle();
  registerNativeAsyncGenerator();
  registerNativeAwaitAllWaitHandle();
  registerNativeConcurrentWaitHandle();
  registerNativeConditionWaitHandle();
  registerNativeSleepWaitHandle();
  registerNativeRescheduleWaitHandle();
  registerNativeExternalThreadEventWaitHandle();
  registerNativeStaticWaitHandle();
  registerNativeAsyncFunctionWaitHandle();
  registerNativeAsyncGeneratorWaitHandle();
}

std::vector<std::string> AsioExtension::hackFiles() const {
  return {"async-generator", "asio"};
}

void AsioExtension::requestInit() { requestInitSingletons(); }

void AsioExtension::registerNativeFunctions() {
  HHVM_FALIAS(
    HH\\asio_get_current_context_idx,
    asio_get_current_context_idx);
  HHVM_FALIAS(HH\\asio_get_running_in_context, asio_get_running_in_context);
  HHVM_FALIAS(HH\\asio_get_running, asio_get_running);
  HHVM_FALIAS(HH\\Asio\\join, join);
  HHVM_FALIAS(HH\\Asio\\cancel, cancel);
  HHVM_FALIAS(HH\\Asio\\backtrace, backtrace);
}

///////////////////////////////////////////////////////////////////////////////
}
