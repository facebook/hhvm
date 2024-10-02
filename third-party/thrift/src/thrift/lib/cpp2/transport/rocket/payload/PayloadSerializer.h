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
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/Flags.h>
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
  std::
      variant<DefaultPayloadSerializerStrategy, LegacyPayloadSerializerStrategy>
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
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      return std::get<DefaultPayloadSerializerStrategy>(strategy_)
          .unpackAsCompressed<T>(std::move(payload));
    } else {
      return std::get<LegacyPayloadSerializerStrategy>(strategy_)
          .unpackAsCompressed<T>(std::move(payload));
    }
  }

  template <class T>
  folly::Try<T> unpack(rocket::Payload&& payload) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      return std::get<DefaultPayloadSerializerStrategy>(strategy_).unpack<T>(
          std::move(payload));
    } else {
      return std::get<LegacyPayloadSerializerStrategy>(strategy_).unpack<T>(
          std::move(payload));
    }
  }

  template <typename T>
  std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      return std::get<DefaultPayloadSerializerStrategy>(strategy_).packCompact(
          std::forward<T>(data));
    } else {
      return std::get<LegacyPayloadSerializerStrategy>(strategy_).packCompact(
          std::forward<T>(data));
    }
  }

  template <typename T>
  size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      return std::get<DefaultPayloadSerializerStrategy>(strategy_)
          .unpackCompact(output, buffer);
    } else {
      return std::get<LegacyPayloadSerializerStrategy>(strategy_).unpackCompact(
          output, buffer);
    }
  }

  template <typename Metadata>
  rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      folly::AsyncTransport* transport) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      return std::get<DefaultPayloadSerializerStrategy>(strategy_)
          .packWithFds<Metadata>(
              metadata, std::move(payload), std::move(fds), transport);
    } else {
      return std::get<LegacyPayloadSerializerStrategy>(strategy_)
          .packWithFds<Metadata>(
              metadata, std::move(payload), std::move(fds), transport);
    }
  }
  template <class PayloadType>
  rocket::Payload pack(
      PayloadType&& payload, folly::AsyncTransport* transport) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      return std::get<DefaultPayloadSerializerStrategy>(strategy_)
          .pack<PayloadType>(std::forward<PayloadType>(payload), transport);
    } else {
      return std::get<LegacyPayloadSerializerStrategy>(strategy_)
          .pack<PayloadType>(std::forward<PayloadType>(payload), transport);
    }
  }

 private:
  /**
   * Resets the singleton instance of the PayloadSerializer to empty. This
   * should only be called in tests.
   */
  static void reset();

  static PayloadSerializerHolder& getPayloadSerializerHolder();

  FRIEND_TEST(PayloadSerializerTest, TestPackWithLegacyStrategy);
  FRIEND_TEST(PayloadSerializerTest, TestPackWitDefaultyStrategy);
};

} // namespace apache::thrift::rocket
