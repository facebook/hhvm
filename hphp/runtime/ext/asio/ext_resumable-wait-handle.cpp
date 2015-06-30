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

#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_ResumableWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnResumableCreate(callback);
}

void c_ResumableWaitHandle::ti_setonawaitcallback(const Variant& callback) {
  AsioSession::Get()->setOnResumableAwait(callback);
}

void c_ResumableWaitHandle::ti_setonsuccesscallback(const Variant& callback) {
  AsioSession::Get()->setOnResumableSuccess(callback);
}

void c_ResumableWaitHandle::ti_setonfailcallback(const Variant& callback) {
  AsioSession::Get()->setOnResumableFail(callback);
}

c_ResumableWaitHandle* c_ResumableWaitHandle::getRunning(ActRec* fp) {
  for (; fp; fp = g_context->getPrevVMState(fp)) {
    if (fp->resumed() && fp->func()->isAsync()) {
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
}
