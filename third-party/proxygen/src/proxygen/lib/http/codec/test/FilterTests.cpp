/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <limits>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/DebugFilter.h>
#include <proxygen/lib/http/codec/FlowControlFilter.h>
#include <proxygen/lib/http/codec/HTTPChecks.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <random>

using namespace proxygen;
using namespace std;
using namespace testing;

namespace {
const uint32_t kInitialCapacity = 12345;

std::unique_ptr<folly::IOBuf> makeIOBuf(const std::string& str) {
  return folly::IOBuf::copyBuffer(str);
}
} // namespace

class MockFlowControlCallback : public FlowControlFilter::Callback {
 public:
  MOCK_METHOD(void, onConnectionSendWindowOpen, ());
  MOCK_METHOD(void, onConnectionSendWindowClosed, ());
};

class FilterTest : public testing::Test {
 public:
  FilterTest()
      : codec_(new MockHTTPCodec()), chain_(unique_ptr<HTTPCodec>(codec_)) {
    EXPECT_CALL(*codec_, setCallback(_))
        .WillRepeatedly(SaveArg<0>(&callbackStart_));
    chain_.setCallback(&callback_);
  }

 protected:
  MockHTTPCodec* codec_;
  HTTPCodec::Callback* callbackStart_;
  HTTPCodecFilterChain chain_;
  MockHTTPCodecCallback callback_;
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};
};

class HTTPChecksTest : public FilterTest {
 public:
  void SetUp() override {
    chain_.add<HTTPChecks>();
  }
};

class DebugFilterTest : public FilterTest {
 public:
  void SetUp() override {
    chain_.add<DebugFilter>(
        "trace-header", 200, [this](std::unique_ptr<folly::IOBuf> ingress) {
          dumpedIngress_.append(std::move(ingress));
        });
  }

 protected:
  folly::IOBufQueue dumpedIngress_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};
};

template <int initSize>
class FlowControlFilterTest : public FilterTest {
 public:
  void SetUp() override {
    EXPECT_CALL(*codec_, getDefaultWindowSize())
        .WillRepeatedly(Return(kInitialCapacity));

    if (initSize > kInitialCapacity) {
      // If the initial size is bigger than the default, a window update
      // will immediately be generated
      EXPECT_CALL(*codec_,
                  generateWindowUpdate(_, 0, initSize - kInitialCapacity))
          .WillOnce(InvokeWithoutArgs([this]() {
            writeBuf_.append(makeBuf(10));
            return 10;
          }));
    }
    EXPECT_CALL(*codec_, generateBody(_, _, _, _, _))
        .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                  HTTPCodec::StreamID /*stream*/,
                                  std::shared_ptr<folly::IOBuf> chain,
                                  folly::Optional<uint8_t> /*padding*/,
                                  bool /*eom*/) {
          auto len = chain->computeChainDataLength() + 4;
          writeBuf.append(makeBuf(len));
          return len;
        }));
    EXPECT_CALL(*codec_, isReusable()).WillRepeatedly(Return(true));

    // Construct flow control filter with capacity of 0, which will be
    // overridden to the codec default, which is the minimum
    filter_ = new FlowControlFilter(flowCallback_, writeBuf_, codec_, initSize);
    chain_.addFilters(std::unique_ptr<FlowControlFilter>(filter_));
  }
  StrictMock<MockFlowControlCallback> flowCallback_;
  FlowControlFilter* filter_;
  int recvWindow_{initSize};
};

using DefaultFlowControl = FlowControlFilterTest<0>;
using BigWindow = FlowControlFilterTest<1000000>;

MATCHER(IsFlowException, "") {
  return arg->hasCodecStatusCode() &&
         arg->getCodecStatusCode() == ErrorCode::FLOW_CONTROL_ERROR &&
         !arg->hasHttpStatusCode() && !arg->hasProxygenError();
}

TEST_F(DefaultFlowControl, FlowControlConstruct) {
  // Constructing the filter with a low capacity defaults to kInitialCapacity
  // initial capacity, so no window update should have been generated in
  // the constructor
  InSequence enforceSequence;
  ASSERT_EQ(writeBuf_.chainLength(), 0);

  // Our send window is limited to kInitialCapacity
  chain_->generateBody(
      writeBuf_, 1, makeBuf(kInitialCapacity - 1), HTTPCodec::NoPadding, false);

  // the window isn't full yet, so getting a window update shouldn't give a
  // callback informing us that it is open again
  callbackStart_->onWindowUpdate(0, 1);

  // Now fill the window (2 more bytes)
  EXPECT_CALL(flowCallback_, onConnectionSendWindowClosed());
  chain_->generateBody(writeBuf_, 1, makeBuf(2), HTTPCodec::NoPadding, false);
  // get the callback informing the window is open once we get a window update
  EXPECT_CALL(flowCallback_, onConnectionSendWindowOpen());
  callbackStart_->onWindowUpdate(0, 1);

  // Overflowing the window is fatal. Write 2 bytes (only 1 byte left in window)
  EXPECT_DEATH_NO_CORE(
      chain_->generateBody(
          writeBuf_, 1, makeBuf(2), HTTPCodec::NoPadding, false),
      ".*");
}

TEST_F(DefaultFlowControl, SendUpdate) {
  // Make sure we send a window update when the window decreases below half
  InSequence enforceSequence;
  EXPECT_CALL(callback_, onBody(_, _, _)).WillRepeatedly(Return());

  // Have half the window outstanding
  callbackStart_->onBody(1, makeBuf(kInitialCapacity / 2 + 1), 0);
  filter_->ingressBytesProcessed(writeBuf_, kInitialCapacity / 2);

  // It should wait until the "+1" is ack'd to generate the coallesced update
  EXPECT_CALL(*codec_, generateWindowUpdate(_, 0, kInitialCapacity / 2 + 1));
  filter_->ingressBytesProcessed(writeBuf_, 1);
}

TEST_F(BigWindow, RecvTooMuch) {
  // Constructing the filter with a large capacity causes a WINDOW_UPDATE
  // for stream zero to be generated
  ASSERT_GT(writeBuf_.chainLength(), 0);

  InSequence enforceSequence;
  EXPECT_CALL(callback_, onBody(_, _, _));
  EXPECT_CALL(callback_, onError(0, IsFlowException(), _))
      .WillOnce(Invoke([](HTTPCodec::StreamID,
                          std::shared_ptr<HTTPException> exc,
                          bool /*newTxn*/) {
        ASSERT_EQ(
            "Failed to reserve receive window, window size=0, "
            "amount=1",
            std::string(exc->what()));
      }));

  // Receive the max amount advertised
  callbackStart_->onBody(1, makeBuf(recvWindow_), 0);
  ASSERT_TRUE(chain_->isReusable());
  // Receive 1 byte too much
  callbackStart_->onBody(1, makeBuf(1), 0);
  ASSERT_FALSE(chain_->isReusable());
}

TEST_F(BigWindow, RemoteIncrease) {
  // The remote side sends us a window update for stream=0, increasing our
  // available window
  InSequence enforceSequence;

  ASSERT_EQ(filter_->getAvailableSend(), kInitialCapacity);
  callbackStart_->onWindowUpdate(0, 10);
  ASSERT_EQ(filter_->getAvailableSend(), kInitialCapacity + 10);

  EXPECT_CALL(flowCallback_, onConnectionSendWindowClosed());
  chain_->generateBody(writeBuf_,
                       1,
                       makeBuf(kInitialCapacity + 10),
                       HTTPCodec::NoPadding,
                       false);
  ASSERT_EQ(filter_->getAvailableSend(), 0);

  // Now the remote side sends a HUGE update (just barely legal)
  // Since the window was full, this generates a callback from the filter
  // telling us the window is no longer full.
  EXPECT_CALL(flowCallback_, onConnectionSendWindowOpen());
  callbackStart_->onWindowUpdate(0, std::numeric_limits<int32_t>::max());
  ASSERT_EQ(filter_->getAvailableSend(), std::numeric_limits<int32_t>::max());

  // Now overflow it by 1
  EXPECT_CALL(callback_, onError(0, IsFlowException(), _))
      .WillOnce(Invoke([](HTTPCodec::StreamID,
                          std::shared_ptr<HTTPException> exc,
                          bool /*newTxn*/) {
        ASSERT_EQ(
            "Failed to update send window, outstanding=0, "
            "amount=1",
            std::string(exc->what()));
      }));
  callbackStart_->onWindowUpdate(0, 1);
  ASSERT_FALSE(chain_->isReusable());
}

TEST_F(HTTPChecksTest, SendTraceBodyDeath) {
  // It is NOT allowed to send a TRACE with a body.

  HTTPMessage msg = getPostRequest();
  msg.setMethod("TRACE");

  EXPECT_DEATH_NO_CORE(chain_->generateHeader(writeBuf_, 0, msg), ".*");
}

TEST_F(HTTPChecksTest, SendGetBody) {
  // It is allowed to send a GET with a content-length. It is up to the
  // server to ignore it.

  EXPECT_CALL(*codec_, generateHeader(_, _, _, _, _, _));

  HTTPMessage msg = getPostRequest();
  msg.setMethod("GET");

  chain_->generateHeader(writeBuf_, 0, msg);
}

TEST_F(HTTPChecksTest, RecvTraceBody) {
  // In proxygen, we deal with receiving a TRACE with a body by 400'ing it

  EXPECT_CALL(callback_, onError(_, _, _))
      .WillOnce(Invoke([](HTTPCodec::StreamID,
                          std::shared_ptr<HTTPException> exc,
                          bool newTxn) {
        ASSERT_TRUE(newTxn);
        ASSERT_EQ(exc->getHttpStatusCode(), 400);
        ASSERT_EQ(0, strcmp("RFC2616: Request Body Not Allowed", exc->what()));
      }));

  auto msg = makePostRequest();
  msg->setMethod("TRACE");

  callbackStart_->onHeadersComplete(0, std::move(msg));
}

TEST_F(DebugFilterTest, NoError) {
  chain_->onIngress(*makeIOBuf("foo"));
  chain_->onIngressEOF();
  callbackStart_->onMessageBegin(1, nullptr);
  callbackStart_->onHeadersComplete(1, makeGetRequest());
  callbackStart_->onMessageComplete(1, false);
  EXPECT_TRUE(dumpedIngress_.empty());
}

TEST_F(DebugFilterTest, NoErrorGoaway) {
  chain_->onIngress(*makeIOBuf("foo"));
  callbackStart_->onGoaway(0, ErrorCode::NO_ERROR, makeIOBuf("bar"));
  EXPECT_TRUE(dumpedIngress_.empty());
}

TEST_F(DebugFilterTest, IngressGoaway) {
  chain_->onIngress(*makeIOBuf("foo"));
  callbackStart_->onGoaway(0, ErrorCode::PROTOCOL_ERROR, makeIOBuf("bar"));
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}

TEST_F(DebugFilterTest, EgressGoaway) {
  chain_->onIngress(*makeIOBuf("foo"));
  chain_->generateGoaway(
      writeBuf_, 0, ErrorCode::PROTOCOL_ERROR, makeIOBuf("bar"));
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}

TEST_F(DebugFilterTest, IngressRstTrackedStream) {
  chain_->onIngress(*makeIOBuf("foo"));
  callbackStart_->onMessageBegin(1, nullptr);
  auto req = makeGetRequest();
  req->getHeaders().add("trace-header", "true");
  callbackStart_->onHeadersComplete(1, std::move(req));
  callbackStart_->onAbort(1, ErrorCode::INTERNAL_ERROR);
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}

TEST_F(DebugFilterTest, EgressRstTrackedStream) {
  chain_->onIngress(*makeIOBuf("foo"));
  callbackStart_->onMessageBegin(1, nullptr);
  auto req = makeGetRequest();
  req->getHeaders().add("trace-header", "true");
  callbackStart_->onHeadersComplete(1, std::move(req));
  chain_->generateRstStream(writeBuf_, 1, ErrorCode::INTERNAL_ERROR);
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}

TEST_F(DebugFilterTest, OnSessionError) {
  chain_->onIngress(*makeIOBuf("foo"));
  HTTPException ex(HTTPException::Direction::INGRESS, "error");
  callbackStart_->onError(0, ex, false);
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}

TEST_F(DebugFilterTest, OnStreamErrorTracked) {
  chain_->onIngress(*makeIOBuf("foo"));
  callbackStart_->onMessageBegin(1, nullptr);
  auto req = makeGetRequest();
  req->getHeaders().add("trace-header", "true");
  callbackStart_->onHeadersComplete(1, std::move(req));
  HTTPException ex(HTTPException::Direction::INGRESS, "error");
  callbackStart_->onError(1, ex, false);
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}

TEST_F(DebugFilterTest, OnStreamErrorPartialMsg) {
  chain_->onIngress(*makeIOBuf("foo"));
  auto req = makeGetRequest();
  req->getHeaders().add("trace-header", "true");
  HTTPException ex(HTTPException::Direction::INGRESS, "error");
  ex.setPartialMsg(std::move(req));
  callbackStart_->onError(1, ex, false);
  EXPECT_EQ(dumpedIngress_.move()->moveToFbString(), std::string("foo"));
}
