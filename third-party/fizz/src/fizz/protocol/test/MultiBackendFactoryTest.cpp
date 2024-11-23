/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/MultiBackendFactory.h>
#include <fizz/test/HandshakeTest.h>

namespace fizz {
namespace test {

class MultiBackendFactoryHandshakeTest : public HandshakeTest,
                                         public WithParamInterface<NamedGroup> {
};

TEST_P(MultiBackendFactoryHandshakeTest, multibackendfactory_handshake_test) {
  auto namedGroup = GetParam();
  auto factory = std::make_shared<MultiBackendFactory>();
  clientContext_->setFactory(factory);
  serverContext_->setFactory(factory);
  clientContext_->setSupportedGroups({namedGroup});
  clientContext_->setDefaultShares({namedGroup});
  serverContext_->setSupportedGroups({namedGroup});
  expected_.group = namedGroup;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

INSTANTIATE_TEST_SUITE_P(
    MultiBackendFactoryHandshakeTests,
    MultiBackendFactoryHandshakeTest,
    Values(
        NamedGroup::secp256r1,
        NamedGroup::secp384r1,
        NamedGroup::secp521r1,
        NamedGroup::x25519
#if FIZZ_HAVE_OQS
        ,
        NamedGroup::x25519_kyber512,
        NamedGroup::secp256r1_kyber512,
        NamedGroup::kyber512,
        NamedGroup::x25519_kyber768_draft00,
        NamedGroup::x25519_kyber768_experimental,
        NamedGroup::x25519_kyber512_experimental,
        NamedGroup::X25519MLKEM512_FB,
        NamedGroup::X25519MLKEM768,
        NamedGroup::secp256r1_kyber768_draft00,
        NamedGroup::secp384r1_kyber768
#endif
        ),
    [](const testing::TestParamInfo<
        MultiBackendFactoryHandshakeTest::ParamType>& info) {
      return toString(info.param);
    });

} // namespace test

struct DigestTestCase {
  HashFunction algorithm;
  std::string hexMsg;
  std::string expectedHexDigest;
};

class MultiBackendFactoryDigestTest
    : public ::testing::Test,
      public ::testing::WithParamInterface<DigestTestCase> {
 protected:
  MultiBackendFactory factory_;
};

TEST_P(MultiBackendFactoryDigestTest, Test) {
  auto testCase = GetParam();

  auto makeHasher = factory_.makeHasherFactory(testCase.algorithm);
  auto hasher = makeHasher->make();
  ASSERT_NE(hasher, nullptr);

  std::vector<uint8_t> out;
  out.resize(hasher->getHashLen());

  std::string input;
  folly::unhexlify(testCase.hexMsg, input);

  std::vector<uint8_t> expectedOutput;
  folly::unhexlify(testCase.expectedHexDigest, expectedOutput);
  ASSERT_EQ(expectedOutput.size(), out.size());

  hasher->hash_update(
      folly::IOBuf::wrapBufferAsValue(input.data(), input.size()));
  hasher->hash_final(folly::MutableByteRange(out.data(), out.size()));

  EXPECT_EQ(memcmp(out.data(), expectedOutput.data(), out.size()), 0);
}

INSTANTIATE_TEST_SUITE_P(
    MultiBackendFactoryDigestTestVectors,
    MultiBackendFactoryDigestTest,
    Values(
        DigestTestCase{
            .algorithm = HashFunction::Sha256,
            .hexMsg =
                "451101250ec6f26652249d59dc974b7361d571a8101cdfd36aba3b5854d3ae086b5fdd4597721b66e3c0dc5d8c606d9657d0e323283a5217d1f53f2f284f57b85c8a61ac8924711f895c5ed90ef17745ed2d728abd22a5f7a13479a462d71b56c19a74a40b655c58edfe0a188ad2cf46cbf30524f65d423c837dd1ff2bf462ac4198007345bb44dbb7b1c861298cdf61982a833afc728fae1eda2f87aa2c9480858bec",
            .expectedHexDigest =
                "3c593aa539fdcdae516cdf2f15000f6634185c88f505b39775fb9ab137a10aa2",
        },
        DigestTestCase{
            .algorithm = HashFunction::Sha384,
            .hexMsg =
                "62c6a169b9be02b3d7b471a964fc0bcc72b480d26aecb2ed460b7f50016ddaf04c51218783f3aadfdff5a04ded030d7b3fb7376b61ba30b90e2da921a4470740d63fb99fa16cc8ed81abaf8ce4016e50df81da832070372c24a80890aa3a26fa675710b8fb718266249d496f313c55d0bada101f8f56eeccee4345a8f98f60a36662cfda794900d12f9414fcbdfdeb85388a814996b47e24d5c8086e7a8edcc53d299d0d033e6bb60c58b83d6e8b57f6c258d6081dd10eb942fdf8ec157ec3e75371235a8196eb9d22b1de3a2d30c2abbe0db7650cf6c7159bacbe29b3a93c92100508",
            .expectedHexDigest =
                "0730e184e7795575569f87030260bb8e54498e0e5d096b18285e988d245b6f3486d1f2447d5f85bcbe59d5689fc49425",
        },
        DigestTestCase{
            .algorithm = HashFunction::Sha512,
            .hexMsg =
                "4f05600950664d5190a2ebc29c9edb89c20079a4d3e6bc3b27d75e34e2fa3d02768502bd69790078598d5fcf3d6779bfed1284bbe5ad72fb456015181d9587d6e864c940564eaafb4f2fead4346ea09b6877d9340f6b82eb1515880872213da3ad88feba9f4f13817a71d6f90a1a17c43a15c038d988b5b29edffe2d6a062813cedbe852cde302b3e33b696846d2a8e36bd680efcc6cd3f9e9a4c1ae8cac10cc5244d131677140399176ed46700019a004a163806f7fa467fc4e17b4617bbd7641aaff7ff56396ba8c08a8be100b33a20b5daf134a2aefa5e1c3496770dcf6baa4f7bb",
            .expectedHexDigest =
                "a9db490c708cc72548d78635aa7da79bb253f945d710e5cb677a474efc7c65a2aab45bc7ca1113c8ce0f3c32e1399de9c459535e8816521ab714b2a6cd200525",
        }));

} // namespace fizz
