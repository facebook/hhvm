/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/CookieCipher.h>

#include <fizz/protocol/HandshakeContext.h>
#include <fizz/record/Extensions.h>
#include <fizz/server/Negotiator.h>

namespace fizz {
namespace server {

Buf getStatelessHelloRetryRequest(
    ProtocolVersion version,
    CipherSuite cipher,
    folly::Optional<NamedGroup> group,
    Buf cookie) {
  Buf encodedHelloRetryRequest;

  HelloRetryRequest hrr;
  hrr.legacy_version = ProtocolVersion::tls_1_2;
  hrr.legacy_session_id_echo = folly::IOBuf::create(0);
  hrr.cipher_suite = cipher;

  ServerSupportedVersions versionExt;
  versionExt.selected_version = version;
  hrr.extensions.push_back(encodeExtension(std::move(versionExt)));

  if (group) {
    HelloRetryRequestKeyShare keyShare;
    keyShare.selected_group = *group;
    hrr.extensions.push_back(encodeExtension(std::move(keyShare)));
  }

  Cookie cookieExt;
  cookieExt.cookie = std::move(cookie);
  hrr.extensions.push_back(encodeExtension(std::move(cookieExt)));

  return encodeHandshake(std::move(hrr));
}

static folly::Optional<NamedGroup> getHrrGroup(
    const std::vector<NamedGroup>& supportedGroups,
    const ClientHello& chlo) {
  auto groupsExt = getExtension<SupportedGroups>(chlo.extensions);
  if (!groupsExt) {
    return folly::none;
  }

  // Group is negotiated solely based on supported groups, without considering
  // which shares were sent.
  auto negotiatedGroup =
      negotiate(supportedGroups, groupsExt->named_group_list);
  if (!negotiatedGroup) {
    // We will deal with any supported group mismatch at the full handshake.
    return folly::none;
  }

  auto clientShares = getExtension<ClientKeyShare>(chlo.extensions);
  if (!clientShares) {
    throw std::runtime_error("supported_groups without key_share");
  }

  for (const auto& share : clientShares->client_shares) {
    if (share.group == *negotiatedGroup) {
      // We already have the right key share.
      return folly::none;
    }
  }

  return negotiatedGroup;
}

CookieState getCookieState(
    const Factory& factory,
    const std::vector<ProtocolVersion>& supportedVersions,
    const std::vector<std::vector<CipherSuite>>& supportedCiphers,
    const std::vector<NamedGroup>& supportedGroups,
    const ClientHello& chlo,
    Buf appToken) {
  auto clientVersions = getExtension<SupportedVersions>(chlo.extensions);
  if (!clientVersions) {
    throw std::runtime_error("no supported versions");
  }
  auto version = negotiate(supportedVersions, clientVersions->versions);
  if (!version) {
    throw std::runtime_error("version mismatch");
  }

  auto cipher = negotiate(supportedCiphers, chlo.cipher_suites);
  if (!cipher) {
    throw std::runtime_error("cipher mismatch");
  }

  auto group = getHrrGroup(supportedGroups, chlo);

  CookieState state;
  state.version = *version;
  state.cipher = *cipher;
  state.group = group;
  state.appToken = std::move(appToken);

  auto handshakeContext = factory.makeHandshakeContext(*cipher);
  handshakeContext->appendToTranscript(*chlo.originalEncoding);
  state.chloHash = handshakeContext->getHandshakeContext();

  return state;
}
} // namespace server
} // namespace fizz
