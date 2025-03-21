/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Extensions.h>

namespace fizz {
namespace server {

struct SharedChloFields {
  folly::Optional<Random> clientRandom;
  void populateSharedFieldsFromClientHello(const ClientHello& chlo);
};

struct HandshakeLogging : SharedChloFields {
  folly::Optional<ProtocolVersion> clientLegacyVersion;
  std::vector<ProtocolVersion> clientSupportedVersions;
  std::vector<CipherSuite> clientCiphers;
  std::vector<ExtensionType> clientExtensions;
  folly::Optional<ProtocolVersion> clientRecordVersion;
  folly::Optional<std::string> clientSni;
  std::vector<NamedGroup> clientSupportedGroups;
  folly::Optional<std::vector<NamedGroup>> clientKeyShares;
  std::vector<PskKeyExchangeMode> clientKeyExchangeModes;
  std::vector<SignatureScheme> clientSignatureAlgorithms;
  folly::Optional<bool> clientSessionIdSent;
  folly::Optional<uint8_t> testExtensionByte;
  std::vector<std::string> clientAlpns;
  size_t originalChloSize{0};
  SharedChloFields outerChloInfo;

  void populateFromClientHello(const ClientHello& chlo);
  void populateFromPreECHClientHello(const ClientHello& chlo);
};
} // namespace server
} // namespace fizz
