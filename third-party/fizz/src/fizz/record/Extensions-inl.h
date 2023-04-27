/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <vector>
#include "folly/io/IOBuf.h"

#include <fizz/record/Types.h>

namespace fizz {

inline std::vector<Extension>::const_iterator findExtension(
    const std::vector<Extension>& extensions,
    ExtensionType type) {
  for (auto it = extensions.begin(); it != extensions.end(); ++it) {
    if (it->extension_type == type) {
      return it;
    }
  }
  return extensions.end();
}

template <class T>
inline folly::Optional<T> getExtension(
    const std::vector<Extension>& extensions) {
  auto it = findExtension(extensions, T::extension_type);
  if (it == extensions.end()) {
    return folly::none;
  }
  folly::io::Cursor cs{it->extension_data.get()};
  auto ret = getExtension<T>(cs);
  if (!cs.isAtEnd()) {
    throw std::runtime_error("didn't read entire extension");
  }
  return ret;
}

template <>
inline SignatureAlgorithms getExtension(folly::io::Cursor& cs) {
  SignatureAlgorithms sigs;
  detail::readVector<uint16_t>(sigs.supported_signature_algorithms, cs);
  return sigs;
}

template <>
inline SupportedGroups getExtension(folly::io::Cursor& cs) {
  SupportedGroups groups;
  detail::readVector<uint16_t>(groups.named_group_list, cs);
  return groups;
}

template <>
inline ClientKeyShare getExtension(folly::io::Cursor& cs) {
  ClientKeyShare share;
  detail::readVector<uint16_t>(share.client_shares, cs);
  return share;
}

template <>
inline ServerKeyShare getExtension(folly::io::Cursor& cs) {
  ServerKeyShare share;
  detail::read(share.server_share, cs);
  return share;
}

template <>
inline HelloRetryRequestKeyShare getExtension(folly::io::Cursor& cs) {
  HelloRetryRequestKeyShare share;
  detail::read(share.selected_group, cs);
  return share;
}

template <>
inline ClientPresharedKey getExtension(folly::io::Cursor& cs) {
  ClientPresharedKey share;
  detail::readVector<uint16_t>(share.identities, cs);
  detail::readVector<uint16_t>(share.binders, cs);
  return share;
}

template <>
inline ServerPresharedKey getExtension(folly::io::Cursor& cs) {
  ServerPresharedKey share;
  detail::read(share.selected_identity, cs);
  return share;
}

template <>
inline ClientEarlyData getExtension(folly::io::Cursor& /* unused */) {
  return ClientEarlyData();
}

template <>
inline ServerEarlyData getExtension(folly::io::Cursor& /* unused */) {
  return ServerEarlyData();
}

template <>
inline TicketEarlyData getExtension(folly::io::Cursor& cs) {
  TicketEarlyData early;
  detail::read(early.max_early_data_size, cs);
  return early;
}

template <>
inline Cookie getExtension(folly::io::Cursor& cs) {
  Cookie cookie;
  detail::readBuf<uint16_t>(cookie.cookie, cs);
  return cookie;
}

template <>
inline SupportedVersions getExtension(folly::io::Cursor& cs) {
  SupportedVersions versions;
  detail::readVector<uint8_t>(versions.versions, cs);
  return versions;
}

template <>
inline ServerSupportedVersions getExtension(folly::io::Cursor& cs) {
  ServerSupportedVersions versions;
  detail::read(versions.selected_version, cs);
  return versions;
}

template <>
inline PskKeyExchangeModes getExtension(folly::io::Cursor& cs) {
  PskKeyExchangeModes modes;
  detail::readVector<uint8_t>(modes.modes, cs);
  return modes;
}

template <>
inline ProtocolNameList getExtension(folly::io::Cursor& cs) {
  ProtocolNameList names;
  detail::readVector<uint16_t>(names.protocol_name_list, cs);
  return names;
}

template <>
inline ServerNameList getExtension(folly::io::Cursor& cs) {
  ServerNameList names;
  detail::readVector<uint16_t>(names.server_name_list, cs);
  return names;
}

template <>
inline CertificateAuthorities getExtension(folly::io::Cursor& cs) {
  CertificateAuthorities authorities;
  detail::readVector<uint16_t>(authorities.authorities, cs);
  return authorities;
}

template <>
inline CertificateCompressionAlgorithms getExtension(folly::io::Cursor& cs) {
  CertificateCompressionAlgorithms cca;
  detail::readVector<uint8_t>(cca.algorithms, cs);
  return cca;
}

template <>
inline Extension encodeExtension(const SignatureAlgorithms& sig) {
  Extension ext;
  ext.extension_type = ExtensionType::signature_algorithms;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(sig.supported_signature_algorithms, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const SupportedGroups& groups) {
  Extension ext;
  ext.extension_type = ExtensionType::supported_groups;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(groups.named_group_list, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ClientKeyShare& share) {
  Extension ext;
  ext.extension_type = ExtensionType::key_share;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(share.client_shares, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ServerKeyShare& share) {
  Extension ext;
  ext.extension_type = ExtensionType::key_share;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::write(share.server_share, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const HelloRetryRequestKeyShare& share) {
  Extension ext;
  ext.extension_type = ExtensionType::key_share;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::write(share.selected_group, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ClientPresharedKey& share) {
  Extension ext;
  ext.extension_type = ExtensionType::pre_shared_key;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(share.identities, appender);
  detail::writeVector<uint16_t>(share.binders, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ServerPresharedKey& share) {
  Extension ext;
  ext.extension_type = ExtensionType::pre_shared_key;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::write(share.selected_identity, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ClientEarlyData&) {
  Extension ext;
  ext.extension_type = ExtensionType::early_data;
  ext.extension_data = folly::IOBuf::create(0);
  return ext;
}

template <>
inline Extension encodeExtension(const ServerEarlyData&) {
  Extension ext;
  ext.extension_type = ExtensionType::early_data;
  ext.extension_data = folly::IOBuf::create(0);
  return ext;
}

template <>
inline Extension encodeExtension(const TicketEarlyData& early) {
  Extension ext;
  ext.extension_type = ExtensionType::early_data;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::write(early.max_early_data_size, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const Cookie& cookie) {
  Extension ext;
  ext.extension_type = ExtensionType::cookie;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeBuf<uint16_t>(cookie.cookie, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const SupportedVersions& versions) {
  Extension ext;
  ext.extension_type = ExtensionType::supported_versions;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint8_t>(versions.versions, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ServerSupportedVersions& versions) {
  Extension ext;
  ext.extension_type = ExtensionType::supported_versions;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::write(versions.selected_version, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const PskKeyExchangeModes& modes) {
  Extension ext;
  ext.extension_type = ExtensionType::psk_key_exchange_modes;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint8_t>(modes.modes, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ProtocolNameList& names) {
  Extension ext;
  ext.extension_type = ExtensionType::application_layer_protocol_negotiation;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(names.protocol_name_list, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ServerNameList& names) {
  Extension ext;
  ext.extension_type = ExtensionType::server_name;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(names.server_name_list, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const CertificateAuthorities& authorities) {
  Extension ext;
  ext.extension_type = ExtensionType::certificate_authorities;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint16_t>(authorities.authorities, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const CertificateCompressionAlgorithms& cca) {
  Extension ext;
  ext.extension_type = ExtensionType::compress_certificate;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint8_t>(cca.algorithms, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const EchOuterExtensions& outerExt) {
  Extension ext;
  ext.extension_type = outerExt.extension_type;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::writeVector<uint8_t>(outerExt.extensionTypes, appender);
  return ext;
}

inline size_t getBinderLength(const ClientHello& chlo) {
  if (chlo.extensions.empty() ||
      chlo.extensions.back().extension_type != ExtensionType::pre_shared_key) {
    throw FizzException(
        "psk not at end of client hello", AlertDescription::decode_error);
  }
  folly::io::Cursor cursor(chlo.extensions.back().extension_data.get());
  uint16_t identitiesLen;
  detail::read(identitiesLen, cursor);
  cursor.skip(identitiesLen);
  uint16_t binderLen;
  detail::read(binderLen, cursor);
  if (cursor.totalLength() != binderLen) {
    throw FizzException(
        "malformed binder length", AlertDescription::decode_error);
  }
  return sizeof(binderLen) + binderLen;
}

namespace detail {

template <>
struct Reader<KeyShareEntry> {
  template <class T>
  size_t read(KeyShareEntry& out, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += detail::read(out.group, cursor);
    len += readBuf<uint16_t>(out.key_exchange, cursor);
    return len;
  }
};

template <>
struct Writer<KeyShareEntry> {
  template <class T>
  void write(const KeyShareEntry& share, folly::io::Appender& out) {
    detail::write(share.group, out);
    detail::writeBuf<uint16_t>(share.key_exchange, out);
  }
};

template <>
struct Sizer<KeyShareEntry> {
  template <class T>
  size_t getSize(const KeyShareEntry& share) {
    return sizeof(NamedGroup) + getBufSize<uint16_t>(share.key_exchange);
  }
};

template <>
struct Reader<PskIdentity> {
  template <class T>
  size_t read(PskIdentity& out, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += readBuf<uint16_t>(out.psk_identity, cursor);
    len += detail::read(out.obfuscated_ticket_age, cursor);
    return len;
  }
};

template <>
struct Writer<PskIdentity> {
  template <class T>
  void write(const PskIdentity& ident, folly::io::Appender& out) {
    writeBuf<uint16_t>(ident.psk_identity, out);
    detail::write(ident.obfuscated_ticket_age, out);
  }
};

template <>
struct Sizer<PskIdentity> {
  template <class T>
  size_t getSize(const PskIdentity& ident) {
    return getBufSize<uint16_t>(ident.psk_identity) + sizeof(uint32_t);
  }
};

template <>
struct Reader<PskBinder> {
  template <class T>
  size_t read(PskBinder& out, folly::io::Cursor& cursor) {
    return readBuf<uint8_t>(out.binder, cursor);
  }
};

template <>
struct Writer<PskBinder> {
  template <class T>
  void write(const PskBinder& binder, folly::io::Appender& out) {
    writeBuf<uint8_t>(binder.binder, out);
  }
};

template <>
struct Sizer<PskBinder> {
  template <class T>
  size_t getSize(const PskBinder& binder) {
    return getBufSize<uint8_t>(binder.binder);
  }
};

template <>
struct Reader<ProtocolName> {
  template <class T>
  size_t read(ProtocolName& name, folly::io::Cursor& cursor) {
    return readBuf<uint8_t>(name.name, cursor);
  }
};

template <>
struct Writer<ProtocolName> {
  template <class T>
  void write(const ProtocolName& name, folly::io::Appender& out) {
    writeBuf<uint8_t>(name.name, out);
  }
};

template <>
struct Sizer<ProtocolName> {
  template <class T>
  size_t getSize(const ProtocolName& name) {
    return getBufSize<uint8_t>(name.name);
  }
};

template <>
struct Reader<ServerName> {
  template <class T>
  size_t read(ServerName& name, folly::io::Cursor& cursor) {
    size_t size = 0;
    size += detail::read(name.name_type, cursor);
    size += readBuf<uint16_t>(name.hostname, cursor);
    return size;
  }
};

template <>
struct Writer<ServerName> {
  template <class T>
  void write(const ServerName& name, folly::io::Appender& out) {
    detail::write(name.name_type, out);
    writeBuf<uint16_t>(name.hostname, out);
  }
};

template <>
struct Sizer<ServerName> {
  template <class T>
  size_t getSize(const ServerName& name) {
    return sizeof(ServerNameType) + getBufSize<uint16_t>(name.hostname);
  }
};

template <>
struct Reader<DistinguishedName> {
  template <class T>
  size_t read(DistinguishedName& dn, folly::io::Cursor& cursor) {
    return readBuf<uint16_t>(dn.encoded_name, cursor);
  }
};

template <>
struct Writer<DistinguishedName> {
  template <class T>
  void write(const DistinguishedName& dn, folly::io::Appender& out) {
    writeBuf<uint16_t>(dn.encoded_name, out);
  }
};

template <>
struct Sizer<DistinguishedName> {
  template <class T>
  size_t getSize(const DistinguishedName& dn) {
    return getBufSize<uint16_t>(dn.encoded_name);
  }
};
} // namespace detail
} // namespace fizz
