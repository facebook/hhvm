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

#include <limits>
#include <type_traits>
#include <thrift/conformance/data/PatchGenerator.h>

#include <boost/mp11.hpp>
#include <fmt/core.h>

#include <thrift/conformance/cpp2/Object.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types_custom_protocol.h>

namespace apache::thrift::conformance::data {

namespace mp11 = boost::mp11;

namespace {

template <typename TT, typename GeneratedValue>
auto toValue(const GeneratedValue& value) {
  if constexpr (std::is_same_v<TT, type::binary_t>) {
    return *folly::IOBuf::copyBuffer(value.data());
  } else {
    return value;
  }
}

template <typename TT, typename GeneratedValue>
PatchOpTestCase makeAssignTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol) {
  PatchOpTestCase opTest;
  auto initialValue = value.value;
  op::clear<TT>(initialValue);
  opTest.value() = registry.store(asValueStruct<TT>(initialValue), protocol);

  auto patch = op::patch_type<TT>();
  patch = toValue<TT>(value.value);
  opTest.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);
  opTest.result() = registry.store(asValueStruct<TT>(value.value), protocol);
  return opTest;
}

template <typename TT, typename GeneratedValue>
PatchOpTestCase makeClearTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol) {
  PatchOpTestCase opTest;
  opTest.value() = registry.store(asValueStruct<TT>(value.value), protocol);

  auto patch = op::patch_type<TT>();
  patch.reset();
  opTest.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);

  auto clearValue = value.value;
  op::clear<TT>(clearValue);
  opTest.result() = registry.store(asValueStruct<TT>(clearValue), protocol);
  return opTest;
}

template <typename TT, typename T>
T makeAddExpectedResult(T value, int add = 1) {
  if constexpr (std::is_convertible_v<TT, type::bool_t>) {
    return !value;
  } else {
    if (std::numeric_limits<T>::max() == value && add > 0) {
      return value;
    } else if (std::numeric_limits<T>::min() == value && add < 0) {
      return value;
    }
    return value + add;
  }
}

template <typename TT, typename GeneratedValue>
PatchOpTestCase makeAddTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol) {
  PatchOpTestCase opTest;
  opTest.value() = registry.store(asValueStruct<TT>(value.value), protocol);

  auto addOp = [&](auto& patch, int toAdd = 1) {
    if constexpr (std::is_same_v<TT, type::bool_t>) {
      patch.invert();
    } else {
      patch += toAdd;
    }
  };

  auto patch = op::patch_type<TT>();
  addOp(patch);
  opTest.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);
  opTest.result() = registry.store(
      asValueStruct<TT>(makeAddExpectedResult<TT>(value.value)), protocol);
  return opTest;
}

template <typename TT>
Test createNumericPatchTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();

  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    auto& assignCase = test.testCases()->emplace_back();
    assignCase.name() = fmt::format("{}/assign", type::getName<TT>());
    auto& tascase = assignCase.test().emplace().objectPatch_ref().emplace();
    tascase = makeAssignTest<TT>(value, registry, protocol);

    auto& clearCase = test.testCases()->emplace_back();
    clearCase.name() = fmt::format("{}/clear", type::getName<TT>());
    auto& tclcase = clearCase.test().emplace().objectPatch_ref().emplace();
    tclcase = makeClearTest<TT>(value, registry, protocol);

    auto& addCase = test.testCases()->emplace_back();
    addCase.name() = fmt::format("{}/add", type::getName<TT>());
    auto& tadcase = addCase.test().emplace().objectPatch_ref().emplace();
    tadcase = makeAddTest<TT>(value, registry, protocol);
  }

  return test;
}

void addPatchToSuite(
    const AnyRegistry& registry, const Protocol& protocol, TestSuite& suite) {
  mp11::mp_for_each<detail::PrimaryTypeTags>([&](auto tt) {
    using TT = decltype(tt);
    if constexpr (
        !std::is_same_v<TT, type::string_t> &&
        !std::is_same_v<TT, type::binary_t>) {
      suite.tests()->emplace_back(
          createNumericPatchTest<TT>(registry, protocol));
    }
  });
}
} // namespace

TestSuite createPatchSuite(
    const std::set<Protocol>& protocols, const AnyRegistry& registry) {
  TestSuite suite;
  suite.name() = "PatchTest";
  for (const auto& protocol : protocols) {
    addPatchToSuite(registry, protocol, suite);
  }
  return suite;
}

} // namespace apache::thrift::conformance::data
