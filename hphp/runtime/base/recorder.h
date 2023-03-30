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

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include <folly/Likely.h>

#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

struct Recorder {
  void requestExit();
  void requestInit();
  void setEntryPoint(const String& entryPoint);

  template<auto f>
  static auto wrapNativeFunc(const char* name) {
    using Wrapper = WrapNativeFunc<MethodToFunc<f>::value>;
    addNativeFuncName(Wrapper::id, name);
    return Wrapper::wrapper;
  }

 private:
  static void addNativeArg(std::uintptr_t id, Variant arg);
  static void addNativeCall(std::uintptr_t id, Variant ret);
  static void addNativeFuncName(std::uintptr_t id, const char* name);
  static bool isEnabled();
  Array toArray() const;
  template<typename T> static Variant toVariant(T value);

  template<auto f>
  struct WrapNativeFunc;

  template<typename R, typename... A, R(*f)(A...)>
  struct WrapNativeFunc<f> {
    static R wrapper(A... args) {
      std::conditional_t<std::is_void_v<R>, std::nullptr_t, R> ret;
      if constexpr (std::is_void_v<R>) {
        f(std::forward<A>(args)...);
      } else {
        ret = f(std::forward<A>(args)...);
      }
      if (UNLIKELY(isEnabled())) {
        addNativeCall(id, toVariant(ret));
        (addNativeArg(id, toVariant(std::forward<A>(args))), ...);
      }
      if constexpr (!std::is_void_v<R>) {
        return ret;
      }
    }
    inline static const auto id{reinterpret_cast<std::uintptr_t>(wrapper)};
  };

  bool m_enabled{false};
  String m_entryPoint;
  req::vector_map<std::uintptr_t, req::vector<
    std::pair<Variant, Array>>> m_nativeCalls;
};

} // namespace HPHP
