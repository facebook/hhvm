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
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <folly/Likely.h>

#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct Recorder {
  void onSessionInit();
  void onSessionExit();

  template<typename F, F f>
  static F wrapNativeFunc(const String& name) {
    getNativeFuncNames()[reinterpret_cast<std::uintptr_t>(f)] = name;
    return NativeFunc<F, f>::record;
  }

 private:
  static std::unordered_map<std::uintptr_t, String>& getNativeFuncNames();
  static Recorder& getRecorder();
  void onNativeCall(std::uintptr_t id, const Variant& ret,
                    const req::vector<Variant>& args);
  Variant toVariant() const;
  template<typename T> static Variant toVariant(T value);

  bool m_enabled{false};
  req::vector_map<std::uintptr_t, req::vector<
    std::pair<Variant, req::vector<Variant>>>> m_nativeCalls;

  template<typename F, F f>
  struct NativeFunc;

  template<typename... Args, void(*f)(Args...)>
  struct NativeFunc<void(*)(Args...), f> {
    static void record(Args... args) {
      f(std::forward<Args>(args)...);
      auto& recorder{getRecorder()};
      if (UNLIKELY(recorder.m_enabled)) {
        static const auto id{reinterpret_cast<std::uintptr_t>(f)};
        req::vector<Variant> vargs{toVariant(std::forward<Args>(args))...};
        recorder.onNativeCall(id, Variant{}, vargs);
      }
    }
  };

  template<typename Ret, typename... Args, Ret(*f)(Args...)>
  struct NativeFunc<Ret(*)(Args...), f> {
    static Ret record(Args... args) {
      const Ret ret = f(std::forward<Args>(args)...);
      auto& recorder{getRecorder()};
      if (UNLIKELY(recorder.m_enabled)) {
        static const auto id{reinterpret_cast<std::uintptr_t>(f)};
        req::vector<Variant> vargs{toVariant(std::forward<Args>(args))...};
        recorder.onNativeCall(id, toVariant(ret), vargs);
      }
      return ret;
    }
  };

};

} // namespace HPHP
