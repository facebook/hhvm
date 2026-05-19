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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftRequestPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
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

inline const std::unique_ptr<folly::IOBuf>& payloadData(
    const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftInitialResponsePayload>()) {
    return msg.payload.get<ThriftInitialResponsePayload>().data;
  }
  return msg.payload.get<ThriftErrorPayload>().data;
}
inline uint32_t payloadErrorCode(const ThriftServerResponseMessage& msg) {
  if (msg.payload.is<ThriftErrorPayload>()) {
    return msg.payload.get<ThriftErrorPayload>().errorCode;
  }
  return 0;
}

apache::thrift::ResponseRpcError deserializeResponseRpcError(
    const folly::IOBuf& buf) {
  apache::thrift::ResponseRpcError error;
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(&buf);
  error.read(&reader);
  return error;
}

// Test child adapter: exposes addMethodHandler + tracks dispatch + records the
// std::string identifier set at registration so tests can prove which child
// served a request.
class TestChildAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<TestChildAdapter, Destructor>;

  explicit TestChildAdapter(std::string id) : id_(std::move(id)) {}

  void registerMethod(std::string_view name) {
    addMethodHandler(
        name,
        +[](ThriftServerAppAdapter* self,
            uint32_t streamId,
            std::unique_ptr<folly::IOBuf>,
            apache::thrift::ProtocolId protocol) noexcept -> Result {
          auto* t = static_cast<TestChildAdapter*>(self);
          t->dispatchedTo = t->id_;
          t->capturedStreamId = streamId;
          t->capturedProtocol = protocol;
          return Result::Success;
        });
  }

  std::string id_;
  std::string dispatchedTo;
  uint32_t capturedStreamId{0};
  apache::thrift::ProtocolId capturedProtocol{};
};

// Construct a request in the post-pipeline shape: metadata is already
// deserialized into the typed RequestRpcMetadata struct, data is its own
// IOBuf. Mirrors what the pipeline's transport adapter hands the composite.
ThriftServerRequestMessage makeRequestMessage(
    uint32_t streamId,
    const std::string& methodName,
    apache::thrift::ProtocolId protocol = apache::thrift::ProtocolId::BINARY,
    apache::thrift::RpcKind kind =
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name() = methodName;
  metadata->kind() = kind;
  metadata->protocol() = protocol;

  ThriftServerRequestMessage msg;
  msg.payload = ThriftServerInboundPayloadVariant{ThriftRequestResponsePayload{
      .data = folly::IOBuf::copyBuffer("payload"),
      .metadata = std::move(metadata)}};
  msg.streamId = streamId;
  return msg;
}

} // namespace

class ThriftServerCompositeAppAdapterTest : public ::testing::Test {
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

    ~BuiltPipeline() {
      if (transportHandler) {
        transportHandler->close(folly::exception_wrapper{});
        transportHandler->resetPipeline();
      }
    }
  };

  // Build a pipeline templated on ThriftServerCompositeAppAdapter so that the
  // composite's shadowing onRead is the one resolved at the tail.
  BuiltPipeline buildPipeline(
      ThriftServerCompositeAppAdapter* composite,
      std::function<Result(
          apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&)> writeHandler = nullptr) {
    auto transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            createMockSocket());

    auto handlerPtr = std::make_unique<MockHandler>();
    if (writeHandler) {
      handlerPtr->setOnWrite(std::move(writeHandler));
    }

    auto pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            ThriftServerCompositeAppAdapter,
            TestAllocator>()
            .setEventBase(evb_)
            .setHead(transportHandler.get())
            .setTail(composite)
            .setAllocator(&allocator_)
            .addNextDuplex<MockHandler>(test_handler_tag, std::move(handlerPtr))
            .build();

    composite->setPipeline(pipeline.get());
    // Move composite (and forwarded children) into Open so onRead is
    // accepted. Real server flow does this via thriftPipeline->activate().
    composite->onPipelineActive();

    return {std::move(transportHandler), std::move(pipeline)};
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  TestAllocator allocator_;
};

// =============================================================================
// Routing Tests
// =============================================================================

TEST_F(ThriftServerCompositeAppAdapterTest, RoutesByMethodNameToOwningChild) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  TestChildAdapter::Ptr userChild{userRaw};
  TestChildAdapter::Ptr monitoringChild{monitoringRaw};

  userChild->registerMethod("userMethod");
  monitoringChild->registerMethod("getStatus");

  ThriftServerAppAdapter::Ptr user{userChild.release()};
  ThriftServerAppAdapter::Ptr monitoring{monitoringChild.release()};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  auto built = buildPipeline(composite.get());

  // userMethod → user
  auto userMsg = makeRequestMessage(1, "userMethod");
  EXPECT_EQ(
      composite->onRead(erase_and_box(std::move(userMsg))), Result::Success);
  EXPECT_EQ(userRaw->dispatchedTo, "user");
  EXPECT_EQ(userRaw->capturedStreamId, 1u);
  EXPECT_EQ(monitoringRaw->dispatchedTo, "")
      << "monitoring child must not be invoked for a user-only method";

  // getStatus → monitoring
  auto monMsg = makeRequestMessage(3, "getStatus");
  EXPECT_EQ(
      composite->onRead(erase_and_box(std::move(monMsg))), Result::Success);
  EXPECT_EQ(monitoringRaw->dispatchedTo, "monitoring");
  EXPECT_EQ(monitoringRaw->capturedStreamId, 3u);
}

TEST_F(ThriftServerCompositeAppAdapterTest, UserWinsOnMethodNameConflict) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  TestChildAdapter::Ptr userChild{userRaw};
  TestChildAdapter::Ptr monitoringChild{monitoringRaw};

  // Both register the same method name; user is passed first to the
  // composite, so user must win.
  userChild->registerMethod("ping");
  monitoringChild->registerMethod("ping");

  ThriftServerAppAdapter::Ptr user{userChild.release()};
  ThriftServerAppAdapter::Ptr monitoring{monitoringChild.release()};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  auto built = buildPipeline(composite.get());

  auto msg = makeRequestMessage(7, "ping");
  EXPECT_EQ(composite->onRead(erase_and_box(std::move(msg))), Result::Success);

  EXPECT_EQ(userRaw->dispatchedTo, "user");
  EXPECT_EQ(monitoringRaw->dispatchedTo, "");
}

TEST_F(ThriftServerCompositeAppAdapterTest, UnknownMethodEmitsFrameworkError) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  TestChildAdapter::Ptr userChild{userRaw};
  TestChildAdapter::Ptr monitoringChild{monitoringRaw};

  userChild->registerMethod("knownUserMethod");
  monitoringChild->registerMethod("knownMonitoringMethod");

  ThriftServerAppAdapter::Ptr user{userChild.release()};
  ThriftServerAppAdapter::Ptr monitoring{monitoringChild.release()};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  apache::thrift::ResponseRpcErrorCode capturedRpcErrorCode{};
  bool writeCalled = false;
  auto built = buildPipeline(
      composite.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        if (auto& d = payloadData(resp); d) {
          auto rpcError = deserializeResponseRpcError(*d);
          capturedRpcErrorCode = *rpcError.code();
        }
        EXPECT_NE(payloadErrorCode(resp), 0u) << "must be ERROR frame";
        return Result::Success;
      });

  auto msg = makeRequestMessage(1, "neitherChildHasThis");
  EXPECT_EQ(composite->onRead(erase_and_box(std::move(msg))), Result::Success);

  EXPECT_TRUE(writeCalled);
  EXPECT_EQ(
      capturedRpcErrorCode,
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD);
  EXPECT_EQ(userRaw->dispatchedTo, "");
  EXPECT_EQ(monitoringRaw->dispatchedTo, "");
}

// =============================================================================
// Per-connection state forwarding
// =============================================================================

TEST_F(ThriftServerCompositeAppAdapterTest, SetPipelineForwardsToBothChildren) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  ThriftServerAppAdapter::Ptr user{userRaw};
  ThriftServerAppAdapter::Ptr monitoring{monitoringRaw};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  // Sanity: children start with no pipeline.
  EXPECT_EQ(userRaw->pipeline(), nullptr);
  EXPECT_EQ(monitoringRaw->pipeline(), nullptr);

  // buildPipeline calls composite->setPipeline; the shadow should propagate.
  auto built = buildPipeline(composite.get());

  EXPECT_NE(composite->pipeline(), nullptr);
  EXPECT_EQ(userRaw->pipeline(), composite->pipeline())
      << "setPipeline must propagate to user child so it can write responses";
  EXPECT_EQ(monitoringRaw->pipeline(), composite->pipeline())
      << "setPipeline must propagate to monitoring child";
}

TEST_F(
    ThriftServerCompositeAppAdapterTest,
    OnPipelineActiveForwardsToBothChildren) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  ThriftServerAppAdapter::Ptr user{userRaw};
  ThriftServerAppAdapter::Ptr monitoring{monitoringRaw};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  // buildPipeline calls setPipeline (Created -> Ready) then onPipelineActive
  // (Ready -> Open). After it returns every adapter should be Open.
  auto built = buildPipeline(composite.get());

  EXPECT_EQ(composite->state(), ThriftServerAppAdapter::State::Open);
  EXPECT_EQ(userRaw->state(), ThriftServerAppAdapter::State::Open)
      << "onPipelineActive must fan out to user child";
  EXPECT_EQ(monitoringRaw->state(), ThriftServerAppAdapter::State::Open)
      << "onPipelineActive must fan out to monitoring child";
}

TEST_F(
    ThriftServerCompositeAppAdapterTest,
    OnPipelineInactiveForwardsToBothChildren) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  ThriftServerAppAdapter::Ptr user{userRaw};
  ThriftServerAppAdapter::Ptr monitoring{monitoringRaw};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  auto built = buildPipeline(composite.get());

  evb_->runInEventBaseThreadAndWait([&] { composite->onPipelineInactive(); });

  EXPECT_EQ(composite->state(), ThriftServerAppAdapter::State::Closed);
  EXPECT_EQ(userRaw->state(), ThriftServerAppAdapter::State::Closed)
      << "onPipelineInactive must fan out to user child";
  EXPECT_EQ(monitoringRaw->state(), ThriftServerAppAdapter::State::Closed)
      << "onPipelineInactive must fan out to monitoring child";
}

TEST_F(ThriftServerCompositeAppAdapterTest, OnExceptionForwardsToBothChildren) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  ThriftServerAppAdapter::Ptr user{userRaw};
  ThriftServerAppAdapter::Ptr monitoring{monitoringRaw};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  auto built = buildPipeline(composite.get());

  evb_->runInEventBaseThreadAndWait([&] {
    composite->onException(
        folly::make_exception_wrapper<std::runtime_error>("boom"));
  });

  EXPECT_EQ(composite->state(), ThriftServerAppAdapter::State::Closed);
  EXPECT_EQ(userRaw->state(), ThriftServerAppAdapter::State::Closed)
      << "onException must fan out to user child";
  EXPECT_EQ(monitoringRaw->state(), ThriftServerAppAdapter::State::Closed)
      << "onException must fan out to monitoring child";
}

// The composite forwards the inbound box to the chosen child's onRead, so
// the RPC-kind reject lives in the child's handleRequestResponse. Verify
// it's still enforced end-to-end through the composite.
TEST_F(ThriftServerCompositeAppAdapterTest, RejectsUnsupportedRpcKind) {
  auto* userRaw = new TestChildAdapter("user");
  auto* monitoringRaw = new TestChildAdapter("monitoring");
  TestChildAdapter::Ptr userChild{userRaw};
  TestChildAdapter::Ptr monitoringChild{monitoringRaw};

  userChild->registerMethod("streamingMethod");

  ThriftServerAppAdapter::Ptr user{userChild.release()};
  ThriftServerAppAdapter::Ptr monitoring{monitoringChild.release()};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(std::move(user));
  composite->addChild(std::move(monitoring));

  apache::thrift::ResponseRpcErrorCode capturedRpcErrorCode{};
  bool writeCalled = false;
  auto built = buildPipeline(
      composite.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& box) {
        writeCalled = true;
        auto& resp = box.get<ThriftServerResponseMessage>();
        if (auto& d = payloadData(resp); d) {
          auto rpcError = deserializeResponseRpcError(*d);
          capturedRpcErrorCode = *rpcError.code();
        }
        EXPECT_NE(payloadErrorCode(resp), 0u) << "must be ERROR frame";
        return Result::Success;
      });

  // Method exists in user, but RPC kind is streaming — child must reject
  // before invoking the user thunk.
  auto msg = makeRequestMessage(
      1,
      "streamingMethod",
      apache::thrift::ProtocolId::BINARY,
      apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE);
  EXPECT_EQ(composite->onRead(erase_and_box(std::move(msg))), Result::Success);

  EXPECT_TRUE(writeCalled);
  EXPECT_EQ(
      capturedRpcErrorCode,
      apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND);
  EXPECT_EQ(userRaw->dispatchedTo, "")
      << "user thunk must NOT be invoked when RPC kind is wrong";
}

} // namespace apache::thrift::fast_thrift::thrift
