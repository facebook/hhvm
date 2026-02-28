/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/test/SignatureTestData.h>
#include <fizz/protocol/Certificate.h>

namespace fizz {
namespace test {

void testCertVerify(
    SignatureTestData testCase,
    std::unique_ptr<PeerCert> (*makePeerCert)(Buf));
} // namespace test
} // namespace fizz
