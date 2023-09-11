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

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <utility>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

struct c_ExternalThreadEventWaitHandle;
struct DebuggerHook;

struct Replayer {
  ~Replayer();
  String file(const String& path) const;
  static Replayer& get();
  String init(const String& path);
  static bool onHasReceived();
  static std::int64_t onProcessSleepEvents();
  static c_ExternalThreadEventWaitHandle* onReceiveSomeUntil();
  static c_ExternalThreadEventWaitHandle* onTryReceiveSome();
  void requestInit() const;
  void setDebuggerHook(DebuggerHook* debuggerHook);

  template<auto f>
  static auto wrapNativeFunc(const char* name) {
    using Wrapper = WrapNativeFunc<MethodToFunc<f>::value>;
    get().m_nativeFuncNames[name] = Wrapper::id;
    return Wrapper::wrapper;
  }

 private:
  struct ExternalThreadEvent;
  struct DebuggerHook;
  template<auto f> struct WrapNativeFunc;

  template<typename R, typename... A, R(*f)(A...)>
  struct WrapNativeFunc<f> {
    inline static const auto id{reinterpret_cast<std::uintptr_t>(f)};
    static R wrapper(A... args) {
      if (auto& replayer{get()}; !replayer.m_inNativeCall) {
        replayer.m_inNativeCall = true;
        SCOPE_EXIT { replayer.m_inNativeCall = false; };
        return replayer.replayNativeFunc<R>(id, std::forward<A>(args)...);
      } else {
        return f(std::forward<A>(args)...);
      }
    }
  };

  Object makeWaitHandle(const NativeCall& call);
  template<typename T> static void nativeArg(const String& recordedArg, T arg);
  NativeCall popNativeCall(std::uintptr_t id);
  template<typename T> static T unserialize(const String& recordedValue);

  template<typename R, typename... A>
  R replayNativeFunc(std::uintptr_t id, A&&... args) {
    const auto call{popNativeCall(id)};
    std::int64_t i{-1};
    (nativeArg<A>(call.args[++i].asCStrRef(), std::forward<A>(args)), ...);
    if constexpr (std::is_same_v<R, Object>) {
      if (call.wh) {
        return makeWaitHandle(call);
      }
    }
    if (call.ret.empty()) {
      // NOLINTNEXTLINE(facebook-hte-ThrowNonStdExceptionIssue)
      throw unserialize<Object>(call.exc);
    } else {
      return unserialize<R>(call.ret);
    }
  }

  std::deque<AsioEvent> m_asioEvents;
  HPHP::DebuggerHook* m_debuggerHook;
  std::unordered_map<std::string, std::string> m_files;
  bool m_inNativeCall;
  std::deque<NativeCall> m_nativeCalls;
  std::unordered_map<std::string, std::uintptr_t> m_nativeFuncNames;
  std::size_t m_nextThreadCreationOrder;
  Array m_serverGlobal;
  std::unordered_map<std::size_t, c_ExternalThreadEventWaitHandle*> m_threads;
};

} // namespace HPHP
