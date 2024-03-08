/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialClientExtension.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialFactory.h>
#ifdef FIZZ_TOOL_ENABLE_BROTLI
#include <fizz/compression/BrotliCertificateDecompressor.h>
#endif
#include <fizz/compression/ZlibCertificateDecompressor.h>
#ifdef FIZZ_TOOL_ENABLE_ZSTD
#include <fizz/compression/ZstdCertificateDecompressor.h>
#endif
#include <fizz/client/PskSerializationUtils.h>
#ifdef FIZZ_TOOL_ENABLE_OQS
#include <fizz/experimental/protocol/HybridKeyExFactory.h>
#endif
#include <fizz/protocol/CertUtils.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <fizz/tool/CertificateVerifiers.h>
#include <fizz/tool/FizzCommandCommon.h>
#include <fizz/util/KeyLogWriter.h>
#include <fizz/util/Parse.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>
#include <folly/ssl/OpenSSLCertUtils.h>

#include <iostream>
#include <string>
#include <vector>

using namespace fizz::client;
using namespace folly;
using namespace folly::ssl;

namespace fizz {
namespace tool {
namespace {

void printUsage() {
  // clang-format off
  std::cerr
    << "Usage: client args\n"
    << "\n"
    << "Supported arguments:\n"
    << " -host host               (use connect instead)\n"
    << " -port port               (use connect instead)\n"
    << " -connect host:port       (set the address to connect to. Default: localhost:8443)\n"
    << " -verify                  (enable server cert verification. Default: false)\n"
    << " -cert cert               (PEM format client certificate to send if requested. Default: none)\n"
    << " -key key                 (PEM format private key for client certificate. Default: none)\n"
    << " -pass password           (private key password. Default: none)\n"
    << " -capath directory        (path to a directory of hashed formed CA certs used for verification.\n"
    << "                           The directory should contain one certificate or CRL per file in PEM format,\n"
    << "                           with a file name of the form hash.N for a certificate, or hash.rN for a CRL.\n"
    << "                           Refer to https://www.openssl.org/docs/man1.1.1/man1/rehash.html for how to generate such files.)\n"
    << " -cafile file             (path to a bundle file of CA certs used for verification; can be used with or without -capath.)\n"
    << " -reconnect               (after connecting, open another connection using a psk. Default: false)\n"
    << " -psk_save file           (after connecting, save the psk to file )\n"
    << " -psk_load file           (given file that contains a serialized psk, deserialize psk and open a connection with it)\n"
    << " -keylog file             (dump TLS secrets to a NSS key log file; for debugging purpose only)\n"
    << " -servername name         (server name to send in SNI. Default: same as host)\n"
    << " -alpn alpn1:...          (colon-separated list of ALPNs to send. Default: none)\n"
    << " -ciphers c1:...          (colon-separated list of ciphers in preference order. Default:\n"
    << "                           TLS_AES_128_GCM_SHA256,TLS_AES_256_GCM_SHA384,TLS_CHACHA20_POLY1305_SHA256)\n"
    << " -sigschemes s1:...       (colon-separated list of signature schemes in preference order.\n"
    << " -curves c1:...           (colon-separated list of supported ECDSA curves. Default: secp256r1, x25519)\n"
    << " -certcompression a1:...  (enables certificate compression support for given algorithms. Default: None)\n"
    << " -early                   (enables sending early data during resumption. Default: false)\n"
    << " -quiet                   (hide informational logging. Default: false)\n"
    << " -v verbosity             (set verbose log level for VLOG macros. Default: 0)\n"
    << " -vmodule m1=N,...        (set per-module verbose log level for VLOG macros. Default: none)\n"
    << " -httpproxy host:port     (set an HTTP proxy to use. Default: none)\n"
    << " -delegatedcred           (enable delegated credential support. Default: false)\n"
    << " -ech                     (use default values to simulate the sending of an encrypted client hello.)\n"
    << " -echconfigs file         (path to read ECH configs from. Format for contents is JSON.)\n"
    << "                          (JSON format: {echconfigs: [${your ECH config here with all the fields..}]})\n"
    << "                          (See FizzCommandCommonTest for an example.)\n"
    << "                          (Note: Setting ech configs implicitly enables ECH.)\n"
    << " -echbase64 echConfigList (base64 encoded string of echconfigs.)"
    << "                          (The echconfigs file argument must match the ECH Config List format specified in the ECH RFC.)\n"
#ifdef FIZZ_TOOL_ENABLE_OQS
    << " -hybridkex               (Use experimental hybrid key exchange. See Types.h for available hybrid named groups.)\n"
#endif
#ifdef FIZZ_TOOL_ENABLE_IO_URING
    << " -io_uring                (use io_uring for I/O. Default: false)\n"
    << " -io_uring_capacity N     (backend capacity for io_uring. Default: 128)\n"
    << " -io_uring_max_submit N   (maximum submit size for io_uring. Default: 64)\n"
    << " -io_uring_max_get N      (maximum get size for io_uring. Default: no limit)\n"
    << " -io_uring_register_fds   (use registered fds with io_uring. Default: false)\n"
    << " -io_uring_async_recv     (use async recv for io_uring. Default: false)\n"
#endif
  ;
  // clang-format on
}

class Connection : public AsyncSocket::ConnectCallback,
                   public AsyncFizzClient::HandshakeCallback,
                   public AsyncTransportWrapper::ReadCallback,
                   public AsyncTransport::ReplaySafetyCallback,
                   public InputHandlerCallback,
                   public SecretCollector {
 public:
  Connection(
      EventBase* evb,
      std::shared_ptr<FizzClientContext> clientContext,
      Optional<std::string> sni,
      std::shared_ptr<StoreCertificateChain> verifier,
      bool willResume,
      std::string proxyTarget,
      std::shared_ptr<ClientExtensions> extensions,
      folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
      bool registerEventCallback)
      : evb_(evb),
        clientContext_(clientContext),
        sni_(sni),
        verifier_(std::move(verifier)),
        willResume_(willResume),
        proxyTarget_(proxyTarget),
        extensions_(extensions),
        echConfigs_(std::move(echConfigs)),
        registerEventCallback_(registerEventCallback) {}

  void connect(const SocketAddress& addr) {
    sock_ = AsyncSocket::UniquePtr(new AsyncSocket(evb_));
    sock_->connect(this, addr);
  }

  void close() override {
    if (transport_) {
      transport_->close();
    } else if (sock_) {
      sock_->close();
    }
  }

  void connectErr(const AsyncSocketException& ex) noexcept override {
    LOG(ERROR) << "Connect error: " << ex.what();
    evb_->terminateLoopSoon();
  }

  void connectSuccess() noexcept override {
    LOG(INFO) << (willResume_ ? "Initial connection" : "Connection")
              << " established.";
    if (!proxyTarget_.empty()) {
      auto connectCommand = IOBuf::create(0);
      folly::io::Appender appender(connectCommand.get(), 10);
      format(
          "CONNECT {} HTTP/1.1\r\n"
          "Host: {}\r\n\r\n",
          proxyTarget_,
          proxyTarget_)(appender);
      sock_->setReadCB(this);
      sock_->writeChain(nullptr, std::move(connectCommand));
    } else {
      doHandshake();
    }
  }

  void doHandshake() {
    AsyncFizzBase::TransportOptions transportOpts;
    transportOpts.registerEventCallback = registerEventCallback_;
    transport_ = AsyncFizzClient::UniquePtr(new AsyncFizzClient(
        std::move(sock_),
        clientContext_,
        extensions_,
        std::move(transportOpts)));
    transport_->setSecretCallback(this);
    auto echConfigs = echConfigs_;
    transport_->connect(this, verifier_, sni_, sni_, std::move(echConfigs));
  }

  void fizzHandshakeSuccess(AsyncFizzClient* /*client*/) noexcept override {
    if (transport_->isReplaySafe()) {
      printHandshakeSuccess();
    } else {
      LOG(INFO) << "Early handshake success.";
      transport_->setReplaySafetyCallback(this);
    }
    transport_->setReadCB(this);
  }

  void fizzHandshakeError(
      AsyncFizzClient* /*client*/,
      exception_wrapper ex) noexcept override {
    LOG(ERROR) << "Handshake error: " << ex.what();
    evb_->terminateLoopSoon();
  }

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    *bufReturn = readBuf_.data();
    *lenReturn = readBuf_.size();
  }

  void readDataAvailable(size_t len) noexcept override {
    readBufferAvailable(IOBuf::copyBuffer(readBuf_.data(), len));
  }

  bool isBufferMovable() noexcept override {
    return true;
  }

  void readBufferAvailable(std::unique_ptr<IOBuf> buf) noexcept override {
    if (!transport_) {
      if (!proxyResponseBuffer_) {
        proxyResponseBuffer_ = std::move(buf);
      } else {
        proxyResponseBuffer_->prependChain(std::move(buf));
      }
      auto currentContents = StringPiece(proxyResponseBuffer_->coalesce());
      auto statusEndPos = currentContents.find("\r\n");
      if (statusEndPos == std::string::npos) {
        // No complete line yet
        return;
      }
      auto statusLine = currentContents.subpiece(0, statusEndPos).str();
      unsigned int httpVer;
      unsigned int httpStatus;
      if (sscanf(statusLine.c_str(), "HTTP/1.%u %u", &httpVer, &httpStatus) !=
          2) {
        LOG(ERROR) << "Failed to parse status: " << statusLine;
        close();
      }

      if (httpStatus / 100 != 2) {
        LOG(ERROR) << "Got non-200 status: " << httpStatus;
        close();
      }

      auto endPos = currentContents.find("\r\n\r\n");
      if (endPos != std::string::npos) {
        endPos += 4;
        auto remainder = currentContents.subpiece(endPos);
        sock_->setReadCB(nullptr);
        if (remainder.size()) {
          sock_->setPreReceivedData(IOBuf::copyBuffer(remainder));
        }
        doHandshake();
      }
    } else {
      std::cout << StringPiece(buf->coalesce()).str();
    }
  }

  void readEOF() noexcept override {
    LOG(INFO) << (willResume_ ? "Initial EOF" : "EOF");
    if (!willResume_) {
      evb_->terminateLoopSoon();
    }
  }

  void readErr(const AsyncSocketException& ex) noexcept override {
    LOG(ERROR) << "Read error: " << ex.what();
    evb_->terminateLoopSoon();
  }

  void onReplaySafe() override {
    printHandshakeSuccess();
  }

  bool connected() const override {
    return transport_ && !transport_->connecting() && transport_->good();
  }

  void write(std::unique_ptr<IOBuf> msg) override {
    if (transport_) {
      transport_->writeChain(nullptr, std::move(msg));
    }
  }

  void setKeyLogWriter(std::unique_ptr<KeyLogWriter> keyLogWriter) {
    keyLogger_ = std::move(keyLogWriter);
  }

 private:
  void printHandshakeSuccess() {
    auto& state = transport_->getState();
    auto serverCert = state.serverCert();
    auto clientCert = state.clientCert();
    LOG(INFO) << (willResume_ ? "Initial handshake" : "Handshake")
              << " succeeded.";
    LOG(INFO) << "  TLS Version: " << toString(*state.version());
    LOG(INFO) << "  Cipher Suite:  " << toString(*state.cipher());
    LOG(INFO) << "  Named Group: "
              << (state.group() ? toString(*state.group()) : "(none)");
    LOG(INFO) << "  Signature Scheme: "
              << (state.sigScheme() ? toString(*state.sigScheme()) : "(none)");
    LOG(INFO) << "  PSK: " << toString(*state.pskType());
    LOG(INFO) << "  PSK Mode: "
              << (state.pskMode() ? toString(*state.pskMode()) : "(none)");
    LOG(INFO) << "  Key Exchange Type: " << toString(*state.keyExchangeType());
    LOG(INFO) << "  Early: " << toString(*state.earlyDataType());
    LOG(INFO) << "  Server Identity: "
              << (serverCert ? serverCert->getIdentity() : "(none)");
    LOG(INFO) << "  Client Identity: "
              << (clientCert ? clientCert->getIdentity() : "(none)");

    LOG(INFO) << "  Certificate Chain:";
    auto certs = verifier_->getCerts();
    for (size_t i = 0; i < certs.size(); i++) {
      auto x509Cert = certs[i]->getX509();
      LOG(INFO) << "   " << i
                << " s:" << OpenSSLCertUtils::getSubject(*x509Cert).value();
      LOG(INFO) << "     i:" << OpenSSLCertUtils::getIssuer(*x509Cert).value();
    }

    if (auto opensslCert =
            dynamic_cast<const folly::OpenSSLTransportCertificate*>(
                serverCert.get())) {
      BioUniquePtr bio(BIO_new(BIO_s_mem()));
      if (!PEM_write_bio_X509(bio.get(), opensslCert->getX509().get())) {
        LOG(ERROR) << "  Couldn't convert server certificate to PEM: "
                   << SSLContext::getErrors();
      } else {
        BUF_MEM* bptr = nullptr;
        BIO_get_mem_ptr(bio.get(), &bptr);
        LOG(INFO) << "  Server Certificate:\n"
                  << std::string(bptr->data, bptr->length);
      }
    }

    if (auto opensslCert =
            dynamic_cast<const folly::OpenSSLTransportCertificate*>(
                clientCert.get())) {
      BioUniquePtr bio(BIO_new(BIO_s_mem()));
      if (!PEM_write_bio_X509(bio.get(), opensslCert->getX509().get())) {
        LOG(ERROR) << "  Couldn't convert client certificate to PEM: "
                   << SSLContext::getErrors();
      } else {
        BUF_MEM* bptr = nullptr;
        BIO_get_mem_ptr(bio.get(), &bptr);
        LOG(INFO) << "  Client Certificate:\n"
                  << std::string(bptr->data, bptr->length);
      }
    }
    LOG(INFO) << "  Server Certificate Compression: "
              << (state.serverCertCompAlgo()
                      ? toString(*state.serverCertCompAlgo())
                      : "(none)");
    LOG(INFO) << "  ALPN: " << state.alpn().value_or("(none)");
    LOG(INFO) << "  Client Random: "
              << folly::hexlify(*transport_->getClientRandom());
    LOG(INFO) << "  Secrets:";
    LOG(INFO) << "    External PSK Binder: " << secretStr(externalPskBinder_);
    LOG(INFO) << "    Resumption PSK Binder: "
              << secretStr(resumptionPskBinder_);
    LOG(INFO) << "    Early Exporter: " << secretStr(earlyExporterSecret_);
    LOG(INFO) << "    Early Client Data: "
              << secretStr(clientEarlyTrafficSecret_);
    LOG(INFO) << "    Client Handshake: "
              << secretStr(clientHandshakeTrafficSecret_);
    LOG(INFO) << "    Server Handshake: "
              << secretStr(serverHandshakeTrafficSecret_);
    LOG(INFO) << "    Exporter Master: " << secretStr(exporterMasterSecret_);
    LOG(INFO) << "    Resumption Master: "
              << secretStr(resumptionMasterSecret_);
    LOG(INFO) << "    Client Traffic: " << secretStr(clientAppTrafficSecret_);
    LOG(INFO) << "    Server Traffic: " << secretStr(serverAppTrafficSecret_);

    if (echConfigs_.has_value()) {
      printECHSuccess(state);
    }

    if (keyLogger_) {
      if (clientEarlyTrafficSecret_) {
        keyLogger_->write(
            *transport_->getClientRandom(),
            KeyLogWriter::Label::CLIENT_EARLY_TRAFFIC_SECRET,
            folly::range(*clientEarlyTrafficSecret_));
      }
      if (clientHandshakeTrafficSecret_) {
        keyLogger_->write(
            *transport_->getClientRandom(),
            KeyLogWriter::Label::CLIENT_HANDSHAKE_TRAFFIC_SECRET,
            folly::range(*clientHandshakeTrafficSecret_));
      }
      if (serverHandshakeTrafficSecret_) {
        keyLogger_->write(
            *transport_->getClientRandom(),
            KeyLogWriter::Label::SERVER_HANDSHAKE_TRAFFIC_SECRET,
            folly::range(*serverHandshakeTrafficSecret_));
      }
      if (exporterMasterSecret_) {
        keyLogger_->write(
            *transport_->getClientRandom(),
            KeyLogWriter::Label::EXPORTER_SECRET,
            folly::range(*exporterMasterSecret_));
      }
      if (clientAppTrafficSecret_) {
        keyLogger_->write(
            *transport_->getClientRandom(),
            KeyLogWriter::Label::CLIENT_TRAFFIC_SECRET_0,
            folly::range(*clientAppTrafficSecret_));
      }
      if (serverAppTrafficSecret_) {
        keyLogger_->write(
            *transport_->getClientRandom(),
            KeyLogWriter::Label::SERVER_TRAFFIC_SECRET_0,
            folly::range(*serverAppTrafficSecret_));
      }
    }
  }

  void printECHSuccess(const State& state) {
    LOG(INFO) << "  Encrypted client hello (ECH) requested: "
              << (state.echState().has_value() ? "Yes" : "No");
    if (state.echState().has_value()) {
      LOG(INFO) << "  Encrypted client hello (ECH) status: "
                << toString(state.echState()->status);

      // Get ECH config content
      const auto& echConfig = echConfigs_.value()[0];
      const auto& configContent = echConfig.ech_config_content;
      folly::io::Cursor cursor(configContent.get());
      auto echConfigContent = decode<ech::ECHConfigContentDraft>(cursor);

      auto ciphersuite = echConfigContent.key_config.cipher_suites[0];
      LOG(INFO) << "    Hash function: "
                << toString(getHashFunction(ciphersuite.kdf_id));
      LOG(INFO) << "    Cipher Suite: "
                << toString(getCipherSuite(ciphersuite.aead_id));
      LOG(INFO) << "    Named Group: "
                << toString(getKexGroup(echConfigContent.key_config.kem_id));
      LOG(INFO) << "    Fake SNI Used: "
                << echConfigContent.public_name->clone()->moveToFbString();
    }
  }

  EventBase* evb_;
  std::shared_ptr<FizzClientContext> clientContext_;
  Optional<std::string> sni_;
  std::shared_ptr<StoreCertificateChain> verifier_;
  AsyncSocket::UniquePtr sock_;
  AsyncFizzClient::UniquePtr transport_;
  bool willResume_{false};
  std::array<char, 8192> readBuf_;
  std::string proxyTarget_;
  std::unique_ptr<IOBuf> proxyResponseBuffer_;
  std::shared_ptr<ClientExtensions> extensions_;
  std::unique_ptr<KeyLogWriter> keyLogger_;
  folly::Optional<std::vector<ech::ECHConfig>> echConfigs_;
  bool registerEventCallback_{false};
};

class ResumptionPskCache : public BasicPskCache {
 public:
  ResumptionPskCache(folly::EventBase* evb, folly::Function<void()> callback)
      : evb_(evb), callback_(std::move(callback)) {}

  void putPsk(const std::string& identity, CachedPsk psk) override {
    BasicPskCache::putPsk(identity, std::move(psk));
    if (callback_) {
      evb_->runInLoop(std::move(callback_));
      callback_ = nullptr;
    }
  }

 private:
  folly::EventBase* evb_;
  folly::Function<void()> callback_;
};

class BasicPersistentPskCache : public BasicPskCache {
 public:
  BasicPersistentPskCache(std::string save_file, std::string load_file)
      : saveFile_(save_file), loadFile_(load_file) {}

  void putPsk(const std::string& /* unused */, CachedPsk psk) override {
    if (saveFile_.empty()) {
      return;
    }
    std::string serializedPsk = serializePsk(psk);
    if (writeFile(serializedPsk, saveFile_.c_str())) {
      LOG(INFO) << "\n Saved PSK to " << saveFile_ << " \n";
    } else {
      LOG(ERROR) << "\n Unable to save PSK " << saveFile_ << " \n";
    }
  }

  folly::Optional<CachedPsk> getPsk(const std::string& /* unused */) override {
    if (loadFile_.empty()) {
      return folly::none;
    }
    LOG(INFO) << "\n Loading PSK from " << loadFile_ << " \n";
    std::string serializedPsk;
    readFile(loadFile_.c_str(), serializedPsk);
    try {
      return deserializePsk(serializedPsk, OpenSSLFactory());
    } catch (const std::exception& e) {
      LOG(ERROR) << "Error deserializing: " << loadFile_ << "\n" << e.what();
      throw;
    }
  }

 private:
  std::string saveFile_, loadFile_;
};

} // namespace

int fizzClientCommand(const std::vector<std::string>& args) {
  std::string host = "localhost";
  uint16_t port = 8443;
  bool verify = false;
  std::string certPath;
  std::string keyPath;
  std::string keyPass;
  std::string caPath;
  std::string caFile;
  std::string pskSaveFile;
  std::string pskLoadFile;
  std::string keyLogFile;
  bool reconnect = false;
  std::string customSNI;
  std::vector<std::string> alpns;
#ifdef FIZZ_TOOL_ENABLE_OQS
  bool useHybridKexFactory = false;
#endif
  folly::Optional<std::vector<CertificateCompressionAlgorithm>> compAlgos;
  bool early = false;
  std::string proxyHost = "";
  uint16_t proxyPort = 0;
  std::vector<CipherSuite> ciphers {
    CipherSuite::TLS_AES_128_GCM_SHA256, CipherSuite::TLS_AES_256_GCM_SHA384,
#if FOLLY_OPENSSL_HAS_CHACHA
        CipherSuite::TLS_CHACHA20_POLY1305_SHA256,
#endif
  };
  std::vector<SignatureScheme> sigSchemes{
      SignatureScheme::ecdsa_secp256r1_sha256,
      SignatureScheme::ecdsa_secp384r1_sha384,
      SignatureScheme::ecdsa_secp521r1_sha512,
      SignatureScheme::rsa_pss_sha256,
  };
  std::vector<NamedGroup> groups{
      NamedGroup::secp256r1,
      NamedGroup::x25519,
  };
  bool delegatedCredentials = false;
  bool ech = false;
  std::string echConfigsFile;
  std::string echConfigsBase64;
  bool uring = false;
  bool uringAsync = false;
  bool uringRegisterFds = false;
  int32_t uringCapacity = 128;
  int32_t uringMaxSubmit = 64;
  int32_t uringMaxGet = -1;

  // clang-format off
  FizzArgHandlerMap handlers = {
    {"-host", {true, [&host](const std::string& arg) { host = arg; }}},
    {"-port", {true, [&port](const std::string& arg) {
        port = portFromString(arg, false);
    }}},
    {"-connect", {true, [&host, &port](const std::string& arg) {
        std::tie(host, port) = hostPortFromString(arg);
     }}},
    {"-verify", {false, [&verify](const std::string&) { verify = true; }}},
    {"-cert", {true, [&certPath](const std::string& arg) { certPath = arg; }}},
    {"-key", {true, [&keyPath](const std::string& arg) { keyPath = arg; }}},
    {"-pass", {true, [&keyPass](const std::string& arg) { keyPass = arg; }}},
    {"-capath", {true, [&caPath](const std::string& arg) { caPath = arg; }}},
    {"-cafile", {true, [&caFile](const std::string& arg) { caFile = arg; }}},
    {"-psk_save", {true, [&pskSaveFile](const std::string& arg) {
      pskSaveFile = arg;
    }}},
    {"-psk_load", {true,[&pskLoadFile](const std::string& arg) {
      pskLoadFile = arg;
    }}},
    {"-keylog", {true,[&keyLogFile](const std::string& arg) {
      keyLogFile = arg;
    }}},
    {"-reconnect", {false, [&reconnect](const std::string&) {
        reconnect = true;
    }}},
    {"-servername", {true, [&customSNI](const std::string& arg) {
        customSNI = arg;
    }}},
    {"-alpn", {true, [&alpns](const std::string& arg) {
        alpns.clear();
        folly::split(',', arg, alpns);
    }}},
    {"-certcompression", {true, [&compAlgos](const std::string& arg) {
        try {
          compAlgos = splitParse<CertificateCompressionAlgorithm>(arg);
        } catch (const std::exception& e) {
          LOG(ERROR) << "Error parsing certificate compression algorithms: " << e.what();
          throw;
        }
    }}},
    {"-early", {false, [&early](const std::string&) { early = true; }}},
    {"-quiet", {false, [](const std::string&) {
        FLAGS_minloglevel = google::GLOG_ERROR;
    }}},
    {"-httpproxy", {true, [&proxyHost, &proxyPort] (const std::string& arg) {
        std::tie(proxyHost, proxyPort) = hostPortFromString(arg);
    }}},
    {"-ciphers", {true, [&ciphers](const std::string& arg) {
        ciphers = splitParse<CipherSuite>(arg);
    }}},
    {"-sigschemes", {true, [&sigSchemes](const std::string& arg) {
        sigSchemes = splitParse<SignatureScheme>(arg);
    }}},
    {"-curves", {true, [&groups](const std::string& arg) {
        groups = splitParse<NamedGroup>(arg);
    }}},
    {"-delegatedcred", {false, [&delegatedCredentials](const std::string&) {
        delegatedCredentials = true;
    }}},
    {"-ech", {false, [&ech](const std::string&) {
        ech = true;
    }}},
    {"-echconfigs", {true, [&echConfigsFile](const std::string& arg) {
        echConfigsFile = arg;
    }}},
    {"-echbase64", {true, [&echConfigsBase64](const std::string& arg) {
        echConfigsBase64 = arg;
    }}}
#ifdef FIZZ_TOOL_ENABLE_OQS
    ,{"-hybridkex", {false, [&useHybridKexFactory](const std::string&) {
        useHybridKexFactory = true;
    }}}
#endif
#ifdef FIZZ_TOOL_ENABLE_IO_URING
    ,{"-io_uring", {false, [&uring](const std::string&) { uring = true; }}},
    {"-io_uring_async_recv", {false, [&uringAsync](const std::string&) {
        uringAsync = true;
    }}},
    {"-io_uring_register_fds", {false, [&uringRegisterFds](const std::string&) {
        uringRegisterFds = true;
    }}},
    {"-io_uring_capacity", {true, [&uringCapacity](const std::string& arg) {
        uringCapacity = folly::to<int32_t>(arg);
    }}},
    {"-io_uring_max_get", {true, [&uringMaxGet](const std::string& arg) {
        uringMaxGet = folly::to<int32_t>(arg);
    }}},
    {"-io_uring_max_submit", {true, [&uringMaxSubmit](const std::string& arg) {
        uringMaxSubmit = folly::to<int32_t>(arg);
    }}}
#endif
  };
  // clang-format on

  try {
    if (parseArguments(args, handlers, printUsage)) {
      // Parsing failed, return
      return 1;
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
    return 1;
  }

  // Sanity check input.
  if (certPath.empty() != keyPath.empty()) {
    LOG(ERROR) << "-cert and -key are both required when specified";
    return 1;
  }

  EventBase evb(folly::EventBase::Options().setBackendFactory([uring,
                                                               uringAsync,
                                                               uringRegisterFds,
                                                               uringCapacity,
                                                               uringMaxSubmit,
                                                               uringMaxGet] {
    return setupBackend(
        uring,
        uringAsync,
        uringRegisterFds,
        uringCapacity,
        uringMaxSubmit,
        uringMaxGet);
  }));

  auto clientContext = std::make_shared<FizzClientContext>();
#ifdef FIZZ_TOOL_ENABLE_OQS
  if (useHybridKexFactory) {
    clientContext->setFactory(std::make_shared<HybridKeyExFactory>());
  }
#endif

  if (!alpns.empty()) {
    clientContext->setSupportedAlpns(std::move(alpns));
  }

  clientContext->setSupportedCiphers(ciphers);
  clientContext->setSupportedSigSchemes(sigSchemes);
  clientContext->setSupportedGroups(groups);
  clientContext->setDefaultShares(groups);

  clientContext->setSupportedVersions(
      {ProtocolVersion::tls_1_3, ProtocolVersion::tls_1_3_28});
  clientContext->setSendEarlyData(early);

  if (compAlgos) {
    auto mgr = std::make_shared<CertDecompressionManager>();
    std::vector<std::shared_ptr<CertificateDecompressor>> decompressors;
    for (const auto& algo : *compAlgos) {
      switch (algo) {
        case CertificateCompressionAlgorithm::zlib:
          decompressors.push_back(
              std::make_shared<ZlibCertificateDecompressor>());
          break;
#ifdef FIZZ_TOOL_ENABLE_BROTLI
        case CertificateCompressionAlgorithm::brotli:
          decompressors.push_back(
              std::make_shared<BrotliCertificateDecompressor>());
          break;
#endif
#ifdef FIZZ_TOOL_ENABLE_ZSTD
        case CertificateCompressionAlgorithm::zstd:
          decompressors.push_back(
              std::make_shared<ZstdCertificateDecompressor>());
          break;
#endif
        default:
          LOG(WARNING) << "Don't know what decompressor to use for "
                       << toString(algo) << ", ignoring...";
          break;
      }
    }
    mgr->setDecompressors(decompressors);
    clientContext->setCertDecompressionManager(std::move(mgr));
  }

  X509StoreUniquePtr connStore;
  X509StoreUniquePtr resumptionStore;
  if (verify) {
    // Initialize CA store first, if given.
    if (!caPath.empty() || !caFile.empty()) {
      connStore.reset(X509_STORE_new());
      auto caFilePtr = caFile.empty() ? nullptr : caFile.c_str();
      auto caPathPtr = caPath.empty() ? nullptr : caPath.c_str();

      if (X509_STORE_load_locations(connStore.get(), caFilePtr, caPathPtr) ==
          0) {
        LOG(ERROR) << "Failed to load CA certificates";
        return 1;
      }
      resumptionStore.reset(connStore.get());
      X509_STORE_up_ref(resumptionStore.get());
    }
  }

  auto makeVerifier = [](X509StoreUniquePtr storePtr)
      -> std::unique_ptr<StoreCertificateChain> {
    std::unique_ptr<CertificateVerifier> verifier;
    if (storePtr) {
      verifier = std::make_unique<DefaultCertificateVerifier>(
          VerificationContext::Client, std::move(storePtr));
    } else {
      verifier = std::make_unique<InsecureAcceptAnyCertificate>();
    }
    auto storeChainVerifier =
        std::make_unique<StoreCertificateChain>(std::move(verifier));

    return storeChainVerifier;
  };

  auto connVerifier = makeVerifier(std::move(connStore));
  auto resumptionVerifier = makeVerifier(std::move(resumptionStore));

  if (!certPath.empty()) {
    std::string certData;
    std::string keyData;
    if (!readFile(certPath.c_str(), certData)) {
      LOG(ERROR) << "Failed to read certificate";
      return 1;
    } else if (!readFile(keyPath.c_str(), keyData)) {
      LOG(ERROR) << "Failed to read private key";
      return 1;
    }

    std::unique_ptr<SelfCert> cert;
    if (!keyPass.empty()) {
      cert = CertUtils::makeSelfCert(certData, keyData, keyPass);
    } else {
      cert = CertUtils::makeSelfCert(certData, keyData);
    }
    clientContext->setClientCertificate(std::move(cert));
  }

  std::shared_ptr<ClientExtensions> extensions;
  if (delegatedCredentials) {
    clientContext->setFactory(
        std::make_shared<extensions::DelegatedCredentialFactory>());
    extensions =
        std::make_shared<extensions::DelegatedCredentialClientExtension>(
            clientContext->getSupportedSigSchemes());
  }

  folly::Optional<ech::ECHConfigList> echConfigList = folly::none;

  if (ech) {
    // Use default ECH config values.
    auto echConfigContents = getDefaultECHConfigs();
    echConfigList->configs = std::move(echConfigContents);
  } else if (!echConfigsBase64.empty()) {
    echConfigList = parseECHConfigsBase64(echConfigsBase64);
    if (!echConfigList.has_value()) {
      LOG(ERROR) << "Unable to parse ECHConfigList base64.";
      return 1;
    }
  } else if (!echConfigsFile.empty()) {
    auto echConfigsJson = readECHConfigsJson(echConfigsFile);
    if (!echConfigsJson.has_value()) {
      LOG(ERROR) << "Unable to load ECH configs from json file";
      return 1;
    }
    echConfigList = parseECHConfigs(echConfigsJson.value());
    if (!echConfigList.has_value()) {
      LOG(ERROR)
          << "Unable to parse JSON file and make ECH config."
          << "Ensure the format matches what is expected."
          << "Rough example of format: {echconfigs: [${your ECH config here with all the fields..}]}"
          << "See FizzCommandCommonTest for a more concrete example.";
      return 1;
    }
  }

  folly::Optional<std::vector<ech::ECHConfig>> echConfigs;
  if (echConfigList.has_value()) {
    echConfigs = std::move(echConfigList->configs);
  }

  try {
    auto sni = customSNI.empty() ? host : customSNI;
    auto connectHost = proxyHost.empty() ? host : proxyHost;
    auto connectPort = proxyHost.empty() ? port : proxyPort;
    auto proxiedHost = proxyHost.empty()
        ? std::string()
        : folly::to<std::string>(host, ":", port);

    SocketAddress addr(connectHost, connectPort, true);
    Connection conn(
        &evb,
        clientContext,
        sni,
        std::move(connVerifier),
        reconnect,
        proxiedHost,
        extensions,
        std::move(echConfigs),
        uringAsync);
    Connection resumptionConn(
        &evb,
        clientContext,
        sni,
        std::move(resumptionVerifier),
        false,
        proxiedHost,
        extensions,
        folly::none,
        uringAsync);

    Connection* inputTarget = &conn;
    if (reconnect) {
      auto pskCache = std::make_shared<ResumptionPskCache>(
          &evb, [&conn, &resumptionConn, addr]() {
            conn.close();
            resumptionConn.connect(addr);
          });
      clientContext->setPskCache(pskCache);
      inputTarget = &resumptionConn;
    }
    if (!pskSaveFile.empty() || !pskLoadFile.empty()) {
      auto pskCache =
          std::make_shared<BasicPersistentPskCache>(pskSaveFile, pskLoadFile);
      clientContext->setPskCache(pskCache);
    }
    if (!keyLogFile.empty()) {
      conn.setKeyLogWriter(std::make_unique<KeyLogWriter>(keyLogFile));
    }
    TerminalInputHandler input(&evb, inputTarget);
    conn.connect(addr);
    evb.loop();
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
    return 1;
  }

  return 0;
}

} // namespace tool
} // namespace fizz
