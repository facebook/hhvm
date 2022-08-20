/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/io/Cursor.h>

namespace fizz {
namespace detail {

// Structure representing a 24 bit type.
struct bits24 {
  static constexpr size_t size = 3;
};

template <class U>
struct Sizer {
  template <class T>
  size_t getSize(const typename std::enable_if<
                 (std::is_enum<T>::value || std::is_unsigned<T>::value) &&
                     std::is_same<U, T>::value,
                 T>::type&) {
    return sizeof(T);
  }
};

template <class N>
size_t getBufSize(const Buf& buf) {
  return sizeof(N) + buf->computeChainDataLength();
}

template <>
inline size_t getBufSize<bits24>(const Buf& buf) {
  return bits24::size + buf->computeChainDataLength();
}

template <class T>
size_t getSize(const T& t) {
  return Sizer<T>().template getSize<T>(t);
}

template <>
struct Sizer<Extension> {
  template <class T>
  size_t getSize(const Extension& proto) {
    return sizeof(ExtensionType) + getBufSize<uint16_t>(proto.extension_data);
  }
};

template <>
struct Sizer<CertificateEntry> {
  template <class T>
  size_t getSize(const CertificateEntry& entry) {
    size_t len = 0;
    len += getBufSize<bits24>(entry.cert_data);
    len += sizeof(uint16_t);
    for (const auto& ext : entry.extensions) {
      len += detail::getSize(ext);
    }
    return len;
  }
};

template <class U>
struct Writer {
  template <class T>
  void write(
      const typename std::enable_if<
          std::is_enum<T>::value && std::is_same<U, T>::value,
          T>::type& in,
      folly::io::Appender& appender) {
    using UT = typename std::underlying_type<U>::type;
    static_assert(
        std::is_unsigned<UT>::value,
        "enums meant to be serialized should be unsigned");
    appender.writeBE<UT>(static_cast<UT>(in));
  }

  template <class T>
  void write(
      const typename std::enable_if<
          !std::is_enum<T>::value && std::is_unsigned<T>::value &&
              std::is_same<U, T>::value,
          T>::type& in,
      folly::io::Appender& appender) {
    appender.writeBE<U>(in);
  }
};

template <class T>
void checkWithin24bits(const typename std::enable_if<
                       std::is_integral<T>::value && !std::is_signed<T>::value,
                       T>::type& value) {
  const uint32_t UINT24_MAX = 0xFFFFFF;
  if (value > UINT24_MAX) {
    throw std::runtime_error("Overflow 24 bit type");
  }
}

template <class T>
void writeBits24(T len, folly::io::Appender& out) {
  static_assert(sizeof(T) > 3, "Type is too short");
  checkWithin24bits<T>(len);
  T lenBE = folly::Endian::big(len);
  uint8_t* addr = reinterpret_cast<uint8_t*>(&lenBE);
  uint8_t offset = sizeof(T) - 3;
  out.push(addr + offset, bits24::size);
}

template <class T>
void write(const T& in, folly::io::Appender& appender) {
  Writer<T>().template write<T>(in, appender);
}

template <class N, class T>
struct WriterVector {
  void writeVector(const std::vector<T>& data, folly::io::Appender& out) {
    // First do a pass to compute the size of the data
    size_t len = 0;
    for (const auto& t : data) {
      len += getSize<T>(t);
    }

    out.writeBE<N>(folly::to<N>(len));
    for (const auto& t : data) {
      write(t, out);
    }
  }
};

template <class T>
struct WriterVector<bits24, T> {
  void writeVector(const std::vector<T>& data, folly::io::Appender& out) {
    // First do a pass to compute the size of the data
    size_t len = 0;
    for (const auto& t : data) {
      len += getSize<T>(t);
    }

    writeBits24(len, out);
    for (const auto& t : data) {
      write(t, out);
    }
  }
};

template <class N, class T>
void writeVector(const std::vector<T>& data, folly::io::Appender& out) {
  return WriterVector<N, T>().writeVector(data, out);
}

inline void writeBufWithoutLength(const Buf& buf, folly::io::Appender& out) {
  auto current = buf.get();
  size_t chainElements = buf->countChainElements();
  for (size_t i = 0; i < chainElements; ++i) {
    // TODO: fina a better way not to require copying all the buffers into
    // the cursor
    out.push(current->data(), current->length());
    current = current->next();
  }
}

template <class N>
void writeBuf(const Buf& buf, folly::io::Appender& out) {
  if (!buf) {
    out.writeBE<N>(folly::to<N>(0));
    return;
  }
  out.writeBE<N>(folly::to<N>(buf->computeChainDataLength()));
  writeBufWithoutLength(buf, out);
}

template <>
struct Writer<Random> {
  template <class T>
  void write(const Random& random, folly::io::Appender& out) {
    out.push(random.data(), random.size());
  }
};

template <>
inline void writeBuf<bits24>(const Buf& buf, folly::io::Appender& out) {
  if (!buf) {
    writeBits24(static_cast<size_t>(0), out);
    return;
  }
  writeBits24(buf->computeChainDataLength(), out);
  writeBufWithoutLength(buf, out);
}

template <>
inline void write<Extension>(
    const Extension& extension,
    folly::io::Appender& out) {
  out.writeBE(static_cast<typename std::underlying_type<ExtensionType>::type>(
      extension.extension_type));
  writeBuf<uint16_t>(extension.extension_data, out);
}

template <>
inline void write<CertificateEntry>(
    const CertificateEntry& entry,
    folly::io::Appender& out) {
  writeBuf<detail::bits24>(entry.cert_data, out);
  writeVector<uint16_t>(entry.extensions, out);
}

inline uint32_t readBits24(folly::io::Cursor& cursor) {
  uint32_t data = 0;
  uint8_t offset = sizeof(data) - 3;
  uint8_t* addr = reinterpret_cast<uint8_t*>(&data);
  cursor.pull(addr + offset, 3);
  uint32_t length = folly::Endian::big(data);
  return length;
}

template <class N>
size_t readBuf(Buf& buf, folly::io::Cursor& cursor) {
  auto len = cursor.readBE<N>();
  cursor.clone(buf, len);
  return sizeof(N) + len;
}

template <>
inline size_t readBuf<bits24>(Buf& buf, folly::io::Cursor& cursor) {
  auto len = readBits24(cursor);
  cursor.clone(buf, len);
  return bits24::size + len;
}

template <class U>
struct Reader {
  template <class T>
  size_t read(
      typename std::enable_if<
          std::is_enum<T>::value && std::is_same<U, T>::value,
          T>::type& out,
      folly::io::Cursor& cursor) {
    using UT = typename std::underlying_type<U>::type;
    static_assert(
        std::is_unsigned<UT>::value,
        "enums meant to be deserialized should be unsigned");
    out = static_cast<U>(cursor.readBE<UT>());
    return sizeof(U);
  }

  template <class T>
  size_t read(
      typename std::enable_if<
          !std::is_enum<T>::value && std::is_unsigned<T>::value &&
              std::is_same<U, T>::value,
          T>::type& out,
      folly::io::Cursor& cursor) {
    out = cursor.readBE<U>();
    return sizeof(U);
  }
};

template <class T>
size_t read(T& out, folly::io::Cursor& cursor) {
  return Reader<T>().template read<T>(out, cursor);
}

template <class N, class T>
struct ReadVector {
  size_t readVector(std::vector<T>& out, folly::io::Cursor& cursor) {
    auto len = cursor.readBE<N>();
    if (cursor.totalLength() < len) {
      throw std::out_of_range("Not enough data");
    }

    size_t consumed = 0;
    while (consumed < len) {
      out.push_back(T());
      consumed += read(*out.rbegin(), cursor);
    }
    if (consumed != len) {
      throw std::runtime_error("Invalid data length supplied");
    }
    return len + sizeof(N);
  }
};

template <class T>
struct ReadVector<bits24, T> {
  size_t readVector(std::vector<T>& out, folly::io::Cursor& cursor) {
    auto len = readBits24(cursor);
    if (cursor.totalLength() < len) {
      throw std::out_of_range("Not enough data");
    }

    size_t consumed = 0;
    while (consumed < len) {
      out.push_back(T());
      consumed += read(*out.rbegin(), cursor);
    }
    if (consumed != len) {
      throw std::runtime_error("Invalid data length supplied");
    }
    return len + bits24::size;
  }
};

template <class N, class T>
size_t readVector(std::vector<T>& out, folly::io::Cursor& cursor) {
  return ReadVector<N, T>().readVector(out, cursor);
}

template <>
struct Reader<Random> {
  template <class T>
  size_t read(Random& out, folly::io::Cursor& cursor) {
    cursor.pull(out.data(), out.size());
    return out.size();
  }
};

template <>
struct Reader<Extension> {
  template <class T>
  size_t read(Extension& extension, folly::io::Cursor& cursor) {
    extension.extension_type = static_cast<ExtensionType>(
        cursor.readBE<typename std::underlying_type<ExtensionType>::type>());
    auto len = readBuf<uint16_t>(extension.extension_data, cursor);
    return sizeof(ExtensionType) + len;
  }
};

template <>
struct Reader<CertificateEntry> {
  template <class T>
  size_t read(CertificateEntry& entry, folly::io::Cursor& cursor) {
    auto len = readBuf<bits24>(entry.cert_data, cursor);
    len += readVector<uint16_t>(entry.extensions, cursor);
    return len;
  }
};
} // namespace detail

template <>
inline Buf encode<ServerHello>(ServerHello&& shlo) {
  auto buf = folly::IOBuf::create(
      sizeof(ProtocolVersion) + sizeof(Random) + sizeof(CipherSuite) + 20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(shlo.legacy_version, appender);
  detail::write(shlo.random, appender);
  if (shlo.legacy_session_id_echo) {
    detail::writeBuf<uint8_t>(shlo.legacy_session_id_echo, appender);
  }
  detail::write(shlo.cipher_suite, appender);
  if (shlo.legacy_session_id_echo) {
    detail::write(shlo.legacy_compression_method, appender);
  }
  detail::writeVector<uint16_t>(shlo.extensions, appender);
  return buf;
}

template <>
inline Buf encode<HelloRetryRequest>(HelloRetryRequest&& shlo) {
  auto buf = folly::IOBuf::create(
      sizeof(ProtocolVersion) + sizeof(Random) + sizeof(CipherSuite) + 20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(shlo.legacy_version, appender);
  detail::write(HelloRetryRequest::HrrRandom, appender);
  detail::writeBuf<uint8_t>(shlo.legacy_session_id_echo, appender);
  detail::write(shlo.cipher_suite, appender);
  detail::write(shlo.legacy_compression_method, appender);
  detail::writeVector<uint16_t>(shlo.extensions, appender);
  return buf;
}

template <>
inline Buf encode<EndOfEarlyData>(EndOfEarlyData&&) {
  return folly::IOBuf::create(0);
}

template <>
inline Buf encode<EncryptedExtensions>(EncryptedExtensions&& extensions) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::writeVector<uint16_t>(extensions.extensions, appender);
  return buf;
}

template <>
inline Buf encode<CertificateRequest>(CertificateRequest&& cr) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::writeBuf<uint8_t>(cr.certificate_request_context, appender);
  detail::writeVector<uint16_t>(cr.extensions, appender);
  return buf;
}

template <>
inline Buf encode<const CertificateMsg&>(const CertificateMsg& cert) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::writeBuf<uint8_t>(cert.certificate_request_context, appender);
  detail::writeVector<detail::bits24>(cert.certificate_list, appender);
  return buf;
}

template <>
inline Buf encode<CertificateMsg&>(CertificateMsg& cert) {
  return encode<const CertificateMsg&>(cert);
}

template <>
inline Buf encode<CertificateMsg>(CertificateMsg&& cert) {
  return encode<CertificateMsg&>(cert);
}

template <>
inline Buf encode<CompressedCertificate&>(CompressedCertificate& cc) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(cc.algorithm, appender);
  detail::writeBits24(cc.uncompressed_length, appender);
  detail::writeBuf<detail::bits24>(cc.compressed_certificate_message, appender);
  return buf;
}

template <>
inline Buf encode<CompressedCertificate>(CompressedCertificate&& cc) {
  return encode<CompressedCertificate&>(cc);
}

template <>
inline Buf encode<CertificateVerify>(CertificateVerify&& certVerify) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(certVerify.algorithm, appender);
  detail::writeBuf<uint16_t>(certVerify.signature, appender);
  return buf;
}

template <>
inline Buf encode<Alert>(Alert&& alert) {
  auto buf = folly::IOBuf::create(2);
  folly::io::Appender appender(buf.get(), 2);
  detail::write(alert.level, appender);
  detail::write(alert.description, appender);
  return buf;
}

template <>
inline Buf encode<const ClientHello&>(const ClientHello& chlo) {
  auto buf = folly::IOBuf::create(
      sizeof(ProtocolVersion) + sizeof(Random) + sizeof(uint8_t) +
      sizeof(CipherSuite) * chlo.cipher_suites.size() + sizeof(uint8_t) + 20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(chlo.legacy_version, appender);
  detail::write(chlo.random, appender);
  detail::writeBuf<uint8_t>(chlo.legacy_session_id, appender);
  detail::writeVector<uint16_t>(chlo.cipher_suites, appender);
  detail::writeVector<uint8_t>(chlo.legacy_compression_methods, appender);
  detail::writeVector<uint16_t>(chlo.extensions, appender);
  return buf;
}

template <>
inline Buf encode<ClientHello&>(ClientHello& chlo) {
  return encode<const ClientHello&>(chlo);
}

template <>
inline Buf encode<ClientHello>(ClientHello&& chlo) {
  return encode<ClientHello&>(chlo);
}

template <>
inline Buf encode<Finished>(Finished&& fin) {
  return std::move(fin.verify_data);
}

template <>
inline Buf encode<NewSessionTicket>(NewSessionTicket&& nst) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(nst.ticket_lifetime, appender);
  detail::write(nst.ticket_age_add, appender);
  if (nst.ticket_nonce) {
    detail::writeBuf<uint8_t>(nst.ticket_nonce, appender);
  }
  detail::writeBuf<uint16_t>(nst.ticket, appender);
  detail::writeVector<uint16_t>(nst.extensions, appender);
  return buf;
}

inline Buf encodeHkdfLabel(
    HkdfLabel&& label,
    const std::string& hkdfLabelPrefix) {
  auto labelBuf = folly::IOBuf::copyBuffer(
      folly::to<std::string>(hkdfLabelPrefix, label.label));
  auto buf = folly::IOBuf::create(sizeof(label.length) + label.label.size());
  folly::io::Appender appender(buf.get(), 20);
  detail::write(label.length, appender);
  detail::writeBuf<uint8_t>(labelBuf, appender);
  detail::writeBuf<uint8_t>(label.hash_value, appender);
  return buf;
}

template <>
inline Buf encode<KeyUpdate>(KeyUpdate&& keyUpdate) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::write(keyUpdate.request_update, appender);
  return buf;
}

template <>
inline Buf encode<message_hash>(message_hash&& hash) {
  return std::move(hash.hash);
}

template <class T>
Buf encodeHandshake(T&& handshakeMsg) {
  auto body = encode(std::forward<T>(handshakeMsg));
  auto buf = folly::IOBuf::create(sizeof(HandshakeType) + detail::bits24::size);
  folly::io::Appender appender(buf.get(), 0);
  constexpr auto handshakeType = std::remove_reference<T>::type::handshake_type;
  detail::write(handshakeType, appender);
  detail::writeBits24(body->computeChainDataLength(), appender);
  buf->prependChain(std::move(body));
  return buf;
}

template <>
inline ClientHello decode(folly::io::Cursor& cursor) {
  ClientHello chlo;
  detail::read(chlo.legacy_version, cursor);
  detail::read(chlo.random, cursor);
  detail::readBuf<uint8_t>(chlo.legacy_session_id, cursor);
  detail::readVector<uint16_t>(chlo.cipher_suites, cursor);
  detail::readVector<uint8_t>(chlo.legacy_compression_methods, cursor);
  // Before TLS 1.3 clients could omit the extensions section entirely. If we're
  // already at the end of the client hello we won't try and read extensions so
  // that this isn't treated as a parse error.
  if (!cursor.isAtEnd()) {
    detail::readVector<uint16_t>(chlo.extensions, cursor);
  }
  return chlo;
}

template <>
inline ServerHello decode(folly::io::Cursor& cursor) {
  ServerHello shlo;
  detail::read(shlo.legacy_version, cursor);
  detail::read(shlo.random, cursor);
  detail::readBuf<uint8_t>(shlo.legacy_session_id_echo, cursor);
  detail::read(shlo.cipher_suite, cursor);
  detail::read(shlo.legacy_compression_method, cursor);
  detail::readVector<uint16_t>(shlo.extensions, cursor);
  return shlo;
}

template <>
inline EndOfEarlyData decode(folly::io::Cursor&) {
  return EndOfEarlyData();
}

template <>
inline EncryptedExtensions decode(folly::io::Cursor& cursor) {
  EncryptedExtensions ee;
  detail::readVector<uint16_t>(ee.extensions, cursor);
  return ee;
}

template <>
inline CertificateRequest decode(folly::io::Cursor& cursor) {
  CertificateRequest cr;
  detail::readBuf<uint8_t>(cr.certificate_request_context, cursor);
  detail::readVector<uint16_t>(cr.extensions, cursor);
  return cr;
}

template <>
inline CertificateMsg decode(folly::io::Cursor& cursor) {
  CertificateMsg cert;
  detail::readBuf<uint8_t>(cert.certificate_request_context, cursor);
  detail::readVector<detail::bits24>(cert.certificate_list, cursor);
  return cert;
}

template <>
inline CompressedCertificate decode(folly::io::Cursor& cursor) {
  CompressedCertificate cc;
  detail::read(cc.algorithm, cursor);
  cc.uncompressed_length = detail::readBits24(cursor);
  detail::readBuf<detail::bits24>(cc.compressed_certificate_message, cursor);
  return cc;
}

template <>
inline CertificateVerify decode(folly::io::Cursor& cursor) {
  CertificateVerify certVerify;
  detail::read(certVerify.algorithm, cursor);
  detail::readBuf<uint16_t>(certVerify.signature, cursor);
  return certVerify;
}

template <>
inline NewSessionTicket decode<NewSessionTicket>(folly::io::Cursor& cursor) {
  NewSessionTicket nst;
  detail::read(nst.ticket_lifetime, cursor);
  detail::read(nst.ticket_age_add, cursor);
  detail::readBuf<uint8_t>(nst.ticket_nonce, cursor);
  detail::readBuf<uint16_t>(nst.ticket, cursor);
  detail::readVector<uint16_t>(nst.extensions, cursor);
  return nst;
}

template <>
inline Alert decode(folly::io::Cursor& cursor) {
  Alert alert;
  detail::read(alert.level, cursor);
  detail::read(alert.description, cursor);
  return alert;
}

template <>
inline Finished decode<Finished>(std::unique_ptr<folly::IOBuf>&& buf) {
  Finished fin;
  fin.verify_data = std::move(buf);
  return fin;
}

template <>
inline KeyUpdate decode<KeyUpdate>(folly::io::Cursor& cursor) {
  KeyUpdate update;
  detail::read(update.request_update, cursor);
  return update;
}

template <class T>
T decode(std::unique_ptr<folly::IOBuf>&& buf) {
  folly::io::Cursor cursor(buf.get());
  auto decoded = decode<T>(cursor);

  if (!cursor.isAtEnd()) {
    throw std::runtime_error("didn't read entire message");
  }

  return decoded;
}

template <typename T>
std::string enumToHex(T enumValue) {
  auto value = folly::Endian::big(
      static_cast<typename std::underlying_type<T>::type>(enumValue));
  return folly::hexlify(folly::ByteRange(
      reinterpret_cast<const uint8_t*>(&value), sizeof(value)));
}
} // namespace fizz
