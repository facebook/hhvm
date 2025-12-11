#include <fizz/experimental/psp/PSP.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/test/MockAsyncSocket.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GMock.h>
#include <folly/test/TestUtils.h>

#include <array>

using namespace testing;

class MockKernelPSP : public ::fizz::psp::KernelPSP {
 public:
  MOCK_METHOD(
      (folly::Expected<::fizz::psp::SA, std::error_code>),
      rxAssoc,
      (fizz::psp::PSPVersion version, int fd),
      (noexcept, override));
  MOCK_METHOD(
      std::error_code,
      txAssoc,
      (const struct ::fizz::psp::SA& sa, int fd),
      (noexcept, override));
};

class MockCallback : public ::fizz::psp::Callback {
 public:
  MOCK_METHOD(
      void,
      pspSuccess,
      (folly::NetworkSocket fd),
      (noexcept, override));
  MOCK_METHOD(
      void,
      pspError,
      (const folly::exception_wrapper& ew),
      (noexcept, override));
};

class AsyncPSPUpgradeTest : public ::testing::Test {
 protected:
  void SetUp() override {
    serverFd_ = 12345;

    mockAsyncSocket_.reset(new folly::test::MockAsyncSocket(&evb_));
    ON_CALL(*mockAsyncSocket_, getNetworkSocket())
        .WillByDefault(Return(folly::NetworkSocket::fromFd(12345)));
  }

  std::shared_ptr<fizz::psp::KernelPSP> kernelPSP() {
    return std::shared_ptr<MockKernelPSP>(
        std::shared_ptr<void>(), &mockKernelPSP_);
  }

  void expectError(folly::ByteRange buf, uint8_t code) {
    auto expected = std::to_array<uint8_t>({0x00, 0x01, code});
    ASSERT_EQ(buf.size(), 3);
    EXPECT_EQ(memcmp(expected.data(), buf.data(), 3), 0);
  }

  void setupMockTransport(folly::test::MockAsyncTransport& transport) {
    // Make sure that transport->getUnderlyingTransport<AsyncSocket>()
    // succeeds and returns a file descriptor
    ON_CALL(transport, getWrappedTransport())
        .WillByDefault(Return(mockAsyncSocket_.get()));
  }

  void expectSuccessfulRxAssoc() {
    fizz::psp::SA sa;
    sa.psp_version = static_cast<uint8_t>(fizz::psp::PSPVersion::VER0);
    sa.spi = 0xdeadbeef;
    sa.key.resize(16);
    memset(sa.key.data(), 0xFE, 16);

    EXPECT_CALL(mockKernelPSP_, rxAssoc(fizz::psp::PSPVersion::VER0, serverFd_))
        .WillOnce(Return(sa));
  }

  void setupLocalAwaitsPeerMessage(
      folly::AsyncTransport::ReadCallback*& readCallback,
      folly::test::MockAsyncTransport& mockTransport,
      std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame>& upgradeFrame) {
    setupMockTransport(mockTransport);

    expectSuccessfulRxAssoc();
    EXPECT_CALL(mockTransport, writeChain(_, _, _))
        .WillOnce([&](auto&& cb, auto&& buf, auto&&) {
          ASSERT_NE(cb, nullptr);

          // The data being sent should be a SA message.
          folly::ByteRange written = buf->coalesce();
          auto saMsg =
              folly::IOBuf::wrapBuffer(written.data() + 2, written.size() - 2);
          auto result = fizz::psp::detail::tryDecodeSA(std::move(saMsg));
          ASSERT_TRUE(result.hasValue());
          cb->writeSuccess();
        });

    EXPECT_CALL(mockTransport, setReadCB(Eq(nullptr))).Times(AnyNumber());
    EXPECT_CALL(mockTransport, setReadCB(NotNull()))
        .WillOnce(SaveArg<0>(&readCallback));

    upgradeFrame = fizz::psp::pspUpgradeV0(
        &mockTransport, fizz::psp::PSPVersion::VER0, kernelPSP());
    upgradeFrame->start(&mockCallback_);
  }

  folly::EventBase evb_;

  int serverFd_{-1};
  StrictMock<MockKernelPSP> mockKernelPSP_;
  StrictMock<MockCallback> mockCallback_;
  folly::test::MockAsyncSocket::UniquePtr mockAsyncSocket_;

  std::string expectedWritten_;
};

TEST_F(AsyncPSPUpgradeTest, DestroyWithoutInitiating) {
  folly::test::MockAsyncTransport mockTransport;
  setupMockTransport(mockTransport);
  auto upgradeFrame = fizz::psp::pspUpgradeV0(
      &mockTransport, fizz::psp::PSPVersion::VER0, kernelPSP());
  (void)upgradeFrame;
}

TEST_F(AsyncPSPUpgradeTest, NonFileDescriptorTransportError) {
  // If for some reason the AsyncTransport passed to the psp upgrade operation
  // does not have a file descriptor, then start() should fail and we should
  // write out an Error message to the peer.
  EXPECT_CALL(mockCallback_, pspError(_))
      .WillOnce([](const folly::exception_wrapper& ew) {
        EXPECT_THAT(ew.get_exception()->what(), HasSubstr("invalid socket"));
      });
  folly::test::MockAsyncTransport mockTransport;
  EXPECT_CALL(mockTransport, writeChain(_, _, _))
      .WillOnce([&](auto&&, auto buf, auto&&) {
        folly::ByteRange msg = buf->coalesce();
        expectError(msg, 0);
      });
  auto upgradeFrame = fizz::psp::pspUpgradeV0(
      &mockTransport, fizz::psp::PSPVersion::VER0, kernelPSP());
  upgradeFrame->start(&mockCallback_);
}

TEST_F(AsyncPSPUpgradeTest, RxAssocFails) {
  // When psp_rx_assoc fails, the error callback is invoked. Furthermore,
  // because this error occurs during start(), an explicit Error message should
  // be sent to the peer
  EXPECT_CALL(mockCallback_, pspError(_))
      .WillOnce([](const folly::exception_wrapper& ew) {
        EXPECT_THAT(
            ew.get_exception()->what(), HasSubstr("psp_rx_assoc failed"));
      });
  EXPECT_CALL(mockKernelPSP_, rxAssoc(fizz::psp::PSPVersion::VER0, serverFd_))
      .WillOnce(Return(
          folly::makeUnexpected<std::error_code>(
              std::error_code(1, std::system_category()))));

  folly::test::MockAsyncTransport mockTransport;
  setupMockTransport(mockTransport);

  EXPECT_CALL(mockTransport, writeChain(_, _, _))
      .WillOnce([&](auto&&, auto buf, auto&&) {
        folly::ByteRange msg = buf->coalesce();
        expectError(msg, 1);
      });

  auto upgradeFrame = fizz::psp::pspUpgradeV0(
      &mockTransport, fizz::psp::PSPVersion::VER0, kernelPSP());
  upgradeFrame->start(&mockCallback_);
}

TEST_F(AsyncPSPUpgradeTest, RxAssocSucceedsButWriteFails) {
  // The scenario here is we were able to successfully rx assoc, but we
  // encountered an I/O error while attempting to write the SA message to
  // the peer
  folly::test::MockAsyncTransport mockTransport;
  setupMockTransport(mockTransport);
  expectSuccessfulRxAssoc();

  EXPECT_CALL(mockCallback_, pspError(_))
      .WillOnce([](const folly::exception_wrapper& ew) {
        EXPECT_THAT(ew.get_exception()->what(), HasSubstr("write sa failed"));
        EXPECT_THAT(
            ew.get_exception()->what(), HasSubstr("connection reset by peer"));
      });

  EXPECT_CALL(mockTransport, writeChain(_, _, _))
      .WillOnce([&](auto&& cb, auto&&, auto&&) {
        folly::AsyncSocketException ex(
            folly::AsyncSocketException::NETWORK_ERROR,
            "connection reset by peer");
        cb->writeErr(0, ex);
      });

  auto upgradeFrame = fizz::psp::pspUpgradeV0(
      &mockTransport, fizz::psp::PSPVersion::VER0, kernelPSP());
  upgradeFrame->start(&mockCallback_);
}

TEST_F(AsyncPSPUpgradeTest, PeerSendsFailure) {
  // The scenario here is when locally, we rx_assoc, but upon reading the peer's
  // message, we discover that the peer has failed initial setup (and the peer
  // sends an explicit Error message)
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(
        ew.get_exception()->what(), HasSubstr("peer signaled error: 125"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  // At this point the upgrader is awaiting the peer's message.
  // The peer sends a failure message
  auto failureMsg = std::to_array<uint8_t>({0x00, 0x01, 125});
  readCallback->readBufferAvailable(
      folly::IOBuf::wrapBuffer(failureMsg.data(), failureMsg.size()));
}

TEST_F(AsyncPSPUpgradeTest, PeerEOF) {
  // The scenario here is when locally, we rx_assoc, but the peer immediately
  // disconnects the connection.
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  // We kick off the upgrade. The write() completes synchronously and we
  // should be able to capture the readcallback.
  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(
        ew.get_exception()->what(), HasSubstr("read peer sa I/O error"));
    EXPECT_THAT(ew.get_exception()->what(), HasSubstr("readEOF"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  readCallback->readEOF();
}

TEST_F(AsyncPSPUpgradeTest, PeerReadErr) {
  // The scenario here is when locally, we rx_assoc, but the read fails
  // for some other I/O issue
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(
        ew.get_exception()->what(), HasSubstr("read peer sa I/O error"));
    EXPECT_THAT(ew.get_exception()->what(), HasSubstr("TLS alert number 123"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::SSL_ERROR, "TLS alert number 123");
  readCallback->readErr(ex);
}

TEST_F(AsyncPSPUpgradeTest, PeerReadTimeout) {
  // The scenario here is when locally, we rx_assoc, we are waiting for the
  // peer response, but we were cancelled (e.g. locally timed out) before
  // the entire data was read.
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(ew.get_exception()->what(), HasSubstr("psp upgrade cancelled"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  // An incomplete TLV of length 30
  auto msg = std::to_array<uint8_t>({0x01, 30, 0xff});
  readCallback->readBufferAvailable(
      folly::IOBuf::copyBuffer(msg.data(), msg.size()));

  // We should still be awaiting the entire message at this point. Now we cancel
  upgradeFrame.reset();
}

TEST_F(AsyncPSPUpgradeTest, PeerSendsInvalidMessage) {
  // The scenario here is when locally, we rx_assoc, and the peer sends us
  // non-PSPv0 data (e.g. an HTTP request). This will cause TLV parsing to
  // return a message with an unknown tag.
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(
        ew.get_exception()->what(),
        HasSubstr("peer sent unknown protocol message"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  std::string msg =
      "GET /foo HTTP/1.1\r\nHost: fizz.test\r\nCookie: abcdefghijilmnopqrstuvwxyz\r\nAccept: application/json\r\n\r\n";
  readCallback->readBufferAvailable(
      folly::IOBuf::copyBuffer(
          folly::ByteRange((unsigned char*)msg.data(), msg.size())));
}
TEST_F(AsyncPSPUpgradeTest, PeerSendsInvalidErrorMessage) {
  // The scenario here is when locally, we rx_assoc, and the peer sends us
  // a message (that is interpreted as an Error) that is invalid
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(
        ew.get_exception()->what(),
        HasSubstr("peer sent invalid error message"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  auto msg = std::to_array<uint8_t>({0x00, 2, 0xff, 0xee});
  readCallback->readBufferAvailable(
      folly::IOBuf::copyBuffer(msg.data(), msg.size()));
}

TEST_F(AsyncPSPUpgradeTest, PeerSendsInvalidSAMessage) {
  // The scenario here is when locally, we rx_assoc, and the peer sends us
  // an invalid SA message (a PSP version that we do not recognize).
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(
        ew.get_exception()->what(), HasSubstr("peer invalid sa message"));
    EXPECT_THAT(
        ew.get_exception()->what(),
        HasSubstr("invalid or unsupported psp version"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  struct fizz::psp::SA sa {};
  sa.psp_version = 123;
  sa.key.resize(16);
  sa.spi = 0x11223344;

  auto msg = fizz::psp::detail::encodeTLV(sa);
  readCallback->readBufferAvailable(std::move(msg));
}

TEST_F(AsyncPSPUpgradeTest, PeerSendsValidMessageTxAssocFails) {
  // The peer sends a valid message which we try to psp_tx_assoc, but that
  // ends up failing.
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockKernelPSP_, txAssoc(_, _))
      .WillOnce([](const auto& sa, int) -> std::error_code {
        EXPECT_EQ(sa.psp_version, 0);
        EXPECT_EQ(sa.spi, 0x11223344);
        EXPECT_EQ(sa.key.size(), 16);
        auto expectedKey = std::array<uint8_t, 16>{};
        memset(expectedKey.data(), 0xfe, 16);
        EXPECT_EQ(memcmp(sa.key.data(), expectedKey.data(), 16), 0);

        return std::error_code(10, std::system_category());
      });

  EXPECT_CALL(mockCallback_, pspError(_)).WillOnce([](auto& ew) {
    EXPECT_THAT(ew.get_exception()->what(), HasSubstr("psp_tx_assoc failure"));
  });

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  struct fizz::psp::SA sa {};
  sa.psp_version = 0;
  sa.key.resize(16);
  memset(sa.key.data(), 0xfe, 16);
  sa.spi = 0x11223344;

  auto msg = fizz::psp::detail::encodeTLV(sa);
  readCallback->readBufferAvailable(std::move(msg));
}

TEST_F(AsyncPSPUpgradeTest, SuccessfulNegotiation) {
  // A successful negotiation
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockKernelPSP_, txAssoc(_, _))
      .WillOnce([](const auto& sa, int) -> std::error_code {
        EXPECT_EQ(sa.psp_version, 0);
        EXPECT_EQ(sa.spi, 0x11223344);
        EXPECT_EQ(sa.key.size(), 16);
        auto expectedKey = std::array<uint8_t, 16>{};
        memset(expectedKey.data(), 0xfe, 16);
        EXPECT_EQ(memcmp(sa.key.data(), expectedKey.data(), 16), 0);

        return std::error_code{};
      });

  EXPECT_CALL(
      mockCallback_, pspSuccess(Eq(folly::NetworkSocket::fromFd(serverFd_))));

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  struct fizz::psp::SA sa {};
  sa.psp_version = 0;
  sa.key.resize(16);
  memset(sa.key.data(), 0xfe, 16);
  sa.spi = 0x11223344;

  auto msg = fizz::psp::detail::encodeTLV(sa);
  readCallback->readBufferAvailable(std::move(msg));
}

TEST_F(AsyncPSPUpgradeTest, SuccessfulNegotiationReadDataAvailableAPI) {
  // A successful negotiation
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockKernelPSP_, txAssoc(_, _))
      .WillOnce([](const auto& sa, int) -> std::error_code {
        EXPECT_EQ(sa.psp_version, 0);
        EXPECT_EQ(sa.spi, 0x11223344);
        EXPECT_EQ(sa.key.size(), 16);
        auto expectedKey = std::array<uint8_t, 16>{};
        memset(expectedKey.data(), 0xfe, 16);
        EXPECT_EQ(memcmp(sa.key.data(), expectedKey.data(), 16), 0);

        return std::error_code{};
      });

  EXPECT_CALL(
      mockCallback_, pspSuccess(Eq(folly::NetworkSocket::fromFd(serverFd_))));

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  struct fizz::psp::SA sa {};
  sa.psp_version = 0;
  sa.key.resize(16);
  memset(sa.key.data(), 0xfe, 16);
  sa.spi = 0x11223344;

  auto msg = fizz::psp::detail::encodeTLV(sa);
  auto br = msg->coalesce();
  void* buf{nullptr};
  size_t bufsize = 0;
  readCallback->getReadBuffer(&buf, &bufsize);

  ASSERT_NE(buf, nullptr);
  ASSERT_NE(bufsize, 0);
  ASSERT_LE(br.size(), bufsize);
  memcpy(buf, br.data(), br.size());

  readCallback->readDataAvailable(br.size());
}

TEST_F(AsyncPSPUpgradeTest, SuccessfulNegotiationMultipleParts) {
  // A successful negotiation, with the peer response not being read at all
  // once
  folly::test::MockAsyncTransport mockTransport;
  folly::AsyncTransport::ReadCallback* readCallback{};
  std::unique_ptr<fizz::psp::AsyncPSPUpgradeFrame> upgradeFrame;

  EXPECT_CALL(mockKernelPSP_, txAssoc(_, _))
      .WillOnce([](const auto& sa, int) -> std::error_code {
        EXPECT_EQ(sa.psp_version, 0);
        EXPECT_EQ(sa.spi, 0x11223344);
        EXPECT_EQ(sa.key.size(), 16);
        auto expectedKey = std::array<uint8_t, 16>{};
        memset(expectedKey.data(), 0xfe, 16);
        EXPECT_EQ(memcmp(sa.key.data(), expectedKey.data(), 16), 0);

        return std::error_code{};
      });

  EXPECT_CALL(
      mockCallback_, pspSuccess(Eq(folly::NetworkSocket::fromFd(serverFd_))));

  setupLocalAwaitsPeerMessage(readCallback, mockTransport, upgradeFrame);
  ASSERT_NE(upgradeFrame.get(), nullptr);
  ASSERT_NE(readCallback, nullptr);

  struct fizz::psp::SA sa {};
  sa.psp_version = 0;
  sa.key.resize(16);
  memset(sa.key.data(), 0xfe, 16);
  sa.spi = 0x11223344;

  auto msg = fizz::psp::detail::encodeTLV(sa);
  folly::IOBufQueue q;
  q.append(std::move(msg));

  auto part1 = q.split(4);
  auto part2 = q.move();

  readCallback->readBufferAvailable(std::move(part1));
  readCallback->readBufferAvailable(std::move(part2));
}
