/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/delegatedcred/SelfDelegatedCredential.h>

#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace extensions {
namespace test {

/* using override */
using namespace testing;

class MockSelfDelegatedCredential : public SelfDelegatedCredential {
 public:
  MOCK_METHOD(std::string, getIdentity, (), (const));
  MOCK_METHOD(std::vector<std::string>, getAltIdentities, (), (const));
  MOCK_METHOD(std::vector<SignatureScheme>, getSigSchemes, (), (const));
  MOCK_METHOD(const DelegatedCredential&, _getDelegatedCredential, (), (const));

  const DelegatedCredential& getDelegatedCredential() const override {
    return _getDelegatedCredential();
  }

  MOCK_METHOD(CertificateMsg, _getCertMessage, (Buf&), (const));
  CertificateMsg getCertMessage(Buf buf) const override {
    return _getCertMessage(buf);
  }
  MOCK_METHOD(
      CompressedCertificate,
      getCompressedCert,
      (CertificateCompressionAlgorithm),
      (const));

  MOCK_METHOD(
      Buf,
      sign,
      (SignatureScheme scheme,
       CertificateVerifyContext context,
       folly::ByteRange toBeSigned),
      (const));
  MOCK_METHOD(folly::ssl::X509UniquePtr, getX509, (), (const));
};

} // namespace test
} // namespace extensions
} // namespace fizz
