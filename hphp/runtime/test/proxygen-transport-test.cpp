#include <gtest/gtest.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <folly/io/async/HHWheelTimer.h>
#include <ostream>

#include "hphp/runtime/server/proxygen/proxygen-server.h"
#include "hphp/runtime/server/proxygen/proxygen-transport.h"

using namespace testing;
using proxygen::HTTPException;
using proxygen::HTTPMessage;
using proxygen::HTTPMethod;
using proxygen::HTTPTransaction;
using proxygen::HTTPPushTransactionHandler;
using proxygen::MockHTTPTransaction;
using proxygen::TransportDirection;
using proxygen::HTTPCodec;
using proxygen::WheelTimerInstance;

namespace boost {
/*
 * Under gcc-4.9, gtest ends up needing this, but not instantiating it.
 * Since its not needed under 4.8, or clang, I'm guessing its a subtle
 * bug in 4.9.
 */
std::ostream& operator << (std::ostream &o,
                           const boost::optional<unsigned char>& a) {
  if (!a) {
    o << "None";
  } else {
    o << *a;
  }
  return o;
}
}

namespace HPHP {

static ServerOptions s_options{std::string(""), 80, 1};

struct MockProxygenServer : ProxygenServer {

  MockProxygenServer()
      : ProxygenServer(s_options) {}

  MOCK_METHOD1(onRequest, void(std::shared_ptr<ProxygenTransport>));
  virtual void putResponseMessage(ResponseMessage&& message) {
    m_messageQueue.emplace_back(std::move(message));
  }

  MOCK_METHOD0(decrementEnqueuedCount, void());

  void deliverMessages(int32_t n = -1) {
    while (n > 0 || (n < 0 && !m_messageQueue.empty())) {
      EXPECT_FALSE(m_messageQueue.empty());
      auto message = std::move(m_messageQueue.front());
      auto m_transport = message.m_transport;
      m_transport->messageAvailable(std::move(message));
      m_messageQueue.pop_front();
      n--;
    }
  }

  std::list<ResponseMessage> m_messageQueue;
};

std::unique_ptr<HTTPMessage> getRequest(HTTPMethod type) {
  auto req = folly::make_unique<HTTPMessage>();
  req->setMethod(type);
  req->setHTTPVersion(1, 1);
  req->setURL("/");
  return req;
}

struct ProxygenTransportTest : testing::Test {
  ProxygenTransportTest()
      : m_timeouts(folly::HHWheelTimer::newTimer(
            &m_eventBase,
            std::chrono::milliseconds(
                folly::HHWheelTimer::DEFAULT_TICK_INTERVAL),
            folly::AsyncTimeout::InternalEnum::NORMAL,
            std::chrono::milliseconds(100))),
        m_txn(
            TransportDirection::DOWNSTREAM,
            HTTPCodec::StreamID(1),
            1,
            m_egressQueue,
            WheelTimerInstance(m_timeouts.get())) {
    m_transport = std::make_shared<ProxygenTransport>(&m_server);
    m_transport->setTransactionReference(m_transport);
    m_transport->setTransaction(&m_txn);
  }

  // Initiates a simple GET request to the transport
  void SetUp() override {
    EXPECT_CALL(m_server, onRequest(_))
      .WillOnce(Invoke([] (std::shared_ptr<ProxygenTransport> transport) {
            transport->setEnqueued();
          }));
    auto req = getRequest(HTTPMethod::GET);
    m_transport->onHeadersComplete(std::move(req));
    m_transport->onEOM();
  }

  // Simulates finishing from the VM thread
  void TearDown() override {
    auto transport = m_transport.get();
    if (!transport) {
      return;
    }
    m_transport->finish(std::move(m_transport));
    EXPECT_EQ(m_server.m_messageQueue.size(), 1);
    m_server.deliverMessages();
    EXPECT_CALL(m_server, decrementEnqueuedCount());
    transport->detachTransaction();
  }

  uint64_t pushResource(Array& headers, uint8_t pri, bool eom = false) {
    m_txn.enablePush();
    EXPECT_CALL(m_txn.mockCodec_, getProtocol())
      .WillRepeatedly(Return(proxygen::CodecProtocol::HTTP_2));
    EXPECT_TRUE(m_transport->supportsServerPush());
    auto id = m_transport->pushResource("foo", "/bar", pri, headers,
                                        nullptr, 0, eom);
    EXPECT_GT(id, 0);
    EXPECT_EQ(m_server.m_messageQueue.size(), 2);
    return id;
  }

  void expectPushPromiseAndHeaders(
    MockHTTPTransaction& pushTxn, uint8_t pri,
    HTTPPushTransactionHandler**pushHandlerPtr) {
    EXPECT_CALL(m_txn, newPushedTransaction(_))
      .WillOnce(DoAll(SaveArg<0>(pushHandlerPtr),
                      Return(&pushTxn)));
    EXPECT_CALL(pushTxn, sendHeaders(_))
      .WillOnce(Invoke([pri] (const HTTPMessage& promise) {
            EXPECT_TRUE(promise.isRequest());
            EXPECT_EQ(promise.getPriority(), pri);
            EXPECT_EQ(promise.getHeaders().getSingleOrEmpty("hello"),
                      std::string("world"));
          }))
      .WillOnce(Invoke([] (const HTTPMessage& response) {
            EXPECT_TRUE(response.isResponse());
          }));
  }

  void sendResponse(const std::string& body) {
    m_transport->sendImpl(body.data(), body.length(), 200, false, true);
    EXPECT_EQ(m_server.m_messageQueue.size(), 1);
    EXPECT_CALL(m_txn, sendHeaders(_));
    EXPECT_CALL(m_txn, sendBody(_));
    EXPECT_CALL(m_txn, sendEOM());
    m_server.deliverMessages();
  }

 protected:
  folly::EventBase m_eventBase;
  folly::HHWheelTimer::UniquePtr m_timeouts;
  proxygen::HTTP2PriorityQueue m_egressQueue;
  MockProxygenServer m_server;
  MockHTTPTransaction m_txn;
  std::shared_ptr<ProxygenTransport> m_transport;
};

struct ProxygenTransportRepostTest : ProxygenTransportTest {

  // Initiates a simple GET request to the transport
  void SetUp() override {
  }

  void TearDown() override {
    auto transport = m_transport.get();
    if (!transport) {
      return;
    }
    transport->detachTransaction();
    EXPECT_EQ(m_transport.use_count(), 1);
  }



};

TEST_F(ProxygenTransportTest, basic) {
  sendResponse("12345");
}

TEST_F(ProxygenTransportTest, push) {
  // Push a resource
  Array headers;
  uint8_t pri = 1;

  headers.append("hello: world"); // vec serialization path
  auto id = pushResource(headers, pri);

  // And some body bytes
  std::string body("12345");
  m_transport->pushResourceBody(id, body.data(), body.length(), false);
  EXPECT_EQ(m_server.m_messageQueue.size(), 3);

  // Creates a new transaction and sends headers/body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(1), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  EXPECT_CALL(pushTxn, sendBody(_));
  m_server.deliverMessages();

  // Send Push EOM
  m_transport->pushResourceBody(id, nullptr, 0, true);
  EXPECT_EQ(m_server.m_messageQueue.size(), 1);
  EXPECT_CALL(pushTxn, sendEOM());
  m_server.deliverMessages();
  pushHandler->detachTransaction();
  // Send response
  sendResponse("12345");
}

TEST_F(ProxygenTransportTest, push_empty_body) {
  // Push a resource
  Array headers;
  uint8_t pri = 1;

  headers.add(String("hello"), String("world"));  // dict serializtion path
  pushResource(headers, pri, true /* eom, no body */);

  // Creates a new transaction and sends headers and an empty body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(1), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  EXPECT_CALL(pushTxn, sendEOM());
  m_server.deliverMessages();

  pushHandler->detachTransaction();
  // Send response
  sendResponse("12345");
}

TEST_F(ProxygenTransportTest, push_abort_incomplete) {
  // Push a resource
  Array headers;
  uint8_t pri = 1;

  headers.add(String("hello"), String("world"));  // dict serializtion path
  pushResource(headers, pri);

  // Creates a new transaction and sends headers, but not body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(1), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  m_server.deliverMessages();
  sendResponse("12345");

  EXPECT_CALL(pushTxn, sendAbort())
    .WillOnce(Invoke([pushHandler] {
          pushHandler->detachTransaction();
        }));
  // Simulate termination of the VM thread while there is an incomplete push
  // This aborts the incomplete push
  TearDown();
}

TEST_F(ProxygenTransportTest, push_abort) {
  // Push a resource
  Array headers;
  uint8_t pri = 1;

  headers.add(String("hello"), String("world"));  // dict serializtion path
  auto id = pushResource(headers, pri);

  // Creates a new transaction and sends headers, but not body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(1), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  m_server.deliverMessages();

  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   "Stream aborted, streamID");
  ex.setProxygenError(proxygen::kErrorStreamAbort);
  ex.setCodecStatusCode(proxygen::ErrorCode::CANCEL);
  pushTxn.onError(ex);
  pushHandler->detachTransaction();
  m_transport->pushResourceBody(id, nullptr, 0, true);
  m_server.deliverMessages();
  sendResponse("12345");
}

TEST_F(ProxygenTransportRepostTest, no_body) {
  InSequence enforceOrder;
  auto req = getRequest(HTTPMethod::POST);

  EXPECT_CALL(m_txn, sendHeaders(_));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->beginPartialPostEcho();
  m_transport->onEOM();
}

TEST_F(ProxygenTransportRepostTest, mid_body) {
  InSequence enforceOrder;
  auto req = getRequest(HTTPMethod::POST);
  auto body1 = folly::IOBuf::copyBuffer(std::string("hello"));
  auto body2 = folly::IOBuf::copyBuffer(std::string("world"));

  EXPECT_CALL(m_txn, sendHeaders(_));
  EXPECT_CALL(m_txn, sendBody(_))
    .Times(2);
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->onBody(std::move(body1));
  m_transport->beginPartialPostEcho();
  m_transport->onBody(std::move(body2));
  m_transport->onEOM();
}

TEST_F(ProxygenTransportRepostTest, after_body) {
  InSequence enforceOrder;
  auto req = getRequest(HTTPMethod::POST);
  auto body1 = folly::IOBuf::copyBuffer(std::string("hello"));
  auto body2 = folly::IOBuf::copyBuffer(std::string("world"));

  EXPECT_CALL(m_txn, sendHeaders(_));
  EXPECT_CALL(m_txn, sendBody(_));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->onBody(std::move(body1));
  m_transport->onBody(std::move(body2));
  m_transport->beginPartialPostEcho();
  m_transport->onEOM();
}

TEST_F(ProxygenTransportRepostTest, mid_body_abort) {
  InSequence enforceOrder;
  auto req = getRequest(HTTPMethod::POST);
  auto body1 = folly::IOBuf::copyBuffer(std::string("hello"));
  auto body2 = folly::IOBuf::copyBuffer(std::string("world"));

  EXPECT_CALL(m_txn, sendHeaders(_));
  EXPECT_CALL(m_txn, sendBody(_))
    .Times(2);
  m_transport->onHeadersComplete(std::move(req));
  m_transport->onBody(std::move(body1));
  m_transport->beginPartialPostEcho();
  m_transport->onBody(std::move(body2));
  m_transport->abort();
}

}
