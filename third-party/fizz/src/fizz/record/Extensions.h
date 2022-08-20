/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/Optional.h>

namespace fizz {

struct SignatureAlgorithms {
  std::vector<SignatureScheme> supported_signature_algorithms;
  static constexpr ExtensionType extension_type =
      ExtensionType::signature_algorithms;
};

struct SupportedGroups {
  std::vector<NamedGroup> named_group_list;
  static constexpr ExtensionType extension_type =
      ExtensionType::supported_groups;
};

struct KeyShareEntry {
  NamedGroup group;
  Buf key_exchange;
};

struct ClientKeyShare {
  std::vector<KeyShareEntry> client_shares;
  static constexpr ExtensionType extension_type = ExtensionType::key_share;
};

struct ServerKeyShare {
  KeyShareEntry server_share;
  static constexpr ExtensionType extension_type = ExtensionType::key_share;
};

struct HelloRetryRequestKeyShare {
  NamedGroup selected_group;
  static constexpr ExtensionType extension_type = ExtensionType::key_share;
};

struct PskIdentity {
  Buf psk_identity;
  uint32_t obfuscated_ticket_age;
};

struct PskBinder {
  Buf binder;
};

struct ClientPresharedKey {
  std::vector<PskIdentity> identities;
  std::vector<PskBinder> binders;
  static constexpr ExtensionType extension_type = ExtensionType::pre_shared_key;
};

struct ServerPresharedKey {
  uint16_t selected_identity;
  static constexpr ExtensionType extension_type = ExtensionType::pre_shared_key;
};

struct ClientEarlyData {
  static constexpr ExtensionType extension_type = ExtensionType::early_data;
};

struct ServerEarlyData {
  static constexpr ExtensionType extension_type = ExtensionType::early_data;
};

struct TicketEarlyData {
  uint32_t max_early_data_size;
  static constexpr ExtensionType extension_type = ExtensionType::early_data;
};

struct Cookie {
  Buf cookie;
  static constexpr ExtensionType extension_type = ExtensionType::cookie;
};

struct SupportedVersions {
  std::vector<ProtocolVersion> versions;
  static constexpr ExtensionType extension_type =
      ExtensionType::supported_versions;
};

struct ServerSupportedVersions {
  ProtocolVersion selected_version;
  static constexpr ExtensionType extension_type =
      ExtensionType::supported_versions;
};

struct PskKeyExchangeModes {
  std::vector<PskKeyExchangeMode> modes;
  static constexpr ExtensionType extension_type =
      ExtensionType::psk_key_exchange_modes;
};

struct ProtocolName {
  Buf name;
};

struct ProtocolNameList {
  std::vector<ProtocolName> protocol_name_list;
  static constexpr ExtensionType extension_type =
      ExtensionType::application_layer_protocol_negotiation;
};

enum class ServerNameType : uint8_t { host_name = 0 };

struct ServerName {
  ServerName() {}
  explicit ServerName(Buf hostnameIn) : hostname(std::move(hostnameIn)) {}

  ServerNameType name_type{ServerNameType::host_name};
  Buf hostname;
};

struct ServerNameList {
  ServerNameList() {}
  explicit ServerNameList(ServerName sn) {
    server_name_list.push_back(std::move(sn));
  }
  std::vector<ServerName> server_name_list;
  static constexpr ExtensionType extension_type = ExtensionType::server_name;
};

struct DistinguishedName {
  Buf encoded_name;
};

struct CertificateAuthorities {
  std::vector<DistinguishedName> authorities;
  static constexpr ExtensionType extension_type =
      ExtensionType::certificate_authorities;
};

struct CertificateCompressionAlgorithms {
  std::vector<CertificateCompressionAlgorithm> algorithms;
  static constexpr ExtensionType extension_type =
      ExtensionType::compress_certificate;
};

struct EchOuterExtensions {
  std::vector<ExtensionType> extensionTypes;
  static constexpr ExtensionType extension_type =
      ExtensionType::ech_outer_extensions;
};

template <class T>
folly::Optional<T> getExtension(const std::vector<Extension>& extension);
template <class T>
T getExtension(folly::io::Cursor& cursor);

template <class T>
Extension encodeExtension(const T& t);

std::vector<Extension>::const_iterator findExtension(
    const std::vector<Extension>& extensions,
    ExtensionType type);

size_t getBinderLength(const ClientHello& chlo);
} // namespace fizz

#include <fizz/record/Extensions-inl.h>
