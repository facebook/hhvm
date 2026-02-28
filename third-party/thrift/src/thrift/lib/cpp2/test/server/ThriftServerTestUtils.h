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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

namespace apache::thrift::test {

enum class TransportType { Header, Rocket };
enum class Compression { Enabled, Disabled, Custom };
enum class ErrorType {
  Overload,
  AppOverload,
  MethodOverload,
  Client,
  Server,
  PreprocessorOverload,
};

class HeaderOrRocketTest : public testing::Test {
 public:
  TransportType transport = TransportType::Rocket;
  Compression compression = Compression::Enabled;

  static constexpr std::string_view kCustomCompressorName = "header_or_rocket";

  auto getTestCaseName() {
    return testing::UnitTest::GetInstance()
        ->current_test_info()
        ->test_case_name();
  }

  auto getCustomCompressorNameForCurrentTestCase() {
    return std::string("header_or_rocket_") + std::string(getTestCaseName());
  }

  auto makeStickyClient(
      ScopedServerInterfaceThread& runner, folly::EventBase* evb) {
    return runner.newStickyClient<TestServiceAsyncClient>(
        evb,
        [&](auto socket) mutable { return makeChannel(std::move(socket)); });
  }
  auto makeClient(ScopedServerInterfaceThread& runner, folly::EventBase* evb) {
    return runner.newClient<apache::thrift::Client<TestService>>(
        evb,
        [&](auto socket) mutable { return makeChannel(std::move(socket)); });
  }

  ClientChannel::Ptr makeChannel(folly::AsyncTransport::UniquePtr socket) {
    auto channel = [&]() -> ClientChannel::Ptr {
      if (transport == TransportType::Header) {
        return HeaderClientChannel::newChannel(
            HeaderClientChannel::WithoutRocketUpgrade{}, std::move(socket));
      } else if (compression == Compression::Custom) {
        RequestSetupMetadata meta;
        auto& custom =
            meta.compressionSetupRequest().ensure().custom().ensure();
        custom.compressorName() = getCustomCompressorNameForCurrentTestCase();
        custom.payload() = "magic_prefix";

        return RocketClientChannel::newChannelWithMetadata(
            std::move(socket), std::move(meta));
      } else {
        return RocketClientChannel::newChannel(std::move(socket));
      }
    }();

    if (compression == Compression::Enabled) {
      apache::thrift::CompressionConfig compressionConfig;
      compressionConfig.codecConfig().ensure().set_zstdConfig();
      channel->setDesiredCompressionConfig(compressionConfig);
    } else if (compression == Compression::Custom) {
      apache::thrift::CompressionConfig compressionConfig;
      compressionConfig.codecConfig().ensure().customConfig().ensure();
      channel->setDesiredCompressionConfig(compressionConfig);
    }
    return channel;
  }
};

} // namespace apache::thrift::test
