/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_ASIO_CONTEXT_ENTER_H_
#error "This should only be included by asio-context-enter.h"
#endif

namespace HPHP { namespace asio {
///////////////////////////////////////////////////////////////////////////////

inline void enter_context(c_WaitableWaitHandle* root, context_idx_t ctx_idx) {
  assert(ctx_idx <= AsioSession::Get()->getCurrentContextIdx());

  // If this wait handle is being finished and there is a parent A that is being
  // unblocked and a parent B that was not unblocked yet, it is possible that
  // the parent A triggered an enterContext() that reaches us back thru the
  // parent B. Unfortunately, the condition below is not enough even if parent's
  // context is guaranteed to be equal or smaller. The issue is that a context
  // of a finished wait handle is no longer accessible.
  if (UNLIKELY(root->isFinished())) {
    return;
  }

  // Already in a more specific context?
  if (LIKELY(root->getContextIdx() >= ctx_idx)) {
    return;
  }

  enter_context_impl(root, ctx_idx);
}

///////////////////////////////////////////////////////////////////////////////
}}
