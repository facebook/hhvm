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

#include <thrift/test/AdapterTest.h>

#include <chrono>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <vector>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/test/gen-cpp2/adapter_clients.h>
#include <thrift/test/gen-cpp2/adapter_constants.h>
#include <thrift/test/gen-cpp2/adapter_handlers.h>
#include <thrift/test/gen-cpp2/adapter_no_uri_types.h>
#include <thrift/test/gen-cpp2/adapter_terse_types.h>
#include <thrift/test/gen-cpp2/adapter_types.h>

#include <thrift/lib/cpp2/protocol/Object.h>

namespace apache::thrift::test {
template <typename Actual, typename Expected>
struct AssertSameType;
template <typename T>
struct AssertSameType<T, T> {};

struct AdapterTest : ::testing::Test {};

TEST_F(AdapterTest, AdaptedEnum) {
  using basic::AdaptedEnum;
  basic::AdaptTemplatedTestStruct myStruct;

  AssertSameType<AdaptedEnum&, decltype(*myStruct.adaptedEnum())>();
  EXPECT_EQ(myStruct.adaptedEnum(), AdaptedEnum::One);
  myStruct.adaptedEnum() = AdaptedEnum::Two;

  auto data = CompactSerializer::serialize<std::string>(myStruct);
  basic::AdaptTemplatedTestStruct myStruct2;
  CompactSerializer::deserialize(data, myStruct2);
  EXPECT_EQ(myStruct2.adaptedEnum(), AdaptedEnum::Two);

  EXPECT_EQ(myStruct, myStruct2);
}

TEST_F(AdapterTest, AdaptedT) {
  AssertSameType<adapt_detail::adapted_t<OverloadedAdapter, int64_t>, Num>();
  AssertSameType<
      adapt_detail::adapted_t<OverloadedAdapter, std::string>,
      String>();
}

TEST_F(AdapterTest, IsMutableRef) {
  EXPECT_FALSE(adapt_detail::is_mutable_ref<int>::value);
  EXPECT_FALSE(adapt_detail::is_mutable_ref<const int>::value);
  EXPECT_FALSE(adapt_detail::is_mutable_ref<const int&>::value);
  EXPECT_FALSE(adapt_detail::is_mutable_ref<const int&&>::value);
  EXPECT_TRUE(adapt_detail::is_mutable_ref<int&>::value);
  EXPECT_TRUE(adapt_detail::is_mutable_ref<int&&>::value);
  EXPECT_TRUE(adapt_detail::is_mutable_ref<volatile int&&>::value);
}

TEST_F(AdapterTest, HasInplaceToThrift) {
  EXPECT_TRUE((adapt_detail::has_inplace_toThrift<
               IndirectionAdapter<IndirectionString>,
               IndirectionString>::value));
  // Does not return a mutable ref.
  EXPECT_FALSE(
      (adapt_detail::has_inplace_toThrift<OverloadedAdapter, Num>::value));

  EXPECT_TRUE((
      adapt_detail::has_inplace_toThrift<IndirectionAdapter<int>, int>::value));

  // `IndirectionAdapter<IndirectionString>::toThrift(int&&)`
  // is invalid, so we get false.
  EXPECT_FALSE((adapt_detail::has_inplace_toThrift<
                IndirectionAdapter<IndirectionAdapter<IndirectionString>>,
                int>::value));
}

namespace basic {
TEST_F(AdapterTest, StructCodeGen_Empty) {
  AdaptTestStruct obj0a;
  EXPECT_EQ(obj0a.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.custom_ref()->val, 13); // Defined in Num.
  EXPECT_EQ(obj0a.timeout_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.indirectionString_ref()->val, "");

  auto data0 = CompactSerializer::serialize<std::string>(obj0a);
  AdaptTestStruct obj0b;
  CompactSerializer::deserialize(data0, obj0b);
  EXPECT_EQ(obj0b.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0b.custom_ref()->val, 13);
  EXPECT_EQ(obj0b.timeout_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.indirectionString_ref()->val, "");

  EXPECT_EQ(obj0b, obj0a);
}

TEST_F(AdapterTest, StructCodeGen_Zero) {
  AdaptTestStruct obj0a;
  EXPECT_EQ(obj0a.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.custom_ref()->val, 13); // Defined in Num.
  obj0a.custom_ref()->val = 0;
  EXPECT_EQ(obj0a.timeout_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.indirectionString_ref()->val, "");

  auto data0 = CompactSerializer::serialize<std::string>(obj0a);
  AdaptTestStruct obj0b;
  CompactSerializer::deserialize(data0, obj0b);
  EXPECT_EQ(obj0b.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0b.custom_ref()->val, 0);
  EXPECT_EQ(obj0b.timeout_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.indirectionString_ref()->val, "");

  EXPECT_EQ(obj0b, obj0a);
}

TEST_F(AdapterTest, StructCodeGen) {
  AdaptTestStruct obj1a;
  AssertSameType<decltype(*obj1a.delay_ref()), std::chrono::milliseconds&>();
  AssertSameType<decltype(*obj1a.custom_ref()), Num&>();
  AssertSameType<decltype(*obj1a.timeout_ref()), std::chrono::milliseconds&>();
  AssertSameType<
      decltype(*obj1a.indirectionString_ref()),
      IndirectionString&>();

  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(0));
  obj1a.delay_ref() = std::chrono::milliseconds(7);
  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(7));

  EXPECT_EQ(obj1a.custom_ref()->val, 13);
  obj1a.custom_ref() = Num{std::numeric_limits<int64_t>::min()};
  EXPECT_EQ(obj1a.custom_ref()->val, std::numeric_limits<int64_t>::min());

  EXPECT_EQ(obj1a.timeout_ref(), std::chrono::milliseconds(0));
  obj1a.timeout_ref() = std::chrono::milliseconds(7);
  EXPECT_EQ(obj1a.timeout_ref(), std::chrono::milliseconds(7));

  EXPECT_EQ(obj1a.indirectionString_ref()->val, "");
  obj1a.indirectionString_ref()->val = "hi";
  EXPECT_EQ(obj1a.indirectionString_ref()->val, "hi");

  auto data1 = CompactSerializer::serialize<std::string>(obj1a);
  AdaptTestStruct obj1b;
  CompactSerializer::deserialize(data1, obj1b);
  EXPECT_EQ(obj1b.delay_ref(), std::chrono::milliseconds(7));
  EXPECT_EQ(obj1b.custom_ref()->val, std::numeric_limits<int64_t>::min());
  EXPECT_EQ(obj1b.timeout_ref(), std::chrono::milliseconds(7));
  EXPECT_EQ(obj1a.indirectionString_ref()->val, "hi");

  EXPECT_EQ(obj1b, obj1b);
  EXPECT_FALSE(obj1b < obj1a);

  obj1b.custom_ref()->val = 1;
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1a.custom_ref() < obj1b.custom_ref());
  EXPECT_FALSE(obj1b.custom_ref() < obj1a.custom_ref());
  EXPECT_TRUE(obj1a < obj1b);
  EXPECT_FALSE(obj1b < obj1a);

  obj1a.delay_ref() = std::chrono::milliseconds(8);
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1b.delay_ref() < obj1a.delay_ref());
  EXPECT_FALSE(obj1a.delay_ref() < obj1b.delay_ref());
  EXPECT_TRUE(obj1b < obj1a);
  EXPECT_FALSE(obj1a < obj1b);

  obj1a.timeout_ref() = std::chrono::milliseconds(8);
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1b.timeout_ref() < obj1a.timeout_ref());
  EXPECT_FALSE(obj1a.timeout_ref() < obj1b.timeout_ref());
  EXPECT_TRUE(obj1b < obj1a);
  EXPECT_FALSE(obj1a < obj1b);

  obj1a = {};
  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj1a.custom_ref()->val, 13);
  EXPECT_EQ(obj1a.timeout_ref(), std::chrono::milliseconds(0));
}
} // namespace basic

namespace terse {
TEST_F(AdapterTest, StructCodeGen_Empty_Terse) {
  AdaptTestStruct obj0a;
  EXPECT_EQ(obj0a.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.custom_ref()->val, 13); // Defined in Num.

  auto data0 = CompactSerializer::serialize<std::string>(obj0a);
  AdaptTestStruct obj0b;
  CompactSerializer::deserialize(data0, obj0b);
  EXPECT_EQ(obj0b.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0b.custom_ref()->val, 13);

  EXPECT_EQ(obj0b, obj0a);
}

TEST_F(AdapterTest, StructCodeGen_Zero_Terse) {
  AdaptTestStruct obj0a;
  EXPECT_EQ(obj0a.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0a.custom_ref()->val, 13); // Defined in Num.
  obj0a.custom_ref()->val = 0;

  auto data0 = CompactSerializer::serialize<std::string>(obj0a);
  AdaptTestStruct obj0b;
  CompactSerializer::deserialize(data0, obj0b);
  EXPECT_EQ(obj0b.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj0b.custom_ref()->val, 0);

  EXPECT_EQ(obj0b, obj0a);
}

TEST_F(AdapterTest, StructCodeGen_Terse) {
  AdaptTestStruct obj1a;
  AssertSameType<decltype(*obj1a.delay_ref()), std::chrono::milliseconds&>();
  AssertSameType<decltype(*obj1a.custom_ref()), Num&>();

  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(0));
  obj1a.delay_ref() = std::chrono::milliseconds(7);
  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(7));

  EXPECT_EQ(obj1a.custom_ref()->val, 13);
  obj1a.custom_ref() = Num{std::numeric_limits<int64_t>::min()};
  EXPECT_EQ(obj1a.custom_ref()->val, std::numeric_limits<int64_t>::min());

  auto data1 = CompactSerializer::serialize<std::string>(obj1a);
  AdaptTestStruct obj1b;
  CompactSerializer::deserialize(data1, obj1b);
  EXPECT_EQ(obj1b.delay_ref(), std::chrono::milliseconds(7));
  EXPECT_EQ(obj1b.custom_ref()->val, std::numeric_limits<int64_t>::min());

  EXPECT_EQ(obj1b, obj1b);
  EXPECT_FALSE(obj1b < obj1a);

  obj1b.custom_ref()->val = 1;
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1a.custom_ref() < obj1b.custom_ref());
  EXPECT_FALSE(obj1b.custom_ref() < obj1a.custom_ref());
  EXPECT_TRUE(obj1a < obj1b);
  EXPECT_FALSE(obj1b < obj1a);

  obj1a.delay_ref() = std::chrono::milliseconds(8);
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1b.delay_ref() < obj1a.delay_ref());
  EXPECT_FALSE(obj1a.delay_ref() < obj1b.delay_ref());
  EXPECT_TRUE(obj1b < obj1a);
  EXPECT_FALSE(obj1a < obj1b);

  obj1a = {};
  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(0));
  EXPECT_EQ(obj1a.custom_ref()->val, 13);
}
} // namespace terse

namespace basic {
TEST_F(AdapterTest, UnionCodeGen_Empty) {
  ThriftAdaptTestUnion obj0a;
  EXPECT_EQ(obj0a.getType(), ThriftAdaptTestUnion::Type::__EMPTY__);

  auto data0 = CompactSerializer::serialize<std::string>(obj0a);
  ThriftAdaptTestUnion obj0b;
  CompactSerializer::deserialize(data0, obj0b);
  EXPECT_EQ(obj0b.getType(), ThriftAdaptTestUnion::Type::__EMPTY__);

  EXPECT_EQ(obj0b, obj0a);
  EXPECT_FALSE(obj0b < obj0a);
}

TEST_F(AdapterTest, UnionCodeGen_Delay_Default) {
  ThriftAdaptTestUnion obj1a;
  EXPECT_EQ(obj1a.delay_ref().ensure(), std::chrono::milliseconds(0));

  auto data1 = CompactSerializer::serialize<std::string>(obj1a);
  ThriftAdaptTestUnion obj1b;
  CompactSerializer::deserialize(data1, obj1b);
  EXPECT_EQ(obj1b.delay_ref().ensure(), std::chrono::milliseconds(0));

  EXPECT_EQ(obj1b, obj1a);
  EXPECT_FALSE(obj1b < obj1a);
}

TEST_F(AdapterTest, UnionCodeGen_Delay) {
  ThriftAdaptTestUnion obj1a;
  EXPECT_EQ(obj1a.delay_ref().ensure(), std::chrono::milliseconds(0));
  obj1a.delay_ref() = std::chrono::milliseconds(7);
  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(7));

  auto data1 = CompactSerializer::serialize<std::string>(obj1a);
  ThriftAdaptTestUnion obj1b;
  CompactSerializer::deserialize(data1, obj1b);
  EXPECT_EQ(obj1b.delay_ref(), std::chrono::milliseconds(7));

  EXPECT_EQ(obj1b, obj1a);
  EXPECT_FALSE(obj1b < obj1a);

  obj1a.delay_ref() = std::chrono::milliseconds(8);
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1b.delay_ref() < obj1a.delay_ref());
  EXPECT_FALSE(obj1a.delay_ref() < obj1b.delay_ref());
  EXPECT_TRUE(obj1b < obj1a);
  EXPECT_FALSE(obj1a < obj1b);
}

TEST_F(AdapterTest, UnionCodeGen_Custom_Default) {
  ThriftAdaptTestUnion obj2a;
  EXPECT_EQ(obj2a.custom_ref().ensure().val, 13); // Defined in Num.

  auto data2 = CompactSerializer::serialize<std::string>(obj2a);
  ThriftAdaptTestUnion obj2b;
  CompactSerializer::deserialize(data2, obj2b);
  EXPECT_EQ(obj2b.custom_ref()->val, 13);

  EXPECT_EQ(obj2b, obj2a);
  EXPECT_FALSE(obj2b < obj2a);
}

TEST_F(AdapterTest, UnionCodeGen_Custom_Zero) {
  ThriftAdaptTestUnion obj2a;
  EXPECT_EQ(obj2a.custom_ref().ensure().val, 13); // Defined in Num.
  obj2a.custom_ref()->val = 0;

  auto data2 = CompactSerializer::serialize<std::string>(obj2a);
  ThriftAdaptTestUnion obj2b;
  CompactSerializer::deserialize(data2, obj2b);
  EXPECT_EQ(obj2b.custom_ref()->val, 0);

  EXPECT_EQ(obj2b, obj2a);
  EXPECT_FALSE(obj2b < obj2a);
}

TEST_F(AdapterTest, UnionCodeGen_Custom) {
  ThriftAdaptTestUnion obj2a;
  EXPECT_EQ(obj2a.custom_ref().ensure().val, 13); // Defined in Num.
  obj2a.custom_ref() = Num{std::numeric_limits<int64_t>::min()};
  EXPECT_EQ(obj2a.custom_ref()->val, std::numeric_limits<int64_t>::min());

  auto data2 = CompactSerializer::serialize<std::string>(obj2a);
  ThriftAdaptTestUnion obj2b;
  CompactSerializer::deserialize(data2, obj2b);
  EXPECT_EQ(obj2b.custom_ref()->val, std::numeric_limits<int64_t>::min());

  EXPECT_EQ(obj2b, obj2a);
  EXPECT_FALSE(obj2b < obj2a);

  obj2b.custom_ref()->val = 1;
  EXPECT_NE(obj2b, obj2a);
  EXPECT_TRUE(obj2a.custom_ref() < obj2b.custom_ref());
  EXPECT_FALSE(obj2b.custom_ref() < obj2a.custom_ref());
  EXPECT_TRUE(obj2a < obj2b);
  EXPECT_FALSE(obj2b < obj2a);
}

TEST_F(AdapterTest, Setter) {
  ThriftAdaptTestUnion obj1;
  obj1.set_i32_string_field("1");
  EXPECT_EQ(obj1.i32_string_field_ref(), "1");
}

TEST_F(AdapterTest, TemplatedTestAdapter_AdaptTemplatedTestStruct) {
  auto obj = AdaptTemplatedTestStruct();
  auto int_map = std::map<int64_t, int64_t>{{1, 1}};
  EXPECT_EQ(obj.adaptedBoolDefault_ref()->value, true);
  EXPECT_EQ(obj.adaptedByteDefault_ref()->value, 1);
  EXPECT_EQ(obj.adaptedShortDefault_ref()->value, 2);
  EXPECT_EQ(obj.adaptedIntegerDefault_ref()->value, 3);
  EXPECT_EQ(obj.adaptedLongDefault_ref()->value, 4);
  EXPECT_EQ(obj.adaptedDoubleDefault_ref()->value, 5);
  EXPECT_EQ(obj.adaptedStringDefault_ref()->value, "6");
  EXPECT_EQ(obj.adaptedListDefault()->value, std::vector<int64_t>{1});
  EXPECT_EQ(obj.adaptedSetDefault()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedMapDefault()->value, int_map);

  obj.adaptedBool_ref() = Wrapper<bool>{true};
  obj.adaptedByte_ref() = Wrapper<int8_t>{1};
  obj.adaptedShort_ref() = Wrapper<int16_t>{2};
  obj.adaptedInteger_ref() = Wrapper<int32_t>{3};
  obj.adaptedLong_ref() = Wrapper<int64_t>{1};
  obj.adaptedDouble_ref() = Wrapper<double>{2};
  obj.adaptedString_ref() = Wrapper<std::string>{"3"};
  obj.adaptedList() = Wrapper<std::vector<int64_t>>{{1}};
  obj.adaptedSet() = Wrapper<std::set<int64_t>>{{1}};
  obj.adaptedMap() = Wrapper<std::map<int64_t, int64_t>>{{{1, 1}}};
  obj.doubleTypedefBool_ref() = Wrapper<bool>{true};

  EXPECT_EQ(obj.adaptedBool_ref()->value, true);
  EXPECT_EQ(obj.adaptedByte_ref()->value, 1);
  EXPECT_EQ(obj.adaptedShort_ref()->value, 2);
  EXPECT_EQ(obj.adaptedInteger_ref()->value, 3);
  EXPECT_EQ(obj.adaptedLong_ref()->value, 1);
  EXPECT_EQ(obj.adaptedDouble_ref()->value, 2);
  EXPECT_EQ(obj.adaptedString_ref()->value, "3");
  EXPECT_EQ(obj.adaptedList()->value, std::vector<int64_t>{1});
  EXPECT_EQ(obj.adaptedSet()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedMap()->value, int_map);
  EXPECT_EQ(obj.doubleTypedefBool_ref()->value, true);

  auto objs = CompactSerializer::serialize<std::string>(obj);
  AdaptTemplatedTestStruct objd;
  CompactSerializer::deserialize(objs, objd);
  EXPECT_EQ(objd.adaptedBool_ref()->value, true);
  EXPECT_EQ(objd.adaptedByte_ref()->value, 1);
  EXPECT_EQ(objd.adaptedShort_ref()->value, 2);
  EXPECT_EQ(objd.adaptedInteger_ref()->value, 3);
  EXPECT_EQ(objd.adaptedLong_ref()->value, 1);
  EXPECT_EQ(objd.adaptedDouble_ref()->value, 2);
  EXPECT_EQ(objd.adaptedString_ref()->value, "3");
  EXPECT_EQ(obj.adaptedList()->value, std::vector<int64_t>{1});
  EXPECT_EQ(obj.adaptedSet()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedMap()->value, int_map);
  EXPECT_EQ(obj.doubleTypedefBool_ref()->value, true);
  EXPECT_EQ(obj, objd);

  // Adapted fields reset to the intrinsic default.
  apache::thrift::clear(obj);
  EXPECT_EQ(obj.adaptedBoolDefault_ref()->value, false);
  EXPECT_EQ(obj.adaptedByteDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedShortDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedIntegerDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedLongDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedDoubleDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedStringDefault_ref()->value, "");
  EXPECT_TRUE(obj.adaptedListDefault()->value.empty());
  EXPECT_TRUE(obj.adaptedSetDefault()->value.empty());
  EXPECT_TRUE(obj.adaptedMapDefault()->value.empty());
}

TEST_F(AdapterTest, TemplatedTestAdapter_AdaptTemplatedNestedTestStruct) {
  auto obj = AdaptTemplatedNestedTestStruct();
  auto int_map = std::map<int64_t, int64_t>{{1, 1}};
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedBoolDefault_ref()->value, true);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedByteDefault_ref()->value, 1);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedShortDefault_ref()->value, 2);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedIntegerDefault_ref()->value, 3);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedLongDefault_ref()->value, 4);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedDoubleDefault_ref()->value, 5);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedStringDefault_ref()->value, "6");
  EXPECT_EQ(
      obj.adaptedStruct()->adaptedListDefault()->value,
      std::vector<int64_t>{1});
  EXPECT_EQ(
      obj.adaptedStruct()->adaptedSetDefault()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedStruct()->adaptedMapDefault()->value, int_map);

  obj.adaptedStruct_ref()->adaptedBool_ref() = Wrapper<bool>{true};
  obj.adaptedStruct_ref()->adaptedByte_ref() = Wrapper<int8_t>{1};
  obj.adaptedStruct_ref()->adaptedShort_ref() = Wrapper<int16_t>{2};
  obj.adaptedStruct_ref()->adaptedInteger_ref() = Wrapper<int32_t>{3};
  obj.adaptedStruct_ref()->adaptedLong_ref() = Wrapper<int64_t>{1};
  obj.adaptedStruct_ref()->adaptedDouble_ref() = Wrapper<double>{2};
  obj.adaptedStruct_ref()->adaptedString_ref() = Wrapper<std::string>{"3"};
  obj.adaptedStruct()->adaptedList() = Wrapper<std::vector<int64_t>>{{1}};
  obj.adaptedStruct()->adaptedSet() = Wrapper<std::set<int64_t>>{{1}};
  obj.adaptedStruct()->adaptedMap() =
      Wrapper<std::map<int64_t, int64_t>>{{{1, 1}}};

  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedBool_ref()->value, true);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedByte_ref()->value, 1);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedShort_ref()->value, 2);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedInteger_ref()->value, 3);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedLong_ref()->value, 1);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedDouble_ref()->value, 2);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedString_ref()->value, "3");
  EXPECT_EQ(obj.adaptedStruct()->adaptedList()->value, std::vector<int64_t>{1});
  EXPECT_EQ(obj.adaptedStruct()->adaptedSet()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedStruct()->adaptedMap()->value, int_map);

  auto objs = CompactSerializer::serialize<std::string>(obj);
  AdaptTemplatedNestedTestStruct objd;
  CompactSerializer::deserialize(objs, objd);

  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedBoolDefault_ref()->value, true);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedByteDefault_ref()->value, 1);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedShortDefault_ref()->value, 2);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedIntegerDefault_ref()->value, 3);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedLongDefault_ref()->value, 4);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedDoubleDefault_ref()->value, 5);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedStringDefault_ref()->value, "6");
  EXPECT_EQ(
      obj.adaptedStruct()->adaptedListDefault()->value,
      std::vector<int64_t>{1});
  EXPECT_EQ(
      obj.adaptedStruct()->adaptedSetDefault()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedStruct()->adaptedMapDefault()->value, int_map);

  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedBool_ref()->value, true);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedByte_ref()->value, 1);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedShort_ref()->value, 2);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedInteger_ref()->value, 3);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedLong_ref()->value, 1);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedDouble_ref()->value, 2);
  EXPECT_EQ(objd.adaptedStruct_ref()->adaptedString_ref()->value, "3");
  EXPECT_EQ(obj.adaptedStruct()->adaptedList()->value, std::vector<int64_t>{1});
  EXPECT_EQ(obj.adaptedStruct()->adaptedSet()->value, std::set<int64_t>{1});
  EXPECT_EQ(obj.adaptedStruct()->adaptedMap()->value, int_map);
  EXPECT_EQ(obj, objd);

  // Adapted fields reset to the intrinsic default.
  apache::thrift::clear(obj);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedBoolDefault_ref()->value, false);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedByteDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedShortDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedIntegerDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedLongDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedDoubleDefault_ref()->value, 0);
  EXPECT_EQ(obj.adaptedStruct_ref()->adaptedStringDefault_ref()->value, "");
  EXPECT_TRUE(obj.adaptedStruct()->adaptedListDefault()->value.empty());
  EXPECT_TRUE(obj.adaptedStruct()->adaptedSetDefault()->value.empty());
  EXPECT_TRUE(obj.adaptedStruct()->adaptedMapDefault()->value.empty());
}

TEST_F(AdapterTest, AdaptedUnion) {
  AdaptedUnion obj1;
  EXPECT_EQ(obj1.getType(), AdaptedUnion::Type::__EMPTY__);
  auto wrapper = NonComparableWrapper<std::string>();
  wrapper.value = "1";
  obj1.field1_ref() = wrapper;
  EXPECT_EQ(obj1.field1_ref()->value, "1");
}

TEST(AdaptTest, ComparisonTestUnion) {
  auto obj = AdaptedUnion();
  auto wrapper = NonComparableWrapper<std::string>();
  wrapper.value = "1";
  obj.field1_ref() = std::move(wrapper);

  auto obj1 = AdaptedUnion();
  auto wrapper1 = NonComparableWrapper<std::string>();
  wrapper1.value = "1";
  obj1.field1_ref() = std::move(wrapper1);

  auto obj2 = AdaptedUnion();
  auto wrapper2 = NonComparableWrapper<std::string>();
  wrapper2.value = "2";
  obj2.field1_ref() = std::move(wrapper2);
  // It uses 'Adapter::toThrift(lhs) == Adapter::toThrift(rhs)' for comparison.
  EXPECT_EQ(obj, obj1);
  EXPECT_FALSE(obj < obj1);
  EXPECT_FALSE(obj > obj1);
  EXPECT_TRUE(obj < obj2);
  EXPECT_FALSE(obj > obj2);
}

TEST(AdaptTest, ComparisonFallbackTest) {
  auto obj1a = AdapterEqualsUnion();
  obj1a.field1_ref() = "1";

  auto obj2a = AdapterEqualsUnion();
  obj2a.field1_ref() = "1";
  // It should use the AdapterEqualsStringAdapter operator== for comparison.
  EXPECT_FALSE(obj1a == obj2a);

  auto obj1b = AdaptedEqualsUnion();
  auto string1 = AdaptedEqualsString();
  string1.val = "1";
  obj1b.field1_ref() = string1;

  auto obj2b = AdaptedEqualsUnion();
  auto string2 = AdaptedEqualsString();
  string2.val = "1";
  obj2b.field1_ref() = string2;
  // It should use the AdaptedEqualsStringAdapter operator== for comparison.
  EXPECT_FALSE(obj1b == obj2b);
}

TEST_F(AdapterTest, StructFieldAdaptedStruct) {
  StructFieldAdaptedStruct obj;
  {
    auto wrapper = Wrapper<ThriftAdaptedStruct>();
    wrapper.value.data_ref() = 42;
    obj.adaptedStruct_ref() = wrapper;
    EXPECT_EQ(obj.adaptedStruct_ref()->value.data_ref(), 42);

    auto objs = CompactSerializer::serialize<std::string>(obj);
    StructFieldAdaptedStruct objd;
    CompactSerializer::deserialize(objs, objd);

    EXPECT_EQ(objd.adaptedStruct_ref()->value.data_ref(), 42);
    EXPECT_EQ(obj, objd);

    // Adapted fields reset to the intrinsic default.
    apache::thrift::clear(obj);
    EXPECT_EQ(obj.adaptedStruct_ref()->value.data_ref(), 0);
  }

  {
    DirectlyAdaptedStruct wrapper;
    wrapper.value.data_ref() = 42;
    obj.directlyAdapted_ref() = wrapper;
    EXPECT_EQ(obj.directlyAdapted_ref()->value.data_ref(), 42);

    auto objs = CompactSerializer::serialize<std::string>(obj);
    StructFieldAdaptedStruct objd;
    CompactSerializer::deserialize(objs, objd);

    EXPECT_EQ(objd.directlyAdapted_ref()->value.data_ref(), 42);
    EXPECT_EQ(obj, objd);

    // Adapted fields reset to the intrinsic default.
    apache::thrift::clear(obj);
    EXPECT_EQ(obj.directlyAdapted_ref()->value.data_ref(), 0);
  }

  {
    TypedefOfDirect wrapper;
    wrapper.value.data_ref() = 42;
    obj.typedefOfAdapted_ref() = wrapper;
    EXPECT_EQ(obj.typedefOfAdapted_ref()->value.data_ref(), 42);

    auto objs = CompactSerializer::serialize<std::string>(obj);
    StructFieldAdaptedStruct objd;
    CompactSerializer::deserialize(objs, objd);

    EXPECT_EQ(objd.typedefOfAdapted_ref()->value.data_ref(), 42);
    EXPECT_EQ(obj, objd);

    // Adapted fields reset to the intrinsic default.
    apache::thrift::clear(obj);
    EXPECT_EQ(obj.typedefOfAdapted_ref()->value.data_ref(), 0);
  }
}
} // namespace basic

namespace no_uri {
TEST(AdaptTest, Union_NoUri) {
  ThriftComparisonUnion obj1;
  obj1.field1_ref().ensure().value = "1";
  EXPECT_EQ(obj1.field1_ref()->value, "1");

  RefUnion obj2;
  obj2.field1_ref().ensure().value = "1";
  EXPECT_EQ(obj2.field1_ref()->value, "1");

  auto obj1b = ThriftComparisonUnion();
  obj1b.field1_ref().ensure().value = "1";

  auto obj2b = ThriftComparisonUnion();
  obj2b.field1_ref().ensure().value = "1";
  EXPECT_TRUE(obj1b == obj2b);
}

TEST(AdaptTest, LessThanComparisonFallbackTest) {
  auto obj1a = AdapterComparisonUnion();
  obj1a.field1_ref() = "1";

  auto obj2a = AdapterComparisonUnion();
  obj2a.field1_ref() = "2";
  // It should use the AdapterComparisonStringAdapter less for comparison.
  EXPECT_TRUE(obj1a > obj2a);
  EXPECT_FALSE(obj1a < obj2a);

  auto obj1b = AdaptedComparisonUnion();
  obj1b.field1_ref().ensure().val = "1";

  auto obj2b = AdaptedComparisonUnion();
  obj2b.field1_ref().ensure().val = "2";
  // It should use the AdaptedComparisonString operator< for comparison.
  EXPECT_TRUE(obj1b > obj2b);
  EXPECT_FALSE(obj1b < obj2b);

  auto obj1c = ThriftComparisonUnion();
  obj1c.field1_ref().ensure().value = "1";

  auto obj2c = ThriftComparisonUnion();
  obj2c.field1_ref().ensure().value = "2";
  // It uses 'Adapter::toThrift(lhs) < Adapter::toThrift(rhs)' for comparison.
  EXPECT_FALSE(obj1c > obj2c);
  EXPECT_TRUE(obj1c < obj2c);
}
} // namespace no_uri

namespace terse {
TEST_F(AdapterTest, UnionCodeGen_Empty_Terse) {
  AdaptTestUnion obj0a;
  EXPECT_EQ(obj0a.getType(), AdaptTestUnion::Type::__EMPTY__);

  auto data0 = CompactSerializer::serialize<std::string>(obj0a);
  AdaptTestUnion obj0b;
  CompactSerializer::deserialize(data0, obj0b);
  EXPECT_EQ(obj0b.getType(), AdaptTestUnion::Type::__EMPTY__);

  EXPECT_EQ(obj0b, obj0a);
  EXPECT_FALSE(obj0b < obj0a);
}

TEST_F(AdapterTest, UnionCodeGen_Delay_Default_Terse) {
  AdaptTestUnion obj1a;
  EXPECT_EQ(obj1a.delay_ref().ensure(), std::chrono::milliseconds(0));

  auto data1 = CompactSerializer::serialize<std::string>(obj1a);
  AdaptTestUnion obj1b;
  CompactSerializer::deserialize(data1, obj1b);
  EXPECT_EQ(obj1b.delay_ref().ensure(), std::chrono::milliseconds(0));

  EXPECT_EQ(obj1b, obj1a);
  EXPECT_FALSE(obj1b < obj1a);
}

TEST_F(AdapterTest, UnionCodeGen_Delay_Terse) {
  AdaptTestUnion obj1a;
  EXPECT_EQ(obj1a.delay_ref().ensure(), std::chrono::milliseconds(0));
  obj1a.delay_ref() = std::chrono::milliseconds(7);
  EXPECT_EQ(obj1a.delay_ref(), std::chrono::milliseconds(7));

  auto data1 = CompactSerializer::serialize<std::string>(obj1a);
  AdaptTestUnion obj1b;
  CompactSerializer::deserialize(data1, obj1b);
  EXPECT_EQ(obj1b.delay_ref(), std::chrono::milliseconds(7));

  EXPECT_EQ(obj1b, obj1a);
  EXPECT_FALSE(obj1b < obj1a);

  obj1a.delay_ref() = std::chrono::milliseconds(8);
  EXPECT_NE(obj1b, obj1a);
  EXPECT_TRUE(obj1b.delay_ref() < obj1a.delay_ref());
  EXPECT_FALSE(obj1a.delay_ref() < obj1b.delay_ref());
  EXPECT_TRUE(obj1b < obj1a);
  EXPECT_FALSE(obj1a < obj1b);
}

TEST_F(AdapterTest, UnionCodeGen_Custom_Default_Terse) {
  AdaptTestUnion obj2a;
  EXPECT_EQ(obj2a.custom_ref().ensure().val, 13); // Defined in Num.

  auto data2 = CompactSerializer::serialize<std::string>(obj2a);
  AdaptTestUnion obj2b;
  CompactSerializer::deserialize(data2, obj2b);
  EXPECT_EQ(obj2b.custom_ref()->val, 13);

  EXPECT_EQ(obj2b, obj2a);
  EXPECT_FALSE(obj2b < obj2a);
}

TEST_F(AdapterTest, UnionCodeGen_Custom_Zero_Terse) {
  AdaptTestUnion obj2a;
  EXPECT_EQ(obj2a.custom_ref().ensure().val, 13); // Defined in Num.
  obj2a.custom_ref()->val = 0;

  auto data2 = CompactSerializer::serialize<std::string>(obj2a);
  AdaptTestUnion obj2b;
  CompactSerializer::deserialize(data2, obj2b);
  EXPECT_EQ(obj2b.custom_ref()->val, 0);

  EXPECT_EQ(obj2b, obj2a);
  EXPECT_FALSE(obj2b < obj2a);
}

TEST_F(AdapterTest, UnionCodeGen_Custom_Terse) {
  AdaptTestUnion obj2a;
  EXPECT_EQ(obj2a.custom_ref().ensure().val, 13); // Defined in Num.
  obj2a.custom_ref() = Num{std::numeric_limits<int64_t>::min()};
  EXPECT_EQ(obj2a.custom_ref()->val, std::numeric_limits<int64_t>::min());

  auto data2 = CompactSerializer::serialize<std::string>(obj2a);
  AdaptTestUnion obj2b;
  CompactSerializer::deserialize(data2, obj2b);
  EXPECT_EQ(obj2b.custom_ref()->val, std::numeric_limits<int64_t>::min());

  EXPECT_EQ(obj2b, obj2a);
  EXPECT_FALSE(obj2b < obj2a);

  obj2b.custom_ref()->val = 1;
  EXPECT_NE(obj2b, obj2a);
  EXPECT_TRUE(obj2a.custom_ref() < obj2b.custom_ref());
  EXPECT_FALSE(obj2b.custom_ref() < obj2a.custom_ref());
  EXPECT_TRUE(obj2a < obj2b);
  EXPECT_FALSE(obj2b < obj2a);
}
} // namespace terse

struct ReferenceAdapter {
  static const int32_t& fromThrift(const int32_t&);
};
struct ReferenceAdapterWithContext {
  template <typename Context>
  static int64_t&& fromThriftField(const int64_t&, Context&&);
};

TEST_F(AdapterTest, FromThriftField) {
  auto obj = basic::AdaptTestStruct();
  AssertSameType<
      decltype(adapt_detail::fromThriftField<ReferenceAdapter, 0>(0, obj)),
      const int32_t&>();
  AssertSameType<
      decltype(adapt_detail::fromThriftField<ReferenceAdapterWithContext, 0>(
          0, obj)),
      int64_t&&>();
}

TEST_F(AdapterTest, StructAdapter) {
  EXPECT_TRUE((std::is_same_v<
               basic::DirectlyAdaptedStruct,
               Wrapper<basic::detail::DirectlyAdaptedStruct>>));
  EXPECT_TRUE((std::is_same_v<
               basic::RenamedStruct,
               Wrapper<basic::detail::UnderlyingRenamedStruct>>));
  EXPECT_TRUE((std::is_same_v<
               basic::SameNamespaceStruct,
               Wrapper<basic::UnderlyingSameNamespaceStruct>>));
}

TEST(AdaptTest, AdapterWithContext) {
  static_assert(folly::is_detected_v<
                adapt_detail::FromThriftFieldType,
                AdapterWithContext,
                int64_t,
                basic::AdaptTestStruct>);
  static_assert(folly::is_detected_v<
                adapt_detail::ConstructType,
                AdapterWithContext,
                AdaptedWithContext<int64_t, basic::AdaptTestStruct, 0>,
                FieldContext<basic::AdaptTestStruct, 0>>);

  auto obj = basic::AdaptTestStruct();
  EXPECT_EQ(obj.data()->meta, &*obj.meta());
  EXPECT_EQ(obj.string_data()->meta, &*obj.meta());
  EXPECT_EQ(obj.binary_data()->meta, &*obj.meta());

  auto copy = basic::AdaptTestStruct(obj);
  EXPECT_EQ(copy.data()->meta, &*copy.meta());
  EXPECT_EQ(copy.string_data()->meta, &*copy.meta());
  EXPECT_EQ(copy.binary_data()->meta, &*copy.meta());

  auto move = basic::AdaptTestStruct(std::move(copy));
  EXPECT_EQ(move.data()->meta, &*move.meta());
  EXPECT_EQ(move.string_data()->meta, &*move.meta());
  EXPECT_EQ(move.binary_data()->meta, &*move.meta());

  obj.data()->value = 42;
  obj.meta() = "foo";
  obj.string_data()->value = "42";
  obj.binary_data()->value = "100";

  EXPECT_EQ(obj.data()->value, 42);
  EXPECT_EQ(obj.data()->fieldId, 4);
  EXPECT_EQ(*obj.data()->meta, "foo");
  EXPECT_EQ(obj.string_data()->value, "42");
  EXPECT_EQ(obj.string_data()->fieldId, 7);
  EXPECT_EQ(*obj.string_data()->meta, "foo");
  EXPECT_EQ(obj.binary_data()->value, "100");
  EXPECT_EQ(obj.binary_data()->fieldId, 10);
  EXPECT_EQ(*obj.binary_data()->meta, "foo");

  auto serialized = CompactSerializer::serialize<std::string>(obj);
  auto obj2 = basic::AdaptTestStruct();
  CompactSerializer::deserialize(serialized, obj2);

  EXPECT_EQ(obj2.data()->value, 42);
  EXPECT_EQ(obj2.data()->fieldId, 4);
  EXPECT_EQ(*obj2.data()->meta, "foo");
  EXPECT_EQ(obj2.string_data()->value, "42");
  EXPECT_EQ(obj2.string_data()->fieldId, 7);
  EXPECT_EQ(*obj2.string_data()->meta, "foo");
  EXPECT_EQ(obj2.binary_data()->value, "100");
  EXPECT_EQ(obj2.binary_data()->fieldId, 10);
  EXPECT_EQ(*obj2.binary_data()->meta, "foo");
}

TEST(AdaptTest, ComposedAdapter) {
  auto obj = basic::AdaptTestStruct();

  obj.double_wrapped_bool() = {Wrapper<bool>{true}};
  obj.double_wrapped_integer()->value = Wrapper<int32_t>{42};
  obj.meta() = "foo";

  EXPECT_EQ(*obj.double_wrapped_integer()->meta, "foo");

  auto serialized = CompactSerializer::serialize<std::string>(obj);
  auto obj2 = basic::AdaptTestStruct();
  CompactSerializer::deserialize(serialized, obj2);

  EXPECT_EQ(obj2.double_wrapped_bool()->value.value, true);
  EXPECT_EQ(obj2.double_wrapped_integer()->value.value, 42);
  EXPECT_EQ(*obj2.double_wrapped_integer()->meta, "foo");
}

TEST(AdaptTest, ComparisonTestStruct) {
  auto obj = basic::ComparisonTestStruct();
  auto s = basic::MyStruct();
  s.field1() = 1;

  obj.non_comparable_adapted_type() = {s};
  obj.intern_box_non_comparable_adapted_type() = {s};
  obj.non_comparable_adapted_unordered_strings() = {{{"foo"}}};

  auto serialized = CompactSerializer::serialize<std::string>(obj);
  auto obj2 = basic::ComparisonTestStruct();
  CompactSerializer::deserialize(serialized, obj2);

  // It uses 'Adapter::toThrift(lhs) == Adapter::toThrift(rhs)' for comparison.
  EXPECT_EQ(obj, obj2);
  EXPECT_FALSE(obj < obj2);
  EXPECT_FALSE(obj > obj2);
}

TEST(AdaptTest, TransitiveAdapter) {
  basic::TransitiveAdapted obj;
  EXPECT_EQ(obj.value, basic::detail::TransitiveAdapted{});
}

TEST(AdaptTest, NumAdapterConversions) {
  // When an adapter implements serializedSize we guarantee to only call
  // toThrift once during deserialization.
  struct Handler : apache::thrift::ServiceHandler<basic::AdapterService> {
    void count(basic::CountingStruct& s) override {
      s.regularInt().ensure();
      s.countingInt().ensure();
      s.regularString().ensure();
    }
  };
  basic::CountingStruct s;
  makeTestClient(std::make_shared<Handler>())->sync_count(s);
  EXPECT_EQ((CountingAdapter<false, int>::count), 2);
  EXPECT_EQ((CountingAdapter<true, int>::count), 1);
  EXPECT_EQ((CountingAdapter<false, std::string>::count), 2);
}

TEST(AdaptTest, EncodeFallback) {
  basic::EncodeStruct obj{};
  obj.num_with_encode() = Num{1};
  obj.num_in_place().ensure().value = 2;
  obj.num_without_encode() = Num{3};

  // num_with_encode should not call to/fromThrift.
  // num_in_place should not call fromThrift.
  auto data = CompactSerializer::serialize<std::string>(obj);
  auto objd = CompactSerializer::deserialize<basic::EncodeStruct>(data);

  EXPECT_EQ(obj, objd);
}

TEST(AdaptTest, EncodeFieldFallback) {
  basic::EncodeFieldStruct obj{};
  obj.num_with_encode().ensure().value = 1;
  obj.num_without_encode().ensure().value = 2;

  // num_with_encode should not call toThrift/fromThriftField.
  auto data = CompactSerializer::serialize<std::string>(obj);
  auto objd = CompactSerializer::deserialize<basic::EncodeFieldStruct>(data);

  EXPECT_EQ(obj, objd);
}

TEST(AdaptTest, EncodeComposedAdapter) {
  auto obj = basic::EncodeComposedStruct();

  obj.double_wrapped_type_encode()->value = Wrapper<int64_t>{42};
  obj.double_wrapped_no_encode()->value = Wrapper<int64_t>{1};
  obj.double_wrapped_both_encode()->value = Wrapper<int64_t>{2};
  obj.double_wrapped_field_encode()->value = Wrapper<int64_t>{3};

  // No toThrift/fromThrift should be called at any adapter with an
  // encode/decode implementation.
  auto serialized = CompactSerializer::serialize<std::string>(obj);
  auto obj2 = basic::EncodeComposedStruct();
  CompactSerializer::deserialize(serialized, obj2);

  EXPECT_EQ(obj, obj2);
}

TEST(AdaptTest, Constants) {
  // const AdaptedBool type_adapted = true;
  EXPECT_EQ(basic::adapter_constants::type_adapted().value, true);

  // const list<AdaptedByte> container_of_adapted = [1, 2, 3];
  EXPECT_EQ(basic::adapter_constants::container_of_adapted()[0].value, 1);
  EXPECT_EQ(basic::adapter_constants::container_of_adapted()[1].value, 2);
  EXPECT_EQ(basic::adapter_constants::container_of_adapted()[2].value, 3);
}

TEST(AdaptTest, ContainerWithAdaptedElement) {
  basic::ContainerWithAdaptedElement obj{};

  obj.list_field().ensure().push_back({1});
  obj.set_field().ensure().insert({2});
  obj.map_field1().ensure().emplace(Wrapper<int8_t>{3}, 3);
  obj.map_field2().ensure().emplace(4, Wrapper<int8_t>{4});
  obj.map_field3().ensure().emplace(Wrapper<int8_t>{5}, Wrapper<int8_t>{5});

  auto serialized = CompactSerializer::serialize<std::string>(obj);
  basic::ContainerWithAdaptedElement obj2{};
  CompactSerializer::deserialize(serialized, obj2);

  EXPECT_EQ(obj2.list_field()[0].value, 1);
  EXPECT_EQ(obj2.set_field()->count({2}), 1);
  EXPECT_EQ(obj2.map_field1()->at({3}), 3);
  EXPECT_EQ(obj2.map_field2()->at(4).value, 4);
  EXPECT_EQ(obj2.map_field3()->at({5}).value, 5);
}

static_assert(
    std::is_same_v<
        folly::remove_cvref_t<decltype(basic::adapter_constants::timeout())>,
        VariableWrapper<std::int32_t>>);

TEST(VariableAdapterTest, Integer) {
  EXPECT_EQ(basic::adapter_constants::timeout().value, 42);
  EXPECT_EQ(basic::adapter_constants::timeout().name, "Foo");
  EXPECT_EQ(
      basic::adapter_constants::timeout().uri,
      "apache.org/thrift/test/basic/timeout");

  EXPECT_EQ(basic::adapter_constants::msg().value, "hello, world");
  EXPECT_EQ(basic::adapter_constants::msg().name, "Bar");
  EXPECT_EQ(
      basic::adapter_constants::msg().uri, "apache.org/thrift/test/basic/msg");

  EXPECT_EQ(basic::adapter_constants::person().value.name(), "DefaultName");
  EXPECT_EQ(basic::adapter_constants::person().name, "NameFromAnnotation");
  EXPECT_EQ(
      basic::adapter_constants::person().uri,
      "apache.org/thrift/test/basic/person");

  EXPECT_EQ(basic::adapter_constants::timeout_no_transitive().value, 420);
  EXPECT_EQ(basic::adapter_constants::timeout_no_transitive().name, "");
  EXPECT_EQ(basic::adapter_constants::timeout_no_transitive().uri, "");

  EXPECT_EQ(
      basic::adapter_constants::msg_no_transitive().value, "hello, world 2");
  EXPECT_EQ(basic::adapter_constants::msg_no_transitive().name, "");
  EXPECT_EQ(basic::adapter_constants::msg_no_transitive().uri, "");

  EXPECT_EQ(
      basic::adapter_constants::person_no_transitive().value.name(),
      "DefaultName 2");
  EXPECT_EQ(basic::adapter_constants::person_no_transitive().name, "");
  EXPECT_EQ(basic::adapter_constants::person_no_transitive().uri, "");
}

TEST_F(AdapterTest, GetClassName) {
  EXPECT_EQ(op::get_class_name_v<basic::MyStruct>, "MyStruct");
  EXPECT_EQ(
      op::get_class_name_v<basic::detail::DirectlyAdaptedStruct>,
      "DirectlyAdaptedStruct");
  EXPECT_EQ(
      op::get_class_name_v<basic::UnderlyingSameNamespaceStruct>,
      "SameNamespaceStruct");
  EXPECT_EQ(
      op::get_class_name_v<basic::detail::UnderlyingRenamedStruct>,
      "RenamedStruct");
}

template <class T>
void testCustomSerializedSize(bool zeroCopy) {
  T a;
  CompactProtocolWriter writer;
  if (!zeroCopy) {
    SerializedSizeAdapter::mockSize = 0;
    auto base = a.serializedSize(&writer);
    SerializedSizeAdapter::mockSize = 10;
    EXPECT_EQ(base + 10, a.serializedSize(&writer));
    return;
  }
  SerializedSizeAdapter::mockSizeZeroCopy = 0;
  auto base = a.serializedSizeZC(&writer);
  SerializedSizeAdapter::mockSizeZeroCopy = 10;
  EXPECT_EQ(base + 10, a.serializedSizeZC(&writer));
}

TEST_F(AdapterTest, CustomSerializedSize) {
  testCustomSerializedSize<basic::CustomSerializedSize>(false);
  testCustomSerializedSize<basic::CustomSerializedSize>(true);
  testCustomSerializedSize<basic::CustomSerializedSizeOpEncode>(false);
  testCustomSerializedSize<basic::CustomSerializedSizeOpEncode>(true);
}

TEST_F(AdapterTest, WrappedMyStruct) {
  basic::StructOfMyStruct s;
  s.myStruct()->toThrift().field1() = 0;
  auto iobuf = *CompactSerializer::serialize<folly::IOBufQueue>(s).move();

  auto obj = protocol::parseObject<CompactProtocolReader>(iobuf);
  EXPECT_EQ(obj[FieldId{1}].as_object()[FieldId{1}].as_i64(), 10);

  CompactSerializer::deserialize(&iobuf, s);
  EXPECT_EQ(s.myStruct()->toThrift().field1(), 20);
}

} // namespace apache::thrift::test
