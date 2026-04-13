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

TEST(PayloadSerializerTest, TestSkipCompressionPreservesBuffer) {
  if (!folly::kIsLinux) {
    return;
  }

  // Test that packWithFds with skipCompression=true preserves the buffer
  // unchanged while still writing the compression algorithm into metadata.
  // This is used by the SR Proxy pass-through path: the payload is already
  // compressed, so we set metadata.compression for the receiver but skip
  // re-compressing the buffer.

  PayloadSerializer::reset();
  PayloadSerializer::initialize(DefaultPayloadSerializerStrategy());
  auto& serializer = *PayloadSerializer::getInstance().get();

  std::string const plaintext = "hello world - test payload for compression";

  // First, compress the buffer to simulate a pre-compressed payload
  auto preCompressed = serializer.compressBuffer(
      folly::IOBuf::fromString(plaintext), CompressionAlgorithm::ZSTD);
  auto preCompressedCopy = preCompressed->clone();

  // Pack with skipCompression=true: metadata says ZSTD, but buffer is NOT
  // re-compressed
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  metadata.compression() = CompressionAlgorithm::ZSTD;
  auto payload = serializer.packWithFds(
      &metadata,
      std::move(preCompressed),
      folly::SocketFds(),
      false, /* encodeMetadataUsingBinary */
      nullptr, /* transport */
      nullptr, /* ioBufFactory */
      true /* skipCompression */);

  // Unpack the payload -- the receiver sees metadata.compression=ZSTD and
  // will decompress the buffer
  auto unpacked = serializer.unpack<RequestPayload>(std::move(payload), false);
  ASSERT_FALSE(unpacked.hasException());
  auto& result = unpacked.value();

  // The metadata should indicate ZSTD compression
  EXPECT_TRUE(result.metadata.compression().has_value());
  EXPECT_EQ(*result.metadata.compression(), CompressionAlgorithm::ZSTD);

  // The unpacked payload should be the pre-compressed bytes (since the
  // receiver's unpack sees compression=ZSTD and decompresses)
  // So we should get back the original plaintext
  EXPECT_EQ(result.payload->toString(), plaintext);

  // Now do the same WITHOUT skipCompression (normal path) -- the buffer
  // would be double-compressed and decompression would yield garbage or fail
  auto preCompressed2 = serializer.compressBuffer(
      folly::IOBuf::fromString(plaintext), CompressionAlgorithm::ZSTD);

  RequestRpcMetadata metadata2;
  metadata2.protocol() = ProtocolId::COMPACT;
  metadata2.compression() = CompressionAlgorithm::ZSTD;
  auto payload2 = serializer.packWithFds(
      &metadata2,
      std::move(preCompressed2),
      folly::SocketFds(),
      false,
      nullptr,
      nullptr,
      false /* skipCompression = false, will double-compress */);

  auto unpacked2 =
      serializer.unpack<RequestPayload>(std::move(payload2), false);
  ASSERT_FALSE(unpacked2.hasException());

  // Double-compressed then single-decompressed: result should NOT be plaintext
  EXPECT_NE(unpacked2.value().payload->toString(), plaintext);
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
