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

#include <thrift/lib/cpp2/op/Hash.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/hash/DeterministicHash.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>

namespace apache::thrift::op {
namespace {

class DebugHasher {
 public:
  DebugHasher() {
    queue_ = std::make_unique<folly::IOBufQueue>();
    appender_.reset(queue_.get(), 1 << 10);
    combine("[");
  }

  DebugHasher(const DebugHasher& other) = delete;
  DebugHasher& operator=(const DebugHasher& other) = delete;

  DebugHasher(DebugHasher&& other) = default;
  DebugHasher& operator=(DebugHasher&& other) = default;

  void finalize() {
    combine("]");
    auto queue = queue_->move();
    auto buf = queue->coalesce();
    result_ = {reinterpret_cast<const char*>(buf.data()), buf.size()};
  }

  const std::string& getResult() const& { return result_; }
  std::string&& getResult() && { return std::move(result_); }

  template <typename Value>
  typename std::enable_if<std::is_arithmetic<Value>::value, void>::type combine(
      const Value& value) {
    handlePrefix();
    combine(folly::to<std::string>(value));
  }

  void combine(const folly::IOBuf& value) {
    handlePrefix();
    for (const auto buf : value) {
      combine(buf);
    }
  }

  void combine(folly::StringPiece value) { combine(folly::ByteRange{value}); }
  void combine(folly::ByteRange value) { appender_.push(value); }
  void combine(const DebugHasher& other) {
    handlePrefix();
    combine(other.result_);
  }

  bool operator<(const DebugHasher& other) { return result_ < other.result_; }

 private:
  void handlePrefix() {
    if (empty_) {
      empty_ = false;
    } else {
      combine(",");
    }
  }

  bool empty_ = true;
  std::unique_ptr<folly::IOBufQueue> queue_{nullptr};
  folly::io::QueueAppender appender_{nullptr, 0};
  std::string result_;
};

template <typename Struct>
auto hashStruct(const Struct& input) {
  return op::hash<type::struct_t<Struct>>(input);
}

template <typename Tag, typename T = type::native_type<Tag>>
auto debugHash(const T& input) {
  auto accumulator = makeDeterministicAccumulator<DebugHasher>();
  op::hash<Tag>(input, accumulator);
  return std::move(accumulator.result()).getResult();
}

TEST(HashTest, HashType) {
  using Tag = type::struct_t<test::OneOfEach>;
  test::OneOfEach value;

  std::unordered_set<std::size_t> s;
  auto check_and_add = [&s](auto tag, const auto& v) {
    using Tag = decltype(tag);
    EXPECT_EQ(s.count(hash<Tag>(v)), 0);
    s.insert(hash<Tag>(v));
    EXPECT_EQ(s.count(hash<Tag>(v)), 1);
  };

  for (auto i = 0; i < 10; i++) {
    value.myI32_ref() = i + 100;
    check_and_add(Tag{}, value);
    value.myList_ref() = {std::to_string(i + 200)};
    check_and_add(Tag{}, value);
    value.mySet_ref() = {std::to_string(i + 300)};
    check_and_add(Tag{}, value);
    value.myMap_ref() = {{std::to_string(i + 400), 0}};
    check_and_add(Tag{}, value);

    check_and_add(type::i32_t{}, *value.myI32_ref());
    check_and_add(type::list<type::string_t>{}, *value.myList_ref());
    check_and_add(type::set<type::string_t>{}, *value.mySet_ref());
    check_and_add(type::map<type::string_t, type::i64_t>{}, *value.myMap_ref());
  }
}

TEST(HashTest, HashDouble) {
  EXPECT_EQ(hash<type::double_t>(-0.0), hash<type::double_t>(+0.0));
  EXPECT_EQ(
      hash<type::double_t>(std::numeric_limits<double>::quiet_NaN()),
      hash<type::double_t>(std::numeric_limits<double>::quiet_NaN()));
}

TEST(HashTest, HashAccumulation) {
  test::OneOfEach value;
  EXPECT_EQ(debugHash<type::i32_t>(*value.myI32()), "[100017]");
  EXPECT_EQ(
      debugHash<type::list<type::string_t>>(*value.myList()),
      "[3,3foo,3bar,3baz]");
  EXPECT_EQ(
      debugHash<type::set<type::string_t>>(*value.mySet()),
      "[3,[[3bar],[3baz],[3foo]]]");
  EXPECT_EQ(
      (debugHash<type::map<type::string_t, type::i64_t>>(*value.myMap())),
      "[3,[[3bar,17],[3baz,19],[3foo,13]]]");
}

template <typename Mode>
class DeterministicProtocolTest : public ::testing::Test {
 public:
  template <typename Struct>
  auto hash(const Struct& input) {
    return Mode::hash(input);
  }
};

struct HasherMode {
  template <typename Struct>
  static auto hash(const Struct& input) {
    return debugHash<type::struct_t<Struct>>(input);
  }
};

struct GeneratorMode {
  template <typename Struct>
  static auto hash(const Struct& input) {
    return thrift::hash::deterministic_hash(
        input, [] { return DebugHasher{}; });
  }
};

using Modes = ::testing::Types<HasherMode, GeneratorMode>;

TYPED_TEST_CASE(DeterministicProtocolTest, Modes);

TYPED_TEST(DeterministicProtocolTest, checkOneOfEach) {
  const char* expected = // struct OneOfEach {
      "["
      // 1: bool myBool = 1;
      "[1,1],"
      // 10: set<string> mySet = ["foo", "bar", "baz"];
      "[10,3,[[3bar],[3baz],[3foo]]],"
      // 11: SubStruct myStruct;
      "[11,[[12,6foobar],[3,17]]],"
      // 12: SubUnion myUnion = kSubUnion;
      "[12,[[209,8glorious]]],"
      // 2: byte myByte = 17;
      "[2,17],"
      // 3: i16 myI16 = 1017;
      "[3,1017],"
      // 4: i32 myI32 = 100017;
      "[4,100017],"
      // 5: i64 myI64 = 5000000017;
      "[5,5000000017],"
      // 6: double myDouble = 5.25;
      "[6,5.25],"
      // 7: float myFloat = 5.25;
      "[7,5.25],"
      // 8: map<string, i64> myMap = {"foo": 13, "bar": 17, "baz": 19};
      "[8,3,[[3bar,17],[3baz,19],[3foo,13]]],"
      // 9: list<string> myList = ["foo", "bar", "baz"];
      "[9,3,3foo,3bar,3baz]"
      "]"; // }
  EXPECT_EQ(this->hash(test::OneOfEach{}), expected);
}

TYPED_TEST(DeterministicProtocolTest, checkOptional) {
  test::OptionalFields input;
  const auto result = this->hash(input);
  input.f1_ref() = {};
  EXPECT_NE(result, this->hash(input));
  input.f1_ref().reset();
  EXPECT_EQ(result, this->hash(input));
  input.f2_ref() = {};
  EXPECT_NE(result, this->hash(input));
  input.f2_ref().reset();
  EXPECT_EQ(result, this->hash(input));
  input.f3_ref() = {};
  EXPECT_NE(result, this->hash(input));
  input.f3_ref().reset();
  EXPECT_EQ(result, this->hash(input));
}

TYPED_TEST(DeterministicProtocolTest, checkOrderDeterminism) {
  const test::OrderedFields orderedFields;
  const auto orderedResult = this->hash(orderedFields);
  const test::UnorderedFields unorderedFields;
  const auto unorderedResult = this->hash(unorderedFields);
  EXPECT_EQ(orderedResult, unorderedResult);
}

TEST(HashTest, Struct) {
  test::OneOfEach input;
  auto result = hashStruct(input);
  auto previousResult = result;
  // 1
  input.myBool_ref() = !input.myBool_ref().value();
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 2
  input.myByte_ref() = input.myByte_ref().value() + 1;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 3
  input.myI16_ref() = input.myI16_ref().value() + 1;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 4
  input.myI32_ref() = input.myI32_ref().value() + 1;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 5
  input.myI64_ref() = input.myI64_ref().value() + 1;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 6
  input.myDouble_ref() = input.myDouble_ref().value() + 1.23;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 7
  input.myFloat_ref() = input.myFloat_ref().value() + 1.23;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 8
  input.myMap_ref() = {{"abc", 1}};
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 9
  input.myList_ref() = {"abc"};
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 10
  input.mySet_ref() = {"abc"};
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 11
  input.myStruct_ref().emplace();
  input.myStruct_ref()->mySubI64_ref() =
      input.myStruct_ref()->mySubI64_ref().value() + 1;
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  input.myStruct_ref()->mySubString_ref() =
      input.myStruct_ref()->mySubString_ref().value() + "a";
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
  previousResult = result;
  // 12
  input.myUnion_ref()->text_ref() = "abc";
  result = hashStruct(input);
  EXPECT_NE(previousResult, result);
}

} // namespace
} // namespace apache::thrift::op
