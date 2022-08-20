/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Params.h>
#include <fizz/protocol/test/ProtocolTest.h>
#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>

namespace fizz {
namespace test {

struct TestMessages {
  template <typename T>
  static void removeExtension(T& msg, ExtensionType ext) {
    auto it = findExtension(msg.extensions, ext);
    msg.extensions.erase(it);
  }

  static ClientHello clientHello() {
    ClientHello chlo;
    chlo.cipher_suites.push_back(CipherSuite::TLS_AES_128_GCM_SHA256);
    chlo.cipher_suites.push_back(CipherSuite::TLS_AES_256_GCM_SHA384);
    chlo.random.fill(0x44);
    chlo.legacy_compression_methods.push_back(0x00);
    chlo.legacy_session_id = folly::IOBuf::create(0);
    SupportedVersions supportedVersions;
    supportedVersions.versions.push_back(TestProtocolVersion);
    chlo.extensions.push_back(encodeExtension(std::move(supportedVersions)));
    SupportedGroups supportedGroups;
    supportedGroups.named_group_list.push_back(NamedGroup::x25519);
    supportedGroups.named_group_list.push_back(NamedGroup::secp256r1);
    chlo.extensions.push_back(encodeExtension(std::move(supportedGroups)));
    ClientKeyShare keyShare;
    KeyShareEntry entry;
    entry.group = NamedGroup::x25519;
    entry.key_exchange = folly::IOBuf::copyBuffer("keyshare");
    keyShare.client_shares.push_back(std::move(entry));
    chlo.extensions.push_back(encodeExtension(std::move(keyShare)));
    SignatureAlgorithms sigAlgs;
    sigAlgs.supported_signature_algorithms.push_back(
        SignatureScheme::ecdsa_secp256r1_sha256);
    sigAlgs.supported_signature_algorithms.push_back(
        SignatureScheme::rsa_pss_sha256);
    chlo.extensions.push_back(encodeExtension(std::move(sigAlgs)));
    ServerNameList sni;
    ServerName sn;
    sn.hostname = folly::IOBuf::copyBuffer("www.hostname.com");
    sni.server_name_list.push_back(std::move(sn));
    chlo.extensions.push_back(encodeExtension(std::move(sni)));
    ProtocolNameList alpn;
    ProtocolName h2;
    h2.name = folly::IOBuf::copyBuffer("h2");
    alpn.protocol_name_list.push_back(std::move(h2));
    chlo.extensions.push_back(encodeExtension(std::move(alpn)));
    PskKeyExchangeModes modes;
    modes.modes.push_back(PskKeyExchangeMode::psk_dhe_ke);
    modes.modes.push_back(PskKeyExchangeMode::psk_ke);
    chlo.extensions.push_back(encodeExtension(std::move(modes)));
    chlo.originalEncoding = folly::IOBuf::copyBuffer("clienthelloencoding");
    return chlo;
  }

  static void addPsk(ClientHello& chlo, uint32_t ticketAge = 100000) {
    ClientPresharedKey cpk;
    PskIdentity ident;
    ident.psk_identity = folly::IOBuf::copyBuffer("ident");
    ident.obfuscated_ticket_age = ticketAge;
    cpk.identities.push_back(std::move(ident));
    PskBinder binder;
    binder.binder = folly::IOBuf::copyBuffer("verifydata");
    cpk.binders.push_back(std::move(binder));
    chlo.extensions.push_back(encodeExtension(std::move(cpk)));
  }

  static ClientHello clientHelloPsk() {
    auto chlo = clientHello();
    addPsk(chlo);
    return chlo;
  }

  static ClientHello clientHelloPskEarly(uint32_t ticketAge = 100000) {
    auto chlo = clientHello();
    chlo.extensions.push_back(encodeExtension(ClientEarlyData()));
    addPsk(chlo, ticketAge);
    return chlo;
  }

  static HelloRetryRequest helloRetryRequest() {
    HelloRetryRequest hrr;
    hrr.legacy_version = ProtocolVersion::tls_1_2;
    hrr.cipher_suite = CipherSuite::TLS_AES_128_GCM_SHA256;
    ServerSupportedVersions supportedVersions;
    supportedVersions.selected_version = TestProtocolVersion;
    hrr.extensions.push_back(encodeExtension(std::move(supportedVersions)));
    HelloRetryRequestKeyShare keyShare;
    keyShare.selected_group = NamedGroup::secp256r1;
    hrr.extensions.push_back(encodeExtension(std::move(keyShare)));
    hrr.originalEncoding = folly::IOBuf::copyBuffer("hrrencoding");
    return hrr;
  }

  static ServerHello serverHello() {
    ServerHello shlo;
    shlo.legacy_version = ProtocolVersion::tls_1_2;
    shlo.legacy_session_id_echo = folly::IOBuf::create(0);
    shlo.random.fill(0x44);
    shlo.cipher_suite = CipherSuite::TLS_AES_128_GCM_SHA256;
    ServerSupportedVersions supportedVersions;
    supportedVersions.selected_version = TestProtocolVersion;
    shlo.extensions.push_back(encodeExtension(std::move(supportedVersions)));
    ServerKeyShare serverKeyShare;
    serverKeyShare.server_share.group = NamedGroup::x25519;
    serverKeyShare.server_share.key_exchange =
        folly::IOBuf::copyBuffer("servershare");
    shlo.extensions.push_back(encodeExtension(std::move(serverKeyShare)));
    shlo.originalEncoding = folly::IOBuf::copyBuffer("shloencoding");
    return shlo;
  }

  static ServerHello serverHelloPsk() {
    auto shlo = serverHello();
    ServerPresharedKey pskExt;
    pskExt.selected_identity = 0;
    shlo.extensions.push_back(encodeExtension(std::move(pskExt)));
    return shlo;
  }

  static EndOfEarlyData endOfEarlyData() {
    EndOfEarlyData eoed;
    eoed.originalEncoding = folly::IOBuf::copyBuffer("eoedencoding");
    return eoed;
  }

  static EncryptedExtensions encryptedExt() {
    EncryptedExtensions encryptedExt;
    ProtocolNameList alpn;
    ProtocolName h2;
    h2.name = folly::IOBuf::copyBuffer("h2");
    alpn.protocol_name_list.push_back(std::move(h2));
    encryptedExt.extensions.push_back(encodeExtension(std::move(alpn)));
    encryptedExt.originalEncoding = folly::IOBuf::copyBuffer("eeencoding");
    return encryptedExt;
  }

  static EncryptedExtensions encryptedExtEarly() {
    auto ee = encryptedExt();
    ee.extensions.push_back(encodeExtension(ServerEarlyData()));
    return ee;
  }

  static CertificateMsg certificate() {
    CertificateMsg certificate;
    certificate.certificate_request_context = folly::IOBuf::copyBuffer("");
    certificate.originalEncoding = folly::IOBuf::copyBuffer("certencoding");
    return certificate;
  }

  static CompressedCertificate compressedCertificate() {
    CompressedCertificate cc;
    cc.algorithm = CertificateCompressionAlgorithm::zlib;
    cc.uncompressed_length = 0x111111;
    cc.compressed_certificate_message =
        folly::IOBuf::copyBuffer("compressedcerts");
    cc.originalEncoding = folly::IOBuf::copyBuffer("compcertencoding");
    return cc;
  }

  static CertificateVerify certificateVerify() {
    CertificateVerify verify;
    verify.algorithm = SignatureScheme::ecdsa_secp256r1_sha256;
    verify.signature = folly::IOBuf::copyBuffer("signature");
    verify.originalEncoding = folly::IOBuf::copyBuffer("certverifyencoding");
    return verify;
  }

  static CertificateRequest certificateRequest() {
    CertificateRequest cr;
    cr.certificate_request_context = folly::IOBuf::copyBuffer("");
    SignatureAlgorithms sigAlgs;
    sigAlgs.supported_signature_algorithms.push_back(
        SignatureScheme::ecdsa_secp256r1_sha256);
    sigAlgs.supported_signature_algorithms.push_back(
        SignatureScheme::rsa_pss_sha256);
    cr.extensions.push_back(encodeExtension(std::move(sigAlgs)));
    cr.originalEncoding = folly::IOBuf::copyBuffer("certrequestencoding");
    return cr;
  }

  static Finished finished() {
    Finished finished;
    finished.verify_data = folly::IOBuf::copyBuffer("verifydata");
    finished.originalEncoding = folly::IOBuf::copyBuffer("finishedencoding");
    return finished;
  }

  static NewSessionTicket newSessionTicket() {
    NewSessionTicket nst;
    nst.ticket_lifetime = 100;
    nst.ticket_age_add = 0x44444444;
    nst.ticket_nonce = folly::IOBuf::create(0);
    nst.ticket = folly::IOBuf::copyBuffer("ticket");
    return nst;
  }

  static AppData appData() {
    AppData appData(folly::IOBuf::copyBuffer("appdata"));
    return appData;
  }

  static AppWrite appWrite() {
    AppWrite appWrite;
    appWrite.data = folly::IOBuf::copyBuffer("appdata");
    return appWrite;
  }

  static EarlyAppWrite earlyAppWrite() {
    EarlyAppWrite appWrite;
    appWrite.data = folly::IOBuf::copyBuffer("appdata");
    return appWrite;
  }

  static KeyUpdate keyUpdate(bool reqUpdate) {
    KeyUpdate keyUpdate;
    keyUpdate.request_update = reqUpdate
        ? KeyUpdateRequest::update_requested
        : KeyUpdateRequest::update_not_requested;
    return keyUpdate;
  }
};
} // namespace test
} // namespace fizz
