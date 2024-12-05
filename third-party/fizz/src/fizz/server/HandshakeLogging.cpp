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

void HandshakeLogging::populateFromClientHello(const ClientHello& chlo) {
  clientLegacyVersion = chlo.legacy_version;
  auto supportedVersions = getExtension<SupportedVersions>(chlo.extensions);
  if (supportedVersions) {
    clientSupportedVersions = supportedVersions->versions;
  }
  clientCiphers = chlo.cipher_suites;
  clientExtensions.clear();
  for (const auto& extension : chlo.extensions) {
    clientExtensions.push_back(extension.extension_type);
    if (extension.extension_type == ExtensionType::test_extension &&
        extension.extension_data->length() == 1) {
      // Special extension we want to log the byte for
      testExtensionByte = *extension.extension_data->data();
    }
  }
  clientAlpns.clear();
  auto alpn = getExtension<ProtocolNameList>(chlo.extensions);
  if (alpn) {
    for (auto& protocol : alpn->protocol_name_list) {
      clientAlpns.push_back(protocol.name->to<std::string>());
    }
  }
  auto sni = getExtension<ServerNameList>(chlo.extensions);
  if (sni && !sni->server_name_list.empty()) {
    clientSni = sni->server_name_list.front().hostname->to<std::string>();
  }
  auto supportedGroups = getExtension<SupportedGroups>(chlo.extensions);
  if (supportedGroups) {
    clientSupportedGroups = std::move(supportedGroups->named_group_list);
  }

  auto keyShare = getExtension<ClientKeyShare>(chlo.extensions);
  if (keyShare && !clientKeyShares) {
    std::vector<NamedGroup> shares;
    for (const auto& entry : keyShare->client_shares) {
      shares.push_back(entry.group);
    }
    clientKeyShares = std::move(shares);
  }

  auto exchangeModes = getExtension<PskKeyExchangeModes>(chlo.extensions);
  if (exchangeModes) {
    clientKeyExchangeModes = std::move(exchangeModes->modes);
  }

  auto clientSigSchemes = getExtension<SignatureAlgorithms>(chlo.extensions);
  if (clientSigSchemes) {
    clientSignatureAlgorithms =
        std::move(clientSigSchemes->supported_signature_algorithms);
  }

  clientSessionIdSent =
      chlo.legacy_session_id && !chlo.legacy_session_id->empty();
  clientRandom = chlo.random;

  if (chlo.originalEncoding.hasValue()) {
    originalChloSize = chlo.originalEncoding.value()->computeChainDataLength();
  }
}
} // namespace server
} // namespace fizz
