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
  };
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
void serverKeyShareGenerationBench(uint64_t n, NamedGroup namedGroup) {
  std::unique_ptr<KeyExchange> kex;
  std::vector<std::unique_ptr<folly::IOBuf>> clientKeyShareIOBuf;
  std::vector<folly::ByteRange> clientKeyShare;
  BENCHMARK_SUSPEND {
    auto f = std::make_unique<HybridKeyExFactory>();
    kex = f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Server);
    kex->generateKeyPair();
    for (uint64_t i = 0; i < n; i++) {
      auto fakeClientKex =
          f->makeKeyExchange(namedGroup, Factory::KeyExchangeMode::Client);
      fakeClientKex->generateKeyPair();
      clientKeyShareIOBuf.push_back(fakeClientKex->getKeyShare());
      clientKeyShare.push_back(clientKeyShareIOBuf[i]->coalesce());
    }
  };
  std::unique_ptr<folly::IOBuf> keyShare;
  for (uint64_t i = 0; i < n; i++) {
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
  };
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
    kyber512,
    NamedGroup::kyber512);
BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    x25519_kyber512,
    NamedGroup::x25519_kyber512);
BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    secp256r1,
    NamedGroup::secp256r1);
BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    secp256r1_kyber512,
    NamedGroup::secp256r1_kyber512);
BENCHMARK_NAMED_PARAM(
    clientKeyShareGenerationBench,
    cecpq2,
    NamedGroup::cecpq2);

BENCHMARK_NAMED_PARAM(
    serverKeyShareGenerationBench,
    x25519,
    NamedGroup::x25519);
BENCHMARK_NAMED_PARAM(
    serverKeyShareGenerationBench,
    kyber512,
    NamedGroup::kyber512);
BENCHMARK_NAMED_PARAM(
    serverKeyShareGenerationBench,
    x25519_kyber512,
    NamedGroup::x25519_kyber512);
BENCHMARK_NAMED_PARAM(
    serverKeyShareGenerationBench,
    secp256r1,
    NamedGroup::secp256r1);
BENCHMARK_NAMED_PARAM(
    serverKeyShareGenerationBench,
    secp256r1_kyber512,
    NamedGroup::secp256r1_kyber512);
BENCHMARK_NAMED_PARAM(
    serverKeyShareGenerationBench,
    cecpq2,
    NamedGroup::cecpq2);

BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    x25519,
    NamedGroup::x25519);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    kyber512,
    NamedGroup::kyber512);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    x25519_kyber512,
    NamedGroup::x25519_kyber512);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    secp256r1,
    NamedGroup::secp256r1);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    secp256r1_kyber512,
    NamedGroup::secp256r1_kyber512);
BENCHMARK_NAMED_PARAM(
    clientSharedSecretDerivationBench,
    cecpq2,
    NamedGroup::cecpq2);

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
