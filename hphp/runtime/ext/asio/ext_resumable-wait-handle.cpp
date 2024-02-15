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

#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"

#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void HHVM_STATIC_METHOD(ResumableWaitHandle, setOnCreateCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnResumableCreate(callback);
}

void HHVM_STATIC_METHOD(ResumableWaitHandle, setOnAwaitCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnResumableAwait(callback);
}

void HHVM_STATIC_METHOD(ResumableWaitHandle, setOnSuccessCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnResumableSuccess(callback);
}

void HHVM_STATIC_METHOD(ResumableWaitHandle, setOnFailCallback,
                        const Variant& callback) {
  AsioSession::Get()->setOnResumableFail(callback);
}

c_ResumableWaitHandle* c_ResumableWaitHandle::getRunning(ActRec* fp) {
  for (; fp; fp = g_context->getPrevVMState(fp)) {
    if (isResumed(fp) && fp->func()->isAsync()) {
      if (fp->func()->isGenerator()) {
        // async generator
        auto generator = frame_async_generator(fp);
        if (!generator->isEagerlyExecuted()) {
          return generator->getWaitHandle();
        }
      } else {
        // async function
        return frame_afwh(fp);
      }
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void AsioExtension::registerNativeResumableWaitHandle() {
#define RWH_SME(meth) \
  HHVM_STATIC_MALIAS(HH\\ResumableWaitHandle, meth, ResumableWaitHandle, meth)
  RWH_SME(setOnCreateCallback);
  RWH_SME(setOnAwaitCallback);
  RWH_SME(setOnSuccessCallback);
  RWH_SME(setOnFailCallback);
#undef RWH_SME

  Native::registerClassExtraDataHandler(
    c_ResumableWaitHandle::className(), finish_class<c_ResumableWaitHandle>);
}

///////////////////////////////////////////////////////////////////////////////
}
