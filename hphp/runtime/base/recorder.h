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
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <folly/Likely.h>

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

struct Recorder {
  void onSessionInit();
  void onSessionExit();
  static void setEntryPoint(std::string entryPoint);

  template<typename F, F f>
  static auto wrapNativeFunc(const String& name) {
    using Wrapper = NativeFuncWrapper<F, f>;
    getNativeFuncNames()[Wrapper::id] = name.toCppString();
    return Wrapper::record;
  }

 private:
  using NativeFuncId = std::uintptr_t;
  using NativeFuncNames = std::unordered_map<NativeFuncId, std::string>;
  using NativeCall = std::pair<Variant, req::vector<Variant>>;
  using NativeCalls = req::vector_map<NativeFuncId, req::vector<NativeCall>>;

  static NativeFuncNames& getNativeFuncNames();
  static Recorder& getRecorder();
  Variant toVariant() const;
  template<typename T> static Variant toVariant(T value);

  bool m_enabled{false};
  std::string m_entryPoint;
  NativeCalls m_nativeCalls;

  template<typename F, F f>
  struct NativeFuncWrapper;

  template<typename Ret, typename... Args, Ret(*f)(Args...)>
  struct NativeFuncWrapper<Ret(*)(Args...), f> {
    static Ret record(Args... args) {
      auto& recorder{getRecorder()};
      if constexpr (std::is_void_v<Ret>) {
        f(std::forward<Args>(args)...);
        if (UNLIKELY(recorder.m_enabled)) {
          req::vector<Variant> vargs{toVariant(std::forward<Args>(args))...};
          recorder.m_nativeCalls[id].emplace_back(toVariant(nullptr), vargs);
        }
      } else {
        const auto ret{f(std::forward<Args>(args)...)};
        if (UNLIKELY(recorder.m_enabled)) {
          req::vector<Variant> vargs{toVariant(std::forward<Args>(args))...};
          recorder.m_nativeCalls[id].emplace_back(toVariant(ret), vargs);
        }
        return ret;
      }
    }
    inline static const auto id{reinterpret_cast<NativeFuncId>(record)};
  };

  template<auto m>
  struct MethodToFunc;

  template<typename Cls, typename Ret, typename... Args, Ret(Cls::*m)(Args...)>
  struct MethodToFunc<m> {
    static constexpr auto f = +[](ObjectData* object, Args... args) {
      union MethodToFuncUnion {
        Ret(Cls::*_)(Args...);
        Ret(*f)(ObjectData*, Args...);
      };
      static const auto f{MethodToFuncUnion{m}.f};
      return f(object, std::forward<Args>(args)...);
    };
  };

  template<typename Cls, typename Ret, typename... Args,
           Ret(Cls::*m)(Args...) const>
  struct MethodToFunc<m> {
    static constexpr auto f = +[](ObjectData* object, Args... args) {
      union MethodToFuncUnion {
        Ret(Cls::*_)(Args...) const;
        Ret(*f)(ObjectData*, Args...);
      };
      static const auto f{MethodToFuncUnion{m}.f};
      return f(object, std::forward<Args>(args)...);
    };
  };

  template<typename Cls, typename Ret, typename... Args, Ret(Cls::*m)(Args...)>
  struct NativeFuncWrapper<Ret(Cls::*)(Args...), m> :
    public NativeFuncWrapper<Ret(*)(ObjectData*, Args...), MethodToFunc<m>::f>
  {};

  template<typename Cls, typename Ret, typename... Args,
           Ret(Cls::*m)(Args...) const>
  struct NativeFuncWrapper<Ret(Cls::*)(Args...) const, m> :
    public NativeFuncWrapper<Ret(*)(ObjectData*, Args...), MethodToFunc<m>::f>
  {};

};

} // namespace HPHP
