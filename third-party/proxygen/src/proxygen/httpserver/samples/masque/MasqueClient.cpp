/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <array>

#include <folly/portability/Sockets.h>

#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/SocketAddress.h>
#include <folly/String.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <proxygen/httpserver/samples/hq/InsecureVerifierDangerousDoNotUseInProduction.h>
#include <proxygen/lib/transport/ConnectUDPUtils.h>
#include <proxygen/lib/transport/H3DatagramAsyncSocket.h>

using namespace folly;
using namespace proxygen;

DEFINE_string(proxy_host, "127.0.0.1", "MASQUE proxy hostname/IP");
DEFINE_int32(proxy_port, 4443, "MASQUE proxy port");
DEFINE_string(target_host, "127.0.0.1", "Target UDP server hostname/IP");
DEFINE_int32(target_port, 9999, "Target UDP server port");
DEFINE_string(
    masque_template,
    "https://{proxy_host}:{proxy_port}/masque?h={target_host}&p={target_port}",
    "URI template for CONNECT-UDP (RFC 9298)");
DEFINE_string(cert, "", "TLS certificate file path");
DEFINE_string(key, "", "TLS private key file path");
DEFINE_int32(payload_size, 16, "Size of test payload in bytes (max 1400)");
DEFINE_string(
    extra_headers,
    "",
    "Comma-separated key:value headers to add to the CONNECT-UDP request");
DEFINE_bool(ipv4_only, false, "Force IPv4 address resolution");

constexpr size_t kMaxReadBufferSize{1500};
constexpr size_t kMaxPayloadSize{1400};

namespace {

folly::SocketAddress resolveAddress(const std::string& host,
                                    uint16_t port,
                                    bool ipv4Only) {
  if (ipv4Only) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo* res = nullptr;
    int err = getaddrinfo(host.c_str(), nullptr, &hints, &res);
    if (err != 0 || res == nullptr) {
      throw std::runtime_error(
          folly::to<std::string>("Failed to resolve IPv4 address for '",
                                 host,
                                 "': ",
                                 gai_strerror(err)));
    }
    SCOPE_EXIT {
      freeaddrinfo(res);
    };
    folly::SocketAddress addr;
    addr.setFromSockaddr(res->ai_addr, res->ai_addrlen);
    addr.setPort(port);
    return addr;
  }
  return {host, port, /* allowNameLookup */ true};
}

class MasqueInteropClient
    : private folly::AsyncUDPSocket::ReadCallback
    , private folly::AsyncTimeout {
 public:
  using folly::AsyncUDPSocket::ReadCallback::OnDataAvailableParams;

  ~MasqueInteropClient() override = default;

  explicit MasqueInteropClient(folly::EventBase* evb,
                               H3DatagramAsyncSocket& socket,
                               size_t payloadSize)
      : folly::AsyncTimeout(evb),
        evb_(evb),
        socket_(socket),
        payloadSize_(payloadSize) {
  }

  void start() {
    CHECK(evb_->isInEventBaseThread());
    try {
      proxyAddress_ = resolveAddress(FLAGS_proxy_host,
                                     static_cast<uint16_t>(FLAGS_proxy_port),
                                     FLAGS_ipv4_only);
      socket_.connect(proxyAddress_);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Failed to connect: " << e.what();
      exitCode_ = 1;
      return;
    }
    socket_.resumeRead(this);

    // Generate random payload
    for (size_t i = 0; i < payloadSize_; ++i) {
      sentPayload_[i] = static_cast<char>(folly::Random::rand32() & 0xff);
    }

    // Write immediately — H3DatagramAsyncSocket buffers datagrams until the
    // transport is connected and the upstream HTTP 200 is received.
    sendPayload();
  }

  [[nodiscard]] int exitCode() const {
    return exitCode_;
  }

 private:
  void sendPayload() {
    auto buf = folly::IOBuf::copyBuffer(sentPayload_.data(), payloadSize_);
    auto hexStr = folly::hexlify(folly::ByteRange(
        reinterpret_cast<const uint8_t*>(sentPayload_.data()), payloadSize_));
    LOG(INFO) << "Sending " << payloadSize_ << " bytes: " << hexStr;
    auto res = socket_.write(proxyAddress_, buf);
    if (res < 0) {
      LOG(ERROR) << "Failed to write: errno=" << errno;
      exitCode_ = 1;
      evb_->terminateLoopSoon();
      return;
    }
    sent_ = true;
    // Set timeout for response
    scheduleTimeout(5000);
  }

  void timeoutExpired() noexcept override {
    LOG(ERROR) << "Timeout waiting for echo response";
    exitCode_ = 1;
    evb_->terminateLoopSoon();
  }

  void getReadBuffer(void** buf, size_t* len) noexcept override {
    *buf = readBuf_.data();
    *len = kMaxReadBufferSize;
  }

  void onDataAvailable(const folly::SocketAddress& /*client*/,
                       size_t len,
                       bool /*truncated*/,
                       OnDataAvailableParams) noexcept override {
    try {
      cancelTimeout();
      auto hexStr = folly::hexlify(folly::ByteRange(
          reinterpret_cast<const uint8_t*>(readBuf_.data()), len));
      LOG(INFO) << "Received " << len << " bytes: " << hexStr;

      if (len == payloadSize_ &&
          memcmp(readBuf_.data(), sentPayload_.data(), payloadSize_) == 0) {
        LOG(INFO) << "Echo payload verified - exact match";
        exitCode_ = 0;
        verified_ = true;
      } else {
        LOG(ERROR) << "Echo payload MISMATCH!";
        exitCode_ = 1;
      }
      evb_->terminateLoopSoon();
    } catch (const std::exception& e) {
      LOG(ERROR) << "Exception in onDataAvailable: " << e.what();
      exitCode_ = 1;
      evb_->terminateLoopSoon();
    }
  }

  void onReadError(const folly::AsyncSocketException& ex) noexcept override {
    cancelTimeout();
    LOG(ERROR) << "Read error: " << ex.what();
    if (!verified_) {
      exitCode_ = 1;
    }
    evb_->terminateLoopSoon();
  }

  void onReadClosed() noexcept override {
    cancelTimeout();
    LOG(INFO) << "Read closed";
    if (!sent_ && !verified_) {
      exitCode_ = 1;
    }
    evb_->terminateLoopSoon();
  }

  folly::EventBase* const evb_{nullptr};
  H3DatagramAsyncSocket& socket_;
  folly::SocketAddress proxyAddress_;
  size_t payloadSize_;
  std::array<char, kMaxPayloadSize> sentPayload_{};
  std::array<char, kMaxReadBufferSize> readBuf_{};
  bool sent_{false};
  bool verified_{false};
  int exitCode_{1};
};

} // namespace

int main(int argc, char* argv[]) {
#if FOLLY_HAVE_LIBGFLAGS
  folly::gflags::SetCommandLineOptionWithMode(
      "logtostderr", "1", folly::gflags::SET_FLAGS_DEFAULT);
#endif
  const folly::Init init(&argc, &argv, false);

  try {
    auto payloadSize = static_cast<size_t>(FLAGS_payload_size);
    CHECK_GT(payloadSize, 0u) << "payload_size must be > 0";
    CHECK_LE(payloadSize, kMaxPayloadSize)
        << "payload_size must be <= " << kMaxPayloadSize;

    // Expand the URI template with proxy address for the template itself,
    // then with target host/port for the actual request
    std::string expandedTemplate = FLAGS_masque_template;
    // Replace {proxy_host} and {proxy_port} in the template
    {
      auto pos = expandedTemplate.find("{proxy_host}");
      while (pos != std::string::npos) {
        expandedTemplate.replace(pos, 12, FLAGS_proxy_host);
        pos = expandedTemplate.find("{proxy_host}",
                                    pos + FLAGS_proxy_host.size());
      }
    }
    {
      auto portStr = folly::to<std::string>(FLAGS_proxy_port);
      auto pos = expandedTemplate.find("{proxy_port}");
      while (pos != std::string::npos) {
        expandedTemplate.replace(pos, 12, portStr);
        pos = expandedTemplate.find("{proxy_port}", pos + portStr.size());
      }
    }

    auto target =
        expandConnectUDPTemplate(expandedTemplate,
                                 FLAGS_target_host,
                                 static_cast<uint16_t>(FLAGS_target_port));

    LOG(INFO) << "Extended CONNECT: :protocol=connect-udp"
              << ", :scheme=" << target.scheme
              << ", :authority=" << target.authority
              << ", :path=" << target.path;

    EventBase evb;

    H3DatagramAsyncSocket::Options options;
    options.mode_ = H3DatagramAsyncSocket::Mode::CLIENT;
    options.txnTimeout_ = std::chrono::milliseconds(10000);
    options.connectTimeout_ = std::chrono::milliseconds(3000);
    options.rfcMode_ = true;
    options.maxDatagramSize_ = kMaxReadBufferSize;
    options.hostname_ = FLAGS_proxy_host;

    options.httpRequest_ = std::make_unique<HTTPMessage>();
    options.httpRequest_->setMethod(HTTPMethod::CONNECT);
    options.httpRequest_->setUpgradeProtocol("connect-udp");
    options.httpRequest_->setSecure(true);
    options.httpRequest_->setURL(target.path);
    options.httpRequest_->getHeaders().set(HTTP_HEADER_HOST, target.authority);
    options.httpRequest_->getHeaders().set("Capsule-Protocol", "?1");

    if (!FLAGS_cert.empty() && !FLAGS_key.empty()) {
      options.certAndKey_ = std::make_pair(FLAGS_cert, FLAGS_key);
    }
    options.certVerifier_ =
        std::make_unique<InsecureVerifierDangerousDoNotUseInProduction>();

    if (!FLAGS_extra_headers.empty()) {
      std::vector<folly::StringPiece> pairs;
      folly::split(',', FLAGS_extra_headers, pairs);
      for (auto& pair : pairs) {
        auto colon = pair.find(':');
        if (colon != folly::StringPiece::npos) {
          auto key = folly::trimWhitespace(pair.subpiece(0, colon));
          auto val = folly::trimWhitespace(pair.subpiece(colon + 1));
          options.httpRequest_->getHeaders().set(key.str(), val.str());
        }
      }
    }

    H3DatagramAsyncSocket datagramSocket(&evb, options);
    MasqueInteropClient client(&evb, datagramSocket, payloadSize);
    client.start();
    evb.loop();

    // Clean shutdown: close the socket and drain pending callbacks before
    // the EventBase destructor runs, to avoid use-after-free on session timers.
    datagramSocket.close();
    evb.loop();

    return client.exitCode();
  } catch (const std::exception& e) {
    LOG(ERROR) << "Fatal error: " << e.what();
    return 1;
  }
}
