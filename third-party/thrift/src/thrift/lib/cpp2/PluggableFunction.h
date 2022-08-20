/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdexcept>

#include <folly/Likely.h>
#include <folly/Portability.h>
#include <folly/Utility.h>
#include <folly/lang/Exception.h>
#include <folly/synchronization/RelaxedAtomic.h>

/**
 * This provides a simple framework for defining functions in core thrift
 * that may be overridden by other modules that get linked in. Note that only
 * one override can be present for each function.
 *
 * Consider the following example:
 *
 *   // MyCoreThriftLibrary.h
 *   THRIFT_PLUGGABLE_FUNC_DECLARE(int, myPluggableFunction, int a, int b);
 *
 *   // MyCoreThriftLibrary.cpp
 *   THRIFT_PLUGGABLE_FUNC_REGISTER(int, myPluggableFunction, int a, int b) {
 *     return a + b;
 *   }
 *
 *   ...
 *
 *   void foo() {
 *     auto result = myPluggableFunction(1, 2);
 *     ...
 *   }
 *
 *   // MyCustomModule.cpp
 *   THRIFT_PLUGGABLE_FUNC_SET(int, myPluggableFunction, int a, int b) {
 *     return a * b;
 *   }
 *
 * If MyCustomModule.cpp is linked in, result in foo() will be 2, otherwise it
 * will be 3.
 */

namespace apache {
namespace thrift {
namespace detail {

template <typename Sig>
class PluggableFunction;

template <typename Ret, typename... Args>
class PluggableFunction<Ret(Args...)> {
 public:
  using signature = Ret(Args...);

  constexpr explicit PluggableFunction(signature& init) noexcept
      : init_{init} {}

  const PluggableFunction& operator=(signature& next) const noexcept {
    if (auto prev = impl_.exchange(&next)) {
      auto msg = prev == &init_
          ? "pluggable function: override after invocation"
          : "pluggable function: override after override";
      folly::terminate_with<std::logic_error>(msg);
    }
    return *this;
  }

  template <typename... A>
  FOLLY_ERASE auto operator()(A&&... a) const
      -> decltype(FOLLY_DECLVAL(signature&)(static_cast<A&&>(a)...)) {
    return choose()(static_cast<A&&>(a)...);
  }

 private:
  FOLLY_ERASE signature* choose() const {
    const auto impl = impl_.load();
    return FOLLY_LIKELY(!!impl) ? impl : choose_slow();
  }

  FOLLY_NOINLINE signature* choose_slow() const {
    auto impl = impl_.load();
    while (!impl) {
      if (impl_.compare_exchange_weak(impl, &init_)) {
        return &init_;
      }
    }
    return impl;
  }

  //  impl_ should be first to avoid extra arithmetic in the fast path
  mutable folly::relaxed_atomic<signature*> impl_{nullptr};
  signature& init_;
};

} // namespace detail
} // namespace thrift
} // namespace apache

#define THRIFT_PLUGGABLE_FUNC_DECLARE(_ret, _name, ...)                       \
  _ret THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name(__VA_ARGS__);                   \
  extern const ::apache::thrift::detail::PluggableFunction<_ret(__VA_ARGS__)> \
      _name

#define THRIFT_PLUGGABLE_FUNC_REGISTER(_ret, _name, ...)                     \
  FOLLY_STORAGE_CONSTEXPR const ::apache::thrift::detail::PluggableFunction< \
      _ret(__VA_ARGS__)>                                                     \
      _name{THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name};                         \
  _ret THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name(__VA_ARGS__)

#define THRIFT_PLUGGABLE_FUNC_SET(_ret, _name, ...)      \
  _ret THRIFT__PLUGGABLE_FUNC_IMPL_##_name(__VA_ARGS__); \
  static bool THRIFT__PLUGGABLE_FUNC_SETTER_##_name =    \
      (_name = THRIFT__PLUGGABLE_FUNC_IMPL_##_name, 0);  \
  _ret THRIFT__PLUGGABLE_FUNC_IMPL_##_name(__VA_ARGS__)
