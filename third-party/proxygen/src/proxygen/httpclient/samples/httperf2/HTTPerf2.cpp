/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * httperf2: replacement for httperf
 *
 * httperf is buggy, unpredictable and has more sophistication than we need
 * to benchmark proxygen.  httperf2 is meant to be a replacement.
 *
 * That httperf2 depends on proxygen/lib/ makes it a bit awkward to
 * benchmark a change, since any improvements could also benefit httperf2.
 * Always use the same version of httperf2 to benchmark proxygen A vs.
 * proxygen B.
 */
#include <fizz/client/PskCache.h>
#include <folly/FileUtil.h>
#include <folly/Random.h>
#include <folly/String.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <folly/ssl/Init.h>
#include <folly/ssl/SSLSession.h>
#include <proxygen/httpclient/samples/httperf2/Client.h>
#include <proxygen/httpclient/samples/httperf2/HTTPerf2.h>
#include <proxygen/httpclient/samples/httperf2/HTTPerfStats.h>
#include <proxygen/lib/http/HTTPMessage.h>

#include <csignal>
#include <iostream>
#include <openssl/engine.h>
#include <random>

using folly::SocketAddress;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using namespace proxygen;

// Duration parameters
DEFINE_int32(clients, 1, "Number of simulated clients across all threads");
DEFINE_int32(clients_at_once, 1, "Max concurrent clients across all threads");
DEFINE_int32(duration, 0, "Run for N seconds, overrides -clients");
DEFINE_int32(threads, 1, "Number of threads to spread clients across");
DEFINE_int32(delaystart, 0, "Milliseconds to sleep between starting threads");
DEFINE_string(
    wait_for_file,
    "",
    "If specified, the test will wait for the file to exist before starting");

// Target Params
DEFINE_string(server, "localhost", "Server name");
DEFINE_int32(port, 8080, "Server port");
DEFINE_string(ip, "", "IP of server to bypass DNS for server");
DEFINE_bool(ssl, false, "Use SSL");
DEFINE_bool(http2, false, "Use HTTP/2 <N>");
DEFINE_string(
    server_name,
    "",
    "Overrides the default server name to advertise with the secure protocol");
DEFINE_bool(fizz, false, "Use fizz (TLS 1.3)");
DEFINE_bool(fizz_psk_ke, false, "Use fizz psk_ke mode on resumptions");
DEFINE_bool(quic, false, "Use quic");
DEFINE_string(cert, "", "Certificate file path");
DEFINE_string(key, "", "Private key file path");

// Tunables
DEFINE_int32(resume_pct, 100, "SSL resume percentage");
DEFINE_int32(ticket_pct, 100, "TLS ticket percentage");
DEFINE_int32(requests, 1, "Number of requests (ie transactions) per client");
DEFINE_double(request_avg,
              0,
              "Average number of requests per client, overrides -requests");
DEFINE_string(ciphers, "HIGH", "OpenSSL cipher preferences");
DEFINE_int32(client_quic_transport_timer_resolution_ms,
             1,
             "Quic transport timer resolution in ms");

// HTTP options
DEFINE_bool(http10, false, "Force HTTP/1.0");
DEFINE_string(url, "/", "Request URL");
DEFINE_bool(nohost, false, "Don't add Host: <servername> to HTTP/1.1 requests");
DEFINE_string(headers, "", ":: separated list of HTTP request headers");
DEFINE_bool(realheaders, false, "Add default headers from Firefox circa 2012");
DEFINE_string(data, "", "File to transfer via POST");

DEFINE_int32(request_timeout_ms, 1000, "request timeout");

// Quic options
DEFINE_string(quic_qlogger_path,
              "",
              "Path to the directory where quic qlog files will be saved to.");

// Output options
DEFINE_string(testname, "", "Test name (prefixed to all the JSON keys");
DEFINE_bool(json, false, "Output as JSON");

namespace {

static std::list<std::pair<char const*, char const*>> f_real_headers = {
    {"User-Agent",
     "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.7; rv:10.0.2) "
     "Gecko/20100101 Firefox/10.0.2"},
    {"Accept",
     "text/html,application/xhtml+xml,application/xml;"
     "q=0.9,*/*;q=0.8"},
    {"Accept-Language", "en-us,en;q=0.5"},
    {"Accept-Encoding", "gzip, deflate"},
    {"Connection", "keep-alive"}};

std::string gRequestData;

/**
 * This class drives the work for a single thread.
 */
class ClientRunner
    : public folly::AsyncTimeout
    , public Client::FinishedCallback {
 public:
  ClientRunner(HTTPerfStats& parentStats,
               folly::Optional<SocketAddress> bindAddr,
               SocketAddress address,
               size_t numClients,
               size_t numRequests,
               size_t clientsAtOnce,
               int32_t quicTransportTimerResolutionMs);

  void run();

  void startClient();

  void clientFinished(Client* client) override;

  void timeoutExpired() noexcept override;

 private:
  using SSLParams =
      std::pair<folly::SSLContextPtr, std::shared_ptr<folly::ssl::SSLSession>>;

  HTTPerfStats& parentStats_;
  HTTPerfStats stats_;
  size_t remainingClients_;
  size_t numRequests_;
  size_t clientsAtOnce_;
  HTTPMessage request_;
  std::string requestData_;
  std::string plaintextProto_;
  folly::EventBase eventBase_;
  WheelTimerInstance transactionTimeouts_;
  folly::Optional<SocketAddress> bindAddr_;
  SocketAddress address_;
  SSLParams ticketSSLParams_;
  SSLParams sessionSSLParams_;
  std::normal_distribution<double> requestDistribution_;
  std::uniform_int_distribution<uint32_t> ticketDistribution_;
  std::uniform_int_distribution<uint32_t> resumeDistribution_;
  uint32_t clientsOutstanding_;
  std::mt19937 rng_;
  std::shared_ptr<fizz::client::BasicPskCache> pskCache_;
  std::shared_ptr<quic::BasicQuicPskCache> quicPskCache_;

  uint32_t getClientRequests();

  const SSLParams& getSSLParams();
  std::string serverName_;
};

} // namespace

namespace proxygen {
int httperf2(folly::Optional<folly::SocketAddress> bindAddress) {
  if (FLAGS_threads <= 0 || FLAGS_clients <= 0 || FLAGS_clients_at_once <= 0 ||
      (FLAGS_request_avg <= 0 && FLAGS_requests <= 0) || FLAGS_ticket_pct < 0 ||
      FLAGS_ticket_pct > 100 || FLAGS_resume_pct < 0 ||
      FLAGS_resume_pct > 100) {
    std::cerr << "Invalid arguments" << std::endl;
    return 1;
  }
  auto numRequests = FLAGS_requests;
  if (FLAGS_request_avg > 0) {
    std::cerr << "Using request_avg" << std::endl;
    numRequests = 0;
  }

  size_t numClients;
  if (FLAGS_duration > 0) {
    std::cerr << "Using duration " << FLAGS_duration << std::endl;
    numClients = 0;
  } else {
    numClients = std::max(1, FLAGS_clients / FLAGS_threads);
  }

  auto clientsAtOnce = std::max(1, FLAGS_clients_at_once / FLAGS_threads);

  if (!FLAGS_wait_for_file.empty()) {
    std::cerr << "Waiting for file " << FLAGS_wait_for_file << std::endl;
    int fd = -1;
    while (fd == -1) {
      fd = folly::openNoInt(FLAGS_wait_for_file.c_str(), O_RDONLY);
    }
    if (fd != -1) {
      folly::closeNoInt(fd);
    }
  }

  SocketAddress address;
  if (!FLAGS_ip.empty()) {
    address.setFromIpPort(FLAGS_ip, FLAGS_port);
  } else {
    address.setFromHostPort(FLAGS_server, FLAGS_port);
  }

  if (!FLAGS_data.empty()) {
    if (!folly::readFile(FLAGS_data.c_str(), gRequestData)) {
      LOG(FATAL) << "Failed to read file";
    }
  }
  std::cerr << "Running test against " << FLAGS_server << ":" << FLAGS_port
            << std::endl;

  system_clock::time_point start = system_clock::now();
  HTTPerfStats stats;
  if (FLAGS_threads == 1) {
    ClientRunner r(stats,
                   std::move(bindAddress),
                   std::move(address),
                   numClients,
                   numRequests,
                   clientsAtOnce,
                   FLAGS_client_quic_transport_timer_resolution_ms);
    r.run();
  } else {
    std::list<std::thread> threads;
    for (int i = 0; i < FLAGS_threads; i++) {
      auto r = std::make_shared<ClientRunner>(
          stats,
          bindAddress,
          address,
          numClients,
          numRequests,
          clientsAtOnce,
          FLAGS_client_quic_transport_timer_resolution_ms);
      threads.emplace_back([r]() { r->run(); });
      if (FLAGS_delaystart > 0 && i + 1 < FLAGS_threads) {
        // @lint-ignore CLANGTIDY
        usleep(FLAGS_delaystart * 1000);
      }
    }
    for (auto& t : threads) {
      if (t.joinable()) {
        t.join();
      }
    }
  }
  milliseconds durationMs =
      std::chrono::duration_cast<milliseconds>(system_clock::now() - start);

  if (FLAGS_json) {
    stats.printStatsInJson(FLAGS_testname, durationMs);
  } else {
    stats.printStats(durationMs);
  }

  return 0;
}
} // namespace proxygen

namespace {
ClientRunner::ClientRunner(HTTPerfStats& parentStats,
                           folly::Optional<SocketAddress> bindAddr,
                           SocketAddress address,
                           size_t numClients,
                           size_t numRequests,
                           size_t clientsAtOnce,
                           int32_t quicTransportTimerResolutionMs)
    : parentStats_(parentStats),
      remainingClients_(numClients),
      numRequests_(numRequests),
      clientsAtOnce_(clientsAtOnce),
      eventBase_(std::chrono::milliseconds(
          FLAGS_quic ? quicTransportTimerResolutionMs
                     : folly::HHWheelTimer::DEFAULT_TICK_INTERVAL)),
      transactionTimeouts_(std::chrono::milliseconds(FLAGS_request_timeout_ms),
                           &eventBase_),
      bindAddr_(std::move(bindAddr)),
      address_(std::move(address)),
      requestDistribution_(FLAGS_request_avg, 1),
      ticketDistribution_(0, 100),
      resumeDistribution_(0, 100),
      clientsOutstanding_(0),
      pskCache_(std::make_shared<fizz::client::BasicPskCache>()),
      quicPskCache_(std::make_shared<quic::BasicQuicPskCache>()) {
  attachEventBase(&eventBase_);

  if (remainingClients_ == 0) {
    remainingClients_ = 0x7fffffff;
  }

  ticketSSLParams_.first.reset(new folly::SSLContext());
  ticketSSLParams_.first->setOptions(SSL_OP_NO_COMPRESSION);
  if (FLAGS_ciphers.length() > 0) {
    ticketSSLParams_.first->ciphers(FLAGS_ciphers);
  }
  ticketSSLParams_.second = nullptr;

  sessionSSLParams_.first.reset(new folly::SSLContext());
  sessionSSLParams_.first->setOptions(SSL_OP_NO_COMPRESSION | SSL_OP_NO_TICKET);
  if (FLAGS_ciphers.length() > 0) {
    sessionSSLParams_.first->ciphers(FLAGS_ciphers);
  }
  sessionSSLParams_.second = nullptr;
  rng_.seed(folly::randomNumberSeed());

  if (FLAGS_http2) {
    std::list<std::string> protos;
    plaintextProto_ = "h2";
    ticketSSLParams_.first->setAdvertisedNextProtocols({plaintextProto_});
    sessionSSLParams_.first->setAdvertisedNextProtocols({plaintextProto_});
  }

  if (FLAGS_quic) {
    plaintextProto_ = kH3FBCurrentDraft;
  }

  // Set up the HTTP request
  if (!FLAGS_data.empty()) {
    request_.setMethod(HTTPMethod::POST);
    requestData_ = gRequestData;
  } else {
    request_.setMethod(HTTPMethod::GET);
  }

  serverName_ = FLAGS_server;
  if (!FLAGS_server_name.empty()) {
    serverName_ = FLAGS_server_name;
  }

  if (FLAGS_http10) {
    request_.setHTTPVersion(1, 0);
  } else {
    request_.setHTTPVersion(1, 1);
    if (!FLAGS_nohost) {
      request_.getHeaders().add(HTTP_HEADER_HOST, serverName_);
    }
  }

  request_.setURL(FLAGS_url);

  if (FLAGS_realheaders) {
    for (const auto& p : f_real_headers) {
      request_.getHeaders().add(p.first, p.second);
    }
  }

  std::vector<std::string> headers;
  folly::split(std::string_view("::"), FLAGS_headers, headers);
  for (const auto& header : headers) {
    if (header.length() == 0) {
      continue;
    }
    auto pos = header.find(':');
    if (pos == std::string::npos) {
      request_.getHeaders().add(header, empty_string);
    } else {
      std::string headerName = header.substr(0, pos);
      do {
        pos++;
      } while (pos < header.length() && isspace(header.at(pos)));
      request_.getHeaders().add(headerName, header.substr(pos));
    }
  }
}

void ClientRunner::run() {
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss, SIGPIPE);
  PCHECK(pthread_sigmask(SIG_BLOCK, &ss, nullptr) == 0);

  auto clients = std::min(remainingClients_, clientsAtOnce_);
  for (size_t i = 0; i < clients; i++) {
    startClient();
  }

  if (FLAGS_duration > 0) {
    scheduleTimeout(FLAGS_duration * 1000);
  }
  eventBase_.loop();

  CHECK_EQ(clientsOutstanding_, 0);
  parentStats_.merge(stats_);
}

void ClientRunner::timeoutExpired() noexcept {
  VLOG(3) << "Duration timeout expired";
  Client::exitAllSoon();
  remainingClients_ = 0;
}

void ClientRunner::startClient() {
  CHECK_GT(remainingClients_, 0);
  Client* client = nullptr;
  client = new Client(&eventBase_,
                      transactionTimeouts_,
                      stats_,
                      bindAddr_,
                      address_,
                      request_,
                      requestData_,
                      getClientRequests(),
                      this,
                      plaintextProto_,
                      serverName_);
  auto resumePct = uint32_t(FLAGS_resume_pct);
  if (FLAGS_ssl) {
    const SSLParams& sslParams = getSSLParams();
    auto session = sslParams.second;
    if (session != nullptr && resumeDistribution_(rng_) > resumePct) {
      session = nullptr;
    }
    client->setSSLParameters(sslParams.first, session);
  } else if (FLAGS_fizz) {
    client->setupFizzContext(resumeDistribution_(rng_) > resumePct ? nullptr
                                                                   : pskCache_,
                             FLAGS_fizz_psk_ke,
                             FLAGS_cert,
                             FLAGS_key);
  } else if (FLAGS_quic) {
    client->setUseQuic(true);
    client->setupFizzContext(nullptr, FLAGS_fizz_psk_ke, FLAGS_cert, FLAGS_key);
    client->setQuicPskCache(
        resumeDistribution_(rng_) > resumePct ? nullptr : quicPskCache_);
    if (!FLAGS_quic_qlogger_path.empty()) {
      client->setQLoggerPath(FLAGS_quic_qlogger_path);
    }
  }
  remainingClients_--;
  clientsOutstanding_++;
  client->start();
}

void ClientRunner::clientFinished(Client* client) {
  if (FLAGS_ssl) {
    // Assume that the session is valid for the length of the test
    auto session = client->extractSSLSession();
    if (client->supportsTickets()) {
      if (ticketSSLParams_.second == nullptr) {
        ticketSSLParams_.second = session;
      }
    } else if (sessionSSLParams_.second == nullptr) {
      // TODO: keep multiple sessions to stress stateful cache lookups
      sessionSSLParams_.second = session;
    }
  }
  clientsOutstanding_--;
  delete client;
  VLOG(3) << __func__ << " clientsOutstanding=" << clientsOutstanding_
          << " remainingClients=" << remainingClients_;
  if (remainingClients_ > 0 && clientsOutstanding_ < clientsAtOnce_) {
    startClient();
  }
}

uint32_t ClientRunner::getClientRequests() {
  if (numRequests_ > 0) {
    return numRequests_;
  }
  return static_cast<uint32_t>(
      std::max(floor(requestDistribution_(rng_) + .5), 1.0));
}

const ClientRunner::SSLParams& ClientRunner::getSSLParams() {
  if (ticketDistribution_(rng_) < uint32_t(FLAGS_ticket_pct)) {
    return ticketSSLParams_;
  } else {
    return sessionSSLParams_;
  }
}
} // namespace
