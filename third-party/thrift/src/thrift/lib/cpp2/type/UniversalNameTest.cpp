/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/type/UniversalName.h>

#include <string>

#include <openssl/evp.h>

#include <fmt/core.h>
#include <gtest/gtest.h>
#include <folly/Demangle.h>
#include <folly/FBString.h>
#include <folly/String.h>
#include <thrift/lib/cpp2/type/UniversalHashAlgorithm.h>

namespace apache::thrift::type {
namespace {

template <typename LHS, typename RHS>
struct IsSame;
template <typename T>
struct IsSame<T, T> {};

constexpr auto kGoodIdSizes = {8, 16, 24, 32};

const auto kBadIdSizes = {1, 7, 33, 255};

auto Hash() {
  return EVP_sha256();
}

TEST(UniversalNameTest, Constants) {
  IsSame<hash_size_t, folly::remove_cvref_t<decltype(kMinHashBytes)>>();

  auto* ctx = EVP_MD_CTX_new();
  ASSERT_NE(ctx, nullptr);
  EXPECT_NE(EVP_DigestInit_ex(ctx, Hash(), nullptr), 0);
  EXPECT_EQ(
      getUniversalHashSize(UniversalHashAlgorithm::Sha2_256),
      EVP_MD_CTX_size(ctx));
  EVP_MD_CTX_free(ctx);
}

TEST(UniversalNameTest, ValidateUniversalHash) {
  EXPECT_THROW(
      validateUniversalHash(
          UniversalHashAlgorithm::Sha2_256, "", kMinHashBytes),
      std::invalid_argument);
  for (const auto& bad : kBadIdSizes) {
    SCOPED_TRACE(bad);
    EXPECT_THROW(
        validateUniversalHash(
            UniversalHashAlgorithm::Sha2_256,
            folly::fbstring(bad, 'a'),
            kMinHashBytes),
        std::invalid_argument);
  }

  for (const auto& good : kGoodIdSizes) {
    SCOPED_TRACE(good);
    try {
      validateUniversalHash(
          UniversalHashAlgorithm::Sha2_256,
          folly::fbstring(good, 'a'),
          kMinHashBytes);
    } catch (const std::exception& ex) {
      GTEST_FAIL() << folly::demangle(typeid(ex)) << ": " << ex.what();
    }
  }
}

TEST(UniversalNameTest, ValidateUniversalHashBytes) {
  validateUniversalHashBytes(kDisableUniversalHash, kMinHashBytes);
  for (const auto& bad : kBadIdSizes) {
    if (bad >= kMinHashBytes) {
      continue; // Upper bounds is not checked, as it can vary.
    }
    SCOPED_TRACE(bad);
    EXPECT_THROW(
        validateUniversalHashBytes(bad, kMinHashBytes), std::invalid_argument);
  }

  for (const auto& good : kGoodIdSizes) {
    SCOPED_TRACE(good);
    validateUniversalHashBytes(good, kMinHashBytes);
  }
}

TEST(UniversalNameTest, UniversalHashSha2_256) {
  // The digest should include the fbthrift:// prefix.
  std::string expected(EVP_MAX_MD_SIZE, 0);
  constexpr std::string_view message = "fbthrift://foo.com/my/type";
  uint32_t size;
  EXPECT_TRUE(EVP_Digest(
      message.data(),
      message.size(),
      reinterpret_cast<uint8_t*>(expected.data()),
      &size,
      Hash(),
      nullptr));
  expected.resize(size);
  EXPECT_EQ(
      getUniversalHash(UniversalHashAlgorithm::Sha2_256, "foo.com/my/type"),
      expected);

  // Make sure the values haven't changed unintentionally.
  EXPECT_EQ(
      expected,
      "\tat$\234\357\255\265\352\rE;\3133\255Tv\001\373\376\304\262\327\225\222N\353g\324[\346F")
      << folly::cEscape<std::string>(expected);
  // Make sure the values differ.
  EXPECT_NE(
      getUniversalHash(
          UniversalHashAlgorithm::Sha2_256, "facebook.com/thrift/Object"),
      expected);
}

TEST(UniversalNameTest, MatchesUniversalHash) {
  EXPECT_FALSE(matchesUniversalHash("0123456789ABCDEF0123456789ABCDEF", ""));
  EXPECT_FALSE(matchesUniversalHash("0123456789ABCDEF0123456789ABCDEF", "1"));
  EXPECT_TRUE(matchesUniversalHash("0123456789ABCDEF0123456789ABCDEF", "0"));
  EXPECT_TRUE(matchesUniversalHash(
      "0123456789ABCDEF0123456789ABCDEF", "0123456789ABCDEF"));
  EXPECT_TRUE(matchesUniversalHash(
      "0123456789ABCDEF0123456789ABCDEF", "0123456789ABCDEF0123456789ABCDEF"));
  EXPECT_FALSE(matchesUniversalHash(
      "0123456789ABCDEF0123456789ABCDEF", "0123456789ABCDEF0123456789ABCDEF0"));
}

TEST(UniversalNameTest, GetUniversalHashPrefix) {
  folly::fbstring fullId(32, 'a');
  EXPECT_EQ(getUniversalHashPrefix(fullId, 0), "");

  for (size_t i = 1; i < fullId.size(); ++i) {
    EXPECT_EQ(getUniversalHashPrefix(fullId, i), fullId.substr(0, i)) << i;
  }
  EXPECT_EQ(getUniversalHashPrefix(fullId, fullId.size()), fullId);
  EXPECT_EQ(getUniversalHashPrefix(fullId, fullId.size() + 1), fullId);
  EXPECT_EQ(
      getUniversalHashPrefix(fullId, std::numeric_limits<hash_size_t>::max()),
      fullId);
}

TEST(UniversalNameTest, MaybeGetUniversalHashPrefix) {
  std::string name(24, 'a');
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(
          UniversalHashAlgorithm::Sha2_256, name, kDisableUniversalHash)
          .size(),
      0);
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 8)
          .size(),
      8);
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 16)
          .size(),
      16);
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 23)
          .size(),
      23);
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 24)
          .size(),
      0);
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 32)
          .size(),
      0);

  name += name;
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 32)
          .size(),
      32);
  EXPECT_EQ(
      maybeGetUniversalHashPrefix(UniversalHashAlgorithm::Sha2_256, name, 33)
          .size(),
      32);
}

TEST(UniversalNameTest, FindByUniversalHash) {
  std::map<std::string, int> uniHashs = {
      {"1233", 0},
      {"1234", 1},
      {"12345", 2},
      {"1235", 3},
  };

  EXPECT_TRUE(containsUniversalHash(uniHashs, "1"));
  EXPECT_THROW(findByUniversalHash(uniHashs, "1"), std::runtime_error);
  EXPECT_TRUE(containsUniversalHash(uniHashs, "12"));
  EXPECT_THROW(findByUniversalHash(uniHashs, "12"), std::runtime_error);
  EXPECT_TRUE(containsUniversalHash(uniHashs, "123"));
  EXPECT_THROW(findByUniversalHash(uniHashs, "123"), std::runtime_error);
  EXPECT_TRUE(containsUniversalHash(uniHashs, "1234"));
  EXPECT_THROW(findByUniversalHash(uniHashs, "1234"), std::runtime_error);

  EXPECT_FALSE(containsUniversalHash(uniHashs, ""));
  EXPECT_EQ(findByUniversalHash(uniHashs, ""), uniHashs.end());

  EXPECT_FALSE(containsUniversalHash(uniHashs, "0"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "0"), uniHashs.end());

  EXPECT_TRUE(containsUniversalHash(uniHashs, "1233"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "1233")->second, 0);

  EXPECT_FALSE(containsUniversalHash(uniHashs, "12333"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "12333"), uniHashs.end());

  EXPECT_TRUE(containsUniversalHash(uniHashs, "12345"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "12345")->second, 2);

  EXPECT_FALSE(containsUniversalHash(uniHashs, "12346"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "12346"), uniHashs.end());

  EXPECT_TRUE(containsUniversalHash(uniHashs, "1235"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "1235")->second, 3);

  EXPECT_FALSE(containsUniversalHash(uniHashs, "2"));
  EXPECT_EQ(findByUniversalHash(uniHashs, "2"), uniHashs.end());
}

} // namespace
} // namespace apache::thrift::type
