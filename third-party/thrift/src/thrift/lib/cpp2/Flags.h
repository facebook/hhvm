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

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include <folly/CPortability.h>
#include <folly/Conv.h>
#include <folly/Indestructible.h>
#include <folly/observer/Observer.h>
#include <folly/observer/SimpleObservable.h>
#include <folly/settings/Settings.h>
#include <folly/synchronization/DelayedInit.h>

#include <thrift/lib/cpp2/PluggableFunction.h>

namespace apache::thrift {
namespace detail {

class FlagsBackend {
 public:
  virtual ~FlagsBackend() = default;

  virtual folly::observer::Observer<std::optional<bool>> getFlagObserverBool(
      std::string_view name) = 0;

  virtual folly::observer::Observer<std::optional<int64_t>>
  getFlagObserverInt64(std::string_view name) = 0;

  virtual folly::observer::Observer<std::optional<std::string>>
  getFlagObserverString(std::string_view name) = 0;
};

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<FlagsBackend>, createFlagsBackend);

THRIFT_PLUGGABLE_FUNC_DECLARE(void, initThriftFlagFollySettings);

FlagsBackend& getFlagsBackend();

template <typename T>
folly::observer::Observer<std::optional<T>> getFlagObserver(
    std::string_view name);

template <>
inline folly::observer::Observer<std::optional<bool>> getFlagObserver<bool>(
    std::string_view name) {
  return getFlagsBackend().getFlagObserverBool(name);
}

template <>
inline folly::observer::Observer<std::optional<int64_t>>
getFlagObserver<int64_t>(std::string_view name) {
  return getFlagsBackend().getFlagObserverInt64(name);
}

template <>
inline folly::observer::Observer<std::optional<std::string>>
getFlagObserver<std::string>(std::string_view name) {
  return getFlagsBackend().getFlagObserverString(name);
}

template <typename T>
struct TypeName;

template <>
struct TypeName<bool> {
  static constexpr std::string_view name = "bool";
};

template <>
struct TypeName<int64_t> {
  static constexpr std::string_view name = "int64";
};

template <>
struct TypeName<std::string> {
  static constexpr std::string_view name = "string";
};

template <typename F>
struct NamedCreator {
  NamedCreator(std::string name, F&& creatorP)
      : name_(std::move(name)), creator_(std::forward<F>(creatorP)) {}

  auto operator()() { return creator_(); }

  const std::string& getName() const { return name_; }

 private:
  std::string name_;
  F creator_;
};

template <typename T>
class FlagWrapper;

template <typename T>
void registerFlagWrapper(std::string_view name, FlagWrapper<T>* wrapper);

template <typename T>
class FlagWrapper {
 public:
  FlagWrapper(std::string_view name, T defaultValue)
      : name_(name), defaultValue_(std::move(defaultValue)) {
    registerFlagWrapper<T>(name, this);
  }

  T get() { return get(ensureInit()); }

  folly::observer::Observer<T> observe() { return observe(ensureInit()); }

  // Methods to set mock value for Flags.
  void setMockValue(T value) {
    mockObservable_.setValue(value);
    folly::observer_detail::ObserverManager::waitForAllUpdates();
  }

  void unmock() {
    mockObservable_.setValue(std::nullopt);
    folly::observer_detail::ObserverManager::waitForAllUpdates();
  }

  bool hasMockValue() const {
    return mockObservable_.getObserver().getSnapshot()->has_value();
  }

 private:
  template <typename U>
  using ReadOptimizedObserver = std::conditional_t<
      std::is_trivially_copyable_v<U>,
      folly::observer::ReadMostlyAtomicObserver<U>,
      folly::observer::Observer<U>>;

  template <typename U>
  static U get(folly::observer::ReadMostlyAtomicObserver<U>& observer) {
    return *observer;
  }
  template <typename U>
  static U get(folly::observer::Observer<U>& observer) {
    return **observer;
  }

  template <typename U>
  static folly::observer::Observer<U> observe(
      folly::observer::ReadMostlyAtomicObserver<U>& observer) {
    return observer.getUnderlyingObserver();
  }
  template <typename U>
  static folly::observer::Observer<U> observe(
      folly::observer::Observer<U>& observer) {
    return observer;
  }

  ReadOptimizedObserver<T>& ensureInit() {
    return observer_.try_emplace_with([this] {
      return folly::observer::makeValueObserver(NamedCreator(
          fmt::format("THRIFT_FLAG_{}_{}", TypeName<T>::name, name_),
          [overrideObserver = getFlagObserver<T>(name_),
           mockObserver = mockObservable_.getObserver(),
           defaultValue = defaultValue_] {
            auto mockSnapshot = mockObserver.getSnapshot();
            if (*mockSnapshot) {
              return **mockSnapshot;
            }
            auto overrideSnapshot = overrideObserver.getSnapshot();
            if (*overrideSnapshot) {
              return **overrideSnapshot;
            }
            return defaultValue;
          }));
    });
  }

  folly::DelayedInit<ReadOptimizedObserver<T>> observer_;
  std::string_view name_;
  const T defaultValue_;
  folly::observer::SimpleObservable<std::optional<T>> mockObservable_{
      std::nullopt};
};
} // namespace detail

#define THRIFT_FLAG_DEFINE_IMPL(_name, _type, _default)                        \
  apache::thrift::detail::FlagWrapper<_type>& THRIFT_FLAG_WRAPPER__##_name() { \
    static constexpr std::string_view flagName = #_name;                       \
    static folly::Indestructible<apache::thrift::detail::FlagWrapper<_type>>   \
        flagWrapper(flagName, _default);                                       \
    return *flagWrapper;                                                       \
  }                                                                            \
  /* Eagerly register the flag and force a trailing semicolon */               \
  auto& THRIFT_FLAG_REGISTER__##_name = THRIFT_FLAG_WRAPPER__##_name()

#define THRIFT_FLAG_DEFINE(_name, _type, _default)   \
  THRIFT_FLAG_DEFINE_IMPL(_name, _type, _default);   \
  FOLLY_SETTING_REGISTER(                            \
      thriftflag,                                    \
      _name,                                         \
      _type,                                         \
      _default,                                      \
      folly::settings::Mutability::Mutable,          \
      folly::settings::CommandLine::RejectOverrides, \
      "Thrift flag")

#define THRIFT_FLAG_DEFINE_FOLLY_SETTING(_name, _type, _default) \
  THRIFT_FLAG_DEFINE_IMPL(_name, _type, _default);               \
  FOLLY_SETTING_DEFINE(                                          \
      thriftflag,                                                \
      _name,                                                     \
      _type,                                                     \
      _default,                                                  \
      folly::settings::Mutability::Mutable,                      \
      folly::settings::CommandLine::RejectOverrides,             \
      "Thrift flag")

#define THRIFT_FLAG_DECLARE(_name, _type)                                     \
  apache::thrift::detail::FlagWrapper<_type>& THRIFT_FLAG_WRAPPER__##_name(); \
  FOLLY_SETTING_DECLARE(thriftflag, _name, _type)

#define THRIFT_FLAG_DEFINE_int64(_name, _default) \
  THRIFT_FLAG_DEFINE(_name, int64_t, _default)

#define THRIFT_FLAG_DEFINE_bool(_name, _default) \
  THRIFT_FLAG_DEFINE(_name, bool, _default)

#define THRIFT_FLAG_DEFINE_string(_name, _default) \
  THRIFT_FLAG_DEFINE(_name, ::std::string, _default)

#define THRIFT_FLAG_DECLARE_int64(_name) THRIFT_FLAG_DECLARE(_name, int64_t)

#define THRIFT_FLAG_DECLARE_bool(_name) THRIFT_FLAG_DECLARE(_name, bool)

#define THRIFT_FLAG_DECLARE_string(_name) \
  THRIFT_FLAG_DECLARE(_name, ::std::string)

#define THRIFT_FLAG(_name) THRIFT_FLAG_WRAPPER__##_name().get()

#define THRIFT_FLAG_OBSERVE(_name) THRIFT_FLAG_WRAPPER__##_name().observe()

#define THRIFT_FLAG_SET_MOCK(_name, _val) \
  THRIFT_FLAG_WRAPPER__##_name().setMockValue(_val)

#define THRIFT_FLAG_UNMOCK(_name) THRIFT_FLAG_WRAPPER__##_name().unmock()

struct ThriftFlagInfo {
  std::string name;
  std::string currentValue;
  std::optional<bool> isMocked;
};

std::vector<ThriftFlagInfo> getAllThriftFlags(bool returnIsMocked = false);
} // namespace apache::thrift

THRIFT_FLAG_DECLARE_bool(server_header_reject_framed);
THRIFT_FLAG_DECLARE_bool(server_header_reject_unframed);
THRIFT_FLAG_DECLARE_bool(server_header_reject_all);
