/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/PerfectIndexMap.h>

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <string>

using namespace folly;
using namespace proxygen;

// This struct exists to WAR the gtest limitation that we can only pass a
// single template parameter to our test fixture.  Thus in this case our single
// test parameter contains the type and value information required to
// instantiate the map under test correctly.
template <typename Key,
          Key OtherKey,
          Key NoneKey,
          Key (*PerfectHashStrToKey)(const std::string&),
          bool AllowDuplicates,
          bool CaseInsensitive,
          uint8_t KeyCommonOffset,
          uint64_t NumKeys>
struct PerfectIndexMapTestsTemplateParams {
  typedef Key TKey;
  static const Key TOtherKey = OtherKey;
  static const Key TNoneKey = NoneKey;
  static const bool TAllowDuplicates = AllowDuplicates;
  static const bool TCaseInsensitive = CaseInsensitive;

  // Pass through wrapper for the hashing method.
  // This method only exists because am unsure how to properly capture it and
  // expose it as a subsequent template parameter for consumers.
  static Key Hash(const std::string& name) {
    return PerfectHashStrToKey(name);
  }

  static const uint8_t TKeyCommonOffset = KeyCommonOffset;
  static const uint64_t TNumKeys = NumKeys;
};

// Wrapper test class allowing us to create the desired PerfectIndexMap via
// the specified template parameter
template <class T>
class PerfectIndexMapTests : public testing::Test {
 protected:
  PerfectIndexMap<typename T::TKey,
                  T::TOtherKey,
                  T::TNoneKey,
                  T::Hash,
                  T::TAllowDuplicates,
                  T::TCaseInsensitive>
      testMap_;
};

// Register the template configurations we wish to automatically test
typedef testing::Types<
    PerfectIndexMapTestsTemplateParams<HTTPHeaderCode,
                                       HTTP_HEADER_OTHER,
                                       HTTP_HEADER_NONE,
                                       HTTPCommonHeaders::hash,
                                       false,
                                       true,
                                       HTTPHeaderCodeCommonOffset,
                                       HTTPCommonHeaders::num_codes>,
    PerfectIndexMapTestsTemplateParams<HTTPHeaderCode,
                                       HTTP_HEADER_OTHER,
                                       HTTP_HEADER_NONE,
                                       HTTPCommonHeaders::hash,
                                       true,
                                       true,
                                       HTTPHeaderCodeCommonOffset,
                                       HTTPCommonHeaders::num_codes>,
    PerfectIndexMapTestsTemplateParams<HTTPHeaderCode,
                                       HTTP_HEADER_OTHER,
                                       HTTP_HEADER_NONE,
                                       HTTPCommonHeaders::hash,
                                       true,
                                       false,
                                       HTTPHeaderCodeCommonOffset,
                                       HTTPCommonHeaders::num_codes>,
    PerfectIndexMapTestsTemplateParams<HTTPHeaderCode,
                                       HTTP_HEADER_OTHER,
                                       HTTP_HEADER_NONE,
                                       HTTPCommonHeaders::hash,
                                       false,
                                       false,
                                       HTTPHeaderCodeCommonOffset,
                                       HTTPCommonHeaders::num_codes>>
    TestTypes;
TYPED_TEST_SUITE(PerfectIndexMapTests, TestTypes);

TYPED_TEST(PerfectIndexMapTests, BasicKeySetAddRemoveGetSingleOrNone) {
  typedef typename TypeParam::TKey Key;

  EXPECT_EQ(this->testMap_.size(), 0);

  // Insert numInserted distinct keys and duplicate values into the map.
  auto numInserted = TypeParam::TNumKeys - TypeParam::TKeyCommonOffset;
  for (uint64_t j = TypeParam::TKeyCommonOffset; j < TypeParam::TNumKeys; ++j) {
    this->testMap_.set(static_cast<Key>(j), std::to_string(j));
  }
  EXPECT_EQ(this->testMap_.size(), numInserted);

  // Setting a duplicate should not increase the size of the map, regardless
  // of whether duplicates are supported
  Key key = static_cast<Key>(TypeParam::TKeyCommonOffset);
  this->testMap_.set(key, std::to_string(TypeParam::TKeyCommonOffset));
  EXPECT_EQ(this->testMap_.size(), numInserted);

  // Adding is only allowed when duplicates are and so here we expect the size
  // of the map to change.
  if (TypeParam::TAllowDuplicates) {
    this->testMap_.add(key, std::to_string(TypeParam::TKeyCommonOffset));
    EXPECT_EQ(this->testMap_.size(), numInserted + 1);
  }

  // Remove the last added element in the map (and its duplicate if applicable)
  // Adjusts numInserted as appropriate
  this->testMap_.remove(key);
  EXPECT_EQ(this->testMap_.size(), --numInserted);

  // Verify the integrity of the map
  for (uint64_t j = TypeParam::TKeyCommonOffset + 1; j < TypeParam::TNumKeys;
       ++j) {
    key = static_cast<Key>(j);
    auto optional = this->testMap_.getSingleOrNone(key);
    ASSERT_TRUE(optional.hasValue());
    ASSERT_EQ(optional.value(), std::to_string(j));
  }
}

TYPED_TEST(PerfectIndexMapTests, BasicOtherKeySetAddRemoveGetSingleOrNone) {
  EXPECT_EQ(this->testMap_.size(), 0);

  // Insert numInserted distinct keys and values in to the map
  int numInserted = 10;
  std::string val;
  for (int num = 0; num < numInserted; ++num) {
    val = std::to_string(num);
    this->testMap_.set(val, val);
  }
  EXPECT_EQ(this->testMap_.size(), numInserted);

  // Setting a duplicate should not increase the size of the map, regardless
  // of whether duplicates are supported
  this->testMap_.set(val, val);
  EXPECT_EQ(this->testMap_.size(), numInserted);

  // Adding is only allowed when duplicates are and so here we expect the size
  // of the map to change.
  if (TypeParam::TAllowDuplicates) {
    this->testMap_.add(val, val);
    EXPECT_EQ(this->testMap_.size(), numInserted + 1);
  }

  // Remove the last added element in the map (and its duplicate if applicable)
  // Adjusts numInserted as appropriate
  this->testMap_.remove(val);
  EXPECT_EQ(this->testMap_.size(), --numInserted);

  // Verify the integrity of the map
  for (int num = 0; num < numInserted; ++num) {
    val = std::to_string(num);
    auto optional = this->testMap_.getSingleOrNone(val);
    ASSERT_TRUE(optional.hasValue());
    ASSERT_EQ(optional.value(), val);
  }
}

// Note there is no corresponding key string case sensitivity test as that is
// controlled by the specified hashing function of the map.  The user in
// creating the map thus chooses whether the hashing function should be case
// sensitive or not and thus does not require explicit testing as part of this
// class
TYPED_TEST(PerfectIndexMapTests, OtherStringCaseSensitivity) {
  std::string testString = "test";
  std::string modTestString = "tEsT";
  std::string addModTestString = "TeSt";
  this->testMap_.set(testString, testString);

  auto currentCount = this->testMap_.size();
  folly::Optional<std::string> optional;
  if (TypeParam::TCaseInsensitive) {
    optional = this->testMap_.getSingleOrNone(modTestString);
    ASSERT_TRUE(optional.has_value());
    ASSERT_EQ(optional.value(), testString);

    this->testMap_.set(modTestString, modTestString);
    EXPECT_EQ(this->testMap_.size(), currentCount);

    optional = this->testMap_.getSingleOrNone(testString);
    ASSERT_TRUE(optional.has_value());
    ASSERT_EQ(optional.value(), modTestString);
  } else {
    this->testMap_.set(modTestString, modTestString);
    EXPECT_EQ(this->testMap_.size(), ++currentCount);

    this->testMap_.set(testString, testString);
    EXPECT_EQ(this->testMap_.size(), currentCount);

    optional = this->testMap_.getSingleOrNone(testString);
    ASSERT_TRUE(optional.has_value());
    ASSERT_EQ(optional.value(), testString);

    optional = this->testMap_.getSingleOrNone(modTestString);
    ASSERT_TRUE(optional.has_value());
    ASSERT_EQ(optional.value(), modTestString);
  }

  if (TypeParam::TAllowDuplicates) {
    // Finally verify that two exact same keys are treated as duplicates.
    this->testMap_.add(addModTestString, addModTestString);
    optional = this->testMap_.getSingleOrNone(testString);
    if (TypeParam::TCaseInsensitive) {
      EXPECT_FALSE(optional.has_value());
    } else {
      EXPECT_TRUE(optional.has_value());
    }
  }
}
