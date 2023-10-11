/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <type_traits>
#include <utility>

#include <folly/Likely.h>

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-optional.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/util/hdf.h"

namespace HPHP {

struct c_Awaitable;
struct c_ExternalThreadEventWaitHandle;
namespace Stream { struct Wrapper; }

struct Recorder {
  static void onGetFactsForRequest(FactsStore*& map);
  static void onHasReceived(bool received);
  static void onProcessSleepEvents(std::int64_t now);
  static void onReceiveSomeUntil(c_ExternalThreadEventWaitHandle* received);
  static void onRuntimeOptionLoad(const IniSettingMap& ini, const Hdf& hdf,
                                  const std::string& cmd);
  static void onTryReceiveSome(c_ExternalThreadEventWaitHandle* received);
  static void onVisitEntitiesToInvalidate();
  static void onVisitEntitiesToInvalidateFast();
  static void onVisitEntity(const std::string& entity);
  void requestExit();
  void requestInit();

  template<auto f>
  static auto wrapNativeFunc(const char* name) {
    using Wrapper = WrapNativeFunc<rr::MethodToFunc<f>::value>;
    rr::addNativeFuncName(Wrapper::ptr, name);
    return Wrapper::wrapper;
  }

 private:
  struct DebuggerHook;
  struct FactsStore;
  struct LoggerHook;
  struct StdoutHook;
  struct StreamWrapper;

  template<auto f>
  struct WrapNativeFunc;

  template<typename R, typename... A, R(*f)(A...)>
  struct WrapNativeFunc<f> {
    static const NativeFunction ptr;
    static R wrapper(A... args) {
      if (const auto recorder{get()}; UNLIKELY(recorder->m_enabled)) {
        static const auto shouldRecord{rr::shouldRecordReplay(ptr)};
        if (shouldRecord) {
          return recorder->recordNativeCall(f, ptr, std::forward<A>(args)...);
        }
      }
      return f(std::forward<A>(args)...);
    }
  };

  static Recorder* get();
  static HPHP::FactsStore* getFactsStore();
  static StdoutHook* getStdoutHook();
  static Stream::Wrapper* getStreamWrapper();
  void onNativeCallArg(const String& arg);
  void onNativeCallEntry(NativeFunction ptr);
  void onNativeCallExit();
  void onNativeCallReturn(const String& ret);
  void onNativeCallThrow(std::exception_ptr exc);
  void onNativeCallWaitHandle(c_Awaitable* wh);
  void resolveWaitHandles();
  Array toArray() const;

  template<typename R, typename... A>
  R recordNativeCall(R(*f)(A...), NativeFunction ptr, A&&... args) {
    onNativeCallEntry(ptr);
    std::conditional_t<std::is_void_v<R>, std::nullptr_t, R> ret;
    std::exception_ptr exc;
    try {
      if constexpr (std::is_void_v<R>) {
        f(std::forward<A>(args)...);
      } else {
        ret = f(std::forward<A>(args)...);
      }
    } catch (...) {
      exc = std::current_exception();
    }
    (onNativeCallArg(rr::serialize(std::forward<A>(args))), ...);
    if (exc) {
      onNativeCallThrow(exc);
      std::rethrow_exception(exc);
    } else {
      ObjectData* obj{nullptr};
      if constexpr (std::is_same_v<R, Object>) {
        obj = ret.get();
      } else if constexpr (std::is_same_v<R, Variant>) {
        if (ret.isObject()) {
          obj = ret.asCObjRef().get();
        }
      }
      if (obj != nullptr && obj->isWaitHandle()) {
        onNativeCallWaitHandle(std::bit_cast<c_Awaitable*>(obj));
      } else {
        onNativeCallReturn(rr::serialize(ret));
      }
      if constexpr (!std::is_void_v<R>) {
        return ret;
      }
    }
  }

  bool m_enabled;
  Array m_factsStore;
  req::vector<rr::NativeCall> m_nativeCalls;
  req::vector<rr::NativeEvent> m_nativeEvents;
  std::size_t m_nextThreadCreationOrder;
  HPHP::FactsStore* m_parentFactsStore;
  Stream::Wrapper* m_parentStreamWrapper;
  req::hash_map<c_Awaitable*, std::size_t> m_pendingWaitHandleToNativeCall;
  Array m_serverGlobal;
  Array m_streamWrapper;
  req::hash_map<const c_ExternalThreadEventWaitHandle*, std::size_t> m_threads;
};

template<typename R, typename... A, R(*f)(A...)>
const NativeFunction Recorder::WrapNativeFunc<f>::ptr{
  reinterpret_cast<NativeFunction>(wrapper)};

} // namespace HPHP
