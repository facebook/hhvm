/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/backend/openssl/crypto/exchange/OpenSSLKeyExchange.h>
#include <fizz/crypto/test/TestKeys.h>
#include <fizz/crypto/test/TestUtil.h>
#include <folly/String.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

#include <openssl/ec.h>

using namespace folly;
using namespace folly::ssl;

namespace fizz {
using namespace test;

namespace openssl {
namespace test {

template <class T>
class Key : public ::testing::Test {
 public:
  static constexpr auto makeKex = fizz::openssl::makeOpenSSLECKeyExchange<T>;
  KeyParams (*getKeyParams)() = fizz::test::getKeyParams<T>;
};

using KeyTypes = ::testing::Types<fizz::P256, fizz::P384, fizz::P521>;
TYPED_TEST_SUITE(Key, KeyTypes);

TYPED_TEST(Key, GenerateKey) {
  auto kex = TestFixture::makeKex();
  kex->generateKeyPair();
}

TYPED_TEST(Key, SharedSecret) {
  auto kex = TestFixture::makeKex();
  kex->generateKeyPair();
  auto shared = kex->generateSharedSecret(kex->getPrivateKey());
  EXPECT_TRUE(shared);
}

TYPED_TEST(Key, ReadFromKey) {
  auto kex = TestFixture::makeKex();
  auto pkey = getPrivateKey(this->getKeyParams().privateKey);
  kex->setPrivateKey(std::move(pkey));

  auto pkey2 = getPrivateKey(this->getKeyParams().privateKey);
  auto kex2 = TestFixture::makeKex();
  kex2->setPrivateKey(std::move(pkey2));
  auto shared = kex->generateSharedSecret(kex2->getPrivateKey());
  EXPECT_TRUE(shared);
}

TYPED_TEST(Key, ReadWrongGroup) {
  auto pkey = getPrivateKey(this->getKeyParams().invalidPrivateKey);
  auto kex = TestFixture::makeKex();
  EXPECT_THROW(kex->setPrivateKey(std::move(pkey)), std::runtime_error);
}

TYPED_TEST(Key, Decode) {
  std::string out = unhexlify(this->getKeyParams().encodedShare);
  auto pub = detail::OpenSSLECKeyDecoder::decode(
      range(out), openssl::Properties<TypeParam>::curveNid);
  EXPECT_TRUE(pub);
}

TYPED_TEST(Key, Encode) {
  std::string out = unhexlify(this->getKeyParams().encodedShare);
  auto pub = detail::OpenSSLECKeyDecoder::decode(
      range(out), openssl::Properties<TypeParam>::curveNid);
  EXPECT_TRUE(pub);
  auto encoded = detail::OpenSSLECKeyEncoder::encode(pub);

  auto encodedStr = encoded->moveToFbString();
  EXPECT_EQ(encodedStr, out);
}

TYPED_TEST(Key, DecodeInvalid) {
  std::string out = unhexlify(this->getKeyParams().invalidEncodedShare);
  EXPECT_THROW(
      detail::OpenSSLECKeyDecoder::decode(
          range(out), openssl::Properties<TypeParam>::curveNid),
      std::runtime_error);
}

TYPED_TEST(Key, DecodeInvalidSmallLength) {
  std::string out = unhexlify(this->getKeyParams().tooSmallEncodedShare);
  EXPECT_THROW(
      detail::OpenSSLECKeyDecoder::decode(
          range(out), openssl::Properties<TypeParam>::curveNid),
      std::runtime_error);
}

class ECDHTest : public ::testing::TestWithParam<Params> {};

int getNid(const Params& param) {
  switch (param.key) {
    case fizz::KeyType::P256:
      return openssl::Properties<P256>::curveNid;
    case fizz::KeyType::P384:
      return openssl::Properties<P384>::curveNid;
    case fizz::KeyType::P521:
      return openssl::Properties<P521>::curveNid;
    default:
      throw std::runtime_error("invalid key type");
  }
}

void setPoint(EcKeyUniquePtr& key, std::string x, std::string y) {
  auto binX = unhexlify(x);
  auto binY = unhexlify(y);
  BIGNUMUniquePtr numX(BN_bin2bn((uint8_t*)binX.data(), binX.size(), nullptr));
  BIGNUMUniquePtr numY(BN_bin2bn((uint8_t*)binY.data(), binY.size(), nullptr));
  EC_KEY_set_public_key_affine_coordinates(key.get(), numX.get(), numY.get());
}

EvpPkeyUniquePtr getKey(const Params& param) {
  auto privKeyBin = unhexlify(param.priv);
  BIGNUMUniquePtr privateBn(
      BN_bin2bn((uint8_t*)privKeyBin.c_str(), privKeyBin.size(), nullptr));
  EcKeyUniquePtr privateKey(EC_KEY_new_by_curve_name(getNid(param)));
  EC_KEY_set_private_key(privateKey.get(), privateBn.get());
  setPoint(privateKey, param.privX, param.privY);
  EvpPkeyUniquePtr pkeyPrivateKey(EVP_PKEY_new());
  EVP_PKEY_set1_EC_KEY(pkeyPrivateKey.get(), privateKey.get());
  return pkeyPrivateKey;
}

void checkShared(std::unique_ptr<folly::IOBuf> shared, const Params& param) {
  ASSERT_TRUE(shared);
  auto sharedString = shared->moveToFbString();
  auto hexShared = hexlify(sharedString);
  if (param.success) {
    EXPECT_EQ(param.shared, hexShared);
  } else {
    EXPECT_NE(param.shared, hexShared);
  }
}

EvpPkeyUniquePtr createPublicKey(const Params& param) {
  // Create the peer key
  EcKeyUniquePtr peerKey(EC_KEY_new_by_curve_name(getNid(param)));
  setPoint(peerKey, param.peerX, param.peerY);

  EvpPkeyUniquePtr pkeyPeerKey(EVP_PKEY_new());
  CHECK_EQ(1, EVP_PKEY_set1_EC_KEY(pkeyPeerKey.get(), peerKey.get()));

  return pkeyPeerKey;
}

TEST_P(ECDHTest, TestKeyAgreement) {
  try {
    auto privateKey = getKey(GetParam());
    ASSERT_TRUE(privateKey);

    auto pkeyPeerKey = createPublicKey(GetParam());

    std::unique_ptr<folly::IOBuf> shared;
    switch (GetParam().key) {
      case fizz::KeyType::P256: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P256>();
        kex->setPrivateKey(std::move(privateKey));
        shared = kex->generateSharedSecret(pkeyPeerKey);
        break;
      }
      case fizz::KeyType::P384: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P384>();
        kex->setPrivateKey(std::move(privateKey));
        shared = kex->generateSharedSecret(pkeyPeerKey);
        break;
      }
      case fizz::KeyType::P521: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P521>();
        kex->setPrivateKey(std::move(privateKey));
        shared = kex->generateSharedSecret(pkeyPeerKey);
        break;
      }
      default:
        throw std::runtime_error("unknown key type");
    }

    checkShared(std::move(shared), GetParam());
  } catch (const std::runtime_error& ex) {
    EXPECT_FALSE(GetParam().success) << ex.what();
  }
}

TEST_P(ECDHTest, TestKexClone) {
  try {
    auto privateKey = getKey(GetParam());
    ASSERT_TRUE(privateKey);

    auto pkeyPeerKey = createPublicKey(GetParam());

    std::unique_ptr<KeyExchange> chosenKex;
    switch (GetParam().key) {
      case fizz::KeyType::P256: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P256>();
        kex->setPrivateKey(std::move(privateKey));

        chosenKex = kex->clone();
        break;
      }
      case fizz::KeyType::P384: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P384>();
        kex->setPrivateKey(std::move(privateKey));

        chosenKex = kex->clone();
        break;
      }
      case fizz::KeyType::P521: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P521>();
        kex->setPrivateKey(std::move(privateKey));

        chosenKex = kex->clone();
        break;
      }
      default:
        throw std::runtime_error("unknown key type");
    }

    folly::ssl::EcKeyUniquePtr ecKey(EVP_PKEY_get1_EC_KEY(pkeyPeerKey.get()));
    auto point = EC_KEY_get0_public_key(ecKey.get());
    if (!point) {
      throw std::runtime_error("public key invalid");
    }

    auto encodedPubKey = detail::encodeECPublicKey(pkeyPeerKey);
    auto shared = chosenKex->generateSharedSecret(encodedPubKey->coalesce());

    checkShared(std::move(shared), GetParam());
  } catch (const std::runtime_error& ex) {
    EXPECT_FALSE(GetParam().success) << ex.what();
  }
}

INSTANTIATE_TEST_SUITE_P(
    TestP256Vectors,
    ECDHTest,
    ::testing::ValuesIn(kP256KeyParams));
INSTANTIATE_TEST_SUITE_P(
    TestP384Vectors,
    ECDHTest,
    ::testing::ValuesIn(kP384KeyParams));
INSTANTIATE_TEST_SUITE_P(
    TestP521Vectors,
    ECDHTest,
    ::testing::ValuesIn(kP521KeyParams));

} // namespace test
} // namespace openssl
} // namespace fizz
