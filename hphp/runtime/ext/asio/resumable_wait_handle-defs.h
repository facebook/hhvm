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

#ifndef incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_DEFS_H_
#define incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_DEFS_H_

#include "hphp/runtime/ext/asio/resumable_wait_handle.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void c_ResumableWaitHandle::resume() {
  switch (getKind()) {
    case Kind::AsyncFunction:
      static_cast<c_AsyncFunctionWaitHandle*>(this)->resume();
      return;
    default:
      not_reached();
  }
}

inline void c_ResumableWaitHandle::exitContext(context_idx_t ctx_idx) {
  switch (getKind()) {
    case Kind::AsyncFunction:
      static_cast<c_AsyncFunctionWaitHandle*>(this)->exitContext(ctx_idx);
      return;
    default:
      not_reached();
  }
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_DEFS_H_
