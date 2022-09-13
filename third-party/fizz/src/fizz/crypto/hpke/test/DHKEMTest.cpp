/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <tuple>

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/crypto/exchange/OpenSSLKeyExchange.h>
#include <fizz/crypto/hpke/DHKEM.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/record/Types.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace test {

class MockKeyExchange : public KeyExchange {
 public:
  explicit MockKeyExchange(std::unique_ptr<KeyExchange> actualKex)
      : actualKex_(std::move(actualKex)) {}
  void generateKeyPair() override {}

  std::unique_ptr<folly::IOBuf> getKeyShare() const override {
    return actualKex_->getKeyShare();
  }

  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override {
    return actualKex_->generateSharedSecret(keyShare);
  }

  std::unique_ptr<KeyExchange> clone() const override {
    return nullptr;
  }

  size_t getExpectedKeyShareSize() const override {
    return actualKex_->getExpectedKeyShareSize();
  }

 private:
  std::unique_ptr<KeyExchange> actualKex_;
  folly::ssl::EvpPkeyUniquePtr privateKey_;
};

std::tuple<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>
testEncapDecap(DHKEM dhkem, std::unique_ptr<folly::IOBuf> publicKey) {
  // Generate the DHKEM shared secret and an encapsulation of the key
  DHKEM::EncapResult encapResult = dhkem.encap(publicKey->coalesce());

  // Recover the DHKEM shared secret from its encapsulated representation "enc"
  std::unique_ptr<folly::IOBuf> gotSharedKey =
      dhkem.decap(encapResult.enc->coalesce());

  return std::make_tuple(
      std::move(encapResult.sharedSecret), std::move(gotSharedKey));
}

DHKEM getDHKEM(std::unique_ptr<KeyExchange> actualKex, NamedGroup group) {
  auto prefix = "HPKE-v1";
  auto hkdf = std::make_unique<fizz::hpke::Hkdf>(
      folly::IOBuf::copyBuffer(prefix),
      std::make_unique<HkdfImpl>(HkdfImpl::create<Sha256>()));
  return DHKEM(
      std::make_unique<MockKeyExchange>(std::move(actualKex)),
      group,
      std::move(hkdf));
}

TEST(DHKEMTest, TestP256EncapDecapEqual) {
  auto actualKex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  auto privateKey = getPrivateKey(kP256Key);
  actualKex->setPrivateKey(std::move(privateKey));
  auto dhkem = getDHKEM(std::move(actualKex), NamedGroup::secp256r1);

  auto publicKey = detail::encodeECPublicKey(getPublicKey(kP256PublicKey));
  std::unique_ptr<folly::IOBuf> sharedKey;
  std::unique_ptr<folly::IOBuf> gotSharedKey;
  std::tie(sharedKey, gotSharedKey) =
      testEncapDecap(std::move(dhkem), std::move(publicKey));

  EXPECT_TRUE(folly::IOBufEqualTo()(sharedKey, gotSharedKey));
}

TEST(DHKEMTest, TestP256EncapDecapNotEqual) {
  auto actualKex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  auto privateKey = getPrivateKey(kP256Key);
  actualKex->setPrivateKey(std::move(privateKey));
  auto dhkem = getDHKEM(std::move(actualKex), NamedGroup::secp256r1);

  // Set incorrect public key
  std::unique_ptr<folly::IOBuf> enc;
  constexpr folly::StringPiece encodedShare =
      "048d5e897c896b17e1679766c14c785dd2c328c3"
      "eecc7dbfd2e2e817cd35c786aceea79bf1286ab8"
      "a5c3c464c46f5ba06338b24ea96ce442a4d13356"
      "902dfcd1e9";
  std::string out = unhexlify(encodedShare);
  std::unique_ptr<folly::IOBuf> publicKey =
      folly::IOBuf::wrapBuffer(folly::StringPiece(out));

  std::unique_ptr<folly::IOBuf> sharedKey;
  std::unique_ptr<folly::IOBuf> gotSharedKey;
  std::tie(sharedKey, gotSharedKey) =
      testEncapDecap(std::move(dhkem), std::move(publicKey));

  EXPECT_FALSE(folly::IOBufEqualTo()(sharedKey, gotSharedKey));
}

TEST(DHKEMTest, TestP384EncapDecapEqual) {
  auto actualKex = std::make_unique<OpenSSLECKeyExchange<P384>>();
  auto privateKey = getPrivateKey(kP384Key);
  actualKex->setPrivateKey(std::move(privateKey));
  auto dhkem = getDHKEM(std::move(actualKex), NamedGroup::secp384r1);

  auto publicKey = detail::encodeECPublicKey(getPublicKey(kP384PublicKey));
  std::unique_ptr<folly::IOBuf> sharedKey;
  std::unique_ptr<folly::IOBuf> gotSharedKey;
  std::tie(sharedKey, gotSharedKey) =
      testEncapDecap(std::move(dhkem), std::move(publicKey));

  EXPECT_TRUE(folly::IOBufEqualTo()(sharedKey, gotSharedKey));
}

} // namespace test
} // namespace fizz
