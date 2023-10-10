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

#include <memory>

#include <folly/io/IOBuf.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <proxygen/httpserver/ScopedHTTPServer.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/cpp2/transport/core/testutil/CoreTestFixture.h>
#include <thrift/lib/cpp2/transport/http2/client/H2ClientConnection.h>
#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>
#include <thrift/lib/cpp2/transport/http2/common/testutil/ChannelTestFixture.h>
#include <thrift/lib/cpp2/transport/http2/common/testutil/FakeProcessors.h>

namespace apache {
namespace thrift {

using std::string;
using std::unordered_map;

class SingleRpcChannelTest
    : public ChannelTestFixture,
      public testing::WithParamInterface<string::size_type> {};

TEST_P(SingleRpcChannelTest, VaryingChunkSizes) {
  EchoProcessor processor("extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> inputHeaders;
  inputHeaders["key1"] = "value1";
  inputHeaders["key2"] = "value2";
  string inputPayload = "single stream payload";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor,
      inputHeaders,
      inputPayload,
      GetParam(),
      outputHeaders,
      outputPayload);
  EXPECT_EQ(3, outputHeaders->size());
  EXPECT_EQ("value1", outputHeaders->at("key1"));
  EXPECT_EQ("value2", outputHeaders->at("key2"));
  EXPECT_EQ("extravalue", outputHeaders->at("extrakey"));
  EXPECT_EQ("single stream payload<eom>", toString(outputPayload));
}

INSTANTIATE_TEST_CASE_P(
    AllChunkSizes, SingleRpcChannelTest, testing::Values(0, 1, 2, 4, 10));

TEST_F(ChannelTestFixture, SingleRpcChannelErrorEmptyBody) {
  EchoProcessor processor("extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> inputHeaders;
  inputHeaders["key1"] = "value1";
  string inputPayload = "";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor,
      inputHeaders,
      inputPayload,
      0,
      outputHeaders,
      outputPayload,
      true);
  EXPECT_EQ(0, outputHeaders->size());
  TApplicationException tae;
  EXPECT_TRUE(CoreTestFixture::deserializeException(outputPayload, &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN, tae.getType());
  EXPECT_EQ("Proxygen stream has no body", tae.getMessage());
}

TEST_F(ChannelTestFixture, SingleRpcChannelErrorNoEnvelope) {
  EchoProcessor processor("extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> inputHeaders;
  inputHeaders["key1"] = "value1";
  string inputPayload = "notempty";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor,
      inputHeaders,
      inputPayload,
      0,
      outputHeaders,
      outputPayload,
      true);
  EXPECT_EQ(0, outputHeaders->size());
  TApplicationException tae;
  EXPECT_TRUE(CoreTestFixture::deserializeException(outputPayload, &tae));
  EXPECT_EQ(TApplicationException::UNKNOWN, tae.getType());
  EXPECT_EQ("Invalid envelope: see logs for error", tae.getMessage());
}

TEST_F(ChannelTestFixture, BadHeaderFields) {
  EchoProcessor processor("extrakey", "extravalue", "<eom>", eventBase_.get());
  unordered_map<string, string> headersExpectNoEncoding{
      {"X-FB-Header-Uppercase", "good value"},
      {"x-fb-header-lowercase", "good value"}};
  unordered_map<string, string> headersExpectEncoding{
      {"good header2", "bad\x01\x02value\r\n"},
      {"bad\x01header", "good value"},
      {"header:with:colon", "bad value\r\n\r\n"},
      {"asdf:gh", "{\"json\":\"data\"}"},
      {"bad header1", "good value"}};
  unordered_map<string, string> inputHeaders;
  inputHeaders.insert(
      headersExpectNoEncoding.begin(), headersExpectNoEncoding.end());
  inputHeaders.insert(
      headersExpectEncoding.begin(), headersExpectEncoding.end());
  string inputPayload = "single stream payload";
  unordered_map<string, string>* outputHeaders;
  IOBuf* outputPayload;
  sendAndReceiveStream(
      &processor, inputHeaders, inputPayload, 0, outputHeaders, outputPayload);
  EXPECT_EQ(
      1 /* extrakey/value */ + headersExpectEncoding.size() +
          headersExpectNoEncoding.size(),
      outputHeaders->size());
  auto numUnencoded = 0;
  for (const auto& elem : *outputHeaders) {
    LOG(INFO) << elem.first << ":" << elem.second;
    if (elem.first.find("encode_") != 0) {
      if (elem.first == "extrakey") {
        EXPECT_EQ(elem.second, "extravalue");
      } else {
        numUnencoded++;
        EXPECT_TRUE(
            headersExpectNoEncoding.find(elem.first) !=
            headersExpectNoEncoding.end());
      }
    }
  }
  EXPECT_EQ(numUnencoded, headersExpectNoEncoding.size());
  EXPECT_EQ("single stream payload<eom>", toString(outputPayload));
}

struct RequestState {
  bool sent{false};
  bool reply{false};
  bool error{false};
  ClientReceiveState receiveState;
};

class TestRequestCallback : public apache::thrift::RequestCallback {
 public:
  explicit TestRequestCallback(folly::Promise<RequestState> promise)
      : promise_(std::move(promise)) {}

  void requestSent() final { rstate_.sent = true; }
  void replyReceived(ClientReceiveState&& state) final {
    rstate_.reply = true;
    rstate_.receiveState = std::move(state);
    promise_.setValue(std::move(rstate_));
  }
  void requestError(ClientReceiveState&& state) final {
    rstate_.error = true;
    rstate_.receiveState = std::move(state);
    promise_.setValue(std::move(rstate_));
  }

 private:
  RequestState rstate_;
  folly::Promise<RequestState> promise_;
};

template <class HandlerType>
std::unique_ptr<proxygen::ScopedHTTPServer> startProxygenServer(
    HandlerType handler) {
  folly::SocketAddress saddr;
  saddr.setFromLocalPort(static_cast<uint16_t>(0));
  proxygen::HTTPServer::IPConfig cfg{
      saddr, proxygen::HTTPServer::Protocol::HTTP2};
  auto f =
      std::make_unique<proxygen::ScopedHandlerFactory<HandlerType>>(handler);
  proxygen::HTTPServerOptions options;
  options.threads = 1;
  options.handlerFactories.push_back(std::move(f));

  return proxygen::ScopedHTTPServer::start(cfg, std::move(options));
}

void httpHandler(
    proxygen::HTTPMessage message,
    std::unique_ptr<folly::IOBuf> /* data */,
    proxygen::ResponseBuilder& builder) {
  auto generateResponse = [](std::string value) {
    auto resp = LegacySerializedResponse(
        protocol::T_COMPACT_PROTOCOL,
        "FooBar",
        SerializedResponse(folly::IOBuf::copyBuffer(value)));
    return std::move(resp.buffer);
  };
  if (message.getURL() == "internal_error") {
    builder.status(500, "Internal Server Error")
        .body(generateResponse("internal error"));
  } else if (message.getURL() == "thrift_serialized_internal_error") {
    builder.status(500, "OOM")
        .header(proxygen::HTTP_HEADER_CONTENT_TYPE, "application/x-thrift")
        .body(generateResponse("oom"));
  } else if (message.getURL() == "app_overloaded") {
    builder.status(503, "Service Unavailable")
        .header(proxygen::HTTP_HEADER_RETRY_AFTER, "0");
  } else if (message.getURL() == "eof") {
    builder.status(200, "OK");
  } else {
    builder.status(200, "OK").body(generateResponse("(y)"));
  }
}

folly::Future<RequestState> sendRequest(
    folly::EventBase& evb,
    apache::thrift::ThriftChannelIf& channel,
    std::string url) {
  folly::Promise<RequestState> promise;
  auto f = promise.getFuture();

  apache::thrift::RequestCallback::Context context;
  context.protocolId = protocol::T_COMPACT_PROTOCOL;

  auto cb = std::make_unique<ThriftClientCallback>(
      &evb,
      false,
      toRequestClientCallbackPtr(
          std::make_unique<TestRequestCallback>(std::move(promise)),
          std::move(context)),
      std::chrono::milliseconds{10000});

  // Send a bad request.
  evb.runInEventBaseThread(
      [&channel, url = std::move(url), cb = std::move(cb)]() mutable {
        ThriftChannelIf::RequestMetadata metadata;
        metadata.url = url;
        metadata.requestRpcMetadata.kind_ref() =
            ::apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
        channel.sendThriftRequest(
            std::move(metadata), folly::IOBuf::create(1), std::move(cb));
      });

  return f;
}

void validateException(
    RequestState& rstate,
    transport::TTransportException::TTransportExceptionType type,
    const char* expectedError) {
  EXPECT_TRUE(rstate.sent);
  EXPECT_TRUE(rstate.error);
  EXPECT_FALSE(rstate.reply);
  ASSERT_TRUE(rstate.receiveState.isException());
  auto* ex = rstate.receiveState.exception()
                 .get_exception<transport::TTransportException>();
  ASSERT_NE(nullptr, ex);
  EXPECT_EQ(type, ex->getType());
  EXPECT_STREQ(expectedError, ex->what());
}

TEST(SingleRpcChannel, ClientExceptions) {
  // Spin up the server.
  auto server = startProxygenServer(&httpHandler);
  ASSERT_NE(nullptr, server);
  const auto port = server->getPort();
  ASSERT_NE(0, port);

  // Spin up the client channel.
  folly::EventBase evb;
  folly::SocketAddress addr;
  addr.setFromLocalPort(port);

  folly::AsyncSocket::UniquePtr sock(new folly::AsyncSocket(&evb, addr));
  auto conn = H2ClientConnection::newHTTP2Connection(std::move(sock));
  EXPECT_TRUE(conn->good());
  auto channel = conn->getChannel();

  auto rstate = sendRequest(evb, *channel, "internal_error").getVia(&evb);

  // Validate that we get proper exception.
  validateException(
      rstate,
      transport::TTransportException::UNKNOWN,
      "Bad status: 500 Internal Server Error");

  // The connection should be still good!
  EXPECT_TRUE(conn->good());

  // Follow up with a request that results in a server error that gets thrift
  // serialized.
  channel = conn->getChannel();
  rstate = sendRequest(evb, *channel, "thrift_serialized_internal_error")
               .getVia(&evb);

  EXPECT_TRUE(rstate.sent);
  EXPECT_FALSE(rstate.error);
  EXPECT_TRUE(rstate.reply);
  ASSERT_FALSE(rstate.receiveState.isException());
  EXPECT_EQ(
      "oom",
      folly::StringPiece(
          rstate.receiveState.serializedResponse().buffer->coalesce()));

  // The connection should be still good!
  EXPECT_TRUE(conn->good());

  // Follow up with a request that results in empty payload.
  channel = conn->getChannel();
  rstate = sendRequest(evb, *channel, "eof").getVia(&evb);

  validateException(
      rstate, transport::TTransportException::END_OF_FILE, "No content");

  // The connection should be still good!
  EXPECT_TRUE(conn->good());

  // Follow up with an OK request.
  channel = conn->getChannel();
  rstate = sendRequest(evb, *channel, "ok").getVia(&evb);

  EXPECT_TRUE(rstate.sent);
  EXPECT_FALSE(rstate.error);
  EXPECT_TRUE(rstate.reply);

  // Ensure that a HTTP 503 with Retry-After is marked as APP_OVERLOADED
  rstate = sendRequest(evb, *channel, "app_overloaded").getVia(&evb);

  EXPECT_TRUE(rstate.sent);
  EXPECT_FALSE(rstate.error);
  EXPECT_TRUE(rstate.reply);
  ASSERT_FALSE(rstate.receiveState.isException());
  auto headers = rstate.receiveState.header()->getHeaders();
  auto iter = headers.find("ex");
  EXPECT_TRUE(iter != headers.end());
  EXPECT_EQ(iter->second, kAppOverloadedErrorCode);

  conn->closeNow();
  evb.loopOnce();
}

TEST(H2ChannelTest, decodeHeaders) {
  // Declare a subclass to expose decodeHeaders for testing
  class FakeChannel : public H2Channel {
    void onH2StreamBegin(
        std::unique_ptr<proxygen::HTTPMessage>) noexcept override {}
    void onH2BodyFrame(std::unique_ptr<folly::IOBuf>) noexcept override {}
    void onH2StreamEnd() noexcept override {}

   public:
    static void decodeHeaders(
        proxygen::HTTPMessage& message,
        transport::THeader::StringToStringMap& otherMetadata,
        RequestRpcMetadata* metadata) {
      H2Channel::decodeHeaders(message, otherMetadata, metadata);
    }
  };

  proxygen::HTTPMessage req;
  req.getHeaders().set(
      proxygen::HTTP_HEADER_CONTENT_TYPE, "application/x-thrift");
  req.getHeaders().set(proxygen::HTTP_HEADER_USER_AGENT, "C++/THttpClient");
  req.getHeaders().set(proxygen::HTTP_HEADER_HOST, "graph.facebook.com");
  req.getHeaders().set("client_timeout", "500");
  req.getHeaders().set("rpckind", "0");
  req.getHeaders().set("other", "metadatametadatametadatametadata");
  req.getHeaders().set("encode_1", "ZW5jb2RlZDpPdGhlcg_aGVsbG8NCndvcmxk");
  req.getHeaders().set(
      proxygen::HTTP_HEADER_CONTENT_LANGUAGE,
      "Arrgh, this be pirate tongue matey");
  RequestRpcMetadata metadata;
  transport::THeader::StringToStringMap otherMetadata;
  FakeChannel::decodeHeaders(req, otherMetadata, &metadata);
  EXPECT_EQ(otherMetadata.size(), 3);
  EXPECT_EQ(
      otherMetadata[std::string("other")], "metadatametadatametadatametadata");
  EXPECT_EQ(otherMetadata[std::string("encoded:Other")], "hello\r\nworld");
  EXPECT_EQ(
      otherMetadata[std::string("Content-Language")],
      "Arrgh, this be pirate tongue matey");
  EXPECT_EQ(*apache::thrift::get_pointer(metadata.clientTimeoutMs()), 500);
  EXPECT_EQ(
      *apache::thrift::get_pointer(metadata.kind()),
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
}

} // namespace thrift
} // namespace apache
