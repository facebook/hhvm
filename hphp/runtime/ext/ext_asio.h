/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_ASIO_H_
#define incl_HPHP_EXT_ASIO_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/asio_session.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int f_asio_get_current_context_idx();
Object f_asio_get_running_in_context(int ctx_idx);
Object f_asio_get_running();
void f_asio_set_on_failed_callback(CVarRef on_failed_cb);
void f_asio_set_on_started_callback(CVarRef on_started_cb);

///////////////////////////////////////////////////////////////////////////////
// class WaitHandle

/**
 * A wait handle is an object that describes operation that is potentially
 * asynchronous. A WaitHandle class is a base class of all such objects. There
 * are multiple types of wait handles, this is their hierarchy:
 *
 * WaitHandle                     - abstract wait handle
 *   StaticWaitHandle             - statically finished wait handle
 *     StaticResultWaitHandle     - statically succeeded wait handle with result
 *     StaticExceptionWaitHandle  - statically failed wait handle with exception
 *   WaitableWaitHandle           - wait handle that can be waited for
 *     BlockableWaitHandle        - wait handle that can be blocked by other WH
 *       ContinuationWaitHandle   - Continuation-powered asynchronous execution
 *       GenArrayWaitHandle       - wait handle representing an array of WHs
 *       GenMapWaitHandle         - wait handle representing an Map of WHs
 *       GenVectorWaitHandle      - wait handle representing an Vector of WHs
 *       SetResultToRefWaitHandle - wait handle that sets result to reference
 *     RescheduleWaitHandle       - wait handle that reschedules execution
 *
 * A wait handle can be either synchronously joined (waited for the operation
 * to finish) or passed in various contexts as a dependency and waited for
 * asynchronously (such as using yield mechanism of ContinuationWaitHandle or
 * passed as an array member of GenArrayWaitHandle).
 */
FORWARD_DECLARE_CLASS_BUILTIN(WaitHandle);
class c_WaitHandle : public ExtObjectData {
 public:
  DECLARE_CLASS(WaitHandle, WaitHandle, ObjectData)

  // need to implement
  public: c_WaitHandle(Class* cls = c_WaitHandle::s_cls);
  public: ~c_WaitHandle();
  public: void t___construct();
  public: static void ti_setonjoincallback(CVarRef callback);
  public: Object t_getwaithandle();
  public: void t_import();
  public: Variant t_join();
  public: bool t_isfinished();
  public: bool t_issucceeded();
  public: bool t_isfailed();
  public: int64_t t_getid();
  public: String t_getname();
  public: Object t_getexceptioniffailed();


 public:
  static c_WaitHandle* fromCell(Cell* cell) {
    return (
        cell->m_type == KindOfObject &&
        cell->m_data.pobj->instanceof(s_cls)
      ) ? static_cast<c_WaitHandle*>(cell->m_data.pobj) : nullptr;
  }
  bool isFinished() { return getState() <= STATE_FAILED; }
  bool isSucceeded() { return getState() == STATE_SUCCEEDED; }
  bool isFailed() { return getState() == STATE_FAILED; }
  Cell& getResult() {
    assert(isSucceeded());
    return m_resultOrException;
  }
  ObjectData* getException() {
    assert(isFailed());
    return m_resultOrException.m_data.pobj;
  }
  virtual String getName() = 0;

 protected:
  uint8_t getState() { return o_subclassData.u8[0]; }
  void setState(uint8_t state) { o_subclassData.u8[0] = state; }

  static const int8_t STATE_SUCCEEDED = 0;
  static const int8_t STATE_FAILED    = 1;

  Cell m_resultOrException;
};

///////////////////////////////////////////////////////////////////////////////
// class StaticWaitHandle

/**
 * A static wait handle is a wait handle that is statically finished. The result
 * of the operation is always available and waiting for the wait handle finishes
 * immediately.
 */
FORWARD_DECLARE_CLASS_BUILTIN(StaticWaitHandle);
class c_StaticWaitHandle : public c_WaitHandle {
 public:
  DECLARE_CLASS(StaticWaitHandle, StaticWaitHandle, WaitHandle)

  // need to implement
  public: c_StaticWaitHandle(Class* cls = c_StaticWaitHandle::s_cls);
  public: ~c_StaticWaitHandle();
  public: void t___construct();

};

///////////////////////////////////////////////////////////////////////////////
// class StaticResultWaitHandle

/**
 * A wait handle that is statically succeeded with a result.
 */
FORWARD_DECLARE_CLASS_BUILTIN(StaticResultWaitHandle);
class c_StaticResultWaitHandle : public c_StaticWaitHandle {
 public:
  DECLARE_CLASS(StaticResultWaitHandle, StaticResultWaitHandle, StaticWaitHandle)

  // need to implement
  public: c_StaticResultWaitHandle(Class* cls = c_StaticResultWaitHandle::s_cls);
  public: ~c_StaticResultWaitHandle();
  public: void t___construct();
  public: static Object ti_create(CVarRef result);

 public:
  static p_StaticResultWaitHandle Create(const Cell& result);
  String getName();
};

///////////////////////////////////////////////////////////////////////////////
// class StaticExceptionWaitHandle

/**
 * A wait handle that is statically failed with an exception.
 */
FORWARD_DECLARE_CLASS_BUILTIN(StaticExceptionWaitHandle);
class c_StaticExceptionWaitHandle : public c_StaticWaitHandle {
 public:
  DECLARE_CLASS(StaticExceptionWaitHandle, StaticExceptionWaitHandle, StaticWaitHandle)

  // need to implement
  public: c_StaticExceptionWaitHandle(Class* cls = c_StaticExceptionWaitHandle::s_cls);
  public: ~c_StaticExceptionWaitHandle();
  public: void t___construct();
  public: static Object ti_create(CObjRef exception);

 public:
  static p_StaticExceptionWaitHandle Create(ObjectData* exception);
  String getName();
};

///////////////////////////////////////////////////////////////////////////////
// class WaitableWaitHandle

/**
 * A waitable wait handle is a wait handle that can be waited for by a blockable
 * wait handle if a result is not yet available. Once the wait handle finishes,
 * all blocked wait handles are notified.
 */
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(BlockableWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(WaitableWaitHandle);
class c_WaitableWaitHandle : public c_WaitHandle {
 public:
  DECLARE_CLASS(WaitableWaitHandle, WaitableWaitHandle, WaitHandle)

  // need to implement
  public: c_WaitableWaitHandle(Class* cls = c_WaitableWaitHandle::s_cls);
  public: ~c_WaitableWaitHandle();
  public: void t___construct();
  public: int t_getcontextidx();
  public: Object t_getcreator();
  public: Array t_getparents();
  public: Array t_getdependencystack();


 public:
  AsioContext* getContext() {
    assert(isInContext());
    return AsioSession::Get()->getContext(getContextIdx());
  }

  c_BlockableWaitHandle* addParent(c_BlockableWaitHandle* parent);

  virtual void enterContext(context_idx_t ctx_idx) = 0;
  void join();

 protected:
  void setResult(const Cell& result);
  void setException(ObjectData* exception);

  context_idx_t getContextIdx() { return o_subclassData.u8[1]; }
  void setContextIdx(context_idx_t ctx_idx) { o_subclassData.u8[1] = ctx_idx; }

  bool isInContext() { return getContextIdx(); }

  c_BlockableWaitHandle* getFirstParent() { return m_firstParent; }

  virtual c_WaitableWaitHandle* getChild();
  bool hasCycle(c_WaitableWaitHandle* start);

  static const int8_t STATE_NEW       = 2;

 private:
  c_ContinuationWaitHandle* m_creator;
  c_BlockableWaitHandle* m_firstParent;
};

///////////////////////////////////////////////////////////////////////////////
// class BlockableWaitHandle

/**
 * A blockable wait handle is a wait handle that can be blocked by a waitable
 * wait handle it is waiting for. Once a wait handle blocking this wait handle
 * is finished, a notification is received and the operation can be resumed.
 */
FORWARD_DECLARE_CLASS_BUILTIN(BlockableWaitHandle);
class c_BlockableWaitHandle : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS(BlockableWaitHandle, BlockableWaitHandle, WaitableWaitHandle)

  // need to implement
  public: c_BlockableWaitHandle(Class* cls = c_BlockableWaitHandle::s_cls);
  public: ~c_BlockableWaitHandle();
  public: void t___construct();


 public:
  c_BlockableWaitHandle* getNextParent();
  c_BlockableWaitHandle* unblock();

  void exitContextBlocked(context_idx_t ctx_idx);

 protected:
  void blockOn(c_WaitableWaitHandle* child);
  virtual void onUnblocked() = 0;
  c_WaitableWaitHandle* getChild() = 0;

  static const int8_t STATE_BLOCKED = 3;

 private:
  void reportCycle(c_WaitableWaitHandle* start);

  c_BlockableWaitHandle* m_nextParent;
};

///////////////////////////////////////////////////////////////////////////////
// class ContinuationWaitHandle

/**
 * A continuation wait handle represents a basic unit of asynchronous execution
 * powered by continuation object. An asynchronous program can be written using
 * continuations; a dependency on another wait handle is set up by yielding such
 * wait handle, giving control of the execution back to the asio framework.
 */
FORWARD_DECLARE_CLASS_BUILTIN(Continuation);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
class c_ContinuationWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(ContinuationWaitHandle, ContinuationWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_ContinuationWaitHandle(Class* cls = c_ContinuationWaitHandle::s_cls);
  public: ~c_ContinuationWaitHandle();
  public: void t___construct();
  public: static void ti_setoncreatecallback(CVarRef callback);
  public: static void ti_setonyieldcallback(CVarRef callback);
  public: static void ti_setonsuccesscallback(CVarRef callback);
  public: static void ti_setonfailcallback(CVarRef callback);
  public: Object t_getprivdata();
  public: void t_setprivdata(CObjRef data);

 public:
  static void Create(c_Continuation* continuation);
  void run();
  uint16_t getDepth() { return m_depth; }
  String getName();
  void enterContext(context_idx_t ctx_idx);
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }
  String getFileName();
  int getLineNumber();

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

 private:
  void initialize(c_Continuation* continuation, uint16_t depth);
  void markAsSucceeded(const Cell& result);
  void markAsFailed(CObjRef exception);

  p_Continuation m_continuation;
  p_WaitHandle m_child;
  Object m_privData;
  uint16_t m_depth;

  static const int8_t STATE_SCHEDULED = 4;
  static const int8_t STATE_RUNNING   = 5;
};

///////////////////////////////////////////////////////////////////////////////
// class GenArrayWaitHandle

/**
 * A wait handle that waits for an array of wait handles. The wait handle
 * finishes once all wait handles in the array are finished. The result value
 * preserves structure (order and keys) of the original array. If one of the
 * wait handles failed, the exception is propagated by failure.
 */
FORWARD_DECLARE_CLASS_BUILTIN(GenArrayWaitHandle);
class c_GenArrayWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(GenArrayWaitHandle, GenArrayWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_GenArrayWaitHandle(Class* cls = c_GenArrayWaitHandle::s_cls);
  public: ~c_GenArrayWaitHandle();
  public: void t___construct();
  public: static void ti_setoncreatecallback(CVarRef callback);
  public: static Object ti_create(CArrRef dependencies);


 public:
  String getName();
  void enterContext(context_idx_t ctx_idx);

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

 private:
  void initialize(CObjRef exception, CArrRef deps,
                  ssize_t iter_pos, c_WaitableWaitHandle* child);

  Object m_exception;
  Array m_deps;
  ssize_t m_iterPos;
};

///////////////////////////////////////////////////////////////////////////////
// class GenMapWaitHandle

/**
 * A wait handle that waits for a map of wait handles. The wait handle
 * finishes once all wait handles in the map are finished. The result value
 * preserves the keys of the original map. If one of the wait handles failed,
 * the exception is propagated by failure.
 */
FORWARD_DECLARE_CLASS_BUILTIN(GenMapWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(Map);
class c_GenMapWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(GenMapWaitHandle, GenMapWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_GenMapWaitHandle(Class* cls = c_GenMapWaitHandle::s_cls);
  public: ~c_GenMapWaitHandle();
  public: void t___construct();
  public: static void ti_setoncreatecallback(CVarRef callback);
  public: static Object ti_create(CVarRef dependencies);

 public:
  String getName();
  void enterContext(context_idx_t ctx_idx);

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

 private:
  void initialize(CObjRef exception, c_Map* deps,
                  ssize_t iter_pos, c_WaitableWaitHandle* child);

  Object m_exception;
  p_Map m_deps;
  ssize_t m_iterPos;
};

///////////////////////////////////////////////////////////////////////////////
// class GenVectorWaitHandle

/**
 * A wait handle that waits for a vector of wait handles. The wait handle
 * finishes once all wait handles in the vector are finished. The result value
 * preserves order of the original vector. If one of the wait handles failed,
 * the exception is propagated by failure.
 */
FORWARD_DECLARE_CLASS_BUILTIN(GenVectorWaitHandle);
FORWARD_DECLARE_CLASS_BUILTIN(Vector);
class c_GenVectorWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(GenVectorWaitHandle, GenVectorWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_GenVectorWaitHandle(Class* cls = c_GenVectorWaitHandle::s_cls);
  public: ~c_GenVectorWaitHandle();
  public: void t___construct();
  public: static void ti_setoncreatecallback(CVarRef callback);
  public: static Object ti_create(CVarRef dependencies);

 public:
  String getName();
  void enterContext(context_idx_t ctx_idx);

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

 private:
  void initialize(CObjRef exception, c_Vector* deps,
                  int64_t iter_pos, c_WaitableWaitHandle* child);

  Object m_exception;
  p_Vector m_deps;
  int64_t m_iterPos;
};

///////////////////////////////////////////////////////////////////////////////
// class SetResultToRefWaitHandle

/**
 * A wait handle that waits for a given dependency and sets its result to
 * a given reference once completed.
 */
FORWARD_DECLARE_CLASS_BUILTIN(SetResultToRefWaitHandle);
class c_SetResultToRefWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(SetResultToRefWaitHandle, SetResultToRefWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_SetResultToRefWaitHandle(Class* cls = c_SetResultToRefWaitHandle::s_cls);
  public: ~c_SetResultToRefWaitHandle();
  public: void t___construct();
  public: static void ti_setoncreatecallback(CVarRef callback);
  public: static Object ti_create(CObjRef wait_handle, VRefParam ref);


 public:
  String getName();
  void enterContext(context_idx_t ctx_idx);

 protected:
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

 private:
  void initialize(c_WaitableWaitHandle* wait_handle, RefData* ref);
  void markAsSucceeded(const Cell& result);
  void markAsFailed(CObjRef exception);

  p_WaitableWaitHandle m_child;
  RefData* m_ref;
};

///////////////////////////////////////////////////////////////////////////////
// class RescheduleWaitHandle

extern const int q_RescheduleWaitHandle$$QUEUE_DEFAULT;
extern const int q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO;

/**
 * A wait handle that is enqueued into a given priority queue and once desired
 * execution priority is eligible for execution, it succeeds with a null result.
 *
 * RescheduleWaitHandle is guaranteed to never finish immediately.
 */
FORWARD_DECLARE_CLASS_BUILTIN(RescheduleWaitHandle);
class c_RescheduleWaitHandle : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS(RescheduleWaitHandle, RescheduleWaitHandle, WaitableWaitHandle)

  // need to implement
  public: c_RescheduleWaitHandle(Class* cls = c_RescheduleWaitHandle::s_cls);
  public: ~c_RescheduleWaitHandle();
  public: void t___construct();
  public: static Object ti_create(int queue, int priority);

 public:
  void run();
  String getName();
  void enterContext(context_idx_t ctx_idx);
  void exitContext(context_idx_t ctx_idx);

 private:
  void initialize(uint32_t queue, uint32_t priority);

  uint32_t m_queue;
  uint32_t m_priority;

  static const int8_t STATE_SCHEDULED = 3;
};

///////////////////////////////////////////////////////////////////////////////
// class ExternalThreadEventWaitHandle

/**
 * A wait handle that synchronizes against C++ operation in external thread.
 *
 * See asio_external_thread_event.h for more details.
 */
class AsioExternalThreadEvent;
FORWARD_DECLARE_CLASS_BUILTIN(ExternalThreadEventWaitHandle);
class c_ExternalThreadEventWaitHandle : public c_WaitableWaitHandle, public Sweepable {
 public:
  DECLARE_CLASS(ExternalThreadEventWaitHandle, ExternalThreadEventWaitHandle, WaitableWaitHandle)

  // need to implement
  public: c_ExternalThreadEventWaitHandle(Class* cls = c_ExternalThreadEventWaitHandle::s_cls);
  public: ~c_ExternalThreadEventWaitHandle();
  public: void t___construct();

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
  void enterContext(context_idx_t ctx_idx);
  void exitContext(context_idx_t ctx_idx);

 private:
  void initialize(AsioExternalThreadEvent* event, ObjectData* priv_data);
  void destroyEvent();

  c_ExternalThreadEventWaitHandle* m_nextToProcess;
  AsioExternalThreadEvent* m_event;
  Object m_privData;
  uint32_t m_index;

  static const uint8_t STATE_WAITING  = 3;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_H_
