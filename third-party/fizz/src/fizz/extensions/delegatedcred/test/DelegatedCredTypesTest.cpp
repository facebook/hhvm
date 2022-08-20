/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <fizz/extensions/delegatedcred/Types.h>

using namespace folly;

namespace fizz {
namespace extensions {
namespace test {

TEST(DelegatedCredTypesTest, TestDecodeCredentialSupportExtension) {
  StringPiece kCredSupport{"00020804"};
  Extension e;
  e.extension_type = ExtensionType::delegated_credential;
  e.extension_data = IOBuf::copyBuffer(folly::unhexlify(kCredSupport));
  std::vector<Extension> vec;
  vec.push_back(std::move(e));
  auto supportExt = getExtension<DelegatedCredentialSupport>(vec);

  ASSERT_TRUE(supportExt);
  ASSERT_EQ(supportExt->supported_signature_algorithms.size(), 1);
  ASSERT_EQ(
      supportExt->supported_signature_algorithms[0],
      SignatureScheme::rsa_pss_sha256);
}

TEST(DelegatedCredTypesTest, TestEncodeCredentialSupportExtension) {
  DelegatedCredentialSupport supp;
  supp.supported_signature_algorithms = {
      SignatureScheme::rsa_pss_sha256,
      SignatureScheme::ed25519,
      SignatureScheme::ecdsa_secp256r1_sha256};

  auto encoded = encodeExtension(supp);
  EXPECT_EQ(encoded.extension_type, ExtensionType::delegated_credential);
  EXPECT_TRUE(IOBufEqualTo()(
      encoded.extension_data,
      IOBuf::copyBuffer(folly::unhexlify("0006080408070403"))));
}

TEST(DelegatedCredTypesTest, TestDecodeCredentialExtension) {
  StringPiece kCredential{
      "00115ee3080400012630820122300d06092a864886f70d01010105000382010f"
      "003082010a0282010100b7a6966524cdd309954dfb9f12ee6e4974115d47bd53"
      "c1b7abb4bf635bfd1f4841ab9d87f7d080f41bf44eb8cb54eac6fde998ae58ac"
      "40c0853097974cac7765c587ccff26aabf869167d675f6ace107bd914229e6ad"
      "6f79c05133b37976b50c032e214edde30daf8583baa7cfc1981582d80c2bcb1d"
      "0e8b605b76ad367ca7b96556c0ca3d5520244e312405e7f3f0fbd8c13d7bea8a"
      "3e5f3c23ee632ac2fc4a3e2e3137163970ca05a848fabbc135b01787d94b657d"
      "546deb0c8d30f6dc32eafc1816163b6ca6e6a4eb3da8f2ffb181d063ae9ccf44"
      "cba429fae275bcdf989113d0f131b41a540cefd4dc708e2e323e32d2727e0aa9"
      "ca5c35e2a631fd144cdd02030100010804010022dd13c34dd465d10baa2d8429"
      "fdec31f1d6a50fbd6cbad0bfd1dc88d95e72c7f8f585baf3130df2900e25b8de"
      "a6576ab95ba4214c09e964c292834aa78c792d3f7870d3baca0f8b56d0c32fdb"
      "f5f113c3359c42ac5b66cb6f4995a1f3b83f1349cce28991aedbc6fb4da5455a"
      "a08a44ea9f0af9587d8eaf725a0c21a5610088f73843f9d0700eae3a7be01b9f"
      "60a0c56c28372350987d2d35ae8e54f6bf6e1fa3e4fd1013702da2a5ff8862c3"
      "548d15bcca295164f863a227e635608bca11dba05b30f66e52a52fa73d916f47"
      "2516188863b128feaa13b544c52810af9cc07af14791965741499382e122eee9"
      "e1efd72f46847973c0199632d38a68284ac2ec"};

  Extension e;
  e.extension_type = ExtensionType::delegated_credential;
  e.extension_data = IOBuf::copyBuffer(folly::unhexlify(kCredential));
  std::vector<Extension> vec;
  vec.push_back(std::move(e));
  auto cred = getExtension<DelegatedCredential>(vec);

  EXPECT_EQ(cred->valid_time, 0x00115ee3);
  EXPECT_EQ(cred->expected_verify_scheme, SignatureScheme::rsa_pss_sha256);
  EXPECT_EQ(cred->credential_scheme, SignatureScheme::rsa_pss_sha256);
  EXPECT_TRUE(IOBufEqualTo()(
      cred->public_key,
      IOBuf::copyBuffer(folly::unhexlify(
          "30820122300d06092a864886f70d01010105000382010f003082010a02820101"
          "00b7a6966524cdd309954dfb9f12ee6e4974115d47bd53c1b7abb4bf635bfd1f"
          "4841ab9d87f7d080f41bf44eb8cb54eac6fde998ae58ac40c0853097974cac77"
          "65c587ccff26aabf869167d675f6ace107bd914229e6ad6f79c05133b37976b5"
          "0c032e214edde30daf8583baa7cfc1981582d80c2bcb1d0e8b605b76ad367ca7"
          "b96556c0ca3d5520244e312405e7f3f0fbd8c13d7bea8a3e5f3c23ee632ac2fc"
          "4a3e2e3137163970ca05a848fabbc135b01787d94b657d546deb0c8d30f6dc32"
          "eafc1816163b6ca6e6a4eb3da8f2ffb181d063ae9ccf44cba429fae275bcdf98"
          "9113d0f131b41a540cefd4dc708e2e323e32d2727e0aa9ca5c35e2a631fd144c"
          "dd0203010001"))));

  EXPECT_TRUE(IOBufEqualTo()(
      cred->signature,
      IOBuf::copyBuffer(folly::unhexlify(
          "22dd13c34dd465d10baa2d8429fdec31f1d6a50fbd6cbad0bfd1dc88d95e72c7"
          "f8f585baf3130df2900e25b8dea6576ab95ba4214c09e964c292834aa78c792d"
          "3f7870d3baca0f8b56d0c32fdbf5f113c3359c42ac5b66cb6f4995a1f3b83f13"
          "49cce28991aedbc6fb4da5455aa08a44ea9f0af9587d8eaf725a0c21a5610088"
          "f73843f9d0700eae3a7be01b9f60a0c56c28372350987d2d35ae8e54f6bf6e1f"
          "a3e4fd1013702da2a5ff8862c3548d15bcca295164f863a227e635608bca11db"
          "a05b30f66e52a52fa73d916f472516188863b128feaa13b544c52810af9cc07a"
          "f14791965741499382e122eee9e1efd72f46847973c0199632d38a68284ac2ec"))));
}

TEST(DelegatedCredTypesTest, TestEncodeCredentialExtension) {
  DelegatedCredential cred;
  cred.valid_time = 0xDEADBEEF;
  cred.expected_verify_scheme = SignatureScheme::ed25519;
  cred.credential_scheme = SignatureScheme::ecdsa_secp256r1_sha256;
  cred.public_key = IOBuf::copyBuffer("pubkey");
  cred.signature = IOBuf::copyBuffer("sign");

  auto encoded = encodeExtension(cred);
  EXPECT_EQ(encoded.extension_type, ExtensionType::delegated_credential);
  EXPECT_TRUE(IOBufEqualTo()(
      encoded.extension_data,
      IOBuf::copyBuffer(
          folly::unhexlify("deadbeef08070000067075626b6579040300047369676e"))));
}

} // namespace test
} // namespace extensions
} // namespace fizz
