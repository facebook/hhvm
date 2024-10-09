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

#include <variant>
#include <folly/Overload.h>
#include <folly/SpinLock.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/payload/ChecksumPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/DefaultPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/LegacyPayloadSerializerStrategy.h>

namespace apache::thrift::rocket {

/**
 * This class is a facade for the different strategies of serializing payloads.
 * Its to make changing the strategy easier in the future / safe. Hides the
 * strategy from the rest of the code (provides type-erasure) with exposing.
 *
 * For reaons with the existing code this class is accessed as a singleton. It
 * can be overridden by calling the initialize() method if you want to use a
 * different strategy.
 */
class PayloadSerializer {
 private:
  std::variant<
      DefaultPayloadSerializerStrategy,
      LegacyPayloadSerializerStrategy,
      ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>,
      ChecksumPayloadSerializerStrategy<LegacyPayloadSerializerStrategy>>
      strategy_;

  struct PayloadSerializerHolder {
    PayloadSerializerHolder() {}
    ~PayloadSerializerHolder();

    PayloadSerializer& get();

    template <typename Strategy>
    void initialize(Strategy&& strategy) {
      auto* serializer =
          new PayloadSerializer(std::forward<Strategy>(strategy));
      delete serializer_.exchange(serializer, std::memory_order_acq_rel);
    }

    void reset();

   private:
    alignas(folly::hardware_destructive_interference_size)
        std::atomic<PayloadSerializer*> serializer_{nullptr};
  };

 public:
  template <
      typename Strategy,
      typename = std::enable_if_t<
          std::is_base_of_v<PayloadSerializerStrategy<Strategy>, Strategy>>>
  explicit PayloadSerializer(Strategy s) : strategy_(std::move(s)) {}
  /**
   * Lets you override the strategy to one of the supported strategies instead
   * of the default. Must be called before the getInstance() method is called
   * for the first time to take effect. Otherwise, it will be ignored.
   */
  template <typename Strategy>
  static void initialize(Strategy&& strategy) {
    getPayloadSerializerHolder().initialize(std::forward<Strategy>(strategy));
  }
  /**
   * Returns the singleton instance of the PayloadSerializer. Either returns
   * the default strategy or the one that was overridden by the initialize()
   * method.
   */
  static PayloadSerializer& getInstance();

  template <class T>
  folly::Try<T> unpackAsCompressed(Payload&& payload) {
    return visit([&](auto& strategy) {
      return strategy.template unpackAsCompressed<T>(std::move(payload));
    });
  }

  template <class T>
  folly::Try<T> unpack(rocket::Payload&& payload) {
    return visit([&](auto& strategy) {
      return strategy.template unpack<T>(std::move(payload));
    });
  }

  template <typename T>
  std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
    return visit([&](auto& strategy) {
      return strategy.template packCompact<T>(std::forward<T>(data));
    });
  }

  template <typename T>
  size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    return visit([&](auto& strategy) {
      return strategy.template unpackCompact<T>(output, buffer);
    });
  }

  template <typename T>
  size_t unpackCompact(T& output, const folly::io::Cursor& cursor) {
    return visit([&](auto& strategy) {
      return strategy.template unpackCompact<T>(output, cursor);
    });
  }

  template <typename Metadata>
  rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      folly::AsyncTransport* transport) {
    return visit([&](auto& strategy) {
      return strategy.template packWithFds<Metadata>(
          metadata, std::move(payload), std::move(fds), transport);
    });
  }

  template <class PayloadType>
  rocket::Payload pack(
      PayloadType&& payload, folly::AsyncTransport* transport) {
    return visit([&](auto& strategy) {
      return strategy.template pack<PayloadType>(
          std::forward<PayloadType>(payload), transport);
    });
  }

  /**
   * Resets the singleton instance of the PayloadSerializer to empty. This
   * should only be called in tests.
   */
  static void reset();

 private:
  static PayloadSerializerHolder& getPayloadSerializerHolder();

  /**
   * Visits the strategy and calls the delegate function with the strategy as
   * the parameter. Done manually instead of using std::visit for performance.
   */
  template <typename DelegateFunc>
  FOLLY_ALWAYS_INLINE decltype(auto) visit(DelegateFunc&& delegate) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      auto& strategy = std::get<DefaultPayloadSerializerStrategy>(strategy_);
      return delegate(strategy);
    } else if (std::holds_alternative<LegacyPayloadSerializerStrategy>(
                   strategy_)) {
      auto& strategy = std::get<LegacyPayloadSerializerStrategy>(strategy_);
      return delegate(strategy);
    } else if (std::holds_alternative<ChecksumPayloadSerializerStrategy<
                   DefaultPayloadSerializerStrategy>>(strategy_)) {
      auto& strategy = std::get<
          ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>>(
          strategy_);
      return delegate(strategy);
    } else {
      auto& strategy = std::get<
          ChecksumPayloadSerializerStrategy<LegacyPayloadSerializerStrategy>>(
          strategy_);
      return delegate(strategy);
    }
  }
};

} // namespace apache::thrift::rocket
