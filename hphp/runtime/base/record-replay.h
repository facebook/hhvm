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

#include <string_view>
#include <utility>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/string-data.h"

namespace HPHP {

namespace Native {
  template<typename T> struct NativeArg;
  using ArrayArg = NativeArg<ArrayData>;
  using ObjectArg = NativeArg<ObjectData>;
  using StringArg = NativeArg<StringData>;
} // namespace Native

constexpr bool shouldRecordReplay(std::string_view) {
  return false; // TODO(sblaha) Change after split DWARF is implemented
}

template<auto m>
struct MethodToFunc;

template<typename R, typename... A, R(*f)(A...)>
struct MethodToFunc<f> {
  static constexpr auto value{f};
};

template<typename R, typename C, typename... A, R(C::*m)(A...)>
struct MethodToFunc<m> {
  static auto value(ObjectData* object, A... args) {
    using F = R(*)(ObjectData*, A...);
    static constexpr union { decltype(m) _; F f; } u{m};
    return u.f(object, std::forward<A>(args)...);
  }
};

template<typename R, typename C, typename... A, R(C::*m)(A...) const>
struct MethodToFunc<m> {
  static auto value(ObjectData* object, A... args) {
    using F = R(*)(const ObjectData*, A...);
    static constexpr union { decltype(m) _; F f; } u{m};
    return u.f(object, std::forward<A>(args)...);
  }
};

} // namespace HPHP
