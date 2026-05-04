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
 * ComposedFrameVariantTest validates the typed-variant wrapper:
 *
 *   - ComposedFrameConcept conformance: every Composed*Frame satisfies
 *     it (catches removal of kFrameType or signature drift on the 4 APIs).
 *
 *   - The 4 typed APIs (frameType, streamId, complete, serialize)
 *     dispatch to the held alternative correctly. frameType is a direct
 *     field load; streamId / complete / serialize use fold-expression
 *     dispatch over the type pack.
 *
 *   - Move semantics: move ctor / move assign transfer the held frame
 *     and leave the source valueless. Destructor cleans up without leaks.
 *
 *   - Discriminator queries: valueless / is<T> / get<T> behave correctly.
 *
 * Per-frame streamId / complete / serialize correctness is exhaustively
 * covered in ComposedFrameTest.cpp; this file only validates the variant
 * wrapping.
 */

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrameVariant.h>

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

#include <string>

using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::write;

namespace {

std::string flatten(const folly::IOBuf& buf) {
  auto coalesced = buf.clone();
  coalesced->coalesce();
  return std::string(
      reinterpret_cast<const char*>(coalesced->data()), coalesced->length());
}

// Pick a representative variant covering: stream-scoped + payload, header-
// only, connection-level, error, and an unrelated alternative kept in the
// pack to verify the dispatch picks the correct arm rather than the first.
using TestVariant = ComposedFrameVariant<
    ComposedRequestResponseFrame,
    ComposedRequestNFrame,
    ComposedSetupFrame,
    ComposedErrorFrame>;

} // namespace

// ============================================================================
// Concept conformance
// ============================================================================
//
// Static assertions; failure shows up at compile time. These guard against
// accidental removal of kFrameType, signature drift on streamId/serialize,
// or noexcept regressions.

static_assert(ComposedFrameConcept<ComposedRequestResponseFrame>);
static_assert(ComposedFrameConcept<ComposedRequestFnfFrame>);
static_assert(ComposedFrameConcept<ComposedRequestStreamFrame>);
static_assert(ComposedFrameConcept<ComposedRequestChannelFrame>);
static_assert(ComposedFrameConcept<ComposedRequestNFrame>);
static_assert(ComposedFrameConcept<ComposedCancelFrame>);
static_assert(ComposedFrameConcept<ComposedPayloadFrame>);
static_assert(ComposedFrameConcept<ComposedErrorFrame>);
static_assert(ComposedFrameConcept<ComposedKeepAliveFrame>);
static_assert(ComposedFrameConcept<ComposedSetupFrame>);
static_assert(ComposedFrameConcept<ComposedMetadataPushFrame>);
static_assert(ComposedFrameConcept<ComposedExtFrame>);

// ============================================================================
// Default construction → valueless
// ============================================================================

TEST(ComposedFrameVariantTest, DefaultConstructedIsValueless) {
  TestVariant v;
  EXPECT_TRUE(v.valueless());
  EXPECT_EQ(v.frameType(), FrameType::RESERVED);
}

// ============================================================================
// Typed-API dispatch: frameType / streamId / serialize
// ============================================================================
//
// Each test verifies the three APIs route to the held alternative. We use
// distinct alternatives across tests to confirm dispatch isn't always
// hitting the first arm.

TEST(ComposedFrameVariantTest, RequestResponseRoundtrip) {
  RequestResponseHeader header{.streamId = 42};
  TestVariant v = ComposedRequestResponseFrame{
      .data = folly::IOBuf::copyBuffer("data"),
      .metadata = folly::IOBuf::copyBuffer("meta"),
      .header = header,
  };

  EXPECT_FALSE(v.valueless());
  EXPECT_EQ(v.frameType(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(v.streamId(), 42u);
  EXPECT_FALSE(v.complete());

  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("meta"),
      folly::IOBuf::copyBuffer("data"));
  auto viaVariant = std::move(v).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaVariant));
}

TEST(ComposedFrameVariantTest, RequestNRoundtripHeaderOnly) {
  // Header-only frame (no metadata/data buffers) — exercises the
  // smallest-storage code path in the variant.
  RequestNHeader header{.streamId = 7, .requestN = 100};
  TestVariant v = ComposedRequestNFrame{.header = header};

  EXPECT_EQ(v.frameType(), FrameType::REQUEST_N);
  EXPECT_EQ(v.streamId(), 7u);
  EXPECT_FALSE(v.complete());

  auto direct = serialize(header);
  auto viaVariant = std::move(v).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaVariant));
}

TEST(ComposedFrameVariantTest, SetupRoundtripConnectionLevel) {
  // Connection-level frame — streamId() must report 0 per RSocket spec
  // even though SetupHeader has no streamId field at all.
  SetupHeader header{
      .majorVersion = 1,
      .minorVersion = 0,
      .keepaliveTime = 30000,
      .maxLifetime = 60000};
  TestVariant v = ComposedSetupFrame{
      .data = folly::IOBuf::copyBuffer("setup_data"),
      .metadata = folly::IOBuf::copyBuffer("setup_meta"),
      .header = header,
  };

  EXPECT_EQ(v.frameType(), FrameType::SETUP);
  EXPECT_EQ(v.streamId(), 0u);
  EXPECT_FALSE(v.complete());

  auto direct = serialize(
      header,
      folly::IOBuf::copyBuffer("setup_meta"),
      folly::IOBuf::copyBuffer("setup_data"));
  auto viaVariant = std::move(v).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaVariant));
}

TEST(ComposedFrameVariantTest, ErrorRoundtripLastAlternativeInPack) {
  // Last alternative in the type pack — guards against fold-expression
  // dispatch terminating early instead of matching the actual tag.
  // Also confirms complete() dispatches to the terminal-by-frame-type
  // arm (ERROR returns true regardless of header fields).
  ErrorHeader header{.streamId = 88, .errorCode = 0x00000201};
  TestVariant v = ComposedErrorFrame{
      .data = folly::IOBuf::copyBuffer("oops"),
      .header = header,
  };

  EXPECT_EQ(v.frameType(), FrameType::ERROR);
  EXPECT_EQ(v.streamId(), 88u);
  EXPECT_TRUE(v.complete());

  auto direct = serialize(header, nullptr, folly::IOBuf::copyBuffer("oops"));
  auto viaVariant = std::move(v).serialize();
  EXPECT_EQ(flatten(*direct), flatten(*viaVariant));
}

TEST(ComposedFrameVariantTest, CompleteForwardsHeaderBitForWireDrivenFrame) {
  // PAYLOAD's complete() reads header.complete — confirm variant dispatch
  // forwards the header bit (not a hardcoded constant) for both values.
  using PayloadVariant = ComposedFrameVariant<ComposedPayloadFrame>;

  PayloadVariant nextV =
      ComposedPayloadFrame{.header = {.streamId = 1, .complete = false}};
  EXPECT_FALSE(nextV.complete());

  PayloadVariant finalV =
      ComposedPayloadFrame{.header = {.streamId = 1, .complete = true}};
  EXPECT_TRUE(finalV.complete());
}

// ============================================================================
// Move semantics
// ============================================================================

TEST(ComposedFrameVariantTest, MoveConstructTransfersAndClearsSource) {
  TestVariant src = ComposedRequestResponseFrame{
      .data = folly::IOBuf::copyBuffer("payload"),
      .header = {.streamId = 99},
  };

  TestVariant dst{std::move(src)};

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(src.valueless());
  EXPECT_EQ(src.frameType(), FrameType::RESERVED);

  EXPECT_FALSE(dst.valueless());
  EXPECT_EQ(dst.frameType(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(dst.streamId(), 99u);
}

TEST(ComposedFrameVariantTest, MoveAssignReplacesExistingAndClearsSource) {
  TestVariant src = ComposedSetupFrame{.header = {.majorVersion = 1}};
  TestVariant dst =
      ComposedErrorFrame{.header = {.streamId = 5, .errorCode = 1}};

  dst = std::move(src);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(src.valueless());
  EXPECT_EQ(dst.frameType(), FrameType::SETUP);
  EXPECT_EQ(dst.streamId(), 0u);
}

TEST(ComposedFrameVariantTest, MoveConstructFromValuelessStaysValueless) {
  TestVariant src;
  TestVariant dst{std::move(src)};
  EXPECT_TRUE(dst.valueless());
}

// ============================================================================
// Discriminator queries
// ============================================================================

TEST(ComposedFrameVariantTest, IsAndGetMatchHeldAlternative) {
  TestVariant v =
      ComposedRequestNFrame{.header = {.streamId = 3, .requestN = 1}};

  EXPECT_TRUE(v.is<ComposedRequestNFrame>());
  EXPECT_FALSE(v.is<ComposedRequestResponseFrame>());
  EXPECT_FALSE(v.is<ComposedSetupFrame>());

  EXPECT_EQ(v.get<ComposedRequestNFrame>().header.streamId, 3u);
  EXPECT_EQ(v.get<ComposedRequestNFrame>().header.requestN, 1u);
}

TEST(ComposedFrameVariantTest, EmplaceReplacesAlternative) {
  TestVariant v = ComposedRequestResponseFrame{.header = {.streamId = 1}};
  v.emplace<ComposedErrorFrame>(ComposedErrorFrame{
      .header = {.streamId = 2, .errorCode = 0x201},
  });

  EXPECT_EQ(v.frameType(), FrameType::ERROR);
  EXPECT_EQ(v.streamId(), 2u);
}
