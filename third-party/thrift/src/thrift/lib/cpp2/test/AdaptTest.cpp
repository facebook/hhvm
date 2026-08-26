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

#include <thrift/lib/cpp2/Adapter.h>

#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <type_traits>
#include <unordered_set>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/CPortability.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::test {

enum class OpType { Adapted, Adapter };

std::ostream& operator<<(std::ostream& os, OpType op_type) {
  switch (op_type) {
    case OpType::Adapted:
      return os << "OpType::Adapted";
    case OpType::Adapter:
      return os << "OpType::Adapter";
  }
  return os << "{Unknown}";
}

struct OpTracker {
  MOCK_METHOD(void, equal, (OpType));
  MOCK_METHOD(void, less, (OpType));
  MOCK_METHOD(void, hash, (OpType));
  MOCK_METHOD(void, toThrift, ());
  MOCK_METHOD(void, fromThrift, ());
};

struct MinType {
  OpTracker* tracker;
  int value;
};

struct FullType {
  OpTracker* tracker;
  int value;

 private:
  friend bool operator==(const FullType& lhs, const FullType& rhs) {
    lhs.tracker->equal(OpType::Adapted);
    return lhs.value == rhs.value;
  }
  friend bool operator<(const FullType& lhs, const FullType& rhs) {
    lhs.tracker->less(OpType::Adapted);
    return lhs.value < rhs.value;
  }
};

} // namespace apache::thrift::test

namespace std {
template <>
struct hash<apache::thrift::test::FullType> {
  std::size_t operator()(
      const apache::thrift::test::FullType& val) const noexcept {
    val.tracker->hash(apache::thrift::test::OpType::Adapted);
    return val.value + 1;
  }
};
} // namespace std

namespace apache::thrift::test {
namespace {

struct MinAdapter {
  static OpTracker* tracker;

  template <typename T>
  static auto& toThrift(const T& val) {
    tracker->toThrift();
    return val.value;
  }

  template <typename T>
  static T fromThrift(int value) {
    tracker->fromThrift();
    return {tracker, value};
  }
};

OpTracker* MinAdapter::tracker = nullptr;

struct FullAdapter : MinAdapter {
  template <typename T>
  static bool equal(const T& lhs, const T& rhs) {
    tracker->equal(OpType::Adapter);
    return lhs.value == rhs.value;
  }

  template <typename T>
  static bool less(const T& lhs, const T& rhs) {
    tracker->less(OpType::Adapter);
    return lhs.value < rhs.value;
  }

  template <typename T>
  static size_t hash(const T& value) {
    tracker->hash(OpType::Adapter);
    return value.value + 2;
  }
};

// These declarations are intentionally undefined and used only in unevaluated
// concept checks.
FOLLY_PUSH_WARNING
FOLLY_CLANG_DISABLE_WARNING("-Wunused-member-function")
FOLLY_CLANG_DISABLE_WARNING("-Wunneeded-member-function")

struct ConceptStruct {};
struct ConceptProtocol {};
struct OtherConceptProtocol {};

struct ConceptAdapter {
  template <typename = void>
  static FullType fromThrift(int);
  template <typename = void>
  static int toThrift(const FullType&);
  template <typename = void>
  static int& toThrift(FullType&);

  template <typename = void>
  static void clear(FullType&);
  template <typename = void>
  static bool isEmpty(const FullType&);
  template <typename = void>
  static bool equal(const FullType&, const FullType&);
  template <typename = void>
  static bool less(const FullType&, const FullType&);
  template <typename = void>
  static folly::ordering compareThreeWay(const FullType&, const FullType&);
  template <typename = void>
  static size_t hash(const FullType&);
  template <typename = void>
  static void construct(FullType&, FieldContext<ConceptStruct, 1>);

  template <bool, typename>
  static uint32_t serializedSize(ConceptProtocol&, const FullType&);

  template <typename>
  static uint32_t encode(ConceptProtocol&, const FullType&);

  template <typename>
  static void decode(ConceptProtocol&, FullType&);
};

struct ConceptFieldAdapter {
  template <typename Struct, int16_t FieldId>
  static FullType fromThriftField(int, FieldContext<Struct, FieldId>);
  template <typename = void>
  static int toThrift(const FullType&);
  template <typename = void>
  static int& toThrift(FullType&);

  template <typename = void>
  static void clear(FullType&);
  template <typename = void>
  static bool isEmpty(const FullType&);
  template <typename = void>
  static bool equal(const FullType&, const FullType&);
  template <typename = void>
  static bool less(const FullType&, const FullType&);
  template <typename = void>
  static folly::ordering compareThreeWay(const FullType&, const FullType&);
  template <typename = void>
  static size_t hash(const FullType&);
  template <typename Struct, int16_t FieldId>
  static void construct(FullType&, FieldContext<Struct, FieldId>);

  template <bool, typename>
  static uint32_t serializedSize(ConceptProtocol&, const FullType&);

  template <typename>
  static uint32_t encode(ConceptProtocol&, const FullType&);

  template <typename>
  static void decode(ConceptProtocol&, FullType&);
};

struct FieldIdSpecificAdapter {
  template <typename = void>
  static FullType fromThriftField(int, FieldContext<ConceptStruct, 1>);
  template <typename = void>
  static int toThrift(const FullType&);
};

struct FieldAdapterNoToThrift {
  template <typename Struct, int16_t FieldId>
  static FullType fromThriftField(int, FieldContext<Struct, FieldId>);
};

struct IncompleteField;
struct IncompleteStruct;

template <typename T, typename Struct, int16_t FieldId>
struct CompleteOnlyField {
  T value;
};

struct IncompleteTypeFieldAdapter {
  template <typename T, typename Struct, int16_t FieldId>
  static CompleteOnlyField<T, Struct, FieldId> fromThriftField(
      T&&, FieldContext<Struct, FieldId>&&);

  template <typename T>
  static const T& toThrift(const T&);
};

struct TrueZeroCopyAdapter : ConceptAdapter {
  template <bool ZeroCopy, typename>
    requires(ZeroCopy)
  static uint32_t serializedSize(ConceptProtocol&, const FullType&);
};

struct FromThriftOnlyAdapter {
  static FullType fromThrift(int value) { return {nullptr, value}; }
};

struct NonSemiregularType {
  NonSemiregularType() = default;
  NonSemiregularType(const NonSemiregularType&) = delete;
  NonSemiregularType(NonSemiregularType&&) = default;
  NonSemiregularType& operator=(const NonSemiregularType&) = delete;
  NonSemiregularType& operator=(NonSemiregularType&&) = default;
};

struct NonSemiregularTypeAdapter {
  static NonSemiregularType fromThrift(int);
  static int toThrift(const NonSemiregularType&);
};

struct MoveOnlyThriftAdapter {
  static std::unique_ptr<int> fromThrift(std::unique_ptr<int>);
  static std::unique_ptr<int> toThrift(const std::unique_ptr<int>&);
};

struct NonSemiregularFieldAdapter {
  template <typename Struct, int16_t FieldId>
  static NonSemiregularType fromThriftField(int, FieldContext<Struct, FieldId>);
  static int toThrift(const NonSemiregularType&);
};

struct MutableLessAdapter : ConceptAdapter {
  template <typename = void>
  static bool less(FullType&, const FullType&);
};

struct ByValueAdapter {
  template <typename = void>
  static FullType fromThrift(int);
  template <typename = void>
  static int toThrift(const FullType&);
};

struct CustomizationOnlyAdapter {
  template <typename = void>
  static int toThrift(const FullType&);
  template <typename = void>
  static int& toThrift(FullType&);

  template <typename = void>
  static void clear(FullType&);
  template <typename = void>
  static bool isEmpty(const FullType&);
  template <typename = void>
  static bool equal(const FullType&, const FullType&);
  template <typename = void>
  static bool less(const FullType&, const FullType&);
  template <typename = void>
  static folly::ordering compareThreeWay(const FullType&, const FullType&);
  template <typename = void>
  static size_t hash(const FullType&);
  template <typename = void>
  static void construct(FullType&, FieldContext<ConceptStruct, 1>);

  template <bool, typename>
  static uint32_t serializedSize(ConceptProtocol&, const FullType&);

  template <typename>
  static uint32_t encode(ConceptProtocol&, const FullType&);

  template <typename>
  static void decode(ConceptProtocol&, FullType&);
};

static_assert(ThriftTypeAdapter<ConceptAdapter, int>);
static_assert(ThriftConstAdapter<ConceptAdapter, int>);
static_assert(ThriftAdapter<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(!ThriftTypeAdapter<FromThriftOnlyAdapter, int>);
static_assert(ThriftConstAdapter<FromThriftOnlyAdapter, int>);
static_assert(!ThriftAdapter<FromThriftOnlyAdapter, int, ConceptStruct, 1>);
static_assert(adapt_detail::TypeAdapter<NonSemiregularTypeAdapter, int>);
static_assert(!ThriftTypeAdapter<NonSemiregularTypeAdapter, int>);
static_assert(ThriftTypeAdapter<MoveOnlyThriftAdapter, std::unique_ptr<int>>);
static_assert(
    adapt_detail::
        FieldAdapter<NonSemiregularFieldAdapter, 1, int, ConceptStruct>);
static_assert(
    !ThriftFieldAdapter<NonSemiregularFieldAdapter, int, ConceptStruct, 1>);
static_assert(
    !adapt_detail::FieldAdapter<FromThriftOnlyAdapter, 1, int, ConceptStruct>);
static_assert(std::is_same_v<
              adapt_detail::
                  adapted_field_t<FromThriftOnlyAdapter, 1, int, ConceptStruct>,
              FullType>);
static_assert(ThriftFieldAdapter<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(ThriftAdapter<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(
    ThriftFieldAdapter<FieldIdSpecificAdapter, int, ConceptStruct, 1>);
static_assert(
    !ThriftFieldAdapter<FieldIdSpecificAdapter, int, ConceptStruct, 2>);
static_assert(ThriftAdapter<FieldIdSpecificAdapter, int, ConceptStruct, 1>);
static_assert(!ThriftAdapter<FieldIdSpecificAdapter, int, ConceptStruct, 2>);
static_assert(
    adapt_detail::FieldAdapter<FieldAdapterNoToThrift, 1, int, ConceptStruct>);
static_assert(
    !ThriftFieldAdapter<FieldAdapterNoToThrift, int, ConceptStruct, 1>);
static_assert(!ThriftTypeAdapter<ConceptFieldAdapter, int>);
static_assert(AdapterWithConstruct<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithClear<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithIsEmpty<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithEqual<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithLess<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(!AdapterWithLess<MutableLessAdapter, int, ConceptStruct, 1>);
static_assert(
    AdapterWithCompareThreeWay<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithHash<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithSerializedSize<
              ConceptAdapter,
              int,
              ConceptStruct,
              1,
              false,
              type::i32_t,
              ConceptProtocol>);
static_assert(!AdapterWithSerializedSize<
              ConceptAdapter,
              int,
              ConceptStruct,
              1,
              false,
              type::i32_t,
              OtherConceptProtocol>);
static_assert(AdapterWithSerializedSize<
              TrueZeroCopyAdapter,
              int,
              ConceptStruct,
              1,
              true,
              type::i32_t,
              ConceptProtocol>);
static_assert(!AdapterWithSerializedSize<
              TrueZeroCopyAdapter,
              int,
              ConceptStruct,
              1,
              false,
              type::i32_t,
              ConceptProtocol>);
static_assert(AdapterWithEncode<
              ConceptAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              ConceptProtocol>);
static_assert(!AdapterWithEncode<
              ConceptAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              OtherConceptProtocol>);
static_assert(AdapterWithDecode<
              ConceptAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              ConceptProtocol>);
static_assert(!AdapterWithDecode<
              ConceptAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              OtherConceptProtocol>);
static_assert(InplaceThriftAdapter<ConceptAdapter, int, ConceptStruct, 1>);
static_assert(ThriftTypeAdapter<ByValueAdapter, int>);
static_assert(!InplaceThriftAdapter<ByValueAdapter, int, ConceptStruct, 1>);

static_assert(AdapterWithConstruct<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithClear<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithIsEmpty<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithEqual<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithLess<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(
    AdapterWithCompareThreeWay<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithHash<ConceptFieldAdapter, int, ConceptStruct, 1>);
static_assert(AdapterWithSerializedSize<
              ConceptFieldAdapter,
              int,
              ConceptStruct,
              1,
              false,
              type::i32_t,
              ConceptProtocol>);
static_assert(AdapterWithEncode<
              ConceptFieldAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              ConceptProtocol>);
static_assert(AdapterWithDecode<
              ConceptFieldAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              ConceptProtocol>);
static_assert(InplaceThriftAdapter<ConceptFieldAdapter, int, ConceptStruct, 1>);

static_assert(!ThriftTypeAdapter<CustomizationOnlyAdapter, int>);
static_assert(!ThriftConstAdapter<CustomizationOnlyAdapter, int>);
static_assert(!ThriftAdapter<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(
    !AdapterWithConstruct<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(
    !AdapterWithClear<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(
    !AdapterWithIsEmpty<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(
    !AdapterWithEqual<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(
    !AdapterWithLess<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(!AdapterWithCompareThreeWay<
              CustomizationOnlyAdapter,
              int,
              ConceptStruct,
              1>);
static_assert(
    !AdapterWithHash<CustomizationOnlyAdapter, int, ConceptStruct, 1>);
static_assert(!AdapterWithSerializedSize<
              CustomizationOnlyAdapter,
              int,
              ConceptStruct,
              1,
              false,
              type::i32_t,
              ConceptProtocol>);
static_assert(!AdapterWithEncode<
              CustomizationOnlyAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              ConceptProtocol>);
static_assert(!AdapterWithDecode<
              CustomizationOnlyAdapter,
              int,
              ConceptStruct,
              1,
              type::i32_t,
              ConceptProtocol>);
static_assert(
    !InplaceThriftAdapter<CustomizationOnlyAdapter, int, ConceptStruct, 1>);

FOLLY_POP_WARNING

class AdaptTest : public ::testing::Test {
 protected:
  testing::StrictMock<OpTracker> tracker_;

  void SetUp() override { MinAdapter::tracker = &tracker_; }
  void TearDown() override { MinAdapter::tracker = nullptr; }

  template <typename T = MinType>
  T val(int value) {
    return T{&tracker_, value};
  }

  template <typename Adapter, typename Adapted>
  bool equal(int lhs, int rhs) {
    const auto lhsVal = val<Adapted>(lhs);
    const auto rhsVal = val<Adapted>(rhs);
    bool result = adapt_detail::equal<Adapter>(lhsVal, rhsVal);
    EXPECT_EQ(
        adapt_detail::equal_opt<Adapter>(
            std::make_optional(lhsVal), std::make_optional(rhsVal)),
        result);
    EXPECT_FALSE(
        adapt_detail::equal_opt<Adapter>(std::make_optional(lhsVal), {}));
    EXPECT_FALSE(
        adapt_detail::equal_opt<Adapter>({}, std::make_optional(rhsVal)));
    EXPECT_TRUE(
        (adapt_detail::equal_opt<Adapter, std::optional<Adapted>>({}, {})));
    return result;
  }

  template <typename Adapter, typename Adapted>
  bool notEqual(int lhs, int rhs) {
    const auto lhsVal = val<Adapted>(lhs);
    const auto rhsVal = val<Adapted>(rhs);
    bool result = adapt_detail::not_equal<Adapter>(lhsVal, rhsVal);
    EXPECT_EQ(
        adapt_detail::not_equal_opt<Adapter>(
            std::make_optional(lhsVal), std::make_optional(rhsVal)),
        result);
    EXPECT_TRUE(
        adapt_detail::not_equal_opt<Adapter>(std::make_optional(lhsVal), {}));
    EXPECT_TRUE(
        adapt_detail::not_equal_opt<Adapter>({}, std::make_optional(rhsVal)));
    EXPECT_FALSE(
        (adapt_detail::not_equal_opt<Adapter, std::optional<Adapted>>({}, {})));
    return result;
    ;
  }

  template <typename Adapter, typename Adapted>
  bool less(int lhs, int rhs) {
    const auto lhsVal = val<Adapted>(lhs);
    const auto rhsVal = val<Adapted>(rhs);
    bool result = adapt_detail::less<Adapter>(lhsVal, rhsVal);
    if (adapt_detail::not_equal<Adapter>(lhsVal, rhsVal)) {
      EXPECT_EQ(
          adapt_detail::neq_less_opt<Adapter>(
              std::make_optional(lhsVal), std::make_optional(rhsVal)),
          result);
      EXPECT_FALSE(
          adapt_detail::neq_less_opt<Adapter>(std::make_optional(lhsVal), {}));
      EXPECT_TRUE(
          adapt_detail::neq_less_opt<Adapter>({}, std::make_optional(rhsVal)));
    }
    return result;
  }

  template <typename Adapter, typename Adapted>
  size_t hash(int value) {
    return adapt_detail::hash<Adapter>(val<Adapted>(value));
  }
};

TEST_F(AdaptTest, Equal) {
  // Uses the thrift type.
  EXPECT_CALL(tracker_, toThrift()).Times(::testing::AtLeast(1));
  EXPECT_TRUE((equal<MinAdapter, MinType>(1, 1)));
  EXPECT_FALSE((equal<MinAdapter, MinType>(1, 2)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses operator==()
  EXPECT_CALL(tracker_, equal(OpType::Adapted)).Times(::testing::AtLeast(1));
  EXPECT_TRUE((equal<MinAdapter, FullType>(1, 1)));
  EXPECT_FALSE((equal<MinAdapter, FullType>(1, 2)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses adapter.
  EXPECT_CALL(tracker_, equal(OpType::Adapter)).Times(::testing::AtLeast(1));
  EXPECT_TRUE((equal<FullAdapter, FullType>(1, 1)));
  EXPECT_FALSE((equal<FullAdapter, FullType>(1, 2)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);
}

TEST_F(AdaptTest, FromThriftFieldFallsBackToFromThrift) {
  ConceptStruct object;
  EXPECT_EQ(
      (adapt_detail::fromThriftField<FromThriftOnlyAdapter, 1>(42, object)
           .value),
      42);
}

TEST_F(AdaptTest, NotEqual) {
  // Uses !operator==()
  EXPECT_CALL(tracker_, equal(OpType::Adapted)).Times(::testing::AtLeast(1));
  EXPECT_FALSE((notEqual<MinAdapter, FullType>(1, 1)));
  EXPECT_TRUE((notEqual<MinAdapter, FullType>(1, 2)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);
}

TEST_F(AdaptTest, Less) {
  // Uses the thrift type.
  EXPECT_CALL(tracker_, toThrift()).Times(::testing::AtLeast(1));
  EXPECT_TRUE((less<MinAdapter, MinType>(1, 2)));
  EXPECT_FALSE((less<MinAdapter, MinType>(1, 1)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses operator<()
  EXPECT_CALL(tracker_, less(OpType::Adapted)).Times(::testing::AtLeast(1));
  EXPECT_CALL(tracker_, equal(OpType::Adapted)).Times(::testing::AnyNumber());
  EXPECT_TRUE((less<MinAdapter, FullType>(1, 2)));
  EXPECT_FALSE((less<MinAdapter, FullType>(1, 1)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses adapter.
  EXPECT_CALL(tracker_, less(OpType::Adapter)).Times(::testing::AtLeast(1));
  EXPECT_CALL(tracker_, equal(OpType::Adapter)).Times(::testing::AnyNumber());
  EXPECT_TRUE((less<FullAdapter, FullType>(1, 2)));
  EXPECT_FALSE((less<FullAdapter, FullType>(1, 1)));
  testing::Mock::VerifyAndClearExpectations(&tracker_);
}

TEST_F(AdaptTest, Hash) {
  // Validate our assumptions about hashing behavior.
  EXPECT_EQ(std::hash<int>()(1), 1);
  EXPECT_CALL(tracker_, hash(OpType::Adapted)).Times(1);
  EXPECT_EQ(std::hash<FullType>()(val<FullType>(1)), 2);
  testing::Mock::VerifyAndClearExpectations(&tracker_);
  EXPECT_CALL(tracker_, hash(OpType::Adapter)).Times(1);
  EXPECT_EQ(FullAdapter::hash(val<MinType>(1)), 3);
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses std::hash<thrift type>.
  EXPECT_CALL(tracker_, toThrift()).Times(2);
  EXPECT_EQ((hash<MinAdapter, MinType>(1)), 1);
  EXPECT_EQ((hash<MinAdapter, MinType>(2)), 2);
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses std::hash<Adapted>() (which adds 1)
  EXPECT_CALL(tracker_, hash(OpType::Adapted)).Times(2);
  EXPECT_EQ((hash<MinAdapter, FullType>(1)), 2);
  EXPECT_EQ((hash<MinAdapter, FullType>(2)), 3);
  testing::Mock::VerifyAndClearExpectations(&tracker_);

  // Uses adpater (which adds 2)
  EXPECT_CALL(tracker_, hash(OpType::Adapter)).Times(2);
  EXPECT_EQ((hash<FullAdapter, FullType>(1)), 3);
  EXPECT_EQ((hash<FullAdapter, FullType>(2)), 4);
  testing::Mock::VerifyAndClearExpectations(&tracker_);
}

TEST_F(AdaptTest, Set) {
  EXPECT_CALL(tracker_, toThrift()).Times(::testing::AtLeast(1));

  std::set<MinType, adapt_detail::adapter_less<MinAdapter, MinType>> my_set;
  my_set.emplace(val(1));
  my_set.emplace(val(3));
  EXPECT_EQ(my_set.size(), 2);
  EXPECT_TRUE(my_set.find(val(0)) == my_set.end());
  EXPECT_TRUE(my_set.find(val(1)) != my_set.end());
  EXPECT_TRUE(my_set.find(val(2)) == my_set.end());
  EXPECT_TRUE(my_set.find(val(3)) != my_set.end());
  EXPECT_TRUE(my_set.find(val(4)) == my_set.end());
}

TEST_F(AdaptTest, UnorderedSet) {
  EXPECT_CALL(tracker_, toThrift()).Times(::testing::AtLeast(1));

  std::unordered_set<
      MinType,
      adapt_detail::adapter_hash<MinAdapter, MinType>,
      adapt_detail::adapter_equal<MinAdapter, MinType>>
      my_set;
  my_set.emplace(val(1));
  my_set.emplace(val(3));
  EXPECT_EQ(my_set.size(), 2);
  EXPECT_TRUE(my_set.find(val(0)) == my_set.end());
  EXPECT_TRUE(my_set.find(val(1)) != my_set.end());
  EXPECT_TRUE(my_set.find(val(2)) == my_set.end());
  EXPECT_TRUE(my_set.find(val(3)) != my_set.end());
  EXPECT_TRUE(my_set.find(val(4)) == my_set.end());
}

TEST_F(AdaptTest, AdaptSetKey_Ordered) {
  static_assert(
      std::is_same_v<
          std::set<int, adapt_detail::adapted_less<test::TestAdapter, int>>,
          adapt_detail::adapt_set_key_t<test::TestAdapter, std::set<int32_t>>>);
}

TEST_F(AdaptTest, AdaptSetKey_Unordered) {
  static_assert(
      std::is_same_v<
          std::unordered_set<
              int,
              adapt_detail::adapted_hash<test::TestAdapter, int>,
              adapt_detail::
                  adapted_equal<apache::thrift::test::TestAdapter, int>>,
          adapt_detail::
              adapt_set_key_t<test::TestAdapter, std::unordered_set<int32_t>>>);
}

TEST_F(AdaptTest, AdpatMapKey_Ordered) {
  static_assert(std::is_same_v<
                std::map<
                    int,
                    std::string,
                    adapt_detail::adapted_less<test::TestAdapter, int>>,
                adapt_detail::adapt_map_key_t<
                    test::TestAdapter,
                    std::map<int32_t, std::string>>>);
}

TEST_F(AdaptTest, AdpatMapKey_Unordered) {
  static_assert(std::is_same_v<
                std::unordered_map<
                    int,
                    std::string,
                    adapt_detail::adapted_hash<test::TestAdapter, int>,
                    adapt_detail::adapted_equal<test::TestAdapter, int>>,
                adapt_detail::adapt_map_key_t<
                    test::TestAdapter,
                    std::unordered_map<int32_t, std::string>>>);
}

} // namespace
} // namespace apache::thrift::test
