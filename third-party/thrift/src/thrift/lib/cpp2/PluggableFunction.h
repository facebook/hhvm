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

#include <mutex>
#include <stdexcept>
#include <unordered_set>

#include <fmt/format.h>
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

namespace apache::thrift::detail {

class TestOnlyFunctionRegistry {
 public:
  static bool isTestOnly(const char* name) {
    return getInstance().isTestOnlyImpl(name);
  }

  static void markAsTestOnly(const char* name) {
    getInstance().markAsTestOnlyImpl(name);
  }

 private:
  FOLLY_EXPORT static TestOnlyFunctionRegistry& getInstance() {
    static TestOnlyFunctionRegistry instance;
    return instance;
  }

  TestOnlyFunctionRegistry() = default;

  bool isTestOnlyImpl(const char* name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return testOnlyFunctions_.find(name) != testOnlyFunctions_.end();
  }

  void markAsTestOnlyImpl(const char* name) {
    std::lock_guard<std::mutex> lock(mutex_);
    testOnlyFunctions_.insert(name);
  }

  mutable std::mutex mutex_;
  std::unordered_set<std::string> testOnlyFunctions_;
};

// Helper function to mark a function as test-only
inline void markFunctionAsTestOnly(const char* name) {
  TestOnlyFunctionRegistry::markAsTestOnly(name);
}

template <typename Sig>
class PluggableFunction;

template <typename Ret, typename... Args>
class PluggableFunction<Ret(Args...)> {
 public:
  using signature = Ret(Args...);

  constexpr explicit PluggableFunction(
      signature& init, const char* name, bool allowLateOverride) noexcept
      : init_{init}, name_{name}, allowLateOverride_(allowLateOverride) {}

  const PluggableFunction& operator=(signature& next) const noexcept {
    if (auto prev = impl_.exchange(&next)) {
      enum class Reason { OVERRIDE_AFTER_INVOCATION, OVERRIDE_AFTER_OVERRIDE };
      Reason reason = prev == &init_ ? Reason::OVERRIDE_AFTER_INVOCATION
                                     : Reason::OVERRIDE_AFTER_OVERRIDE;
      const char* reasonText = [&]() {
        switch (reason) {
          case Reason::OVERRIDE_AFTER_INVOCATION:
            return " pluggable function: override after invocation";
          case Reason::OVERRIDE_AFTER_OVERRIDE:
            return " pluggable function: override after override";
        }
      }();
      // Allow overrides if they're coming from test functions or if late
      // overrides are allowed
      if (!(allowLateOverride_ &&
            reason == Reason::OVERRIDE_AFTER_INVOCATION) &&
          !(TestOnlyFunctionRegistry::isTestOnly(name_) &&
            reason == Reason::OVERRIDE_AFTER_OVERRIDE)) {
        std::string msg = fmt::format("[{}] {}", name_, reasonText);
        folly::terminate_with<std::logic_error>(msg);
      }
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
  const char* name_;
  const bool allowLateOverride_;
};

} // namespace apache::thrift::detail

#define THRIFT_PLUGGABLE_FUNC_DECLARE(_ret, _name, ...)                       \
  _ret THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name(__VA_ARGS__);                   \
  extern const ::apache::thrift::detail::PluggableFunction<_ret(__VA_ARGS__)> \
      _name

#define THRIFT_PLUGGABLE_FUNC_REGISTER(_ret, _name, ...)                     \
  FOLLY_STORAGE_CONSTEXPR const ::apache::thrift::detail::PluggableFunction< \
      _ret(__VA_ARGS__)>                                                     \
      _name{THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name, #_name, false};          \
  _ret THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name(__VA_ARGS__)

#define THRIFT_PLUGGABLE_FUNC_REGISTER_ALLOW_LATE_OVERRIDE(_ret, _name, ...) \
  FOLLY_STORAGE_CONSTEXPR const ::apache::thrift::detail::PluggableFunction< \
      _ret(__VA_ARGS__)>                                                     \
      _name{THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name, #_name, true};           \
  _ret THRIFT__PLUGGABLE_FUNC_DEFAULT_##_name(__VA_ARGS__)

#define THRIFT_PLUGGABLE_FUNC_SET(_ret, _name, ...)      \
  _ret THRIFT__PLUGGABLE_FUNC_IMPL_##_name(__VA_ARGS__); \
  static bool THRIFT__PLUGGABLE_FUNC_SETTER_##_name =    \
      (_name = THRIFT__PLUGGABLE_FUNC_IMPL_##_name, 0);  \
  _ret THRIFT__PLUGGABLE_FUNC_IMPL_##_name(__VA_ARGS__)

/**
 * This is a test-friendly version of THRIFT_PLUGGABLE_FUNC_SET that
 * overrides any existing implementation, including previously set ones.
 * This is useful in tests where we need to ensure our test implementation
 * is used regardless of any existing implementations.
 *
 * When used, this macro will define the implementation function and
 * forcibly set it as the active implementation, overriding any existing ones.
 *
 * Additionally, this macro marks the function as "test-only", which prevents
 * it from being overridden by other functions using THRIFT_PLUGGABLE_FUNC_SET.
 */
#define THRIFT_PLUGGABLE_FUNC_SET_TEST(_ret, _name, ...)                 \
  _ret THRIFT__PLUGGABLE_FUNC_IMPL_##_name(__VA_ARGS__);                 \
  static bool THRIFT__PLUGGABLE_FUNC_TEST_MARKER_##_name =               \
      (::apache::thrift::detail::markFunctionAsTestOnly(#_name), false); \
  static bool THRIFT__PLUGGABLE_FUNC_SETTER_##_name =                    \
      (_name = THRIFT__PLUGGABLE_FUNC_IMPL_##_name, 0);                  \
  _ret THRIFT__PLUGGABLE_FUNC_IMPL_##_name(__VA_ARGS__)
