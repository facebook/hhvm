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

#include <folly/experimental/io/IoUringBackend.h>
#include <thrift/conformance/stresstest/server/StressTestServer.h>

#include <folly/portability/GFlags.h>
#include <wangle/ssl/SSLContextConfig.h>

#include <thrift/conformance/stresstest/util/IoUringUtil.h>

#include <scripts/rroeser/src/executor/WorkStealingExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

DEFINE_int32(port, 5000, "Server port");
DEFINE_int32(io_threads, 0, "Number of IO threads (0 == number of cores)");
DEFINE_int32(cpu_threads, 0, "Number of CPU threads (0 == number of cores)");
DEFINE_int32(
    max_requests,
    -1,
    "Configures max requests, 0 will disable max request limit");
DEFINE_bool(io_uring, false, "Enables io_uring if available when set to true");
DEFINE_bool(
    work_stealing,
    false,
    "Enable work stealing test. Implies --enable_resource_pools");
DEFINE_bool(
    work_stealing_executor_only,
    false,
    "Enable work stealing test without request pile or concurrency controller."
    " Implies --work_stealing and --enable_resource_pools");
DEFINE_bool(
    se_parallel_concurrency_controller,
    false,
    "Enable SEParallelConcurrencyController. Implies --enable_resource_pools");
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
DEFINE_bool(
    disable_active_request_tracking, false, "Disabled Active Request Tracking");
DEFINE_bool(enable_checksum, false, "Enable Server Side Checksum support");
DEFINE_bool(aligned_parser, false, "Enable AlignedParser");

namespace apache {
namespace thrift {
namespace stress {

using namespace apache::thrift::rocket;

namespace {

bool isResourcePoolsEnabled() {
  // These flags explicitly or implicitly enable resource pools.
  return FLAGS_enable_resource_pools || FLAGS_work_stealing ||
      FLAGS_work_stealing_executor_only ||
      FLAGS_se_parallel_concurrency_controller;
}

bool isWorkStealingEnabled() {
  // These flags explicitly or implicitly enable work stealing executor.
  return FLAGS_work_stealing || FLAGS_work_stealing_executor_only;
}

bool isRoundRobinRequestPileEnabled() {
  return isResourcePoolsEnabled() && !FLAGS_work_stealing_executor_only;
}

bool isParallelConcurrencyControllerEnabled() {
  return isResourcePoolsEnabled() && !FLAGS_work_stealing_executor_only &&
      !FLAGS_se_parallel_concurrency_controller;
}

bool isSEParallelConcurrencyControllerEnabled() {
  return isResourcePoolsEnabled() && !FLAGS_work_stealing_executor_only &&
      FLAGS_se_parallel_concurrency_controller;
}

uint32_t sanitizeNumThreads(int32_t n) {
  return n <= 0 ? std::thread::hardware_concurrency() : n;
}

std::shared_ptr<wangle::SSLContextConfig> getSSLConfig() {
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(FLAGS_certPath.c_str(), FLAGS_keyPath.c_str(), "");
  sslConfig->clientCAFiles = std::vector<std::string>{FLAGS_caPath.c_str()};
  sslConfig->clientVerification =
      folly::SSLContext::VerifyClientCertificate::IF_PRESENTED;
  sslConfig->setNextProtocols(**ThriftServer::defaultNextProtocols());
  sslConfig->sslCiphers =
      folly::join(":", folly::ssl::SSLOptions2021::ciphers());
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
    LOG(INFO) << "using io_uring EventBase backend";
    folly::EventBaseBackendBase::FactoryFunc func(getEventBaseBackendFunc);
    static folly::EventBaseManager ebm(
        folly::EventBase::Options().setBackendFactory(std::move(func)));

    auto* evb = folly::EventBaseManager::get()->getEventBase();
    // use the same EventBase for the main thread
    if (evb) {
      ebm.setEventBase(evb, false);
    }
    return std::make_shared<folly::IOThreadPoolExecutor>(
        numThreads, threadFactory, &ebm);
  } else {
    return std::make_shared<folly::IOThreadPoolExecutor>(
        numThreads, threadFactory);
  }
}

std::shared_ptr<ThriftServer> createStressTestServer(
    std::shared_ptr<apache::thrift::ServiceHandler<StressTest>> handler) {
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
  server->setNumCPUWorkerThreads(sanitizeNumThreads(FLAGS_cpu_threads));

  LOG(INFO) << "Enable Checksum Support: " << FLAGS_enable_checksum;
  if (FLAGS_enable_checksum) {
    PayloadSerializer::initialize(
        ChecksumPayloadSerializerStrategy<LegacyPayloadSerializerStrategy>(
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

  LOG(INFO) << "Active Request Tracking Disabled: "
            << FLAGS_disable_active_request_tracking;
  if (FLAGS_disable_active_request_tracking) {
    server->disableActiveRequestsTracking();
  }

  if (FLAGS_max_requests > -1) {
    LOG(INFO) << "Setting max server requests: " << FLAGS_max_requests;
    server->setMaxRequests(FLAGS_max_requests);
  }
  server->setSSLPolicy(apache::thrift::SSLPolicy::PERMITTED);
  if (!FLAGS_certPath.empty() && !FLAGS_keyPath.empty() &&
      !FLAGS_caPath.empty()) {
    server->setSSLConfig(getSSLConfig());
  }

  if (!isResourcePoolsEnabled()) {
    return server;
  }

  LOG(INFO) << "Resource pools enabled";
  std::shared_ptr<folly::Executor> executor;
  auto t = sanitizeNumThreads(FLAGS_cpu_threads);

  if (isWorkStealingEnabled()) {
    LOG(INFO) << "Work Stealing Executor enabled";
    executor = std::make_shared<folly::WorkStealingExecutor>(FLAGS_cpu_threads);
  } else {
    LOG(INFO) << "CPU Thread Pool Executor enabled";
    executor = std::make_shared<folly::CPUThreadPoolExecutor>(
        t, folly::CPUThreadPoolExecutor::makeThrottledLifoSemQueue());
  }

  std::unique_ptr<RequestPileInterface> requestPile;
  if (isRoundRobinRequestPileEnabled()) {
    LOG(INFO) << "Round Robin Request Pile enabled";
    requestPile = std::make_unique<RoundRobinRequestPile>(
        RoundRobinRequestPile::Options{});
  }

  std::unique_ptr<ConcurrencyControllerInterface> concurrencyController;
  if (isParallelConcurrencyControllerEnabled()) {
    LOG(INFO) << "Parallel Concurrency Controller enabled";
    concurrencyController = std::make_unique<ParallelConcurrencyController>(
        *requestPile.get(), *executor.get());
  } else if (isSEParallelConcurrencyControllerEnabled()) {
    LOG(INFO) << "ParallelConcurrencyController with SerialExecutor enabled";
    concurrencyController = std::make_unique<ParallelConcurrencyController>(
        *requestPile.get(),
        *executor.get(),
        ParallelConcurrencyController::RequestExecutionMode::Serial);
  }

  server->resourcePoolSet().setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(requestPile),
      executor,
      std::move(concurrencyController));
  server->ensureResourcePools();
  server->requireResourcePools();

  return server;
}

} // namespace stress
} // namespace thrift
} // namespace apache
