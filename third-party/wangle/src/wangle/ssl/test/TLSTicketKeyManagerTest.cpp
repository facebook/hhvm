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

#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/test/AsyncSSLSocketTest.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/portability/OpenSSL.h>
#include <wangle/ssl/SSLStats.h>
#include <wangle/ssl/TLSTicketKeyManager.h>
#include <wangle/ssl/test/MockSSLStats.h>

#if defined(WANGLE_USE_FOLLY_TESTUTIL)
#include <folly/io/async/test/TestSSLServer.h>
#include <folly/testing/TestUtil.h>

namespace {
std::string get_resource(const char* res) {
  return folly::test::find_resource(res).string();
}
} // namespace

using folly::test::kTestCA;
using folly::test::kTestCert;
using folly::test::kTestKey;
#else
namespace {
std::string get_resource(const char* res) {
  return res;
}

const char* kTestCert = "folly/io/async/test/certs/tests-cert.pem";
const char* kTestKey = "folly/io/async/test/certs/tests-key.pem";
const char* kTestCA = "folly/io/async/test/certs/ca-cert.pem";
} // namespace
#endif

using ::testing::InSequence;
using wangle::MockSSLStats;

TEST(TLSTicketKeyManager, TestSetGetTLSTicketKeySeeds) {
  std::vector<std::string> origOld = {"67"};
  std::vector<std::string> origCurr = {"68"};
  std::vector<std::string> origNext = {"69"};

  wangle::TLSTicketKeyManager manager;

  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  std::vector<std::string> old;
  std::vector<std::string> curr;
  std::vector<std::string> next;
  manager.getTLSTicketKeySeeds(old, curr, next);
  ASSERT_EQ(origOld, old);
  ASSERT_EQ(origCurr, curr);
  ASSERT_EQ(origNext, next);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsSuccess) {
  MockSSLStats stats;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(2);

  std::vector<std::string> origOld = {"67", "77"};
  std::vector<std::string> origCurr = {"68", "78"};
  std::vector<std::string> origNext = {"69", "79"};

  // The new ticket seeds are compatible
  std::vector<std::string> newOld = {"68", "78"};
  std::vector<std::string> newCurr = {"69", "79"};
  std::vector<std::string> newNext = {"70", "80"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(newOld, newCurr, newNext);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsIdempotent) {
  MockSSLStats stats;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(2);

  std::vector<std::string> origOld = {"67", "77"};
  std::vector<std::string> origCurr = {"68", "78"};
  std::vector<std::string> origNext = {"69", "79"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsFailure) {
  MockSSLStats stats;
  InSequence inSequence;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(1);
  EXPECT_CALL(stats, recordTLSTicketRotation(false)).Times(1);

  std::vector<std::string> origOld = {"67", "77"};
  std::vector<std::string> origCurr = {"68", "78"};
  std::vector<std::string> origNext = {"69", "79"};

  // The new seeds are incompatible
  std::vector<std::string> newOld = {"69", "79"};
  std::vector<std::string> newCurr = {"70", "80"};
  std::vector<std::string> newNext = {"71", "81"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(newOld, newCurr, newNext);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsSubsetPass) {
  MockSSLStats stats;
  InSequence inSequence;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(2);

  std::vector<std::string> origOld = {"67"};
  std::vector<std::string> origCurr = {"68"};
  std::vector<std::string> origNext = {"69"};

  // The new ticket seeds are compatible
  std::vector<std::string> newOld = {"68", "78"};
  std::vector<std::string> newCurr = {"69"};
  std::vector<std::string> newNext = {"70", "80"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(newOld, newCurr, newNext);
}

class TestConnectCallback : public folly::AsyncSSLSocket::ConnectCallback {
 public:
  void connectSuccess() noexcept override {}
  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    FAIL() << "Expected success but got: " << ex.what();
  }
};

/**
 * Simple ReadCallback implementation used by AsyncSSLSocket to read
 * NewSessionTickets. Should not deliver any application data (shouldn't be any
 * anyway).
 */
class NewSessionTicketCb : public folly::AsyncTransport::ReadCallback {
 public:
  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    *bufReturn = buffer_;
    *lenReturn = sizeof(buffer_);
  }

  void readDataAvailable(size_t) noexcept override {
    FAIL();
  }
  void readEOF() noexcept override {
    // this will fire when the connection closes
  }
  void readErr(const folly::AsyncSocketException&) noexcept override {
    FAIL();
  }

 private:
  char buffer_[1024];
};

/**
 * Meant to verify our approach to bypassing the bug reported in
 * https://github.com/openssl/openssl/issues/18977, which is not backported to
 * OpenSSL 1.1.1.
 *
 * Performs a TLS handshake, confirms that the ticket key manager doesn't crash,
 * retrieves the session and attempts to resume it in a new connection. That
 * should fail as the ticket is non-resumable.
 */
TEST(
    TLSTicketKeyManager,
    TestNewSessionTicketGeneratedCorrectlyButNotResumable) {
  // The OpenSSL bug occurs with TLS 1.3 PSKs only, SSLContext should enable
  // TLS 1.3 by default.
  auto serverCtx = std::make_shared<folly::SSLContext>();
  serverCtx->loadCertificate(get_resource(kTestCert).c_str());
  serverCtx->loadPrivateKey(get_resource(kTestKey).c_str());

  // don't configure any seeds
  auto ticketHandler = std::make_unique<wangle::TLSTicketKeyManager>();
  serverCtx->setTicketHandler(std::move(ticketHandler));

  // start listening on a local port
  folly::test::ReadCallback readCallback(nullptr);
  readCallback.state = folly::test::STATE_SUCCEEDED;
  folly::test::HandshakeCallback handshakeCallback(&readCallback);
  folly::test::SSLServerAcceptCallback acceptCallback(&handshakeCallback);
  folly::test::TestSSLServer server(&acceptCallback, std::move(serverCtx));

  folly::EventBase evb;
  auto clientCtx = std::make_shared<folly::SSLContext>();
  clientCtx->setVerificationOption(
      folly::SSLContext::VerifyServerCertificate::IF_PRESENTED);
  clientCtx->loadTrustedCertificates(get_resource(kTestCA).c_str());

  TestConnectCallback connectCallback;
  // connect and grab the session (ticket)
  auto client = folly::AsyncSSLSocket::newSocket(clientCtx, &evb);
  client->connect(&connectCallback, server.getAddress());
  evb.loop();

  NewSessionTicketCb newSessionTicketCb;
  client->setReadCB(&newSessionTicketCb);
  // should be sufficient to read the NewSessionTicket(s)
  evb.loopOnce();

  auto sslSession = client->getSSLSession();
  ASSERT_TRUE(sslSession != nullptr);

  client = folly::AsyncSSLSocket::newSocket(clientCtx, &evb);
  client->setSSLSession(std::move(sslSession));
  client->connect(&connectCallback, server.getAddress());
  evb.loop();

  EXPECT_FALSE(client->getSSLSessionReused());
}

TEST(TLSTicketKeyManager, RenewTicketsOnResumeDefaultOff) {
  wangle::TLSTicketKeyManager manager;
  manager.setTLSTicketKeySeeds({"67"}, {"68"}, {"69"});

  unsigned char keyName[16] = {};
  unsigned char iv[EVP_MAX_IV_LENGTH] = {};
  EVP_CIPHER_CTX* cipherCtx = EVP_CIPHER_CTX_new();
  HMAC_CTX* hmacCtx = HMAC_CTX_new();

  // Encrypt a ticket
  int encResult = manager.ticketCallback(
      nullptr, keyName, iv, cipherCtx, hmacCtx, 1 /* encrypt */);
  EXPECT_EQ(encResult, 1);

  // Decrypt should return 1 (do not renew) by default
  int decResult = manager.ticketCallback(
      nullptr, keyName, iv, cipherCtx, hmacCtx, 0 /* decrypt */);
  EXPECT_EQ(decResult, 1);

  EVP_CIPHER_CTX_free(cipherCtx);
  HMAC_CTX_free(hmacCtx);
}

TEST(TLSTicketKeyManager, RenewTicketsOnResumeWithNullSsl) {
  wangle::TLSTicketKeyManager manager;
  manager.setTLSTicketKeySeeds({"67"}, {"68"}, {"69"});
  manager.setRenewTicketsOnResume(true);

  unsigned char keyName[16] = {};
  unsigned char iv[EVP_MAX_IV_LENGTH] = {};
  EVP_CIPHER_CTX* cipherCtx = EVP_CIPHER_CTX_new();
  HMAC_CTX* hmacCtx = HMAC_CTX_new();

  // Encrypt a ticket
  int encResult = manager.ticketCallback(
      nullptr, keyName, iv, cipherCtx, hmacCtx, 1 /* encrypt */);
  EXPECT_EQ(encResult, 1);

  // With null SSL, renewal should not trigger even when enabled
  int decResult = manager.ticketCallback(
      nullptr, keyName, iv, cipherCtx, hmacCtx, 0 /* decrypt */);
  EXPECT_EQ(decResult, 1);

  EVP_CIPHER_CTX_free(cipherCtx);
  HMAC_CTX_free(hmacCtx);
}

TEST(TLSTicketKeyManager, RenewTicketsOnResumeTLS13Returns2) {
  wangle::TLSTicketKeyManager manager;
  manager.setTLSTicketKeySeeds({"67"}, {"68"}, {"69"});
  manager.setRenewTicketsOnResume(true);

  // Create a minimal SSL object configured for TLS 1.3
  SSL_CTX* sslCtx = SSL_CTX_new(TLS_method());
  ASSERT_NE(sslCtx, nullptr);
  SSL_CTX_set_min_proto_version(sslCtx, TLS1_3_VERSION);
  SSL_CTX_set_max_proto_version(sslCtx, TLS1_3_VERSION);
  SSL* ssl = SSL_new(sslCtx);
  ASSERT_NE(ssl, nullptr);

  unsigned char keyName[16] = {};
  unsigned char iv[EVP_MAX_IV_LENGTH] = {};
  EVP_CIPHER_CTX* cipherCtx = EVP_CIPHER_CTX_new();
  HMAC_CTX* hmacCtx = HMAC_CTX_new();

  // Encrypt a ticket to populate keyName with the current encryption key
  int encResult = manager.ticketCallback(
      ssl, keyName, iv, cipherCtx, hmacCtx, 1 /* encrypt */);
  EXPECT_EQ(encResult, 1);

  // Decrypt with TLS 1.3 SSL and renewal enabled should return 2
  int decResult = manager.ticketCallback(
      ssl, keyName, iv, cipherCtx, hmacCtx, 0 /* decrypt */);
  EXPECT_EQ(decResult, 2);

  EVP_CIPHER_CTX_free(cipherCtx);
  HMAC_CTX_free(hmacCtx);
  SSL_free(ssl);
  SSL_CTX_free(sslCtx);
}
