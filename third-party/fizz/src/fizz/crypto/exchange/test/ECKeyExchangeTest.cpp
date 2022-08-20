/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/exchange/ECCurveKeyExchange.h>
#include <fizz/crypto/signature/Signature.h>
#include <fizz/crypto/test/TestUtil.h>
#include <folly/String.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

#include <openssl/ec.h>

using namespace folly;
using namespace folly::ssl;

namespace fizz {
namespace test {

struct KeyParams {
  StringPiece privateKey;
  StringPiece invalidPrivateKey;
  // Obtained from a key share from firefox nightly for p256
  // For others openssl ec -in {KEY} -pubout -text
  StringPiece encodedShare;
  // Just fudged the previous share slightly
  StringPiece invalidEncodedShare;
  StringPiece tooSmallEncodedShare;
};

template <class T>
class Key : public ::testing::Test {
 public:
  using KeyExch = OpenSSLECKeyExchange<T>;
  using KeyEncoder = detail::OpenSSLECKeyEncoder;
  using KeyDecoder = detail::OpenSSLECKeyDecoder<T>;

  KeyParams getKeyParams();
};

template <>
KeyParams Key<P256>::getKeyParams() {
  return KeyParams{
      kP256Key,
      kP256K1Key,
      "048d5e897c896b17e1679766c14c785dd2c328c3"
      "eecc7dbfd2e2e817cd35c786aceea79bf1286ab8"
      "a5c3c464c46f5ba06338b24ea96ce442a4d13356"
      "902dfcd1e9",
      "048d5e897c896b17e1679766c14c785dd2c328c3"
      "eecc7dbfd2e2e817cd35c786abeea79bf1286ab8"
      "a5c3c464c46f5ba06338b24ea96ce442a4d13356"
      "902dfcd1e9",
      "048d5e897c896b17e1679766c14c785dd2c328c3"};
}

template <>
KeyParams Key<P384>::getKeyParams() {
  return KeyParams{
      kP384Key,
      kP256Key,
      "04811041962bbac9859eb2aa19a475"
      "2e573949d3f1eb8abcb131c1674e75"
      "061f5e17b6a24724eeb73268ed82fd"
      "3d2bfc93db88cabbf8c9da9e4c1479"
      "ffdb9a54b522990aef401c1ab004aa"
      "1fdf0e26aa8692ca5860ad6f6cf8ea"
      "3bb95dbecb82a3",
      "04811041962bbac9859eb2aa19a475"
      "2e573949d3f1eb8abcb131c1674e75"
      "061f5e17b6a24724eeb73268ed82fd"
      "3d2bfc93db88cabbf8c9da9e4c1479"
      "ffdb9a54b522990aef401c1ab004aa"
      "1fdf0e26aa8621ca5860ad6f6cf8ea"
      "3bb95dbecb82a3",
      "3bb95dbecb82a3"};
}

template <>
KeyParams Key<P521>::getKeyParams() {
  return KeyParams{
      kP521Key,
      kP384Key,
      "0401a76d5b0c42f557e418bf982b35"
      "6e903f937b9d530cca8250e07c98ca"
      "03293958b2fa8da20f9c4e9f7684df"
      "3e872b470dac5368697b664ce4c677"
      "3feb8862707b5f01434182eb962a3a"
      "a8a707c868caa6edee62dd8456172e"
      "f0bd738fac3abb04e87bd3e9ea9c1b"
      "d09f8720a994c6162cb42546f86e32"
      "8de930387e8ecc2c68ec41bcf2",
      "0401a76d5b0c42f557e418bf982b35"
      "6e903f937b9d530cca8250e07c98bd"
      "03293958b2fa8da20f9c4e9f7684df"
      "3e872b470dac5368697b664ce4c677"
      "3feb8862707b5f01434182eb962a3a"
      "a8a707c868caa6edee62dd8456172e"
      "f0bd738fac3abb04e87bd3e9ea9c1b"
      "d09f8720a994c6162cb42546f86e32"
      "8de930387e8ecc2c68ec41bcf2",
      "8de930387e8ecc2c68ec41bcf2"};
}
using KeyTypes = ::testing::Types<P256, P384, P521>;
TYPED_TEST_SUITE(Key, KeyTypes);

TYPED_TEST(Key, GenerateKey) {
  typename TestFixture::KeyExch kex;
  kex.generateKeyPair();
}

TYPED_TEST(Key, SharedSecret) {
  typename TestFixture::KeyExch kex;
  kex.generateKeyPair();
  auto shared = kex.generateSharedSecret(kex.getPrivateKey());
  EXPECT_TRUE(shared);
}

TYPED_TEST(Key, ReadFromKey) {
  typename TestFixture::KeyExch kex;
  auto pkey = getPrivateKey(this->getKeyParams().privateKey);
  kex.setPrivateKey(std::move(pkey));

  auto pkey2 = getPrivateKey(this->getKeyParams().privateKey);
  typename TestFixture::KeyExch kex2;
  kex2.setPrivateKey(std::move(pkey2));
  auto shared = kex.generateSharedSecret(kex2.getPrivateKey());
  EXPECT_TRUE(shared);
}

TYPED_TEST(Key, ReadWrongGroup) {
  auto pkey = getPrivateKey(this->getKeyParams().invalidPrivateKey);
  typename TestFixture::KeyExch kex;
  EXPECT_THROW(kex.setPrivateKey(std::move(pkey)), std::runtime_error);
}

TYPED_TEST(Key, Decode) {
  std::string out = unhexlify(this->getKeyParams().encodedShare);
  auto pub = detail::OpenSSLECKeyDecoder<TypeParam>::decode(range(out));
  EXPECT_TRUE(pub);
}

TYPED_TEST(Key, Encode) {
  std::string out = unhexlify(this->getKeyParams().encodedShare);
  auto pub = detail::OpenSSLECKeyDecoder<TypeParam>::decode(range(out));
  EXPECT_TRUE(pub);
  auto encoded = detail::OpenSSLECKeyEncoder::encode(pub);

  auto encodedStr = encoded->moveToFbString();
  EXPECT_EQ(encodedStr, out);
}

TYPED_TEST(Key, DecodeInvalid) {
  std::string out = unhexlify(this->getKeyParams().invalidEncodedShare);
  EXPECT_THROW(
      detail::OpenSSLECKeyDecoder<TypeParam>::decode(range(out)),
      std::runtime_error);
}

TYPED_TEST(Key, DecodeInvalidSmallLength) {
  std::string out = unhexlify(this->getKeyParams().tooSmallEncodedShare);
  EXPECT_THROW(
      detail::OpenSSLECKeyDecoder<TypeParam>::decode(range(out)),
      std::runtime_error);
}

struct Params {
  std::string peerPriv; // dsCAVS
  std::string peerX; // QsCAVSx
  std::string peerY; // QsCAVSy
  std::string priv; // dIUT
  std::string privX; // dIUTx
  std::string privY; // dIUTy
  std::string shared; // Z
  bool success;
  KeyType key;
};

class ECDHTest : public ::testing::TestWithParam<Params> {};

int getNid(const Params& param) {
  switch (param.key) {
    case KeyType::P256:
      return P256::curveNid;
    case KeyType::P384:
      return P384::curveNid;
    case KeyType::P521:
      return P521::curveNid;
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
      case KeyType::P256: {
        P256KeyExchange kex;
        kex.setPrivateKey(std::move(privateKey));
        shared = kex.generateSharedSecret(pkeyPeerKey);
        break;
      }
      case KeyType::P384: {
        P384KeyExchange kex;
        kex.setPrivateKey(std::move(privateKey));
        shared = kex.generateSharedSecret(pkeyPeerKey);
        break;
      }
      case KeyType::P521: {
        P521KeyExchange kex;
        kex.setPrivateKey(std::move(privateKey));
        shared = kex.generateSharedSecret(pkeyPeerKey);
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
      case KeyType::P256: {
        P256KeyExchange kex;
        kex.setPrivateKey(std::move(privateKey));

        chosenKex = kex.clone();
        break;
      }
      case KeyType::P384: {
        P384KeyExchange kex;
        kex.setPrivateKey(std::move(privateKey));

        chosenKex = kex.clone();
        break;
      }
      case KeyType::P521: {
        P521KeyExchange kex;
        kex.setPrivateKey(std::move(privateKey));

        chosenKex = kex.clone();
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

/***
 * Test vectors sourced from
 * https://github.com/pyca/cryptography/blob/1a6628e55126ec1c98c98a46c04f777f77eff934/vectors/cryptography_vectors/asymmetric/ECDH/KASValidityTest_ECCStaticUnified_NOKC_ZZOnly_init.fax
 * These are NIST test vectors.
 */
// clang-format off
INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    ECDHTest,
    ::testing::
        Values(
            Params{
                "e19007eb245d6995ffa8b3ede1e59193aa6cfaaf4cc1e1d126948610c9b3f44c",
                "758b2f0e79a3d0a94f521ae31dcff50fabd394bb4bbec8fa37d1566f463444e7",
                "b981e686e53e9e9dc2e3f263e810c89b4c271e62392f59ed45ed30ac3a5bfd33",
                "8171000763de347d0eb650dd6fddac2ad48ec122c162d66c3df257aea13192fb",
                "c22ac2ee50e771a93b2b6a42c5e9b76b45a56e0d0011e34aa790283ede61f3d9",
                "0ef754edae5e79c518f1056aa5179cbb6a3a4b7c9654b5048f4259bd2597e57d",
                "5cbea453310285b22f128178bd09b906fde9e660b5a17a7cec809a5a9a1e9287",
                true,
                KeyType::P256},
            Params{
                "0ced658b6113979f8d05fd7b305ce0b8d70f45034d021b052cbcb318e0cfd602",
                "acbcb31f5f6798a00f28aa4a634873744768db612925336efca98122a76d1b5e",
                "7dcefeb3ccb530029a8b62e5a7f00c42fc7ebeac8f469c289ea77b6186d661f0",
                "64e23f7a2d279930f1de66b4bc147786b168d059f581268c24f6650362246e63",
                "ba393b401354aa9552c4289b7a55288d97590429a4003913a243081bacf88acf",
                "d089687aa5442684d71b805ea2b36f6c1c783833346dfdd8208768ed2a7e767d",
                "f70e4fc9ba68aafe07be1767620e64dd5e5bb7ab279f0657465cddeb69e36fa9",
                true,
                KeyType::P256},
            Params{
                "0ced658b6113979f8d05fd7b305ce0b8d70f45034d021b052cbcb318e0cfd602",
                "758b2f0e79a3d0a94f521ae31dcff50fabd394bb4bbec8fa37d1566f463444e7",
                "b981e686e53e9e9dc2e3f263e810c89b4c271e62392f59ed45ed30ac3a5bfd33",
                "8171000763de347d0eb650dd6fddac2ad48ec122c162d66c3df257aea13192fb",
                "c22ac2ee50e771a93b2b6a42c5e9b76b45a56e0d0011e34aa790283ede61f3d9",
                "0ef754edae5e79c518f1056aa5179cbb6a3a4b7c9654b5048f4259bd2597e57d",
                "5cbea453310285b22f128178bd09b906fde9e660b5a17a7cec809a5a9a1e9287",
                true,
                KeyType::P256},
            Params{
                "639ef9ee75a3888617fdd7ed89d62f7398b0eb4f20ccbd35026e150fc937c927",
                "1d2dda4a3735be1f3aedfa8a7bb1410c3867c5d67f55a3dd5376b137352f113d",
                "eca92fb210b1813f51ea2483ff461eb24786afb41f1a00870cf65aab5bbd725e",
                "e062138981049c3b4b964fa5a28569e0142c2c51d6ca0bebdb3270e2ab77fb30",
                "9aa8dd75f7d929b1f5f123aa9f3265be34f771c20bb50deea684a139a10938f8",
                "2b74f503fa7b08db1c76d97c2e571cb91f68a93413daf102c47fee1b8a264d93",
                "9f5f64d76f9bb2f2af24debdd47323d5df9d2b84fc7c7aac1f6d41678adec7de",
                false,
                KeyType::P256},
            Params{
                "305dfb4a8850cc59280891147baf457bfe5e2bae984571634a77dc8d3472fa9b",
                "202cb5a224e6c2a84e624094486edf04116c8d68ec1f4a0e0ed9ee090e1a900b",
                "cacf3a5789bb33954be600425d62d9eae5371f90f88167258814213e4a4f4b1a",
                "72cc52808f294b64b6f7233c3d2f5d96cc1d29287320e39e1c151deef0bc14eb",
                "49a768c9a4ca56e374f685dd76a461b1016c59dcded2c8d8cbd9f23ca453831f",
                "b1e3bb9b5f12a3b5ae788535d4554bd8c46e0e6130075e4e437d3854cf8f1c34",
                "c0147c3c2691b450b5edc08b51aea224d9f4359ff67aab6da3146f396dbceaea",
                false,
                KeyType::P256},
            /*
             * P384
             */
            Params{
                "0e5c98ff2d2a3aab14ad0067b60dbe64e4f541ab5bed11c5a0c55ae1e60b51ff5faaf377837977d80cbfdc33c2ff542b",
                "d1bf2ac21637d66d6398aac01dcd56ac6f065fb45d1f6f16747bab9e9b01b4630b59b20927aea147355bf41838acb482",
                "4c9e23f1c5a41647d094086bf4ed31708651f21d996c47780688ac10f77deee2e43b5241b6caecd2fd5444bc50472e0e",
                "f865418473e5bf7d2e1bbcd9bd5a9270c003a9dd35e778133ca59fcab4bb64fe24d6800e7047bdd033abc8bfa8db35b5",
                "32b72ab9b558249dcbc6cbade234f58e4f7aa5d3f6420ea99a5f997e8c2a91fb7fd83779d0d2169428683771c745fd1a",
                "c749e02a3719bb56bf1dfc4ba3820309c01ab6e84cb29db7cdd80f127233f5295687f8178f3a8704c1063b84c2ee472f",
                "a781430e6078a179df3f9ee27cd8fdc6188f161b6c4ccc4053ef6c6ca6fc222946883a53c06db08f0a020023ced055aa",
                true,
                KeyType::P384},
            Params{
                "925065e50f7eae1319865720ae483de4dba7449dc144c4b50b8e96c4f4fcd9f8b3fbf693262429c91c69d8dd8355fdd8",
                "4c619593e459e4aca7c572834064134d93e8b1b676388b0ff67b893722cf7e8396cd953df38a42e4131dbad29518b2af",
                "dbe261af918baade39070e88f62eabe4ee9d5f2563707271cc566dd7a2380aa4707d15d6eef142a52d75a67429afd0c6",
                "c40c805488115e43279c99797bcbfff8d9922ea9aec471d9ba659590034c3fb739d23661c0aae6e7ada2a1de36eb63a1",
                "a95b7ad4de234e9be8b260313ddb96a404b75b3a7e0f234981f3b189d86ac31bbe3937776c0608024871ef64815784b3",
                "4890ee227219ca5ce34632cbca5732accc2d276fa1b1cc9d14f6b7e4b0f5e116a92cbef743de3e69537e250f27125e82",
                "128934db846099c8d8caa788b0b84bb65bd57b20774ff7f8e7f29939297d72bbc232c5d2b2c8ef1d63e3d75d3908e778",
                true,
                KeyType::P384},
            Params{
                "259c53eec19d01d19428f6cc5cd445ab685578c53325f6d2f157bd0328deca4ffb8c21dca891d59e34a807203f7e9a5d",
                "d18832f4cf3013e514b4fb4c42bbeb15708e20ed91b982598deb2d67b37275587fcb35cf214039ec4fd1834d704c1faf",
                "917c49465bef063949957256eddbece762090ef10f8c48566bd71dc0ad9eb3bc00f0b3f6a88d614210804dd633ca6e68",
                "3120ee90dcce02bac90a66d30ad602e94edcce482dbc5245a136df1d0509b04f6ee305fedd290775c71f1631684f5c0a",
                "7a3e0fd767acf409cebe6fe0222974438bccdefb13f0d668b8daad43a6fdbdf46845c45b9594ffd6742a615dd834f1a5",
                "e3cc9f7153b6ffd3b285349996b952905466942d17f6a9685a11318b0c2c72904961d246cbe3ef98903dbbfb0fbe55b0",
                "5c8d476518873208e1c97685c033a1ba6ec279b8674060cc1a99469908604353043bf488109150b9b2f9b3b1440a426c",
                true,
                KeyType::P384},
            Params{
                "4e422a76b085ae939a8166afb98e344b9ac30bab610e90b3d7449de0a0de6b88fadb095e4818521bb1473540a7ec73ee",
                "de464ed2129eef42ef23cb1bfad27382293edad91d867b1b8b82a19bd17516daf0a73548787a733d66ca4f4d40b52da1",
                "f07bd6003229de37d6293513cbace59ebc24efb571c065df2bc1465c4570928fd612a59a46805b6adce7fb94e36ad043",
                "f8f6697507aeec09fe024f974e00e67f12a14c130df875732b450b3ae8d2cbbb1fcf46bac9207e89a297352eeade7df5",
                "c1db2e81eca0e5b02e8736ff351aa8e59bdadd1c9c7428faf93493c7b93153ec2c55964a157025ff6ddb4523d8e0a453",
                "98daa370a6610ecf604e38e1e70b7f57657c49908eb15634a3a65a175ca7b8cfe1ad535037928ae95c80107a1e018b3c",
                "d80415977c3ce1737fd574596e981c767ecaf5c70084fae6c2072f0ccecc8bdda54a0bf44357981303a2ecc78b26e97c",
                false,
                KeyType::P384},
            Params{
                "4ee6a24b5dca82b215c92cb298d5ac24b77820c7b7e9c8fe4d05d7a297b8f6be40f8d8cf09a4bb238cfdbe05f851c493",
                "1f9af3b14439d4c56b353b5892dd85b6a27053e1d23f5f05ff95ea3fa942eb463e1074b01828a79c865b16fde0fc0f24",
                "01a81aaef502fe84508b054924d90078f68d8d4e9ae5299bd0ea60a97167c549011f4a024e964292d378e82b6874caad",
                "3d715cd0d6490ba5607e7eae2c74361be65de413f10576c1d996658e20c78f8110c70104067922d48303f6ccb1b23f46",
                "2fa7d2ea93945b41c929279a750ded758d9db93d120b5eeda3d9994e36e3381f653d19dcae0eccb1a12153ce4025a62e",
                "04c6e45983485529e9dacba8d609cffaec849f7c5e5cf4a025ce2c0ff9ae6679b4564b9af58fab58c7f92837597b830b",
                "5e77e466aeae9c10ff8d255e07cf0ad9eae50c8a6fd2c7cd889d73d968c423a15c0462b05bf985cbf679a629e380ce80",
                false,
                KeyType::P384},
            /*
             * P521
             */
            Params{
                "0000002fef62381162942889a6094a6bb9ac1f4ddf66d9cda9f618232d31b90c50d7da78a47ed91d40cae946898571db972dc294b109815f38feee9eaac0d5f7c3250728",
                "0000004b05ffa025113390797f2736174aa1c784f4dd34e764ee40d40e4d2442677ebea3498086c9473e5c92789cbdb02bb327bbd61d58690f6a83d9ca73bccbde37dec4",
                "0000004da67cffc98070b82af61feba78787efefb13bd810d80ff92304788e49a4e5b634b3565474a8ecb1615d7b1b77a7a27875adb73a8a5d8f3f84e5e8b744cda250b0",
                "00000311a5e520e238141527671a38cb6f776d96a9f82ef70dffa11dc0895f4060f1abbb9ad6fd259e4a7beaf5f7266ea1bb45bcbfebfda2705e5c551e710fb1d745f57e",
                "0000010ba3778cb2cc965834c0a9593adc6a222692656d657fb0d15293edf0ab33762384a96a16fddea7540b7ccbcca46ec4ac9bcf95fdb5aa18e158aab4d91981bd733e",
                "0000018522df93ddd636e5bc94daecdc600fa241686ec18634fd30b7cbdfdc9ffba1166ac08df34a31896f6fad191414929261ebd7187afb72919f8a0c926be37f99c1e5",
                "01a5e4b31be4b1346e53906b6767b1fe94ec1a8a5abc28fb6f01518c056959af3bc9335dddab178b52318cc5512559931b8dc18de0ce810c2c7f15769d7ce70e719c",
                false,
                KeyType::P521},
            Params{
                "0000013262b7f4f36de274b2c60e4a812866740e545186953f798ac07b5d74e4f0cc1123a9bfcbf77850e2d856246065c54a2437a2816a7d79d6180a9a6ba17f18df8b65",
                "000001474af758238005238b2fa253c4f9c557de664d64c66d7d88f334555997591ccc242ff3c6d0e34d07fc835aaae024c2bd21f2ba5c0b0a8ac3fdd90e1c479cb8e538",
                "000000a721b04654204ca19064b37a4abf4247413a6d29e9211a0df9d50975c7d8d4654dc04a478455ea24993f0fef5460f189c4729c15fba385fd85ef42ae7610e39450",
                "000001a23de55fb7ad153e7e65d21ad10c77fdf2072b2a5aa116aaf1bc31a345420c1fe8eaccbbcdeb07b73ba2fa706c7498e3a4828f348b00af9df1551780e709a7754c",
                "000000ed7bc3a95ef74dc3a9893a5ec7c9fab723c16dc21b30341c374c25e1334dada7f92d62967f3bca4da70b0a77fa0732dac05a3d19cb04e50b3cd1894549ac1d3c6e",
                "000001c30d23ea4c140aabfba3753ce40c788010342c15c51d2dd6e9b079194e593ba965ea419ef37746f954027fa1b8159ccd3b54aece67451c7e892d6f6e1fb44c548c",
                "01e8ff7a8c57b017f8b8266adab65fe2c417173566087e15c5fc972071cc560ac93908a806468bc44b8312a8a48464886ca0d767db938447387a348f3f56c6463796",
                true,
                KeyType::P521},
            Params{
                "000000364b6cc953c841d2e1c8c292fadaa673ea073d150da4cec9c0c6544c92373c6a42a781283bedb080a4de720b99769869019a38023019b9d15c99b2a08a02104cc7",
                "000001abbbcd3912723cbb3147145a12b8f2f40d1fe6484fa831250435ffb8eb751bccdf425e81d8e79518ec84c56bd28763c417f30a16c213abbfd8e5871d93dcb3f0d0",
                "00000237c413a4527ca89dee7c8a5c45fc0207627e3db79353ee506857d68f746ea2c7b4d04ab1c10df56b1a706012f616ec9a5510467c49a56941841a1636f620dc3ac4",
                "000000e04b0e32496f1d43ff48d759dc57d586c6c55efb6460fc1148021afa1943180aea2a1be7721a69ebf9d55585a4d4bdb6ff3e177448568c0b45542eddb936fbd870",
                "00000195a305d23a94794e680b211b8ec4a5c2150a2002afdc4ff5da254c438b5686b12dda72bd8dbb009d07d469c3d664e67da274de7f2e2dc84661d2a054638cf6fc98",
                "000000e56b77c46a997c06da59033e1c57939f6f8513329ebe93dcb00857a8ff850428473d937528e85c15a5fe46df1ffb7aeed4b17f7fc9ad08fb121045e2eea5a316b2",
                "013eeaad91133eb338a4c1f27af66aa7f7614619b6c21c20f0e2d68fca2e9cbaa224104a45aa4dbefcb1e49b7f1a36171a35102d2b7d2ffbade07e600b4a7e28c05c",
                false,
                KeyType::P521},
            Params{
                "000001ef093db892daaa05d43e70654bd702d7e177465b00a5ff70a9a072774d7cda3893c181ad1dbb23f8352d3098025406a442c60afaf949dd37ee9c36ee2f785dc693",
                "0000017e817b93e7e3eb95291545afffa2dcd1ff90b97c2b7d9cc7a46511cbdf7a7afe7a2ce126822d08bc71519542743dfece750e18f1e01bb5201e622e2697e5b1faa6",
                "00000025e5e302514ccb15ed84291ba30746b1176f2b080bfa78ef7ce5721fd00d3d717421932d61ad74068c36885a9a0e6537183209a79054fcb8df43b83066f60c47d5",
                "0000004e9f4ebd9f4772df7c003828210d087aa41c0154394428f5f8624536a15c157634c5200de037e46698d4f8484feb164374899b2d9d5e258080dae6b3429c965154",
                "0000021fb91b2abbdd7dee5e2750a1fce602e235a6512b429ab0a3845bd8c8a630d9d35850e0ba78f01c01ffae6ca1a88c93c153a6fdc09f76fae768bdd449e7f6730b68",
                "0000005cb38e5272648f2adcb5ec7a37f3b2135c93a0202f2c822383eae29468392f4baeddabf0acd3bee0cbc1dd540b435d34ecb008408ac093978976741d1afa374c72",
                "01bca2f6325ccf7a6a213b2678ded8ce3e687497c7f7f46a4a06bf3e69dabb745a883d130936c786debf2fe5ef8880f1bb976a79e33b68d30d7356b5a4672e65fcea",
                false,
                KeyType::P521},
            Params{
                "0000014199e2e81aa1f98bf0f39d069e720820d0433222f5b8ca26606fe7c4d0cd619b415d2a629674479a6ec6171f05f657a168aad9342e8489a39cf485a396a8400c59",
                "00000070ce983ca6d0a7aa7849bef13e11e3dc1d62e1ddb85b54d4a3c9de5b47a078846f92105304efe4a4e71d7ff91c19a7b4e03fd87c2d8fb70732e274b114a0bf3bd2",
                "0000009f564f73a5c7282556437dc4c2fbdf87d5fcfbaab0feecf1f7a88104f15a7314b083f711bd7b3a2c74b1096fdccdac01e0a35b8fd86d99ba9d64d10e190033c3df",
                "000000c0b19361224245482bbe42110f9aa44f2818b5146710c24b4825a9a3e0a4743fc7983df560bd85d30639bbdb4d7fc0c56a862e0ef349801bef06ed0a9774463e9f",
                "000000231f2c5c2ad3894b81ad2c8942221d06041f37b8d520e926701b9fb38e32833f9b2b99142d0e54baef33c17b2bd0f23ec3e1b08b9a87f65942384e5863607ba65c",
                "0000005e3b734d4207b51259a1fca526161250176eab1364930d7d06259f43e68202962d9bb818a44bf3bf4a385e634a8d52489c58e2c9234670ae9f30668a2f5553c261",
                "01186d749c6b2a215c59df35ef6f2f6ed1745a9b2e15fca225e79faa7c5b9af44821d50765b45c3c66e210ec78d4bccb7f8c44c19cf80f5357938df48e320fa1ed88",
                true,
                KeyType::P521}));
// clang-format on
} // namespace test
} // namespace fizz
