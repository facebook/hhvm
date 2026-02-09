/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/HandshakeLogging.h>

namespace fizz {
namespace server {

void SharedChloFields::populateSharedFieldsFromClientHello(
    const ClientHello& chlo) {
  clientRandom = chlo.random;
}

void HandshakeLogging::populateFromClientHello(const ClientHello& chlo) {
  clientLegacyVersion = chlo.legacy_version;
  folly::Optional<SupportedVersions> supportedVersions;
  Error err;
  FIZZ_THROW_ON_ERROR(
      getExtension(supportedVersions, err, chlo.extensions), err);
  if (supportedVersions) {
    clientSupportedVersions = supportedVersions->versions;
  }
  clientCiphers = chlo.cipher_suites;
  clientExtensions.clear();
  clientExtensions.reserve(chlo.extensions.size());
  for (const auto& extension : chlo.extensions) {
    clientExtensions.push_back(extension.extension_type);
    if (extension.extension_type == ExtensionType::test_extension &&
        extension.extension_data->length() == 1) {
      // Special extension we want to log the byte for
      testExtensionByte = *extension.extension_data->data();
    }
  }
  clientAlpns.clear();
  folly::Optional<ProtocolNameList> alpn;
  FIZZ_THROW_ON_ERROR(getExtension(alpn, err, chlo.extensions), err);
  if (alpn) {
    clientAlpns.reserve(alpn->protocol_name_list.size());
    for (auto& protocol : alpn->protocol_name_list) {
      clientAlpns.push_back(protocol.name->to<std::string>());
    }
  }
  folly::Optional<ServerNameList> sni;
  FIZZ_THROW_ON_ERROR(getExtension(sni, err, chlo.extensions), err);
  if (sni && !sni->server_name_list.empty()) {
    clientSni = sni->server_name_list.front().hostname->to<std::string>();
  }
  folly::Optional<SupportedGroups> supportedGroups;
  FIZZ_THROW_ON_ERROR(getExtension(supportedGroups, err, chlo.extensions), err);
  if (supportedGroups) {
    clientSupportedGroups = std::move(supportedGroups->named_group_list);
  }

  folly::Optional<ClientKeyShare> keyShare;
  FIZZ_THROW_ON_ERROR(getExtension(keyShare, err, chlo.extensions), err);
  if (keyShare && !clientKeyShares) {
    std::vector<NamedGroup> shares;
    shares.reserve(keyShare->client_shares.size());
    for (const auto& entry : keyShare->client_shares) {
      shares.push_back(entry.group);
    }
    clientKeyShares = std::move(shares);
  }

  folly::Optional<PskKeyExchangeModes> exchangeModes;
  FIZZ_THROW_ON_ERROR(getExtension(exchangeModes, err, chlo.extensions), err);
  if (exchangeModes) {
    clientKeyExchangeModes = std::move(exchangeModes->modes);
  }

  folly::Optional<SignatureAlgorithms> clientSigSchemes;
  FIZZ_THROW_ON_ERROR(
      getExtension(clientSigSchemes, err, chlo.extensions), err);
  if (clientSigSchemes) {
    clientSignatureAlgorithms =
        std::move(clientSigSchemes->supported_signature_algorithms);
  }

  clientSessionIdSent =
      chlo.legacy_session_id && !chlo.legacy_session_id->empty();

  populateSharedFieldsFromClientHello(chlo);
}

void HandshakeLogging::populateFromPreECHClientHello(const ClientHello& chlo) {
  if (chlo.originalEncoding.hasValue()) {
    originalChloSize = chlo.originalEncoding.value()->computeChainDataLength();
  }
  outerChloInfo.populateSharedFieldsFromClientHello(chlo);
}
} // namespace server
} // namespace fizz
