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

#include <atomic>
#include <functional>
#include <type_traits>

#include <folly/Optional.h>
#include <folly/SharedMutex.h>
#include <folly/experimental/observer/SimpleObservable.h>
#include <folly/synchronization/DelayedInit.h>

namespace apache {
namespace thrift {

/*
 * ServerAttribute provides a mechanism for setting values which have varying
 * precedence depending on who set it. The resolved value (`.get()`)
 * prioritizes the value in the following order, falling back to the next one
 * if the value is reset:
 *   1. explicit application override through legacy BaseThriftServer setters
 *   2. baseline from configuration mechanism
 *   3. default provided in constructor, or through ThriftServerInitialConfig
 */

// source of a server's attribute, precedence takes place in descending order
// (APP will override CONF). see comment on ServerAttribute to learn more
enum class AttributeSource : uint32_t {
  OVERRIDE, // when set directly in application code
  BASELINE, // e.g., may come from external configuration mechanism
};

/**
 * A thread-safe, dynamic ServerAttribute which uses folly::observer internally
 * but reads are cached via `folly::observer::AtomicObserver`.
 */
template <typename T>
struct ServerAttributeAtomic;

/**
 * A thread-safe, dynamic ServerAttribute which uses folly::observer internally
 * but reads are cached via `folly::observer::TLObserver`.
 */
template <typename T>
struct ServerAttributeThreadLocal;

/**
 * A thread-safe, dynamic ServerAttribute of suitable type. Dynamic
 * ServerAttribute's can change after the server has already begun serving.
 */
template <typename T>
using ServerAttributeDynamic = std::conditional_t<
    sizeof(T) <= sizeof(std::uint64_t) && std::is_trivially_copyable<T>::value,
    ServerAttributeAtomic<T>,
    ServerAttributeThreadLocal<T>>;

/**
 * A static ServerAttribute without thread-safety. Static ServerAttribute's
 * cannot change after the server has started. This is suitable for properties
 * which are hard to change at runtime (such as number of threads in a
 * thread-pool).
 * These attributes should be set in the main thread before the server starts.
 * As such, there is no need for synchronization.
 */
template <typename T>
struct ServerAttributeStatic;

namespace detail {

template <typename T>
struct ServerAttributeRawValues {
  T baseline_;
  T override_;

  template <typename U, typename V>
  ServerAttributeRawValues(U&& baseline, V&& override)
      : baseline_(std::forward<U>(baseline)),
        override_(std::forward<V>(override)) {}

  T& choose(AttributeSource source) {
    return source == AttributeSource::OVERRIDE ? override_ : baseline_;
  }
};

template <typename T>
T& mergeServerAttributeRawValues(
    std::optional<T>& override, std::optional<T>& baseline, T& defaultValue) {
  return override ? *override : baseline ? *baseline : defaultValue;
}

template <typename T>
struct ServerAttributeObservable {
  explicit ServerAttributeObservable(T defaultValue)
      : ServerAttributeObservable(
            folly::observer::makeStaticObserver<T>(std::move(defaultValue))) {}
  explicit ServerAttributeObservable(folly::observer::Observer<T> defaultValue)
      : default_(std::move(defaultValue)) {}
  void setDefault(folly::observer::Observer<T> defaultValue) {
    default_.setValue(std::move(defaultValue));
  }
  T get() const { return **getObserver(); }

  const folly::observer::Observer<T>& getObserver() const {
    return mergedObserver_.try_emplace_with([&] {
      return folly::observer::makeObserver(
          [overrideObserver = rawValues_.override_.getObserver(),
           baselineObserver = rawValues_.baseline_.getObserver(),
           defaultObserver =
               default_.getObserver()]() mutable -> std::shared_ptr<T> {
            std::optional<T> override = **overrideObserver;
            std::optional<T> baseline = **baselineObserver;
            T defaultValue = **defaultObserver;
            return std::make_shared<T>(
                apache::thrift::detail::mergeServerAttributeRawValues(
                    override, baseline, defaultValue));
          });
    });
  }

  void set(
      folly::observer::Observer<std::optional<T>> value,
      AttributeSource source) {
    rawValues_.choose(source).setValue(std::move(value));
    if (source == AttributeSource::OVERRIDE) {
      // For backward compatibility reasons, we need to block until the observer
      // value is updated. This ensures that reads always produce the latest
      // value after a write from the writer thread. We need to be careful and
      // only block if the source is an OVERRIDE, which should only happen from
      // the main thread before the server has started.
      folly::observer_detail::ObserverManager::waitForAllUpdates();
    }
  }

 protected:
  ServerAttributeRawValues<folly::observer::SimpleObservable<
      folly::observer::Observer<std::optional<T>>>>
      rawValues_{
          folly::observer::makeStaticObserver<std::optional<T>>(std::nullopt),
          folly::observer::makeStaticObserver<std::optional<T>>(std::nullopt)};
  folly::observer::SimpleObservable<folly::observer::Observer<T>> default_;
  mutable folly::DelayedInit<folly::observer::Observer<T>> mergedObserver_;
};

} // namespace detail

template <typename T>
struct ServerAttributeAtomic
    : private apache::thrift::detail::ServerAttributeObservable<T> {
  using apache::thrift::detail::ServerAttributeObservable<
      T>::ServerAttributeObservable;
  using apache::thrift::detail::ServerAttributeObservable<T>::set;
  using apache::thrift::detail::ServerAttributeObservable<T>::getObserver;
  using apache::thrift::detail::ServerAttributeObservable<T>::setDefault;

  T get() const { return *getAtomicObserver(); }

  const folly::observer::AtomicObserver<T>& getAtomicObserver() const {
    return atomicObserver_.try_emplace(getObserver());
  }

 private:
  mutable folly::DelayedInit<folly::observer::AtomicObserver<T>>
      atomicObserver_;
};

template <typename T>
struct ServerAttributeThreadLocal
    : private apache::thrift::detail::ServerAttributeObservable<T> {
  using apache::thrift::detail::ServerAttributeObservable<
      T>::ServerAttributeObservable;
  using apache::thrift::detail::ServerAttributeObservable<T>::set;
  using apache::thrift::detail::ServerAttributeObservable<T>::getObserver;
  using apache::thrift::detail::ServerAttributeObservable<T>::setDefault;

  const T& get() const { return **getTLObserver(); }

  const folly::observer::TLObserver<T>& getTLObserver() const {
    return tlObserver_.try_emplace(getObserver());
  }

 private:
  mutable folly::DelayedInit<folly::observer::TLObserver<T>> tlObserver_;
};

template <typename T>
struct ServerAttributeStatic {
  explicit ServerAttributeStatic(T defaultValue)
      : default_{std::move(defaultValue)}, merged_{default_} {}

  void set(T value, AttributeSource source) {
    rawValues_.choose(source) = value;
    updateMergedValue();
  }
  void reset(AttributeSource source) {
    rawValues_.choose(source) = std::nullopt;
    updateMergedValue();
  }

  void setDefault(T value) { default_ = value; }
  const T& get() const { return merged_.get(); }

 protected:
  void updateMergedValue() {
    auto& merged = apache::thrift::detail::mergeServerAttributeRawValues(
        rawValues_.override_, rawValues_.baseline_, default_);
    merged_ = std::cref(merged);
  }

  apache::thrift::detail::ServerAttributeRawValues<std::optional<T>> rawValues_{
      std::nullopt, std::nullopt};
  T default_;
  std::reference_wrapper<const T> merged_;
};

} // namespace thrift
} // namespace apache
