/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/crypto/hpke/Hpke.h>
#include <fizz/protocol/Factory.h>
#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/Types.h>

namespace fizz {
namespace ech {

struct SupportedECHConfig {
  ECHConfig config;
  ECHCipherSuite cipherSuite;
};

// Used to indicate to the Decrypter that extension expansion failed (which is
// a hard error)
class OuterExtensionsError : public std::runtime_error {
 public:
  explicit OuterExtensionsError(const std::string& what)
      : std::runtime_error(what) {}
};

folly::Optional<SupportedECHConfig> selectECHConfig(
    const std::vector<ECHConfig>& configs,
    std::vector<hpke::KEMId> supportedKEMs,
    std::vector<hpke::AeadId> supportedAeads);

hpke::SetupResult constructHpkeSetupResult(
    std::unique_ptr<KeyExchange> kex,
    const SupportedECHConfig& supportedConfig);

std::unique_ptr<folly::IOBuf> makeClientHelloAad(
    ECHCipherSuite cipherSuite,
    const std::unique_ptr<folly::IOBuf>& configId,
    const std::unique_ptr<folly::IOBuf>& enc,
    const std::unique_ptr<folly::IOBuf>& clientHello);

ServerHello makeDummyServerHello(const ServerHello& shlo);

bool checkECHAccepted(
    const ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler>& scheduler);

void setAcceptConfirmation(
    ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler>& scheduler);

ClientECH encryptClientHelloHRR(
    const SupportedECHConfig& supportedConfig,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult);

ClientECH encryptClientHello(
    const SupportedECHConfig& supportedConfig,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult);

ClientHello decryptECHWithContext(
    const ClientHello& clientHelloOuter,
    const ECHConfig& echConfig,
    ECHCipherSuite& cipherSuite,
    std::unique_ptr<folly::IOBuf> encapsulatedKey,
    std::unique_ptr<folly::IOBuf> configId,
    std::unique_ptr<folly::IOBuf> encryptedCh,
    ECHVersion version,
    std::unique_ptr<hpke::HpkeContext>& context);

std::unique_ptr<hpke::HpkeContext> setupDecryptionContext(
    const ECHConfig& echConfig,
    ECHCipherSuite cipherSuite,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey,
    std::unique_ptr<KeyExchange> kex,
    uint64_t seqNum);

std::unique_ptr<folly::IOBuf> constructConfigId(
    hpke::KDFId kdfId,
    ECHConfig echConfig);

std::unique_ptr<folly::IOBuf> getRecordDigest(
    const ECHConfig& echConfig,
    hpke::KDFId id);

std::vector<Extension> substituteOuterExtensions(
    std::vector<Extension>&& innerExt,
    const std::vector<Extension>& outerExt);

} // namespace ech
} // namespace fizz
