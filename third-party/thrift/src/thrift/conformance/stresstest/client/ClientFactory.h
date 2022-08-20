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

#pragma once

#include <fizz/client/FizzClientContext.h>
#include <fizz/protocol/CertificateVerifier.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <thrift/conformance/stresstest/if/gen-cpp2/StressTest.h>

namespace apache {
namespace thrift {
namespace stress {

enum class ClientSecurity {
  None = 0,
  TLS,
  FIZZ,
};

struct ClientConnectionConfig {
  folly::SocketAddress serverHost;
  ClientSecurity security;
  std::string certPath;
  std::string keyPath;
  std::string trustedCertsPath;
};

class ClientFactory {
 public:
  static std::unique_ptr<StressTestAsyncClient> createClient(
      folly::EventBase* evb, const ClientConnectionConfig& cfg);

  static void useCustomSslContext(
      std::function<std::shared_ptr<folly::SSLContext>()> fn);
  static void useCustomFizzClientContext(
      std::function<std::shared_ptr<fizz::client::FizzClientContext>()> fn);
  static void useCustomFizzVerifier(
      std::function<std::shared_ptr<fizz::CertificateVerifier>()> fn);
};

} // namespace stress
} // namespace thrift
} // namespace apache
