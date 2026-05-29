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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/HandlerNode.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/portability/GTest.h>

#include <memory>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

HANDLER_TAG(test_handler);

class HandlerNodeTest : public ::testing::Test {
 protected:
  void SetUp() override { MockHandler::resetOrderCounter(); }
};

TEST_F(HandlerNodeTest, CreateHandlerNode) {
  auto handler = std::make_unique<MockHandler>();
  auto node = detail::makeHandlerNode<MockHandler>(
      test_handler_tag.id, std::move(handler));

  EXPECT_EQ(node.handlerId, test_handler_tag.id);
  EXPECT_NE(node.handlerPtr, nullptr);
  EXPECT_NE(node.owner.get(), nullptr);
}

TEST_F(HandlerNodeTest, FunctionPointersAreSet) {
  auto handler = std::make_unique<MockHandler>();
  auto node = detail::makeHandlerNode<MockHandler>(
      test_handler_tag.id, std::move(handler));

  // All function pointers should be set for MockHandler (duplex handler)
  EXPECT_NE(node.onPipelineActivatedFn, nullptr);
  EXPECT_NE(node.onReadFn, nullptr);
  EXPECT_NE(node.onWriteFn, nullptr);
  EXPECT_NE(node.onExceptionFn, nullptr);
  EXPECT_NE(node.onWriteReadyFn, nullptr);
  EXPECT_NE(node.onPipelineDeactivatedFn, nullptr);
  EXPECT_NE(node.handlerAddedFn, nullptr);
  EXPECT_NE(node.handlerRemovedFn, nullptr);
}

TEST_F(HandlerNodeTest, InboundOnlyHandlerHasPassthroughOutbound) {
  auto handler = std::make_unique<InboundOnlyHandler>();
  auto node = detail::makeHandlerNode<InboundOnlyHandler>(
      test_handler_tag.id, std::move(handler));

  // Inbound function pointers should be set
  EXPECT_NE(node.onPipelineActivatedFn, nullptr);
  EXPECT_NE(node.onReadFn, nullptr);
  EXPECT_NE(node.onExceptionFn, nullptr);

  // Outbound function pointers should still be set (passthrough)
  EXPECT_NE(node.onWriteFn, nullptr);
  EXPECT_NE(node.onWriteReadyFn, nullptr);
  EXPECT_NE(node.onPipelineDeactivatedFn, nullptr);
}

TEST_F(HandlerNodeTest, OutboundOnlyHandlerHasPassthroughInbound) {
  auto handler = std::make_unique<OutboundOnlyHandler>();
  auto node = detail::makeHandlerNode<OutboundOnlyHandler>(
      test_handler_tag.id, std::move(handler));

  // Outbound function pointers should be set
  EXPECT_NE(node.onWriteFn, nullptr);
  EXPECT_NE(node.onWriteReadyFn, nullptr);
  EXPECT_NE(node.onPipelineDeactivatedFn, nullptr);

  // Inbound function pointers should still be set (passthrough)
  EXPECT_NE(node.onPipelineActivatedFn, nullptr);
  EXPECT_NE(node.onReadFn, nullptr);
  EXPECT_NE(node.onExceptionFn, nullptr);
}

TEST_F(HandlerNodeTest, HandlerNodeIsMoveOnly) {
  auto handler = std::make_unique<MockHandler>();
  auto node1 = detail::makeHandlerNode<MockHandler>(
      test_handler_tag.id, std::move(handler));

  void* original_ptr = node1.handlerPtr;
  HandlerId original_id = node1.handlerId;

  // Move to new node
  auto node2 = std::move(node1);

  EXPECT_EQ(node2.handlerPtr, original_ptr);
  EXPECT_EQ(node2.handlerId, original_id);
  EXPECT_NE(node2.owner.get(), nullptr);

  // Original node should be empty after move
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_EQ(node1.handlerPtr, nullptr);
  EXPECT_EQ(node1.owner.get(), nullptr);
}

TEST_F(HandlerNodeTest, HandlerIsDestroyedWhenNodeIsDestroyed) {
  bool destroyed = false;

  // Create a handler that sets a flag on destruction
  struct DestructorTestHandler {
    bool* destroyed_flag;

    explicit DestructorTestHandler(bool* flag) : destroyed_flag(flag) {}

    ~DestructorTestHandler() { *destroyed_flag = true; }

    void handlerAdded(detail::ContextImpl&) noexcept {}
    void handlerRemoved(detail::ContextImpl&) noexcept {}
  };

  {
    auto handler = std::make_unique<DestructorTestHandler>(&destroyed);
    auto node = detail::makeHandlerNode<DestructorTestHandler>(
        test_handler_tag.id, std::move(handler));
    EXPECT_FALSE(destroyed);
  } // node goes out of scope

  EXPECT_TRUE(destroyed);
}

TEST_F(HandlerNodeTest, HandlerIdMatchesTag) {
  auto handler = std::make_unique<MockHandler>();
  auto node = detail::makeHandlerNode<MockHandler>(
      test_handler_tag.id, std::move(handler));

  // HandlerId should be the FNV-1a hash of "test_handler"
  EXPECT_EQ(node.handlerId, fnv1a_hash("test_handler"));
  EXPECT_EQ(node.handlerId, test_handler_tag.id);
}

TEST_F(HandlerNodeTest, MultipleNodesHaveDistinctHandlerPointers) {
  auto handler1 = std::make_unique<MockHandler>();
  auto handler2 = std::make_unique<MockHandler>();

  // Use different handler IDs (computed manually rather than HANDLER_TAG macro
  // which uses inline and can't be in function scope)
  constexpr HandlerId handler1_id = fnv1a_hash("handler1");
  constexpr HandlerId handler2_id = fnv1a_hash("handler2");

  auto node1 =
      detail::makeHandlerNode<MockHandler>(handler1_id, std::move(handler1));
  auto node2 =
      detail::makeHandlerNode<MockHandler>(handler2_id, std::move(handler2));

  EXPECT_NE(node1.handlerPtr, node2.handlerPtr);
  EXPECT_NE(node1.handlerId, node2.handlerId);
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
