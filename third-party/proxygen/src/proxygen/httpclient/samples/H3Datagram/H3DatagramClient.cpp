/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <proxygen/httpclient/samples/curl/CurlClient.h>
#include <proxygen/httpserver/samples/hq/InsecureVerifierDangerousDoNotUseInProduction.h>
#include <proxygen/lib/transport/ConnectUDPUtils.h>
#include <proxygen/lib/transport/H3DatagramAsyncSocket.h>

using namespace folly;
using namespace proxygen;

DEFINE_string(host, "::1", "Remote Server hostname/IP");
DEFINE_int32(port, 8888, "Remote Server port");

DEFINE_string(proxy_host, "::1", "Proxy hostname/IP");
DEFINE_int32(proxy_port, 6666, "Proxy port");

DEFINE_string(cert, "", "Certificate file path");
DEFINE_string(key, "", "Private key file path");

DEFINE_string(headers, "", "List of N=V headers separated by ,");

DEFINE_string(
    masque_template,
    "",
    "URI template for RFC 9298 CONNECT-UDP (e.g., "
    "https://proxy:4443/masque?h={target_host}&p={target_port}). "
    "When set, uses RFC 9298 Extended CONNECT instead of legacy CONNECT-UDP.");

constexpr size_t kMaxReadBufferSize{1232};

namespace {

/*
 * This class provides a sample client that speaks HTTP/3 Datagrams.
 * It supports connecting to a remote UDP server through a proxy that supports
 * masque connect-udp.
 *
 * The sample application logic implements the following.
 * The datagram payload contains just an ASCII encoded integer number:
 *  - the client starts by sending a datagram with 0
 *  - on receiving an equally encoded datagram from the server, the client
 *    increment the received number by 1 and sends a new datagram
 *  - the client stops when reaching a maximum configured integer value
 */
class DatagramClient
    : private folly::AsyncUDPSocket::ReadCallback
    , private folly::AsyncTimeout {
 public:
  using folly::AsyncUDPSocket::ReadCallback::OnDataAvailableParams;

  ~DatagramClient() override = default;

  explicit DatagramClient(folly::EventBase* evb, H3DatagramAsyncSocket& socket)
      : folly::AsyncTimeout(evb), evb_(evb), socket_(socket) {
  }

  void start() {
    CHECK(evb_->isInEventBaseThread());
    try {
      socket_.connect(SocketAddress(FLAGS_proxy_host, FLAGS_proxy_port));
    } catch (const std::system_error& e) {
      LOG(ERROR) << "Failed to connect to " << FLAGS_proxy_host << ":"
                 << FLAGS_proxy_port << ": " << e.what();
      return;
    }
    socket_.resumeRead(this);

    sendPing();
  }

  void shutdown() {
    CHECK(evb_->isInEventBaseThread());
    socket_.pauseRead();
    socket_.close();
    closing_ = true;
  }

  void sendPing() {
    if (n_ == 2000) {
      shutdown();
      return;
    }

    scheduleTimeout(1000);
    writePing(folly::IOBuf::copyBuffer(folly::to<std::string>(n_)));
  }

  virtual void writePing(std::unique_ptr<folly::IOBuf> buf) {
    VLOG(2) << "Writing Datagram";
    auto res =
        socket_.write(SocketAddress(FLAGS_proxy_host, FLAGS_proxy_port), buf);
    if (res < 0) {
      LOG(ERROR) << "Failure to write: errno=" << errno;
    }
  }

  void getReadBuffer(void** buf, size_t* len) noexcept override {
    *buf = buf_.data();
    *len = buf_.size();
  }

  void onDataAvailable(const folly::SocketAddress& client,
                       size_t len,
                       bool truncated,
                       OnDataAvailableParams) noexcept override {
    ++pongRecvd_;
    VLOG(4) << "Read " << len << " bytes (trun:" << truncated << ") from "
            << client.describe() << " - " << std::string(buf_.data(), len);
    auto datagramString = std::string(buf_.data(), len);
    auto datagramInt = folly::tryTo<uint16_t>(datagramString);
    if (!datagramInt.hasValue()) {
      VLOG(2) << "Received Datagram without Integer value. Stopping. len="
              << datagramString.length();
      return;
    }
    VLOG(2) << "Received Datagram with Integer value (" << (int)*datagramInt
            << ")";
    if (*datagramInt >= std::numeric_limits<uint16_t>::max()) {
      VLOG(2) << "Received Datagram with large Integer value. Stopping";
      return;
    }
    VLOG(2) << "Sending Datagram with Integer value ("
            << (int)(*datagramInt + 1) << ")";
    n_ = (int)(*datagramInt + 1);

    scheduleTimeout(1000);
  }

  void onReadError(const folly::AsyncSocketException& ex) noexcept override {
    LOG(ERROR) << ex.what();
  }

  void onReadClosed() noexcept override {
    shutdown();
  }

  void timeoutExpired() noexcept override {
    LOG(INFO) << "Timeout expired";
    if (!closing_) {
      sendPing();
    }
  }

 private:
  folly::EventBase* const evb_{nullptr};
  H3DatagramAsyncSocket& socket_;
  int pongRecvd_{0};
  int n_{0};
  std::array<char, kMaxReadBufferSize> buf_;
  bool closing_{false};
};
}; // namespace

int main(int argc, char* argv[]) {
#if FOLLY_HAVE_LIBGFLAGS
  // Enable glog logging to stderr by default.
  folly::gflags::SetCommandLineOptionWithMode(
      "logtostderr", "1", folly::gflags::SET_FLAGS_DEFAULT);
#endif
  const folly::Init init(&argc, &argv, false);

  EventBase evb;

  H3DatagramAsyncSocket::Options options;
  options.mode_ = H3DatagramAsyncSocket::Mode::CLIENT;
  options.txnTimeout_ = std::chrono::milliseconds(10000);
  options.connectTimeout_ = std::chrono::milliseconds(500);
  options.httpRequest_ = std::make_unique<HTTPMessage>();

  if (!FLAGS_masque_template.empty()) {
    // RFC 9298 Extended CONNECT mode
    auto target = proxygen::expandConnectUDPTemplate(
        FLAGS_masque_template, FLAGS_host, FLAGS_port);
    options.httpRequest_->setMethod(proxygen::HTTPMethod::CONNECT);
    options.httpRequest_->setUpgradeProtocol("connect-udp");
    options.httpRequest_->setSecure(true);
    options.httpRequest_->setURL(target.path);
    options.httpRequest_->getHeaders().set(proxygen::HTTP_HEADER_HOST,
                                           target.authority);
    options.httpRequest_->getHeaders().set("Capsule-Protocol", "?1");
    options.rfcMode_ = true;
  } else {
    // Legacy CONNECT-UDP mode
    options.httpRequest_->setMethod(proxygen::HTTPMethod::CONNECT_UDP);
    options.httpRequest_->setURL(fmt::format("{}:{}", FLAGS_host, FLAGS_port));
    options.httpRequest_->setMasque();
  }

  auto parsedHeaders = CurlService::CurlClient::parseHeaders(FLAGS_headers);
  parsedHeaders.forEach(
      [&options](const std::string& name, const std::string& value) {
        options.httpRequest_->getHeaders().add(name, value);
      });
  options.certAndKey_ = std::make_pair(FLAGS_cert, FLAGS_key);
  options.certVerifier_ = std::make_unique<
      proxygen::InsecureVerifierDangerousDoNotUseInProduction>();
  options.maxDatagramSize_ = kMaxReadBufferSize;

  H3DatagramAsyncSocket datagramSocket(&evb, options);
  DatagramClient client(&evb, datagramSocket);
  client.start();
  evb.loop();

  return EXIT_SUCCESS;
}
