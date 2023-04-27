/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/experimental/ktls/KTLS.h>
#include <folly/test/TestUtils.h>
#include <linux/tls.h>

using namespace testing;
using namespace fizz;

class KTLSTest : public Test {
  void SetUp() override {
    // We take fizz::platformSupportsKTLS() as axiomatic and not run if the
    // host environment does not support KTLS.
    if (!fizz::platformSupportsKTLS()) {
      SKIP() << "kTLS tests require ktls support";
    }
  }
};

template <class CipherSpec>
static TrafficKey createKey() {
  TrafficKey key;
  constexpr size_t ivLen = CipherSpec::kIVLength;
  constexpr size_t keyLen = CipherSpec::kKeyLength;
  key.iv = folly::IOBuf::create(ivLen);
  key.iv->append(ivLen);
  memset(key.iv->writableData(), 'A', ivLen);

  key.key = folly::IOBuf::create(keyLen);
  key.key->append(keyLen);
  memset(key.key->writableData(), 'B', keyLen);
  return key;
}

static const TrafficKey kAES128TrafficKey = createKey<AESGCM128>();
static const TrafficKey kAES256TrafficKey = createKey<AESGCM256>();

TEST_F(KTLSTest, TestSockoptFormat) {
  // An unsupported ktls cipher suite should not work
  {
    KTLSCryptoParams params{};
    params.ciphersuite = CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL;
    params.key = kAES128TrafficKey.clone();
    params.recordSeq = 12;

    EXPECT_EQ(params.toSockoptFormat(), nullptr);
  }

  // Neither should a params struct with a malformed key
  {
    KTLSCryptoParams params{};
    params.ciphersuite = CipherSuite::TLS_AES_128_GCM_SHA256;
    params.key = {};
    params.recordSeq = 12;

    EXPECT_EQ(params.toSockoptFormat(), nullptr);
  }

  // A valid configuration (aes-128-gcm)
  {
    KTLSCryptoParams params{};
    params.ciphersuite = CipherSuite::TLS_AES_128_GCM_SHA256;
    params.key = kAES128TrafficKey.clone();
    params.recordSeq = 12;

    auto buf = params.toSockoptFormat();
    auto bufSlice = buf->coalesce();
    ASSERT_NE(buf, nullptr);
    EXPECT_EQ(bufSlice.size(), sizeof(struct tls12_crypto_info_aes_gcm_128));
    struct tls12_crypto_info_aes_gcm_128 info;
    memcpy(&info, bufSlice.data(), bufSlice.size());

    EXPECT_EQ(info.info.cipher_type, TLS_CIPHER_AES_GCM_128);
    EXPECT_EQ(info.info.version, TLS_1_3_VERSION);
    auto key = params.key.key->coalesce();
    auto iv = params.key.iv->coalesce();
    EXPECT_EQ(0, memcmp(info.key, key.data(), key.size()));
    EXPECT_EQ(0, memcmp(info.salt, iv.data(), sizeof(info.salt)));
    EXPECT_EQ(
        0, memcmp(info.iv, iv.data() + sizeof(info.salt), sizeof(info.iv)));
  }

  // Another valid configuration (aes-256-gcm)
  {
    KTLSCryptoParams params{};
    params.ciphersuite = CipherSuite::TLS_AES_256_GCM_SHA384;
    params.key = kAES256TrafficKey.clone();
    params.recordSeq = 12;

    auto buf = params.toSockoptFormat();
    auto bufSlice = buf->coalesce();
    ASSERT_NE(buf, nullptr);
    EXPECT_EQ(bufSlice.size(), sizeof(struct tls12_crypto_info_aes_gcm_256));
    struct tls12_crypto_info_aes_gcm_256 info;
    memcpy(&info, bufSlice.data(), bufSlice.size());

    EXPECT_EQ(info.info.cipher_type, TLS_CIPHER_AES_GCM_256);
    EXPECT_EQ(info.info.version, TLS_1_3_VERSION);
    auto key = params.key.key->coalesce();
    auto iv = params.key.iv->coalesce();
    EXPECT_EQ(0, memcmp(info.key, key.data(), key.size()));
    EXPECT_EQ(0, memcmp(info.salt, iv.data(), sizeof(info.salt)));
    EXPECT_EQ(
        0, memcmp(info.iv, iv.data() + sizeof(info.salt), sizeof(info.iv)));
  }

  // An invalid configuration: Supplying an AES 128 parameter to a
  // AES 256 requested ciphersuite
  {
    KTLSCryptoParams params{};
    params.ciphersuite = CipherSuite::TLS_AES_256_GCM_SHA384;
    params.key = kAES128TrafficKey.clone();
    params.recordSeq = 12;
    EXPECT_EQ(params.toSockoptFormat(), nullptr);
  }
}
