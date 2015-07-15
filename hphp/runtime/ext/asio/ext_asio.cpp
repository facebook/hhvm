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

#include "hphp/runtime/ext/asio/ext_asio.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {

int64_t HHVM_FUNCTION(asio_get_current_context_idx) {
  return AsioSession::Get()->getCurrentContextIdx();
}

Object HHVM_FUNCTION(asio_get_running_in_context, int ctx_idx) {
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
    return c_ResumableWaitHandle::getRunning(fp);
  } else {
    VMRegAnchor _;
    return c_ResumableWaitHandle::getRunning(vmfp());
  }
}

}

Object HHVM_FUNCTION(asio_get_running) {
  VMRegAnchor _;
  return c_ResumableWaitHandle::getRunning(vmfp());
}

static AsioExtension s_asio_extension;

void AsioExtension::initFunctions() {
  HHVM_FALIAS(
    HH\\asio_get_current_context_idx,
    asio_get_current_context_idx);
  HHVM_FALIAS(HH\\asio_get_running_in_context, asio_get_running_in_context);
  HHVM_FALIAS(HH\\asio_get_running, asio_get_running);

  loadSystemlib();
}

///////////////////////////////////////////////////////////////////////////////
}
