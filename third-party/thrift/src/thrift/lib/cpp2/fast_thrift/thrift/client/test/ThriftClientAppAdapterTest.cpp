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
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
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

// Test client that holds the adapter as a member (composition).
class TestAppAdapterClient {
 public:
  ThriftClientAppAdapter& adapter() { return *adapter_; }

 private:
  ThriftClientAppAdapter::Ptr adapter_{new ThriftClientAppAdapter()};
};

ThriftResponseMessage makeResponse(
    void* requestContext,
    apache::thrift::fast_thrift::frame::FrameType frameType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE) {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftInitialResponsePayload{
          .data = folly::IOBuf::copyBuffer("response"),
          .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(),
          .streamId = 1,
      },
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  response.requestContext =
      apache::thrift::fast_thrift::rocket::borrow(requestContext);
  response.streamType = frameType;
  return response;
}

void sendBasicRequest(
    ThriftClientAppAdapter& adapter,
    ThriftClientAppAdapter::RequestResponseHandler handler) {
  apache::thrift::RpcOptions options;
  adapter.sendRequestResponse(
      options,
      std::string_view{"method"},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      folly::IOBuf::copyBuffer("data"),
      std::move(handler));
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
    ThriftClientAppAdapter* adapter{nullptr};

    ~BuiltPipeline() {
      if (transportHandler) {
        transportHandler->close(folly::exception_wrapper{});
        transportHandler->resetPipeline();
      }
      if (pipeline) {
        pipeline->deactivate();
        pipeline->close();
      }
      if (adapter) {
        adapter->resetPipeline();
      }
    }
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

    return {
        std::move(transportHandler), std::move(pipeline), rawHandler, adapter};
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
  void* capturedUserContext = nullptr;
  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        capturedUserContext =
            box.get<ThriftRequestMessage>().requestContext.release();
        return Result::Success;
      });

  bool handlerCalled = false;
  std::string capturedData;

  evb_->runInEventBaseThreadAndWait([&] {
    sendBasicRequest(
        client.adapter(),
        [&](folly::Expected<
                std::unique_ptr<folly::IOBuf>,
                folly::exception_wrapper>&& result,
            const apache::thrift::RpcTransportStats&) noexcept {
          EXPECT_TRUE(result.hasValue());
          handlerCalled = true;
          if (result.value()) {
            capturedData = result.value()->moveToFbString().toStdString();
          }
        });
  });

  ASSERT_NE(capturedUserContext, nullptr);
  auto response = makeResponse(capturedUserContext);
  auto result = client.adapter().onRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handlerCalled);
  EXPECT_EQ(capturedData, "response");
}

TEST_F(ThriftClientAppAdapterTest, OnMessageNullUserContextSucceeds) {
  TestAppAdapterClient client;
  auto built = buildPipeline(&client.adapter());

  // Response without a requestContext stamped — adapter logs and returns
  // Success.
  auto response = makeResponse(nullptr);
  auto result = client.adapter().onRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
}

// =============================================================================
// sendRequestResponse Tests
// =============================================================================

TEST_F(ThriftClientAppAdapterTest, EachRequestGetsUniqueUserContext) {
  TestAppAdapterClient client;
  std::vector<void*> capturedUserContexts;

  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        capturedUserContexts.push_back(
            box.get<ThriftRequestMessage>().requestContext.release());
        return Result::Success;
      });

  evb_->runInEventBaseThreadAndWait([&] {
    sendBasicRequest(client.adapter(), [](auto&&, const auto&) noexcept {});
  });

  ASSERT_EQ(capturedUserContexts.size(), 1u);
  EXPECT_NE(capturedUserContexts[0], nullptr);

  evb_->runInEventBaseThreadAndWait([&] {
    sendBasicRequest(client.adapter(), [](auto&&, const auto&) noexcept {});
  });

  ASSERT_EQ(capturedUserContexts.size(), 2u);
  EXPECT_NE(capturedUserContexts[1], nullptr);
  EXPECT_NE(capturedUserContexts[0], capturedUserContexts[1]);

  // Deliver synthetic responses so per-request contexts are freed.
  evb_->runInEventBaseThreadAndWait([&] {
    for (auto* ctx : capturedUserContexts) {
      (void)client.adapter().onRead(erase_and_box(makeResponse(ctx)));
    }
  });
}

TEST_F(ThriftClientAppAdapterTest, OffEventBaseThreadSchedulesWrite) {
  // Caller is the test thread (not evb_'s thread). The adapter must take
  // the slow path and schedule onto the EventBase before invoking the
  // handler.
  TestAppAdapterClient client;
  bool writeCalled = false;
  void* capturedUserContext = nullptr;
  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        capturedUserContext =
            box.get<ThriftRequestMessage>().requestContext.release();
        return Result::Success;
      });

  sendBasicRequest(client.adapter(), [](auto&&, const auto&) noexcept {});

  // Drain the EB to let the scheduled lambda run.
  evb_->runInEventBaseThreadAndWait([] {});
  EXPECT_TRUE(writeCalled);

  // Deliver a synthetic response so the per-request context is freed.
  evb_->runInEventBaseThreadAndWait([&] {
    (void)client.adapter().onRead(
        erase_and_box(makeResponse(capturedUserContext)));
  });
}

TEST_F(ThriftClientAppAdapterTest, SendRequestResponseBuildsRequestMessage) {
  TestAppAdapterClient client;
  client.adapter().setProtocolId(
      static_cast<uint16_t>(apache::thrift::ProtocolId::COMPACT));

  std::unique_ptr<apache::thrift::RequestRpcMetadata> capturedMetadata;
  std::unique_ptr<folly::IOBuf> capturedData;
  apache::thrift::RpcKind capturedRpcKind{};
  void* capturedUserContext = nullptr;

  auto built = buildPipeline(
      &client.adapter(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        auto& msg = box.get<ThriftRequestMessage>();
        auto& reqResp = msg.payload.get<ThriftRequestResponsePayload>();
        capturedMetadata = std::move(reqResp.metadata);
        capturedData = std::move(reqResp.data);
        capturedRpcKind = msg.payload.rpcKind();
        capturedUserContext = msg.requestContext.release();
        return Result::Success;
      });

  apache::thrift::RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(250));

  evb_->runInEventBaseThreadAndWait([&] {
    client.adapter().sendRequestResponse(
        options,
        std::string_view{"myMethod"},
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
        folly::IOBuf::copyBuffer("payload-data"),
        [](auto&&, const auto&) noexcept {});
  });

  ASSERT_NE(capturedMetadata, nullptr);
  ASSERT_NE(capturedData, nullptr);
  EXPECT_EQ(
      capturedRpcKind, apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_EQ(
      capturedData->moveToFbString().toStdString(),
      std::string{"payload-data"});

  EXPECT_EQ(capturedMetadata->name()->str(), "myMethod");
  EXPECT_EQ(
      *capturedMetadata->kind(),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_EQ(*capturedMetadata->protocol(), apache::thrift::ProtocolId::COMPACT);
  EXPECT_EQ(*capturedMetadata->clientTimeoutMs(), 250);

  // Deliver a synthetic response so the per-request context is freed.
  evb_->runInEventBaseThreadAndWait([&] {
    (void)client.adapter().onRead(
        erase_and_box(makeResponse(capturedUserContext)));
  });
}

TEST_F(
    ThriftClientAppAdapterTest,
    SendRequestResponseWithoutPipelineReturnsInternalError) {
  ThriftClientAppAdapter::Ptr adapter{new ThriftClientAppAdapter()};

  bool errorReceived = false;
  folly::exception_wrapper capturedError;

  apache::thrift::RpcOptions options;
  adapter->sendRequestResponse(
      options,
      std::string_view{"myMethod"},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      folly::IOBuf::copyBuffer("data"),
      [&](folly::Expected<
              std::unique_ptr<folly::IOBuf>,
              folly::exception_wrapper>&& result,
          const apache::thrift::RpcTransportStats&) noexcept {
        ASSERT_TRUE(result.hasError());
        errorReceived = true;
        capturedError = std::move(result.error());
      });

  EXPECT_TRUE(errorReceived);
  auto* ex =
      capturedError.get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::TApplicationException::INTERNAL_ERROR);
  EXPECT_EQ(std::string(ex->what()), "Pipeline not set");
}

} // namespace apache::thrift::fast_thrift::thrift
