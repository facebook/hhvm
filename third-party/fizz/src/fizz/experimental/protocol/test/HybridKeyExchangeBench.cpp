/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/experimental/protocol/HybridKeyExFactory.h>
#include <fizz/test/HandshakeTest.h>
#include <folly/Benchmark.h>
#include <folly/init/Init.h>

using namespace fizz;

/**
 * Test key share generation speed. In ECDH, it's done in generateKeyPair(). In
 * KEM it's done in getKeyShare(). This simulates the first client operation.
 */
void clientKeyShareGenerationBench(uint64_t n, NamedGroup namedGroup) {
  std::unique_ptr<KeyExchange> kex;
  BENCHMARK_SUSPEND {
    auto f = std::make_unique<HybridKeyExFactory>();
    kex = f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Client);
  }
  std::unique_ptr<folly::IOBuf> keyShare;
  for (uint64_t i = 0; i < n; i++) {
    kex->generateKeyPair();
    keyShare = kex->getKeyShare();
  }
  folly::doNotOptimizeAway(keyShare);
}

/**
 * Test shared secret generation speed for EDCH and cipher generation speed for
 * KEM. This simulates the only server operation.
 */
void serverSharedSecretDerivationBench(uint64_t n, NamedGroup namedGroup) {
  std::unique_ptr<KeyExchange> kex;
  std::vector<std::unique_ptr<folly::IOBuf>> clientKeyShareIOBuf;
  std::vector<folly::ByteRange> clientKeyShare;
  BENCHMARK_SUSPEND {
    auto f = std::make_unique<HybridKeyExFactory>();
    kex = f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Server);
    for (uint64_t i = 0; i < n; i++) {
      auto fakeClientKex =
          f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Client);
      fakeClientKex->generateKeyPair();
      clientKeyShareIOBuf.push_back(fakeClientKex->getKeyShare());
      clientKeyShare.push_back(clientKeyShareIOBuf[i]->coalesce());
    }
  }
  std::unique_ptr<folly::IOBuf> keyShare;
  for (uint64_t i = 0; i < n; i++) {
    kex->generateKeyPair();
    keyShare = kex->generateSharedSecret(clientKeyShare[i]);
  }
  folly::doNotOptimizeAway(keyShare);
}

/**
 * Test decap() performance of KEMs. As a comparison, ECDH will run
 * generateSharedSecret(). This simulates the second client operation.
 */
void clientSharedSecretDerivationBench(uint64_t n, NamedGroup namedGroup) {
  std::unique_ptr<KeyExchange> kex;
  std::vector<std::unique_ptr<folly::IOBuf>> serverKeyShareIOBuf;
  std::vector<folly::ByteRange> serverKeyShare;
  BENCHMARK_SUSPEND {
    auto f = std::make_unique<HybridKeyExFactory>();
    kex = f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Client);
    kex->generateKeyPair();
    kex->getKeyShare();
    for (uint64_t i = 0; i < n; i++) {
      auto fakeServerKex =
          f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Server);
      fakeServerKex->generateKeyPair();
      fakeServerKex->generateSharedSecret(
          kex->clone()->getKeyShare()->coalesce());
      serverKeyShareIOBuf.push_back(fakeServerKex->getKeyShare());
      serverKeyShare.push_back(serverKeyShareIOBuf[i]->coalesce());
    }
  }
  std::unique_ptr<folly::IOBuf> keyShare;
  for (uint64_t i = 0; i < n; i++) {
    keyShare = kex->generateSharedSecret(serverKeyShare[i]);
  }
  folly::doNotOptimizeAway(keyShare);
}

BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    x25519,
    NamedGroup::x25519);
BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    x25519_kyber512,
    NamedGroup::x25519_kyber512);
BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    x25519_kyber768_draft00,
    NamedGroup::x25519_kyber768_draft00);
BENCHMARK_NAMED_PARAM(
    serverSharedSecretDerivationBench,
    x25519,
    NamedGroup::x25519);
BENCHMARK_NAMED_PARAM(
    serverSharedSecretDerivationBench,
    x25519_kyber512,
    NamedGroup::x25519_kyber512);
BENCHMARK_NAMED_PARAM(
    serverSharedSecretDerivationBench,
    x25519_kyber768_draft00,
    NamedGroup::x25519_kyber768_draft00);

BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    x25519,
    NamedGroup::x25519);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    x25519_kyber512,
    NamedGroup::x25519_kyber512);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    x25519_kyber768_draft00,
    NamedGroup::x25519_kyber768_draft00);

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
