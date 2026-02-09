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

// Generated via secure/tokenbinding in fbandroid
StringPiece tokenBindingMessageRSA2048_PKCS1_5{
    "020e000001060100afe6c3f9d7c68e4bc8eb5059ecd4fcf45a1ec98bcd3a0ad02e6c209244014dbcbf10c5bf84bdc7442002acbf155f26882efa71e0611d09255e07bc3125d5db3af31ad6a6bb8a94ae9d4ee7116986b15072b268debf5c8647e520d1cdce72224ceb99bba412fa9784a13c2d413a7dfb809e2e7cdbcb11991fab76f43ffbec49712479640f19811410b8e94c93c50bae6e6746814a2807960087b03b8276370145b5b259644735597a76852825df5429bf2f309615e5a2a21cc16f75f1e3091c247dce4ef0b982849c305d9490018db6699868cfd5941060496a37e0bc835af9857aa551352929c7f17a9b69c948a021552a91f7d718bb9ab49bb91ef2bb077967030100010100000c0ceb9dc96d6e7c84014d4f1467d779ac6dc55ca2f7a32f76fa09e243f4f78ddd28c06154b6e642359679ab5fc170a05fd15c00c797149ea4dccfffae525a360f7f689a1e270e7f0ff0df5bc1eaaa21833daaef12fcf5fc034706ca421b43cd0f1be8d9c4a5f46c3d7954ac7021ee9b22b83c6318534487c68a0692b7f0978ea5076037d834f667c781f0823d5cb8aa098468e695c975c51f096792f068b82cc6a6b5c38acad44a939126251009a45a4aee6f75bbf0a911d50e1226e39bd23ff873b80cbbbe6cf40f95a5cabbadcf14f21f9cea386d1fe30475aa493a990f8c9ce34e15b479f661c419973cc9b5c65b51d2340f5bf9efbb13e2b11b51d7380000"};

StringPiece tokenBindingMessageRSA2048_PSS{
    "020e000101060100be432a7bd7a0c727489695245911c9b25c9fd6b5a569c110e33c8811a86a6cd216e24dff4b0c27bcd390a6e39c4ff2e47d6712afb512951160ef054e1791278e96ceb1d2f1f891334e6ded38ce48fb0c69fc1964981aa79e438fa08726f91ebafa2e6ca0522847e6ba652a27184deb14bb93e9bc0498171613fe34687cf0e0ae63c414e51f43bd3161dce2a166f3b25e5047f747c38c47944964f5ddd249d8d7db0fbd71ed66aad375502687fbedb57b70e420bb5242832442e183acea27ab0c0b9547d9a833c34ab913f4cc28af9357a4774c8fe7b470a63be3c76fe9d6ce9ca72bfb908a2b0cd9c41e5361862fd6e01351ac2ea9d49f9e3b1742710bec939f030100010100112730f11c86600083a8246c3ed47f048e4788724f31b12bd4963f08e23f0c872bd6fdec820e8178697f31f73c3b2386d68f5ea45e28fc60af0395412f291e954c916e8994c52cfbcf9946d6660d4089aa35ffee8e28d8dbab8b2cd5e3e5b306fafe542d84b6799b46a53c5a4f35663acff727258869868f37b1f9b338267640faef5af1e59649bb69b963953049e791d050de554065ec8bd8e8ae0454b166775ea8730e0babba425064141faf594ae254acd657ff816fed4df99edb7d26bd978891104268694ba1b01bcf0ad50330883ab29f87bb2b3e0561c68664bf498a85c5736f9c24682e5fffc1f488ad23aa25d4b1722026d5e2f5fc017e62d0eed32b0000"};

StringPiece tokenBindingMessageECDSAP256{
    "00890002004140e5b1cf521e26e5819c37c0e929278faea5b714d8409f211f7d49afdbbb97a1e2fabeb248b736e8e23b834db06beec085db7eeee7b828e6ee816eaac2713ae74f004009078cc06441cf0a8b31c369497dbd84f05816c4ec01fc5be62cee900eaf3c98009a170ba40d0a62c981bfb10b7401688e3515775dac50da103eced9ff0178c40000"};

namespace fizz {
namespace test {

TEST_F(ExtensionsTest, TokenBindingParameters) {
  auto exts = getExtensions(tokenBinding);
  folly::Optional<TokenBindingParameters> ext;
  Error err;
  EXPECT_EQ(
      getExtension<TokenBindingParameters>(ext, err, exts), Status::Success);

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

TEST_F(ExtensionsTest, TokenBindingMessageRSA2048_PKCS1_5) {
  auto buf = getBuf(tokenBindingMessageRSA2048_PKCS1_5);
  auto message = decode<TokenBindingMessage>(std::move(buf));
  EXPECT_EQ(message.tokenbindings.size(), 1);
  auto& tokenbinding = message.tokenbindings.front();
  EXPECT_EQ(
      tokenbinding.tokenbinding_type, TokenBindingType::provided_token_binding);
  auto& id = tokenbinding.tokenbindingid;
  EXPECT_EQ(id.key_parameters, TokenBindingKeyParameters::rsa2048_pkcs1_5);
}

TEST_F(ExtensionsTest, TokenBindingMessageRSA2048_PSS) {
  auto buf = getBuf(tokenBindingMessageRSA2048_PSS);
  auto message = decode<TokenBindingMessage>(std::move(buf));
  EXPECT_EQ(message.tokenbindings.size(), 1);
  auto& tokenbinding = message.tokenbindings.front();
  EXPECT_EQ(
      tokenbinding.tokenbinding_type, TokenBindingType::provided_token_binding);
  auto& id = tokenbinding.tokenbindingid;
  EXPECT_EQ(id.key_parameters, TokenBindingKeyParameters::rsa2048_pss);
}

TEST_F(ExtensionsTest, TokenBindingMessageECDSAP256) {
  auto buf = getBuf(tokenBindingMessageECDSAP256);
  auto message = decode<TokenBindingMessage>(std::move(buf));
  EXPECT_EQ(message.tokenbindings.size(), 1);
  auto& tokenbinding = message.tokenbindings.front();
  EXPECT_EQ(
      tokenbinding.tokenbinding_type, TokenBindingType::provided_token_binding);
  auto& id = tokenbinding.tokenbindingid;
  EXPECT_EQ(id.key_parameters, TokenBindingKeyParameters::ecdsap256);
}

} // namespace test
} // namespace fizz
