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

#ifndef incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class ExternalThreadEventWaitHandle

/**
 * A wait handle that synchronizes against C++ operation in external thread.
 *
 * See asio_external_thread_event.h for more details.
 */
class AsioExternalThreadEvent;
FORWARD_DECLARE_CLASS(ExternalThreadEventWaitHandle);
class c_ExternalThreadEventWaitHandle
  : public c_WaitableWaitHandle, public Sweepable {
 public:
  DECLARE_CLASS(ExternalThreadEventWaitHandle)

  explicit c_ExternalThreadEventWaitHandle(Class* cls =
      c_ExternalThreadEventWaitHandle::classof())
    : c_WaitableWaitHandle(cls)
  {}
  ~c_ExternalThreadEventWaitHandle() {}

  void t___construct();

 public:
  static c_ExternalThreadEventWaitHandle* Create(AsioExternalThreadEvent* event,
                                                 ObjectData* priv_data);

  c_ExternalThreadEventWaitHandle* getNextToProcess() {
    assert(getState() == STATE_WAITING);
    return m_nextToProcess;
  }
  void setNextToProcess(c_ExternalThreadEventWaitHandle* next) {
    assert(getState() == STATE_WAITING);
    m_nextToProcess = next;
  }
  ObjectData* getPrivData() { return m_privData.get(); }
  void setIndex(uint32_t ete_idx) {
    assert(getState() == STATE_WAITING);
    m_index = ete_idx;
  }

  void abandon(bool sweeping);
  void process();
  String getName();
  void enterContextImpl(context_idx_t ctx_idx);
  void exitContext(context_idx_t ctx_idx);

 private:
  void setState(uint8_t s) { setKindState(Kind::ExternalThreadEvent, s); }
  void initialize(AsioExternalThreadEvent* event, ObjectData* priv_data);
  void destroyEvent(bool sweeping = false);
  void registerToContext();
  void unregisterFromContext();

  c_ExternalThreadEventWaitHandle* m_nextToProcess;
  AsioExternalThreadEvent* m_event;
  Object m_privData;
  uint32_t m_index;

  static const uint8_t STATE_WAITING = 2;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_WAIT_HANDLE_H_
