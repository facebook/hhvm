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

#include <thrift/conformance/stresstest/server/StressTestServer.h>

#include <memory>

#include <glog/logging.h>
#include <folly/experimental/io/IoUringBackend.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>

#include <gflags/gflags.h>
#include <wangle/ssl/SSLContextConfig.h>

#include <thrift/conformance/stresstest/util/IoUringUtil.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include "common/services/cpp/TLSConfig.h"

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/thread_factory/InitThreadFactory.h>
#include <thrift/conformance/stresstest/server/StressTestServerModule.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/TokenBucketConcurrencyController.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

DEFINE_int32(port, 5000, "Server port");
DEFINE_int32(io_threads, 0, "Number of IO threads (0 == number of cores)");
DEFINE_int32(cpu_threads, 0, "Number of CPU threads (0 == number of cores)");
DEFINE_int32(
    max_requests,
    -1,
    "Configures max requests. Special value -1 will skip setting max request limit. Special value 0 will set max requests to maximum value.");
DEFINE_int32(
    concurrency_limit,
    -1,
    "Configures concurrency limit. Special value -1 will skip setting concurrency limit. Special value 0 will set concurrency limit to maximum value.");
DEFINE_bool(
    default_sync_max_requests_to_concurrency_limit,
    false,
    "Sets Thrift Server's default_sync_max_requests_to_concurrency_limit flag.");
DEFINE_int32(
    max_qps,
    -1,
    "Configures max qps. Special value -1 will skip setting max qps. Special value 0 will set max qps to maximum value.");
DEFINE_int32(
    execution_rate,
    -1,
    "Configures execution rate. Special value -1 will skip setting execution rate. Special value 0 will set execution rate to maximum value.");
DEFINE_bool(
    default_sync_max_qps_to_execution_rate,
    false,
    "Sets Thrift Server's default_sync_max_qps_to_execution_rate flag.");
DEFINE_bool(io_uring, false, "Enables io_uring if available when set to true");
DEFINE_bool(
    parallel_concurrency_controller,
    false,
    "Enable ParallelConcurrencyController.");
DEFINE_bool(
    se_parallel_concurrency_controller,
    false,
    "Enable SEParallelConcurrencyController.");
DEFINE_bool(
    token_bucket_concurrency_controller,
    false,
    "Enable TokenBucketConcurrencyController.");
DEFINE_string(
    certPath,
    "folly/io/async/test/certs/tests-cert.pem",
    "Path to client certificate file");
DEFINE_string(
    keyPath,
    "folly/io/async/test/certs/tests-key.pem",
    "Path to client key file");
DEFINE_string(
    caPath,
    "folly/io/async/test/certs/ca-cert.pem",
    "Path to client trusted CA file");
DEFINE_bool(enable_overload_checker, false, "Enable overload checker");
DEFINE_bool(enable_resource_pools, false, "Enable resource pools");
DEFINE_bool(stopTLSv1, false, "Enable stopTLS v1");
DEFINE_bool(stopTLSv2, false, "Enable stopTLS v2");
DEFINE_bool(
    disable_active_request_tracking, false, "Disabled Active Request Tracking");
DEFINE_bool(enable_checksum, false, "Enable Server Side Checksum support");
DEFINE_bool(aligned_parser, false, "Enable AlignedParser");
DEFINE_string(
    plaintext_parser,
    "",
    "Parser override to use for plaintext connections (strategy, allocating, aligned)");

#if FOLLY_HAVE_WEAK_SYMBOLS
FOLLY_ATTR_WEAK int callback_assign_func(
    folly::AsyncServerSocket*, folly::NetworkSocket);
#else
static int callback_assign_func(
    folly::AsyncServerSocket*, folly::NetworkSocket) {
  return -1;
}
#endif

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(
    std::string, getSocketParser, folly::AsyncTransport& socket) {
  if (socket.getUnderlyingTransport<fizz::AsyncFizzBase>() ||
      socket.getUnderlyingTransport<folly::AsyncSSLSocket>()) {
    return THRIFT_FLAG(rocket_frame_parser);
  }
  return FLAGS_plaintext_parser.empty() ? THRIFT_FLAG(rocket_frame_parser)
                                        : FLAGS_plaintext_parser;
}
} // namespace apache::thrift::detail

namespace apache::thrift::stress {

using namespace apache::thrift::rocket;

namespace {

uint32_t sanitizeNumThreads(int32_t n) {
  return n <= 0 ? std::thread::hardware_concurrency() : n;
}

std::shared_ptr<wangle::SSLContextConfig> getSSLConfig() {
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(FLAGS_certPath.c_str(), FLAGS_keyPath.c_str(), "");
  if (FLAGS_stopTLSv1) {
    sslConfig->clientVerification =
        folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
    sslConfig->setNextProtocols({"rs"});
  } else {
    sslConfig->clientCAFiles = std::vector<std::string>{FLAGS_caPath.c_str()};
    sslConfig->clientVerification =
        folly::SSLContext::VerifyClientCertificate::IF_PRESENTED;
    sslConfig->setNextProtocols(**ThriftServer::defaultNextProtocols());
    sslConfig->sslCiphers =
        folly::join(":", folly::ssl::SSLOptions2021::ciphers());
  }
  return sslConfig;
}
} // namespace

std::shared_ptr<StressTestHandler> createStressTestHandler() {
  return std::make_shared<StressTestHandler>();
}

std::unique_ptr<folly::EventBaseBackendBase> getEventBaseBackendFunc() {
#if FOLLY_HAS_LIBURING
  try {
    // TODO numa node affinitization
    // static int sqSharedCore = 0;
    // LOG(INFO) << "Sharing eb sq poll on core: " << sqSharedCore;
    // options.setSQGroupName("fast_eb").setSQCpu(sqSharedCore);
    return std::make_unique<folly::IoUringBackend>(getIoUringOptions());
  } catch (const std::exception& ex) {
    LOG(FATAL) << "Failed to create io_uring backend: "
               << folly::exceptionStr(ex);
  }
#else
  LOG(FATAL) << "io_uring not supported";
#endif
}

std::shared_ptr<folly::IOThreadPoolExecutor> getIOThreadPool(
    const std::string& name, size_t numThreads) {
  auto threadFactory = std::make_shared<folly::NamedThreadFactory>(name);
  if (FLAGS_io_uring) {
    auto initThreadFactory = std::make_shared<folly::InitThreadFactory>(
        std::move(threadFactory), [] {
          // Preinitialize EventBase with custom settings on startup.
          auto* eventBase = new folly::EventBase(
              folly::EventBase::Options().setBackendFactory(
                  getEventBaseBackendFunc));
          folly::EventBaseManager::get()->setEventBase(
              eventBase, true /* takeOwnership */);
        });
    return std::make_shared<folly::IOThreadPoolExecutor>(
        numThreads, std::move(initThreadFactory));
  } else {
    return std::make_shared<folly::IOThreadPoolExecutor>(
        numThreads, std::move(threadFactory));
  }
}

std::shared_ptr<ThriftServer> createStressTestServer(
    std::shared_ptr<apache::thrift::ServiceHandler<StressTest>> handler) {
  auto numCpuWorkerThreads = sanitizeNumThreads(FLAGS_cpu_threads);

  if (!handler) {
    handler = createStressTestHandler();
  }

  if (FLAGS_aligned_parser) {
    THRIFT_FLAG_SET_MOCK(rocket_frame_parser, "aligned");
  }

  auto server = std::make_shared<ThriftServer>();
  server->setInterface(std::move(handler));
  server->setPort(FLAGS_port);
  server->setPreferIoUring(FLAGS_io_uring);
  server->setIOThreadPool(
      getIOThreadPool("thrift_eventbase", FLAGS_io_threads));
  server->setNumCPUWorkerThreads(numCpuWorkerThreads);
  server->addModule(std::make_unique<StressTestServerModule>());
  facebook::services::TLSConfig tlsConfig;
  tlsConfig.applyToThriftServer(server);

  if (FLAGS_enable_checksum) {
    LOG(INFO) << "Checksum support enabled";
    PayloadSerializer::initialize(
        ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>(
            ChecksumPayloadSerializerStrategyOptions{
                .recordChecksumFailure =
                    [] { LOG(FATAL) << "Checksum failure detected"; },
                .recordChecksumSuccess =
                    [] {
                      LOG_EVERY_N(INFO, 1'000'000)
                          << "Checksum success detected";
                    },
                .recordChecksumCalculated =
                    [] {
                      LOG_EVERY_N(INFO, 1'000'000) << "Checksum calculated";
                    }}));
  }

  if (FLAGS_disable_active_request_tracking) {
    LOG(INFO) << "Active request tracking disabled";
    server->disableActiveRequestsTracking();
  }

  server->setSSLPolicy(apache::thrift::SSLPolicy::PERMITTED);
  if (!FLAGS_certPath.empty() && !FLAGS_keyPath.empty() &&
      !FLAGS_caPath.empty()) {
    LOG(INFO) << "SSL config enabled";
    server->setSSLConfig(getSSLConfig());
    if (FLAGS_stopTLSv1 || FLAGS_stopTLSv2) {
      ThriftTlsConfig thriftConfig;
      thriftConfig.enableThriftParamsNegotiation = true;
      if (FLAGS_stopTLSv1) {
        thriftConfig.enableStopTLS = true;
      } else {
        thriftConfig.enableStopTLSV2 = true;
      }
      server->setThriftConfig(thriftConfig);
    }
  }

  if (FLAGS_io_uring) {
    server->setCallbackAssignFunc(callback_assign_func);
  }

  std::shared_ptr<folly::Executor> executor;
  if (FLAGS_parallel_concurrency_controller ||
      FLAGS_se_parallel_concurrency_controller ||
      FLAGS_token_bucket_concurrency_controller) {
    LOG(INFO) << "CPUThreadPoolExecutor enabled";
    executor = std::make_shared<folly::CPUThreadPoolExecutor>(
        numCpuWorkerThreads,
        folly::CPUThreadPoolExecutor::makeThrottledLifoSemQueue());
  }

  std::unique_ptr<RequestPileInterface> requestPile;
  if (FLAGS_parallel_concurrency_controller ||
      FLAGS_se_parallel_concurrency_controller ||
      FLAGS_token_bucket_concurrency_controller) {
    LOG(INFO) << "RoundRobinRequestPile enabled";
    requestPile = std::make_unique<RoundRobinRequestPile>(
        RoundRobinRequestPile::Options{});
  }

  std::unique_ptr<ConcurrencyControllerInterface> concurrencyController;
  if (FLAGS_parallel_concurrency_controller) {
    LOG(INFO) << "ParallelConcurrencyController enabled";
    concurrencyController = std::make_unique<ParallelConcurrencyController>(
        *requestPile.get(), *executor.get());
  } else if (FLAGS_se_parallel_concurrency_controller) {
    LOG(INFO) << "ParallelConcurrencyController with SerialExecutor enabled";
    concurrencyController = std::make_unique<ParallelConcurrencyController>(
        *requestPile.get(),
        *executor.get(),
        ParallelConcurrencyController::RequestExecutionMode::Serial);
  } else if (FLAGS_token_bucket_concurrency_controller) {
    LOG(INFO) << "TokenBucketConcurrencyController enabled";
    concurrencyController = std::make_unique<TokenBucketConcurrencyController>(
        *requestPile.get(), *executor.get());
  }

  if (executor != nullptr) {
    LOG(INFO) << "Resource pools enabled";
    server->resourcePoolSet().setResourcePool(
        ResourcePoolHandle::defaultAsync(),
        std::move(requestPile),
        executor,
        std::move(concurrencyController));
    server->ensureResourcePools();
    server->requireResourcePools();
  }

  if (FLAGS_max_requests > -1) {
    LOG(INFO) << "Setting maxRequests: " << FLAGS_max_requests;
    server->setMaxRequests(FLAGS_max_requests);
  }
  if (FLAGS_max_qps > -1) {
    LOG(INFO) << "Setting maxQps: " << FLAGS_max_qps;
    server->setMaxQps(FLAGS_max_qps);
  }
  if (FLAGS_concurrency_limit > -1) {
    LOG(INFO) << "Setting concurrencyLimit: " << FLAGS_concurrency_limit;
    server->setConcurrencyLimit(FLAGS_concurrency_limit);
  }
  if (FLAGS_execution_rate > -1) {
    LOG(INFO) << "Setting executionRate: " << FLAGS_execution_rate;
    server->setExecutionRate(FLAGS_execution_rate);
  }
  if (FLAGS_default_sync_max_requests_to_concurrency_limit == true) {
    LOG(INFO)
        << "Setting THRIFT_FLAG default_sync_max_requests_to_concurrency_limit: true";
    THRIFT_FLAG_SET_MOCK(default_sync_max_requests_to_concurrency_limit, true);
  }
  if (FLAGS_default_sync_max_qps_to_execution_rate == true) {
    LOG(INFO) << "Setting defaultSyncMaxQpsToExecutionRate: true";
    THRIFT_FLAG_SET_MOCK(default_sync_max_qps_to_execution_rate, true);
  }

  return server;
}

} // namespace apache::thrift::stress
