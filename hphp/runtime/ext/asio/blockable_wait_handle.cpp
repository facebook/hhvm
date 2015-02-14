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

#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_BlockableWaitHandle::exitContextBlocked(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);
  assert(AsioSession::Get()->getContext(ctx_idx));

  // not in a context being exited
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    return;
  }

  // move us to the parent context
  setContextIdx(getContextIdx() - 1);

  // recursively move all wait handles blocked by us
  getParentChain().exitContext(ctx_idx);
}

///////////////////////////////////////////////////////////////////////////////
}
