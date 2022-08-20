/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/PersistentQuicPskCache.h>

#include <fizz/protocol/test/Utilities.h>
#include <folly/FileUtil.h>
#include <folly/portability/GTest.h>
#include <wangle/client/persistence/test/TestUtil.h>

using namespace testing;
using namespace quic;

namespace proxygen { namespace test {

class PersistentQuicPskCacheTest : public Test {
 public:
  void SetUp() override {
    file_ = wangle::getPersistentCacheFilename();
    createCache();

    std::shared_ptr<fizz::PeerCert> serverCert = fizz::test::getPeerCert(
        fizz::test::createCert("server", false, nullptr));

    std::shared_ptr<fizz::PeerCert> clientCert = fizz::test::getPeerCert(
        fizz::test::createCert("client", false, nullptr));

    auto& fizzPsk1 = quicPsk1_.cachedPsk;
    fizzPsk1.psk = "PSK1";
    fizzPsk1.secret = "secret1";
    fizzPsk1.type = fizz::PskType::Resumption;
    fizzPsk1.version = fizz::ProtocolVersion::tls_1_3;
    fizzPsk1.cipher = fizz::CipherSuite::TLS_AES_128_GCM_SHA256;
    fizzPsk1.group = fizz::NamedGroup::x25519;
    fizzPsk1.serverCert = serverCert;
    fizzPsk1.clientCert = clientCert;
    fizzPsk1.alpn = "h2";
    fizzPsk1.ticketAgeAdd = 0x11111111;
    fizzPsk1.ticketIssueTime =
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(1));
    fizzPsk1.ticketExpirationTime =
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::seconds(2));
    fizzPsk1.maxEarlyDataSize = 0;
    quicPsk1_.transportParams.initialMaxStreamDataBidiLocal = 100;
    quicPsk1_.transportParams.initialMaxStreamDataBidiRemote = 100;
    quicPsk1_.transportParams.initialMaxStreamDataUni = 100;
    quicPsk1_.transportParams.initialMaxData = 200;
    quicPsk1_.transportParams.idleTimeout = 60;
    quicPsk1_.transportParams.maxRecvPacketSize = 500;
    quicPsk1_.transportParams.initialMaxStreamsBidi = 234;
    quicPsk1_.transportParams.initialMaxStreamsUni = 123;
    quicPsk1_.appParams = "QPACK param";

    auto& fizzPsk2 = quicPsk2_.cachedPsk;
    fizzPsk2.psk = "PSK2";
    fizzPsk2.secret = "secret2";
    fizzPsk2.type = fizz::PskType::Resumption;
    fizzPsk2.version = fizz::ProtocolVersion::tls_1_2;
    fizzPsk2.cipher = fizz::CipherSuite::TLS_AES_256_GCM_SHA384;
    fizzPsk2.group = folly::none;
    fizzPsk2.serverCert = serverCert;
    fizzPsk2.clientCert = nullptr;
    fizzPsk2.alpn = folly::none;
    fizzPsk2.ticketAgeAdd = 0x22222222;
    fizzPsk2.ticketIssueTime =
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::seconds(1507653827));
    fizzPsk2.ticketExpirationTime =
        fizzPsk2.ticketIssueTime + std::chrono::seconds(100);
    fizzPsk2.maxEarlyDataSize = 10000;
    quicPsk2_.transportParams.initialMaxStreamDataBidiLocal = 400;
    quicPsk2_.transportParams.initialMaxStreamDataBidiRemote = 400;
    quicPsk2_.transportParams.initialMaxStreamDataUni = 400;
    quicPsk2_.transportParams.initialMaxData = 800;
    quicPsk2_.transportParams.idleTimeout = 90;
    quicPsk2_.transportParams.maxRecvPacketSize = 800;
    quicPsk1_.transportParams.initialMaxStreamsBidi = 2345;
    quicPsk1_.transportParams.initialMaxStreamsUni = 1233;
  }

  void TearDown() override {
    cache_.reset();
    unlink(file_.c_str());
  }

  void createCache() {
    cache_.reset();
    cache_ = std::make_unique<PersistentQuicPskCache>(
        file_,
        wangle::PersistentCacheConfig::Builder()
            .setCapacity(50)
            .setSyncInterval(std::chrono::seconds(1))
            .build());
  }

  QuicCachedPsk quicPsk1_;
  QuicCachedPsk quicPsk2_;
  std::unique_ptr<PersistentQuicPskCache> cache_;
  std::string file_;
};

static void expectMatch(const QuicCachedPsk& a, const QuicCachedPsk& b) {
  const auto& fizzPskA = a.cachedPsk;
  const auto& fizzPskB = b.cachedPsk;
  EXPECT_EQ(fizzPskA.psk, fizzPskB.psk);
  EXPECT_EQ(fizzPskA.secret, fizzPskB.secret);
  EXPECT_EQ(fizzPskA.type, fizzPskB.type);
  EXPECT_EQ(fizzPskA.version, fizzPskB.version);
  EXPECT_EQ(fizzPskA.cipher, fizzPskB.cipher);
  EXPECT_EQ(fizzPskA.group, fizzPskB.group);
  EXPECT_EQ((fizzPskA.serverCert.get() != nullptr),
            (fizzPskB.serverCert.get() != nullptr));
  if (!fizzPskA.alpn) {
    EXPECT_TRUE(!fizzPskB.alpn);
  } else {
    EXPECT_EQ(*fizzPskA.alpn, *fizzPskB.alpn);
  }
  EXPECT_EQ(fizzPskA.ticketAgeAdd, fizzPskB.ticketAgeAdd);
  EXPECT_EQ(fizzPskA.ticketIssueTime, fizzPskB.ticketIssueTime);
  EXPECT_EQ(fizzPskA.ticketExpirationTime, fizzPskB.ticketExpirationTime);
  EXPECT_EQ(fizzPskA.maxEarlyDataSize, fizzPskB.maxEarlyDataSize);

  const auto& paramsA = a.transportParams;
  const auto& paramsB = b.transportParams;
  EXPECT_EQ(paramsA.initialMaxStreamDataBidiLocal,
            paramsB.initialMaxStreamDataBidiLocal);
  EXPECT_EQ(paramsA.initialMaxStreamDataBidiRemote,
            paramsB.initialMaxStreamDataBidiRemote);
  EXPECT_EQ(paramsA.initialMaxStreamDataUni, paramsB.initialMaxStreamDataUni);
  EXPECT_EQ(paramsA.initialMaxData, paramsB.initialMaxData);
  EXPECT_EQ(paramsA.idleTimeout, paramsB.idleTimeout);
  EXPECT_EQ(paramsA.maxRecvPacketSize, paramsB.maxRecvPacketSize);
  EXPECT_EQ(paramsA.initialMaxStreamsBidi, paramsB.initialMaxStreamsBidi);
  EXPECT_EQ(paramsA.initialMaxStreamsUni, paramsB.initialMaxStreamsUni);

  EXPECT_EQ(a.appParams, b.appParams);
}

TEST_F(PersistentQuicPskCacheTest, TestInsert2Hosts) {
  cache_->putPsk("facebook.com", quicPsk1_);
  cache_->putPsk("something.com", quicPsk2_);

  expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);
  expectMatch(*cache_->getPsk("something.com"), quicPsk2_);
  EXPECT_FALSE(cache_->getPsk("somethingelse.com").has_value());

  createCache();

  expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);
  expectMatch(*cache_->getPsk("something.com"), quicPsk2_);
  EXPECT_FALSE(cache_->getPsk("somethingelse.com").has_value());
}

TEST_F(PersistentQuicPskCacheTest, TestOverwrite) {
  cache_->putPsk("facebook.com", quicPsk1_);
  cache_->putPsk("facebook.com", quicPsk2_);

  expectMatch(*cache_->getPsk("facebook.com"), quicPsk2_);

  createCache();

  expectMatch(*cache_->getPsk("facebook.com"), quicPsk2_);
}

TEST_F(PersistentQuicPskCacheTest, TestRemove) {
  cache_->putPsk("facebook.com", quicPsk1_);
  expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);
  cache_->removePsk("facebook.com");
  EXPECT_FALSE(cache_->getPsk("facebook.com").has_value());
}

TEST_F(PersistentQuicPskCacheTest, TestCompletelyCorruptedCache) {
  cache_->putPsk("facebook.com", quicPsk1_);
  cache_.reset();

  folly::writeFile(std::string("HI!!!"), file_.c_str());

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentQuicPskCacheTest, TestSomewhatCorruptedCache) {
  cache_->putPsk("facebook.com", quicPsk1_);
  cache_.reset();

  auto otherCache =
      std::make_unique<wangle::FilePersistentCache<std::string, double>>(
          file_,
          wangle::PersistentCacheConfig::Builder()
              .setCapacity(20)
              .setSyncInterval(std::chrono::seconds(1))
              .build());
  otherCache->put("facebook.com", 1.1111);
  otherCache.reset();

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentQuicPskCacheTest, TestEntryCorruptedCache) {
  cache_->putPsk("facebook.com", quicPsk1_);
  cache_.reset();

  auto otherCache = std::make_unique<
      wangle::FilePersistentCache<std::string, PersistentQuicCachedPsk>>(
      file_,
      wangle::PersistentCacheConfig::Builder()
          .setCapacity(20)
          .setSyncInterval(std::chrono::seconds(1))
          .build());
  otherCache->put("facebook.com",
                  PersistentQuicCachedPsk{"I'm a PSK?", "params"});
  otherCache.reset();

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentQuicPskCacheTest, TestUnlimitedUses) {
  cache_->setMaxPskUses(0);

  cache_->putPsk("facebook.com", quicPsk1_);

  for (size_t i = 0; i < 10; i++) {
    expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);
  }
}

TEST_F(PersistentQuicPskCacheTest, TestLimitedUses) {
  cache_->setMaxPskUses(3);

  cache_->putPsk("facebook.com", quicPsk1_);

  for (size_t i = 0; i < 3; i++) {
    expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);
  }

  EXPECT_FALSE(cache_->getPsk("facebook.com").has_value());
}

TEST_F(PersistentQuicPskCacheTest, TestLimitedUsesSerialize) {
  cache_->setMaxPskUses(3);

  cache_->putPsk("facebook.com", quicPsk1_);

  expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);

  createCache();
  cache_->setMaxPskUses(3);

  expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);
  expectMatch(*cache_->getPsk("facebook.com"), quicPsk1_);

  EXPECT_FALSE(cache_->getPsk("facebook.com").has_value());
}

TEST_F(PersistentQuicPskCacheTest, TestGetPskUses) {
  cache_->setMaxPskUses(3);

  EXPECT_EQ(folly::none, cache_->getPskUses("facebook.com"));
  cache_->putPsk("facebook.com", quicPsk1_);
  EXPECT_EQ(0u, cache_->getPskUses("facebook.com"));

  cache_->getPsk("facebook.com");
  EXPECT_EQ(1u, cache_->getPskUses("facebook.com"));

  cache_->getPsk("facebook.com");
  EXPECT_EQ(2u, cache_->getPskUses("facebook.com"));

  createCache();
  cache_->setMaxPskUses(3);

  cache_->getPsk("facebook.com");
  EXPECT_EQ(folly::none, cache_->getPskUses("facebook.com"));
}
}} // namespace proxygen::test
