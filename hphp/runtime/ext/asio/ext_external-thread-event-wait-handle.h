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

#ifndef incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class ExternalThreadEventWaitHandle

/**
 * A wait handle that synchronizes against C++ operation in external thread.
 *
 * See asio-external-thread-event.h for more details.
 */
struct AsioExternalThreadEvent;
struct c_ExternalThreadEventWaitHandle final :
    c_WaitableWaitHandle,
    SystemLib::ClassLoader<"HH\\ExternalThreadEventWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\ExternalThreadEventWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\ExternalThreadEventWaitHandle">::className;
  WAITHANDLE_DTOR(ExternalThreadEventWaitHandle);
  void sweep();

  explicit c_ExternalThreadEventWaitHandle()
    : c_WaitableWaitHandle(classof(), HeaderKind::WaitHandle,
        type_scan::getIndexForMalloc<c_ExternalThreadEventWaitHandle>()) {}
  ~c_ExternalThreadEventWaitHandle() {}

 public:
  void initialize(AsioExternalThreadEvent* event, ObjectData* priv_data);

  c_ExternalThreadEventWaitHandle* getNextToProcess() {
    assertx(getState() == STATE_WAITING);
    return m_nextToProcess;
  }
  void setNextToProcess(c_ExternalThreadEventWaitHandle* next) {
    assertx(getState() == STATE_WAITING);
    m_nextToProcess = next;
  }
  ObjectData* getPrivData() { return m_privData.get(); }

  void abandon(bool sweeping);
  bool cancel(const Object& exception);
  void process();
  String getName();
  void exitContext(context_idx_t ctx_idx);
  void registerToContext();
  void unregisterFromContext();

 private:
  void setState(uint8_t s) { setKindState(Kind::ExternalThreadEvent, s); }
  void destroyEvent(bool sweeping = false);

 private:
  // Manipulated by other threads; logically part of the linked list
  // owned by AsioExternalThreadEventQueue::m_received.
  c_ExternalThreadEventWaitHandle* m_nextToProcess;

  // The i/o thread-owned event object, one per ETEWH
  AsioExternalThreadEvent* m_event;

  Object m_privData;

  // Register for sweep, making this ETEWH also a root. AETE's could
  // also be tracked as roots but its more complicated since they
  // are malloc'd and accessed by other threads.
  SweepableMember<c_ExternalThreadEventWaitHandle> m_sweepable;
 public:
  static const uint8_t STATE_WAITING = 2;

  friend struct SweepableMember<c_ExternalThreadEventWaitHandle>;
};

inline c_ExternalThreadEventWaitHandle* c_Awaitable::asExternalThreadEvent() {
  assertx(getKind() == Kind::ExternalThreadEvent);
  return static_cast<c_ExternalThreadEventWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_EXTERNAL_THREAD_EVENT_WAIT_HANDLE_H_
