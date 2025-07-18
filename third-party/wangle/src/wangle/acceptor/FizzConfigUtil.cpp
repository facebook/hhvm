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

#include <wangle/acceptor/FizzConfigUtil.h>

#include <fmt/format.h>

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <folly/String.h>

using fizz::DefaultCertificateVerifier;
using fizz::FizzUtil;
using fizz::ProtocolVersion;
using fizz::VerificationContext;
using fizz::openssl::CertUtils;
using fizz::server::ClientAuthMode;

namespace wangle {

bool FizzConfigUtil::addCertsToManager(
    const std::vector<SSLContextConfig>& configs,
    fizz::server::CertManager& manager,
    const std::shared_ptr<PasswordInFileFactory>& pwFactory,
    bool strictSSL) {
  bool loadedCert = false;
  for (const auto& sslConfig : configs) {
    for (const auto& cert : sslConfig.certificates) {
      try {
        std::unique_ptr<fizz::SelfCert> selfCert;
        if (cert.isBuffer) {
          selfCert = CertUtils::makeSelfCert(cert.certPath, cert.keyPath);
        } else {
          auto x509Chain = FizzUtil::readChainFile(cert.certPath);
          std::shared_ptr<folly::PasswordInFile> pw;
          if (pwFactory) {
            pw = pwFactory->getPasswordCollector(cert.passwordPath);
          } else {
            pw = std::make_shared<folly::PasswordInFile>(cert.passwordPath);
          }

          auto pkey = FizzUtil::readPrivateKey(cert.keyPath, pw);
          selfCert =
              CertUtils::makeSelfCert(std::move(x509Chain), std::move(pkey));
        }
        if (sslConfig.isDefault) {
          manager.addCertAndSetDefault(std::move(selfCert));
        } else {
          manager.addCert(std::move(selfCert));
        }
        loadedCert = true;
      } catch (const std::runtime_error& ex) {
        auto msg = fmt::format(
            "Failed to load cert or key at key path {}, cert path {}",
            cert.keyPath,
            cert.certPath);
        if (strictSSL) {
          throw std::runtime_error(ex.what() + msg);
        } else {
          LOG(ERROR) << msg << ex.what();
        }
      }
    }
  }
  return loadedCert;
}

std::unique_ptr<fizz::server::CertManager> FizzConfigUtil::createCertManager(
    const std::vector<SSLContextConfig>& sslContextConfigs,
    const std::shared_ptr<PasswordInFileFactory>& pwFactory,
    bool strictSSL) {
  auto certMgr = std::make_unique<fizz::server::CertManager>();
  if (!addCertsToManager(sslContextConfigs, *certMgr, pwFactory, strictSSL)) {
    return nullptr;
  }
  return certMgr;
}

std::shared_ptr<fizz::server::FizzServerContext>
FizzConfigUtil::createFizzContext(
    const std::vector<SSLContextConfig>& sslContextConfigs,
    const FizzConfig& fizzConfig,
    bool strictSSL) {
  if (sslContextConfigs.empty()) {
    return nullptr;
  }
  auto ctx = std::make_shared<fizz::server::FizzServerContext>();
  ctx->setSupportedVersions(
      {ProtocolVersion::tls_1_3,
       ProtocolVersion::tls_1_3_28,
       ProtocolVersion::tls_1_3_26});
  ctx->setVersionFallbackEnabled(true);

  if (!fizzConfig.supportedPskModes.empty()) {
    ctx->setSupportedPskModes(fizzConfig.supportedPskModes);
  }

  // Fizz does not yet support randomized next protocols so we use the highest
  // weighted list on the first context.
  const auto& list = sslContextConfigs.front().nextProtocols;
  if (!list.empty()) {
    ctx->setSupportedAlpns(FizzUtil::getAlpnsFromNpnList(list));
  }

  if (sslContextConfigs.front().alpnAllowMismatch) {
    ctx->setAlpnMode(fizz::server::AlpnMode::AllowMismatch);
  } else {
    ctx->setAlpnMode(fizz::server::AlpnMode::Optional);
  }

  auto verify = sslContextConfigs.front().clientVerification;
  switch (verify) {
    case folly::SSLContext::VerifyClientCertificate::ALWAYS:
      ctx->setClientAuthMode(ClientAuthMode::Required);
      break;
    case folly::SSLContext::VerifyClientCertificate::IF_PRESENTED:
      ctx->setClientAuthMode(ClientAuthMode::Optional);
      break;
    case folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST:
      ctx->setClientAuthMode(ClientAuthMode::None);
  }

  const auto& caFile = sslContextConfigs.front().clientCAFile;
  const auto& caFiles = sslContextConfigs.front().clientCAFiles;

  std::vector<std::string> combinedCAFiles = {};

  if (!caFile.empty()) {
    combinedCAFiles.push_back(caFile);
  }
  for (const auto& singleCAFile : caFiles) {
    if (!singleCAFile.empty()) {
      combinedCAFiles.push_back((singleCAFile));
    }
  }

  if (!combinedCAFiles.empty()) {
    try {
      auto verifier = DefaultCertificateVerifier::createFromCAFiles(
          VerificationContext::Server, combinedCAFiles);
      ctx->setClientCertVerifier(std::move(verifier));
    } catch (const std::runtime_error& ex) {
      auto msg = fmt::format(
          " Failed to load ca file at {}", folly::join(", ", combinedCAFiles));
      if (strictSSL) {
        throw std::runtime_error(ex.what() + msg);
      } else {
        LOG(ERROR) << msg << ex.what();
        return nullptr;
      }
    }
  }

  return ctx;
}

} // namespace wangle
