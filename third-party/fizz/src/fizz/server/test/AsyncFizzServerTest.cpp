/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/AsyncFizzServer.h>

#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/server/test/Mocks.h>
#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace server {
namespace test {

using namespace fizz::extensions;
using namespace folly;
using namespace folly::test;
using namespace testing;

template <typename... Args>
AsyncActions actions(Args&&... act) {
  return fizz::server::detail::actions(std::forward<Args>(act)...);
}

class MockServerStateMachineInstance : public MockServerStateMachine {
 public:
  MockServerStateMachineInstance() {
    instance = this;
  }
  static MockServerStateMachineInstance* instance;
};
MockServerStateMachineInstance* MockServerStateMachineInstance::instance;

class AsyncFizzServerTest : public Test {
 public:
  void SetUp() override {
    context_ = std::make_shared<FizzServerContext>();
    socket_ = new MockAsyncTransport();
    auto transport = AsyncTransportWrapper::UniquePtr(socket_);
    server_.reset(new AsyncFizzServerT<MockServerStateMachineInstance>(
        std::move(transport),
        context_,
        std::make_shared<MockServerExtensions>()));
    machine_ = MockServerStateMachineInstance::instance;
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

  void accept() {
    expectTransportReadCallback();
    EXPECT_CALL(*socket_, getEventBase()).WillOnce(Return(&evb_));
    EXPECT_CALL(*machine_, _processAccept(_, &evb_, _, _))
        .WillOnce(InvokeWithoutArgs([evb = &evb_]() {
          return actions(MutateState([evb](State& newState) {
            // need an executor to drive async actions
            newState.executor() = evb;
          }));
        }));
    server_->accept(&handshakeCallback_);
  }

  void fullHandshakeSuccess(
      std::shared_ptr<const Cert> clientCert = nullptr,
      std::shared_ptr<const Cert> serverCert = nullptr) {
    EXPECT_CALL(*machine_, _processSocketData(_, _, _))
        .WillOnce(InvokeWithoutArgs([clientCert,
                                     serverCert,
                                     cipher = negotiatedCipher_,
                                     protocolVersion = protocolVersion_]() {
          MutateState addExporterToState([=](State& newState) {
            auto exporterMaster =
                folly::IOBuf::copyBuffer("12345678901234567890123456789012");
            newState.exporterMasterSecret() = std::move(exporterMaster);
            newState.cipher() = cipher;
            newState.version() = protocolVersion;
            newState.clientCert() = clientCert;
            newState.serverCert() = serverCert;
          });
          return actions(
              std::move(addExporterToState),
              ReportHandshakeSuccess(),
              WaitForData());
        }));
    socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  }

  void completeHandshake() {
    accept();
    EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
    fullHandshakeSuccess();
  }

  AsyncFizzServerT<MockServerStateMachineInstance>::UniquePtr server_;
  std::shared_ptr<FizzServerContext> context_;
  MockAsyncTransport* socket_;
  MockServerStateMachineInstance* machine_;
  AsyncTransportWrapper::ReadCallback* socketReadCallback_;
  MockHandshakeCallbackT<MockServerStateMachineInstance> handshakeCallback_;
  MockReadCallback readCallback_;
  MockWriteCallback writeCallback_;
  EventBase evb_;
  CipherSuite negotiatedCipher_ = CipherSuite::TLS_AES_128_GCM_SHA256;
  ProtocolVersion protocolVersion_ = ProtocolVersion::tls_1_3;
};

MATCHER_P(BufMatches, expected, "") {
  folly::IOBufEqualTo eq;
  return eq(*arg, *expected);
}

TEST_F(AsyncFizzServerTest, TestAccept) {
  accept();
}

TEST_F(AsyncFizzServerTest, TestReadSingle) {
  accept();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() { return actions(WaitForData()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzServerTest, TestReadMulti) {
  accept();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() { return actions(); }))
      .WillOnce(InvokeWithoutArgs([]() { return actions(WaitForData()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzServerTest, TestWrite) {
  accept();
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return actions(); }));
  server_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
}

TEST_F(AsyncFizzServerTest, TestWriteMulti) {
  accept();
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return actions(); }));
  server_->writeChain(nullptr, IOBuf::copyBuffer("HTTP GET"));
  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return actions(); }));
  server_->writeChain(nullptr, IOBuf::copyBuffer("HTTP POST"));
}

TEST_F(AsyncFizzServerTest, TestWriteErrorState) {
  accept();
  ON_CALL(*socket_, error()).WillByDefault(Return(true));
  EXPECT_CALL(writeCallback_, writeErr_(0, _));
  server_->writeChain(&writeCallback_, IOBuf::copyBuffer("test"));
}

TEST_F(AsyncFizzServerTest, TestWriteNotGoodState) {
  accept();
  ON_CALL(*socket_, good()).WillByDefault(Return(false));
  EXPECT_CALL(writeCallback_, writeErr_(0, _));
  server_->writeChain(&writeCallback_, IOBuf::copyBuffer("test"));
}

TEST_F(AsyncFizzServerTest, TestHandshake) {
  completeHandshake();
}

TEST_F(AsyncFizzServerTest, TestExporterAPISimple) {
  completeHandshake();
  server_->getExportedKeyingMaterial(kTokenBindingExporterLabel, nullptr, 32);
}

TEST_F(AsyncFizzServerTest, TestExporterAPIIncompleteHandshake) {
  EXPECT_EQ(
      server_->getExportedKeyingMaterial(
          kTokenBindingExporterLabel, nullptr, 32),
      nullptr);
}

TEST_F(AsyncFizzServerTest, TestHandshakeError) {
  accept();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          []() { return actions(ReportError("unit test"), WaitForData()); }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzServerTest, TestDeliverAppData) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return actions(DeliverAppData{IOBuf::copyBuffer("HI")}, WaitForData());
      }));
  EXPECT_CALL(readCallback_, readBufferAvailable_(_));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzServerTest, TestSendTicketWithAppToken) {
  completeHandshake();
  EXPECT_CALL(*machine_, _processWriteNewSessionTicket(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        WriteToSocket write;
        TLSContent record;
        record.contentType = ContentType::handshake;
        record.encryptionLevel = EncryptionLevel::AppTraffic;
        record.data = IOBuf::copyBuffer("XYZ");
        write.contents.emplace_back(std::move(record));
        return actions(std::move(write));
      }));
  EXPECT_CALL(*socket_, writeChain(_, _, _));
  server_->sendTicketWithAppToken(IOBuf::copyBuffer("testAppToken"));
}

TEST_F(AsyncFizzServerTest, TestWriteToSocket) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        WriteToSocket write;
        TLSContent record;
        record.contentType = ContentType::application_data;
        record.encryptionLevel = EncryptionLevel::AppTraffic;
        record.data = IOBuf::copyBuffer("XYZ");
        write.contents.emplace_back(std::move(record));
        return actions(std::move(write), WaitForData());
      }));
  EXPECT_CALL(*socket_, writeChain(_, _, _));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzServerTest, TestMutateState) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  uint32_t numTimesRun = 0;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([&numTimesRun]() {
        return actions(
            MutateState([&numTimesRun](State& newState) {
              numTimesRun++;
              newState.state() = StateEnum::Error;
            }),
            WaitForData());
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  EXPECT_EQ(server_->getState().state(), StateEnum::Error);
  EXPECT_EQ(numTimesRun, 1);
}

TEST_F(AsyncFizzServerTest, TestAttemptVersionFallback) {
  accept();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return actions(
            MutateState(
                [](State& newState) { newState.state() = StateEnum::Error; }),
            AttemptVersionFallback{
                IOBuf::copyBuffer("ClientHello"),
                folly::Optional<std::string>("www.hostname.com")});
      }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeAttemptFallback(_))
      .WillOnce(Invoke([&](AttemptVersionFallback& fallback) {
        // The mock machine does not move the read buffer so there will be a 2nd
        // ClientHello.
        EXPECT_TRUE(IOBufEqualTo()(
            fallback.clientHello, IOBuf::copyBuffer("ClientHelloClientHello")));
        EXPECT_EQ(fallback.sni, "www.hostname.com");
        server_.reset();
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
}

TEST_F(AsyncFizzServerTest, TestDeleteAsyncEvent) {
  accept();
  Promise<Actions> p1;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          [&p1]() { return AsyncActions(p1.getSemiFuture()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  server_.reset();
  Promise<Actions> p2;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          [&p2]() { return AsyncActions(p2.getSemiFuture()); }));
  p1.setValue(detail::actions());
  p2.setValue(detail::actions(WaitForData()));
  // necessary to drive the async actions to completion
  evb_.loopOnce();
}

TEST_F(AsyncFizzServerTest, TestCloseHandshake) {
  accept();
  expectAppCloseImmediate();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_));
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  server_->closeNow();
}

TEST_F(AsyncFizzServerTest, TestTLSShutdown) {
  accept();
  expectAppClose();
  EXPECT_CALL(*socket_, close()).Times(0);
  server_->tlsShutdown();
}

TEST_F(AsyncFizzServerTest, TestShutdownWrite) {
  accept();
  expectAppClose();
  EXPECT_CALL(*socket_, shutdownWrite()).Times(1);
  server_->shutdownWrite();
}

TEST_F(AsyncFizzServerTest, TestShutdownWriteNow) {
  accept();
  expectAppClose();
  EXPECT_CALL(*socket_, shutdownWriteNow()).Times(1);
  server_->shutdownWriteNow();
}

TEST_F(AsyncFizzServerTest, TestCloseNowInFlightAction) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  Promise<Actions> p;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          [&p]() { return AsyncActions(p.getSemiFuture()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  server_->writeChain(&writeCallback_, IOBuf::copyBuffer("queued write"));
  EXPECT_CALL(writeCallback_, writeErr_(0, _));
  EXPECT_CALL(readCallback_, readEOF_());
  EXPECT_CALL(*socket_, closeNow()).Times(AtLeast(1));
  server_->closeNow();
  p.setValue(detail::actions(WaitForData()));
}

TEST_F(AsyncFizzServerTest, TestCloseInFlightAction) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  Promise<Actions> p;
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          [&p]() { return AsyncActions(p.getSemiFuture()); }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  server_->writeChain(&writeCallback_, IOBuf::copyBuffer("queued write"));
  server_->close();

  EXPECT_CALL(*machine_, _processAppWrite(_, _))
      .WillOnce(InvokeWithoutArgs([]() { return actions(); }));
  expectAppCloseImmediate();
  p.setValue(detail::actions(WaitForData()));
}

TEST_F(AsyncFizzServerTest, TestIsDetachable) {
  completeHandshake();
  AsyncTransportWrapper::ReadCallback* readCb = socketReadCallback_;
  ON_CALL(*socket_, isDetachable()).WillByDefault(Return(false));
  EXPECT_FALSE(server_->isDetachable());
  ON_CALL(*socket_, isDetachable()).WillByDefault(Return(true));
  EXPECT_TRUE(server_->isDetachable());
  Promise<Actions> p;

  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          [&p]() { return AsyncActions(p.getSemiFuture()); }));
  socket_->setReadCB(readCb);
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_FALSE(server_->isDetachable());
  p.setValue(detail::actions(WaitForData()));
  // necessary to drive all async actions to completion so there aren't any
  // still in processing when we check whether it's detachable
  evb_.loopOnce();
  EXPECT_TRUE(server_->isDetachable());
}

// Similar to TestIsDetachable but verifies that a completed future async action
// will complete inline and won't bloack the detach
TEST_F(AsyncFizzServerTest, TestIsDetachableCompletedFuture) {
  completeHandshake();
  AsyncTransportWrapper::ReadCallback* readCb = socketReadCallback_;
  ON_CALL(*socket_, isDetachable()).WillByDefault(Return(false));
  EXPECT_FALSE(server_->isDetachable());
  ON_CALL(*socket_, isDetachable()).WillByDefault(Return(true));
  EXPECT_TRUE(server_->isDetachable());
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        // action should run immediately since future is completed
        return AsyncActions(
            folly::makeSemiFuture(detail::actions(WaitForData())));
      }));
  socket_->setReadCB(readCb);
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_TRUE(server_->isDetachable());
}

TEST_F(AsyncFizzServerTest, TestConnecting) {
  ON_CALL(*socket_, connecting()).WillByDefault(Return(true));
  EXPECT_TRUE(server_->connecting());
  ON_CALL(*socket_, connecting()).WillByDefault(Return(false));
  accept();
  EXPECT_TRUE(server_->connecting());
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs(
          []() { return actions(ReportHandshakeSuccess(), WaitForData()); }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  EXPECT_FALSE(server_->connecting());
}

TEST_F(AsyncFizzServerTest, TestGoodSocket) {
  accept();
  ON_CALL(*socket_, good()).WillByDefault(Return(true));
  EXPECT_TRUE(server_->good());
  ON_CALL(*socket_, good()).WillByDefault(Return(false));
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestGoodState) {
  completeHandshake();
  ON_CALL(*socket_, good()).WillByDefault(Return(true));
  EXPECT_TRUE(server_->good());
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return actions(MutateState(
            [](State& newState) { newState.state() = StateEnum::Error; }));
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestEarlySuccess) {
  accept();
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return actions(ReportEarlyHandshakeSuccess(), WaitForData());
      }));
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));

  fullHandshakeSuccess();
}

TEST_F(AsyncFizzServerTest, TestErrorStopsActions) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return actions(
            MutateState(
                [](State& newState) { newState.state() = StateEnum::Error; }),
            ReportError("unit test"));
      }));
  EXPECT_FALSE(server_->error());
  EXPECT_TRUE(server_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_TRUE(server_->error());
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestTransportError) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _)).Times(0);
  EXPECT_FALSE(server_->error());
  EXPECT_TRUE(server_->good());
  ON_CALL(*socket_, error()).WillByDefault(Return(true));
  AsyncSocketException ase(AsyncSocketException::UNKNOWN, "unit test");
  socketReadCallback_->readErr(ase);
  EXPECT_TRUE(server_->error());
  EXPECT_FALSE(server_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_TRUE(server_->error());
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestTransportEof) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  EXPECT_CALL(*machine_, _processSocketData(_, _, _)).Times(0);
  EXPECT_FALSE(server_->error());
  EXPECT_TRUE(server_->good());
  ON_CALL(*socket_, good()).WillByDefault(Return(false));
  socketReadCallback_->readEOF();
  EXPECT_FALSE(server_->error());
  EXPECT_FALSE(server_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_FALSE(server_->error());
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestGetCertsNone) {
  completeHandshake();
  EXPECT_EQ(server_->getSelfCertificate(), nullptr);
  EXPECT_EQ(server_->getPeerCertificate(), nullptr);
}

TEST_F(AsyncFizzServerTest, TestGetCerts) {
  auto clientCert = std::make_shared<MockCert>();
  auto serverCert = std::make_shared<MockCert>();
  accept();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeSuccess());
  fullHandshakeSuccess(clientCert, serverCert);
  EXPECT_NE(server_->getSelfCertificate(), nullptr);
  EXPECT_NE(server_->getPeerCertificate(), nullptr);
}

TEST_F(AsyncFizzServerTest, TestTransportNotGoodOnSuccess) {
  accept();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_));
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([&]() {
        // Make the socket return not good
        EXPECT_CALL(*socket_, good()).WillOnce(Return(false));
        return actions(ReportHandshakeSuccess());
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestTransportNotGoodOnEarlySuccess) {
  accept();
  EXPECT_CALL(handshakeCallback_, _fizzHandshakeError(_));
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([&]() {
        // Make the socket return not good
        EXPECT_CALL(*socket_, good()).WillOnce(Return(false));
        return actions(ReportEarlyHandshakeSuccess());
      }));
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("ClientHello"));
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestRemoteClosed) {
  completeHandshake();
  server_->setReadCB(&readCallback_);
  EXPECT_CALL(readCallback_, readEOF_());
  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return actions(
            MutateState([](State& s) { s.state() = StateEnum::Closed; }),
            EndOfData());
      }));
  EXPECT_CALL(*socket_, closeNow());
  EXPECT_TRUE(server_->good());
  socketReadCallback_->readBufferAvailable(IOBuf::copyBuffer("Data"));
  EXPECT_FALSE(server_->good());
}

TEST_F(AsyncFizzServerTest, TestHandshakeRecordAlignedReads) {
  server_->setHandshakeRecordAlignedReads(true);

  accept();

  void* buf;
  size_t len;
  socketReadCallback_->getReadBuffer(&buf, &len);

  // Handshake record aligned reads begin with a read of 5 bytes for the
  // record header.
  EXPECT_EQ(len, 5);

  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([] { return actions(WaitForData{10}); }));
  socketReadCallback_->readDataAvailable(5);

  socketReadCallback_->getReadBuffer(&buf, &len);
  EXPECT_EQ(len, 10);

  EXPECT_CALL(*machine_, _processSocketData(_, _, _))
      .WillOnce(Invoke([](auto&&, auto&& queue, auto&&) {
        queue.move();
        return actions(WaitForData{5});
      }));
  socketReadCallback_->readDataAvailable(10);

  // Handshake successs event should have now transitioned the socket to
  // not perform record aligned reads, so subsequent allocations should be
  // larger
  fullHandshakeSuccess();
  socketReadCallback_->getReadBuffer(&buf, &len);

  // Not caring about record alignment, we should be able to get a buffer of
  // at least AsyncFizzBase::kMinReadSize
  EXPECT_GE(len, 1460);
}

} // namespace test
} // namespace server
} // namespace fizz
