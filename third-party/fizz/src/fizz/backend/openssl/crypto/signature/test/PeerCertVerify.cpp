/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/crypto/test/Signature.h>
#include <fizz/crypto/test/SignatureTestData.h>

using namespace testing;

namespace fizz {
namespace openssl {
namespace test {

class VerifyTest : public Test,
                   public WithParamInterface<fizz::test::SignatureTestData> {};

TEST_P(VerifyTest, PeerCertVerify) {
  fizz::test::testCertVerify(GetParam(), openssl::CertUtils::makePeerCert);
}

INSTANTIATE_TEST_SUITE_P(
    SignatureTestVectors,
    VerifyTest,
    ValuesIn(fizz::test::kSignatureTestVectors));

} // namespace test
} // namespace openssl
} // namespace fizz
