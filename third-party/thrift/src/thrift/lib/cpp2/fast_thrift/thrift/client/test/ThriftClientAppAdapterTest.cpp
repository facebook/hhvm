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
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::thrift {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::channel_pipeline::test::MockHandler;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;

HANDLER_TAG(test_handler);

namespace {

// Test client that holds the adapter as a member (composition).
class TestAppAdapterClient {
 public:
  ThriftClientAppAdapter& adapter() { return *adapter_; }

 private:
  ThriftClientAppAdapter::Ptr adapter_{new ThriftClientAppAdapter()};
};

ThriftResponseMessage makeResponse(
    uint32_t handle,
    apache::thrift::fast_thrift::frame::FrameType frameType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE) {
  auto data = folly::IOBuf::copyBuffer("response");
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1, .follows = false, .complete = true, .next = true},
      nullptr,
      std::move(data));

  ThriftResponseMessage response;
  response.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
  response.requestHandle = handle;
  response.streamType = frameType;
  return response;
}

TypeErasedBox makeRequestBox() {
  ThriftRequestMessage msg{
      .payload = ThriftRequestPayload{
          .metadata = folly::IOBuf::copyBuffer("meta"),
          .data = folly::IOBuf::copyBuffer("data"),
          .rpcKind = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
          .complete = true}};
  return erase_and_box(std::move(msg));
}

} // namespace

class ThriftClientAppAdapterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MockHandler::resetOrderCounter();
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();
    allocator_.reset();
  }

  void TearDown() override { evbThread_.reset(); }

  folly::AsyncTransport::UniquePtr createMockSocket() {
    auto* sock = new testing::NiceMock<folly::test::MockAsyncTransport>();
    ON_CALL(*sock, getEventBase()).WillByDefault(testing::Return(evb_));
    return folly::AsyncTransport::UniquePtr(sock);
  }

  struct BuiltPipeline {
    apache::thrift::fast_thrift::transport::TransportHandler::Ptr
        transportHandler;
    PipelineImpl::Ptr pipeline;
    MockHandler* handler{nullptr};
  };

  BuiltPipeline buildPipeline(
      ThriftClientAppAdapter* adapter,
      std::function<Result(
          apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&)> writeHandler = nullptr) {
    auto transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            createMockSocket());

    auto handlerPtr = std::make_unique<MockHandler>();
    auto* rawHandler = handlerPtr.get();

    if (writeHandler) {
      handlerPtr->setOnWrite(std::move(writeHandler));
    }

    auto pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            ThriftClientAppAdapter,
            TestAllocator>()
            .setEventBase(evb_)
            .setHead(transportHandler.get())
            .setTail(adapter)
            .setAllocator(&allocator_)
            .addNextDuplex<MockHandler>(test_handler_tag, std::move(handlerPtr))
            .build();

    adapter->setPipeline(pipeline.get());

    return {std::move(transportHandler), std::move(pipeline), rawHandler};
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  TestAllocator allocator_;
};

// =============================================================================
// ProtocolId Tests
// =============================================================================

TEST_F(ThriftClientAppAdapterTest, DefaultProtocolIdIsZero) {
  TestAppAdapterClient client;
  EXPECT_EQ(client.adapter().getProtocolId(), 0);
}

TEST_F(ThriftClientAppAdapterTest, ConstructorSetsProtocolId) {
  ThriftClientAppAdapter::Ptr adapter{new ThriftClientAppAdapter(42)};
  EXPECT_EQ(adapter->getProtocolId(), 42);
}

TEST_F(ThriftClientAppAdapterTest, SetProtocolId) {
  TestAppAdapterClient client;
  client.adapter().setProtocolId(7);
  EXPECT_EQ(client.adapter().getProtocolId(), 7);
}

// =============================================================================
// onMessage Tests
// =============================================================================

TEST_F(ThriftClientAppAdapterTest, OnMessageRoutesToHandler) {
  TestAppAdapterClient client;
  auto built = buildPipeline(
      &client.adapter(), [](auto&, auto&&) { return Result::Success; });

  bool handlerCalled = false;
  ThriftResponseMessage captured;

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().write(
        [&](folly::Expected<ThriftResponseMessage, folly::exception_wrapper>&&
                result) noexcept {
          EXPECT_TRUE(result.hasValue());
          handlerCalled = true;
          captured = std::move(result.value());
        },
        makeRequestBox());
  });

  auto response = makeResponse(0);
  auto result = client.adapter().onRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handlerCalled);
  EXPECT_EQ(captured.requestHandle, 0u);
}

TEST_F(ThriftClientAppAdapterTest, OnMessageUnknownHandle) {
  TestAppAdapterClient client;
  auto built = buildPipeline(&client.adapter());

  auto response = makeResponse(99999);
  auto result = client.adapter().onRead(erase_and_box(std::move(response)));

  // Unknown handle returns Success (logged warning, not an error)
  EXPECT_EQ(result, Result::Success);
}

// =============================================================================
// onException Tests
// =============================================================================

TEST_F(ThriftClientAppAdapterTest, OnExceptionNotifiesAllHandlers) {
  TestAppAdapterClient client;
  auto built = buildPipeline(
      &client.adapter(), [](auto&, auto&&) { return Result::Success; });

  int errorCount = 0;

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().write(
        [&](folly::Expected<ThriftResponseMessage, folly::exception_wrapper>&&
                result) noexcept {
          EXPECT_TRUE(result.hasError());
          ++errorCount;
        },
        makeRequestBox());

    client.adapter().write(
        [&](folly::Expected<ThriftResponseMessage, folly::exception_wrapper>&&
                result) noexcept {
          EXPECT_TRUE(result.hasError());
          ++errorCount;
        },
        makeRequestBox());
  });

  client.adapter().onException(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));

  EXPECT_EQ(errorCount, 2);
}

TEST_F(ThriftClientAppAdapterTest, OnExceptionClearsPendingRequests) {
  TestAppAdapterClient client;
  auto built = buildPipeline(
      &client.adapter(), [](auto&, auto&&) { return Result::Success; });

  int errorCount = 0;

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().write(
        [&](folly::Expected<
            ThriftResponseMessage,
            folly::exception_wrapper>&&) noexcept { ++errorCount; },
        makeRequestBox());
  });

  client.adapter().onException(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));
  EXPECT_EQ(errorCount, 1);

  // Subsequent onMessage for handle 0 should be a no-op
  // (handle was cleared by onException)
  auto response = makeResponse(0);
  auto result = client.adapter().onRead(erase_and_box(std::move(response)));
  EXPECT_EQ(result, Result::Success); // Unknown handle -> Success
  EXPECT_EQ(errorCount, 1); // Handler not called again
}

// =============================================================================
// write Tests
// =============================================================================

TEST_F(ThriftClientAppAdapterTest, WriteSetsRequestHandle) {
  TestAppAdapterClient client;
  uint32_t capturedHandle = 0;

  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        capturedHandle = box.get<ThriftRequestMessage>().requestHandle;
        return Result::Success;
      });

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().write([](auto&&) noexcept {}, makeRequestBox());
  });

  EXPECT_EQ(capturedHandle, 0u);

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().write([](auto&&) noexcept {}, makeRequestBox());
  });

  EXPECT_EQ(capturedHandle, 1u);
}

TEST_F(ThriftClientAppAdapterTest, WriteWithoutPipelineReturnsInternalError) {
  // No setPipeline() call — pre-send must reject with INTERNAL_ERROR.
  ThriftClientAppAdapter::Ptr adapter{new ThriftClientAppAdapter()};

  bool errorReceived = false;
  folly::exception_wrapper capturedError;

  adapter->write(
      [&](folly::Expected<ThriftResponseMessage, folly::exception_wrapper>&&
              result) noexcept {
        ASSERT_TRUE(result.hasError());
        errorReceived = true;
        capturedError = std::move(result.error());
      },
      makeRequestBox());

  EXPECT_TRUE(errorReceived);
  auto* ex =
      capturedError.get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::TApplicationException::INTERNAL_ERROR);
  EXPECT_EQ(std::string(ex->what()), "Pipeline not set");
}

TEST_F(ThriftClientAppAdapterTest, WriteFromOffEventBaseThreadCompletes) {
  // Caller is the test thread (not evb_'s thread). write() must take the
  // slow path and schedule onto the EventBase before invoking the handler.
  TestAppAdapterClient client;
  bool writeCalled = false;
  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        writeCalled = true;
        return Result::Success;
      });

  client.adapter().write([](auto&&) noexcept {}, makeRequestBox());

  // Drain the EB to let the scheduled lambda run.
  evb_->runInEventBaseThreadAndWait([] {});
  EXPECT_TRUE(writeCalled);
}

TEST_F(ThriftClientAppAdapterTest, WriteFiresWrite) {
  TestAppAdapterClient client;
  bool writeCalled = false;

  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        writeCalled = true;
        return Result::Success;
      });

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().write([](auto&&) noexcept {}, makeRequestBox());
  });

  EXPECT_TRUE(writeCalled);
}

} // namespace apache::thrift::fast_thrift::thrift
