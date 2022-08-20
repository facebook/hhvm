/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/ech/Encryption.h>
#include <fizz/protocol/ech/test/TestUtil.h>

namespace fizz {
namespace ech {
namespace test {

std::vector<Extension> getExtensions(folly::StringPiece hex) {
  auto buf = folly::IOBuf::copyBuffer(folly::unhexlify((hex.toString())));
  folly::io::Cursor cursor(buf.get());
  Extension ext;
  CHECK_EQ(detail::read(ext, cursor), buf->computeChainDataLength());
  CHECK(cursor.isAtEnd());
  std::vector<Extension> exts;
  exts.push_back(std::move(ext));
  return exts;
}

ECHConfigContentDraft getECHConfigContent() {
  ECHCipherSuite suite{
      hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  ECHConfigContentDraft echConfigContent;
  echConfigContent.public_name = folly::IOBuf::copyBuffer("v9 publicname");
  echConfigContent.public_key = detail::encodeECPublicKey(
      ::fizz::test::getPublicKey(::fizz::test::kP256PublicKey));
  echConfigContent.kem_id = hpke::KEMId::secp256r1;
  echConfigContent.cipher_suites = {suite};
  echConfigContent.maximum_name_length = 1000;
  folly::StringPiece cookie{"002c00080006636f6f6b6965"};
  echConfigContent.extensions = getExtensions(cookie);
  return echConfigContent;
}

ECHConfig getECHConfig() {
  ECHConfig config;
  config.version = ECHVersion::Draft9;
  config.ech_config_content = encode(getECHConfigContent());
  return config;
}

ClientHello getClientHelloOuter() {
  // Create fake client hello outer
  ClientHello chloOuter;
  chloOuter.legacy_session_id =
      folly::IOBuf::copyBuffer("test legacy session id");

  // Set fake server name
  ServerNameList sni;
  ServerName sn;
  sn.hostname = folly::IOBuf::copyBuffer("fake host name");
  sni.server_name_list.push_back(std::move(sn));
  chloOuter.extensions.push_back(encodeExtension(std::move(sni)));

  // Set different random
  chloOuter.random.fill(0x00);

  return chloOuter;
}

} // namespace test
} // namespace ech
} // namespace fizz
