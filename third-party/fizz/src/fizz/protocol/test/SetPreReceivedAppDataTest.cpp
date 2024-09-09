/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/FizzServerContext.h>
#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/coro/GmockHelpers.h>
#include <folly/coro/Task.h>
#include <folly/coro/Timeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/SocketPair.h>
#include <folly/portability/GTest.h>
#include <folly/testing/TestUtil.h>

#if !defined(TEST_CERTS_DIR)
#define TEST_CERTS_DIR "fizz/protocol/test/certs"
#endif

using folly::test::find_resource;

namespace fizz {

namespace {

const char* kTestCert = TEST_CERTS_DIR "/tests-cert.pem";
const char* kTestKey = TEST_CERTS_DIR "/tests-key.pem";
const size_t kReadSize{1024};
const int kTimeoutMs{1000};
folly::StringPiece kPreReceivedAppData{"This is early app data."};
folly::StringPiece kTestData{"oasiufsakjfhlkaskdfkjaslfhkjsahdfksadfjhs"};

} // namespace

class SetPreReceivedAppDataTest : public testing::TestWithParam<std::string> {
 public:
  explicit SetPreReceivedAppDataTest() {
    serverCtx_ = std::make_shared<server::FizzServerContext>();
    std::string certData, keyData;
    folly::readFile(find_resource(kTestCert).c_str(), certData);
    folly::readFile(find_resource(kTestKey).c_str(), keyData);
    auto cert = fizz::openssl::CertUtils::makeSelfCert(
        std::move(certData), std::move(keyData));
    auto certManager = std::make_shared<fizz::server::CertManager>();
    certManager->addCertAndSetDefault(std::move(cert));
    serverCtx_->setCertManager(std::move(certManager));

    clientCtx_ = std::make_shared<client::FizzClientContext>();
  }

 protected:
  std::shared_ptr<server::FizzServerContext> serverCtx_;
  std::shared_ptr<client::FizzClientContext> clientCtx_;
};

class TestReadCallback : public folly::AsyncTransport::ReadCallback {
 public:
  explicit TestReadCallback(const std::string& testType)
      : testType_(testType) {}

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    std::tie(*bufReturn, *lenReturn) = queue_.preallocate(kReadSize, kReadSize);
  }

  void readDataAvailable(size_t len) noexcept override {
    queue_.postallocate(len);
    receivedLen_ += len;
    if (receivedLen_ >= (testType_ == "with" ? kPreReceivedAppData.size() : 0) +
            kTestData.size()) {
      baton_.post();
    }
  }

  void readEOF() noexcept override {
    error_ = folly::make_exception_wrapper<std::runtime_error>("Closed.");
    baton_.post();
  }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    error_ = folly::make_exception_wrapper<folly::AsyncSocketException>(ex);
    baton_.post();
  }

  folly::coro::Task<std::unique_ptr<folly::IOBuf>> start() {
    auto cancelToken = co_await folly::coro::co_current_cancellation_token;

    folly::CancellationCallback cancellationCallback{
        cancelToken, [this] { baton_.post(); }};

    co_await baton_;

    if (cancelToken.isCancellationRequested()) {
      co_yield folly::coro::co_cancelled;
    }

    if (error_) {
      co_yield folly::coro::co_error(std::move(error_));
    }

    co_return queue_.move();
  }

 private:
  folly::coro::Baton baton_;
  folly::IOBufQueue queue_{folly::IOBufQueue::cacheChainLength()};
  folly::exception_wrapper error_;
  size_t receivedLen_{0};
  const std::string& testType_;
};

INSTANTIATE_TEST_CASE_P(
    SetPreReceivedAppDataTest,
    SetPreReceivedAppDataTest,
    testing::Values("with", "without"));

CO_TEST_P(SetPreReceivedAppDataTest, earlyAppData) {
  std::string testType = GetParam();
  folly::SocketPair fds;
  folly::EventBase evb;
  auto clientSock =
      client::AsyncFizzClient::UniquePtr(new client::AsyncFizzClient(
          folly::AsyncSocket::newSocket(&evb, fds.extractNetworkSocket0()),
          clientCtx_));

  auto serverSock =
      server::AsyncFizzServer::UniquePtr(new server::AsyncFizzServer(
          folly::AsyncSocket::newSocket(&evb, fds.extractNetworkSocket1()),
          serverCtx_));

  TestReadCallback readCB(testType);

  std::thread eventBaseThread([&]() {
    serverSock->accept(nullptr);
    if (testType == "with") {
      serverSock->setPreReceivedAppData(
          folly::IOBuf::copyBuffer(kPreReceivedAppData));
    }
    serverSock->setReadCB(&readCB);

    clientSock->connect(
        nullptr,
        nullptr,
        folly::none,
        folly::none,
        folly::none,
        std::chrono::milliseconds(kTimeoutMs));
    clientSock->writeChain(nullptr, folly::IOBuf::copyBuffer(kTestData));

    evb.loopForever();
  });

  auto readTry = co_await folly::coro::co_awaitTry(folly::coro::timeout(
      readCB.start(), std::chrono::milliseconds(kTimeoutMs)));
  evb.runInEventBaseThreadAndWait([&]() { serverSock->setReadCB(nullptr); });

  CO_ASSERT_FALSE(readTry.hasException());

  std::unique_ptr<folly::IOBuf> expectedData;
  if (testType == "with") {
    expectedData = folly::IOBuf::copyBuffer(kPreReceivedAppData);
    expectedData->appendToChain(folly::IOBuf::copyBuffer(kTestData));
  } else {
    expectedData = folly::IOBuf::copyBuffer(kTestData);
  }

  EXPECT_TRUE(folly::IOBufEqualTo()(readTry.value(), expectedData));

  evb.runInEventBaseThreadAndWait([&]() {
    clientSock->close();
    serverSock->close();
  });

  evb.terminateLoopSoon();
  eventBaseThread.join();
}

} // namespace fizz
