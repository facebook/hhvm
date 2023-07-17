/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/transport/H3DatagramAsyncSocket.h>
#include <proxygen/lib/transport/test/H3DatagramAsyncSocketTest.h>

#include <folly/portability/GTest.h>

using namespace proxygen;
using namespace quic;
using namespace testing;

void H3DatagramAsyncSocketTest::SetUp() {
  // Create the socket
  options_.mode_ = H3DatagramAsyncSocket::Mode::CLIENT;
  options_.txnTimeout_ = std::chrono::milliseconds(1000);
  options_.connectTimeout_ = std::chrono::milliseconds(500);
  options_.httpRequest_ = std::make_unique<HTTPMessage>();
  options_.httpRequest_->setMethod(proxygen::HTTPMethod::GET);
  options_.httpRequest_->setURL(getRemoteAddress().describe());
  options_.httpRequest_->setMasque();
  options_.maxDatagramSize_ = kMaxDatagramSize;

  datagramSocket_ =
      std::make_unique<H3DatagramAsyncSocket>(&eventBase_, options_);

  session_ = new HQUpstreamSession(options_.txnTimeout_,
                                   options_.connectTimeout_,
                                   nullptr,
                                   proxygen::mockTransportInfo,
                                   nullptr);

  socketDriver_ = std::make_unique<MockQuicSocketDriver>(
      &eventBase_,
      session_,
      session_,
      MockQuicSocketDriver::TransportEnum::CLIENT,
      "h3");
  socketDriver_->setMaxUniStreams(3);

  session_->setSocket(socketDriver_->getSocket());
  session_->setEgressSettings({{proxygen::SettingsId::_HQ_DATAGRAM, 1}});

  datagramSocket_->setUpstreamSession(session_);
  datagramSocket_->setRcvBuf(kMaxDatagramsBufferedRead * kMaxDatagramSize);
  datagramSocket_->setSndBuf(kMaxDatagramsBufferedWrite * kMaxDatagramSize);

  EXPECT_CALL(readCallbacks_, getReadBuffer_(_, _))
      .WillRepeatedly(Invoke([this](void** buf, size_t* len) {
        *buf = &buf_;
        *len = kMaxDatagramSize;
      }));
  quic::QuicSocket::TransportInfo transportInfo;
  EXPECT_CALL(*socketDriver_->getSocket(), getTransportInfo())
      .WillRepeatedly(testing::Return(transportInfo));
  EXPECT_CALL(*socketDriver_->getSocket(), getClientChosenDestConnectionId())
      .WillRepeatedly(Return(ConnectionId::createRandom(2)));
}

void H3DatagramAsyncSocketTest::TearDown() {
  if (datagramSocket_) {
    datagramSocket_->close();
  }
  // Needed for proper teardown of the upstream session
  eventBase_.loop();
}

folly::SocketAddress& H3DatagramAsyncSocketTest::getLocalAddress() {
  static folly::SocketAddress localAddr("::", 0);
  return localAddr;
}

folly::SocketAddress& H3DatagramAsyncSocketTest::getRemoteAddress() {
  static folly::SocketAddress remoteAddr("::", 12345);
  return remoteAddr;
}

ssize_t H3DatagramAsyncSocketTest::sendDatagramUpstream(
    std::unique_ptr<folly::IOBuf> datagram) {
  CHECK(datagramSocket_);
  return datagramSocket_->write(getRemoteAddress(), datagram);
}

TEST_F(H3DatagramAsyncSocketTest, Connect) {
  datagramSocket_->connect(getRemoteAddress());
}

TEST_F(H3DatagramAsyncSocketTest, ConnectAndReady) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
}

TEST_F(H3DatagramAsyncSocketTest, ConnectErrorBeforeReadCallbackSet) {
  datagramSocket_->connect(getRemoteAddress());
  connectError(quic::QuicError(LocalErrorCode::CONNECT_FAILED, "unreachable"));
  EXPECT_CALL(readCallbacks_, onReadError_(_))
      .Times(1)
      .WillOnce(Invoke([&](auto err) {
        EXPECT_EQ(err.getType(), folly::AsyncSocketException::NETWORK_ERROR);
        EXPECT_EQ(std::string(err.what()),
                  "AsyncSocketException: connectError: 'unreachable', type = "
                  "Network error");
      }));
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  datagramSocket_->resumeRead(&readCallbacks_);
}

TEST_F(H3DatagramAsyncSocketTest, ConnectErrorAfterReadCallbackSet) {
  datagramSocket_->connect(getRemoteAddress());
  EXPECT_CALL(readCallbacks_, onReadError_(_))
      .Times(1)
      .WillOnce(Invoke([&](auto err) {
        EXPECT_EQ(err.getType(), folly::AsyncSocketException::NETWORK_ERROR);
        EXPECT_EQ(std::string(err.what()),
                  "AsyncSocketException: connectError: 'unreachable', type = "
                  "Network error");
      }));
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  datagramSocket_->resumeRead(&readCallbacks_);
  connectError(quic::QuicError(LocalErrorCode::CONNECT_FAILED, "unreachable"));
}

TEST_F(H3DatagramAsyncSocketTest, HTTPNon200Response) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  datagramSocket_->resumeRead(&readCallbacks_);
  EXPECT_CALL(readCallbacks_, onReadError_(_))
      .Times(1)
      .WillOnce(Invoke([&](auto err) {
        EXPECT_EQ(err.getType(), folly::AsyncSocketException::INTERNAL_ERROR);
        EXPECT_EQ(std::string(err.what()),
                  "AsyncSocketException: HTTP Error: status code 407, type = "
                  "Internal error");
      }));
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  onHeadersComplete(makeResponse(407));
}

TEST_F(H3DatagramAsyncSocketTest, CloseAfter200Response) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  datagramSocket_->resumeRead(&readCallbacks_);
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  onHeadersComplete(makeResponse(200));
}

TEST_F(H3DatagramAsyncSocketTest, EOMAfter200Response) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  datagramSocket_->resumeRead(&readCallbacks_);
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  onHeadersComplete(makeResponse(200));
  onEOM();
}

TEST_F(H3DatagramAsyncSocketTest, DatagramsBeforeReadCallback) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  for (auto i = 0; i <= kMaxDatagramsBufferedRead * 2; ++i) {
    onDatagram(folly::IOBuf::copyBuffer(fmt::format("{0:010}", i)));
  }
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _))
      .Times(kMaxDatagramsBufferedRead)
      .WillRepeatedly(Invoke(
          [this](const folly::SocketAddress& client,
                 size_t len,
                 bool /*truncated*/,
                 const MockUDPReadCallback::OnDataAvailableParams& /*params*/) {
            EXPECT_EQ(client, session_->getPeerAddress());
            EXPECT_EQ(len, 10);
            auto datagramString = std::string(buf_, len);
            EXPECT_EQ(datagramString.size(), 10);
            // Check that only the furst kMaxDatagramsBufferedRead were
            // buffered
            auto num = folly::to<int>(datagramString);
            EXPECT_LT(num, kMaxDatagramsBufferedRead);
          }));
  datagramSocket_->resumeRead(&readCallbacks_);
  onEOM();
}

TEST_F(H3DatagramAsyncSocketTest, DatagramsAndEOMBeforeReadCallback) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  auto payload =
      "This will be buffered and delivered when resuming reads, after EOM";
  onDatagram(folly::IOBuf::copyBuffer(payload));
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _))
      .Times(1)
      .WillRepeatedly(Invoke(
          [&](const folly::SocketAddress& client,
              size_t len,
              bool /*truncated*/,
              const MockUDPReadCallback::OnDataAvailableParams& /*params*/) {
            EXPECT_EQ(client, session_->getPeerAddress());
            auto datagramString = std::string(buf_, len);
            EXPECT_EQ(datagramString, payload);
          }));
  onEOM();
  datagramSocket_->resumeRead(&readCallbacks_);
}

TEST_F(H3DatagramAsyncSocketTest, DiscardDatagramsBufferTooSmall) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  auto shortPayload = "short";
  auto longPayload = "this is way too long; it'll get discarded";

  EXPECT_CALL(readCallbacks_, getReadBuffer_(_, _))
      .WillRepeatedly(Invoke([this](void** buf, size_t* len) {
        *buf = &buf_;
        *len = 10;
      }));
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _))
      .Times(1)
      .WillRepeatedly(Invoke(
          [&](const folly::SocketAddress& client,
              size_t len,
              bool /*truncated*/,
              const MockUDPReadCallback::OnDataAvailableParams& /*params*/) {
            EXPECT_EQ(client, session_->getPeerAddress());
            auto datagramString = std::string(buf_, len);
            EXPECT_EQ(datagramString, shortPayload);
          }));
  datagramSocket_->resumeRead(&readCallbacks_);
  onDatagram(folly::IOBuf::copyBuffer(shortPayload));
  onDatagram(folly::IOBuf::copyBuffer(longPayload));
  onEOM();
}

TEST_F(H3DatagramAsyncSocketTest, WriteBeforeConnect) {
  auto ret =
      sendDatagramUpstream(folly::IOBuf::copyBuffer("This will be discarded"));
  EXPECT_EQ(ret, -1);
  EXPECT_EQ(errno, ENOTCONN);
}

TEST_F(H3DatagramAsyncSocketTest, WriteInvalidBuf) {
  std::unique_ptr<folly::IOBuf> nullptrBuf;
  auto ret = sendDatagramUpstream(std::move(nullptrBuf));
  EXPECT_EQ(ret, -1);
  EXPECT_EQ(errno, EINVAL);
}

TEST_F(H3DatagramAsyncSocketTest, WriteWrongAddress) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  auto ret = datagramSocket_->write(
      folly::SocketAddress("1.2.3.4", 0),
      folly::IOBuf::copyBuffer(
          "send to wrong address. I am going to be discarded"));
  EXPECT_EQ(ret, -1);
  EXPECT_EQ(errno, EINVAL);
}

TEST_F(H3DatagramAsyncSocketTest, BufferWriteBeforeConnectSuccess) {
  datagramSocket_->connect(getRemoteAddress());
  std::string payload = "This will be buffered";
  auto ret = sendDatagramUpstream(folly::IOBuf::copyBuffer(payload));
  EXPECT_EQ(ret, payload.size());
  EXPECT_EQ(socketDriver_->outDatagrams_.size(), 0);
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  EXPECT_EQ(socketDriver_->outDatagrams_.size(), 1);
}

TEST_F(H3DatagramAsyncSocketTest, DiscardWhenApplicationWriteBufferFull) {
  datagramSocket_->connect(getRemoteAddress());
  for (auto i = 0; i <= kMaxDatagramsBufferedWrite; ++i) {
    auto payload = fmt::format("{}", i);
    auto ret = sendDatagramUpstream(folly::IOBuf::copyBuffer(payload));
    if (i < kMaxDatagramsBufferedWrite) {
      EXPECT_EQ(ret, payload.size());
    } else {
      EXPECT_EQ(errno, ENOBUFS);
    }
    EXPECT_EQ(socketDriver_->outDatagrams_.size(), 0);
  }
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  EXPECT_EQ(socketDriver_->outDatagrams_.size(), kMaxDatagramsBufferedWrite);
}

TEST_F(H3DatagramAsyncSocketTest, CheckErrorWhenTransportFails) {
  EXPECT_CALL(*socketDriver_->getSocket(), writeDatagram(testing::_))
      .WillRepeatedly(
          Return(folly::makeUnexpected(LocalErrorCode::INVALID_WRITE_DATA)));

  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  auto ret = sendDatagramUpstream(folly::IOBuf::copyBuffer(
      "This is going to be discarded by the transport"));
  EXPECT_EQ(ret, -1);
  EXPECT_EQ(errno, ENOBUFS);
  EXPECT_EQ(socketDriver_->outDatagrams_.size(), 0);
}

TEST_F(H3DatagramAsyncSocketTest, WriteLargerThanDatagramSizeLimit) {
  EXPECT_CALL(*socketDriver_->getSocket(), getDatagramSizeLimit())
      .WillRepeatedly(Return(10));
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  auto ret = sendDatagramUpstream(folly::IOBuf::copyBuffer(
      "This is way larger than 10 bytes. It'll be discarded"));
  EXPECT_EQ(ret, -1);
  EXPECT_EQ(errno, EMSGSIZE);
  EXPECT_EQ(socketDriver_->outDatagrams_.size(), 0);
}

TEST_F(H3DatagramAsyncSocketTest, DatagramsPauseResumeRead) {
  InSequence enforceOrder;
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  datagramSocket_->resumeRead(&readCallbacks_);
  auto payload1 = "This will be delivered immediately";
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _)).Times(1);
  onDatagram(folly::IOBuf::copyBuffer(payload1));
  datagramSocket_->pauseRead();

  auto payload2 = "This will be delivered after reads are resumed";
  onDatagram(folly::IOBuf::copyBuffer(payload2));
  onEOM();
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _)).Times(1);
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  datagramSocket_->resumeRead(&readCallbacks_);
}

TEST_F(H3DatagramAsyncSocketTest, DatagramsTriggerPauseRead) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  auto payload1 = "This will trigger pauseReads";
  onDatagram(folly::IOBuf::copyBuffer(payload1));
  auto payload2 = "This will be delivered after reads are resumed";
  onDatagram(folly::IOBuf::copyBuffer(payload2));
  onEOM();
  int datagramsDelivered = 0;
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _))
      .Times(2)
      .WillRepeatedly(Invoke(
          [&](const folly::SocketAddress& client,
              size_t len,
              bool /*truncated*/,
              const MockUDPReadCallback::OnDataAvailableParams& /*params*/) {
            EXPECT_EQ(client, session_->getPeerAddress());
            datagramsDelivered++;
            EXPECT_EQ(datagramSocket_->isReading(), true);
            datagramSocket_->pauseRead();
            EXPECT_EQ(datagramSocket_->isReading(), false);
          }));
  datagramSocket_->resumeRead(&readCallbacks_);
  EXPECT_EQ(datagramsDelivered, 1);
  datagramSocket_->resumeRead(&readCallbacks_);
  EXPECT_EQ(datagramsDelivered, 2);
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  datagramSocket_->resumeRead(&readCallbacks_);
}

// Test resumeReads from onDataAvailable
TEST_F(H3DatagramAsyncSocketTest, ResumeReadReentrancy) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  onHeadersComplete(makeResponse(200));
  auto payload1 = "This will trigger resumeReads";
  onDatagram(folly::IOBuf::copyBuffer(payload1));
  auto payload2 = "This will be delivered just fine";
  onDatagram(folly::IOBuf::copyBuffer(payload2));
  onEOM();
  EXPECT_CALL(readCallbacks_, onDataAvailable_(_, _, _, _))
      .Times(2)
      .WillRepeatedly(Invoke(
          [&](const folly::SocketAddress& client,
              size_t len,
              bool /*truncated*/,
              const MockUDPReadCallback::OnDataAvailableParams& /*params*/) {
            EXPECT_EQ(client, session_->getPeerAddress());
            datagramSocket_->resumeRead(&readCallbacks_);
          }));
  EXPECT_CALL(readCallbacks_, onReadClosed_()).Times(1);
  datagramSocket_->resumeRead(&readCallbacks_);
}

// Test delete before TransportReady
TEST_F(H3DatagramAsyncSocketTest, DeleteSocketBeforeTransportReady) {
  datagramSocket_->connect(getRemoteAddress());
  datagramSocket_.reset();
  eventBase_.loop();
  session_->onConnectionError(quic::QuicError(
      quic::QuicErrorCode(quic::TransportErrorCode::INTERNAL_ERROR),
      std::string("connect timeout expired")));
}

// Test that calling close twice doesn't violate the HTTP state machine by
// trying to send EOM twice
TEST_F(H3DatagramAsyncSocketTest, CloseTwice) {
  datagramSocket_->connect(getRemoteAddress());
  session_->onTransportReady();
  session_->onReplaySafe();
  datagramSocket_->close();
  datagramSocket_->close();
}
