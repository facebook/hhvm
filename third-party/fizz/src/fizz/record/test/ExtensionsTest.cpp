/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/record/Extensions.h>
#include <fizz/record/test/ExtensionTestsBase.h>
#include <folly/String.h>

using namespace folly;

StringPiece alpn{"00100017001502683208737064792f332e3108687474702f312e31"};
StringPiece sni{"0000001500130000107777772e66616365626f6f6b2e636f6d"};
StringPiece helloRetryRequestKeyShare{"003300020017"};
StringPiece clientEarlyData{"002a0000"};
StringPiece serverEarlyData{"002a0000"};
StringPiece ticketEarlyData{"002a000400000005"};
StringPiece cookie{"002c00080006636f6f6b6965"};
StringPiece authorities{
    "002f005400520028434e3d4c696d696e616c6974792c204f553d46697a7a2c204f3d46616365626f6f6b2c20433d55530026434e3d457465726e6974792c204f553d46697a7a2c204f3d46616365626f6f6b2c20433d5553"};
StringPiece certCompressionAlgorithms{"001b0003020001"};

namespace fizz {
namespace test {

TEST_F(ExtensionsTest, TestProtocolNameList) {
  auto exts = getExtensions(alpn);
  auto ext = getExtension<ProtocolNameList>(exts);

  EXPECT_EQ(ext->protocol_name_list.size(), 3);
  EXPECT_EQ(
      StringPiece(ext->protocol_name_list[0].name->coalesce()),
      StringPiece("h2"));
  EXPECT_EQ(
      StringPiece(ext->protocol_name_list[1].name->coalesce()),
      StringPiece("spdy/3.1"));
  EXPECT_EQ(
      StringPiece(ext->protocol_name_list[2].name->coalesce()),
      StringPiece("http/1.1"));

  checkEncode(std::move(*ext), alpn);
}

TEST_F(ExtensionsTest, TestServerNameList) {
  auto exts = getExtensions(sni);
  auto ext = getExtension<ServerNameList>(exts);

  EXPECT_EQ(ext->server_name_list.size(), 1);
  EXPECT_EQ(ext->server_name_list[0].name_type, ServerNameType::host_name);
  EXPECT_EQ(
      StringPiece(ext->server_name_list[0].hostname->coalesce()),
      StringPiece("www.facebook.com"));

  checkEncode(std::move(*ext), sni);
}

TEST_F(ExtensionsTest, TestHelloRetryRequestKeyShare) {
  auto exts = getExtensions(helloRetryRequestKeyShare);
  auto ext = getExtension<HelloRetryRequestKeyShare>(exts);

  EXPECT_EQ(ext->selected_group, NamedGroup::secp256r1);

  checkEncode(std::move(*ext), helloRetryRequestKeyShare);
}

TEST_F(ExtensionsTest, TestClientEarlyData) {
  auto exts = getExtensions(clientEarlyData);
  auto ext = getExtension<ClientEarlyData>(exts);
  checkEncode(std::move(*ext), clientEarlyData);
}

TEST_F(ExtensionsTest, TestServerEarlyData) {
  auto exts = getExtensions(serverEarlyData);
  auto ext = getExtension<ServerEarlyData>(exts);
  checkEncode(std::move(*ext), serverEarlyData);
}

TEST_F(ExtensionsTest, TestTicketEarlyData) {
  auto exts = getExtensions(ticketEarlyData);
  auto ext = getExtension<TicketEarlyData>(exts);
  EXPECT_EQ(ext->max_early_data_size, 5);
  checkEncode(std::move(*ext), ticketEarlyData);
}

TEST_F(ExtensionsTest, TestCookie) {
  auto exts = getExtensions(cookie);
  auto ext = getExtension<Cookie>(exts);

  EXPECT_EQ(StringPiece(ext->cookie->coalesce()), StringPiece("cookie"));

  checkEncode(std::move(*ext), cookie);
}

TEST_F(ExtensionsTest, TestCertificateAuthorities) {
  auto exts = getExtensions(authorities);
  auto ext = getExtension<CertificateAuthorities>(exts);

  EXPECT_EQ(ext->authorities.size(), 2);
  EXPECT_EQ(
      StringPiece(ext->authorities[0].encoded_name->coalesce()),
      StringPiece("CN=Liminality, OU=Fizz, O=Facebook, C=US"));
  EXPECT_EQ(
      StringPiece(ext->authorities[1].encoded_name->coalesce()),
      StringPiece("CN=Eternity, OU=Fizz, O=Facebook, C=US"));

  checkEncode(std::move(*ext), authorities);
}

TEST_F(ExtensionsTest, TestCertificateCompressionAlgorithms) {
  auto exts = getExtensions(certCompressionAlgorithms);
  auto ext = getExtension<CertificateCompressionAlgorithms>(exts);

  EXPECT_EQ(ext->algorithms.size(), 1);
  EXPECT_EQ(ext->algorithms[0], CertificateCompressionAlgorithm::zlib);
  checkEncode(std::move(*ext), certCompressionAlgorithms);
}

TEST_F(ExtensionsTest, TestBadlyFormedExtension) {
  auto buf = getBuf(sni);
  buf->reserve(0, 1);
  buf->append(1);
  std::vector<Extension> exts;
  Extension ext;
  ext.extension_type = ExtensionType::server_name;
  ext.extension_data = std::move(buf);
  exts.push_back(std::move(ext));
  EXPECT_THROW(getExtension<ServerNameList>(exts), std::runtime_error);
}
} // namespace test
} // namespace fizz
