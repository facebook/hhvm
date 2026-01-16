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

#include <fizz/server/DefaultCertManager.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/TicketTypes.h>
#include <fizz/util/FizzUtil.h>

#include <wangle/acceptor/ServerSocketConfig.h>
#include <wangle/ssl/PasswordInFileFactory.h>

namespace wangle {

class FizzConfigUtil {
 public:
  /*
   * Adds certs to the cert manager from context configs.
   * If strictSSL is specified will throw on any failed cert load.
   * Returns true if atleast one cert was successfully loaded
   */
  static bool addCertsToManager(
      const std::vector<SSLContextConfig>& configs,
      fizz::server::DefaultCertManager& manager,
      const std::shared_ptr<PasswordInFileFactory>& pwFactory,
      bool strictSSL);

  /**
   * Creates a new cert manager and adds the certs from the given
   * context configs using addCertsToManager.
   * Returns nullptr if fails to add certificate for at least one
   * context config.
   */
  static std::unique_ptr<fizz::server::CertManager> createCertManager(
      const std::vector<SSLContextConfig>& sslContextConfigs,
      const std::shared_ptr<PasswordInFileFactory>& pwFactory,
      bool strictSSL);

  static std::shared_ptr<fizz::server::FizzServerContext> createFizzContext(
      const std::vector<SSLContextConfig>& sslContextConfigs,
      const FizzConfig& fizzConfig,
      bool strictSSL);

  // Creates a TicketCipher with given params
  template <class TicketCipherT>
  static std::unique_ptr<TicketCipherT> createTicketCipher(
      const TLSTicketKeySeeds& seeds,
      std::chrono::seconds validity,
      std::chrono::seconds handshakeValidity,
      std::shared_ptr<fizz::Factory> factory,
      std::shared_ptr<fizz::server::CertManager> certManager,
      folly::Optional<std::string> pskContext) {
    if (seeds.currentSeeds.empty()) {
      return fizz::FizzUtil::createTicketCipher<TicketCipherT>(
          seeds.oldSeeds,
          "",
          seeds.newSeeds,
          validity,
          handshakeValidity,
          std::move(factory),
          std::move(certManager),
          std::move(pskContext));
    } else {
      return fizz::FizzUtil::createTicketCipher<TicketCipherT>(
          seeds.oldSeeds,
          seeds.currentSeeds.at(0),
          seeds.newSeeds,
          validity,
          handshakeValidity,
          std::move(factory),
          std::move(certManager),
          std::move(pskContext));
    }
  }

  // Creates AES128TicketCipher as default fizz ticket cipher
  static std::shared_ptr<fizz::server::TicketCipher> createFizzTicketCipher(
      const wangle::TLSTicketKeySeeds& seeds,
      std::chrono::seconds validity,
      std::chrono::seconds handshakeValidity,
      std::shared_ptr<fizz::Factory> factory,
      std::shared_ptr<fizz::server::CertManager> certManager,
      folly::Optional<std::string> pskContext) {
    return wangle::FizzConfigUtil::createTicketCipher<
        fizz::server::AES128TicketCipher>(
        seeds,
        validity,
        handshakeValidity,
        std::move(factory),
        std::move(certManager),
        std::move(pskContext));
  }
};

} // namespace wangle
