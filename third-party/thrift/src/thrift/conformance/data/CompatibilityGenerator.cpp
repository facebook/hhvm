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

#include <thrift/conformance/data/CompatibilityGenerator.h>

#include <cstdio>
#include <random>
#include <set>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <boost/mp11.hpp>
#include <fmt/core.h>
#include <folly/container/Foreach.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/conformance/data/internal/TestGenerator.h>
#include <thrift/conformance/if/gen-cpp2/test_suite_types.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/test/testset/Testset.h>
#include <thrift/test/testset/gen-cpp2/Enum_types.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

// TODO: use FieldQualifier
using apache::thrift::conformance::data::detail::
    genCompatibilityRoundTripTestCase;
using apache::thrift::conformance::data::detail::serializeThriftStruct;
using apache::thrift::test::testset::FieldModifier;
using apache::thrift::test::testset::detail::exception_ByFieldType;
using apache::thrift::test::testset::detail::mod_set;
using apache::thrift::test::testset::detail::struct_ByFieldType;
using apache::thrift::test::testset::detail::union_ByFieldType;
namespace mp11 = boost::mp11;

namespace apache::thrift::conformance::data {

namespace {

template <class TT>
[[nodiscard]] TestCase addFieldTestCase(const Protocol& protocol) {
  const typename struct_ByFieldType<TT, mod_set<>>::type def;
  return genCompatibilityRoundTripTestCase(
      protocol,
      fmt::format("testset.{}/AddField", type::getName<TT>()),
      protocol::Object{},
      def);
}

template <class TT>
[[nodiscard]] TestCase addFieldWithCustomDefaultTestCase(
    const Protocol& protocol) {
  const typename struct_ByFieldType<TT, mod_set<FieldModifier::CustomDefault>>::
      type def;
  return genCompatibilityRoundTripTestCase(
      protocol,
      fmt::format("testset.{}/AddFieldWithCustomDefault", type::getName<TT>()),
      protocol::Object{},
      def);
}

template <class TT>
[[nodiscard]] TestCase addOptionalFieldWithCustomDefaultTestCase(
    const Protocol& protocol) {
  const typename struct_ByFieldType<
      TT,
      mod_set<FieldModifier::Optional, FieldModifier::CustomDefault>>::type def;
  auto ret = genCompatibilityRoundTripTestCase(
      protocol,
      fmt::format(
          "testset.{}/AddOptionalFieldWithCustomDefault", type::getName<TT>()),
      def,
      def);
  // Optional field that's not in payload should always be empty, even if there
  // is custom default
  ret.test()->roundTrip_ref()->expectedResponse().ensure().value()->data() =
      *serializeThriftStruct(protocol::Object{}, protocol);
  return ret;
}

template <class TT>
[[nodiscard]] TestCase addTerseFieldWithCustomDefaultTestCase(
    const Protocol& protocol) {
  const typename struct_ByFieldType<
      TT,
      mod_set<FieldModifier::Terse, FieldModifier::CustomDefault>>::type def;
  auto ret = genCompatibilityRoundTripTestCase(
      protocol,
      fmt::format(
          "testset.{}/AddTerseFieldWithCustomDefault", type::getName<TT>()),
      protocol::Object{},
      def);
  // Since terse field can not distinguish from skipping serialization for field
  // and field is missing, it always clear to intrinsic default before
  // deserialization.
  ret.test()->roundTrip_ref()->expectedResponse().ensure().value()->data() =
      *serializeThriftStruct(protocol::Object{}, protocol);
  return ret;
}

template <class TT, bool IsOptional>
[[nodiscard]] TestCase changeFieldCustomDefaultTestCase(
    const Protocol& protocol) {
  using OldModSet = std::conditional_t<
      IsOptional,
      mod_set<FieldModifier::Optional, FieldModifier::CustomDefault>,
      mod_set<FieldModifier::CustomDefault>>;

  using NewModSet = std::conditional_t<
      IsOptional,
      mod_set<FieldModifier::Optional, FieldModifier::AlternativeCustomDefault>,
      mod_set<FieldModifier::AlternativeCustomDefault>>;

  const typename struct_ByFieldType<TT, OldModSet>::type oldData;
  const typename struct_ByFieldType<TT, NewModSet>::type newData;

  auto ret = genCompatibilityRoundTripTestCase(
      protocol,
      fmt::format(
          "testset.{}/Change{}FieldCustomDefault",
          type::getName<TT>(),
          IsOptional ? "Optional" : ""),
      oldData,
      newData);
  ret.test()->roundTrip_ref()->expectedResponse().ensure().value()->data() =
      *serializeThriftStruct(oldData, protocol);
  return ret;
}

template <class T>
[[nodiscard]] protocol::Object toObject(const T& t) {
  protocol::Value v;
  ::apache::thrift::protocol::detail::ObjectWriter writer{&v};
  t.write(&writer);
  return std::move(v.as_object());
}

template <class TT>
[[nodiscard]] std::vector<TestCase> removeFieldTestCase(
    const Protocol& protocol) {
  const typename struct_ByFieldType<TT, mod_set<>>::type def;
  std::vector<TestCase> ret;

  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    typename struct_ByFieldType<TT, mod_set<>>::type data;
    data.field_1() = value.value;

    protocol::Object obj = toObject(def);
    protocol::Object dataObj = toObject(data);

    int16_t idx = 1;
    for (auto&& i : *dataObj.members()) {
      // Add new field with non-existing field id
      while (obj.contains(FieldId{idx})) {
        idx++;
      }
      obj[FieldId{idx}] = i.second;
    }

    ret.push_back(genCompatibilityRoundTripTestCase(
        protocol,
        fmt::format(
            "testset.{}/RemoveField/{}", type::getName<TT>(), value.name),
        obj,
        def));
  }

  return ret;
}
constexpr auto alwaysReturnTrue = [](auto&&) { return true; };

template <
    class Old,
    class New,
    bool compatible,
    class ShouldTest = decltype(alwaysReturnTrue)>
[[nodiscard]] std::vector<TestCase> changeFieldTypeTestCase(
    const Protocol& protocol, ShouldTest shouldTest = alwaysReturnTrue) {
  static_assert(!std::is_same_v<Old, New>);

  std::vector<TestCase> ret;

  for (const auto& value : ValueGenerator<Old>::getInterestingValues()) {
    if (!shouldTest(value)) {
      continue;
    }

    typename struct_ByFieldType<Old, mod_set<FieldModifier::Optional>>::type
        old_data;
    typename struct_ByFieldType<New, mod_set<FieldModifier::Optional>>::type
        new_data;

    old_data.field_1() = value.value;

    if constexpr (compatible) {
      // If type change is compatible, new data will be deserialized as old data
      new_data.field_1() = static_cast<type::native_type<New>>(value.value);
    }

    ret.push_back(genCompatibilityRoundTripTestCase(
        protocol,
        fmt::format(
            "testset.{}.{}/ChangeFieldType/{}",
            type::getName<Old>(),
            type::getName<New>(),
            value.name),
        old_data,
        new_data));
  }

  return ret;
}

template <class T>
[[nodiscard]] const char* getQualifierName() {
  if (std::is_same_v<T, mod_set<>>) {
    return "Unqualified";
  }
  // TODO(ytj): use std::is_same_v<T, mod_set<type::FieldQualifier::Optional>>
  if (std::is_same_v<T, mod_set<FieldModifier::Optional>>) {
    return "Optional";
  }
  if (std::is_same_v<T, mod_set<FieldModifier::Required>>) {
    return "Required";
  }
  if (std::is_same_v<T, mod_set<FieldModifier::Terse>>) {
    return "Terse";
  }
  throw std::runtime_error("Unknown ModSet");
}

template <class Old, class New>
[[nodiscard]] std::vector<TestCase> changeQualifierTestCase(
    const Protocol& protocol) {
  using TypeTag = type::list<type::i32_t>;
  std::vector<TestCase> ret;

  for (const auto& value : ValueGenerator<TypeTag>::getInterestingValues()) {
    typename struct_ByFieldType<TypeTag, Old>::type old_data;
    typename struct_ByFieldType<TypeTag, New>::type new_data;

    old_data.field_1() = value.value;
    new_data.field_1() = value.value;

    ret.push_back(genCompatibilityRoundTripTestCase(
        protocol,
        fmt::format(
            "testset.{}.{}/ChangeQualifier/{}",
            getQualifierName<Old>(),
            getQualifierName<New>(),
            value.name),
        old_data,
        new_data));
  }

  return ret;
}

using test::testset::DifferentNameEnumStruct;
using test::testset::DifferentValueEnumStruct;
using test::testset::LessFieldEnumStruct;
using test::testset::NoZeroEnumStruct;
using test::testset::StandardEnumStruct;

template <class EnumStruct>
std::string getEnumStructName() {
  if constexpr (std::is_same_v<EnumStruct, StandardEnumStruct>) {
    return "Standard";
  } else if constexpr (std::is_same_v<EnumStruct, NoZeroEnumStruct>) {
    return "NoZero";
  } else if constexpr (std::is_same_v<EnumStruct, LessFieldEnumStruct>) {
    return "MissingField";
  } else if constexpr (std::is_same_v<EnumStruct, DifferentNameEnumStruct>) {
    return "NameMismatch";
  } else if constexpr (std::is_same_v<EnumStruct, DifferentValueEnumStruct>) {
    return "ValueMismatch";
  } else {
    static_assert(sizeof(EnumStruct) == 0);
  }
  return "";
}

template <class E>
void setEnum(E& e, std::underlying_type_t<E> i) {
  e = E(i);
}

template <class Old, class New>
[[nodiscard]] TestCase changeEnumValueTestCase(
    const Protocol& protocol,
    Old old_data,
    int32_t old_value,
    New new_data,
    int32_t new_value,
    std::string description) {
  setEnum(old_data.field().ensure(), old_value);
  setEnum(new_data.field().ensure(), new_value);
  auto name = fmt::format(
      "testset/ChangeEnumType/{}.{}.{}.{}",
      getEnumStructName<decltype(old_data)>(),
      old_value,
      getEnumStructName<decltype(new_data)>(),
      new_value);
  for (char& c : name) {
    if (c == '-') {
      c = '_';
    }
  }
  return genCompatibilityRoundTripTestCase(
      protocol, name, old_data, new_data, std::move(description));
}

[[nodiscard]] std::vector<TestCase> changeEnumValueTestCases(
    const Protocol& protocol) {
  std::vector<TestCase> ret;

  using Enums = std::tuple<
      StandardEnumStruct,
      NoZeroEnumStruct,
      LessFieldEnumStruct,
      DifferentNameEnumStruct,
      DifferentValueEnumStruct>;

  folly::for_each(Enums{}, [&](auto old_data) {
    folly::for_each(Enums{}, [&](auto new_data) {
      ret.push_back(changeEnumValueTestCase(
          protocol,
          old_data,
          0,
          new_data,
          0,
          "Testing `0` as enum value, it should not change after round-trip."));
      ret.push_back(changeEnumValueTestCase(
          protocol,
          old_data,
          1,
          new_data,
          1,
          "Testing non-zero as enum value, it should not change after round-trip."));
    });
  });

  // Remove field 0
  ret.push_back(changeEnumValueTestCase(
      protocol,
      StandardEnumStruct{},
      0,
      NoZeroEnumStruct{},
      0,
      "Test when removing `0` from Enum Struct. "
      "The old value should be retained."));

  // Remove field
  ret.push_back(changeEnumValueTestCase(
      protocol,
      StandardEnumStruct{},
      2,
      LessFieldEnumStruct{},
      2,
      "Test when removing non-zero enum value. "
      "The old value should be retained."));

  // Rename field
  ret.push_back(changeEnumValueTestCase(
      protocol,
      StandardEnumStruct{},
      2,
      DifferentNameEnumStruct{},
      2,
      "Test when renaming enum value. "
      "The old value should be retained."));

  // Change value
  ret.push_back(changeEnumValueTestCase(
      protocol,
      StandardEnumStruct{},
      2,
      DifferentValueEnumStruct{},
      2,
      "Test when changing enum value. "
      "The old value should be retained."));

  return ret;
}

template <class TT>
[[nodiscard]] std::vector<TestCase> changeStructType(const Protocol& protocol) {
  std::vector<TestCase> ret;
  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    typename struct_ByFieldType<TT, mod_set<>>::type s;
    typename union_ByFieldType<TT, mod_set<>>::type u;
    typename exception_ByFieldType<TT, mod_set<>>::type e;

    s.field_1() = value.value;
    u.field_1_ref() = value.value;
    e.field_1() = value.value;

    auto get_name = [&](const auto& i) -> std::string_view {
      if constexpr (std::is_same_v<
                        folly::remove_cvref_t<decltype(i)>,
                        folly::remove_cvref_t<decltype(s)>>) {
        return "Struct";
      } else if constexpr (std::is_same_v<
                               folly::remove_cvref_t<decltype(i)>,
                               folly::remove_cvref_t<decltype(u)>>) {
        return "Union";
      } else if constexpr (std::is_same_v<
                               folly::remove_cvref_t<decltype(i)>,
                               folly::remove_cvref_t<decltype(e)>>) {
        return "Exception";
      } else {
        static_assert(sizeof(i) == 0);
      }
    };

    folly::for_each(std::tuple(s, u, e), [&](auto old_data) {
      folly::for_each(std::tuple(s, u, e), [&](auto new_data) {
        if constexpr (std::is_same_v<decltype(old_data), decltype(new_data)>) {
          return;
        }

        ret.push_back(genCompatibilityRoundTripTestCase(
            protocol,
            fmt::format(
                "testset.{}/{}To{}/{}",
                type::getName<TT>(),
                get_name(old_data),
                get_name(new_data),
                value.name),
            old_data,
            new_data));
      });
    });
  }
  return ret;
}

template <class TT>
[[nodiscard]] std::vector<TestCase> switchSingularAndContainer(
    const Protocol& protocol) {
  std::vector<TestCase> ret;
  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    typename struct_ByFieldType<TT, mod_set<>>::type singular, defSingular;
    singular.field_1() = value.value;

    typename struct_ByFieldType<type::list<TT>, mod_set<>>::type container,
        defContainer;
    container.field_1()->push_back(value.value);

    ret.push_back(genCompatibilityRoundTripTestCase(
        protocol,
        fmt::format(
            "testset.{}/ChangeSingularToContainer/{}",
            type::getName<TT>(),
            value.name),
        singular,
        container));

    // Due to type mismatch, struct should have default initialized value after
    // deserialization.
    ret.back()
        .test()
        ->roundTrip_ref()
        ->expectedResponse()
        .ensure()
        .value()
        ->data() = *serializeThriftStruct(defContainer, protocol);

    ret.push_back(genCompatibilityRoundTripTestCase(
        protocol,
        fmt::format(
            "testset.{}/ChangeContainerToSingular/{}",
            type::getName<TT>(),
            value.name),
        container,
        singular));

    // Due to type mismatch, struct should have default initialized value after
    // deserialization.
    ret.back()
        .test()
        ->roundTrip_ref()
        ->expectedResponse()
        .ensure()
        .value()
        ->data() = *serializeThriftStruct(defSingular, protocol);
  }

  return ret;
}

template <typename TT>
Test createCompatibilityTestWithTypeTag(const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();
  test.tags()->emplace("spec/definition/compatibility/#data");

  auto addToTest = [&](std::vector<TestCase>&& tests) {
    for (auto& t : tests) {
      test.testCases()->push_back(std::move(t));
    }
  };

  addToTest({addFieldTestCase<TT>(protocol)});
  addToTest(removeFieldTestCase<TT>(protocol));
  addToTest({addFieldWithCustomDefaultTestCase<TT>(protocol)});
  addToTest({addOptionalFieldWithCustomDefaultTestCase<TT>(protocol)});
  addToTest({addTerseFieldWithCustomDefaultTestCase<TT>(protocol)});
  addToTest(changeStructType<TT>(protocol));
  addToTest(switchSingularAndContainer<TT>(protocol));

  addToTest({changeFieldCustomDefaultTestCase<TT, false>(protocol)});
  addToTest({changeFieldCustomDefaultTestCase<TT, true>(protocol)});

  return test;
}

Test createCompatibilityTest(const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();
  test.tags()->emplace("spec/definition/compatibility/#data");

  auto addToTest = [&](std::vector<TestCase>&& tests) {
    for (auto& t : tests) {
      test.testCases()->push_back(std::move(t));
    }
  };

  addToTest(changeFieldTypeTestCase<type::i32_t, type::i16_t, false>(protocol));
  addToTest(changeFieldTypeTestCase<type::i32_t, type::i64_t, false>(protocol));
  addToTest(
      changeFieldTypeTestCase<type::string_t, type::binary_t, true>(protocol));
  addToTest(changeFieldTypeTestCase<type::binary_t, type::string_t, true>(
      protocol, [](auto&& value) { return value.name != "bad_utf8"; }));
  addToTest(changeFieldTypeTestCase<type::binary_t, type::string_t, false>(
      protocol, [](auto&& value) { return value.name == "bad_utf8"; }));
  addToTest(changeFieldTypeTestCase<
            type::set<type::i64_t>,
            type::list<type::i64_t>,
            false>(protocol));
  addToTest(changeFieldTypeTestCase<
            type::list<type::i64_t>,
            type::set<type::i64_t>,
            false>(protocol));

  // TODO: Test change between enum and integer.

  addToTest(
      changeQualifierTestCase<mod_set<>, mod_set<FieldModifier::Optional>>(
          protocol));
  addToTest(
      changeQualifierTestCase<mod_set<>, mod_set<FieldModifier::Required>>(
          protocol));
  addToTest(
      changeQualifierTestCase<mod_set<FieldModifier::Optional>, mod_set<>>(
          protocol));
  addToTest(changeQualifierTestCase<
            mod_set<FieldModifier::Optional>,
            mod_set<FieldModifier::Required>>(protocol));
  addToTest(
      changeQualifierTestCase<mod_set<FieldModifier::Required>, mod_set<>>(
          protocol));
  addToTest(changeQualifierTestCase<
            mod_set<FieldModifier::Required>,
            mod_set<FieldModifier::Optional>>(protocol));
  addToTest(changeQualifierTestCase<mod_set<>, mod_set<FieldModifier::Terse>>(
      protocol));
  addToTest(changeQualifierTestCase<mod_set<FieldModifier::Terse>, mod_set<>>(
      protocol));
  addToTest(changeQualifierTestCase<
            mod_set<FieldModifier::Optional>,
            mod_set<FieldModifier::Terse>>(protocol));

  addToTest(changeEnumValueTestCases(protocol));

  return test;
}
} // namespace

TestSuite createCompatibilitySuite() {
  TestSuite suite;
  suite.name() = "CompatibilityTest";
  for (const auto& protocol : detail::toProtocols(detail::kDefaultProtocols)) {
    suite.tests()->emplace_back(createCompatibilityTest(protocol));
    mp11::mp_for_each<detail::PrimaryTypeTags>([&](auto t) {
      suite.tests()->emplace_back(
          createCompatibilityTestWithTypeTag<decltype(t)>(protocol));
    });
  }
  return suite;
}

} // namespace apache::thrift::conformance::data
