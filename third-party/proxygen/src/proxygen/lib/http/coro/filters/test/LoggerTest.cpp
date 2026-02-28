/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/Logger.h"
#include "proxygen/lib/http/codec/test/TestUtils.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"

#include "proxygen/lib/http/coro/test/Mocks.h"
#include <folly/coro/GmockHelpers.h>
#include <folly/coro/GtestHelpers.h>

using namespace testing;

namespace {
std::chrono::steady_clock::time_point nullTime;
using proxygen::coro::Logger;
using TestLoggerCb = std::function<void(const Logger&, uint32_t weight)>;

class TestLogger : public Logger::SampledLogger {
 public:
  TestLogger(TestLoggerCb&& logImpl, double rate)
      : logImpl_(std::move(logImpl)), rate_(rate) {
  }

  uint32_t getLoggingWeight(bool error) override {
    proxygen::Sampling sampling{rate_};
    return sampling.isLucky() ? sampling.getWeight() : 0;
  }

  void log(const Logger& logger, uint32_t weight) override {
    if (logImpl_) {
      logImpl_(logger, weight);
    }
  }

  TestLoggerCb logImpl_{nullptr};
  double rate_{0.0};
};

std::shared_ptr<Logger::SampledLogger> makeTestLogger(TestLoggerCb&& logImpl,
                                                      double rate = 1.0) {
  return std::make_shared<TestLogger>(std::move(logImpl), rate);
}

} // namespace

namespace proxygen::coro::test {

class LoggerTest : public testing::Test {
 public:
  void SetUp() override {
    EXPECT_CALL(mockSession_, getSessionID())
        .WillRepeatedly(Return(0xfaceb00c));
    EXPECT_CALL(mockSession_, getCodecProtocol())
        .WillRepeatedly(Return(CodecProtocol::HTTP_3));
    EXPECT_CALL(mockSession_, getLocalAddress())
        .WillRepeatedly(ReturnRef(localAddr_));
    EXPECT_CALL(mockSession_, getPeerAddress())
        .WillRepeatedly(ReturnRef(peerAddr_));
    tinfo_.secure = true;
    EXPECT_CALL(mockSession_, getSetupTransportInfo())
        .WillRepeatedly(ReturnRef(tinfo_));
    EXPECT_CALL(mockSession_, getCurrentTransportInfo(_, _))
        .WillRepeatedly(Invoke([](wangle::TransportInfo* tinfo, bool) {
          tinfo->rtt = std::chrono::microseconds(123);
          return true;
        }));
  }

  folly::coro::Task<void> run(HTTPSource* reqSource,
                              HTTPSource* respSource,
                              TestLoggerCb&& logFn,
                              double rate = 1.0) {
    Logger logger(mockSession_.acquireKeepAlive(),
                  makeTestLogger(std::move(logFn), rate),
                  /*logOnDestroy=*/true);
    HTTPSourceReader reqReader;
    reqReader.setSource(logger.getRequestFilter(reqSource));
    co_await reqReader.read();
    HTTPSourceReader respReader;
    respReader.setSource(logger.getResponseFilter(respSource));
    co_await co_awaitTry(respReader.read());
  }

 protected:
  MockHTTPSessionContext mockSession_;
  folly::SocketAddress localAddr_{"0.0.0.0", 1234};
  folly::SocketAddress peerAddr_{"0.0.0.0", 5678};
  wangle::TransportInfo tinfo_;
};

CO_TEST_F(LoggerTest, Basic) {
  co_await run(
      HTTPFixedSource::makeFixedRequest(
          URL("https://www.facebook.com/foo?param=value")),
      HTTPFixedSource::makeFixedResponse(200, "success"),
      [this](const Logger& logger, uint32_t weight) {
        EXPECT_EQ(weight, 1);
        EXPECT_FALSE(logger.reqFilter.streamID.hasValue());
        EXPECT_TRUE(logger.reqFilter.finalHeaderTime.hasValue());
        EXPECT_TRUE(logger.reqFilter.firstByteTime.hasValue());
        EXPECT_NE(logger.reqFilter.endTime, nullTime);
        EXPECT_FALSE(logger.reqFilter.error.hasValue());
        // header size not set with fixed req
        EXPECT_FALSE(logger.reqFilter.headerSize.hasValue());
        EXPECT_EQ(logger.reqFilter.httpVersion, HTTPMessage::kHTTPVersion11);
        EXPECT_EQ(logger.reqFilter.priority.urgency, 3);
        EXPECT_EQ(logger.reqFilter.method, "GET");
        EXPECT_EQ(logger.reqFilter.host, "www.facebook.com");
        EXPECT_EQ(logger.reqFilter.url, "/foo?param=value");
        EXPECT_EQ(logger.getAuthority(), "www.facebook.com");
        EXPECT_EQ(logger.getPath(), "/foo");
        EXPECT_EQ(logger.reqFilter.statusCode, folly::none);
        EXPECT_EQ(logger.reqFilter.bodyBytes, 0);

        EXPECT_FALSE(logger.respFilter.streamID.hasValue());
        EXPECT_TRUE(logger.respFilter.finalHeaderTime.hasValue());
        EXPECT_TRUE(logger.respFilter.firstByteTime.hasValue());
        EXPECT_NE(logger.respFilter.endTime, nullTime);
        EXPECT_FALSE(logger.respFilter.error.hasValue());
        // header size not set with fixed req
        EXPECT_FALSE(logger.respFilter.headerSize.hasValue());
        EXPECT_EQ(logger.respFilter.httpVersion, HTTPMessage::kHTTPVersion11);
        EXPECT_EQ(logger.respFilter.priority.urgency, 3);
        EXPECT_TRUE(logger.respFilter.method.empty());
        EXPECT_TRUE(logger.respFilter.host.empty());
        EXPECT_TRUE(logger.respFilter.url.empty());
        EXPECT_EQ(logger.respFilter.statusCode, 200);
        EXPECT_EQ(logger.respFilter.bodyBytes, 7);

        EXPECT_EQ(logger.localAddr, localAddr_);
        EXPECT_EQ(logger.peerAddr, peerAddr_);
        EXPECT_EQ(logger.protocol, CodecProtocol::HTTP_3);
        EXPECT_EQ(logger.sessionID, 0xfaceb00c);
        EXPECT_GE(logger.timeToFirstHeaderByte().count(), 0);
        EXPECT_GE(logger.timeToFirstByte().count(), 0);
        EXPECT_GE(logger.timeToLastByte().count(), 0);
        EXPECT_GE(logger.timeToLastByte().count(), 0);
        EXPECT_TRUE(logger.transportInfo.secure);
        EXPECT_EQ(logger.transportInfo.rtt.count(), 123);
      });
}

CO_TEST_F(LoggerTest, StreamIDAndIngressHeaderSize) {
  MockHTTPSource mockSource;
  EXPECT_CALL(mockSource, readHeaderEvent())
      .WillOnce(folly::coro::gmock_helpers::CoInvoke(
          [&]() -> folly::coro::Task<HTTPHeaderEvent> {
            auto msg = std::make_unique<HTTPMessage>(getGetRequest("/"));
            HTTPHeaderSize size;
            size.uncompressed = 100;
            size.compressed = 50;
            msg->setIngressHeaderSize(size);
            co_return HTTPHeaderEvent(std::move(msg), true);
          }));
  EXPECT_CALL(mockSource, getStreamID()).WillOnce(Return(7));
  co_await run(&mockSource,
               HTTPFixedSource::makeFixedResponse(200, "success"),
               [](const Logger& logger, uint32_t weight) {
                 EXPECT_EQ(weight, 1);
                 EXPECT_EQ(logger.getStreamID(), 7);
                 EXPECT_EQ(logger.reqFilter.headerSize->uncompressed, 100);
                 EXPECT_EQ(logger.reqFilter.headerSize->compressed, 50);
               });
}

CO_TEST_F(LoggerTest, StopReading) {
  Logger logger(mockSession_.acquireKeepAlive(),
                makeTestLogger([](const Logger& logger, uint32_t weight) {
                  EXPECT_EQ(weight, 1);
                  EXPECT_EQ(logger.reqFilter.headerSize->uncompressed, 100);
                  EXPECT_EQ(logger.reqFilter.headerSize->compressed, 50);
                  EXPECT_NE(logger.reqFilter.endTime, nullTime);
                }));
  HTTPSourceHolder reqSource(
      logger.getRequestFilter(HTTPFixedSource::makeFixedRequest(
          "http://www.facebook.com", HTTPMethod::POST, makeBuf(10))));
  auto headerEvent = co_await reqSource.readHeaderEvent();
  EXPECT_NE(headerEvent.egressHeadersFn, nullptr);
  HTTPHeaderSize size;
  size.uncompressed = 100;
  size.compressed = 50;
  headerEvent.egressHeadersFn(size);
}

CO_TEST_F(LoggerTest, HeadersError) {
  NiceMock<MockHTTPSource> mockSource;
  EXPECT_CALL(mockSource, readHeaderEvent())
      .WillOnce(folly::coro::gmock_helpers::CoInvoke(
          [&]() -> folly::coro::Task<HTTPHeaderEvent> {
            co_yield folly::coro::co_error(std::runtime_error("err"));
          }));
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/foo")),
      &mockSource,
      [](const Logger& logger, uint32_t weight) {
        EXPECT_EQ(weight, 1);
        EXPECT_TRUE(logger.respFilter.error.hasValue());
      });
}

CO_TEST_F(LoggerTest, BodyError) {
  NiceMock<MockHTTPSource> mockSource;
  EXPECT_CALL(mockSource, readHeaderEvent())
      .WillOnce(folly::coro::gmock_helpers::CoInvoke(
          [&]() -> folly::coro::Task<HTTPHeaderEvent> {
            co_return HTTPHeaderEvent(makeResponse(200), false);
          }));
  EXPECT_CALL(mockSource, readBodyEvent(_))
      .WillOnce(folly::coro::gmock_helpers::CoInvoke(
          [&](uint64_t) -> folly::coro::Task<HTTPBodyEvent> {
            co_yield folly::coro::co_error(std::runtime_error("err"));
          }));
  auto reqSource =
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/foo"));
  reqSource->msg_->getHeaders().remove(HTTP_HEADER_HOST);
  reqSource->msg_->setURL("https://www.facebook.com/foo");
  co_await run(
      reqSource, &mockSource, [](const Logger& logger, uint32_t weight) {
        // Sneak in a test for getAuthority for absolute URLs
        EXPECT_EQ(logger.getAuthority(), "www.facebook.com");
        EXPECT_EQ(logger.respFilter.statusCode, 200);
        EXPECT_TRUE(logger.respFilter.error.hasValue());
        EXPECT_EQ(weight, 1);
      });
}

CO_TEST_F(LoggerTest, DefaultLogger) {
  int oldLevel = FLAGS_v;
  FLAGS_v = 2;
  gflags::SetCommandLineOption("minloglevel", "0");
  co_await run(
      HTTPFixedSource::makeFixedRequest(URL("https://www.facebook.com/foo")),
      HTTPFixedSource::makeFixedResponse(200, "success"),
      nullptr);
  FLAGS_v = oldLevel;
  gflags::SetCommandLineOption("minloglevel", "0");
}

CO_TEST_F(LoggerTest, Weight) {
  // Set to 1%, weight = 100
  co_await run(
      HTTPFixedSource::makeFixedRequest(
          URL("https://www.facebook.com/foo?param=value")),
      HTTPFixedSource::makeFixedResponse(200, "success"),
      [](const Logger&, uint32_t weight) { EXPECT_EQ(weight, 100); },
      0.01);
  // Set to 50%, weight = 2
  co_await run(
      HTTPFixedSource::makeFixedRequest(
          URL("https://www.facebook.com/foo?param=value")),
      HTTPFixedSource::makeFixedResponse(200, "success"),
      [](const Logger&, uint32_t weight) { EXPECT_EQ(weight, 2); },
      0.5);
  // Error, weight = 1
  co_await run(HTTPFixedSource::makeFixedRequest(
                   URL("https://www.facebook.com/foo?param=value")),
               new HTTPErrorSource(HTTPError(HTTPErrorCode::INTERNAL_ERROR)),
               [](const Logger&, uint32_t weight) { EXPECT_EQ(weight, 1); });
}
// error reading headers
// error reading body
} // namespace proxygen::coro::test
