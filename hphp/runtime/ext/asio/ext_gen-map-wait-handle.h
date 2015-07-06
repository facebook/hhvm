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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class GenMapWaitHandle

/**
 * A wait handle that waits for a map of wait handles. The wait handle
 * finishes once all wait handles in the map are finished. The result value
 * preserves the keys of the original map. If one of the wait handles failed,
 * the exception is propagated by failure.
 */
class c_Map;
class c_GenMapWaitHandle final : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(GenMapWaitHandle)

  explicit c_GenMapWaitHandle(Class* cls = c_GenMapWaitHandle::classof())
    : c_WaitableWaitHandle(cls) {}
  ~c_GenMapWaitHandle() {}

  static void ti_setoncreatecallback(const Variant& callback);
  static Object ti_create(const Variant& dependencies);

 public:
  static constexpr ptrdiff_t blockableOff() {
    return offsetof(c_GenMapWaitHandle, m_blockable);
  }

  void onUnblocked();
  String getName();
  c_WaitableWaitHandle* getChild();
  template<typename T> void forEachChild(T fn);

 private:
  void setState(uint8_t state) { setKindState(Kind::GenMap, state); }
  void initialize(const Object& exception, c_Map* deps,
                  ssize_t iter_pos, context_idx_t ctx_idx,
                  c_WaitableWaitHandle* child);

  Object m_exception;
  req::ptr<c_Map> m_deps;
  ssize_t m_iterPos;
  AsioBlockable m_blockable;

 public:
  static const int8_t STATE_BLOCKED = 2;
};

inline c_GenMapWaitHandle* c_WaitHandle::asGenMap() {
  assert(getKind() == Kind::GenMap);
  return static_cast<c_GenMapWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/ext_gen-map-wait-handle-inl.h"

#endif // incl_HPHP_EXT_ASIO_GEN_MAP_WAIT_HANDLE_H_
