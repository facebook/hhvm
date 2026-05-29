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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ResponseMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

std::unique_ptr<folly::IOBuf> serializeMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makePayloadFrame(
    std::unique_ptr<folly::IOBuf> metadata = nullptr,
    std::unique_ptr<folly::IOBuf> data = nullptr) {
  auto raw = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1, .follows = false, .complete = true, .next = true},
      std::move(metadata),
      std::move(data));
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(raw));
}

} // namespace

// =============================================================================
// deserializeResponseMetadata
// =============================================================================

TEST(ResponseMetadataTest, DeserializesEmptyMetadata) {
  apache::thrift::ResponseRpcMetadata expected;
  auto frame = makePayloadFrame(serializeMetadata(expected));

  apache::thrift::ResponseRpcMetadata result;
  auto error = deserializeResponseMetadata(frame, result);

  EXPECT_FALSE(error);
}

TEST(ResponseMetadataTest, DeserializesResponsePayloadMetadata) {
  apache::thrift::ResponseRpcMetadata expected;
  expected.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});

  auto frame = makePayloadFrame(serializeMetadata(expected));

  apache::thrift::ResponseRpcMetadata result;
  auto error = deserializeResponseMetadata(frame, result);

  EXPECT_FALSE(error);
  ASSERT_TRUE(result.payloadMetadata().has_value());
  EXPECT_EQ(
      result.payloadMetadata()->getType(),
      apache::thrift::PayloadMetadata::Type::responseMetadata);
}

TEST(ResponseMetadataTest, DeserializeFailsOnGarbageMetadata) {
  auto garbage = folly::IOBuf::copyBuffer("not valid binary protocol!");
  auto frame = makePayloadFrame(std::move(garbage));

  apache::thrift::ResponseRpcMetadata result;
  auto error = deserializeResponseMetadata(frame, result);

  EXPECT_TRUE(error);
  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
}

TEST(ResponseMetadataTest, DeserializeSucceedsWithNoMetadata) {
  // Frame with no metadata section
  auto frame = makePayloadFrame(nullptr, folly::IOBuf::copyBuffer("data"));

  apache::thrift::ResponseRpcMetadata result;
  auto error = deserializeResponseMetadata(frame, result);

  EXPECT_FALSE(error);
}

// =============================================================================
// processPayloadMetadata
// =============================================================================

TEST(ResponseMetadataTest, ProcessPayloadMetadataNoPayload) {
  apache::thrift::ResponseRpcMetadata metadata;
  // No payloadMetadata set

  auto error = processPayloadMetadata(metadata);

  EXPECT_FALSE(error);
}

TEST(ResponseMetadataTest, ProcessPayloadMetadataNormalResponse) {
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});

  auto error = processPayloadMetadata(metadata);

  EXPECT_FALSE(error);
}

TEST(ResponseMetadataTest, ProcessPayloadMetadataDeclaredException) {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::PayloadExceptionMetadata exceptionMeta;
  exceptionMeta.declaredException() =
      apache::thrift::PayloadDeclaredExceptionMetadata{};

  apache::thrift::PayloadExceptionMetadataBase exBase;
  exBase.name_utf8() = "MyException";
  exBase.what_utf8() = "Something went wrong";
  exBase.metadata() = std::move(exceptionMeta);

  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exBase));

  auto error = processPayloadMetadata(metadata);

  // Declared exceptions are not errors
  EXPECT_FALSE(error);

  // Headers should be populated
  auto& otherMeta = *metadata.otherMetadata();
  EXPECT_EQ(
      otherMeta.at(std::string(apache::thrift::detail::kHeaderUex)),
      "MyException");
  EXPECT_EQ(
      otherMeta.at(std::string(apache::thrift::detail::kHeaderUexw)),
      "Something went wrong");
}

TEST(ResponseMetadataTest, ProcessPayloadMetadataUndeclaredException) {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::PayloadExceptionMetadata exceptionMeta;
  exceptionMeta.appUnknownException() =
      apache::thrift::PayloadAppUnknownExceptionMetdata{};

  apache::thrift::PayloadExceptionMetadataBase exBase;
  exBase.name_utf8() = "UnknownEx";
  exBase.what_utf8() = "unexpected error";
  exBase.metadata() = std::move(exceptionMeta);

  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exBase));

  auto error = processPayloadMetadata(metadata);

  // Undeclared exceptions produce an error
  EXPECT_TRUE(error);
  auto* ex = error.get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(std::string(ex->what()), "unexpected error");
}

TEST(ResponseMetadataTest, ProcessPayloadMetadataMissingInnerMetadata) {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::PayloadExceptionMetadataBase exBase;
  exBase.name_utf8() = "BadEx";
  // No metadata() set

  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exBase));

  auto error = processPayloadMetadata(metadata);

  EXPECT_TRUE(error);
  auto* ex = error.get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(std::string(ex->what()), "Missing payload exception metadata");
}

} // namespace apache::thrift::fast_thrift::thrift
