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

#ifndef incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class WaitHandle

/**
 * A wait handle is an object that describes operation that is potentially
 * asynchronous. A WaitHandle class is a base class of all such objects. There
 * are multiple types of wait handles, this is their hierarchy:
 *
 * WaitHandle                      - abstract wait handle
 *  StaticWaitHandle               - statically finished wait handle
 *  WaitableWaitHandle             - wait handle that can be waited for
 *   ResumableWaitHandle           - wait handle that can resume PHP execution
 *    AsyncFunctionWaitHandle      - async function-based async execution
 *    AsyncGeneratorWaitHandle     - async generator-based async execution
 *   AwaitAllWaitHandle            - wait handle representing a collection of
 *                                     WHs, does not propagate results
 *   ConditionWaitHandle           - wait handle implementing condition variable
 *   RescheduleWaitHandle          - wait handle that reschedules execution
 *   SleepWaitHandle               - wait handle that finishes after a timeout
 *   ExternalThreadEventWaitHandle - thread-powered asynchronous execution
 *
 *   // DEPRECATED
 *   GenArrayWaitHandle            - wait handle representing an array of WHs
 *   GenMapWaitHandle              - wait handle representing an Map of WHs
 *   GenVectorWaitHandle           - wait handle representing an Vector of WHs
 *
 * A wait handle can be either synchronously joined (waited for the operation
 * to finish) or passed in various contexts as a dependency and waited for
 * asynchronously (such as using await mechanism of async function or
 * passed as an array member of GenArrayWaitHandle).
 */

class c_AsyncFunctionWaitHandle;
class c_AsyncGeneratorWaitHandle;
class c_AwaitAllWaitHandle;
class c_GenArrayWaitHandle;
class c_GenMapWaitHandle;
class c_GenVectorWaitHandle;
class c_ConditionWaitHandle;
class c_RescheduleWaitHandle;
class c_SleepWaitHandle;
class c_ExternalThreadEventWaitHandle;
class c_WaitHandle : public ExtObjectDataFlags<ObjectData::IsWaitHandle|
                                               ObjectData::NoDestructor> {
 public:
  DECLARE_CLASS_NO_SWEEP(WaitHandle)

  enum class Kind : uint8_t {
    Static,
    AsyncFunction,
    AsyncGenerator,
    AwaitAll,
    GenArray,
    GenMap,
    GenVector,
    Condition,
    Reschedule,
    Sleep,
    ExternalThreadEvent,
  };

  explicit c_WaitHandle(Class* cls = c_WaitHandle::classof(),
                        HeaderKind kind = HeaderKind::Object) noexcept
    : ExtObjectDataFlags(cls, kind, NoInit{}) {}
  ~c_WaitHandle() {}

  void t___construct();
  static void ti_setoniowaitentercallback(const Variant& callback);
  static void ti_setoniowaitexitcallback(const Variant& callback);
  static void ti_setonjoincallback(const Variant& callback);
  Object t_getwaithandle();
  void t_import();
  Variant t_join();
  Variant t_result();
  bool t_isfinished();
  bool t_issucceeded();
  bool t_isfailed();
  int64_t t_getid();
  String t_getname();

 public:
  static constexpr ptrdiff_t stateOff() {
    return offsetof(c_WaitHandle, m_kind_state);
  }
  static constexpr ptrdiff_t resultOff() {
    return offsetof(c_WaitHandle, m_resultOrException);
  }

  static c_WaitHandle* fromCell(const Cell* cell) {
    return (
        cell->m_type == KindOfObject &&
        cell->m_data.pobj->getAttribute(ObjectData::IsWaitHandle)
      ) ? static_cast<c_WaitHandle*>(cell->m_data.pobj) : nullptr;
  }
  static c_WaitHandle* fromCellAssert(const Cell* cell) {
    assert(cell->m_type == KindOfObject);
    assert(cell->m_data.pobj->getAttribute(ObjectData::IsWaitHandle));
    return static_cast<c_WaitHandle*>(cell->m_data.pobj);
  }
  bool isFinished() const { return getState() <= STATE_FAILED; }
  bool isSucceeded() const { return getState() == STATE_SUCCEEDED; }
  bool isFailed() const { return getState() == STATE_FAILED; }
  Cell getResult() const {
    assert(isSucceeded());
    return m_resultOrException;
  }
  ObjectData* getException() const {
    assert(isFailed());
    return m_resultOrException.m_data.pobj;
  }

  Kind getKind() const { return static_cast<Kind>(m_kind_state >> 4); }
  uint8_t getState() const { return m_kind_state & 0x0F; }
  static uint8_t toKindState(Kind kind, uint8_t state) {
    assert((uint8_t)kind < 0x10 && state < 0x10);
    return ((uint8_t)kind << 4) | state;
  }
  void setKindState(Kind kind, uint8_t state) {
    m_kind_state = toKindState(kind, state);
  }
  void setContextVectorIndex(uint32_t idx) {
    m_ctxVecIndex = idx;
  }

  c_AsyncFunctionWaitHandle* asAsyncFunction();
  c_AsyncGeneratorWaitHandle* asAsyncGenerator();
  c_AwaitAllWaitHandle* asAwaitAll();
  c_GenArrayWaitHandle* asGenArray();
  c_GenMapWaitHandle* asGenMap();
  c_GenVectorWaitHandle* asGenVector();
  c_ConditionWaitHandle* asCondition();
  c_RescheduleWaitHandle* asReschedule();
  c_ResumableWaitHandle* asResumable();
  c_SleepWaitHandle* asSleep();
  c_ExternalThreadEventWaitHandle* asExternalThreadEvent();

  // The code in the TC will depend on the values of these constants.
  // See emitAwait().
  static const int8_t STATE_SUCCEEDED = 0;
  static const int8_t STATE_FAILED    = 1;

 private: // layout, ignoring ObjectData fields.
  // 0                           8             9             10 12
  // [m_parentChain             ][m_contextIdx][m_kind_state][ ][m_ctxVecIndex]
  // [m_resultOrException.m_data][m_type]                       [m_aux]
  static void checkLayout() {
    constexpr auto data = offsetof(c_WaitHandle, m_resultOrException);
    constexpr auto type = data + offsetof(TypedValue, m_type);
    constexpr auto aux  = data + offsetof(TypedValue, m_aux);
    static_assert(offsetof(c_WaitHandle, m_parentChain) == data, "");
    static_assert(offsetof(c_WaitHandle, m_contextIdx) == type, "");
    static_assert(offsetof(c_WaitHandle, m_kind_state) < aux, "");
    static_assert(offsetof(c_WaitHandle, m_ctxVecIndex) == aux, "");
  }

 protected:
  union {
    // STATE_SUCCEEDED || STATE_FAILED
    Cell m_resultOrException;

    // !STATE_SUCCEEDED && !STATE_FAILED
    struct {
      // WaitableWaitHandle: !STATE_SUCCEEDED && !STATE_FAILED
      AsioBlockableChain m_parentChain;

      // WaitableWaitHandle: !STATE_SUCCEEDED && !STATE_FAILED
      context_idx_t m_contextIdx;

      // valid in any WaitHandle state. doesn't overlap Cell fields.
      uint8_t m_kind_state;

      union {
        // ExternalThreadEventWaitHandle: STATE_WAITING
        // SleepWaitHandle: STATE_WAITING
        uint32_t m_ctxVecIndex;
      };
    };
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_
