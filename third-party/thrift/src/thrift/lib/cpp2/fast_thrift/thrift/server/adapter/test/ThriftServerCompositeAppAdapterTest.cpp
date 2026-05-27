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
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ThriftServerConnection.h>
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
// served a request. Also counts lifecycle hooks via shadowing — the
// composite's vtable templates on T so `static_cast<TestChildAdapter*>(p)`
// resolves to these shadows. State-bearing base methods (onPipelineActive,
// onPipelineInactive, onException) must be chained from the shadows;
// otherwise the child never reaches Open and onRead rejects with Error.
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
            apache::thrift::ProtocolId protocol,
            std::unique_ptr<ThriftRequestContext>) noexcept -> Result {
          auto* t = static_cast<TestChildAdapter*>(self);
          t->dispatchedTo = t->id_;
          t->capturedStreamId = streamId;
          t->capturedProtocol = protocol;
          return Result::Success;
        });
  }

  // Lifecycle shadows — bump counters so tests can verify fan-out, then
  // chain to base for state-bearing methods. The composite's vtable resolves
  // on T = TestChildAdapter, so without chaining the base's state machine
  // (Created→Ready→Open and onException close) never runs and downstream
  // onRead rejects with Result::Error.
  //
  // onPipelineActive's base DCHECKs state==Ready, so guard the chain — some
  // tests exercise fan-out without a real pipeline setup and never reach
  // Ready. onPipelineInactive / onException have no such precondition.
  void handlerAdded() noexcept { ++handlerAddedCount; }
  void handlerRemoved() noexcept { ++handlerRemovedCount; }
  void onPipelineActive() noexcept {
    ++onPipelineActiveCount;
    if (state() == State::Ready) {
      ThriftServerAppAdapter::onPipelineActive();
    }
  }
  void onPipelineInactive() noexcept {
    ++onPipelineInactiveCount;
    ThriftServerAppAdapter::onPipelineInactive();
  }
  void onWriteReady() noexcept { ++onWriteReadyCount; }
  void onException(folly::exception_wrapper&& e) noexcept {
    ++onExceptionCount;
    lastException = e;
    ThriftServerAppAdapter::onException(folly::exception_wrapper{e});
  }
  // No chain — base startDrain would attempt to write CONNECTION_CLOSE
  // through the pipeline. Fan-out tests don't always wire one up.
  void startDrain() noexcept { ++startDrainCount; }

  std::string id_;
  std::string dispatchedTo;
  uint32_t capturedStreamId{0};
  apache::thrift::ProtocolId capturedProtocol{};

  int handlerAddedCount{0};
  int handlerRemovedCount{0};
  int onPipelineActiveCount{0};
  int onPipelineInactiveCount{0};
  int onWriteReadyCount{0};
  int onExceptionCount{0};
  int startDrainCount{0};
  folly::exception_wrapper lastException;
};

// Second concrete child type — proves heterogeneous children that satisfy
// the concepts can coexist in the same composite without sharing a
// derived type. Identical surface to TestChildAdapter; the distinct C++
// type is the point.
class OtherChildAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<OtherChildAdapter, Destructor>;

  explicit OtherChildAdapter(std::string id) : id_(std::move(id)) {}

  void registerMethod(std::string_view name) {
    addMethodHandler(
        name,
        +[](ThriftServerAppAdapter* self,
            uint32_t streamId,
            std::unique_ptr<folly::IOBuf>,
            apache::thrift::ProtocolId,
            std::unique_ptr<ThriftRequestContext>) noexcept -> Result {
          auto* t = static_cast<OtherChildAdapter*>(self);
          t->dispatchedTo = t->id_;
          t->capturedStreamId = streamId;
          return Result::Success;
        });
  }

  std::string id_;
  std::string dispatchedTo;
  uint32_t capturedStreamId{0};
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
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  userChild->registerMethod("userMethod");
  monitoringChild->registerMethod("getStatus");

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

  auto built = buildPipeline(composite.get());

  // userMethod → user
  auto userMsg = makeRequestMessage(1, "userMethod");
  EXPECT_EQ(
      composite->onRead(erase_and_box(std::move(userMsg))), Result::Success);
  EXPECT_EQ(userChild->dispatchedTo, "user");
  EXPECT_EQ(userChild->capturedStreamId, 1u);
  EXPECT_EQ(monitoringChild->dispatchedTo, "")
      << "monitoring child must not be invoked for a user-only method";

  // getStatus → monitoring
  auto monMsg = makeRequestMessage(3, "getStatus");
  EXPECT_EQ(
      composite->onRead(erase_and_box(std::move(monMsg))), Result::Success);
  EXPECT_EQ(monitoringChild->dispatchedTo, "monitoring");
  EXPECT_EQ(monitoringChild->capturedStreamId, 3u);
  // Symmetric cross-pollination check: monitoring's request must not have
  // also been delivered to user. (TestChildAdapter records the last
  // dispatch, so a leak would surface as user's captured streamId moving
  // to 3 or dispatchedTo flipping.)
  EXPECT_EQ(userChild->capturedStreamId, 1u)
      << "user child must not see monitoring's request";
  EXPECT_EQ(userChild->dispatchedTo, "user");
}

TEST_F(ThriftServerCompositeAppAdapterTest, UserWinsOnMethodNameConflict) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  // Both register the same method name; user is added first to the
  // composite, so user must win.
  userChild->registerMethod("ping");
  monitoringChild->registerMethod("ping");

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

  auto built = buildPipeline(composite.get());

  auto msg = makeRequestMessage(7, "ping");
  EXPECT_EQ(composite->onRead(erase_and_box(std::move(msg))), Result::Success);

  EXPECT_EQ(userChild->dispatchedTo, "user");
  EXPECT_EQ(monitoringChild->dispatchedTo, "");
}

TEST_F(ThriftServerCompositeAppAdapterTest, UnknownMethodEmitsFrameworkError) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  userChild->registerMethod("knownUserMethod");
  monitoringChild->registerMethod("knownMonitoringMethod");

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

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
  EXPECT_EQ(userChild->dispatchedTo, "");
  EXPECT_EQ(monitoringChild->dispatchedTo, "");
}

// =============================================================================
// Per-connection state forwarding
// =============================================================================

TEST_F(ThriftServerCompositeAppAdapterTest, SetPipelineForwardsToBothChildren) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

  // Sanity: children start with no pipeline.
  EXPECT_EQ(userChild->pipeline(), nullptr);
  EXPECT_EQ(monitoringChild->pipeline(), nullptr);

  // buildPipeline calls composite->setPipeline; should fan out to children.
  auto built = buildPipeline(composite.get());

  EXPECT_NE(composite->pipeline(), nullptr);
  EXPECT_EQ(userChild->pipeline(), composite->pipeline())
      << "setPipeline must propagate to user child so it can write responses";
  EXPECT_EQ(monitoringChild->pipeline(), composite->pipeline())
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
  composite->addChild(userRaw);
  composite->addChild(monitoringRaw);

  // buildPipeline calls setPipeline (Created -> Ready) then onPipelineActive
  // (Ready -> Open). After it returns every adapter should be Open.
  auto built = buildPipeline(composite.get());

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
  composite->addChild(userRaw);
  composite->addChild(monitoringRaw);

  auto built = buildPipeline(composite.get());

  evb_->runInEventBaseThreadAndWait([&] { composite->onPipelineInactive(); });

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
  composite->addChild(userRaw);
  composite->addChild(monitoringRaw);

  auto built = buildPipeline(composite.get());

  evb_->runInEventBaseThreadAndWait([&] {
    composite->onException(
        folly::make_exception_wrapper<std::runtime_error>("boom"));
  });

  EXPECT_EQ(userRaw->state(), ThriftServerAppAdapter::State::Closed)
      << "onException must fan out to user child";
  EXPECT_EQ(monitoringRaw->state(), ThriftServerAppAdapter::State::Closed)
      << "onException must fan out to monitoring child";
}

// The composite forwards the inbound box to the chosen child's onRead, so
// the RPC-kind reject lives in the child's handleRequestResponse. Verify
// it's still enforced end-to-end through the composite.
TEST_F(ThriftServerCompositeAppAdapterTest, RejectsUnsupportedRpcKind) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  userChild->registerMethod("streamingMethod");

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

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
  EXPECT_EQ(userChild->dispatchedTo, "")
      << "user thunk must NOT be invoked when RPC kind is wrong";
}

// =============================================================================
// Lifecycle fan-out
// =============================================================================

TEST_F(ThriftServerCompositeAppAdapterTest, OnExceptionFansOutToBothChildren) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

  auto built = buildPipeline(composite.get());

  composite->onException(
      folly::make_exception_wrapper<std::runtime_error>("boom"));

  EXPECT_EQ(userChild->onExceptionCount, 1)
      << "onException must fan out to user child";
  EXPECT_EQ(monitoringChild->onExceptionCount, 1)
      << "onException must fan out to monitoring child";
  EXPECT_TRUE(userChild->lastException);
  EXPECT_TRUE(monitoringChild->lastException);
}

TEST_F(
    ThriftServerCompositeAppAdapterTest, LifecycleHooksFanOutToBothChildren) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

  composite->handlerAdded();
  composite->onPipelineActive();
  composite->onWriteReady();
  composite->onPipelineInactive();
  composite->handlerRemoved();

  for (auto* c : {userChild.get(), monitoringChild.get()}) {
    EXPECT_EQ(c->handlerAddedCount, 1) << c->id_ << " handlerAdded";
    EXPECT_EQ(c->onPipelineActiveCount, 1) << c->id_ << " onPipelineActive";
    EXPECT_EQ(c->onWriteReadyCount, 1) << c->id_ << " onWriteReady";
    EXPECT_EQ(c->onPipelineInactiveCount, 1) << c->id_ << " onPipelineInactive";
    EXPECT_EQ(c->handlerRemovedCount, 1) << c->id_ << " handlerRemoved";
  }
}

// Composite's startDrain must reach every child via vtable — children are
// stored as concept-erased entries, so the dispatch goes through the
// per-T trampoline (`static_cast<T*>(p)->startDrain()`). If the vtable
// slot were ever misrouted, graceful shutdown silently no-ops.
TEST_F(ThriftServerCompositeAppAdapterTest, StartDrainFansOutToBothChildren) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  TestChildAdapter::Ptr monitoringChild{new TestChildAdapter("monitoring")};

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(monitoringChild.get());

  composite->startDrain();

  EXPECT_EQ(userChild->startDrainCount, 1);
  EXPECT_EQ(monitoringChild->startDrainCount, 1);
}

// =============================================================================
// Heterogeneous children — the concept-based design's headline capability
// =============================================================================

TEST_F(
    ThriftServerCompositeAppAdapterTest, RoutesAcrossHeterogeneousChildTypes) {
  TestChildAdapter::Ptr userChild{new TestChildAdapter("user")};
  OtherChildAdapter::Ptr otherChild{new OtherChildAdapter("other")};

  userChild->registerMethod("userMethod");
  otherChild->registerMethod("otherMethod");

  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  composite->addChild(userChild.get());
  composite->addChild(otherChild.get());

  auto built = buildPipeline(composite.get());

  auto userMsg = makeRequestMessage(1, "userMethod");
  EXPECT_EQ(
      composite->onRead(erase_and_box(std::move(userMsg))), Result::Success);
  EXPECT_EQ(userChild->dispatchedTo, "user");
  EXPECT_EQ(otherChild->dispatchedTo, "");

  auto otherMsg = makeRequestMessage(3, "otherMethod");
  EXPECT_EQ(
      composite->onRead(erase_and_box(std::move(otherMsg))), Result::Success);
  EXPECT_EQ(otherChild->dispatchedTo, "other");
  EXPECT_EQ(otherChild->capturedStreamId, 3u);
}

// =============================================================================
// Edge cases
// =============================================================================

TEST_F(
    ThriftServerCompositeAppAdapterTest, EmptyCompositeLifecycleHooksAreSafe) {
  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  // No children. None of these should crash.
  composite->setPipeline(nullptr);
  composite->handlerAdded();
  composite->onPipelineActive();
  composite->onWriteReady();
  composite->onException(
      folly::make_exception_wrapper<std::runtime_error>("x"));
  composite->onPipelineInactive();
  composite->handlerRemoved();
}

// Regression guard for the ThriftServerConnection::setCloseCallback wiring on
// the CompositeTail branch: the cb must land on composite.adapter (whose
// dtor fallback fires it), not on composite.children.front() — the latter
// has no dtor fallback, so a misrouted cb would be silently dropped on
// teardown.
TEST_F(
    ThriftServerCompositeAppAdapterTest,
    ThriftServerConnectionRoutesCloseCallbackToComposite) {
  server::ThriftServerConnection::CompositeTail tail;
  tail.children.push_back(
      ThriftServerAppAdapter::Ptr{new TestChildAdapter("user")});
  tail.adapter = ThriftServerCompositeAppAdapter::Ptr{
      new ThriftServerCompositeAppAdapter()};
  tail.adapter->addChild(tail.children.front().get());

  server::ThriftServerConnection conn;
  conn.tail = std::move(tail);

  bool cbFired = false;
  conn.setCloseCallback([&] { cbFired = true; });

  conn = {};

  EXPECT_TRUE(cbFired);
}

TEST_F(
    ThriftServerCompositeAppAdapterTest,
    UnknownMethodReturnsErrorWhenPipelineUnset) {
  ThriftServerCompositeAppAdapter::Ptr composite{
      new ThriftServerCompositeAppAdapter()};
  // Intentionally do NOT call setPipeline. The unknown-method path tries to
  // fire a framework error through the pipeline; with no pipeline it must
  // surface Result::Error instead of crashing.
  auto msg = makeRequestMessage(1, "nobodyOwnsThis");
  EXPECT_EQ(composite->onRead(erase_and_box(std::move(msg))), Result::Error);
}

} // namespace apache::thrift::fast_thrift::thrift
