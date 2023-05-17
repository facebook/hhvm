/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/fizz-config.h>

#include <folly/portability/GTest.h>

#include <fizz/crypto/aead/AEGISCipher.h>
#include <fizz/crypto/aead/test/TestUtil.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/record/Types.h>
#include <folly/ExceptionWrapper.h>
#include <folly/String.h>

#include <list>
#include <stdexcept>

using namespace folly;

#if FIZZ_HAS_AEGIS
namespace fizz {
namespace test {

struct AEGISCipherParams {
  std::string key;
  std::string iv;
  std::string aad;
  std::string plaintext;
  std::string ciphertext;
  bool valid;
};

class AEGISCipherTest : public ::testing::TestWithParam<AEGISCipherParams> {};

std::unique_ptr<Aead> getTestCipher(const AEGISCipherParams& params) {
  std::unique_ptr<Aead> cipher = AEGISCipher::makeCipher();

  TrafficKey trafficKey;
  trafficKey.key = toIOBuf(params.key);
  trafficKey.iv = toIOBuf(params.iv);
  cipher->setKey(std::move(trafficKey));
  return cipher;
}

TEST_P(AEGISCipherTest, TestEncrypt) {
  auto cipher = getTestCipher(GetParam());
  auto plaintext = toIOBuf(GetParam().plaintext);
  auto aad = toIOBuf(GetParam().aad);
  auto origLength = plaintext->computeChainDataLength();
  auto nonce_iobuf = toIOBuf(GetParam().iv);
  auto nonce = folly::ByteRange(nonce_iobuf->data(), nonce_iobuf->length());
  auto out = cipher->encrypt(std::move(plaintext), aad.get(), nonce);
  bool valid = IOBufEqualTo()(toIOBuf(GetParam().ciphertext), out);
  EXPECT_EQ(valid, GetParam().valid);
  EXPECT_EQ(
      out->computeChainDataLength(), origLength + cipher->getCipherOverhead());
}

TEST_P(AEGISCipherTest, TestDecrypt) {
  auto cipher = getTestCipher(GetParam());
  auto ciphertext = toIOBuf(GetParam().ciphertext);
  auto aad = toIOBuf(GetParam().aad);
  auto nonce_iobuf = toIOBuf(GetParam().iv);
  auto nonce = folly::ByteRange(nonce_iobuf->data(), nonce_iobuf->length());
  auto origLength = ciphertext->computeChainDataLength();
  try {
    auto out = cipher->decrypt(std::move(ciphertext), aad.get(), nonce);
    EXPECT_TRUE(GetParam().valid);
    EXPECT_TRUE(IOBufEqualTo()(toIOBuf(GetParam().plaintext), out));
    EXPECT_EQ(
        out->computeChainDataLength(),
        origLength - cipher->getCipherOverhead());
  } catch (const std::runtime_error&) {
    EXPECT_FALSE(GetParam().valid);
  }
}

// Adapted from libsodium's aegis 128l testing values
INSTANTIATE_TEST_SUITE_P(
    AEGIS128TestVectors,
    AEGISCipherTest,
    ::testing::Values(
        AEGISCipherParams{
            "54662e55bb4771f9711fe5301d7412fe",
            "e51d417ab10a2931d8d22a9fffb98e3a",
            "3b762e3ab5d06cb2896b852ea70303f289f2775401b7808e30272f",
            "04f672f8cdb3e71d032d52c064bc33ecf8aad3d40c41d5806cc306766c057c50b500af5c550d076d34cc3a74a2b4bed195ffa3e8eddf953aefe9aed2bc14349c700ab7e4cb974fb31615a9ff70fb44307055523ab378b133fefc883013ce23bb01b23aeda15f85e65cdf02a291a0454900cb261872d5205737fd7410",
            "d6736371f35eb067244dd7963ad2e0cd3949452cbd4c220be55082498ed3b230f579d78844311652a9958e82f172bb8072c4b1114ec531a6ccb340ddd86caf32a0d4c9c45738e9ec9c0d9154612f7d90465f3a277bebd667c0af0edb6935d8dffbdee96c1a96e4c4318f5d3bc90c1c8d5729e1a402f765bdc9b26b0853c2fd22b035bf3f3658ede47ef11b9d",
            true},
        AEGISCipherParams{
            "46a5c72e03d900b48f829df00ecb88b9",
            "b25187e4b77b6770c35c7a962584597d",
            "b73c81239e01cd81b0de13247ca4e3528b87f3078e2b674a667430b1dbdc3e93657131e654a4182b4c4ab01a33b36e946f1fcc55aab06fc6f56d",
            "fc8083311b38a80c04e57d069661b273264310906781eb7e4e44c6416f7336267674a44a7c54ed6361b43ef9500514e5d9e71f8b5c33aece756b64f3ed011922facbec7c3ffd27d01a853435bde551372806bd0c",
            "51189448af53ae3630c06a167ceefe6b9b5eba746fb9b53f4b3104d2b15b6020fa8998e182eb9c9d6b6463939e50723780f983733206ae6f11b986d95abe83555e64f8d3242d7e8055fcb8e2df8e41d318f06728f9b7e561ac0316d2ed7debc2484cc3e1",
            true},
        AEGISCipherParams{
            "e343d75de99e6d73543968437d3dcf6a",
            "317a5808ed5debf6f527a780e0896b2d",
            "323094c01e",
            "247045cb40dea9c514a885444c526ac867b1b80e4728a23b63f596",
            "18cb5d2fc5e27bdda5ba16f1320da42049759368548e5bd96f2dbc1659e7d7193b12b4c90ba1e4314ef055",
            true},
        AEGISCipherParams{
            "7db9c2721a03931c880f9e714bbf2211",
            "27f642398299ada7fdda1895ee4589f0",
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            true},
        AEGISCipherParams{
            "00000000000000000000000000000000",
            "00000000000000000000000000000000",
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            false}));
} // namespace test
} // namespace fizz
#else
TEST(AegisCipherTest, AegisNotSupported) {}
#endif
