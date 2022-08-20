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

#include <folly/portability/GFlags.h>
#include <wangle/ssl/SSLContextConfig.h>

DEFINE_int32(port, 5000, "Server port");
DEFINE_int32(io_threads, 0, "Number of IO threads (0 == number of cores)");
DEFINE_int32(cpu_threads, 0, "Number of CPU threads (0 == number of cores)");
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

namespace apache {
namespace thrift {
namespace stress {

namespace {
uint32_t sanitizeNumThreads(int32_t n) {
  return n <= 0 ? std::thread::hardware_concurrency() : n;
}

std::shared_ptr<wangle::SSLContextConfig> getSSLConfig() {
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(FLAGS_certPath.c_str(), FLAGS_keyPath.c_str(), "");
  sslConfig->clientCAFile = FLAGS_caPath.c_str();
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

std::shared_ptr<ThriftServer> createStressTestServer(
    std::shared_ptr<StressTestHandler> handler) {
  if (!handler) {
    handler = createStressTestHandler();
  }
  auto server = std::make_shared<ThriftServer>();
  server->setInterface(std::move(handler));
  server->setPort(FLAGS_port);
  server->setNumIOWorkerThreads(sanitizeNumThreads(FLAGS_io_threads));
  server->setNumCPUWorkerThreads(sanitizeNumThreads(FLAGS_cpu_threads));
  server->setSSLPolicy(apache::thrift::SSLPolicy::PERMITTED);
  if (!FLAGS_certPath.empty() && !FLAGS_keyPath.empty() &&
      !FLAGS_caPath.empty()) {
    server->setSSLConfig(getSSLConfig());
  }
  return server;
}

} // namespace stress
} // namespace thrift
} // namespace apache
