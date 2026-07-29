/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <functional>
#include <memory>
#include <vector>

#include <folly/Benchmark.h>
#include <folly/Random.h>
#include <folly/init/Init.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/crypto/KeyDerivation.h>
#include <fizz/crypto/Utils.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/util/Status.h>

using namespace fizz;

namespace {

// Produces a KeyScheduler under test. Parameterizing the benchmark on this
// factory lets the same handshake body be run against different KeyScheduler
// (or KeyDerivation) implementations.
using KeySchedulerFactory = std::function<std::unique_ptr<KeyScheduler>()>;

// AES-128-GCM traffic key and IV lengths, matching TLS_AES_128_GCM_SHA256.
constexpr size_t kKeyLength = 16;
constexpr size_t kIvLength = 12;

// Distinct transcript hashes consumed per handshake: the three early-secret
// getSecret calls, the two handshake-secret getSecret calls, the two
// master-secret getSecret calls, and deriveAppTrafficSecrets.
constexpr size_t kTranscriptsPerHandshake = 8;

std::vector<uint8_t> makeRandomBytes(size_t n) {
  std::vector<uint8_t> bytes(n);
  for (auto& byte : bytes) {
    byte = static_cast<uint8_t>(folly::Random::rand32());
  }
  return bytes;
}

// Derives the traffic key and IV from a traffic secret.
void deriveTrafficKey(
    const KeyScheduler& keyScheduler,
    const DerivedSecret& secret) {
  Error err;
  TrafficKey trafficKey;
  FIZZ_THROW_ON_ERROR(
      keyScheduler.getTrafficKey(
          trafficKey,
          err,
          folly::ByteRange(secret.secret.data(), secret.secret.size()),
          kKeyLength,
          kIvLength),
      err);
  folly::doNotOptimizeAway(trafficKey);
}

// The default KeyScheduler backed by a SHA-256 key derivation implementation.
std::unique_ptr<KeyScheduler> makeDefaultSha256KeyScheduler() {
  return std::make_unique<KeyScheduler>(
      std::make_unique<KeyDerivationImpl>(openssl::hasherFactory<Sha256>()));
}

// Exercises the KeyScheduler invocations of a full TLS 1.3 handshake using both
// a PSK and an ECDHE share. In each state it fetches every secret that is
// available (skipping ExternalPskBinder and the ECH secrets), derives a traffic
// key from every traffic secret, and derives the application traffic secrets.
// Key updates are intentionally excluded.
void fullHandshake(
    uint32_t iters,
    const KeySchedulerFactory& makeKeyScheduler) {
  std::vector<uint8_t> psk;
  std::vector<uint8_t> ecdhe;
  // A fresh transcript hash is used for every transcript-consuming call. They
  // are pre-generated as one contiguous buffer (kTranscriptsPerHandshake per
  // iteration) so the random generation is excluded from the timed section.
  std::vector<uint8_t> transcripts;
  BENCHMARK_SUSPEND {
    // SHA-256 produces 32-byte secrets and transcript hashes.
    psk = makeRandomBytes(Sha256::HashLen);
    ecdhe = makeRandomBytes(Sha256::HashLen);
    transcripts = makeRandomBytes(
        static_cast<size_t>(iters) * kTranscriptsPerHandshake *
        Sha256::HashLen);
  }

  const folly::ByteRange pskRange(psk.data(), psk.size());
  const folly::ByteRange ecdheRange(ecdhe.data(), ecdhe.size());

  size_t transcriptOffset = 0;
  const auto nextTranscript = [&]() {
    folly::ByteRange transcript(
        transcripts.data() + transcriptOffset, Sha256::HashLen);
    transcriptOffset += Sha256::HashLen;
    return transcript;
  };

  for (uint32_t i = 0; i < iters; ++i) {
    auto keyScheduler = makeKeyScheduler();
    Error err;
    DerivedSecret secret;

    // Early secret, derived from the PSK.
    FIZZ_THROW_ON_ERROR(keyScheduler->deriveEarlySecret(err, pskRange), err);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret, err, EarlySecrets::ResumptionPskBinder, nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret, err, EarlySecrets::ClientEarlyTraffic, nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);
    deriveTrafficKey(*keyScheduler, secret);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret, err, EarlySecrets::EarlyExporter, nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);

    // Handshake secret, mixing in the ECDHE share.
    FIZZ_THROW_ON_ERROR(
        keyScheduler->deriveHandshakeSecret(err, ecdheRange), err);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret,
            err,
            HandshakeSecrets::ClientHandshakeTraffic,
            nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);
    deriveTrafficKey(*keyScheduler, secret);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret,
            err,
            HandshakeSecrets::ServerHandshakeTraffic,
            nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);
    deriveTrafficKey(*keyScheduler, secret);

    // Master secret.
    FIZZ_THROW_ON_ERROR(keyScheduler->deriveMasterSecret(err), err);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret, err, MasterSecrets::ExporterMaster, nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);
    FIZZ_THROW_ON_ERROR(
        keyScheduler->getSecret(
            secret, err, MasterSecrets::ResumptionMaster, nextTranscript()),
        err);
    folly::doNotOptimizeAway(secret);

    // Application traffic secrets.
    FIZZ_THROW_ON_ERROR(
        keyScheduler->deriveAppTrafficSecrets(err, nextTranscript()), err);
    auto clientAppSecret =
        keyScheduler->getSecret(AppTrafficSecrets::ClientAppTraffic);
    folly::doNotOptimizeAway(clientAppSecret);
    deriveTrafficKey(*keyScheduler, clientAppSecret);
    auto serverAppSecret =
        keyScheduler->getSecret(AppTrafficSecrets::ServerAppTraffic);
    folly::doNotOptimizeAway(serverAppSecret);
    deriveTrafficKey(*keyScheduler, serverAppSecret);
  }
}

} // namespace

BENCHMARK_NAMED_PARAM(
    fullHandshake,
    default_sha256,
    makeDefaultSha256KeyScheduler);

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  Error err;
  FIZZ_THROW_ON_ERROR(CryptoUtils::init(err), err);
  folly::runBenchmarks();
  return 0;
}
