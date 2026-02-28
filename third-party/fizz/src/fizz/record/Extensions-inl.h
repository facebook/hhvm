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
inline Status getExtension(
    folly::Optional<T>& ret,
    Error& err,
    const std::vector<Extension>& extensions) {
  auto it = findExtension(extensions, T::extension_type);
  if (it == extensions.end()) {
    ret = folly::none;
    return Status::Success;
  }
  folly::io::Cursor cs{it->extension_data.get()};
  T extVal;
  FIZZ_RETURN_ON_ERROR(getExtension<T>(extVal, err, cs));
  if (!cs.isAtEnd()) {
    return err.error("didn't read entire extension");
  }
  ret = std::move(extVal);
  return Status::Success;
}

template <>
inline Status
getExtension(SignatureAlgorithms& ret, Error& err, folly::io::Cursor& cs) {
  SignatureAlgorithms sigs;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(
          len, err, sigs.supported_signature_algorithms, cs));
  ret = std::move(sigs);
  return Status::Success;
}

template <>
inline Status
getExtension(SupportedGroups& ret, Error& err, folly::io::Cursor& cs) {
  SupportedGroups groups;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, groups.named_group_list, cs));
  ret = std::move(groups);
  return Status::Success;
}

template <>
inline Status
getExtension(ClientKeyShare& ret, Error& err, folly::io::Cursor& cs) {
  ClientKeyShare share;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, share.client_shares, cs));
  ret = std::move(share);
  return Status::Success;
}

template <>
inline Status
getExtension(ServerKeyShare& ret, Error& err, folly::io::Cursor& cs) {
  ServerKeyShare share;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, share.server_share, cs));
  ret = std::move(share);
  return Status::Success;
}

template <>
inline Status getExtension(
    HelloRetryRequestKeyShare& ret,
    Error& err,
    folly::io::Cursor& cs) {
  HelloRetryRequestKeyShare share;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, share.selected_group, cs));
  ret = std::move(share);
  return Status::Success;
}

template <>
inline Status
getExtension(ClientPresharedKey& ret, Error& err, folly::io::Cursor& cs) {
  ClientPresharedKey share;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, share.identities, cs));
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, share.binders, cs));
  ret = std::move(share);
  return Status::Success;
}

template <>
inline Status
getExtension(ServerPresharedKey& ret, Error& err, folly::io::Cursor& cs) {
  ServerPresharedKey share;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, share.selected_identity, cs));
  ret = std::move(share);
  return Status::Success;
}

template <>
inline Status getExtension(
    ClientEarlyData& ret,
    Error& /* err */,
    folly::io::Cursor& /* unused */) {
  ret = ClientEarlyData();
  return Status::Success;
}

template <>
inline Status getExtension(
    ServerEarlyData& ret,
    Error& /* err */,
    folly::io::Cursor& /* unused */) {
  ret = ServerEarlyData();
  return Status::Success;
}

template <>
inline Status
getExtension(TicketEarlyData& ret, Error& err, folly::io::Cursor& cs) {
  TicketEarlyData early;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, early.max_early_data_size, cs));
  ret = std::move(early);
  return Status::Success;
}

template <>
inline Status
getExtension(Cookie& ret, Error& /* err */, folly::io::Cursor& cs) {
  Cookie cookie;
  detail::readBuf<uint16_t>(cookie.cookie, cs);
  ret = std::move(cookie);
  return Status::Success;
}

template <>
inline Status
getExtension(SupportedVersions& ret, Error& err, folly::io::Cursor& cs) {
  SupportedVersions versions;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint8_t>(len, err, versions.versions, cs));
  ret = std::move(versions);
  return Status::Success;
}

template <>
inline Status
getExtension(ServerSupportedVersions& ret, Error& err, folly::io::Cursor& cs) {
  ServerSupportedVersions versions;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, versions.selected_version, cs));
  ret = std::move(versions);
  return Status::Success;
}

template <>
inline Status
getExtension(PskKeyExchangeModes& ret, Error& err, folly::io::Cursor& cs) {
  PskKeyExchangeModes modes;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::readVector<uint8_t>(len, err, modes.modes, cs));
  ret = std::move(modes);
  return Status::Success;
}

template <>
inline Status
getExtension(ProtocolNameList& ret, Error& err, folly::io::Cursor& cs) {
  ProtocolNameList names;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, names.protocol_name_list, cs));
  ret = std::move(names);
  return Status::Success;
}

template <>
inline Status
getExtension(ServerNameList& ret, Error& err, folly::io::Cursor& cs) {
  ServerNameList names;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, names.server_name_list, cs));
  ret = std::move(names);
  return Status::Success;
}

template <>
inline Status
getExtension(CertificateAuthorities& ret, Error& err, folly::io::Cursor& cs) {
  CertificateAuthorities authorities;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, authorities.authorities, cs));
  ret = std::move(authorities);
  return Status::Success;
}

template <>
inline Status getExtension(
    CertificateCompressionAlgorithms& ret,
    Error& err,
    folly::io::Cursor& cs) {
  CertificateCompressionAlgorithms cca;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint8_t>(len, err, cca.algorithms, cs));
  ret = std::move(cca);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const SignatureAlgorithms& sig) {
  Extension ext;
  ext.extension_type = ExtensionType::signature_algorithms;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(
          err, sig.supported_signature_algorithms, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const SupportedGroups& groups) {
  Extension ext;
  ext.extension_type = ExtensionType::supported_groups;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, groups.named_group_list, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const ClientKeyShare& share) {
  Extension ext;
  ext.extension_type = ExtensionType::key_share;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, share.client_shares, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const ServerKeyShare& share) {
  Extension ext;
  ext.extension_type = ExtensionType::key_share;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(detail::write(err, share.server_share, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status encodeExtension(
    Extension& ret,
    Error& err,
    const HelloRetryRequestKeyShare& share) {
  Extension ext;
  ext.extension_type = ExtensionType::key_share;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(detail::write(err, share.selected_group, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const ClientPresharedKey& share) {
  Extension ext;
  ext.extension_type = ExtensionType::pre_shared_key;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, share.identities, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, share.binders, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const ServerPresharedKey& share) {
  Extension ext;
  ext.extension_type = ExtensionType::pre_shared_key;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(detail::write(err, share.selected_identity, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& /* err */, const ClientEarlyData&) {
  Extension ext;
  ext.extension_type = ExtensionType::early_data;
  ext.extension_data = folly::IOBuf::create(0);
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& /* err */, const ServerEarlyData&) {
  Extension ext;
  ext.extension_type = ExtensionType::early_data;
  ext.extension_data = folly::IOBuf::create(0);
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const TicketEarlyData& early) {
  Extension ext;
  ext.extension_type = ExtensionType::early_data;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(detail::write(err, early.max_early_data_size, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const Cookie& cookie) {
  Extension ext;
  ext.extension_type = ExtensionType::cookie;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint16_t>(err, cookie.cookie, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const SupportedVersions& versions) {
  Extension ext;
  ext.extension_type = ExtensionType::supported_versions;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint8_t>(err, versions.versions, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status encodeExtension(
    Extension& ret,
    Error& err,
    const ServerSupportedVersions& versions) {
  Extension ext;
  ext.extension_type = ExtensionType::supported_versions;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(detail::write(err, versions.selected_version, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const PskKeyExchangeModes& modes) {
  Extension ext;
  ext.extension_type = ExtensionType::psk_key_exchange_modes;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint8_t>(err, modes.modes, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const ProtocolNameList& names) {
  Extension ext;
  ext.extension_type = ExtensionType::application_layer_protocol_negotiation;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, names.protocol_name_list, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status
encodeExtension(Extension& ret, Error& err, const ServerNameList& names) {
  Extension ext;
  ext.extension_type = ExtensionType::server_name;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, names.server_name_list, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status encodeExtension(
    Extension& ret,
    Error& err,
    const CertificateAuthorities& authorities) {
  Extension ext;
  ext.extension_type = ExtensionType::certificate_authorities;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, authorities.authorities, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status encodeExtension(
    Extension& ret,
    Error& err,
    const CertificateCompressionAlgorithms& cca) {
  Extension ext;
  ext.extension_type = ExtensionType::compress_certificate;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint8_t>(err, cca.algorithms, appender));
  ret = std::move(ext);
  return Status::Success;
}

template <>
inline Status encodeExtension(
    Extension& ret,
    Error& err,
    const EchOuterExtensions& outerExt) {
  Extension ext;
  ext.extension_type = outerExt.extension_type;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint8_t>(err, outerExt.extensionTypes, appender));
  ret = std::move(ext);
  return Status::Success;
}

inline Status
getBinderLength(size_t& ret, Error& err, const ClientHello& chlo) {
  if (chlo.extensions.empty() ||
      chlo.extensions.back().extension_type != ExtensionType::pre_shared_key) {
    return err.error(
        "psk not at end of client hello", AlertDescription::decode_error);
  }
  folly::io::Cursor cursor(chlo.extensions.back().extension_data.get());
  uint16_t identitiesLen;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, identitiesLen, cursor));
  cursor.skip(identitiesLen);
  uint16_t binderLen;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, binderLen, cursor));
  if (cursor.totalLength() != binderLen) {
    return err.error("malformed binder length", AlertDescription::decode_error);
  }
  ret = sizeof(binderLen) + binderLen;
  return Status::Success;
}

namespace detail {

template <>
struct Reader<KeyShareEntry> {
  template <class T>
  Status
  read(size_t& ret, Error& err, KeyShareEntry& out, folly::io::Cursor& cursor) {
    size_t len = 0;
    size_t lenRead;
    FIZZ_RETURN_ON_ERROR(detail::read(lenRead, err, out.group, cursor));
    len += lenRead;
    len += readBuf<uint16_t>(out.key_exchange, cursor);
    ret = len;
    return Status::Success;
  }
};

template <>
struct Writer<KeyShareEntry> {
  template <class T>
  Status
  write(Error& err, const KeyShareEntry& share, folly::io::Appender& out) {
    FIZZ_RETURN_ON_ERROR(detail::write(err, share.group, out));
    FIZZ_RETURN_ON_ERROR(
        detail::writeBuf<uint16_t>(err, share.key_exchange, out));
    return Status::Success;
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
  Status
  read(size_t& ret, Error& err, PskIdentity& out, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += readBuf<uint16_t>(out.psk_identity, cursor);
    size_t lenRead;
    FIZZ_RETURN_ON_ERROR(
        detail::read(lenRead, err, out.obfuscated_ticket_age, cursor));
    len += lenRead;
    ret = len;
    return Status::Success;
  }
};

template <>
struct Writer<PskIdentity> {
  template <class T>
  Status write(Error& err, const PskIdentity& ident, folly::io::Appender& out) {
    FIZZ_RETURN_ON_ERROR(writeBuf<uint16_t>(err, ident.psk_identity, out));
    FIZZ_RETURN_ON_ERROR(detail::write(err, ident.obfuscated_ticket_age, out));
    return Status::Success;
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
  Status read(
      size_t& ret,
      Error& /* err */,
      PskBinder& out,
      folly::io::Cursor& cursor) {
    ret = readBuf<uint8_t>(out.binder, cursor);
    return Status::Success;
  }
};

template <>
struct Writer<PskBinder> {
  template <class T>
  Status write(Error& err, const PskBinder& binder, folly::io::Appender& out) {
    FIZZ_RETURN_ON_ERROR(writeBuf<uint8_t>(err, binder.binder, out));
    return Status::Success;
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
  Status read(
      size_t& ret,
      Error& /* err */,
      ProtocolName& name,
      folly::io::Cursor& cursor) {
    ret = readBuf<uint8_t>(name.name, cursor);
    return Status::Success;
  }
};

template <>
struct Writer<ProtocolName> {
  template <class T>
  Status write(Error& err, const ProtocolName& name, folly::io::Appender& out) {
    FIZZ_RETURN_ON_ERROR(writeBuf<uint8_t>(err, name.name, out));
    return Status::Success;
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
  Status
  read(size_t& ret, Error& err, ServerName& name, folly::io::Cursor& cursor) {
    size_t size = 0;
    size_t lenRead;
    FIZZ_RETURN_ON_ERROR(detail::read(lenRead, err, name.name_type, cursor));
    size += lenRead;
    size += readBuf<uint16_t>(name.hostname, cursor);
    ret = size;
    return Status::Success;
  }
};

template <>
struct Writer<ServerName> {
  template <class T>
  Status write(Error& err, const ServerName& name, folly::io::Appender& out) {
    FIZZ_RETURN_ON_ERROR(detail::write(err, name.name_type, out));
    FIZZ_RETURN_ON_ERROR(writeBuf<uint16_t>(err, name.hostname, out));
    return Status::Success;
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
  Status read(
      size_t& ret,
      Error& /* err */,
      DistinguishedName& dn,
      folly::io::Cursor& cursor) {
    ret = readBuf<uint16_t>(dn.encoded_name, cursor);
    return Status::Success;
  }
};

template <>
struct Writer<DistinguishedName> {
  template <class T>
  Status
  write(Error& err, const DistinguishedName& dn, folly::io::Appender& out) {
    FIZZ_RETURN_ON_ERROR(writeBuf<uint16_t>(err, dn.encoded_name, out));
    return Status::Success;
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
