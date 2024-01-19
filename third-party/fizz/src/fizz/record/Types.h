/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <fizz/protocol/Events.h>

namespace fizz {

constexpr folly::StringPiece kHkdfLabelPrefix = "tls13 ";

using Buf = std::unique_ptr<folly::IOBuf>;

enum class ProtocolVersion : uint16_t {
  tls_1_0 = 0x0301,
  tls_1_1 = 0x0302,
  tls_1_2 = 0x0303,
  tls_1_3 = 0x0304,
  tls_1_3_23 = 0x7f17,
  tls_1_3_23_fb = 0xfb17,
  tls_1_3_26 = 0x7f1a,
  tls_1_3_26_fb = 0xfb1a,
  tls_1_3_28 = 0x7f1c,
};

ProtocolVersion getRealDraftVersion(ProtocolVersion);

std::string toString(ProtocolVersion);

enum class ContentType : uint8_t {
  alert = 21,
  handshake = 22,
  application_data = 23,

  change_cipher_spec = 20,
};

struct TLSMessage {
  ContentType type;
  Buf fragment;
};

constexpr folly::StringPiece FakeChangeCipherSpec{
    "\x14\x03\x03\x00\x01\x01",
    6};

enum class HandshakeType : uint8_t {
  client_hello = 1,
  server_hello = 2,
  new_session_ticket = 4,
  end_of_early_data = 5,
  hello_retry_request = 6,
  encrypted_extensions = 8,
  certificate = 11,
  certificate_request = 13,
  certificate_verify = 15,
  finished = 20,
  key_update = 24,
  compressed_certificate = 25,
  message_hash = 254,
};

constexpr size_t kMaxHandshakeSize = 0x20000; // 128k

struct message_hash {
  static constexpr HandshakeType handshake_type = HandshakeType::message_hash;
  std::unique_ptr<folly::IOBuf> hash;
};

template <Event e, HandshakeType t>
struct HandshakeStruct : EventType<e> {
  static constexpr HandshakeType handshake_type = t;

  /*
   * Original encoding of the message, populated on received handshake messages.
   */
  folly::Optional<Buf> originalEncoding;
};

enum class ExtensionType : uint16_t {
  server_name = 0,
  supported_groups = 10,
  signature_algorithms = 13,
  application_layer_protocol_negotiation = 16,
  token_binding = 24,
  compress_certificate = 27,
  delegated_credential = 34,
  pre_shared_key = 41,
  early_data = 42,
  supported_versions = 43,
  cookie = 44,
  psk_key_exchange_modes = 45,
  certificate_authorities = 47,
  post_handshake_auth = 49,
  signature_algorithms_cert = 50,
  key_share = 51,
  ech_outer_extensions = 0xfd00,
  quic_transport_parameters = 0x39,
  quic_transport_parameters_draft = 0xffa5,

  // alternate_server_name = 0xfb00,
  // draft_delegated_credential = 0xff02,
  test_extension = 0xff04,
  encrypted_client_hello = 0xfe0d,
  ech_nonce = 0xff03,
  thrift_parameters = 0xff41,
};

std::string toString(ExtensionType);

enum class AlertDescription : uint8_t {
  close_notify = 0,
  end_of_early_data = 1,
  unexpected_message = 10,
  bad_record_mac = 20,
  record_overflow = 22,
  handshake_failure = 40,
  bad_certificate = 42,
  unsupported_certificate = 43,
  certificate_revoked = 44,
  certificate_expired = 45,
  certificate_unknown = 46,
  illegal_parameter = 47,
  unknown_ca = 48,
  access_denied = 49,
  decode_error = 50,
  decrypt_error = 51,
  protocol_version = 70,
  insufficient_security = 71,
  internal_error = 80,
  inappropriate_fallback = 86,
  user_canceled = 90,
  missing_extension = 109,
  unsupported_extension = 110,
  certificate_unobtainable = 111,
  unrecognized_name = 112,
  bad_certificate_status_response = 113,
  bad_certificate_hash_value = 114,
  unknown_psk_identity = 115,
  certificate_required = 116,
  no_application_protocol = 120,
  ech_required = 121
};

std::string toString(AlertDescription);

enum class CipherSuite : uint16_t {
  TLS_AES_128_GCM_SHA256 = 0x1301,
  TLS_AES_256_GCM_SHA384 = 0x1302,
  TLS_CHACHA20_POLY1305_SHA256 = 0x1303,
  TLS_AEGIS_256_SHA512 = 0x1306,
  TLS_AEGIS_128L_SHA256 = 0x1307,
  // experimental cipher suites
  TLS_AES_128_OCB_SHA256_EXPERIMENTAL = 0xFF01,
};

std::string toString(CipherSuite);

enum class PskKeyExchangeMode : uint8_t { psk_ke = 0, psk_dhe_ke = 1 };

std::string toString(PskKeyExchangeMode);

enum class CertificateCompressionAlgorithm : uint16_t {
  zlib = 1,
  brotli = 2,
  zstd = 3,
};

std::string toString(CertificateCompressionAlgorithm);

struct Extension {
  ExtensionType extension_type;
  Buf extension_data; // Limited to 2^16-1 bytes.

  Extension clone() const {
    Extension clone;
    clone.extension_type = this->extension_type;
    if (this->extension_data) {
      clone.extension_data = this->extension_data->clone();
    }
    return clone;
  }
};

struct HkdfLabel {
  uint16_t length;
  const std::string label;
  Buf hash_value;
};

using Random = std::array<uint8_t, 32>;

struct ClientHello
    : HandshakeStruct<Event::ClientHello, HandshakeType::client_hello> {
  ProtocolVersion legacy_version = ProtocolVersion::tls_1_2;
  Random random;
  Buf legacy_session_id;
  std::vector<CipherSuite> cipher_suites;
  std::vector<uint8_t> legacy_compression_methods;
  std::vector<Extension> extensions;

  ClientHello clone() const {
    ClientHello clone;

    clone.legacy_version = this->legacy_version;
    clone.random = this->random;
    if (this->legacy_session_id) {
      clone.legacy_session_id = this->legacy_session_id->clone();
    }
    clone.cipher_suites = this->cipher_suites;
    clone.legacy_compression_methods = this->legacy_compression_methods;

    for (const auto& ext : this->extensions) {
      clone.extensions.push_back(ext.clone());
    }

    return clone;
  }
};

struct ServerHello
    : HandshakeStruct<Event::ServerHello, HandshakeType::server_hello> {
  ProtocolVersion legacy_version = ProtocolVersion::tls_1_2;
  Random random;
  // If legacy_session_id_echo is non-null the ServerHello will be encoded with
  // it and legacy_compression_method.
  Buf legacy_session_id_echo;
  CipherSuite cipher_suite;
  uint8_t legacy_compression_method{0};
  std::vector<Extension> extensions;
};

struct HelloRetryRequest
    : HandshakeStruct<Event::HelloRetryRequest, HandshakeType::server_hello> {
  ProtocolVersion legacy_version = ProtocolVersion::tls_1_2;
  static constexpr Random HrrRandom{
      {0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C,
       0x02, 0x1E, 0x65, 0xB8, 0x91, 0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB,
       0x8C, 0x5E, 0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C}};
  Buf legacy_session_id_echo;
  CipherSuite cipher_suite;
  uint8_t legacy_compression_method{0};
  std::vector<Extension> extensions;
};

struct EndOfEarlyData
    : HandshakeStruct<Event::EndOfEarlyData, HandshakeType::end_of_early_data> {
};

struct EncryptedExtensions : HandshakeStruct<
                                 Event::EncryptedExtensions,
                                 HandshakeType::encrypted_extensions> {
  std::vector<Extension> extensions;
};

struct CertificateEntry {
  Buf cert_data;
  std::vector<Extension> extensions;
};

struct CertificateMsg
    : HandshakeStruct<Event::Certificate, HandshakeType::certificate> {
  Buf certificate_request_context;
  std::vector<CertificateEntry> certificate_list;
};

struct CompressedCertificate : HandshakeStruct<
                                   Event::CompressedCertificate,
                                   HandshakeType::compressed_certificate> {
  CertificateCompressionAlgorithm algorithm;
  uint32_t uncompressed_length;
  Buf compressed_certificate_message;
};

struct CertificateRequest : HandshakeStruct<
                                Event::CertificateRequest,
                                HandshakeType::certificate_request> {
  Buf certificate_request_context;
  std::vector<Extension> extensions;
};

enum class SignatureScheme : uint16_t {
  ecdsa_secp256r1_sha256 = 0x0403,
  ecdsa_secp384r1_sha384 = 0x0503,
  ecdsa_secp521r1_sha512 = 0x0603,
  rsa_pss_sha256 = 0x0804,
  rsa_pss_sha384 = 0x0805,
  rsa_pss_sha512 = 0x0806,
  ed25519 = 0x0807,
  ed448 = 0x0808,
  // all batch scheme type numbers are temporarially assigned
  ecdsa_secp256r1_sha256_batch = 0xFE00,
  ecdsa_secp384r1_sha384_batch = 0xFE01,
  ecdsa_secp521r1_sha512_batch = 0xFE02,
  ed25519_batch = 0xFE03,
  ed448_batch = 0xFE04,
  rsa_pss_sha256_batch = 0xFE05,
};

std::string toString(SignatureScheme);

struct CertificateVerify : HandshakeStruct<
                               Event::CertificateVerify,
                               HandshakeType::certificate_verify> {
  SignatureScheme algorithm;
  Buf signature;
};

struct Finished : HandshakeStruct<Event::Finished, HandshakeType::finished> {
  Buf verify_data;
};

struct NewSessionTicket : HandshakeStruct<
                              Event::NewSessionTicket,
                              HandshakeType::new_session_ticket> {
  uint32_t ticket_lifetime;
  uint32_t ticket_age_add;
  // Ticket nonce is set to null iff pre-draft 21.
  Buf ticket_nonce;
  Buf ticket;
  std::vector<Extension> extensions;
};

enum class KeyUpdateRequest : uint8_t {
  update_not_requested = 0,
  update_requested = 1
};

struct KeyUpdate
    : HandshakeStruct<Event::KeyUpdate, HandshakeType::key_update> {
  KeyUpdateRequest request_update;
};

enum class NamedGroup : uint16_t {
  secp256r1 = 23,
  secp384r1 = 24,
  secp521r1 = 25,
  x25519 = 29,

  /**
   * x25519 and secp256r1 hybrids with NIST Round 3 version of Kyber, see
   * https://datatracker.ietf.org/doc/draft-tls-westerbaan-xyber768d00/02/
   */
  x25519_kyber768_draft00 = 25497,
  secp256r1_kyber768_draft00 = 25498,

  // experimental
  /**
   * Hybrid of secp521r1 and x25519. TLS Supported Group 510 is reserved for
   * private use, see
   * https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8
   */
  secp521r1_x25519 = 510,

  // Standardized algorithms. See
  // https://datatracker.ietf.org/doc/html/draft-ietf-tls-hybrid-design-05#section-5

  /**
   * Experimental ID, see
   * https://github.com/aws/s2n-tls/blob/main/tls/s2n_tls_parameters.h#L69
   */
  x25519_kyber512 = 12089,

  /**
   * Experimental ID, see
   * https://github.com/aws/s2n-tls/blob/main/tls/s2n_tls_parameters.h#L70
   */
  secp256r1_kyber512 = 12090,

  /**
   * Performance test only. Purely relying on unverified post-quantum crypto may
   * cause security flaws.
   */
  kyber512 = 511,

  /**
   * Experimental ID, see
   * https://github.com/open-quantum-safe/boringssl/blob/master/include/openssl/ssl.h#L2410
   */
  secp384r1_kyber768 = 12092,
};

std::string toString(NamedGroup);

struct Alert : EventType<Event::Alert> {
  uint8_t level = 0x02;
  AlertDescription description;

  Alert() = default;
  explicit Alert(AlertDescription desc) : description(desc) {}
};

struct CloseNotify : EventType<Event::CloseNotify> {
  CloseNotify() = default;
  explicit CloseNotify(std::unique_ptr<folly::IOBuf> data)
      : ignoredPostCloseData(std::move(data)) {}
  std::unique_ptr<folly::IOBuf> ignoredPostCloseData;
};

class FizzException : public std::runtime_error {
 public:
  FizzException(const std::string& msg, folly::Optional<AlertDescription> alert)
      : std::runtime_error(msg), alert_(alert) {}

  folly::Optional<AlertDescription> getAlert() const {
    return alert_;
  }

 private:
  folly::Optional<AlertDescription> alert_;
};

template <class T>
Buf encode(T&&);
template <class T>
Buf encodeHandshake(T&& t);
template <class T>
T decode(std::unique_ptr<folly::IOBuf>&& buf);
template <class T>
T decode(folly::io::Cursor& cursor);
template <typename T>
std::string enumToHex(T enumValue);

Buf encodeHkdfLabel(HkdfLabel&& label, const std::string& hkdfLabelPrefix);
} // namespace fizz

#ifdef FOLLY_MOBILE
namespace std {
template <>
struct hash<fizz::ExtensionType> {
  size_t operator()(fizz::ExtensionType t) const noexcept {
    using underlying_type = std::underlying_type_t<decltype(t)>;
    return std::hash<underlying_type>{}(static_cast<underlying_type>(t));
  }
};
} // namespace std
#endif

template <>
struct fmt::formatter<fizz::ExtensionType> : formatter<unsigned> {
  auto format(fizz::ExtensionType t, format_context& ctx) {
    return formatter<unsigned>::format(folly::to_underlying(t), ctx);
  }
};

#include <fizz/record/Types-inl.h>
