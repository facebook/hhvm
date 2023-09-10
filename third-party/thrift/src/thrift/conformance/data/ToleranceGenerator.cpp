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

#include <thrift/conformance/data/ToleranceGenerator.h>

#include <boost/mp11.hpp>
#include <fmt/core.h>
#include <glog/logging.h>

#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/conformance/data/internal/TestGenerator.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/test/testset/Testset.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

using apache::thrift::conformance::data::detail::
    genCompatibilityRoundTripTestCase;
using apache::thrift::test::testset::detail::mod_set;
using apache::thrift::test::testset::detail::union_ByFieldType;
namespace mp11 = boost::mp11;

namespace apache::thrift::conformance::data {

namespace {
template <class T>
[[nodiscard]] protocol::Object toObject(const T& t) {
  protocol::Value v;
  ::apache::thrift::protocol::detail::ObjectWriter writer{&v};
  t.write(&writer);
  return std::move(v.as_object());
}

template <class TT>
[[nodiscard]] std::vector<TestCase> multipleFieldsInUnionTestCase(
    const Protocol& protocol) {
  std::vector<TestCase> ret;

  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    typename union_ByFieldType<TT, mod_set<>>::type data;

    // Create a union object with both fields active
    data.field_1_ref() = value.value;
    protocol::Object obj = toObject(data);

    data.field_2_ref() = value.value;
    auto map = *toObject(data).members();
    obj.members()->insert(map.begin(), map.end());
    CHECK_EQ(obj.members()->size(), 2);

    auto testCase = genCompatibilityRoundTripTestCase(
        protocol,
        fmt::format(
            "testset.{}/MultipleFieldsInUnion/{}",
            type::getName<TT>(),
            value.name),
        obj,
        data);
    testCase.test()->roundTrip_ref()->expectException() = true;
    ret.push_back(std::move(testCase));
  }

  return ret;
}
template <typename TT>
Test createToleranceTest(const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();
  test.tags()->emplace("spec/definition/tolerance/");

  auto addToTest = [&](std::vector<TestCase>&& tests) {
    for (auto& t : tests) {
      test.testCases()->push_back(std::move(t));
    }
  };

  addToTest({multipleFieldsInUnionTestCase<TT>(protocol)});

  return test;
}
} // namespace

TestSuite createToleranceTestSuite() {
  TestSuite suite;
  suite.name() = "ThriftToleranceTest";
  for (const auto& protocol : detail::toProtocols(detail::kDefaultProtocols)) {
    mp11::mp_for_each<detail::PrimaryTypeTags>([&](auto t) {
      suite.tests()->emplace_back(createToleranceTest<decltype(t)>(protocol));
    });
  }
  return suite;
}

} // namespace apache::thrift::conformance::data
