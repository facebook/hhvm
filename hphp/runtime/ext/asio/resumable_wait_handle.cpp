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

#include "hphp/runtime/ext/asio/resumable_wait_handle.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_ResumableWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ResumableWaitHandle::onStart: on_start_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnResumableCreateCallback(callback.getObjectDataOrNull());
}

void c_ResumableWaitHandle::ti_setonawaitcallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ResumableWaitHandle::onAwait: on_await_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnResumableAwaitCallback(callback.getObjectDataOrNull());
}

void c_ResumableWaitHandle::ti_setonsuccesscallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ResumableWaitHandle::onSuccess: on_success_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnResumableSuccessCallback(callback.getObjectDataOrNull());
}

void c_ResumableWaitHandle::ti_setonfailcallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ResumableWaitHandle::onFail: on_fail_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnResumableFailCallback(callback.getObjectDataOrNull());
}

c_ResumableWaitHandle* c_ResumableWaitHandle::getRunning(ActRec* fp) {
  while (fp && !(fp->resumed() && fp->func()->isAsync())) {
    fp = g_context->getPrevVMState(fp);
  }

  if (!fp) {
    return nullptr;
  } else if (fp->func()->isAsyncFunction()) {
    return frame_afwh(fp);
  } else if (fp->func()->isAsyncGenerator()) {
    // not implemented yet
    not_reached();
  } else {
    not_reached();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
