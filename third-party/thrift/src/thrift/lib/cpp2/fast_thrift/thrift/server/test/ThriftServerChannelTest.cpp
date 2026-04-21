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
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::channel_pipeline::test::MockHandler;
using MockTransportHandler =
    apache::thrift::fast_thrift::channel_pipeline::test::MockHeadHandler;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;

HANDLER_TAG(test_handler);

namespace {

/**
 * Deserialize ResponseRpcMetadata from a pre-serialized IOBuf.
 * Used by tests to inspect response metadata that ThriftServerChannel
 * pre-serializes into the ThriftServerResponseMessage.
 */
apache::thrift::ResponseRpcMetadata deserializeResponseMetadata(
    const folly::IOBuf& buf) {
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(&buf);
  metadata.read(&reader);
  return metadata;
}

/**
 * MockAsyncProcessor - captures requests for test verification.
 *
 * When a request arrives, calls the configured response callback
 * to exercise sendReply/sendErrorWrapped paths.
 */
class MockAsyncProcessor : public apache::thrift::AsyncProcessor {
 public:
  using OnRequestCallback = std::function<void(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*)>;

  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata& /*methodMetadata*/,
      apache::thrift::protocol::PROTOCOL_TYPES protocolId,
      apache::thrift::Cpp2RequestContext* ctx,
      folly::EventBase* /*eb*/,
      apache::thrift::concurrency::ThreadManager* /*tm*/) override {
    requestCount_++;
    lastProtocolId_ = protocolId;
    if (ctx) {
      lastMethodName_ = ctx->getMethodName();
    }
    if (onRequestCallback_) {
      onRequestCallback_(
          std::move(req), std::move(serializedRequest), protocolId, ctx);
    }
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {}

  void setOnRequest(OnRequestCallback cb) {
    onRequestCallback_ = std::move(cb);
  }

  int requestCount() const { return requestCount_; }

  const std::string& lastMethodName() const { return lastMethodName_; }

  apache::thrift::protocol::PROTOCOL_TYPES lastProtocolId() const {
    return lastProtocolId_;
  }

 private:
  int requestCount_{0};
  std::string lastMethodName_;
  apache::thrift::protocol::PROTOCOL_TYPES lastProtocolId_{};
  OnRequestCallback onRequestCallback_;
};

/**
 * MockAsyncProcessorFactory - wraps a MockAsyncProcessor.
 */
class MockAsyncProcessorFactory : public apache::thrift::AsyncProcessorFactory {
 public:
  explicit MockAsyncProcessorFactory(
      std::shared_ptr<MockAsyncProcessor> processor)
      : processor_(std::move(processor)) {}

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    // Return a new wrapper that delegates to the shared processor
    return std::make_unique<DelegatingProcessor>(processor_);
  }

  std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
      override {
    return {};
  }

 private:
  // Thin wrapper that delegates to the shared MockAsyncProcessor
  class DelegatingProcessor : public apache::thrift::AsyncProcessor {
   public:
    explicit DelegatingProcessor(std::shared_ptr<MockAsyncProcessor> delegate)
        : delegate_(std::move(delegate)) {}

    void processSerializedCompressedRequestWithMetadata(
        apache::thrift::ResponseChannelRequest::UniquePtr req,
        apache::thrift::SerializedCompressedRequest&& serializedRequest,
        const MethodMetadata& methodMetadata,
        apache::thrift::protocol::PROTOCOL_TYPES protocolId,
        apache::thrift::Cpp2RequestContext* ctx,
        folly::EventBase* eb,
        apache::thrift::concurrency::ThreadManager* tm) override {
      delegate_->processSerializedCompressedRequestWithMetadata(
          std::move(req),
          std::move(serializedRequest),
          methodMetadata,
          protocolId,
          ctx,
          eb,
          tm);
    }

    void processInteraction(apache::thrift::ServerRequest&&) override {}

   private:
    std::shared_ptr<MockAsyncProcessor> delegate_;
  };

  std::shared_ptr<MockAsyncProcessor> processor_;
};

} // namespace

class ThriftServerChannelTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MockHandler::resetOrderCounter();
    evb_ = std::make_unique<folly::EventBase>();
    allocator_.reset();
    mockProcessor_ = std::make_shared<MockAsyncProcessor>();
    auto factory = std::make_shared<MockAsyncProcessorFactory>(mockProcessor_);
    channel_ = std::make_unique<ThriftServerChannel>(std::move(factory));
    mockTransport_.setOnWriteCallback(
        [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  }

  void TearDown() override {
    pipeline_.reset();
    channel_.reset();
    evb_.reset();
  }

  // Helper to create IOBuf with content via the allocator
  apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
      const void* data, size_t len) {
    auto buf = allocator_.allocate(len);
    std::memcpy(buf->writableData(), data, len);
    buf->append(len);
    return buf;
  }

  apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
      folly::StringPiece s) {
    return copyBuffer(s.data(), s.size());
  }

  PipelineImpl::Ptr buildPipelineWithHandler(
      std::function<Result(
          apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&)> writeHandler) {
    handler_ = std::make_unique<MockHandler>();
    handler_->setOnWrite(std::move(writeHandler));

    return PipelineBuilder<
               MockTransportHandler,
               ThriftServerChannel,
               TestAllocator>()
        .setEventBase(evb_.get())
        .setHead(&mockTransport_)
        .setTail(channel_.get())
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(test_handler_tag, std::move(handler_))
        .build();
  }

  // Helper to create a ThriftServerRequestMessage with metadata baked into the
  // frame
  apache::thrift::fast_thrift::thrift::ThriftServerRequestMessage
  createRequestMessage(
      uint32_t streamId,
      const std::string& methodName,
      const std::string& data = "",
      apache::thrift::RpcKind kind =
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE) {
    apache::thrift::RequestRpcMetadata metadata;
    metadata.name() = methodName;
    metadata.protocol() = apache::thrift::ProtocolId::COMPACT;
    metadata.kind() = kind;

    // Serialize metadata into the frame
    apache::thrift::BinaryProtocolWriter writer;
    folly::IOBufQueue metadataQueue(folly::IOBufQueue::cacheChainLength());
    writer.setOutput(&metadataQueue);
    metadata.write(&writer);
    auto serializedMetadata = metadataQueue.move();

    std::unique_ptr<folly::IOBuf> dataBuffer;
    if (!data.empty()) {
      dataBuffer = copyBuffer(data);
    }

    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
            .streamId = streamId,
            .follows = false,
        },
        std::move(serializedMetadata),
        std::move(dataBuffer));

    auto parsedFrame =
        apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

    return apache::thrift::fast_thrift::thrift::ThriftServerRequestMessage{
        .frame = std::move(parsedFrame), .streamId = streamId};
  }

  std::unique_ptr<folly::EventBase> evb_;
  MockTransportHandler mockTransport_;
  TestAllocator allocator_;
  std::shared_ptr<MockAsyncProcessor> mockProcessor_;
  std::unique_ptr<ThriftServerChannel> channel_;
  PipelineImpl::Ptr pipeline_;
  std::unique_ptr<MockHandler> handler_;
};

// =============================================================================
// onMessage - Request Dispatch
// =============================================================================

TEST_F(ThriftServerChannelTest, OnMessageDispatchesToProcessor) {
  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        // Send a simple reply
        req->sendReply(apache::thrift::ResponsePayload(this->copyBuffer("ok")));
      });

  pipeline_ = buildPipelineWithHandler(
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "testMethod", "request data");
  auto result = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(mockProcessor_->requestCount(), 1);
}

TEST_F(ThriftServerChannelTest, OnMessageExtractsMethodName) {
  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        req->sendReply(apache::thrift::ResponsePayload(this->copyBuffer("ok")));
      });

  pipeline_ = buildPipelineWithHandler(
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "myServiceMethod", "data");
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(mockProcessor_->lastMethodName(), "myServiceMethod");
}

TEST_F(ThriftServerChannelTest, OnMessageExtractsProtocol) {
  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        req->sendReply(apache::thrift::ResponsePayload(this->copyBuffer("ok")));
      });

  pipeline_ = buildPipelineWithHandler(
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "method", "data");
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(
      mockProcessor_->lastProtocolId(),
      apache::thrift::protocol::T_COMPACT_PROTOCOL);
}

// =============================================================================
// onMessage - Error Handling
// =============================================================================

// =============================================================================
// sendReply - Response Path
// =============================================================================

TEST_F(ThriftServerChannelTest, SendReplyPreservesResponsePayload) {
  std::string capturedPayload;

  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        req->sendReply(
            apache::thrift::ResponsePayload(this->copyBuffer("hello world")));
      });

  pipeline_ = buildPipelineWithHandler(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& response = msg.get<
            apache::thrift::fast_thrift::thrift::ThriftServerResponseMessage>();
        if (response.payload.data) {
          folly::io::Cursor cursor(response.payload.data.get());
          capturedPayload = cursor.readFixedString(
              response.payload.data->computeChainDataLength());
        }
        return Result::Success;
      });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "method", "data");
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(capturedPayload, "hello world");
}

TEST_F(ThriftServerChannelTest, SendReplySetsResponseMetadata) {
  apache::thrift::PayloadMetadata::Type capturedType{};

  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        req->sendReply(apache::thrift::ResponsePayload(this->copyBuffer("ok")));
      });

  pipeline_ = buildPipelineWithHandler(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& response = msg.get<
            apache::thrift::fast_thrift::thrift::ThriftServerResponseMessage>();
        if (response.payload.metadata) {
          auto meta = deserializeResponseMetadata(*response.payload.metadata);
          auto ref = meta.payloadMetadata();
          if (ref) {
            capturedType = ref->getType();
          }
        }
        return Result::Success;
      });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "method", "data");
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(
      capturedType, apache::thrift::PayloadMetadata::Type::responseMetadata);
}

// =============================================================================
// sendErrorWrapped - Error Response Path
// =============================================================================

TEST_F(ThriftServerChannelTest, SendErrorWrappedSetsExceptionMetadata) {
  apache::thrift::PayloadMetadata::Type capturedPayloadType{};
  std::string capturedWhat;

  mockProcessor_->setOnRequest(
      [](apache::thrift::ResponseChannelRequest::UniquePtr req,
         apache::thrift::SerializedCompressedRequest&&,
         apache::thrift::protocol::PROTOCOL_TYPES,
         apache::thrift::Cpp2RequestContext*) {
        req->sendErrorWrapped(
            folly::make_exception_wrapper<
                apache::thrift::TApplicationException>(
                apache::thrift::TApplicationException::INTERNAL_ERROR,
                "something broke"),
            "1");
      });

  pipeline_ = buildPipelineWithHandler(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& response = msg.get<
            apache::thrift::fast_thrift::thrift::ThriftServerResponseMessage>();
        if (response.payload.metadata) {
          auto meta = deserializeResponseMetadata(*response.payload.metadata);
          auto ref = meta.payloadMetadata();
          if (ref) {
            capturedPayloadType = ref->getType();
            if (capturedPayloadType ==
                apache::thrift::PayloadMetadata::Type::exceptionMetadata) {
              auto& exBase = ref->get_exceptionMetadata();
              if (exBase.what_utf8()) {
                capturedWhat = *exBase.what_utf8();
              }
              auto metaRef = exBase.metadata();
              EXPECT_TRUE(metaRef.has_value());
              if (metaRef) {
                EXPECT_EQ(
                    metaRef->getType(),
                    apache::thrift::PayloadExceptionMetadata::Type::
                        appUnknownException);
              }
            }
          }
        }
        return Result::Success;
      });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "method", "data");
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(
      capturedPayloadType,
      apache::thrift::PayloadMetadata::Type::exceptionMetadata);
  EXPECT_NE(capturedWhat.find("something broke"), std::string::npos);
}

// =============================================================================
// Oneway Detection
// =============================================================================

TEST_F(ThriftServerChannelTest, OnewayRequestDetected) {
  bool isOnewayCaptured = false;

  mockProcessor_->setOnRequest(
      [&](apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        isOnewayCaptured = req->isOneway();
      });

  pipeline_ = buildPipelineWithHandler(
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(
      1,
      "onewayMethod",
      "data",
      apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE);
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_TRUE(isOnewayCaptured);
}

TEST_F(ThriftServerChannelTest, RequestResponseIsNotOneway) {
  bool isOnewayCaptured = true;

  mockProcessor_->setOnRequest(
      [&](apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        isOnewayCaptured = req->isOneway();
        req->sendReply(apache::thrift::ResponsePayload(this->copyBuffer("ok")));
      });

  pipeline_ = buildPipelineWithHandler(
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(
      1,
      "method",
      "data",
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_FALSE(isOnewayCaptured);
}

// =============================================================================
// Multiple Requests
// =============================================================================

TEST_F(ThriftServerChannelTest, MultipleRequestsDispatchedCorrectly) {
  std::vector<uint32_t> capturedStreamIds;

  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        req->sendReply(apache::thrift::ResponsePayload(this->copyBuffer("ok")));
      });

  pipeline_ = buildPipelineWithHandler(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& response = msg.get<
            apache::thrift::fast_thrift::thrift::ThriftServerResponseMessage>();
        capturedStreamIds.push_back(response.streamId);
        return Result::Success;
      });
  channel_->setPipeline(std::move(pipeline_));

  auto req1 = createRequestMessage(1, "method1", "data1");
  auto req2 = createRequestMessage(3, "method2", "data2");
  auto req3 = createRequestMessage(5, "method3", "data3");

  std::ignore = channel_->onRead(erase_and_box(std::move(req1)));
  std::ignore = channel_->onRead(erase_and_box(std::move(req2)));
  std::ignore = channel_->onRead(erase_and_box(std::move(req3)));

  EXPECT_EQ(mockProcessor_->requestCount(), 3);
  ASSERT_EQ(capturedStreamIds.size(), 3u);
  EXPECT_EQ(capturedStreamIds[0], 1u);
  EXPECT_EQ(capturedStreamIds[1], 3u);
  EXPECT_EQ(capturedStreamIds[2], 5u);
}

// =============================================================================
// Double Send Prevention
// =============================================================================

TEST_F(ThriftServerChannelTest, DoubleSendReplyIsIgnored) {
  int responseCount = 0;

  mockProcessor_->setOnRequest(
      [this](
          apache::thrift::ResponseChannelRequest::UniquePtr req,
          apache::thrift::SerializedCompressedRequest&&,
          apache::thrift::protocol::PROTOCOL_TYPES,
          apache::thrift::Cpp2RequestContext*) {
        // First sendReply will succeed, second will be ignored (active_ =
        // false). The UniquePtr owns the request throughout.
        req->sendReply(
            apache::thrift::ResponsePayload(this->copyBuffer("first")));
        req->sendReply(
            apache::thrift::ResponsePayload(this->copyBuffer("second")));
      });

  pipeline_ = buildPipelineWithHandler(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&) {
        responseCount++;
        return Result::Success;
      });
  channel_->setPipeline(std::move(pipeline_));

  auto request = createRequestMessage(1, "method", "data");
  std::ignore = channel_->onRead(erase_and_box(std::move(request)));

  EXPECT_EQ(responseCount, 1);
}

} // namespace apache::thrift::fast_thrift::thrift
