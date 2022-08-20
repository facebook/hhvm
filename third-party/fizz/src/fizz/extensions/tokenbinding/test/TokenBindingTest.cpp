/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>
#include <fizz/record/test/ExtensionTestsBase.h>

using namespace fizz::extensions;

using namespace folly;

StringPiece tokenBinding{"0018000401000102"};

// Received from chrome using ServerSocket.cpp
StringPiece tokenBindingMessage{
    "00890002004140dd2fa2430a0f54ca96454bdf23c264353a252812bc5fe7b851a6fe9d620424be43f20e50a4ca0a1769f4024db346ca5075eecdb7f62d0018cf1642b75f679d98004089915dea6f6d4d46db349993c5194e709fd22e10bc63ed2b1dfc1f58f300abc13d28c4ecb4dc1dadc8597f813d5f129a58181d3489db69766d832919fbe38a940000"};

namespace fizz {
namespace test {

TEST_F(ExtensionsTest, TokenBindingParameters) {
  auto exts = getExtensions(tokenBinding);
  auto ext = getExtension<TokenBindingParameters>(exts);

  EXPECT_EQ(ext->version, TokenBindingProtocolVersion::token_binding_1_0);
  EXPECT_EQ(ext->key_parameters_list.size(), 1);
  EXPECT_EQ(ext->key_parameters_list[0], TokenBindingKeyParameters::ecdsap256);

  checkEncode(std::move(*ext), tokenBinding);
}

TEST_F(ExtensionsTest, TokenBindingMessageFromChrome) {
  auto buf = getBuf(tokenBindingMessage);
  auto message = decode<TokenBindingMessage>(std::move(buf));
  EXPECT_EQ(message.tokenbindings.size(), 1);
  auto& tokenbinding = message.tokenbindings.front();
  EXPECT_EQ(
      tokenbinding.tokenbinding_type, TokenBindingType::provided_token_binding);
  auto& id = tokenbinding.tokenbindingid;
  EXPECT_EQ(id.key_parameters, TokenBindingKeyParameters::ecdsap256);

  // 32 * 2 for the key + 1 for size of point
  EXPECT_EQ(id.key->computeChainDataLength(), 65);

  // No extensions
  EXPECT_EQ(tokenbinding.extensions->computeChainDataLength(), 0);

  // 32 * 2 for the signature with this algorithm
  EXPECT_EQ(tokenbinding.signature->computeChainDataLength(), 64);
  auto encodedBuf = encode(std::move(message));
  EXPECT_TRUE(folly::IOBufEqualTo()(encodedBuf, buf));
}

TEST_F(ExtensionsTest, TokenBindingMessageSelfCreated) {
  TokenBindingMessage message;
  TokenBinding tokenBinding;
  tokenBinding.tokenbinding_type = TokenBindingType::referred_token_binding;
  TokenBindingID id;
  id.key_parameters = TokenBindingKeyParameters::rsa2048_pkcs1_5;
  id.key = folly::IOBuf::create(10);
  tokenBinding.tokenbindingid = std::move(id);
  tokenBinding.signature = folly::IOBuf::create(10);
  tokenBinding.extensions = folly::IOBuf::create(10);
  message.tokenbindings.push_back(std::move(tokenBinding));

  auto encoded = encode(std::move(message));
  auto decoded = decode<TokenBindingMessage>(std::move(encoded));

  EXPECT_EQ(decoded.tokenbindings.size(), 1);
  auto& decodedTokenBinding = message.tokenbindings.front();
  auto& decodedId = decodedTokenBinding.tokenbindingid;
  EXPECT_EQ(
      decodedTokenBinding.tokenbinding_type,
      TokenBindingType::referred_token_binding);
  EXPECT_EQ(
      decodedId.key_parameters, TokenBindingKeyParameters::rsa2048_pkcs1_5);
}

TEST(ContextTest, TestDoubleNotSet) {
  std::vector<TokenBindingProtocolVersion> two{
      TokenBindingProtocolVersion::token_binding_1_0,
      TokenBindingProtocolVersion::token_binding_1_0};
  TokenBindingContext ctx;
  auto before = ctx.getSupportedVersions();
  EXPECT_THROW(ctx.setSupportedVersions(two), std::runtime_error);
  auto after = ctx.getSupportedVersions();
  EXPECT_EQ(before, after);
}

TEST(ContextTest, TestEmptySet) {
  std::vector<TokenBindingProtocolVersion> empty;
  TokenBindingContext ctx;
  ctx.setSupportedVersions(empty);
  auto after = ctx.getSupportedVersions();
  EXPECT_EQ(empty, after);
}

TEST(ContextTest, TestSingleSet) {
  std::vector<TokenBindingProtocolVersion> single{
      TokenBindingProtocolVersion::token_binding_1_0};
  TokenBindingContext ctx;
  ctx.setSupportedVersions(single);
  auto after = ctx.getSupportedVersions();
  EXPECT_EQ(single, after);
}

} // namespace test
} // namespace fizz
