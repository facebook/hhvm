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

#include <string_view>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/UniversalName.h>
#include <thrift/test/gen-cpp2/FieldMask_types.h>

using apache::thrift::protocol::allMask;
using apache::thrift::protocol::asValueStruct;
using apache::thrift::protocol::DynamicMaskBuilder;
using apache::thrift::protocol::Mask;
using apache::thrift::protocol::MaskBuilder;
using apache::thrift::protocol::MaskRef;
using apache::thrift::protocol::noneMask;
using namespace apache::thrift::protocol::detail;
using namespace std::literals;

namespace apache::thrift::test {

bool literallyEqual(MaskRef actual, MaskRef expected) {
  return actual.mask == expected.mask &&
      actual.is_exclusion == expected.is_exclusion;
}

void assertSmartPointerStructIsEmpty(SmartPointerStruct& obj) {
  EXPECT_FALSE(bool(obj.unique()));
  EXPECT_FALSE(bool(obj.shared()));
  EXPECT_FALSE(bool(obj.boxed()));
}

void assertPointerHasAllValues(auto&& ptr) {
  ASSERT_TRUE(bool(ptr));
  EXPECT_EQ(ptr->field_1(), 0);
  EXPECT_EQ(ptr->field_2(), 0);
}

void assertSmartPointerStructHasAllValues(SmartPointerStruct& obj) {
  assertPointerHasAllValues(obj.unique());
  assertPointerHasAllValues(obj.shared());
  assertPointerHasAllValues(obj.boxed());
}

static Mask noTypeMask = []() {
  Mask m;
  m.includes_type().ensure();
  return m;
}();

static Mask allTypeMask = []() {
  Mask m;
  m.excludes_type().ensure();
  return m;
}();

// We don't want to provide an official API for this
template <typename T>
T getViaIdenticalType(const type::AnyStruct& any, const type::Type& type) {
  CHECK(type::identicalType(*any.type(), type));

  T ret;
  CompactProtocolReader reader;
  reader.setInput(&any.data().value());
  op::decode<type::struct_t<Foo>>(reader, ret);
  return ret;
}

// Works only for struct types
void convertToHashedURI(type::Type& type) {
  auto& typeUriUnion = *type.toThrift().name()->structType();

  auto hashed =
      type::getUniversalHashPrefix(
          type::getUniversalHash(
              type::UniversalHashAlgorithm::Sha2_256, typeUriUnion.get_uri()),
          type::kDefaultTypeHashBytes)
          .toString();
  typeUriUnion.typeHashPrefixSha2_256().ensure() = hashed;
}

TEST(FieldMaskTest, ExampleFieldMask) {
  // includes{7: excludes{},
  //          9: includes{5: excludes{},
  //                      6: excludes{}}}
  Mask m;
  auto& includes = m.includes().emplace();
  includes[7] = allMask();
  auto& nestedIncludes = includes[9].includes().emplace();
  nestedIncludes[5] = allMask();
  nestedIncludes[6] = allMask();
  includes[8] = noneMask(); // not required
}

TEST(FieldMaskTest, ExampleMapMask) {
  // includes_map{7: excludes_map{},
  //              3: excludes{}}
  Mask m;
  auto& includes_map = m.includes_map().emplace();
  includes_map[7].excludes_map().emplace();
  includes_map[3] = allMask();
}

TEST(FieldMaskTest, ExampleStringMapMask) {
  // includes_string_map{"7": excludes_string_map{},
  //                     "3": excludes{}}
  Mask m;
  auto& includes_string_map = m.includes_string_map().emplace();
  includes_string_map["7"].excludes_string_map().emplace();
  includes_string_map["3"] = allMask();
}

TEST(FieldMaskTest, ExampleTypeMask) {
  // includes_type{foo: allMask(), bar: noneMask()}
  Mask m;
  auto& includes_type = m.includes_type().emplace();
  includes_type[type::infer_tag<Foo>{}] = allMask();
  includes_type[type::infer_tag<Bar>{}] = noneMask();
}

TEST(FieldMaskTest, Constant) {
  EXPECT_EQ(allMask().excludes()->size(), 0);
  EXPECT_EQ(noneMask().includes()->size(), 0);
}

TEST(FieldMaskTest, IsAllMask) {
  EXPECT_TRUE((MaskRef{allMask(), false}).isAllMask());
  EXPECT_TRUE((MaskRef{noneMask(), true}).isAllMask());
  EXPECT_FALSE((MaskRef{noneMask(), false}).isAllMask());
  EXPECT_FALSE((MaskRef{allMask(), true}).isAllMask());
  {
    Mask m;
    m.excludes().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
  }
  {
    Mask m;
    m.includes_map().emplace()[3].includes_map().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
  }
  {
    Mask m;
    m.includes_string_map().emplace()["3"].includes_string_map().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, true}).isAllMask());
    EXPECT_FALSE((MaskRef{m, false}).isAllMask());
  }
}

TEST(FieldMaskTest, IsNoneMask) {
  EXPECT_TRUE((MaskRef{noneMask(), false}).isNoneMask());
  EXPECT_TRUE((MaskRef{allMask(), true}).isNoneMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isNoneMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isNoneMask());
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_FALSE((MaskRef{m, false}).isNoneMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneMask());
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, true}).isNoneMask());
    EXPECT_FALSE((MaskRef{m, false}).isNoneMask());
  }
}

TEST(FieldMaskTest, IsAllMapMask) {
  auto allMapMask = [] {
    Mask m;
    m.excludes_map().emplace();
    return m;
  };

  auto noneMapMask = [] {
    Mask m;
    m.includes_map().emplace();
    return m;
  };

  EXPECT_TRUE((MaskRef{allMapMask(), false}).isAllMapMask());
  EXPECT_TRUE((MaskRef{noneMapMask(), true}).isAllMapMask());
  EXPECT_FALSE((MaskRef{noneMapMask(), false}).isAllMapMask());
  EXPECT_FALSE((MaskRef{allMapMask(), true}).isAllMapMask());
  {
    Mask m;
    m.excludes().emplace()[5] = allMask();
    EXPECT_THROW((MaskRef{m, false}).isAllMapMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isAllMapMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map().emplace()[3].includes_map().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMapMask());
  }
  {
    Mask m;
    m.includes_string_map().emplace()["3"].includes_string_map().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isAllMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllMapMask());
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_THROW((MaskRef{m, true}).isAllMapMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).isAllMapMask(), std::runtime_error);
  }
}

TEST(FieldMaskTest, IsNoneMapMask) {
  auto allMapMask = [] {
    Mask m;
    m.excludes_map().emplace();
    return m;
  };

  auto noneMapMask = [] {
    Mask m;
    m.includes_map().emplace();
    return m;
  };

  EXPECT_TRUE((MaskRef{noneMapMask(), false}).isNoneMapMask());
  EXPECT_TRUE((MaskRef{allMapMask(), true}).isNoneMapMask());
  EXPECT_FALSE((MaskRef{allMapMask(), false}).isNoneMapMask());
  EXPECT_FALSE((MaskRef{noneMapMask(), true}).isNoneMapMask());
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_THROW((MaskRef{m, false}).isNoneMapMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isNoneMapMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map().emplace()[3].includes_map().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isNoneMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneMapMask());
  }
  {
    Mask m;
    m.includes_string_map().emplace()["3"].includes_string_map().emplace();
    EXPECT_FALSE((MaskRef{m, false}).isNoneMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneMapMask());
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_THROW((MaskRef{m, true}).isNoneMapMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).isNoneMapMask(), std::runtime_error);
  }
}

TEST(FieldMaskTest, IsAllTypeMask) {
  auto allTypeMask = [] {
    Mask m;
    m.excludes_type().emplace();
    return m;
  };

  auto noneTypeMask = [] {
    Mask m;
    m.includes_type().emplace();
    return m;
  };

  EXPECT_TRUE((MaskRef{allTypeMask(), false}).isAllTypeMask());
  EXPECT_TRUE((MaskRef{noneTypeMask(), true}).isAllTypeMask());
  EXPECT_FALSE((MaskRef{noneTypeMask(), false}).isAllTypeMask());
  EXPECT_FALSE((MaskRef{allTypeMask(), true}).isAllTypeMask());
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_THROW((MaskRef{m, false}).isAllTypeMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isAllTypeMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map().emplace()[3].includes_map().emplace();
    EXPECT_THROW((MaskRef{m, false}).isAllTypeMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isAllTypeMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_string_map().emplace()["3"].includes_string_map().emplace();
    EXPECT_THROW((MaskRef{m, false}).isAllTypeMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isAllTypeMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isAllTypeMask());
    EXPECT_FALSE((MaskRef{m, true}).isAllTypeMask());
  }
}

TEST(FieldMaskTest, IsNoneTypeMask) {
  auto allTypeMask = [] {
    Mask m;
    m.excludes_type().emplace();
    return m;
  };

  auto noneTypeMask = [] {
    Mask m;
    m.includes_type().emplace();
    return m;
  };

  EXPECT_TRUE((MaskRef{noneTypeMask(), false}).isNoneTypeMask());
  EXPECT_TRUE((MaskRef{allTypeMask(), true}).isNoneTypeMask());
  EXPECT_FALSE((MaskRef{allTypeMask(), false}).isNoneTypeMask());
  EXPECT_FALSE((MaskRef{noneTypeMask(), true}).isNoneTypeMask());
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_THROW((MaskRef{m, false}).isNoneTypeMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isNoneTypeMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map().emplace()[3].includes_map().emplace();
    EXPECT_THROW((MaskRef{m, false}).isNoneTypeMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isNoneTypeMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_string_map().emplace()["3"].includes_string_map().emplace();
    EXPECT_THROW((MaskRef{m, false}).isNoneTypeMask(), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).isNoneTypeMask(), std::runtime_error);
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isNoneTypeMask());
    EXPECT_FALSE((MaskRef{m, true}).isNoneTypeMask());
  }
}

TEST(FieldMaskTest, IsExclusive) {
  EXPECT_FALSE((MaskRef{noneMask(), false}).isExclusive());
  EXPECT_FALSE((MaskRef{allMask(), true}).isExclusive());
  EXPECT_TRUE((MaskRef{allMask(), false}).isExclusive());
  EXPECT_TRUE((MaskRef{noneMask(), true}).isExclusive());
  {
    Mask m;
    m.includes().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.includes_map().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes_map().emplace()[5] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.includes_string_map().emplace()["5"] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes_string_map().emplace()["5"] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isExclusive());
    EXPECT_TRUE((MaskRef{m, true}).isExclusive());
  }
  {
    Mask m;
    m.excludes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_TRUE((MaskRef{m, false}).isExclusive());
    EXPECT_FALSE((MaskRef{m, true}).isExclusive());
  }
}

TEST(FieldMaskTest, MaskRefIsMask) {
  EXPECT_TRUE((MaskRef{allMask(), false}).isFieldMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isMapMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isIntegerMapMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isStringMapMask());
  EXPECT_FALSE((MaskRef{allMask(), false}).isTypeMask());
  EXPECT_TRUE((MaskRef{noneMask(), true}).isFieldMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isMapMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isIntegerMapMask());
  EXPECT_FALSE((MaskRef{noneMask(), true}).isStringMapMask());
  EXPECT_FALSE((MaskRef{noneMask(), false}).isTypeMask());
  {
    Mask m;
    m.includes().emplace()[5] = allMask();
    EXPECT_TRUE((MaskRef{m, false}).isFieldMask());
    EXPECT_FALSE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_TRUE((MaskRef{m, false}).isFieldMask());
    EXPECT_FALSE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.includes_map().emplace()[5] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.excludes_map().emplace()[5] = noneMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.includes_string_map().emplace()["5"] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isStringMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.excludes_string_map().emplace()["5"] = noneMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_TRUE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isStringMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.includes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_FALSE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isTypeMask());
  }
  {
    Mask m;
    m.excludes_type().emplace()[type::infer_tag<Foo>{}] = allMask();
    EXPECT_FALSE((MaskRef{m, false}).isFieldMask());
    EXPECT_FALSE((MaskRef{m, true}).isMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isIntegerMapMask());
    EXPECT_FALSE((MaskRef{m, true}).isStringMapMask());
    EXPECT_TRUE((MaskRef{m, true}).isTypeMask());
  }
}

TEST(FieldMaskTest, MaskRefGetMask) {
  {
    Mask m;
    m.includes().emplace()[5] = allMask();
    EXPECT_EQ(getFieldMask(m), &*m.includes());
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.excludes().emplace()[5] = noneMask();
    EXPECT_EQ(getFieldMask(m), &*m.excludes());
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.includes_map().emplace()[5] = allMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), &*m.includes_map());
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.excludes_map().emplace()[5] = noneMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), &*m.excludes_map());
    EXPECT_EQ(getStringMapMask(m), nullptr);
  }
  {
    Mask m;
    m.includes_string_map().emplace()["5"] = allMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), &*m.includes_string_map());
  }
  {
    Mask m;
    m.excludes_string_map().emplace()["5"] = noneMask();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), &*m.excludes_string_map());
  }
  {
    Mask m;
    m.includes_type().ensure();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), nullptr);
    EXPECT_EQ(getTypeMask(m), &*m.includes_type());
  }
  {
    Mask m;
    m.excludes_type().ensure();
    EXPECT_EQ(getFieldMask(m), nullptr);
    EXPECT_EQ(getIntegerMapMask(m), nullptr);
    EXPECT_EQ(getStringMapMask(m), nullptr);
    EXPECT_EQ(getTypeMask(m), &*m.excludes_type());
  }
}

TEST(FieldMaskTest, ThrowIfContainsMapMask) {
  throwIfContainsMapMask(allMask()); // don't throw
  throwIfContainsMapMask(noneMask()); // don't throw
  {
    Mask m;
    m.includes_map().emplace()[5] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_map().emplace()[5] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    m.includes_string_map().emplace()["5"] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_string_map().emplace()["5"] = allMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes().emplace();
    includes[1] = allMask();
    includes[2].excludes().emplace()[5] = noneMask();
    throwIfContainsMapMask(m); // don't throw
  }
  {
    Mask m;
    auto& includes = m.includes().emplace();
    includes[1] = allMask();
    includes[2].includes_map().emplace()[5] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes().emplace();
    includes[1] = allMask();
    includes[2].excludes_map().emplace()[5] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes().emplace();
    includes[1] = allMask();
    includes[2].includes_string_map().emplace()["5"] = noneMask();
    EXPECT_THROW(throwIfContainsMapMask(m), std::runtime_error);
  }
  {
    Mask m;
    auto& includes = m.includes().emplace();
    includes[1] = allMask();
    includes[2].excludes_string_map().emplace()["5"] = noneMask();
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

  // MaskRef::get
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

  // MaskRef::tryGet
  EXPECT_FALSE(ref.tryGet(getId<Id>(7)).has_value()); // doesn't exist
  EXPECT_FALSE(refExclusion.tryGet(getId<Id>(7)).has_value()); // doesn't exist
  EXPECT_TRUE(ref.tryGet(getId<Id>(8))->isAllMask());
  EXPECT_TRUE(refExclusion.tryGet(getId<Id>(8))->isNoneMask());
  EXPECT_TRUE(literallyEqual(
      ref.tryGet(getId<Id>(9)).value(), (MaskRef{nested, false})));
  EXPECT_TRUE(literallyEqual(
      refExclusion.tryGet(getId<Id>(9)).value(), (MaskRef{nested, true})));
  // recursive calls to MaskRef::tryGet
  EXPECT_TRUE(ref.tryGet(getId<Id>(9))->tryGet(getId<Id>(4))->isAllMask());
  EXPECT_TRUE(
      refExclusion.tryGet(getId<Id>(9))->tryGet(getId<Id>(4))->isNoneMask());
  EXPECT_FALSE(ref.tryGet(getId<Id>(9))
                   ->tryGet(getId<Id>(5))
                   .has_value()); // doesn't exist
  EXPECT_FALSE(refExclusion.tryGet(getId<Id>(9))
                   ->tryGet(getId<Id>(5))
                   .has_value()); // doesn't exist
}

TEST(FieldMaskTest, MaskRefGetIncludes) {
  Mask mask;
  // includes{8: excludes{},
  //          9: includes{4: excludes{}}
  auto& includes = mask.includes().emplace();
  includes[8] = allMask();
  includes[9].includes().emplace()[4] = allMask();

  testMaskRefGetIncludes<FieldId>(mask, includes[9]);
}

TEST(FieldMaskTest, MaskRefGetIncludesMap) {
  Mask mask;
  // includes_map{8: excludes{},
  //              9: includes_map{4: excludes{}}
  auto& includes = mask.includes_map().emplace();
  includes[8] = allMask();
  includes[9].includes_map().emplace()[4] = allMask();

  testMaskRefGetIncludes<MapId>(mask, includes[9]);
}

TEST(FieldMaskTest, MaskRefGetIncludesStringMap) {
  Mask mask;
  // includes_string_map{"8": excludes{},
  //                     "9": includes_string_map{"4": excludes{}}
  auto& includes = mask.includes_string_map().emplace();
  includes["8"] = allMask();
  includes["9"].includes_string_map().emplace()["4"] = allMask();

  testMaskRefGetIncludes<std::string>(mask, includes["9"]);
}

// test MaskRef get method with excludes mask. nested is mask.excludes[_map][9].
template <typename Id>
void testMaskRefGetExcludes(const Mask& mask, const Mask& nested) {
  MaskRef ref{mask, false};
  MaskRef refExclusion{mask, true};

  // MaskRef::get
  EXPECT_TRUE(ref.get(getId<Id>(7)).isAllMask()); // doesn't exist
  EXPECT_TRUE(refExclusion.get(getId<Id>(7)).isNoneMask()); // doesn't exist
  EXPECT_TRUE(ref.get(getId<Id>(8)).isNoneMask());
  EXPECT_TRUE(refExclusion.get(getId<Id>(8)).isAllMask());
  EXPECT_TRUE(literallyEqual(ref.get(getId<Id>(9)), (MaskRef{nested, true})));
  EXPECT_TRUE(
      literallyEqual(refExclusion.get(getId<Id>(9)), (MaskRef{nested, false})));
  // recursive calls to MaskRef::get
  EXPECT_TRUE(ref.get(getId<Id>(9)).get(getId<Id>(4)).isNoneMask());
  EXPECT_TRUE(refExclusion.get(getId<Id>(9)).get(getId<Id>(4)).isAllMask());
  EXPECT_TRUE(
      ref.get(getId<Id>(9)).get(getId<Id>(5)).isAllMask()); // doesn't exist
  EXPECT_TRUE(refExclusion.get(getId<Id>(9))
                  .get(getId<Id>(5))
                  .isNoneMask()); // doesn't exist

  // MaskRef::tryGet
  EXPECT_FALSE(ref.tryGet(getId<Id>(7)).has_value()); // doesn't exist
  EXPECT_FALSE(refExclusion.tryGet(getId<Id>(7)).has_value()); // doesn't exist
  EXPECT_TRUE(ref.tryGet(getId<Id>(8))->isNoneMask());
  EXPECT_TRUE(refExclusion.tryGet(getId<Id>(8))->isAllMask());
  EXPECT_TRUE(literallyEqual(
      ref.tryGet(getId<Id>(9)).value(), (MaskRef{nested, true})));
  EXPECT_TRUE(literallyEqual(
      refExclusion.tryGet(getId<Id>(9)).value(), (MaskRef{nested, false})));
  // recursive calls to MaskRef::tryGet
  EXPECT_TRUE(ref.tryGet(getId<Id>(9))->tryGet(getId<Id>(4))->isNoneMask());
  EXPECT_TRUE(
      refExclusion.tryGet(getId<Id>(9))->tryGet(getId<Id>(4))->isAllMask());
  EXPECT_FALSE(ref.tryGet(getId<Id>(9))
                   ->tryGet(getId<Id>(5))
                   .has_value()); // doesn't exist
  EXPECT_FALSE(refExclusion.tryGet(getId<Id>(9))
                   ->tryGet(getId<Id>(5))
                   .has_value()); // doesn't exist
}

TEST(FieldMaskTest, MaskRefGetExcludes) {
  Mask mask;
  // excludes{8: excludes{},
  //          9: includes{4: excludes{}}
  auto& excludes = mask.excludes().emplace();
  excludes[8] = allMask();
  excludes[9].includes().emplace()[4] = allMask();

  testMaskRefGetExcludes<FieldId>(mask, excludes[9]);
}

TEST(FieldMaskTest, MaskRefGetExcludesMap) {
  Mask mask;
  // excludes_map{8: excludes{},
  //              9: includes_map{4: excludes{}}
  auto& excludes = mask.excludes_map().emplace();
  excludes[8] = allMask();
  excludes[9].includes_map().emplace()[4] = allMask();

  testMaskRefGetExcludes<MapId>(mask, excludes[9]);
}

TEST(FieldMaskTest, MaskRefGetExcludesStringMap) {
  Mask mask;
  // excludes_string_map{"8": excludes{},
  //                     "9": includes_string_map{4: excludes{}}
  auto& excludes = mask.excludes_string_map().emplace();
  excludes["8"] = allMask();
  excludes["9"].includes_string_map().emplace()["4"] = allMask();

  testMaskRefGetExcludes<std::string>(mask, excludes["9"]);
}

TEST(FieldMaskTest, MaskRefGetAllMaskNoneMask) {
  {
    MaskRef ref{allMask(), false};
    EXPECT_TRUE(ref.get(FieldId{1}).isAllMask());
    EXPECT_TRUE(ref.get(MapId{1}).isAllMask());
    EXPECT_TRUE(ref.get("1"sv).isAllMask());
  }
  {
    MaskRef ref{allMask(), true};
    EXPECT_TRUE(ref.get(FieldId{1}).isNoneMask());
    EXPECT_TRUE(ref.get(MapId{1}).isNoneMask());
    EXPECT_TRUE(ref.get("1"sv).isNoneMask());
  }
  {
    MaskRef ref{noneMask(), false};
    EXPECT_TRUE(ref.get(FieldId{1}).isNoneMask());
    EXPECT_TRUE(ref.get(MapId{1}).isNoneMask());
    EXPECT_TRUE(ref.get("1"sv).isNoneMask());
  }
  {
    MaskRef ref{noneMask(), true};
    EXPECT_TRUE(ref.get(FieldId{1}).isAllMask());
    EXPECT_TRUE(ref.get(MapId{1}).isAllMask());
    EXPECT_TRUE(ref.get("1"sv).isAllMask());
  }
}

TEST(FieldMaskTest, MaskRefGetException) {
  {
    Mask m;
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get(MapId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{1}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("1"sv), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("1"sv), std::runtime_error);
  }
  {
    Mask m;
    m.includes_map().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"sv), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"sv), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_map().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"sv), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"sv), std::runtime_error);
  }
  {
    Mask m;
    m.includes().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"sv), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"sv), std::runtime_error);
  }
  {
    Mask m;
    m.excludes().emplace()[4] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get("4"sv), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get("4"sv), std::runtime_error);
  }
  {
    Mask m;
    m.includes_string_map().emplace()["4"] = allMask();
    EXPECT_THROW((MaskRef{m, false}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(FieldId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, false}).get(MapId{4}), std::runtime_error);
    EXPECT_THROW((MaskRef{m, true}).get(MapId{4}), std::runtime_error);
  }
  {
    Mask m;
    m.excludes_string_map().emplace()["4"] = allMask();
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
  auto& includes = mask.includes().emplace();
  includes[2] = allMask();
  auto& nestedExcludes = includes[1].excludes().emplace();
  nestedExcludes[5].excludes().emplace()[5] =
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
    auto& includes = m.includes().emplace();
    includes[2].includes().emplace()[4] = noneMask();
    EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
  }

  {
    protocol::Object fooObject, barObject;
    // bar{1: foo{2: 20}, 2: "40"}
    fooObject[FieldId{2}].ensure_i32() = 20;
    barObject[FieldId{1}].ensure_object() = fooObject;
    barObject[FieldId{2}].ensure_string() = "40";
    Mask m; // object[1][2] is not an object but has an object mask.
    auto& includes = m.includes().emplace();
    includes[1].includes().emplace()[2].excludes().emplace()[5] = allMask();
    includes[2] = allMask();
    EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
  }

  {
    protocol::Object barObject;
    barObject[FieldId{1}] =
        asValueStruct<type::map<type::i32_t, type::string_t>>({{1, "1"}});
    Mask m; // object[1] is a map but has an object mask.
    m.includes().emplace()[1].includes().emplace()[1] = allMask();
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
  auto& includes = mask.includes().emplace();
  includes[2] = allMask();
  auto& nestedExcludes = includes[1].excludes_map().emplace();
  nestedExcludes[5].excludes_map().emplace()[5] =
      allMask(); // The object doesn't have this field.
  nestedExcludes[1].includes_map().emplace()[1] = allMask();
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
  auto& includes = mask.includes().emplace();
  includes[2] = allMask();
  auto& nestedExcludes = includes[1].excludes_string_map().emplace();
  nestedExcludes["5"].excludes_string_map().emplace()["5"] =
      allMask(); // The object doesn't have this field.
  nestedExcludes["1"].includes_string_map().emplace()["1"] = allMask();
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
      auto& includes = m.includes().emplace();
      includes[2].includes_map().emplace()[4] = noneMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[2] is not a map but has a string map mask.
      auto& includes = m.includes().emplace();
      includes[2].includes_string_map().emplace()["4"] = noneMask();
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
      auto& includes = m.includes().emplace();
      includes[1].includes().emplace()[2].excludes_map().emplace()[5] =
          allMask();
      includes[2] = allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[1][2] is not a map but has a string map mask.
      auto& includes = m.includes().emplace();
      includes[1].includes().emplace()[2].excludes_string_map().emplace()["5"] =
          allMask();
      includes[2] = allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
  }

  {
    protocol::Object barObject, fooObject;
    barObject[FieldId{1}].ensure_object() = fooObject;
    {
      Mask m; // object[1] is an object but has an integer map mask.
      m.includes().emplace()[1].includes_map().emplace()[1] = allMask();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[1] is an object but has a string map mask.
      m.includes().emplace()[1].includes_string_map().emplace()["1"] =
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
      m.includes().emplace()[1].includes_map().emplace();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
    {
      Mask m; // object[1] has invalid map key.
      m.includes().emplace()[1].includes_string_map().emplace();
      EXPECT_THROW(protocol::clear(m, barObject), std::runtime_error);
    }
  }
}

// Test that:
// 1. filter(mask, filter(mask, src)) == filter(mask, src)
// 2. filter(mask, asObject(src)) == asObject(filter(mask, src))
// returns filter(mask, src)
template <typename T>
T testFilter(const Mask& mask, const T& src) {
  auto filtered = protocol::filter(mask, src);
  auto filteredObj = protocol::filter(mask, protocol::asObject(src));

  EXPECT_EQ(
      filtered, protocol::fromObjectStruct<type::infer_tag<T>>(filteredObj));
  return filtered;
}

TEST(FieldMaskTest, FilterSimple) {
  {
    Foo src;

    MaskBuilder<Foo> m(noneMask());
    auto filtered = testFilter(m.toThrift(), src);
    EXPECT_EQ(src, filtered);
  }
  {
    Foo src;
    src.field1() = 1;
    src.field2() = 2;

    MaskBuilder<Foo> m(allMask());
    auto filtered = testFilter(m.toThrift(), src);
    EXPECT_EQ(src, filtered);
  }
  {
    Foo src;
    src.field1() = 1;
    src.field2() = 2;
    protocol::Object fooObj(protocol::asObject(folly::copy(src)));

    MaskBuilder<Foo> m(noneMask());
    m.includes<ident::field1>();

    auto filtered = testFilter(m.toThrift(), src);
    EXPECT_EQ(filtered.field1(), 1);
    EXPECT_EQ(filtered.field2(), 0);
  }
  {
    // nested struct
    Bar src;
    auto& foo = src.foo().ensure();
    foo.field1() = 1;
    foo.field2() = 2;

    MaskBuilder<Bar> m(noneMask());
    m.includes<ident::foo, ident::field1>();

    auto filtered = testFilter(m.toThrift(), src);
    EXPECT_EQ(filtered.foo()->field1(), 1);
    EXPECT_EQ(filtered.foo()->field2(), 0);
  }
  {
    // nested struct + optional field
    Bar2 src, filtered;
    src.field_3().emplace().field_1() = 1;

    MaskBuilder<Bar2> m(noneMask());
    m.includes<ident::field_3, ident::field_1>();
    filtered = testFilter(m.toThrift(), src);
    EXPECT_EQ(filtered.field_3().value().field_1().value(), 1);

    m.reset_to_none().includes<ident::field_3, ident::field_2>();
    filtered = testFilter(m.toThrift(), src);
    // No sub field filtered, so field_3 should be null
    EXPECT_FALSE(filtered.field_3().has_value());
  }
}

TEST(FieldMaskTest, FilterException) {
  {
    protocol::Object fooObject, barObject, bazObject;
    // bar{1: foo{2: 20}, 2: "40"}
    fooObject[FieldId{2}].ensure_i32() = 20;
    barObject[FieldId{1}].ensure_object() = fooObject;
    barObject[FieldId{2}].ensure_string() = "40";
    // baz{2: {3: 40}}
    bazObject[FieldId{2}].ensure_object()[FieldId{3}].ensure_i32() = 40;

    Mask m1; // bar[2] is not an object but has an object mask.
    m1.includes().emplace()[2].includes().emplace()[3] = allMask();
    EXPECT_THROW(protocol::filter(m1, barObject), std::runtime_error);
    // baz[2] is an object, but since bar[2] is not, it still throws an error.
    EXPECT_THROW(protocol::filter(m1, barObject), std::runtime_error);
  }
  {
    Bar2 src;
    Mask m1; // m1 = includes{2: includes{4: includes{}}}
    auto& includes = m1.includes().emplace();
    includes[2].includes().emplace()[4] = noneMask();
    EXPECT_THROW(protocol::filter(m1, src), std::runtime_error);

    Mask m2; // m2 = includes{1: includes{2: includes{5: excludes{}}}}
    auto& includes2 = m2.includes().emplace();
    includes2[1].includes().emplace()[2].excludes().emplace()[5] = allMask();
    includes2[2] = allMask();
    EXPECT_THROW(protocol::filter(m2, src), std::runtime_error);
  }
}

TEST(FieldMaskTest, SchemalessFilterSimpleMap) {
  protocol::Object src, expected;
  // src{1: map{1: "1",
  //            2: "2"}}
  src[FieldId{1}] = asValueStruct<type::map<type::i64_t, type::string_t>>(
      {{1, "1"}, {2, "2"}});
  // expected{1: map{1: "1"}
  expected[FieldId{1}] =
      asValueStruct<type::map<type::i64_t, type::string_t>>({{1, "1"}});

  Mask mask;
  mask.includes().emplace()[1].includes_map().emplace()[1] = allMask();
  EXPECT_EQ(protocol::filter(mask, src), expected);
}

TEST(FieldMaskTest, SchemalessFilterSimpleStringMap) {
  protocol::Object src, dst, expected;
  // src{1: map{"1": "1", "2": "2"}}
  {
    std::map<std::string, std::string> map = {{"1", "1"}, {"2", "2"}};
    src[FieldId{1}] =
        asValueStruct<type::map<type::string_t, type::string_t>>(map);
  }
  // expected{1: map{"1": "1"}}
  {
    std::map<std::string, std::string> map = {{"1", "1"}};
    expected[FieldId{1}] =
        asValueStruct<type::map<type::string_t, type::string_t>>(map);
  }

  Mask mask;
  mask.includes().emplace()[1].includes_string_map().emplace()["1"] = allMask();
  EXPECT_EQ(protocol::filter(mask, src), expected);
}

TEST(FieldMaskTest, SchemalessFilterNestedMap) {
  protocol::Object src, expected;
  // src{1: map{1: map{1: "1",
  //                   2: "2"},
  //            2: map{3: "3"}}}
  src[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{1, "1"}, {2, "2"}}}, {2, {{3, "3"}}}});
  expected[FieldId{1}] = asValueStruct<
      type::map<type::i64_t, type::map<type::i16_t, type::string_t>>>(
      {{1, {{2, "2"}}}, {2, {{3, "3"}}}});

  Mask mask;
  // includes{1: includes_map{1: excludes_map{1: allMask()},
  //                          2: allMask()}}
  auto& nestedIncludes = mask.includes().emplace()[1].includes_map().emplace();
  nestedIncludes[1].excludes_map().emplace()[1] = allMask();
  nestedIncludes[2] = allMask();
  // this copies src[1][1][2] and src[1][2]
  EXPECT_EQ(protocol::filter(mask, src), expected);
}

TEST(FieldMaskTest, SchemalessFilterNestedStringMap) {
  protocol::Object src, expected;
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
  // expected{1: map{"1": map{"1": "1",
  //                          "2": "6"}},
  //                 "2": map{"3": "3"}}}
  {
    std::map<std::string, std::map<std::string, std::string>> map = {
        {"1", {{"2", "2"}}}, {"2", {{"3", "3"}}}};
    expected[FieldId{1}] = asValueStruct<
        type::map<type::string_t, type::map<type::string_t, type::string_t>>>(
        map);
  }

  Mask mask;
  // includes{1: includes_string_map{"1": excludes_string_map{"1": allMask()},
  //                                 "2": allMask()}}
  auto& nestedIncludes =
      mask.includes().emplace()[1].includes_string_map().emplace();
  nestedIncludes["1"].excludes_string_map().emplace()["1"] = allMask();
  nestedIncludes["2"] = allMask();
  EXPECT_EQ(protocol::filter(mask, src), expected);
}

TEST(FieldMaskTest, SchemalessFilterExceptionMap) {
  protocol::Object barObject, bazObject;
  // bar{1: map{2: 20}, 2: "40"}
  barObject[FieldId{1}] =
      asValueStruct<type::map<type::byte_t, type::i32_t>>({{2, 20}});
  barObject[FieldId{2}].ensure_string() = "40";

  Mask m1; // bar[2] is not a map but has an integer map mask.
  m1.includes().emplace()[2].includes_map().emplace()[3] = allMask();
  EXPECT_THROW(protocol::filter(m1, barObject), std::runtime_error);
}

TEST(FieldMaskTest, SchemalessFilterExceptionStringMap) {
  protocol::Object barObject;
  // bar{1: map{"2": 20}, 2: "40"}
  barObject[FieldId{1}] =
      asValueStruct<type::map<type::string_t, type::i32_t>>({{"2", 20}});
  barObject[FieldId{2}].ensure_string() = "40";

  Mask m1; // bar[2] is not a map but has a string map mask.
  m1.includes().emplace()[2].includes_string_map().emplace()["3"] = allMask();
  EXPECT_THROW(protocol::filter(m1, barObject), std::runtime_error);
}

TEST(FieldMaskTest, FilterTerseWrite) {
  // Makes sure filter doesn't use has_value() for all fields.
  TerseWrite src;
  src.field() = 4;
  src.foo()->field1() = 5;
  auto dst = protocol::filter(allMask(), src);
  EXPECT_EQ(dst, src);
}

TEST(FieldMaskTest, FilterSmartPointer) {
  // test with allMask and noneMask
  SmartPointerStruct full, dst, empty;
  protocol::ensure(allMask(), full);

  dst = protocol::filter(allMask(), empty);
  assertSmartPointerStructIsEmpty(dst);

  dst = protocol::filter(noneMask(), full);
  assertSmartPointerStructIsEmpty(dst);

  dst = protocol::filter(allMask(), full);
  assertSmartPointerStructHasAllValues(dst);

  dst = protocol::filter(allMask(), empty);
  assertSmartPointerStructIsEmpty(dst);
}

TEST(FieldMaskTest, FilterUnion) {
  MaskBuilder<RecursiveUnion> m(noneMask());
  RecursiveUnion leaf, parent, dst;

  leaf.foo().emplace().field1() = 1;
  leaf.foo()->field2() = 2;

  dst = protocol::filter(allMask(), leaf);
  EXPECT_EQ(leaf, dst);

  dst = protocol::filter(noneMask(), leaf);
  EXPECT_EQ(dst.getType(), RecursiveUnion::Type::__EMPTY__);

  parent.recurse().emplace(leaf);
  dst = protocol::filter(allMask(), parent);
  EXPECT_EQ(parent, dst);

  m.includes<ident::recurse, ident::bar>();
  dst = m.filter(parent);
  // Filter failed, union-ref should remain empty
  EXPECT_EQ(dst.getType(), RecursiveUnion::Type::__EMPTY__);

  m.reset_to_none().includes<ident::recurse, ident::foo, ident::field1>();
  dst = m.filter(parent);
  EXPECT_EQ(*dst.recurse().value().foo().value().field1(), 1);
  EXPECT_EQ(*dst.recurse().value().foo().value().field2(), 0); // not filtered
}

bool compareAny(const type::AnyStruct& lhs, const type::AnyStruct& rhs) {
  if (lhs.type() != rhs.type()) {
    return false;
  }

  if (lhs.type() == type::Type{}) {
    return true;
  }

  return protocol::detail::parseValueFromAny(lhs) ==
      protocol::detail::parseValueFromAny(rhs);
}

// Tests that schemaful + schemless clear do the same thing
template <typename T, typename Eq>
T applyFilter(const Mask& m, const T& val, Eq eq) {
  auto filtered = protocol::filter(m, val);
  auto filteredObject = protocol::filter(m, protocol::asObject(val));

  // Make sure filtered and filteredObject are the same
  EXPECT_TRUE(
      eq(filtered,
         protocol::fromObjectStruct<type::infer_tag<T>>(filteredObject)));

  return filtered;
}

type::AnyStruct applyFilterOnAny(const Mask& m, const type::AnyStruct& val) {
  return applyFilter(m, val, compareAny);
}

StructWithAny applyFilterOnStructWithAny(
    const Mask& m, const StructWithAny& val) {
  return applyFilter(
      m, val, [](const StructWithAny& lhs, const StructWithAny& rhs) {
        if (!compareAny(*lhs.rawAny(), *rhs.rawAny())) {
          return false;
        }
        if (!compareAny(
                lhs.adaptedAny()->toThrift(), rhs.adaptedAny()->toThrift())) {
          return false;
        }
        if (lhs.optAny().has_value() != rhs.optAny().has_value()) {
          return false;
        }
        if (lhs.optAny().has_value()) {
          return compareAny(*lhs.optAny(), *rhs.optAny());
        }
        return true;
      });
}

template <typename Tag, typename T = type::native_type<Tag>>
void testFilterAnyStruct(T data) {
  // Baz is used for type mismatch test
  static_assert(!std::is_same_v<T, Baz>);
  auto wrappedData = type::AnyData::toAny<Tag>(folly::copy(data)).toThrift();

  MaskBuilder<type::AnyStruct> m;

  // noneMask
  m.reset_to_none();
  EXPECT_EQ(applyFilterOnAny(m.toThrift(), wrappedData), type::AnyStruct{});

  // type mismatch
  m.reset_to_none().includes_type<>(type::infer_tag<Baz>{});
  EXPECT_EQ(applyFilterOnAny(m.toThrift(), wrappedData), type::AnyStruct{});

  // type match
  m.reset_to_none().includes_type<>(Tag{});
  EXPECT_EQ(
      type::AnyData(applyFilterOnAny(m.toThrift(), wrappedData)).get<Tag>(),
      data);
}

TEST(FieldMaskTest, FilterAny) {
  Foo foo;
  foo.field1() = 123;
  MaskBuilder<Foo> fooMask;
  fooMask.reset_to_none().includes<ident::field1>();

  testFilterAnyStruct<type::bool_t>(true);
  testFilterAnyStruct<type::i32_t>(123);
  testFilterAnyStruct<type::binary_t>("foobar");
  testFilterAnyStruct<type::list<type::float_t>>(std::vector<float>{1.0, 3.0});
  testFilterAnyStruct<type::set<type::i64_t>>(std::set<int64_t>{1, 2, 3});
  testFilterAnyStruct<type::map<type::binary_t, type::i64_t>>(
      std::map<std::string, int64_t>{{"1", 1}, {"3", 3}});
  testFilterAnyStruct<type::infer_tag<Foo>>(foo);

  // nested field filter
  StructWithAny s;
  s.rawAny() = type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift();
  MaskBuilder<StructWithAny> m;
  m.reset_to_none().includes_type<ident::rawAny>(
      type::infer_tag<Foo>{}, fooMask.toThrift());
  EXPECT_EQ(
      foo.field1(),
      type::AnyData(*applyFilterOnStructWithAny(m.toThrift(), s).rawAny())
          .get<type::infer_tag<Foo>>()
          .field1());

  // nested field filter that fails
  s.optAny() = type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift();
  m.reset_to_none().includes_type<ident::optAny>(
      type::struct_t<Bar>{}, MaskBuilder<Bar>().reset_to_all().toThrift());
  EXPECT_FALSE(
      applyFilterOnStructWithAny(m.toThrift(), s).optAny().has_value());

  // Nested any
  // StructWithAny -> Any -> Any -> Foo
  s.rawAny() = type::AnyData::toAny<type::infer_tag<type::AnyStruct>>(
                   type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift())
                   .toThrift();
  m.reset_to_none().includes_type<ident::rawAny>(
      type::infer_tag<type::AnyStruct>{},
      MaskBuilder<type::AnyStruct>()
          .reset_to_none()
          .includes_type<>(type::infer_tag<Foo>{}, fooMask.toThrift())
          .toThrift());
  EXPECT_EQ(
      foo.field1(),
      type::AnyData(*applyFilterOnStructWithAny(m.toThrift(), s).rawAny())
          .get<type::infer_tag<type::AnyData>>()
          .get<type::infer_tag<Foo>>()
          .field1());
}

TEST(FieldMaskTest, FilterAnyWithHashedURI) {
  auto fooType = type::Type::get<type::infer_tag<Foo>>();
  convertToHashedURI(fooType);

  Foo foo;
  foo.field1() = 1;
  auto fooAny = type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift();
  fooAny.type() = fooType;

  MaskBuilder<type::AnyStruct> m;

  // type mismatch
  m.reset_to_none().includes_type<>(type::struct_t<Bar>{}, allMask());
  type::AnyStruct filtered = applyFilterOnAny(m.toThrift(), fooAny);
  EXPECT_EQ(filtered, type::AnyStruct{});

  // type match
  m.reset_to_none().includes_type<>(type::struct_t<Foo>{}, allMask());
  filtered = applyFilterOnAny(m.toThrift(), fooAny);
  EXPECT_EQ(getViaIdenticalType<Foo>(filtered, fooType), foo);
}

TEST(FieldMaskTest, IsCompatibleWithSimple) {
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Foo>>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Foo>>(noneMask()));

  Mask m;
  auto& includes = m.includes().emplace();
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
  m.includes().emplace()[1] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{2: includes{}}
  m.includes().emplace()[2] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{1: includes{1: excludes{}}, 2: excludes{}}
  auto& includes = m.includes().emplace();
  includes[1].includes().emplace()[1] = allMask();
  includes[2] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Bar>>(m));

  // There are invalid field masks for Bar.
  // includes{3: excludes{}}
  m.includes().emplace()[3] = allMask();
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{2: includes{1: includes{}}}
  m.includes().emplace()[2].includes().emplace()[1] = noneMask();
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
  // includes{1: excludes{2: excludes{}}}
  m.includes().emplace()[1].excludes().emplace()[2] = allMask();
  EXPECT_FALSE(protocol::is_compatible_with<type::struct_t<Bar>>(m));
}

TEST(FieldMaskTest, IsCompatibleWithAdaptedField) {
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Baz>>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Baz>>(noneMask()));

  Mask m;
  // includes{1: excludes{}}
  m.includes().emplace()[1] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<type::struct_t<Baz>>(m));
  // includes{1: includes{1: excludes{}}}
  m.includes().emplace()[1].includes().emplace()[1] = allMask();
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
    auto& includes = mask.includes().emplace();
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
    auto& includes = mask.includes().emplace();
    includes[1].excludes().emplace()[2] = allMask();
    includes[2].excludes().emplace()[2] = noneMask();
    includes[3].includes().emplace()[1] = allMask();
    EXPECT_TRUE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
  {
    Mask mask;
    // includes{1: excludes{100: excludes{}}}
    mask.includes().emplace()[1].excludes().emplace()[100] = allMask();
    EXPECT_FALSE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
  {
    Mask mask;
    // includes{2: includes{100: excludes{}}}
    mask.includes().emplace()[2].includes().emplace()[100] = allMask();
    EXPECT_FALSE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
  {
    Mask mask;
    // includes{3: includes{1: excludes{1: excludes{}}}}
    auto& includes = mask.includes().emplace();
    includes[3].includes().emplace()[1].excludes().emplace()[1] = allMask();
    EXPECT_FALSE(
        protocol::is_compatible_with<type::struct_t<SmartPointerStruct>>(mask));
  }
}

TEST(FieldMaskTest, IsCompatibleWithMap) {
  Mask m;
  auto& includes = m.includes_map().emplace()[1].includes().emplace();
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
  auto& includes = m.includes_string_map().emplace()["1"].includes().emplace();
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
  auto& includes = m.includes_map()
                       .emplace()[1]
                       .includes_map()
                       .emplace()[1]
                       .includes()
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
  auto& includes = m.includes_string_map()
                       .emplace()["1"]
                       .includes_string_map()
                       .emplace()["1"]
                       .includes()
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
  mask.includes().emplace()[1] = allMask();

  EXPECT_TRUE(protocol::is_compatible_with<type::i32_t>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<type::i32_t>(noneMask()));
  EXPECT_FALSE(protocol::is_compatible_with<type::i32_t>(mask));
  EXPECT_TRUE(
      protocol::is_compatible_with<type::list<type::string_t>>(allMask()));
  EXPECT_TRUE(
      protocol::is_compatible_with<type::list<type::string_t>>(noneMask()));
  EXPECT_FALSE(protocol::is_compatible_with<type::list<type::string_t>>(mask));
}

TEST(FieldMaskTest, IsCompatibleWithUnion) {
  using UnionTag = type::union_t<RecursiveUnion>;
  EXPECT_TRUE(protocol::is_compatible_with<RecursiveUnion>(allMask()));
  EXPECT_TRUE(protocol::is_compatible_with<RecursiveUnion>(noneMask()));

  Mask m;
  auto& includes = m.includes().emplace();
  includes[1] = allMask();
  EXPECT_TRUE(protocol::is_compatible_with<UnionTag>(m));
  includes[2] = noneMask();
  EXPECT_TRUE(protocol::is_compatible_with<UnionTag>(m));

  Mask invalid(m);
  (*invalid.includes())[100] = allMask(); // doesn't exist
  EXPECT_FALSE(protocol::is_compatible_with<UnionTag>(invalid));

  Mask nested(m);
  (*nested.includes())[4] = m;
  EXPECT_TRUE(protocol::is_compatible_with<UnionTag>(m));

  {
    Mask invalidNested(nested);
    (*(*invalidNested.includes())[4].includes())[4] = invalid;
    EXPECT_FALSE(protocol::is_compatible_with<UnionTag>(invalidNested));
  }

  Mask mapMask;
  auto& mapIncludes = mapMask.includes_string_map().emplace();
  mapIncludes["1"] = Mask(m);
  mapIncludes["2"] = Mask(m);

  (*nested.includes())[5] = mapMask;
  EXPECT_TRUE(protocol::is_compatible_with<UnionTag>(m));
}

TEST(FieldMaskTest, Ensure) {
  Mask mask;
  // mask = includes{1: includes{2: excludes{}},
  //                 2: excludes{}}
  auto& includes = mask.includes().emplace();
  includes[1].includes().emplace()[2] = allMask();
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
  auto& excludes = mask.excludes().emplace();
  auto& nestedIncludes = excludes[1].includes().emplace();
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
    assertPointerHasAllValues(obj.unique());
    EXPECT_FALSE(bool(obj.shared()));
    EXPECT_FALSE(bool(obj.boxed()));
  }
  {
    SmartPointerStruct obj;
    // Mask includes unique.field_1 and boxed except for boxed.field_2.
    MaskBuilder<SmartPointerStruct> builder(noneMask());
    builder.includes<ident::unique, ident::field_1>();
    builder.includes<ident::boxed>();
    builder.excludes<ident::boxed, ident::field_2>();
    protocol::ensure(builder.toThrift(), obj);
    ASSERT_TRUE(bool(obj.unique()));
    EXPECT_EQ(obj.unique()->field_1(), 0);
    EXPECT_FALSE(obj.unique()->field_2().has_value());
    EXPECT_FALSE(bool(obj.shared()));
    ASSERT_TRUE(bool(obj.boxed()));
    EXPECT_EQ(obj.boxed()->field_1(), 0);
    EXPECT_FALSE(obj.boxed()->field_2().has_value());
  }

  // Test ensure works with struct that has a shared const pointer field.
  {
    SharedConstPointerStruct obj;
    MaskBuilder<SharedConstPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    builder.ensure(obj);
    assertPointerHasAllValues(obj.unique());
    EXPECT_FALSE(obj.shared_const());

    // Cannot ensure the shared const field.
    builder.includes<ident::shared_const>();
    EXPECT_THROW(builder.ensure(obj), std::runtime_error);
  }
}

TEST(FieldMaskTest, EnsureException) {
  Mask mask;
  // mask = includes{1: includes{2: excludes{1: includes{}}}}
  mask.includes().emplace()[1].includes().emplace()[2].excludes().emplace()[1] =
      noneMask();
  Bar2 bar;
  EXPECT_THROW(ensure(mask, bar), std::runtime_error); // incompatible

  // includes{1: includes{1: excludes{}}}
  mask.includes().emplace()[1].includes().emplace()[1] = allMask();
  Baz baz;
  // adapted field cannot be masked.
  EXPECT_THROW(ensure(mask, baz), std::runtime_error);

  // includes{3: excludes{}}
  mask.includes().emplace()[3] = allMask();
  EXPECT_THROW(ensure(mask, bar), std::runtime_error); // incompatible
}

TEST(FieldMaskTest, EnsureUnion) {
  {
    RecursiveUnion u;

    ensure(noneMask(), u);
    EXPECT_EQ(u.getType(), RecursiveUnion::Type::__EMPTY__);

    // test validation for multiple fields
    EXPECT_THROW(ensure(allMask(), u), std::runtime_error);
  }
  {
    // Simple ensure
    RecursiveUnion u;
    MaskBuilder<RecursiveUnion> m(noneMask());
    m.includes<ident::foo>();
    ensure(m.toThrift(), u);
    EXPECT_TRUE(u.foo().has_value());

    m.includes<ident::bar>();
    // multiple fields by inclusion
    EXPECT_THROW(ensure(m.toThrift(), u), std::runtime_error);
  }
  {
    // Nested ensure
    RecursiveUnion u;
    MaskBuilder<RecursiveUnion> m(noneMask());
    m.includes<ident::recurse, ident::foo>();
    ensure(m.toThrift(), u);
    EXPECT_TRUE(u.recurse().has_value());
    EXPECT_TRUE(u.recurse()->foo().has_value());
  }
}

TEST(FieldMaskTest, EnsureAny) {
  MaskBuilder<StructWithAny> m(noneMask());
  m.includes_type<ident::rawAny>(type::infer_tag<Foo>{});
  StructWithAny s;
  EXPECT_THROW(m.ensure(s), std::runtime_error);
}

TEST(FieldMaskTest, SchemafulClear) {
  Mask mask;
  // mask = includes{1: includes{2: excludes{}},
  //                 2: excludes{}}
  auto& includes = mask.includes().emplace();
  includes[1].includes().emplace()[2] = allMask();
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
  auto& excludes = mask.excludes().emplace();
  excludes[1].includes().emplace()[1] = allMask();
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
  m.includes().emplace()[2].includes().emplace()[1] = allMask();
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
    EXPECT_FALSE(bool(obj.unique()));
    assertPointerHasAllValues(obj.shared());
    assertPointerHasAllValues(obj.boxed());
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
    ASSERT_TRUE(bool(obj.unique()));
    EXPECT_EQ(obj.unique()->field_1(), 0);
    EXPECT_FALSE(obj.unique()->field_2().has_value());
    EXPECT_FALSE(bool(obj.shared()));
    assertPointerHasAllValues(obj.boxed());
  }

  // Test clear works with struct that has a shared const pointer field.
  {
    SharedConstPointerStruct obj;
    MaskBuilder<SharedConstPointerStruct> builder(noneMask());
    builder.includes<ident::unique>();
    builder.ensure(obj);
    builder.clear(obj);
    EXPECT_EQ(*obj.unique(), Foo2{});
    EXPECT_FALSE(obj.shared_const());

    // Cannot clear a field inside the shared const field.
    builder.includes<ident::shared_const, ident::field_1>();
    obj.shared_const() = std::make_shared<Foo2>(Foo2{});
    EXPECT_THROW(builder.clear(obj), std::runtime_error);
  }
}

TEST(FieldMaskTest, SchemafulClearException) {
  Bar2 bar;
  Mask m1; // m1 = includes{2: includes{4: includes{}}}
  auto& includes = m1.includes().emplace();
  includes[2].includes().emplace()[4] = noneMask();
  EXPECT_THROW(protocol::clear(m1, bar), std::runtime_error);

  Mask m2; // m2 = includes{1: includes{2: includes{5: excludes{}}}}
  auto& includes2 = m2.includes().emplace();
  includes2[1].includes().emplace()[2].excludes().emplace()[5] = allMask();
  includes2[2] = allMask();
  EXPECT_THROW(protocol::clear(m2, bar), std::runtime_error);
}

TEST(FieldMaskTest, ClearUnion) {
  {
    RecursiveUnion u;
    protocol::clear(noneMask(), u);
    EXPECT_EQ(u.getType(), RecursiveUnion::Type::__EMPTY__);
    protocol::clear(allMask(), u);
    EXPECT_EQ(u.getType(), RecursiveUnion::Type::__EMPTY__);
  }
  {
    // Clear union arm
    RecursiveUnion u;
    u.foo().emplace();
    MaskBuilder<RecursiveUnion> m(noneMask());
    m.includes<ident::bar>();
    protocol::clear(m.toThrift(), u);
    EXPECT_TRUE(u.foo().has_value());
    m.includes<ident::foo>();
    protocol::clear(m.toThrift(), u);
    EXPECT_EQ(u.getType(), RecursiveUnion::Type::__EMPTY__);
  }
  {
    // Clear nested field in arm
    RecursiveUnion u;
    u.foo().emplace().field1() = 1;
    MaskBuilder<RecursiveUnion> m(noneMask());
    // {includes: {1: includes: {1: allMask()}}}
    m.includes<ident::foo, ident::field1>();
    protocol::clear(m.toThrift(), u);
    // Make sure union is still set but nested field is cleared
    EXPECT_TRUE(u.foo().has_value());
    EXPECT_EQ(u.foo()->field1(), 0);
  }
}

// Tests that schemaful + schemless clear do the same thing
template <typename T>
void applyClear(const Mask& m, T& val) {
  auto obj = protocol::asObject(val);
  protocol::clear(m, val);
  protocol::clear(m, obj);
  EXPECT_EQ(val, protocol::fromObjectStruct<type::infer_tag<T>>(obj));
}

template <typename Tag, typename T = type::native_type<Tag>>
void testClearAnyStruct(T data) {
  // Baz is used for type mismatch test
  static_assert(!std::is_same_v<T, Baz>);
  auto wrappedData = type::AnyData::toAny<Tag>(folly::copy(data)).toThrift();

  MaskBuilder<type::AnyStruct> m;

  // noneMask
  m.reset_to_none();
  applyClear(m.toThrift(), wrappedData);
  EXPECT_EQ(data, type::AnyData(wrappedData).get<Tag>());

  // type mismatch
  m.reset_to_none().includes_type<>(type::infer_tag<Baz>{});
  applyClear(m.toThrift(), wrappedData);
  EXPECT_EQ(data, type::AnyData(wrappedData).get<Tag>());

  // type match
  m.reset_to_none().includes_type<>(Tag{});
  applyClear(m.toThrift(), wrappedData);
  EXPECT_EQ(op::getDefault<Tag>(), type::AnyData(wrappedData).get<Tag>());
}

TEST(FieldMaskTest, ClearAnyStruct) {
  Foo foo;
  foo.field1() = 123;
  MaskBuilder<Foo> fooMask;
  fooMask.reset_to_none().includes<ident::field1>();

  testClearAnyStruct<type::bool_t>(true);
  testClearAnyStruct<type::i32_t>(123);
  testClearAnyStruct<type::binary_t>("foobar");
  testClearAnyStruct<type::list<type::float_t>>(std::vector<float>{1.0, 3.0});
  testClearAnyStruct<type::set<type::i64_t>>(std::set<int64_t>{1, 2, 3});
  testClearAnyStruct<type::map<type::binary_t, type::i64_t>>(
      std::map<std::string, int64_t>{{"1", 1}, {"3", 3}});
  testClearAnyStruct<type::infer_tag<Foo>>(foo);

  // nested field clear
  StructWithAny s;
  s.rawAny() = type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift();
  MaskBuilder<StructWithAny> m;
  m.reset_to_none().includes_type<ident::rawAny>(
      type::infer_tag<Foo>{}, fooMask.toThrift());
  m.clear(s);
  EXPECT_EQ(
      0, *type::AnyData(*s.rawAny()).get<type::infer_tag<Foo>>().field1());

  // Nested any
  // StructWithAny -> Any -> Any -> Foo
  s.rawAny() = type::AnyData::toAny<type::infer_tag<type::AnyStruct>>(
                   type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift())
                   .toThrift();
  m.reset_to_none().includes_type<ident::rawAny>(
      type::infer_tag<type::AnyStruct>{}, [&]() {
        MaskBuilder<type::AnyStruct> m1;
        m1.reset_to_none().includes_type<>(
            type::infer_tag<Foo>{}, fooMask.toThrift());
        return m1.toThrift();
      }());
  m.clear(s);
  EXPECT_EQ(
      0,
      *type::AnyData(
           type::AnyData(*s.rawAny()).get<type::infer_tag<type::AnyStruct>>())
           .get<type::infer_tag<Foo>>()
           .field1());
}
TEST(FieldMaskTest, ClearAnyStructWithHashedUri) {
  auto fooType = type::Type::get<type::infer_tag<Foo>>();
  convertToHashedURI(fooType);

  Foo foo;
  foo.field1() = 1;
  auto fooAny = type::AnyData::toAny<type::infer_tag<Foo>>(foo).toThrift();
  fooAny.type() = fooType;

  MaskBuilder<type::AnyStruct> m;

  // type mismatch
  m.reset_to_none().includes_type<>(type::struct_t<Bar>{}, allMask());
  applyClear(m.toThrift(), fooAny);
  EXPECT_EQ(getViaIdenticalType<Foo>(fooAny, fooType), foo);

  // type match
  m.reset_to_none().includes_type<>(type::struct_t<Foo>{}, allMask());
  applyClear(m.toThrift(), fooAny);
  EXPECT_EQ(getViaIdenticalType<Foo>(fooAny, fooType), Foo{});
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
TEST(FieldMaskTest, LogicalOpEmpty) {
  Mask a, b;
  EXPECT_THROW(a | b, std::runtime_error);
  EXPECT_THROW(a & b, std::runtime_error);
  EXPECT_THROW(a - b, std::runtime_error);
}

TEST(FieldMaskTest, LogicalOpSimple) {
  // maskA = includes{1: excludes{},
  //                  2: excludes{},
  //                  3: includes{}}
  Mask maskA;
  {
    auto& includes = maskA.includes().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = noneMask();
  }

  // maskB = includes{2: excludes{},
  //                  3: excludes{}}
  Mask maskB;
  {
    auto& includes = maskB.includes().emplace();
    includes[2] = allMask();
    includes[3] = allMask();
  }

  // maskA | maskB == includes{1: excludes{},
  //                           2: excludes{},
  //                           3: excludes{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes{2: excludes{}}
  Mask maskIntersect;
  { maskIntersect.includes().emplace()[2] = allMask(); }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{1: excludes{}}
  Mask maskSubtractAB;
  { maskSubtractAB.includes().emplace()[1] = allMask(); }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes{3: excludes{}}
  Mask maskSubtractBA;
  { maskSubtractBA.includes().emplace()[3] = allMask(); }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);
}

// Similar to previous unit-test, but for map
TEST(FieldMaskTest, LogicalOpSimpleMap) {
  // maskA = includes_map{1: excludes{},
  //                      2: excludes{},
  //                      3: includes{}}
  Mask maskA;
  {
    auto& includes = maskA.includes_map().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = noneMask();
  }

  // maskB = includes_map{2: excludes{},
  //                      3: excludes{}}
  Mask maskB;
  {
    auto& includes = maskB.includes_map().emplace();
    includes[2] = allMask();
    includes[3] = allMask();
  }

  // maskA | maskB == includes_map{1: excludes{},
  //                               2: excludes{},
  //                               3: excludes{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes_map().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    includes[3] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes_map{2: excludes{}}
  Mask maskIntersect;
  { maskIntersect.includes_map().emplace()[2] = allMask(); }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes_map{1: excludes{}}
  Mask maskSubtractAB;
  { maskSubtractAB.includes_map().emplace()[1] = allMask(); }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes_map{3: excludes{}}
  Mask maskSubtractBA;
  { maskSubtractBA.includes_map().emplace()[3] = allMask(); }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpSimpleStringMap) {
  // maskA = includes_string_map{1: excludes_string_map{},
  //                             2: excludes_string_map{},
  //                             3: includes_string_map{}}
  Mask maskA;
  {
    auto& includes = maskA.includes_string_map().emplace();
    includes["1"] = allMask();
    includes["2"] = allMask();
    includes["3"] = noneMask();
  }

  // maskB = includes_string_map{2: excludes_string_map{},
  //                             3: excludes_string_map{}}
  Mask maskB;
  {
    auto& includes = maskB.includes_string_map().emplace();
    includes["2"] = allMask();
    includes["3"] = allMask();
  }

  // maskA | maskB == includes_string_map{1: excludes_string_map{},
  //                                      2: excludes_string_map{},
  //                                      3: excludes_string_map{}}
  Mask maskUnion;
  {
    auto& includes = maskUnion.includes_string_map().emplace();
    includes["1"] = allMask();
    includes["2"] = allMask();
    includes["3"] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes_string_map{2: excludes_string_map{}}
  Mask maskIntersect;
  { maskIntersect.includes_string_map().emplace()["2"] = allMask(); }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes_string_map{1: excludes_string_map{}}
  Mask maskSubtractAB;
  { maskSubtractAB.includes_string_map().emplace()["1"] = allMask(); }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes_string_map{3: excludes_string_map{}}
  Mask maskSubtractBA;
  { maskSubtractBA.includes_string_map().emplace()["3"] = allMask(); }
  EXPECT_EQ(maskB - maskA, maskSubtractBA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpAny) {
  Mask includesFoo;
  includesFoo.includes_type().ensure()[type::infer_tag<Foo>{}] = allMask();

  // maskA = includes_type{Foo: allMask(), Any: includes_type{Foo: allMask()}}
  Mask maskA;
  {
    auto& includes_type = maskA.includes_type().ensure();
    includes_type[type::infer_tag<Foo>{}] = allMask();
    includes_type[type::infer_tag<type::AnyStruct>{}] = includesFoo;
  }

  // maskB = includes_type{Any: excludes_type{}, Baz: allMask()}
  Mask maskB;
  {
    auto& includes_type = maskB.includes_type().ensure();
    includes_type[type::infer_tag<type::AnyStruct>{}] = allTypeMask;
    includes_type[type::infer_tag<Baz>{}] = allMask();
  }

  // includes_type{Foo: allMask(), Any: excludes_type{}, Baz: allMask()}
  Mask maskUnion;
  {
    auto& includes_type = maskUnion.includes_type().ensure();
    includes_type[type::infer_tag<Foo>{}] = allMask();
    includes_type[type::infer_tag<type::AnyStruct>{}] = allTypeMask;
    includes_type[type::infer_tag<Baz>{}] = allMask();
  }
  EXPECT_EQ(maskUnion, maskA | maskB);

  // maskIntersect = includes_type{Any: includes_type{Foo: allMask()}}
  Mask maskIntersect;
  maskIntersect.includes_type().ensure()[type::infer_tag<type::AnyStruct>{}] =
      includesFoo;
  EXPECT_EQ(maskIntersect, maskA & maskB);

  // subtractAB = includes_type{Foo: allMask(), Any: includes_type{}}
  Mask subtractAB;
  {
    auto& includes_type = subtractAB.includes_type().ensure();
    includes_type[type::infer_tag<Foo>{}] = allMask();
    includes_type[type::infer_tag<type::AnyStruct>{}] = []() {
      Mask m;
      m.includes_type().emplace();
      return m;
    }();
  }
  EXPECT_EQ(subtractAB, maskA - maskB);

  // subtractBA = includes_type{Any: excludes_type{Foo: allMask()}, Baz:
  //                            allMask()}
  Mask subtractBA;
  {
    auto& includes_type = subtractBA.includes_type().ensure();
    includes_type[type::infer_tag<type::AnyStruct>{}] = []() {
      Mask excludesFoo;
      excludesFoo.excludes_type().ensure()[type::infer_tag<Foo>{}] = allMask();
      return excludesFoo;
    }();
    includes_type[type::infer_tag<Baz>{}] = allMask();
  }
  EXPECT_EQ(subtractBA, maskB - maskA);

  testLogicalOperations(maskA, maskB);
}

TEST(FieldMaskTest, LogicalOpBothIncludes) {
  // maskA = includes{1: includes{2: excludes{}},
  //                  3: includes{4: excludes{},
  //                              5: excludes{}}}
  Mask maskA;
  {
    auto& includes = maskA.includes().emplace();
    includes[1].includes().emplace()[2] = allMask();
    auto& includes2 = includes[3].includes().emplace();
    includes2[4] = allMask();
    includes2[5] = allMask();
  }

  // maskB = includes{1: excludes{},
  //                  3: includes{5: excludes{},
  //                              6: excludes{}},
  //                  7: excludes{}}
  Mask maskB;
  {
    auto& includes = maskB.includes().emplace();
    includes[1] = allMask();
    auto& includes2 = includes[3].includes().emplace();
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
    auto& includes = maskUnion.includes().emplace();
    includes[1] = allMask();
    auto& includes2 = includes[3].includes().emplace();
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
    auto& includes = maskIntersect.includes().emplace();
    includes[1].includes().emplace()[2] = allMask();
    includes[3].includes().emplace()[5] = allMask();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{3: includes{4: excludes{}}}
  Mask maskSubtractAB;
  {
    maskSubtractAB.includes().emplace()[3].includes().emplace()[4] = allMask();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes{1: excludes{2: excludes{}},
  //                           3: includes{6: excludes{}},
  //                           7: excludes{}}
  Mask maskSubtractBA;
  {
    auto& includes = maskSubtractBA.includes().emplace();
    includes[1].excludes().emplace()[2] = allMask();
    auto& includes2 = includes[3].includes().emplace();
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
    auto& excludes = maskA.excludes().emplace();
    excludes[1].excludes().emplace()[2] = allMask();
    auto& includes = excludes[3].includes().emplace();
    includes[4] = noneMask();
    includes[5] = allMask();
  }

  // maskB = excludes{1: includes{},
  //                  3: includes{5: excludes{},
  //                              6: excludes{}},
  //                  7: excludes{}}
  Mask maskB;
  {
    auto& excludes = maskB.excludes().emplace();
    excludes[1] = noneMask();
    auto& includes = excludes[3].includes().emplace();
    includes[5] = allMask();
    includes[6] = allMask();
    excludes[7] = allMask();
  }

  // maskA | maskB == excludes{3: includes{5: excludes{}}}
  Mask maskUnion;
  { maskUnion.excludes().emplace()[3].includes().emplace()[5] = allMask(); }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == excludes{1: excludes{2: excludes{}},
  //                           3: includes{5: excludes{},
  //                                       6: excludes{}},
  //                           7: excludes{}}
  Mask maskIntersect;
  {
    auto& excludes = maskIntersect.excludes().emplace();
    excludes[1].excludes().emplace()[2] = allMask();
    auto& includes = excludes[3].includes().emplace();
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
    auto& includes = maskSubtractAB.includes().emplace();
    includes[3].includes().emplace()[6] = allMask();
    includes[7] = allMask();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == includes{1: excludes{2: excludes{}}}
  Mask maskSubtractBA;
  {
    maskSubtractBA.includes().emplace()[1].excludes().emplace()[2] = allMask();
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
    auto& includes = maskA.includes().emplace();
    includes[1].includes().emplace()[2] = allMask();
    auto& includes2 = includes[3].includes().emplace();
    includes2[4] = allMask();
    includes2[5] = allMask();
  }

  // maskB = excludes{1: includes{},
  //                  3: includes{5: excludes{},
  //                              6: excludes{}},
  //                  7: excludes{}}
  Mask maskB;
  {
    auto& excludes = maskB.excludes().emplace();
    excludes[1] = noneMask();
    auto& includes = excludes[3].includes().emplace();
    includes[5] = allMask();
    includes[6] = allMask();
    excludes[7] = allMask();
  }

  // maskA | maskB == excludes{3: includes{6: excludes{}},
  //                           7: excludes{}}
  Mask maskUnion;
  {
    auto& excludes = maskUnion.excludes().emplace();
    excludes[3].includes().emplace()[6] = allMask();
    excludes[7] = allMask();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB == includes{1: includes{2: excludes{}},
  //                           3: includes{4: excludes{}}}
  Mask maskIntersect;
  {
    auto& includes = maskIntersect.includes().emplace();
    includes[1].includes().emplace()[2] = allMask();
    includes[3].includes().emplace()[4] = allMask();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{3: includes{5: excludes{}}}
  Mask maskSubtractAB;
  {
    maskSubtractAB.includes().emplace()[3].includes().emplace()[5] = allMask();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA == excludes{1: includes{2: excludes{}},
  //                           3: includes{4: excludes{},
  //                                       5: excludes{},
  //                                       6: excludes{}},
  //                           7: excludes{}}
  Mask maskSubtractBA;
  {
    auto& excludes = maskSubtractBA.excludes().emplace();
    excludes[1].includes().emplace()[2] = allMask();
    auto& includes = excludes[3].includes().emplace();
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
    auto& includes = maskA.includes().emplace();
    auto add = [](auto& m) {
      auto& includes_string_map = m[1].includes_string_map_ref().emplace();
      includes_string_map["1"].includes_ref().emplace();
      includes_string_map["2"].excludes_ref().emplace();
    };
    add(includes[1].includes_map().emplace());
  }

  // maskB = excludes{1: excludes_map{1: excludes_string_map{"1": excludes{},
  //                                                         "2": includes{}},
  //                                  2: includes_string_map{"1": excludes{}}
  //                                                         "2": includes{}}}}
  Mask maskB;
  {
    auto& excludes = maskB.excludes().emplace();
    auto add = [](auto& m) {
      auto& excludes_string_map = m[1].excludes_string_map_ref().emplace();
      excludes_string_map["1"].excludes_ref().emplace();
      excludes_string_map["2"].includes_ref().emplace();
    };
    add(excludes[1].excludes_map().emplace());
  }

  // maskA | maskB ==
  //     excludes{1: excludes_map{1: excludes_string_map{"1": excludes{}}}}
  Mask maskUnion;
  {
    auto& excludes = maskUnion.excludes().emplace();
    excludes[1]
        .excludes_map()
        .emplace()[1]
        .excludes_string_map()
        .emplace()["1"]
        .excludes()
        .emplace();
  }
  EXPECT_EQ(maskA | maskB, maskUnion);
  EXPECT_EQ(maskB | maskA, maskUnion);

  // maskA & maskB ==
  //     includes{1: includes_map{1: includes_string_map{"2": excludes{}}}}
  Mask maskIntersect;
  {
    auto& includes = maskIntersect.includes().emplace();
    includes[1]
        .includes_map()
        .emplace()[1]
        .includes_string_map()
        .emplace()["2"]
        .excludes()
        .emplace();
  }
  EXPECT_EQ(maskA & maskB, maskIntersect);
  EXPECT_EQ(maskB & maskA, maskIntersect);

  // maskA - maskB == includes{1: includes_map{1: includes_string_map{}}}
  Mask maskSubtractAB;
  {
    auto& includes = maskSubtractAB.includes().emplace();
    includes[1].includes_map().emplace()[1].includes_string_map().emplace();
  }
  EXPECT_EQ(maskA - maskB, maskSubtractAB);

  // maskB - maskA ==
  //     excludes{1: excludes_map{1: excludes_string_map{"1": excludes{},
  //                                                     "2": excludes{}}}
  Mask maskSubtractBA;
  {
    auto& excludes = maskSubtractBA.excludes().emplace();
    auto& string_map =
        excludes[1].excludes_map().emplace()[1].excludes_string_map().emplace();
    string_map["1"].excludes().emplace();
    string_map["2"].excludes().emplace();
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

template <typename T>
struct MaskBuilderTester {
  explicit MaskBuilderTester(const Mask& m)
      : builder_by_ident(m), builder_by_name(m), builder_by_field_id{m} {}

  MaskBuilderTester& reset_to_none() {
    builder_by_ident.reset_to_none();
    builder_by_name.reset_to_none();
    builder_by_field_id.reset_to_none();
    return *this;
  }

  MaskBuilderTester& reset_to_all() {
    builder_by_ident.reset_to_all();
    builder_by_name.reset_to_all();
    builder_by_field_id.reset_to_all();
    return *this;
  }

  MaskBuilderTester& invert() {
    builder_by_ident.invert();
    builder_by_name.invert();
    builder_by_field_id.invert();
    return *this;
  }

  template <typename... Id>
  MaskBuilderTester& includes(const Mask& mask = allMask()) {
    builder_by_ident.template includes<Id...>(mask);
    builder_by_name.includes(identToFieldName<T, Id...>(), mask);
    builder_by_field_id.includes(identToFieldPath<T, Id...>(), mask);
    return *this;
  }

  template <typename... Id>
  MaskBuilderTester& includes_map_element(
      int64_t key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_map().emplace()[key] = mask;
    return includes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilderTester& includes_map_element(
      std::string key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_string_map().emplace().emplace(std::move(key), mask);
    return includes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilderTester& includes_type(
      type::Type type, const Mask& mask = allMask()) {
    Mask typeMap;
    typeMap.includes_type().emplace().emplace(std::move(type), mask);
    return includes<Id...>(typeMap);
  }
  template <typename... Id>
  MaskBuilderTester& excludes(const Mask& mask = allMask()) {
    builder_by_ident.template excludes<Id...>(mask);
    builder_by_name.excludes(identToFieldName<T, Id...>(), mask);
    builder_by_field_id.excludes(identToFieldPath<T, Id...>(), mask);
    return *this;
  }

  template <typename... Id>
  MaskBuilderTester& excludes_map_element(
      int64_t key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_map().emplace()[key] = mask;
    return excludes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilderTester& excludes_map_element(
      std::string key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_string_map().emplace().emplace(std::move(key), mask);
    return excludes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilderTester& excludes_type(
      type::Type type, const Mask& mask = allMask()) {
    Mask typeMap;
    typeMap.includes_type().emplace().emplace(std::move(type), mask);
    return excludes<Id...>(typeMap);
  }

  void check(const Mask& expected) {
    EXPECT_EQ(builder_by_ident.toThrift(), expected);
    EXPECT_EQ(builder_by_name.toThrift(), expected);
    EXPECT_EQ(builder_by_field_id.toThrift(), expected);
  }

 public:
  MaskBuilder<T> builder_by_ident;
  MaskBuilder<T> builder_by_name;
  DynamicMaskBuilder builder_by_field_id;

 private:
  template <typename S>
  void identToFieldNameImpl(std::vector<folly::StringPiece>&) {}
  template <typename S, typename Id, typename... Ids>
  void identToFieldNameImpl(std::vector<folly::StringPiece>& v) {
    v.push_back(op::get_name_v<S, Id>);
    identToFieldNameImpl<op::get_native_type<S, Id>, Ids...>(v);
  }
  template <typename S, typename... Ids>
  std::vector<folly::StringPiece> identToFieldName() {
    std::vector<folly::StringPiece> v;
    v.reserve(sizeof...(Ids));
    identToFieldNameImpl<S, Ids...>(v);
    return v;
  }

  template <typename S>
  void identToFieldPathImpl(FieldPath&) {}
  template <typename S, typename Id, typename... Ids>
  void identToFieldPathImpl(FieldPath& v) {
    v.push_back(op::get_field_id_v<S, Id>);
    identToFieldPathImpl<op::get_native_type<S, Id>, Ids...>(v);
  }
  template <typename S, typename... Ids>
  FieldPath identToFieldPath() {
    FieldPath v;
    v.reserve(sizeof...(Ids));
    identToFieldPathImpl<S, Ids...>(v);
    return v;
  }
};

TEST(FieldMaskTest, MaskBuilderSimple) {
  // Test includes
  {
    // includes{1: excludes{},
    //          3: excludes{}}
    MaskBuilderTester<Foo> builder(noneMask());
    builder.includes<ident::field1>();
    builder.includes<ident::field2>();

    Mask expected;
    auto& includes = expected.includes().emplace();
    includes[1] = allMask();
    includes[3] = allMask();
    builder.check(expected);
  }
  {
    MaskBuilderTester<Foo> builder(allMask());
    builder.includes<ident::field1>();
    builder.includes<ident::field2>();

    builder.check(allMask());
  }
  {
    // includes{1: excludes{}}
    MaskBuilderTester<Foo> builder(noneMask());
    // including twice is fine
    builder.includes<ident::field1>();
    builder.includes<ident::field1>();

    Mask expected;
    expected.includes().emplace()[1] = allMask();
    builder.check(expected);
  }

  // Test excludes
  {
    // excludes{3: excludes{}}
    MaskBuilderTester<Foo> builder(allMask());
    builder.excludes<ident::field1>(noneMask());
    builder.excludes<ident::field2>();

    Mask expected;
    expected.excludes().emplace()[3] = allMask();
    builder.check(expected);
  }
  {
    MaskBuilderTester<Foo> builder(noneMask());
    builder.excludes<ident::field1>();
    builder.excludes<ident::field2>();

    builder.check(noneMask());
  }
  {
    // excludes{3: excludes{}}
    MaskBuilderTester<Foo> builder(allMask());
    // excluding twice is fine
    builder.excludes<ident::field2>();
    builder.excludes<ident::field2>();

    Mask expected;
    expected.excludes().emplace()[3] = allMask();
    builder.check(expected);
  }

  // Test includes and excludes
  {
    MaskBuilderTester<Foo> builder(noneMask());
    // excluding the field we included
    builder.includes<ident::field2>();
    builder.excludes<ident::field2>();

    builder.check(noneMask());
  }
  {
    MaskBuilderTester<Foo> builder(allMask());
    // including the field we excluded
    builder.excludes<ident::field2>();
    builder.includes<ident::field2>();

    builder.check(allMask());
  }
}

TEST(FieldMaskTest, MaskBuilderNested) {
  // Test includes
  {
    // includes{1: includes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(noneMask());
    builder.includes<ident::field_3, ident::field_1>();

    Mask expected;
    expected.includes().emplace()[1].includes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // includes{1: includes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(noneMask());
    // including twice is fine
    builder.includes<ident::field_3, ident::field_1>();
    builder.includes<ident::field_3, ident::field_1>();

    Mask expected;
    expected.includes().emplace()[1].includes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // includes{1: includes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(noneMask());
    Mask nestedMask;
    nestedMask.includes().emplace()[1] = allMask();
    builder.includes<ident::field_3>(nestedMask);

    Mask expected;
    expected.includes().emplace()[1].includes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // includes{1: includes{1: excludes{},
    //                      2: excludes{}},
    //          2: excludes{}}
    MaskBuilderTester<Bar2> builder(noneMask());
    builder.includes<ident::field_4>();
    builder.includes<ident::field_3, ident::field_1>();
    builder.includes<ident::field_3, ident::field_2>();

    Mask expected;
    auto& includes = expected.includes().emplace();
    includes[2] = allMask();
    auto& includes2 = includes[1].includes().emplace();
    includes2[1] = allMask();
    includes2[2] = allMask();
    builder.check(expected);
  }
  {
    // includes{1: excludes{}}
    MaskBuilderTester<Bar2> builder(noneMask());
    builder.includes<ident::field_3, ident::field_1>();
    builder.includes<ident::field_3>();

    Mask expected;
    expected.includes().emplace()[1] = allMask();
    builder.check(expected);
  }

  // Test excludes
  {
    // excludes{1: includes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(allMask());
    builder.excludes<ident::field_3, ident::field_1>();

    Mask expected;
    expected.excludes().emplace()[1].includes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // excludes{1: includes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(allMask());
    // excluding twice is fine
    builder.excludes<ident::field_3, ident::field_1>();
    builder.excludes<ident::field_3, ident::field_1>();

    Mask expected;
    expected.excludes().emplace()[1].includes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // excludes{1: includes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(allMask());
    Mask nestedMask;
    nestedMask.includes().emplace()[1] = allMask();
    builder.excludes<ident::field_3>(nestedMask);

    Mask expected;
    expected.excludes().emplace()[1].includes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // excludes{1: includes{1: excludes{},
    //                      2: excludes{}},
    //          2: excludes{}}
    MaskBuilderTester<Bar2> builder(allMask());
    builder.excludes<ident::field_4>();
    builder.excludes<ident::field_3, ident::field_1>();
    builder.excludes<ident::field_3, ident::field_2>();

    Mask expected;
    auto& excludes = expected.excludes().emplace();
    excludes[2] = allMask();
    auto& includes = excludes[1].includes().emplace();
    includes[1] = allMask();
    includes[2] = allMask();
    builder.check(expected);
  }
  {
    // excludes{1: excludes{}}
    MaskBuilderTester<Bar2> builder(allMask());
    builder.excludes<ident::field_3, ident::field_1>();
    builder.excludes<ident::field_3>();

    Mask expected;
    expected.excludes().emplace()[1] = allMask();
    builder.check(expected);
  }

  // Test includes and excludes
  {
    // includes{1: excludes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(noneMask());
    builder.includes<ident::field_3>();
    builder.excludes<ident::field_3, ident::field_1>();

    Mask expected;
    expected.includes().emplace()[1].excludes().emplace()[1] = allMask();
    builder.check(expected);
  }
  {
    // excludes{1: excludes{1: excludes{}}}
    MaskBuilderTester<Bar2> builder(allMask());
    builder.excludes<ident::field_3>();
    builder.includes<ident::field_3, ident::field_1>();

    Mask expected;
    expected.excludes().emplace()[1].excludes().emplace()[1] = allMask();
    builder.check(expected);
  }
}

TEST(FieldMaskTest, MaskBuilderUnion) {
  {
    // simple
    Mask expected;
    expected.includes().emplace()[1] = allMask();

    MaskBuilderTester<RecursiveUnion> builder(noneMask());
    builder.includes<ident::foo>();
    builder.check(expected);
  }
  {
    // nested
    Mask nested;
    nested.excludes().emplace()[11] = allMask();

    Mask expected;
    expected.includes().emplace()[1] = nested;

    MaskBuilderTester<RecursiveUnion> builder(noneMask());
    builder.includes<ident::foo>(nested);
    builder.check(expected);
  }
  {
    // recurse
    Mask expected;
    expected.includes().emplace()[4].includes().emplace()[1] = allMask();

    MaskBuilderTester<RecursiveUnion> builder(noneMask());
    builder.includes<ident::recurse, ident::foo>();
    builder.check(expected);
  }
}

TEST(FieldMaskTest, MaskBuilderAny) {
  MaskBuilder<Foo> fooMask(noneMask());
  fooMask.includes<ident::field1>();

  Mask simpleExpected;
  simpleExpected.includes()
      .emplace()[1]
      .includes_type()
      .emplace()[type::infer_tag<Foo>{}] = fooMask.toThrift();

  Mask nestedExpected;
  nestedExpected.includes()
      .emplace()[1]
      .includes_type()
      .emplace()[type::infer_tag<StructWithAny>{}] = simpleExpected;

  MaskBuilderTester<StructWithAny> b(noneMask());
  b.includes_type<ident::rawAny>(type::infer_tag<Foo>{}, fooMask.toThrift());
  b.check(simpleExpected);

  b.reset_to_none().includes_type<ident::rawAny>(
      type::infer_tag<StructWithAny>{}, simpleExpected);
  b.check(nestedExpected);

  b.reset_to_none().includes_type<ident::rawAny>(
      type::Type::get<type::infer_tag<Foo>>(), fooMask.toThrift());
  b.check(simpleExpected);

  b.reset_to_none().includes_type<ident::rawAny>(
      type::Type::get<type::infer_tag<StructWithAny>>(), simpleExpected);
  b.check(nestedExpected);

  b.reset_to_all().excludes_type<ident::rawAny>(
      type::infer_tag<Foo>{}, fooMask.toThrift());
  b.invert().check(simpleExpected);

  b.reset_to_all().excludes_type<ident::rawAny>(
      type::infer_tag<StructWithAny>{}, simpleExpected);
  b.invert().check(nestedExpected);

  b.reset_to_all().excludes_type<ident::rawAny>(
      type::Type::get<type::infer_tag<Foo>>(), fooMask.toThrift());
  b.invert().check(simpleExpected);

  b.reset_to_all().excludes_type<ident::rawAny>(
      type::Type::get<type::infer_tag<StructWithAny>>(), simpleExpected);
  b.invert().check(nestedExpected);
}

TEST(FieldMaskTest, MaskBuilderAdaptedAny) {
  MaskBuilder<Foo> fooMask(noneMask());
  fooMask.includes<ident::field1>();

  Mask simpleExpected;
  simpleExpected.includes()
      .emplace()[2]
      .includes_type()
      .emplace()[type::infer_tag<Foo>{}] = fooMask.toThrift();

  Mask nestedExpected;
  nestedExpected.includes()
      .emplace()[2]
      .includes_type()
      .emplace()[type::infer_tag<StructWithAny>{}] = simpleExpected;

  MaskBuilderTester<StructWithAny> b(noneMask());
  b.includes_type<ident::adaptedAny>(
      type::infer_tag<Foo>{}, fooMask.toThrift());
  b.check(simpleExpected);

  b.reset_to_none().includes_type<ident::adaptedAny>(
      type::infer_tag<StructWithAny>{}, simpleExpected);
  b.check(nestedExpected);
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
  MaskBuilderTester<HasMap> builder(noneMask());
  Mask expected;

  builder.includes_map_element<ident::foos>(1);
  expected.includes().ensure()[1].includes_map().ensure()[1] = allMask();
  builder.check(expected);

  builder.includes<ident::foo>();
  expected.includes().ensure()[2] = allMask();
  builder.check(expected);
  ;

  builder.includes_map_element<ident::foos>(2);
  expected.includes().ensure()[1].includes_map().ensure()[2] = allMask();
  builder.check(expected);

  builder.excludes_map_element<ident::foos>(2);
  expected.includes().ensure()[1].includes_map().ensure().erase(2);
  builder.check(expected);

  builder.excludes_map_element<ident::foos>(1);
  expected.includes().ensure()[1].includes_map().ensure().erase(1);
  builder.check(expected);
}

TEST(FieldMaskTest, MaskBuilderStringMap) {
  MaskBuilderTester<HasMap> builder(noneMask());
  Mask expected;

  builder.includes_map_element<ident::string_map>("1");
  expected.includes().ensure()[3].includes_string_map().ensure()["1"] =
      allMask();
  builder.check(expected);

  builder.includes_map_element<ident::string_map>("2");
  expected.includes().ensure()[3].includes_string_map().ensure()["2"] =
      allMask();
  builder.check(expected);

  builder.excludes_map_element<ident::string_map>("2");
  expected.includes().ensure()[3].includes_string_map().ensure().erase("2");
  builder.check(expected);

  builder.excludes_map_element<ident::string_map>("1");
  expected.includes().ensure()[3].includes_string_map().ensure().erase("1");
  builder.check(expected);
}

TEST(FieldMaskTest, ReverseMask) {
  EXPECT_EQ(reverseMask(allMask()), noneMask());
  EXPECT_EQ(reverseMask(noneMask()), allMask());
  // inclusiveMask and exclusiveMask are reverse of each other.
  Mask inclusiveMask;
  auto& includes = inclusiveMask.includes().emplace();
  includes[1] = allMask();
  includes[2].includes().emplace()[3] = allMask();
  Mask exclusiveMask;
  auto& excludes = exclusiveMask.excludes().emplace();
  excludes[1] = allMask();
  excludes[2].includes().emplace()[3] = allMask();

  EXPECT_EQ(reverseMask(inclusiveMask), exclusiveMask);
  EXPECT_EQ(reverseMask(exclusiveMask), inclusiveMask);

  Mask empty;
  EXPECT_THROW(reverseMask(empty), std::runtime_error);
}

TEST(FieldMaskTest, ReverseMapMask) {
  Mask inclusiveMask;
  auto& includes = inclusiveMask.includes_map().emplace();
  includes[1] = allMask();
  includes[2].includes().emplace()[3] = allMask();
  Mask exclusiveMask;
  auto& excludes = exclusiveMask.excludes_map().emplace();
  excludes[1] = allMask();
  excludes[2].includes().emplace()[3] = allMask();

  EXPECT_EQ(reverseMask(inclusiveMask), exclusiveMask);
  EXPECT_EQ(reverseMask(exclusiveMask), inclusiveMask);
}

TEST(FieldMaskTest, ReverseStringMapMask) {
  Mask inclusiveMask;
  auto& includes = inclusiveMask.includes_string_map().emplace();
  includes["1"] = allMask();
  includes["2"].includes().emplace()[3] = allMask();
  Mask exclusiveMask;
  auto& excludes = exclusiveMask.excludes_string_map().emplace();
  excludes["1"] = allMask();
  excludes["2"].includes().emplace()[3] = allMask();

  EXPECT_EQ(reverseMask(inclusiveMask), exclusiveMask);
  EXPECT_EQ(reverseMask(exclusiveMask), inclusiveMask);
}

TEST(FieldMaskTest, ReverseTypeMask) {
  EXPECT_EQ(reverseMask(noTypeMask), allTypeMask);
  EXPECT_EQ(noTypeMask, reverseMask(allTypeMask));

  // m = includes_type{Foo: allMap(), Any: includes_type{Foo: allMap()}}
  Mask inclusiveMask;
  {
    auto& includes_type = inclusiveMask.includes_type().ensure();
    includes_type[type::infer_tag<Foo>{}] = allMask();
    includes_type[type::infer_tag<type::Any>{}] = allMask();
  }

  // m = excludes_type{Foo: allMap(), Any: includes_type{Foo: allMap()},
  Mask exclusiveMask;
  {
    auto& excludes_type = exclusiveMask.excludes_type().ensure();
    excludes_type[type::infer_tag<Foo>{}] = allMask();
    excludes_type[type::infer_tag<type::Any>{}] = allMask();
  }

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
    expected.includes().emplace()[2] = allMask();
    expected.includes().value()[1].includes().emplace()[1] = allMask();
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
    EXPECT_EQ(mask.includes().value()[1], allMask());
    EXPECT_EQ(mask.includes().value()[2], allMask());
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
    EXPECT_EQ(mask.includes().value()[1], allMask());
    EXPECT_EQ(mask.includes().value()[2], allMask());
    EXPECT_EQ(mask.includes().value()[3], allMask());
    EXPECT_EQ(protocol::compare(empty, src), mask); // commutative
  }
  {
    SmartPointerStruct src, dst;
    protocol::ensure(allMask(), src);
    protocol::ensure(allMask(), dst);
    // Compare should create a mask for nested fields.
    src.unique()->field_1() = 1;
    dst.unique()->field_1() = 10;
    dst.shared() = nullptr;
    // mask = {1: includes{1: allMask()},
    //         2: allMask()}
    Mask mask = protocol::compare(src, dst);
    Mask expected;
    auto& includes = expected.includes().emplace();
    includes[1].includes().emplace()[1] = allMask();
    includes[2] = allMask();
    EXPECT_EQ(mask, expected);
    EXPECT_EQ(protocol::compare(dst, src), mask); // commutative
  }
}

TEST(FieldMaskTest, UnionCompare) {
  {
    // compare with self
    RecursiveUnion src;
    EXPECT_EQ(noneMask(), protocol::compare(src, src));

    src.foo().emplace();
    src.foo()->field1() = 1;
    EXPECT_EQ(noneMask(), protocol::compare(src, src));
  }
  {
    // single field mismatch
    RecursiveUnion src;
    src.foo().emplace();
    src.foo()->field1() = 1;

    RecursiveUnion dst(src);
    dst.foo()->field1() = 2;

    MaskBuilder<RecursiveUnion> m(noneMask());
    m.includes<ident::foo, ident::field1>();
    EXPECT_EQ(m.toThrift(), protocol::compare(src, dst));
  }
  {
    // different fields set
    RecursiveUnion src;
    src.foo().emplace();

    RecursiveUnion dst;
    dst.bar().emplace();

    MaskBuilder<RecursiveUnion> m(noneMask());
    m.includes<ident::foo>();
    m.includes<ident::bar>();
    EXPECT_EQ(m.toThrift(), protocol::compare(src, dst));
  }
  {
    // nested field mismatch
    RecursiveUnion src;
    auto& nestedSrc = src.recurse().emplace();
    RecursiveUnion dst;
    auto& nestedDst = dst.recurse().emplace();

    nestedSrc.foo().emplace();
    nestedSrc.foo()->field1() = 1;
    EXPECT_EQ(noneMask(), protocol::compare(src, src));

    nestedDst.foo().emplace();
    nestedDst.foo()->field2() = 2;

    MaskBuilder<RecursiveUnion> m(noneMask());
    m.includes<ident::recurse, ident::foo, ident::field1>();
    m.includes<ident::recurse, ident::foo, ident::field2>();
    EXPECT_EQ(m.toThrift(), protocol::compare(src, dst));
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
    MaskBuilderTester<Bar2> builder(allMask());
    builder.reset_to_none();
    builder.check(noneMask());
    builder.includes<ident::field_3>().reset_to_none();
    builder.check(noneMask());
  }

  // reset to all
  {
    MaskBuilderTester<Bar2> builder(noneMask());
    builder.reset_to_all();
    builder.check(allMask());
    builder.includes<ident::field_3>().reset_to_all();
    builder.check(allMask());
  }
}

TEST(FieldMaskTest, MaskBuilderMaskAPIs) {
  Bar2 src, dst;

  MaskBuilder<Bar2> builder(noneMask());
  builder.includes({"field_3", "field_1"}).includes({"field_4"});
  // test invert
  Mask expectedMask = reverseMask(builder.toThrift());
  EXPECT_EQ(builder.invert().toThrift(), expectedMask);
  // Mask includes bar.field_3().field_2().
  // test ensure, clear, and filter
  Bar2 expected, cleared;
  expected.field_3().emplace().field_2() = 0;
  cleared.field_3().emplace();
  builder.ensure(src);
  EXPECT_EQ(src, expected);
  dst = builder.filter(src);
  EXPECT_EQ(dst, expected);
  builder.clear(src);
  EXPECT_EQ(src, cleared);
}

TEST(FieldMaskTest, MaskBuilderMaskNotCompatible) {
  // Mask is not compatible with both Bar and Foo.
  Mask mask;
  mask.includes().emplace()[5] = allMask();
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

TEST(FieldMaskTest, NestedMap) {
  MaskBuilder<Foo> fooMaskBuilder(noneMask());
  fooMaskBuilder.includes_map_element<ident::field3>("1");

  MaskBuilder<HasMap> hasMapMaskBuilder(noneMask());
  hasMapMaskBuilder.includes_map_element<ident::foos>(
      1, fooMaskBuilder.toThrift());
  Mask mask = hasMapMaskBuilder.toThrift();
  EXPECT_EQ(
      mask.includes()
          .value()[1]
          .includes_map()
          .value()[1]
          .includes()
          .value()[11]
          .includes_string_map()
          .value()["1"],
      allMask());
}

TEST(FieldMaskTest, testDuplicateEntryInTypeMaskList) {
  std::vector<protocol::TypeAndMaskEntry> entries;
  entries.emplace_back().type() = type::infer_tag<Foo>{};
  entries.emplace_back().type() = type::infer_tag<Foo>{};

  EXPECT_THROW(
      (protocol::detail::TypeToMaskAdapter<protocol::TypeAndMaskEntry, Mask>::
           fromThrift(std::move(entries))),
      std::runtime_error);
}
TEST(FieldMaskTest, testIncompleteType) {
  Mask m;

  type::Type type;

  // Scoped name
  type.toThrift().name()->structType().ensure().scopedName() = "foo.Bar";
  EXPECT_THROW(m.includes_type().ensure()[type], std::runtime_error);

  // typeHash
  type.toThrift().name()->structType().ensure().typeHashPrefixSha2_256() =
      "abcde123";
  EXPECT_THROW(
      m.excludes_type().ensure().emplace(type, Mask{}), std::runtime_error);

  type.toThrift().name()->unionType().ensure().definitionKey() = "abcde123";
  EXPECT_THROW(
      m.excludes_type().ensure().emplace(type, Mask{}), std::runtime_error);
}

TEST(FieldMaskTest, FieldMaskLogicalOperatorAllMask) {
  auto checkMask = [](const Mask& mask) {
    EXPECT_EQ(mask | allMask(), allMask());
    EXPECT_EQ(allMask() | mask, allMask());
    EXPECT_EQ(mask & allMask(), mask);
    EXPECT_EQ(allMask() & mask, mask);
    EXPECT_EQ(mask - allMask(), noneMask());
    EXPECT_EQ(allMask() - mask, reverseMask(mask));
  };
  {
    // Field Mask
    Mask mask;
    mask.includes().ensure()[1] = allMask();
    checkMask(mask);
    checkMask(reverseMask(mask));
  }
  {
    // Map Mask
    Mask mask;
    mask.includes_map().ensure()[1] = allMask();
    checkMask(mask);
    checkMask(reverseMask(mask));
  }
  {
    // Type Mask
    Mask mask;
    mask.includes_type().ensure()[type::infer_tag<Foo>{}] = allMask();
    checkMask(mask);
    checkMask(reverseMask(mask));
  }
}

TEST(FieldMaskTest, FieldMaskLogicalOperatorNoneMask) {
  auto checkMask = [](const Mask& mask) {
    EXPECT_EQ(mask | noneMask(), mask);
    EXPECT_EQ(noneMask() | mask, mask);
    EXPECT_EQ(mask & noneMask(), noneMask());
    EXPECT_EQ(noneMask() & mask, noneMask());
    EXPECT_EQ(noneMask() - mask, noneMask());
    EXPECT_EQ(mask - noneMask(), mask);
  };
  {
    // Field Mask
    Mask mask;
    mask.includes().ensure()[1] = allMask();
    checkMask(mask);
    checkMask(reverseMask(mask));
  }
  {
    // Map Mask
    Mask mask;
    mask.includes_map().ensure()[1] = allMask();
    checkMask(mask);
    checkMask(reverseMask(mask));
  }
  {
    // Type Mask
    Mask mask;
    mask.includes_type().ensure()[type::infer_tag<Foo>{}] = allMask();
    checkMask(mask);
    checkMask(reverseMask(mask));
  }
}

TEST(FieldMaskTest, ValidateSinglePathInvalid) {
  {
    // empty
    Mask mask;
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // noneMask
    Mask mask = noneMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive map mask
    Mask mask;
    mask.includes_map().ensure();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // exclusive map mask
    Mask mask;
    mask.excludes_map().ensure();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive string map mask
    Mask mask;
    mask.includes_string_map().ensure();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // exclusive string map mask
    Mask mask;
    mask.excludes_string_map().ensure();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive type mask
    Mask mask;
    mask.includes_type().ensure();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // exclusive type mask
    Mask mask;
    mask.excludes_type().ensure();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
}

TEST(FieldMaskTest, ValidateSinglePathInvalidNested) {
  auto type = type::Type::get<type::infer_tag<Foo>>();
  {
    // exclusive field mask
    Mask mask;
    mask.excludes().ensure()[1] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // exclusive map mask
    Mask mask;
    mask.excludes_map().ensure()[1] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // exclusive string map mask
    Mask mask;
    mask.excludes_string_map().ensure()["1"] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // exclusive type mask
    Mask mask;
    mask.excludes_type().ensure()[type] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive field mask
    Mask mask;
    mask.includes().ensure()[1].excludes().ensure()[1] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive map mask
    Mask mask;
    mask.includes_map().ensure()[1].excludes().ensure()[1] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive string map mask
    Mask mask;
    mask.includes_string_map().ensure()["1"].excludes().ensure()[1] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
  {
    // inclusive type mask
    Mask mask;
    mask.includes_type().ensure()[type].excludes().ensure()[1] = allMask();
    EXPECT_THROW(validateSinglePath(mask), std::runtime_error);
  }
}

TEST(FieldMaskTest, ValidateSinglePathValid) {
  auto type = type::Type::get<type::infer_tag<Foo>>();
  {
    // allMask
    Mask mask = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive field mask
    Mask mask;
    mask.includes().ensure()[1] = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive map mask
    Mask mask;
    mask.includes_map().ensure()[1] = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive string map mask
    Mask mask;
    mask.includes_string_map().ensure()["1"] = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive type mask
    Mask mask;
    mask.includes_type().ensure()[type] = allMask();
    validateSinglePath(mask);
  }
}

TEST(FieldMaskTest, ValidateSinglePathValidNested) {
  auto type = type::Type::get<type::infer_tag<Foo>>();
  {
    // inclusive field mask
    Mask mask;
    mask.includes().ensure()[1].includes().ensure()[2] = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive map mask
    Mask mask;
    mask.includes_map().ensure()[1].includes().ensure()[2] = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive string map mask
    Mask mask;
    mask.includes_string_map().ensure()["1"].includes().ensure()[2] = allMask();
    validateSinglePath(mask);
  }
  {
    // inclusive type mask
    Mask mask;
    mask.includes_type().ensure()[type].includes().ensure()[2] = allMask();
    validateSinglePath(mask);
  }
}

TEST(FieldMaskTest, getValueAsMapId) {
  auto byteVal = protocol::asValueStruct<type::byte_t>(1);
  auto i16Val = protocol::asValueStruct<type::i16_t>(1);
  auto i32Val = protocol::asValueStruct<type::i32_t>(1);
  auto i64Val = protocol::asValueStruct<type::i64_t>(1);
  auto stringVal = protocol::asValueStruct<type::string_t>("1");

  // Valid
  for (const auto& as : {byteVal, i16Val, i32Val, i64Val}) {
    auto v = getValueAs(MapId{1}, as);
    EXPECT_EQ(v, as);
  }

  // Invalid `as`
  EXPECT_THROW(getValueAs(MapId{1}, stringVal), std::runtime_error);

  // Overflow
  for (const auto& as : {byteVal, i16Val, i32Val}) {
    EXPECT_THROW(
        getValueAs(
            MapId{
                static_cast<int64_t>(std::numeric_limits<std::int32_t>::max()) +
                1},
            as),
        std::runtime_error);
  }
}

TEST(FieldMaskTest, getValueAsString) {
  auto byteVal = protocol::asValueStruct<type::byte_t>(1);
  auto stringVal = protocol::asValueStruct<type::string_t>("1");
  auto binaryVal = protocol::asValueStruct<type::binary_t>("1");

  // Valid
  for (const auto& as : {stringVal, binaryVal}) {
    auto v = getValueAs("1", as);
    EXPECT_EQ(v, as);
  }

  // Invalid `as`
  EXPECT_THROW(getValueAs("1", byteVal), std::runtime_error);
}

// TODO(dokwon): Consider providing isEmpty method in MaskRef.
// Currently, empty mask does not have any semantic meaning.
TEST(FieldMaskTest, emptyMask) {
  Mask mask;
  MaskRef maskRef{mask, false};
  EXPECT_FALSE(maskRef.isAllMask());
  EXPECT_FALSE(maskRef.isNoneMask());
  EXPECT_THROW(maskRef.isAllMapMask(), std::runtime_error);
  EXPECT_THROW(maskRef.isNoneMapMask(), std::runtime_error);
  EXPECT_THROW(maskRef.isAllTypeMask(), std::runtime_error);
  EXPECT_THROW(maskRef.isNoneTypeMask(), std::runtime_error);
  EXPECT_FALSE(maskRef.isExclusive());
  EXPECT_FALSE(maskRef.isFieldMask());
  EXPECT_FALSE(maskRef.isIntegerMapMask());
  EXPECT_FALSE(maskRef.isStringMapMask());
  EXPECT_FALSE(maskRef.isTypeMask());

  // get fails with empty mask
  EXPECT_THROW(maskRef.get(FieldId{42}), std::runtime_error);
  EXPECT_THROW(maskRef.get(MapId{42}), std::runtime_error);
  EXPECT_THROW(maskRef.get("42"sv), std::runtime_error);
  EXPECT_THROW(maskRef.get(type::Type::get<type::i32_t>()), std::runtime_error);
}

// Demonstration of difference between MaskRef::get and MaskRef::tryGet
TEST(FieldMaskTest, MaskRefGetAndTryGet) {
  MaskBuilder<Bar2> mb(noneMask());
  // field_3 is specied with noneMask.
  mb.includes<ident::field_3>(noneMask());

  auto mr = MaskRef{mb.toThrift()};

  // field_2 not included.
  EXPECT_TRUE(mr.get(op::get_field_id_v<Bar2, ident::field_2>).isNoneMask());

  // field_3 not included.
  EXPECT_TRUE(mr.get(op::get_field_id_v<Bar2, ident::field_3>).isNoneMask());

  // field_2 not included.
  auto field2 = mr.tryGet(op::get_field_id_v<Bar2, ident::field_2>);
  EXPECT_FALSE(field2.has_value());

  // field_3 included but specified with noneMask.
  auto field3 = mr.tryGet(op::get_field_id_v<Bar2, ident::field_3>);
  EXPECT_TRUE(field3.has_value());
  EXPECT_TRUE(field3->isNoneMask());
}

TEST(FieldMaskTest, DynamicMaskBuilderInvalid) {
  {
    // Empty
    DynamicMaskBuilder builder(noneMask());
    EXPECT_THROW(builder.includes({}), std::runtime_error);
    EXPECT_THROW(builder.excludes({}), std::runtime_error);
    EXPECT_THROW(builder.includes_map_element({}, 42), std::runtime_error);
    EXPECT_THROW(builder.excludes_map_element({}, 42), std::runtime_error);
    EXPECT_THROW(builder.includes_map_element({}, "key"), std::runtime_error);
    EXPECT_THROW(builder.excludes_map_element({}, "key"), std::runtime_error);
    EXPECT_THROW(
        builder.includes_type({}, type::Type::get<type::i32_t>()),
        std::runtime_error);
    EXPECT_THROW(
        builder.excludes_type({}, type::Type::get<type::i32_t>()),
        std::runtime_error);
  }
}

} // namespace apache::thrift::test
