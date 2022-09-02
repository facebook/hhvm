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
TEST_F(HandshakeTest, secp521r1_x25519) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::secp521r1_x25519});
  clientContext_->setDefaultShares({NamedGroup::secp521r1_x25519});
  serverContext_->setSupportedGroups({NamedGroup::secp521r1_x25519});
  expected_.group = NamedGroup::secp521r1_x25519;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, secp384r1_bikel3) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::secp384r1_bikel3});
  clientContext_->setDefaultShares({NamedGroup::secp384r1_bikel3});
  serverContext_->setSupportedGroups({NamedGroup::secp384r1_bikel3});
  expected_.group = NamedGroup::secp384r1_bikel3;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, secp256r1_kyber512) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::secp256r1_kyber512});
  clientContext_->setDefaultShares({NamedGroup::secp256r1_kyber512});
  serverContext_->setSupportedGroups({NamedGroup::secp256r1_kyber512});
  expected_.group = NamedGroup::secp256r1_kyber512;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, x25519_kyber512) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::x25519_kyber512});
  clientContext_->setDefaultShares({NamedGroup::x25519_kyber512});
  serverContext_->setSupportedGroups({NamedGroup::x25519_kyber512});
  expected_.group = NamedGroup::x25519_kyber512;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, secp384r1_kyber768) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::secp384r1_kyber768});
  clientContext_->setDefaultShares({NamedGroup::secp384r1_kyber768});
  serverContext_->setSupportedGroups({NamedGroup::secp384r1_kyber768});
  expected_.group = NamedGroup::secp384r1_kyber768;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, x25519_kyber768) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::x25519_kyber768});
  clientContext_->setDefaultShares({NamedGroup::x25519_kyber768});
  serverContext_->setSupportedGroups({NamedGroup::x25519_kyber768});
  expected_.group = NamedGroup::x25519_kyber768;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, x25519UsingHybridFactory) {
  auto factory = std::make_shared<HybridKeyExFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({NamedGroup::x25519});
  clientContext_->setDefaultShares({NamedGroup::x25519});
  serverContext_->setSupportedGroups({NamedGroup::x25519});
  expected_.group = NamedGroup::x25519;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}
} // namespace test
} // namespace fizz
