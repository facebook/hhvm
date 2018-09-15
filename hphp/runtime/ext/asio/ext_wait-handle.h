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

#ifndef incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class Awaitable

void HHVM_STATIC_METHOD(Awaitable, setOnIoWaitEnterCallback,
                        const Variant& callback);
void HHVM_STATIC_METHOD(Awaitable, setOnIoWaitExitCallback,
                        const Variant& callback);
void HHVM_STATIC_METHOD(Awaitable, setOnJoinCallback,
                        const Variant& callback);
bool HHVM_METHOD(Awaitable, isFinished);
bool HHVM_METHOD(Awaitable, isSucceeded);
bool HHVM_METHOD(Awaitable, isFailed);
String HHVM_METHOD(Awaitable, getName);

/**
 * A wait handle is an object that describes operation that is potentially
 * asynchronous. A WaitHandle class is a base class of all such objects. There
 * are multiple types of wait handles, this is their hierarchy:
 *
 * Awaitable                       - abstract wait handle
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
 * A wait handle can be either synchronously joined (waited for the operation
 * to finish) or passed in various contexts as a dependency and waited for
 * asynchronously (such as using await mechanism of async function or
 * passed to AwaitAllWaitHandle).
 */

struct c_AsyncFunctionWaitHandle;
struct c_AsyncGeneratorWaitHandle;
struct c_AwaitAllWaitHandle;
struct c_ConditionWaitHandle;
struct c_RescheduleWaitHandle;
struct c_SleepWaitHandle;
struct c_ExternalThreadEventWaitHandle;

#define WAITHANDLE_CLASSOF(cn) \
  static Class* classof() { \
    static Class* cls = Unit::lookupClass(makeStaticString("HH\\" #cn)); \
    return cls; \
  }

#define WAITHANDLE_DTOR(cn) \
  static void instanceDtor(ObjectData* obj, const Class*) { \
    auto wh = wait_handle<c_##cn>(obj); \
    wh->~c_##cn(); \
    tl_heap->objFree(obj, sizeof(c_##cn)); \
  }

template<class T>
T* wait_handle(const ObjectData* obj) {
  assertx(obj->instanceof(T::classof()));
  assertx(obj->isWaitHandle());
  return static_cast<T*>(const_cast<ObjectData*>(obj));
}

struct c_Awaitable : ObjectData {
  WAITHANDLE_CLASSOF(Awaitable);
  WAITHANDLE_DTOR(Awaitable);

  enum class Kind : uint8_t {
    Static,
    AsyncFunction,
    AsyncGenerator,
    AwaitAll,
    Condition,
    Reschedule,
    Sleep,
    ExternalThreadEvent,
  };

  explicit c_Awaitable(Class* cls, HeaderKind kind,
                        type_scan::Index tyindex) noexcept
    : ObjectData(cls, NoInit{}, ObjectData::NoDestructor, kind),
      m_tyindex(tyindex)
  {
    assertx(type_scan::isKnownType(tyindex));
  }

  ~c_Awaitable()
  {}

 public:
  static constexpr ptrdiff_t stateOff() {
    return offsetof(c_Awaitable, m_kind_state);
  }
  static constexpr ptrdiff_t resultOff() {
    return offsetof(c_Awaitable, m_resultOrException);
  }

  static c_Awaitable* fromCell(Cell cell) {
    return (
        cell.m_type == KindOfObject && cell.m_data.pobj->isWaitHandle()
      ) ? static_cast<c_Awaitable*>(cell.m_data.pobj) : nullptr;
  }
  static c_Awaitable* fromCellAssert(Cell cell) {
    assertx(cell.m_type == KindOfObject);
    assertx(cell.m_data.pobj->isWaitHandle());
    return static_cast<c_Awaitable*>(cell.m_data.pobj);
  }
  bool isFinished() const { return getState() <= STATE_FAILED; }
  bool isSucceeded() const { return getState() == STATE_SUCCEEDED; }
  bool isFailed() const { return getState() == STATE_FAILED; }
  Cell getResult() const {
    assertx(isSucceeded());
    return m_resultOrException;
  }
  ObjectData* getException() const {
    assertx(isFailed());
    return m_resultOrException.m_data.pobj;
  }

  Kind getKind() const { return static_cast<Kind>(m_kind_state >> 4); }
  uint8_t getState() const { return m_kind_state & 0x0F; }
  static uint8_t toKindState(Kind kind, uint8_t state) {
    assertx((uint8_t)kind < 0x10 && state < 0x10);
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
  c_ConditionWaitHandle* asCondition();
  c_RescheduleWaitHandle* asReschedule();
  c_ResumableWaitHandle* asResumable();
  c_SleepWaitHandle* asSleep();
  c_ExternalThreadEventWaitHandle* asExternalThreadEvent();

  // The code in the TC will depend on the values of these constants.
  // See emitAwait().
  static const int8_t STATE_SUCCEEDED = 0; // completed with result
  static const int8_t STATE_FAILED    = 1; // completed with exception

  void scan(type_scan::Scanner&) const;

 private: // layout, ignoring ObjectData fields.
  // 0                         8           9           10       12
  // [parentChain             ][contextIdx][kind_state][tyindex][ctxVecIndex]
  // [resultOrException.m_data][m_type]                         [aux]
  static void checkLayout() {
    constexpr auto data = offsetof(c_Awaitable, m_resultOrException);
    constexpr auto type = data + offsetof(TypedValue, m_type);
    constexpr auto aux  = data + offsetof(TypedValue, m_aux);
    static_assert(offsetof(c_Awaitable, m_parentChain) == data, "");
    static_assert(offsetof(c_Awaitable, m_contextIdx) == type, "");
    static_assert(offsetof(c_Awaitable, m_kind_state) < aux, "");
    static_assert(offsetof(c_Awaitable, m_ctxVecIndex) == aux, "");
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

      // type index of concrete waithandle for gc-scanning
      type_scan::Index m_tyindex;

      union {
        // ExternalThreadEventWaitHandle: STATE_WAITING
        // SleepWaitHandle: STATE_WAITING
        uint32_t m_ctxVecIndex;
      };
    };
  };

  TYPE_SCAN_CUSTOM() {
    if (isFinished()) {
      scanner.scan(m_resultOrException);
    } else {
      scanner.scan(m_parentChain);
    }
  }
};

template<class T>
T* wait_handle(Cell cell) {
  return wait_handle<T>(c_Awaitable::fromCellAssert(cell));
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_
