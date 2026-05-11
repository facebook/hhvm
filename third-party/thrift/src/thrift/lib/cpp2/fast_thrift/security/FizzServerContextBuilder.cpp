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

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <fmt/core.h>
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/server/DefaultCertManager.h>

namespace apache::thrift::fast_thrift::security {

namespace {

std::string readFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error(fmt::format("Failed to open {}", path));
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

} // namespace

std::shared_ptr<const fizz::server::FizzServerContext> buildFizzServerContext(
    const FizzServerCertConfig& certConfig,
    const ThriftTlsConfig& /*thriftConfig*/) {
  const bool hasPaths = !certConfig.certPath.empty();
  const bool hasBuffers = !certConfig.certPem.empty();
  if (hasPaths == hasBuffers) {
    throw std::runtime_error(
        "FizzServerCertConfig: exactly one of {certPath,keyPath} or "
        "{certPem,keyPem} must be set");
  }
  if (hasPaths && certConfig.keyPath.empty()) {
    throw std::runtime_error(
        "FizzServerCertConfig: keyPath required with certPath");
  }
  if (hasBuffers && certConfig.keyPem.empty()) {
    throw std::runtime_error(
        "FizzServerCertConfig: keyPem required with certPem");
  }

  std::string certData =
      hasPaths ? readFile(certConfig.certPath) : certConfig.certPem;
  std::string keyData =
      hasPaths ? readFile(certConfig.keyPath) : certConfig.keyPem;

  std::shared_ptr<fizz::SelfCert> selfCert = certConfig.keyPassword.empty()
      ? fizz::openssl::CertUtils::makeSelfCert(
            std::move(certData), std::move(keyData))
      : fizz::openssl::CertUtils::makeSelfCert(
            std::move(certData), std::move(keyData), certConfig.keyPassword);

  auto certManager = std::make_shared<fizz::server::DefaultCertManager>();
  certManager->addCertAndSetDefault(std::move(selfCert));

  auto ctx = std::make_shared<fizz::server::FizzServerContext>();
  ctx->setCertManager(std::move(certManager));
  ctx->setSupportedAlpns(certConfig.alpnProtocols);
  ctx->setClientAuthMode(certConfig.clientAuth);

  return ctx;
}

} // namespace apache::thrift::fast_thrift::security
