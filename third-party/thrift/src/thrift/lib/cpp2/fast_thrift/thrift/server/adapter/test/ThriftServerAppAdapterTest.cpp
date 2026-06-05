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

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
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
  if (msg.payload.is<ThriftInitialResponsePayload>()) {
    return msg.payload.get<ThriftInitialResponsePayload>().data;
  }
  return msg.payload.get<ThriftErrorPayload>().data;
}
// Returns the typed ResponseRpcMetadata held by a ThriftInitialResponsePayload,
// or nullptr if the message carries an error payload instead.
inline const apache::thrift::ResponseRpcMetadata* payloadMetadata(
    const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftInitialResponsePayload>()) {
    return msg.payload.get<ThriftInitialResponsePayload>().metadata.get();
  }
  return nullptr;
}
inline uint32_t payloadStreamId(const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftInitialResponsePayload>()) {
    return msg.payload.get<ThriftInitialResponsePayload>().streamId;
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
  ThriftRequestContext* capturedRequestContext{nullptr};
  int method1Count{0};
  int method2Count{0};
};

apache::thrift::ResponseRpcError deserializeResponseRpcError(
    const folly::IOBuf& buf) {
  apache::thrift::ResponseRpcError error;
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(&buf);
  error.read(&reader);
  return error;
}

ThriftServerRequestMessage makeTypedRequestMessage(
    uint32_t streamId,
    const std::string& methodName,
    apache::thrift::RpcKind kind,
    const std::string& data) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name() = methodName;
  metadata->kind() = kind;
  metadata->protocol() = apache::thrift::ProtocolId::BINARY;

  std::unique_ptr<folly::IOBuf> payloadData;
  if (!data.empty()) {
    payloadData = folly::IOBuf::copyBuffer(data);
  }

  ThriftServerRequestMessage msg;
  msg.streamId = streamId;
  msg.payload = ThriftServerInboundPayloadVariant{ThriftRequestResponsePayload{
      .data = std::move(payloadData), .metadata = std::move(metadata)}};
  return msg;
}

ThriftServerRequestMessage makeRequestMessage(
    uint32_t streamId,
    const std::string& methodName,
    const std::string& data = "payload") {
  return makeTypedRequestMessage(
      streamId,
      methodName,
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      data);
}

// Mirrors the old "FNF wire frame" test fixture in the new typed-payload
// world: AppAdapter rejection now triggers off metadata.kind, not the
// originating wire frame type (transport adapter would have already
// rejected non-RR wire frames at fromRocketFrame).
ThriftServerRequestMessage makeFnfRequestMessage(
    uint32_t streamId, const std::string& methodName) {
  return makeTypedRequestMessage(
      streamId,
      methodName,
      apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      "fnf");
}

ThriftServerRequestMessage makeStreamingRequestMessage(
    uint32_t streamId, const std::string& methodName) {
  return makeTypedRequestMessage(
      streamId,
      methodName,
      apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE,
      "payload");
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
    // Raw ptr — owned by the test body's local adapter Ptr. Held here so
    // ~BuiltPipeline can release the adapter's pipelineGuard_ before its
    // own `pipeline` field auto-destroys; otherwise the pipeline's
    // destroy() is deferred by the guard and fires synchronously inside
    // ~ThriftServerAppAdapter -> 4-byte UAF on a partially-destroyed tail.
    TestServerAppAdapter* adapter{nullptr};

    ~BuiltPipeline() {
      if (transportHandler) {
        transportHandler->close(folly::exception_wrapper{});
        transportHandler->resetPipeline();
      }
      if (adapter) {
        adapter->resetPipeline();
      }
    }
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
    // Move adapter into Open so onRead/writeResponse are accepted. In real
    // server flows this happens via TransportHandler::onConnect -> pipeline
    // activate -> handler onPipelineActive.
    adapter->onPipelineActive();

    return {
        std::move(transportHandler), std::move(pipeline), rawHandler, adapter};
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
          apache::thrift::ProtocolId,
          std::unique_ptr<ThriftRequestContext>) noexcept {
        auto* t = static_cast<TestServerAppAdapter*>(self);
        t->handlerCalled = true;
        t->capturedStreamId = streamId;
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
          apache::thrift::ProtocolId protocol,
          std::unique_ptr<ThriftRequestContext>) noexcept {
        static_cast<TestServerAppAdapter*>(self)->capturedProtocol = protocol;
      });

  auto built = buildPipeline(adapter.get());

  auto msg = makeRequestMessage(1, "testMethod");
  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(adapter->capturedProtocol, apache::thrift::ProtocolId::BINARY);
}

TEST_F(ThriftServerAppAdapterTest, OnReadForwardsRequestContextToHandler) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  adapter->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          uint32_t,
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::ProtocolId,
          std::unique_ptr<ThriftRequestContext> requestContext) noexcept {
        static_cast<TestServerAppAdapter*>(self)->capturedRequestContext =
            requestContext.get();
      });

  auto built = buildPipeline(adapter.get());

  auto msg = makeRequestMessage(1, "testMethod");
  auto* stampedContext = new ThriftRequestContext();
  msg.requestContext.reset(stampedContext);

  auto result = adapter->onRead(erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(adapter->capturedRequestContext, stampedContext);
}

TEST_F(ThriftServerAppAdapterTest, OnReadMultipleMethodsDispatched) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  adapter->registerMethod(
      "method1",
      +[](ThriftServerAppAdapter* self,
          uint32_t,
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::ProtocolId,
          std::unique_ptr<ThriftRequestContext>) noexcept {
        static_cast<TestServerAppAdapter*>(self)->method1Count++;
      });

  adapter->registerMethod(
      "method2",
      +[](ThriftServerAppAdapter* self,
          uint32_t,
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::ProtocolId,
          std::unique_ptr<ThriftRequestContext>) noexcept {
        static_cast<TestServerAppAdapter*>(self)->method2Count++;
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
// onPipelineInactive close-callback tests
// =============================================================================

TEST_F(ThriftServerAppAdapterTest, OnConnectionClosedEventDefersCloseCallback) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  bool closeCalled = false;
  adapter->setCloseCallback([&] { closeCalled = true; });

  // closeCallback fires in response to a ConnectionClosed inbound event
  // (the upstream close handler's "in-flight settled" signal) but is
  // deferred onto the EVB — firing synchronously could destroy the
  // pipeline mid-walk.
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->onEvent(ThriftServerEventType::ConnectionClosed, TypeErasedBox{});
    EXPECT_FALSE(closeCalled)
        << "Close callback must not fire synchronously during onEvent "
           "(use-after-free risk)";
  });

  evb_->runInEventBaseThreadAndWait([&] {});
  EXPECT_TRUE(closeCalled);
}

TEST_F(ThriftServerAppAdapterTest, OnPipelineInactiveIsNoopWithNoCallbackSet) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  evb_->runInEventBaseThreadAndWait([&] { adapter->onPipelineInactive(); });
  // onPipelineInactive no longer carries any adapter-side state work;
  // the only requirement is that it doesn't crash.
}

TEST_F(ThriftServerAppAdapterTest, OnPipelineInactiveDoesNotFireCloseCallback) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  bool closeCalled = false;
  adapter->setCloseCallback([&] { closeCalled = true; });

  // The adapter is intentionally close-coordination-unaware post-refactor;
  // closeCallback only fires from the upstream ConnectionClosed event.
  evb_->runInEventBaseThreadAndWait([&] { adapter->onPipelineInactive(); });
  evb_->runInEventBaseThreadAndWait([&] {});
  EXPECT_FALSE(closeCalled);

  // Clear so the dtor fallback doesn't fire it after closeCalled goes
  // out of scope.
  adapter->setCloseCallback({});
}

TEST_F(ThriftServerAppAdapterTest, OnExceptionDoesNotFireCloseCallback) {
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};
  auto built = buildPipeline(adapter.get());

  bool closeCalled = false;
  adapter->setCloseCallback([&] { closeCalled = true; });

  // closeCallback only fires from onPipelineInactive — the canonical
  // teardown edge. Pipeline-level exceptions, benign or otherwise, must
  // not fire it on their own.
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->onException(
        folly::make_exception_wrapper<
            apache::thrift::transport::TTransportException>(
            apache::thrift::transport::TTransportException::END_OF_FILE,
            "peer FIN"));
    adapter->onException(
        folly::make_exception_wrapper<std::runtime_error>("protocol error"));
  });
  evb_->runInEventBaseThreadAndWait([&] {});
  EXPECT_FALSE(closeCalled);

  // Clear the callback so the dtor fallback doesn't fire it after
  // closeCalled goes out of scope.
  adapter->setCloseCallback({});
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

  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeResponseMessage(
        /*streamId=*/42,
        folly::IOBuf::copyBuffer("response"),
        /*metadata=*/nullptr));
  });

  EXPECT_TRUE(writeCalled);
  EXPECT_EQ(capturedStreamId, 42u);
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
        if (auto* m = payloadMetadata(resp); m) {
          if (auto pmRef = m->payloadMetadata(); pmRef &&
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

  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillAppErrorResponseMetadata(
      *md,
      "my.thrift.MyAppError",
      "client did bad",
      apache::thrift::ErrorBlame::CLIENT);
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeResponseMessage(
        /*streamId=*/7,
        /*data=*/nullptr,
        std::move(md)));
  });

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
        if (auto* m = payloadMetadata(resp); m) {
          if (auto pmRef = m->payloadMetadata(); pmRef &&
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

  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillAppErrorResponseMetadata(
      *md,
      "my.thrift.MyAppError",
      "server bug",
      apache::thrift::ErrorBlame::SERVER);
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeResponseMessage(
        /*streamId=*/7,
        /*data=*/nullptr,
        std::move(md)));
  });

  EXPECT_TRUE(captured.writeCalled);
  EXPECT_EQ(captured.errorCode, 0u) << "Should be PAYLOAD frame";
  EXPECT_EQ(captured.name, "my.thrift.MyAppError");
  EXPECT_EQ(captured.what, "server bug");
  EXPECT_TRUE(captured.hasBlame);
  EXPECT_EQ(captured.blame, apache::thrift::ErrorBlame::SERVER);
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
        if (auto* m = payloadMetadata(resp); m) {
          if (auto pmRef = m->payloadMetadata(); pmRef &&
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

  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillDeclaredExceptionMetadata(
      *md, "my.thrift.MyDeclaredException", "expected failure", classification);
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeResponseMessage(
        /*streamId=*/7,
        folly::IOBuf::copyBuffer("serialized exception struct"),
        std::move(md)));
  });

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
// Unknown exception / framework error message paths
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
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(
        makeUnknownExceptionMessage(7, ew, apache::thrift::ErrorBlame::CLIENT));
  });

  ASSERT_TRUE(writeCalled);
  EXPECT_EQ(payloadStreamId(captured), 7u);
  EXPECT_EQ(payloadErrorCode(captured), 0u) << "PAYLOAD frame, not ERROR";
  EXPECT_EQ(payloadData(captured), nullptr);
  // Terminal-and-data invariant is enforced in toRocketFrame and verified
  // by ThriftPayloadTest.

  const auto* md = payloadMetadata(captured);
  ASSERT_NE(md, nullptr);
  ASSERT_TRUE(md->payloadMetadata().has_value());
  auto& base = md->payloadMetadata()->get_exceptionMetadata();
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

  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeFrameworkErrorMessage(
        9,
        apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
        "bad bytes"));
  });

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

// The adapter no longer carries its own terminal-phase state machine.
// Close/drain semantics (CloseConnection event emission, in-flight
// tracking, deferred closeCallback via ConnectionClosed inbound event)
// live entirely in ThriftServerConnectionCloseHandler and are covered
// by ThriftServerConnectionCloseHandlerTest.

// =============================================================================
// Force-close: writeResponse is dropped after ConnectionClosed
// =============================================================================

TEST_F(
    ThriftServerAppAdapterTest, WriteResponseAfterConnectionClosedIsDropped) {
  // Stragglers (in-flight FastHandlerCallbacks completing after the
  // reap timer's force-close) still call writeResponse. The adapter
  // must drop the write silently — pipelineGuard_ has been released
  // and pipeline_ may be dangling, so dereferencing it would UAF.
  TestServerAppAdapter::Ptr adapter{new TestServerAppAdapter()};

  int writeCount = 0;
  auto built = buildPipeline(
      adapter.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        ++writeCount;
        return Result::Success;
      });

  // Sanity: writes work before close.
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeResponseMessage(
        /*streamId=*/1,
        folly::IOBuf::copyBuffer("ok"),
        /*metadata=*/nullptr));
  });
  ASSERT_EQ(writeCount, 1);

  // Simulate the reap-timeout force-close — close handler fires
  // ConnectionClosed inbound.
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->onEvent(ThriftServerEventType::ConnectionClosed, TypeErasedBox{});
  });

  // Straggler write — must NOT dispatch through the pipeline.
  evb_->runInEventBaseThreadAndWait([&] {
    adapter->writeResponse(makeResponseMessage(
        /*streamId=*/2,
        folly::IOBuf::copyBuffer("late"),
        /*metadata=*/nullptr));
  });

  EXPECT_EQ(writeCount, 1)
      << "Straggler write after ConnectionClosed must be dropped silently";
}

} // namespace apache::thrift::fast_thrift::thrift
