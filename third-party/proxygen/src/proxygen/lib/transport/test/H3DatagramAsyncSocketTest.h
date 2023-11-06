/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/futures/Future.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GTest.h>
#include <limits>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <proxygen/lib/transport/H3DatagramAsyncSocket.h>
#include <quic/api/test/MockQuicSocket.h>
#include <utility>

constexpr uint16_t kMaxDatagramSize = 100;
constexpr uint16_t kMaxDatagramsBufferedRead = 5;
constexpr uint16_t kMaxDatagramsBufferedWrite = 10;

class MockUDPReadCallback : public folly::AsyncUDPSocket::ReadCallback {
 public:
  ~MockUDPReadCallback() override = default;

  MOCK_METHOD(void, getReadBuffer_, (void**, size_t*));
  void getReadBuffer(void** buf, size_t* len) noexcept override {
    getReadBuffer_(buf, len);
  }

  MOCK_METHOD(
      void,
      onDataAvailable_,
      (const folly::SocketAddress&, size_t, bool, OnDataAvailableParams));
  void onDataAvailable(const folly::SocketAddress& client,
                       size_t len,
                       bool truncated,
                       OnDataAvailableParams params) noexcept override {
    onDataAvailable_(client, len, truncated, params);
  }

  MOCK_METHOD(void, onReadError_, (const folly::AsyncSocketException&));
  void onReadError(const folly::AsyncSocketException& ex) noexcept override {
    onReadError_(ex);
  }

  MOCK_METHOD(void, onReadClosed_, ());
  void onReadClosed() noexcept override {
    onReadClosed_();
  }
};

namespace proxygen {

class H3DatagramAsyncSocketTest : public testing::Test {
 public:
  void SetUp() override;
  void TearDown() override;

 protected:
  folly::SocketAddress& getLocalAddress();
  folly::SocketAddress& getRemoteAddress();
  ssize_t sendDatagramUpstream(std::unique_ptr<folly::IOBuf> datagram);

  /*
   * Wrap private members of H3DatagramAsyncSocketTest so they can be called
   * from tests
   * */
  void setUpstreamSession(proxygen::HQUpstreamSession* session) {
    datagramSocket_->setUpstreamSession(session);
  }
  void onDatagram(std::unique_ptr<folly::IOBuf> datagram) {
    datagramSocket_->onDatagram(std::move(datagram));
  }
  void connectError(quic::QuicError error) {
    datagramSocket_->connectError(std::move(error));
  }
  void onHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
    datagramSocket_->onHeadersComplete(std::move(msg));
  }
  void onEOM() {
    datagramSocket_->onEOM();
  }

  folly::EventBase eventBase_;
  std::unique_ptr<quic::MockQuicSocketDriver> socketDriver_;
  std::unique_ptr<proxygen::H3DatagramAsyncSocket> datagramSocket_;
  MockUDPReadCallback readCallbacks_;
  proxygen::HQUpstreamSession* session_{nullptr};
  proxygen::H3DatagramAsyncSocket::Options options_;
  char buf_[kMaxDatagramSize];
};
} // namespace proxygen
