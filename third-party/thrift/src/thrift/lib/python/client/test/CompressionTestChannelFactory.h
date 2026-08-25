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
#include <utility>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/python/client/RequestChannel.h>

namespace apache::thrift::python::client::test {

// Request ZSTD on a real Rocket channel so the offload flag leaves compressed
// response bytes for the caller-thread hooks under test.
class CompressionTestChannelFactory final : public ChannelFactory {
 public:
  folly::Future<apache::thrift::RequestChannel::Ptr> createThriftChannelTCP(
      const std::string& host,
      const std::string& httpHost,
      uint16_t port,
      uint32_t connectTimeout,
      std::optional<uint32_t> channelTimeout,
      CLIENT_TYPE clientType,
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      const std::string& endpoint,
      int32_t keepAliveTimeoutMs) override {
    return delegate_
        .createThriftChannelTCP(
            host,
            httpHost,
            port,
            connectTimeout,
            channelTimeout,
            clientType,
            protocol,
            endpoint,
            keepAliveTimeoutMs)
        .thenValue([](apache::thrift::RequestChannel::Ptr channel) {
          auto* rocket =
              dynamic_cast<apache::thrift::RocketClientChannel*>(channel.get());
          if (rocket == nullptr) {
            throw std::runtime_error(
                "CompressionTestChannelFactory requires a Rocket channel");
          }
          apache::thrift::CompressionConfig config;
          config.codecConfig().ensure().set_zstdConfig();
          rocket->setDesiredCompressionConfig(config);
          return channel;
        });
  }

  [[noreturn]] folly::Future<apache::thrift::RequestChannel::Ptr>
  createThriftChannelUnix(
      const std::string& /* path */,
      uint32_t /* connectTimeout */,
      std::optional<uint32_t> /* channelTimeout */,
      CLIENT_TYPE /* clientType */,
      apache::thrift::protocol::PROTOCOL_TYPES /* protocol */,
      int32_t /* keepAliveTimeoutMs */) override {
    throw std::runtime_error("CompressionTestChannelFactory supports TCP only");
  }

  [[noreturn]] folly::Future<apache::thrift::RequestChannel::Ptr>
  createThriftChannelSSL(
      const std::shared_ptr<folly::SSLContext>& /* context */,
      const std::string& /* host */,
      const std::string& /* httpHost */,
      uint16_t /* port */,
      uint32_t /* connectTimeout */,
      uint32_t /* sslTimeout */,
      std::optional<uint32_t> /* channelTimeout */,
      CLIENT_TYPE /* clientType */,
      apache::thrift::protocol::PROTOCOL_TYPES /* protocol */,
      const std::string& /* endpoint */,
      int32_t /* keepAliveTimeoutMs */) override {
    throw std::runtime_error("CompressionTestChannelFactory supports TCP only");
  }

 private:
  DefaultChannelFactory delegate_;
};

inline void setCompressionOffloadEnabled(bool enabled) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, enabled);
}

inline void resetCompressionOffloadFlag() {
  THRIFT_FLAG_UNMOCK(thrift_client_compress_request_on_cpu);
}

} // namespace apache::thrift::python::client::test
