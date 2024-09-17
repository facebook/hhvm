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

  template <typename Strategy>
  explicit PayloadSerializer(Strategy s) : strategy_(std::move(s)) {}

 public:
  /**
   * Lets you override the strategy to one of the supported strategies instead
   * of the default. Must be called before the getInstance() method is called
   * for the first time to take effect. Otherwise, it will be ignored.
   */
  template <typename Strategy>
  static void initialize(Strategy strategy) {
    tryInitialize(PayloadSerializer(std::move(strategy)));
  }

  /**
   * Returns the singleton instance of the PayloadSerializer. Either returns
   * the default strategy or the one that was overridden by the initialize()
   * method.
   */
  static PayloadSerializer& getInstance();

  template <class T>
  folly::Try<T> unpackAsCompressed(Payload&& payload, bool useBinary) {
    return folly::variant_match(
        strategy_,
        [payload = std::move(payload), useBinary](auto& strategy) mutable {
          return strategy.template unpackAsCompressed<T>(
              std::move(payload), useBinary);
        });
  }

  template <class T>
  folly::Try<T> unpack(rocket::Payload&& payload, bool useBinary) {
    return folly::variant_match(
        strategy_,
        [payload = std::move(payload),
         useBinary = useBinary](auto& strategy) mutable {
          return strategy.template pack<T>(std::move(payload), useBinary);
        });
  }

  template <typename T>
  std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
    return folly::variant_match(
        strategy_, [data = std::forward<T>(data)](auto& strategy) mutable {
          return strategy.packCompact(std::forward<T>(data));
        });
  }

  template <typename T>
  size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    return folly::variant_match(
        strategy_, [&output, buffer](auto& strategy) mutable {
          return strategy.unpackCompact(output, buffer);
        });
  }

  template <typename Metadata>
  rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      folly::AsyncTransport* transport) {
    return folly::variant_match(
        strategy_,
        [metadata,
         payload = std::move(payload),
         fds = std::move(fds),
         transport](auto& strategy) mutable {
          return strategy.template packWithFds<Metadata>(
              metadata, std::move(payload), std::move(fds), transport);
        });
  }

  template <class PayloadType>
  Payload pack(PayloadType&& payload, folly::AsyncTransport* transport) {
    return folly::variant_match(
        strategy_,
        [payload = std::forward<PayloadType>(payload),
         transport](auto& strategy) mutable {
          return strategy.template pack<PayloadType>(
              std::forward<PayloadType>(payload));
        });
  }

 private:
  /**
   * Resets the singleton instance of the PayloadSerializer to empty. This
   * should only be called in tests.
   */
  static void reset();

  static void tryInitialize(PayloadSerializer&& src);

  template <typename... Args>
  static void tryInitializeEmplace(Args&&... args) {
    tryInitialize(std::forward<Args>(args)...);
  }

  static void tryInitializeDefault();

  FRIEND_TEST(PayloadSerializerTest, TestPackWithLegacyStrategy);
  FRIEND_TEST(PayloadSerializerTest, TestPackWitDefaultyStrategy);
};

} // namespace apache::thrift::rocket
