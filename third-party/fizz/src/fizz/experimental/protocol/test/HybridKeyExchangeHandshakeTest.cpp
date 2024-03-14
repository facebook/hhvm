/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/experimental/protocol/HybridKeyExFactory.h>
#include <fizz/test/HandshakeTest.h>

namespace fizz {
namespace test {

class HybridKeyExchangeHandshakeTest : public HandshakeTest,
                                       public WithParamInterface<NamedGroup> {};

TEST_P(HybridKeyExchangeHandshakeTest, hybrid_key_exchange_handshake_test) {
  auto namedGroup = GetParam();
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({namedGroup});
  clientContext_->setDefaultShares({namedGroup});
  serverContext_->setSupportedGroups({namedGroup});
  expected_.group = namedGroup;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

INSTANTIATE_TEST_SUITE_P(
    HybridKeyExchangeHandshakeTests,
    HybridKeyExchangeHandshakeTest,
    Values(
        NamedGroup::secp521r1_x25519,
        NamedGroup::x25519_kyber512,
        NamedGroup::secp256r1_kyber512,
        NamedGroup::x25519_kyber768_draft00,
        NamedGroup::x25519_kyber768_experimental,
        NamedGroup::secp256r1_kyber768_draft00,
        NamedGroup::secp384r1_kyber768,
        NamedGroup::x25519 // Non-hybrid named group using hybrid factory
        ),
    [](const testing::TestParamInfo<HybridKeyExchangeHandshakeTest::ParamType>&
           info) { return toString(info.param); });

} // namespace test
} // namespace fizz
