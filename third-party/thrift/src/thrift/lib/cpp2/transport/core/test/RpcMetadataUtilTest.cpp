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

#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>

using namespace apache::thrift;
using namespace apache::thrift::detail;

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(
    std::unique_ptr<folly::IOBuf>,
    makeFrameworkMetadata,
    const RpcOptions& rpcOptions,
    folly::dynamic&) {
  EXPECT_EQ(rpcOptions.getShardId(), "123");
  return folly::IOBuf::copyBuffer(std::string("linked"));
}
} // namespace apache::thrift::detail

TEST(RpcMetadataUtil, frameworkMetadata) {
  RpcOptions rpcOptions;
  auto kind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  std::string methodName = "foo";
  std::string serviceName = "bar";
  std::chrono::milliseconds timeout(100);
  std::variant<InteractionCreate, int64_t, std::monostate> interactionHandle =
      std::monostate{};
  transport::THeader header;
  header.setProtocolId(protocol::T_COMPACT_PROTOCOL);

  rpcOptions.setShardId("123");
  MethodMetadata methodMetadata(MethodMetadata::Data(
      methodName, FunctionQualifier::Unspecified, serviceName));
  auto requestRpcMetadata = makeRequestRpcMetadata(
      rpcOptions,
      kind,
      std::move(methodMetadata),
      timeout,
      interactionHandle,
      false,
      3000,
      header,
      nullptr,
      /*customCompressionEnabled=*/false);
  const auto& buf = **requestRpcMetadata.frameworkMetadata();
  std::string content(reinterpret_cast<const char*>(buf.data()), buf.length());
  EXPECT_EQ(content, "linked");
}

TEST(RpcMetadataUtil, interceptorFrameworkMetadata) {
  RpcOptions rpcOptions;
  auto kind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  std::string methodName = "foo";
  std::string serviceName = "bar";
  std::chrono::milliseconds timeout(100);
  std::variant<InteractionCreate, int64_t, std::monostate> interactionHandle =
      std::monostate{};
  transport::THeader header;
  header.setProtocolId(protocol::T_COMPACT_PROTOCOL);

  rpcOptions.setShardId("123");
  MethodMetadata methodMetadata(MethodMetadata::Data(
      methodName, FunctionQualifier::Unspecified, serviceName));
  auto requestRpcMetadata = makeRequestRpcMetadata(
      rpcOptions,
      kind,
      std::move(methodMetadata),
      timeout,
      interactionHandle,
      false,
      3000,
      header,
      folly::IOBuf::copyBuffer(std::string("interceptor_metadata")),
      /*customCompressionEnabled=*/false);
  const auto& buf = **requestRpcMetadata.frameworkMetadata();
  std::string content(reinterpret_cast<const char*>(buf.data()), buf.length());
  EXPECT_EQ(content, "interceptor_metadata");
}

TEST(RpcMetadataUtil, CustomCompressionFallback) {
  RpcOptions rpcOptions;
  auto kind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  std::string methodName = "foo";
  std::string serviceName = "bar";
  std::chrono::milliseconds timeout(100);
  std::variant<InteractionCreate, int64_t, std::monostate> interactionHandle =
      std::monostate{};

  transport::THeader header;
  header.setProtocolId(protocol::T_COMPACT_PROTOCOL);

  {
    CompressionConfig compressionConfig;
    compressionConfig.codecConfig().ensure().customConfig().emplace();
    header.setDesiredCompressionConfig(std::move(compressionConfig));

    MethodMetadata methodMetadata(MethodMetadata::Data(
        methodName, FunctionQualifier::Unspecified, serviceName));
    auto requestRpcMetadata = makeRequestRpcMetadata(
        rpcOptions,
        kind,
        std::move(methodMetadata),
        timeout,
        interactionHandle,
        /*serverZstdSupported=*/false,
        3000,
        header,
        folly::IOBuf::copyBuffer(std::string("")),
        /*customCompressionEnabled=*/false);

    // use zlib if server does not support zstd and custom compression is not
    // supported
    EXPECT_TRUE(requestRpcMetadata.compressionConfig()
                    .value()
                    .codecConfig()
                    ->zlibConfig()
                    .has_value());
    EXPECT_EQ(
        requestRpcMetadata.compression().value(), CompressionAlgorithm::ZLIB);
  }

  {
    CompressionConfig compressionConfig;
    compressionConfig.codecConfig().ensure().customConfig().emplace();
    header.setDesiredCompressionConfig(std::move(compressionConfig));

    MethodMetadata methodMetadata(MethodMetadata::Data(
        methodName, FunctionQualifier::Unspecified, serviceName));
    auto requestRpcMetadata = makeRequestRpcMetadata(
        rpcOptions,
        kind,
        std::move(methodMetadata),
        timeout,
        interactionHandle,
        /*serverZstdSupported=*/true,
        3000,
        header,
        folly::IOBuf::copyBuffer(std::string("")),
        /*customCompressionEnabled=*/false);

    // use zstd if server supports zstd and custom compression is not supported
    EXPECT_TRUE(requestRpcMetadata.compressionConfig()
                    .value()
                    .codecConfig()
                    ->zstdConfig()
                    .has_value());
    EXPECT_EQ(
        requestRpcMetadata.compression().value(), CompressionAlgorithm::ZSTD);
  }

  {
    CompressionConfig compressionConfig;
    compressionConfig.codecConfig().ensure().customConfig().emplace();
    header.setDesiredCompressionConfig(std::move(compressionConfig));

    MethodMetadata methodMetadata(MethodMetadata::Data(
        methodName, FunctionQualifier::Unspecified, serviceName));
    auto requestRpcMetadata = makeRequestRpcMetadata(
        rpcOptions,
        kind,
        std::move(methodMetadata),
        timeout,
        interactionHandle,
        /*serverZstdSupported=*/false,
        3000,
        header,
        folly::IOBuf::copyBuffer(std::string("")),
        /*customCompressionEnabled=*/true);

    // use custom compression if it is supported
    EXPECT_TRUE(requestRpcMetadata.compressionConfig()
                    .value()
                    .codecConfig()
                    ->customConfig()
                    .has_value());
    EXPECT_EQ(
        requestRpcMetadata.compression().value(), CompressionAlgorithm::CUSTOM);
  }

  {
    CompressionConfig compressionConfig;
    compressionConfig.codecConfig().ensure().zstdConfig().emplace();
    header.setDesiredCompressionConfig(std::move(compressionConfig));

    MethodMetadata methodMetadata(MethodMetadata::Data(
        methodName, FunctionQualifier::Unspecified, serviceName));
    auto requestRpcMetadata = makeRequestRpcMetadata(
        rpcOptions,
        kind,
        std::move(methodMetadata),
        timeout,
        interactionHandle,
        /*serverZstdSupported=*/false,
        3000,
        header,
        folly::IOBuf::copyBuffer(std::string("")),
        /*customCompressionEnabled=*/true);

    // even if custom compression is supported, only use it if user requests it
    EXPECT_TRUE(requestRpcMetadata.compressionConfig()
                    .value()
                    .codecConfig()
                    ->zstdConfig()
                    .has_value());
    EXPECT_EQ(
        requestRpcMetadata.compression().value(), CompressionAlgorithm::ZSTD);
  }
}
