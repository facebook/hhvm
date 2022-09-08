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

#include <iterator>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
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
  PatchOpRequest req;
  auto initialValue = value.value;
  op::clear<TT>(initialValue);
  req.value() = registry.store(asValueStruct<TT>(initialValue), protocol);

  auto patch = op::patch_type<TT>();
  patch = toValue<TT>(value.value);
  req.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);
  opTest.request() = req;
  opTest.result() = registry.store(asValueStruct<TT>(value.value), protocol);
  return opTest;
}

template <typename TT, typename GeneratedValue>
PatchOpTestCase makeClearTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol) {
  PatchOpTestCase opTest;
  PatchOpRequest req;
  req.value() = registry.store(asValueStruct<TT>(value.value), protocol);

  auto patch = op::patch_type<TT>();
  patch.clear();
  req.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);

  auto clearValue = value.value;
  op::clear<TT>(clearValue);
  opTest.request() = req;
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

template <typename TT, typename GeneratedValue, typename T>
PatchOpTestCase makeAddTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol,
    T toAdd) {
  PatchOpTestCase opTest;
  PatchOpRequest req;
  req.value() = registry.store(asValueStruct<TT>(value.value), protocol);

  auto addOp = [&](auto& patch) {
    if constexpr (std::is_same_v<TT, type::bool_t>) {
      patch.invert();
    } else {
      patch += toAdd;
    }
  };

  auto patch = op::patch_type<TT>();
  addOp(patch);
  req.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);
  opTest.request() = req;
  opTest.result() = registry.store(
      asValueStruct<TT>(makeAddExpectedResult<TT>(value.value)), protocol);
  return opTest;
}

template <typename TT, typename GeneratedValue>
PatchOpTestCase makePrependTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol) {
  PatchOpTestCase opTest;
  PatchOpRequest req;
  req.value() = registry.store(asValueStruct<TT>(value.value), protocol);

  auto patch = op::patch_type<TT>();
  patch.prepend(toValue<TT>(value.value));
  req.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);
  opTest.request() = req;
  auto expected = value.value + value.value;
  opTest.result() =
      registry.store(asValueStruct<TT>(toValue<TT>(expected)), protocol);
  return opTest;
}

template <typename TT, typename GeneratedValue>
PatchOpTestCase makeAppendTest(
    const GeneratedValue& value,
    const AnyRegistry& registry,
    const Protocol& protocol) {
  PatchOpTestCase opTest;
  PatchOpRequest req;
  req.value() = registry.store(asValueStruct<TT>(value.value), protocol);

  auto patch = op::patch_type<TT>();
  patch.append(toValue<TT>(value.value));
  req.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);
  opTest.request() = req;
  auto expected = value.value + value.value;
  opTest.result() =
      registry.store(asValueStruct<TT>(toValue<TT>(expected)), protocol);
  return opTest;
}

template <typename TT>
Test createNumericPatchTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();

  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    auto& assignCase = test.testCases()->emplace_back();
    assignCase.name() =
        fmt::format("{}/assign.{}", type::getName<TT>(), value.name);
    auto& tascase = assignCase.test().emplace().objectPatch_ref().emplace();
    tascase = makeAssignTest<TT>(value, registry, protocol);

    // TODO(afuller): decide if bool and numeric should have clear()

    using ValueType = decltype(value.value);
    auto addAddTestCase = [&](ValueType toAdd) {
      auto& addCase = test.testCases()->emplace_back();
      addCase.name() =
          fmt::format("{}/add.{}_{}", type::getName<TT>(), value.name, toAdd);
      auto& tadcase = addCase.test().emplace().objectPatch_ref().emplace();
      tadcase = makeAddTest<TT>(value, registry, protocol, toAdd);
    };

    addAddTestCase(1);
    if constexpr (!std::is_same_v<TT, type::bool_t>) {
      addAddTestCase(-1);
      if (value.value > 0) {
        addAddTestCase(-value.value);
      }
    }
  }

  return test;
}

template <typename TT>
Test createStringLikePatchTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();

  for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
    auto& assignCase = test.testCases()->emplace_back();
    assignCase.name() =
        fmt::format("{}/assign.{}", type::getName<TT>(), value.name);
    auto& tascase = assignCase.test().emplace().objectPatch_ref().emplace();
    tascase = makeAssignTest<TT>(value, registry, protocol);

    auto& clearCase = test.testCases()->emplace_back();
    clearCase.name() = fmt::format("{}/clear", type::getName<TT>());
    auto& tclcase = clearCase.test().emplace().objectPatch_ref().emplace();
    tclcase = makeClearTest<TT>(value, registry, protocol);

    auto& prependCase = test.testCases()->emplace_back();
    prependCase.name() =
        fmt::format("{}/prepend.{}", type::getName<TT>(), value.name);
    auto& tprcase = prependCase.test().emplace().objectPatch_ref().emplace();
    tprcase = makePrependTest<TT>(value, registry, protocol);

    auto& appendCase = test.testCases()->emplace_back();
    appendCase.name() =
        fmt::format("{}/append.{}", type::getName<TT>(), value.name);
    auto& tapcase = appendCase.test().emplace().objectPatch_ref().emplace();
    tapcase = makeAppendTest<TT>(value, registry, protocol);
  }

  return test;
}

namespace {
template <typename T>
struct container_from_class {
  static_assert(std::is_same_v<T, type::container_c>, "Not a container class");
};

template <>
struct container_from_class<type::list_c> {
  template <typename ValueType>
  using type = type::list<ValueType>;
};

template <>
struct container_from_class<type::set_c> {
  template <typename ValueType>
  using type = type::set<ValueType>;
};

template <typename C, typename... Types>
using container_from_class_t =
    typename container_from_class<C>::template type<Types...>;

template <typename Tag>
Object patchAddOperation(Object&& patch, op::PatchOp operation, auto value) {
  auto opId = static_cast<int16_t>(operation);
  patch.members().ensure()[opId] = asValueStruct<Tag>(value);
  return std::move(patch);
}

template <typename Tag>
Value makePatchObject(auto operation, auto value) {
  Value result;
  result.objectValue_ref() = patchAddOperation<Tag>(Object{}, operation, value);
  return result;
}

template <typename ContainerTag>
PatchOpTestCase makeValueContainerAssignTC(
    const AnyRegistry& registry, const Protocol& protocol, auto value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::standard_type<ContainerTag>;

  Container initial;
  Container expected = {value.value};

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchObject<ContainerTag>(op::PatchOp::Assign, expected), protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

template <typename ContainerTag>
PatchOpTestCase makeValueContainerClearTC(
    const AnyRegistry& registry, const Protocol& protocol, auto value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::standard_type<ContainerTag>;

  Container initial = {value.value};
  Container expected;

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchObject<type::bool_t>(op::PatchOp::Clear, true), protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

template <typename ContainerTag, typename ValueTag>
PatchOpTestCase makeValueContainerRemoveTC(
    const AnyRegistry& registry, const Protocol& protocol, auto value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::native_type<ContainerTag>;

  Container initial = {value.value};
  Container expected;

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchObject<type::set<ValueTag>>(
          op::PatchOp::Remove,
          std::set<type::standard_type<ValueTag>>{value.value}),
      protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

template <typename ContainerTag, typename TT, typename PatchTag = ContainerTag>
PatchOpTestCase makeValueContainerPrependTC(
    const AnyRegistry& registry, const Protocol& protocol, auto value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::native_type<ContainerTag>;
  using PatchContainer = type::standard_type<PatchTag>;

  auto patchValue = value.value;
  op::clear<TT>(patchValue);
  Container initial = {value.value};
  Container expected = {patchValue, value.value};
  if (patchValue == value.value && !std::is_same_v<PatchTag, ContainerTag>) {
    expected = initial;
  }
  PatchContainer patchData = {patchValue};

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchObject<PatchTag>(op::PatchOp::Add, patchData), protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

template <typename ContainerTag, typename TT>
PatchOpTestCase makeValueContainerAppendTC(
    const AnyRegistry& registry, const Protocol& protocol, auto value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::native_type<ContainerTag>;

  auto patchValue = value.value;
  op::clear<TT>(patchValue);
  Container initial = {value.value};
  Container expected = {value.value, patchValue};
  Container patchData = {patchValue};

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchObject<ContainerTag>(op::PatchOp::Put, patchData), protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

} // namespace

template <typename TT>
Test createListSetPatchTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();

  using ValueContainers = boost::mp11::mp_list<type::list_c, type::set_c>;
  mp11::mp_for_each<ValueContainers>([&](auto cc) {
    using CC = decltype(cc);
    for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
      using ContainerTag = container_from_class_t<CC, TT>;
      auto& assignCase = test.testCases()->emplace_back();
      assignCase.name() = fmt::format(
          "{}<{}>/assign.{}",
          type::getName<CC>(),
          type::getName<TT>(),
          value.name);
      assignCase.test().emplace().objectPatch_ref() =
          makeValueContainerAssignTC<ContainerTag>(registry, protocol, value);

      auto& clearCase = test.testCases()->emplace_back();
      clearCase.name() = fmt::format(
          "{}<{}>/clear.{}",
          type::getName<CC>(),
          type::getName<TT>(),
          value.name);
      clearCase.test().emplace().objectPatch_ref() =
          makeValueContainerClearTC<ContainerTag>(registry, protocol, value);

      auto& removeCase = test.testCases()->emplace_back();
      removeCase.name() = fmt::format(
          "{}<{}>/remove.{}",
          type::getName<CC>(),
          type::getName<TT>(),
          value.name);
      removeCase.test().emplace().objectPatch_ref() =
          makeValueContainerRemoveTC<ContainerTag, TT>(
              registry, protocol, value);

      auto& prependCase = test.testCases()->emplace_back();
      prependCase.name() = fmt::format(
          "{}<{}>/prepend.{}",
          type::getName<CC>(),
          type::getName<TT>(),
          value.name);
      prependCase.test().emplace().objectPatch_ref() =
          makeValueContainerPrependTC<ContainerTag, TT>(
              registry, protocol, value);

      if constexpr (std::is_same_v<CC, type::list_c>) {
        auto& prependSetCase = test.testCases()->emplace_back();
        prependSetCase.name() = fmt::format(
            "{}<{}>/prepend_set.{}",
            type::getName<CC>(),
            type::getName<TT>(),
            value.name);
        prependSetCase.test().emplace().objectPatch_ref() =
            makeValueContainerPrependTC<ContainerTag, TT, type::set<TT>>(
                registry, protocol, value);
      }

      auto& appendCase = test.testCases()->emplace_back();
      appendCase.name() = fmt::format(
          "{}<{}>/append.{}",
          type::getName<CC>(),
          type::getName<TT>(),
          value.name);
      appendCase.test().emplace().objectPatch_ref() =
          makeValueContainerAppendTC<ContainerTag, TT>(
              registry, protocol, value);
    }
  });

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
    } else {
      suite.tests()->emplace_back(
          createStringLikePatchTest<TT>(registry, protocol));
    }

    if constexpr (!std::is_same_v<TT, type::bool_t>) {
      suite.tests()->emplace_back(
          createListSetPatchTest<TT>(registry, protocol));
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
