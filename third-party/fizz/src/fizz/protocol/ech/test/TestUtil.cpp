/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/ech/Encryption.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <fizz/protocol/test/TestUtil.h>

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

ParsedECHConfig getParsedECHConfig(ECHConfigParam param) {
  ParsedECHConfig parsedECHConfig;
  parsedECHConfig.key_config.config_id = param.configId;
  parsedECHConfig.key_config.kem_id = hpke::KEMId::secp256r1;
  parsedECHConfig.key_config.public_key = openssl::detail::encodeECPublicKey(
      ::fizz::test::getPublicKey(::fizz::test::kP256PublicKey));
  parsedECHConfig.key_config.cipher_suites.push_back(
      HpkeSymmetricCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256});
  parsedECHConfig.maximum_name_length = 50;
  parsedECHConfig.public_name = std::move(param.publicName);
  if (param.cookie.hasValue()) {
    parsedECHConfig.extensions =
        getExtensions(folly::StringPiece(param.cookie.value()));
  }
  return parsedECHConfig;
}

ECHConfig getECHConfig(ECHConfigParam param) {
  ECHConfig config;
  config.version = ECHVersion::Draft15;
  config.ech_config_content = encode(getParsedECHConfig(std::move(param)));
  return config;
}

ClientHello getClientHelloOuter() {
  // Create fake client hello outer
  ClientHello chloOuter = ::fizz::test::TestMessages::clientHello();
  chloOuter.legacy_session_id =
      folly::IOBuf::copyBuffer("test legacy session id");

  // Set fake server name
  ServerNameList sni;
  ServerName sn;
  sn.hostname = folly::IOBuf::copyBuffer("public.dummy.com");
  sni.server_name_list.push_back(std::move(sn));
  chloOuter.extensions.push_back(encodeExtension(std::move(sni)));

  // Set different random
  chloOuter.random.fill(0x00);

  return chloOuter;
}

bool isEqual(const ParsedECHConfig& l, const ParsedECHConfig& r) {
  auto toBuf = [](const ParsedECHConfig& config) {
    auto buf = folly::IOBuf::create(detail::getSize(config));
    folly::io::Appender out(buf.get(), 0);
    detail::write(config, out);
    return buf;
  };
  return folly::IOBufEqualTo()(toBuf(l), toBuf(r));
}

} // namespace test
} // namespace ech
} // namespace fizz
