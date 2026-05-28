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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftControlPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadConcept.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftRequestPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/PayloadVariants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

// =============================================================================
// Concept satisfaction (compile-time)
// =============================================================================
//
// Positive: the right alternatives satisfy each concept.
// Negative: alternatives that shouldn't satisfy a concept are excluded —
// guards against accidental satisfaction (e.g. someone adds a
// getRequestRpcMetadata stub to ThriftErrorPayload).

static_assert(ThriftPayloadConcept<ThriftRequestResponsePayload>);
static_assert(ThriftPayloadConcept<ThriftInitialResponsePayload>);
static_assert(ThriftPayloadConcept<ThriftStreamInitialResponsePayload>);
static_assert(ThriftPayloadConcept<ThriftStreamPayload>);
static_assert(ThriftPayloadConcept<ThriftErrorPayload>);
static_assert(ThriftPayloadConcept<ThriftCancelPayload>);
static_assert(ThriftPayloadConcept<ThriftRequestNPayload>);

// Request concept: only the 5 initial-request alternatives.
static_assert(ThriftRequestPayloadConcept<ThriftRequestResponsePayload>);
static_assert(ThriftRequestPayloadConcept<ThriftRequestFnfPayload>);
static_assert(ThriftRequestPayloadConcept<ThriftRequestStreamPayload>);
static_assert(ThriftRequestPayloadConcept<ThriftRequestSinkPayload>);
static_assert(ThriftRequestPayloadConcept<ThriftRequestBidiPayload>);

static_assert(!ThriftRequestPayloadConcept<ThriftInitialResponsePayload>);
static_assert(!ThriftRequestPayloadConcept<ThriftStreamPayload>);
static_assert(!ThriftRequestPayloadConcept<ThriftErrorPayload>);
static_assert(!ThriftRequestPayloadConcept<ThriftCancelPayload>);
static_assert(!ThriftRequestPayloadConcept<ThriftRequestNPayload>);

// Initial-response concept: only the two initial-response alternatives.
static_assert(
    ThriftInitialResponsePayloadConcept<ThriftInitialResponsePayload>);
static_assert(
    ThriftInitialResponsePayloadConcept<ThriftStreamInitialResponsePayload>);

static_assert(
    !ThriftInitialResponsePayloadConcept<ThriftRequestResponsePayload>);
static_assert(!ThriftInitialResponsePayloadConcept<ThriftStreamPayload>);
static_assert(!ThriftInitialResponsePayloadConcept<ThriftErrorPayload>);
static_assert(!ThriftInitialResponsePayloadConcept<ThriftCancelPayload>);

// =============================================================================
// Variant accessor availability (compile-time)
// =============================================================================
//
// The constrained accessors on ThriftPayloadVariant are only well-formed
// when every alternative satisfies the corresponding refined concept.
// Verify the accessor is present where the alts qualify — regression
// guards if someone breaks the constraint clause and the accessor stops
// being instantiable on these variants.

static_assert(requires(const ThriftServerInboundPayloadVariant& v) {
  v.getRequestRpcMetadata();
});
static_assert(requires(const ThriftClientOutboundPayloadVariant& v) {
  v.getRequestRpcMetadata();
});

// =============================================================================
// ThriftInitialResponsePayload — terminal RR/Sink invariant
// =============================================================================

namespace {

std::unique_ptr<apache::thrift::ResponseRpcMetadata> makeResponseMetadata() {
  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  md->payloadMetadata().emplace();
  return md;
}

std::unique_ptr<apache::thrift::StreamPayloadMetadata>
makeStreamPayloadMetadata() {
  auto md = std::make_unique<apache::thrift::StreamPayloadMetadata>();
  md->compression() = apache::thrift::CompressionAlgorithm::ZSTD;
  return md;
}

} // namespace

// Regression guard: if someone re-adds `complete` / `next` fields to
// ThriftInitialResponsePayload, this test still pins the wire output to
// terminal+data. The struct's API may change but the produced frame must not.
TEST(
    ThriftInitialResponsePayloadTest,
    ToRocketFrameAlwaysTerminalAndCarriesData) {
  ThriftInitialResponsePayload payload{
      .data = folly::IOBuf::copyBuffer("response-data"),
      .metadata = makeResponseMetadata(),
      .streamId = 42,
  };
  auto frame = std::move(payload).toRocketFrame(
      rocket::server::MetadataProtocol::COMPACT);
  EXPECT_EQ(frame.streamId, 42u);
  EXPECT_TRUE(frame.complete);
  EXPECT_TRUE(frame.next);
  EXPECT_NE(frame.data, nullptr);
  EXPECT_NE(frame.metadata, nullptr);
}

// =============================================================================
// ThriftStreamInitialResponsePayload — caller-controlled flags
// =============================================================================

TEST(ThriftStreamInitialResponsePayloadTest, ToRocketFramePropagatesFlags) {
  ThriftStreamInitialResponsePayload payload{
      .data = folly::IOBuf::copyBuffer("first-chunk"),
      .metadata = makeResponseMetadata(),
      .streamId = 7,
      .complete = false,
      .next = true,
  };
  auto frame = std::move(payload).toRocketFrame(
      rocket::server::MetadataProtocol::BINARY);
  EXPECT_EQ(frame.streamId, 7u);
  EXPECT_FALSE(frame.complete);
  EXPECT_TRUE(frame.next);
  EXPECT_NE(frame.data, nullptr);
  EXPECT_NE(frame.metadata, nullptr);
}

TEST(ThriftStreamInitialResponsePayloadTest, ToRocketFrameTerminalChunk) {
  ThriftStreamInitialResponsePayload payload{
      .data = nullptr,
      .metadata = makeResponseMetadata(),
      .streamId = 9,
      .complete = true,
      .next = false,
  };
  auto frame = std::move(payload).toRocketFrame(
      rocket::server::MetadataProtocol::COMPACT);
  EXPECT_EQ(frame.streamId, 9u);
  EXPECT_TRUE(frame.complete);
  EXPECT_FALSE(frame.next);
  EXPECT_EQ(frame.data, nullptr);
  EXPECT_NE(frame.metadata, nullptr);
}

// =============================================================================
// ThriftStreamPayload — continuing chunk, optional metadata
// =============================================================================

TEST(ThriftStreamPayloadTest, ToRocketFrameWithMetadata) {
  ThriftStreamPayload payload{
      .data = folly::IOBuf::copyBuffer("chunk-data"),
      .metadata = makeStreamPayloadMetadata(),
      .streamId = 13,
      .complete = false,
      .next = true,
  };
  auto frame = std::move(payload).toRocketFrame(
      rocket::server::MetadataProtocol::BINARY);
  EXPECT_EQ(frame.streamId, 13u);
  EXPECT_FALSE(frame.complete);
  EXPECT_TRUE(frame.next);
  EXPECT_NE(frame.data, nullptr);
  EXPECT_NE(frame.metadata, nullptr);
}

TEST(ThriftStreamPayloadTest, ToRocketFrameWithoutMetadata) {
  ThriftStreamPayload payload{
      .data = folly::IOBuf::copyBuffer("chunk-data"),
      .metadata = nullptr,
      .streamId = 13,
      .complete = false,
      .next = true,
  };
  auto frame = std::move(payload).toRocketFrame(
      rocket::server::MetadataProtocol::BINARY);
  EXPECT_EQ(frame.streamId, 13u);
  EXPECT_FALSE(frame.complete);
  EXPECT_TRUE(frame.next);
  EXPECT_NE(frame.data, nullptr);
  EXPECT_EQ(frame.metadata, nullptr);
}

TEST(ThriftStreamPayloadTest, ToRocketFrameTerminalChunk) {
  ThriftStreamPayload payload{
      .data = nullptr,
      .metadata = nullptr,
      .streamId = 21,
      .complete = true,
      .next = false,
  };
  auto frame = std::move(payload).toRocketFrame(
      rocket::server::MetadataProtocol::BINARY);
  EXPECT_EQ(frame.streamId, 21u);
  EXPECT_TRUE(frame.complete);
  EXPECT_FALSE(frame.next);
}

} // namespace apache::thrift::fast_thrift::thrift
