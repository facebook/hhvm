/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/MultiBackendFactory.h>
#include <fizz/test/HandshakeTest.h>

namespace fizz {
namespace test {

class MultiBackendFactoryHandshakeTest : public HandshakeTest,
                                         public WithParamInterface<NamedGroup> {
};

TEST_P(MultiBackendFactoryHandshakeTest, multibackendfactory_handshake_test) {
  auto namedGroup = GetParam();
  auto factory = std::make_shared<MultiBackendFactory>();
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
    MultiBackendFactoryHandshakeTests,
    MultiBackendFactoryHandshakeTest,
    Values(
        NamedGroup::secp256r1,
        NamedGroup::secp384r1,
        NamedGroup::secp521r1,
        NamedGroup::x25519
#if FIZZ_HAVE_OQS
        ,
        NamedGroup::x25519_kyber512,
        NamedGroup::secp256r1_kyber512,
        NamedGroup::kyber512,
        NamedGroup::x25519_kyber768_draft00,
        NamedGroup::x25519_kyber768_experimental,
        NamedGroup::x25519_kyber512_experimental,
        NamedGroup::secp256r1_kyber768_draft00,
        NamedGroup::secp384r1_kyber768
#endif
        ),
    [](const testing::TestParamInfo<
        MultiBackendFactoryHandshakeTest::ParamType>& info) {
      return toString(info.param);
    });

} // namespace test
} // namespace fizz
