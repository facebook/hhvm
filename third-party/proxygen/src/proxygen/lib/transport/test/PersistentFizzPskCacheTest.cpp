/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/PersistentFizzPskCache.h>

#include <fizz/protocol/test/Utilities.h>
#include <folly/FileUtil.h>
#include <folly/portability/GTest.h>
#include <wangle/client/persistence/test/TestUtil.h>

using namespace testing;

namespace proxygen { namespace test {

class PersistentFizzPskCacheTest : public Test {
 public:
  void SetUp() override {
    file_ = wangle::getPersistentCacheFilename();
    createCache();

    std::shared_ptr<fizz::PeerCert> serverCert = fizz::test::getPeerCert(
        fizz::test::createCert("server", false, nullptr));

    std::shared_ptr<fizz::PeerCert> clientCert = fizz::test::getPeerCert(
        fizz::test::createCert("client", false, nullptr));

    psk1_.psk = "PSK1";
    psk1_.secret = "secret1";
    psk1_.type = fizz::PskType::Resumption;
    psk1_.version = fizz::ProtocolVersion::tls_1_3;
    psk1_.cipher = fizz::CipherSuite::TLS_AES_128_GCM_SHA256;
    psk1_.group = fizz::NamedGroup::x25519;
    psk1_.serverCert = serverCert;
    psk1_.clientCert = clientCert;
    psk1_.alpn = "h2";
    psk1_.ticketAgeAdd = 0x11111111;
    psk1_.ticketIssueTime = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::milliseconds(1));
    psk1_.ticketExpirationTime =
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::seconds(2));
    psk1_.ticketHandshakeTime =
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(1));
    psk1_.maxEarlyDataSize = 0;

    psk2_.psk = "PSK2";
    psk2_.secret = "secret2";
    psk2_.type = fizz::PskType::Resumption;
    psk2_.version = fizz::ProtocolVersion::tls_1_2;
    psk2_.cipher = fizz::CipherSuite::TLS_AES_256_GCM_SHA384;
    psk2_.group = folly::none;
    psk2_.serverCert = serverCert;
    psk2_.clientCert = nullptr;
    psk2_.alpn = folly::none;
    psk2_.ticketAgeAdd = 0x22222222;
    psk2_.ticketIssueTime = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(1507653827));
    psk2_.ticketExpirationTime =
        psk2_.ticketIssueTime + std::chrono::seconds(100);
    psk2_.ticketHandshakeTime =
        psk2_.ticketIssueTime - std::chrono::seconds(100);
    psk2_.maxEarlyDataSize = 10000;
  }

  void TearDown() override {
    cache_.reset();
    unlink(file_.c_str());
  }

  void createCache() {
    cache_.reset();
    cache_ = std::make_unique<PersistentFizzPskCache>(
        file_,
        wangle::PersistentCacheConfig::Builder()
            .setCapacity(50)
            .setSyncInterval(std::chrono::seconds(1))
            .build());
  }

  fizz::client::CachedPsk psk1_;
  fizz::client::CachedPsk psk2_;
  std::unique_ptr<PersistentFizzPskCache> cache_;
  std::string file_;
};

static void expectMatch(const fizz::client::CachedPsk& a,
                        const fizz::client::CachedPsk& b) {
  EXPECT_EQ(a.psk, b.psk);
  EXPECT_EQ(a.secret, b.secret);
  EXPECT_EQ(a.type, b.type);
  EXPECT_EQ(a.version, b.version);
  EXPECT_EQ(a.cipher, b.cipher);
  EXPECT_EQ(a.group, b.group);
  EXPECT_EQ((a.serverCert.get() != nullptr), (b.serverCert.get() != nullptr));
  if (!a.alpn) {
    EXPECT_TRUE(!b.alpn);
  } else {
    EXPECT_EQ(*a.alpn, *b.alpn);
  }
  EXPECT_EQ(a.ticketAgeAdd, b.ticketAgeAdd);
  EXPECT_EQ(a.ticketIssueTime, b.ticketIssueTime);
  EXPECT_EQ(a.ticketExpirationTime, b.ticketExpirationTime);
  EXPECT_EQ(a.ticketHandshakeTime, b.ticketHandshakeTime);
  EXPECT_EQ(a.maxEarlyDataSize, b.maxEarlyDataSize);
}

TEST_F(PersistentFizzPskCacheTest, TestInsert2Hosts) {
  cache_->putPsk("facebook.com", psk1_);
  cache_->putPsk("something.com", psk2_);

  expectMatch(*cache_->getPsk("facebook.com"), psk1_);
  expectMatch(*cache_->getPsk("something.com"), psk2_);
  EXPECT_FALSE(cache_->getPsk("somethingelse.com").has_value());

  createCache();

  expectMatch(*cache_->getPsk("facebook.com"), psk1_);
  expectMatch(*cache_->getPsk("something.com"), psk2_);
  EXPECT_FALSE(cache_->getPsk("somethingelse.com").has_value());
}

TEST_F(PersistentFizzPskCacheTest, TestOverwrite) {
  cache_->putPsk("facebook.com", psk1_);
  cache_->putPsk("facebook.com", psk2_);

  expectMatch(*cache_->getPsk("facebook.com"), psk2_);

  createCache();

  expectMatch(*cache_->getPsk("facebook.com"), psk2_);
}

TEST_F(PersistentFizzPskCacheTest, TestRemove) {
  cache_->putPsk("facebook.com", psk1_);
  expectMatch(*cache_->getPsk("facebook.com"), psk1_);
  cache_->removePsk("facebook.com");
  EXPECT_FALSE(cache_->getPsk("facebook.com").has_value());
}

TEST_F(PersistentFizzPskCacheTest, TestCompletelyCorruptedCache) {
  cache_->putPsk("facebook.com", psk1_);
  cache_.reset();

  folly::writeFile(std::string("HI!!!"), file_.c_str());

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentFizzPskCacheTest, TestSomewhatCorruptedCache) {
  cache_->putPsk("facebook.com", psk1_);
  cache_.reset();
  auto config = wangle::PersistentCacheConfig::Builder()
                    .setCapacity(20)
                    .setSyncInterval(std::chrono::seconds(1))
                    .build();
  auto otherCache =
      std::make_unique<wangle::FilePersistentCache<std::string, double>>(
          file_, std::move(config));
  otherCache->put("facebook.com", 1.1111);
  otherCache.reset();

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentFizzPskCacheTest, TestEntryCorruptedCache) {
  cache_->putPsk("facebook.com", psk1_);
  cache_.reset();
  auto config = wangle::PersistentCacheConfig::Builder()
                    .setSyncInterval(std::chrono::seconds(1))
                    .setCapacity(20)
                    .build();
  auto otherCache = std::make_unique<
      wangle::FilePersistentCache<std::string, PersistentCachedPsk>>(
      file_, std::move(config));
  otherCache->put("facebook.com", PersistentCachedPsk{"I'm a PSK?", 2});
  otherCache.reset();

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentFizzPskCacheTest, TestTruncatedEntry) {
  cache_.reset();
  auto config = wangle::PersistentCacheConfig::Builder()
                    .setSyncInterval(std::chrono::seconds(1))
                    .setCapacity(20)
                    .build();
  auto otherCache = std::make_unique<
      wangle::FilePersistentCache<std::string, PersistentCachedPsk>>(
      file_, std::move(config));
  auto psk1Serialized = serializePsk(psk1_);
  // Store PSK with last 12 characters (64 bits + 32 bits) truncated
  otherCache->put("facebook.com",
                  PersistentCachedPsk{
                      psk1Serialized.substr(0, psk1Serialized.size() - 12), 2});
  otherCache.reset();

  createCache();
  EXPECT_FALSE(cache_->getPsk("facebook.com"));
}

TEST_F(PersistentFizzPskCacheTest, TestTruncatedHandshakeTime) {
  cache_.reset();
  auto config = wangle::PersistentCacheConfig::Builder()
                    .setSyncInterval(std::chrono::seconds(1))
                    .setCapacity(20)
                    .build();
  auto otherCache = std::make_unique<
      wangle::FilePersistentCache<std::string, PersistentCachedPsk>>(
      file_, std::move(config));
  auto psk1Serialized = serializePsk(psk1_);
  // Store PSK with last 12 characters (64 bits) truncated
  otherCache->put("facebook.com",
                  PersistentCachedPsk{
                      psk1Serialized.substr(0, psk1Serialized.size() - 8), 2});
  otherCache.reset();

  createCache();
  // Ought to succeed, handshake time should be assigned current time
  auto before = std::chrono::system_clock::now();
  auto psk = cache_->getPsk("facebook.com");
  auto after = std::chrono::system_clock::now();
  EXPECT_TRUE(psk);
  EXPECT_TRUE(psk->ticketHandshakeTime > before);
  EXPECT_TRUE(psk->ticketHandshakeTime < after);
}

TEST_F(PersistentFizzPskCacheTest, TestUnlimitedUses) {
  cache_->setMaxPskUses(0);

  cache_->putPsk("facebook.com", psk1_);

  for (size_t i = 0; i < 10; i++) {
    expectMatch(*cache_->getPsk("facebook.com"), psk1_);
  }
}

TEST_F(PersistentFizzPskCacheTest, TestLimitedUses) {
  cache_->setMaxPskUses(3);

  cache_->putPsk("facebook.com", psk1_);

  for (size_t i = 0; i < 3; i++) {
    expectMatch(*cache_->getPsk("facebook.com"), psk1_);
  }

  EXPECT_FALSE(cache_->getPsk("facebook.com").has_value());
}

TEST_F(PersistentFizzPskCacheTest, TestLimitedUsesSerialize) {
  cache_->setMaxPskUses(3);

  cache_->putPsk("facebook.com", psk1_);

  expectMatch(*cache_->getPsk("facebook.com"), psk1_);

  createCache();
  cache_->setMaxPskUses(3);

  expectMatch(*cache_->getPsk("facebook.com"), psk1_);
  expectMatch(*cache_->getPsk("facebook.com"), psk1_);

  EXPECT_FALSE(cache_->getPsk("facebook.com").has_value());
}

TEST_F(PersistentFizzPskCacheTest, TestGetPskUses) {
  cache_->setMaxPskUses(3);

  EXPECT_EQ(folly::none, cache_->getPskUses("facebook.com"));
  cache_->putPsk("facebook.com", psk1_);
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
