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
#include <cstring>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/util/assertions.h"

namespace HPHP {

struct Replayer {

  String file(std::string path);
  std::string init(std::string path);

  template<typename F, F f>
  static auto wrapNativeFunc(const String& name) {
    using Wrapper = NativeFuncWrapper<F, f>;
    getNativeFuncNames()[Wrapper::id()] = name.toCppString();
    return Wrapper::replay;
  }

 private:
  using NativeFuncId = std::uintptr_t;
  using NativeFuncNames = std::unordered_map<NativeFuncId, std::string>;

  template<typename T> static T fromVariant(const Variant& value);
  static std::pair<Array, Variant> getNativeCall(NativeFuncId id);
  static NativeFuncNames& getNativeFuncNames();

  Array m_files;
  Array m_header;
  Array m_nativeCalls;

  template<typename F, auto f>
  struct NativeFuncWrapper;

  template<typename Ret, typename... Args, auto f>
  struct NativeFuncWrapper<Ret(*)(Args...), f> {
    static Ret replay(Args... args) {
      auto call{getNativeCall(id())};
      if constexpr (sizeof...(Args) > 0) {
        std::int64_t i{0};
        (handleOutArg(std::forward<Args>(args), call.first[i++]), ...);
      }
      if constexpr (!std::is_void_v<Ret>) {
        return fromVariant<Ret>(call.second);
      }
    }

    static NativeFuncId id() {
      union NativeFuncToId {
        decltype(f) _;
        NativeFuncId id;
      };
      static const auto id{NativeFuncToId{f}.id};
      return id;
    }

   private:
    template<typename T>
    static void handleOutArg(T arg, Variant recordedArg) {
      if constexpr (std::is_reference_v<T>) {
        using R = std::remove_reference_t<T>;
        if constexpr (!std::is_const_v<R>) {
          arg = fromVariant<R>(recordedArg);
        }
      }
    }
  };

  template<typename Cls, typename Ret, typename... Args, auto f>
  struct NativeFuncWrapper<Ret(Cls::*)(Args...), f> :
    public NativeFuncWrapper<Ret(*)(ObjectData*, Args...), f> {};

  template<typename Cls, typename Ret, typename... Args, auto f>
  struct NativeFuncWrapper<Ret(Cls::*)(Args...) const, f> :
    public NativeFuncWrapper<Ret(Cls::*)(Args...), f> {};

};

} // namespace HPHP
