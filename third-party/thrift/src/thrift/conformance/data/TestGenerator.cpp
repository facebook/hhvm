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

#include <thrift/conformance/cpp2/Object.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/test/testset/Testset.h>

namespace mp11 = boost::mp11;

namespace apache::thrift::conformance::data {

namespace {

template <typename TT>
Test createRoundTripTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  using namespace apache::thrift::test::testset::detail;

  Test test;
  test.name() = protocol.name();
  test.tags()->emplace(getSpecDefinitionTag<TT>());
  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    // Test case #1: Use ValueStruct
    RoundTripTestCase roundTrip;
    roundTrip.request()->value() =
        registry.store(asValueStruct<TT>(value.value), protocol);

    auto& testCase1 = test.testCases()->emplace_back();
    testCase1.name() = fmt::format("{}/{}", type::getName<TT>(), value.name);
    testCase1.test()->set_roundTrip(roundTrip);

    // Test case #2: Use Testset
    typename struct_ByFieldType<TT, mod_set<>>::type data;
    data.field_1() = value.value;
    roundTrip.request()->value() = registry.store(data, protocol);
    auto& testCase2 = test.testCases()->emplace_back();
    testCase2.name() =
        fmt::format("testset.{}/{}", type::getName<TT>(), value.name);
    testCase2.test()->set_roundTrip(roundTrip);
  }

  return test;
}

void addRoundTripToSuite(
    const AnyRegistry& registry, const Protocol& protocol, TestSuite& suite) {
  mp11::mp_for_each<detail::PrimaryTypeTags>([&](auto tt) {
    suite.tests()->emplace_back(
        createRoundTripTest<decltype(tt)>(registry, protocol));
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
