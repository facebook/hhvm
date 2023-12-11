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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/test/gen-cpp2/FieldMask_types.h>

using apache::thrift::protocol::allMask;
using apache::thrift::protocol::asValueStruct;
using apache::thrift::protocol::Mask;
using apache::thrift::protocol::MaskBuilder;
using apache::thrift::protocol::MaskRef;
using apache::thrift::protocol::noneMask;
using namespace apache::thrift::protocol::detail;

namespace apache::thrift::test {

bool literallyEqual(MaskRef actual, MaskRef expected) {
  return actual.mask == expected.mask &&
      actual.is_exclusion == expected.is_exclusion;
}

TEST(FieldMaskTest, ExampleFieldMask) {
  // includes{7: excludes{},
  //          9: includes{5: excludes{},
  //                      6: excludes{}}}
  Mask m;
  auto& includes = m.includes_ref().emplace();
  includes[7] = allMask();
  auto& nestedIncludes = includes[9].includes_ref().emplace();
  nestedIncludes[5] = allMask();
  nestedIncludes[6] = allMask();
  includes[8] = noneMask(); // not required
}

TEST(FieldMaskTest, ExampleMapMask) {
  // includes_map{7: excludes_map{},
  //              3: excludes{}}
  Mask m;
  auto& includes_map = m.includes_map_ref().emplace();
  includes_map[7].excludes_map_ref().emplace();
  includes_map[3] = allMask();
}

TEST(FieldMaskTest, ExampleStringMapMask) {
  // includes_string_map{"7": excludes_string_map{},
  //                     "3": excludes{}}
  Mask m;
  auto& includes_string_map = m.includes_string_map_ref().emplace();
  includes_string_map["7"].excludes_string_map_ref().emplace();
  includes_string_map["3"] = allMask();
}

TEST(FieldMaskTest, Constant) {
  EXPECT_EQ(allMask().excludes_ref()->size(), 0);
  EXPECT_EQ(noneMask().includes_ref()->size(), 0);
}

TEST(FieldMaskTest, IsAllMask) {
  EXPECT_TRUE((MaskRef{allMask(), false}).isAllMask());
  EXPECT_TRUE((MaskRef{noneMask(), true}).isAllMask());
  EXPECT_FALSE((MaskRef{noneMask(), false}).isAllMask());
  EXPECT_FALSE((MaskRef{allMask(), true}).isAllMask());
  {
    Mask m;
    m.excludes_ref().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[3].includes_map_ref().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
  }
  {
    Mask m;
    m.includes_string_map_ref()
        .emplace()["3"]
        .includes_string_map_ref()
        .emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
  }
}

TEST(FieldMaskTest, IsNoneMask) {
  EXPECT_TRUE((MaskRef{noneMask(), false}).isNoneMask());
  EXPECT_TRUE((MaskRef{allMask(), true}).isNoneMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isNoneMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isNoneMask());
  {
    Mask m;
    m.excludes_ref().emplace()[5] = noneMask();
    EXPECT_FALSE((MaskRef{m, false}).isNoneMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneMask());
  }
}

TEST(FieldMaskTest, IsAllMapMask) {
  auto allMapMask = [] {
    Mask m;
    m.excludes_map_ref().emplace();
    return m;
  };

  auto noneMapMask = [] {
    Mask m;
    m.includes_map_ref().emplace();
    return m;
  };

  EXPECT_TRUE((MaskRef{allMapMask(), false}).isAllMapMask());
  EXPECT_TRUE((MaskRef{noneMapMask(), true}).isAllMapMask());
  EXPECT_FALSE((MaskRef{noneMapMask(), false}).isAllMapMask());
  EXPECT_FALSE((MaskRef{allMapMask(), true}).isAllMapMask());
  {
    Mask m;
    m.excludes_ref().emplace()[5] = allMask();
    EXPECT_THROW((MaskRef{m, false}).isAllMapMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isAllMapMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[3].includes_map_ref().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMapMask());
  }
  {
    Mask m;
    m.includes_string_map_ref()
        .emplace()["3"]
        .includes_string_map_ref()
        .emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMapMask());
  }
}

TEST(FieldMaskTest, IsNoneMapMask) {
  auto allMapMask = [] {
    Mask m;
    m.excludes_map_ref().emplace();
    return m;
  };

  auto noneMapMask = [] {
    Mask m;
    m.includes_map_ref().emplace();
    return m;
  };

  EXPECT_TRUE((MaskRef{noneMapMask(), false}).isNoneMapMask());
  EXPECT_TRUE((MaskRef{allMapMask(), true}).isNoneMapMask());
  EXPECT_FALSE((MaskRef{allMapMask(), false}).isNoneMapMask());
  EXPECT_FALSE((MaskRef{noneMapMask(), true}).isNoneMapMask());
  {
    Mask m;
    m.excludes_ref().emplace()[5] = noneMask();
    EXPECT_THROW((MaskRef{m, false}).isNoneMapMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isNoneMapMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[3].includes_map_ref().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isNoneMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneMapMask());
  }
  {
    Mask m;
    m.includes_string_map_ref()
        .emplace()["3"]
        .includes_string_map_ref()
        .emplace();
    EXPECT_FALSE((MaskRef{m, false}).isNoneMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneMapMask());
  }
}

TEST(FieldMaskTest, IsExclusive) {
  EXPECT_FALSE((MaskRef{noneMask(), false}).isExclusive());
  EXPECT_FALSE((MaskRef{allMask(), true}).isExclusive());
  EXPECT_TRUE((MaskRef{allMask(), false}).isExclusive());
  EXPECT_TRUE((MaskRef{noneMask(), true}).isExclusive());
  {
    Mask m;
    m.includes_ref().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes_ref().emplace()[5] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes_map_ref().emplace()[5] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.includes_string_map_ref().emplace()["5"] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes_string_map_ref().emplace()["5"] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
}

TEST(FieldMaskTest, MaskRefIsMask) {
  EXPECT_TRUE((MaskRef{allMask(), false}).isFieldMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isMapMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isIntegerMapMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isStringMapMask());
  EXPECT_TRUE((MaskRef{noneMask(), true}).isFieldMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isMapMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isIntegerMapMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isStringMapMask());
  {
    Mask m;
    m.includes_ref().emplace()[5] = allMask();
    EXPECT_TRUE((MaskRef{m, false}).isFieldMask());
    EXPECT_FALSE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
  }
  {
    Mask m;
    m.excludes_ref().emplace()[5] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isFieldMask());
    EXPECT_FALSE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
  }
  {
    Mask m;
    m.excludes_map_ref().emplace()[5] = noneMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
  }
  {
    Mask m;
    m.includes_string_map_ref().emplace()["5"] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isStringMapMask());
  }
  {
    Mask m;
    m.excludes_string_map_ref().emplace()["5"] = noneMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isStringMapMask());
  }
}

TEST(FieldMaskTest, MaskRefGetMask) {
  {
    Mask m;
    m.includes_ref().emplace()[5] = allMask();
    EXPECT_EQ(getFieldMask(m), &*m.includes_ref());
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.excludes_ref().emplace()[5] = noneMask();
    EXPECT_EQ(getFieldMask(m), &*m.excludes_ref());
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[5] = allMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), &*m.includes_map_ref());
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.excludes_map_ref().emplace()[5] = noneMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), &*m.excludes_map_ref());
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.includes_string_map_ref().emplace()["5"] = allMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), &*m.includes_string_map_ref());
  }
  {
    Mask m;
    m.excludes_string_map_ref().emplace()["5"] = noneMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), &*m.excludes_string_map_ref());
  }
}

TEST(FieldMaskTest, ThrowIfContainsMapMask) {
  throwIfContainsMapMask(allMask()); // don't throw
  throwIfContainsMapMask(noneMask()); // don't throw
  {
    Mask m;
    m.includes_map_ref().emplace()[5] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_map_ref().emplace()[5] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    m.includes_string_map_ref().emplace()["5"] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_string_map_ref().emplace()["5"] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes_ref().emplace();
    includes[1] = allMask();
    includes[2].excludes_ref().emplace()[5] = noneMask();
    throwIfContainsMapMask(m); // don't throw
  }
  {
    Mask m;
    auto& includes = m.includes_ref().emplace();
    includes[1] = allMask();
    includes[2].includes_map_ref().emplace()[5] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes_ref().emplace();
    includes[1] = allMask();
    includes[2].excludes_map_ref().emplace()[5] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes_ref().emplace();
    includes[1] = allMask();
    includes[2].includes_string_map_ref().emplace()["5"] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes_ref().emplace();
    includes[1] = allMask();
    includes[2].excludes_string_map_ref().emplace()["5"] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
}

namespace {
template <typename Id>
auto getId(std::int16_t id) {
  if constexpr (std::is_same_v<Id, std::string>) {
    return std::to_string(id);
  } else {
    return Id{id};
  }
}
} // namespace

// test MaskRef get method with includes mask. nested is mask.includes[_map][9].
template <typename Id>
void testMaskRefGetIncludes(const Mask& mask, const Mask& nested) {
  MaskRef ref{mask, false};
  MaskRef refExclusion{mask, true};

  EXPECT_TRUE(ref.get(getId<Id>(7)).isNoneMask()); // doesn't exist
  EXPECT_TRUE(refExclusion.get(getId<Id>(7)).isAllMask()); // doesn't exist
  EXPECT_TRUE(ref.get(getId<Id>(8)).isAllMask());
  EXPECT_TRUE(refExclusion.get(getId<Id>(8)).isNoneMask());
  EXPECT_TRUE(literallyEqual(ref.get(getId<Id>(9)), (MaskRef{nested, false})));
  EXPECT_TRUE(
      literallyEqual(refExclusion.get(getId<Id>(9)), (MaskRef{nested, true})));
  // recursive calls to MaskRef Get
  EXPECT_TRUE(ref.get(getId<Id>(9)).get(getId<Id>(4)).isAllMask());
  EXPECT_TRUE(refExclusion.get(getId<Id>(9)).get(getId<Id>(4)).isNoneMask());
  EXPECT_TRUE(
      ref.get(getId<Id>(9)).get(getId<Id>(5)).isNoneMask()); // doesn't exist
  EXPECT_TRUE(refExclusion.get(getId<Id>(9))
                  .get(getId<Id>(5))
                  .isAllMask()); // doesn't exist
}

TEST(FieldMaskTest, MaskRefGetIncludes) {
  Mask mask;
  // includes{8: excludes{},
  //          9: includes{4: excludes{}}
  auto& includes = mask.includes_ref().emplace();
  includes[8] = allMask();
  includes[9].includes_ref().emplace()[4] = allMask();

  testMaskRefGetIncludes<FieldId>(mask, includes[9]);
}

TEST(FieldMaskTest, MaskRefGetIncludesMap) {
  Mask mask;
  // includes_map{8: excludes{},
  //              9: includes_map{4: excludes{}}
  auto& includes = mask.includes_map_ref().emplace();
  includes[8] = allMask();
  includes[9].includes_map_ref().emplace()[4] = allMask();

  testMaskRefGetIncludes<MapId>(mask, includes[9]);
}

TEST(FieldMaskTest, MaskRefGetIncludesStringMap) {
  Mask mask;
  // includes_string_map{"8": excludes{},
  //                     "9": includes_string_map{"4": excludes{}}
  auto& includes = mask.includes_string_map_ref().emplace();
  includes["8"] = allMask();
  includes["9"].includes_string_map_ref().emplace()["4"] = allMask();

  testMaskRefGetIncludes<std::string>(mask, includes["9"]);
}

// test MaskRef get method with excludes mask. nested is mask.excludes[_map][9].
template <typename Id>
void testMaskRefGetExcludes(const Mask& mask, const Mask& nested) {
  MaskRef ref{mask, false};
  MaskRef refExclusion{mask, true};

  EXPECT_TRUE(ref.get(getId<Id>(7)).isAllMask()); // doesn't exist
  EXPECT_TRUE(refExclusion.get(getId<Id>(7)).isNoneMask()); // doesn't exist
  EXPECT_TRUE(ref.get(getId<Id>(8)).isNoneMask());
  EXPECT_TRUE(refExclusion.get(getId<Id>(8)).isAllMask());
  EXPECT_TRUE(literallyEqual(ref.get(getId<Id>(9)), (MaskRef{nested, true})));
  EXPECT_TRUE(
      literallyEqual(refExclusion.get(getId<Id>(9)), (MaskRef{nested, false})));
  // recursive calls to MaskRef Get
  EXPECT_TRUE(ref.get(getId<Id>(9)).get(getId<Id>(4)).isNoneMask());
  EXPECT_TRUE(refExclusion.get(getId<Id>(9)).get(getId<Id>(4)).isAllMask());
  EXPECT_TRUE(
      ref.get(getId<Id>(9)).get(getId<Id>(5)).isAllMask()); // doesn't exist
  EXPECT_TRUE(refExclusion.get(getId<Id>(9))
                  .get(getId<Id>(5))
                  .isNoneMask()); // doesn't exist
}

TEST(FieldMaskTest, MaskRefGetExcludes) {
  Mask mask;
  // excludes{8: excludes{},
  //          9: includes{4: excludes{}}
  auto& excludes = mask.excludes_ref().emplace();
  excludes[8] = allMask();
  excludes[9].includes_ref().emplace()[4] = allMask();

  testMaskRefGetExcludes<FieldId>(mask, excludes[9]);
}

TEST(FieldMaskTest, MaskRefGetExcludesMap) {
  Mask mask;
  // excludes_map{8: excludes{},
  //              9: includes_map{4: excludes{}}
  auto& excludes = mask.excludes_map_ref().emplace();
  excludes[8] = allMask();
  excludes[9].includes_map_ref().emplace()[4] = allMask();

  testMaskRefGetExcludes<MapId>(mask, excludes[9]);
}

TEST(FieldMaskTest, MaskRefGetExcludesStringMap) {
  Mask mask;
  // excludes_string_map{"8": excludes{},
  //                     "9": includes_string_map{4: excludes{}}
  auto& excludes = mask.excludes_string_map_ref().emplace();
  excludes["8"] = allMask();
  excludes["9"].includes_string_map_ref().emplace()["4"] = allMask();

  testMaskRefGetExcludes<std::string>(mask, excludes["9"]);
}

TEST(FieldMaskTest, MaskRefGetAllMaskNoneMask) {
  {
    MaskRef ref{allMask(), false};
    EXPECT_TRUE(ref.get(FieldId{1}).isAllMask());
    EXPECT_TRUE(ref.get(MapId{1}).isAllMask());
    EXPECT_TRUE(ref.get("1").isAllMask());
  }
  {
    MaskRef ref{allMask(), true};
    EXPECT_TRUE(ref.get(FieldId{1}).isNoneMask());
    EXPECT_TRUE(ref.get(MapId{1}).isNoneMask());
    EXPECT_TRUE(ref.get("1").isNoneMask());
  }
  {
    MaskRef ref{noneMask(), false};
    EXPECT_TRUE(ref.get(FieldId{1}).isNoneMask());
    EXPECT_TRUE(ref.get(MapId{1}).isNoneMask());
    EXPECT_TRUE(ref.get("1").isNoneMask());
  }
  {
    MaskRef ref{noneMask(), true};
    EXPECT_TRUE(ref.get(FieldId{1}).isAllMask());
    EXPECT_TRUE(ref.get(MapId{1}).isAllMask());
    EXPECT_TRUE(ref.get("1").isAllMask());
  }
}

TEST(FieldMaskTest, MaskRefGetException) {
  {
    Mask m;
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get(MapId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("1"), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("1"), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map_ref().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_map_ref().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"), std::runtime_error);
  }
  {
    Mask m;
    m.includes_ref().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_ref().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"), std::runtime_error);
  }
  {
    Mask m;
    m.includes_string_map_ref().emplace()["4"] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_string_map_ref().emplace()["4"] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
  }
}

TEST(FieldMaskTest, SchemalessClear) {
  protocol::Object fooObject, barObject, bazObject;
  // bar{1: foo{1: baz{1: 30},
  //            2: 10},
  //     2: "40",
  //     3: 5}
  bazObject[FieldId{1}].ensure_i32() = 30;
  fooObject[FieldId{1}].ensure_object() = bazObject;
  fooObject[FieldId{2}].ensure_i32() = 10;
  barObject[FieldId{1}].ensure_object() = fooObject;
  barObject[FieldId{2}].ensure_string() = "40";
  barObject[FieldId{3}].ensure_i32() = 5;

  Mask mask;
  // includes {2: excludes{},
  //           1: excludes{5: excludes{5: excludes{}},
  //                       1: excludes{}}}
  auto& includes = mask.includes_ref().emplace();
  includes[2] = allMask();
  auto& nestedExcludes = includes[1].excludes_ref().emplace();
  nestedExcludes[5].excludes_ref().emplace()[5] =
      allMask(); // The object doesn't have this field.
  nestedExcludes[1] = allMask();
  // This clears object[1][2] and object[2].
  protocol::clear(mask, barObject);

  ASSERT_TRUE(barObject.contains(FieldId{1}));
  protocol::Object& foo = barObject.at(FieldId{1}).as_object();
  ASSERT_TRUE(foo.contains(FieldId{1}));
  ASSERT_TRUE(foo.at(FieldId{1}).objectValue_ref()->contains(FieldId{1}));
  EXPECT_EQ(foo.at(FieldId{1}).objectValue_ref()->at(FieldId{1}).as_i32(), 30);
  EXPECT_FALSE(foo.contains(FieldId{2}));
  EXPECT_FALSE(barObject.contains(FieldId{2}));
  EXPECT_TRUE(barObject.contains(FieldId{3}));
  EXPECT_EQ(barObject.at(FieldId{3}).as_i32(), 5);
}

TEST(FieldMaskTest, SchemalessClearException) {
  {
    protocol::Object barObject;
    // bar{2: "40"}
    barObject[FieldId{2}].ensure_string() = "40";
    Mask m; // object[2] is not an object but has an object mask.
    auto& includes = m.includes_ref().emplace();
    includes[2].includes_ref().emplace()[4] = noneMask();
    EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
  }

  {
    protocol::Object fooObject, barObject;
    // bar{1: foo{2: 20}, 2: "40"}
    fooObject[FieldId{2}].ensure_i32() = 20;
    barObject[FieldId{1}].ensure_object() = fooObject;
    barObject[FieldId{2}].ensure_string() = "40";
    Mask m; // object[1][2] is not an object but has an object mask.
    auto& includes = m.includes_ref().emplace();
    includes[1].includes_ref().emplace()[2].excludes_ref().emplace()[5] =
        allMask();
    includes[2] = allMask();
    EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
  }

  {
    protocol::Object barObject;
    barObject[FieldId{1}] =
        asValueStruct<type::map<type::i32_t, type::string_t>>({{1, "1"}});
    Mask m; // object[1] is a map but has an object mask.
    m.includes_ref().emplace()[1].includes_ref().emplace()[1] = allMask();
    EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
  }
}

TEST(FieldMaskTest, SchemalessClearMap) {
  protocol::Object barObject;
  // bar{1: map{1: map{1: "1",
  //                   2: "2"},
  //            2: map{3: "3"}},
  //     2: "40",
  //     3: 5}
  barObject[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{1, "1"}, {2, "2"}}}, {2, {{3, "3"}}}});
  barObject[FieldId{2}].ensure_string() = "40";
  barObject[FieldId{3}].ensure_i32() = 5;

  Mask mask;
  // includes{2: excludes{},
  //          1: excludes_map{5: excludes_map{5: excludes{}},
  //                          1: includes_map{1: excludes{}}}}
  auto& includes = mask.includes_ref().emplace();
  includes[2] = allMask();
  auto& nestedExcludes = includes[1].excludes_map_ref().emplace();
  nestedExcludes[5].excludes_map_ref().emplace()[5] =
      allMask(); // The object doesn't have this field.
  nestedExcludes[1].includes_map_ref().emplace()[1] = allMask();
  // This clears object[1][2], object[1][1][2], and object[2].
  protocol::clear(mask, barObject);

  ASSERT_TRUE(barObject.contains(FieldId{1}));
  auto& m1 = barObject.at(FieldId{1}).mapValue_ref().value();
  ASSERT_NE(m1.find(asValueStruct<type::i64_t>(1)), m1.end());
  EXPECT_EQ(m1.find(asValueStruct<type::i64_t>(2)), m1.end());
  auto& m2 = m1[asValueStruct<type::i64_t>(1)].mapValue_ref().value();
  EXPECT_EQ(m2[asValueStruct<type::i16_t>(1)].stringValue_ref().value(), "1");
  EXPECT_EQ(m2.find(asValueStruct<type::i16_t>(2)), m2.end());
  EXPECT_FALSE(barObject.contains(FieldId{2}));
  ASSERT_TRUE(barObject.contains(FieldId{3}));
  EXPECT_EQ(barObject.at(FieldId{3}).as_i32(), 5);
}

TEST(FieldMaskTest, SchemalessClearStringMap) {
  protocol::Object barObject;
  // bar{1: map{"1": map{"1": "1",
  //                     "2": "2"},
  //            "2": map{"3": "3"}},
  //     2: "40",
  //     3: 5}
  barObject[FieldId{1}] = asValueStruct<
      type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
      {{"1", {{"1", "1"}, {"2", "2"}}}, {"2", {{"3", "3"}}}});
  barObject[FieldId{2}].ensure_string() = "40";
  barObject[FieldId{3}].ensure_i32() = 5;

  Mask mask;
  // includes{2: excludes{},
  //          1: excludes_string_map{"5": excludes_string_map{"5": excludes{}},
  //                                 "1": includes_string_map{"1": excludes{}}}}
  auto& includes = mask.includes_ref().emplace();
  includes[2] = allMask();
  auto& nestedExcludes = includes[1].excludes_string_map_ref().emplace();
  nestedExcludes["5"].excludes_string_map_ref().emplace()["5"] =
      allMask(); // The object doesn't have this field.
  nestedExcludes["1"].includes_string_map_ref().emplace()["1"] = allMask();
  // This clears object[1][2], object[1][1][2], and object[2].
  protocol::clear(mask, barObject);

  ASSERT_TRUE(barObject.contains(FieldId{1}));
  auto& m1 = barObject.at(FieldId{1}).mapValue_ref().value();
  ASSERT_NE(m1.find(asValueStruct<type::string_t>("1")), m1.end());
  EXPECT_EQ(m1.find(asValueStruct<type::string_t>("2")), m1.end());
  auto& m2 = m1[asValueStruct<type::string_t>("1")].mapValue_ref().value();
  EXPECT_EQ(
      m2[asValueStruct<type::string_t>("1")].stringValue_ref().value(), "1");
  EXPECT_EQ(m2.find(asValueStruct<type::string_t>("2")), m2.end());
  EXPECT_FALSE(barObject.contains(FieldId{2}));
  ASSERT_TRUE(barObject.contains(FieldId{3}));
  EXPECT_EQ(barObject.at(FieldId{3}).as_i32(), 5);
}

TEST(FieldMaskTest, SchemalessClearExceptionMap) {
  {
    protocol::Object barObject;
    // bar{2: "40"}
    barObject[FieldId{2}].ensure_string() = "40";
    {
      Mask m; // object[2] is not a map but has an integer map mask.
      auto& includes = m.includes_ref().emplace();
      includes[2].includes_map_ref().emplace()[4] = noneMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[2] is not a map but has a string map mask.
      auto& includes = m.includes_ref().emplace();
      includes[2].includes_string_map_ref().emplace()["4"] = noneMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
  }

  {
    protocol::Object fooObject, barObject;
    // bar{1: foo{2: 20}, 2: "40"}
    fooObject[FieldId{2}].ensure_i32() = 20;
    barObject[FieldId{1}].ensure_object() = fooObject;
    barObject[FieldId{2}].ensure_string() = "40";
    {
      Mask m; // object[1][2] is not a map but has an integer map mask.
      auto& includes = m.includes_ref().emplace();
      includes[1].includes_ref().emplace()[2].excludes_map_ref().emplace()[5] =
          allMask();
      includes[2] = allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[1][2] is not a map but has a string map mask.
      auto& includes = m.includes_ref().emplace();
      includes[1]
          .includes_ref()
          .emplace()[2]
          .excludes_string_map_ref()
          .emplace()["5"] = allMask();
      includes[2] = allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
  }

  {
    protocol::Object barObject, fooObject;
    barObject[FieldId{1}].ensure_object() = fooObject;
    {
      Mask m; // object[1] is an object but has an integer map mask.
      m.includes_ref().emplace()[1].includes_map_ref().emplace()[1] = allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[1] is an object but has a string map mask.
      m.includes_ref().emplace()[1].includes_string_map_ref().emplace()["1"] =
          allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
  }

  // Invalid map key type.
  {
    protocol::Object barObject;
    // bar{1: {[1]: [1]}}
    barObject[FieldId{1}] = asValueStruct<
        type::map<type::list<type::i32_t>, type::list<type::i32_t>>>(
        {{{1}, {1}}});
    {
      Mask m; // object[1] has invalid map key.
      m.includes_ref().emplace()[1].includes_map_ref().emplace();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[1] has invalid map key.
      m.includes_ref().emplace()[1].includes_string_map_ref().emplace();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
  }
}

// Calls copy on the mask, src, and dst.
// Then checks the read-write consistency of the copy.
void testCopy(
    const Mask& mask, const protocol::Object& src, protocol::Object& dst) {
  protocol::copy(mask, src, dst);
  // copy(dst, src) should be no-op.
  protocol::Object result;
  result = src;
  protocol::copy(mask, dst, result);
  EXPECT_EQ(result, src);
}

TEST(FieldMaskTest, SchemalessCopySimpleIncludes) {
  protocol::Object fooObject, barObject, bazObject;
  // foo{1: 10}
  // bar{1: "30", 2: 20}
  // baz = bar
  fooObject[FieldId{1}].ensure_i32() = 10;
  barObject[FieldId{2}].ensure_string() = "30";
  barObject[FieldId{2}].ensure_i32() = 20;
  bazObject = barObject;

  Mask m;
  // includes{1: exludes{}}
  auto& includes = m.includes_ref().emplace();
  includes[1] = allMask();
  testCopy(m, fooObject, barObject);
  // bar becomes bar{1: 10, 2: 20}
  ASSERT_TRUE(barObject.contains(FieldId{1}));
  EXPECT_EQ(barObject.at(FieldId{1}).as_i32(), 10);
  ASSERT_TRUE(barObject.contains(FieldId{2}));
  EXPECT_EQ(barObject.at(FieldId{2}).as_i32(), 20);

  testCopy(allMask(), fooObject, bazObject);
  // baz becomes baz{1: 10}
  ASSERT_TRUE(bazObject.contains(FieldId{1}));
  EXPECT_EQ(bazObject.at(FieldId{1}).as_i32(), 10);
  ASSERT_FALSE(bazObject.contains(FieldId{2}));

  // includes{1: exludes{}, 2: excludes{}, 3: excludes{}}
  includes[2] = allMask();
  includes[3] = allMask(); // no-op
  testCopy(m, barObject, bazObject);
  // baz becomes baz{1: 10, 2: 20}
  ASSERT_TRUE(bazObject.contains(FieldId{1}));
  EXPECT_EQ(bazObject.at(FieldId{1}).as_i32(), 10);
  ASSERT_TRUE(bazObject.contains(FieldId{2}));
  EXPECT_EQ(bazObject.at(FieldId{2}).as_i32(), 20);
  ASSERT_FALSE(bazObject.contains(FieldId{3}));
}

TEST(FieldMaskTest, SchemalessCopySimpleExcludes) {
  protocol::Object fooObject, barObject;
  // foo{1: 10, 3: 40}
  // bar{1: "30", 2: 20}
  fooObject[FieldId{1}].ensure_i32() = 10;
  fooObject[FieldId{3}].ensure_i32() = 40;
  barObject[FieldId{1}].ensure_string() = "30";
  barObject[FieldId{2}].ensure_i32() = 20;

  Mask m;
  // excludes{1: exludes{}}
  m.excludes_ref().emplace()[1] = allMask();
  testCopy(m, fooObject, barObject);
  // bar becomes bar{1: "30", 3: 40}
  ASSERT_TRUE(barObject.contains(FieldId{1}));
  EXPECT_EQ(barObject.at(FieldId{1}).as_string(), "30");
  ASSERT_TRUE(barObject.contains(FieldId{3}));
  EXPECT_EQ(barObject.at(FieldId{3}).as_i32(), 40);
  ASSERT_FALSE(barObject.contains(FieldId{2}));
}

TEST(FieldMaskTest, SchemalessCopyNestedRecursive) {
  protocol::Object fooObject, barObject;
  // bar{1: foo{1: 10,
  //            2: 20},
  //     2: "40"}
  fooObject[FieldId{1}].ensure_i32() = 10;
  fooObject[FieldId{2}].ensure_i32() = 20;
  barObject[FieldId{1}].ensure_object() = fooObject;
  barObject[FieldId{2}].ensure_string() = "40";

  protocol::Object dst = barObject;
  protocol::Object& nestedObject = dst[FieldId{1}].objectValue_ref().value();
  nestedObject[FieldId{1}].ensure_object() = fooObject;
  nestedObject[FieldId{2}].ensure_i32() = 30;
  // dst{1: {1: {1: 10,
  //             2: 20},
  //         2: 30},
  //     2: "40"}

  Mask m;
  // excludes{1: exludes{1: excludes{}}}
  m.excludes_ref().emplace()[1].excludes_ref().emplace()[1] = allMask();
  testCopy(m, barObject, dst);
  // dst becomes
  // dst{1: {1: 10
  //         2: 30},
  //     2: "40"}
  ASSERT_TRUE(dst.contains(FieldId{1}));
  protocol::Object& nested = dst.at(FieldId{1}).as_object();
  ASSERT_TRUE(nested.contains(FieldId{1}));
  EXPECT_EQ(nested.at(FieldId{1}).as_i32(), 10);
  ASSERT_TRUE(nested.contains(FieldId{2}));
  EXPECT_EQ(nested.at(FieldId{2}).as_i32(), 30);
  ASSERT_TRUE(dst.contains(FieldId{2}));
  EXPECT_EQ(dst.at(FieldId{2}).as_string(), "40");
}

TEST(FieldMaskTest, SchemalessCopyNestedAddField) {
  protocol::Object fooObject, barObject;
  // bar{1: foo{1: 10,
  //            2: 20},
  //     2: "40"}
  fooObject[FieldId{1}].ensure_i32() = 10;
  fooObject[FieldId{2}].ensure_i32() = 20;
  barObject[FieldId{1}].ensure_object() = fooObject;
  barObject[FieldId{2}].ensure_string() = "40";

  Mask m1;
  // includes{1: includes{2: excludes{}},
  //          2: excludes{}}
  auto& includes = m1.includes_ref().emplace();
  includes[1].includes_ref().emplace()[2] = allMask();
  includes[2] = allMask();

  protocol::Object dst1;
  testCopy(m1, barObject, dst1);
  // dst1 becomes dst1{1: {2: 20}, 2: "40"}
  ASSERT_TRUE(dst1.contains(FieldId{1}));
  protocol::Object& nested = dst1.at(FieldId{1}).as_object();
  ASSERT_TRUE(nested.contains(FieldId{2}));
  EXPECT_EQ(nested.at(FieldId{2}).as_i32(), 20);
  ASSERT_FALSE(nested.contains(FieldId{1}));
  ASSERT_TRUE(dst1.contains(FieldId{2}));
  EXPECT_EQ(dst1.at(FieldId{2}).as_string(), "40");

  Mask m2;
  // excludes{1: includes{1: excludes{}, 2: excludes{}, 3: includes{}},
  //          2: includes{}}
  auto& excludes = m2.excludes_ref().emplace();
  auto& nestedIncludes = excludes[1].includes_ref().emplace();
  nestedIncludes[1] = allMask();
  nestedIncludes[2] = allMask();
  nestedIncludes[3] = allMask();
  excludes[2] = noneMask();

  protocol::Object dst2;
  testCopy(m2, barObject, dst2);
  // dst2 becomes dst2{2: "40"} (doesn't create nested object).
  ASSERT_FALSE(dst2.contains(FieldId{1}));
  ASSERT_TRUE(dst2.contains(FieldId{2}));
  EXPECT_EQ(dst2.at(FieldId{2}).as_string(), "40");
}

TEST(FieldMaskTest, SchemalessCopyNestedRemoveField) {
  protocol::Object fooObject, barObject;
  // bar{1: foo{1: 10,
  //            2: 20},
  //     2: "40"}
  fooObject[FieldId{1}].ensure_i32() = 10;
  fooObject[FieldId{2}].ensure_i32() = 20;
  barObject[FieldId{1}].ensure_object() = fooObject;
  barObject[FieldId{2}].ensure_string() = "40";

  Mask m;
  // includes{1: includes{2: excludes{}},
  //          2: excludes{}}
  auto& includes = m.includes_ref().emplace();
  includes[1].includes_ref().emplace()[2] = allMask();
  includes[2] = allMask();

  protocol::Object src;
  testCopy(m, src, barObject);
  // bar becomes bar{1: foo{1: 10}} (doesn't delete foo object).
  ASSERT_TRUE(barObject.contains(FieldId{1}));
  protocol::Object& nested = barObject.at(FieldId{1}).as_object();
  ASSERT_TRUE(nested.contains(FieldId{1}));
  EXPECT_EQ(nested.at(FieldId{1}).as_i32(), 10);
  ASSERT_FALSE(nested.contains(FieldId{2}));
  ASSERT_FALSE(barObject.contains(FieldId{2}));
}

TEST(FieldMaskTest, SchemalessCopyException) {
  protocol::Object fooObject, barObject, bazObject;
  // bar{1: foo{2: 20}, 2: "40"}
  fooObject[FieldId{2}].ensure_i32() = 20;
  barObject[FieldId{1}].ensure_object() = fooObject;
  barObject[FieldId{2}].ensure_string() = "40";
  // baz{2: {3: 40}}
  bazObject[FieldId{2}].ensure_object()[FieldId{3}].ensure_i32() = 40;

  Mask m1; // bar[2] is not an object but has an object mask.
  m1.includes_ref().emplace()[2].includes_ref().emplace()[3] = allMask();
  protocol::Object copy = barObject;
  EXPECT_THROW(protocol::copy(m1, copy, barObject), std::runtime_error);
  protocol::Object empty;
  EXPECT_THROW(protocol::copy(m1, barObject, empty), std::runtime_error);
  EXPECT_THROW(protocol::copy(m1, empty, barObject), std::runtime_error);
  // baz[2] is an object, but since bar[2] is not, it still throws an error.
  EXPECT_THROW(protocol::copy(m1, barObject, bazObject), std::runtime_error);
  EXPECT_THROW(protocol::copy(m1, bazObject, barObject), std::runtime_error);
}

TEST(FieldMaskTest, SchemalessCopySimpleMap) {
  protocol::Object src, dst, expected;
  // src{1: map{1: "1",
  //            2: "2"}}
  src[FieldId{1}] = asValueStruct<type::map<type::i64_t, type::string_t>>(
      {{1, "1"}, {2, "2"}});
  // dst{1: map{1: "3",
  //            2: "4"}}
  dst[FieldId{1}] = asValueStruct<type::map<type::i64_t, type::string_t>>(
      {{1, "3"}, {2, "4"}});
  // expected{1: map{1: "1",
  //                 2: "4"}}
  expected[FieldId{1}] = asValueStruct<type::map<type::i64_t, type::string_t>>(
      {{1, "1"}, {2, "4"}});

  Mask mask;
  mask.includes_ref().emplace()[1].includes_map_ref().emplace()[1] = allMask();
  testCopy(mask, src, dst);
  EXPECT_EQ(dst, expected);
}

TEST(FieldMaskTest, SchemalessCopySimpleStringMap) {
  protocol::Object src, dst, expected;
  // src{1: map{"1": "1",
  //            "2": "2"}}
  {
    std::map<std::string, std::string> map = {{"1", "1"}, {"2", "2"}};
    src[FieldId{1}] =
        asValueStruct<type::map<type::string_t, type::string_t>>(map);
  }
  // dst{1: map{"1": "3",
  //            "2": "4"}}
  {
    std::map<std::string, std::string> map = {{"1", "3"}, {"2", "4"}};
    dst[FieldId{1}] =
        asValueStruct<type::map<type::string_t, type::string_t>>(map);
  }
  // expected{1: map{"1": "1",
  //                 "2": "4"}}
  {
    std::map<std::string, std::string> map = {{"1", "1"}, {"2", "4"}};
    expected[FieldId{1}] =
        asValueStruct<type::map<type::string_t, type::string_t>>(map);
  }

  Mask mask;
  mask.includes_ref().emplace()[1].includes_string_map_ref().emplace()["1"] =
      allMask();
  testCopy(mask, src, dst);
  EXPECT_EQ(dst, expected);
}

TEST(FieldMaskTest, SchemalessCopyNestedMap) {
  protocol::Object src, dst, expected;
  // src{1: map{1: map{1: "1",
  //                   2: "2"},
  //            2: map{3: "3"}}}
  src[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{1, "1"}, {2, "2"}}}, {2, {{3, "3"}}}});
  // dst{1: map{1: map{1: "5",
  //                   2: "6",
  //                   3: "7"}}}
  dst[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{1, "5"}, {2, "6"}, {3, "7"}}}});
  // expected{1: map{1: map{1: "1",
  //                        2: "6"}},
  //                 2: map{3: "3"}}}
  expected[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{1, "5"}, {2, "2"}}}, {2, {{3, "3"}}}});

  Mask mask;
  // includes{1: includes_map{1: excludes_map{1: allMask()},
  //                          2: allMask(),
  //                          5: excludes_map{5: allMask()}}}
  auto& nestedIncludes =
      mask.includes_ref().emplace()[1].includes_map_ref().emplace();
  nestedIncludes[1].excludes_map_ref().emplace()[1] = allMask();
  nestedIncludes[2] = allMask();
  nestedIncludes[5].excludes_map_ref().emplace()[5] =
      allMask(); // The object doesn't have this field.
  // this copies src[1][1][2], src[1][1][3], and src[1][2]
  testCopy(mask, src, dst);
  EXPECT_EQ(dst, expected);
}

TEST(FieldMaskTest, SchemalessCopyNestedStringMap) {
  protocol::Object src, dst, expected;
  // src{1: map{"1": map{"1": "1",
  //                     "2": "2"},
  //            "2": map{"3": "3"}}}
  {
    std::map<std::string, std::map<std::string, std::string>> map = {
        {"1", {{"1", "1"}, {"2", "2"}}}, {"2", {{"3", "3"}}}};
    src[FieldId{1}] = asValueStruct<
        type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
        map);
  }
  // dst{1: map{"1": map{"1": "5",
  //                     "2": "6",
  //                     "3": "7"}}}
  {
    std::map<std::string, std::map<std::string, std::string>> map = {
        {"1", {{"1", "5"}, {"2", "6"}, {"3", "7"}}}};
    dst[FieldId{1}] = asValueStruct<
        type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
        map);
  }
  // expected{1: map{"1": map{"1": "1",
  //                          "2": "6"}},
  //                 "2": map{"3": "3"}}}
  {
    std::map<std::string, std::map<std::string, std::string>> map = {
        {"1", {{"1", "5"}, {"2", "2"}}}, {"2", {{"3", "3"}}}};
    expected[FieldId{1}] = asValueStruct<
        type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
        map);
  }

  Mask mask;
  // includes{1: includes_string_map{"1": excludes_string_map{"1": allMask()},
  //                                 "2": allMask(),
  //                                 "5": excludes_string_map{"5": allMask()}}}
  auto& nestedIncludes =
      mask.includes_ref().emplace()[1].includes_string_map_ref().emplace();
  nestedIncludes["1"].excludes_string_map_ref().emplace()["1"] = allMask();
  nestedIncludes["2"] = allMask();
  nestedIncludes["5"].excludes_string_map_ref().emplace()["5"] =
      allMask(); // The object doesn't have this field.
  // this copies src[1][1][2], src[1][1][3], and src[1][2]
  testCopy(mask, src, dst);
  EXPECT_EQ(dst, expected);
}

TEST(FieldMaskTest, SchemalessCopyMapAddRemoveKey) {
  protocol::Object src;
  // src{1: map{1: map{1: "1",
  //                   2: "2"},
  //            2: map{3: "3"}}}
  src[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{1, "1"}, {2, "2"}}}, {2, {{3, "3"}}}});
  {
    Mask mask;
    mask.includes_ref()
        .emplace()[1]
        .includes_map_ref()
        .emplace()[1]
        .includes_map_ref()
        .emplace()[1] = allMask();

    protocol::Object dst, expected;
    expected[FieldId{1}] = asValueStruct<
        type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
        {{1, {{1, "1"}}}});

    // This creates a new map at dst[1]
    testCopy(mask, src, dst);
    EXPECT_EQ(dst, expected);
  }
  {
    Mask mask;
    mask.includes_ref()
        .emplace()[1]
        .includes_map_ref()
        .emplace()[1]
        .includes_map_ref()
        .emplace()[3] = allMask();

    protocol::Object dst, expected;

    // This doesn't create a new map at dst[1]
    testCopy(mask, src, dst);
    EXPECT_EQ(dst, expected);
  }
}

TEST(FieldMaskTest, SchemalessCopyStringMapAddRemoveKey) {
  protocol::Object src;
  // src{1: map{"1": map{"1": "1",
  //                     "2": "2"},
  //            "2": map{"3": "3"}}}
  {
    std::map<std::string, std::map<std::string, std::string>> map = {
        {"1", {{"1", "1"}, {"2", "2"}}}, {"2", {{"3", "3"}}}};
    src[FieldId{1}] = asValueStruct<
        type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
        map);
  }
  {
    Mask mask;
    mask.includes_ref()
        .emplace()[1]
        .includes_string_map_ref()
        .emplace()["1"]
        .includes_string_map_ref()
        .emplace()["1"] = allMask();

    protocol::Object dst, expected;
    std::map<std::string, std::map<std::string, std::string>> map = {
        {"1", {{"1", "1"}}}};
    expected[FieldId{1}] = asValueStruct<
        type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
        map);

    // This creates a new map at dst[1]
    testCopy(mask, src, dst);
    EXPECT_EQ(dst, expected);
  }
  {
    Mask mask;
    mask.includes_ref()
        .emplace()[1]
        .includes_string_map_ref()
        .emplace()["1"]
        .includes_string_map_ref()
        .emplace()["3"] = allMask();

    protocol::Object dst, expected;

    // This doesn't create a new map at dst[1]
    testCopy(mask, src, dst);
    EXPECT_EQ(dst, expected);
  }
}

TEST(FieldMaskTest, SchemalessCopyExceptionMap) {
  protocol::Object barObject, bazObject;
  // bar{1: map{2: 20}, 2: "40"}
  barObject[FieldId{1}] =
      asValueStruct<type::map<type::byte_t, type::i32_t>>({{2, 20}});
  barObject[FieldId{2}].ensure_string() = "40";
  // baz{2: map{3: 40}}
  bazObject[FieldId{2}] =
      asValueStruct<type::map<type::byte_t, type::i32_t>>({{3, 40}});

  Mask m1; // bar[2] is not a map but has an integer map mask.
  m1.includes_ref().emplace()[2].includes_map_ref().emplace()[3] = allMask();
  protocol::Object copy = barObject;
  EXPECT_THROW(protocol::copy(m1, copy, barObject), std::runtime_error);
  protocol::Object empty;
  EXPECT_THROW(protocol::copy(m1, barObject, empty), std::runtime_error);
  EXPECT_THROW(protocol::copy(m1, empty, barObject), std::runtime_error);
  // baz[2] is a map, but since bar[2] is not, it still throws an error.
  EXPECT_THROW(protocol::copy(m1, barObject, bazObject), std::runtime_error);
  EXPECT_THROW(protocol::copy(m1, bazObject, barObject), std::runtime_error);
}

TEST(FieldMaskTest, SchemalessCopyExceptionStringMap) {
  protocol::Object barObject, bazObject;
  // bar{1: map{"2": 20}, 2: "40"}
  barObject[FieldId{1}] =
      asValueStruct<type::map<type::string_t, type::i32_t>>({{"2", 20}});
  barObject[FieldId{2}].ensure_string() = "40";
  // baz{2: map{"3": 40}}
  bazObject[FieldId{2}] =
      asValueStruct<type::map<type::string_t, type::i32_t>>({{"3", 40}});

  Mask m1; // bar[2] is not a map but has a string map mask.
  m1.includes_ref().emplace()[2].includes_string_map_ref().emplace()["3"] =
      allMask();
  protocol::Object copy = barObject;
  EXPECT_THROW(protocol::copy(m1, copy, barObject), std::runtime_error);
  protocol::Object empty;
  EXPECT_THROW(protocol::copy(m1, barObject, empty), std::runtime_error);
  EXPECT_THROW(protocol::copy(m1, empty, barObject), std::runtime_error);
  // baz[2] is a map, but since bar[2] is not, it still throws an error.
  EXPECT_THROW(protocol::copy(m1, barObject, bazObject), std::runtime_error);
  EXPECT_THROW(protocol::copy(m1, bazObject, barObject), std::runtime_error);
}

TEST(FieldMaskTest, IsCompatibleWithSimple) {
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Foo>>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Foo>>(noneMask()));

  Mask m;
  auto& includes = m.includes_ref().emplace();
  includes[1] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Foo>>(m));
  includes[3] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Foo>>(m));

  includes[2] = allMask(); // doesn't exist
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Foo>>(m));
}

TEST(FieldMaskTest, IsCompatibleWithNested) {
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(noneMask()));

  // These are valid field masks for Bar.
  Mask m;
  // includes{1: excludes{}}
  m.includes_ref().emplace()[1] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{2: includes{}}
  m.includes_ref().emplace()[2] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{1: includes{1: excludes{}}, 2: excludes{}}
  auto& includes = m.includes_ref().emplace();
  includes[1].includes_ref().emplace()[1] = allMask();
  includes[2] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(m));

  // There are invalid field masks for Bar.
  // includes{3: excludes{}}
  m.includes_ref().emplace()[3] = allMask();
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{2: includes{1: includes{}}}
  m.includes_ref().emplace()[2].includes_ref().emplace()[1] = noneMask();
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{1: excludes{2: excludes{}}}
  m.includes_ref().emplace()[1].excludes_ref().emplace()[2] = allMask();
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
}

TEST(FieldMaskTest, IsCompatibleWithAdaptedField) {
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Baz>>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Baz>>(noneMask()));

  Mask m;
  // includes{1: excludes{}}
  m.includes_ref().emplace()[1] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Baz>>(m));
  // includes{1: includes{1: excludes{}}}
  m.includes_ref().emplace()[1].includes_ref().emplace()[1] = allMask();
  // adapted struct field is treated as non struct field.
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Baz>>(m));
}

TEST(FieldMaskTest, IsCompatibleWithSmartPointer) {
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(
      allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(
      noneMask()));

  {
    Mask mask;
    // includes{1: excludes{},
    //          2: excludes{},
    //          3: includes{}}
    auto& includes = mask.includes_ref().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = noneMask();
    EXPECT_TRUE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }

  {
    Mask mask;
    // includes{1: excludes{2: excludes{}},
    //          2: excludes{2: includes{}},
    //          3: includes{1: excludes{}}}
    auto& includes = mask.includes_ref().emplace();
    includes[1].excludes_ref().emplace()[2] = allMask();
    includes[2].excludes_ref().emplace()[2] = noneMask();
    includes[3].includes_ref().emplace()[1] = allMask();
    EXPECT_TRUE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
  {
    Mask mask;
    // includes{1: excludes{100: excludes{}}}
    mask.includes_ref().emplace()[1].excludes_ref().emplace()[100] = allMask();
    EXPECT_FALSE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
  {
    Mask mask;
    // includes{2: includes{100: excludes{}}}
    mask.includes_ref().emplace()[2].includes_ref().emplace()[100] = allMask();
    EXPECT_FALSE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
  {
    Mask mask;
    // includes{3: includes{1: excludes{1: excludes{}}}}
    auto& includes = mask.includes_ref().emplace();
    includes[3].includes_ref().emplace()[1].excludes_ref().emplace()[1] =
        allMask();
    EXPECT_FALSE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
}

TEST(FieldMaskTest, IsCompatibleWithMap) {
  Mask m;
  auto& includes = m.includes_map_ref().emplace()[1].includes_ref().emplace();
  includes[1] = allMask();
  using Tag = type::map<type::i32_t, type::struct_t<Foo>>;
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[3] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[2] = allMask(); // field 2 doesn't exist in Foo
  EXPECT_FALSE(protocol::is_compatible_with<Tag>(m));
}

TEST(FieldMaskTest, IsCompatibleWithStringMap) {
  Mask m;
  auto& includes =
      m.includes_string_map_ref().emplace()["1"].includes_ref().emplace();
  includes[1] = allMask();
  using Tag = type::map<type::string_t, type::struct_t<Foo>>;
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[3] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[2] = allMask(); // field 2 doesn't exist in Foo
  EXPECT_FALSE(protocol::is_compatible_with<Tag>(m));
}

TEST(FieldMaskTest, IsCompatibleWithNestedMap) {
  Mask m;
  auto& includes = m.includes_map_ref()
                       .emplace()[1]
                       .includes_map_ref()
                       .emplace()[1]
                       .includes_ref()
                       .emplace();
  includes[1] = allMask();
  using Tag =
      type::map<type::i32_t, type::map<type::i64_t, type::struct_t<Foo>>>;
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[3] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[2] = allMask(); // field 2 doesn't exist in Foo
  EXPECT_FALSE(protocol::is_compatible_with<Tag>(m));
}

TEST(FieldMaskTest, IsCompatibleWithNestedStringMap) {
  Mask m;
  auto& includes = m.includes_string_map_ref()
                       .emplace()["1"]
                       .includes_string_map_ref()
                       .emplace()["1"]
                       .includes_ref()
                       .emplace();
  includes[1] = allMask();
  using Tag =
      type::map<type::string_t, type::map<type::string_t, type::struct_t<Foo>>>;
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[3] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<Tag>(m));
  includes[2] = allMask(); // field 2 doesn't exist in Foo
  EXPECT_FALSE(protocol::is_compatible_with<Tag>(m));
}

TEST(FieldMaskTest, IsCompatibleWithOtherTypes) {
  Mask mask;
  mask.includes_ref().emplace()[1] = allMask();

  EXPECT_TRUE(protocol::is_compatible_with<type::i32_t>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::i32_t>(noneMask()));
  EXPECT_FALSE(protocol::is_compatible_with<type::i32_t>(mask));
  EXPECT_TRUE(
      protocol::is_compatible_with<type::list<type::string_t>>(allMask()));
  EXPECT_TRUE(
      protocol::is_compatible_with<type::list<type::string_t>>(noneMask()));
  EXPECT_FALSE(protocol::is_compatible_with<type::list<type::string_t>>(mask));
}

TEST(FieldMaskTest, Ensure) {
  Mask mask;
  // mask = includes{1: includes{2: excludes{}},
  //                 2: excludes{}}
  auto& includes = mask.includes_ref().emplace();
  includes[1].includes_ref().emplace()[2] = allMask();
  includes[2] = allMask();

  {
    Bar2 bar;
    ensure(mask, bar);
    ASSERT_TRUE(bar.field_3().has_value());
    ASSERT_FALSE(bar.field_3()->field_1().has_value());
    ASSERT_TRUE(bar.field_3()->field_2().has_value());
    EXPECT_EQ(bar.field_3()->field_2(), 0);
    ASSERT_TRUE(bar.field_4().has_value());
    EXPECT_EQ(bar.field_4(), "");
  }

  // mask = includes{1: includes{2: excludes{}},
  //                 2: includes{}}
  includes[2] = noneMask();

  {
    Bar2 bar;
    ensure(mask, bar);
    ASSERT_TRUE(bar.field_3().has_value());
    ASSERT_FALSE(bar.field_3()->field_1().has_value());
    ASSERT_TRUE(bar.field_3()->field_2().has_value());
    EXPECT_EQ(bar.field_3()->field_2(), 0);
    ASSERT_FALSE(bar.field_4().has_value());
  }

  // test excludes mask
  // mask = excludes{1: includes{1: excludes{},
  //                             2: excludes{}}}
  auto& excludes = mask.excludes_ref().emplace();
  auto& nestedIncludes = excludes[1].includes_ref().emplace();
  nestedIncludes[1] = allMask();
  nestedIncludes[2] = allMask();

  {
    Bar2 bar;
    ensure(mask, bar);
    ASSERT_TRUE(bar.field_3().has_value());
    ASSERT_FALSE(bar.field_3()->field_1().has_value());
    ASSERT_FALSE(bar.field_3()->field_2().has_value());
    ASSERT_TRUE(bar.field_4().has_value());
    EXPECT_EQ(bar.field_4(), "");
  }
}

void assertSmartPointerStructIsEmpty(SmartPointerStruct& obj) {
  EXPECT_FALSE(bool(obj.unique_ref()));
  EXPECT_FALSE(bool(obj.shared_ref()));
  EXPECT_FALSE(bool(obj.boxed_ref()));
}

void assertPointerHasAllValues(auto&& ptr) {
  ASSERT_TRUE(bool(ptr));
  EXPECT_EQ(ptr->field_1(), 0);
  EXPECT_EQ(ptr->field_2(), 0);
}

void assertSmartPointerStructHasAllValues(SmartPointerStruct& obj) {
  assertPointerHasAllValues(obj.unique_ref());
  assertPointerHasAllValues(obj.shared_ref());
  assertPointerHasAllValues(obj.boxed_ref());
}

TEST(FieldMaskTest, EnsureSmartPointer) {
  // test with allMask and noneMask
  {
    SmartPointerStruct obj;
    protocol::ensure(noneMask(), obj);
    assertSmartPointerStructIsEmpty(obj);

    protocol::ensure(allMask(), obj);
    assertSmartPointerStructHasAllValues(obj);

    protocol::ensure(allMask(), obj);
    assertSmartPointerStructHasAllValues(obj);
  }

  {
    SmartPointerStruct obj;
    // mask = includes{1: excludes{}}
    MaskBuilder<SmartPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    protocol::ensure(builder.toThrift(), obj);
    assertPointerHasAllValues(obj.unique_ref());
    EXPECT_FALSE(bool(obj.shared_ref()));
    EXPECT_FALSE(bool(obj.boxed_ref()));
  }
  {
    SmartPointerStruct obj;
    // Mask includes unique.field_1 and boxed except for boxed.field_2.
    MaskBuilder<SmartPointerStruct> builder(noneMask());
    builder.includes<ident::unique, ident::field_1>();
    builder.includes<ident::boxed>();
    builder.excludes<ident::boxed, ident::field_2>();
    protocol::ensure(builder.toThrift(), obj);
    ASSERT_TRUE(bool(obj.unique_ref()));
    EXPECT_EQ(obj.unique_ref()->field_1(), 0);
    EXPECT_FALSE(obj.unique_ref()->field_2().has_value());
    EXPECT_FALSE(bool(obj.shared_ref()));
    ASSERT_TRUE(bool(obj.boxed_ref()));
    EXPECT_EQ(obj.boxed_ref()->field_1(), 0);
    EXPECT_FALSE(obj.boxed_ref()->field_2().has_value());
  }

  // Test ensure works with struct that has a shared const pointer field.
  {
    SharedConstPointerStruct obj;
    MaskBuilder<SharedConstPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    builder.ensure(obj);
    assertPointerHasAllValues(obj.unique_ref());
    EXPECT_FALSE(obj.shared_const_ref());

    // Cannot ensure the shared const field.
    builder.includes<ident::shared_const>();
    EXPECT_THROW(builder.ensure(obj), std::runtime_error);
  }
}

TEST(FieldMaskTest, EnsureException) {
  Mask mask;
  // mask = includes{1: includes{2: excludes{1: includes{}}}}
  mask.includes_ref()
      .emplace()[1]
      .includes_ref()
      .emplace()[2]
      .excludes_ref()
      .emplace()[1] = noneMask();
  Bar2 bar;
  EXPECT_THROW(ensure(mask, bar), std::runtime_error); // incompatible

  // includes{1: includes{1: excludes{}}}
  mask.includes_ref().emplace()[1].includes_ref().emplace()[1] = allMask();
  Baz baz;
  // adapted field cannot be masked.
  EXPECT_THROW(ensure(mask, baz), std::runtime_error);

  // includes{3: excludes{}}
  mask.includes_ref().emplace()[3] = allMask();
  EXPECT_THROW(ensure(mask, bar), std::runtime_error); // incompatible
}

TEST(FieldMaskTest, SchemafulClear) {
  Mask mask;
  // mask = includes{1: includes{2: excludes{}},
  //                 2: excludes{}}
  auto& includes = mask.includes_ref().emplace();
  includes[1].includes_ref().emplace()[2] = allMask();
  includes[2] = allMask();

  {
    Bar2 bar;
    ensure(mask, bar);
    protocol::clear(mask, bar); // clear fields that are ensured
    ASSERT_TRUE(bar.field_3().has_value());
    ASSERT_FALSE(bar.field_3()->field_1().has_value());
    ASSERT_FALSE(bar.field_3()->field_2().has_value());
    ASSERT_TRUE(bar.field_4().has_value());
    EXPECT_EQ(bar.field_4().value(), "");
  }

  {
    Bar2 bar;
    protocol::clear(mask, bar); // no-op
    ASSERT_FALSE(bar.field_3().has_value());
    ASSERT_FALSE(bar.field_4().has_value());
  }

  {
    Bar2 bar;
    bar.field_4() = "Hello";
    protocol::clear(noneMask(), bar); // no-op
    ASSERT_TRUE(bar.field_4().has_value());
    EXPECT_EQ(bar.field_4().value(), "Hello");

    protocol::clear(allMask(), bar);
    ASSERT_TRUE(bar.field_4().has_value());
    EXPECT_EQ(bar.field_4().value(), "");
  }

  // test optional fields
  {
    Bar2 bar;
    bar.field_3().ensure();
    protocol::clear(allMask(), bar);
    EXPECT_FALSE(bar.field_3().has_value());
  }
  {
    Bar2 bar;
    bar.field_3().ensure();
    bar.field_3()->field_1() = 1;
    bar.field_3()->field_2() = 2;
    protocol::clear(mask, bar);
    ASSERT_TRUE(bar.field_3().has_value());
    ASSERT_TRUE(bar.field_3()->field_1().has_value());
    EXPECT_EQ(bar.field_3()->field_1(), 1);
    ASSERT_FALSE(bar.field_3()->field_2().has_value());
    ASSERT_FALSE(bar.field_4().has_value());

    protocol::clear(allMask(), bar);
    ASSERT_FALSE(bar.field_3().has_value());
    ASSERT_FALSE(bar.field_4().has_value());
  }
}

TEST(FieldMaskTest, SchemafulClearExcludes) {
  // tests excludes mask
  // mask2 = excludes{1: includes{1: excludes{}}}
  Mask mask;
  auto& excludes = mask.excludes_ref().emplace();
  excludes[1].includes_ref().emplace()[1] = allMask();
  Bar bar;
  bar.foo().emplace().field1() = 1;
  bar.foo()->field2() = 2;
  protocol::clear(mask, bar);
  ASSERT_TRUE(bar.foo().has_value());
  ASSERT_TRUE(bar.foo()->field1().has_value());
  EXPECT_EQ(bar.foo()->field1(), 1);
  ASSERT_TRUE(bar.foo()->field2().has_value());
  EXPECT_EQ(bar.foo()->field2(), 0);
  ASSERT_FALSE(bar.foos().has_value());
}

TEST(FieldMaskTest, SchemafulClearEdgeCase) {
  // Clear sets the field to intristic default even if the field isn't set.
  CustomDefault obj;
  protocol::clear(allMask(), obj);
  EXPECT_EQ(obj.field(), "");

  // Makes sure clear doesn't use has_value() for all fields.
  TerseWrite obj2;
  protocol::clear(allMask(), obj2);

  Mask m;
  m.includes_ref().emplace()[2].includes_ref().emplace()[1] = allMask();
  protocol::clear(m, obj2);
}

TEST(FieldMaskTest, SchemafulClearSmartPointer) {
  // test with allMask and noneMask
  {
    SmartPointerStruct obj;
    protocol::ensure(allMask(), obj);
    protocol::clear(noneMask(), obj);
    assertSmartPointerStructHasAllValues(obj);

    protocol::clear(allMask(), obj);
    assertSmartPointerStructIsEmpty(obj);

    protocol::clear(allMask(), obj);
    assertSmartPointerStructIsEmpty(obj);
  }

  {
    SmartPointerStruct obj;
    protocol::ensure(allMask(), obj);
    // mask = includes{1: excludes{}}
    MaskBuilder<SmartPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    protocol::clear(builder.toThrift(), obj);
    EXPECT_FALSE(bool(obj.unique_ref()));
    assertPointerHasAllValues(obj.shared_ref());
    assertPointerHasAllValues(obj.boxed_ref());
  }

  {
    SmartPointerStruct obj;
    MaskBuilder<SmartPointerStruct> setup(allMask());
    setup.excludes<ident::shared, ident::field_1>();
    protocol::ensure(setup.toThrift(), obj);
    // mask = excludes{1: includes{1: excludes{}},
    //                 3: excludes{}}
    MaskBuilder<SmartPointerStruct> builder(allMask());
    builder.excludes<ident::unique, ident::field_1>();
    builder.excludes<ident::boxed>();
    protocol::clear(builder.toThrift(), obj);
    ASSERT_TRUE(bool(obj.unique_ref()));
    EXPECT_EQ(obj.unique_ref()->field_1(), 0);
    EXPECT_FALSE(obj.unique_ref()->field_2().has_value());
    EXPECT_FALSE(bool(obj.shared_ref()));
    assertPointerHasAllValues(obj.boxed());
  }

  // Test clear works with struct that has a shared const pointer field.
  {
    SharedConstPointerStruct obj;
    MaskBuilder<SharedConstPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    builder.ensure(obj);
    builder.clear(obj);
    EXPECT_EQ(*obj.unique_ref(), Foo2{});
    EXPECT_FALSE(obj.shared_const_ref());

    // Cannot clear a field inside the shared const field.
    builder.includes<ident::shared_const, ident::field_1>();
    obj.shared_const_ref() = std::make_shared<Foo2>(Foo2{});
    EXPECT_THROW(builder.clear(obj), std::runtime_error);
  }
}

TEST(FieldMaskTest, SchemafulClearException) {
  Bar2 bar;
  Mask m1; // m1 = includes{2: includes{4: includes{}}}
  auto& includes = m1.includes_ref().emplace();
  includes[2].includes_ref().emplace()[4] = noneMask();
  EXPECT_THROW(protocol::clear(m1, bar), std::runtime_error);

  Mask m2; // m2 = includes{1: includes{2: includes{5: excludes{}}}}
  auto& includes2 = m2.includes_ref().emplace();
  includes2[1].includes_ref().emplace()[2].excludes_ref().emplace()[5] =
      allMask();
  includes2[2] = allMask();
  EXPECT_THROW(protocol::clear(m2, bar), std::runtime_error);
}

TEST(FieldMaskTest, SchemafulCopy) {
  Bar2 src, empty;
  // src = {1: {1: 10, 2: 20}, 2: "40"}
  src.field_3().ensure().field_1() = 10;
  src.field_3()->field_2() = 20;
  src.field_4() = "40";

  Mask mask;
  // includes{1: includes{2: excludes{}},
  //          2: excludes{}}
  auto& includes = mask.includes_ref().emplace();
  includes[1].includes_ref().emplace()[2] = allMask();
  includes[2] = allMask();

  // copy empty
  {
    Bar2 dst;
    copy(mask, empty, dst);
    EXPECT_EQ(dst, empty);
  }

  // test copy field to dst
  {
    Bar2 dst;
    dst.field_3().ensure().field_1() = 30;
    dst.field_3()->field_2() = 40;
    copy(mask, src, dst);
    ASSERT_TRUE(dst.field_3().has_value());
    ASSERT_TRUE(dst.field_3()->field_1().has_value());
    EXPECT_EQ(dst.field_3()->field_1().value(), 30);
    ASSERT_TRUE(dst.field_3()->field_2().has_value());
    EXPECT_EQ(dst.field_3()->field_2().value(), 20);
    ASSERT_TRUE(dst.field_4().has_value());
    EXPECT_EQ(dst.field_4(), "40");
  }

  // test add field to dst
  {
    Bar2 dst;
    copy(mask, src, dst);
    ASSERT_TRUE(dst.field_3().has_value());
    ASSERT_FALSE(dst.field_3()->field_1().has_value());
    ASSERT_TRUE(dst.field_3()->field_2().has_value());
    EXPECT_EQ(dst.field_3()->field_2().value(), 20);
    EXPECT_EQ(dst.field_4(), "40");
  }

  // test remove field from src
  {
    Bar2 dst;
    copy(mask, empty, src);
    // src = {1: {1: 10}, 2: "40"}
    ASSERT_TRUE(src.field_3().has_value());
    ASSERT_TRUE(src.field_3()->field_1().has_value());
    EXPECT_EQ(src.field_3()->field_1().value(), 10);
    ASSERT_FALSE(src.field_3()->field_2().has_value());
    EXPECT_EQ(src.field_4(), "");

    src.field_4() = "40";
    copy(mask, src, dst); // this does not create a new object.
    ASSERT_FALSE(dst.field_3().has_value());
    ASSERT_TRUE(dst.field_4().has_value());
    EXPECT_EQ(dst.field_4().value(), "40");

    copy(allMask(), empty, src);
    ASSERT_FALSE(src.field_3().has_value());
    EXPECT_EQ(src.field_4(), "");
  }
}
TEST(FieldMaskTest, SchemafulCopyExcludes) {
  // test excludes mask
  // mask2 = excludes{1: includes{1: excludes{}}}
  Mask mask;
  auto& excludes = mask.excludes_ref().emplace();
  excludes[1].includes_ref().emplace()[1] = allMask();
  Bar src, dst;
  src.foo().emplace().field1() = 1;
  src.foo()->field2() = 2;
  protocol::copy(mask, src, dst);
  ASSERT_FALSE(dst.foo()->field1().has_value());
  ASSERT_TRUE(dst.foo()->field2().has_value());
  EXPECT_EQ(dst.foo()->field2(), 2);
  ASSERT_FALSE(dst.foos().has_value());
}

TEST(FieldMaskTest, SchemafulCopyTerseWrite) {
  // Makes sure copy doesn't use has_value() for all fields.
  TerseWrite src, dst;
  src.field() = 4;
  src.foo()->field1() = 5;
  protocol::copy(allMask(), src, dst);
  EXPECT_EQ(dst, src);

  Mask m;
  m.includes_ref().emplace()[2].includes_ref().emplace()[1] = allMask();
  protocol::copy(m, src, dst);
  EXPECT_EQ(dst, src);
}

TEST(FieldMaskTest, SchemafulCopySmartPointer) {
  // test with allMask and noneMask
  {
    SmartPointerStruct full, dst, empty;
    protocol::ensure(allMask(), full);

    protocol::copy(allMask(), empty, dst);
    assertSmartPointerStructIsEmpty(dst);

    protocol::copy(noneMask(), full, dst);
    assertSmartPointerStructIsEmpty(dst);

    protocol::copy(allMask(), full, dst);
    assertSmartPointerStructHasAllValues(dst);
    protocol::copy(allMask(), full, dst);
    assertSmartPointerStructHasAllValues(dst);

    protocol::copy(noneMask(), empty, dst);
    assertSmartPointerStructHasAllValues(dst);
    protocol::copy(allMask(), empty, dst);
    assertSmartPointerStructIsEmpty(dst);
  }

  // Test copy works with struct that has a shared const pointer field.
  {
    SharedConstPointerStruct src, dst;
    MaskBuilder<SharedConstPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    builder.ensure(src);
    builder.copy(src, dst);
    assertPointerHasAllValues(dst.unique_ref());

    // Cannot copy to a field inside the shared const field.
    builder.includes<ident::shared_const, ident::field_1>();
    src.shared_const_ref() = std::make_shared<Foo2>(Foo2{});
    dst.shared_const_ref() = std::make_shared<Foo2>(Foo2{});
    EXPECT_THROW(builder.copy(src, dst), std::runtime_error);
  }
}

TEST(FieldMaskTest, SchemafulCopySmartPointerAddField) {
  SmartPointerStruct src, dst;
  // src contains unique field except for unique.field_1 and box field.
  MaskBuilder<SmartPointerStruct> setup(allMask());
  setup.excludes<ident::unique, ident::field_1>();
  setup.excludes<ident::shared>();
  protocol::ensure(setup.toThrift(), src);
  {
    MaskBuilder<SmartPointerStruct> builder(noneMask());
    builder.includes<ident::unique, ident::field_1>();
    protocol::copy(builder.toThrift(), src, dst); // doesn't create object
    assertSmartPointerStructIsEmpty(dst);
  }
  {
    MaskBuilder<SmartPointerStruct> builder2(noneMask());
    builder2.includes<ident::boxed, ident::field_1>();
    protocol::copy(builder2.toThrift(), src, dst); // creates object
    EXPECT_FALSE(bool(dst.unique_ref()));
    EXPECT_FALSE(bool(dst.shared_ref()));
    ASSERT_TRUE(bool(dst.boxed_ref()));
    EXPECT_EQ(dst.boxed_ref()->field_1(), 0);
    EXPECT_FALSE(dst.boxed_ref()->field_2().has_value());
  }
}

TEST(FieldMaskTest, SchemafulCopySmartPointerRemoveField) {
  SmartPointerStruct src, dst;
  // dst contains unique field except for unique.field_1 and box field.
  MaskBuilder<SmartPointerStruct> setup(allMask());
  setup.excludes<ident::unique, ident::field_1>();
  setup.excludes<ident::shared>();
  protocol::ensure(setup.toThrift(), dst);

  MaskBuilder<SmartPointerStruct> builder(noneMask());
  builder.includes<ident::unique, ident::field_2>();
  builder.includes<ident::boxed, ident::field_1>();
  protocol::copy(builder.toThrift(), src, dst);
  ASSERT_TRUE(bool(dst.unique_ref()));
  EXPECT_FALSE(dst.unique_ref()->field_1().has_value());
  EXPECT_FALSE(dst.unique_ref()->field_2().has_value());
  EXPECT_FALSE(bool(dst.shared_ref()));
  ASSERT_TRUE(bool(dst.boxed_ref()));
  EXPECT_FALSE(dst.boxed_ref()->field_1().has_value());
  EXPECT_EQ(dst.boxed_ref()->field_2(), 0);
}

TEST(FieldMaskTest, SchemafulCopyException) {
  Bar2 src, dst;
  Mask m1; // m1 = includes{2: includes{4: includes{}}}
  auto& includes = m1.includes_ref().emplace();
  includes[2].includes_ref().emplace()[4] = noneMask();
  EXPECT_THROW(protocol::copy(m1, src, dst), std::runtime_error);

  Mask m2; // m2 = includes{1: includes{2: includes{5: excludes{}}}}
  auto& includes2 = m2.includes_ref().emplace();
  includes2[1].includes_ref().emplace()[2].excludes_ref().emplace()[5] =
      allMask();
  includes2[2] = allMask();
  EXPECT_THROW(protocol::copy(m2, src, dst), std::runtime_error);
}

void testLogicalOperations(Mask A, Mask B) {
  Mask maskUnion = A | B;
  Mask maskIntersect = A & B;
  Mask maskSubtractAB = A - B;
  Mask maskSubtractBA = B - A;

  EXPECT_EQ(maskUnion, maskIntersect | maskSubtractAB | maskSubtractBA);

  EXPECT_EQ(maskIntersect, maskUnion - maskSubtractAB - maskSubtractBA);
  EXPECT_EQ(maskIntersect, B - maskSubtractBA);
  EXPECT_EQ(maskIntersect, A - maskSubtractAB);
  EXPECT_EQ(maskIntersect, maskUnion & maskIntersect);

  // TODO(dokwon): Add testing operator to check if two mask is semantically
  // identical.
  // EXPECT_EQ(A, maskUnion & A);
  // EXPECT_EQ(A, A | maskIntersect);
  // EXPECT_EQ(A, A | maskSubtractAB);

  // EXPECT_EQ(B, maskUnion & B);
  // EXPECT_EQ(B, B | maskIntersect);
  // EXPECT_EQ(B, B | maskSubtractBA);

  EXPECT_EQ(maskSubtractAB, A & maskSubtractAB);
  EXPECT_EQ(maskSubtractAB, maskUnion - B);

  EXPECT_EQ(maskSubtractBA, B & maskSubtractBA);
  EXPECT_EQ(maskSubtractBA, maskUnion - A);
}

TEST(FIeldMaskTest, LogicalOpSimple) {
  // maskA = includes{1: excludes{},
  //                  2: excludes{},
  //                  3: includes{}}
  Mask maskA;
  {
    auto& includes = maskA.includes_ref().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = noneMask();
  }

  // maskB = includes{2: excludes{},
  //                  3: excludes{}}
  Mask maskB;
  {
    auto& includes = maskB.includes_ref().emplace();
    includes[2] = allMask();
    includes[3] = allMask();
  }

  // maskA | maskB == includes{1: excludes{},
  //                           2: excludes{},
  //                           3: excludes{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes_ref().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes{2: excludes{}}
  Mask maskIntersect;
  { maskIntersect.includes_ref().emplace()[2] = allMask(); }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{1: excludes{}}
  Mask maskSubtractAB;
  { maskSubtractAB.includes_ref().emplace()[1] = allMask(); }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes{3: excludes{}}
  Mask maskSubtractBA;
  { maskSubtractBA.includes_ref().emplace()[3] = allMask(); }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);
}

// Similar to previous unit-test, but for map
TEST(FieldMaskTest, LogicalOpSimpleMap) {
  // maskA = includes_map{1: excludes{},
  //                      2: excludes{},
  //                      3: includes{}}
  Mask maskA;
  {
    auto& includes = maskA.includes_map_ref().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = noneMask();
  }

  // maskB = includes_map{2: excludes{},
  //                      3: excludes{}}
  Mask maskB;
  {
    auto& includes = maskB.includes_map_ref().emplace();
    includes[2] = allMask();
    includes[3] = allMask();
  }

  // maskA | maskB == includes_map{1: excludes{},
  //                               2: excludes{},
  //                               3: excludes{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes_map_ref().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes_map{2: excludes{}}
  Mask maskIntersect;
  { maskIntersect.includes_map_ref().emplace()[2] = allMask(); }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes_map{1: excludes{}}
  Mask maskSubtractAB;
  { maskSubtractAB.includes_map_ref().emplace()[1] = allMask(); }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes_map{3: excludes{}}
  Mask maskSubtractBA;
  { maskSubtractBA.includes_map_ref().emplace()[3] = allMask(); }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpSimpleStringMap) {
  // maskA = includes_string_map{1: excludes_string_map{},
  //                             2: excludes_string_map{},
  //                             3: includes_string_map{}}
  Mask maskA;
  {
    auto& includes = maskA.includes_string_map_ref().emplace();
    includes["1"] = allMask();
    includes["2"] = allMask();
    includes["3"] = noneMask();
  }

  // maskB = includes_string_map{2: excludes_string_map{},
  //                             3: excludes_string_map{}}
  Mask maskB;
  {
    auto& includes = maskB.includes_string_map_ref().emplace();
    includes["2"] = allMask();
    includes["3"] = allMask();
  }

  // maskA | maskB == includes_string_map{1: excludes_string_map{},
  //                                      2: excludes_string_map{},
  //                                      3: excludes_string_map{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes_string_map_ref().emplace();
    includes["1"] = allMask();
    includes["2"] = allMask();
    includes["3"] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes_string_map{2: excludes_string_map{}}
  Mask maskIntersect;
  { maskIntersect.includes_string_map_ref().emplace()["2"] = allMask(); }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes_string_map{1: excludes_string_map{}}
  Mask maskSubtractAB;
  { maskSubtractAB.includes_string_map_ref().emplace()["1"] = allMask(); }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes_string_map{3: excludes_string_map{}}
  Mask maskSubtractBA;
  { maskSubtractBA.includes_string_map_ref().emplace()["3"] = allMask(); }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpBothIncludes) {
  // maskA = includes{1: includes{2: excludes{}},
  //                  3: includes{4: excludes{},
  //                              5: excludes{}}}
  Mask maskA;
  {
    auto& includes = maskA.includes_ref().emplace();
    includes[1].includes_ref().emplace()[2] = allMask();
    auto& includes2 = includes[3].includes_ref().emplace();
    includes2[4] = allMask();
    includes2[5] = allMask();
  }

  // maskB = includes{1: excludes{},
  //                  3: includes{5: excludes{},
  //                              6: excludes{}},
  //                  7: excludes{}}
  Mask maskB;
  {
    auto& includes = maskB.includes_ref().emplace();
    includes[1] = allMask();
    auto& includes2 = includes[3].includes_ref().emplace();
    includes2[5] = allMask();
    includes2[6] = allMask();
    includes[7] = allMask();
  }

  // maskA | maskB == includes{1: excludes{},
  //                           3: includes{4: excludes{},
  //                                       5: excludes{},
  //                                       6: excludes{}},
  //                           7: excludes{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes_ref().emplace();
    includes[1] = allMask();
    auto& includes2 = includes[3].includes_ref().emplace();
    includes2[4] = allMask();
    includes2[5] = allMask();
    includes2[6] = allMask();
    includes[7] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes{1: includes{2: excludes{}},
  //                           3: includes{5: excludes{}}}
  Mask maskIntersect;
  {
    auto& includes = maskIntersect.includes_ref().emplace();
    includes[1].includes_ref().emplace()[2] = allMask();
    includes[3].includes_ref().emplace()[5] = allMask();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{3: includes{4: excludes{}}}
  Mask maskSubtractAB;
  {
    maskSubtractAB.includes_ref().emplace()[3].includes_ref().emplace()[4] =
        allMask();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes{1: excludes{2: excludes{}},
  //                           3: includes{6: excludes{}},
  //                           7: excludes{}}
  Mask maskSubtractBA;
  {
    auto& includes = maskSubtractBA.includes_ref().emplace();
    includes[1].excludes_ref().emplace()[2] = allMask();
    auto& includes2 = includes[3].includes_ref().emplace();
    includes2[6] = allMask();
    includes[7] = allMask();
  }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpBothExcludes) {
  // maskA = excludes{1: excludes{2: excludes{}},
  //                  3: includes{4: includes{},
  //                              5: excludes{}}}
  Mask maskA;
  {
    auto& excludes = maskA.excludes_ref().emplace();
    excludes[1].excludes_ref().emplace()[2] = allMask();
    auto& includes = excludes[3].includes_ref().emplace();
    includes[4] = noneMask();
    includes[5] = allMask();
  }

  // maskB = excludes{1: includes{},
  //                  3: includes{5: excludes{},
  //                              6: excludes{}},
  //                  7: excludes{}}
  Mask maskB;
  {
    auto& excludes = maskB.excludes_ref().emplace();
    excludes[1] = noneMask();
    auto& includes = excludes[3].includes_ref().emplace();
    includes[5] = allMask();
    includes[6] = allMask();
    excludes[7] = allMask();
  }

  // maskA | maskB == excludes{3: includes{5: excludes{}}}
  Mask maskUnion;
  {
    maskUnion.excludes_ref().emplace()[3].includes_ref().emplace()[5] =
        allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == excludes{1: excludes{2: excludes{}},
  //                           3: includes{5: excludes{},
  //                                       6: excludes{}},
  //                           7: excludes{}}
  Mask maskIntersect;
  {
    auto& excludes = maskIntersect.excludes_ref().emplace();
    excludes[1].excludes_ref().emplace()[2] = allMask();
    auto& includes = excludes[3].includes_ref().emplace();
    includes[5] = allMask();
    includes[6] = allMask();
    excludes[7] = allMask();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{3: includes{6: excludes{}},
  //                           7: excludes{}}
  Mask maskSubtractAB;
  {
    auto& includes = maskSubtractAB.includes_ref().emplace();
    includes[3].includes_ref().emplace()[6] = allMask();
    includes[7] = allMask();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes{1: excludes{2: excludes{}}}
  Mask maskSubtractBA;
  {
    maskSubtractBA.includes_ref().emplace()[1].excludes_ref().emplace()[2] =
        allMask();
  }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpIncludesExcludes) {
  // maskA = includes{1: includes{2: excludes{}},
  //                  3: includes{4: excludes{},
  //                              5: excludes{}}}
  Mask maskA;
  {
    auto& includes = maskA.includes_ref().emplace();
    includes[1].includes_ref().emplace()[2] = allMask();
    auto& includes2 = includes[3].includes_ref().emplace();
    includes2[4] = allMask();
    includes2[5] = allMask();
  }

  // maskB = excludes{1: includes{},
  //                  3: includes{5: excludes{},
  //                              6: excludes{}},
  //                  7: excludes{}}
  Mask maskB;
  {
    auto& excludes = maskB.excludes_ref().emplace();
    excludes[1] = noneMask();
    auto& includes = excludes[3].includes_ref().emplace();
    includes[5] = allMask();
    includes[6] = allMask();
    excludes[7] = allMask();
  }

  // maskA | maskB == excludes{3: includes{6: excludes{}},
  //                           7: excludes{}}
  Mask maskUnion;
  {
    auto& excludes = maskUnion.excludes_ref().emplace();
    excludes[3].includes_ref().emplace()[6] = allMask();
    excludes[7] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes{1: includes{2: excludes{}},
  //                           3: includes{4: excludes{}}}
  Mask maskIntersect;
  {
    auto& includes = maskIntersect.includes_ref().emplace();
    includes[1].includes_ref().emplace()[2] = allMask();
    includes[3].includes_ref().emplace()[4] = allMask();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{3: includes{5: excludes{}}}
  Mask maskSubtractAB;
  {
    maskSubtractAB.includes_ref().emplace()[3].includes_ref().emplace()[5] =
        allMask();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == excludes{1: includes{2: excludes{}},
  //                           3: includes{4: excludes{},
  //                                       5: excludes{},
  //                                       6: excludes{}},
  //                           7: excludes{}}
  Mask maskSubtractBA;
  {
    auto& excludes = maskSubtractBA.excludes_ref().emplace();
    excludes[1].includes_ref().emplace()[2] = allMask();
    auto& includes = excludes[3].includes_ref().emplace();
    includes[4] = allMask();
    includes[5] = allMask();
    includes[6] = allMask();
    excludes[7] = allMask();
  }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpMixed) {
  // maskA = includes{1: includes_map{1: includes_string_map{"1": includes{},
  //                                                         "2": excludes{}},
  //                                  2: excludes_string_map{"1": includes{}}
  //                                                         "2": excludes{}}}}
  Mask maskA;
  {
    auto& includes = maskA.includes_ref().emplace();
    auto add = [](auto& m) {
      auto& includes_string_map = m[1].includes_string_map_ref().emplace();
      includes_string_map["1"].includes_ref().emplace();
      includes_string_map["2"].excludes_ref().emplace();
    };
    add(includes[1].includes_map_ref().emplace());
  }

  // maskB = excludes{1: excludes_map{1: excludes_string_map{"1": excludes{},
  //                                                         "2": includes{}},
  //                                  2: includes_string_map{"1": excludes{}}
  //                                                         "2": includes{}}}}
  Mask maskB;
  {
    auto& excludes = maskB.excludes_ref().emplace();
    auto add = [](auto& m) {
      auto& excludes_string_map = m[1].excludes_string_map_ref().emplace();
      excludes_string_map["1"].excludes_ref().emplace();
      excludes_string_map["2"].includes_ref().emplace();
    };
    add(excludes[1].excludes_map_ref().emplace());
  }

  // maskA | maskB ==
  //     excludes{1: excludes_map{1: excludes_string_map{"1": excludes{}}}}
  Mask maskUnion;
  {
    auto& excludes = maskUnion.excludes_ref().emplace();
    excludes[1]
        .excludes_map_ref()
        .emplace()[1]
        .excludes_string_map_ref()
        .emplace()["1"]
        .excludes_ref()
        .emplace();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB ==
  //     includes{1: includes_map{1: includes_string_map{"2": excludes{}}}}
  Mask maskIntersect;
  {
    auto& includes = maskIntersect.includes_ref().emplace();
    includes[1]
        .includes_map_ref()
        .emplace()[1]
        .includes_string_map_ref()
        .emplace()["2"]
        .excludes_ref()
        .emplace();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{1: includes_map{1: includes_string_map{}}}
  Mask maskSubtractAB;
  {
    auto& includes = maskSubtractAB.includes_ref().emplace();
    includes[1]
        .includes_map_ref()
        .emplace()[1]
        .includes_string_map_ref()
        .emplace();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA ==
  //     excludes{1: excludes_map{1: excludes_string_map{"1": excludes{},
  //                                                     "2": excludes{}}}
  Mask maskSubtractBA;
  {
    auto& excludes = maskSubtractBA.excludes_ref().emplace();
    auto& string_map = excludes[1]
                           .excludes_map_ref()
                           .emplace()[1]
                           .excludes_string_map_ref()
                           .emplace();
    string_map["1"].excludes_ref().emplace();
    string_map["2"].excludes_ref().emplace();
  }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, MaskBuilderIncludeExcludeEmpty) {
  {
    MaskBuilder<Foo> builder(noneMask());
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.excludes();
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.includes();
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.excludes(noneMask());
    EXPECT_EQ(builder.toThrift(), allMask());
  }
  {
    MaskBuilder<Foo> builder(allMask());
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.includes();
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.excludes();
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.includes(noneMask());
    EXPECT_EQ(builder.toThrift(), noneMask());
  }
  // test when field name list is empty
  {
    MaskBuilder<Foo> builder(noneMask());
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.excludes({});
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.includes({});
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.excludes({}, noneMask());
    EXPECT_EQ(builder.toThrift(), allMask());
  }
  {
    MaskBuilder<Foo> builder(allMask());
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.includes({});
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.excludes({});
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.includes({}, noneMask());
    EXPECT_EQ(builder.toThrift(), noneMask());
  }
}

void testMaskBuilderSimple(bool testFieldName) {
  // Test includes
  {
    // includes{1: excludes{},
    //          3: excludes{}}
    MaskBuilder<Foo> builder(noneMask());
    if (testFieldName) {
      builder.includes({"field1"});
      builder.includes({"field2"});
    } else {
      builder.includes<ident::field1>();
      builder.includes<ident::field2>();
    }
    Mask expected;
    auto& includes = expected.includes_ref().emplace();
    includes[1] = allMask();
    includes[3] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    MaskBuilder<Foo> builder(allMask());
    if (testFieldName) {
      builder.includes({"field1"});
      builder.includes({"field2"});
    } else {
      builder.includes<ident::field1>();
      builder.includes<ident::field2>();
    }
    EXPECT_EQ(builder.toThrift(), allMask());
  }
  {
    // includes{1: excludes{}}
    MaskBuilder<Foo> builder(noneMask());
    // including twice is fine
    if (testFieldName) {
      builder.includes({"field1"});
      builder.includes({"field1"});
    } else {
      builder.includes<ident::field1>();
      builder.includes<ident::field1>();
    }
    Mask expected;
    expected.includes_ref().emplace()[1] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }

  // Test excludes
  {
    // excludes{3: excludes{}}
    MaskBuilder<Foo> builder(allMask());
    if (testFieldName) {
      builder.excludes({"field1"}, noneMask());
      builder.excludes({"field2"});
    } else {
      builder.excludes<ident::field1>(noneMask());
      builder.excludes<ident::field2>();
    }
    Mask expected;
    expected.excludes_ref().emplace()[3] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    MaskBuilder<Foo> builder(noneMask());
    if (testFieldName) {
      builder.excludes({"field1"});
      builder.excludes({"field2"});
    } else {
      builder.excludes<ident::field1>();
      builder.excludes<ident::field2>();
    }
    EXPECT_EQ(builder.toThrift(), noneMask());
  }
  {
    // excludes{3: excludes{}}
    MaskBuilder<Foo> builder(allMask());
    // excluding twice is fine
    if (testFieldName) {
      builder.excludes({"field2"});
      builder.excludes({"field2"});
    } else {
      builder.excludes<ident::field2>();
      builder.excludes<ident::field2>();
    }
    Mask expected;
    expected.excludes_ref().emplace()[3] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }

  // Test includes and excludes
  {
    MaskBuilder<Foo> builder(noneMask());
    // excluding the field we included
    if (testFieldName) {
      builder.includes({"field2"});
      builder.excludes({"field2"});
    } else {
      builder.includes<ident::field2>();
      builder.excludes<ident::field2>();
    }
    EXPECT_EQ(builder.toThrift(), noneMask());
  }
  {
    MaskBuilder<Foo> builder(allMask());
    // including the field we excluded
    if (testFieldName) {
      builder.excludes({"field2"});
      builder.includes({"field2"});
    } else {
      builder.excludes<ident::field2>();
      builder.includes<ident::field2>();
    }
    EXPECT_EQ(builder.toThrift(), allMask());
  }
}

void testMaskBuilderNested(bool testFieldName) {
  // Test includes
  {
    // includes{1: includes{1: excludes{}}}
    MaskBuilder<Bar2> builder(noneMask());
    if (testFieldName) {
      builder.includes({"field_3", "field_1"});
    } else {
      builder.includes<ident::field_3, ident::field_1>();
    }
    Mask expected;
    expected.includes_ref().emplace()[1].includes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // includes{1: includes{1: excludes{}}}
    MaskBuilder<Bar2> builder(noneMask());
    // including twice is fine
    if (testFieldName) {
      builder.includes({"field_3", "field_1"});
      builder.includes({"field_3", "field_1"});
    } else {
      builder.includes<ident::field_3, ident::field_1>();
      builder.includes<ident::field_3, ident::field_1>();
    }
    Mask expected;
    expected.includes_ref().emplace()[1].includes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // includes{1: includes{1: excludes{}}}
    MaskBuilder<Bar2> builder(noneMask());
    Mask nestedMask;
    nestedMask.includes_ref().emplace()[1] = allMask();
    if (testFieldName) {
      builder.includes({"field_3"}, nestedMask);
    } else {
      builder.includes<ident::field_3>(nestedMask);
    }
    Mask expected;
    expected.includes_ref().emplace()[1].includes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // includes{1: includes{1: excludes{},
    //                      2: excludes{}},
    //          2: excludes{}}
    MaskBuilder<Bar2> builder(noneMask());
    if (testFieldName) {
      builder.includes({"field_4"});
      builder.includes({"field_3", "field_1"});
      builder.includes({"field_3", "field_2"});
    } else {
      builder.includes<ident::field_4>();
      builder.includes<ident::field_3, ident::field_1>();
      builder.includes<ident::field_3, ident::field_2>();
    }
    Mask expected;
    auto& includes = expected.includes_ref().emplace();
    includes[2] = allMask();
    auto& includes2 = includes[1].includes_ref().emplace();
    includes2[1] = allMask();
    includes2[2] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // includes{1: excludes{}}
    MaskBuilder<Bar2> builder(noneMask());
    if (testFieldName) {
      builder.includes({"field_3", "field_1"});
      builder.includes({"field_3"});
    } else {
      builder.includes<ident::field_3, ident::field_1>();
      builder.includes<ident::field_3>();
    }
    Mask expected;
    expected.includes_ref().emplace()[1] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }

  // Test excludes
  {
    // excludes{1: includes{1: excludes{}}}
    MaskBuilder<Bar2> builder(allMask());
    if (testFieldName) {
      builder.excludes({"field_3", "field_1"});
    } else {
      builder.excludes<ident::field_3, ident::field_1>();
    }
    Mask expected;
    expected.excludes_ref().emplace()[1].includes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // excludes{1: includes{1: excludes{}}}
    MaskBuilder<Bar2> builder(allMask());
    // excluding twice is fine
    if (testFieldName) {
      builder.excludes({"field_3", "field_1"});
      builder.excludes({"field_3", "field_1"});
    } else {
      builder.excludes<ident::field_3, ident::field_1>();
      builder.excludes<ident::field_3, ident::field_1>();
    }
    Mask expected;
    expected.excludes_ref().emplace()[1].includes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // excludes{1: includes{1: excludes{}}}
    MaskBuilder<Bar2> builder(allMask());
    Mask nestedMask;
    nestedMask.includes_ref().emplace()[1] = allMask();
    if (testFieldName) {
      builder.excludes({"field_3"}, nestedMask);
    } else {
      builder.excludes<ident::field_3>(nestedMask);
    }
    Mask expected;
    expected.excludes_ref().emplace()[1].includes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // excludes{1: includes{1: excludes{},
    //                      2: excludes{}},
    //          2: excludes{}}
    MaskBuilder<Bar2> builder(allMask());
    if (testFieldName) {
      builder.excludes({"field_4"});
      builder.excludes({"field_3", "field_1"});
      builder.excludes({"field_3", "field_2"});
    } else {
      builder.excludes<ident::field_4>();
      builder.excludes<ident::field_3, ident::field_1>();
      builder.excludes<ident::field_3, ident::field_2>();
    }
    Mask expected;
    auto& excludes = expected.excludes_ref().emplace();
    excludes[2] = allMask();
    auto& includes = excludes[1].includes_ref().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // excludes{1: excludes{}}
    MaskBuilder<Bar2> builder(allMask());
    if (testFieldName) {
      builder.excludes({"field_3", "field_1"});
      builder.excludes({"field_3"});
    } else {
      builder.excludes<ident::field_3, ident::field_1>();
      builder.excludes<ident::field_3>();
    }
    Mask expected;
    expected.excludes_ref().emplace()[1] = allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }

  // Test includes and excludes
  {
    // includes{1: excludes{1: excludes{}}}
    MaskBuilder<Bar2> builder(noneMask());
    if (testFieldName) {
      builder.includes({"field_3"});
      builder.excludes({"field_3", "field_1"});
    } else {
      builder.includes<ident::field_3>();
      builder.excludes<ident::field_3, ident::field_1>();
    }
    Mask expected;
    expected.includes_ref().emplace()[1].excludes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
  {
    // excludes{1: excludes{1: excludes{}}}
    MaskBuilder<Bar2> builder(allMask());
    if (testFieldName) {
      builder.excludes({"field_3"});
      builder.includes({"field_3", "field_1"});
    } else {
      builder.excludes<ident::field_3>();
      builder.includes<ident::field_3, ident::field_1>();
    }
    Mask expected;
    expected.excludes_ref().emplace()[1].excludes_ref().emplace()[1] =
        allMask();
    EXPECT_EQ(builder.toThrift(), expected);
  }
}

TEST(FieldMaskTest, MaskBuilderSimple) {
  testMaskBuilderSimple(false);
}

TEST(FieldMaskTest, MaskBuilderNested) {
  testMaskBuilderNested(false);
}

TEST(FieldMaskTest, MaskBuilderSimpleWithFieldName) {
  testMaskBuilderSimple(true);
}

TEST(FieldMaskTest, MaskBuilderNestedWithFieldName) {
  testMaskBuilderNested(true);
}

TEST(FieldMaskTest, MaskBuilderWithFieldNameError) {
  MaskBuilder<Bar2> builder(noneMask());
  // field not found
  EXPECT_THROW(builder.includes({"random"}), std::runtime_error);
  EXPECT_THROW(builder.excludes({"field_3, random"}), std::runtime_error);
  // field_4 is not a thrift struct.
  EXPECT_THROW(builder.includes({"field_4, random"}), std::runtime_error);
}

TEST(FieldMaskTest, MaskBuilderOtherTypes) {
  MaskBuilder<Bar2> builder(noneMask());
  builder.includes<ident::field_3>();
  builder.excludes<ident::field_3, ident::field_2>();
  {
    MaskBuilder<Bar2> builder2(noneMask());
    builder2.includes<type::ordinal<1>>();
    builder2.excludes<type::ordinal<1>, type::ordinal<2>>();
    EXPECT_EQ(builder.toThrift(), builder2.toThrift());
  }
  {
    MaskBuilder<Bar2> builder2(noneMask());
    builder2.includes<type::field_id<1>>();
    builder2.excludes<type::field_id<1>, type::field_id<2>>();
    EXPECT_EQ(builder.toThrift(), builder2.toThrift());
  }
  {
    MaskBuilder<Bar2> builder2(noneMask());
    builder2.includes<op::get_field_tag<Bar2, ident::field_3>>();
    builder2.excludes<
        op::get_field_tag<Bar2, ident::field_3>,
        op::get_field_tag<Foo2, ident::field_2>>();
    EXPECT_EQ(builder.toThrift(), builder2.toThrift());
  }
  {
    MaskBuilder<Bar2> builder2(noneMask());
    builder2.includes<op::get_type_tag<Bar2, ident::field_3>>();
    builder2.excludes<
        op::get_type_tag<Bar2, ident::field_3>,
        op::get_type_tag<Foo2, ident::field_2>>();
    EXPECT_EQ(builder.toThrift(), builder2.toThrift());
  }
  // test mixing different id type
  {
    MaskBuilder<Bar2> builder2(noneMask());
    builder2.includes<op::get_type_tag<Bar2, type::ordinal<1>>>();
    builder2
        .excludes<type::field_id<1>, op::get_type_tag<Foo2, ident::field_2>>();
    EXPECT_EQ(builder.toThrift(), builder2.toThrift());
  }
  {
    MaskBuilder<Bar2> builder2(noneMask());
    builder2.includes<op::get_type_tag<Bar2, ident::field_3>>();
    builder2
        .excludes<type::ordinal<1>, op::get_field_tag<Foo2, ident::field_2>>();
    EXPECT_EQ(builder.toThrift(), builder2.toThrift());
  }
}

TEST(FieldMaskTest, MaskBuilderMap) {
  MaskBuilder<HasMap> builder(noneMask());
  Mask expected;

  builder.includes_map_element<ident::foos>(1);
  expected.includes_ref().ensure()[1].includes_map_ref().ensure()[1] =
      allMask();
  EXPECT_EQ(builder.toThrift(), expected);

  builder.includes<ident::foo>();
  expected.includes_ref().ensure()[2] = allMask();
  EXPECT_EQ(builder.toThrift(), expected);

  builder.includes_map_element<ident::foos>(2);
  expected.includes_ref().ensure()[1].includes_map_ref().ensure()[2] =
      allMask();
  EXPECT_EQ(builder.toThrift(), expected);

  builder.excludes_map_element<ident::foos>(2);
  expected.includes_ref().ensure()[1].includes_map_ref().ensure().erase(2);
  EXPECT_EQ(builder.toThrift(), expected);

  builder.excludes_map_element<ident::foos>(1);
  expected.includes_ref().ensure()[1].includes_map_ref().ensure().erase(1);
  EXPECT_EQ(builder.toThrift(), expected);
}

TEST(FieldMaskTest, MaskBuilderStringMap) {
  MaskBuilder<HasMap> builder(noneMask());
  Mask expected;

  builder.includes_map_element<ident::string_map>("1");
  expected.includes_ref().ensure()[3].includes_string_map_ref().ensure()["1"] =
      allMask();
  EXPECT_EQ(builder.toThrift(), expected);

  builder.includes_map_element<ident::string_map>("2");
  expected.includes_ref().ensure()[3].includes_string_map_ref().ensure()["2"] =
      allMask();
  EXPECT_EQ(builder.toThrift(), expected);

  builder.excludes_map_element<ident::string_map>("2");
  expected.includes_ref().ensure()[3].includes_string_map_ref().ensure().erase(
      "2");
  EXPECT_EQ(builder.toThrift(), expected);

  builder.excludes_map_element<ident::string_map>("1");
  expected.includes_ref().ensure()[3].includes_string_map_ref().ensure().erase(
      "1");
  EXPECT_EQ(builder.toThrift(), expected);
}

TEST(FieldMaskTest, ReverseMask) {
  EXPECT_EQ(reverseMask(allMask()), noneMask());
  EXPECT_EQ(reverseMask(noneMask()), allMask());
  // inclusiveMask and exclusiveMask are reverse of each other.
  Mask inclusiveMask;
  auto& includes = inclusiveMask.includes_ref().emplace();
  includes[1] = allMask();
  includes[2].includes_ref().emplace()[3] = allMask();
  Mask exclusiveMask;
  auto& excludes = exclusiveMask.excludes_ref().emplace();
  excludes[1] = allMask();
  excludes[2].includes_ref().emplace()[3] = allMask();

  EXPECT_EQ(reverseMask(inclusiveMask), exclusiveMask);
  EXPECT_EQ(reverseMask(exclusiveMask), inclusiveMask);

  Mask empty;
  EXPECT_THROW(reverseMask(empty), std::runtime_error);
}

TEST(FieldMaskTest, ReverseMapMask) {
  Mask inclusiveMask;
  auto& includes = inclusiveMask.includes_map_ref().emplace();
  includes[1] = allMask();
  includes[2].includes_ref().emplace()[3] = allMask();
  Mask exclusiveMask;
  auto& excludes = exclusiveMask.excludes_map_ref().emplace();
  excludes[1] = allMask();
  excludes[2].includes_ref().emplace()[3] = allMask();

  EXPECT_EQ(reverseMask(inclusiveMask), exclusiveMask);
  EXPECT_EQ(reverseMask(exclusiveMask), inclusiveMask);
}

TEST(FieldMaskTest, ReverseStringMapMask) {
  Mask inclusiveMask;
  auto& includes = inclusiveMask.includes_string_map_ref().emplace();
  includes["1"] = allMask();
  includes["2"].includes_ref().emplace()[3] = allMask();
  Mask exclusiveMask;
  auto& excludes = exclusiveMask.excludes_string_map_ref().emplace();
  excludes["1"] = allMask();
  excludes["2"].includes_ref().emplace()[3] = allMask();

  EXPECT_EQ(reverseMask(inclusiveMask), exclusiveMask);
  EXPECT_EQ(reverseMask(exclusiveMask), inclusiveMask);
}

TEST(FieldMaskTest, Compare) {
  Bar2 original;
  Foo2 foo;
  foo.field_1() = 1;
  foo.field_2() = 2;
  original.field_3() = foo;
  original.field_4() = 4;

  {
    Bar2 modified = original;
    modified.field_3()->field_1() = 5;
    modified.field_4() = 6;
    // obj[1][1] and obj[2] are different
    Mask mask = protocol::compare(original, modified);
    Mask expected;
    expected.includes_ref().emplace()[2] = allMask();
    expected.includes_ref().value()[1].includes_ref().emplace()[1] = allMask();
    EXPECT_EQ(mask, expected);
    EXPECT_EQ(protocol::compare(modified, original), mask); // commutative
  }
  {
    Bar2 modified = original;
    Mask mask = protocol::compare(original, modified);
    // This doesn't create a nested mask.
    EXPECT_EQ(mask, noneMask());
    EXPECT_EQ(protocol::compare(modified, original), mask); // commutative
  }

  // tests if this works with unset optional fields
  {
    Bar2 empty;
    Mask mask = protocol::compare(original, empty);
    EXPECT_EQ(mask.includes_ref().value()[1], allMask());
    EXPECT_EQ(mask.includes_ref().value()[2], allMask());
    EXPECT_EQ(protocol::compare(empty, original), mask); // commutative
  }
  {
    Bar2 empty, empty2;
    Mask mask = protocol::compare(empty, empty2);
    EXPECT_EQ(mask, noneMask());
  }

  // test if this works with smart pointer fields
  {
    SmartPointerStruct src, dst;
    protocol::ensure(allMask(), src);
    protocol::ensure(allMask(), dst);
    // This compares by value, so the resulting mask is noneMask.
    Mask mask = protocol::compare(src, dst);
    EXPECT_EQ(mask, noneMask());
  }
  {
    SmartPointerStruct src, empty;
    protocol::ensure(allMask(), src);
    Mask mask = protocol::compare(src, empty);
    EXPECT_EQ(mask.includes_ref().value()[1], allMask());
    EXPECT_EQ(mask.includes_ref().value()[2], allMask());
    EXPECT_EQ(mask.includes_ref().value()[3], allMask());
    EXPECT_EQ(protocol::compare(empty, src), mask); // commutative
  }
  {
    SmartPointerStruct src, dst;
    protocol::ensure(allMask(), src);
    protocol::ensure(allMask(), dst);
    // Compare should create a mask for nested fields.
    src.unique_ref()->field_1() = 1;
    dst.unique_ref()->field_1() = 10;
    dst.shared_ref() = nullptr;
    // mask = {1: includes{1: allMask()},
    //         2: allMask()}
    Mask mask = protocol::compare(src, dst);
    Mask expected;
    auto& includes = expected.includes_ref().emplace();
    includes[1].includes_ref().emplace()[1] = allMask();
    includes[2] = allMask();
    EXPECT_EQ(mask, expected);
    EXPECT_EQ(protocol::compare(dst, src), mask); // commutative
  }
}

TEST(FieldMaskTest, MaskAdapter) {
  MaskStruct foo;
  static_assert(std::is_same_v<
                std::remove_reference_t<decltype(foo.mask().value())>,
                MaskBuilder<Bar>>);
  EXPECT_EQ(foo.mask()->toThrift(), Mask{});
  static_assert(std::is_same_v<
                std::remove_reference_t<decltype(foo.mask2().value())>,
                MaskBuilder<Bar>>);
  EXPECT_EQ(foo.mask2()->toThrift(), Mask{});
}

TEST(FieldMaskTest, MaskBuilderReset) {
  // reset to none
  {
    MaskBuilder<Bar2> builder;
    builder.reset_to_none();
    EXPECT_EQ(builder.toThrift(), noneMask());
    builder.includes<ident::field_3>().reset_to_none();
    EXPECT_EQ(builder.toThrift(), noneMask());
  }

  // reset to all
  {
    MaskBuilder<Bar2> builder;
    builder.reset_to_all();
    EXPECT_EQ(builder.toThrift(), allMask());
    builder.includes<ident::field_3>().reset_to_all();
    EXPECT_EQ(builder.toThrift(), allMask());
  }

  // reset and includes
  {
    MaskBuilder<Bar2> expected(noneMask());
    expected.includes<ident::field_4>();
    {
      MaskBuilder<Bar2> builder;
      builder.reset_and_includes<ident::field_4>();
      EXPECT_EQ(builder.toThrift(), expected.toThrift());
    }
    {
      MaskBuilder<Bar2> builder;
      builder.reset_to_all()
          .includes({"field_3"})
          .reset_and_includes({"field_4"});
      EXPECT_EQ(builder.toThrift(), expected.toThrift());
    }
  }

  // reset and excludes
  {
    MaskBuilder<Bar2> expected(allMask());
    expected.excludes<ident::field_3, ident::field_1>();
    {
      MaskBuilder<Bar2> builder;
      builder.reset_and_excludes<ident::field_3, ident::field_1>();
      EXPECT_EQ(builder.toThrift(), expected.toThrift());
    }
    {
      MaskBuilder<Bar2> builder;
      builder.reset_to_none()
          .includes({"field_3"})
          .reset_and_excludes({"field_3", "field_1"});
      EXPECT_EQ(builder.toThrift(), expected.toThrift());
    }
  }
}

TEST(FieldMaskTest, MaskBuilderMaskAPIs) {
  Bar2 src, dst;

  MaskBuilder<Bar2> builder;
  builder.reset_and_includes({"field_3", "field_1"}).includes({"field_4"});
  // test invert
  Mask expectedMask = reverseMask(builder.toThrift());
  EXPECT_EQ(builder.invert().toThrift(), expectedMask);
  // Mask includes bar.field_3().field_2().
  // test ensure, clear, and copy
  Bar2 expected, cleared;
  expected.field_3().emplace().field_2() = 0;
  cleared.field_3().emplace();
  builder.ensure(src);
  EXPECT_EQ(src, expected);
  builder.copy(src, dst);
  EXPECT_EQ(dst, expected);
  builder.clear(src);
  EXPECT_EQ(src, cleared);
  builder.copy(src, dst);
  EXPECT_EQ(dst, cleared);
}

TEST(FieldMaskTest, MaskBuilderMaskNotCompatible) {
  // Mask is not compatible with both Bar and Foo.
  Mask mask;
  mask.includes_ref().emplace()[5] = allMask();
  EXPECT_THROW(MaskBuilder<Bar> temp(mask), std::runtime_error);
  MaskBuilder<Bar> builder(noneMask());
  EXPECT_THROW(builder.includes(mask), std::runtime_error);
  EXPECT_THROW(builder.includes({}, mask), std::runtime_error);
  EXPECT_THROW(builder.includes<ident::foo>(mask), std::runtime_error);
  EXPECT_THROW(builder.includes({"foo"}, mask), std::runtime_error);
  EXPECT_THROW(
      builder.includes_map_element<ident::foo>(1, mask), std::runtime_error);
  EXPECT_THROW(
      builder.includes_map_element<ident::foo>("1", mask), std::runtime_error);
}

} // namespace apache::thrift::test
