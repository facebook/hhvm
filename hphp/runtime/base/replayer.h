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
#include <deque>
#include <exception>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-nonnull-ret.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/util/hdf.h"

namespace HPHP {

struct c_ExternalThreadEventWaitHandle;
struct DebuggerHook;
namespace Stream { struct Wrapper; }

struct Replayer {
  static std::string getEntryPoint();
  static HPHP::FactsStore* onGetFactsForRequest();
  static bool onHasReceived();
  static std::int64_t onProcessSleepEvents();
  static c_ExternalThreadEventWaitHandle* onReceiveSomeUntil();
  static void onRuntimeOptionLoad(IniSettingMap& ini, Hdf& hdf,
                                  const std::string& path);
  static c_ExternalThreadEventWaitHandle* onTryReceiveSome();
  static void onVisitEntitiesToInvalidate(const Variant& visitor);
  static void onVisitEntitiesToInvalidateFast(const Variant& visitor);
  static void requestExit();
  static void requestInit();
  static void setDebuggerHook(DebuggerHook* debuggerHook);

  template<auto f>
  static auto wrapNativeFunc(const char* name) {
    using Wrapper = WrapNativeFunc<rr::MethodToFunc<f>::value>;
    rr::addNativeFuncName(Wrapper::ptr, name);
    return Wrapper::wrapper;
  }

 private:
  struct DebuggerHook;
  struct ExternalThreadEvent;
  struct FactsStore;
  struct StreamWrapper;

  template<auto f>
  struct WrapNativeFunc;

  template<typename R, typename... A, R(*f)(A...)>
  struct WrapNativeFunc<f> {
    static const NativeFunction ptr;
    static R wrapper(A... args) {
      static const auto shouldReplay{rr::shouldRecordReplay(ptr)};
      if (shouldReplay) {
        if (auto& replayer{get()}; !replayer.m_inNativeCall) {
          replayer.m_inNativeCall = true;
          SCOPE_EXIT { replayer.m_inNativeCall = false; };
          return replayer.replayNativeCall<R>(ptr, std::forward<A>(args)...);
        }
      }
      return f(std::forward<A>(args)...);
    }
  };

  static Replayer& get();
  static HPHP::FactsStore* getFactsStore();
  static Stream::Wrapper* getStreamWrapper();
  Object makeWaitHandle(const rr::NativeCall& call);
  template<typename T> static void nativeArg(const String& recordedArg, T arg);
  rr::NativeCall popNativeCall(NativeFunction ptr);

  template<typename R, typename... A>
  R replayNativeCall(NativeFunction ptr, A&&... args) {
    const auto call{popNativeCall(ptr)};
    std::int64_t i{-1};
    (nativeArg<A>(call.args[++i].asCStrRef(), std::forward<A>(args)), ...);
    if constexpr (std::is_same_v<R, Object>) {
      if (call.wh) {
        return makeWaitHandle(call);
      }
    }
    if (call.ret.empty()) {
      std::rethrow_exception(rr::unserialize<std::exception_ptr>(call.exc));
    } else {
      return rr::unserialize<R>(call.ret);
    }
  }

  HPHP::DebuggerHook* m_debuggerHook;
  std::string m_entryPoint;
  Array m_factsStore;
  bool m_inNativeCall;
  std::deque<rr::NativeCall> m_nativeCalls;
  std::deque<rr::NativeEvent> m_nativeEvents;
  std::unordered_map<NativeFunction, std::string> m_nativeFuncIds;
  std::size_t m_nextThreadCreationOrder;
  Array m_serverGlobal;
  Array m_streamWrapper;
  std::unordered_map<std::size_t, c_ExternalThreadEventWaitHandle*> m_threads;
};

template<typename R, typename... A, R(*f)(A...)>
const NativeFunction Replayer::WrapNativeFunc<f>::ptr{
  reinterpret_cast<NativeFunction>(wrapper)};

} // namespace HPHP
