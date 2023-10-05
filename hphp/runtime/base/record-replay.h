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

struct NativeArgs;
using NativeFunction = void(*)(NativeArgs*);

void addNativeFuncName(NativeFunction ptr, std::string_view name);
std::string_view getNativeFuncName(NativeFunction ptr);
NativeFunction getNativeFuncPtr(std::string_view name);
bool shouldRecordReplay(NativeFunction ptr);

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
  NativeFunction ptr{nullptr};
  Array stdouts{Array::CreateVec()};
  Array args{Array::CreateVec()};
  String ret{empty_string()};
  String exc{empty_string()};
  std::uint8_t wh{0};
};

struct NativeEvent {
  enum class Type {
    UNKNOWN,
    HAS_RECEIVED,
    PROCESS_SLEEP_EVENTS,
    RECEIVE_SOME_UNTIL,
    TRY_RECEIVE_SOME,
    VISIT_ENTITIES_TO_INVALIDATE,
    VISIT_ENTITIES_TO_INVALIDATE_FAST,
  };
  Type type{Type::UNKNOWN};
  Array value{Array::CreateVec()};
};

} // namespace HPHP
