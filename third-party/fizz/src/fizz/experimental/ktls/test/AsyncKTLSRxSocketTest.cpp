/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/experimental/ktls/AsyncKTLSSocket.h>
#include <fizz/experimental/ktls/KTLS.h>
#include <fizz/record/EncryptedRecordLayer.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/test/TestUtils.h>

using namespace ::testing;
static auto toIOBuf(std::string&& s) -> fizz::Buf {
  auto buf = folly::IOBuf::create(s.size());
  memcpy(buf->writableTail(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

static fizz::TrafficKey unhexlifyKey(
    std::string_view key,
    std::string_view iv) {
  return {toIOBuf(folly::unhexlify(key)), toIOBuf(folly::unhexlify(iv))};
}

template <class T>
static std::unique_ptr<T> makeEncryptedRecordLayer(
    fizz::TrafficKey key,
    size_t seq) {
  // TODO: Change EncryptedRecordLayer to remove the unused ByteRange first
  // parameter
  static unsigned char dummy;

  auto aead = fizz::OpenSSLEVPCipher::makeCipher<fizz::AESGCM128>();
  aead->setKey(std::move(key));
  auto rl = std::make_unique<T>(fizz::EncryptionLevel::AppTraffic);
  rl->setAead(folly::ByteRange(&dummy, 1), std::move(aead));
  rl->setSequenceNumber(seq);
  return rl;
}

// Encodes and splits a handshake message across n different records.
template <class HandshakeMessage>
std::vector<fizz::TLSMessage> encodeHandshakeAmongRecords(
    HandshakeMessage&& msg,
    size_t n) {
  std::vector<fizz::TLSMessage> ret;
  auto encodedHandshake =
      fizz::encodeHandshake(std::forward<HandshakeMessage>(msg));
  encodedHandshake->coalesce();

  CHECK_GT(n, 0);
  size_t splitSize = encodedHandshake->length() / n;
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  queue.append(std::move(encodedHandshake));

  for (size_t i = 0; i < n; i++) {
    auto fragment = queue.split(i == n - 1 ? queue.chainLength() : splitSize);
    ret.emplace_back(
        fizz::TLSMessage{fizz::ContentType::handshake, std::move(fragment)});
  }

  return ret;
}

// Creates a pair of connected TCP sockets. Returns (client, server) raw
// file descriptors.
static std::pair<int, int> makeTCPPair() {
  int listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  PCHECK(listener > 0);
  SCOPE_EXIT {
    ::close(listener);
  };

  struct sockaddr_in listenAddr {};
  listenAddr.sin_family = AF_INET;
  listenAddr.sin_port = htons(0);
  listenAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  PCHECK(::bind(listener, (sockaddr*)&listenAddr, sizeof(listenAddr)) == 0);
  PCHECK(::listen(listener, 1) == 0);
  socklen_t len = sizeof(listenAddr);
  PCHECK(::getsockname(listener, (sockaddr*)&listenAddr, &len) == 0);

  int client = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  PCHECK(client > 0);
  PCHECK(::connect(client, (sockaddr*)&listenAddr, sizeof(listenAddr)) == 0);

  int server = ::accept(listener, nullptr, nullptr);
  PCHECK(server > 0);

  return std::make_pair(client, server);
}

struct TrafficParameters {
  std::string key;
  std::string iv;
  uint64_t sequence;

  fizz::TrafficKey toTrafficKey() const {
    return unhexlifyKey(key, iv);
  }

  template <fizz::TrafficDirection D>
  fizz::KTLSDirectionalCryptoParams<D> toKTLSParams() const {
    fizz::KTLSDirectionalCryptoParams<D> params;
    params.ciphersuite = fizz::CipherSuite::TLS_AES_128_GCM_SHA256;
    params.key = toTrafficKey();
    params.recordSeq = sequence;
    return params;
  }

  std::unique_ptr<fizz::EncryptedWriteRecordLayer> write() {
    return makeEncryptedRecordLayer<fizz::EncryptedWriteRecordLayer>(
        toTrafficKey(), sequence);
  }
  std::unique_ptr<fizz::EncryptedReadRecordLayer> read() {
    return makeEncryptedRecordLayer<fizz::EncryptedReadRecordLayer>(
        toTrafficKey(), sequence);
  }
};

static TrafficParameters clientToServer{
    "00000000000000000000000000000000",
    "aabbccddeeffaabbccdd0011",
    12};
static TrafficParameters serverToClient{
    "11111111111111111111111111111111",
    "ffffffffffffffffffffffff",
    8};

static fizz::Buf writeAppData(
    fizz::EncryptedWriteRecordLayer& writer,
    std::string_view data) {
  auto payload = writer.writeAppData(
      folly::IOBuf::copyBuffer(data.data(), data.size()), {});
  payload.data->coalesce();
  return std::move(payload.data);
}

class KTLSTest : public ::testing::Test {
 protected:
  virtual void innerSetup() = 0;
  void SetUp() override {
    // fizz::platformSupportsKTLS() is taken to be axiomatic.
    if (!fizz::platformSupportsKTLS()) {
      SKIP() << "kTLS tests require ktls support";
    }
    innerSetup();
  }

  folly::EventBase evb_;
};

class MockTLSCallback : public fizz::AsyncKTLSSocket::TLSCallback {
 public:
  MOCK_METHOD(
      void,
      receivedNewSessionTicket,
      (fizz::AsyncKTLSSocket*, fizz::NewSessionTicket));
};

// Tests where we test the read path for AsyncKTLSRxSocket.
// kTLS socket represents the "server", we manually write on a socket as the
// client
class KTLSReadTest : public KTLSTest {
 protected:
  void innerSetup() override {
    int server;
    std::tie(client_, server) = makeTCPPair();
    PCHECK(
        folly::netops::set_socket_non_blocking(folly::NetworkSocket(server)) ==
        0);
    auto serverSocket =
        folly::AsyncSocket::newSocket(&evb_, folly::NetworkSocket(server));

    auto tlsCb = std::make_unique<StrictMock<MockTLSCallback>>();
    mockTLSCB_ = tlsCb.get();

    auto ktlsFDResult = fizz::KTLSNetworkSocket::tryEnableKTLS(
        serverSocket->getNetworkSocket(),
        clientToServer.toKTLSParams<fizz::TrafficDirection::Receive>());
    ASSERT_TRUE(ktlsFDResult.hasValue());

    serverConn_.reset(new fizz::AsyncKTLSRxSocket(
        serverSocket.get(),
        std::move(tlsCb),
        nullptr,
        nullptr,
        serverToClient.write()));

    clientWrite_ = clientToServer.write();
    clientRead_ = serverToClient.read();
  }

  void TearDown() override {
    ::close(client_);
    KTLSTest::TearDown();
  }

  fizz::AsyncKTLSSocket::UniquePtr serverConn_;
  int client_;
  std::unique_ptr<fizz::EncryptedWriteRecordLayer> clientWrite_;
  std::unique_ptr<fizz::EncryptedReadRecordLayer> clientRead_;
  StrictMock<folly::test::MockReadCallback> mockReadCB_;
  StrictMock<MockTLSCallback>* mockTLSCB_{nullptr};
  folly::IOBufQueue clientReadQueue_{folly::IOBufQueue::cacheChainLength()};
};

// The purpose of this test is to ensure that the KTLS socket can read a small
// piece of application data, and write a small piece of application data that
// another compatible TLS client would be able to decrypt.
TEST_F(KTLSReadTest, BasicReadWrite) {
  // Client writes "hello world" to ktls server, manually encrypting it.
  {
    auto contents = writeAppData(*clientWrite_, "hello world");
    EXPECT_EQ(
        contents->computeChainDataLength(),
        ::send(client_, contents->data(), contents->length(), 0));

    std::array<char, 64> buf = {};
    EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
        .WillOnce(Invoke([&](void** ret, size_t* size) {
          *ret = buf.data();
          *size = buf.size();
        }));
    EXPECT_CALL(mockReadCB_, readDataAvailable_(strlen("hello world")))
        .WillOnce(InvokeWithoutArgs([&] { serverConn_->setReadCB(nullptr); }));
    serverConn_->setReadCB(&mockReadCB_);
    evb_.loop();

    EXPECT_EQ(strlen("hello world"), serverConn_->getAppBytesReceived());
    EXPECT_EQ(std::string(buf.data()), "hello world");
  }

  // Server writes "goodbye world" to client with normal write() call.
  // Client is able to manually decrypt the application data record.
  {
    StrictMock<folly::test::MockWriteCallback> writeCB;
    EXPECT_CALL(writeCB, writeSuccess_());
    serverConn_->write(&writeCB, "goodbye world", strlen("goodbye world"));
    evb_.loop();

    // EXPECT_EQ(strlen("goodbye world"), serverConn_->getAppBytesWritten());

    std::array<char, 128> buf;
    ssize_t nread = ::recv(client_, buf.data(), buf.size(), 0);
    PCHECK(nread > 0);

    clientReadQueue_.append(folly::IOBuf::copyBuffer(buf.data(), nread));
    auto message =
        clientRead_->read(clientReadQueue_, fizz::Aead::AeadOptions());
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->type, fizz::ContentType::application_data);
    auto data = message->fragment->moveToFbString().toStdString();
    EXPECT_EQ("goodbye world", data);
  }
}

// The purpose of this test is to ensure that alerts sent after kTLS is
// established result in a readErr callback on the installed read callback.
TEST_F(KTLSReadTest, ExceptionalAlert) {
  // Client writes decode_error alert to server.
  {
    auto contents = clientWrite_->writeAlert(
        fizz::Alert(fizz::AlertDescription::decode_error));
    contents.data->coalesce();
    EXPECT_EQ(
        contents.data->computeChainDataLength(),
        ::send(client_, contents.data->data(), contents.data->length(), 0));
  }

  std::array<char, 128> buf;
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillOnce(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));
  EXPECT_CALL(mockReadCB_, readErr_(_)).WillOnce(Invoke([&](const auto& exc) {
    EXPECT_THAT(
        exc.what(), HasSubstr("kTLS received alert from peer: decode_error"));
  }));
  serverConn_->setReadCB(&mockReadCB_);
  evb_.loop();
  EXPECT_FALSE(serverConn_->good());
}

// The purpose of this test is to ensure that close_notify, which is special,
// results in a readEOF rather than a readErr that normal TLS alerts are
// translated to.
TEST_F(KTLSReadTest, CloseNotifyAlert) {
  // Client writes decode_error alert to server.
  {
    fizz::Alert alert(fizz::AlertDescription::close_notify);
    alert.level = 0x01;

    auto contents = clientWrite_->writeAlert(std::move(alert));
    contents.data->coalesce();
    EXPECT_EQ(
        contents.data->computeChainDataLength(),
        ::send(client_, contents.data->data(), contents.data->length(), 0));
  }

  std::array<char, 128> buf;
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillOnce(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));
  EXPECT_CALL(mockReadCB_, readEOF_());
  serverConn_->setReadCB(&mockReadCB_);
  evb_.loop();
  EXPECT_FALSE(serverConn_->good());
}

// The purpose of this test is to ensure that if we try to create an
// AsyncKTLSSocket on an fd that already has ktls upper layer protocol enabled,
// that creation fails.
TEST_F(KTLSReadTest, MultipleKTLSError) {
  auto serverfd = serverConn_->getNetworkSocket();

  auto ktlsfd = fizz::KTLSNetworkSocket::tryEnableKTLS(
      serverfd,
      clientToServer.toKTLSParams<fizz::TrafficDirection::Receive>(),
      serverToClient.toKTLSParams<fizz::TrafficDirection::Transmit>());
  EXPECT_TRUE(ktlsfd.hasError());
}

// The purpose of this test is to ensure that if we receive any handshake
// message if we don't have a tls callback installed, that we will get
// a readErr.
TEST_F(KTLSReadTest, NoTLSCallbackCausesReadErrOnHandshake) {
  fizz::AsyncKTLSSocket::UniquePtr conn;
  conn.reset(
      new fizz::AsyncKTLSSocket(serverConn_.get(), nullptr, nullptr, nullptr));

  // Client writes a NewSessionTicket to server.
  {
    fizz::NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 20;
    nst.ticket_nonce = toIOBuf("abc");
    nst.ticket = toIOBuf("123");
    auto content =
        clientWrite_->writeHandshake(fizz::encodeHandshake(std::move(nst)));
    content.data->coalesce();
    EXPECT_EQ(
        content.data->computeChainDataLength(),
        ::send(client_, content.data->data(), content.data->length(), 0));
  }

  std::array<char, 64> buf{};
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillOnce(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));
  EXPECT_CALL(mockReadCB_, readErr_(_)).WillOnce(Invoke([&](const auto& exc) {
    EXPECT_THAT(
        exc.what(),
        HasSubstr("without a tls callback implementation to handle"));
  }));
  conn->setReadCB(&mockReadCB_);
  evb_.loop();
}

TEST_F(KTLSReadTest, HandshakeDispatch) {
  std::array<char, 64> buf{};
  // Client writes a NewSessionTicket to server.
  {
    fizz::NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 20;
    nst.ticket_nonce = toIOBuf("abc");
    nst.ticket = toIOBuf("123");
    auto content =
        clientWrite_->writeHandshake(fizz::encodeHandshake(std::move(nst)));
    content.data->coalesce();
    EXPECT_EQ(
        content.data->computeChainDataLength(),
        ::send(client_, content.data->data(), content.data->length(), 0));
  }

  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillOnce(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));
  EXPECT_CALL(*mockTLSCB_, receivedNewSessionTicket(_, _))
      .WillOnce(Invoke([&](auto&&, fizz::NewSessionTicket ticket) {
        EXPECT_EQ(ticket.ticket_lifetime, 100);
        EXPECT_EQ(ticket.ticket_age_add, 20);
        EXPECT_EQ("abc", ticket.ticket_nonce->moveToFbString().toStdString());
        EXPECT_EQ("123", ticket.ticket->moveToFbString().toStdString());
        serverConn_->setReadCB(nullptr);
      }));
  serverConn_->setReadCB(&mockReadCB_);
  evb_.loop();

  // Handshake data should not be counted as application bytes
  EXPECT_EQ(0, serverConn_->getAppBytesReceived());

  // TODO: Any other handshake message should cause a fatal error
}

// The purpose of this test is to validate how we handle handshake record types
// when the ReadCallback `getReadBuffer()` gives us a tiny buffer that cannot
// fit the entire handshake message.
//
// This is similar to the case when handshake messages are split across records.
// In both cases, we need to internally buffer the partial handshake data.
TEST_F(KTLSReadTest, HandshakeRecordSmallBuffer) {
  std::array<char, 1> buf;

  // Client writes a NewSessionTicket to server.
  {
    fizz::NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 20;
    nst.ticket_nonce = toIOBuf("abc");
    nst.ticket = toIOBuf("123");
    auto content =
        clientWrite_->writeHandshake(fizz::encodeHandshake(std::move(nst)));
    content.data->coalesce();
    EXPECT_EQ(
        content.data->computeChainDataLength(),
        ::send(client_, content.data->data(), content.data->length(), 0));
  }

  // getReadBuffer() will need to be called multiple times, since each time
  // we take 1 byte of data from the kernel ktls buffer
  size_t count = 0;
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillRepeatedly(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
        if (count > 0) {
          EXPECT_TRUE(serverConn_->hasBufferedHandshakeData());
        }
        count++;
      }));

  EXPECT_CALL(*mockTLSCB_, receivedNewSessionTicket(_, _))
      .WillOnce(Invoke([&](auto&&, fizz::NewSessionTicket ticket) {
        EXPECT_EQ(ticket.ticket_lifetime, 100);
        EXPECT_EQ(ticket.ticket_age_add, 20);
        EXPECT_EQ("abc", ticket.ticket_nonce->moveToFbString().toStdString());
        EXPECT_EQ("123", ticket.ticket->moveToFbString().toStdString());
        serverConn_->setReadCB(nullptr);
      }));
  serverConn_->setReadCB(&mockReadCB_);
  evb_.loop();
}

// The purpose of this test is to ensure that handshake messages that span
// records are properly handled.
//
// This is very similar to the HandshakeRecordSmallBuffer test. However,
// in this test, the user supplied buffer is large enough to fit a record,
// but the handshake payload itself is divided between 3 record boundaries.
TEST_F(KTLSReadTest, HandshakeMessageAcrossRecords) {
  // Client writes a NewSessionTicket to server in 3 records.
  fizz::Buf lastPart;
  {
    fizz::NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 20;
    nst.ticket_nonce = toIOBuf("abc");
    nst.ticket = toIOBuf("123");
    auto records = encodeHandshakeAmongRecords(std::move(nst), 3);
    EXPECT_EQ(records.size(), 3);

    // Send the first two records immediately.
    for (size_t i = 0; i < records.size(); i++) {
      auto encrypted = clientWrite_->write(std::move(records[i]), {});
      encrypted.data->coalesce();

      if (i == records.size() - 1) {
        lastPart = std::move(encrypted.data);
      } else {
        EXPECT_EQ(
            encrypted.data->computeChainDataLength(),
            ::send(
                client_, encrypted.data->data(), encrypted.data->length(), 0));
      }
    }
  }

  // getReadBuffer() will need to be called multiple times.
  std::array<char, 128> buf;
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillRepeatedly(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));

  serverConn_->setReadCB(&mockReadCB_);
  evb_.loopOnce();
  EXPECT_TRUE(serverConn_->hasBufferedHandshakeData());

  // Now send the last record, which will complete the handshake message, and
  // should trigger our callback.
  lastPart->coalesce();
  EXPECT_EQ(
      lastPart->computeChainDataLength(),
      ::send(client_, lastPart->data(), lastPart->length(), 0));

  EXPECT_CALL(*mockTLSCB_, receivedNewSessionTicket(_, _))
      .WillOnce(Invoke([&](auto&&, fizz::NewSessionTicket ticket) {
        EXPECT_EQ(ticket.ticket_lifetime, 100);
        EXPECT_EQ(ticket.ticket_age_add, 20);
        EXPECT_EQ("abc", ticket.ticket_nonce->moveToFbString().toStdString());
        EXPECT_EQ("123", ticket.ticket->moveToFbString().toStdString());
        serverConn_->setReadCB(nullptr);
      }));
  evb_.loop();
}

// The purpose of this test is to ensure that application data records that are
// sent while a pending handshake message is being processed leads to a fatal
// connection error.
TEST_F(KTLSReadTest, SplicedHandshakeDataByAppdataFailsConnection) {
  // Client writes a NewSessionTicket message with two records, but with
  // an application data record sandwiched between.
  {
    fizz::NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 20;
    nst.ticket_nonce = toIOBuf("abc");
    nst.ticket = toIOBuf("123");
    auto records = encodeHandshakeAmongRecords(std::move(nst), 2);
    EXPECT_EQ(records.size(), 2);

    records.insert(
        records.begin() + 1,
        fizz::TLSMessage{
            fizz::ContentType::application_data, toIOBuf("splice")});
    EXPECT_EQ(records.size(), 3);

    for (size_t i = 0; i < records.size(); i++) {
      auto encrypted = clientWrite_->write(std::move(records[i]), {});
      encrypted.data->coalesce();
      EXPECT_EQ(
          encrypted.data->computeChainDataLength(),
          ::send(client_, encrypted.data->data(), encrypted.data->length(), 0));
    }
  }

  // getReadBuffer() will need to be called multiple times.
  std::array<char, 128> buf;
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillRepeatedly(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));
  EXPECT_CALL(mockReadCB_, readErr_(_)).WillOnce(Invoke([&](const auto& exc) {
    EXPECT_THAT(exc.what(), HasSubstr("received spliced"));
  }));

  serverConn_->setReadCB(&mockReadCB_);
  evb_.loop();
}

// The purpose of this test is to ensure that even alerts that splice
// pending handshake data will lead to a fatal connection error.
TEST_F(KTLSReadTest, SplicedHandshakeDataByAlertFailsConnection) {
  // Client writes a NewSessionTicket message with two records, but with
  // an application data record sandwiched between.
  {
    fizz::NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 20;
    nst.ticket_nonce = toIOBuf("abc");
    nst.ticket = toIOBuf("123");

    fizz::Alert alert(fizz::AlertDescription::close_notify);
    alert.level = 0x01;

    auto records = encodeHandshakeAmongRecords(std::move(nst), 2);
    EXPECT_EQ(records.size(), 2);

    records.insert(
        records.begin() + 1,
        fizz::TLSMessage{fizz::ContentType::alert, encode(std::move(alert))});
    EXPECT_EQ(records.size(), 3);

    for (size_t i = 0; i < records.size(); i++) {
      auto encrypted = clientWrite_->write(std::move(records[i]), {});
      encrypted.data->coalesce();
      EXPECT_EQ(
          encrypted.data->computeChainDataLength(),
          ::send(client_, encrypted.data->data(), encrypted.data->length(), 0));
    }
  }

  // getReadBuffer() will need to be called multiple times.
  std::array<char, 128> buf;
  EXPECT_CALL(mockReadCB_, getReadBuffer(_, _))
      .WillRepeatedly(Invoke([&](void** ret, size_t* size) {
        *ret = buf.data();
        *size = buf.size();
      }));
  EXPECT_CALL(mockReadCB_, readErr_(_)).WillOnce(Invoke([&](const auto& exc) {
    EXPECT_THAT(
        exc.what(),
        HasSubstr(
            "received non handshake data while currently processing handshake data"));
  }));

  serverConn_->setReadCB(&mockReadCB_);
  evb_.loop();
}
