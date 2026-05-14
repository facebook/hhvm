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
#include <fizz/util/Status.h>
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
  Error err;
  EXPECT_EQ(kex->generateKeyPair(err), Status::Success);
}

TYPED_TEST(Key, SharedSecret) {
  auto kex = TestFixture::makeKex();
  Error err;
  EXPECT_EQ(kex->generateKeyPair(err), Status::Success);
  std::unique_ptr<folly::IOBuf> shared;
  EXPECT_EQ(
      kex->generateSharedSecret(shared, err, kex->getPrivateKey()),
      Status::Success);
  EXPECT_TRUE(shared);
}

TYPED_TEST(Key, ReadFromKey) {
  auto kex = TestFixture::makeKex();
  auto pkey = getPrivateKey(this->getKeyParams().privateKey);
  Error err;
  EXPECT_EQ(kex->setPrivateKey(err, std::move(pkey)), Status::Success);

  auto pkey2 = getPrivateKey(this->getKeyParams().privateKey);
  auto kex2 = TestFixture::makeKex();
  EXPECT_EQ(kex2->setPrivateKey(err, std::move(pkey2)), Status::Success);
  std::unique_ptr<folly::IOBuf> shared;
  EXPECT_EQ(
      kex->generateSharedSecret(shared, err, kex2->getPrivateKey()),
      Status::Success);
  EXPECT_TRUE(shared);
}

TYPED_TEST(Key, ReadWrongGroup) {
  auto pkey = getPrivateKey(this->getKeyParams().invalidPrivateKey);
  auto kex = TestFixture::makeKex();
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(kex->setPrivateKey(err, std::move(pkey)), err),
      std::runtime_error);
}

TYPED_TEST(Key, Decode) {
  std::string out = unhexlify(this->getKeyParams().encodedShare);
  folly::ssl::EvpPkeyUniquePtr pub;
  Error err;
  EXPECT_EQ(
      detail::OpenSSLECKeyDecoder::decode(
          pub, err, range(out), openssl::Properties<TypeParam>::curveNid),
      Status::Success);
  EXPECT_TRUE(pub);
}

TYPED_TEST(Key, Encode) {
  std::string out = unhexlify(this->getKeyParams().encodedShare);
  folly::ssl::EvpPkeyUniquePtr pub;
  Error err;
  EXPECT_EQ(
      detail::OpenSSLECKeyDecoder::decode(
          pub, err, range(out), openssl::Properties<TypeParam>::curveNid),
      Status::Success);
  EXPECT_TRUE(pub);
  std::unique_ptr<folly::IOBuf> encoded;
  EXPECT_EQ(
      detail::OpenSSLECKeyEncoder::encode(encoded, err, pub), Status::Success);

  auto encodedStr = encoded->moveToFbString();
  EXPECT_EQ(encodedStr, out);
}

TYPED_TEST(Key, DecodeInvalid) {
  std::string out = unhexlify(this->getKeyParams().invalidEncodedShare);
  Error err;
  EXPECT_THROW(
      {
        folly::ssl::EvpPkeyUniquePtr pub;
        FIZZ_THROW_ON_ERROR(
            detail::OpenSSLECKeyDecoder::decode(
                pub, err, range(out), openssl::Properties<TypeParam>::curveNid),
            err);
      },
      std::runtime_error);
}

TYPED_TEST(Key, DecodeInvalidSmallLength) {
  std::string out = unhexlify(this->getKeyParams().tooSmallEncodedShare);
  Error err;
  EXPECT_THROW(
      {
        folly::ssl::EvpPkeyUniquePtr pub;
        FIZZ_THROW_ON_ERROR(
            detail::OpenSSLECKeyDecoder::decode(
                pub, err, range(out), openssl::Properties<TypeParam>::curveNid),
            err);
      },
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
  FIZZ_CHECK_EQ(1, EVP_PKEY_set1_EC_KEY(pkeyPeerKey.get(), peerKey.get()));

  return pkeyPeerKey;
}

TEST_P(ECDHTest, TestKeyAgreement) {
  try {
    auto privateKey = getKey(GetParam());
    ASSERT_TRUE(privateKey);

    auto pkeyPeerKey = createPublicKey(GetParam());

    std::unique_ptr<folly::IOBuf> shared;
    Error err;
    switch (GetParam().key) {
      case fizz::KeyType::P256: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P256>();
        FIZZ_THROW_ON_ERROR(
            kex->setPrivateKey(err, std::move(privateKey)), err);
        FIZZ_THROW_ON_ERROR(
            kex->generateSharedSecret(shared, err, pkeyPeerKey), err);
        break;
      }
      case fizz::KeyType::P384: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P384>();
        FIZZ_THROW_ON_ERROR(
            kex->setPrivateKey(err, std::move(privateKey)), err);
        FIZZ_THROW_ON_ERROR(
            kex->generateSharedSecret(shared, err, pkeyPeerKey), err);
        break;
      }
      case fizz::KeyType::P521: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P521>();
        FIZZ_THROW_ON_ERROR(
            kex->setPrivateKey(err, std::move(privateKey)), err);
        FIZZ_THROW_ON_ERROR(
            kex->generateSharedSecret(shared, err, pkeyPeerKey), err);
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
    Error err;
    switch (GetParam().key) {
      case fizz::KeyType::P256: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P256>();
        FIZZ_THROW_ON_ERROR(
            kex->setPrivateKey(err, std::move(privateKey)), err);

        FIZZ_THROW_ON_ERROR(kex->clone(chosenKex, err), err);
        break;
      }
      case fizz::KeyType::P384: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P384>();
        FIZZ_THROW_ON_ERROR(
            kex->setPrivateKey(err, std::move(privateKey)), err);

        FIZZ_THROW_ON_ERROR(kex->clone(chosenKex, err), err);
        break;
      }
      case fizz::KeyType::P521: {
        auto kex = makeOpenSSLECKeyExchange<fizz::P521>();
        FIZZ_THROW_ON_ERROR(
            kex->setPrivateKey(err, std::move(privateKey)), err);

        FIZZ_THROW_ON_ERROR(kex->clone(chosenKex, err), err);
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

    auto encodedPubKey = std::unique_ptr<folly::IOBuf>{};
    FIZZ_THROW_ON_ERROR(
        detail::encodeECPublicKey(encodedPubKey, err, pkeyPeerKey), err);
    std::unique_ptr<folly::IOBuf> shared;

    EXPECT_EQ(
        chosenKex->generateSharedSecret(shared, err, encodedPubKey->coalesce()),
        Status::Success);

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
