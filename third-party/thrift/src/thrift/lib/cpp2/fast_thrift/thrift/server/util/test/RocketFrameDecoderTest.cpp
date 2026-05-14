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
#include <thrift/lib/cpp2/fast_thrift/thrift/common/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RocketFrameDecoder.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

apache::thrift::RequestRpcMetadata makePopulatedRequestMetadata(
    const std::string& method) {
  apache::thrift::RequestRpcMetadata md;
  md.name() = method;
  md.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  md.protocol() = apache::thrift::ProtocolId::COMPACT;
  return md;
}

frame::read::ParsedFrame makeRequestResponseFrame(
    uint32_t streamId,
    apache::thrift::RequestRpcMetadata md,
    std::unique_ptr<folly::IOBuf> data) {
  ThriftRequestResponsePayload payload{
      .data = std::move(data),
      .metadata =
          std::make_unique<apache::thrift::RequestRpcMetadata>(std::move(md))};
  auto wire = std::move(payload)
                  .toRocketFrame(
                      ::apache::thrift::fast_thrift::rocket::server::
                          MetadataProtocol::BINARY)
                  .serialize();
  // The header.streamId in the constructed frame is kInvalidStreamId
  // because toRocketFrame hardcodes it for client outbound. Patch the
  // first 4 bytes so parsing yields the desired streamId.
  auto* p = wire->writableData();
  p[0] = static_cast<uint8_t>(streamId >> 24);
  p[1] = static_cast<uint8_t>(streamId >> 16);
  p[2] = static_cast<uint8_t>(streamId >> 8);
  p[3] = static_cast<uint8_t>(streamId);
  return frame::read::parseFrame(std::move(wire));
}

frame::read::ParsedFrame makeFnfFrame(uint32_t streamId) {
  frame::ComposedRequestFnfFrame frame{
      .data = folly::IOBuf::copyBuffer("fnf"),
      .metadata = nullptr,
      .header = {.streamId = streamId}};
  auto wire = std::move(frame).serialize();
  return frame::read::parseFrame(std::move(wire));
}

frame::read::ParsedFrame makeKeepAliveFrame() {
  frame::ComposedKeepAliveFrame frame{.header = {}};
  auto wire = std::move(frame).serialize();
  return frame::read::parseFrame(std::move(wire));
}

} // namespace

TEST(FromRocketFrameTest, RequestResponseDecodesToTypedPayload) {
  auto frame = makeRequestResponseFrame(
      /*streamId=*/7,
      makePopulatedRequestMetadata("Service.method"),
      folly::IOBuf::copyBuffer("hello"));

  auto result = fromRocketFrame(
      std::move(frame),
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
  ASSERT_TRUE(result.hasValue());
  ASSERT_TRUE(result->is<ThriftRequestResponsePayload>());

  auto& rr = result->get<ThriftRequestResponsePayload>();
  ASSERT_NE(rr.metadata, nullptr);
  ASSERT_TRUE(rr.metadata->name().has_value());
  EXPECT_EQ(rr.metadata->name()->view(), "Service.method");
  ASSERT_NE(rr.data, nullptr);
  EXPECT_EQ(rr.data->moveToFbString().toStdString(), "hello");
}

TEST(FromRocketFrameTest, RequestResponseWithNoMetadataYieldsEmptyMetadata) {
  ThriftRequestResponsePayload payload{
      .data = folly::IOBuf::copyBuffer("body"),
      .metadata = std::make_unique<apache::thrift::RequestRpcMetadata>()};
  auto wire = std::move(payload)
                  .toRocketFrame(
                      ::apache::thrift::fast_thrift::rocket::server::
                          MetadataProtocol::BINARY)
                  .serialize();
  auto parsed = frame::read::parseFrame(std::move(wire));

  auto result = fromRocketFrame(
      std::move(parsed),
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
  ASSERT_TRUE(result.hasValue());
  auto& rr = result->get<ThriftRequestResponsePayload>();
  ASSERT_NE(rr.metadata, nullptr);
  EXPECT_FALSE(rr.metadata->name().has_value());
}

TEST(FromRocketFrameTest, MalformedMetadataReturnsError) {
  // Replace the metadata bytes with garbage that won't parse as Binary
  // protocol RequestRpcMetadata.
  frame::ComposedRequestResponseFrame frame{
      .data = folly::IOBuf::copyBuffer("data"),
      .metadata = folly::IOBuf::copyBuffer("\xFF\xFF\xFF\xFF garbage"),
      .header = {.streamId = 5}};
  auto wire = std::move(frame).serialize();
  auto parsed = frame::read::parseFrame(std::move(wire));

  auto result = fromRocketFrame(
      std::move(parsed),
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
  EXPECT_FALSE(result.hasValue());
}

TEST(FromRocketFrameTest, RequestFnfNotYetWired) {
  auto result = fromRocketFrame(
      makeFnfFrame(/*streamId=*/3),
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
  EXPECT_FALSE(result.hasValue());
}

TEST(FromRocketFrameTest, UnexpectedFrameTypeReturnsError) {
  auto result = fromRocketFrame(
      makeKeepAliveFrame(),
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
  EXPECT_FALSE(result.hasValue());
}

} // namespace apache::thrift::fast_thrift::thrift
