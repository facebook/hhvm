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

/*
 * ComposedFrameTest validates the composed-frame struct API:
 *
 *   - streamId(): guards against field-typo bugs (e.g.
 *     ComposedRequestNFrame accidentally returning header.requestN
 *     instead of header.streamId), and asserts that connection-level
 *     frames (Setup/KeepAlive/MetadataPush) report 0 per RSocket spec.
 *
 *   - serialize() &&: guards against forwarder argument-swap bugs (e.g.
 *     data/metadata reversed) by comparing wire bytes against a direct
 *     call to the matching write::serialize(header, ...) worker.
 *
 * The header-based workers themselves are exhaustively tested in
 * FrameWriterTest.cpp; this file only validates the composed-frame
 * wrappers.
 */

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

#include <string>

using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::write;

namespace {

// Coalesce an IOBuf chain into a flat string for byte-equality comparison.
// We only care that wire bytes match, not that the chain shape matches.
std::string flatten(const folly::IOBuf& buf) {
  auto coalesced = buf.clone();
  coalesced->coalesce();
  return std::string(
      reinterpret_cast<const char*>(coalesced->data()), coalesced->length());
}

} // namespace

// ============================================================================
// streamId() — one test per composed-frame type
// ============================================================================

// --- Stream-scoped frames: streamId() returns header.streamId ---

TEST(ComposedFrameTest, RequestResponseStreamId) {
  ComposedRequestResponseFrame frame{.header = {.streamId = 11}};
  EXPECT_EQ(frame.streamId(), 11u);
}

TEST(ComposedFrameTest, RequestFnfStreamId) {
  ComposedRequestFnfFrame frame{.header = {.streamId = 22}};
  EXPECT_EQ(frame.streamId(), 22u);
}

TEST(ComposedFrameTest, RequestStreamStreamId) {
  ComposedRequestStreamFrame frame{
      .header = {.streamId = 33, .initialRequestN = 1}};
  EXPECT_EQ(frame.streamId(), 33u);
}

TEST(ComposedFrameTest, RequestChannelStreamId) {
  ComposedRequestChannelFrame frame{
      .header = {.streamId = 44, .initialRequestN = 1}};
  EXPECT_EQ(frame.streamId(), 44u);
}

TEST(ComposedFrameTest, RequestNStreamId) {
  // Header has both streamId and requestN; this guards against
  // accidentally returning the wrong field.
  ComposedRequestNFrame frame{.header = {.streamId = 55, .requestN = 999}};
  EXPECT_EQ(frame.streamId(), 55u);
}

TEST(ComposedFrameTest, CancelStreamId) {
  ComposedCancelFrame frame{.header = {.streamId = 66}};
  EXPECT_EQ(frame.streamId(), 66u);
}

TEST(ComposedFrameTest, PayloadStreamId) {
  ComposedPayloadFrame frame{.header = {.streamId = 77}};
  EXPECT_EQ(frame.streamId(), 77u);
}

TEST(ComposedFrameTest, ErrorStreamId) {
  // Header has both streamId and errorCode; guard against returning
  // errorCode by mistake.
  ComposedErrorFrame frame{.header = {.streamId = 88, .errorCode = 0x201}};
  EXPECT_EQ(frame.streamId(), 88u);
}

TEST(ComposedFrameTest, ExtStreamId) {
  // Header has streamId and extendedType; guard against field swap.
  ComposedExtFrame frame{.header = {.streamId = 99, .extendedType = 1}};
  EXPECT_EQ(frame.streamId(), 99u);
}

// --- Connection-level frames: streamId() returns 0 per RSocket spec ---

TEST(ComposedFrameTest, KeepAliveStreamIdIsZero) {
  // KeepAlive header has lastReceivedPosition (uint64_t) — must NOT
  // surface that as the stream id.
  ComposedKeepAliveFrame frame{.header = {.lastReceivedPosition = 12345}};
  EXPECT_EQ(frame.streamId(), 0u);
}

TEST(ComposedFrameTest, SetupStreamIdIsZero) {
  // Setup header has several uint32 fields; guard against returning any
  // of them as a stream id.
  ComposedSetupFrame frame{
      .header = {.majorVersion = 1, .keepaliveTime = 100, .maxLifetime = 200}};
  EXPECT_EQ(frame.streamId(), 0u);
}

TEST(ComposedFrameTest, MetadataPushStreamIdIsZero) {
  ComposedMetadataPushFrame frame{};
  EXPECT_EQ(frame.streamId(), 0u);
}

// ============================================================================
// serialize() — one test per composed-frame type
// ============================================================================
//
// Catches forwarder bugs (notably data/metadata argument swap) by
// asserting byte-equivalence with a direct call to the worker.

TEST(ComposedFrameTest, RequestResponseSerializeMatchesWorker) {
  RequestResponseHeader header{.streamId = 1, .follows = true};
  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("meta"),
      folly::IOBuf::copyBuffer("data"));
  ComposedRequestResponseFrame frame{
      .data = folly::IOBuf::copyBuffer("data"),
      .metadata = folly::IOBuf::copyBuffer("meta"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, RequestFnfSerializeMatchesWorker) {
  RequestFnfHeader header{.streamId = 50};
  auto direct =
      serialize(header, nullptr, folly::IOBuf::copyBuffer("fire and forget"));
  ComposedRequestFnfFrame frame{
      .data = folly::IOBuf::copyBuffer("fire and forget"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, RequestStreamSerializeMatchesWorker) {
  RequestStreamHeader header{.streamId = 42, .initialRequestN = 100};
  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("meta"),
      folly::IOBuf::copyBuffer("request data"));
  ComposedRequestStreamFrame frame{
      .data = folly::IOBuf::copyBuffer("request data"),
      .metadata = folly::IOBuf::copyBuffer("meta"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, RequestChannelSerializeMatchesWorker) {
  RequestChannelHeader header{
      .streamId = 51, .initialRequestN = 1000, .complete = true};
  auto direct =
      serialize(header, nullptr, folly::IOBuf::copyBuffer("channel data"));
  ComposedRequestChannelFrame frame{
      .data = folly::IOBuf::copyBuffer("channel data"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, RequestNSerializeMatchesWorker) {
  RequestNHeader header{.streamId = 8, .requestN = 50};
  auto direct = serialize(header);
  ComposedRequestNFrame frame{.header = header};
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, CancelSerializeMatchesWorker) {
  CancelHeader header{.streamId = 7};
  auto direct = serialize(header);
  ComposedCancelFrame frame{.header = header};
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, PayloadSerializeMatchesWorker) {
  PayloadHeader header{.streamId = 5, .complete = true, .next = true};
  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("meta"),
      folly::IOBuf::copyBuffer("response"));
  ComposedPayloadFrame frame{
      .data = folly::IOBuf::copyBuffer("response"),
      .metadata = folly::IOBuf::copyBuffer("meta"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, ErrorSerializeMatchesWorker) {
  ErrorHeader header{.streamId = 3, .errorCode = 0x00000201};
  auto direct = serialize(
      header, nullptr, folly::IOBuf::copyBuffer("Something went wrong"));
  ComposedErrorFrame frame{
      .data = folly::IOBuf::copyBuffer("Something went wrong"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, KeepAliveSerializeMatchesWorker) {
  KeepAliveHeader header{.lastReceivedPosition = 12345678, .respond = true};
  auto direct = serialize(header, folly::IOBuf::copyBuffer("ping"));
  ComposedKeepAliveFrame frame{
      .data = folly::IOBuf::copyBuffer("ping"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, SetupSerializeMatchesWorker) {
  SetupHeader header{
      .majorVersion = 1,
      .minorVersion = 0,
      .keepaliveTime = 30000,
      .maxLifetime = 60000};
  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("setup_meta"),
      folly::IOBuf::copyBuffer("setup_data"));
  ComposedSetupFrame frame{
      .data = folly::IOBuf::copyBuffer("setup_data"),
      .metadata = folly::IOBuf::copyBuffer("setup_meta"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, MetadataPushSerializeMatchesWorker) {
  MetadataPushHeader header{};
  auto direct = serialize(header, folly::IOBuf::copyBuffer("pushed metadata"));
  ComposedMetadataPushFrame frame{
      .metadata = folly::IOBuf::copyBuffer("pushed metadata"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}

TEST(ComposedFrameTest, ExtSerializeMatchesWorker) {
  ExtHeader header{.streamId = 60, .extendedType = 0x00000001, .ignore = true};
  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("ext_meta"),
      folly::IOBuf::copyBuffer("extension data"));
  ComposedExtFrame frame{
      .data = folly::IOBuf::copyBuffer("extension data"),
      .metadata = folly::IOBuf::copyBuffer("ext_meta"),
      .header = header,
  };
  auto viaFrame = std::move(frame).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaFrame));
}
