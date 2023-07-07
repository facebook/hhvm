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
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {

constexpr bool shouldRecordReplay(std::string_view name) {
  // #lizard forgives (This disables 'Large Cyclomatic Complexity' check)
  return !(
    // Native functions participating in serialization/deserialization
    name.ends_with("->__sleep") ||
    name.ends_with("->__wakeup") ||

    // Native functions for async which are handled separately
    name == "HH\\Asio\\join" ||
    name == "HH\\Awaitable->isFinished" ||
    name == "HH\\AwaitAllWaitHandle::fromDict" ||
    name == "HH\\AwaitAllWaitHandle::fromVec" ||
    name == "HH\\RescheduleWaitHandle::create" ||

    // Native functions that call back to user code
    name == "array_map" ||
    name == "call_user_func_array" ||
    name == "call_user_func" ||
    name == "fb_intercept2" ||
    name == "fb_set_exit_callback" ||
    name == "fb_setprofile" ||
    name == "header_register_callback" ||
    name == "HH\\Awaitable::setOnIOWaitEnterCallback" ||
    name == "HH\\Awaitable::setOnIOWaitExitCallback" ||
    // FIXME Disabling memory threshold until surprise flags are handled
    // name == "HH\\set_mem_threshold_callback" ||
    name == "hphp_invoke_method" ||
    name == "hphp_invoke" ||
    name == "preg_replace_callback" ||
    name == "register_postsend_function" ||
    name == "register_shutdown_function" ||
    name == "set_error_handler" ||
    name == "set_exception_handler" ||

    // Native functions that affect interpreter state
    name == "error_reporting" ||
    name == "fb_rename_function" ||

    // Native functions that return unserializable values
    name == "HH\\dynamic_class_meth" ||

    // Native functions that output directly to stdout/stderr
    name == "printf" ||
    name == "sprintf" ||
    name == "vprintf" ||
    name == "vsprintf" ||

    // FIXME: Below are temporarily not being recorded to unblock OBA.

    // This is called by the IO enter/exit callbacks which are difficult to
    // capture deterministically. While this function is non-deterministic,
    // it appears to only be used for logging how long the IO wait takes.
    name == "clock_gettime_ns" ||

    // Native functions that are causing issues with OBA
    name == "array_key_exists" ||
    name == "array_reverse" ||
    name == "DateTime->__construct" ||
    name == "DateTime->setTimestamp" ||
    name == "get_class" ||
    name == "gettype" ||
    name == "HH\\ImmMap->__construct" ||
    name == "HH\\ImmSet->__construct" ||
    name == "HH\\ImmSet->contains" ||
    name == "HH\\is_meth_caller" ||
    name == "HH\\Lib\\_Private\\_Str\\slice_l" ||
    name == "HH\\Vector->__construct" ||
    name == "is_a" ||
    name == "is_callable_with_name" ||
    name == "is_callable" ||
    name == "NativeTrimmableMap->get" ||
    name == "NativeTrimmableMap->remove" ||
    name == "NativeTrimmableMap->set" ||
    name == "NativeTrimmableMap2->get" ||
    name == "NativeTrimmableMap2->remove" ||
    name == "NativeTrimmableMap2->set" ||
    name == "spl_object_hash" ||
    name == "substr" ||
    name == "var_dump" ||

    // Native functions that are common and known to be deterministic
    name == "HH\\Lib\\_Private\\Native\\last" ||
    name == "count" ||
    name == "ord" ||
    name == "strlen" ||
    name == "HH\\ImmVector->__construct" ||
    name == "chr" ||
    name == "HH\\BuiltinEnum::coerce" ||
    name == "enum_exists" ||
    name == "floor" ||
    name == "HH\\Lib\\_Private\\_Str\\strpos_l" ||
    name == "HH\\Lib\\_Private\\_Str\\starts_with_l" ||
    name == "is_numeric" ||
    name == "HH\\Lib\\_Private\\_Str\\split_l" ||
    name == "HH\\Lib\\_Private\\Native\\first" ||
    name == "inet_pton"
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
  std::uintptr_t waitHandle{0};
};

} // namespace HPHP
