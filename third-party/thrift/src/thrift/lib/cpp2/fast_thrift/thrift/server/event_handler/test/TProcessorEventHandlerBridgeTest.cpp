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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/event_handler/TProcessorEventHandlerBridge.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::thrift::server {

namespace {

using channel_pipeline::erase_and_box;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;

class FakeContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    read.push_back(std::move(msg));
    return Result::Success;
  }
  Result fireWrite(TypeErasedBox&& msg) noexcept {
    written.push_back(std::move(msg));
    return Result::Success;
  }

  std::vector<TypeErasedBox> read;
  std::vector<TypeErasedBox> written;
};

using Bridge = TProcessorEventHandlerBridge<FakeContext>;

// Records the callback sequence across every request on the connection, and
// what each callback was told, so ordering and naming can be asserted
// together.
struct CallLog {
  std::vector<std::string> calls;
  std::string serviceName;
  std::string methodName;
  // Only valid until the request's response is written — the bridge destroys
  // the per-request contexts there.
  const apache::thrift::Cpp2RequestContext* readContext{nullptr};
  const apache::thrift::Cpp2ConnContext* connFromNewConnection{nullptr};
  const apache::thrift::Cpp2ConnContext* connFromRequest{nullptr};
  std::string ambientMarker;
  uint32_t postReadBytes{0};
  uint32_t postWriteBytes{0};
  bool refusePreRead{false};
  bool refuseGetServiceContext{false};
};

// Stamped onto the ambient context by preRead, the way an event handler
// publishes per-request state for whatever runs downstream of it.
struct AmbientMarker : folly::RequestData {
  explicit AmbientMarker(std::string v) : value(std::move(v)) {}
  bool hasCallback() override { return false; }
  std::string value;
};

const folly::RequestToken& markerToken() {
  static const folly::RequestToken token{
      "processor_event_handler_bridge_test_marker"};
  return token;
}

class RecordingEventHandler : public apache::thrift::TProcessorEventHandler {
 public:
  explicit RecordingEventHandler(CallLog* log) : log_(log) {}

  void* getServiceContext(
      std::string_view serviceName,
      std::string_view fnName,
      apache::thrift::TConnectionContext* connectionContext) override {
    log_->calls.emplace_back("getServiceContext");
    log_->serviceName = std::string(serviceName);
    log_->methodName = std::string(fnName);
    if (log_->refuseGetServiceContext) {
      throw apache::thrift::TApplicationException("no context for you");
    }
    return connectionContext;
  }

  void preRead(void* ctx, std::string_view /*fnName*/) override {
    log_->calls.emplace_back("preRead");
    auto* reqCtx = static_cast<apache::thrift::Cpp2RequestContext*>(ctx);
    log_->readContext = reqCtx;
    log_->connFromRequest = reqCtx->getConnectionContext();
    folly::RequestContext::get()->setContextData(
        markerToken(), std::make_unique<AmbientMarker>("stamped"));
    if (log_->refusePreRead) {
      reqCtx->getHeader()->setHeader("denied-by", "recording-handler");
      throw apache::thrift::TApplicationException("refused");
    }
  }

  void postRead(
      void* /*ctx*/,
      std::string_view /*fnName*/,
      apache::thrift::transport::THeader* header,
      uint32_t bytes) override {
    log_->calls.emplace_back("postRead");
    log_->postReadBytes = bytes;
    // The read headers the request arrived with are visible here.
    if (header != nullptr && header->getHeaders().contains("cat")) {
      log_->calls.emplace_back("sawCatHeader");
    }
  }

  void preWrite(void* /*ctx*/, std::string_view /*fnName*/) override {
    log_->calls.emplace_back("preWrite");
  }

  void postWrite(
      void* ctx, std::string_view /*fnName*/, uint32_t bytes) override {
    log_->calls.emplace_back("postWrite");
    log_->postWriteBytes = bytes;
    if (auto* data =
            folly::RequestContext::get()->getContextData(markerToken())) {
      log_->ambientMarker = static_cast<AmbientMarker*>(data)->value;
    }
    static_cast<apache::thrift::Cpp2RequestContext*>(ctx)
        ->getHeader()
        ->setHeader("minted", "token");
  }

  void freeContext(void* /*ctx*/, std::string_view /*fnName*/) override {
    log_->calls.emplace_back("freeContext");
  }

 private:
  CallLog* log_;
};

class RecordingServerEventHandler
    : public apache::thrift::server::TServerEventHandler {
 public:
  explicit RecordingServerEventHandler(CallLog* log) : log_(log) {}

  void newConnection(apache::thrift::server::TConnectionContext* ctx) override {
    log_->calls.emplace_back("newConnection");
    log_->connFromNewConnection =
        static_cast<apache::thrift::Cpp2ConnContext*>(ctx);
  }
  void connectionDestroyed(
      apache::thrift::server::TConnectionContext* /*ctx*/) override {
    log_->calls.emplace_back("connectionDestroyed");
  }

 private:
  CallLog* log_;
};

TProcessorEventHandlerBridgeConfig makeConfig(CallLog* log) {
  auto handlers = std::make_shared<TProcessorEventHandlers>();
  handlers->serviceName = "TestService";
  handlers->processor.push_back(std::make_shared<RecordingEventHandler>(log));
  handlers->server.push_back(
      std::make_shared<RecordingServerEventHandler>(log));
  return TProcessorEventHandlerBridgeConfig{
      .handlers = std::move(handlers), .identityResolver = nullptr};
}

// The slot plan a server that registered the bridge's extension builds at
// start(). Static so it outlives every request context below.
const ExtensionLayout& bridgeLayout() {
  static const ExtensionLayout layout = [] {
    ExtensionLayoutBuilder builder;
    builder.add(Cpp2BridgeExtension::kId);
    return std::move(builder).build();
  }();
  return layout;
}

ThriftServerRequestMessage makeRequest(
    const boost::intrusive_ptr<ThriftConnContext>& conn,
    uint32_t streamId,
    std::string_view method,
    ThriftRequestContext::HeaderMap headers = {}) {
  ThriftServerRequestMessage req;
  req.streamId = streamId;
  req.requestContext = std::make_unique<ThriftRequestContext>();
  req.requestContext->installExtensions(bridgeLayout());
  req.requestContext->setConnectionContext(conn);
  req.requestContext->setHeaders(std::move(headers));

  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name() = std::string(method);
  req.payload = ThriftRequestResponsePayload{
      .data = folly::IOBuf::copyBuffer("0123456789"),
      .metadata = std::move(metadata)};
  return req;
}

ThriftServerResponseMessage makeResponse(uint32_t streamId) {
  auto metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  return ThriftServerResponseMessage{
      .payload = ThriftInitialResponsePayload{
          .data = folly::IOBuf::copyBuffer("hello"),
          .metadata = std::move(metadata),
          .streamId = streamId}};
}

// A framework error frame: no reply body, so nothing for the write-side
// callbacks to bracket.
ThriftServerResponseMessage makeErrorResponse(uint32_t streamId) {
  return ThriftServerResponseMessage{
      .payload = ThriftErrorPayload{
          .data = folly::IOBuf::copyBuffer("boom"),
          .metadata = nullptr,
          .streamId = streamId,
          .errorCode = 0}};
}

// The answer to a request that went downstream, carrying that request's
// context back the way the handler callback hands it over.
ThriftServerResponseMessage makeResponseFor(
    FakeContext& ctx, uint32_t streamId) {
  auto response = makeResponse(streamId);
  for (auto& box : ctx.read) {
    auto& request = box.get<ThriftServerRequestMessage>();
    if (request.streamId == streamId && request.requestContext != nullptr) {
      response.requestContext = std::move(request.requestContext);
      break;
    }
  }
  return response;
}

// Drives the connection to the point where it can carry requests: the bridge
// latches the context off the setup message, then SetupComplete builds the
// Cpp2 context and announces the connection.
void establish(
    Bridge& bridge,
    FakeContext& ctx,
    const boost::intrusive_ptr<ThriftConnContext>& conn) {
  auto setup = std::make_unique<ConnectionSetupData>();
  setup->connContext = conn.get();
  ThriftServerRequestMessage setupMsg;
  setupMsg.payload = ThriftConnectionSetupPayload{.setup = std::move(setup)};
  (void)bridge.onRead(ctx, erase_and_box(std::move(setupMsg)));

  ThriftServerSetupCompleteEvent event{};
  bridge.onEvent(
      ctx, ThriftServerEventType::SetupComplete, erase_and_box(&event));
}

boost::intrusive_ptr<ThriftConnContext> makeConn() {
  boost::intrusive_ptr<ThriftConnContext> conn{new ThriftConnContext()};
  conn->setPeerAddress(folly::SocketAddress("127.0.0.1", 4321));
  return conn;
}

} // namespace

// The classic callback order around one request, and the names the handler is
// given: the service, and the method qualified with it.
TEST(TProcessorEventHandlerBridgeTest, DrivesTheClassicCallbackOrder) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  EXPECT_EQ(
      bridge.onRead(ctx, erase_and_box(makeRequest(conn, 7, "ping"))),
      Result::Success);
  EXPECT_EQ(ctx.read.size(), 2); // setup + request
  EXPECT_EQ(
      bridge.onWrite(ctx, erase_and_box(makeResponse(7))), Result::Success);

  EXPECT_EQ(
      log.calls,
      (std::vector<std::string>{
          "newConnection",
          "getServiceContext",
          "preRead",
          "postRead",
          "preWrite",
          "postWrite",
          "freeContext"}));
  EXPECT_EQ(log.serviceName, "TestService");
  EXPECT_EQ(log.methodName, "TestService.ping");
  // The connection the server event handler was told about is the one every
  // request on it reports.
  ASSERT_NE(log.connFromNewConnection, nullptr);
  EXPECT_EQ(log.connFromNewConnection, log.connFromRequest);
  EXPECT_EQ(log.postReadBytes, 10);
  EXPECT_EQ(log.postWriteBytes, 5);
}

// Request headers reach the THeader handlers read, and response headers they
// write on the way out reach the response.
TEST(TProcessorEventHandlerBridgeTest, HeadersCrossInBothDirections) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  (void)bridge.onRead(
      ctx, erase_and_box(makeRequest(conn, 1, "ping", {{"cat", "token"}})));
  (void)bridge.onWrite(ctx, erase_and_box(makeResponse(1)));

  EXPECT_NE(
      std::find(log.calls.begin(), log.calls.end(), "sawCatHeader"),
      log.calls.end());

  ASSERT_EQ(ctx.written.size(), 1);
  auto& response = ctx.written.front().get<ThriftServerResponseMessage>();
  const auto& other = *response.payload.get<ThriftInitialResponsePayload>()
                           .metadata->otherMetadata();
  ASSERT_TRUE(other.contains("minted"));
  EXPECT_EQ(other.at("minted"), "token");
}

// A request served by the state a previous one returned sees none of that
// request's headers: the state's header is rebound, not added to.
TEST(TProcessorEventHandlerBridgeTest, ReusedStateDropsThePriorRequestHeaders) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  (void)bridge.onRead(
      ctx, erase_and_box(makeRequest(conn, 1, "ping", {{"cat", "token"}})));
  (void)bridge.onWrite(ctx, erase_and_box(makeResponseFor(ctx, 1)));

  (void)bridge.onRead(ctx, erase_and_box(makeRequest(conn, 3, "ping")));
  (void)bridge.onWrite(ctx, erase_and_box(makeResponseFor(ctx, 3)));

  EXPECT_EQ(std::count(log.calls.begin(), log.calls.end(), "sawCatHeader"), 1);
}

// The ambient context a handler stamps in preRead is the one still installed
// when its write-side callbacks run — one context per request, not the
// process-global default.
TEST(TProcessorEventHandlerBridgeTest, AmbientContextSpansBothDirections) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  (void)bridge.onRead(ctx, erase_and_box(makeRequest(conn, 1, "ping")));
  (void)bridge.onWrite(ctx, erase_and_box(makeResponse(1)));

  EXPECT_EQ(log.ambientMarker, "stamped");
  // Nothing leaked into the caller's context.
  EXPECT_EQ(
      folly::RequestContext::get()->getContextData(markerToken()), nullptr);
}

// A handler throwing from preRead is refusing the request: it is dropped
// rather than forwarded, the client is answered, and — as on a classic server,
// where the exception precedes dispatch — the write-side callbacks never run.
TEST(
    TProcessorEventHandlerBridgeTest, RefusalDropsTheRequestAndSkipsPostWrite) {
  CallLog log;
  log.refusePreRead = true;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  EXPECT_EQ(
      bridge.onRead(ctx, erase_and_box(makeRequest(conn, 3, "ping"))),
      Result::Success);

  // Only the setup message went downstream; the request did not.
  EXPECT_EQ(ctx.read.size(), 1);
  ASSERT_EQ(ctx.written.size(), 1);
  EXPECT_EQ(
      std::find(log.calls.begin(), log.calls.end(), "postWrite"),
      log.calls.end());

  // The refusal carries the header the handler set before throwing.
  auto& response = ctx.written.front().get<ThriftServerResponseMessage>();
  const auto& other = *response.payload.get<ThriftInitialResponsePayload>()
                           .metadata->otherMetadata();
  ASSERT_TRUE(other.contains("denied-by"));
  EXPECT_EQ(other.at("denied-by"), "recording-handler");
}

// A handler throwing while it is being handed the request's context is
// refusing the request too — not escaping the pipeline callback, which is
// noexcept. The contexts the handlers ahead of it already gave up come back.
TEST(TProcessorEventHandlerBridgeTest, GetServiceContextRefusalIsCaught) {
  CallLog log;
  log.refuseGetServiceContext = true;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  EXPECT_EQ(
      bridge.onRead(ctx, erase_and_box(makeRequest(conn, 3, "ping"))),
      Result::Success);

  // Only the setup message went downstream; the request did not.
  EXPECT_EQ(ctx.read.size(), 1);
  EXPECT_EQ(ctx.written.size(), 1);
  EXPECT_EQ(
      std::find(log.calls.begin(), log.calls.end(), "preRead"),
      log.calls.end());
  EXPECT_NE(
      std::find(log.calls.begin(), log.calls.end(), "freeContext"),
      log.calls.end());
}

// The method names a connection can accumulate are capped, so a peer sending
// more distinct ones than the cap still gets each one qualified correctly —
// the name just comes from the request's own state rather than the cache.
TEST(TProcessorEventHandlerBridgeTest, QualifiesBeyondTheMethodCacheCap) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  constexpr uint32_t kDistinctMethods = 200;
  for (uint32_t i = 1; i <= kDistinctMethods; ++i) {
    const auto method = "method" + std::to_string(i);
    (void)bridge.onRead(ctx, erase_and_box(makeRequest(conn, i, method)));
    (void)bridge.onWrite(ctx, erase_and_box(makeResponse(i)));
    EXPECT_EQ(log.methodName, "TestService." + method);
  }
}

// The write-side callbacks bracket serializing a reply body, so a framework
// error that carries none does not get them — matching the classic server,
// where an error sent outside the reply path never reaches them. The handlers'
// contexts are still returned, which is the pairing they are promised.
TEST(TProcessorEventHandlerBridgeTest, ErrorFrameSkipsWriteCallbacksButFrees) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  (void)bridge.onRead(ctx, erase_and_box(makeRequest(conn, 5, "ping")));
  (void)bridge.onWrite(ctx, erase_and_box(makeErrorResponse(5)));

  EXPECT_EQ(
      std::find(log.calls.begin(), log.calls.end(), "preWrite"),
      log.calls.end());
  EXPECT_EQ(
      std::find(log.calls.begin(), log.calls.end(), "postWrite"),
      log.calls.end());
  EXPECT_NE(
      std::find(log.calls.begin(), log.calls.end(), "freeContext"),
      log.calls.end());
}

// A response the bridge never saw a request for — a connection-level frame, or
// the answer to a request it refused — passes through untouched.
TEST(TProcessorEventHandlerBridgeTest, UnmatchedResponsePassesThrough) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  EXPECT_EQ(
      bridge.onWrite(ctx, erase_and_box(makeResponse(99))), Result::Success);

  EXPECT_EQ(ctx.written.size(), 1);
  EXPECT_EQ(
      std::find(log.calls.begin(), log.calls.end(), "preWrite"),
      log.calls.end());
}

// Every request gets its own contexts even though they share the connection's.
TEST(TProcessorEventHandlerBridgeTest, ConcurrentRequestsAreTrackedSeparately) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  (void)bridge.onRead(ctx, erase_and_box(makeRequest(conn, 1, "ping")));
  const auto* first = log.readContext;
  (void)bridge.onRead(ctx, erase_and_box(makeRequest(conn, 2, "echo")));
  const auto* second = log.readContext;

  EXPECT_NE(first, second);
  EXPECT_EQ(first->getConnectionContext(), second->getConnectionContext());

  // Responses arrive out of order; each still finds its own request.
  (void)bridge.onWrite(ctx, erase_and_box(makeResponse(2)));
  (void)bridge.onWrite(ctx, erase_and_box(makeResponse(1)));
  EXPECT_EQ(std::count(log.calls.begin(), log.calls.end(), "postWrite"), 2);
}

// A request that arrives with no context is a misconfigured server, not a
// request to let through: the bridge cannot tell whether a handler was going
// to authorize it.
TEST(TProcessorEventHandlerBridgeTest, RequestWithoutContextIsRefused) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  auto request = makeRequest(conn, 5, "ping");
  request.requestContext.reset();
  EXPECT_EQ(
      bridge.onRead(ctx, erase_and_box(std::move(request))), Result::Success);

  EXPECT_EQ(ctx.read.size(), 1); // setup only
  EXPECT_EQ(ctx.written.size(), 1);
  EXPECT_EQ(
      std::find(log.calls.begin(), log.calls.end(), "preRead"),
      log.calls.end());
}

// A handler is promised one connectionDestroyed for the newConnection it was
// told about, however many times the event arrives.
TEST(TProcessorEventHandlerBridgeTest, ConnectionClosedIsAnnouncedOnce) {
  CallLog log;
  Bridge bridge(makeConfig(&log));
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  bridge.onEvent(ctx, ThriftServerEventType::ConnectionClosed, TypeErasedBox{});
  bridge.onEvent(ctx, ThriftServerEventType::ConnectionClosed, TypeErasedBox{});

  EXPECT_EQ(
      std::count(log.calls.begin(), log.calls.end(), "connectionDestroyed"), 1);
}

// With nothing installed the bridge is a pass-through, so a server can wire it
// unconditionally.
TEST(TProcessorEventHandlerBridgeTest, NoHandlersForwardsUntouched) {
  CallLog log;
  Bridge bridge(
      TProcessorEventHandlerBridgeConfig{
          .handlers = nullptr, .identityResolver = nullptr});
  FakeContext ctx;
  auto conn = makeConn();
  establish(bridge, ctx, conn);

  EXPECT_EQ(
      bridge.onRead(ctx, erase_and_box(makeRequest(conn, 1, "ping"))),
      Result::Success);

  EXPECT_EQ(ctx.read.size(), 2);
  EXPECT_TRUE(ctx.written.empty());
  EXPECT_TRUE(log.calls.empty());
}

} // namespace apache::thrift::fast_thrift::thrift::server
