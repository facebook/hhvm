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

#ifndef incl_HPHP_EXT_ASIO_GEN_MAP_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_GEN_MAP_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class GenMapWaitHandle

/**
 * A wait handle that waits for a map of wait handles. The wait handle
 * finishes once all wait handles in the map are finished. The result value
 * preserves the keys of the original map. If one of the wait handles failed,
 * the exception is propagated by failure.
 */
FORWARD_DECLARE_CLASS(GenMapWaitHandle);
FORWARD_DECLARE_CLASS(Map);
class c_GenMapWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(GenMapWaitHandle)

  explicit c_GenMapWaitHandle(Class* cls = c_GenMapWaitHandle::classof())
    : c_BlockableWaitHandle(cls)
  {}
  ~c_GenMapWaitHandle() {}

  void t___construct();
  static void ti_setoncreatecallback(const Variant& callback);
  static Object ti_create(const Variant& dependencies);

 public:
  String getName();

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();
  void enterContextImpl(context_idx_t ctx_idx);

 private:
  void initialize(const Object& exception, c_Map* deps,
                  ssize_t iter_pos, c_WaitableWaitHandle* child);

  Object m_exception;
  p_Map m_deps;
  ssize_t m_iterPos;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_GEN_MAP_WAIT_HANDLE_H_
