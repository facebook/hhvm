/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/protocol/Exporter.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/FizzServerContext.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncIoUringSocket.h>
#include <folly/io/async/AsyncIoUringSocketFactory.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/IoUringBackend.h>
#include <thrift/lib/cpp2/security/SSLUtil.h>

using namespace apache::thrift;

class ToFDSocketZeroCopyTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto serverSock = folly::netops::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(serverSock, folly::NetworkSocket());

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    ASSERT_EQ(
        0,
        folly::netops::bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)));
    ASSERT_EQ(0, folly::netops::listen(serverSock, 1));

    socklen_t len = sizeof(addr);
    ASSERT_EQ(
        0,
        folly::netops::getsockname(serverSock, (struct sockaddr*)&addr, &len));

    clientFd_ = folly::netops::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(clientFd_, folly::NetworkSocket());
    ASSERT_EQ(
        0,
        folly::netops::connect(
            clientFd_, (struct sockaddr*)&addr, sizeof(addr)));

    serverFd_ = folly::netops::accept(serverSock, nullptr, nullptr);
    ASSERT_NE(serverFd_, folly::NetworkSocket());
    folly::netops::close(serverSock);
  }

  void TearDown() override {
    if (serverFd_ != folly::NetworkSocket()) {
      folly::netops::close(serverFd_);
    }
    if (clientFd_ != folly::NetworkSocket()) {
      folly::netops::close(clientFd_);
    }
  }

  void testClient(bool enableZeroCopy) {
    auto sock = folly::AsyncSocket::newSocket(&evb_, clientFd_);
    clientFd_ = folly::NetworkSocket();
    if (enableZeroCopy) {
      sock->setZeroCopy(true);
      if (!sock->getZeroCopy()) {
        // SO_ZEROCOPY not supported on this platform; nothing to verify.
        return;
      }
    }
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    auto fizzClient = fizz::client::AsyncFizzClient::UniquePtr(
        new fizz::client::AsyncFizzClient(std::move(sock), std::move(ctx)));
    auto result = toFDSocket(fizzClient.get(), kSecurityProtocolStopTLS);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getZeroCopy(), enableZeroCopy);
  }

  void testServer(bool enableZeroCopy) {
    auto sock = folly::AsyncSocket::newSocket(&evb_, serverFd_);
    serverFd_ = folly::NetworkSocket();
    if (enableZeroCopy) {
      sock->setZeroCopy(true);
      if (!sock->getZeroCopy()) {
        // SO_ZEROCOPY not supported on this platform; nothing to verify.
        return;
      }
    }
    auto serverCtx = std::make_shared<fizz::server::FizzServerContext>();
    auto fizzServer = fizz::server::AsyncFizzServer::UniquePtr(
        new fizz::server::AsyncFizzServer(std::move(sock), serverCtx));
    auto result = toFDSocket(fizzServer.get(), kSecurityProtocolStopTLS);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getZeroCopy(), enableZeroCopy);
  }

  folly::NetworkSocket serverFd_;
  folly::NetworkSocket clientFd_;
  folly::EventBase evb_;
};

TEST_F(ToFDSocketZeroCopyTest, PreservesZeroCopyOnClient) {
  testClient(true);
}

TEST_F(ToFDSocketZeroCopyTest, NoZeroCopyOnClient) {
  testClient(false);
}

TEST_F(ToFDSocketZeroCopyTest, PreservesZeroCopyOnServer) {
  testServer(true);
}

TEST_F(ToFDSocketZeroCopyTest, NoZeroCopyOnServer) {
  testServer(false);
}

TEST_F(ToFDSocketZeroCopyTest, NullInputReturnsNull) {
  fizz::client::AsyncFizzClient* nullClient = nullptr;
  auto result = toFDSocket(nullClient, kSecurityProtocolStopTLS);
  EXPECT_EQ(result, nullptr);
}

using ToWrappedSocketTestParams = std::tuple<bool, bool, bool>;
// tuple order: isClient, useIoUring, enableZeroCopy

namespace {
std::string paramToString(const ToWrappedSocketTestParams& p) {
  bool isClient = std::get<0>(p);
  bool useIoUring = std::get<1>(p);
  bool enableZeroCopy = std::get<2>(p);
  return std::string(isClient ? "Client" : "Server") +
      std::string(useIoUring ? "IoUring" : "AsyncSocket") +
      std::string(enableZeroCopy ? "Zc" : "NoZc");
}

std::unique_ptr<folly::EventBase> makeEventBase(bool wantIoUring) {
  if (!wantIoUring) {
    return std::make_unique<folly::EventBase>();
  }
#if !FOLLY_HAS_LIBURING
  return nullptr;
#else
  try {
    auto eb = std::make_unique<folly::EventBase>(
        folly::EventBase::Options().setBackendFactory(
            []() -> std::unique_ptr<folly::EventBaseBackendBase> {
              folly::IoUringBackend::Options opts;
              // Need buffer provider for AsyncIoUringSocket::supports()
              opts.setInitialProvidedBuffers(2048, 1000).setCapacity(16384);
              return std::make_unique<folly::IoUringBackend>(std::move(opts));
            }));
    if (!folly::AsyncIoUringSocketFactory::supports(eb.get())) {
      return nullptr;
    }
    return eb;
  } catch (const folly::IoUringBackend::NotAvailable&) {
    return nullptr;
  }
#endif
}

folly::AsyncSocketTransport::UniquePtr createUnderlyingTransport(
    folly::EventBase* evb, folly::NetworkSocket& fd, bool useIoUring) {
  folly::AsyncSocketTransport::UniquePtr transport;
  if (useIoUring) {
    transport = folly::AsyncIoUringSocketFactory::create<
        folly::AsyncSocketTransport::UniquePtr>(evb, fd);
  } else {
    transport = folly::AsyncSocket::newSocket(evb, fd);
  }
  fd = folly::NetworkSocket(); // ownership transferred
  return transport;
}
} // namespace

class ToWrappedAsyncSocketTestBase : public ::testing::Test {
 protected:
  void SetUp() override {
    auto serverSock = folly::netops::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(serverSock, folly::NetworkSocket());

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    ASSERT_EQ(
        0,
        folly::netops::bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)));
    ASSERT_EQ(0, folly::netops::listen(serverSock, 1));

    socklen_t len = sizeof(addr);
    ASSERT_EQ(
        0,
        folly::netops::getsockname(serverSock, (struct sockaddr*)&addr, &len));

    clientFd_ = folly::netops::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(clientFd_, folly::NetworkSocket());
    ASSERT_EQ(
        0,
        folly::netops::connect(
            clientFd_, (struct sockaddr*)&addr, sizeof(addr)));

    serverFd_ = folly::netops::accept(serverSock, nullptr, nullptr);
    ASSERT_NE(serverFd_, folly::NetworkSocket());
    folly::netops::close(serverSock);
  }

  void TearDown() override {
    if (serverFd_ != folly::NetworkSocket()) {
      folly::netops::close(serverFd_);
    }
    if (clientFd_ != folly::NetworkSocket()) {
      folly::netops::close(clientFd_);
    }
  }

  folly::NetworkSocket serverFd_;
  folly::NetworkSocket clientFd_;
  folly::EventBase evb_;
};

class ToWrappedAsyncSocketTest
    : public ToWrappedAsyncSocketTestBase,
      public ::testing::WithParamInterface<ToWrappedSocketTestParams> {};

TEST_P(ToWrappedAsyncSocketTest, PreservesSecurityAndZeroCopy) {
  const auto& param = GetParam();
  bool isClient = std::get<0>(param);
  bool useIoUring = std::get<1>(param);
  bool enableZeroCopy = std::get<2>(param);

  auto evbPtr = makeEventBase(useIoUring);
  if (useIoUring && !evbPtr) {
    GTEST_SKIP() << "IoUring backend not available";
  }
  folly::EventBase* evb = evbPtr ? evbPtr.get() : &evb_;

  folly::NetworkSocket fd = isClient ? clientFd_ : serverFd_;
  // Prevent double-close in TearDown after we move fd.
  if (isClient) {
    clientFd_ = folly::NetworkSocket();
  } else {
    serverFd_ = folly::NetworkSocket();
  }

  auto underlying = createUnderlyingTransport(evb, fd, useIoUring);
  ASSERT_NE(underlying, nullptr);

#if FOLLY_HAS_LIBURING
  if (useIoUring) {
    ASSERT_NE(
        underlying->getUnderlyingTransport<folly::AsyncIoUringSocket>(),
        nullptr)
        << "Expected AsyncIoUringSocket underlying transport";
  } else
#endif
  {
    ASSERT_NE(underlying->getUnderlyingTransport<folly::AsyncSocket>(), nullptr)
        << "Expected AsyncSocket underlying transport";
  }

  if (enableZeroCopy) {
    underlying->setZeroCopy(true);
    if (!underlying->getZeroCopy()) {
      GTEST_SKIP() << "Zero-copy not supported on this platform";
    }
  }

  const std::string expectedSecurityProtocol = kSecurityProtocolStopTLS;
  folly::AsyncTransport::UniquePtr wrappedResult;

  static const std::string kTestEkmLabel = "test label";
  static constexpr uint16_t kTestEkmLength = 32;
  static constexpr auto kTestCipher = fizz::CipherSuite::TLS_AES_128_GCM_SHA256;
  static const std::string kTestSecret(32, 's');

  auto injectTestSecurityState = [](auto& fizzSocket) {
    auto& mutableState =
        const_cast<std::decay_t<decltype(fizzSocket.getState())>&>(
            fizzSocket.getState());
    mutableState.cipher() = kTestCipher;
    mutableState.exporterMasterSecret() = folly::IOBuf::copyBuffer(kTestSecret);
  };

  auto wrapAndValidate = [&](auto& fizzSocket) {
    injectTestSecurityState(fizzSocket);

    auto alpnBefore = fizzSocket.getApplicationProtocol();
    auto peerCertBefore = fizzSocket.getPeerCertificate();
    auto selfCertBefore = fizzSocket.getSelfCertificate();
    auto cipherBefore = fizzSocket.getCipher();
    EXPECT_TRUE(cipherBefore.hasValue());
    EXPECT_EQ(cipherBefore.value(), kTestCipher);

    auto expectedEkm = fizzSocket.getExportedKeyingMaterial(
        kTestEkmLabel, nullptr, kTestEkmLength);
    EXPECT_NE(expectedEkm, nullptr);
    if (!expectedEkm) {
      return folly::AsyncTransport::UniquePtr{};
    }

    auto result = toWrappedAsyncSocket(&fizzSocket, expectedSecurityProtocol);
    EXPECT_NE(result, nullptr);
    if (!result) {
      return folly::AsyncTransport::UniquePtr{};
    }

    EXPECT_NE(
        fizzSocket.template getUnderlyingTransport<folly::AsyncSocket>(),
        nullptr);

    EXPECT_EQ(result->getSecurityProtocol(), expectedSecurityProtocol);
    EXPECT_EQ(result->getApplicationProtocol(), alpnBefore);
    EXPECT_EQ(result->getPeerCertificate(), peerCertBefore);
    EXPECT_EQ(result->getSelfCertificate(), selfCertBefore);

    auto actualEkm = result->getExportedKeyingMaterial(
        kTestEkmLabel, nullptr, kTestEkmLength);
    EXPECT_NE(actualEkm, nullptr);
    if (expectedEkm && actualEkm) {
      EXPECT_TRUE(folly::IOBufEqualTo{}(*expectedEkm, *actualEkm));
    }

    return result;
  };

  if (isClient) {
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    auto fizzClient = fizz::client::AsyncFizzClient::UniquePtr(
        new fizz::client::AsyncFizzClient(std::move(underlying), ctx));
    wrappedResult = wrapAndValidate(*fizzClient);
  } else {
    auto serverCtx = std::make_shared<fizz::server::FizzServerContext>();
    auto fizzServer = fizz::server::AsyncFizzServer::UniquePtr(
        new fizz::server::AsyncFizzServer(std::move(underlying), serverCtx));
    wrappedResult = wrapAndValidate(*fizzServer);
  }
  ASSERT_NE(wrappedResult, nullptr);

  EXPECT_EQ(wrappedResult->getZeroCopy(), enableZeroCopy);

  const auto* inner = wrappedResult->getWrappedTransport();
  ASSERT_NE(inner, nullptr);
#if FOLLY_HAS_LIBURING
  if (useIoUring) {
    EXPECT_NE(
        inner->getUnderlyingTransport<folly::AsyncIoUringSocket>(), nullptr);
  } else
#endif
  {
    EXPECT_NE(inner->getUnderlyingTransport<folly::AsyncSocket>(), nullptr);
  }
}

INSTANTIATE_TEST_SUITE_P(
    AllCombinations,
    ToWrappedAsyncSocketTest,
    ::testing::Combine(
        ::testing::Bool(), // isClient
#if FOLLY_HAS_LIBURING
        ::testing::Bool(), // useIoUring
#else
        ::testing::Values(false),
#endif
        ::testing::Bool() // enableZeroCopy
        ),
    [](const ::testing::TestParamInfo<ToWrappedSocketTestParams>& info) {
      return paramToString(info.param);
    });
