/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/crypto/RandomGenerator.h>
#include <fizz/crypto/Utils.h>
#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/TicketTypes.h>
#include <folly/String.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/SSLContext.h>
#include <folly/portability/GFlags.h>

using namespace fizz;
using namespace fizz::client;
using namespace fizz::server;
using namespace folly;
using namespace folly::ssl;

DEFINE_int32(port, 0, "port to connect to");
DEFINE_bool(server, false, "act as a server, otherwise act as a client");
DEFINE_string(key_file, "", "key file");
DEFINE_string(cert_file, "", "cert file");
DEFINE_int32(resume_count, 0, "number of additional connections to open");

static constexpr int kUnimplemented = 89;

static std::vector<std::string> kKnownFlags{
    "port",
    "server",
    "key_file",
    "cert_file",
    "resume_count"};

class BogoTestServer : public AsyncSocket::ConnectCallback,
                       public AsyncFizzServer::HandshakeCallback,
                       public AsyncSSLSocket::HandshakeCB,
                       public AsyncTransportWrapper::ReadCallback {
 public:
  BogoTestServer(
      EventBase* evb,
      uint16_t port,
      std::shared_ptr<FizzServerContext> serverContext,
      std::shared_ptr<SSLContext> sslContext)
      : evb_(evb), serverContext_(serverContext), sslContext_(sslContext) {
    socket_ = AsyncSocket::UniquePtr(new AsyncSocket(evb));
    socket_->connect(this, "::", port, 1000);
  }

  void connectSuccess() noexcept override {
    transport_ = AsyncFizzServer::UniquePtr(
        new AsyncFizzServer(std::move(socket_), serverContext_));
    transport_->accept(this);
  }

  void connectErr(const AsyncSocketException& ex) noexcept override {
    LOG(INFO) << "TCP connect failed: " << ex.what();
    socket_.reset();
    success_ = false;
  }

  void fizzHandshakeSuccess(AsyncFizzServer*) noexcept override {
    success_ = true;
    transport_->setReadCB(this);
  }

  void fizzHandshakeError(
      AsyncFizzServer*,
      folly::exception_wrapper ex) noexcept override {
    LOG(INFO) << "Handshake error: " << ex.what();
    transport_.reset();
    success_ = false;
  }

  void fizzHandshakeAttemptFallback(AttemptVersionFallback fallback) override {
    auto fd = transport_->getUnderlyingTransport<AsyncSocket>()
                  ->detachNetworkSocket()
                  .toFd();
    transport_.reset();
    if (!sslContext_) {
      unimplemented_ = true;
    } else {
      sslSocket_ = AsyncSSLSocket::UniquePtr(new AsyncSSLSocket(
          sslContext_, evb_, folly::NetworkSocket::fromFd(fd)));
      sslSocket_->setPreReceivedData(std::move(fallback.clientHello));
      sslSocket_->sslAccept(this);
    }
  }

  void getReadBuffer(void** /* bufReturn */, size_t* /* lenReturn */) override {
    throw std::runtime_error("getReadBuffer not implemented");
  }

  void readDataAvailable(size_t /* len */) noexcept override {
    CHECK(false) << "readDataAvailable not implemented";
  }

  bool isBufferMovable() noexcept override {
    return true;
  }

  void readBufferAvailable(std::unique_ptr<IOBuf> buf) noexcept override {
    io::Cursor cursor(buf.get());
    std::unique_ptr<IOBuf> write = IOBuf::create(0);
    io::Appender appender(write.get(), 50);
    while (!cursor.isAtEnd()) {
      uint8_t byte;
      cursor.pull(&byte, 1);
      byte ^= 0xff;
      appender.push(&byte, 1);
    }
    transport_->writeChain(nullptr, std::move(write));
  }

  void readEOF() noexcept override {}

  void readErr(const AsyncSocketException&) noexcept override {}

  void handshakeSuc(folly::AsyncSSLSocket*) noexcept override {
    success_ = true;
  }

  void handshakeErr(
      folly::AsyncSSLSocket*,
      const folly::AsyncSocketException& ex) noexcept override {
    LOG(INFO) << "SSL Handshake error: " << ex.what();
    sslSocket_.reset();
    success_ = false;
  }

  bool unimplemented() const {
    return unimplemented_;
  }

  bool success() const {
    return *success_;
  }

 private:
  EventBase* evb_;
  AsyncSocket::UniquePtr socket_;

  std::shared_ptr<FizzServerContext> serverContext_;
  AsyncFizzServer::UniquePtr transport_;

  std::shared_ptr<SSLContext> sslContext_;
  AsyncSSLSocket::UniquePtr sslSocket_;

  bool unimplemented_{false};
  Optional<bool> success_;
};

class BogoTestClient : public AsyncSocket::ConnectCallback,
                       public AsyncFizzClient::HandshakeCallback,
                       public AsyncTransportWrapper::ReadCallback {
 public:
  BogoTestClient(
      EventBase* evb,
      uint16_t port,
      std::shared_ptr<const FizzClientContext> clientContext)
      : clientContext_(clientContext) {
    socket_ = AsyncSocket::UniquePtr(new AsyncSocket(evb));
    socket_->connect(this, "::", port, 1000);
  }

  void connectSuccess() noexcept override {
    transport_ = AsyncFizzClient::UniquePtr(
        new AsyncFizzClient(std::move(socket_), clientContext_));
    transport_->connect(
        this,
        nullptr,
        folly::none,
        std::string("resumption-id"),
        folly::Optional<std::vector<fizz::ech::ECHConfig>>(folly::none));
  }

  void connectErr(const AsyncSocketException& ex) noexcept override {
    LOG(INFO) << "TCP connect failed: " << ex.what();
    socket_.reset();
    success_ = false;
  }

  void fizzHandshakeSuccess(AsyncFizzClient*) noexcept override {
    success_ = true;
    transport_->setReadCB(this);
  }

  void fizzHandshakeError(
      AsyncFizzClient*,
      folly::exception_wrapper ex) noexcept override {
    LOG(INFO) << "Handshake error: " << ex.what();
    transport_.reset();

    // If the server sent us a protocol_version alert assume that
    if (ex.what().find(
            "received alert: protocol_version, in state ExpectingServerHello") !=
        std::string::npos) {
      unimplemented_ = true;
    }
    success_ = false;
  }

  void getReadBuffer(void** /* bufReturn */, size_t* /* lenReturn */) override {
    throw std::runtime_error("getReadBuffer not implemented");
  }

  void readDataAvailable(size_t /* len */) noexcept override {
    CHECK(false) << "readDataAvailable not implemented";
  }

  bool isBufferMovable() noexcept override {
    return true;
  }

  void readBufferAvailable(std::unique_ptr<IOBuf> buf) noexcept override {
    io::Cursor cursor(buf.get());
    std::unique_ptr<IOBuf> write = IOBuf::create(0);
    io::Appender appender(write.get(), 50);
    while (!cursor.isAtEnd()) {
      uint8_t byte;
      cursor.pull(&byte, 1);
      byte ^= 0xff;
      appender.push(&byte, 1);
    }
    transport_->writeChain(nullptr, std::move(write));
  }

  void readEOF() noexcept override {}

  void readErr(const AsyncSocketException&) noexcept override {}

  bool unimplemented() const {
    return unimplemented_;
  }

  bool success() const {
    return *success_;
  }

 private:
  AsyncSocket::UniquePtr socket_;

  std::shared_ptr<const FizzClientContext> clientContext_;
  AsyncFizzClient::UniquePtr transport_;

  bool unimplemented_{false};
  Optional<bool> success_;
};

class TestRsaCert : public OpenSSLSelfCertImpl<KeyType::RSA> {
 public:
  using OpenSSLSelfCertImpl<KeyType::RSA>::OpenSSLSelfCertImpl;
  std::string getIdentity() const override {
    return "testrsacert";
  }
};

class TestP256Cert : public OpenSSLSelfCertImpl<KeyType::P256> {
 public:
  using OpenSSLSelfCertImpl<KeyType::P256>::OpenSSLSelfCertImpl;
  std::string getIdentity() const override {
    return "testp256cert";
  }
};

std::unique_ptr<SelfCert> readSelfCert() {
  BioUniquePtr b(BIO_new(BIO_s_file()));
  BIO_read_filename(b.get(), FLAGS_cert_file.c_str());
  std::vector<X509UniquePtr> certs;
  while (true) {
    X509UniquePtr x509(PEM_read_bio_X509(b.get(), nullptr, nullptr, nullptr));
    if (!x509) {
      break;
    } else {
      certs.push_back(std::move(x509));
    }
  }
  if (certs.empty()) {
    throw std::runtime_error("could not read cert");
  }

  b.reset(BIO_new(BIO_s_file()));
  BIO_read_filename(b.get(), FLAGS_key_file.c_str());
  EvpPkeyUniquePtr key(
      PEM_read_bio_PrivateKey(b.get(), nullptr, nullptr, nullptr));

  std::unique_ptr<SelfCert> cert;
  if (EVP_PKEY_id(key.get()) == EVP_PKEY_RSA) {
    return std::make_unique<TestRsaCert>(std::move(key), std::move(certs));
  } else if (EVP_PKEY_id(key.get()) == EVP_PKEY_EC) {
    return std::make_unique<TestP256Cert>(std::move(key), std::move(certs));
  } else {
    throw std::runtime_error("unknown cert type");
  }
}

int serverTest() {
  auto certManager = std::make_shared<CertManager>();
  certManager->addCert(readSelfCert(), true);

  auto serverContext = std::make_shared<FizzServerContext>();
  serverContext->setCertManager(certManager);
  serverContext->setSupportedAlpns({"h2", "http/1.1"});
  serverContext->setVersionFallbackEnabled(true);

  auto ticketCipher = std::make_shared<AES128TicketCipher>(
      serverContext->getFactoryPtr(), std::move(certManager));
  auto ticketSeed = RandomGenerator<32>().generateRandom();
  ticketCipher->setTicketSecrets({{range(ticketSeed)}});
  server::TicketPolicy policy;
  policy.setTicketValidity(std::chrono::seconds(60));
  ticketCipher->setPolicy(std::move(policy));

  serverContext->setTicketCipher(ticketCipher);

  EventBase evb;
  std::vector<std::unique_ptr<BogoTestServer>> servers;
  for (size_t i = 0; i <= size_t(FLAGS_resume_count); i++) {
    servers.push_back(std::make_unique<BogoTestServer>(
        &evb, FLAGS_port, serverContext, nullptr));
  }
  evb.loop();
  for (const auto& server : servers) {
    if (server->unimplemented()) {
      LOG(INFO) << "Testing unimplemented feature.";
      return kUnimplemented;
    }
  }
  for (const auto& server : servers) {
    if (!server->success()) {
      LOG(INFO) << "Connection failed.";
      return 1;
    }
  }

  return 0;
}

int clientTest() {
  auto clientContext = std::make_shared<FizzClientContext>();
  clientContext->setCompatibilityMode(true);

  if (!FLAGS_cert_file.empty()) {
    clientContext->setClientCertificate(readSelfCert());
  }

  EventBase evb;
  if (FLAGS_resume_count >= 1) {
    return kUnimplemented;
  }
  auto client =
      std::make_unique<BogoTestClient>(&evb, FLAGS_port, clientContext);
  evb.loop();
  if (client->unimplemented()) {
    LOG(INFO) << "Testing unimplemented feature.";
    return kUnimplemented;
  }
  if (!client->success()) {
    LOG(INFO) << "Connection failed.";
    return 1;
  }

  return 0;
}

int main(int argc, char** argv) {
  // Convert "-" in args to "_" so that we can use GFLAGS.
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (char* j = argv[i] + 2; *j; j++) {
        if (*j == '-') {
          *j = '_';
        }
      }
      if (std::find(
              kKnownFlags.begin(),
              kKnownFlags.end(),
              std::string(argv[i] + 1)) == kKnownFlags.end()) {
        LOG(INFO) << "unknown flag: " << argv[i];
        return kUnimplemented;
      }
    }
  }

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  CryptoUtils::init();

  if (FLAGS_port == 0) {
    throw std::runtime_error("must specify port");
  }

  if (FLAGS_server) {
    return serverTest();
  } else {
    return clientTest();
  }
}
