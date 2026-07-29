/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/test/CertUtil.h>
#include <folly/portability/GTest.h>
#include <folly/ssl/OpenSSLCertUtils.h>

using namespace fizz;
using namespace fizz::test;
using namespace fizz::openssl;

static const CertAndKey& getCert() {
  static auto c = [] {
    return fizz::test::createCert(/*cn=*/"test",
                                  /*ca=*/false,
                                  /*issuer=*/nullptr,
                                  /*keyType=*/fizz::KeyType::P256);
  }();
  return c;
}

TEST(OpenSSLPeerCertImplTest, GetDERRoundTrips) {
  std::unique_ptr<OpenSSLPeerCertImpl<openssl::KeyType::P256>> cert;
  Error err;
  FIZZ_THROW_ON_ERROR(
      openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>::create(
          cert, err, folly::ssl::X509UniquePtr(X509_dup(getCert().cert.get()))),
      err);

  auto der = cert->getDER();
  ASSERT_NE(der, std::nullopt);

  auto decoded = folly::ssl::OpenSSLCertUtils::derDecode(
      folly::ByteRange((const unsigned char*)der->data(), der->size()));
  EXPECT_EQ(X509_cmp(decoded.get(), getCert().cert.get()), 0);
}

TEST(OpenSSLSelfCertImplTest, GetDERRoundTrips) {
  std::unique_ptr<OpenSSLSelfCertImpl<openssl::KeyType::P256>> cert;
  Error err;

  std::vector<folly::ssl::X509UniquePtr> x509s;
  x509s.push_back(folly::ssl::X509UniquePtr(X509_dup(getCert().cert.get())));

  folly::ssl::EvpPkeyUniquePtr key(getCert().key.get());
  EVP_PKEY_up_ref(key.get());

  FIZZ_THROW_ON_ERROR(
      openssl::OpenSSLSelfCertImpl<openssl::KeyType::P256>::create(
          cert, err, std::move(key), std::move(x509s)),
      err);

  auto der = cert->getDER();
  ASSERT_NE(der, std::nullopt);

  auto decoded = folly::ssl::OpenSSLCertUtils::derDecode(
      folly::ByteRange((const unsigned char*)der->data(), der->size()));
  EXPECT_EQ(X509_cmp(decoded.get(), getCert().cert.get()), 0);
}
