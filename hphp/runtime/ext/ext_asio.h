/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_ASIO_H__
#define __EXT_ASIO_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void f_asio_enter_context();
void f_asio_exit_context();
Object f_asio_get_current();
void f_asio_set_on_failed_callback(CObjRef on_failed_cb);

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
 *       SetResultToRefWaitHandle - wait handle that sets result to reference
 *
 * A wait handle can be either synchronously joined (waited for the operation
 * to finish) or passed in various contexts as a dependency and waited for
 * asynchronously (such as using yield mechanism of ContinuationWaitHandle or
 * passed as an array member of GenArrayWaitHandle).
 */
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(WaitHandle);
class c_WaitHandle : public ExtObjectData {
 public:
  DECLARE_CLASS(WaitHandle, WaitHandle, ObjectData)

  // need to implement
  public: c_WaitHandle(const ObjectStaticCallbacks *cb = &cw_WaitHandle);
  public: ~c_WaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: void t_import();
  DECLARE_METHOD_INVOKE_HELPERS(import);
  public: Variant t_join();
  DECLARE_METHOD_INVOKE_HELPERS(join);
  public: int64 t_getid();
  DECLARE_METHOD_INVOKE_HELPERS(getid);
  public: String t_getname();
  DECLARE_METHOD_INVOKE_HELPERS(getname);
  public: Object t_getexceptioniffailed();
  DECLARE_METHOD_INVOKE_HELPERS(getexceptioniffailed);

  // implemented by HPHP
  public: c_WaitHandle *create();

 public:
  virtual bool isFinished();
  virtual bool isSucceeded();
  virtual bool isFailed();
  virtual TypedValue* getResult();
  virtual ObjectData* getException();
  virtual String getName();
  virtual void enterContext(AsioContext* ctx);

 protected:
  virtual const TypedValue* join();
};

///////////////////////////////////////////////////////////////////////////////
// class StaticWaitHandle

/**
 * A static wait handle is a wait handle that is statically finished. The result
 * of the operation is always available and waiting for the wait handle finishes
 * immediately.
 */
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(StaticWaitHandle);
class c_StaticWaitHandle : public c_WaitHandle {
 public:
  DECLARE_CLASS(StaticWaitHandle, StaticWaitHandle, WaitHandle)

  // need to implement
  public: c_StaticWaitHandle(const ObjectStaticCallbacks *cb = &cw_StaticWaitHandle);
  public: ~c_StaticWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);

  // implemented by HPHP
  public: c_StaticWaitHandle *create();

 public:
  inline bool isFinished() { return true; }
  void enterContext(AsioContext* ctx);
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
  public: c_StaticResultWaitHandle(const ObjectStaticCallbacks *cb = &cw_StaticResultWaitHandle);
  public: ~c_StaticResultWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_create(const char* cls , CVarRef result);
  public: static Object t_create(CVarRef result) {
    return ti_create("staticresultwaithandle", result);
  }
  DECLARE_METHOD_INVOKE_HELPERS(create);

  // implemented by HPHP
  public: c_StaticResultWaitHandle *create();

 public:
  inline bool isSucceeded() { return true; }
  inline bool isFailed() { return false; }
  inline TypedValue* getResult() { return &m_result; }
  inline ObjectData* getException() {
    throw FatalErrorException(
        "Invariant violation: static result does not have exception");
  }
  String getName();

 protected:
  const TypedValue* join();

 private:
  TypedValue m_result;
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
  public: c_StaticExceptionWaitHandle(const ObjectStaticCallbacks *cb = &cw_StaticExceptionWaitHandle);
  public: ~c_StaticExceptionWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_create(const char* cls , CObjRef exception);
  public: static Object t_create(CObjRef exception) {
    return ti_create("staticexceptionwaithandle", exception);
  }
  DECLARE_METHOD_INVOKE_HELPERS(create);

  // implemented by HPHP
  public: c_StaticExceptionWaitHandle *create();

 public:
  inline bool isSucceeded() { return false; }
  inline bool isFailed() { return true; }
  inline TypedValue* getResult() {
    throw FatalErrorException(
        "Invariant violation: static exception does not have result");
  }
  inline ObjectData* getException() { return m_exception.get(); }
  String getName();

 protected:
  const TypedValue* join();

 private:
  Object m_exception;
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
  public: c_WaitableWaitHandle(const ObjectStaticCallbacks *cb = &cw_WaitableWaitHandle);
  public: ~c_WaitableWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: Array t_getparents();
  DECLARE_METHOD_INVOKE_HELPERS(getparents);
  public: Array t_getstacktrace();
  DECLARE_METHOD_INVOKE_HELPERS(getstacktrace);

  // implemented by HPHP
  public: c_WaitableWaitHandle *create();

 public:
  inline bool isFinished() {
    return getState() == STATE_SUCCEEDED || getState() == STATE_FAILED;
  }
  inline bool isSucceeded() { return getState() == STATE_SUCCEEDED; }
  inline bool isFailed() { return getState() == STATE_FAILED; }
  TypedValue* getResult();
  ObjectData* getException();

 public:
  c_BlockableWaitHandle* addParent(c_BlockableWaitHandle* parent);
  inline c_BlockableWaitHandle** getFirstParentPtr() { return &m_firstParent; }
  inline AsioContext* getContext() { return m_context; }

 protected:
  inline uint8_t getState() { return ((uint8_t*)&o_subclassData)[0]; }
  inline void setState(uint8_t state) {
    ((uint8_t*)&o_subclassData)[0] = state;
  }

  void setResult(const TypedValue* result);
  void setException(ObjectData* exception);

  inline void setContext(AsioContext* context) { m_context = context; }

  inline c_BlockableWaitHandle* getFirstParent() { return m_firstParent; }
  c_BlockableWaitHandle* getParentInContext(AsioContext* ctx);


  const TypedValue* join();

  static const int8_t STATE_NEW       = 0;
  static const int8_t STATE_SUCCEEDED = 1;
  static const int8_t STATE_FAILED    = 2;

 private:
  TypedValue m_resultOrException;
  AsioContext* m_context;
  c_BlockableWaitHandle* m_firstParent;
};

///////////////////////////////////////////////////////////////////////////////
// class BlockableWaitHandle

/**
 * A blockable wait handle is a wait handle that can be blocked by a waitable
 * wait handle it is waiting for. Once a wait handle blocking this wait handle
 * is finished, a notification is received and the operation can be resumed.
 */
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(BlockableWaitHandle);
class c_BlockableWaitHandle : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS(BlockableWaitHandle, BlockableWaitHandle, WaitableWaitHandle)

  // need to implement
  public: c_BlockableWaitHandle(const ObjectStaticCallbacks *cb = &cw_BlockableWaitHandle);
  public: ~c_BlockableWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);

  // implemented by HPHP
  public: c_BlockableWaitHandle *create();

 public:
  c_BlockableWaitHandle* getNextParent();
  c_BlockableWaitHandle* unblock();

  void exitContextBlocked(AsioContext* ctx);
  void killCycle();

 protected:
  virtual c_WaitableWaitHandle* getBlockedOn();
  void blockOn(c_WaitableWaitHandle* child);
  virtual void onUnblocked();
  virtual void failBlock(CObjRef exception);

  static const int8_t STATE_BLOCKED = 3;

 private:
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
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(Continuation);
FORWARD_DECLARE_CLASS_BUILTIN(ContinuationWaitHandle);
class c_ContinuationWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(ContinuationWaitHandle, ContinuationWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_ContinuationWaitHandle(const ObjectStaticCallbacks *cb = &cw_ContinuationWaitHandle);
  public: ~c_ContinuationWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_start(const char* cls , CObjRef continuation, int prio = 0);
  public: static Object t_start(CObjRef continuation, int prio = 0) {
    return ti_start("continuationwaithandle", continuation, prio);
  }
  DECLARE_METHOD_INVOKE_HELPERS(start);
  public: static void ti_markcurrentassucceeded(const char* cls , CVarRef result);
  public: static void t_markcurrentassucceeded(CVarRef result) {
    return ti_markcurrentassucceeded("continuationwaithandle", result);
  }
  DECLARE_METHOD_INVOKE_HELPERS(markcurrentassucceeded);
  public: static void ti_markcurrentastailcall(const char* cls );
  public: static void t_markcurrentastailcall() {
    return ti_markcurrentastailcall("continuationwaithandle");
  }
  DECLARE_METHOD_INVOKE_HELPERS(markcurrentastailcall);
  public: Object t_getprivdata();
  DECLARE_METHOD_INVOKE_HELPERS(getprivdata);
  public: void t_setprivdata(CObjRef data);
  DECLARE_METHOD_INVOKE_HELPERS(setprivdata);

  // implemented by HPHP
  public: c_ContinuationWaitHandle *create();

 public:
  void run();
  inline uint16_t getDepth() { return m_depth; }
  String getName();
  void enterContext(AsioContext* ctx);
  void exitContext(AsioContext* ctx);

 protected:
  c_WaitableWaitHandle* getBlockedOn();
  void onUnblocked();
  void failBlock(CObjRef exception);

 private:
  void start(c_Continuation* continuation, uint32_t prio, uint16_t depth);
  void markAsSucceeded(const TypedValue* result);
  void markAsFailed(CObjRef exception);

  p_Continuation m_continuation;
  p_WaitHandle m_child;
  Object m_privData;
  uint32_t m_prio;
  uint16_t m_depth;
  bool m_tailCall;

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
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(GenArrayWaitHandle);
class c_GenArrayWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(GenArrayWaitHandle, GenArrayWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_GenArrayWaitHandle(const ObjectStaticCallbacks *cb = &cw_GenArrayWaitHandle);
  public: ~c_GenArrayWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_create(const char* cls , CArrRef dependencies);
  public: static Object t_create(CArrRef dependencies) {
    return ti_create("genarraywaithandle", dependencies);
  }
  DECLARE_METHOD_INVOKE_HELPERS(create);

  // implemented by HPHP
  public: c_GenArrayWaitHandle *create();

 public:
  String getName();
  void enterContext(AsioContext* ctx);

 protected:
  c_WaitableWaitHandle* getBlockedOn();
  void onUnblocked();
  void failBlock(CObjRef exception);

 private:
  void initialize(CObjRef exception, CArrRef deps, ssize_t iter_pos, c_WaitableWaitHandle* child);

  Object m_exception;
  Array m_deps;
  ssize_t m_iterPos;
};

///////////////////////////////////////////////////////////////////////////////
// class SetResultToRefWaitHandle

/**
 * A wait handle that waits for a given dependency and sets its result to
 * a given reference once completed.
 */
class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(SetResultToRefWaitHandle);
class c_SetResultToRefWaitHandle : public c_BlockableWaitHandle {
 public:
  DECLARE_CLASS(SetResultToRefWaitHandle, SetResultToRefWaitHandle, BlockableWaitHandle)

  // need to implement
  public: c_SetResultToRefWaitHandle(const ObjectStaticCallbacks *cb = &cw_SetResultToRefWaitHandle);
  public: ~c_SetResultToRefWaitHandle();
  public: void t___construct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_create(const char* cls , CObjRef wait_handle, VRefParam ref);
  public: static Object t_create(CObjRef wait_handle, VRefParam ref) {
    return ti_create("setresulttorefwaithandle", wait_handle, ref);
  }
  DECLARE_METHOD_INVOKE_HELPERS(create);

  // implemented by HPHP
  public: c_SetResultToRefWaitHandle *create();

 public:
  String getName();
  void enterContext(AsioContext* ctx);

 protected:
  c_WaitableWaitHandle* getBlockedOn();
  void onUnblocked();
  void failBlock(CObjRef exception);

 private:
  void initialize(c_WaitableWaitHandle* wait_handle, RefData* ref);

  p_WaitableWaitHandle m_child;
  RefData* m_ref;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ASIO_H__
