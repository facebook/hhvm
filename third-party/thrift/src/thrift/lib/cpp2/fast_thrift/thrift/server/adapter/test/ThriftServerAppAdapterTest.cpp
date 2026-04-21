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

#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

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

// Test subclass that exposes addMethodHandler for testing.
class TestServerAppAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<TestServerAppAdapter, Destructor>;

  void registerMethod(std::string_view name, ProcessFn handler) {
    addMethodHandler(name, handler);
  }

  bool handlerCalled{false};
  uint32_t capturedStreamId{0};
  apache::thrift::ProtocolId capturedProtocol{};
  int method1Count{0};
  int method2Count{0};
};

std::unique_ptr<folly::IOBuf> serializeRequestMetadata(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

ThriftServerRequestMessage makeRequestMessage(
    uint32_t streamId,
    const std::string& methodName,
    const std::string& data = "payload") {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;

  auto serializedMetadata = serializeRequestMetadata(metadata);
  auto payloadData = folly::IOBuf::copyBuffer(data);

  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId},
      std::move(serializedMetadata),
      std::move(payloadData));

  ThriftServerRequestMessage msg;
  msg.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
  msg.streamId = streamId;
  return msg;
}

ThriftServerRequestMessage makeFnfRequestMessage(
    uint32_t streamId, const std::string& methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;

  auto serializedMetadata = serializeRequestMetadata(metadata);
  auto payloadData = folly::IOBuf::copyBuffer("fnf");

  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestFnfHeader{
          .streamId = streamId},
      std::move(serializedMetadata),
      std::move(payloadData));

  ThriftServerRequestMessage msg;
  msg.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
  msg.streamId = streamId;
  return msg;
}

ThriftServerRequestMessage makeInvalidMetadataMessage(uint32_t streamId) {
  auto garbageMetadata = folly::IOBuf::copyBuffer("not-valid-thrift");
  auto payloadData = folly::IOBuf::copyBuffer("payload");

  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId},
      std::move(garbageMetadata),
      std::move(payloadData));

  ThriftServerRequestMessage msg;
  msg.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
  msg.streamId = streamId;
  return msg;
}

ThriftServerRequestMessage makeStreamingRequestMessage(
    uint32_t streamId, const std::string& methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;

  auto serializedMetadata = serializeRequestMetadata(metadata);
  auto payloadData = folly::IOBuf::copyBuffer("payload");

  // Use REQUEST_RESPONSE frame (the only type onRead accepts),
  // but with streaming RPC kind in metadata
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId},
      std::move(serializedMetadata),
      std::move(payloadData));

  ThriftServerRequestMessage msg;
  msg.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
  msg.streamId = streamId;
  return msg;
}

} // namespace

class ThriftServerAppAdapterTest : public ::testing::Test {
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
      TestServerAppAdapter* adapter,
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

    // Thrift pipeline: adapter (tail) ← handler → transport (head)
    // Reads go from head → tail (transport → adapter)
    auto pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            TestServerAppAdapter,
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
// onRead Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, OnReadDispatchesToRegisteredHandler) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  adapter->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&& request,
          apache::thrift::ProtocolId) noexcept -> Result {
        auto* t = static_cast<TestServerAppAdapter*>(self);
        t->handlerCalled = true;
        t->capturedStreamId = request.streamId;
        return Result::Success;
      });

  auto built = buildPipeline(adapter.get());

  auto msg = makeRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(adapter->handlerCalled);
  EXPECT_EQ(adapter->capturedStreamId, 1u);
}

TEST_F(ThriftServerAppAdapterTest, OnReadUnknownMethodSendsErrorResponse) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        writeCalled = true;
        return Result::Success;
      });

  auto msg = makeRequestMessage(1, "nonExistentMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled) << "Should send error response for unknown method";
}

TEST_F(ThriftServerAppAdapterTest, OnReadPassesProtocolId) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  adapter->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId protocol) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->capturedProtocol = protocol;
        return Result::Success;
      });

  auto built = buildPipeline(adapter.get());

  auto msg = makeRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(adapter->capturedProtocol, apache::thrift::ProtocolId::BINARY);
}

TEST_F(ThriftServerAppAdapterTest, OnReadMultipleMethodsDispatched) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  adapter->registerMethod(
      "method1",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->method1Count++;
        return Result::Success;
      });

  adapter->registerMethod(
      "method2",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->method2Count++;
        return Result::Success;
      });

  auto built = buildPipeline(adapter.get());

  auto msg1 = makeRequestMessage(1, "method1");
  std::ignore = adapter->onRead(erase_and_box(std::move(msg1)));

  auto msg2 = makeRequestMessage(3, "method2");
  std::ignore = adapter->onRead(erase_and_box(std::move(msg2)));

  auto msg3 = makeRequestMessage(5, "method1");
  std::ignore = adapter->onRead(erase_and_box(std::move(msg3)));

  EXPECT_EQ(adapter->method1Count, 2);
  EXPECT_EQ(adapter->method2Count, 1);
}

// =============================================================================
// onException Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, OnExceptionDefersCloseCallback) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  bool closeCalled = false;
  adapter->setCloseCallback([&] { closeCalled = true; });

  // Verify that the close callback is NOT called synchronously during
  // onException. This is critical: calling the close callback inline
  // can destroy the pipeline while it's still propagating the exception,
  // causing use-after-free.
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->onException(
        folly::make_exception_wrapper<std::runtime_error>("connection lost"));
    EXPECT_FALSE(closeCalled) << "Close callback must not fire synchronously "
                                 "during onException (use-after-free risk)";
  });

  // After draining, the deferred callback should have fired.
  evb_->runInEventBaseThreadAndWait([&] {});
  EXPECT_TRUE(closeCalled);
}

TEST_F(ThriftServerAppAdapterTest, OnExceptionWithNoCallbackDoesNotCrash) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  adapter->onException(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));
  // Should not crash
}

// =============================================================================
// writeResponse Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, WriteResponseFiresWrite) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  uint32_t capturedStreamId = 0;

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedStreamId = resp.streamId;
        return Result::Success;
      });

  ThriftServerResponseMessage response{
      .payload =
          ThriftServerResponsePayload{
              .data = folly::IOBuf::copyBuffer("response"),
              .metadata = nullptr,
              .complete = true},
      .streamId = 42};

  adapter->writeResponse(std::move(response));

  EXPECT_TRUE(writeCalled);
  EXPECT_EQ(capturedStreamId, 42u);
}

TEST_F(ThriftServerAppAdapterTest, WriteResponseWithNoPipelineDoesNotCrash) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  // No pipeline set

  ThriftServerResponseMessage response{
      .payload =
          ThriftServerResponsePayload{
              .data = folly::IOBuf::copyBuffer("response"),
              .metadata = nullptr,
              .complete = true},
      .streamId = 1};

  adapter->writeResponse(std::move(response));
  // Should not crash
}

// =============================================================================
// setPipeline Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, SetPipelineStoresPipeline) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  EXPECT_NE(adapter->pipeline(), nullptr);
}

// =============================================================================
// Close Callback Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, DestructorInvokesCloseCallback) {
  bool closeCalled = false;

  {
    TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
    auto built = buildPipeline(adapter.get());

    adapter->setCloseCallback([&] { closeCalled = true; });
    // adapter goes out of scope here
  }

  EXPECT_TRUE(closeCalled);
}

// =============================================================================
// onRead Unsupported Frame Type Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, OnReadUnsupportedFrameSendsErrorResponse) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        writeCalled = true;
        return Result::Success;
      });

  // FNF frame is not REQUEST_RESPONSE, so should trigger error response
  auto msg = makeFnfRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled) << "Should send error response for unsupported "
                              "frame type (not silently drop)";
}

// =============================================================================
// handleRequestResponse Metadata Deserialization Failure Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterTest,
    HandleRequestResponseBadMetadataSendsErrorResponse) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        writeCalled = true;
        return Result::Success;
      });

  auto msg = makeInvalidMetadataMessage(1);
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled) << "Should send error response when metadata "
                              "deserialization fails (not silently drop)";
}

// =============================================================================
// writeResponse Result Check Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterTest,
    WriteResponseLogsWarningOnPipelineWriteFailure) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        return Result::Error; // Simulate pipeline write failure
      });

  ThriftServerResponseMessage response{
      .payload =
          ThriftServerResponsePayload{
              .data = folly::IOBuf::copyBuffer("response"),
              .metadata = nullptr,
              .complete = true},
      .streamId = 42};

  // Should not crash, just log warning
  adapter->writeResponse(std::move(response));
}

// =============================================================================
// writeErr Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, WriteErrSendsErrorResponse) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  uint32_t capturedStreamId = 0;

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedStreamId = resp.streamId;
        // Verify the response has serialized data (the TApplicationException)
        EXPECT_NE(resp.payload.data, nullptr);
        EXPECT_NE(resp.payload.metadata, nullptr);
        return Result::Success;
      });

  adapter->writeErr<apache::thrift::BinaryProtocolWriter>(
      7, folly::make_exception_wrapper<std::runtime_error>("something broke"));

  EXPECT_TRUE(writeCalled) << "writeErr should send error response";
  EXPECT_EQ(capturedStreamId, 7u);
}

TEST_F(ThriftServerAppAdapterTest, WriteErrWithNoPipelineDoesNotCrash) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  // No pipeline set

  adapter->writeErr<apache::thrift::BinaryProtocolWriter>(
      1, folly::make_exception_wrapper<std::runtime_error>("error"));
  // Should not crash
}

// =============================================================================
// Unsupported RPC Kind Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterTest,
    HandleRequestResponseRejectsUnsupportedRpcKind) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        writeCalled = true;
        return Result::Success;
      });

  // Streaming RPC kind in a REQUEST_RESPONSE frame should be rejected
  auto msg = makeStreamingRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled)
      << "Should send error response for unsupported RPC kind";
}

} // namespace apache::thrift::fast_thrift::thrift
