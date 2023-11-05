/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <vector>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/futures/Promise.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/TimeoutManager.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/DirectErrorsRateLimitFilter.h>
#include <proxygen/lib/http/codec/HTTPCodecFactory.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/observer/HTTPSessionObserverInterface.h>
#include <proxygen/lib/http/session/HTTPDirectResponseHandler.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPSessionTest.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockByteEventTracker.h>
#include <proxygen/lib/http/session/test/MockHTTPSessionStats.h>
#include <proxygen/lib/http/session/test/MockSessionObserver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <proxygen/lib/test/TestAsyncTransport.h>
#include <wangle/acceptor/ConnectionManager.h>

using namespace proxygen;
using namespace std;
using namespace testing;
using namespace std::chrono;
using folly::Promise;

template <typename C>
class HTTPDownstreamTest : public testing::Test {
 public:
  explicit HTTPDownstreamTest(std::vector<int64_t> flowControl = {-1, -1, -1},
                              bool startImmediately = true)
      : eventBase_(),
        transport_(new TestAsyncTransport(&eventBase_)),
        transactionTimeouts_(makeTimeoutSet(&eventBase_)),
        flowControl_(flowControl) {
    EXPECT_CALL(mockController_, getGracefulShutdownTimeout())
        .WillRepeatedly(Return(std::chrono::milliseconds(0)));
    EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
        .WillRepeatedly(Return(HeaderIndexingStrategy::getDefaultInstance()));

    {
      InSequence s;
      EXPECT_CALL(mockController_, attachSession(_));
      EXPECT_CALL(mockController_, onTransportReady(_));
    }

    HTTPSession::setDefaultReadBufferLimit(65536);
    HTTPTransaction::setEgressBufferLimit(65536);
    auto codec = makeServerCodec<typename C::Codec>(C::version);
    rawCodec_ = codec.get();

    // If the codec is H2, getHeaderIndexingStrategy will be called when setting
    // up the codec
    if (rawCodec_->getProtocol() == CodecProtocol::HTTP_2) {
      EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
          .WillOnce(Return(&testH2IndexingStrat_));
    }

    httpSession_ = new HTTPDownstreamSession(
        transactionTimeouts_.get(),
        std::move(folly::AsyncTransport::UniquePtr(transport_)),
        localAddr,
        peerAddr,
        &mockController_,
        std::move(codec),
        mockTransportInfo /* no stats for now */,
        &infoCb_);
    for (auto& param : flowControl) {
      if (param < 0) {
        param = rawCodec_->getDefaultWindowSize();
      }
    }

    // Ensure the H2 header indexing strategy was setup correctly if applicable
    if (rawCodec_->getProtocol() == CodecProtocol::HTTP_2) {
      HTTP2Codec* recastedCodec = dynamic_cast<HTTP2Codec*>(rawCodec_);
      EXPECT_EQ(recastedCodec->getHeaderIndexingStrategy(),
                &testH2IndexingStrat_);
    }

    httpSession_->setFlowControl(
        flowControl[0], flowControl[1], flowControl[2]);
    httpSession_->setEgressSettings({{SettingsId::MAX_CONCURRENT_STREAMS, 200},
                                     {SettingsId::HEADER_TABLE_SIZE, 5555},
                                     {SettingsId::ENABLE_PUSH, 1},
                                     {SettingsId::ENABLE_EX_HEADERS, 1}});
    if (startImmediately) {
      httpSession_->startNow();
    }
    clientCodec_ = makeClientCodec<typename C::Codec>(C::version);
    if (clientCodec_->getProtocol() == CodecProtocol::HTTP_2) {
      clientCodec_->getEgressSettings()->setSetting(
          SettingsId::ENABLE_EX_HEADERS, 1);
    }
    clientCodec_->generateConnectionPreface(requests_);
    clientCodec_->generateSettings(requests_);
    clientCodec_->setCallback(&callbacks_);
  }

  HTTPCodec::StreamID sendRequest(const std::string& url = "/",
                                  int8_t priority = 0,
                                  bool eom = true) {
    auto req = getGetRequest();
    req.setURL(url);
    req.setPriority(priority);
    return sendRequest(req, eom);
  }

  HTTPCodec::StreamID sendRequest(const HTTPMessage& req, bool eom = true) {
    auto streamID = clientCodec_->createStream();
    clientCodec_->generateHeader(requests_, streamID, req, eom);
    return streamID;
  }

  HTTPCodec::StreamID sendHeader() {
    return sendRequest("/", 0, false);
  }

  Promise<folly::Unit> sendRequestLater(HTTPMessage req, bool eof = false) {
    Promise<folly::Unit> reqp;
    reqp.getSemiFuture().via(&eventBase_).thenValue([=](auto&&) {
      sendRequest(req);
      transport_->addReadEvent(requests_, milliseconds(0));
      if (eof) {
        transport_->addReadEOF(milliseconds(0));
      }
    });
    return reqp;
  }

  void SetUp() override {
    folly::EventBaseManager::get()->clearEventBase();
    HTTPSession::setDefaultWriteBufferLimit(65536);
    HTTP2PriorityQueue::setNodeLifetime(std::chrono::milliseconds(2));
    EXPECT_CALL(infoCb_, onTransactionAttached(_)).WillRepeatedly([this]() {
      onTransactionSymmetricCounter++;
    });
    EXPECT_CALL(infoCb_, onTransactionDetached(_)).WillRepeatedly([this]() {
      onTransactionSymmetricCounter--;
    });
  }

  void TearDown() override {
    EXPECT_EQ(onTransactionSymmetricCounter, 0);
  }

  void cleanup() {
    EXPECT_CALL(mockController_, detachSession(_));
    httpSession_->dropConnection();
  }

  std::unique_ptr<testing::StrictMock<MockHTTPHandler>>
  addSimpleStrictHandler() {
    std::unique_ptr<testing::StrictMock<MockHTTPHandler>> handler =
        std::make_unique<testing::StrictMock<MockHTTPHandler>>();

    // The ownership model here is suspect, but assume the callers won't destroy
    // handler before it's requested
    auto rawHandler = handler.get();
    EXPECT_CALL(mockController_, getRequestHandler(testing::_, testing::_))
        .WillOnce(testing::Return(rawHandler))
        .RetiresOnSaturation();

    EXPECT_CALL(*handler, _setTransaction(testing::_))
        .WillOnce(testing::SaveArg<0>(&handler->txn_));

    return handler;
  }

  std::unique_ptr<testing::NiceMock<MockHTTPHandler>> addSimpleNiceHandler() {
    std::unique_ptr<testing::NiceMock<MockHTTPHandler>> handler =
        std::make_unique<testing::NiceMock<MockHTTPHandler>>();

    // See comment above
    auto rawHandler = handler.get();
    EXPECT_CALL(mockController_, getRequestHandler(testing::_, testing::_))
        .WillOnce(testing::Return(rawHandler))
        .RetiresOnSaturation();

    EXPECT_CALL(*handler, _setTransaction(testing::_))
        .WillOnce(testing::SaveArg<0>(&handler->txn_))
        .RetiresOnSaturation();

    return handler;
  }

  void onEOMTerminateHandlerExpectShutdown(MockHTTPHandler& handler) {
    handler.expectEOM([&] { handler.terminate(); });
    handler.expectDetachTransaction();
    expectDetachSession();
  }

  void expectDetachSession() {
    EXPECT_CALL(mockController_, detachSession(testing::_));
  }

  void addSingleByteReads(const char* data, milliseconds delay = {}) {
    for (const char* p = data; *p != '\0'; ++p) {
      transport_->addReadEvent(p, 1, delay);
    }
  }

  void flushRequestsAndLoop(
      bool eof = false,
      milliseconds eofDelay = milliseconds(0),
      milliseconds initialDelay = milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>()) {
    flushRequests(eof, eofDelay, initialDelay, extraEventsFn);
    eventBase_.loop();
  }

  void flushRequestsAndLoopN(
      uint64_t n,
      bool eof = false,
      milliseconds eofDelay = milliseconds(0),
      milliseconds initialDelay = milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>()) {
    flushRequests(eof, eofDelay, initialDelay, extraEventsFn);
    for (uint64_t i = 0; i < n; i++) {
      eventBase_.loopOnce();
    }
  }

  void flushRequests(
      bool eof = false,
      milliseconds eofDelay = milliseconds(0),
      milliseconds initialDelay = milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>()) {
    transport_->addReadEvent(requests_, initialDelay);
    requests_.move();
    if (extraEventsFn) {
      extraEventsFn();
    }
    if (eof) {
      transport_->addReadEOF(eofDelay);
    }
    transport_->startReadEvents();
  }

  void testSimpleUpgrade(const std::string& upgradeHeader,
                         CodecProtocol expectedProtocol,
                         const std::string& expectedUpgradeHeader);

  void gracefulShutdown() {
    folly::DelayedDestruction::DestructorGuard g(httpSession_);
    clientCodec_->generateGoaway(this->requests_, 0, ErrorCode::NO_ERROR);
    expectDetachSession();
    flushRequestsAndLoop(true);
  }

  void testPriorities(uint32_t numPriorities);

  void testChunks(bool trailers);

  void expect101(CodecProtocol expectedProtocol,
                 const std::string& expectedUpgrade,
                 bool expect100 = false) {
    NiceMock<MockHTTPCodecCallback> callbacks;

    EXPECT_CALL(callbacks, onMessageBegin(_, _));
    EXPECT_CALL(callbacks, onNativeProtocolUpgrade(_, _, _, _))
        .WillOnce(Invoke([this, expectedUpgrade](HTTPCodec::StreamID,
                                                 CodecProtocol,
                                                 const std::string&,
                                                 HTTPMessage& msg) {
          EXPECT_EQ(msg.getStatusCode(), 101);
          EXPECT_EQ(msg.getStatusMessage(), "Switching Protocols");
          EXPECT_EQ(msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_UPGRADE),
                    expectedUpgrade);
          // also connection and date
          EXPECT_EQ(msg.getHeaders().size(), 3);
          breakParseOutput_ = true;
          return true;
        }));
    // this comes before 101, but due to gmock this is backwards
    if (expect100) {
      EXPECT_CALL(callbacks, onMessageBegin(_, _)).RetiresOnSaturation();
      EXPECT_CALL(callbacks, onHeadersComplete(_, _))
          .WillOnce(
              Invoke([](HTTPCodec::StreamID, std::shared_ptr<HTTPMessage> msg) {
                LOG(INFO) << "100 headers";
                EXPECT_EQ(msg->getStatusCode(), 100);
              }))
          .RetiresOnSaturation();
    }
    clientCodec_->setCallback(&callbacks);
    parseOutput(*clientCodec_);
    clientCodec_ = HTTPCodecFactory::getCodec(expectedProtocol,
                                              TransportDirection::UPSTREAM);
    clientCodec_->createStream();
    clientCodec_->setCallback(&callbacks_);
  }
  void expectResponse(uint32_t code, bool expectBody, bool stopParsing = true) {
    expectResponse(
        code, ErrorCode::NO_ERROR, false, false, expectBody, stopParsing);
  }

  void expectResponse(uint32_t code = 200,
                      ErrorCode errorCode = ErrorCode::NO_ERROR,
                      bool expect100 = false,
                      bool expectGoaway = false,
                      bool expectBody = true,
                      bool stopParsing = false) {
    expectResponses(
        1, code, errorCode, expect100, expectGoaway, expectBody, stopParsing);
  }
  void expectResponses(uint32_t n,
                       uint32_t code = 200,
                       ErrorCode errorCode = ErrorCode::NO_ERROR,
                       bool expect100 = false,
                       bool expectGoaway = false,
                       bool expectBody = true,
                       bool stopParsing = false) {
    clientCodec_->setCallback(&callbacks_);
    if (isParallelCodecProtocol(clientCodec_->getProtocol())) {
      EXPECT_CALL(callbacks_, onSettings(_))
          .WillOnce(Invoke([this](const SettingsList& settings) {
            if (flowControl_[0] > 0) {
              bool foundInitialWindow = false;
              for (const auto& setting : settings) {
                if (setting.id == SettingsId::INITIAL_WINDOW_SIZE) {
                  EXPECT_EQ(flowControl_[0], setting.value);
                  foundInitialWindow = true;
                }
              }
              EXPECT_TRUE(foundInitialWindow);
            }
          }));
    }
    if (flowControl_[2] > 0) {
      int64_t sessionDelta =
          flowControl_[2] - clientCodec_->getDefaultWindowSize();
      if (clientCodec_->supportsSessionFlowControl() && sessionDelta) {
        EXPECT_CALL(callbacks_, onWindowUpdate(0, sessionDelta));
      }
    }
    if (flowControl_[1] > 0) {
      size_t initWindow = flowControl_[0] > 0
                              ? flowControl_[0]
                              : clientCodec_->getDefaultWindowSize();
      int64_t streamDelta = flowControl_[1] - initWindow;
      if (clientCodec_->supportsStreamFlowControl() && streamDelta) {
        EXPECT_CALL(callbacks_, onWindowUpdate(1, streamDelta));
      }
    }

    if (expectGoaway) {
      EXPECT_CALL(callbacks_,
                  onGoaway(HTTPCodec::StreamID(1), ErrorCode::NO_ERROR, _));
    }

    for (uint32_t i = 0; i < n; i++) {
      uint8_t times = (expect100) ? 2 : 1;
      EXPECT_CALL(callbacks_, onMessageBegin(_, _))
          .Times(times)
          .RetiresOnSaturation();
      EXPECT_CALL(callbacks_, onHeadersComplete(_, _))
          .WillOnce(Invoke(
              [code](HTTPCodec::StreamID, std::shared_ptr<HTTPMessage> msg) {
                EXPECT_EQ(msg->getStatusCode(), code);
              }));
      if (expect100) {
        EXPECT_CALL(callbacks_, onHeadersComplete(_, _))
            .WillOnce(Invoke(
                [](HTTPCodec::StreamID, std::shared_ptr<HTTPMessage> msg) {
                  EXPECT_EQ(msg->getStatusCode(), 100);
                }))
            .RetiresOnSaturation();
      }
      if (errorCode != ErrorCode::NO_ERROR) {
        EXPECT_CALL(callbacks_, onAbort(_, _))
            .WillOnce(Invoke([errorCode](HTTPCodec::StreamID, ErrorCode error) {
              EXPECT_EQ(error, errorCode);
            }));
      }
      if (expectBody) {
        EXPECT_CALL(callbacks_, onBody(_, _, _)).RetiresOnSaturation();
      }
      EXPECT_CALL(callbacks_, onMessageComplete(_, _))
          .WillOnce(InvokeWithoutArgs([i, n, stopParsing, this] {
            if (stopParsing && i == n - 1) {
              clientCodec_->setParserPaused(true);
              breakParseOutput_ = true;
            }
          }))
          .RetiresOnSaturation();
    }
    parseOutput(*clientCodec_);
  }

  void parseOutput(HTTPCodec& clientCodec) {
    auto writeEvents = transport_->getWriteEvents();
    clientCodec.setParserPaused(false);
    breakParseOutput_ = false;
    while (!breakParseOutput_ &&
           (!writeEvents->empty() || !parseOutputStream_.empty())) {
      if (!writeEvents->empty()) {
        auto event = writeEvents->front();
        auto vec = event->getIoVec();
        for (size_t i = 0; i < event->getCount(); i++) {
          parseOutputStream_.append(
              folly::IOBuf::copyBuffer(vec[i].iov_base, vec[i].iov_len));
        }
        writeEvents->pop_front();
      }
      uint32_t consumed = clientCodec.onIngress(*parseOutputStream_.front());
      ASSERT_TRUE(consumed > 0 || !writeEvents->empty());
      parseOutputStream_.split(consumed);
    }
    if (!breakParseOutput_) {
      EXPECT_EQ(parseOutputStream_.chainLength(), 0);
    }
    breakParseOutput_ = false;
  }

  void resumeWritesInLoop() {
    eventBase_.runInLoop([this] { transport_->resumeWrites(); });
  }

  void resumeWritesAfterDelay(milliseconds delay) {
    eventBase_.runAfterDelay([this] { transport_->resumeWrites(); },
                             delay.count());
  }

  MockByteEventTracker* setMockByteEventTracker() {
    auto byteEventTracker = new NiceMock<MockByteEventTracker>(nullptr);
    httpSession_->setByteEventTracker(
        std::unique_ptr<ByteEventTracker>(byteEventTracker));
    EXPECT_CALL(*byteEventTracker, preSend(_, _, _, _))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*byteEventTracker, drainByteEvents()).WillRepeatedly(Return(0));
    EXPECT_CALL(*byteEventTracker, processByteEvents(_, _))
        .WillRepeatedly(Invoke([](std::shared_ptr<ByteEventTracker> self,
                                  uint64_t bytesWritten) {
          return self->ByteEventTracker::processByteEvents(self, bytesWritten);
        }));

    return byteEventTracker;
  }

  std::unique_ptr<MockSessionObserver> addMockSessionObserver(
      MockSessionObserver::EventSet eventSet) {
    auto observer = std::make_unique<NiceMock<MockSessionObserver>>(eventSet);
    EXPECT_CALL(*observer, attached(_));
    httpSession_->addObserver(observer.get());
    return observer;
  }

  std::shared_ptr<MockSessionObserver> addMockSessionObserverShared(
      MockSessionObserver::EventSet eventSet) {
    auto observer = std::make_shared<NiceMock<MockSessionObserver>>(eventSet);
    EXPECT_CALL(*observer, attached(_));
    httpSession_->addObserver(observer);
    return observer;
  }

 protected:
  folly::EventBase eventBase_;
  TestAsyncTransport* transport_; // invalid once httpSession_ is destroyed
  folly::HHWheelTimer::UniquePtr transactionTimeouts_;
  std::vector<int64_t> flowControl_;
  StrictMock<MockController> mockController_;
  HTTPDownstreamSession* httpSession_;
  folly::IOBufQueue requests_{folly::IOBufQueue::cacheChainLength()};
  unique_ptr<HTTPCodec> clientCodec_;
  NiceMock<MockHTTPCodecCallback> callbacks_;
  folly::IOBufQueue parseOutputStream_{folly::IOBufQueue::cacheChainLength()};
  bool breakParseOutput_{false};
  typename C::Codec* rawCodec_{nullptr};
  HeaderIndexingStrategy testH2IndexingStrat_;
  testing::NiceMock<proxygen::MockHTTPSessionInfoCallback> infoCb_;
  uint64_t onTransactionSymmetricCounter{0};
};

// Uses TestAsyncTransport
using HTTPDownstreamSessionTest = HTTPDownstreamTest<HTTP1xCodecPair>;
namespace {
class HTTP2DownstreamSessionTest : public HTTPDownstreamTest<HTTP2CodecPair> {
 public:
  HTTP2DownstreamSessionTest() : HTTPDownstreamTest<HTTP2CodecPair>() {
  }

  void SetUp() override {
    HTTPDownstreamTest<HTTP2CodecPair>::SetUp();
  }

  void SetupControlStream(HTTPCodec::StreamID cStreamId) {
    // enable EX_HEADERS
    clientCodec_->getEgressSettings()->setSetting(SettingsId::ENABLE_EX_HEADERS,
                                                  1);
    clientCodec_->generateSettings(requests_);
    // create a control stream
    clientCodec_->generateHeader(
        requests_, cStreamId, getGetRequest("/cc"), true, nullptr);
  }

  void TearDown() override {
  }
};
} // namespace

namespace {
class HTTP2DownstreamSessionEarlyShutdownTest
    : public HTTPDownstreamTest<HTTP2CodecPair> {
 public:
  HTTP2DownstreamSessionEarlyShutdownTest()
      : HTTPDownstreamTest<HTTP2CodecPair>({-1, -1, -1}, false) {
  }

  void SetUp() override {
    HTTPDownstreamTest<HTTP2CodecPair>::SetUp();
  }

  void TearDown() override {
  }
};
} // namespace

TEST_F(HTTP2DownstreamSessionEarlyShutdownTest, EarlyShutdown) {
  folly::DelayedDestruction::DestructorGuard g(httpSession_);

  // Try shutting down the session and then starting it. This should be properly
  // handled by the HTTPSession such that no HTTP/2 frames are sent in the
  // wrong order.
  StrictMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);
  EXPECT_CALL(callbacks, onFrameHeader(_, _, _, _, _)).Times(2);
  EXPECT_CALL(callbacks, onSettings(_)).Times(1);
  EXPECT_CALL(callbacks, onGoaway(_, _, _)).Times(1);
  expectDetachSession();
  httpSession_->notifyPendingShutdown();
  httpSession_->startNow();
  eventBase_.loop();
  parseOutput(*clientCodec_);
}

TEST_F(HTTP2DownstreamSessionEarlyShutdownTest, EarlyShutdownDoubleGoaway) {
  folly::DelayedDestruction::DestructorGuard g(httpSession_);
  httpSession_->enableDoubleGoawayDrain();

  StrictMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);
  EXPECT_CALL(callbacks, onFrameHeader(_, _, _, _, _)).Times(3);
  EXPECT_CALL(callbacks, onSettings(_)).Times(1);
  EXPECT_CALL(callbacks, onGoaway(_, _, _)).Times(2);
  expectDetachSession();
  httpSession_->notifyPendingShutdown();
  httpSession_->startNow();
  eventBase_.loop();
  parseOutput(*clientCodec_);
}

TEST_F(HTTP2DownstreamSessionTest, ShutdownDoubleGoaway) {
  folly::DelayedDestruction::DestructorGuard g(httpSession_);
  httpSession_->enableDoubleGoawayDrain();

  StrictMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);
  EXPECT_CALL(callbacks, onFrameHeader(_, _, _, _, _)).Times(3);
  EXPECT_CALL(callbacks, onSettings(_)).Times(1);
  EXPECT_CALL(callbacks, onGoaway(_, _, _)).Times(2);
  expectDetachSession();
  httpSession_->notifyPendingShutdown();
  eventBase_.loop();
  parseOutput(*clientCodec_);
}

TEST_F(HTTPDownstreamSessionTest, ImmediateEof) {
  // Send EOF without any request data
  EXPECT_CALL(mockController_, getRequestHandler(_, _)).Times(0);
  expectDetachSession();

  flushRequestsAndLoop(true, milliseconds(0));
}

TEST_F(HTTPDownstreamSessionTest, IdleTimeoutWithOpenStreamGraceful) {
  // Send a request and simulate idle timeout.  Then send response.  Session
  // will include Connection: close and terminate.
  auto id = sendRequest(getGetRequest());
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();

  // Simulate a read timeout, then send response in next loop
  eventBase_.runAfterDelay(
      [&] {
        httpSession_->timeoutExpired();
        eventBase_.runInLoop(
            [&handler] { handler->sendReplyWithBody(200, 100); });
      },
      transactionTimeouts_->getDefaultTimeout().count());
  handler->expectDetachTransaction();
  expectDetachSession();

  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop(true, milliseconds(0));
  NiceMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);

  EXPECT_CALL(callbacks, onHeadersComplete(id, _))
      .WillOnce(Invoke([](HTTPCodec::StreamID,
                          std::shared_ptr<HTTPMessage> msg) {
        EXPECT_EQ(msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONNECTION),
                  "close");
      }));
  parseOutput(*clientCodec_);
}

TEST_F(HTTPDownstreamSessionTest, IdleTimeoutNoStreams) {
  httpSession_->timeoutExpired();
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, IdleTimeoutWithOpenStreamUngraceful) {
  // Send a request, send response header, then simulate idle timeout.
  // and send the remaining response.  Session will not include
  // Connection: close and terminate.
  sendRequest(getGetRequest());
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] { handler->sendHeaders(200, 100); });
  handler->expectEOM();

  // Simulate a read timeout, then send response in next loop
  eventBase_.runAfterDelay(
      [&] {
        httpSession_->timeoutExpired();
        eventBase_.runInLoop([&handler] {
          handler->sendBody(100);
          handler->txn_->sendEOM();
        });
      },
      transactionTimeouts_->getDefaultTimeout().count());
  handler->expectDetachTransaction();
  expectDetachSession();

  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop(true, milliseconds(0));
  NiceMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);

  EXPECT_CALL(callbacks, onHeadersComplete(1, _))
      .WillOnce(Invoke([](HTTPCodec::StreamID,
                          std::shared_ptr<HTTPMessage> msg) {
        EXPECT_EQ(msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONNECTION),
                  "keep-alive");
      }));
  parseOutput(*clientCodec_);
}

TEST_F(HTTPDownstreamSessionTest, Http10NoHeaders) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/", msg->getURL());
    EXPECT_EQ("/", msg->getPathAsStringPiece());
    EXPECT_EQ("", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(0, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  auto req = getGetRequest();
  req.setHTTPVersion(1, 0);
  sendRequest(req);
  flushRequestsAndLoop();
}

TEST_F(HTTPDownstreamSessionTest, Http10NoHeadersEof) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("http://example.com/foo?bar", msg->getURL());
    EXPECT_EQ("/foo", msg->getPathAsStringPiece());
    EXPECT_EQ("bar", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(0, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  const char* req = "GET http://example.com/foo?bar HTTP/1.0\r\n\r\n";
  requests_.append(req, strlen(req));
  flushRequestsAndLoop(true, milliseconds(0));
}

TEST_F(HTTPDownstreamSessionTest, SingleBytes) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(2, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("connection"));

    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/somepath.php?param=foo", msg->getURL());
    EXPECT_EQ("/somepath.php", msg->getPathAsStringPiece());
    EXPECT_EQ("param=foo", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  addSingleByteReads(
      "GET /somepath.php?param=foo HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n"
      "\r\n");
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, SingleBytesWithBody) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(3, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("content-length"));
    EXPECT_TRUE(hdrs.exists("myheader"));

    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/somepath.php?param=foo", msg->getURL());
    EXPECT_EQ("/somepath.php", msg->getPathAsStringPiece());
    EXPECT_EQ("param=foo", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("1"))
      .WillOnce(ExpectString("2"))
      .WillOnce(ExpectString("3"))
      .WillOnce(ExpectString("4"))
      .WillOnce(ExpectString("5"));
  onEOMTerminateHandlerExpectShutdown(*handler);

  addSingleByteReads(
      "POST /somepath.php?param=foo HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "MyHeader: FooBar\r\n"
      "Content-Length: 5\r\n"
      "\r\n"
      "12345");
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, SplitBody) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(2, hdrs.size());
  });
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("12345"))
      .WillOnce(ExpectString("abcde"));
  onEOMTerminateHandlerExpectShutdown(*handler);

  transport_->addReadEvent(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "\r\n"
      "12345",
      milliseconds(0));
  transport_->addReadEvent("abcde", milliseconds(5));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, MovableBuffer) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(2, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("connection"));

    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/somepath.php?param=foo", msg->getURL());
    EXPECT_EQ("/somepath.php", msg->getPathAsStringPiece());
    EXPECT_EQ("param=foo", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  transport_->addMovableReadEvent(
      folly::IOBuf::copyBuffer("GET /somepath.php?param=foo HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Connection: close\r\n"
                               "\r\n"));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, MovableBufferChained) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(2, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("connection"));

    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/somepath.php?param=foo", msg->getURL());
    EXPECT_EQ("/somepath.php", msg->getPathAsStringPiece());
    EXPECT_EQ("param=foo", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  auto buf = folly::IOBuf::copyBuffer(
      "GET /somepath.php?param=foo HTTP/1.1\r\n"
      "Host: example.com\r\n");
  buf->prependChain(
      folly::IOBuf::copyBuffer("Connection: close\r\n"
                               "\r\n"));
  transport_->addMovableReadEvent(std::move(buf));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, MovableBufferMultiple) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(2, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("connection"));

    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/somepath.php?param=foo", msg->getURL());
    EXPECT_EQ("/somepath.php", msg->getPathAsStringPiece());
    EXPECT_EQ("param=foo", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  transport_->addMovableReadEvent(
      folly::IOBuf::copyBuffer("GET /somepath.php?param=foo HTTP/1.1\r\n"
                               "Host: example.com\r\n"));
  transport_->addMovableReadEvent(
      folly::IOBuf::copyBuffer("Connection: close\r\n"
                               "\r\n"));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, MovableBufferChainedEmptyBuffer) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(2, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("connection"));

    EXPECT_FALSE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("/somepath.php?param=foo", msg->getURL());
    EXPECT_EQ("/somepath.php", msg->getPathAsStringPiece());
    EXPECT_EQ("param=foo", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  onEOMTerminateHandlerExpectShutdown(*handler);

  auto buf = folly::IOBuf::copyBuffer(
      "GET /somepath.php?param=foo HTTP/1.1\r\n"
      "Host: example.com\r\n");
  buf->prependChain(folly::IOBuf::create(0));
  buf->prependChain(
      folly::IOBuf::copyBuffer("Connection: close\r\n"
                               "\r\n"));
  transport_->addMovableReadEvent(std::move(buf));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, PostChunked) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&](std::shared_ptr<HTTPMessage> msg) {
    const HTTPHeaders& hdrs = msg->getHeaders();
    EXPECT_EQ(3, hdrs.size());
    EXPECT_TRUE(hdrs.exists("host"));
    EXPECT_TRUE(hdrs.exists("content-type"));
    EXPECT_TRUE(hdrs.exists("transfer-encoding"));
    EXPECT_TRUE(msg->getIsChunked());
    EXPECT_FALSE(msg->getIsUpgraded());
    EXPECT_EQ("http://example.com/cgi-bin/foo.aspx?abc&def", msg->getURL());
    EXPECT_EQ("/cgi-bin/foo.aspx", msg->getPathAsStringPiece());
    EXPECT_EQ("abc&def", msg->getQueryStringAsStringPiece());
    EXPECT_EQ(1, msg->getHTTPVersion().first);
    EXPECT_EQ(1, msg->getHTTPVersion().second);
  });
  EXPECT_CALL(*handler, _onChunkHeader(3));
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _)).WillOnce(ExpectString("bar"));
  EXPECT_CALL(*handler, _onChunkComplete());
  EXPECT_CALL(*handler, _onChunkHeader(0x22));
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("0123456789abcdef\nfedcba9876543210\n"));
  EXPECT_CALL(*handler, _onChunkComplete());
  EXPECT_CALL(*handler, _onChunkHeader(3));
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _)).WillOnce(ExpectString("foo"));
  EXPECT_CALL(*handler, _onChunkComplete());
  onEOMTerminateHandlerExpectShutdown(*handler);

  transport_->addReadEvent(
      "POST http://example.com/cgi-bin/foo.aspx?abc&def "
      "HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Type: text/pla",
      milliseconds(0));
  transport_->addReadEvent(
      "in; charset=utf-8\r\n"
      "Transfer-encoding: chunked\r\n"
      "\r",
      milliseconds(2));
  transport_->addReadEvent(
      "\n"
      "3\r\n"
      "bar\r\n"
      "22\r\n"
      "0123456789abcdef\n"
      "fedcba9876543210\n"
      "\r\n"
      "3\r",
      milliseconds(3));
  transport_->addReadEvent(
      "\n"
      "foo\r\n"
      "0\r\n\r\n",
      milliseconds(1));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, MultiMessage) {
  InSequence enforceOrder;

  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders();
  EXPECT_CALL(*handler1, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("foo"))
      .WillOnce(ExpectString("bar9876"));
  handler1->expectEOM([&] { handler1->sendReply(); });
  handler1->expectDetachTransaction();

  auto handler2 = addSimpleNiceHandler();
  handler2->expectHeaders();
  EXPECT_CALL(*handler2, _onChunkHeader(0xa));
  EXPECT_CALL(*handler2, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("some "))
      .WillOnce(ExpectString("data\n"));
  EXPECT_CALL(*handler2, _onChunkComplete());
  onEOMTerminateHandlerExpectShutdown(*handler2);

  transport_->addReadEvent(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "\r\n"
      "foo",
      milliseconds(0));
  transport_->addReadEvent(
      "bar9876"
      "POST /foo HTTP/1.1\r\n"
      "Host: exa",
      milliseconds(20));
  transport_->addReadEvent(
      "mple.com\r\n"
      "Connection: close\r\n"
      "Trans",
      milliseconds(0));
  transport_->addReadEvent(
      "fer-encoding: chunked\r\n"
      "\r\n",
      milliseconds(20));
  transport_->addReadEvent("a\r\nsome ", milliseconds(0));
  transport_->addReadEvent("data\n\r\n0\r\n\r\n", milliseconds(20));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, ClientPipelined) {
  InSequence enforceOrder;

  std::vector<std::unique_ptr<testing::NiceMock<MockHTTPHandler>>> handlers;
  for (auto i = 0; i < 4; i++) {
    auto handler = addSimpleNiceHandler();
    handler->expectHeaders([i](std::shared_ptr<HTTPMessage> req) {
      EXPECT_EQ(req->getHeaders().getSingleOrEmpty("Id"),
                folly::to<std::string>(i + 1));
    });
    handler->expectEOM([h = handler.get()] { h->sendReplyWithBody(200, 100); });
    handler->expectDetachTransaction();
    handlers.push_back(std::move(handler));
  }

  transport_->addReadEvent(
      "GET /echo HTTP/1.1\r\n"
      "Host: jojo\r\n"
      "Id: 1\r\n"
      "\r\n"
      "GET /echo HTTP/1.1\r\n"
      "Host: jojo\r\n"
      "Id: 2\r\n"
      "\r\n"
      "GET /echo HTTP/1.1\r\n"
      "Host: jojo\r\n"
      "Id: 3\r\n"
      "\r\n"
      "GET /echo HTTP/1.1\r\n"
      "Host: jojo\r\n"
      "Id: 4\r\n"
      "\r\n",
      milliseconds(0));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, BadContentLength) {
  InSequence enforceOrder;

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders();
  handler->expectBody([](uint64_t, std::shared_ptr<folly::IOBuf> body) {
    EXPECT_EQ(body->computeChainDataLength(), 6);
  });
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();

  // Test sending more bytes than advertised in Content-Length.  The proxy will
  // ignore these since Connection: close was also specified.
  //
  // One could argue it would be better to 400 this kind of request.
  auto req = getGetRequest();
  req.setHTTPVersion(1, 0);
  req.setWantsKeepalive(false);
  req.getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, "6");
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  expectDetachSession();
  flushRequestsAndLoop();
}

TEST_F(HTTPDownstreamSessionTest, Connect) {
  InSequence enforceOrder;

  auto handler = addSimpleStrictHandler();
  // Send HTTP 200 OK to accept the CONNECT request
  handler->expectHeaders([&handler] { handler->sendHeaders(200, 100); });

  EXPECT_CALL(*handler, _onUpgrade(_));

  // Data should be received using onBody
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("12345"))
      .WillOnce(ExpectString("abcde"));
  onEOMTerminateHandlerExpectShutdown(*handler);

  transport_->addReadEvent(
      "CONNECT test HTTP/1.1\r\n"
      "\r\n"
      "12345",
      milliseconds(0));
  transport_->addReadEvent("abcde", milliseconds(5));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, ConnectRejected) {
  InSequence enforceOrder;

  auto handler = addSimpleStrictHandler();
  // Send HTTP 400 to reject the CONNECT request
  handler->expectHeaders([&handler] { handler->sendReplyCode(400); });

  onEOMTerminateHandlerExpectShutdown(*handler);

  transport_->addReadEvent(
      "CONNECT test HTTP/1.1\r\n"
      "\r\n"
      "12345",
      milliseconds(0));
  transport_->addReadEvent("abcde", milliseconds(5));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, HttpUpgrade) {
  InSequence enforceOrder;

  auto handler = addSimpleStrictHandler();
  // Send HTTP 101 Switching Protocls to accept the upgrade request
  handler->expectHeaders([&handler] { handler->sendHeaders(101, 100); });

  // Send the response in the new protocol after upgrade
  EXPECT_CALL(*handler, _onUpgrade(_))
      .WillOnce(Invoke([&handler](UpgradeProtocol /*protocol*/) {
        handler->sendReplyCode(100);
      }));

  onEOMTerminateHandlerExpectShutdown(*handler);

  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_UPGRADE, "TEST/1.0");
  req.getHeaders().add(HTTP_HEADER_CONNECTION, "upgrade");
  sendRequest(req);
  flushRequestsAndLoop(true, milliseconds(0));
}

TEST(HTTPDownstreamTest, ParseErrorNoTxn) {
  // 1) Get a parse error on SYN_STREAM for streamID == 1
  // 2) Expect that the codec should be asked to generate an abort on
  //    streamID==1
  folly::EventBase evb;

  // Setup the controller and its expecations.
  NiceMock<MockController> mockController;

  // Setup the codec, its callbacks, and its expectations.
  auto codec = makeDownstreamParallelCodec();
  HTTPCodec::Callback* codecCallback = nullptr;
  EXPECT_CALL(*codec, setCallback(_))
      .WillRepeatedly(SaveArg<0>(&codecCallback));
  // Expect egress abort for streamID == 1
  EXPECT_CALL(*codec, generateRstStream(_, 1, _));

  // Setup transport
  bool transportGood = true;
  auto transport = newMockTransport(&evb);
  EXPECT_CALL(*transport, good()).WillRepeatedly(ReturnPointee(&transportGood));
  EXPECT_CALL(*transport, closeNow())
      .WillRepeatedly(Assign(&transportGood, false));
  EXPECT_CALL(*transport, writeChain(_, _, _))
      .WillRepeatedly(
          Invoke([&](folly::AsyncTransport::WriteCallback* callback,
                     const shared_ptr<folly::IOBuf>&,
                     folly::WriteFlags) { callback->writeSuccess(); }));

  // Create the downstream session, thus initializing codecCallback
  auto transactionTimeouts = makeInternalTimeoutSet(&evb);
  auto session =
      new HTTPDownstreamSession(transactionTimeouts.get(),
                                folly::AsyncTransport::UniquePtr(transport),
                                localAddr,
                                peerAddr,
                                &mockController,
                                std::move(codec),
                                mockTransportInfo,
                                nullptr);
  session->startNow();
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, "foo");
  ex.setProxygenError(kErrorParseHeader);
  ex.setCodecStatusCode(ErrorCode::REFUSED_STREAM);
  codecCallback->onError(HTTPCodec::StreamID(1), ex, true);

  // cleanup
  session->dropConnection();
  evb.loop();
}

TEST(HTTPDownstreamTest, ByteEventsDrained) {
  // Test that byte events are drained before socket is closed
  folly::EventBase evb;

  NiceMock<MockController> mockController;
  auto codec = makeDownstreamParallelCodec();
  auto byteEventTracker = new MockByteEventTracker(nullptr);
  auto transport = newMockTransport(&evb);
  auto transactionTimeouts = makeInternalTimeoutSet(&evb);

  // Create the downstream session
  auto session =
      new HTTPDownstreamSession(transactionTimeouts.get(),
                                folly::AsyncTransport::UniquePtr(transport),
                                localAddr,
                                peerAddr,
                                &mockController,
                                std::move(codec),
                                mockTransportInfo,
                                nullptr);
  session->setByteEventTracker(
      std::unique_ptr<ByteEventTracker>(byteEventTracker));

  InSequence enforceOrder;

  session->startNow();

  // Byte events should be drained first
  EXPECT_CALL(*byteEventTracker, drainByteEvents()).Times(1);
  EXPECT_CALL(*transport, closeNow()).Times(AtLeast(1));

  // Close the socket
  session->dropConnection();
  evb.loop();
}

TEST_F(HTTPDownstreamSessionTest, HttpWithAckTiming) {
  // This is to test cases where holding a byte event to a finished HTTP/1.1
  // transaction does not masquerade as HTTP pipelining.
  auto byteEventTracker = setMockByteEventTracker();
  InSequence enforceOrder;

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1]() {
    handler1->sendChunkedReplyWithBody(200, 100, 100, false);
  });
  // Hold a pending byte event
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _))
      .WillOnce(Invoke(
          [](HTTPTransaction* txn, uint64_t /*byteNo*/, ByteEvent::Callback) {
            txn->incrementPendingByteEvents();
          }));
  sendRequest();
  flushRequestsAndLoop();
  expectResponse();

  // Send the secode request after receiving the first response (eg: clearly
  // not pipelined)
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&handler2]() {
    handler2->sendChunkedReplyWithBody(200, 100, 100, false);
  });
  // This txn processed and destroyed before txn1
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _));
  handler2->expectDetachTransaction();

  sendRequest();
  flushRequestsAndLoop();
  expectResponse();

  // Now clear the pending byte event (simulate ack) and the first txn
  // goes away too
  handler1->expectDetachTransaction();
  handler1->txn_->decrementPendingByteEvents();
  gracefulShutdown();
}

TEST_F(HTTPDownstreamSessionTest, TestOnContentMismatch) {
  // Test the behavior when the reported content-length on the header
  // is different from the actual length of the body.
  // The expectation is simply to log the behavior, such as:
  // ".. HTTPTransaction.cpp ] Content-Length/body mismatch: expected: .. "

  NiceMock<MockHTTPSessionStats> stats;
  httpSession_->setSessionStats(&stats);
  EXPECT_CALL(stats, _recordEgressContentLengthMismatches()).Times(2);

  folly::EventBase base;
  InSequence enforceOrder;
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1]() {
    // over-estimate the content-length on the header
    handler1->sendHeaders(200, 105);
    handler1->sendBody(100);
    handler1->txn_->sendEOM();
  });
  sendRequest();
  flushRequestsAndLoop();

  auto handler2 = addSimpleNiceHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&handler2]() {
    // under-estimate the content-length on the header
    handler2->sendHeaders(200, 95);
    handler2->sendBody(100);
    handler2->txn_->sendEOM();
  });
  sendRequest();
  flushRequestsAndLoop();
  gracefulShutdown();
}

TEST_F(HTTPDownstreamSessionTest, HttpWithAckTimingPipeline) {
  // Test a real pipelining case as well.  First request is done waiting for
  // ack, then receive two pipelined requests.
  auto byteEventTracker = setMockByteEventTracker();
  InSequence enforceOrder;

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1]() {
    handler1->sendChunkedReplyWithBody(200, 100, 100, false);
  });
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _))
      .WillOnce(Invoke(
          [](HTTPTransaction* txn, uint64_t /*byteNo*/, ByteEvent::Callback) {
            txn->incrementPendingByteEvents();
          }));
  sendRequest();
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&handler2]() {
    handler2->sendChunkedReplyWithBody(200, 100, 100, false);
  });
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _));
  handler2->expectDetachTransaction();

  sendRequest();
  sendRequest();
  auto handler3 = addSimpleStrictHandler();
  handler3->expectHeaders();
  handler3->expectEOM([&handler3]() {
    handler3->sendChunkedReplyWithBody(200, 100, 100, false);
  });
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _));
  handler3->expectDetachTransaction();
  flushRequestsAndLoop();
  expectResponses(3);
  handler1->expectDetachTransaction();
  handler1->txn_->decrementPendingByteEvents();
  gracefulShutdown();
}

TEST_F(HTTPDownstreamSessionTest, HttpWithAckTimingPipelineError) {
  HTTPDirectResponseHandler* errorHandler =
      new HTTPDirectResponseHandler(400, "Bad Request");
  EXPECT_CALL(mockController_, getParseErrorHandler(_, _, _))
      .WillOnce(Return(errorHandler));

  // Test a real pipelining case as well.  First request is done waiting for
  // ack, then receive a pipelined request and an error.
  auto byteEventTracker = setMockByteEventTracker();
  InSequence enforceOrder;

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1]() { handler1->sendReplyWithBody(200, 100); });
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _))
      .WillOnce(Invoke(
          [](HTTPTransaction* txn, uint64_t /*byteNo*/, ByteEvent::Callback) {
            txn->incrementPendingByteEvents();
          }));
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM();

  // send first request
  sendRequest();
  // send a second request too, response for request 1 will arrive
  sendRequest();
  flushRequestsAndLoopN(2);
  expectResponse(200);

  // send a garbage character which will trigger a 400
  transport_->addReadEvent("?", milliseconds(0));
  flushRequestsAndLoop();

  // When the byte event is cleared, txn1 will go away
  handler1->expectDetachTransaction();
  handler1->txn_->decrementPendingByteEvents();
  flushRequestsAndLoop();

  // Now send a reply to 2.  Reads will be resumed and we'll get a 400 for the
  // garbage character
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _));
  handler2->expectDetachTransaction();
  handler2->sendReplyWithBody(200, 100);
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop();
  expectResponse(200, true);
  expectResponse(400, false, false);
  expectDetachSession();
}

TEST_F(HTTPDownstreamSessionTest, HttpWithAckTimingConnError) {
  // Send a request, response waits on a byte event
  // Then send an error.  The session should close when the byte event completes
  HTTPDirectResponseHandler* errorHandler =
      new HTTPDirectResponseHandler(400, "Bad Request");
  EXPECT_CALL(mockController_, getParseErrorHandler(_, _, _))
      .WillOnce(Return(errorHandler));

  auto byteEventTracker = setMockByteEventTracker();
  InSequence enforceOrder;

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1]() { handler1->sendReplyWithBody(200, 100); });
  EXPECT_CALL(*byteEventTracker, addLastByteEvent(_, _, _))
      .WillOnce(Invoke(
          [](HTTPTransaction* txn, uint64_t /*byteNo*/, ByteEvent::Callback) {
            txn->incrementPendingByteEvents();
          }));

  // send first request
  sendRequest();
  flushRequestsAndLoopN(2);
  expectResponse(200);

  // send a garbage character which will trigger a 400
  transport_->addReadEvent("?", milliseconds(0));
  flushRequestsAndLoop();

  expectResponse(400, false, false);
  // When the byte event is cleared, txn1 will go away
  handler1->expectDetachTransaction();
  // Add a 1 MB read - it shouldn't be buffered in the session
  transport_->addMovableReadEvent(makeBuf(1000000), milliseconds(0));
  eventBase_.loop();
  expectDetachSession();
  handler1->txn_->decrementPendingByteEvents();
}

TEST_F(HTTP2DownstreamSessionTest, TestPing) {
  // send a request with a PING, should get the PING first
  auto handler = addSimpleStrictHandler();
  sendRequest();
  auto pingData = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
  clientCodec_->generatePingRequest(requests_, pingData);
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectGoaway();
  flushRequestsAndLoopN(1);
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);
  gracefulShutdown();

  NiceMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);

  InSequence enforceOrder;
  EXPECT_CALL(callbacks, onPingReply(pingData));
  EXPECT_CALL(callbacks, onHeadersComplete(_, _));
  parseOutput(*clientCodec_);
}

TEST_F(HTTP2DownstreamSessionTest, TestPingWithPreSendSplit) {
  auto byteEventTracker = new NiceMock<MockByteEventTracker>(nullptr);
  EXPECT_CALL(*byteEventTracker, drainByteEvents()).WillRepeatedly(Return(0));
  EXPECT_CALL(*byteEventTracker, processByteEvents(_, _))
      .WillRepeatedly(Invoke([](std::shared_ptr<ByteEventTracker> self,
                                uint64_t bytesWritten) {
        return self->ByteEventTracker::processByteEvents(self, bytesWritten);
      }));

  // send a request with a PING, should get the PING first
  auto pingData = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
  auto handler = addSimpleStrictHandler();
  sendRequest();
  handler->expectHeaders();
  handler->expectEOM([this, &handler, byteEventTracker, pingData] {
    // set the new tracker now, so we don't get invoked when sending SETTINGS
    httpSession_->setByteEventTracker(
        std::unique_ptr<ByteEventTracker>(byteEventTracker));
    // Pause writes so only the first write goes through and the remainder
    // gets buffered in writeBuf_
    transport_->pauseWrites();
    handler->sendReplyWithBody(200, 100);
    eventBase_.runInLoop([this, pingData] {
      folly::IOBufQueue pingBuf{folly::IOBufQueue::cacheChainLength()};
      clientCodec_->generatePingRequest(pingBuf, pingData);
      transport_->addReadEvent(pingBuf, milliseconds(0));
      transport_->resumeWrites();
    });
  });
  // Split the write buffer on a non-frame boundary the first time
  EXPECT_CALL(*byteEventTracker, preSend(_, _, _, _))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(2);
  HTTPSession::DestructorGuard g(httpSession_);
  gracefulShutdown();

  NiceMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);

  InSequence enforceOrder;
  EXPECT_CALL(callbacks, onHeadersComplete(_, _));
  EXPECT_CALL(callbacks, onPingReply(pingData));
  parseOutput(*clientCodec_);
}

/*
 * The sequence of streams are generated in the following order:
 * - [client --> server] regular request 1st stream (getGetRequest())
 * - [server --> client] respond 1st stream (res, 100 bytes, without EOM)
 * - [server --> client] request 2nd stream (pub, 200 bytes, EOM)
 * - [client --> server] respond 2nd stream (OK, EOM)
 * - [client --> server] EOM on the 1st stream
 */
TEST_F(HTTP2DownstreamSessionTest, ExheaderFromServer) {
  auto cStreamId = clientCodec_->createStream();
  SetupControlStream(cStreamId);

  // Create a dummy request and a dummy response messages
  auto pub = getGetRequest("/sub/fyi");
  // set up the priority for fun
  pub.setHTTP2Priority(std::make_tuple(0, false, 7));

  InSequence handlerSequence;
  auto cHandler = addSimpleStrictHandler();
  StrictMock<MockHTTPHandler> pubHandler;

  cHandler->expectHeaders([&] {
    cHandler->txn_->pauseIngress();
    // Generate response for the control stream
    cHandler->txn_->sendHeaders(getResponse(200, 0));
    cHandler->txn_->sendBody(makeBuf(100));

    auto* pubTxn = cHandler->txn_->newExTransaction(&pubHandler);
    // Generate a pub request (encapsulated in EX_HEADERS frame)
    pubTxn->sendHeaders(pub);
    pubTxn->sendBody(makeBuf(200));
    pubTxn->sendEOM();
  });

  EXPECT_CALL(pubHandler, _setTransaction(_));
  EXPECT_CALL(callbacks_, onSettings(_)).WillOnce(InvokeWithoutArgs([&] {
    clientCodec_->generateSettingsAck(requests_);
  }));
  EXPECT_CALL(callbacks_, onMessageBegin(cStreamId, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(cStreamId, _));
  EXPECT_CALL(callbacks_, onExMessageBegin(2, _, _, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(2, _));
  EXPECT_CALL(callbacks_, onMessageComplete(2, _));

  EXPECT_CALL(pubHandler, _onHeadersComplete(_));
  EXPECT_CALL(pubHandler, _onEOM());
  EXPECT_CALL(pubHandler, _detachTransaction());

  EXPECT_CALL(*cHandler, _onEOM());
  EXPECT_CALL(*cHandler, _detachTransaction());

  transport_->addReadEvent(requests_, milliseconds(0));
  transport_->startReadEvents();

  eventBase_.runAfterDelay(
      [&] {
        parseOutput(*clientCodec_);
        // send a response from client to server
        clientCodec_->generateExHeader(
            requests_,
            2,
            getResponse(200, 0),
            HTTPCodec::ExAttributes(cStreamId, false),
            true,
            nullptr);
        transport_->addReadEvent(requests_, milliseconds(0));
        transport_->startReadEvents();
        parseOutput(*clientCodec_);
        cHandler->txn_->resumeIngress();
        cHandler->txn_->sendEOM();
        transport_->addReadEOF(milliseconds(0));
      },
      100);

  HTTPSession::DestructorGuard g(httpSession_);
  expectDetachSession();
  eventBase_.loop();
}

/*
 * The sequence of streams are generated in the following order:
 * - [client --> server] regular request on control stream 1
 * - [client --> server] Pub request on stream 3
 * - [server --> client] response on stream 1 (OK, )
 * - [server --> client] response on stream 3 (OK, EOM)
 * - [server --> client] response on stream 1 (EOM)
 */
TEST_F(HTTP2DownstreamSessionTest, ExheaderFromClient) {
  auto cStreamId = clientCodec_->createStream();
  SetupControlStream(cStreamId);

  // generate an EX_HEADERS
  auto exStreamId = clientCodec_->createStream();
  clientCodec_->generateExHeader(requests_,
                                 exStreamId,
                                 getGetRequest("/pub"),
                                 HTTPCodec::ExAttributes(cStreamId, false),
                                 true,
                                 nullptr);

  auto cHandler = addSimpleStrictHandler();
  cHandler->expectHeaders([&] {
    // send back the response for control stream, but EOM
    cHandler->txn_->sendHeaders(getResponse(200, 0));
  });
  EXPECT_CALL(*cHandler, _onEOM());

  StrictMock<MockHTTPHandler> pubHandler;
  EXPECT_CALL(*cHandler, _onExTransaction(_))
      .WillOnce(Invoke([&pubHandler](HTTPTransaction* exTxn) {
        exTxn->setHandler(&pubHandler);
        pubHandler.txn_ = exTxn;
      }));

  InSequence handlerSequence;
  EXPECT_CALL(pubHandler, _setTransaction(_));
  pubHandler.expectHeaders([&] {
    // send back the response for the pub request
    pubHandler.txn_->sendHeadersWithEOM(getResponse(200, 0));
  });
  EXPECT_CALL(pubHandler, _onEOM());
  EXPECT_CALL(pubHandler, _detachTransaction());
  cHandler->expectDetachTransaction();

  EXPECT_CALL(callbacks_, onMessageBegin(cStreamId, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(cStreamId, _));
  EXPECT_CALL(callbacks_, onExMessageBegin(exStreamId, _, _, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(exStreamId, _));
  EXPECT_CALL(callbacks_, onMessageComplete(exStreamId, _));
  EXPECT_CALL(callbacks_, onMessageComplete(cStreamId, _));

  transport_->addReadEvent(requests_, milliseconds(0));
  transport_->startReadEvents();
  transport_->addReadEOF(milliseconds(0));
  eventBase_.loop();

  HTTPSession::DestructorGuard g(httpSession_);
  expectDetachSession();
  cHandler->txn_->sendEOM();
  eventBase_.loop();
  parseOutput(*clientCodec_);
}

/*
 * The sequence of streams are generated in the following order:
 * - [client --> server] regular request 1st stream (getGetRequest())
 * - [server --> client] request 2nd stream (unidirectional)
 * - [server --> client] response + EOM on the 1st stream
 */
TEST_F(HTTP2DownstreamSessionTest, UnidirectionalExTransaction) {
  auto cStreamId = clientCodec_->createStream();
  SetupControlStream(cStreamId);
  InSequence handlerSequence;
  auto cHandler = addSimpleStrictHandler();
  StrictMock<MockHTTPHandler> uniHandler;

  cHandler->expectHeaders([&] {
    auto* uniTxn = cHandler->txn_->newExTransaction(&uniHandler, true);
    EXPECT_TRUE(uniTxn->isIngressComplete());
    uniTxn->sendHeaders(getGetRequest("/uni"));
    uniTxn->sendEOM();

    // close control stream
    cHandler->txn_->sendHeadersWithEOM(getResponse(200, 0));
  });

  EXPECT_CALL(uniHandler, _setTransaction(_));
  EXPECT_CALL(*cHandler, _onEOM());
  EXPECT_CALL(uniHandler, _detachTransaction());
  EXPECT_CALL(*cHandler, _detachTransaction());

  transport_->addReadEvent(requests_, milliseconds(0));
  transport_->startReadEvents();
  eventBase_.runAfterDelay([&] { transport_->addReadEOF(milliseconds(0)); },
                           100);

  HTTPSession::DestructorGuard g(httpSession_);
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, PauseResumeControlStream) {
  auto cStreamId = clientCodec_->createStream();
  SetupControlStream(cStreamId);

  // generate an EX_HEADERS
  auto exStreamId = clientCodec_->createStream();
  clientCodec_->generateExHeader(requests_,
                                 exStreamId,
                                 getGetRequest(),
                                 HTTPCodec::ExAttributes(cStreamId, false),
                                 true,
                                 nullptr);

  auto cHandler = addSimpleStrictHandler();
  cHandler->expectHeaders([&] {
    cHandler->txn_->pauseIngress();
    // send back the response for control stream, but EOM
    cHandler->txn_->sendHeaders(getResponse(200, 0));
  });
  EXPECT_CALL(*cHandler, _onEOM());

  StrictMock<MockHTTPHandler> pubHandler;
  EXPECT_CALL(*cHandler, _onExTransaction(_))
      .WillOnce(Invoke([&pubHandler](HTTPTransaction* exTxn) {
        exTxn->setHandler(&pubHandler);
        pubHandler.txn_ = exTxn;
      }));

  InSequence handlerSequence;
  EXPECT_CALL(pubHandler, _setTransaction(_));
  pubHandler.expectHeaders([&] {
    // send back the response for the pub request
    pubHandler.txn_->sendHeadersWithEOM(getResponse(200, 0));
  });
  EXPECT_CALL(pubHandler, _onEOM());
  EXPECT_CALL(pubHandler, _detachTransaction());
  cHandler->expectDetachTransaction();

  EXPECT_CALL(callbacks_, onMessageBegin(cStreamId, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(cStreamId, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(exStreamId, _));
  EXPECT_CALL(callbacks_, onMessageComplete(exStreamId, _));
  EXPECT_CALL(callbacks_, onMessageComplete(cStreamId, _));

  HTTPSession::DestructorGuard g(httpSession_);
  transport_->addReadEvent(requests_, milliseconds(0));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();

  cHandler->txn_->resumeIngress();
  cHandler->txn_->sendEOM();
  eventBase_.loop();

  expectDetachSession();
  parseOutput(*clientCodec_);
}

TEST_F(HTTP2DownstreamSessionTest, InvalidControlStream) {
  auto cStreamId = clientCodec_->createStream();
  SetupControlStream(cStreamId);

  // generate an EX_HEADERS, but with a non-existing control stream
  auto exStreamId = clientCodec_->createStream();
  clientCodec_->generateExHeader(requests_,
                                 exStreamId,
                                 getGetRequest(),
                                 HTTPCodec::ExAttributes(cStreamId + 4, false),
                                 true,
                                 nullptr);

  auto cHandler = addSimpleStrictHandler();
  InSequence handlerSequence;
  cHandler->expectHeaders([&] {
    // send back the response for control stream, but EOM
    cHandler->txn_->sendHeaders(getResponse(200, 0));
  });
  EXPECT_CALL(*cHandler, _onExTransaction(_)).Times(0);
  EXPECT_CALL(*cHandler, _onEOM());
  cHandler->expectDetachTransaction();

  EXPECT_CALL(callbacks_, onMessageBegin(cStreamId, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(cStreamId, _));
  EXPECT_CALL(callbacks_, onAbort(exStreamId, _));

  HTTPSession::DestructorGuard g(httpSession_);
  transport_->addReadEvent(requests_, milliseconds(0));
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();

  cHandler->txn_->sendEOM();
  eventBase_.loop();

  expectDetachSession();
  parseOutput(*clientCodec_);
}

TEST_F(HTTP2DownstreamSessionTest, SetByteEventTracker) {
  // Send two requests with writes paused, which will queue several byte events,
  // including last byte events which are holding a reference to the
  // transaction.
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1]() { handler1->sendReplyWithBody(200, 100); });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&handler2, this]() {
    handler2->sendReplyWithBody(200, 100);
    transport_->pauseWrites();
  });

  sendRequest();
  sendRequest();
  // Resume writes from the loop callback
  eventBase_.runInLoop([this] { transport_->resumeWrites(); });

  // Graceful shutdown will notify of GOAWAY
  EXPECT_CALL(*handler1, _onGoaway(ErrorCode::NO_ERROR));
  EXPECT_CALL(*handler2, _onGoaway(ErrorCode::NO_ERROR));
  // The original byteEventTracker will process the last byte event of the
  // first transaction, and detach by deleting the event.  Swap out the tracker.
  handler1->expectDetachTransaction([this] {
    auto tracker = std::make_unique<ByteEventTracker>(httpSession_);
    httpSession_->setByteEventTracker(std::move(tracker));
  });
  // handler2 should also be detached immediately because the new
  // ByteEventTracker continues procesing where the old one left off.
  handler2->expectDetachTransaction();
  gracefulShutdown();
}

TEST_F(HTTPDownstreamSessionTest, TestTrackedByteEventTracker) {
  auto byteEventTracker = setMockByteEventTracker();
  InSequence enforceOrder;

  auto handler1 = addSimpleStrictHandler();
  size_t bytesToSend = 200;
  size_t expectedTrackedByteOffset = bytesToSend + 99;
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, &bytesToSend]() {
    handler1->sendHeaders(200, 200);
    handler1->sendBodyWithLastByteFlushedTracking(bytesToSend);
    handler1->txn_->sendEOM();
  });

  EXPECT_CALL(*byteEventTracker,
              addTrackedByteEvent(_, expectedTrackedByteOffset, _))
      .WillOnce(Invoke(
          [](HTTPTransaction* txn, uint64_t /*byteNo*/, ByteEvent::Callback) {
            txn->incrementPendingByteEvents();
          }));
  sendRequest();
  flushRequestsAndLoop();
  handler1->expectDetachTransaction();
  handler1->txn_->decrementPendingByteEvents();
  gracefulShutdown();
}

struct TestOnTxnByteEventWrittenToBufParams {
  uint64_t byteOffset{0};
  ByteEvent::EventType eventType;
  bool timestampTx{false};
  bool timestampAck{false};
};

class OnTxnByteEventWrittenToBufTest
    : public HTTPDownstreamSessionTest
    , public ::testing::WithParamInterface<
          TestOnTxnByteEventWrittenToBufParams> {
 public:
  static std::vector<TestOnTxnByteEventWrittenToBufParams> getTestingValues() {
    std::vector<TestOnTxnByteEventWrittenToBufParams> vals;
    for (const auto& byteOffset : {0, 1, 2}) {
      for (const auto& eventType : {ByteEvent::EventType::FIRST_BYTE,
                                    ByteEvent::EventType::LAST_BYTE,
                                    ByteEvent::EventType::TRACKED_BYTE}) {
        for (const auto& timestampTx : {true, false}) {
          for (const auto& timestampAck : {true, false}) {
            TestOnTxnByteEventWrittenToBufParams params;
            params.byteOffset = byteOffset;
            params.eventType = eventType;
            params.timestampTx = timestampTx;
            params.timestampAck = timestampAck;
            vals.push_back(params);
          }
        }
      }
    }
    return vals;
  }
};

INSTANTIATE_TEST_SUITE_P(
    HTTPDownstreamSessionTest,
    OnTxnByteEventWrittenToBufTest,
    ::testing::ValuesIn(OnTxnByteEventWrittenToBufTest::getTestingValues()));

TEST_P(OnTxnByteEventWrittenToBufTest, TestOnTxnByteEventWrittenToBuf) {
  HTTP2PriorityQueue txnEgressQueue;
  NiceMock<MockHTTPTransactionTransport> transport;
  HTTPTransaction txn{TransportDirection::DOWNSTREAM,
                      HTTPCodec::StreamID(1),
                      1,
                      transport,
                      txnEgressQueue};

  const auto byteEventTracker = setMockByteEventTracker();
  const auto params = GetParam();
  InSequence enforceOrder;

  transport_->setEorTracking(true);
  transport_->setAppBytesWritten(params.byteOffset);
  transport_->setRawBytesWritten(params.byteOffset);
  if (params.timestampTx) {
    EXPECT_CALL(*byteEventTracker,
                addTxByteEvent(params.byteOffset, params.eventType, &txn, _))
        .WillOnce(Invoke([&](uint64_t /*offset*/,
                             ByteEvent::EventType /*eventType*/,
                             HTTPTransaction* /*txn*/,
                             ByteEvent::Callback) {
          // do nothing
        }));
  }
  if (params.timestampAck) {
    EXPECT_CALL(*byteEventTracker,
                addAckByteEvent(params.byteOffset, params.eventType, &txn, _))
        .WillOnce(Invoke([&](uint64_t /*offset*/,
                             ByteEvent::EventType /*eventType*/,
                             HTTPTransaction* /*txn*/,
                             ByteEvent::Callback) {
          // do nothing
        }));
  }

  const auto byteEvent = make_shared<TransactionByteEvent>(
      params.byteOffset, params.eventType, &txn);
  byteEvent->timestampTx_ = params.timestampTx;
  byteEvent->timestampAck_ = params.timestampAck;
  byteEventTracker->onTxnByteEventWrittenToBuf(*byteEvent);

  cleanup();
}

TEST_F(HTTP2DownstreamSessionTest, Trailers) {
  InSequence enforceOrder;

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler]() {
    handler->sendReplyWithBody(
        200, 100, true /* keepalive */, true /* sendEOM */, true /*trailers*/);
  });
  handler->expectDetachTransaction();

  HTTPSession::DestructorGuard g(httpSession_);
  sendRequest();
  flushRequestsAndLoop(true, milliseconds(0));

  EXPECT_CALL(callbacks_, onMessageBegin(1, _)).Times(1);
  EXPECT_CALL(callbacks_, onHeadersComplete(1, _)).Times(1);
  EXPECT_CALL(callbacks_, onBody(1, _, _));
  EXPECT_CALL(callbacks_, onTrailersComplete(1, _));
  EXPECT_CALL(callbacks_, onMessageComplete(1, _));

  parseOutput(*clientCodec_);
  expectDetachSession();
}

TEST_F(HTTPDownstreamSessionTest, Trailers) {
  testChunks(true);
}

TEST_F(HTTPDownstreamSessionTest, ExplicitChunks) {
  testChunks(false);
}

template <class C>
void HTTPDownstreamTest<C>::testChunks(bool trailers) {
  InSequence enforceOrder;

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler, trailers]() {
    handler->sendChunkedReplyWithBody(200, 100, 17, trailers);
  });
  handler->expectDetachTransaction();

  HTTPSession::DestructorGuard g(httpSession_);
  sendRequest();
  flushRequestsAndLoop(true, milliseconds(0));

  EXPECT_CALL(callbacks_, onMessageBegin(1, _)).Times(1);
  EXPECT_CALL(callbacks_, onHeadersComplete(1, _)).Times(1);
  for (int i = 0; i < 6; i++) {
    EXPECT_CALL(callbacks_, onChunkHeader(1, _));
    EXPECT_CALL(callbacks_, onBody(1, _, _));
    EXPECT_CALL(callbacks_, onChunkComplete(1));
  }
  if (trailers) {
    EXPECT_CALL(callbacks_, onTrailersComplete(1, _));
  }
  EXPECT_CALL(callbacks_, onMessageComplete(1, _));

  parseOutput(*clientCodec_);
  expectDetachSession();
}

TEST_F(HTTPDownstreamSessionTest, HttpDrain) {
  InSequence enforceOrder;

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders([this, &handler1] {
    handler1->sendHeaders(200, 100);
    httpSession_->notifyPendingShutdown();
  });
  handler1->expectEOM([&handler1] {
    handler1->sendBody(100);
    handler1->txn_->sendEOM();
  });
  handler1->expectDetachTransaction();

  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders([&handler2] { handler2->sendHeaders(200, 100); });
  handler2->expectEOM([&handler2] {
    handler2->sendBody(100);
    handler2->txn_->sendEOM();
  });
  handler2->expectDetachTransaction();

  expectDetachSession();

  sendRequest();
  sendRequest();
  flushRequestsAndLoop();
}

// 1) receive full request
// 2) notify pending shutdown
// 3) wait for session read timeout -> should be ignored
// 4) response completed
TEST_F(HTTPDownstreamSessionTest, HttpDrainLongRunning) {
  InSequence enforceSequence;

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([this, &handler] {
    httpSession_->notifyPendingShutdown();
    eventBase_.tryRunAfterDelay(
        [this] {
          // simulate read timeout
          httpSession_->timeoutExpired();
        },
        100);
    eventBase_.tryRunAfterDelay(
        [&handler] { handler->sendReplyWithBody(200, 100); }, 200);
  });
  handler->expectEOM();
  handler->expectDetachTransaction();

  expectDetachSession();

  sendRequest();
  flushRequestsAndLoop();
}

TEST_F(HTTPDownstreamSessionTest, EarlyAbort) {
  StrictMock<MockHTTPHandler> handler;

  InSequence enforceOrder;
  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));

  EXPECT_CALL(handler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) {
        handler.txn_ = txn;
        handler.txn_->sendAbort();
      }));
  handler.expectDetachTransaction();
  expectDetachSession();

  addSingleByteReads(
      "GET /somepath.php?param=foo HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n"
      "\r\n");
  transport_->addReadEOF(milliseconds(0));
  transport_->startReadEvents();
  eventBase_.loop();
}

TEST_F(HTTPDownstreamSessionTest, HttpWritesDrainingTimeout) {
  sendRequest();
  sendHeader();

  InSequence handlerSequence;
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, this] {
    transport_->pauseWrites();
    handler1->sendHeaders(200, 1000);
  });
  handler1->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
    ASSERT_EQ(folly::to<std::string>("WriteTimeout on transaction id: ",
                                     handler1->txn_->getID()),
              std::string(ex.what()));
    handler1->txn_->sendAbort();
  });
  handler1->expectDetachTransaction();
  expectDetachSession();

  flushRequestsAndLoop();
}

TEST_F(HTTPDownstreamSessionTest, HttpRateLimitNormal) {
  // The rate-limiting code grabs the event base from the EventBaseManager,
  // so we need to set it.
  folly::EventBaseManager::get()->setEventBase(&eventBase_, false);

  // Create a request
  sendRequest();

  InSequence handlerSequence;

  // Set a low rate-limit on the transaction
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders([&] {
    uint32_t rateLimit_kbps = 640;
    handler1->txn_->setEgressRateLimit(rateLimit_kbps * 1024);
  });
  // Send a somewhat big response that we know will get rate-limited
  handler1->expectEOM([&handler1] {
    // At 640kbps, this should take slightly over 800ms
    uint32_t rspLengthBytes = 100000;
    handler1->sendHeaders(200, rspLengthBytes);
    handler1->sendBody(rspLengthBytes);
    handler1->txn_->sendEOM();
  });
  handler1->expectDetachTransaction();

  // Keep the session around even after the event base loop completes so we can
  // read the counters on a valid object.
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop();

  proxygen::TimePoint timeFirstWrite =
      transport_->getWriteEvents()->front()->getTime();
  proxygen::TimePoint timeLastWrite =
      transport_->getWriteEvents()->back()->getTime();
  int64_t writeDuration =
      (int64_t)millisecondsBetween(timeLastWrite, timeFirstWrite).count();
  EXPECT_GE(writeDuration, 800);

  cleanup();
}

TEST_F(HTTP2DownstreamSessionTest, RateLimitNormal) {
  // The rate-limiting code grabs the event base from the EventBaseManager,
  // so we need to set it.
  folly::EventBaseManager::get()->setEventBase(&eventBase_, false);

  clientCodec_->getEgressSettings()->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                                                100000);
  clientCodec_->generateSettings(requests_);
  clientCodec_->generateWindowUpdate(requests_, 0, 10000);
  sendRequest();

  InSequence handlerSequence;
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders([&] {
    uint32_t rateLimit_kbps = 640;
    handler1->txn_->setEgressRateLimit(rateLimit_kbps * 1024);
  });

  handler1->expectEOM([&handler1] {
    // At 640kbps, this should take slightly over 800ms
    uint32_t rspLengthBytes = 100000;
    handler1->sendHeaders(200, rspLengthBytes);
    handler1->sendBody(rspLengthBytes);
    handler1->txn_->sendEOM();
  });
  handler1->expectDetachTransaction();

  // Keep the session around even after the event base loop completes so we can
  // read the counters on a valid object.
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop(true, milliseconds(50));

  proxygen::TimePoint timeFirstWrite =
      transport_->getWriteEvents()->front()->getTime();
  proxygen::TimePoint timeLastWrite =
      transport_->getWriteEvents()->back()->getTime();
  int64_t writeDuration =
      (int64_t)millisecondsBetween(timeLastWrite, timeFirstWrite).count();
  EXPECT_GE(writeDuration, 800);
  expectDetachSession();
}

/**
 * This test will reset the connection while the server is waiting around
 * to send more bytes (so as to keep under the rate limit).
 */
TEST_F(HTTP2DownstreamSessionTest, RateLimitRst) {
  // The rate-limiting code grabs the event base from the EventBaseManager,
  // so we need to set it.
  folly::EventBaseManager::get()->setEventBase(&eventBase_, false);

  folly::IOBufQueue rst{folly::IOBufQueue::cacheChainLength()};
  clientCodec_->getEgressSettings()->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                                                100000);
  clientCodec_->generateSettings(requests_);
  auto streamID = sendRequest();
  clientCodec_->generateRstStream(rst, streamID, ErrorCode::CANCEL);

  InSequence handlerSequence;
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders([&] {
    uint32_t rateLimit_kbps = 640;
    handler1->txn_->setEgressRateLimit(rateLimit_kbps * 1024);
  });
  handler1->expectEOM([&handler1] {
    uint32_t rspLengthBytes = 100000;
    handler1->sendHeaders(200, rspLengthBytes);
    handler1->sendBody(rspLengthBytes);
    handler1->txn_->sendEOM();
  });
  handler1->expectError();
  handler1->expectDetachTransaction();
  expectDetachSession();

  flushRequestsAndLoop(true, milliseconds(50), milliseconds(0), [&] {
    transport_->addReadEvent(rst, milliseconds(10));
  });
}

// Send a 1.0 request, egress the EOM with the last body chunk on a paused
// socket, and let it timeout.  dropConnection()
// to removeTransaction with writesDraining_=true
TEST_F(HTTPDownstreamSessionTest, WriteTimeout) {
  HTTPMessage req = getGetRequest();
  req.setHTTPVersion(1, 0);
  sendRequest(req);

  InSequence handlerSequence;
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, this] {
    handler1->sendHeaders(200, 100);
    eventBase_.tryRunAfterDelay(
        [&handler1, this] {
          transport_->pauseWrites();
          handler1->sendBody(100);
          handler1->txn_->sendEOM();
        },
        50);
  });
  handler1->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
    ASSERT_EQ(folly::to<std::string>("WriteTimeout on transaction id: ",
                                     handler1->txn_->getID()),
              std::string(ex.what()));
  });
  handler1->expectDetachTransaction();

  expectDetachSession();

  flushRequestsAndLoop();
}

// Send an abort from the write timeout path while pipelining
TEST_F(HTTPDownstreamSessionTest, WriteTimeoutPipeline) {
  const char* buf =
      "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  requests_.append(buf, strlen(buf));

  auto handler2 = addSimpleNiceHandler();
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, this] {
    handler1->sendHeaders(200, 100);
    eventBase_.runInLoop([&handler1, this] {
      transport_->pauseWrites();
      handler1->sendBody(100);
      handler1->txn_->sendEOM();
    });
  });
  handler2->expectHeaders();
  handler2->expectEOM();
  handler2->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
    ASSERT_EQ(folly::to<std::string>("WriteTimeout on transaction id: ",
                                     handler2->txn_->getID()),
              std::string(ex.what()));
    handler2->txn_->sendAbort();
  });
  handler2->expectDetachTransaction();
  handler1->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
    ASSERT_EQ(folly::to<std::string>("WriteTimeout on transaction id: ",
                                     handler1->txn_->getID()),
              std::string(ex.what()));
    handler1->txn_->sendAbort();
  });
  handler1->expectDetachTransaction();
  expectDetachSession();

  flushRequestsAndLoop();
}

TEST_F(HTTPDownstreamSessionTest, BodyPacketization) {
  HTTPMessage req = getGetRequest();
  req.setHTTPVersion(1, 0);
  req.setWantsKeepalive(false);
  sendRequest(req);

  InSequence handlerSequence;
  auto handler1 = addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1] { handler1->sendReplyWithBody(200, 32768); });
  handler1->expectDetachTransaction();

  expectDetachSession();

  // Keep the session around even after the event base loop completes so we can
  // read the counters on a valid object.
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop();

  EXPECT_EQ(transport_->getWriteEvents()->size(), 1);
}

TEST_F(HTTPDownstreamSessionTest, HttpMalformedPkt1) {
  // Create a HTTP connection and keep sending just '\n' to the HTTP1xCodec.
  std::string data(90000, '\n');
  requests_.append(data.data(), data.length());

  expectDetachSession();

  flushRequestsAndLoop(true, milliseconds(0));
}

TEST_F(HTTPDownstreamSessionTest, BigExplcitChunkWrite) {
  // even when the handler does a massive write, the transport only gets small
  // writes
  sendRequest();

  auto handler = addSimpleNiceHandler();
  handler->expectHeaders([&handler] {
    handler->sendHeaders(200, 100, false);
    size_t len = 16 * 1024 * 1024;
    handler->txn_->sendChunkHeader(len);
    auto chunk = makeBuf(len);
    handler->txn_->sendBody(std::move(chunk));
    handler->txn_->sendChunkTerminator();
    handler->txn_->sendEOM();
  });
  handler->expectDetachTransaction();

  expectDetachSession();

  // Keep the session around even after the event base loop completes so we can
  // read the counters on a valid object.
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop();

  EXPECT_GT(transport_->getWriteEvents()->size(), 250);
}

// ==== upgrade tests ====

// Test upgrade to a protocol unknown to HTTPSession
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNonNative) {
  auto handler = addSimpleStrictHandler();

  handler->expectHeaders([&handler, this] {
    // TODO: there is serious weirdness in this case.  The HTTP parser must
    // be paused from this point until the response headers are sent.  If we
    // delay the 101 headers by 1 loop, the case breaks without the
    // pause/resume
    handler->txn_->pauseIngress();
    eventBase_.runInLoop([&handler] {
      handler->sendHeaders(101, 0, true, {{"Upgrade", "blarf"}});
      handler->txn_->resumeIngress();
    });
  });
  EXPECT_CALL(*handler, _onUpgrade(UpgradeProtocol::TCP));

  sendRequest(getUpgradeRequest("blarf"), /*eom=*/false);
  // Enough to receive the request, wait a loop, send the response
  flushRequestsAndLoopN(3);

  // Now send random blarf data
  handler->expectBody([&handler] {
    handler->txn_->sendBody(makeBuf(100));
    handler->txn_->sendEOM();
  });

  folly::IOBufQueue bq{folly::IOBufQueue::cacheChainLength()};
  bq.append(makeBuf(100));
  transport_->addReadEvent(bq, milliseconds(0));

  // The server sent EOM, which means transport writes should be closed now
  flushRequestsAndLoopN(2);
  EXPECT_FALSE(transport_->good());

  // Add EOF from the client to complete the teardown
  handler->expectEOM();
  handler->expectDetachTransaction();
  expectDetachSession();
  transport_->addReadEOF(milliseconds(0));
  eventBase_.loop();
}

// Test upgrade to a protocol unknown to HTTPSession, but don't switch
// protocols
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNonNativeIgnore) {
  auto handler = addSimpleStrictHandler();

  handler->expectHeaders([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectEOM();
  handler->expectDetachTransaction();

  sendRequest(getUpgradeRequest("blarf"));

  expectDetachSession();
  flushRequestsAndLoop(true);
}

// Test upgrade to a protocol unknown to HTTPSession
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNonNativePipeline) {
  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders([&handler1](std::shared_ptr<HTTPMessage> msg) {
    EXPECT_EQ(msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_UPGRADE), "blarf");
    handler1->sendReplyWithBody(200, 100);
  });
  handler1->expectEOM();
  handler1->expectDetachTransaction();

  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders(
      [&handler2] { handler2->sendReplyWithBody(200, 100); });
  handler2->expectEOM();
  handler2->expectDetachTransaction();

  sendRequest(getUpgradeRequest("blarf"));
  transport_->addReadEvent(
      "GET / HTTP/1.1\r\n"
      "\r\n");
  expectDetachSession();
  flushRequestsAndLoop(true);
}

// Helper that does a simple upgrade test - request an upgrade, receive a 101
// and an upgraded response
template <class C>
void HTTPDownstreamTest<C>::testSimpleUpgrade(
    const std::string& upgradeHeader,
    CodecProtocol expectedProtocol,
    const std::string& expectedUpgradeHeader) {
  this->rawCodec_->setAllowedUpgradeProtocols({expectedUpgradeHeader});

  auto handler = addSimpleStrictHandler();

  HeaderIndexingStrategy testH2IndexingStrat;
  handler->expectHeaders();
  EXPECT_CALL(mockController_, onSessionCodecChange(httpSession_));
  handler->expectEOM([&handler,
                      expectedProtocol,
                      expectedUpgradeHeader,
                      &testH2IndexingStrat] {
    EXPECT_FALSE(handler->txn_->getSetupTransportInfo().secure);
    EXPECT_EQ(*handler->txn_->getSetupTransportInfo().appProtocol,
              expectedUpgradeHeader);
    if (expectedProtocol == CodecProtocol::HTTP_2) {
      const HTTP2Codec* codec = dynamic_cast<const HTTP2Codec*>(
          &handler->txn_->getTransport().getCodec());
      ASSERT_NE(codec, nullptr);
      EXPECT_EQ(codec->getHeaderIndexingStrategy(), &testH2IndexingStrat);
    }
    handler->sendReplyWithBody(200, 100);
  });
  handler->expectDetachTransaction();

  if (expectedProtocol == CodecProtocol::HTTP_2) {
    EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
        .WillOnce(Return(&testH2IndexingStrat));
  }

  HTTPMessage req = getUpgradeRequest(upgradeHeader);
  if (upgradeHeader == http2::kProtocolCleartextString) {
    HTTP2Codec::requestUpgrade(req);
  }
  sendRequest(req);
  flushRequestsAndLoop();

  expect101(expectedProtocol, expectedUpgradeHeader);
  expectResponse();

  gracefulShutdown();
}

// Upgrade to HTTP/2
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativeH2) {
  testSimpleUpgrade("h2c", CodecProtocol::HTTP_2, "h2c");
}

class HTTPDownstreamSessionUpgradeFlowControlTest
    : public HTTPDownstreamSessionTest {
 public:
  HTTPDownstreamSessionUpgradeFlowControlTest()
      : HTTPDownstreamSessionTest({100000, 105000, 110000}) {
  }
};

// Upgrade to HTTP/2, with non-default flow control settings
TEST_F(HTTPDownstreamSessionUpgradeFlowControlTest, UpgradeH2Flowcontrol) {
  testSimpleUpgrade("h2c", CodecProtocol::HTTP_2, "h2c");
}

// Upgrade to H2 with a non-native proto in the list
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativeUnknown) {
  // This is maybe weird, the client asked for non-native as first choice,
  // but we go native
  testSimpleUpgrade("blarf, h2c, spdy/3", CodecProtocol::HTTP_2, "h2c");
}

// Upgrade header with extra whitespace
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativeWhitespace) {
  testSimpleUpgrade(" \th2c\t , spdy/3", CodecProtocol::HTTP_2, "h2c");
}

// Upgrade header with random junk
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativeJunk) {
  testSimpleUpgrade(
      ",,,,   ,,\t~^%$(*&@(@$^^*(,h2c", CodecProtocol::HTTP_2, "h2c");
}

// Attempt to upgrade on second txn
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativeTxn2) {
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1] { handler1->sendReplyWithBody(200, 100); });
  handler1->expectDetachTransaction();
  sendRequest(getGetRequest());
  flushRequestsAndLoop();
  expectResponse();

  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&handler2] { handler2->sendReplyWithBody(200, 100); });
  handler2->expectDetachTransaction();

  sendRequest(getUpgradeRequest("h2c"));
  flushRequestsAndLoop();
  expectResponse();
  gracefulShutdown();
}

// Upgrade on POST
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativePost) {
  EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
      .WillOnce(Return(HeaderIndexingStrategy::getDefaultInstance()));
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectBody();
  EXPECT_CALL(mockController_, onSessionCodecChange(httpSession_));
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();

  HTTPMessage req = getUpgradeRequest("h2c", HTTPMethod::POST, 10);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  // cheat and not sending EOM, it's a no-op
  flushRequestsAndLoop();
  expect101(CodecProtocol::HTTP_2, "h2c");
  expectResponse();
  gracefulShutdown();
}

// Upgrade on POST with a reply that comes before EOM, don't switch protocols
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativePostEarlyResp) {
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();

  HTTPMessage req = getUpgradeRequest("h2c", HTTPMethod::POST, 10);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  flushRequestsAndLoop();
  expectResponse();
  gracefulShutdown();
}

TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativePostEarlyPartialResp) {
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] { handler->sendHeaders(200, 100); });
  handler->expectBody();
  handler->expectEOM([&handler] {
    handler->sendBody(100);
    handler->txn_->sendEOM();
  });
  handler->expectDetachTransaction();

  HTTPMessage req = getUpgradeRequest("h2c", HTTPMethod::POST, 10);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  flushRequestsAndLoop();
  expectResponse();
  gracefulShutdown();
}

// Upgrade but with a pipelined HTTP request.  It is parsed as H2 and
// rejected
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativeExtra) {
  EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
      .WillOnce(Return(HeaderIndexingStrategy::getDefaultInstance()));
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  EXPECT_CALL(mockController_, onSessionCodecChange(httpSession_));
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectError();
  handler->expectDetachTransaction();

  sendRequest(getUpgradeRequest("h2c"));
  flushRequests();
  // It's a fatal to send this out on the HTTP1xCodec, so hack it manually
  transport_->addReadEvent(
      "GET / HTTP/1.1\r\n"
      "Upgrade: h2c\r\n"
      "\r\n");
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop();
  expect101(CodecProtocol::HTTP_2, "h2c");
  EXPECT_CALL(callbacks_, onGoaway(_, _, _));
  parseOutput(*clientCodec_);
  expectDetachSession();
}

// Upgrade on POST with Expect: 100-Continue.  If the 100 goes out
// before the EOM is parsed, the 100 will be in HTTP.  This should be the normal
// case since the client *should* wait a bit for the 100 continue to come back
// before sending the POST.  But if the 101 is delayed beyond EOM, the 101
// will come via H2.
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativePost100) {
  EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
      .WillOnce(Return(HeaderIndexingStrategy::getDefaultInstance()));
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] { handler->sendHeaders(100, 0); });
  handler->expectBody();
  EXPECT_CALL(mockController_, onSessionCodecChange(httpSession_));
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();

  HTTPMessage req = getUpgradeRequest("h2c", HTTPMethod::POST, 10);
  req.getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  flushRequestsAndLoop();
  expect101(CodecProtocol::HTTP_2, "h2c", true /* expect 100 continue */);
  expectResponse();
  gracefulShutdown();
}

TEST_F(HTTPDownstreamSessionTest, HttpUpgradeNativePost100Late) {
  EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
      .WillOnce(Return(HeaderIndexingStrategy::getDefaultInstance()));
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectBody();
  EXPECT_CALL(mockController_, onSessionCodecChange(httpSession_));
  handler->expectEOM([&handler] {
    handler->sendHeaders(100, 0);
    handler->sendReplyWithBody(200, 100);
  });
  handler->expectDetachTransaction();

  HTTPMessage req = getUpgradeRequest("h2c", HTTPMethod::POST, 10);
  req.getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  flushRequestsAndLoop();
  expect101(CodecProtocol::HTTP_2, "h2c");
  expectResponse(200, ErrorCode::NO_ERROR, true /* expect 100 via H2 */);
  gracefulShutdown();
}

// Test sending a GOAWAY while the downstream session is still processing
// the request that was an upgrade.  The reply GOAWAY should have last good
// stream = 1, not 0.
TEST_F(HTTPDownstreamSessionTest, HttpUpgradeGoawayDrain) {
  this->rawCodec_->setAllowedUpgradeProtocols({"h2c"});
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectBody();
  EXPECT_CALL(mockController_, onSessionCodecChange(httpSession_));
  handler->expectEOM();
  handler->expectGoaway();
  handler->expectDetachTransaction();

  EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
      .WillOnce(Return(&testH2IndexingStrat_));

  HTTPMessage req = getUpgradeRequest("h2c", HTTPMethod::POST, 10);
  HTTP2Codec::requestUpgrade(req);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(10), HTTPCodec::NoPadding, true);
  // cheat and not sending EOM, it's a no-op

  flushRequestsAndLoop();
  expect101(CodecProtocol::HTTP_2, "h2c");
  clientCodec_->generateConnectionPreface(requests_);
  clientCodec_->generateSettings(requests_);
  clientCodec_->generateGoaway(requests_, 0, ErrorCode::NO_ERROR);
  flushRequestsAndLoop();
  eventBase_.runInLoop([&handler] { handler->sendReplyWithBody(200, 100); });
  HTTPSession::DestructorGuard g(httpSession_);
  eventBase_.loop();
  expectResponse(200, ErrorCode::NO_ERROR, false, true);
  expectDetachSession();
}

template <class C>
void HTTPDownstreamTest<C>::testPriorities(uint32_t numPriorities) {
  uint32_t iterations = 10;
  uint32_t maxPriority = numPriorities - 1;
  std::vector<std::unique_ptr<testing::NiceMock<MockHTTPHandler>>> handlers;
  for (int pri = numPriorities - 1; pri >= 0; pri--) {
    for (uint32_t i = 0; i < iterations; i++) {
      sendRequest("/", pri * (8 / numPriorities));
      InSequence handlerSequence;
      auto handler = addSimpleNiceHandler();
      auto rawHandler = handler.get();
      handlers.push_back(std::move(handler));
      rawHandler->expectHeaders();
      rawHandler->expectEOM(
          [rawHandler] { rawHandler->sendReplyWithBody(200, 1000); });
      rawHandler->expectDetachTransaction([] {});
    }
  }

  auto buf = requests_.move();
  buf->coalesce();
  requests_.append(std::move(buf));

  flushRequestsAndLoop();

  std::list<HTTPCodec::StreamID> streams;
  EXPECT_CALL(callbacks_, onMessageBegin(_, _))
      .Times(iterations * numPriorities);
  EXPECT_CALL(callbacks_, onHeadersComplete(_, _))
      .Times(iterations * numPriorities);
  // body is variable and hence ignored
  EXPECT_CALL(callbacks_, onMessageComplete(_, _))
      .Times(iterations * numPriorities)
      .WillRepeatedly(Invoke([&](HTTPCodec::StreamID stream, bool /*upgrade*/) {
        streams.push_back(stream);
      }));

  parseOutput(*clientCodec_);

  // transactions finish in priority order (higher streamIDs first)
  EXPECT_EQ(streams.size(), iterations * numPriorities);
  auto txn = streams.begin();
  for (int band = maxPriority; band >= 0; band--) {
    auto upperID = iterations * 2 * (band + 1);
    auto lowerID = iterations * 2 * band;
    for (uint32_t i = 0; i < iterations; i++) {
      EXPECT_LE(lowerID, (uint32_t)*txn);
      EXPECT_GE(upperID, (uint32_t)*txn);
      ++txn;
    }
  }
}

// Verifies that the read timeout is not running when no ingress is expected/
// required to proceed
TEST_F(HTTP2DownstreamSessionTest, H2Timeout) {
  sendRequest();

  InSequence handlerSequence;
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&] {
    handler1->sendHeaders(200, 1000);
    handler1->sendBody(1000);
    eventBase_.runAfterDelay([&handler1] { handler1->txn_->sendEOM(); }, 600);
  });
  handler1->expectDetachTransaction();

  flushRequestsAndLoop(false, milliseconds(0), milliseconds(10));

  cleanup();
}

// Verifies that the read timer is running while a transaction is blocked
// on a window update
TEST_F(HTTP2DownstreamSessionTest, H2TimeoutWin) {
  clientCodec_->getEgressSettings()->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                                                500);
  clientCodec_->generateSettings(requests_);
  auto streamID = sendRequest();

  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&] { handler->sendReplyWithBody(200, 1000); });
  handler->expectEgressPaused();
  handler->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
    ASSERT_EQ(folly::to<std::string>("ingress timeout, streamID=", streamID),
              std::string(ex.what()));
    handler->terminate();
  });
  handler->expectDetachTransaction();

  flushRequestsAndLoop();

  cleanup();
}

TYPED_TEST_SUITE_P(HTTPDownstreamTest);

TYPED_TEST_P(HTTPDownstreamTest, TestMaxTxnOverriding) {
  this->httpSession_->setEgressSettings(
      {{SettingsId::MAX_CONCURRENT_STREAMS, 1}});

  auto handler = this->addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();

  this->sendRequest();
  // This one is over the limit
  auto streamId = this->sendRequest();

  this->flushRequestsAndLoop();

  EXPECT_CALL(this->callbacks_, onSettings(_));
  EXPECT_CALL(this->callbacks_, onAbort(streamId, ErrorCode::REFUSED_STREAM));

  this->parseOutput(*this->clientCodec_);
  handler->sendReplyWithBody(200, 100);
  handler->expectDetachTransaction();

  this->flushRequestsAndLoop();
  this->cleanup();
}

TYPED_TEST_P(HTTPDownstreamTest, TestWritesDraining) {
  auto badCodec =
      makeServerCodec<typename TypeParam::Codec>(TypeParam::version);
  this->sendRequest();
  badCodec->generatePushPromise(
      this->requests_, 2 /* bad */, getGetRequest(), 1);

  this->expectDetachSession();

  InSequence handlerSequence;
  auto handler1 = this->addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM();
  handler1->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorMalformedInput);

    ASSERT_TRUE(folly::StringPiece(ex.what()).startsWith(
        "Shutdown transport: MalformedInput"))
        << ex.what();
  });
  handler1->expectDetachTransaction();

  this->flushRequestsAndLoop();
}

TYPED_TEST_P(HTTPDownstreamTest, TestBodySizeLimit) {
  this->clientCodec_->generateWindowUpdate(this->requests_, 0, 65536);
  this->sendRequest();
  this->sendRequest();

  InSequence handlerSequence;
  auto handler1 = this->addSimpleNiceHandler();
  handler1->expectHeaders();
  handler1->expectEOM();
  auto handler2 = this->addSimpleNiceHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] {
    handler1->sendReplyWithBody(200, 33000);
    handler2->sendReplyWithBody(200, 33000);
  });
  handler1->expectDetachTransaction();
  handler2->expectDetachTransaction();

  this->flushRequestsAndLoop();

  std::list<HTTPCodec::StreamID> streams;
  EXPECT_CALL(this->callbacks_, onMessageBegin(1, _));
  EXPECT_CALL(this->callbacks_, onHeadersComplete(1, _));
  EXPECT_CALL(this->callbacks_, onMessageBegin(3, _));
  EXPECT_CALL(this->callbacks_, onHeadersComplete(3, _));
  for (uint32_t i = 0; i < 8; i++) {
    EXPECT_CALL(this->callbacks_, onBody(1, _, _));
    EXPECT_CALL(this->callbacks_, onBody(3, _, _));
  }
  EXPECT_CALL(this->callbacks_, onBody(1, _, _));
  EXPECT_CALL(this->callbacks_, onMessageComplete(1, _));
  EXPECT_CALL(this->callbacks_, onBody(3, _, _));
  EXPECT_CALL(this->callbacks_, onMessageComplete(3, _));

  this->parseOutput(*this->clientCodec_);

  this->cleanup();
}

#define IF_HTTP2(X)                                                 \
  if (this->clientCodec_->getProtocol() == CodecProtocol::HTTP_2) { \
    X;                                                              \
  }

// Test exceeding the MAX_CONCURRENT_STREAMS setting.  The txn should get
// REFUSED_STREAM, and other streams can complete normally
TYPED_TEST_P(HTTPDownstreamTest, TestMaxTxns) {
  auto settings = this->rawCodec_->getEgressSettings();
  auto maxTxns = settings->getSetting(SettingsId::MAX_CONCURRENT_STREAMS, 100);
  std::list<unique_ptr<StrictMock<MockHTTPHandler>>> handlers;
  {
    for (auto i = 0U; i < maxTxns; i++) {
      this->sendRequest();
      auto handler = this->addSimpleStrictHandler();
      handler->expectHeaders();
      handler->expectEOM();
      handlers.push_back(std::move(handler));
    }
    auto streamID = this->sendRequest();
    this->clientCodec_->generateGoaway(this->requests_, 0, ErrorCode::NO_ERROR);

    for (auto& handler : handlers) {
      EXPECT_CALL(*handler, _onGoaway(ErrorCode::NO_ERROR));
    }

    this->flushRequestsAndLoop();

    EXPECT_CALL(this->callbacks_, onSettings(_));
    EXPECT_CALL(this->callbacks_, onAbort(streamID, ErrorCode::REFUSED_STREAM));

    this->parseOutput(*this->clientCodec_);
  }
  // handlers can finish out of order?
  for (auto& handler : handlers) {
    handler->sendReplyWithBody(200, 100);
    handler->expectDetachTransaction();
  }
  this->expectDetachSession();
  this->eventBase_.loop();
}

// Set max streams=1
// send two h2 requests a few ms apart.
// Block writes
// generate a complete response for txn=1 before parsing txn=3
// HTTPSession should allow the txn=3 to be served rather than refusing it
TEST_F(HTTP2DownstreamSessionTest, H2MaxConcurrentStreams) {
  HTTPMessage req = getGetRequest();
  req.setHTTPVersion(1, 0);
  req.setWantsKeepalive(false);
  sendRequest(req);
  auto req2p = sendRequestLater(req, true);

  httpSession_->setEgressSettings({{SettingsId::MAX_CONCURRENT_STREAMS, 1}});

  InSequence handlerSequence;
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, req, this, &req2p] {
    transport_->pauseWrites();
    handler1->sendReplyWithBody(200, 100);
    req2p.setValue();
  });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([this] { resumeWritesInLoop(); });
  handler1->expectDetachTransaction(
      [&handler2] { handler2->sendReplyWithBody(200, 100); });
  handler2->expectDetachTransaction();

  expectDetachSession();

  flushRequestsAndLoop();
}

REGISTER_TYPED_TEST_SUITE_P(HTTPDownstreamTest,
                            TestWritesDraining,
                            TestBodySizeLimit,
                            TestMaxTxns,
                            TestMaxTxnOverriding);

using ParallelCodecs = ::testing::Types<HTTP2CodecPair>;
INSTANTIATE_TYPED_TEST_SUITE_P(ParallelCodecs,
                               HTTPDownstreamTest,
                               ParallelCodecs);

class HTTP2DownstreamSessionFCTest : public HTTPDownstreamTest<HTTP2CodecPair> {
 public:
  HTTP2DownstreamSessionFCTest()
      : HTTPDownstreamTest<HTTP2CodecPair>(
            {-1, -1, 2 * http2::kInitialWindow}) {
  }
};

TEST_F(HTTP2DownstreamSessionFCTest, TestSessionFlowControl) {
  eventBase_.loopOnce();

  InSequence sequence;
  EXPECT_CALL(callbacks_, onSettings(_));
  EXPECT_CALL(callbacks_, onWindowUpdate(0, http2::kInitialWindow));
  parseOutput(*clientCodec_);

  cleanup();
}

TEST_F(HTTP2DownstreamSessionTest, TestEOFOnBlockedStream) {
  sendRequest();

  auto handler1 = addSimpleStrictHandler();

  InSequence handlerSequence;
  handler1->expectHeaders();
  handler1->expectEOM([&handler1] { handler1->sendReplyWithBody(200, 80000); });
  handler1->expectEgressPaused();

  handler1->expectError([&](const HTTPException& ex) {
    // Not optimal to have a different error code here than the session
    // flow control case, but HTTPException direction is immutable and
    // building another one seems not future proof.
    EXPECT_EQ(ex.getDirection(), HTTPException::Direction::INGRESS_AND_EGRESS);
  });
  handler1->expectDetachTransaction();

  expectDetachSession();

  flushRequestsAndLoop(true, milliseconds(10));
}

TEST_F(HTTP2DownstreamSessionFCTest, TestEOFOnBlockedSession) {
  HTTPTransaction::setEgressBufferLimit(25000);
  transport_->pauseWrites();
  sendRequest();
  sendRequest();

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1] {
    handler1->sendHeaders(200, 40000);
    handler1->sendBody(32769);
  });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&handler2, this] {
    handler2->sendHeaders(200, 40000);
    handler2->sendBody(32768);
    eventBase_.runInLoop([this] { transport_->addReadEOF(milliseconds(0)); });
  });
  handler1->expectEgressPaused();
  handler2->expectEgressPaused();
  handler1->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getDirection(), HTTPException::Direction::INGRESS_AND_EGRESS);
  });
  handler1->expectDetachTransaction();
  handler2->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getDirection(), HTTPException::Direction::INGRESS_AND_EGRESS);
  });
  handler2->expectDetachTransaction();

  expectDetachSession();

  flushRequestsAndLoop();
}

TEST_F(HTTP2DownstreamSessionTest, NewTxnEgressPaused) {
  // Send 1 request with prio=0
  // Have egress pause while sending the first response
  // Send a second request with prio=1
  // Finish the body and eom both responses
  // Unpause egress
  // The first txn should complete first

  auto id1 = sendRequest("/", 0);
  auto req2 = getGetRequest();
  req2.setPriority(1);
  auto req2p = sendRequestLater(req2, true);

  HTTPTransaction::setEgressBufferLimit(500);
  transport_->pauseWrites();

  unique_ptr<StrictMock<MockHTTPHandler>> handler1;
  unique_ptr<StrictMock<MockHTTPHandler>> handler2;

  {
    InSequence handlerSequence;
    handler1 = addSimpleStrictHandler();
    handler1->expectHeaders();
    handler1->expectEOM([&handler1, &req2p] {
      handler1->sendHeaders(200, 1000);
      handler1->sendBody(750); // over the limit
      req2p.setValue();
    });
    handler1->expectEgressPaused([] { LOG(INFO) << "paused 1"; });

    handler2 = addSimpleStrictHandler();
    handler2->expectHeaders();
    handler2->expectEOM([&] {
      // Technically shouldn't send while handler is egress paused, but meh.
      handler1->sendBody(250);
      handler1->txn_->sendEOM();
      handler2->sendReplyWithBody(200, 1000);
      resumeWritesInLoop();
    });
    handler2->expectEgressPaused();
    handler1->expectDetachTransaction();
    handler2->expectDetachTransaction();
  }
  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop();

  std::list<HTTPCodec::StreamID> streams;
  EXPECT_CALL(callbacks_, onMessageBegin(_, _)).Times(2);
  EXPECT_CALL(callbacks_, onHeadersComplete(_, _)).Times(2);
  // body is variable and hence ignored;
  EXPECT_CALL(callbacks_, onMessageComplete(_, _))
      .WillRepeatedly(Invoke([&](HTTPCodec::StreamID stream, bool /*upgrade*/) {
        streams.push_back(stream);
      }));
  parseOutput(*clientCodec_);

  EXPECT_EQ(*streams.begin(), id1);
  cleanup();
}

TEST_F(HTTP2DownstreamSessionTest, ZeroDeltaWindowUpdate) {
  // generateHeader() will create a session and a transaction
  auto streamID = sendHeader();
  // First generate a frame with delta=1 so as to pass the checks, and then
  // hack the frame so that delta=0 without modifying other checks
  clientCodec_->generateWindowUpdate(requests_, streamID, 1);
  requests_.trimEnd(http2::kFrameWindowUpdateSize);
  folly::io::QueueAppender appender(&requests_, http2::kFrameWindowUpdateSize);
  appender.writeBE<uint32_t>(0);

  auto handler = addSimpleStrictHandler();

  InSequence handlerSequence;
  handler->expectHeaders();
  handler->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getCodecStatusCode(), ErrorCode::PROTOCOL_ERROR);
    ASSERT_EQ("streamID=1 with window update delta=0", std::string(ex.what()));
  });
  handler->expectDetachTransaction();
  expectDetachSession();

  flushRequestsAndLoop();
}

TEST_F(HTTP2DownstreamSessionTest, PaddingFlowControl) {
  // generateHeader() will create a session and a transaction
  auto streamID = sendHeader();
  // This sends a total of 33kb including padding, so we should get a session
  // and stream window update
  for (auto i = 0; i < 129; i++) {
    clientCodec_->generateBody(requests_, streamID, makeBuf(1), 255, false);
  }

  auto handler = addSimpleStrictHandler();

  InSequence handlerSequence;
  handler->expectHeaders([&] {
    handler->txn_->pauseIngress();
    eventBase_.runAfterDelay([&] { handler->txn_->resumeIngress(); }, 100);
  });
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _)).Times(129);
  handler->expectError();
  handler->expectDetachTransaction();

  HTTPSession::DestructorGuard g(httpSession_);
  flushRequestsAndLoop(false, milliseconds(0), milliseconds(0), [&] {
    clientCodec_->generateRstStream(requests_, streamID, ErrorCode::CANCEL);
    clientCodec_->generateGoaway(requests_, 0, ErrorCode::NO_ERROR);
    transport_->addReadEvent(requests_, milliseconds(110));
  });

  std::list<HTTPCodec::StreamID> streams;
  EXPECT_CALL(callbacks_, onWindowUpdate(0, _));
  EXPECT_CALL(callbacks_, onWindowUpdate(1, _));
  parseOutput(*clientCodec_);
  expectDetachSession();
}

TEST_F(HTTP2DownstreamSessionTest, GracefulDrainOnTimeout) {
  InSequence handlerSequence;
  std::chrono::milliseconds gracefulTimeout(200);
  httpSession_->enableDoubleGoawayDrain();
  EXPECT_CALL(mockController_, getGracefulShutdownTimeout())
      .WillOnce(InvokeWithoutArgs([&] {
        // Once session asks for graceful shutdown timeout, expect the client
        // to receive the first GOAWAY
        eventBase_.runInLoop([&] {
          EXPECT_CALL(callbacks_,
                      onGoaway(std::numeric_limits<int32_t>::max(),
                               ErrorCode::NO_ERROR,
                               _));
          parseOutput(*clientCodec_);
        });
        return gracefulTimeout;
      }));

  // Simulate ConnectionManager idle timeout
  eventBase_.runAfterDelay([&] { httpSession_->timeoutExpired(); },
                           transactionTimeouts_->getDefaultTimeout().count());
  HTTPSession::DestructorGuard g(httpSession_);
  auto start = getCurrentTime();
  eventBase_.loop();
  auto finish = getCurrentTime();
  auto minDuration =
      gracefulTimeout + transactionTimeouts_->getDefaultTimeout();
  EXPECT_GE((finish - start).count(), minDuration.count());
  EXPECT_CALL(callbacks_, onGoaway(0, ErrorCode::NO_ERROR, _));
  parseOutput(*clientCodec_);
  expectDetachSession();
}

/*
 * The sequence of streams are generated in the following order:
 * - [client --> server] request 1st stream (getGetRequest())
 * - [server --> client] respond 1st stream (res with length 100)
 * - [server --> client] request 2nd stream (req)
 * - [server --> client] respond 2nd stream (res with length 200 + EOM)
 * - [client --> server] RST_STREAM on the 1st stream
 */
TEST_F(HTTP2DownstreamSessionTest, ServerPush) {
  // Create a dummy request and a dummy response messages
  HTTPMessage req, res;
  req.getHeaders().set("HOST", "www.foo.com");
  req.setURL("https://www.foo.com/");
  res.setStatusCode(200);
  res.setStatusMessage("Ohai");

  // enable server push
  clientCodec_->getEgressSettings()->setSetting(SettingsId::ENABLE_PUSH, 1);
  clientCodec_->generateSettings(requests_);
  // generateHeader() will create a session and a transaction
  auto assocStreamId = clientCodec_->createStream();
  clientCodec_->generateHeader(
      requests_, assocStreamId, getGetRequest(), false, nullptr);

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler;

  InSequence handlerSequence;
  handler->expectHeaders([&] {
    // Generate response for the associated stream
    handler->txn_->sendHeaders(res);
    handler->txn_->sendBody(makeBuf(100));
    handler->txn_->pauseIngress();

    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler);
    ASSERT_NE(pushTxn, nullptr);
    // Generate a push request (PUSH_PROMISE)
    auto outgoingStreams = httpSession_->getNumOutgoingStreams();
    pushTxn->sendHeaders(req);
    EXPECT_EQ(httpSession_->getNumOutgoingStreams(), outgoingStreams);
    // Generate a push response
    auto pri = handler->txn_->getPriority();
    res.setHTTP2Priority(
        std::make_tuple(pri.streamDependency, pri.exclusive, pri.weight));
    pushTxn->sendHeaders(res);
    EXPECT_EQ(httpSession_->getNumOutgoingStreams(), outgoingStreams + 1);
    pushTxn->sendBody(makeBuf(200));
    pushTxn->sendEOM();

    eventBase_.runAfterDelay([&] { handler->txn_->resumeIngress(); }, 100);
  });
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));
  EXPECT_CALL(pushHandler, _detachTransaction());
  handler->expectError();
  handler->expectDetachTransaction();

  transport_->addReadEvent(requests_, milliseconds(0));
  clientCodec_->generateRstStream(requests_, assocStreamId, ErrorCode::CANCEL);
  clientCodec_->generateGoaway(requests_, 2, ErrorCode::NO_ERROR);
  transport_->addReadEvent(requests_, milliseconds(200));
  transport_->startReadEvents();
  HTTPSession::DestructorGuard g(httpSession_);
  eventBase_.loop();

  EXPECT_CALL(callbacks_, onMessageBegin(1, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(1, _));
  EXPECT_CALL(callbacks_, onPushMessageBegin(2, 1, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(2, _));
  EXPECT_CALL(callbacks_, onMessageBegin(2, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(2, _));
  EXPECT_CALL(callbacks_, onMessageComplete(2, _));

  parseOutput(*clientCodec_);
  expectDetachSession();
}

TEST_F(HTTP2DownstreamSessionTest, ServerPushAbortPaused) {
  HTTPTransaction::setEgressBufferLimit(500);
  // Create a dummy request and a dummy response messages
  HTTPMessage req, res;
  req.getHeaders().set("HOST", "www.foo.com");
  req.setURL("https://www.foo.com/");
  res.setStatusCode(200);
  res.setStatusMessage("Ohai");

  transport_->pauseWrites();

  // enable server push
  clientCodec_->getEgressSettings()->setSetting(SettingsId::ENABLE_PUSH, 1);
  clientCodec_->generateSettings(requests_);
  // generateHeader() will create a session and a transaction
  auto assocStreamId = HTTPCodec::StreamID(1);
  clientCodec_->generateHeader(
      requests_, assocStreamId, getGetRequest(), false, nullptr);

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler;

  InSequence handlerSequence;
  HTTPTransaction* pushTxn;
  handler->expectHeaders([&] {
    // Generate response for the associated stream
    handler->txn_->sendHeaders(res);
    handler->txn_->sendBody(makeBuf(1000));
    handler->txn_->pauseIngress();

    pushTxn = handler->txn_->newPushedTransaction(&pushHandler);
    ASSERT_NE(pushTxn, nullptr);
    // Generate a push request (PUSH_PROMISE)
    pushTxn->sendHeaders(req);
    // Send headers and some body
    pushTxn->sendHeaders(res);
    pushTxn->sendBody(makeBuf(1000));
  });

  handler->expectEgressPaused();
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));
  EXPECT_CALL(pushHandler, _onEgressPaused());

  // Expect error on the associated stream.
  // The push transaction can still push data.
  handler->expectError([&](const HTTPException&) {
    pushTxn->sendBody(makeBuf(1000));
    pushTxn->sendEOM();
    resumeWritesInLoop();
    httpSession_->closeWhenIdle();
  });

  // Associated stream is destroyed.
  handler->expectDetachTransaction();

  // Push stream finishes on transaction timeout.
  EXPECT_CALL(pushHandler, _detachTransaction());

  transport_->addReadEvent(requests_, milliseconds(0));
  // Cancels associated stream
  clientCodec_->generateRstStream(requests_, assocStreamId, ErrorCode::CANCEL);
  transport_->addReadEvent(requests_, milliseconds(10));
  transport_->startReadEvents();
  HTTPSession::DestructorGuard g(httpSession_);

  expectDetachSession();
  eventBase_.loop();

  parseOutput(*clientCodec_);

  EXPECT_EQ(httpSession_->getNumOutgoingStreams(), 0);
}

TEST_F(HTTP2DownstreamSessionTest, ServerPushAfterWriteTimeout) {
  HTTPTransaction::setEgressBufferLimit(500);
  // Create a dummy request and a dummy response messages
  HTTPMessage req, res;
  req.getHeaders().set("HOST", "www.foo.com");
  req.setURL("https://www.foo.com/");
  res.setStatusCode(200);
  res.setStatusMessage("Ohai");

  transport_->pauseWrites();

  // enable server push
  clientCodec_->getEgressSettings()->setSetting(SettingsId::ENABLE_PUSH, 1);
  clientCodec_->generateSettings(requests_);
  // generateHeader() will create a session and a transaction
  auto assocStreamId = HTTPCodec::StreamID(3);
  clientCodec_->generateHeader(
      requests_, assocStreamId, getGetRequest(), true, nullptr);

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler1;

  // InSequence handlerSequence;
  handler->expectHeaders([&] {
    // Generate response for the associated stream
    handler->txn_->sendHeaders(res);
    handler->txn_->sendBody(makeBuf(1000));

    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler1);
    ASSERT_NE(pushTxn, nullptr);
    // Generate a push request (PUSH_PROMISE)
    pushTxn->sendHeaders(req);
    // Send headers and some body
    pushTxn->sendHeaders(res);
    pushTxn->sendBody(makeBuf(1000));
  });
  handler->expectEgressPaused();
  EXPECT_CALL(pushHandler1, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler1.txn_ = txn; }));
  handler->expectEOM();
  EXPECT_CALL(pushHandler1, _onEgressPaused());

  EXPECT_CALL(pushHandler1, _onError(_)).WillOnce(InvokeWithoutArgs([&] {
    StrictMock<MockHTTPPushHandler> pushHandler2;
    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler2);
    EXPECT_EQ(pushTxn, nullptr);
  }));
  handler->expectError();
  handler->expectDetachTransaction();
  EXPECT_CALL(pushHandler1, _detachTransaction());

  transport_->addReadEvent(requests_, milliseconds(0));
  transport_->startReadEvents();
  HTTPSession::DestructorGuard g(httpSession_);
  eventBase_.loop();

  parseOutput(*clientCodec_);
  expectDetachSession();
  EXPECT_EQ(httpSession_->getNumOutgoingStreams(), 0);
}

TEST_F(HTTP2DownstreamSessionTest, TestPriorityWeightsTinyRatio) {
  // Create a transaction with egress and a ratio small enough that
  // ratio*4096 < 1.
  //
  //     root
  //     /  \                                                 level 1
  //   256   1 (no egress)
  //        / \                                               level 2
  //      256  1  <-- has ratio (1/257)^2
  InSequence enforceOrder;
  auto req1 = getGetRequest();
  auto req2 = getGetRequest();
  req1.setHTTP2Priority(HTTPMessage::HTTP2Priority{0, false, 255});
  req2.setHTTP2Priority(HTTPMessage::HTTP2Priority{0, false, 0});

  sendRequest(req1);
  auto id2 = sendRequest(req2);
  req1.setHTTP2Priority(HTTPMessage::HTTP2Priority{id2, false, 255});
  req2.setHTTP2Priority(HTTPMessage::HTTP2Priority{id2, false, 0});
  sendRequest(req1);
  sendRequest(req2);

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&] { handler1->sendReplyWithBody(200, 4 * 1024); });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM();
  auto handler3 = addSimpleStrictHandler();
  handler3->expectHeaders();
  handler3->expectEOM([&] { handler3->sendReplyWithBody(200, 15); });
  auto handler4 = addSimpleStrictHandler();
  handler4->expectHeaders();
  handler4->expectEOM([&] { handler4->sendReplyWithBody(200, 1); });

  handler1->expectDetachTransaction();
  handler3->expectDetachTransaction();
  handler4->expectDetachTransaction(
      [&handler2] { handler2->txn_->sendAbort(); });
  handler2->expectDetachTransaction();
  flushRequestsAndLoop();
  httpSession_->closeWhenIdle();
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, TestPriorityDependentTransactions) {
  // Create a dependent transaction to test the priority blocked by dependency.
  // ratio*4096 < 1.
  //
  //     root
  //      \                                                 level 1
  //      16
  //       \                                                level 2
  //       16
  InSequence enforceOrder;
  auto req1 = getGetRequest();
  req1.setHTTP2Priority(HTTPMessage::HTTP2Priority{0, false, 15});
  auto id1 = sendRequest(req1);

  auto req2 = getGetRequest();
  req2.setHTTP2Priority(HTTPMessage::HTTP2Priority{id1, false, 15});
  sendRequest(req2);

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&] { handler1->sendReplyWithBody(200, 1024); });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] { handler2->sendReplyWithBody(200, 1024); });

  handler1->expectDetachTransaction();
  handler2->expectDetachTransaction([&] { handler2->txn_->sendAbort(); });
  flushRequestsAndLoop();
  httpSession_->closeWhenIdle();
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, TestDisablePriorities) {
  // turn off HTTP2 priorities
  httpSession_->setHTTP2PrioritiesEnabled(false);

  InSequence enforceOrder;
  HTTPMessage req1 = getGetRequest();
  req1.setHTTP2Priority(HTTPMessage::HTTP2Priority{0, false, 0});
  sendRequest(req1);

  HTTPMessage req2 = getGetRequest();
  req2.setHTTP2Priority(HTTPMessage::HTTP2Priority{0, false, 255});
  sendRequest(req2);

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&] { handler1->sendReplyWithBody(200, 4 * 1024); });

  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] { handler2->sendReplyWithBody(200, 4 * 1024); });

  // expecting handler 1 to finish first irrespective of
  // request 2 having higher weight
  handler1->expectDetachTransaction();
  handler2->expectDetachTransaction();

  flushRequestsAndLoop();
  httpSession_->closeWhenIdle();
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, TestPriorityWeights) {
  // virtual priority node with pri=4
  auto priGroupID = clientCodec_->createStream();
  clientCodec_->generatePriority(
      requests_, priGroupID, HTTPMessage::HTTP2Priority(0, false, 3));
  // Both txn's are at equal pri=16
  auto id1 = sendRequest();
  auto id2 = sendRequest();

  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders();
  handler1->expectEOM([&] {
    handler1->sendHeaders(200, 12 * 1024);
    handler1->txn_->sendBody(makeBuf(4 * 1024));
  });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] {
    handler2->sendHeaders(200, 12 * 1024);
    handler2->txn_->sendBody(makeBuf(4 * 1024));
  });

  // twice- once to send and once to receive
  flushRequestsAndLoopN(2);
  EXPECT_CALL(callbacks_, onSettings(_));
  EXPECT_CALL(callbacks_, onMessageBegin(id1, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(id1, _));
  EXPECT_CALL(callbacks_, onMessageBegin(id2, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(id2, _));
  EXPECT_CALL(callbacks_, onBody(id1, _, _)).WillOnce(ExpectBodyLen(4 * 1024));
  EXPECT_CALL(callbacks_, onBody(id2, _, _)).WillOnce(ExpectBodyLen(4 * 1024));
  parseOutput(*clientCodec_);

  // update handler2 to be in the pri-group (which has lower weight)
  clientCodec_->generatePriority(
      requests_, id2, HTTPMessage::HTTP2Priority(priGroupID, false, 15));

  eventBase_.runInLoop([&] {
    handler1->txn_->sendBody(makeBuf(4 * 1024));
    handler2->txn_->sendBody(makeBuf(4 * 1024));
  });
  flushRequestsAndLoopN(2);

  EXPECT_CALL(callbacks_, onBody(id1, _, _)).WillOnce(ExpectBodyLen(4 * 1024));
  EXPECT_CALL(callbacks_, onBody(id2, _, _))
      .WillOnce(ExpectBodyLen(1 * 1024))
      .WillOnce(ExpectBodyLen(3 * 1024));
  parseOutput(*clientCodec_);

  // update vnode weight to match txn1 weight
  clientCodec_->generatePriority(
      requests_, priGroupID, HTTPMessage::HTTP2Priority(0, false, 15));
  eventBase_.runInLoop([&] {
    handler1->txn_->sendBody(makeBuf(4 * 1024));
    handler1->txn_->sendEOM();
    handler2->txn_->sendBody(makeBuf(4 * 1024));
    handler2->txn_->sendEOM();
  });
  handler1->expectDetachTransaction();
  handler2->expectDetachTransaction();
  flushRequestsAndLoopN(2);

  // expect 32/32
  EXPECT_CALL(callbacks_, onBody(id1, _, _)).WillOnce(ExpectBodyLen(4 * 1024));
  EXPECT_CALL(callbacks_, onMessageComplete(id1, _));
  EXPECT_CALL(callbacks_, onBody(id2, _, _)).WillOnce(ExpectBodyLen(4 * 1024));
  EXPECT_CALL(callbacks_, onMessageComplete(id2, _));
  parseOutput(*clientCodec_);

  httpSession_->closeWhenIdle();
  expectDetachSession();
  this->eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, TestPriorityFCBlocked) {
  // Send two requests that are not stream f/c blocked but are conn f/c blocked
  // Send a third request with higher priority, ensure it is not blocked by
  // any low pri bytes.

  // virtual priority node
  auto priGroupID = clientCodec_->createStream();
  clientCodec_->generatePriority(
      requests_, priGroupID, HTTPMessage::HTTP2Priority(0, false, 3));
  auto req = getGetRequest();
  req.setHTTP2Priority(HTTPMessage::HTTP2Priority{priGroupID, false, 255});
  auto id1 = sendRequest(req);
  auto id2 = sendRequest(req);

  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&] {
    handler1->sendHeaders(200, 36 * 1024);
    handler1->txn_->sendBody(makeBuf(32 * 1024));
  });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] {
    handler2->sendHeaders(200, 36 * 1024);
    handler2->txn_->sendBody(makeBuf(32 * 1024));
  });
  flushRequestsAndLoopN(2);

  // Send high pri request.  It shouldn't pause, and send a reply
  req.setHTTP2Priority(HTTPMessage::HTTP2Priority{0, true, 3});
  auto id3 = sendRequest(req);
  clientCodec_->generateWindowUpdate(requests_, 0, 64 * 1024);
  auto handler3 = addSimpleStrictHandler();
  handler3->expectHeaders();
  handler3->expectEOM([&] {
    handler3->expectDetachTransaction();
    handler3->sendReplyWithBody(200, 32 * 1024);
    handler1->expectDetachTransaction();
    handler1->txn_->sendBody(makeBuf(4 * 1024));
    handler1->txn_->sendEOM();
    handler2->expectDetachTransaction();
    handler2->txn_->sendBody(makeBuf(4 * 1024));
    handler2->txn_->sendEOM();
  });

  // twice- once to send and once to receive
  flushRequestsAndLoopN(2);
  {
    InSequence enforceOrder;
    EXPECT_CALL(callbacks_, onSettings(_));
    EXPECT_CALL(callbacks_, onMessageBegin(id1, _));
    EXPECT_CALL(callbacks_, onHeadersComplete(id1, _));
    EXPECT_CALL(callbacks_, onMessageBegin(id2, _));
    EXPECT_CALL(callbacks_, onHeadersComplete(id2, _));
    EXPECT_CALL(callbacks_, onMessageBegin(id3, _));
    EXPECT_CALL(callbacks_, onHeadersComplete(id3, _));
    EXPECT_CALL(callbacks_, onMessageComplete(id3, _));
    EXPECT_CALL(callbacks_, onMessageComplete(id1, _));
    EXPECT_CALL(callbacks_, onMessageComplete(id2, _));
    parseOutput(*clientCodec_);
  }

  httpSession_->closeWhenIdle();
  expectDetachSession();
  this->eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, TestHeadersRateLimitExceeded) {
  httpSession_->setRateLimitParams(
      RateLimitFilter::Type::HEADERS, 100, std::chrono::seconds(0));

  std::vector<std::unique_ptr<testing::StrictMock<MockHTTPHandler>>> handlers;
  for (int i = 0; i < 100; i++) {
    auto handler = addSimpleStrictHandler();
    auto rawHandler = handler.get();
    handlers.push_back(std::move(handler));
    rawHandler->expectHeaders();
    rawHandler->expectEOM(
        [rawHandler] { rawHandler->sendReplyWithBody(200, 100); });
    sendRequest();
  }
  // Straw that breaks the camel's back
  sendRequest();
  for (int i = 0; i < 100; i++) {
    handlers[i]->expectGoaway();
    handlers[i]->expectDetachTransaction();
  }
  expectDetachSession();
  flushRequestsAndLoopN(2);
}

TEST_F(HTTP2DownstreamSessionTest, TestControlMsgRateLimitExceeded) {
  auto streamid = clientCodec_->createStream();

  httpSession_->setControlMessageRateLimitParams(100);

  // Send 97 PRIORITY, 1 SETTINGS, and 3 PING frames. This should exceed the
  // limit of 10, causing us to drop the connection.
  for (uint32_t i = 0; i < kMaxControlMsgsPerIntervalLowerBound - 3; i++) {
    clientCodec_->generatePriority(
        requests_, streamid, HTTPMessage::HTTP2Priority(0, false, 3));
  }

  clientCodec_->generateSettings(requests_);

  for (int i = 0; i < 3; i++) {
    clientCodec_->generatePingRequest(requests_);
  }

  expectDetachSession();

  flushRequestsAndLoopN(1);
}

TEST_F(HTTP2DownstreamSessionTest, TestControlMsgResetRateLimitTouched) {
  // clear pending settings frame
  flushRequestsAndLoop();

  auto streamid = clientCodec_->createStream();

  httpSession_->setControlMessageRateLimitParams(10, milliseconds(0));

  // Send 97 PRIORITY, 1 SETTINGS, and 2 PING frames. This doesn't exceed the
  // limit of 10.
  for (uint32_t i = 0; i < kMaxControlMsgsPerIntervalLowerBound - 3; i++) {
    clientCodec_->generatePriority(
        requests_, streamid, HTTPMessage::HTTP2Priority(0, false, 3));
  }

  clientCodec_->generateSettings(requests_);

  for (int i = 0; i < 2; i++) {
    clientCodec_->generatePingRequest(requests_);
  }
  //

  // We should reset the number of control frames seen, enabling us to send
  // more without hitting the rate limit
  flushRequestsAndLoop();

  // Send 10 control frames. This is just within the rate limits that we have
  // set.
  for (int i = 0; i < 5; i++) {
    clientCodec_->generatePriority(
        requests_, streamid, HTTPMessage::HTTP2Priority(0, false, 3));
  }

  clientCodec_->generateSettings(requests_);

  for (int i = 0; i < 4; i++) {
    clientCodec_->generatePingRequest(requests_);
  }

  flushRequestsAndLoopN(2);

  httpSession_->closeWhenIdle();
  expectDetachSession();
  this->eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, DirectErrorHandlingLimitTouched) {
  httpSession_->setRateLimitParams(RateLimitFilter::Type::DIRECT_ERROR_HANDLING,
                                   10,
                                   std::chrono::milliseconds(0));

  // Send 50 messages, each of which cause direct error handling. Since
  // this doesn't exceed the limit, this should not cause the connection
  // to be dropped.
  for (uint32_t i = 0;
       i < DirectErrorsRateLimitFilter::kMaxEventsPerIntervalLowerBound;
       i++) {
    auto req = getGetRequest();
    // Invalid method, causes the error to be handled directly
    req.setMethod("11111111");
    sendRequest(req, false);
  }

  EXPECT_CALL(mockController_, getParseErrorHandler(_, _, _))
      .WillRepeatedly(Return(nullptr));

  flushRequestsAndLoop();

  for (int i = 0; i < 10; i++) {
    auto req = getGetRequest();
    // Invalid method, causes the error to be handled directly
    req.setMethod("11111111");
    sendRequest(req, false);
  }

  EXPECT_CALL(mockController_, getParseErrorHandler(_, _, _))
      .WillRepeatedly(Return(nullptr));

  flushRequestsAndLoop();
  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, DirectErrorHandlingLimitExceeded) {
  httpSession_->setRateLimitParams(RateLimitFilter::Type::DIRECT_ERROR_HANDLING,
                                   10,
                                   std::chrono::milliseconds(0));

  // Send eleven messages, each of which causes direct error handling. Since
  // this exceeds the limit, the connection should be dropped.
  for (uint32_t i = 0;
       i < DirectErrorsRateLimitFilter::kMaxEventsPerIntervalLowerBound + 1;
       i++) {
    auto req = getGetRequest();
    // Invalid method, causes the error to be handled directly
    req.setMethod("11111111");
    sendRequest(req, false);
  }

  EXPECT_CALL(mockController_, getParseErrorHandler(_, _, _))
      .WillRepeatedly(Return(nullptr));

  expectDetachSession();
  flushRequestsAndLoopN(2);
}

TEST_F(HTTP2DownstreamSessionTest, TestPriorityWeightsTinyWindow) {
  httpSession_->setWriteBufferLimit(2 * 65536);
  InSequence enforceOrder;
  auto id1 = sendRequest();
  auto id2 = sendRequest();

  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders();
  handler1->expectEOM([&] { handler1->sendReplyWithBody(200, 32 * 1024); });
  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] { handler2->sendReplyWithBody(200, 32 * 1024); });

  handler1->expectDetachTransaction();

  // twice- once to send and once to receive
  flushRequestsAndLoopN(2);
  EXPECT_CALL(callbacks_, onSettings(_));
  EXPECT_CALL(callbacks_, onMessageBegin(id1, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(id1, _));
  EXPECT_CALL(callbacks_, onMessageBegin(id2, _));
  EXPECT_CALL(callbacks_, onHeadersComplete(id2, _));
  for (auto i = 0; i < 7; i++) {
    EXPECT_CALL(callbacks_, onBody(id1, _, _))
        .WillOnce(ExpectBodyLen(4 * 1024));
    EXPECT_CALL(callbacks_, onBody(id2, _, _))
        .WillOnce(ExpectBodyLen(4 * 1024));
  }
  EXPECT_CALL(callbacks_, onBody(id1, _, _))
      .WillOnce(ExpectBodyLen(4 * 1024 - 1));
  EXPECT_CALL(callbacks_, onBody(id2, _, _))
      .WillOnce(ExpectBodyLen(4 * 1024 - 1));
  EXPECT_CALL(callbacks_, onBody(id1, _, _)).WillOnce(ExpectBodyLen(1));
  EXPECT_CALL(callbacks_, onMessageComplete(id1, _));
  parseOutput(*clientCodec_);

  // open the window
  clientCodec_->generateWindowUpdate(requests_, 0, 100);
  handler2->expectDetachTransaction();
  flushRequestsAndLoopN(2);

  EXPECT_CALL(callbacks_, onBody(id2, _, _)).WillOnce(ExpectBodyLen(1));
  EXPECT_CALL(callbacks_, onMessageComplete(id2, _));
  parseOutput(*clientCodec_);

  httpSession_->closeWhenIdle();
  expectDetachSession();
  this->eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, TestShortContentLength) {
  auto req = getPostRequest(10);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(20), HTTPCodec::NoPadding, true);
  auto handler1 = addSimpleStrictHandler();

  InSequence enforceOrder;
  handler1->expectHeaders();
  handler1->expectError([&handler1](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorParseBody);
    handler1->txn_->sendAbort();
  });
  handler1->expectDetachTransaction();
  flushRequestsAndLoop();

  gracefulShutdown();
}

/**
 * If handler chooses to untie itself with transaction during onError,
 * detachTransaction shouldn't be expected
 */
TEST_F(HTTP2DownstreamSessionTest, TestBadContentLengthUntieHandler) {
  auto req = getPostRequest(10);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(20), HTTPCodec::NoPadding, true);
  auto handler1 = addSimpleStrictHandler();

  InSequence enforceOrder;
  handler1->expectHeaders();
  handler1->expectError([&](const HTTPException&) {
    if (handler1->txn_) {
      handler1->txn_->setHandler(nullptr);
    }
    handler1->txn_ = nullptr;
  });
  flushRequestsAndLoop();

  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestLongContentLength) {
  auto req = getPostRequest(30);
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(20), HTTPCodec::NoPadding, true);
  auto handler1 = addSimpleStrictHandler();

  InSequence enforceOrder;
  handler1->expectHeaders();
  handler1->expectBody();
  handler1->expectError([&handler1](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorParseBody);
    handler1->txn_->sendAbort();
  });
  handler1->expectDetachTransaction();
  flushRequestsAndLoop();

  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestMalformedContentLength) {
  auto req = getPostRequest();
  req.getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, "malformed");
  auto streamID = sendRequest(req, false);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(20), HTTPCodec::NoPadding, true);
  auto handler1 = addSimpleStrictHandler();

  InSequence enforceOrder;
  handler1->expectHeaders();
  handler1->expectBody();
  handler1->expectEOM([&handler1] { handler1->sendReplyWithBody(200, 100); });
  handler1->expectDetachTransaction();
  flushRequestsAndLoop();

  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestHeadContentLength) {
  InSequence enforceOrder;
  auto req = getGetRequest();
  req.setMethod(HTTPMethod::HEAD);
  sendRequest(req);
  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders();
  handler1->expectEOM([&handler1] {
    handler1->sendHeaders(200, 100);
    // no body for head
    handler1->txn_->sendEOM();
  });
  handler1->expectDetachTransaction();
  flushRequestsAndLoop();

  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, Test304ContentLength) {
  // This test covers egress, where we log, but don't fail or crash,
  InSequence enforceOrder;
  auto req = getGetRequest();
  req.setMethod(HTTPMethod::GET);
  sendRequest(req);
  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders();
  handler1->expectEOM([&handler1] {
    handler1->sendHeaders(304, 100);
    handler1->txn_->sendEOM();
  });
  handler1->expectDetachTransaction();
  flushRequestsAndLoop();

  gracefulShutdown();
}

// chunked with wrong content-length
TEST_F(HTTPDownstreamSessionTest, HttpShortContentLength) {
  InSequence enforceOrder;
  auto req = getPostRequest(10);
  req.setIsChunked(true);
  req.getHeaders().add(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
  auto streamID = sendRequest(req, false);
  clientCodec_->generateChunkHeader(requests_, streamID, 20);
  clientCodec_->generateBody(
      requests_, streamID, makeBuf(20), HTTPCodec::NoPadding, false);
  clientCodec_->generateChunkTerminator(requests_, streamID);
  clientCodec_->generateEOM(requests_, streamID);
  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders();
  EXPECT_CALL(*handler1, _onChunkHeader(20));

  handler1->expectError([&handler1](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorParseBody);
    handler1->txn_->sendAbort();
  });
  handler1->expectDetachTransaction();
  expectDetachSession();
  flushRequestsAndLoop();
}

TEST_F(HTTP2DownstreamSessionTest, TestSessionStallByFlowControl) {
  NiceMock<MockHTTPSessionStats> stats;
  // By default the send and receive windows are 64K each.
  // If we use only a single transaction, that transaction
  // will be paused on reaching 64K. Therefore, to pause the session,
  // it is used 2 transactions each sending 32K.

  // Make write buffer limit exceding 64K, for example 128K
  httpSession_->setWriteBufferLimit(128 * 1024);
  httpSession_->setSessionStats(&stats);

  InSequence enforceOrder;
  sendRequest();
  sendRequest();

  auto handler1 = addSimpleStrictHandler();

  handler1->expectHeaders();
  handler1->expectEOM([&] { handler1->sendReplyWithBody(200, 32 * 1024); });

  auto handler2 = addSimpleStrictHandler();
  handler2->expectHeaders();
  handler2->expectEOM([&] { handler2->sendReplyWithBody(200, 32 * 1024); });

  EXPECT_CALL(stats, _recordSessionStalled()).Times(1);

  handler1->expectDetachTransaction();

  // twice- once to send and once to receive
  flushRequestsAndLoopN(2);

  // open the window
  clientCodec_->generateWindowUpdate(requests_, 0, 100);
  handler2->expectDetachTransaction();
  flushRequestsAndLoopN(2);

  httpSession_->closeWhenIdle();
  expectDetachSession();
  flushRequestsAndLoop();
}

TEST_F(HTTP2DownstreamSessionTest, TestTransactionStallByFlowControl) {
  NiceMock<MockHTTPSessionStats> stats;

  httpSession_->setSessionStats(&stats);

  // Set the client side stream level flow control wind to 500 bytes,
  // and try to send 1000 bytes through it.
  // Then the flow control kicks in and stalls the transaction.
  clientCodec_->getEgressSettings()->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                                                500);
  clientCodec_->generateSettings(requests_);

  auto streamID = sendRequest();

  EXPECT_CALL(stats, _recordTransactionOpened());

  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&] { handler->sendReplyWithBody(200, 1000); });

  EXPECT_CALL(stats, _recordTransactionStalled());
  handler->expectEgressPaused();

  handler->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
    ASSERT_EQ(folly::to<std::string>("ingress timeout, streamID=", streamID),
              std::string(ex.what()));
    handler->terminate();
  });

  handler->expectDetachTransaction();

  EXPECT_CALL(stats, _recordTransactionClosed());

  flushRequestsAndLoop();
  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestTransactionNotStallByFlowControl) {
  NiceMock<MockHTTPSessionStats> stats;

  httpSession_->setSessionStats(&stats);

  clientCodec_->getEgressSettings()->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                                                500);
  clientCodec_->generateSettings(requests_);

  sendRequest();

  EXPECT_CALL(stats, _recordTransactionOpened());

  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&] { handler->sendReplyWithBody(200, 500); });

  // The egtress paused is notified due to existing logics,
  // but egress transaction should not be counted as stalled by flow control,
  // because there is nore more bytes to send
  handler->expectEgressPaused();

  handler->expectDetachTransaction();

  EXPECT_CALL(stats, _recordTransactionClosed());

  flushRequestsAndLoop();
  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestSetEgressSettings) {
  SettingsList settings = {{SettingsId::HEADER_TABLE_SIZE, 5555},
                           {SettingsId::MAX_FRAME_SIZE, 16384},
                           {SettingsId::ENABLE_PUSH, 1}};

  const HTTPSettings* codecSettings = rawCodec_->getEgressSettings();
  for (const auto& setting : settings) {
    const HTTPSetting* currSetting = codecSettings->getSetting(setting.id);
    if (currSetting) {
      EXPECT_EQ(setting.value, currSetting->value);
    }
  }

  flushRequestsAndLoop();
  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestDuplicateRequestStream) {
  // Send the following:
  // HEADERS id=1
  // HEADERS id=2
  // HEADERS id=1 (trailers)
  // HEADERS id=2 -> contains pseudo-headers after EOM so ignored
  auto handler2 = addSimpleStrictHandler();
  auto handler1 = addSimpleStrictHandler();
  auto streamID1 = sendRequest("/withtrailers", 0, false);
  auto streamID2 = sendRequest();
  HTTPHeaders trailers;
  trailers.add("Foo", "Bar");
  // generate trailers includes FIN=true
  clientCodec_->generateTrailers(requests_, streamID1, trailers);

  clientCodec_->generateHeader(requests_, streamID2, getGetRequest(), false);
  handler1->expectHeaders();
  handler2->expectHeaders();
  handler2->expectEOM();
  handler1->expectTrailers();
  handler1->expectEOM([&] {
    handler1->sendReplyWithBody(200, 100);
    // 2 got an error after EOM, which gets ignored - need a response to
    // cleanly terminate it
    handler2->sendReplyWithBody(200, 100);
  });
  handler2->expectError();
  handler1->expectDetachTransaction();
  handler2->expectDetachTransaction();
  flushRequestsAndLoop();
  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, TestPingPreserveData) {
  auto pingData = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
  clientCodec_->generatePingRequest(requests_, pingData);
  EXPECT_CALL(callbacks_, onPingReply(pingData));
  flushRequestsAndLoop();
  parseOutput(*clientCodec_);
  gracefulShutdown();
}

TEST_F(HTTP2DownstreamSessionTest, DropConnectionWithPendingShutdownCallback) {
  // A request to set up EOM writing in the loop later.
  auto handler = addSimpleStrictHandler();
  sendRequest();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  flushRequestsAndLoopN(1);
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);

  // To avoid onWriteComplete which will shutdownTransport
  transport_->pauseWrites();

  // To schedule the ShutdownTransportCallback
  flushRequestsAndLoopN(1, true);

  expectDetachSession();
  httpSession_->dropConnection("Async drop");
}

TEST_F(HTTP2DownstreamSessionTest, DropAlreadyShuttingDownConnection) {
  // A request to set up EOM writing in the loop later.
  auto handler = addSimpleStrictHandler();
  sendRequest();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  flushRequestsAndLoopN(1);
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);

  // This would shutdownTransport
  flushRequestsAndLoopN(1, true);

  expectDetachSession();
  httpSession_->dropConnection("Async drop");
}

TEST_F(HTTP2DownstreamSessionTest, PingProbes) {
  // Send an immediate ping probe, and send a reply
  httpSession_->enablePingProbes(std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 /*extendIntervalOnIngress=*/true,
                                 /*immediate=*/true);
  MockHTTPSessionStats stats;
  httpSession_->setSessionStats(&stats);
  eventBase_.loopOnce();
  uint64_t pingVal = 0;
  EXPECT_CALL(callbacks_, onPingRequest(_)).WillOnce(SaveArg<0>(&pingVal));
  EXPECT_CALL(stats, _recordSessionPeriodicPingProbeTimeout()).Times(0);
  parseOutput(*clientCodec_);
  clientCodec_->generatePingReply(requests_, pingVal);
  flushRequestsAndLoopN(1);
  httpSession_->closeWhenIdle();
  expectDetachSession();
  flushRequestsAndLoopN(1);
}

TEST_F(HTTP2DownstreamSessionTest, PingProbeTimeout) {
  // Send an immediate ping probe, but don't reply.  Connection is dropped.
  httpSession_->enablePingProbes(std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 /*extendIntervalOnIngress=*/true,
                                 /*immediate=*/true);
  MockHTTPSessionStats stats;
  httpSession_->setSessionStats(&stats);
  eventBase_.loopOnce();
  uint64_t pingVal = 0;
  EXPECT_CALL(callbacks_, onPingRequest(_)).WillOnce(SaveArg<0>(&pingVal));
  EXPECT_CALL(stats, _recordSessionPeriodicPingProbeTimeout()).Times(1);
  parseOutput(*clientCodec_);
  expectDetachSession();
  flushRequestsAndLoop();
}

TEST_F(HTTP2DownstreamSessionTest, PingProbeTimeoutRefresh) {
  httpSession_->enablePingProbes(std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 /*extendIntervalOnIngress=*/true,
                                 /*immediate=*/false);
  MockHTTPSessionStats stats;
  httpSession_->setSessionStats(&stats);
  EXPECT_CALL(stats, _recordSessionPeriodicPingProbeTimeout()).Times(1);
  // Don't send an immediate probe.  Send a request after 250ms, which starts
  // the probe interval timer. The ping probe interval fires at 1250 and times
  // out at 2250.
  proxygen::TimePoint start = getCurrentTime();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  eventBase_.runAfterDelay(
      [this] {
        sendRequest();
        flushRequests();
      },
      250);
  handler->expectError();
  handler->expectDetachTransaction();
  expectDetachSession();
  eventBase_.loop();
  auto duration = millisecondsBetween(getCurrentTime(), start);
  EXPECT_GE(duration.count(), 2250);
}

TEST_F(HTTP2DownstreamSessionTest, PingProbeInvalid) {
  // Send an immediate ping probe, send a reply with a different value.
  // It doesn't drop the connection.
  httpSession_->enablePingProbes(std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 /*extendIntervalOnIngress=*/true,
                                 /*immediate=*/true);
  eventBase_.loopOnce();
  uint64_t pingVal = 0;
  EXPECT_CALL(callbacks_, onPingRequest(_)).WillOnce(SaveArg<0>(&pingVal));
  parseOutput(*clientCodec_);
  // Send a reply for a ping the prober didn't send (we could send one with
  // sendPing, but meh)
  clientCodec_->generatePingReply(requests_, pingVal + 1);
  flushRequestsAndLoopN(1);
  httpSession_->closeWhenIdle();
  expectDetachSession();
  eventBase_.loop();
}

TEST_F(HTTP2DownstreamSessionTest, CancelPingProbesOnRequest) {
  // Send an immediate ping probe, don't reply, but send a request/response.
  // When the session goes idle, the ping probe timeout should be cancelled.
  httpSession_->enablePingProbes(std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 /*extendIntervalOnIngress=*/true,
                                 /*immediate=*/true);
  eventBase_.loopOnce();
  auto handler = addSimpleStrictHandler();
  sendRequest();
  handler->expectHeaders();
  handler->expectEOM([&handler, this] {
    handler->sendReplyWithBody(200, 100);
    eventBase_.runAfterDelay([this] { httpSession_->timeoutExpired(); }, 1250);
  });
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);
  expectDetachSession();
  flushRequestsAndLoop();
  EXPECT_CALL(callbacks_, onPingRequest(_));
  parseOutput(*clientCodec_);
  // Session idle times out
  EXPECT_EQ(httpSession_->getConnectionCloseReason(),
            ConnectionCloseReason::TIMEOUT);
}

TEST_F(HTTP2DownstreamSessionTest, Observer_Attach_Detach_Destroy) {
  MockSessionObserver::EventSet eventSet;

  // Test attached/detached callbacks when adding/removing observers
  {
    auto observer = addMockSessionObserver(eventSet);
    EXPECT_CALL(*observer, detached(_));
    httpSession_->removeObserver(observer.get());
  }

  // Test destroyed callback when session is destroyed
  {
    auto observer = addMockSessionObserver(eventSet);
    auto handler = addSimpleStrictHandler();
    handler->expectHeaders();
    handler->expectEOM([&handler]() {
      handler->sendReplyWithBody(200 /* status code */,
                                 100 /* content size */,
                                 true /* keepalive */,
                                 true /* sendEOM */,
                                 false /*trailers*/);
    });
    handler->expectDetachTransaction();
    HTTPSession::DestructorGuard g(httpSession_);
    HTTPMessage req = getGetRequest();
    sendRequest(req);
    flushRequestsAndLoop(true, milliseconds(0));

    EXPECT_CALL(*observer, destroyed(_, _));
    expectDetachSession();
    httpSession_->closeWhenIdle();
  }
}

TEST_F(HTTP2DownstreamSessionTest, Observer_Attach_Detach_Destroy_Shared) {
  MockSessionObserver::EventSet eventSet;

  // Test attached/detached callbacks when adding/removing observers
  {
    auto observer = addMockSessionObserverShared(eventSet);
    EXPECT_CALL(*observer, detached(_));
    httpSession_->removeObserver(observer.get());
  }

  // Test destroyed callback when session is destroyed
  {
    auto observer = addMockSessionObserverShared(eventSet);
    auto handler = addSimpleStrictHandler();
    handler->expectHeaders();
    handler->expectEOM([&handler]() {
      handler->sendReplyWithBody(200 /* status code */,
                                 100 /* content size */,
                                 true /* keepalive */,
                                 true /* sendEOM */,
                                 false /*trailers*/);
    });
    handler->expectDetachTransaction();
    HTTPSession::DestructorGuard g(httpSession_);
    HTTPMessage req = getGetRequest();
    sendRequest(req);
    flushRequestsAndLoop(true, milliseconds(0));

    EXPECT_CALL(*observer, destroyed(_, _));
    expectDetachSession();
    httpSession_->closeWhenIdle();
  }
}

TEST_F(HTTP2DownstreamSessionTest, Observer_RequestStarted) {

  // Add an observer NOT subscribed to the RequestStarted event
  auto observerUnsubscribed =
      addMockSessionObserver(MockSessionObserver::EventSetBuilder().build());
  httpSession_->addObserver(observerUnsubscribed.get());

  // Add an observer subscribed to this event
  auto observerSubscribed = addMockSessionObserver(
      MockSessionObserver::EventSetBuilder()
          .enable(HTTPSessionObserverInterface::Events::requestStarted)
          .build());
  httpSession_->addObserver(observerSubscribed.get());

  EXPECT_CALL(*observerUnsubscribed, requestStarted(_, _)).Times(0);

  // Subscribed observer expects to receive RequestStarted callback, with a
  // request header x-meta-test-header and value "abc123"
  EXPECT_CALL(*observerSubscribed, requestStarted(_, _))
      .WillOnce(Invoke(
          [](HTTPSessionObserverAccessor*,
             const proxygen::HTTPSessionObserverInterface::RequestStartedEvent&
                 event) {
            auto hdrs = event.requestHeaders;
            EXPECT_EQ(hdrs.getSingleOrEmpty("x-meta-test-header"), "abc123");
          }));

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler]() {
    handler->sendReplyWithBody(200 /* status code */,
                               100 /* content size */,
                               true /* keepalive */,
                               true /* sendEOM */,
                               false /*trailers*/);
  });
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);
  HTTPMessage req = getGetRequest();
  req.getHeaders().add("x-meta-test-header", "abc123");
  sendRequest(req);

  flushRequestsAndLoop(true, milliseconds(0));

  expectDetachSession();
}

TEST_F(HTTP2DownstreamSessionTest, Observer_SendPingByObserver) {

  // Add an observer subscribed to this event
  auto observerSubscribed = addMockSessionObserver(
      MockSessionObserver::EventSetBuilder()
          .enable(HTTPSessionObserverInterface::Events::preWrite)
          .build());
  httpSession_->addObserver(observerSubscribed.get());

  // Subscribed observer expects to receive PreWrite callback
  // Check if transactions are about to write a threshold amount of bytes
  auto pingData = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
  EXPECT_CALL(*observerSubscribed, preWrite(_, _))
      .WillOnce(Invoke(
          [&pingData](
              HTTPSessionObserverAccessor* sessionObserverAccessor_,
              const proxygen::HTTPSessionObserverInterface::PreWriteEvent&
                  event) {
            EXPECT_EQ(event.pendingEgressBytes, 100);
            EXPECT_THAT(sessionObserverAccessor_, NotNull());
            sessionObserverAccessor_->sendPing(pingData);
          }));

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler]() {
    handler->sendReplyWithBody(200 /* status code */,
                               100 /* content size */,
                               true /* keepalive */,
                               true /* sendEOM */,
                               false /*trailers*/);
  });
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);
  HTTPMessage req = getGetRequest();
  sendRequest(req);

  flushRequestsAndLoop(true, milliseconds(0));
  expectDetachSession();
  NiceMock<MockHTTPCodecCallback> callbacks;
  clientCodec_->setCallback(&callbacks);

  InSequence enforceOrder;
  EXPECT_CALL(callbacks, onPingRequest(pingData));

  parseOutput(*clientCodec_);
}

TEST_F(HTTP2DownstreamSessionTest, Observer_PingReply) {

  // Add an observer subscribed to this event
  auto observerSubscribed = addMockSessionObserver(
      MockSessionObserver::EventSetBuilder()
          .enable(HTTPSessionObserverInterface::Events::pingReply)
          .build());
  httpSession_->addObserver(observerSubscribed.get());
  uint64_t pingId = 0;
  EXPECT_CALL(callbacks_, onPingRequest(_)).WillOnce(SaveArg<0>(&pingId));
  httpSession_->sendPing();
  eventBase_.loopOnce();
  EXPECT_CALL(*observerSubscribed, pingReply(_, _))
      .WillOnce(Invoke(
          [&pingId](
              HTTPSessionObserverAccessor* sessionObserverAccessor_,
              const proxygen::HTTPSessionObserverInterface::PingReplyEvent&
                  event) {
            EXPECT_EQ(event.id, pingId);
            EXPECT_THAT(sessionObserverAccessor_, NotNull());
          }));
  parseOutput(*clientCodec_);
  clientCodec_->generatePingReply(requests_, pingId);
  flushRequestsAndLoopN(1);
  httpSession_->closeWhenIdle();
  expectDetachSession();
  flushRequestsAndLoopN(1);
}

TEST_F(HTTP2DownstreamSessionTest, Observer_PreWrite) {

  // Add an observer NOT subscribed to the PreWrite event
  auto observerUnsubscribed =
      addMockSessionObserver(MockSessionObserver::EventSetBuilder().build());
  httpSession_->addObserver(observerUnsubscribed.get());

  // Add an observer subscribed to this event
  auto observerSubscribed = addMockSessionObserver(
      MockSessionObserver::EventSetBuilder()
          .enable(HTTPSessionObserverInterface::Events::preWrite)
          .build());
  httpSession_->addObserver(observerSubscribed.get());

  EXPECT_CALL(*observerUnsubscribed, preWrite(_, _)).Times(0);

  // Subscribed observer expects to receive PreWrite callback
  // Check if transactions are about to write a threshold amount of bytes
  EXPECT_CALL(*observerSubscribed, preWrite(_, _))
      .WillOnce(
          Invoke([](HTTPSessionObserverAccessor*,
                    const proxygen::HTTPSessionObserverInterface::PreWriteEvent&
                        event) { EXPECT_EQ(event.pendingEgressBytes, 100); }));

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler]() {
    handler->sendReplyWithBody(200 /* status code */,
                               100 /* content size */,
                               true /* keepalive */,
                               true /* sendEOM */,
                               false /*trailers*/);
  });
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(httpSession_);
  HTTPMessage req = getGetRequest();
  sendRequest(req);

  flushRequestsAndLoop(true, milliseconds(0));

  expectDetachSession();
}
