/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPTransactionAdaptorSource.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/test/Mocks.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <folly/coro/Baton.h>
#include <folly/coro/DetachOnCancel.h>

using namespace proxygen::coro;
using namespace testing;
using namespace proxygen;

namespace proxygen::coro::test {

namespace {
static std::chrono::milliseconds kTestDefaultTimeout =
    std::chrono::milliseconds(1000);
static folly::SocketAddress kTestAddress("127.0.0.1", 80);
static wangle::TransportInfo kTestTransportInfo;

// Source that delivers headers synchronously, then suspends on a baton
// before returning trailers. This simulates a real network source where
// trailers arrive asynchronously.
class AsyncTrailerSource : public HTTPSource {
 public:
  explicit AsyncTrailerSource(std::unique_ptr<HTTPMessage> msg)
      : msg_(std::move(msg)) {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    HTTPHeaderEvent event(std::move(msg_), false /* eom */);
    auto guard = folly::makeGuard(lifetime(event));
    co_return event;
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t) override {
    co_await baton_;
    auto trailers = std::make_unique<HTTPHeaders>();
    trailers->add("Test", "Success");
    HTTPBodyEvent event(std::move(trailers));
    auto guard = folly::makeGuard(lifetime(event));
    co_return event;
  }

  void stopReading(
      folly::Optional<const HTTPErrorCode> = folly::none) noexcept override {
    delete this;
  }

  folly::coro::Baton baton_;
  std::unique_ptr<HTTPMessage> msg_;
};

} // namespace

class HTTPTransactionAdaptorSourceTests : public HTTPClientTests {
 protected:
  void SetUp() override {
    HTTPClientTests::SetUp();
    mockTxn_ = createMockHTTPTransaction();
    adaptor_ = HTTPTransactionAdaptorSource::create(&evb_);
    getHandler()->setTransaction(mockTxn_.get());

    ON_CALL(ctx_, getPeerAddress()).WillByDefault(ReturnRef(kTestAddress));
    ON_CALL(ctx_, getSetupTransportInfo())
        .WillByDefault(ReturnRef(kTestTransportInfo));
  }

  std::unique_ptr<::testing::StrictMock<MockHTTPTransaction>>
  createMockHTTPTransaction(
      TransportDirection direction = TransportDirection::DOWNSTREAM) {
    auto& egressQueue = direction == TransportDirection::DOWNSTREAM
                            ? downstreamEgressQueue_
                            : upstreamEgressQueue_;

    auto mock = std::make_unique<::testing::StrictMock<MockHTTPTransaction>>(
        direction,
        /*streamId=*/1,
        /*seqNo=*/0,
        egressQueue,
        /*timer=*/wheelTimer_.get(),
        /*transactionTimeout=*/std::chrono::milliseconds(60000),
        /*stats=*/nullptr,
        /*useFlowControl=*/false,
        /*receiveInitialWindowSize=*/0,
        /*sendInitialWindowSize=*/0,
        http2::DefaultPriority,
        folly::none);

    ON_CALL(*mock, pauseIngress())
        .WillByDefault(::testing::Invoke([mockPtr = mock.get()] {
          mockPtr->HTTPTransaction::pauseIngress();
        }));
    ON_CALL(*mock, resumeIngress())
        .WillByDefault(::testing::Invoke([mockPtr = mock.get()] {
          mockPtr->HTTPTransaction::resumeIngress();
        }));
    EXPECT_CALL(*mock, resumeIngress()).Times(AnyNumber());

    return mock;
  }

  HTTPTransactionHandler* getHandler() {
    return static_cast<HTTPTransactionHandler*>(adaptor_);
  }

  struct SourceExpectations {
    std::optional<std::string> maybeMethod;
    std::optional<std::string> maybeBody;
    bool eom{true};
    bool error{false};
  };

  folly::coro::Task<void> consumeAndValidateSource(

      HTTPSourceHolder&& source, const SourceExpectations& expectations) {
    bool headersComplete = false;
    bool bodyReceived = false;
    bool eomComplete = false;
    bool error = false;

    HTTPSourceReader reader(std::move(source));
    reader
        .onHeaders([&](std::unique_ptr<HTTPMessage> msg, auto, bool eom) {
          headersComplete = true;
          if (msg && expectations.maybeMethod.has_value()) {
            EXPECT_EQ(msg->getHeaders().getSingleOrEmpty("x-method"),
                      expectations.maybeMethod.value());
          }
          eomComplete = eom;
          return HTTPSourceReader::Continue;
        })
        .onBody([&](BufQueue body, bool eom) {
          if (expectations.maybeBody.has_value()) {
            EXPECT_EQ(body.move()->moveToFbString().toStdString(),
                      expectations.maybeBody.value());
          }
          bodyReceived = true;
          eomComplete = eom;
          return HTTPSourceReader::Continue;
        })
        .onError([&](auto, auto) { error = true; });

    co_await reader.read();

    if (expectations.maybeMethod.has_value()) {
      CO_ASSERT_TRUE(headersComplete);
    }
    if (expectations.maybeBody.has_value()) {
      CO_ASSERT_TRUE(bodyReceived);
    }
    CO_ASSERT_EQ(eomComplete, expectations.eom);
  }

  void setTransactionExpectations(const SourceExpectations& expectations) {
    if (expectations.maybeMethod.has_value()) {
      EXPECT_CALL(*mockTxn_, sendHeadersWithOptionalEOM)
          .Times(1)
          .WillRepeatedly([expectations](const HTTPMessage& msg, auto) {
            EXPECT_EQ(msg.getHeaders().getSingleOrEmpty("x-method"),
                      expectations.maybeMethod.value());
          });
    } else {
      EXPECT_CALL(*mockTxn_, sendHeaders).Times(0);
    }

    if (expectations.maybeBody.has_value()) {
      EXPECT_CALL(*mockTxn_, sendBody)
          .Times(1)
          .WillOnce([expectations](auto body) {
            EXPECT_EQ(body->moveToFbString().toStdString(),
                      expectations.maybeBody.value());
          });
    } else {
      EXPECT_CALL(*mockTxn_, sendBody).Times(0);
    }

    if (expectations.eom) {
      EXPECT_CALL(*mockTxn_, sendEOM).Times(1);
    } else {
      EXPECT_CALL(*mockTxn_, sendEOM).Times(0);
    }

    if (expectations.error) {
      EXPECT_CALL(*mockTxn_, sendAbort(_)).Times(1);
    } else {
      EXPECT_CALL(*mockTxn_, sendAbort(_)).Times(0);
    }
  }

  folly::HHWheelTimer::UniquePtr wheelTimer_;
  HTTP2PriorityQueue downstreamEgressQueue_;
  HTTP2PriorityQueue upstreamEgressQueue_;
  std::unique_ptr<::testing::StrictMock<MockHTTPTransaction>> mockTxn_;
  MockHTTPSessionContext ctx_;
  HTTPTransactionAdaptorSource* adaptor_;
};

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, IngressFlow) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);
  co_await consumeAndValidateSource(std::move(response),
                                    SourceExpectations{
                                        .maybeMethod = "GET",
                                        .maybeBody = "hello world",
                                    });
  getHandler()->detachTransaction();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, IngressBackPressure) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));

  EXPECT_CALL(*mockTxn_, pauseIngress()).Times(1);
  std::string body(70000, 'a');
  getHandler()->onBody(folly::IOBuf::copyBuffer(body));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);
  EXPECT_CALL(*mockTxn_, resumeIngress()).Times(1);
  co_await consumeAndValidateSource(std::move(response),
                                    SourceExpectations{
                                        .maybeMethod = "GET",
                                        .maybeBody = std::move(body),
                                    });
  getHandler()->detachTransaction();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, IngressError) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  EXPECT_CALL(*mockTxn_, sendAbort(_)).Times(1);
  getHandler()->onError(
      HTTPException(HTTPException::Direction::INGRESS, "oops"));

  auto response = co_await co_awaitTry(folly::coro::detachOnCancel(
      handler->handleRequest(&evb_, ctx_.acquireKeepAlive(), ingressSource)));
  EXPECT_TRUE(response.hasException());
  getHandler()->detachTransaction();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, IngressEarlyTermination) {
  auto handler = std::make_shared<TestHandler>();
  auto ingressSource = adaptor_->getIngressSource();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  getHandler()->onEOM();

  auto readCoro =
      folly::coro::timeout(folly::coro::detachOnCancel(handler->handleRequest(
                               &evb_, ctx_.acquireKeepAlive(), ingressSource)),
                           kTestDefaultTimeout);

  getHandler()->detachTransaction();
  co_await std::move(readCoro);
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, EgressFlow) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);

  setTransactionExpectations(SourceExpectations{
      .maybeMethod = "GET",
      .maybeBody = "hello world",
  });

  ON_CALL(*mockTxn_, sendEOM).WillByDefault([&]() {
    getHandler()->detachTransaction();
  });
  adaptor_->setEgressSource(std::move(response));
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, EgressBackPressure) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);

  getHandler()->onEgressPaused();

  setTransactionExpectations(SourceExpectations{.eom = false});
  adaptor_->setEgressSource(std::move(response));

  setTransactionExpectations(SourceExpectations{
      .maybeMethod = "GET",
      .maybeBody = "hello world",
  });

  ON_CALL(*mockTxn_, sendEOM).WillByDefault([&]() {
    getHandler()->detachTransaction();
  });
  getHandler()->onEgressResumed();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, UpstreamEgressErrors) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("test/bodyError_11");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);

  setTransactionExpectations(
      SourceExpectations{.maybeMethod = "" /* response */,
                         .maybeBody = "super long",
                         .eom = false,
                         .error = true});
  adaptor_->setEgressSource(std::move(response));

  ON_CALL(*mockTxn_, sendAbort).WillByDefault([&]() {
    getHandler()->detachTransaction();
  });
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests,
            DownstreamEarlyEgressTermination) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);

  adaptor_->setEgressSource(std::move(response));

  // Terminate before egress loop is complete.
  getHandler()->detachTransaction();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests,
            EgressTrailersAfterDetachNoCrash) {
  // Reproduce the production crash: the egress loop suspends waiting for the
  // next body event (async source). While suspended, detachTransaction fires
  // — setting txn_ = nullptr and requesting cancellation. The source then
  // delivers trailers. reader.read() resumes and dispatches to the TRAILERS
  // switch case without re-checking the stop flag, so the onTrailers
  // callback runs with a null txn_. Without the cancellation guard in
  // onTrailers, this is a SIGSEGV.
  auto ingressSource = adaptor_->getIngressSource();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  // Buffer ingress — consumed later for lifecycle cleanup.
  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onEOM();

  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  auto* asyncSource = new AsyncTrailerSource(std::move(resp));
  auto* batonPtr = &asyncSource->baton_;

  folly::coro::Baton headersSent;
  EXPECT_CALL(*mockTxn_, sendHeadersWithOptionalEOM)
      .Times(1)
      .WillOnce([&](const HTTPMessage&, auto) { headersSent.post(); });
  EXPECT_CALL(*mockTxn_, sendTrailers).Times(0);
  EXPECT_CALL(*mockTxn_, sendEOM).Times(0);

  adaptor_->setEgressSource(HTTPSourceHolder(asyncSource));

  // Wait until egress headers are sent. The egress loop is now suspended
  // inside readBodyEvent at the baton.
  co_await headersSent;

  // Detach sets txn_ = nullptr and requests cancellation. IngressComplete
  // is not yet set, so the gate does not fire and the adaptor stays alive.
  getHandler()->detachTransaction();

  // Unblock readBodyEvent — it returns the TRAILERS event. The reader
  // dispatches to the onTrailers callback without rechecking the stop flag.
  batonPtr->post();

  // Yield so the egress loop continuation runs and processes trailers.
  co_await folly::coro::co_reschedule_on_current_executor;

  // Consume the buffered ingress to trigger IngressComplete and allow the
  // gate to fire for proper adaptor cleanup.
  HTTPSourceReader ingressReader;
  ingressReader.setSource(ingressSource)
      .onHeaders([](auto, auto, bool) { return HTTPSourceReader::Continue; })
      .onError([](auto, auto) {});
  co_await ingressReader.read();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, DownstreamErrors) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  getHandler()->onEOM();

  auto response = co_await folly::coro::timeout(
      folly::coro::detachOnCancel(handler->handleRequest(
          &evb_, ctx_.acquireKeepAlive(), ingressSource)),
      kTestDefaultTimeout);

  EXPECT_CALL(*mockTxn_, sendHeadersWithOptionalEOM)
      .Times(1)
      .WillRepeatedly([&](const HTTPMessage& /* msg */, auto) {
        getHandler()->onError(
            HTTPException(HTTPException::Direction::EGRESS, "oops"));
        // Delay the detach to the next loop.
        evb_.runInEventBaseThreadAlwaysEnqueue(
            [&]() { getHandler()->detachTransaction(); });
      });

  EXPECT_CALL(*mockTxn_, sendBody).Times(0);
  EXPECT_CALL(*mockTxn_, sendAbort(_)).Times(1);

  adaptor_->setEgressSource(std::move(response));
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests, IngressNoErrorAbortNoCrash) {
  auto ingressSource = adaptor_->getIngressSource();
  auto handler = std::make_shared<TestHandler>();

  auto msg = std::make_unique<HTTPMessage>();
  msg->setMethod(HTTPMethod::GET);
  msg->setURL("https://www.facebook.com/");

  getHandler()->onHeadersComplete(std::move(msg));
  getHandler()->onBody(folly::IOBuf::copyBuffer("hello world"));
  // Do not send EOM to simulate incomplete ingress

  // Simulate an inbound RST_STREAM(NO_ERROR) via codec status on the exception
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   "rst_stream_no_error");
  ex.setCodecStatusCode(ErrorCode::NO_ERROR);
  EXPECT_CALL(*mockTxn_, sendAbort(_)).Times(1);
  getHandler()->onError(ex);

  // The adaptor will abort the ingress source; consuming should surface an
  // error
  auto responseTry = co_await co_awaitTry(folly::coro::detachOnCancel(
      handler->handleRequest(&evb_, ctx_.acquireKeepAlive(), ingressSource)));
  EXPECT_TRUE(responseTry.hasException());

  getHandler()->detachTransaction();
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests,
            WebSocketPausesIngressAndResumesOnEgressHeaders) {
  auto ingressSource = adaptor_->getIngressSource();

  auto req = std::make_unique<HTTPMessage>();
  req->setURL("https://www.facebook.com/");
  req->setIngressWebsocketUpgrade();

  // pauseIngress() should be called on websocket upgrade request.
  EXPECT_CALL(*mockTxn_, pauseIngress()).Times(1);
  getHandler()->onHeadersComplete(std::move(req));

  auto headerEvent =
      co_await co_nothrow(HTTPSourceHolder(ingressSource).readHeaderEvent());
  EXPECT_FALSE(headerEvent.eom);
  EXPECT_TRUE(headerEvent.headers->isIngressWebsocketUpgrade());

  auto resp = std::make_unique<HTTPMessage>();
  auto egressSource = HTTPFixedSource::makeFixedSource(std::move(resp));

  // When the response comes back, resumeIngress() should be called.
  EXPECT_CALL(*mockTxn_, sendHeadersWithOptionalEOM).Times(1);
  EXPECT_CALL(*mockTxn_, resumeIngress()).Times(1);

  ON_CALL(*mockTxn_, resumeIngress()).WillByDefault([&] {
    mockTxn_->HTTPTransaction::resumeIngress();
    getHandler()->detachTransaction();
  });
  adaptor_->setEgressSource(egressSource);
}

CO_TEST_P_X(HTTPTransactionAdaptorSourceTests,
            WebSocketWithBodyDoesNotResumeIngressUntilWindowOpens) {
  auto ingressSource = adaptor_->getIngressSource();

  auto req = std::make_unique<HTTPMessage>();
  req->setURL("https://www.facebook.com/");
  req->setIngressWebsocketUpgrade();

  // pauseIngress() should be called twice:
  // 1. On websocket upgrade request
  // 2. When body fills the flow control window
  EXPECT_CALL(*mockTxn_, pauseIngress()).Times(2);
  getHandler()->onHeadersComplete(std::move(req));

  // Send a large body to fill the buffer which will close the window.
  std::string body(70000, 'a');
  getHandler()->onBody(folly::IOBuf::copyBuffer(body));
  getHandler()->onEOM();

  HTTPSourceHolder sourceHolder(ingressSource);
  auto headerEvent = co_await co_nothrow(sourceHolder.readHeaderEvent());
  EXPECT_FALSE(headerEvent.eom);
  EXPECT_TRUE(headerEvent.headers->isIngressWebsocketUpgrade());

  // resumeIngress() should only be called once, when the window opens again.
  EXPECT_CALL(*mockTxn_, resumeIngress()).Times(1);
  auto resp = std::make_unique<HTTPMessage>();
  auto egressSource = HTTPFixedSource::makeFixedSource(std::move(resp));

  // Used to track where resumeIngress() is called.
  bool resumeIngressCalled = false;
  folly::coro::Baton headersSentBaton;

  ON_CALL(*mockTxn_, resumeIngress()).WillByDefault([&] {
    resumeIngressCalled = true;
    mockTxn_->HTTPTransaction::resumeIngress();
  });
  EXPECT_CALL(*mockTxn_, sendHeadersWithOptionalEOM).Times(1).WillOnce([&] {
    headersSentBaton.post();
  });

  adaptor_->setEgressSource(egressSource);

  // Wait for egress headers to be sent.
  co_await headersSentBaton;

  EXPECT_FALSE(resumeIngressCalled)
      << "resumeIngress should not be called on egress headers when window is "
         "closed";

  // Now read the body from ingress source. This drains the buffer and
  // triggers windowOpen callback, which should call resumeIngress().
  auto bodyEvent = co_await co_nothrow(sourceHolder.readBodyEvent());

  // Verify resumeIngress WAS called after body was read (window opened)
  EXPECT_TRUE(resumeIngressCalled)
      << "resumeIngress should be called when window opens after body drain";

  getHandler()->detachTransaction();
}

INSTANTIATE_TEST_SUITE_P(HTTPTransactionAdaptorSourceTests,
                         HTTPTransactionAdaptorSourceTests,
                         Values(TransportType::TCP),
                         transportTypeToTestName);

} // namespace proxygen::coro::test
