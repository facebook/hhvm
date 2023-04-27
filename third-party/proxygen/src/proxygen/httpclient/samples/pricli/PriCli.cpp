/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GFlags.h>

#include <folly/SocketAddress.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/SSLOptions.h>
#include <proxygen/lib/http/HTTPConnector.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>

using namespace folly;
using namespace proxygen;

DEFINE_string(server, "localhost", "server to connect to");
DEFINE_int32(port, 443, "port to connect to");
DEFINE_bool(use_tls, true, "use TLS to connect");
DEFINE_string(ca_path,
              "/etc/ssl/certs/ca-certificates.crt",
              "Path to trusted CA file"); // default for Ubuntu 14.04
DEFINE_int32(http_client_connect_timeout,
             1000,
             "connect timeout in milliseconds");
DEFINE_int32(request_timeout, 1000, "Request timeout in milliseconds");
DEFINE_int32(recv_window, 65536, "Flow control receive window for h2/spdy");
DEFINE_string(next_protos,
              "h2,h2-14,spdy/3.1,spdy/3,http/1.1",
              "Next protocol string for NPN/ALPN");
DEFINE_string(plaintext_proto, "", "plaintext protocol");
DEFINE_bool(excl, true, "make high-pri exclusive");

namespace {

class PriorityClient
    : public proxygen::HTTPConnector::Callback
    , public AsyncTimeout {
 public:
  explicit PriorityClient(folly::EventBase* evb) : AsyncTimeout(evb) {
  }

 private:
  class InfiniteHandler : public proxygen::HTTPTransactionHandler {
   public:
    explicit InfiniteHandler(PriorityClient& priCli)
        : priCli_(priCli), start_(std::chrono::steady_clock::now()) {
    }

    void setTransaction(HTTPTransaction*) noexcept override {
    }

    void detachTransaction() noexcept override {
      delete this;
    }

    void onHeadersComplete(
        std::unique_ptr<HTTPMessage> headers) noexcept override {
      LOG(INFO) << "Infinite headers; code=" << headers->getStatusCode();
    }

    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
      auto len = body->computeChainDataLength();
      priCli_.onLowPri(len);
      total_ += len;
      VLOG(1) << "Infinite body len=" << len << " total=" << total_;
    }

    void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept override {
    }

    void onEOM() noexcept override {
      auto finish = std::chrono::steady_clock::now();
      LOG(INFO) << "Infinite EOM, lat="
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       finish - start_)
                       .count();
      priCli_.sendLong();
    }

    void onError(const HTTPException& error) noexcept override {
      LOG(ERROR) << "Infinite error=" << error.what();
      exit(1);
    }

    void onUpgrade(UpgradeProtocol) noexcept override {
    }
    void onEgressPaused() noexcept override {
    }
    void onEgressResumed() noexcept override {
    }

    PriorityClient& priCli_;
    uint64_t total_{0};
    std::chrono::steady_clock::time_point start_;
  };

  class SegmentHandler : public proxygen::HTTPTransaction::Handler {
   private:
    uint32_t segNo_{0};
    PriorityClient& priCli_;
    std::chrono::steady_clock::time_point start_;

   public:
    SegmentHandler(uint32_t segNo, PriorityClient& priCli)
        : segNo_(segNo),
          priCli_(priCli),
          start_(std::chrono::steady_clock::now()) {
    }

    void setTransaction(HTTPTransaction*) noexcept override {
    }

    void detachTransaction() noexcept override {
      delete this;
    }

    void onHeadersComplete(
        std::unique_ptr<HTTPMessage> headers) noexcept override {
      LOG(INFO) << "Segment=" << segNo_
                << " headers; code=" << headers->getStatusCode();
      priCli_.highPriHeaders();
    }

    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
      VLOG(1) << "Segment=" << segNo_
              << " body len=" << body->computeChainDataLength();
    }

    void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept override {
    }

    void onEOM() noexcept override {
      auto finish = std::chrono::steady_clock::now();
      LOG(INFO) << "Segment=" << segNo_ << " EOM, lat="
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       finish - start_)
                       .count();
      priCli_.highPriComplete();
      priCli_.scheduleTimeout(2000);
    }

    void onError(const HTTPException& error) noexcept override {
      LOG(ERROR) << "Segment=" << segNo_ << " error=" << error.what();
      priCli_.scheduleTimeout(2000);
    }

    void onUpgrade(UpgradeProtocol) noexcept override {
    }
    void onEgressPaused() noexcept override {
    }
    void onEgressResumed() noexcept override {
    }
  };

  // HTTPConnector methods
  void connectSuccess(proxygen::HTTPUpstreamSession* session) override {
    session_ = session;
    session_->setFlowControl(
        FLAGS_recv_window, FLAGS_recv_window, FLAGS_recv_window * 10);
    lowPriID_ = session_->sendPriority({0, true, 255});
    sendLong();
    timeoutExpired();
  }

  void sendLong() {
    sendRequest(
        "/1000000000", {lowPriID_, false, 0}, new InfiniteHandler(*this));
  }

  void connectError(const folly::AsyncSocketException& ex) override {
    LOG(ERROR) << "Coudln't connect to " << FLAGS_server << ":" << FLAGS_port
               << ":" << ex.what();
  }

  void timeoutExpired() noexcept override {
    LOG(INFO) << "Requesting Segment=" << segNo_;
    sendRequest(
        "/128000", {0, FLAGS_excl, 255}, new SegmentHandler(segNo_++, *this));
    lowPriSinceHighPri_ = 0;
  }

  void onLowPri(size_t len) {
    lowPriSinceHighPri_ += len;
  }

  void highPriHeaders() {
    LOG(INFO) << "lowPriSinceHighPri_ before headers=" << lowPriSinceHighPri_;
    lowPriSinceHighPri_ = 0;
  }

  void highPriComplete() {
    LOG(INFO) << "lowPriSinceHighPri_ before eom=" << lowPriSinceHighPri_;
  }

  HTTPCodec::StreamID sendRequest(std::string url,
                                  http2::PriorityUpdate pri,
                                  HTTPTransactionHandler* handler) {
    HTTPMessage req;
    req.setMethod(HTTPMethod::GET);
    req.setURL(url);
    req.setHTTP2Priority({pri.streamDependency, pri.exclusive, pri.weight});
    req.setHTTPVersion(1, 1);
    auto txn = session_->newTransaction(handler);
    CHECK(txn);
    auto streamID = txn->getID();
    txn->sendHeadersWithOptionalEOM(req, true);
    return streamID;
  }

  HTTPUpstreamSession* session_;
  uint32_t segNo_{0};
  HTTPCodec::StreamID lowPriID_;
  size_t lowPriSinceHighPri_{0};
};

std::shared_ptr<folly::SSLContext> initializeSsl(
    const std::string& caPath, const std::string& nextProtos) {
  auto sslContext = std::make_shared<folly::SSLContext>();
  sslContext->setOptions(SSL_OP_NO_COMPRESSION);
  folly::ssl::setCipherSuites<folly::ssl::SSLCommonOptions>(*sslContext);
  if (!caPath.empty()) {
    sslContext->loadTrustedCertificates(caPath.c_str());
  }
  std::list<std::string> nextProtoList;
  folly::splitTo<std::string>(
      ',', nextProtos, std::inserter(nextProtoList, nextProtoList.begin()));
  sslContext->setAdvertisedNextProtocols(nextProtoList);
  return sslContext;
}

} // namespace

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  EventBase evb;
  PriorityClient pricli(&evb);

  SocketAddress addr(FLAGS_server, FLAGS_port, true);
  LOG(INFO) << "Trying to connect to " << addr;

  HTTPConnector connector(
      &pricli,
      WheelTimerInstance(std::chrono::milliseconds(FLAGS_request_timeout),
                         &evb));
  if (!FLAGS_plaintext_proto.empty()) {
    connector.setPlaintextProtocol(FLAGS_plaintext_proto);
  }
  static const SocketOptionMap opts{{{SOL_SOCKET, SO_REUSEADDR}, 1}};

  if (FLAGS_use_tls) {
    auto sslContext = initializeSsl(FLAGS_ca_path, FLAGS_next_protos);
    connector.connectSSL(
        &evb,
        addr,
        sslContext,
        nullptr,
        std::chrono::milliseconds(FLAGS_http_client_connect_timeout),
        opts,
        folly::AsyncSocket::anyAddress(),
        FLAGS_server);
  } else {
    connector.connect(
        &evb,
        addr,
        std::chrono::milliseconds(FLAGS_http_client_connect_timeout),
        opts);
  }

  evb.loop();

  return EXIT_SUCCESS;
}
