/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/AsyncFizzClient.h>

#include <fizz/client/test/Mocks.h>
#include <fizz/protocol/test/Mocks.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/test/AsyncSocketTest.h>
#include <folly/io/async/test/MockAsyncSocket.h>
#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace client {
namespace test {

using namespace fizz::test;
using namespace folly;
using namespace folly::test;
using namespace testing;

class MockClientStateMachineInstance : public MockClientStateMachine {
 public:
  MockClientStateMachineInstance() {
    instance = this;
  }
  static MockClientStateMachineInstance* instance;
};
MockClientStateMachineInstance* MockClientStateMachineInstance::instance;

class MockConnectCallback : public AsyncSocket::ConnectCallback {
 public:
  MOCK_METHOD(void, _connectSuccess, ());
  MOCK_METHOD(void, _connectErr, (const AsyncSocketException&));

  void connectSuccess() noexcept override {
    _connectSuccess();
  }

  void connectErr(const AsyncSocketException& ex) noexcept override {
    _connectErr(ex);
  }
};

class AsyncFizzClientTest : public Test {
 public:
  void SetUp() override {
    context_ = std::make_shared<FizzClientContext>();
    context_->setSendEarlyData(true);
    mockPskCache_ = std::make_shared<MockPskCache>();
    context_->setPskCache(mockPskCache_);
    socket_ = new MockAsyncTransport();
    auto transport = AsyncTransportWrapper::UniquePtr(socket_);
    client_.reset(new AsyncFizzClientT<MockClientStateMachineInstance>(
        std::move(transport), context_));
    machine_ = MockClientStateMachineInstance::instance;
    ON_CALL(*socket_, good()).WillByDefault(Return(true));
    ON_CALL(readCallback_, isBufferMovable_()).WillByDefault(Return(true));
  }

 protected:
  void expectTransportReadCallback() {
    EXPECT_CALL(*socket_, setReadCB(_))
        .WillRepeatedly(SaveArg<0>(&socketReadCallback_));
  }

  void expectAppClose() {
    EXPECT_CALL(*machine_, _processAppClose(_))
        .WillOnce(InvokeWithoutArgs([]() {
          WriteToSocket write;
          TLSContent record;
          record.contentType = ContentType::alert;
          record.encryptionLevel = EncryptionLevel::Handshake;
          record.data = IOBuf::copyBuffer("closenotify");
          write.contents.emplace_back(std::move(record));
          return detail::actions(
              MutateState(
                  [](State& newState) { newState.state() = StateEnum::Error; }),
              std::move(write));
        }));
  }

  void expectAppCloseImmediate() {
    EXPECT_CALL(*machine_, _processAppCloseImmediate(_))
        .WillOnce(InvokeWithoutArgs([]() {
          TLSContent record;
          record.contentType = ContentType::handshake;
          record.data = IOBuf::copyBuffer("closenotify");
          record.encryptionLevel = EncryptionLevel::Handshake;
          WriteToSocket write;
          write.contents.emplace_back(std::move(record));
          return detail::actions(
              MutateState(
                  [](State& newState) { newState.state() = StateEnum::Error; }),
              std::move(write));
        }));
  }

  void connect() {
    expectTransportReadCallback();
    EXPECT_CALL(*machine_, _processConnect(_, _, _, _, _, _, _))
        .WillOnce(InvokeWithoutArgs([]() { return Actions(); }));
    const auto sni = std::string("www.example.com");
    client_->connect(
        &handshakeCallback_, nullptr, sni, pskIdentity_, echConfigs_);
  }

  enum class ECHMode { NotRequested, Accepted, Rejected };

  void fullHandshakeSuccess(
      bool acceptEarlyData,
      std::string alpn = "h2",
      std::shared_ptr<const Cert> clientCert = nullptr,
      std::shared_ptr<const Cert> serverCert = nullptr,
      bool pskResumed = false,
      ECHMode echMode = ECHMode::NotRequested) {
    EXPECT_CALL(*machine_, _processSocketData(_, _, _))
        .WillOnce(InvokeWithoutArgs([=]() {
          MutateState addToState([=](State& newState) {
            newState.exporterMasterSecret() =
                folly::IOBuf::copyBuffer("12345678901234567890123456789012");
            newState.cipher() = CipherSuite::TLS_AES_128_GCM_SHA256;
            newState.version() = ProtocolVersion::tls_1_3;
            if (alpn.empty()) {
              newState.alpn() = none;
            } else {
              newState.alpn() = alpn;
            }
            newState.clientCert() = clientCert;
            newState.serverCert() = serverCert;

            if (acceptEarlyData || pskResumed) {
              newState.pskMode() = PskKeyExchangeMode::psk_ke;
              newState.pskType() = PskType::Resumption;
              newState.handshakeTime() =
                  std::chrono::system_clock::now() - std::chrono::hours(1);
            } else {
              newState.handshakeTime() = std::chrono::system_clock::now();
            }
            if (echMode != ECHMode::NotRequested) {
              newState.echState().emplace();
              if (echMode == ECHMode::Accepted) {
                newState.echState()->status = ECHStatus::Accepted;
              } else {
                newState.echState()->status = ECHStatus::Rejected;
                ech::ECHConfig cfg;
                cfg.version = ech::ECHVersion::Draft15;
                cfg.ech_config_content =
                    folly::IOBuf::copyBuffer("retryconfig");
                newState.echState()->retryConfigs.emplace({std::move(cfg)});
              }
            }
          });
          ReportHandshakeSuccess reportSuccess;
          reportSuccess.earlyDataAccepted = acceptEarlyData;
          return detail::actions(
              std::move(addToState), std::move(reportSuccess), WaitForData());
        }));
    socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ServerData"));
  }

  void completeHandshake() {
    connect();
    EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
    fullHandshakeSuccess(false);
  }

  static EarlyDataParams getEarlyDataParams() {
    EarlyDataParams params;
    params.version = ProtocolVersion::tls_1_3;
    params.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
    params.alpn = "h2";
    return params;
  }

  void completeEarlyHandshake(EarlyDataParams params = getEarlyDataParams()) {
    connect();
    EXPECT_CALL(*machine_, _processSocketData(_, _, _))
        .WillOnce(InvokeWithoutArgs([&params]() {
          MutateState addParamsToState(
              [params = std::move(params)](State& newState) mutable {
                newState.earlyDataParams() = std::move(params);
              });
          ReportEarlyHandshakeSuccess reportSuccess;
          reportSuccess.maxEarlyDataSize = 1000;
          return detail::actions(
              std::move(addParamsToState),
              std::move(reportSuccess),
              WaitForData());
        }));
    EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
    socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ServerData"));
    EXPECT_FALSE(client_->isReplaySafe());
  }

  AsyncFizzClientT<MockClientStateMachineInstance>::UniquePtr client_;
  std::shared_ptr<FizzClientContext> context_;
  std::shared_ptr<MockPskCache> mockPskCache_;
  MockAsyncTransport* socket_;
  MockClientStateMachineInstance* machine_;
  AsyncTransportWrapper::ReadCallback* socketReadCallback_;
  MockHandshakeCallbackT<MockClientStateMachineInstance> handshakeCallback_;
  MockReadCallback readCallback_;
  MockWriteCallback writeCallback_;
  EventBase evb_;
  MockReplaySafetyCallback mockReplayCallback_;
  folly::Optional<std::string> pskIdentity_{"pskIdentity"};
  folly::Optional<std::vector<ech::ECHConfig>> echConfigs_;
};

MATCHER_P(BufMatches, expected, "") {
  folly::IOBufEqualTo eq;
  return eq(*arg, *expected);
}

TEST_F(AsyncFizzClientTest, TestConnect) {
  connect();
}

TEST_F(AsyncFizzClientTest, TestReadSingle) {
  connect();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(
          InvokeWithoutArgs([]() { return detail::actions(WaitForData()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzClientTest, TestReadMulti) {
  connect();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() { return detail::actions(); }))
      .WillOnce(
          InvokeWithoutArgs([]() { return detail::actions(WaitForData()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzClientTest, TestWrite) {
  completeHandshake();
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return detail::actions(); }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
}

TEST_F(AsyncFizzClientTest, TestWriteMulti) {
  completeHandshake();
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .Times(2)
      .WillRepeatedly(InvokeWithoutArgs([]() { return detail::actions(); }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP POST"));
}

// Tests that queue for writes pending a handshake success isn't used
TEST_F(AsyncFizzClientTest, TestWriteQueuePendingHandshakeWithEarlyData) {
  // early data is set to true in the context in SetUp()
  connect();
  Sequence s;
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .Times(2)
      .InSequence(s)
      .WillRepeatedly(InvokeWithoutArgs([]() { return detail::actions(); }));
  // writes will skip queue and be written directly to fizz client, which hasn't
  // been connected yet
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP POST"));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess()).InSequence(s);
  fullHandshakeSuccess(true);
}

// Tests that queue for writes pending a handshake is used and that writes are
// flushed only after handshake is complete
TEST_F(AsyncFizzClientTest, TestWriteQueuePendingHandshakeWithoutEarlyData) {
  context_->setSendEarlyData(false);
  connect();
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP POST"));
  Sequence s;
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess()).InSequence(s);
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .Times(2)
      .InSequence(s)
      .WillRepeatedly(InvokeWithoutArgs([]() { return detail::actions(); }));
  fullHandshakeSuccess(true);
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(InvokeWithoutArgs([]() { return detail::actions(); }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP PUT"));
}

// Tests that the error callback for queued writes is called and the queue is
// cleared when an error occurs
TEST_F(AsyncFizzClientTest, TestPendingHandshakeQueueErrorCallback) {
  context_->setSendEarlyData(false);
  connect();
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
  client_->writeChain(&writeCallback_, IOBuf::copyBuffer("HTTP POST"));
  client_->writeChain(&writeCallback_, IOBuf::copyBuffer("HTTP POST"));
  Sequence s;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .InSequence(s)
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(ReportError("unit test"), WaitForData());
      }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_)).InSequence(s);
  // two of three write callbacks were registered
  EXPECT_CALL(writeCallback_, writeErr_(0, _)).Times(2).InSequence(s);
  // trigger the _processSocketData expectation
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ignore"));
}

TEST_F(AsyncFizzClientTest, TestWriteErrorState) {
  connect();
  ON_CALL(*socket_, error()).WillByDefault(Return(true));
  EXPECT_CALL(writeCallback_, writeErr_(0, _));
  client_->writeChain(&writeCallback_, IOBuf::copyBuffer("test"));
}

TEST_F(AsyncFizzClientTest, TestWriteNotGoodState) {
  connect();
  ON_CALL(*socket_, good()).WillByDefault(Return(false));
  EXPECT_CALL(writeCallback_, writeErr_(0, _));
  client_->writeChain(&writeCallback_, IOBuf::copyBuffer("test"));
}

TEST_F(AsyncFizzClientTest, TestHandshake) {
  completeHandshake();
  EXPECT_TRUE(client_->isReplaySafe());
}

TEST_F(AsyncFizzClientTest, TestExporterAPI) {
  EXPECT_THROW(
      client_->getExportedKeyingMaterial("EXPORTER-Some-Label", nullptr, 32),
      std::runtime_error);
  completeHandshake();
  client_->getExportedKeyingMaterial("EXPORTER-Some-Label", nullptr, 32);
}

TEST_F(AsyncFizzClientTest, TestHandshakeError) {
  connect();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(ReportError("unit test"), WaitForData());
      }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzClientTest, TestHandshakeErrorDelete) {
  connect();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(ReportError("unit test"), WaitForData());
      }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_))
      .WillOnce(InvokeWithoutArgs([this]() { client_.reset(); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzClientTest, TestDeliverAppData) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(
            DeliverAppData{IOBuf::copyBuffer("HI")}, WaitForData());
      }));
  EXPECT_CALL(readCallback_, readBufferAvailable_(_));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzClientTest, TestWriteToSocket) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        TLSContent record;
        record.contentType = ContentType::handshake;
        record.data = IOBuf::copyBuffer("XYZ");
        record.encryptionLevel = EncryptionLevel::Handshake;
        WriteToSocket write;
        write.contents.emplace_back(std::move(record));
        return detail::actions(std::move(write), WaitForData());
      }));
  EXPECT_CALL(*socket_, writeChain(_, _, _));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzClientTest, TestMutateState) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  uint32_t numTimesRun = 0;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([&numTimesRun]() {
        return detail::actions(
            MutateState([&numTimesRun](State& newState) {
              numTimesRun++;
              newState.state() = StateEnum::Error;
            }),
            WaitForData());
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  EXPECT_EQ(client_->getState().state(), StateEnum::Error);
  EXPECT_EQ(numTimesRun, 1);
}

TEST_F(AsyncFizzClientTest, TestCloseHandshake) {
  connect();
  expectAppCloseImmediate();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  client_->closeNow();
}

TEST_F(AsyncFizzClientTest, TestTLSShutdown) {
  connect();
  expectAppClose();
  EXPECT_CALL(*socket_, close()).Times(0);
  client_->tlsShutdown();
}

TEST_F(AsyncFizzClientTest, TestShutdownWrite) {
  connect();
  expectAppClose();
  EXPECT_CALL(*socket_, shutdownWrite()).Times(1);
  client_->shutdownWrite();
}

TEST_F(AsyncFizzClientTest, TestShutdownWriteNow) {
  connect();
  expectAppClose();
  EXPECT_CALL(*socket_, shutdownWriteNow()).Times(1);
  client_->shutdownWriteNow();
}

TEST_F(AsyncFizzClientTest, TestConnecting) {
  ON_CALL(*socket_, connecting()).WillByDefault(Return(true));
  EXPECT_TRUE(client_->connecting());
  ON_CALL(*socket_, connecting()).WillByDefault(Return(false));
  connect();
  EXPECT_TRUE(client_->connecting());
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(ReportHandshakeSuccess(), WaitForData());
      }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  EXPECT_FALSE(client_->connecting());
}

TEST_F(AsyncFizzClientTest, TestGoodSocket) {
  connect();
  ON_CALL(*socket_, good()).WillByDefault(Return(true));
  EXPECT_TRUE(client_->good());
  ON_CALL(*socket_, good()).WillByDefault(Return(false));
  EXPECT_FALSE(client_->good());
}

TEST_F(AsyncFizzClientTest, TestGoodState) {
  completeHandshake();
  ON_CALL(*socket_, good()).WillByDefault(Return(true));
  EXPECT_TRUE(client_->good());
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(MutateState(
            [](State& newState) { newState.state() = StateEnum::Error; }));
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_FALSE(client_->good());
}

TEST_F(AsyncFizzClientTest, TestSocketConnect) {
  MockConnectCallback cb;
  EventBase evb;
  auto evbClient = AsyncFizzClientT<MockClientStateMachineInstance>::UniquePtr(
      new AsyncFizzClientT<MockClientStateMachineInstance>(&evb, context_));

  machine_ = MockClientStateMachineInstance::instance;
  auto server = std::make_unique<TestServer>();

  EXPECT_CALL(*machine_, _processConnect(_, _, _, _, _, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(ReportHandshakeSuccess(), WaitForData());
      }));
  EXPECT_CALL(cb, _connectSuccess()).WillOnce(Invoke([&evbClient]() {
    evbClient->closeNow();
  }));

  evbClient->connect(
      server->getAddress(),
      &cb,
      nullptr,
      std::string("www.example.com"),
      pskIdentity_);

  evb.loop();
}

TEST_F(AsyncFizzClientTest, TestSocketConnectWithUnsupportedTransport) {
  MockConnectCallback cb;
  EXPECT_CALL(cb, _connectErr(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_THAT(ex.what(), HasSubstr("could not find underlying socket"));
      }));
  EXPECT_CALL(*socket_, getWrappedTransport()).WillOnce(Return(nullptr));
  client_->connect(
      SocketAddress(),
      &cb,
      nullptr,
      std::string("www.example.com"),
      pskIdentity_);
}

TEST_F(AsyncFizzClientTest, TestHandshakeConnectWithUnopenedSocket) {
  client_.reset();
  EventBase evb;
  auto evbClient = AsyncFizzClientT<MockClientStateMachineInstance>::UniquePtr(
      new AsyncFizzClientT<MockClientStateMachineInstance>(&evb, context_));
  machine_ = MockClientStateMachineInstance::instance;
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_))
      .WillOnce(Invoke([](exception_wrapper ex) {
        EXPECT_THAT(
            ex.what().toStdString(),
            HasSubstr("handshake connect called but socket isn't open"));
      }));
  EXPECT_CALL(*machine_, _processConnect(_, _, _, _, _, _, _)).Times(0);
  evbClient->connect(
      &handshakeCallback_,
      nullptr,
      std::string("www.example.com"),
      pskIdentity_,
      folly::Optional<std::vector<ech::ECHConfig>>(folly::none));
  EXPECT_FALSE(evbClient->good());
}

TEST_F(AsyncFizzClientTest, TestSocketConnectWithOpenSocket) {
  MockConnectCallback cb;
  EXPECT_CALL(cb, _connectErr(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_THAT(ex.what(), HasSubstr("socket already open"));
      }));
  EventBase evb;
  MockAsyncSocket mockSocket(&evb);
  EXPECT_CALL(*socket_, getWrappedTransport()).WillOnce(Return(&mockSocket));
  EXPECT_CALL(mockSocket, connect_(_, _, _, _, _, _))
      .WillOnce(Invoke([](AsyncSocket::ConnectCallback* cb,
                          const SocketAddress&,
                          int,
                          const SocketOptionMap&,
                          const SocketAddress&,
                          const std::string&) {
        cb->connectErr(AsyncSocketException(
            AsyncSocketException::ALREADY_OPEN, "socket already open"));
      }));
  EXPECT_CALL(*machine_, _processConnect(_, _, _, _, _, _, _)).Times(0);
  client_->connect(
      SocketAddress(),
      &cb,
      nullptr,
      std::string("www.example.com"),
      pskIdentity_);
}

TEST_F(AsyncFizzClientTest, TestApplicationProtocol) {
  completeHandshake();
  EXPECT_EQ(client_->getApplicationProtocol(), "h2");
}

TEST_F(AsyncFizzClientTest, TestApplicationProtocolNone) {
  connect();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(false, "");
  EXPECT_EQ(client_->getApplicationProtocol(), "");
}

TEST_F(AsyncFizzClientTest, TestPskResumed) {
  connect();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(false, "h2", nullptr, nullptr, true);
  EXPECT_TRUE(client_->pskResumed());
}

TEST_F(AsyncFizzClientTest, TestNoPskResumption) {
  connect();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(false, "h2", nullptr, nullptr, false);
  EXPECT_FALSE(client_->pskResumed());
}

TEST_F(AsyncFizzClientTest, TestNoECHPolicy) {
  auto echPolicy = std::make_shared<MockECHPolicy>();
  // Sanity check: ECHPolicy::getConfig should not be called if no ECH policy on
  // context
  EXPECT_CALL(*echPolicy, getConfig(_)).Times(0);
  completeHandshake();
}

TEST_F(AsyncFizzClientTest, TestECHPolicyNoSNI) {
  auto echPolicy = std::make_shared<MockECHPolicy>();
  context_->setECHPolicy(echPolicy);
  // Sanity check: ECHPolicy::getConfig should not be called if no ECH policy on
  // context
  EXPECT_CALL(*echPolicy, getConfig(_)).Times(0);
  EXPECT_CALL(*machine_, _processConnect(_, _, _, _, _, _, _))
      .WillOnce(InvokeWithoutArgs([]() { return Actions(); }));
  client_->connect(
      &handshakeCallback_, nullptr, folly::none, pskIdentity_, folly::none);
}

TEST_F(AsyncFizzClientTest, TestOverrideECHPolicy) {
  auto echPolicy = std::make_shared<MockECHPolicy>();
  context_->setECHPolicy(echPolicy);
  // When an ECH config vector is passed to FizzClient::connect() the ECHPolicy
  // lookup should be overridden.
  echConfigs_ = std::vector<ech::ECHConfig>{};
  EXPECT_CALL(*echPolicy, getConfig("www.example.com")).Times(0);
  completeHandshake();
}

TEST_F(AsyncFizzClientTest, TestECHPolicyGet) {
  auto echPolicy = std::make_shared<MockECHPolicy>();
  context_->setECHPolicy(echPolicy);

  std::vector<ech::ECHConfig> expectedEchConfigList;
  ech::ECHConfig echConfig;
  ech::ECHConfigContentDraft echConfigContent;
  echConfigContent.key_config.kem_id = hpke::KEMId::x25519;
  echConfigContent.key_config.config_id = 1;
  echConfigContent.public_name =
      folly::IOBuf::copyBuffer("www.super.secret.sni.com");
  echConfigContent.maximum_name_length = 100;
  echConfigContent.key_config.public_key = folly::IOBuf::copyBuffer(
      "1d77eb1c522d08605b179d4214ee4a3635df7e17c336ea9006655a73fcaad63e");
  auto kdfId = hpke::KDFId::Sha256;
  auto aeadId = hpke::AeadId::TLS_AES_128_GCM_SHA256;
  ech::HpkeSymmetricCipherSuite suite{kdfId, aeadId};
  echConfigContent.key_config.cipher_suites.push_back(suite);
  echConfig.version = ech::ECHVersion::Draft15;
  echConfig.ech_config_content = encode(std::move(echConfigContent));
  expectedEchConfigList.push_back(std::move(echConfig));

  EXPECT_CALL(*echPolicy, getConfig("www.example.com"))
      .WillOnce(Return(expectedEchConfigList));

  // processConnect() should be called with the ECH config list returned from
  // ECHPolicy::getConfig()
  EXPECT_CALL(
      *machine_,
      _processConnect(
          _,
          _,
          _,
          _,
          _,
          _,
          Truly([&expectedEchConfigList](
                    const folly::Optional<std::vector<ech::ECHConfig>>&
                        configList) {
            return configList.hasValue() &&
                configList->at(0).ech_config_content->coalesce() ==
                expectedEchConfigList[0].ech_config_content->coalesce();
          })))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(ReportHandshakeSuccess(), WaitForData());
      }));
  const auto sni = std::string("www.example.com");
  client_->connect(
      &handshakeCallback_, nullptr, sni, pskIdentity_, folly::none);
}

TEST_F(AsyncFizzClientTest, TestECHAccepted) {
  connect();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(false, "h2", nullptr, nullptr, false, ECHMode::Accepted);
  EXPECT_TRUE(client_->echRequested());
  EXPECT_TRUE(client_->echAccepted());
}

TEST_F(AsyncFizzClientTest, TestECHRejected) {
  connect();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(false, "h2", nullptr, nullptr, false, ECHMode::Rejected);
  EXPECT_TRUE(client_->echRequested());
  EXPECT_FALSE(client_->echAccepted());
  auto retryConfigs = client_->getEchRetryConfigs();
  EXPECT_TRUE(retryConfigs.has_value());
  EXPECT_EQ(retryConfigs->size(), 1);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      retryConfigs->at(0).ech_config_content,
      folly::IOBuf::copyBuffer("retryconfig")));
}

TEST_F(AsyncFizzClientTest, TestGetCertsNone) {
  completeHandshake();
  EXPECT_EQ(client_->getSelfCertificate(), nullptr);
  EXPECT_EQ(client_->getPeerCertificate(), nullptr);
}

TEST_F(AsyncFizzClientTest, TestGetCerts) {
  auto clientCert = std::make_shared<MockCert>();
  auto serverCert = std::make_shared<MockCert>();
  connect();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(false, "h2", clientCert, serverCert);
  EXPECT_NE(client_->getSelfCertificate(), nullptr);
  EXPECT_NE(client_->getPeerCertificate(), nullptr);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshake) {
  completeEarlyHandshake();
  fullHandshakeSuccess(true);
  EXPECT_TRUE(client_->isReplaySafe());
  EXPECT_TRUE(client_->pskResumed());
}

TEST_F(AsyncFizzClientTest, TestEarlyParams) {
  auto clientCert = std::make_shared<MockCert>();
  auto serverCert = std::make_shared<MockCert>();
  auto params = getEarlyDataParams();
  params.clientCert = clientCert;
  params.serverCert = serverCert;
  completeEarlyHandshake(std::move(params));
  EXPECT_EQ(client_->getApplicationProtocol(), "h2");
  EXPECT_NE(client_->getSelfCertificate(), nullptr);
  EXPECT_NE(client_->getPeerCertificate(), nullptr);
}

TEST_F(AsyncFizzClientTest, TestEarlyApplicationProtocolNone) {
  auto params = getEarlyDataParams();
  params.alpn = none;
  completeEarlyHandshake(std::move(params));
  EXPECT_EQ(client_->getApplicationProtocol(), "");
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeWrite) {
  completeEarlyHandshake();

  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return detail::actions(); }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));

  fullHandshakeSuccess(true);

  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return detail::actions(); }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP POST"));
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeReplaySafeCallback) {
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);

  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return detail::actions(); }));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).WillOnce(Invoke([this]() {
    client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP POST"));
  }));
  fullHandshakeSuccess(true);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeReplaySafeCallbackRemoved) {
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReplaySafetyCallback(nullptr);

  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(true);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeOverLimit) {
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);

  auto earlyWrite = IOBuf::copyBuffer("earlywrite");
  auto longWrite = IOBuf::create(2000);
  std::memset(longWrite->writableData(), 'a', 2000);
  longWrite->append(2000);
  auto shortWrite = IOBuf::copyBuffer("shortwrite");
  auto replaySafeWrite = IOBuf::copyBuffer("replaysafe");

  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([&earlyWrite](const State&, EarlyAppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, earlyWrite));
        return detail::actions();
      }));
  client_->writeChain(nullptr, earlyWrite->clone());
  client_->writeChain(nullptr, longWrite->clone());
  client_->writeChain(nullptr, shortWrite->clone());

  Sequence s;
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(Invoke([&longWrite](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, longWrite));
        return detail::actions();
      }));
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(Invoke([&shortWrite](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, shortWrite));
        return detail::actions();
      }));
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(Invoke([&replaySafeWrite](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, replaySafeWrite));
        return detail::actions();
      }));

  EXPECT_CALL(mockReplayCallback_, onReplaySafe_())
      .WillOnce(Invoke([this, &replaySafeWrite]() {
        client_->writeChain(nullptr, replaySafeWrite->clone());
      }));
  fullHandshakeSuccess(true);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeAllOverLimit) {
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);

  auto buf = IOBuf::create(2000);
  std::memset(buf->writableData(), 'a', 2000);
  buf->append(2000);
  client_->writeChain(nullptr, buf->clone());

  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(Invoke([&buf](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, buf));
        return detail::actions();
      }));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_());
  fullHandshakeSuccess(true);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeRejectedFatalError) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::FatalConnectionError);
  completeEarlyHandshake();

  auto buf = IOBuf::create(2000);
  std::memset(buf->writableData(), 'a', 2000);
  buf->append(2000);
  client_->writeChain(nullptr, std::move(buf));
  client_->writeChain(&writeCallback_, IOBuf::copyBuffer("write"));

  EXPECT_CALL(writeCallback_, writeErr_(0, _));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeRejectedPendingWriteError) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::FatalConnectionError);
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeRejectedAutoResendNoData) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_());
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeRejectedAutoResend) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  completeEarlyHandshake();

  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([](const State&, EarlyAppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("aaaa")));
        return detail::actions();
      }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("aaaa"));
  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([](const State&, EarlyAppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("bbbb")));
        return detail::actions();
      }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("bbbb"));

  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(Invoke([](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("aaaabbbb")));
        return detail::actions();
      }));
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeRejectedAutoResendOrder) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);

  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([](const State&, EarlyAppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("aaaa")));
        return detail::actions();
      }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("aaaa"));
  auto buf = IOBuf::create(2000);
  std::memset(buf->writableData(), 'b', 2000);
  buf->append(2000);
  client_->writeChain(nullptr, buf->clone());

  Sequence s;
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(Invoke([](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("aaaa")));
        return detail::actions();
      }));
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(Invoke([&buf](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, buf));
        return detail::actions();
      }));
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .InSequence(s)
      .WillOnce(Invoke([](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("cccc")));
        return detail::actions();
      }));

  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).WillOnce(Invoke([this]() {
    client_->writeChain(nullptr, IOBuf::copyBuffer("cccc"));
  }));
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyHandshakeRejectedAutoResendDeletedBuffer) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  completeEarlyHandshake();

  auto buf = IOBuf::copyBuffer("aaaa");
  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([&buf](const State&, EarlyAppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("aaaa")));
        buf.reset();
        return detail::actions();
      }));
  client_->write(nullptr, buf->data(), buf->length());

  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(Invoke([](const State&, AppWrite& write) {
        EXPECT_TRUE(IOBufEqualTo()(write.data, IOBuf::copyBuffer("aaaa")));
        return detail::actions();
      }));
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectResendDifferentAlpn) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false, "h3");
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectResendDifferentNoAlpn) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  completeEarlyHandshake();
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false, "h3");
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectResendDifferentVersion) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto params = getEarlyDataParams();
  params.version = ProtocolVersion::tls_1_2;
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectResendDifferentCipher) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto params = getEarlyDataParams();
  params.cipher = CipherSuite::TLS_AES_256_GCM_SHA384;
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectNoClientCert) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto params = getEarlyDataParams();
  params.clientCert = std::make_shared<MockCert>();
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectNoServerCert) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto params = getEarlyDataParams();
  params.clientCert = std::make_shared<MockCert>();
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectDifferentServerIdentity) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto cert1 = std::make_shared<MockCert>();
  auto cert2 = std::make_shared<MockCert>();
  auto params = getEarlyDataParams();
  params.serverCert = cert1;
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  EXPECT_CALL(*cert1, getIdentity()).WillOnce(Return("id1"));
  EXPECT_CALL(*cert2, getIdentity()).WillOnce(Return("id2"));
  fullHandshakeSuccess(false, "h2", nullptr, cert2);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectSameServerIdentity) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto cert1 = std::make_shared<MockCert>();
  auto cert2 = std::make_shared<MockCert>();
  auto params = getEarlyDataParams();
  params.serverCert = cert1;
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_());
  EXPECT_CALL(*cert1, getIdentity()).WillOnce(Return("id"));
  EXPECT_CALL(*cert2, getIdentity()).WillOnce(Return("id"));
  fullHandshakeSuccess(false, "h2", nullptr, cert2);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectDifferentClientIdentity) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto cert1 = std::make_shared<MockCert>();
  auto cert2 = std::make_shared<MockCert>();
  auto params = getEarlyDataParams();
  params.clientCert = cert1;
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readErr_(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
      }));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_()).Times(0);
  EXPECT_CALL(*cert1, getIdentity()).WillOnce(Return("id1"));
  EXPECT_CALL(*cert2, getIdentity()).WillOnce(Return("id2"));
  fullHandshakeSuccess(false, "h2", cert2, nullptr);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectSameClientIdentity) {
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  auto cert1 = std::make_shared<MockCert>();
  auto cert2 = std::make_shared<MockCert>();
  auto params = getEarlyDataParams();
  params.clientCert = cert1;
  completeEarlyHandshake(std::move(params));
  client_->setReplaySafetyCallback(&mockReplayCallback_);
  EXPECT_CALL(mockReplayCallback_, onReplaySafe_());
  EXPECT_CALL(*cert1, getIdentity()).WillOnce(Return("id"));
  EXPECT_CALL(*cert2, getIdentity()).WillOnce(Return("id"));
  fullHandshakeSuccess(false, "h2", cert2, nullptr);
}

TEST_F(AsyncFizzClientTest, TestEarlyRejectRemovePsk) {
  EXPECT_CALL(*mockPskCache_, removePsk(*pskIdentity_));
  completeEarlyHandshake();
  fullHandshakeSuccess(false);
}

TEST_F(AsyncFizzClientTest, TestEarlyWriteRejected) {
  completeEarlyHandshake();
  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([](const State&, EarlyAppWrite& write) {
        ReportEarlyWriteFailed failed;
        failed.write = std::move(write);
        return detail::actions(std::move(failed));
      }));
  EXPECT_CALL(writeCallback_, writeSuccess_());
  client_->writeChain(&writeCallback_, IOBuf::copyBuffer("HTTP GET"));
}

TEST_F(AsyncFizzClientTest, TestEarlyWriteRejectedNullCallback) {
  completeEarlyHandshake();
  EXPECT_CALL(*machine_, _processEarlyAppWrite(_, _))
      .WillOnce(Invoke([](const State&, EarlyAppWrite& write) {
        ReportEarlyWriteFailed failed;
        failed.write = std::move(write);
        return detail::actions(std::move(failed));
      }));
  client_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
}

TEST_F(AsyncFizzClientTest, TestErrorStopsActions) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return detail::actions(
            MutateState(
                [](State& newState) { newState.state() = StateEnum::Error; }),
            ReportError("unit test"));
      }));
  EXPECT_FALSE(client_->error());
  EXPECT_TRUE(client_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_TRUE(client_->error());
  EXPECT_FALSE(client_->good());
}

TEST_F(AsyncFizzClientTest, TestTransportError) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _)).Times(0);
  EXPECT_FALSE(client_->error());
  EXPECT_TRUE(client_->good());
  ON_CALL(*socket_, error()).WillByDefault(Return(true));
  AsyncSocketException ase(AsyncSocketException::UNKNOWN, "unit test");
  socketReadCallback_->readErr(ase);
  EXPECT_TRUE(client_->error());
  EXPECT_FALSE(client_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_TRUE(client_->error());
  EXPECT_FALSE(client_->good());
}

TEST_F(AsyncFizzClientTest, TestTransportEof) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _)).Times(0);
  EXPECT_FALSE(client_->error());
  EXPECT_TRUE(client_->good());
  ON_CALL(*socket_, good()).WillByDefault(Return(false));
  socketReadCallback_->readEOF();
  EXPECT_FALSE(client_->error());
  EXPECT_FALSE(client_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_FALSE(client_->error());
  EXPECT_FALSE(client_->good());
}

TEST_F(AsyncFizzClientTest, TestNewCachedPskActions) {
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          []() { return detail::actions(NewCachedPsk(), WaitForData()); }));
  EXPECT_CALL(*mockPskCache_, putPsk(*pskIdentity_, _));
  socketReadCallback_->readBufferAvailable(
      IOBuf::copyBuffer("NewSessionTicket"));
}

TEST_F(AsyncFizzClientTest, TestNewCachedPskActionsWithEmptyPskIdentity) {
  pskIdentity_ = folly::none;
  completeHandshake();
  client_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          []() { return detail::actions(NewCachedPsk(), WaitForData()); }));
  EXPECT_CALL(*mockPskCache_, putPsk(_, _)).Times(0);
  socketReadCallback_->readBufferAvailable(
      IOBuf::copyBuffer("NewSessionTicket"));
}

TEST_F(AsyncFizzClientTest, TestAsyncFizzClientDestructor) {
  socket_ = new MockAsyncTransport();
  auto transport = AsyncTransportWrapper::UniquePtr(socket_);
  auto fizzClient = new AsyncFizzClientT<MockClientStateMachineInstance>(
      std::move(transport), context_);
  bool destructible = std::is_destructible<
      AsyncFizzClientT<MockClientStateMachineInstance>>::value;
  EXPECT_FALSE(destructible);
  std::shared_ptr<AsyncTransportWrapper> client{
      fizzClient, folly::DelayedDestruction::Destructor()};
}

TEST_F(AsyncFizzClientTest, TestHandshakeRecordAlignedReads) {
  client_->setHandshakeRecordAlignedReads(true);

  connect();

  void* buf;
  size_t len;
  socketReadCallback_->getReadBuffer(&buf, &len);

  // Handshake record aligned reads begin with a read of 5 bytes for the
  // record header.
  EXPECT_EQ(len, 5);

  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(
          InvokeWithoutArgs([] { return detail::actions(WaitForData{10}); }));
  socketReadCallback_->readDataAvailable(5);

  socketReadCallback_->getReadBuffer(&buf, &len);
  EXPECT_EQ(len, 10);

  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(Invoke([](auto&&, auto&& queue, auto&&) {
        queue.move();
        return detail::actions(WaitForData{5});
      }));
  socketReadCallback_->readDataAvailable(10);

  // Handshake successs event should have now transitioned the socket to
  // not perform record aligned reads, so subsequent allocations should be
  // larger
  fullHandshakeSuccess(false);
  socketReadCallback_->getReadBuffer(&buf, &len);

  // Not caring about record alignment, we should be able to get a buffer of
  // at least TransportOptions::readBufferMinReadSize
  AsyncFizzBase::TransportOptions options;
  EXPECT_GE(len, options.readBufferMinReadSize);
}

} // namespace test
} // namespace client
} // namespace fizz
