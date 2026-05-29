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

#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>

#include <cstring>
#include <string_view>
#include <vector>

namespace apache::thrift::fast_thrift::frame::read::handler {
namespace {

// ============================================================================
// Test Utilities
// ============================================================================

/**
 * Helper to create a ParsedFrame with specific properties.
 */
ParsedFrame makeFrame(
    FrameType type,
    uint32_t streamId,
    bool hasFollows,
    std::string_view data,
    std::string_view metadata = {}) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(type);
  frame.metadata.streamId = streamId;
  frame.metadata.flags_ = 0;

  if (hasFollows) {
    frame.metadata.flags_ |=
        ::apache::thrift::fast_thrift::frame::detail::kFollowsBit;
  }
  if (!metadata.empty()) {
    frame.metadata.flags_ |=
        ::apache::thrift::fast_thrift::frame::detail::kMetadataBit;
  }

  auto totalSize = metadata.size() + data.size();
  frame.metadata.metadataSize = static_cast<uint16_t>(metadata.size());
  frame.metadata.payloadSize = static_cast<uint32_t>(totalSize);
  frame.metadata.payloadOffset = 0;

  auto buf = folly::IOBuf::create(totalSize);
  buf->append(totalSize);

  // Write metadata first, then data
  if (!metadata.empty()) {
    std::memcpy(buf->writableData(), metadata.data(), metadata.size());
  }
  if (!data.empty()) {
    std::memcpy(
        buf->writableData() + metadata.size(), data.data(), data.size());
  }

  frame.buffer = std::move(buf);
  return frame;
}

/**
 * Helper to create a CANCEL frame.
 */
ParsedFrame makeCancelFrame(uint32_t streamId) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::CANCEL);
  frame.metadata.streamId = streamId;
  frame.metadata.flags_ = 0;
  frame.metadata.payloadSize = 0;
  frame.metadata.metadataSize = 0;
  frame.metadata.payloadOffset = 0;
  frame.buffer = folly::IOBuf::create(0);
  return frame;
}

/**
 * Helper to create an ERROR frame.
 */
ParsedFrame makeErrorFrame(uint32_t streamId) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::ERROR);
  frame.metadata.streamId = streamId;
  frame.metadata.flags_ = 0;
  frame.metadata.payloadSize = 0;
  frame.metadata.metadataSize = 0;
  frame.metadata.payloadOffset = 0;
  frame.buffer = folly::IOBuf::create(0);
  return frame;
}

/**
 * Mock context for handler testing.
 */
class MockContext {
 public:
  apache::thrift::fast_thrift::channel_pipeline::Result fireRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&& msg) {
    receivedFrames.push_back(msg.take<ParsedFrame>());
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) {
    receivedExceptions.push_back(std::move(e));
  }

  apache::thrift::fast_thrift::channel_pipeline::BytesPtr allocate(
      size_t size) noexcept {
    return folly::IOBuf::create(size);
  }

  std::vector<ParsedFrame> receivedFrames;
  std::vector<folly::exception_wrapper> receivedExceptions;
};

/**
 * Helper to extract data as string from a ParsedFrame.
 */
std::string extractData(const ParsedFrame& frame) {
  if (!frame.buffer || frame.dataSize() == 0) {
    return "";
  }
  auto cursor = frame.dataCursor();
  std::string result;
  result.resize(frame.dataSize());
  cursor.pull(result.data(), result.size());
  return result;
}

// ============================================================================
// Basic Case Tests
// ============================================================================

TEST(FrameDefragmentationHandlerTest, NonFragmentedFramePassesThrough) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  auto frame = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/1,
      /*hasFollows=*/false,
      /*data=*/"hello");

  auto result = handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(frame)));

  EXPECT_EQ(
      result, apache::thrift::fast_thrift::channel_pipeline::Result::Success);
  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(ctx.receivedFrames[0].streamId(), 1);
  EXPECT_FALSE(ctx.receivedFrames[0].hasFollows());
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "hello");
  EXPECT_EQ(handler.pendingCount(), 0);
}

TEST(FrameDefragmentationHandlerTest, TwoFragmentsAssemble) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment
  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/1,
      /*hasFollows=*/true,
      /*data=*/"hel");

  auto result1 = handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  EXPECT_EQ(
      result1, apache::thrift::fast_thrift::channel_pipeline::Result::Success);
  EXPECT_EQ(ctx.receivedFrames.size(), 0);
  EXPECT_EQ(handler.pendingCount(), 1);
  EXPECT_TRUE(handler.hasPendingFragment(1));

  // Final fragment (PAYLOAD type for continuation)
  auto last = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/1,
      /*hasFollows=*/false,
      /*data=*/"lo");

  auto result2 = handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  EXPECT_EQ(
      result2, apache::thrift::fast_thrift::channel_pipeline::Result::Success);
  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(ctx.receivedFrames[0].streamId(), 1);
  EXPECT_FALSE(ctx.receivedFrames[0].hasFollows());
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "hello");
  EXPECT_EQ(handler.pendingCount(), 0);
}

TEST(FrameDefragmentationHandlerTest, ThreeFragmentsAssemble) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment
  auto f1 = makeFrame(
      FrameType::REQUEST_STREAM,
      /*streamId=*/5,
      /*hasFollows=*/true,
      /*data=*/"one");

  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f1)));
  EXPECT_EQ(handler.pendingCount(), 1);

  // Middle fragment
  auto f2 = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/5,
      /*hasFollows=*/true,
      /*data=*/"two");

  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f2)));
  EXPECT_EQ(ctx.receivedFrames.size(), 0);
  EXPECT_EQ(handler.pendingCount(), 1);

  // Final fragment
  auto f3 = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/5,
      /*hasFollows=*/false,
      /*data=*/"three");

  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f3)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_STREAM);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "onetwothree");
  EXPECT_EQ(handler.pendingCount(), 0);
}

TEST(FrameDefragmentationHandlerTest, PayloadDataCombinedInOrder) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  std::vector<std::string> fragments = {"A", "B", "C", "D", "E"};

  // First fragment
  auto first = makeFrame(
      FrameType::REQUEST_FNF,
      /*streamId=*/42,
      /*hasFollows=*/true,
      /*data=*/fragments[0]);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  // Middle fragments
  for (size_t i = 1; i < fragments.size() - 1; ++i) {
    auto mid = makeFrame(
        FrameType::PAYLOAD,
        /*streamId=*/42,
        /*hasFollows=*/true,
        /*data=*/fragments[i]);
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(mid)));
  }

  // Final fragment
  auto last = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/42,
      /*hasFollows=*/false,
      /*data=*/fragments.back());
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "ABCDE");
}

TEST(FrameDefragmentationHandlerTest, OriginalFrameTypePreserved) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment is REQUEST_CHANNEL
  auto first = makeFrame(
      FrameType::REQUEST_CHANNEL,
      /*streamId=*/7,
      /*hasFollows=*/true,
      /*data=*/"start");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  // Continuation is PAYLOAD
  auto last = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/7,
      /*hasFollows=*/false,
      /*data=*/"end");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  // Reassembled frame should have original type, not PAYLOAD
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_CHANNEL);
}

TEST(FrameDefragmentationHandlerTest, MetadataSizePreserved) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment with metadata
  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/8,
      /*hasFollows=*/true,
      /*data=*/"data1",
      /*metadata=*/"meta");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  // Continuation (no metadata per RSocket spec)
  auto last = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/8,
      /*hasFollows=*/false,
      /*data=*/"data2");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].metadataSize(), 4); // "meta".size()
  EXPECT_TRUE(ctx.receivedFrames[0].hasMetadata());
}

TEST(FrameDefragmentationHandlerTest, FollowsBitCleared) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/9,
      /*hasFollows=*/true,
      /*data=*/"a");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  auto last = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/9,
      /*hasFollows=*/false,
      /*data=*/"b");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_FALSE(ctx.receivedFrames[0].hasFollows());
}

// ============================================================================
// Cancellation Tests
// ============================================================================

TEST(FrameDefragmentationHandlerTest, CancelWithPendingFragments) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragmenting
  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/10,
      /*hasFollows=*/true,
      /*data=*/"partial");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));
  EXPECT_TRUE(handler.hasPendingFragment(10));

  // Cancel arrives
  auto cancel = makeCancelFrame(10);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(cancel)));

  // Pending state should be cleared
  EXPECT_FALSE(handler.hasPendingFragment(10));
  EXPECT_EQ(handler.pendingCount(), 0);

  // CANCEL frame should be forwarded
  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::CANCEL);
}

TEST(FrameDefragmentationHandlerTest, CancelUpdatesTotalPendingBytes) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragmenting with known data size
  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/10,
      /*hasFollows=*/true,
      /*data=*/"12345678"); // 8 bytes
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));
  EXPECT_EQ(handler.totalPendingBytes(), 8);

  // Add continuation fragment
  auto mid = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/10,
      /*hasFollows=*/true,
      /*data=*/"abcd"); // 4 more bytes
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(mid)));
  EXPECT_EQ(handler.totalPendingBytes(), 12);

  // Cancel should clear totalPendingBytes
  auto cancel = makeCancelFrame(10);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(cancel)));

  EXPECT_EQ(handler.pendingCount(), 0);
  EXPECT_EQ(handler.totalPendingBytes(), 0);
}

TEST(FrameDefragmentationHandlerTest, CancelDoesNotAffectOtherStreams) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragmenting on stream 1
  auto f1 = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/1,
      /*hasFollows=*/true,
      /*data=*/"stream1");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f1)));

  // Start fragmenting on stream 2
  auto f2 = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/2,
      /*hasFollows=*/true,
      /*data=*/"stream2");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f2)));

  EXPECT_EQ(handler.pendingCount(), 2);

  // Cancel stream 1 only
  auto cancel = makeCancelFrame(1);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(cancel)));

  // Only stream 1 should be cleared
  EXPECT_FALSE(handler.hasPendingFragment(1));
  EXPECT_TRUE(handler.hasPendingFragment(2));
  EXPECT_EQ(handler.pendingCount(), 1);
}

TEST(FrameDefragmentationHandlerTest, CancelOnlyUpdatesOwnStreamBytes) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragmenting on stream 1 with 8 bytes
  auto f1 = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/1,
      /*hasFollows=*/true,
      /*data=*/"12345678"); // 8 bytes
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f1)));

  // Start fragmenting on stream 2 with 4 bytes
  auto f2 = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/2,
      /*hasFollows=*/true,
      /*data=*/"abcd"); // 4 bytes
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f2)));

  EXPECT_EQ(handler.totalPendingBytes(), 12);

  // Cancel stream 1 only - should subtract only stream 1's bytes
  auto cancel = makeCancelFrame(1);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(cancel)));

  EXPECT_EQ(handler.pendingCount(), 1);
  EXPECT_EQ(handler.totalPendingBytes(), 4); // Only stream 2's bytes remain
}

TEST(FrameDefragmentationHandlerTest, CancelForNonPendingStream) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragmenting on stream 5
  auto f = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/5,
      /*hasFollows=*/true,
      /*data=*/"data");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f)));
  EXPECT_EQ(handler.pendingCount(), 1);

  // Cancel for stream 99 (no pending fragments)
  auto cancel = makeCancelFrame(99);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(cancel)));

  // Stream 5 should still be pending
  EXPECT_TRUE(handler.hasPendingFragment(5));
  EXPECT_EQ(handler.pendingCount(), 1);

  // CANCEL should still be forwarded
  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::CANCEL);
}

TEST(FrameDefragmentationHandlerTest, FragmentsAfterCancelStartFresh) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment
  auto f1 = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/11,
      /*hasFollows=*/true,
      /*data=*/"old");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f1)));

  // Cancel
  auto cancel = makeCancelFrame(11);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(cancel)));

  ctx.receivedFrames.clear(); // Clear the forwarded CANCEL

  // New first fragment after cancel (same stream ID)
  auto f2 = makeFrame(
      FrameType::REQUEST_STREAM,
      /*streamId=*/11,
      /*hasFollows=*/true,
      /*data=*/"new");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f2)));

  // Final fragment
  auto f3 = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/11,
      /*hasFollows=*/false,
      /*data=*/"data");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f3)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_STREAM);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "newdata");
}

// ============================================================================
// Interleaving Tests
// ============================================================================

TEST(FrameDefragmentationHandlerTest, TwoInterleavedStreams) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Stream 1 first fragment
  auto s1f1 = makeFrame(FrameType::REQUEST_RESPONSE, 1, true, "s1-a");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s1f1)));

  // Stream 2 first fragment
  auto s2f1 = makeFrame(FrameType::REQUEST_STREAM, 2, true, "s2-x");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s2f1)));

  EXPECT_EQ(handler.pendingCount(), 2);

  // Stream 1 final fragment
  auto s1f2 = makeFrame(FrameType::PAYLOAD, 1, false, "s1-b");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s1f2)));

  EXPECT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(handler.pendingCount(), 1);

  // Stream 2 final fragment
  auto s2f2 = makeFrame(FrameType::PAYLOAD, 2, false, "s2-y");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s2f2)));

  ASSERT_EQ(ctx.receivedFrames.size(), 2);
  EXPECT_EQ(handler.pendingCount(), 0);

  // Verify both streams assembled correctly
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(ctx.receivedFrames[0].streamId(), 1);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "s1-as1-b");

  EXPECT_EQ(ctx.receivedFrames[1].type(), FrameType::REQUEST_STREAM);
  EXPECT_EQ(ctx.receivedFrames[1].streamId(), 2);
  EXPECT_EQ(extractData(ctx.receivedFrames[1]), "s2-xs2-y");
}

TEST(FrameDefragmentationHandlerTest, InterleavedWithComplete) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Stream 1 first fragment
  auto s1f1 = makeFrame(FrameType::REQUEST_RESPONSE, 1, true, "frag1");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s1f1)));

  // Stream 3 complete (non-fragmented)
  auto s3 = makeFrame(FrameType::REQUEST_FNF, 3, false, "complete");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s3)));

  // Complete frame should pass through immediately
  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].streamId(), 3);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "complete");

  // Stream 1 still pending
  EXPECT_EQ(handler.pendingCount(), 1);

  // Stream 1 final fragment
  auto s1f2 = makeFrame(FrameType::PAYLOAD, 1, false, "frag2");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s1f2)));

  ASSERT_EQ(ctx.receivedFrames.size(), 2);
  EXPECT_EQ(ctx.receivedFrames[1].streamId(), 1);
  EXPECT_EQ(extractData(ctx.receivedFrames[1]), "frag1frag2");
}

TEST(FrameDefragmentationHandlerTest, ManyInterleavedStreams) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  const int numStreams = 5;
  const int numFragmentsPerStream = 3;

  // Start all streams with first fragment
  for (int i = 1; i <= numStreams; ++i) {
    auto first = makeFrame(
        FrameType::REQUEST_RESPONSE, i, true, "s" + std::to_string(i) + "-f1");
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(first)));
  }
  EXPECT_EQ(handler.pendingCount(), numStreams);
  EXPECT_EQ(ctx.receivedFrames.size(), 0);

  // Add middle fragments (interleaved)
  for (int frag = 2; frag < numFragmentsPerStream; ++frag) {
    for (int i = 1; i <= numStreams; ++i) {
      auto mid =
          makeFrame(FrameType::PAYLOAD, i, true, "-f" + std::to_string(frag));
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(mid)));
    }
  }
  EXPECT_EQ(ctx.receivedFrames.size(), 0);

  // Complete all streams with final fragment (in reverse order)
  for (int i = numStreams; i >= 1; --i) {
    auto last = makeFrame(
        FrameType::PAYLOAD,
        i,
        false,
        "-f" + std::to_string(numFragmentsPerStream));
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(last)));
  }

  EXPECT_EQ(handler.pendingCount(), 0);
  EXPECT_EQ(ctx.receivedFrames.size(), numStreams);

  // All reassembled correctly (arrived in reverse order due to completion
  // order)
  for (int i = 0; i < numStreams; ++i) {
    auto& frame = ctx.receivedFrames[i];
    EXPECT_EQ(frame.type(), FrameType::REQUEST_RESPONSE);
    EXPECT_FALSE(frame.hasFollows());
  }
}

// ============================================================================
// Handler Lifecycle Tests
// ============================================================================

TEST(FrameDefragmentationHandlerTest, HandlerRemovedClearsPending) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start some fragments
  auto f1 = makeFrame(FrameType::REQUEST_RESPONSE, 1, true, "a");
  auto f2 = makeFrame(FrameType::REQUEST_RESPONSE, 2, true, "b");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f1)));
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f2)));

  EXPECT_EQ(handler.pendingCount(), 2);

  // Remove handler
  handler.handlerRemoved(ctx);

  EXPECT_EQ(handler.pendingCount(), 0);
}

TEST(FrameDefragmentationHandlerTest, OnExceptionPassesThrough) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler.onException(ctx, std::move(ex));

  ASSERT_EQ(ctx.receivedExceptions.size(), 1);
  EXPECT_TRUE(
      ctx.receivedExceptions[0].is_compatible_with<std::runtime_error>());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(FrameDefragmentationHandlerTest, EmptyFirstFragmentWithContinuation) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment with NO payload (edge case - payloadSize == 0)
  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/1,
      /*hasFollows=*/true,
      /*data=*/"",
      /*metadata=*/"");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  EXPECT_TRUE(handler.hasPendingFragment(1));
  EXPECT_EQ(handler.totalPendingBytes(), 0);

  // Continuation fragment with actual data
  auto mid = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/1,
      /*hasFollows=*/true,
      /*data=*/"hello");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(mid)));

  EXPECT_EQ(handler.totalPendingBytes(), 5);

  // Final fragment
  auto last = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/1,
      /*hasFollows=*/false,
      /*data=*/"world");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "helloworld");
  EXPECT_EQ(handler.pendingCount(), 0);
  EXPECT_EQ(handler.totalPendingBytes(), 0);
}

TEST(FrameDefragmentationHandlerTest, EmptyPayloadFragments) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment with data
  auto first = makeFrame(FrameType::REQUEST_RESPONSE, 1, true, "data");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  // Empty continuation
  auto mid = makeFrame(FrameType::PAYLOAD, 1, true, "");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(mid)));

  // Final fragment with data
  auto last = makeFrame(FrameType::PAYLOAD, 1, false, "end");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(extractData(ctx.receivedFrames[0]), "dataend");
}

TEST(FrameDefragmentationHandlerTest, LargePayload) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Create large payload fragments
  std::string chunk(1024, 'X'); // 1KB per chunk
  const int numChunks = 10;

  // First fragment
  auto first = makeFrame(FrameType::REQUEST_RESPONSE, 1, true, chunk);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));

  // Middle fragments
  for (int i = 1; i < numChunks - 1; ++i) {
    auto mid = makeFrame(FrameType::PAYLOAD, 1, true, chunk);
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(mid)));
  }

  // Final fragment
  auto last = makeFrame(FrameType::PAYLOAD, 1, false, chunk);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(last)));

  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(
      ctx.receivedFrames[0].payloadSize(),
      static_cast<uint32_t>(numChunks * chunk.size()));
}

// ============================================================================
// ERROR Frame Tests
// ============================================================================

TEST(FrameDefragmentationHandlerTest, ErrorWithPendingFragments) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // First fragment
  auto f1 = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/10,
      /*hasFollows=*/true,
      /*data=*/"partial");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(f1)));

  EXPECT_TRUE(handler.hasPendingFragment(10));

  // ERROR arrives
  auto error = makeErrorFrame(10);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(error)));

  // Pending fragments should be cleared
  EXPECT_FALSE(handler.hasPendingFragment(10));
  EXPECT_EQ(handler.pendingCount(), 0);

  // ERROR frame should be forwarded
  ASSERT_EQ(ctx.receivedFrames.size(), 1);
  EXPECT_EQ(ctx.receivedFrames[0].type(), FrameType::ERROR);
}

TEST(FrameDefragmentationHandlerTest, ErrorUpdatesTotalPendingBytes) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragmenting with known data sizes
  auto first = makeFrame(
      FrameType::REQUEST_RESPONSE,
      /*streamId=*/10,
      /*hasFollows=*/true,
      /*data=*/"12345678"); // 8 bytes
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(first)));
  EXPECT_EQ(handler.totalPendingBytes(), 8);

  // Add continuation fragment
  auto mid = makeFrame(
      FrameType::PAYLOAD,
      /*streamId=*/10,
      /*hasFollows=*/true,
      /*data=*/"abcd"); // 4 more bytes
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(mid)));
  EXPECT_EQ(handler.totalPendingBytes(), 12);

  // ERROR should clear totalPendingBytes
  auto error = makeErrorFrame(10);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(error)));

  EXPECT_EQ(handler.pendingCount(), 0);
  EXPECT_EQ(handler.totalPendingBytes(), 0);
}

TEST(FrameDefragmentationHandlerTest, ErrorDoesNotAffectOtherStreams) {
  FrameDefragmentationHandler handler;
  MockContext ctx;

  // Start fragments for two streams
  auto s1 = makeFrame(FrameType::REQUEST_RESPONSE, 1, true, "stream1");
  auto s2 = makeFrame(FrameType::REQUEST_STREAM, 2, true, "stream2");
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s1)));
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(s2)));

  EXPECT_EQ(handler.pendingCount(), 2);
  EXPECT_TRUE(handler.hasPendingFragment(1));
  EXPECT_TRUE(handler.hasPendingFragment(2));

  // ERROR stream 1 only
  auto error = makeErrorFrame(1);
  (void)handler.onRead(
      ctx,
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(error)));

  // Stream 1 cleared, stream 2 remains
  EXPECT_EQ(handler.pendingCount(), 1);
  EXPECT_FALSE(handler.hasPendingFragment(1));
  EXPECT_TRUE(handler.hasPendingFragment(2));
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read::handler
