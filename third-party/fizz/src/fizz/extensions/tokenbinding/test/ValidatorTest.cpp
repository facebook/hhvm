/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/extensions/tokenbinding/Validator.h>

using namespace folly;

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

// Test values from using chrome to hit my sandbox
StringPiece chrome_session_ekm{
    "9d20b2acf86f893a240642593cfc53102b9fb76b37f059d4bff47a0e6fee25e7"};
StringPiece chrome_session_key{
    "40dd2fa2430a0f54ca96454bdf23c264353a252812bc5fa7b851a6fa9d620424bf43e20e50a4ca0a1769f4024db346ca5075eecdb7f62d0018cf1642b75f679d98"};
StringPiece chrome_session_signature{
    "d2c9c04957013f38369a18a5d5b47d6492f0f0f5c8772a27cc3770f23dda94d30fc3a6d0dc110c78e668a44c3b8b61842a6e72795f61f51f398f8dedd2ceb9a3"};
StringPiece ed25519_session_key{
    "20fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025"};
StringPiece ed25519_session_signature{
    "ccf3489b6880d1151d6cb99d2989dca8531fbd9867715995f81b6b037ac7559a7fc89e71a5c1c5194fa142175fd09f7c1dc696563cf161771e809cc40b592900"};

class ValidatorTest : public Test {
 public:
  void SetUp() override {
    OpenSSL_add_all_algorithms();
    ekm_ = getBuf(chrome_session_ekm);
  }

  TokenBinding setUpWithKeyParameters(TokenBindingKeyParameters params) {
    TokenBinding tokenBinding;
    tokenBinding.tokenbinding_type = TokenBindingType::provided_token_binding;
    tokenBinding.extensions = folly::IOBuf::create(0);
    TokenBindingID id;
    id.key_parameters = params;
    switch (params) {
      case TokenBindingKeyParameters::ecdsap256: {
        id.key = getBuf(chrome_session_key);
        tokenBinding.tokenbindingid = std::move(id);
        tokenBinding.signature = getBuf(chrome_session_signature);
        return tokenBinding;
      }
#if FIZZ_OPENSSL_HAS_ED25519
      case TokenBindingKeyParameters::ed25519_experimental: {
        id.key = getBuf(ed25519_session_key);
        tokenBinding.tokenbindingid = std::move(id);
        tokenBinding.signature = getBuf(ed25519_session_signature);
        return tokenBinding;
      }
#endif
      default: // rsa_pss and rsa_pkcs
        throw std::runtime_error("not implemented");
    }
  }

  Buf getBuf(StringPiece hex) {
    auto data = unhexlify(hex);
    return folly::IOBuf::copyBuffer(data.data(), data.size());
  }

  Buf ekm_;
};

TEST_F(ValidatorTest, TestBadKeySent) {
  StringPiece bad_ecdsa_key{
      "3060FED4BA255A9D31C961EB74C6356D68C049B8923B41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4461FA6CE669622E60F29FB67903FE1008B8BC99A62299"};
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);

  binding.tokenbindingid.key = getBuf(bad_ecdsa_key);
  EXPECT_FALSE(
      Validator::validateTokenBinding(
          std::move(binding), ekm_, TokenBindingKeyParameters::ecdsap256)
          .has_value());
}

TEST_F(ValidatorTest, TestMismatchKeyParams) {
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);
  EXPECT_FALSE(
      Validator::validateTokenBinding(
          std::move(binding), ekm_, TokenBindingKeyParameters::rsa2048_pss)
          .has_value());
}

TEST_F(ValidatorTest, TestChromeSignature) {
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);
  EXPECT_TRUE(
      Validator::validateTokenBinding(
          std::move(binding), ekm_, TokenBindingKeyParameters::ecdsap256)
          .has_value());
}

TEST_F(ValidatorTest, TestInvalidSignature) {
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);
  *binding.signature->writableData() ^= 0x04;
  EXPECT_FALSE(
      Validator::validateTokenBinding(
          std::move(binding), ekm_, TokenBindingKeyParameters::ecdsap256)
          .has_value());
}

TEST_F(ValidatorTest, TestTruncatedSignature) {
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);
  binding.signature->trimEnd(4);
  EXPECT_FALSE(
      Validator::validateTokenBinding(
          std::move(binding), ekm_, TokenBindingKeyParameters::ecdsap256)
          .has_value());
}

TEST_F(ValidatorTest, TestMultipleSupportedParameters) {
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);
  EXPECT_TRUE(Validator::validateTokenBinding(
                  std::move(binding),
                  ekm_,
                  {TokenBindingKeyParameters::rsa2048_pkcs1_5,
                   TokenBindingKeyParameters::ecdsap256,
                   TokenBindingKeyParameters::ed25519_experimental})
                  .has_value());
}

TEST_F(ValidatorTest, TestSentParameterNotSupported) {
  auto binding = setUpWithKeyParameters(TokenBindingKeyParameters::ecdsap256);
  EXPECT_FALSE(Validator::validateTokenBinding(
                   std::move(binding),
                   ekm_,
                   {TokenBindingKeyParameters::rsa2048_pkcs1_5,
                    TokenBindingKeyParameters::rsa2048_pss,
                    TokenBindingKeyParameters::ed25519_experimental})
                   .has_value());
}

#if FIZZ_OPENSSL_HAS_ED25519
// The tests below are mostly Ed25519 variants of the tests above
TEST_F(ValidatorTest, TestValidEd25519Signature) {
  auto binding =
      setUpWithKeyParameters(TokenBindingKeyParameters::ed25519_experimental);
  EXPECT_TRUE(Validator::validateTokenBinding(
                  std::move(binding),
                  ekm_,
                  TokenBindingKeyParameters::ed25519_experimental)
                  .has_value());
}

TEST_F(ValidatorTest, TestBadEd25519KeySent) {
  // Some random key with length specified correctly as 0x20
  StringPiece bad_ed25519_key{
      "204ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb"};
  auto binding =
      setUpWithKeyParameters(TokenBindingKeyParameters::ed25519_experimental);

  binding.tokenbindingid.key = getBuf(bad_ed25519_key);
  EXPECT_FALSE(Validator::validateTokenBinding(
                   std::move(binding),
                   ekm_,
                   TokenBindingKeyParameters::ed25519_experimental)
                   .has_value());
}

TEST_F(ValidatorTest, TestBadEd25519KeyLength) {
  // Key length specified as 0x21 instead of 0x20
  StringPiece bad_ed25519_key{
      "21fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025"};
  auto binding =
      setUpWithKeyParameters(TokenBindingKeyParameters::ed25519_experimental);

  binding.tokenbindingid.key = getBuf(bad_ed25519_key);
  EXPECT_FALSE(Validator::validateTokenBinding(
                   std::move(binding),
                   ekm_,
                   TokenBindingKeyParameters::ed25519_experimental)
                   .has_value());
}

TEST_F(ValidatorTest, TestInvalidEd25519Signature) {
  auto binding =
      setUpWithKeyParameters(TokenBindingKeyParameters::ed25519_experimental);
  *binding.signature->writableData() ^= 0x04;
  EXPECT_FALSE(Validator::validateTokenBinding(
                   std::move(binding),
                   ekm_,
                   TokenBindingKeyParameters::ed25519_experimental)
                   .has_value());
}

TEST_F(ValidatorTest, TestTruncatedEd25519Signature) {
  auto binding =
      setUpWithKeyParameters(TokenBindingKeyParameters::ed25519_experimental);
  binding.signature->trimEnd(4);
  EXPECT_FALSE(Validator::validateTokenBinding(
                   std::move(binding),
                   ekm_,
                   TokenBindingKeyParameters::ed25519_experimental)
                   .has_value());
}
#endif
} // namespace test
} // namespace extensions
} // namespace fizz
