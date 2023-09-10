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

#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/lib/cpp/util/SaturatingMath.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types_custom_protocol.h>
#include <thrift/test/testset/Testset.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

using apache::thrift::protocol::asValueStruct;

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

  auto patch = op::patch_type<TT>::createClear();
  req.patch() =
      registry.store(asValueStruct<type::struct_c>(patch.toThrift()), protocol);

  auto clearValue = value.value;
  op::clear<TT>(clearValue);
  opTest.request() = req;
  opTest.result() = registry.store(asValueStruct<TT>(clearValue), protocol);
  return opTest;
}

template <typename TT, typename T>
T makeAddExpectedResult(T value, T add) {
  if constexpr (std::is_convertible_v<TT, type::bool_t>) {
    return !value;
  } else {
    return util::add_saturating(value, add);
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
      asValueStruct<TT>(makeAddExpectedResult<TT>(value.value, toAdd)),
      protocol);
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

    auto& clearCase = test.testCases()->emplace_back();
    clearCase.name() =
        fmt::format("{}/clear.{}", type::getName<TT>(), value.name);
    auto& tclcase = clearCase.test().emplace().objectPatch_ref().emplace();
    tclcase = makeClearTest<TT>(value, registry, protocol);

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

template <typename Tag, typename T, typename = void>
struct target_type_impl {
  using type = T;
};

template <typename Tag, typename T>
struct target_type_impl<Tag, T, folly::void_t<type::standard_type<Tag>>> {
  using type = type::standard_type<Tag>;
};

template <typename Tag, typename T>
using target_type = typename target_type_impl<Tag, T>::type;

protocol::Object setObjectMemeber(
    protocol::Object&& object, int16_t id, protocol::Value value) {
  object[FieldId{id}] = value;
  return std::move(object);
}

protocol::Value wrapObjectInValue(protocol::Object object) {
  protocol::Value result;
  result.emplace_object(object);
  return result;
}

template <typename Tag>
protocol::Object patchAddOperation(
    protocol::Object&& patch, op::PatchOp operation, auto value) {
  return setObjectMemeber(
      std::move(patch),
      static_cast<int16_t>(operation),
      asValueStruct<Tag>(value));
}

protocol::Object patchAddOperation(
    protocol::Object&& patch, op::PatchOp operation, protocol::Value value) {
  return setObjectMemeber(
      std::move(patch), static_cast<int16_t>(operation), value);
}

template <typename Tag>
protocol::Value makePatchValue(auto operation, auto value) {
  return wrapObjectInValue(
      patchAddOperation<Tag>(protocol::Object{}, operation, value));
}

protocol::Value makePatchValue(auto operation, protocol::Value value) {
  return wrapObjectInValue(
      patchAddOperation(protocol::Object{}, operation, value));
}

template <typename Tag, typename Type>
PatchOpTestCase makeAssignTC(
    const AnyRegistry& registry, const Protocol& protocol, Type value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = target_type<Tag, Type>;

  Container initial;
  Container expected = {value};

  req.value() = registry.store(asValueStruct<Tag>(initial), protocol);
  req.patch() = registry.store(
      makePatchValue<Tag>(op::PatchOp::Assign, expected), protocol);
  tascase.request() = req;
  tascase.result() = registry.store(asValueStruct<Tag>(expected), protocol);
  return tascase;
}

template <typename Tag, typename Type>
PatchOpTestCase makeClearTC(
    const AnyRegistry& registry, const Protocol& protocol, Type value) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = target_type<Tag, Type>;

  Container initial = {value};
  Container expected;

  req.value() = registry.store(asValueStruct<Tag>(initial), protocol);
  req.patch() = registry.store(
      makePatchValue<type::bool_t>(op::PatchOp::Clear, true), protocol);
  tascase.request() = req;
  auto expectedValue = asValueStruct<Tag>(expected);
  if (auto obj = expectedValue.if_object()) {
    obj->members()->clear();
  }
  tascase.result() = registry.store(expectedValue, protocol);
  return tascase;
}

template <typename ContainerTag, typename ValueTag>
PatchOpTestCase makeContainerRemoveTC(
    const AnyRegistry& registry,
    const Protocol& protocol,
    auto value,
    auto toRemove) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::native_type<ContainerTag>;

  Container initial = {value};
  Container expected;

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchValue<type::set<ValueTag>>(
          op::PatchOp::Remove,
          std::set<type::standard_type<ValueTag>>{toRemove}),
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
      makePatchValue<PatchTag>(op::PatchOp::Add, patchData), protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

template <typename ContainerTag>
PatchOpTestCase makeContainerAppendTC(
    const AnyRegistry& registry,
    const Protocol& protocol,
    auto value,
    auto toAppend) {
  PatchOpTestCase tascase;
  PatchOpRequest req;

  using Container = type::native_type<ContainerTag>;

  Container initial = {value};
  Container expected = {value, toAppend};
  Container patchData = {toAppend};

  req.value() = registry.store(asValueStruct<ContainerTag>(initial), protocol);
  req.patch() = registry.store(
      makePatchValue<ContainerTag>(op::PatchOp::Put, patchData), protocol);
  tascase.request() = req;
  tascase.result() =
      registry.store(asValueStruct<ContainerTag>(expected), protocol);
  return tascase;
}

template <typename Tag, typename Type>
PatchOpTestCase makePatchXTC(
    const AnyRegistry& registry,
    const Protocol& protocol,
    Type initial,
    op::PatchOp patchOp,
    protocol::Value valuePatch,
    Type expected) {
  PatchOpTestCase tascase;
  PatchOpRequest req;
  req.value() = registry.store(asValueStruct<Tag>(initial), protocol);
  req.patch() = registry.store(makePatchValue(patchOp, valuePatch), protocol);
  tascase.request() = req;
  tascase.result() = registry.store(asValueStruct<Tag>(expected), protocol);
  return tascase;
}

template <typename TT>
type::standard_type<TT> valueToAssign() {
  if constexpr (std::is_convertible_v<TT, type::number_c>) {
    return 1;
  } else {
    return "test";
  }
}

template <typename Tag, typename Type>
PatchOpTestCase makeEnsureTC(
    const AnyRegistry& registry,
    const Protocol& protocol,
    Type initial,
    Type expected,
    op::PatchOp patchOp = op::PatchOp::EnsureStruct) {
  PatchOpTestCase tascase;
  PatchOpRequest req;
  req.value() = registry.store(asValueStruct<Tag>(initial), protocol);
  req.patch() = registry.store(
      makePatchValue(patchOp, asValueStruct<Tag>(expected)), protocol);
  tascase.request() = req;
  tascase.result() = registry.store(asValueStruct<Tag>(expected), protocol);
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

    auto makeTestName = [&](const auto& value, auto op) {
      return fmt::format(
          "{}<{}>/{}.{}",
          type::getName<CC>(),
          type::getName<TT>(),
          op,
          value.name);
    };
    for (const auto& value : ValueGenerator<TT>::getInterestingValues()) {
      using ContainerTag = container_from_class_t<CC, TT>;
      auto& assignCase = test.testCases()->emplace_back();
      assignCase.name() = makeTestName(value, "assign");
      assignCase.test().emplace().objectPatch_ref() =
          makeAssignTC<ContainerTag>(registry, protocol, value.value);

      auto& clearCase = test.testCases()->emplace_back();
      clearCase.name() = makeTestName(value, "clear");
      clearCase.test().emplace().objectPatch_ref() =
          makeClearTC<ContainerTag>(registry, protocol, value.value);

      if constexpr (std::is_same_v<CC, type::set_c>) {
        auto& removeCase = test.testCases()->emplace_back();
        removeCase.name() = makeTestName(value, "remove");
        removeCase.test().emplace().objectPatch_ref() =
            makeContainerRemoveTC<ContainerTag, TT>(
                registry, protocol, value.value, value.value);
      }

      auto& prependCase = test.testCases()->emplace_back();
      prependCase.name() = makeTestName(value, "prepend");
      prependCase.test().emplace().objectPatch_ref() =
          makeValueContainerPrependTC<ContainerTag, TT>(
              registry, protocol, value);

      if constexpr (std::is_same_v<CC, type::list_c>) {
        auto& prependSetCase = test.testCases()->emplace_back();
        prependSetCase.name() = makeTestName(value, "prepend_set");
        prependSetCase.test().emplace().objectPatch_ref() =
            makeValueContainerPrependTC<ContainerTag, TT, type::set<TT>>(
                registry, protocol, value);
      }

      auto patchValue = value.value;
      op::clear<TT>(patchValue);
      auto& appendCase = test.testCases()->emplace_back();
      appendCase.name() = makeTestName(value, "append");
      appendCase.test().emplace().objectPatch_ref() =
          makeContainerAppendTC<ContainerTag>(
              registry, protocol, value.value, patchValue);
    }
  });

  return test;
}

template <typename TT>
Test createMapPatchTest(const AnyRegistry& registry, const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();

  using KeyTypeTags = boost::mp11::mp_list<
      type::byte_t,
      type::i16_t,
      type::i32_t,
      type::i64_t,
      type::string_t>;
  mp11::mp_for_each<KeyTypeTags>([&](auto kk) {
    using KK = decltype(kk);
    using ContainerTag = type::map<KK, TT>;

    auto makeTestName = [](const auto& key, auto op) {
      return fmt::format(
          "map<{},{}>/{}.{}",
          type::getName<KK>(),
          type::getName<TT>(),
          op,
          key.name);
    };

    for (const auto& key : ValueGenerator<KK>::getInterestingValues()) {
      type::standard_type<TT> value;
      op::clear<TT>(value);

      auto& assignCase = test.testCases()->emplace_back();
      assignCase.name() = makeTestName(key, "assign");
      assignCase.test().emplace().objectPatch_ref() =
          makeAssignTC<ContainerTag>(
              registry, protocol, std::pair{key.value, value});

      auto& clearCase = test.testCases()->emplace_back();
      clearCase.name() = makeTestName(key, "clear");
      clearCase.test().emplace().objectPatch_ref() = makeClearTC<ContainerTag>(
          registry, protocol, std::pair{key.value, value});

      auto& removeCase = test.testCases()->emplace_back();
      removeCase.name() = makeTestName(key, "remove");
      removeCase.test().emplace().objectPatch_ref() =
          makeContainerRemoveTC<ContainerTag, KK>(
              registry, protocol, std::pair{key.value, value}, key.value);

      auto patchkey = key.value;
      op::clear<KK>(patchkey);
      auto& appendCase = test.testCases()->emplace_back();
      appendCase.name() = makeTestName(key, "append");
      appendCase.test().emplace().objectPatch_ref() =
          makeContainerAppendTC<ContainerTag>(
              registry,
              protocol,
              std::pair{key.value, value},
              std::pair{patchkey, value});

      using Container = type::native_type<ContainerTag>;

      Container initial = {{key.value, value}};
      auto newValue = valueToAssign<TT>();
      auto keyValue = asValueStruct<KK>(key.value);
      auto keyValuePatch = asValueStruct<type::struct_c>(
          (op::patch_type<TT>() = toValue<TT>(newValue)).toThrift());
      Container expected = {{key.value, newValue}};
      protocol::Value patchValue;
      patchValue.ensure_map()[keyValue] = keyValuePatch;

      auto& patchPriorCase = test.testCases()->emplace_back();
      patchPriorCase.name() = makeTestName(key, "patch_prior");
      patchPriorCase.test().emplace().objectPatch_ref() =
          makePatchXTC<ContainerTag>(
              registry,
              protocol,
              initial,
              op::PatchOp::PatchPrior,
              patchValue,
              expected);

      auto& patchCase = test.testCases()->emplace_back();
      patchCase.name() = makeTestName(key, "patch");
      patchCase.test().emplace().objectPatch_ref() = makePatchXTC<ContainerTag>(
          registry,
          protocol,
          initial,
          op::PatchOp::PatchAfter,
          patchValue,
          expected);

      initial = {};
      auto& ensureCase = test.testCases()->emplace_back();
      ensureCase.name() = makeTestName(key, "ensure");
      ensureCase.test().emplace().objectPatch_ref() =
          makeEnsureTC<ContainerTag>(registry, protocol, initial, expected);
    }
  });

  return test;
}

template <typename TT>
Test createStructPatchTest(
    const AnyRegistry& registry, const Protocol& protocol) {
  Test test;
  test.name() = protocol.name();

  using Struct = test::testset::struct_with<TT>;

  auto makeTestName = [](auto op) {
    return fmt::format("struct<{}>/{}", type::getName<TT>(), op);
  };

  type::standard_type<TT> value;
  op::clear<TT>(value);

  Struct initial;
  initial.field_1_ref() = value;

  auto& assignCase = test.testCases()->emplace_back();
  assignCase.name() = makeTestName("assign");
  assignCase.test().emplace().objectPatch_ref() =
      makeAssignTC<type::struct_c>(registry, protocol, initial);

  auto& clearCase = test.testCases()->emplace_back();
  clearCase.name() = makeTestName("clear");
  clearCase.test().emplace().objectPatch_ref() =
      makeClearTC<type::struct_c>(registry, protocol, initial);

  Struct expected = initial;
  expected.field_1_ref() = valueToAssign<TT>();
  auto patch = op::patch_type<TT>();
  patch = toValue<TT>(*expected.field_1_ref());
  auto patchValue = wrapObjectInValue(setObjectMemeber(
      protocol::Object{}, 1, asValueStruct<type::struct_c>(patch.toThrift())));

  auto& patchPriorCase = test.testCases()->emplace_back();
  patchPriorCase.name() = makeTestName("patch_prior");
  patchPriorCase.test().emplace().objectPatch_ref() =
      makePatchXTC<type::struct_c>(
          registry,
          protocol,
          initial,
          op::PatchOp::PatchPrior,
          patchValue,
          expected);

  auto& patchCase = test.testCases()->emplace_back();
  patchCase.name() = makeTestName("patch");
  patchCase.test().emplace().objectPatch_ref() = makePatchXTC<type::struct_c>(
      registry,
      protocol,
      initial,
      op::PatchOp::PatchAfter,
      patchValue,
      expected);

  using StructOptional =
      test::testset::struct_with<TT, test::testset::FieldModifier::Optional>;

  StructOptional initialOpt;
  StructOptional expectedOpt;
  expectedOpt.field_1_ref() = valueToAssign<TT>();
  auto& ensureCase = test.testCases()->emplace_back();
  ensureCase.name() = makeTestName("ensureStruct");
  ensureCase.test().emplace().objectPatch_ref() =
      makeEnsureTC<type::struct_c>(registry, protocol, initialOpt, expectedOpt);

  using UnionStruct = test::testset::union_with<TT>;
  UnionStruct initialUnion;
  UnionStruct expectedUnion;
  expectedUnion.field_1_ref() = valueToAssign<TT>();
  auto& ensureUnionCase = test.testCases()->emplace_back();
  ensureUnionCase.name() = makeTestName("ensureUnion");
  ensureUnionCase.test().emplace().objectPatch_ref() =
      makeEnsureTC<type::struct_c>(
          registry,
          protocol,
          initialUnion,
          expectedUnion,
          op::PatchOp::EnsureUnion);

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

    suite.tests()->emplace_back(createMapPatchTest<TT>(registry, protocol));

    suite.tests()->emplace_back(createStructPatchTest<TT>(registry, protocol));
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
