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
#include <utility>

#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

struct Replayer {
  String file(const String& path);
  String init(const String& path);

  template<auto f>
  static auto wrapNativeFunc(const char* name) {
    using Wrapper = WrapNativeFunc<MethodToFunc<f>::value>;
    addNativeFuncName(Wrapper::id, name);
    return Wrapper::wrapper;
  }

 private:
  static void addNativeFuncName(std::uintptr_t id, const char* name);
  template<typename T> static T fromVariant(const Variant& value);
  static Variant popNativeArg(std::uintptr_t id);
  template<typename T> static void popNativeArg(std::uintptr_t id, T arg);
  static Variant popNativeCall(std::uintptr_t id);

  template<auto f>
  struct WrapNativeFunc;

  template<typename R, typename... A, R(*f)(A...)>
  struct WrapNativeFunc<f> {
    static R wrapper(A... args) {
      (popNativeArg(id, std::forward<A>(args)), ...);
      return fromVariant<R>(popNativeCall(id));
    }
    inline static const auto id{reinterpret_cast<std::uintptr_t>(wrapper)};
  };

  Array m_files;
  Array m_nativeCalls;
};

} // namespace HPHP
