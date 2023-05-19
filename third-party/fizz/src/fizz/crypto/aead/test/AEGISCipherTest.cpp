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
  CipherSuite cipher;
};

class AEGISCipherTest : public ::testing::TestWithParam<AEGISCipherParams> {};

std::unique_ptr<Aead> getTestCipher(const AEGISCipherParams& params) {
  auto cipher = getCipher(params.cipher);

  TrafficKey trafficKey;
  trafficKey.key = toIOBuf(params.key);
  trafficKey.iv = toIOBuf(params.iv);
  cipher->setKey(std::move(trafficKey));
  return cipher;
}

TEST_P(AEGISCipherTest, TestEncrypt) {
  std::unique_ptr<Aead> cipher = getTestCipher(GetParam());
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
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256_EXPERIMENTAL},
        AEGISCipherParams{
            "46a5c72e03d900b48f829df00ecb88b9",
            "b25187e4b77b6770c35c7a962584597d",
            "b73c81239e01cd81b0de13247ca4e3528b87f3078e2b674a667430b1dbdc3e93657131e654a4182b4c4ab01a33b36e946f1fcc55aab06fc6f56d",
            "fc8083311b38a80c04e57d069661b273264310906781eb7e4e44c6416f7336267674a44a7c54ed6361b43ef9500514e5d9e71f8b5c33aece756b64f3ed011922facbec7c3ffd27d01a853435bde551372806bd0c",
            "51189448af53ae3630c06a167ceefe6b9b5eba746fb9b53f4b3104d2b15b6020fa8998e182eb9c9d6b6463939e50723780f983733206ae6f11b986d95abe83555e64f8d3242d7e8055fcb8e2df8e41d318f06728f9b7e561ac0316d2ed7debc2484cc3e1",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256_EXPERIMENTAL},
        AEGISCipherParams{
            "e343d75de99e6d73543968437d3dcf6a",
            "317a5808ed5debf6f527a780e0896b2d",
            "323094c01e",
            "247045cb40dea9c514a885444c526ac867b1b80e4728a23b63f596",
            "18cb5d2fc5e27bdda5ba16f1320da42049759368548e5bd96f2dbc1659e7d7193b12b4c90ba1e4314ef055",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256_EXPERIMENTAL},
        AEGISCipherParams{
            "7db9c2721a03931c880f9e714bbf2211",
            "27f642398299ada7fdda1895ee4589f0",
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256_EXPERIMENTAL},
        AEGISCipherParams{
            "00000000000000000000000000000000",
            "00000000000000000000000000000000",
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            false,
            CipherSuite::TLS_AEGIS_128L_SHA256_EXPERIMENTAL},
        AEGISCipherParams{
            "7083505997f52fdf86548d86ee87c1429ed91f108cd56384dc840269ef7fdd73",
            "18cd778e6f5b1d35d4ca975fd719a17aaf22c3eba01928b6a78bac5810c92c75",
            "af5b16a480e6a1400be15c8e6b194c2aca175e3b5c3f3fbbeca865f9390a",
            "5d6691271eb1b2261d1b34fa7560e274b83373343c2e49b2b6a82bc0f20cee85cd608d195c1a16679d720441c95fae86631f3f2cd27f38f71cedc79aaca7fdddbd4da4eeb97632366db65ca21acd85b41fd1a9de688bddff433a4757eb084e6816dbc8ff93f5995804",
            "0943a3e659b86e267ffea969ddd6d6d63aa35d1a1f31fb6f47205104b132da65799cc64cc9f66ffa5ec479550c2c5dfa006f827ef02e3ab4dae3446bf93ccb5c17e1ec0393f161fca94f2944d041f162e9c964558b6b57d3bb393b9743b1f8338ff878a154800fd16c6eacac942353072bdeb9fcf85e5b6c04",
            true,
            CipherSuite::TLS_AEGIS_256_SHA384_EXPERIMENTAL},
        AEGISCipherParams{
            "c88bb05b2aec1218e1a5026511e6d44de7bd502588e9e2a01591b39c5ead76ff",
            "4a485f226a73f0c4e16242e8234841cdf6af1771eb278e7f35428d03eb5b4cf0",
            "38a9809dbdd2579010d38bf5314f255b",
            "2a4c06941ec356390542d7d7833fd68fc85a00c0452281f87dee6f10180d02182791232c7007fde35dfd5a901afa896296f9f344db717994d078fbd3a4cec8d782d2bdc205f3709827b776fd5c863a952fea97a14a6c2ee3f20432b8baa084470179078bd6a83597478b2fd9ae00ecb424822cb0d61e9a55a4",
            "b8565db06c2fa493e09b6764f4d09296422095eb6e9890f606654713bfee6f362a123688b61f254f315f18b20bcc5ed8b0b4f2224de9f498e3ef03532a8bcddb361f5ace8ff491bab8b3d06550496501264f9f48ebad277e7492146789d0fc1a3b1e3e81598370a4183683d1fee25a9a1fe359c836932746b983d01767ad4b9b3d70cc917fe57e41e0",
            true,
            CipherSuite::TLS_AEGIS_256_SHA384_EXPERIMENTAL},
        AEGISCipherParams{"77b473865175ebd5ddf9c382bac227029c25bdb836e683a138e4618cc964488b", "f183d8de1e6dd4ccefa79fe22fabfda58e68dd29116d13408042f0713a4ee5f8", "0679fd74a846965e33e558676115d843e440fa37092fbd5c57c82fd914210fcf948f911b04632d66be46248d772b3eb9f55b537e54b1ec751b63f035c8", "9888b8ee03c3217a777b7558a31e331909570ea196f02c8cffad2c8dc6499b8125363c06a71c057842666bfb5c6acc937d2eecd960330c2361abdd88a4b191557ddf5102de75ddc7e09aee9862f32e24f1db3847a5f5b379fb32e2ef7ffb0d3a60", "3464d835302583ade6ed99e23333e865d3308f31a6cb65bcefdc9a1b9b4d0e0f75513188480dac4a64922af4441324ce7de74eb9f7f4e414f6177a4814edc96313694b99ff8dd36b2f7f79c7ecd70ec475abe1c1909238767f172fd6b95e92c025b1f8c9704d7b845964e14ccb333f0d4b", true, CipherSuite::TLS_AEGIS_256_SHA384_EXPERIMENTAL},
        AEGISCipherParams{
            "b8c6e8cea59ca9fd2922530ee61911c1ed1c5af98be8fb03cbb449adcea0ed83",
            "af5bc1abe7bafadee790390277874cdfcc1ac1955f249d1131555d345832f555",
            "d899366a0b4e4d86cce5ba61aca2a84349c8de5757e008e94e7d7a3703",
            "b6c15f560be043d06aa27e15d8c901af6b19db7a15e1",
            "4c8496dfa6c419ef3c4867769a9014bd17118c22eef5f0f7ed5cb9ba59df21310c274cf9a585",
            true,
            CipherSuite::TLS_AEGIS_256_SHA384_EXPERIMENTAL},
        AEGISCipherParams{
            "0000000000000000000000000000000000000000000000000000000000000000",
            "0000000000000000000000000000000000000000000000000000000000000000",
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            false,
            CipherSuite::TLS_AEGIS_256_SHA384_EXPERIMENTAL}));
} // namespace test
} // namespace fizz
#else
TEST(AegisCipherTest, AegisNotSupported) {}
#endif
