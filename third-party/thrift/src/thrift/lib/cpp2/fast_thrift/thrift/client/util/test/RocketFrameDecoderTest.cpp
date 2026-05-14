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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

apache::thrift::ResponseRpcMetadata makePopulatedResponseMetadata() {
  apache::thrift::ResponseRpcMetadata md;
  md.otherMetadata().emplace();
  (*md.otherMetadata())["x-trace-id"] = "abc123";
  return md;
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makePayloadFrame(
    uint32_t streamId, bool complete, bool next) {
  ThriftFirstResponsePayload payload{
      .data = folly::IOBuf::copyBuffer("hello"),
      .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(
          makePopulatedResponseMetadata()),
      .streamId = streamId,
      .complete = complete,
      .next = next};
  auto wire = std::move(payload)
                  .toRocketFrame(
                      ::apache::thrift::fast_thrift::rocket::server::
                          MetadataProtocol::BINARY)
                  .serialize();
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(wire));
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeErrorFrame(
    uint32_t streamId, uint32_t errorCode) {
  apache::thrift::fast_thrift::frame::ComposedErrorFrame frame{
      .data = folly::IOBuf::copyBuffer("err-payload"),
      .metadata = nullptr,
      .header = {.streamId = streamId, .errorCode = errorCode}};
  auto wire = std::move(frame).serialize();
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(wire));
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeCancelFrame(
    uint32_t streamId) {
  apache::thrift::fast_thrift::frame::ComposedCancelFrame frame{
      .header = {.streamId = streamId}};
  auto wire = std::move(frame).serialize();
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(wire));
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeRequestNFrame(
    uint32_t streamId, uint32_t requestN) {
  apache::thrift::fast_thrift::frame::ComposedRequestNFrame frame{
      .header = {.streamId = streamId, .requestN = requestN}};
  auto wire = std::move(frame).serialize();
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(wire));
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeMetadataPushFrame(
    folly::StringPiece bytes) {
  apache::thrift::fast_thrift::frame::ComposedMetadataPushFrame frame{
      .metadata = folly::IOBuf::copyBuffer(bytes), .header = {}};
  auto wire = std::move(frame).serialize();
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(wire));
}

} // namespace

TEST(FromRocketFrameTest, RRPayloadDeserializesAsFirstResponse) {
  auto frame =
      makePayloadFrame(/*streamId=*/7, /*complete=*/true, /*next=*/true);

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  ASSERT_TRUE(result.hasValue());
  ASSERT_TRUE(result->is<ThriftFirstResponsePayload>());

  auto& first = result->get<ThriftFirstResponsePayload>();
  EXPECT_EQ(first.streamId, 7u);
  EXPECT_TRUE(first.complete);
  EXPECT_TRUE(first.next);
  ASSERT_NE(first.metadata, nullptr);
  ASSERT_TRUE(first.metadata->otherMetadata().has_value());
  EXPECT_EQ((*first.metadata->otherMetadata())["x-trace-id"], "abc123");
  ASSERT_NE(first.data, nullptr);
  EXPECT_EQ(first.data->moveToFbString().toStdString(), "hello");
}

TEST(FromRocketFrameTest, ErrorFrameDeserializesAsErrorPayload) {
  auto frame = makeErrorFrame(/*streamId=*/3, /*errorCode=*/42);

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  ASSERT_TRUE(result.hasValue());
  ASSERT_TRUE(result->is<ThriftErrorPayload>());

  auto& err = result->get<ThriftErrorPayload>();
  EXPECT_EQ(err.streamId, 3u);
  EXPECT_EQ(err.errorCode, 42u);
  ASSERT_NE(err.data, nullptr);
  EXPECT_EQ(err.data->moveToFbString().toStdString(), "err-payload");
}

TEST(FromRocketFrameTest, CancelFrameDeserializesAsCancelPayload) {
  auto frame = makeCancelFrame(/*streamId=*/9);

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE);
  ASSERT_TRUE(result.hasValue());
  ASSERT_TRUE(result->is<ThriftCancelPayload>());
  EXPECT_EQ(result->get<ThriftCancelPayload>().streamId, 9u);
}

TEST(FromRocketFrameTest, RequestNFrameDeserializesAsRequestNPayload) {
  auto frame = makeRequestNFrame(/*streamId=*/11, /*requestN=*/64);

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE);
  ASSERT_TRUE(result.hasValue());
  ASSERT_TRUE(result->is<ThriftRequestNPayload>());

  auto& reqN = result->get<ThriftRequestNPayload>();
  EXPECT_EQ(reqN.streamId, 11u);
  EXPECT_EQ(reqN.requestN, 64u);
}

TEST(FromRocketFrameTest, MetadataPushFrameDeserializesAsMetadataPushPayload) {
  auto frame = makeMetadataPushFrame("server-version-bytes");

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  ASSERT_TRUE(result.hasValue());
  ASSERT_TRUE(result->is<ThriftMetadataPushPayload>());

  auto& push = result->get<ThriftMetadataPushPayload>();
  ASSERT_NE(push.metadata, nullptr);
  EXPECT_EQ(
      push.metadata->moveToFbString().toStdString(), "server-version-bytes");
}

TEST(FromRocketFrameTest, PayloadOnNonRRRpcKindReturnsError) {
  auto frame =
      makePayloadFrame(/*streamId=*/1, /*complete=*/false, /*next=*/true);

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE);
  EXPECT_FALSE(result.hasValue());
}

TEST(FromRocketFrameTest, MalformedMetadataReturnsError) {
  // Build a PAYLOAD frame with metadata bytes that aren't valid binary
  // protocol — fromRocketFrame should surface deserialization failure.
  apache::thrift::fast_thrift::frame::ComposedPayloadFrame frame{
      .data = folly::IOBuf::copyBuffer("data"),
      .metadata = folly::IOBuf::copyBuffer("\xFF\xFF\xFF\xFF garbage"),
      .header = {.streamId = 5, .complete = true, .next = true}};
  auto wire = std::move(frame).serialize();
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(wire));

  auto result = fromRocketFrame(
      std::move(parsed),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_FALSE(result.hasValue());
}

} // namespace apache::thrift::fast_thrift::thrift
