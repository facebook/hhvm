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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/RequestPayload.h>
#include <thrift/lib/cpp2/transport/rocket/payload/CustomCompressionPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/DefaultPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

void testPackAndUnpackWithCompactProtocol(PayloadSerializer& serializer) {
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  auto payload = serializer.packCompact(metadata);
  EXPECT_GT(payload->computeChainDataLength(), 1);

  RequestRpcMetadata other;
  serializer.unpack<RequestRpcMetadata>(other, payload.get(), false);
  EXPECT_EQ(other, metadata);
  EXPECT_EQ(other.protocol(), ProtocolId::COMPACT);
}

TEST(PayloadSerializerTest, TestPackWithDefaultStrategy) {
  PayloadSerializer::reset();
  PayloadSerializer::initialize(DefaultPayloadSerializerStrategy());
  auto& serializer = *PayloadSerializer::getInstance().get();
  testPackAndUnpackWithCompactProtocol(serializer);
}

TEST(PayloadSerializerTest, TestPackWithoutChecksumUsingFacade) {
  PayloadSerializer::reset();
  PayloadSerializer::initialize(
      ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>());
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  auto payload = PayloadSerializer::getInstance()->packWithFds(
      &metadata,
      folly::IOBuf::copyBuffer("test"),
      folly::SocketFds(),
      false, /* encodeMetadataUsingBinary */
      nullptr);

  auto other = PayloadSerializer::getInstance()->unpack<RequestPayload>(
      std::move(payload), false);
  EXPECT_EQ(other.hasException(), false);
}

TEST(PayloadSerializerTest, TestPtrCoOwnership) {
  std::unique_ptr<PayloadSerializer::Ptr> ptr = nullptr;

  {
    PayloadSerializer::initialize(
        ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>());
    ptr = std::make_unique<PayloadSerializer::Ptr>(
        PayloadSerializer::getInstance());
    testPackAndUnpackWithCompactProtocol(**ptr);
  }

  PayloadSerializer::initialize(
      ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>());

  // **ptr is still valid here, despite the re-initialization
  testPackAndUnpackWithCompactProtocol(**ptr);
}

TEST(PayloadSerializerTest, TestMakeAndNonOwningPtr) {
  std::unique_ptr<PayloadSerializer::Ptr> ptr = nullptr;

  {
    auto ps = PayloadSerializer::make();
    ptr = std::make_unique<PayloadSerializer::Ptr>(ps.getNonOwningPtr());
    // valid here while ps is in scope
    testPackAndUnpackWithCompactProtocol(**ptr);
  }

  // ptr does not own, so it is not valid here
  // testPackAndUnpackWithCompactProtocol(**ptr);
}

struct MyCustomCompressor : public CustomCompressor {
  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer) override {
    return folly::compression::getCodec(folly::compression::CodecType::ZSTD)
        ->compress(buffer.get());
  }

  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer) override {
    return folly::compression::getCodec(folly::compression::CodecType::ZSTD)
        ->uncompress(buffer.get());
  }
};

TEST(PayloadSerializerTest, TestMakeCustomCompression) {
  CustomCompressionPayloadSerializerStrategyOptions options;
  options.compressor = std::make_shared<MyCustomCompressor>();

  auto ps = PayloadSerializer::make<CustomCompressionPayloadSerializerStrategy<
      DefaultPayloadSerializerStrategy>>(options);
  testPackAndUnpackWithCompactProtocol(ps);
}

TEST(PayloadSerializerTest, TestCompressionAndUncompression) {
  if (!folly::kIsLinux) {
    // on non-linux platforms
    return;
  }

  std::vector<std::pair<
      std::unique_ptr<PayloadSerializer>,
      bool /*supports custom compression*/>>
      payloadSerializers;
  payloadSerializers.emplace_back(
      std::make_unique<PayloadSerializer>(DefaultPayloadSerializerStrategy()),
      false);
  payloadSerializers.emplace_back(
      std::make_unique<PayloadSerializer>(ChecksumPayloadSerializerStrategy<
                                          DefaultPayloadSerializerStrategy>()),
      false);

  CustomCompressionPayloadSerializerStrategyOptions options;
  options.compressor = std::make_shared<MyCustomCompressor>();
  payloadSerializers.emplace_back(
      std::make_unique<PayloadSerializer>(
          CustomCompressionPayloadSerializerStrategy<
              DefaultPayloadSerializerStrategy>(options)),
      true);

  std::vector<CompressionAlgorithm> compressionAlgorithms;
  if (folly::kIsApple) {
    compressionAlgorithms = {
        CompressionAlgorithm::NONE,
        CompressionAlgorithm::ZSTD,
        CompressionAlgorithm::ZLIB,
        CompressionAlgorithm::CUSTOM,
    };
  } else {
    compressionAlgorithms = {
        CompressionAlgorithm::NONE,
        CompressionAlgorithm::ZSTD,
        CompressionAlgorithm::ZLIB,
        CompressionAlgorithm::LZ4,
        CompressionAlgorithm::CUSTOM,
    };
  }

  std::string const expected = "hello world";

  for (auto& [ps, supportsCustomCompression] : payloadSerializers) {
    for (const auto& compressionAlgorithm : compressionAlgorithms) {
      auto compressedBuf = ps->compressBuffer(
          folly::IOBuf::fromString(expected), compressionAlgorithm);

      bool compressionIsTrivial = false;
      if (compressionAlgorithm == CompressionAlgorithm::NONE) {
        compressionIsTrivial = true;
      } else if (
          compressionAlgorithm == CompressionAlgorithm::CUSTOM &&
          !supportsCustomCompression) {
        compressionIsTrivial = true;
      }

      if (compressionIsTrivial) {
        EXPECT_EQ(compressedBuf->toString(), expected);
      } else {
        EXPECT_NE(compressedBuf->toString(), expected);
      }

      const auto actual =
          ps->uncompressBuffer(std::move(compressedBuf), compressionAlgorithm);
      EXPECT_EQ(actual->toString(), expected);
    }
  }
}

} // namespace apache::thrift::rocket
