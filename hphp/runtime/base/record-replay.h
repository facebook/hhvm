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
    // Native functions for collections and other primitives
    name.starts_with("DateTime->") ||
    name.starts_with("HH\\ImmMap->") ||
    name.starts_with("HH\\ImmSet->") ||
    name.starts_with("HH\\ImmVector->") ||
    name.starts_with("HH\\Map->") ||
    name.starts_with("HH\\Set->") ||
    name.starts_with("HH\\Vector->") ||
    name.starts_with("NativeTrimmableMap->") ||
    name.starts_with("NativeTrimmableMap2->") ||
    name.starts_with("NativeTrimmableMap3->") ||

    // Native functions participating in serialization/deserialization
    name.ends_with("->__sleep") ||
    name.ends_with("->__wakeup") ||

    // Native functions for async which are handled separately
    name == "HH\\Asio\\join" ||
    name == "HH\\Awaitable->isFinished" ||
    name == "HH\\AwaitAllWaitHandle::fromDict" ||
    name == "HH\\AwaitAllWaitHandle::fromVec" ||
    name == "HH\\RescheduleWaitHandle::create" ||

    // Native functions for configerator which are handled separately
    name == "ConfigeratorExtensionApi::invalidateEntitiesWithPureVistor" ||
    name == "ConfigeratorExtensionApi::visitEntitiesToInvalidate" ||

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

    // Native functions that return unserializable values
    name == "create_opaque_value_internal" ||
    name == "debug_backtrace" ||
    name == "HH\\dynamic_class_meth" ||
    name == "unwrap_opaque_value" ||

    // Native functions that affect interpreter state
    name == "error_reporting" ||
    name == "fb_rename_function" ||
    name == "HH\\clear_instance_memoization" ||
    name == "HH\\clear_lsb_memoization" ||
    name == "HH\\clear_static_memoization" ||

    // Native functions that output directly to stdout/stderr
    name == "printf" ||
    name == "sprintf" ||
    name == "vprintf" ||
    name == "vsprintf" ||

    // Native functions that are common and known to be deterministic
    name == "HH\\BuiltinEnum::coerce" ||
    name == "HH\\Lib\\_Private\\Native\\last" ||
    name == "chr" ||
    name == "count" ||
    name == "get_class" ||
    name == "implode" ||
    name == "ord" ||
    name == "strlen"
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
  std::uint8_t wh{0};
};

struct NativeEvent {
  enum class Type {
    HAS_RECEIVED,
    PROCESS_SLEEP_EVENTS,
    RECEIVE_SOME_UNTIL,
    TRY_RECEIVE_SOME,
    VISIT_ENTITIES_TO_INVALIDATE,
    VISIT_ENTITIES_TO_INVALIDATE_FAST,
  };
  Type type;
  Array value{Array::CreateVec()};
};

} // namespace HPHP
