#include <folly/portability/GTest.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <folly/io/async/HHWheelTimer.h>
#include <limits>
#include <ostream>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/proxygen/proxygen-server.h"
#include "hphp/runtime/server/proxygen/proxygen-transport.h"

using namespace testing;
using proxygen::HTTPCodec;
using proxygen::HTTPException;
using proxygen::HTTP_HEADER_EXPECT;
using proxygen::HTTP_HEADER_CONTENT_LENGTH;
using proxygen::HTTPMessage;
using proxygen::HTTPMethod;
using proxygen::HTTPTransaction;
using proxygen::HTTPPushTransactionHandler;
using proxygen::MockHTTPTransaction;
using proxygen::TransportDirection;
using proxygen::WheelTimerInstance;

MATCHER_P(IsResponseStatusCode, statusCode, "") {
  return arg.getStatusCode() == statusCode;
}

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

  MOCK_METHOD1(onRequestError, void(Transport*));

  MOCK_METHOD1(onRequest, void(std::shared_ptr<ProxygenTransport>));

  MOCK_METHOD0(decrementEnqueuedCount, void());
};

struct MockHPHPWorkerThread : HPHPWorkerThread {
  explicit MockHPHPWorkerThread(MockProxygenServer* server)
      : HPHPWorkerThread(nullptr, server) {}

  void putResponseMessage(ResponseMessage&& message) override {
    m_messageQueue.emplace_back(std::move(message));
  }

  void deliverMessages(int32_t n = -1) {
    while (n > 0 || (n < 0 && !m_messageQueue.empty())) {
      EXPECT_FALSE(m_messageQueue.empty());
      auto message = std::move(m_messageQueue.front());
      auto transport = message.m_transport;
      transport->messageAvailable(std::move(message));
      m_messageQueue.pop_front();
      n--;
    }
  }

  std::list<ResponseMessage> m_messageQueue;
};

std::unique_ptr<HTTPMessage> getRequest(HTTPMethod type) {
  auto req = std::make_unique<HTTPMessage>();
  req->setMethod(type);
  req->setHTTPVersion(1, 1);
  req->setURL("/");
  return req;
}

struct ProxygenTransportBasicTest : testing::Test {
  ProxygenTransportBasicTest()
      : m_timeouts(folly::HHWheelTimer::newTimer(
            &m_eventBase,
            std::chrono::milliseconds(
                folly::HHWheelTimer::DEFAULT_TICK_INTERVAL),
            folly::AsyncTimeout::InternalEnum::NORMAL,
            std::chrono::milliseconds(100))),
        m_worker(&m_server),
        m_txn(
            TransportDirection::DOWNSTREAM,
            HTTPCodec::StreamID(1),
            1,
            m_egressQueue,
            WheelTimerInstance(m_timeouts.get())) {
    m_transport = std::make_shared<ProxygenTransport>(&m_server, &m_worker);
    m_transport->setTransactionReference(m_transport);
    m_transport->setTransaction(&m_txn);
  }

  void SetUp() override {
  }

  void TearDown() override {
  }

 protected:
  folly::EventBase m_eventBase;
  folly::HHWheelTimer::UniquePtr m_timeouts;
  proxygen::HTTP2PriorityQueue m_egressQueue;
  StrictMock<MockProxygenServer> m_server;
  StrictMock<MockHPHPWorkerThread> m_worker;
  StrictMock<MockHTTPTransaction> m_txn;
  std::shared_ptr<ProxygenTransport> m_transport;
};

struct ProxygenTransportTest : ProxygenTransportBasicTest {

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
    EXPECT_EQ(m_worker.m_messageQueue.size(), 1);
    m_worker.deliverMessages();
    EXPECT_CALL(m_server, decrementEnqueuedCount());
    transport->detachTransaction();
  }

  uint64_t pushResource(Array& promiseHeaders, Array& responseHeaders,
                        uint8_t pri, bool eom = false) {
    m_txn.enablePush();
    m_txn.setupCodec(proxygen::CodecProtocol::HTTP_2);
    EXPECT_TRUE(m_transport->supportsServerPush());
    auto id = m_transport->pushResource("foo", "/bar", pri, promiseHeaders,
                                        responseHeaders,  nullptr, 0, eom);
    EXPECT_GT(id, 0);
    EXPECT_EQ(m_worker.m_messageQueue.size(), 2);
    return id;
  }

  void expectPushPromiseAndHeaders(
    MockHTTPTransaction& pushTxn, uint8_t pri,
    HTTPPushTransactionHandler**pushHandlerPtr) {
#ifdef HHVM_FACEBOOK
    EXPECT_CALL(m_txn, newPushedTransaction(_,_))
      .WillOnce(DoAll(SaveArg<0>(pushHandlerPtr),
                      Return(&pushTxn)));
#else
    EXPECT_CALL(m_txn, newPushedTransaction(_))
        .WillOnce(DoAll(SaveArg<0>(pushHandlerPtr),
                        Return(&pushTxn)));
#endif
    EXPECT_CALL(pushTxn, sendHeaders(_))
      .WillOnce(Invoke([pri] (const HTTPMessage& promise) {
            EXPECT_TRUE(promise.isRequest());
            EXPECT_EQ(promise.getPriority(), pri);
            EXPECT_EQ(promise.getHeaders().getSingleOrEmpty("hello"),
                      std::string("world"));
          }))
      .WillOnce(Invoke([] (const HTTPMessage& response) {
            EXPECT_TRUE(response.isResponse());
            EXPECT_EQ(response.getHeaders().getSingleOrEmpty("foo"),
                      std::string("bar"));
          }));
  }

  void sendResponse(const std::string& body) {
    m_transport->sendImpl(body.data(), body.length(), 200, false, true);
    EXPECT_EQ(m_worker.m_messageQueue.size(), 1);
    EXPECT_CALL(m_txn, sendHeaders(_));
    EXPECT_CALL(m_txn, sendBody(_));
    EXPECT_CALL(m_txn, sendEOM());
    m_worker.deliverMessages();
  }
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

TEST_F(ProxygenTransportBasicTest, unsupported_method) {
  auto req = getRequest(HTTPMethod::UNSUB);
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(400)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->onEOM();
}

TEST_F(ProxygenTransportBasicTest, body_after_413) {
  auto req = getRequest(HTTPMethod::POST);
  m_transport->setMaxPost(RuntimeOption::MaxPostSize, 30);
  auto length = folly::to<std::string>(RuntimeOption::MaxPostSize + 1);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, length);
  EXPECT_CALL(m_server, onRequestError(_));
  // The WillOnces will call the non-mocked functions to move the state machine.
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(413)))
  .WillOnce(Invoke(
    [&](const HTTPMessage& m) {
      m_txn.HTTPTransaction::sendHeaders(m);
    }));
  EXPECT_CALL(m_txn, sendEOM())
  .WillOnce(Invoke(
    [&]() {
      m_txn.HTTPTransaction::sendEOM();
    }));
  EXPECT_CALL(m_txn, sendAbort());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->onBody(folly::IOBuf::copyBuffer("I still have so much to say"));
  m_transport->onBody(folly::IOBuf::copyBuffer("I still have so much to say"));
}

TEST_F(ProxygenTransportBasicTest, invalid_expect) {
  auto req = getRequest(HTTPMethod::POST);
  auto length = folly::to<std::string>(RuntimeOption::MaxPostSize);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, length);
  req->getHeaders().add(HTTP_HEADER_EXPECT, "105-stop");
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(417)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, valid_expect) {
  auto req = getRequest(HTTPMethod::POST);
  auto length = folly::to<std::string>(RuntimeOption::MaxPostSize);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, length);
  req->getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(100)));
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, valid_expect_overlarge_length) {
  auto req = getRequest(HTTPMethod::POST);
  auto length = folly::to<std::string>(RuntimeOption::MaxPostSize + 1);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, length);
  req->getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(417)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, invalid_length) {
  auto req = getRequest(HTTPMethod::POST);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "blarf");
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(400)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, weird_length) {
  auto req = getRequest(HTTPMethod::POST);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "25, 200");
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(400)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, negative_length) {
  auto req = getRequest(HTTPMethod::POST);
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "-500");
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(400)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, overflowed_length) {
  auto req = getRequest(HTTPMethod::POST);
  std::string length(std::numeric_limits<long long>::digits10 + 1, '9');
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, length);
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(413)));
  EXPECT_CALL(m_txn, sendEOM());
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, no_length) {
  auto req = getRequest(HTTPMethod::POST);
  m_transport->onHeadersComplete(std::move(req));
}

TEST_F(ProxygenTransportBasicTest, overlarge_body) {
  auto req = getRequest(HTTPMethod::POST);
  EXPECT_CALL(m_server, onRequestError(_));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(413)))
  .WillOnce(Invoke(
    [&](const HTTPMessage& m) {
      m_txn.HTTPTransaction::sendHeaders(m);
    }));
  EXPECT_CALL(m_txn, sendEOM())
  .WillOnce(Invoke(
    [&]() {
      m_txn.HTTPTransaction::sendEOM();
    }));
  EXPECT_CALL(m_txn, sendAbort());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->setMaxPost(10, 5);
  m_transport->onBody(folly::IOBuf::copyBuffer("More than 10 bytes"));
  // Give it multiple bodies since realistically this can happen.
  m_transport->onBody(folly::IOBuf::copyBuffer("Even more than 10 bytes"));
}

TEST_F(ProxygenTransportTest, push) {
  // Push a resource
  Array promiseHeaders;
  Array responseHeaders;
  uint8_t pri = 1;

  promiseHeaders.append("hello: world"); // vec serialization path
  responseHeaders.append("foo: bar");
  auto id = pushResource(promiseHeaders, responseHeaders, pri);

  // And some body bytes
  std::string body("12345");
  m_transport->pushResourceBody(id, body.data(), body.length(), false);
  EXPECT_EQ(m_worker.m_messageQueue.size(), 3);

  // Creates a new transaction and sends headers/body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(2), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  EXPECT_CALL(pushTxn, sendBody(_));
  m_worker.deliverMessages();

  // Send Push EOM
  m_transport->pushResourceBody(id, nullptr, 0, true);
  EXPECT_EQ(m_worker.m_messageQueue.size(), 1);
  EXPECT_CALL(pushTxn, sendEOM());
  m_worker.deliverMessages();
  pushHandler->detachTransaction();
  // Send response
  sendResponse("12345");
}

TEST_F(ProxygenTransportTest, push_empty_body) {
  // Push a resource
  Array promiseHeaders;
  Array responseHeaders;
  uint8_t pri = 1;

  promiseHeaders.set(String("hello"),
                     String("world"));  // dict serializtion path
  responseHeaders.set(String("foo"), String("bar"));  // dict serializtion path
  pushResource(promiseHeaders, responseHeaders, pri, true /* eom, no body */);

  // Creates a new transaction and sends headers and an empty body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(2), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  EXPECT_CALL(pushTxn, sendEOM());
  m_worker.deliverMessages();

  pushHandler->detachTransaction();
  // Send response
  sendResponse("12345");
}

TEST_F(ProxygenTransportTest, push_abort_incomplete) {
  // Push a resource
  Array promiseHeaders;
  Array responseHeaders;
  uint8_t pri = 1;

  promiseHeaders.set(String("hello"),
                     String("world"));  // dict serializtion path
  responseHeaders.set(String("foo"), String("bar"));  // dict serializtion path
  pushResource(promiseHeaders, responseHeaders, pri);

  // Creates a new transaction and sends headers, but not body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(2), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  m_worker.deliverMessages();
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
  Array promiseHeaders;
  Array responseHeaders;
  uint8_t pri = 1;

  promiseHeaders.set(String("hello"),
                     String("world"));  // dict serializtion path
  responseHeaders.set(String("foo"), String("bar"));  // dict serializtion path
  auto id = pushResource(promiseHeaders, responseHeaders, pri);

  // Creates a new transaction and sends headers, but not body
  MockHTTPTransaction pushTxn(TransportDirection::DOWNSTREAM,
                              HTTPCodec::StreamID(2), 1, m_egressQueue,
                              WheelTimerInstance(m_timeouts.get()));
  HTTPPushTransactionHandler* pushHandler = nullptr;
  expectPushPromiseAndHeaders(pushTxn, pri, &pushHandler);
  m_worker.deliverMessages();

  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   "Stream aborted, streamID");
  ex.setProxygenError(proxygen::kErrorStreamAbort);
  ex.setCodecStatusCode(proxygen::ErrorCode::CANCEL);
  pushTxn.onError(ex);
  pushHandler->detachTransaction();
  m_transport->pushResourceBody(id, nullptr, 0, true);
  m_worker.deliverMessages();
  sendResponse("12345");
}

TEST_F(ProxygenTransportTest, client_timeout) {
  // Verify a Http 408 response would be returned on client request timeout
  HTTPException ex(HTTPException::Direction::INGRESS,
      folly::to<std::string>("ingress timeout, requestId=test"));
  ex.setProxygenError(proxygen::kErrorTimeout);
  ex.setCodecStatusCode(proxygen::ErrorCode::CANCEL);

  // Setting up the expectation that canSendHeaders returns true is required
  // due to the fact that most of the methods that control internal state
  // within the MockHTTPTransaction are mocked and thus its internal state is
  // invalid
  EXPECT_CALL(m_txn, canSendHeaders()).WillOnce(Return(true));
  EXPECT_CALL(m_txn, sendHeaders(IsResponseStatusCode(408)));
  EXPECT_CALL(m_txn, sendEOM());
  EXPECT_CALL(m_server, onRequestError(_));
  m_transport->onError(ex);
}

TEST_F(ProxygenTransportTest, client_timeout_incomplete_reply) {
  // Verify the connection is aborted in case where there is a client timeout
  // but we are in a mid reply
  HTTPException ex(HTTPException::Direction::INGRESS,
      folly::to<std::string>("ingress timeout, requestId=test"));
  ex.setProxygenError(proxygen::kErrorTimeout);
  ex.setCodecStatusCode(proxygen::ErrorCode::CANCEL);

  // Setting up the expectation that canSendHeaders returns false is required
  // due to the fact that most of the methods that control internal state
  // within the MockHTTPTransaction are mocked and thus its internal state is
  // invalid
  EXPECT_CALL(m_txn, canSendHeaders()).WillOnce(Return(false));
  EXPECT_CALL(m_txn, sendAbort());
  m_transport->onError(ex);
}

TEST_F(ProxygenTransportRepostTest, no_body) {
  InSequence enforceOrder;
  auto req = getRequest(HTTPMethod::POST);

  EXPECT_CALL(m_txn, canSendHeaders());
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

  EXPECT_CALL(m_txn, canSendHeaders());
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

  EXPECT_CALL(m_txn, canSendHeaders());
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

  EXPECT_CALL(m_txn, canSendHeaders());
  EXPECT_CALL(m_txn, sendHeaders(_));
  EXPECT_CALL(m_txn, sendBody(_))
    .Times(2);
  EXPECT_CALL(m_txn, sendAbort());
  m_transport->onHeadersComplete(std::move(req));
  m_transport->onBody(std::move(body1));
  m_transport->beginPartialPostEcho();
  m_transport->onBody(std::move(body2));
  m_transport->abort();
}

}
