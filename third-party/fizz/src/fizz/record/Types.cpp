/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/Types.h>
#include <folly/String.h>

namespace fizz {

#if __cplusplus < 201703L
constexpr Random HelloRetryRequest::HrrRandom;
#endif

ProtocolVersion getRealDraftVersion(ProtocolVersion version) {
  switch (version) {
    case ProtocolVersion::tls_1_3:
      return ProtocolVersion::tls_1_3;
    case ProtocolVersion::tls_1_3_23:
    case ProtocolVersion::tls_1_3_23_fb:
      return ProtocolVersion::tls_1_3_23;
    case ProtocolVersion::tls_1_3_26:
    case ProtocolVersion::tls_1_3_26_fb:
      return ProtocolVersion::tls_1_3_26;
    case ProtocolVersion::tls_1_3_28:
      return ProtocolVersion::tls_1_3_28;
    default:
      throw std::runtime_error(folly::to<std::string>(
          "getRealDraftVersion() called with ", toString(version)));
  }
}

std::string toString(ProtocolVersion version) {
  switch (version) {
    case ProtocolVersion::tls_1_0:
      return "TLSv1.0";
    case ProtocolVersion::tls_1_1:
      return "TLSv1.1";
    case ProtocolVersion::tls_1_2:
      return "TLSv1.2";
    case ProtocolVersion::tls_1_3:
      return "TLSv1.3";
    case ProtocolVersion::tls_1_3_23:
      return "TLSv1.3-draft-23";
    case ProtocolVersion::tls_1_3_23_fb:
      return "TLSv1.3-draft-23-fb";
    case ProtocolVersion::tls_1_3_26:
      return "TLSv1.3-draft-26";
    case ProtocolVersion::tls_1_3_26_fb:
      return "TLSv1.3-draft-26-fb";
    case ProtocolVersion::tls_1_3_28:
      return "TLSv1.3-draft-28";
  }
  return enumToHex(version);
}

std::string toString(ExtensionType extType) {
  switch (extType) {
    case ExtensionType::server_name:
      return "server_name";
    case ExtensionType::supported_groups:
      return "supported_groups";
    case ExtensionType::signature_algorithms:
      return "signature_algorithms";
    case ExtensionType::application_layer_protocol_negotiation:
      return "application_layer_protocol_negotiation";
    case ExtensionType::token_binding:
      return "token_binding";
    case ExtensionType::quic_transport_parameters_draft:
      return "quic_transport_parameters_draft";
    case ExtensionType::quic_transport_parameters:
      return "quic_transport_parameters";
    case ExtensionType::pre_shared_key:
      return "pre_shared_key";
    case ExtensionType::early_data:
      return "early_data";
    case ExtensionType::supported_versions:
      return "supported_version";
    case ExtensionType::cookie:
      return "cookie";
    case ExtensionType::psk_key_exchange_modes:
      return "psk_key_exchange_modes";
    case ExtensionType::certificate_authorities:
      return "certificate_authorities";
    case ExtensionType::post_handshake_auth:
      return "post_handshake_auth";
    case ExtensionType::signature_algorithms_cert:
      return "signature_algorithms_cert";
    case ExtensionType::key_share:
      return "key_share";
    case ExtensionType::compress_certificate:
      return "compress_certificate";
    case ExtensionType::thrift_parameters:
      return "thrift_parameters";
    case ExtensionType::test_extension:
      return "test_extension";
    case ExtensionType::delegated_credential:
      return "delegated_credential";
    case ExtensionType::encrypted_client_hello:
      return "encrypted_client_hello";
    case ExtensionType::ech_nonce:
      return "ech_nonce";
    case ExtensionType::ech_outer_extensions:
      return "ech_outer_extensions";
  }
  return enumToHex(extType);
}

std::string toString(AlertDescription alertDesc) {
  switch (alertDesc) {
    case AlertDescription::close_notify:
      return "close_notify";
    case AlertDescription::end_of_early_data:
      return "end_of_early_data";
    case AlertDescription::unexpected_message:
      return "unexpected_message";
    case AlertDescription::bad_record_mac:
      return "bad_record_mac";
    case AlertDescription::record_overflow:
      return "record_overflow";
    case AlertDescription::handshake_failure:
      return "handshake_failure";
    case AlertDescription::bad_certificate:
      return "bad_certificate";
    case AlertDescription::unsupported_certificate:
      return "unsupported_certificate";
    case AlertDescription::certificate_revoked:
      return "certificate_revoked";
    case AlertDescription::certificate_expired:
      return "certificate_expired";
    case AlertDescription::certificate_unknown:
      return "certificate_unknown";
    case AlertDescription::illegal_parameter:
      return "illegal_parameter";
    case AlertDescription::unknown_ca:
      return "unknown_ca";
    case AlertDescription::access_denied:
      return "access_denied";
    case AlertDescription::decode_error:
      return "decode_error";
    case AlertDescription::decrypt_error:
      return "decrypt_error";
    case AlertDescription::protocol_version:
      return "protocol_version";
    case AlertDescription::insufficient_security:
      return "insufficient_security";
    case AlertDescription::internal_error:
      return "internal_error";
    case AlertDescription::inappropriate_fallback:
      return "inappropriate_fallback";
    case AlertDescription::user_canceled:
      return "user_canceled";
    case AlertDescription::missing_extension:
      return "missing_extension";
    case AlertDescription::unsupported_extension:
      return "unsupported_extension";
    case AlertDescription::certificate_unobtainable:
      return "certificate_unobtainable";
    case AlertDescription::unrecognized_name:
      return "unrecognized_name";
    case AlertDescription::bad_certificate_status_response:
      return "bad_certificate_status_response";
    case AlertDescription::bad_certificate_hash_value:
      return "bad_certificate_hash_value";
    case AlertDescription::unknown_psk_identity:
      return "unknown_psk_identity";
    case AlertDescription::certificate_required:
      return "certificate_required";
    case AlertDescription::no_application_protocol:
      return "no_application_protocol";
    case AlertDescription::ech_required:
      return "ech_required";
  }
  return enumToHex(alertDesc);
}

std::string toString(CipherSuite cipher) {
  switch (cipher) {
    case CipherSuite::TLS_AES_128_GCM_SHA256:
      return "TLS_AES_128_GCM_SHA256";
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      return "TLS_AES_256_GCM_SHA384";
    case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      return "TLS_CHACHA20_POLY1305_SHA256";
    case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
      return "TLS_AES_128_OCB_SHA256_EXPERIMENTAL";
    case CipherSuite::TLS_AEGIS_256_SHA512:
      return "TLS_AEGIS_256_SHA512";
    case CipherSuite::TLS_AEGIS_128L_SHA256:
      return "TLS_AEGIS_128L_SHA256";
  }
  return enumToHex(cipher);
}

std::string toString(PskKeyExchangeMode pskKeMode) {
  switch (pskKeMode) {
    case PskKeyExchangeMode::psk_ke:
      return "psk_ke";
    case PskKeyExchangeMode::psk_dhe_ke:
      return "psk_dhe_ke";
  }
  return enumToHex(pskKeMode);
}

std::string toString(SignatureScheme sigScheme) {
  switch (sigScheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256:
      return "ecdsa_secp256r1_sha256";
    case SignatureScheme::ecdsa_secp384r1_sha384:
      return "ecdsa_secp384r1_sha384";
    case SignatureScheme::ecdsa_secp521r1_sha512:
      return "ecdsa_secp521r1_sha512";
    case SignatureScheme::rsa_pss_sha256:
      return "rsa_pss_sha256";
    case SignatureScheme::rsa_pss_sha384:
      return "rsa_pss_sha384";
    case SignatureScheme::rsa_pss_sha512:
      return "rsa_pss_sha512";
    case SignatureScheme::ed25519:
      return "ed25519";
    case SignatureScheme::ed448:
      return "ed448";
    case SignatureScheme::ecdsa_secp256r1_sha256_batch:
      return "ecdsa_secp256r1_sha256_batch";
    case SignatureScheme::ecdsa_secp384r1_sha384_batch:
      return "ecdsa_secp384r1_sha384_batch";
    case SignatureScheme::ecdsa_secp521r1_sha512_batch:
      return "ecdsa_secp521r1_sha512_batch";
    case SignatureScheme::ed25519_batch:
      return "ed25519_batch";
    case SignatureScheme::ed448_batch:
      return "ed448_batch";
    case SignatureScheme::rsa_pss_sha256_batch:
      return "rsa_pss_sha256_batch";
  }
  return enumToHex(sigScheme);
}

std::string toString(NamedGroup group) {
  switch (group) {
    case NamedGroup::secp256r1:
      return "secp256r1";
    case NamedGroup::secp384r1:
      return "secp384r1";
    case NamedGroup::secp521r1:
      return "secp521r1";
    case NamedGroup::x25519:
      return "x25519";
    case NamedGroup::secp521r1_x25519:
      return "secp521r1_x25519";
    case NamedGroup::x25519_kyber512:
      return "x25519_kyber512";
    case NamedGroup::secp256r1_kyber512:
      return "secp256r1_kyber512";
    case NamedGroup::kyber512:
      return "kyber512";
    case NamedGroup::x25519_kyber768_draft00:
      return "x25519_kyber768_draft00";
    case NamedGroup::secp256r1_kyber768_draft00:
      return "secp256r1_kyber768_draft00";
    case NamedGroup::secp384r1_kyber768:
      return "secp384r1_kyber768";
  }
  return enumToHex(group);
}

std::string toString(CertificateCompressionAlgorithm algo) {
  switch (algo) {
    case CertificateCompressionAlgorithm::zlib:
      return "zlib";
    case CertificateCompressionAlgorithm::brotli:
      return "brotli";
    case CertificateCompressionAlgorithm::zstd:
      return "zstd";
  }
  return enumToHex(algo);
}
} // namespace fizz
