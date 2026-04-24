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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>

#include <memory>
#include <string>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

HANDLER_TAG(codec);
HANDLER_TAG(thrift);
HANDLER_TAG(logging);

class PipelineBuilderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MockHandler::resetOrderCounter();
    transport_.reset();
    app_.reset();
    allocator_.reset();
  }

  folly::EventBase evb_;
  MockHeadHandler transport_; // Head = transport (onWrite)
  MockTailHandler app_; // Tail = app (onRead)
  TestAllocator allocator_;
};

TEST_F(PipelineBuilderTest, BuildEmptyPipeline) {
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .build();

  EXPECT_NE(pipeline.get(), nullptr);
  EXPECT_EQ(pipeline->handlerCount(), 0);
  EXPECT_EQ(pipeline->eventBase(), &evb_);
}

TEST_F(PipelineBuilderTest, BuildSingleHandlerPipeline) {
  auto handler = std::make_unique<MockHandler>();
  auto* handler_ptr = handler.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(codec_tag, std::move(handler))
          .build();

  EXPECT_EQ(pipeline->handlerCount(), 1);
  EXPECT_EQ(handler_ptr->handlerAddedCount(), 1);
}

TEST_F(PipelineBuilderTest, BuildMultiHandlerPipeline) {
  auto codec = std::make_unique<MockHandler>();
  auto thrift = std::make_unique<MockHandler>();
  auto logging = std::make_unique<MockHandler>();

  auto* codec_ptr = codec.get();
  auto* thrift_ptr = thrift.get();
  auto* logging_ptr = logging.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(codec_tag, std::move(codec))
          .addNextDuplex<MockHandler>(thrift_tag, std::move(thrift))
          .addNextDuplex<MockHandler>(logging_tag, std::move(logging))
          .build();

  EXPECT_EQ(pipeline->handlerCount(), 3);

  // All handlers should have handlerAdded called
  EXPECT_EQ(codec_ptr->handlerAddedCount(), 1);
  EXPECT_EQ(thrift_ptr->handlerAddedCount(), 1);
  EXPECT_EQ(logging_ptr->handlerAddedCount(), 1);
}

TEST_F(PipelineBuilderTest, HandlerAddedCalledInOrder) {
  auto codec = std::make_unique<MockHandler>();
  auto thrift = std::make_unique<MockHandler>();
  auto logging = std::make_unique<MockHandler>();

  auto* codec_ptr = codec.get();
  auto* thrift_ptr = thrift.get();
  auto* logging_ptr = logging.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(codec_tag, std::move(codec))
          .addNextDuplex<MockHandler>(thrift_tag, std::move(thrift))
          .addNextDuplex<MockHandler>(logging_tag, std::move(logging))
          .build();

  // handlerAdded should be called in order: codec(0), thrift(1), logging(2)
  EXPECT_EQ(codec_ptr->handlerAddedOrder(), 0);
  EXPECT_EQ(thrift_ptr->handlerAddedOrder(), 1);
  EXPECT_EQ(logging_ptr->handlerAddedOrder(), 2);
}

TEST_F(PipelineBuilderTest, ContextLookupByTag) {
  auto codec = std::make_unique<MockHandler>();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(codec_tag, std::move(codec))
          .build();

  auto* ctx = pipeline->context(codec_tag);
  EXPECT_NE(ctx, nullptr);
  EXPECT_EQ(ctx->handlerId(), codec_tag.id);

  // Non-existent handler returns nullptr
  auto* missing_ctx = pipeline->context(thrift_tag);
  EXPECT_EQ(missing_ctx, nullptr);
}

TEST_F(PipelineBuilderTest, BuildThrowsWithoutEventBase) {
  using Builder =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>;
  EXPECT_THROW(
      Builder()
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .build(),
      std::runtime_error);
}

TEST_F(PipelineBuilderTest, BuildThrowsWithoutHead) {
  using Builder =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>;
  EXPECT_THROW(
      Builder()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .build(),
      std::runtime_error);
}

TEST_F(PipelineBuilderTest, BuildThrowsWithoutTail) {
  using Builder =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>;
  EXPECT_THROW(
      Builder()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .build(),
      std::runtime_error);
}

TEST_F(PipelineBuilderTest, BuildThrowsWithoutAllocator) {
  using Builder =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>;
  EXPECT_THROW(
      Builder().setEventBase(&evb_).setHead(&transport_).setTail(&app_).build(),
      std::runtime_error);
}

TEST_F(PipelineBuilderTest, AddNextDuplexWithInPlaceConstruction) {
  struct TestHandler {
    std::string name;

    explicit TestHandler(std::string n) : name(std::move(n)) {}

    Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
      return ctx.fireRead(std::move(msg));
    }
    void onException(
        detail::ContextImpl& ctx, folly::exception_wrapper e) noexcept {
      ctx.fireException(std::move(e));
    }
    void onPipelineActivated(detail::ContextImpl&) noexcept {}
    void onReadReady(detail::ContextImpl&) noexcept {}

    Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
      return ctx.fireWrite(std::move(msg));
    }
    void onWriteReady(detail::ContextImpl&) noexcept {}
    void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

    void handlerAdded(detail::ContextImpl&) noexcept {}
    void handlerRemoved(detail::ContextImpl&) noexcept {}
  };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<TestHandler>(codec_tag, "my_codec")
          .build();

  EXPECT_EQ(pipeline->handlerCount(), 1);
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
