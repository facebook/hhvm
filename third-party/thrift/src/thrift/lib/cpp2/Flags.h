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
#include <string>
#include <type_traits>

#include <folly/CPortability.h>
#include <folly/Indestructible.h>
#include <folly/Optional.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/experimental/observer/SimpleObservable.h>
#include <folly/synchronization/DelayedInit.h>

#include <thrift/lib/cpp2/PluggableFunction.h>

namespace apache {
namespace thrift {
namespace detail {

class FlagsBackend {
 public:
  virtual ~FlagsBackend() = default;

  virtual folly::observer::Observer<folly::Optional<bool>> getFlagObserverBool(
      folly::StringPiece name) = 0;

  virtual folly::observer::Observer<folly::Optional<int64_t>>
  getFlagObserverInt64(folly::StringPiece name) = 0;

  virtual folly::observer::Observer<folly::Optional<std::string>>
  getFlagObserverString(folly::StringPiece name) = 0;
};

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<FlagsBackend>, createFlagsBackend);

FlagsBackend& getFlagsBackend();

template <typename T>
folly::observer::Observer<folly::Optional<T>> getFlagObserver(
    folly::StringPiece name);

template <>
inline folly::observer::Observer<folly::Optional<bool>> getFlagObserver<bool>(
    folly::StringPiece name) {
  return getFlagsBackend().getFlagObserverBool(name);
}

template <>
inline folly::observer::Observer<folly::Optional<int64_t>>
getFlagObserver<int64_t>(folly::StringPiece name) {
  return getFlagsBackend().getFlagObserverInt64(name);
}

template <>
inline folly::observer::Observer<folly::Optional<std::string>>
getFlagObserver<std::string>(folly::StringPiece name) {
  return getFlagsBackend().getFlagObserverString(name);
}

template <typename T>
class FlagWrapper {
 public:
  FlagWrapper(folly::StringPiece name, T defaultValue)
      : name_(name), defaultValue_(std::move(defaultValue)) {}

  T get() { return get(ensureInit()); }

  folly::observer::Observer<T> observe() { return observe(ensureInit()); }

  // Methods to set mock value for Flags.
  void setMockValue(T value) {
    mockObservable_.setValue(value);
    folly::observer_detail::ObserverManager::waitForAllUpdates();
  }

  void unmock() {
    mockObservable_.setValue(folly::none);
    folly::observer_detail::ObserverManager::waitForAllUpdates();
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
      return folly::observer::makeValueObserver(
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
          });
    });
  }

  folly::DelayedInit<ReadOptimizedObserver<T>> observer_;
  folly::StringPiece name_;
  const T defaultValue_;
  folly::observer::SimpleObservable<folly::Optional<T>> mockObservable_{
      folly::none};
};

} // namespace detail

#define THRIFT_FLAG_DEFINE(_name, _type, _default)                             \
  apache::thrift::detail::FlagWrapper<_type>& THRIFT_FLAG_WRAPPER__##_name() { \
    static constexpr folly::StringPiece flagName = #_name;                     \
    static folly::Indestructible<apache::thrift::detail::FlagWrapper<_type>>   \
        flagWrapper(flagName, _default);                                       \
    return *flagWrapper;                                                       \
  }                                                                            \
  /* This is here just to force a semicolon */                                 \
  apache::thrift::detail::FlagWrapper<_type>& THRIFT_FLAG_WRAPPER__##_name()

#define THRIFT_FLAG_DECLARE(_name, _type) \
  apache::thrift::detail::FlagWrapper<_type>& THRIFT_FLAG_WRAPPER__##_name()

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

} // namespace thrift
} // namespace apache
