/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/io/async/PasswordInFile.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/OpenSSLCertUtils.h>

#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/TicketPolicy.h>

namespace fizz {

class FizzUtil {
 public:
  // Read a vector of certs from a file
  static std::vector<folly::ssl::X509UniquePtr> readChainFile(
      const std::string& filename);

  static folly::ssl::EvpPkeyUniquePtr readPrivateKeyFromBuf(
      folly::ByteRange privateKey,
      const std::string& passwordFilename);

  static folly::ssl::EvpPkeyUniquePtr readPrivateKey(
      const std::string& filename,
      const std::shared_ptr<folly::PasswordInFile>& pf);

  static folly::ssl::EvpPkeyUniquePtr readPrivateKey(
      const std::string& filename,
      const std::string& passwordFilename);

  // Fizz does not yet support randomized next protocols so we use the highest
  // weighted list on the first context.
  static std::vector<std::string> getAlpnsFromNpnList(
      const std::list<folly::SSLContext::NextProtocolsItem>& list);

  // TODO richardsonnick Fix callsites that use std::string version
  static folly::ssl::EvpPkeyUniquePtr decryptPrivateKey(
      const std::string& data,
      folly::PasswordInFile* pf);

  static folly::ssl::EvpPkeyUniquePtr decryptPrivateKey(
      folly::ByteRange data,
      folly::PasswordInFile* pf);

  /**
   * `createKeyExchangeFromBuf` creates a `KeyExchange` object that
   * can be used as a KEM.
   *
   *  @param kemId   Determines the type of KEM to create, as well as the format
   * of what `privKey` conveys.
   *  @param privKey For NIST curve based KEMs, this is the PEM encoding of the
   * private key. For X25519 KEM, this is the hexlified representation of the 32
   * byte private key.
   *
   */

  static std::unique_ptr<KeyExchange> createKeyExchangeFromBuf(
      hpke::KEMId kemId,
      folly::ByteRange privKey);

  static std::tuple<std::string, std::string> generateKeypairCurve25519();

  // Creates a TicketCipherT with given params
  template <class TicketCipherT>
  static std::unique_ptr<TicketCipherT> createTicketCipher(
      const std::vector<std::string>& oldSecrets,
      const std::string& currentSecret,
      const std::vector<std::string>& newSecrets,
      std::chrono::seconds ticketValidity,
      std::chrono::seconds handshakeValidity,
      std::shared_ptr<Factory> factory,
      std::shared_ptr<server::CertManager> certManager,
      folly::Optional<std::string> pskContext) {
    std::unique_ptr<TicketCipherT> cipher;
    if (pskContext.hasValue()) {
      cipher = std::make_unique<TicketCipherT>(
          std::move(factory), std::move(certManager), std::move(*pskContext));
    } else {
      cipher = std::make_unique<TicketCipherT>(
          std::move(factory), std::move(certManager));
    }
    cipher->setTicketSecrets(
        compileSecrets(oldSecrets, currentSecret, newSecrets));
    server::TicketPolicy policy;
    policy.setTicketValidity(ticketValidity);
    policy.setHandshakeValidity(handshakeValidity);
    cipher->setPolicy(std::move(policy));
    return cipher;
  }

  // Creates a TokenCipher with given params
  template <class TokenCipherT>
  static std::unique_ptr<TokenCipherT> createTokenCipher(
      const std::vector<std::string>& oldSecrets,
      const std::string& currentSecret,
      const std::vector<std::string>& newSecrets,
      folly::Optional<std::string> pskContext,
      const std::string& codecContext) {
    std::unique_ptr<TokenCipherT> cipher;
    if (pskContext.hasValue()) {
      cipher = std::make_unique<TokenCipherT>(std::vector<std::string>(
          {codecContext, std::move(pskContext.value())}));
    } else {
      cipher = std::make_unique<TokenCipherT>(
          std::vector<std::string>({codecContext}));
    }
    cipher->setSecrets(compileSecrets(oldSecrets, currentSecret, newSecrets));
    return cipher;
  }

 private:
  static std::vector<folly::ByteRange> compileSecrets(
      const std::vector<std::string>& oldSecrets,
      const std::string& currentSecret,
      const std::vector<std::string>& newSecrets) {
    std::vector<folly::ByteRange> ticketSecrets;
    if (!currentSecret.empty()) {
      ticketSecrets.push_back(folly::StringPiece(currentSecret));
    }
    for (const auto& secret : oldSecrets) {
      ticketSecrets.push_back(folly::StringPiece(secret));
    }
    for (const auto& secret : newSecrets) {
      ticketSecrets.push_back(folly::StringPiece(secret));
    }
    return ticketSecrets;
  }
};

} // namespace fizz
