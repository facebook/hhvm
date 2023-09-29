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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/test/gen-cpp2/StructPatchTest_types.h>

namespace apache::thrift {
namespace {

using namespace test::patch;

// A callback to add a PatchOp to a Patch
template <class Patch>
using AddOp = std::function<void(Patch&)>;

template <class Patch>
using AddOps = std::vector<AddOp<Patch>>;

template <class Tag, class Patch>
void testMergePatchOps(
    const AddOps<Patch>& addOpList, const typename Patch::value_type& value) {
  Patch mergedPatch, patchWithMultipleOps;
  auto v1 = value;
  auto dv1 = protocol::asValueStruct<Tag>(value);
  for (const auto& addOp : addOpList) {
    Patch patch;
    addOp(patch);
    // v1 is the result when we apply each patch individually, e.g.,
    //
    //   p1.Method1()
    //   p1.apply(v1)
    //   p2.Method2()
    //   p2.apply(v1)
    //   p3.Method3()
    //   p3.apply(v1)
    patch.apply(v1);

    protocol::applyPatch(patch.toObject(), dv1);
    EXPECT_TRUE(op::equal<Tag>(v1, protocol::fromValueStruct<Tag>(dv1)));

    {
      // 1. Test whether applying mergedPatch patch is equivalent to applying
      // each patch individually, e.g., testing
      //
      //   p1.apply(v1)
      //   p2.apply(v1)
      //   p3.apply(v1)
      //
      // is equivalent to
      //
      //   Patch mergedPatch
      //   mergedPatch.merge(p1)
      //   mergedPatch.merge(p2)
      //   mergedPatch.merge(p3)
      //   mergedPatch.apply(v2)
      auto v2 = value;
      mergedPatch.merge(patch);
      mergedPatch.apply(v2);
      EXPECT_TRUE(op::equal<Tag>(v1, v2));

      auto dv2 = protocol::asValueStruct<Tag>(value);
      protocol::applyPatch(mergedPatch.toObject(), dv2);
      EXPECT_TRUE(op::equal<Tag>(v2, protocol::fromValueStruct<Tag>(dv2)));
    }

    {
      // 2. Test whether patch APIs have merge semantics. e.g., testing
      //
      //   p1.Method1()
      //   p1.apply(v1)
      //   p2.Method2()
      //   p2.apply(v1)
      //   p3.Method3()
      //   p3.apply(v1)
      //
      // is equivalent to
      //
      //   Patch patchWithMultipleOps
      //   patchWithMultipleOps.Method1()
      //   patchWithMultipleOps.Method2()
      //   patchWithMultipleOps.Method3()
      //   patchWithMultipleOps.apply(v2)
      auto v2 = value;
      addOp(patchWithMultipleOps);
      patchWithMultipleOps.apply(v2);
      EXPECT_TRUE(op::equal<Tag>(v1, v2));

      auto dv2 = protocol::asValueStruct<Tag>(value);
      protocol::applyPatch(patchWithMultipleOps.toObject(), dv2);
      EXPECT_TRUE(op::equal<Tag>(v2, protocol::fromValueStruct<Tag>(dv2)));
    }
  }
}

template <class Tag, class Patch>
void pickMultipleOpsAndTest(
    const AddOps<Patch>& candidates,
    const std::vector<typename Patch::value_type>& values,
    const int numberOfOpToPick, // Ideally we want to pick as many as operations
                                // as possible, but the test would be slow if we
                                // pick too many operations.
    AddOps<Patch> pickedOps = {}) {
  if (numberOfOpToPick == 0) {
    for (const auto& v : values) {
      testMergePatchOps<Tag>(pickedOps, v);
    }
    return;
  }

  for (auto op : candidates) {
    pickedOps.push_back(op);
    pickMultipleOpsAndTest<Tag>(
        candidates, values, numberOfOpToPick - 1, pickedOps);
    pickedOps.pop_back();
  }
}

template <class Patch>
void pickMultipleOpsAndTest(
    const AddOps<Patch>& candidates,
    const std::vector<typename Patch::value_type>& values,
    const int numberOfOpToPick) {
  pickMultipleOpsAndTest<type::infer_tag<typename Patch::value_type>>(
      candidates, values, numberOfOpToPick);
}

TEST(PatchMergeTest, BoolPatch) {
  AddOps<op::BoolPatch> ops;
  ops.push_back([](auto& patch) { patch = true; });
  ops.push_back([](auto& patch) { patch = false; });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch.invert(); });
  pickMultipleOpsAndTest(ops, {true, false}, 6);
}

template <class Patch, bool isFloat = false>
void testNumberPatch() {
  SCOPED_TRACE(folly::pretty_name<Patch>());
  AddOps<Patch> ops;
  ops.push_back([](auto& patch) { patch = 0; });
  ops.push_back([](auto& patch) { patch = 42; });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch += 0; });
  ops.push_back([](auto& patch) { patch += 10; });
  ops.push_back([](auto& patch) { patch -= 0; });
  ops.push_back([](auto& patch) { patch -= 20; });
  if constexpr (isFloat) {
    ops.push_back([](auto& patch) { patch = 1.5; });
    ops.push_back([](auto& patch) { patch += 2.5; });
    ops.push_back([](auto& patch) { patch -= 3.5; });
  }
  pickMultipleOpsAndTest(ops, {0, 10, 20, 42}, 4);
}

TEST(PatchMergeTest, BytePatch) {
  testNumberPatch<op::BytePatch>();
}

TEST(PatchMergeTest, I16Patch) {
  testNumberPatch<op::I16Patch>();
}

TEST(PatchMergeTest, I32Patch) {
  testNumberPatch<op::I32Patch>();
}

TEST(PatchMergeTest, I64Patch) {
  testNumberPatch<op::I64Patch>();
}

TEST(PatchMergeTest, FloatPatch) {
  testNumberPatch<op::FloatPatch, true>();
}

TEST(PatchMergeTest, DoublePatch) {
  testNumberPatch<op::DoublePatch, true>();
}

template <class Patch>
auto stringOrBinary(std::string s) {
  if constexpr (std::is_same_v<Patch, op::StringPatch>) {
    return s;
  } else {
    return *folly::IOBuf::copyBuffer(s);
  }
}

template <class Patch>
auto genStringPatchOps() {
  AddOps<Patch> ops;
  ops.push_back([](auto& patch) { patch = stringOrBinary<Patch>(""); });
  ops.push_back([](auto& patch) { patch = stringOrBinary<Patch>("42"); });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch.prepend(""); });
  ops.push_back([](auto& patch) { patch.prepend("("); });
  ops.push_back([](auto& patch) { patch.append(""); });
  ops.push_back([](auto& patch) { patch.append(")"); });
  return ops;
}

template <class Tag, class Patch>
void testBaseStringPatch() {
  SCOPED_TRACE(folly::pretty_name<Patch>());
  pickMultipleOpsAndTest<Tag>(
      genStringPatchOps<Patch>(),
      {stringOrBinary<Patch>(""), stringOrBinary<Patch>("1")},
      4);
}

TEST(PatchMergeTest, StringPatch) {
  testBaseStringPatch<type::string_t, op::StringPatch>();
}

TEST(PatchMergeTest, BinaryPatch) {
  testBaseStringPatch<
      type::cpp_type<folly::IOBuf, type::binary_t>,
      op::BinaryPatch>();
}

TEST(PatchMergeTest, ListPatch) {
  using ListPatch =
      std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optListVal())>;
  AddOps<ListPatch> ops;
  ops.push_back([](auto& patch) { patch = typename ListPatch::value_type{}; });
  ops.push_back([](auto& patch) { patch = {0}; });
  ops.push_back([](auto& patch) { patch = {10}; });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch.push_back(0); });
  ops.push_back([](auto& patch) { patch.push_back(10); });
  pickMultipleOpsAndTest(ops, {{}, {0}}, 4);
}

TEST(PatchMergeTest, SetPatch) {
  using SetPatch =
      std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optSetVal())>;
  AddOps<SetPatch> ops;
  ops.push_back([](auto& patch) { patch = typename SetPatch::value_type{}; });
  ops.push_back([](auto& patch) { patch = {"10"}; });
  ops.push_back([](auto& patch) { patch = {"20"}; });
  ops.push_back([](auto& patch) { patch = {"30"}; });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch.insert("10"); });
  ops.push_back([](auto& patch) { patch.insert("20"); });
  ops.push_back([](auto& patch) { patch.erase("10"); });
  ops.push_back([](auto& patch) { patch.erase("30"); });
  pickMultipleOpsAndTest<type::set<type::binary_t>>(
      ops, {{}, {"10"}, {"10", "20"}, {"10", "20", "30"}}, 3);
}

TEST(PatchMergeTest, MapPatch) {
  using MapPatch =
      std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optMapVal())>;
  AddOps<MapPatch> ops;
  ops.push_back([](auto& patch) { patch = typename MapPatch::value_type{}; });
  ops.push_back([](auto& patch) { patch = {{"10", "1"}}; });
  ops.push_back([](auto& patch) { patch = {{"20", "2"}}; });
  ops.push_back([](auto& patch) { patch = {{"30", "3"}}; });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch.insert_or_assign("10", "100"); });
  ops.push_back([](auto& patch) { patch.insert_or_assign("20", "100"); });
  ops.push_back([](auto& patch) { patch.insert_or_assign("30", "300"); });
  ops.push_back([](auto& patch) { patch.add({{"10", "1000"}}); });
  ops.push_back([](auto& patch) { patch.add({{"20", "2000"}}); });
  ops.push_back([](auto& patch) { patch.add({{"30", "3000"}}); });
  ops.push_back([](auto& patch) { patch.erase("10"); });
  ops.push_back([](auto& patch) { patch.erase("30"); });
  for (auto f : genStringPatchOps<op::StringPatch>()) {
    ops.push_back([f](auto& patch) { f(patch.patchByKey("10")); });
    ops.push_back([f](auto& patch) { f(patch.ensureAndPatchByKey("10")); });
  }
  std::vector<MapPatch::value_type> values;
  values.emplace_back();
  values.emplace_back(values.back())["10"] = "1";
  values.emplace_back(values.back())["20"] = "2";
  values.emplace_back(values.back())["30"] = "3";
  pickMultipleOpsAndTest<type::map<type::binary_t, type::binary_t>>(
      ops, values, 2);
}

TEST(PatchMergeTest, StructPatch) {
  AddOps<MyStructPatch> ops;
  MyStruct foo;
  ops.push_back([=](MyStructPatch& patch) { patch = foo; });
  foo.optStringVal() = "10";
  ops.push_back([=](MyStructPatch& patch) { patch = foo; });
  foo.optStringVal().reset();
  foo.stringVal() = "10";
  ops.push_back([=](MyStructPatch& patch) { patch = foo; });
  ops.push_back([](MyStructPatch& patch) { patch.clear(); });
  ops.push_back(
      [](MyStructPatch& patch) { (patch.ensure<ident::stringVal>()); });
  ops.push_back(
      [](MyStructPatch& patch) { (patch.ensure<ident::optStringVal>()); });
  ops.push_back(
      [](MyStructPatch& patch) { (patch.ensure<ident::optStringVal>("10")); });
  for (auto f : genStringPatchOps<op::StringPatch>()) {
    ops.push_back(
        [f](MyStructPatch& patch) { f(patch.patchIfSet<ident::stringVal>()); });
    ops.push_back([f](MyStructPatch& patch) {
      f(patch.patchIfSet<ident::optStringVal>());
    });
    ops.push_back(
        [f](MyStructPatch& patch) { f(patch.patch<ident::stringVal>()); });
    ops.push_back(
        [f](MyStructPatch& patch) { f(patch.patch<ident::optStringVal>()); });
  }
  std::vector<MyStruct> values;
  values.emplace_back();
  values.emplace_back().optStringVal() = "";
  values.emplace_back().optStringVal() = "10";
  values.emplace_back().stringVal() = "";
  values.emplace_back().stringVal() = "10";
  pickMultipleOpsAndTest(ops, values, 2);
}

} // namespace
} // namespace apache::thrift
