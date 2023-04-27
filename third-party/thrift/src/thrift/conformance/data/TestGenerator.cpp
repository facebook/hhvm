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

#include <thrift/conformance/data/TestGenerator.h>

#include <boost/mp11.hpp>
#include <fmt/core.h>

#include <thrift/conformance/data/TestUtil.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/test/testset/Testset.h>

#include <thrift/test/AdapterTest.h>

namespace mp11 = boost::mp11;
using apache::thrift::protocol::asValueStruct;
using apache::thrift::test::testset::FieldModifier;

namespace apache::thrift::conformance::data {

namespace {

template <typename ElementTag, bool is_adapted>
Test createRoundTripTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  using namespace apache::thrift::test::testset::detail;

  const std::string& typeName = getTestTypeName<ElementTag>();
  Test test;
  test.name() = protocol.name();
  test.tags()->emplace(getSpecDefinitionTag<ElementTag>());
  for (const auto& value : ValueGenerator<ElementTag>::getInterestingValues()) {
    RoundTripTestCase roundTrip;
    // asValueStruct doesn't work with adapted values
    if constexpr (!is_adapted) {
      // Test case #1: Use ValueStruct
      roundTrip.request()->value() =
          registry.store(asValueStruct<ElementTag>(value.value), protocol);

      auto& testCase = test.testCases()->emplace_back();
      testCase.name() = fmt::format("{}/{}", typeName, value.name);
      testCase.test()->roundTrip_ref() = roundTrip;
    }

    auto addTestCase = [&](auto data, auto msg) {
      roundTrip.request()->value() = registry.store(data, protocol);
      auto& testCase = test.testCases()->emplace_back();
      testCase.name() = msg;
      testCase.test()->roundTrip_ref() = roundTrip;
    };

    auto addStruct = [&](auto modSet, auto msg) {
      using ModSet = decltype(modSet);
      typename struct_ByFieldType<ElementTag, ModSet>::type data;
      if constexpr (
          std::is_same_v<mod_set<FieldModifier::Adapter>, ModSet> ||
          std::is_same_v<mod_set<FieldModifier::FieldAdapter>, ModSet>) {
        data.field_1().ensure().value =
            type::native_type<ElementTag>{value.value};
      } else {
        data.field_1() = type::native_type<ElementTag>{value.value};
      }
      addTestCase(
          data, fmt::format("testset.{}{}/{}", msg, typeName, value.name));
    };

    // Test case #2: Unqualified field
    addStruct(mod_set<>{}, "");

    typename union_ByFieldType<ElementTag, mod_set<>>::type unionData;
    unionData.field_1_ref() = type::native_type<ElementTag>{value.value};
    addTestCase(
        unionData, fmt::format("testset.union.{}/{}", typeName, value.name));

    // these aren't tested with adapter
    if constexpr (!is_adapted) {
      // Test case #3: Optional field
      addStruct(mod_set<FieldModifier::Optional>{}, "Optional.");

      addTestCase(
          typename struct_ByFieldType<
              ElementTag,
              mod_set<FieldModifier::Optional>>::type{},
          fmt::format("testset.Optional.{}/empty_optional", typeName));

      // Test case #4: Terse field
      addStruct(mod_set<FieldModifier::Terse>{}, "Terse.");
    }

    // Test case #5: Adapted field
    addStruct(mod_set<FieldModifier::Adapter>{}, "Adapted.");

    // Test case #6: Adapted field with a Field Adapter
    addStruct(mod_set<FieldModifier::FieldAdapter>{}, "FieldAdapted.");
  }

  return test;
}

template <typename Tag>
using TypeAdapter =
    type::adapted<::apache::thrift::test::TemplatedTestAdapter, Tag>;

void addRoundTripToSuite(
    const AnyRegistry& registry, const Protocol& protocol, TestSuite& suite) {
  mp11::mp_for_each<detail::PrimaryTypeTags>([&](auto element_tag) {
    using ElementTag = decltype(element_tag);
    suite.tests()->emplace_back(
        createRoundTripTest<ElementTag, false>(registry, protocol));
    suite.tests()->emplace_back(
        createRoundTripTest<TypeAdapter<ElementTag>, true>(registry, protocol));
    if constexpr (!std::is_same_v<ElementTag, type::bool_t>) {
      suite.tests()->emplace_back(
          createRoundTripTest<type::list<ElementTag>, false>(
              registry, protocol));
      suite.tests()->emplace_back(
          createRoundTripTest<type::list<TypeAdapter<ElementTag>>, true>(
              registry, protocol));
    }
    mp11::mp_for_each<detail::KeyTypeTags>([&](auto key_tag) {
      using KeyTag = decltype(key_tag);
      suite.tests()->emplace_back(
          createRoundTripTest<type::map<KeyTag, ElementTag>, false>(
              registry, protocol));
      suite.tests()->emplace_back(
          createRoundTripTest<
              type::map<TypeAdapter<KeyTag>, TypeAdapter<ElementTag>>,
              true>(registry, protocol));
    });
  });
  mp11::mp_for_each<detail::KeyTypeTags>([&](auto key_tag) {
    using KeyTag = decltype(key_tag);
    suite.tests()->emplace_back(
        createRoundTripTest<type::set<KeyTag>, false>(registry, protocol));
    suite.tests()->emplace_back(
        createRoundTripTest<type::set<TypeAdapter<KeyTag>>, true>(
            registry, protocol));
  });
}

} // namespace

TestSuite createRoundTripSuite(
    const std::set<Protocol>& protocols, const AnyRegistry& registry) {
  TestSuite suite;
  suite.name() = "RoundTripTest";
  for (const auto& protocol : protocols) {
    addRoundTripToSuite(registry, protocol, suite);
  }
  return suite;
}

} // namespace apache::thrift::conformance::data
