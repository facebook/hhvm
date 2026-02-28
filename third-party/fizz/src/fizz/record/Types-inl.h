/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/util/Status.h>
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
  size_t getSize(
      const typename std::enable_if<
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

template <class N>
size_t getStringSize(const std::string& str) {
  return sizeof(N) + str.size();
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
  Status write(
      Error& /* err */,
      const typename std::enable_if<
          std::is_enum<T>::value && std::is_same<U, T>::value,
          T>::type& in,
      folly::io::Appender& appender) {
    using UT = typename std::underlying_type<U>::type;
    static_assert(
        std::is_unsigned<UT>::value,
        "enums meant to be serialized should be unsigned");
    appender.writeBE<UT>(static_cast<UT>(in));
    return Status::Success;
  }

  template <class T>
  Status write(
      Error& /* err */,
      const typename std::enable_if<
          !std::is_enum<T>::value && std::is_unsigned<T>::value &&
              std::is_same<U, T>::value,
          T>::type& in,
      folly::io::Appender& appender) {
    appender.writeBE<U>(in);
    return Status::Success;
  }
};

template <class T>
Status checkWithin24bits(
    Error& err,
    const typename std::enable_if<
        std::is_integral<T>::value && !std::is_signed<T>::value,
        T>::type& value) {
  const uint32_t UINT24_MAX = 0xFFFFFF;
  if (value > UINT24_MAX) {
    return err.error("Overflow 24 bit type");
  }
  return Status::Success;
}

template <class T>
Status writeBits24(Error& err, T len, folly::io::Appender& out) {
  static_assert(sizeof(T) > 3, "Type is too short");
  FIZZ_RETURN_ON_ERROR(checkWithin24bits<T>(err, len));
  T lenBE = folly::Endian::big(len);
  uint8_t* addr = reinterpret_cast<uint8_t*>(&lenBE);
  uint8_t offset = sizeof(T) - 3;
  out.push(addr + offset, bits24::size);
  return Status::Success;
}

template <class T>
Status write(Error& err, const T& in, folly::io::Appender& appender) {
  return Writer<T>().template write<T>(err, in, appender);
}

template <class N, class T>
struct WriterVector {
  Status writeVector(
      Error& err,
      const std::vector<T>& data,
      folly::io::Appender& out) {
    // First do a pass to compute the size of the data
    size_t len = 0;
    for (const auto& t : data) {
      len += getSize<T>(t);
    }

    out.writeBE<N>(folly::to<N>(len));
    for (const auto& t : data) {
      FIZZ_RETURN_ON_ERROR(write(err, t, out));
    }
    return Status::Success;
  }
};

template <class T>
struct WriterVector<bits24, T> {
  Status writeVector(
      Error& err,
      const std::vector<T>& data,
      folly::io::Appender& out) {
    // First do a pass to compute the size of the data
    size_t len = 0;
    for (const auto& t : data) {
      len += getSize<T>(t);
    }

    FIZZ_RETURN_ON_ERROR(writeBits24(err, len, out));
    for (const auto& t : data) {
      FIZZ_RETURN_ON_ERROR(write(err, t, out));
    }
    return Status::Success;
  }
};

template <class N, class T>
Status
writeVector(Error& err, const std::vector<T>& data, folly::io::Appender& out) {
  return WriterVector<N, T>().writeVector(err, data, out);
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
Status writeBuf(Error& /* err */, const Buf& buf, folly::io::Appender& out) {
  if (!buf) {
    out.writeBE<N>(folly::to<N>(0));
    return Status::Success;
  }
  out.writeBE<N>(folly::to<N>(buf->computeChainDataLength()));
  writeBufWithoutLength(buf, out);
  return Status::Success;
}

template <class N>
void writeString(const std::string& str, folly::io::Appender& out) {
  out.writeBE<N>(folly::to<N>(str.size()));
  out.push(folly::ByteRange(str));
}

template <>
struct Writer<Random> {
  template <class T>
  Status
  write(Error& /* err */, const Random& random, folly::io::Appender& out) {
    out.push(random.data(), random.size());
    return Status::Success;
  }
};

template <>
inline Status
writeBuf<bits24>(Error& err, const Buf& buf, folly::io::Appender& out) {
  if (!buf) {
    FIZZ_RETURN_ON_ERROR(writeBits24(err, static_cast<size_t>(0), out));
    return Status::Success;
  }
  FIZZ_RETURN_ON_ERROR(writeBits24(err, buf->computeChainDataLength(), out));
  writeBufWithoutLength(buf, out);
  return Status::Success;
}

template <>
inline Status write<Extension>(
    Error& err,
    const Extension& extension,
    folly::io::Appender& out) {
  out.writeBE(
      static_cast<typename std::underlying_type<ExtensionType>::type>(
          extension.extension_type));
  FIZZ_RETURN_ON_ERROR(writeBuf<uint16_t>(err, extension.extension_data, out));
  return Status::Success;
}

template <>
inline Status write<CertificateEntry>(
    Error& err,
    const CertificateEntry& entry,
    folly::io::Appender& out) {
  FIZZ_RETURN_ON_ERROR(writeBuf<detail::bits24>(err, entry.cert_data, out));
  FIZZ_RETURN_ON_ERROR(writeVector<uint16_t>(err, entry.extensions, out));
  return Status::Success;
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

template <class N>
size_t readString(std::string& str, folly::io::Cursor& cursor) {
  auto len = cursor.readBE<N>();
  str = cursor.readFixedString(len);
  return sizeof(N) + len;
}

template <class U>
struct Reader {
  template <class T>
  Status read(
      size_t& ret,
      Error& /* err */,
      typename std::enable_if<
          std::is_enum<T>::value && std::is_same<U, T>::value,
          T>::type& out,
      folly::io::Cursor& cursor) {
    using UT = typename std::underlying_type<U>::type;
    static_assert(
        std::is_unsigned<UT>::value,
        "enums meant to be deserialized should be unsigned");
    out = static_cast<U>(cursor.readBE<UT>());
    ret = sizeof(U);
    return Status::Success;
  }

  template <class T>
  Status read(
      size_t& ret,
      Error& /* err */,
      typename std::enable_if<
          !std::is_enum<T>::value && std::is_unsigned<T>::value &&
              std::is_same<U, T>::value,
          T>::type& out,
      folly::io::Cursor& cursor) {
    out = cursor.readBE<U>();
    ret = sizeof(U);
    return Status::Success;
  }
};

template <class T>
Status read(size_t& ret, Error& err, T& out, folly::io::Cursor& cursor) {
  return Reader<T>().template read<T>(ret, err, out, cursor);
}

template <class N, class T>
struct ReadVector {
  Status readVector(
      size_t& ret,
      Error& err,
      std::vector<T>& out,
      folly::io::Cursor& cursor) {
    auto len = cursor.readBE<N>();
    if (cursor.totalLength() < len) {
      return err.error(
          "Not enough data", folly::none, Error::Category::StdOutOfRange);
    }

    size_t consumed = 0;
    size_t lenRead;
    while (consumed < len) {
      out.push_back(T());
      FIZZ_RETURN_ON_ERROR(read(lenRead, err, *out.rbegin(), cursor));
      consumed += lenRead;
    }
    if (consumed != len) {
      return err.error("Invalid data length supplied");
    }
    ret = len + sizeof(N);
    return Status::Success;
  }
};

template <class T>
struct ReadVector<bits24, T> {
  Status readVector(
      size_t& ret,
      Error& err,
      std::vector<T>& out,
      folly::io::Cursor& cursor) {
    auto len = readBits24(cursor);
    if (cursor.totalLength() < len) {
      return err.error(
          "Not enough data", folly::none, Error::Category::StdOutOfRange);
    }

    size_t consumed = 0;
    size_t lenRead;
    while (consumed < len) {
      out.push_back(T());
      FIZZ_RETURN_ON_ERROR(read(lenRead, err, *out.rbegin(), cursor));
      consumed += lenRead;
    }
    if (consumed != len) {
      return err.error("Invalid data length supplied");
    }
    ret = len + bits24::size;
    return Status::Success;
  }
};

template <class N, class T>
Status readVector(
    size_t& ret,
    Error& err,
    std::vector<T>& out,
    folly::io::Cursor& cursor) {
  return ReadVector<N, T>().readVector(ret, err, out, cursor);
}

template <>
struct Reader<Random> {
  template <class T>
  Status
  read(size_t& ret, Error& /* err */, Random& out, folly::io::Cursor& cursor) {
    cursor.pull(out.data(), out.size());
    ret = out.size();
    return Status::Success;
  }
};

template <>
struct Reader<Extension> {
  template <class T>
  Status read(
      size_t& ret,
      Error& /* err */,
      Extension& extension,
      folly::io::Cursor& cursor) {
    extension.extension_type = static_cast<ExtensionType>(
        cursor.readBE<typename std::underlying_type<ExtensionType>::type>());
    auto len = readBuf<uint16_t>(extension.extension_data, cursor);
    ret = sizeof(ExtensionType) + len;
    return Status::Success;
  }
};

template <>
struct Reader<CertificateEntry> {
  template <class T>
  Status read(
      size_t& ret,
      Error& err,
      CertificateEntry& entry,
      folly::io::Cursor& cursor) {
    auto len = readBuf<bits24>(entry.cert_data, cursor);
    size_t lenVec;
    FIZZ_RETURN_ON_ERROR(
        readVector<uint16_t>(lenVec, err, entry.extensions, cursor));
    ret = len + lenVec;
    return Status::Success;
  }
};
} // namespace detail

template <>
inline Status encode<ServerHello>(Buf& ret, Error& err, ServerHello&& shlo) {
  auto buf = folly::IOBuf::create(
      sizeof(ProtocolVersion) + sizeof(Random) + sizeof(CipherSuite) + 20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, shlo.legacy_version, appender));
  FIZZ_RETURN_ON_ERROR(detail::write(err, shlo.random, appender));
  if (shlo.legacy_session_id_echo) {
    FIZZ_RETURN_ON_ERROR(
        detail::writeBuf<uint8_t>(err, shlo.legacy_session_id_echo, appender));
  }
  FIZZ_RETURN_ON_ERROR(detail::write(err, shlo.cipher_suite, appender));
  if (shlo.legacy_session_id_echo) {
    FIZZ_RETURN_ON_ERROR(
        detail::write(err, shlo.legacy_compression_method, appender));
  }
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, shlo.extensions, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status
encode<HelloRetryRequest>(Buf& ret, Error& err, HelloRetryRequest&& shlo) {
  auto buf = folly::IOBuf::create(
      sizeof(ProtocolVersion) + sizeof(Random) + sizeof(CipherSuite) + 20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, shlo.legacy_version, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::write(err, HelloRetryRequest::HrrRandom, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint8_t>(err, shlo.legacy_session_id_echo, appender));
  FIZZ_RETURN_ON_ERROR(detail::write(err, shlo.cipher_suite, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::write(err, shlo.legacy_compression_method, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, shlo.extensions, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status
encode<EndOfEarlyData>(Buf& ret, Error& /* err */, EndOfEarlyData&&) {
  ret = folly::IOBuf::create(0);
  return Status::Success;
}

template <>
inline Status encode<EncryptedExtensions>(
    Buf& ret,
    Error& err,
    EncryptedExtensions&& extensions) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, extensions.extensions, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status
encode<CertificateRequest>(Buf& ret, Error& err, CertificateRequest&& cr) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint8_t>(err, cr.certificate_request_context, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, cr.extensions, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status encode<const CertificateMsg&>(
    Buf& ret,
    Error& err,
    const CertificateMsg& cert) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint8_t>(
          err, cert.certificate_request_context, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<detail::bits24>(
          err, cert.certificate_list, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status
encode<CertificateMsg&>(Buf& ret, Error& err, CertificateMsg& cert) {
  return encode<const CertificateMsg&>(ret, err, cert);
}

template <>
inline Status
encode<CertificateMsg>(Buf& ret, Error& err, CertificateMsg&& cert) {
  return encode<CertificateMsg&>(ret, err, cert);
}

template <>
inline Status encode<CompressedCertificate&>(
    Buf& ret,
    Error& err,
    CompressedCertificate& cc) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, cc.algorithm, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBits24(err, cc.uncompressed_length, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<detail::bits24>(
          err, cc.compressed_certificate_message, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status encode<CompressedCertificate>(
    Buf& ret,
    Error& err,
    CompressedCertificate&& cc) {
  return encode<CompressedCertificate&>(ret, err, cc);
}

template <>
inline Status encode<CertificateVerify>(
    Buf& ret,
    Error& err,
    CertificateVerify&& certVerify) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, certVerify.algorithm, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint16_t>(err, certVerify.signature, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status encode<Alert>(Buf& ret, Error& err, Alert&& alert) {
  auto buf = folly::IOBuf::create(2);
  folly::io::Appender appender(buf.get(), 2);
  FIZZ_RETURN_ON_ERROR(detail::write(err, alert.level, appender));
  FIZZ_RETURN_ON_ERROR(detail::write(err, alert.description, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status
encode<const ClientHello&>(Buf& ret, Error& err, const ClientHello& chlo) {
  auto buf = folly::IOBuf::create(
      sizeof(ProtocolVersion) + sizeof(Random) + sizeof(uint8_t) +
      sizeof(CipherSuite) * chlo.cipher_suites.size() + sizeof(uint8_t) + 20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, chlo.legacy_version, appender));
  FIZZ_RETURN_ON_ERROR(detail::write(err, chlo.random, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint8_t>(err, chlo.legacy_session_id, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, chlo.cipher_suites, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint8_t>(
          err, chlo.legacy_compression_methods, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, chlo.extensions, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status encode<ClientHello&>(Buf& ret, Error& err, ClientHello& chlo) {
  return encode<const ClientHello&>(ret, err, chlo);
}

template <>
inline Status encode<ClientHello>(Buf& ret, Error& err, ClientHello&& chlo) {
  return encode<ClientHello&>(ret, err, chlo);
}

template <>
inline Status encode<Finished>(Buf& ret, Error& /* err */, Finished&& fin) {
  ret = std::move(fin.verify_data);
  return Status::Success;
}

template <>
inline Status
encode<NewSessionTicket>(Buf& ret, Error& err, NewSessionTicket&& nst) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, nst.ticket_lifetime, appender));
  FIZZ_RETURN_ON_ERROR(detail::write(err, nst.ticket_age_add, appender));
  if (nst.ticket_nonce) {
    FIZZ_RETURN_ON_ERROR(
        detail::writeBuf<uint8_t>(err, nst.ticket_nonce, appender));
  }
  FIZZ_RETURN_ON_ERROR(detail::writeBuf<uint16_t>(err, nst.ticket, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeVector<uint16_t>(err, nst.extensions, appender));
  ret = std::move(buf);
  return Status::Success;
}

inline Status encodeHkdfLabel(
    Buf& ret,
    Error& err,
    HkdfLabel&& label,
    folly::StringPiece hkdfLabelPrefix) {
  auto labelBuf = folly::IOBuf::copyBuffer(
      folly::to<std::string>(hkdfLabelPrefix, label.label));
  auto buf = folly::IOBuf::create(sizeof(label.length) + label.label.size());
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, label.length, appender));
  FIZZ_RETURN_ON_ERROR(detail::writeBuf<uint8_t>(err, labelBuf, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<uint8_t>(err, label.hash_value, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status encode<KeyUpdate>(Buf& ret, Error& err, KeyUpdate&& keyUpdate) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write(err, keyUpdate.request_update, appender));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status
encode<message_hash>(Buf& ret, Error& /* err */, message_hash&& hash) {
  ret = std::move(hash.hash);
  return Status::Success;
}

template <class T>
Status encodeHandshake(Buf& ret, Error& err, T&& handshakeMsg) {
  Buf body;
  FIZZ_RETURN_ON_ERROR(encode(body, err, std::forward<T>(handshakeMsg)));
  auto buf = folly::IOBuf::create(sizeof(HandshakeType) + detail::bits24::size);
  folly::io::Appender appender(buf.get(), 0);
  constexpr auto handshakeType = std::remove_reference<T>::type::handshake_type;
  FIZZ_RETURN_ON_ERROR(detail::write(err, handshakeType, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBits24(err, body->computeChainDataLength(), appender));
  buf->prependChain(std::move(body));
  ret = std::move(buf);
  return Status::Success;
}

template <>
inline Status decode(ClientHello& ret, Error& err, folly::io::Cursor& cursor) {
  ClientHello chlo;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, chlo.legacy_version, cursor));
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, chlo.random, cursor));
  detail::readBuf<uint8_t>(chlo.legacy_session_id, cursor);
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, chlo.cipher_suites, cursor));
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint8_t>(
          len, err, chlo.legacy_compression_methods, cursor));
  // Before TLS 1.3 clients could omit the extensions section entirely. If we're
  // already at the end of the client hello we won't try and read extensions so
  // that this isn't treated as a parse error.
  if (!cursor.isAtEnd()) {
    FIZZ_RETURN_ON_ERROR(
        detail::readVector<uint16_t>(len, err, chlo.extensions, cursor));
  }
  ret = std::move(chlo);
  return Status::Success;
}

template <>
inline Status decode(ServerHello& ret, Error& err, folly::io::Cursor& cursor) {
  ServerHello shlo;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, shlo.legacy_version, cursor));
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, shlo.random, cursor));
  detail::readBuf<uint8_t>(shlo.legacy_session_id_echo, cursor);
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, shlo.cipher_suite, cursor));
  FIZZ_RETURN_ON_ERROR(
      detail::read(len, err, shlo.legacy_compression_method, cursor));
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, shlo.extensions, cursor));
  ret = std::move(shlo);
  return Status::Success;
}

template <>
inline Status
decode(EndOfEarlyData& ret, Error& /* err */, folly::io::Cursor&) {
  ret = EndOfEarlyData();
  return Status::Success;
}

template <>
inline Status
decode(EncryptedExtensions& ret, Error& err, folly::io::Cursor& cursor) {
  EncryptedExtensions ee;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, ee.extensions, cursor));
  ret = std::move(ee);
  return Status::Success;
}

template <>
inline Status
decode(CertificateRequest& ret, Error& err, folly::io::Cursor& cursor) {
  CertificateRequest cr;
  detail::readBuf<uint8_t>(cr.certificate_request_context, cursor);
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, cr.extensions, cursor));
  ret = std::move(cr);
  return Status::Success;
}

template <>
inline Status
decode(CertificateMsg& ret, Error& err, folly::io::Cursor& cursor) {
  CertificateMsg cert;
  detail::readBuf<uint8_t>(cert.certificate_request_context, cursor);
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<detail::bits24>(
          len, err, cert.certificate_list, cursor));
  ret = std::move(cert);
  return Status::Success;
}

template <>
inline Status
decode(CompressedCertificate& ret, Error& err, folly::io::Cursor& cursor) {
  CompressedCertificate cc;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, cc.algorithm, cursor));
  cc.uncompressed_length = detail::readBits24(cursor);
  detail::readBuf<detail::bits24>(cc.compressed_certificate_message, cursor);
  ret = std::move(cc);
  return Status::Success;
}

template <>
inline Status
decode(CertificateVerify& ret, Error& err, folly::io::Cursor& cursor) {
  CertificateVerify certVerify;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, certVerify.algorithm, cursor));
  detail::readBuf<uint16_t>(certVerify.signature, cursor);
  ret = std::move(certVerify);
  return Status::Success;
}

template <>
inline Status decode<NewSessionTicket>(
    NewSessionTicket& ret,
    Error& err,
    folly::io::Cursor& cursor) {
  NewSessionTicket nst;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, nst.ticket_lifetime, cursor));
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, nst.ticket_age_add, cursor));
  detail::readBuf<uint8_t>(nst.ticket_nonce, cursor);
  detail::readBuf<uint16_t>(nst.ticket, cursor);
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, nst.extensions, cursor));
  ret = std::move(nst);
  return Status::Success;
}

template <>
inline Status decode(Alert& ret, Error& err, folly::io::Cursor& cursor) {
  Alert alert;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, alert.level, cursor));
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, alert.description, cursor));
  ret = std::move(alert);
  return Status::Success;
}

template <>
inline Status decode<Finished>(
    Finished& ret,
    Error& /* err */,
    std::unique_ptr<folly::IOBuf>&& buf) {
  Finished fin;
  fin.verify_data = std::move(buf);
  ret = std::move(fin);
  return Status::Success;
}

template <>
inline Status
decode<KeyUpdate>(KeyUpdate& ret, Error& err, folly::io::Cursor& cursor) {
  KeyUpdate update;
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, update.request_update, cursor));
  ret = std::move(update);
  return Status::Success;
}

template <class T>
Status decode(T& ret, Error& err, std::unique_ptr<folly::IOBuf>&& buf) {
  folly::io::Cursor cursor(buf.get());
  T decoded;
  FIZZ_RETURN_ON_ERROR(decode<T>(decoded, err, cursor));

  if (!cursor.isAtEnd()) {
    return err.error("didn't read entire message");
  }

  ret = std::move(decoded);
  return Status::Success;
}

template <typename T>
std::string enumToHex(T enumValue) {
  auto value = folly::Endian::big(
      static_cast<typename std::underlying_type<T>::type>(enumValue));
  return folly::hexlify(
      folly::ByteRange(
          reinterpret_cast<const uint8_t*>(&value), sizeof(value)));
}
} // namespace fizz
