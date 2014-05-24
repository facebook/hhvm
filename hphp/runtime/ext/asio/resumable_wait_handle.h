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

#ifndef incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class ResumableWaitHandle

/**
 * A resumable wait handle is a wait handle that represents asynchronous
 * execution of PHP code that can be resumed once the result of awaited
 * WaitHandle becomes available.
 */
FORWARD_DECLARE_CLASS(ResumableWaitHandle)
class c_ResumableWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(ResumableWaitHandle)

  explicit c_ResumableWaitHandle(Class* cls =
      c_ResumableWaitHandle::classof())
    : c_BlockableWaitHandle(cls)
  {}
  ~c_ResumableWaitHandle() {}

 public:
  void resume();
  void exitContext(context_idx_t ctx_idx);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_RESUMABLE_WAIT_HANDLE_H_
