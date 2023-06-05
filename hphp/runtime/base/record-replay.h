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
#include <string_view>

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {

constexpr bool shouldRecordReplay(std::string_view name) {
  return !(
    // Native functions for async which are handled separately
    name == "HH\\Asio\\join" ||
    name == "HH\\RescheduleWaitHandle::create" ||

    // Native functions that call back to user code
    name == "array_map" ||
    name == "call_user_func" ||
    name == "call_user_func_array" ||
    name == "fb_intercept2" ||
    name == "fb_setprofile" ||
    name == "hphp_invoke" ||
    name == "hphp_invoke_method" ||
    name == "preg_replace_callback" ||
    name == "set_error_handler" ||
    name == "set_exception_handler" ||

    // Native functions that affect interpreter state
    name == "error_reporting" ||
    name == "fb_rename_function" ||

    // Native functions that output directly to stdout/stderr
    name == "printf" ||
    name == "sprintf" ||
    name == "vprintf" ||
    name == "vsprintf"
  );
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
    return u.f(object, args...);
  }
};

template<typename R, typename C, typename... A, R(C::*m)(A...) const>
struct MethodToFunc<m> {
  static auto value(ObjectData* object, A... args) {
    using F = R(*)(const ObjectData*, A...);
    static constexpr union { decltype(m) _; F f; } u{m};
    return u.f(object, args...);
  }
};

struct NativeCall {
  std::uintptr_t id{0};
  Array stdouts{Array::CreateVec()};
  Array args{Array::CreateVec()};
  String ret{empty_string()};
  String exc{empty_string()};
};

} // namespace HPHP
