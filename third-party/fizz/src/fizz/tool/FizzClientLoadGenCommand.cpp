/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/PskSerializationUtils.h>
#include <fizz/experimental/protocol/BatchSignatureFactory.h>
#include <fizz/tool/FizzCommandCommon.h>
#include <fizz/util/Parse.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/io/async/SSLContext.h>
#include <folly/json/json.h>
#include <folly/stats/Histogram.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace fizz::client;
using namespace folly;

namespace fizz {
namespace tool {
namespace {

/**
 * Client Side LoadGen Tool:
 * We simpilfy the settings to focus on the performance evaluation.
 *
 *  - Enforce verification of server certificate
 *  - Enforce TLS_AES_128_GCM_SHA256 for cipher
 *  - No early data in ClientHello
 *  - No Application Layer Protocol Negotiation (ALPN)
 *  - No certificate compression
 *  - No client side certificate verification
 */

void printUsage() {
  // clang-format off
  std::cerr
    << "Usage: client_loadgen args\n"
    << "\n"
    << "Supported arguments:\n"
    << " -connect host:port       (set the address to connect to. Default: localhost:8433)\n"
    << " -threads num             (set the # of threads for clients; a client will be created for each thread; Default 1)\n"
    << " -tasks num               (set the total # of TLS handshake tasks to perform per second; Default to be same as # of threads)\n"
    << " -time seconds            (set the total time in seconds of load generation; Default 10)\n"
    << " -cafile file             (path to bundle of CA certs used for verification)\n"
    << " -0rtt file               (given file that contains a serialized psk, deserialize psk and open a connection with it)\n"
    << " -json                    (print the benchmark statistics to the standard output)\n"
    << " -p num                   (print an additional num% percentile of TLS handshake latency when -json is used.\n"
    << "                           0 < num < 100. Default percentiles: 25%, 50%, 75%, 90%)\n"
    << " -min num                 (the minimum time elapse (in microsecond) allowed for statistics when -json is used. Default: 1000\n"
    << " -max num                 (the maximum time elapse (in microsecond) allowed for statistics when -json is used. Default: 1000000)\n"
    << " -batch                   (use the batch signature scheme ecdsa_secp256r1_sha256_batch)\n";
  // clang-format on
}

class ClientTask : public AsyncSocket::ConnectCallback,
                   public AsyncFizzClient::HandshakeCallback {
 public:
  ClientTask(
      EventBase* evb,
      std::shared_ptr<FizzClientContext> clientContext,
      std::shared_ptr<const CertificateVerifier> verifier,
      std::chrono::microseconds* dataPoint)
      : evb_(evb),
        clientContext_(clientContext),
        verifier_(verifier),
        dataPoint_(dataPoint) {}

  void start(const SocketAddress& addr) {
    sock_ = AsyncSocket::UniquePtr(new AsyncSocket(evb_));
    start_ = std::chrono::high_resolution_clock::now();
    sock_->connect(this, addr);
  }

  void endTask() {
    delete this;
  }

  void connectSuccess() noexcept override {
    fizzClient_ = AsyncFizzClient::UniquePtr(
        new AsyncFizzClient(std::move(sock_), clientContext_));
    fizzClient_->connect(
        this,
        verifier_,
        none,
        none,
        folly::Optional<std::vector<ech::ECHConfig>>(folly::none));
  }

  void connectErr(const AsyncSocketException& ex) noexcept override {
    VLOG(1) << "Error: " << ex.what();
    endTask();
  }

  void fizzHandshakeSuccess(AsyncFizzClient*) noexcept override {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - start_;
    *dataPoint_ =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    endTask();
  }

  void fizzHandshakeError(AsyncFizzClient*, exception_wrapper ex) noexcept
      override {
    VLOG(1) << "Error: " << ex.what();
    endTask();
  }

 private:
  EventBase* evb_;
  std::shared_ptr<FizzClientContext> clientContext_;
  std::shared_ptr<const CertificateVerifier> verifier_;
  std::chrono::microseconds* dataPoint_;
  AsyncSocket::UniquePtr sock_;
  AsyncFizzClient::UniquePtr fizzClient_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

struct ClientLoadgenConfig {
  std::string host = "localhost";
  uint16_t port = 8443;
  int threadNum = 1;
  int numTaskPerSecond = 0;
  int totalTime = 10;
  std::string caFile;
  std::string pskLoadFile;
  bool jsonOutput = false;
  int minLatency = 1000; // 1ms as the default minimum latency
  int maxLatency = 1000000; // 1s as the default maximum latency measured
  std::vector<float> percentiles = {0.25, 0.5, 0.75, 0.9};
};

std::string getJsonStr(
    uint32_t totalLatency,
    const Histogram<uint32_t>& histo,
    size_t numSuccess,
    const ClientLoadgenConfig& config) {
  dynamic percentile = dynamic::object;
  for (const auto& percent : config.percentiles) {
    percentile[folly::sformat("{0:3.1f}%", percent * 100)] =
        histo.getPercentileEstimate(percent);
  }
  size_t avg_latency = 0;
  if (numSuccess > 0) {
    avg_latency = totalLatency / numSuccess;
  }
  // clang-format off
  dynamic forJson = dynamic::object
    ("total_tasks", config.numTaskPerSecond * config.totalTime)
    ("tasks_per_second", config.numTaskPerSecond)
    ("threads", config.threadNum)
    ("success_tasks", numSuccess)
    ("unit", "microseconds")
    ("average_latency", avg_latency)
    ("precentile", percentile);
  // clang-format on
  return toPrettyJson(forJson);
}

} // namespace

int fizzClientLoadGenCommand(const std::vector<std::string>& args) {
  // configurable parameters
  struct ClientLoadgenConfig config;
  bool enableBatch = false;

  // Argument Handler Map
  // clang-format off
  FizzArgHandlerMap handlers = {
    {"-connect", {true, [&config](const std::string& arg) {
        std::tie(config.host, config.port) = hostPortFromString(arg);
     }}},
    {"-threads", {true, [&config](const std::string& arg) {
      config.threadNum = std::stoi(arg);
    }}},
    {"-tasks", {true, [&config](const std::string& arg) {
      config.numTaskPerSecond = std::stoi(arg);
    }}},
    {"-time", {true, [&config](const std::string& arg) {
      config.totalTime = std::stoi(arg);
    }}},
    {"-cafile", {true, [&config](const std::string& arg) {
      config.caFile = arg;
    }}},
    {"-0rtt", {true, [&config](const std::string& arg) {
      config.pskLoadFile = arg;
    }}},
    {"-json", {false, [&config](const std::string&) {
      config.jsonOutput = true;
    }}},
    {"-p", {true, [&config](const std::string& arg) {
      config.percentiles.emplace_back((float)std::stoi(arg) / 100);
    }}},
    {"-min", {true, [&config](const std::string& arg) {
      config.minLatency = std::stoi(arg);
    }}},
    {"-max", {true, [&config](const std::string& arg) {
      config.maxLatency = std::stoi(arg);
    }}},
    {"-batch", {false, [&enableBatch](const std::string&) {
      enableBatch = true;
    }}}
  };
  // clang-format on

  // parse arguments
  try {
    if (parseArguments(args, handlers, printUsage)) {
      return 1;
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
    return 1;
  }
  if (config.numTaskPerSecond <= 0) {
    config.numTaskPerSecond = config.threadNum;
  }

  // set up the IO Thread Pool and get the EventBase
  auto threadExe = std::make_shared<IOThreadPoolExecutor>(
      config.threadNum,
      std::make_shared<NamedThreadFactory>("LoadGenClientPool"),
      folly::EventBaseManager::get(),
      IOThreadPoolExecutor::Options().setWaitForAll(true));

  // Prepare FizzClientContext
  auto clientContext = std::make_shared<FizzClientContext>();
  clientContext->setSupportedCiphers({CipherSuite::TLS_AES_128_GCM_SHA256});
  clientContext->setSupportedVersions(
      {ProtocolVersion::tls_1_3, ProtocolVersion::tls_1_3_28});
  clientContext->setSupportedSigSchemes(
      {SignatureScheme::rsa_pss_sha256,
       SignatureScheme::ecdsa_secp256r1_sha256,
       SignatureScheme::ecdsa_secp384r1_sha384});
  if (enableBatch) {
    clientContext->setFactory(BatchSignatureFactory::makeBatchSignatureFactory(
        clientContext->getFactoryPtr()));
    clientContext->setSupportedSigSchemes(
        {SignatureScheme::rsa_pss_sha256_batch,
         SignatureScheme::ecdsa_secp256r1_sha256_batch});
  }

  std::shared_ptr<const CertificateVerifier> verifier;

  if (!config.caFile.empty()) {
    // Initialize CA store and the verifier for server certificate verification
    folly::ssl::X509StoreUniquePtr storePtr;
    storePtr.reset(X509_STORE_new());
    if (X509_STORE_load_locations(
            storePtr.get(), config.caFile.c_str(), nullptr) == 0) {
      VLOG(1) << "Failed to load CA certificates";
      return 1;
    }
    verifier = std::make_shared<const DefaultCertificateVerifier>(
        VerificationContext::Client, std::move(storePtr));
  }

  // Start creating clients and connecting
  std::vector<std::chrono::microseconds> stats;
  stats.resize(
      config.numTaskPerSecond * config.totalTime,
      std::chrono::microseconds::zero());
  auto it = stats.begin();
  auto periodMS = 1000 / config.numTaskPerSecond;
  SocketAddress addr(config.host, config.port, true);
  for (size_t i = 0; i < stats.size(); i++) {
    via(threadExe->weakRef()).thenValue([=](auto&&) {
      auto evb = folly::EventBaseManager::get()->getEventBase();
      auto task = new ClientTask(evb, clientContext, verifier, &(*it));
      task->start(addr);
    });
    it++;
    std::this_thread::sleep_for(std::chrono::milliseconds(periodMS));
  }

  // Wait for assignments finished
  threadExe->join();

  // generate statistics
  if (config.jsonOutput) {
    const int buckWidth = 1000; // 1ms as the bucket width
    Histogram<uint32_t> histo(buckWidth, config.minLatency, config.maxLatency);
    uint32_t totalLatency = 0;
    size_t numSuccess = 0;
    for (const auto& val : stats) {
      if (val != std::chrono::microseconds::zero()) {
        numSuccess++;
        histo.addValue(val.count());
        totalLatency += val.count();
      }
    }
    std::cout << getJsonStr(totalLatency, histo, numSuccess, config)
              << std::endl;
  }
  return 0;
}

} // namespace tool
} // namespace fizz
