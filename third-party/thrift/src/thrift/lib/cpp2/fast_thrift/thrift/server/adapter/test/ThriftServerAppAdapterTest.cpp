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
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
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

// Test helpers — extract data/metadata/streamId/errorCode from
// ThriftServerResponseMessage's variant payload regardless of held alternative.
inline const std::unique_ptr<folly::IOBuf>& payloadData(
    const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftResponsePayload>()) {
    return msg.payload.get<ThriftResponsePayload>().data;
  }
  return msg.payload.get<ThriftErrorPayload>().data;
}
inline const std::unique_ptr<folly::IOBuf>& payloadMetadata(
    const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftResponsePayload>()) {
    return msg.payload.get<ThriftResponsePayload>().metadata;
  }
  return msg.payload.get<ThriftErrorPayload>().metadata;
}
inline uint32_t payloadStreamId(const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftResponsePayload>()) {
    return msg.payload.get<ThriftResponsePayload>().streamId;
  }
  return msg.payload.get<ThriftErrorPayload>().streamId;
}
inline uint32_t payloadErrorCode(const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftErrorPayload>()) {
    return msg.payload.get<ThriftErrorPayload>().errorCode;
  }
  return 0;
}

// Test subclass that exposes addMethodHandler for testing.
class TestServerAppAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<TestServerAppAdapter, Destructor>;

  void registerMethod(std::string_view name, RequestResponseProcessFn handler) {
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

apache::thrift::ResponseRpcError deserializeResponseRpcError(
    const folly::IOBuf& buf) {
  apache::thrift::ResponseRpcError error;
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(&buf);
  error.read(&reader);
  return error;
}

apache::thrift::ResponseRpcMetadata deserializeResponseMetadata(
    const folly::IOBuf& buf) {
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(&buf);
  metadata.read(&reader);
  return metadata;
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
          uint32_t streamId,
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::ProtocolId) noexcept -> Result {
        auto* t = static_cast<TestServerAppAdapter*>(self);
        t->handlerCalled = true;
        t->capturedStreamId = streamId;
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
  uint32_t capturedErrorCode = 0;
  apache::thrift::ResponseRpcErrorCode capturedRpcErrorCode{};

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedErrorCode = payloadErrorCode(resp);
        if (auto& d = payloadData(resp); d) {
          auto rpcError = deserializeResponseRpcError(*d);
          capturedRpcErrorCode = *rpcError.code();
        }
        return Result::Success;
      });

  auto msg = makeRequestMessage(1, "nonExistentMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled) << "Should send error response for unknown method";
  EXPECT_NE(capturedErrorCode, 0u) << "Should be ERROR frame (errorCode != 0)";
  EXPECT_EQ(
      capturedRpcErrorCode,
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD);
}

TEST_F(ThriftServerAppAdapterTest, OnReadPassesProtocolId) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  adapter->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          uint32_t,
          std::unique_ptr<folly::IOBuf>,
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
          uint32_t,
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::ProtocolId) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->method1Count++;
        return Result::Success;
      });

  adapter->registerMethod(
      "method2",
      +[](ThriftServerAppAdapter* self,
          uint32_t,
          std::unique_ptr<folly::IOBuf>,
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
        capturedStreamId = payloadStreamId(resp);
        return Result::Success;
      });

  auto result = adapter->writeResponse(
      /*streamId=*/42,
      folly::IOBuf::copyBuffer("response"),
      /*metadata=*/nullptr,
      /*complete=*/true);

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled);
  EXPECT_EQ(capturedStreamId, 42u);
}

TEST_F(ThriftServerAppAdapterTest, WriteResponseWithNoPipelineReturnsError) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  // No pipeline set

  auto result = adapter->writeResponse(
      /*streamId=*/1,
      folly::IOBuf::copyBuffer("response"),
      /*metadata=*/nullptr,
      /*complete=*/true);

  // Without a pipeline there's nothing to write to — surface that as Error.
  EXPECT_EQ(result, Result::Error);
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
  uint32_t capturedErrorCode = 0;
  apache::thrift::ResponseRpcErrorCode capturedRpcErrorCode{};

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedErrorCode = payloadErrorCode(resp);
        if (auto& d = payloadData(resp); d) {
          auto rpcError = deserializeResponseRpcError(*d);
          capturedRpcErrorCode = *rpcError.code();
        }
        return Result::Success;
      });

  // FNF frame is not REQUEST_RESPONSE, so should trigger error response
  auto msg = makeFnfRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled) << "Should send error response for unsupported "
                              "frame type (not silently drop)";
  EXPECT_NE(capturedErrorCode, 0u) << "Should be ERROR frame (errorCode != 0)";
  EXPECT_EQ(
      capturedRpcErrorCode,
      apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND);
}

// =============================================================================
// writeAppError Tests
// =============================================================================

namespace {

// Captured fields from a writeAppError invocation, used for assertions.
struct CapturedAppError {
  bool writeCalled{false};
  uint32_t errorCode{0};
  std::string name;
  std::string what;
  bool hasBlame{false};
  apache::thrift::ErrorBlame blame{};
};

} // namespace

TEST_F(ThriftServerAppAdapterTest, WriteAppErrorWithClientBlame) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  CapturedAppError captured;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        captured.writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        captured.errorCode = payloadErrorCode(resp);
        if (auto& m = payloadMetadata(resp); m) {
          auto meta = deserializeResponseMetadata(*m);
          if (auto pmRef = meta.payloadMetadata(); pmRef &&
              pmRef->getType() ==
                  apache::thrift::PayloadMetadata::Type::exceptionMetadata) {
            auto& exBase = pmRef->get_exceptionMetadata();
            if (auto n = exBase.name_utf8()) {
              captured.name = *n;
            }
            if (auto w = exBase.what_utf8()) {
              captured.what = *w;
            }
            if (auto metaRef = exBase.metadata(); metaRef &&
                metaRef->getType() ==
                    apache::thrift::PayloadExceptionMetadata::Type::
                        appUnknownException) {
              auto& appEx = metaRef->get_appUnknownException();
              if (auto ec = appEx.errorClassification()) {
                if (auto b = ec->blame()) {
                  captured.hasBlame = true;
                  captured.blame = *b;
                }
              }
            }
          }
        }
        return Result::Success;
      });

  EXPECT_EQ(
      adapter->writeResponse(
          /*streamId=*/7,
          /*data=*/nullptr,
          makeAppErrorResponseMetadata(
              "my.thrift.MyAppError",
              "client did bad",
              apache::thrift::ErrorBlame::CLIENT),
          /*complete=*/true),
      Result::Success);

  EXPECT_TRUE(captured.writeCalled);
  EXPECT_EQ(captured.errorCode, 0u) << "Should be PAYLOAD frame";
  EXPECT_EQ(captured.name, "my.thrift.MyAppError");
  EXPECT_EQ(captured.what, "client did bad");
  EXPECT_TRUE(captured.hasBlame);
  EXPECT_EQ(captured.blame, apache::thrift::ErrorBlame::CLIENT);
}

TEST_F(ThriftServerAppAdapterTest, WriteAppErrorWithServerBlame) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  CapturedAppError captured;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        captured.writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        captured.errorCode = payloadErrorCode(resp);
        if (auto& m = payloadMetadata(resp); m) {
          auto meta = deserializeResponseMetadata(*m);
          if (auto pmRef = meta.payloadMetadata(); pmRef &&
              pmRef->getType() ==
                  apache::thrift::PayloadMetadata::Type::exceptionMetadata) {
            auto& exBase = pmRef->get_exceptionMetadata();
            if (auto n = exBase.name_utf8()) {
              captured.name = *n;
            }
            if (auto w = exBase.what_utf8()) {
              captured.what = *w;
            }
            if (auto metaRef = exBase.metadata(); metaRef &&
                metaRef->getType() ==
                    apache::thrift::PayloadExceptionMetadata::Type::
                        appUnknownException) {
              auto& appEx = metaRef->get_appUnknownException();
              if (auto ec = appEx.errorClassification()) {
                if (auto b = ec->blame()) {
                  captured.hasBlame = true;
                  captured.blame = *b;
                }
              }
            }
          }
        }
        return Result::Success;
      });

  EXPECT_EQ(
      adapter->writeResponse(
          /*streamId=*/7,
          /*data=*/nullptr,
          makeAppErrorResponseMetadata(
              "my.thrift.MyAppError",
              "server bug",
              apache::thrift::ErrorBlame::SERVER),
          /*complete=*/true),
      Result::Success);

  EXPECT_TRUE(captured.writeCalled);
  EXPECT_EQ(captured.errorCode, 0u) << "Should be PAYLOAD frame";
  EXPECT_EQ(captured.name, "my.thrift.MyAppError");
  EXPECT_EQ(captured.what, "server bug");
  EXPECT_TRUE(captured.hasBlame);
  EXPECT_EQ(captured.blame, apache::thrift::ErrorBlame::SERVER);
}

// =============================================================================
// handleRequestResponse Metadata Deserialization Failure Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterTest,
    HandleRequestResponseBadMetadataSendsErrorResponse) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  uint32_t capturedErrorCode = 0;
  apache::thrift::ResponseRpcErrorCode capturedRpcErrorCode{};

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedErrorCode = payloadErrorCode(resp);
        if (auto& d = payloadData(resp); d) {
          auto rpcError = deserializeResponseRpcError(*d);
          capturedRpcErrorCode = *rpcError.code();
        }
        return Result::Success;
      });

  auto msg = makeInvalidMetadataMessage(1);
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled) << "Should send error response when metadata "
                              "deserialization fails (not silently drop)";
  EXPECT_NE(capturedErrorCode, 0u) << "Should be ERROR frame (errorCode != 0)";
  EXPECT_EQ(
      capturedRpcErrorCode,
      apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE);
}

// =============================================================================
// writeResponse Result Check Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, WriteResponseClosesPipelineOnWriteFailure) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        return Result::Error; // Simulate pipeline write failure
      });

  EXPECT_EQ(built.handler->handlerRemovedCount(), 0);

  auto result = adapter->writeResponse(
      /*streamId=*/42,
      folly::IOBuf::copyBuffer("response"),
      /*metadata=*/nullptr,
      /*complete=*/true);

  // Adapter must surface the Error to its caller and close the pipeline so
  // subsequent writes on the dead connection short-circuit. Pipeline close
  // calls handlerRemoved on every handler — we use that as the witness.
  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(built.handler->handlerRemovedCount(), 1);
}

// =============================================================================
// Unsupported RPC Kind Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterTest,
    HandleRequestResponseRejectsUnsupportedRpcKind) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  uint32_t capturedErrorCode = 0;
  apache::thrift::ResponseRpcErrorCode capturedRpcErrorCode{};

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedErrorCode = payloadErrorCode(resp);
        if (auto& d = payloadData(resp); d) {
          auto rpcError = deserializeResponseRpcError(*d);
          capturedRpcErrorCode = *rpcError.code();
        }
        return Result::Success;
      });

  // Streaming RPC kind in a REQUEST_RESPONSE frame should be rejected
  auto msg = makeStreamingRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeCalled)
      << "Should send error response for unsupported RPC kind";
  EXPECT_NE(capturedErrorCode, 0u) << "Should be ERROR frame (errorCode != 0)";
  EXPECT_EQ(
      capturedRpcErrorCode,
      apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND);
}

TEST_F(
    ThriftServerAppAdapterTest, WriteResponseErrorProducesDeclaredException) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  bool writeCalled = false;
  uint32_t capturedErrorCode = 0;
  std::string capturedName;
  std::string capturedWhat;
  bool hasDeclaredException = false;
  bool hasClassification = false;
  apache::thrift::ErrorBlame capturedBlame{};
  std::string capturedData;

  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        capturedErrorCode = payloadErrorCode(resp);
        if (auto& d = payloadData(resp); d) {
          capturedData = d->moveToFbString().toStdString();
        }
        if (auto& m = payloadMetadata(resp); m) {
          auto meta = deserializeResponseMetadata(*m);
          if (auto pmRef = meta.payloadMetadata(); pmRef &&
              pmRef->getType() ==
                  apache::thrift::PayloadMetadata::Type::exceptionMetadata) {
            auto& exBase = pmRef->get_exceptionMetadata();
            if (auto n = exBase.name_utf8()) {
              capturedName = *n;
            }
            if (auto w = exBase.what_utf8()) {
              capturedWhat = *w;
            }
            if (auto metaRef = exBase.metadata(); metaRef &&
                metaRef->getType() ==
                    apache::thrift::PayloadExceptionMetadata::Type::
                        declaredException) {
              hasDeclaredException = true;
              auto& declared = metaRef->get_declaredException();
              if (auto ec = declared.errorClassification()) {
                if (auto b = ec->blame()) {
                  hasClassification = true;
                  capturedBlame = *b;
                }
              }
            }
          }
        }
        return Result::Success;
      });

  apache::thrift::ErrorClassification classification;
  classification.blame() = apache::thrift::ErrorBlame::CLIENT;

  EXPECT_EQ(
      adapter->writeResponse(
          /*streamId=*/7,
          folly::IOBuf::copyBuffer("serialized exception struct"),
          makeDeclaredExceptionMetadata(
              "my.thrift.MyDeclaredException",
              "expected failure",
              classification),
          /*complete=*/true),
      Result::Success);

  EXPECT_TRUE(writeCalled);
  EXPECT_EQ(capturedErrorCode, 0u) << "Should be PAYLOAD frame";
  EXPECT_EQ(capturedData, "serialized exception struct")
      << "Exception data should be carried in payload.data";
  EXPECT_EQ(capturedName, "my.thrift.MyDeclaredException");
  EXPECT_EQ(capturedWhat, "expected failure");
  EXPECT_TRUE(hasDeclaredException);
  EXPECT_TRUE(hasClassification);
  EXPECT_EQ(capturedBlame, apache::thrift::ErrorBlame::CLIENT);
}

// =============================================================================
// writeUnknownException / writeFrameworkError member helpers
// =============================================================================

// Undeclared exception → PAYLOAD frame (errorCode=0), null data, metadata
// carrying appUnknownException with the wrapper's name + what + caller's
// blame. Mirrors the legacy process_throw_wrapped_handler_error fallback.
TEST_F(
    ThriftServerAppAdapterTest, WriteUnknownExceptionEmitsAppUnknownPayload) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  ThriftServerResponseMessage captured;
  bool writeCalled = false;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        captured = std::move(box.get<ThriftServerResponseMessage>());
        return Result::Success;
      });

  auto ew = folly::make_exception_wrapper<std::runtime_error>("boom");
  EXPECT_EQ(
      adapter->writeUnknownException(7, ew, apache::thrift::ErrorBlame::CLIENT),
      Result::Success);

  ASSERT_TRUE(writeCalled);
  EXPECT_EQ(payloadStreamId(captured), 7u);
  EXPECT_EQ(payloadErrorCode(captured), 0u) << "PAYLOAD frame, not ERROR";
  EXPECT_EQ(payloadData(captured), nullptr);
  EXPECT_TRUE(captured.payload.get<ThriftResponsePayload>().complete);

  auto& md_buf = payloadMetadata(captured);
  ASSERT_NE(md_buf, nullptr);
  auto md = deserializeResponseMetadata(*md_buf);
  ASSERT_TRUE(md.payloadMetadata().has_value());
  auto& base = md.payloadMetadata()->get_exceptionMetadata();
  ASSERT_TRUE(base.metadata().has_value());
  EXPECT_EQ(
      base.metadata()->getType(),
      apache::thrift::PayloadExceptionMetadata::Type::appUnknownException);
  EXPECT_EQ(
      base.metadata()->get_appUnknownException().errorClassification()->blame(),
      apache::thrift::ErrorBlame::CLIENT);
  // exception_wrapper::what() prepends the class name; assert substring so
  // we don't pin folly's formatting.
  EXPECT_NE(base.what_utf8()->find("boom"), std::string::npos);
  EXPECT_NE(base.name_utf8()->find("runtime_error"), std::string::npos);
}

// Framework error → ERROR frame. errorCode is the rocket code mapped from
// the ResponseRpcErrorCode's category (REQUEST_PARSING_FAILURE is in the
// INVALID_REQUEST category → INVALID); data carries a CompactProtocol-
// serialized ResponseRpcError with the original code + message.
TEST_F(ThriftServerAppAdapterTest, WriteFrameworkErrorEmitsErrorFrame) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  ThriftServerResponseMessage captured;
  bool writeCalled = false;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        captured = std::move(box.get<ThriftServerResponseMessage>());
        return Result::Success;
      });

  EXPECT_EQ(
      adapter->writeFrameworkError(
          9,
          apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
          "bad bytes"),
      Result::Success);

  ASSERT_TRUE(writeCalled);
  EXPECT_EQ(payloadStreamId(captured), 9u);
  EXPECT_EQ(
      payloadErrorCode(captured),
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID))
      << "ERROR frame; INVALID_REQUEST category maps to INVALID";

  auto& d = payloadData(captured);
  ASSERT_NE(d, nullptr);
  auto rpcError = deserializeResponseRpcError(*d);
  ASSERT_TRUE(rpcError.code().has_value());
  EXPECT_EQ(
      *rpcError.code(),
      apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE);
  EXPECT_EQ(*rpcError.what_utf8(), "bad bytes");
}

} // namespace apache::thrift::fast_thrift::thrift
