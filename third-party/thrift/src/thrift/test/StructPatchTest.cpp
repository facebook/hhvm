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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/op/Testing.h>
#include <thrift/lib/cpp2/op/detail/StructPatch.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_fatal_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_types.h>

namespace apache::thrift {
namespace {

using namespace test::patch;

static_assert(test::same_type<
              MyStructPatch,
              ::apache::thrift::op::patch_type<type::struct_t<MyStruct>>>);

static_assert(
    test::same_type<MyStructPatch, ::apache::thrift::op::patch_type<MyStruct>>);

static_assert(test::same_type<
              MyUnionPatch,
              ::apache::thrift::op::patch_type<type::union_t<MyUnion>>>);

static_assert(
    test::same_type<MyUnionPatch, ::apache::thrift::op::patch_type<MyUnion>>);

static_assert(::apache::thrift::adapt_detail::has_inplace_toThrift<
              ::apache::thrift::op::detail::FieldPatchAdapter<
                  MyStructFieldPatchStruct>,
              MyStructFieldPatch>::value);

static_assert(test::same_type<
              type::infer_tag<MyStructPatch>,
              type::adapted<
                  InlineAdapter<MyStructPatch>,
                  type::struct_t<MyStructPatchStruct>>>);

using ListPatch =
    std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optListVal())>;
using ListDequePatch =
    std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->longList())>;
using SetPatch =
    std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optSetVal())>;
using MapPatch =
    std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optMapVal())>;

MyStruct testValue() {
  MyStruct val;
  val.boolVal() = true;
  val.byteVal() = 2;
  val.i16Val() = 3;
  val.i32Val() = 4;
  val.i64Val() = 5;
  val.floatVal() = 6;
  val.doubleVal() = 7;
  val.stringVal() = "8";
  val.binaryVal() = StringTraits<folly::IOBuf>::fromStringLiteral("9");
  val.structVal()->data1() = "Ba";
  return val;
}

MyStructPatch testPatch() {
  auto val = testValue();
  MyStructPatch result;
  EXPECT_FALSE(result.modifies<ident::boolVal>());
  result.patchIfSet<ident::boolVal>() = !op::BoolPatch{};
  result.patchIfSet<ident::byteVal>() = val.byteVal();
  result.patchIfSet<ident::i16Val>() += 2;
  result.patchIfSet<ident::i32Val>() += 3;
  result.patchIfSet<ident::i64Val>() += 4;
  result.patchIfSet<ident::floatVal>() += 5;
  result.patchIfSet<ident::doubleVal>() += 6;
  result.patchIfSet<ident::stringVal>() = "_" + op::StringPatch{} + "_";
  result.patchIfSet<ident::structVal>().patchIfSet<ident::data1>().append("Na");
  EXPECT_TRUE(result.modifies<ident::boolVal>());
  return result;
}

MyStructPatch testOptPatch() {
  auto val = testValue();
  MyStructPatch result;
  EXPECT_FALSE(result.modifies<ident::optBoolVal>());
  result.patchIfSet<ident::optBoolVal>() = !op::BoolPatch{};
  result.patchIfSet<ident::optByteVal>() = val.byteVal();
  result.patchIfSet<ident::optI16Val>() += 2;
  result.patchIfSet<ident::optI32Val>() += 3;
  result.patchIfSet<ident::optI64Val>() += 4;
  result.patchIfSet<ident::optFloatVal>() += 5;
  result.patchIfSet<ident::optDoubleVal>() += 6;
  result.patchIfSet<ident::optStringVal>() = "_" + op::StringPatch{} + "_";
  EXPECT_TRUE(result.modifies<ident::optBoolVal>());
  MyData data;
  data.data2() = 5;
  result.patchIfSet<ident::optStructVal>() = std::move(data);
  return result;
}

TEST(StructPatchTest, Noop) {
  // Empty patch does nothing.
  MyStructPatch patch;
  test::expectPatch(patch, {}, {});
}

TEST(StructPatchTest, Assign) {
  // Assign in a single step.
  auto patch = MyStructPatch::createAssign(testValue());
  test::expectPatch(patch, {}, testValue());
}

TEST(StructPatchTest, AssignSplit) {
  auto original = testValue();
  auto patch = MyStructPatch::createAssign(original);
  // For the patch to break apart the assign and check the result;
  patch.patchIfSet<ident::optI64Val>();
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_FALSE(*patch.toThrift().clear());
  test::expectPatch(patch, {}, original);
}

TEST(StructPatchTest, Clear) {
  // Clear patch, clears all fields (even ones with defaults)
  test::expectPatch(MyStructPatch::createClear(), testValue(), {});
  test::expectPatch(op::StringPatch::createClear(), {"hi"}, "");
}

TEST(StructPatchTest, ClearField_BoolPatch) {
  MyStruct actual;

  actual.boolVal() = true;
  op::BoolPatch::createClear().apply(actual.boolVal());
  EXPECT_FALSE(*actual.boolVal());

  actual.optBoolVal() = true;
  op::BoolPatch::createClear().apply(actual.optBoolVal());
  EXPECT_FALSE(actual.optBoolVal().has_value());
}

TEST(StructPatchTest, ClearField_NumberPatch) {
  MyStruct actual;

  actual.i16Val() = 1;
  op::I16Patch::createClear().apply(actual.i16Val());
  EXPECT_EQ(*actual.boolVal(), 0);

  actual.optDoubleVal() = 1;
  op::DoublePatch::createClear().apply(actual.optDoubleVal());
  EXPECT_FALSE(actual.optDoubleVal().has_value());
}

TEST(StructPatchTest, ClearField_StringPatch) {
  MyStruct actual;

  actual.stringVal() = "hi";
  op::StringPatch::createClear().apply(actual.stringVal());
  EXPECT_EQ(*actual.stringVal(), "");

  actual.optBinaryVal().ensure();
  op::BinaryPatch::createClear().apply(actual.optBinaryVal());
  EXPECT_FALSE(actual.optBinaryVal().has_value());
}

TEST(StructPatchTest, Patch) {
  MyStruct val;
  val.stringVal() = "hi";
  val.structVal()->data1() = "Ba";

  MyStruct expected1, expected2;
  expected1.boolVal() = true;
  expected2.boolVal() = false;
  expected1.byteVal() = 2;
  expected2.byteVal() = 2;
  expected1.i16Val() = 2;
  expected2.i16Val() = 4;
  expected1.i32Val() = 3;
  expected2.i32Val() = 6;
  expected1.i64Val() = 4;
  expected2.i64Val() = 8;
  expected1.floatVal() = 5;
  expected2.floatVal() = 10;
  expected1.doubleVal() = 6;
  expected2.doubleVal() = 12;
  expected1.stringVal() = "_hi_";
  expected2.stringVal() = "__hi__";
  expected1.structVal()->data1() = "BaNa";
  expected2.structVal()->data1() = "BaNaNa";

  auto patch = testPatch();
  test::expectPatch(patch, val, expected1, expected2);

  // Make sure prior is being applied, if present.
  patch.patch<ident::stringVal>().prepend("p");
  expected1.stringVal() = "p_hi_";
  expected2.stringVal() = "p_p_hi__";
  test::expectPatch(patch, val, expected1, expected2);

  patch.merge(MyStructPatch::createClear());
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_TRUE(patch.toThrift().patchPrior()->empty());
  EXPECT_TRUE(patch.toThrift().patch()->empty());
  EXPECT_TRUE(*patch.toThrift().clear());
  test::expectPatch(patch, testValue(), {});
}

TEST(StructPatchTest, ClearAssign) {
  auto patch = MyStructPatch::createClear();
  EXPECT_TRUE(patch.modifies<ident::structVal>());
  patch.merge(MyStructPatch::createAssign(testValue()));
  // Assign takes precedence, like usual.
  test::expectPatch(patch, {}, testValue());
}

TEST(StructPatchTest, AssignClear) {
  auto patch = MyStructPatch::createAssign(testValue());
  EXPECT_TRUE(patch.modifies<ident::structVal>());
  patch.merge(MyStructPatch::createClear());
  test::expectPatch(patch, testValue(), {});

  // Clear patch takes precedence (as it is smaller to encode and slightly
  // stronger in the presense of non-terse non-optional fields).
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_TRUE(*patch.toThrift().clear());
}

TEST(StructPatchTest, OptionalField_Ensure) {
  MyStructPatch patch;
  patch.ensure<ident::optListVal>({1});
  patch.ensure<ident::optListVal>({2});

  MyStruct actual;
  actual.optListVal() = {3};
  test::expectPatch(patch, actual, actual);

  MyStruct oneVal;
  oneVal.optListVal().ensure() = {1};
  test::expectPatch(patch, {}, oneVal);
}

TEST(StructPatchTest, OptionalField_Clear) {
  MyStructPatch patch;
  patch.clear<ident::optBoolVal>();
  MyStruct actual;
  actual.optBoolVal() = true;
  test::expectPatch(patch, actual, {});
}

TEST(StructPatchTest, OptionalField_Assign) {
  MyStructPatch patch;
  // = -> clear + ensure + assign;
  patch.assign<ident::optBoolVal>(true);
  MyStruct expected;
  expected.optBoolVal() = true;
  test::expectPatch(patch, {}, expected);
}

TEST(StructPatchTest, AssignClearEnsure) {
  MyStructPatch assign;
  assign.patch<ident::optI32Val>() = 1;

  MyStructPatch clearEnsure;
  clearEnsure.clear<ident::optI32Val>();
  clearEnsure.ensure<ident::optI32Val>(2);

  MyStruct actual;
  assign.apply(actual);
  EXPECT_EQ(actual.optI32Val(), 1);
  clearEnsure.apply(actual);
  EXPECT_EQ(actual.optI32Val(), 2);

  // Merging and applying should have the same result.
  MyStructPatch assignClearEnsure = assign;
  assignClearEnsure.merge(clearEnsure);
  actual = {};
  assignClearEnsure.apply(actual);
  ASSERT_TRUE(actual.optI32Val().has_value());
  EXPECT_EQ(*actual.optI32Val(), 2);
}

TEST(StructPatchTest, ClearAndEnsure) {
  MyData data;
  data.data1() = "42";

  MyStructPatch patch;
  patch.patch<ident::optStructVal>().clear();
  patch.ensure<ident::optStructVal>(data);

  MyStruct actual;
  patch.apply(actual);
  ASSERT_TRUE(actual.optStructVal().has_value());
  EXPECT_EQ(*actual.optStructVal(), data);
}

TEST(StructPatchTest, OptionalField_PatchPrior) {
  // unset -> true -> false.
  // set -> invert -> invert
  MyStructPatch patch;
  patch.patchIfSet<ident::optBoolVal>().invert();
  patch.ensure<ident::optBoolVal>(true);

  MyStruct trueVal;
  trueVal.optBoolVal() = true;
  MyStruct falseVal;
  falseVal.optBoolVal() = false;

  // unset -> true -> false.
  test::expectPatch(patch, {}, trueVal, falseVal);
  // set -> invert -> invert
  test::expectPatch(patch, falseVal, trueVal, falseVal);
  test::expectPatch(patch, trueVal, falseVal, trueVal);
}

TEST(StructPatchTest, OptionalFields) {
  MyStructPatch patch = testPatch();
  MyStructPatch optPatch = testOptPatch();

  MyStruct actual;

  // Applying a value patch to void does nothing.
  test::expectPatch(optPatch, {}, {});
  EXPECT_EQ(actual, MyStruct{});
  patch.patchIfSet<ident::boolVal>().apply(actual.optBoolVal());
  patch.patchIfSet<ident::byteVal>().apply(actual.optByteVal());
  patch.patchIfSet<ident::i16Val>().apply(actual.optI16Val());
  patch.patchIfSet<ident::i32Val>().apply(actual.optI32Val());
  patch.patchIfSet<ident::i64Val>().apply(actual.optI64Val());
  patch.patchIfSet<ident::floatVal>().apply(actual.optFloatVal());
  patch.patchIfSet<ident::doubleVal>().apply(actual.optDoubleVal());
  patch.patchIfSet<ident::stringVal>().apply(actual.optStringVal());
  patch.patchIfSet<ident::structVal>().apply(actual.optStructVal());
  EXPECT_EQ(actual, MyStruct{});

  // Applying a value patch to values, patches.
  actual.optBoolVal().ensure();
  actual.optByteVal().ensure();
  actual.optI16Val().ensure();
  actual.optI32Val().ensure();
  actual.optI64Val().ensure();
  actual.optFloatVal().ensure();
  actual.optDoubleVal().ensure();
  actual.optStringVal() = "hi";
  actual.optStructVal().ensure().data1() = "Ba";

  MyStruct expected1, expected2;
  expected1.optBoolVal() = true;
  expected2.optBoolVal() = false;
  expected1.optByteVal() = 2;
  expected2.optByteVal() = 2;
  expected1.optI16Val() = 2;
  expected2.optI16Val() = 4;
  expected1.optI32Val() = 3;
  expected2.optI32Val() = 6;
  expected1.optI64Val() = 4;
  expected2.optI64Val() = 8;
  expected1.optFloatVal() = 5;
  expected2.optFloatVal() = 10;
  expected1.optDoubleVal() = 6;
  expected2.optDoubleVal() = 12;
  expected1.optStringVal() = "_hi_";
  expected2.optStringVal() = "__hi__";
  expected1.optStructVal().ensure().data2() = 5;
  expected2.optStructVal().ensure().data2() = 5;
  test::expectPatch(optPatch, actual, expected1, expected2);

  patch.patchIfSet<ident::boolVal>().apply(actual.optBoolVal());
  patch.patchIfSet<ident::byteVal>().apply(actual.optByteVal());
  patch.patchIfSet<ident::i16Val>().apply(actual.optI16Val());
  patch.patchIfSet<ident::i32Val>().apply(actual.optI32Val());
  patch.patchIfSet<ident::i64Val>().apply(actual.optI64Val());
  patch.patchIfSet<ident::floatVal>().apply(actual.optFloatVal());
  patch.patchIfSet<ident::doubleVal>().apply(actual.optDoubleVal());
  patch.patchIfSet<ident::stringVal>().apply(actual.optStringVal());
  patch.patchIfSet<ident::structVal>().apply(actual.optStructVal());
  EXPECT_EQ(*actual.optBoolVal(), true);
  EXPECT_EQ(*actual.optByteVal(), 2);
  EXPECT_EQ(*actual.optI16Val(), 2);
  EXPECT_EQ(*actual.optI32Val(), 3);
  EXPECT_EQ(*actual.optI64Val(), 4);
  EXPECT_EQ(*actual.optFloatVal(), 5);
  EXPECT_EQ(*actual.optDoubleVal(), 6);
  EXPECT_EQ(*actual.optStringVal(), "_hi_");
  ASSERT_TRUE(actual.optStructVal().has_value());
  EXPECT_EQ(*actual.optStructVal()->data1(), "BaNa");
}

TEST(StructPatchTest, ListPatch) {
  ListPatch patch;
  ListPatch::value_type actual{5, 6};
  patch.append({7, 8});
  patch.emplace_back(9);
  patch.push_back(10);
  test::expectPatch(
      patch, actual, {5, 6, 7, 8, 9, 10}, {5, 6, 7, 8, 9, 10, 7, 8, 9, 10});
}

TEST(StructPatchTest, ListDequePatch) {
  ListDequePatch patch;
  ListDequePatch::value_type actual{5, 6};
  patch.append({7, 8});
  patch.emplace_back(9);
  patch.push_back(10);
  test::expectPatch(
      patch, actual, {5, 6, 7, 8, 9, 10}, {5, 6, 7, 8, 9, 10, 7, 8, 9, 10});
}

TEST(StructPatchTest, SetPatch) {
  SetPatch patch;
  patch.erase("a");
  patch.emplace("a");
  patch.insert("b");
  patch.add({"c"});
  patch.remove({"c", "d"});

  test::expectPatch(patch, {}, {"a", "b"});
  test::expectPatch(patch, {"a", "d", "e"}, {"a", "b", "e"});

  auto assignPatch = SetPatch::createAssign({"a", "d", "e"});
  assignPatch.erase("a");
  assignPatch.erase("f");
  assignPatch.emplace("a");
  assignPatch.insert("b");
  assignPatch.add({"c"});
  assignPatch.remove({"c", "d"});
  EXPECT_THAT(
      *assignPatch.toThrift().assign(), ::testing::ElementsAre("a", "b", "e"));
}

TEST(StructPatchTest, MapPatch) {
  MapPatch patch;
  patch.patchByKey("a") += "2";
  patch.put({{"a", "1"}, {"b", "2"}, {"e", "5"}});
  patch.erase("e");
  patch.insert_or_assign("b", "3");
  patch.insert_or_assign("c", "4");

  test::expectPatch(patch, {}, {{"a", "1"}, {"b", "3"}, {"c", "4"}});
  test::expectPatch(
      patch, {{"a", "5"}, {"c", "6"}}, {{"a", "1"}, {"b", "3"}, {"c", "4"}});

  auto assignPatch = MapPatch::createAssign({{"a", "5"}, {"c", "6"}});
  assignPatch.put({{"a", "1"}, {"b", "2"}});
  assignPatch.insert_or_assign("b", "3");
  assignPatch.insert_or_assign("c", "4");
  test::expectPatch(assignPatch, {}, {{"a", "1"}, {"b", "3"}, {"c", "4"}});
  assignPatch.patchByKey("a") += "2";
  test::expectPatch(assignPatch, {}, {{"a", "12"}, {"b", "3"}, {"c", "4"}});

  MapPatch addPatch;
  addPatch.add({{"a", "1"}, {"b", "2"}});
  test::expectPatch(addPatch, {}, {{"a", "1"}, {"b", "2"}});
  test::expectPatch(
      addPatch, {{"a", "0"}, {"c", "3"}}, {{"a", "0"}, {"b", "2"}, {"c", "3"}});

  // "a" is already added. It's no-op to add it again into the Patch.
  addPatch.add({{"a", "3"}});
  test::expectPatch(addPatch, {}, {{"a", "1"}, {"b", "2"}});
  addPatch.erase("a");
  test::expectPatch(
      addPatch, {{"a", "0"}, {"c", "3"}}, {{"b", "2"}, {"c", "3"}});
  addPatch.add({{"a", "4"}});
  test::expectPatch(
      addPatch, {{"a", "0"}, {"c", "3"}}, {{"a", "4"}, {"b", "2"}, {"c", "3"}});

  MapPatch putPatch;
  putPatch.put({{"a", "1"}, {"b", "2"}});
  test::expectPatch(putPatch, {}, {{"a", "1"}, {"b", "2"}});
  test::expectPatch(
      putPatch, {{"a", "0"}, {"c", "3"}}, {{"a", "1"}, {"b", "2"}, {"c", "3"}});

  MapPatch erasePatch;
  erasePatch.add({{"a", "1"}, {"b", "2"}});
  erasePatch.erase("c");
  test::expectPatch(erasePatch, {}, {{"a", "1"}, {"b", "2"}});
  test::expectPatch(
      erasePatch, {{"a", "0"}, {"c", "3"}}, {{"a", "0"}, {"b", "2"}});

  MapPatch removePatch;
  removePatch.add({{"a", "1"}, {"b", "2"}});
  removePatch.remove({"c", "d"});
  test::expectPatch(removePatch, {}, {{"a", "1"}, {"b", "2"}});
  test::expectPatch(
      removePatch,
      {{"a", "0"}, {"c", "3"}, {"d", "4"}},
      {{"a", "0"}, {"b", "2"}});

  MapPatch elementPatch;
  elementPatch.patchByKey("a") += "1";
  elementPatch.add({{"b", "2"}});
  elementPatch.patchByKey("b") += "1";

  EXPECT_TRUE(elementPatch.toThrift().patchPrior()->contains("a"));
  EXPECT_FALSE(elementPatch.toThrift().patchPrior()->contains("b"));
  EXPECT_TRUE(elementPatch.toThrift().patch()->contains("b"));
  EXPECT_FALSE(elementPatch.toThrift().patch()->contains("a"));

  test::expectPatch(
      elementPatch,
      {{"a", "0"}},
      {{"a", "01"}, {"b", "21"}},
      {{"a", "011"}, {"b", "211"}});

  MapPatch ensuredElementPatch;
  ensuredElementPatch.ensureAndPatchByKey("k") += "1";
  ensuredElementPatch.add({{"w", "2"}});
  ensuredElementPatch.ensureAndPatchByKey("w") += "1";
  ensuredElementPatch.erase("b");
  ensuredElementPatch.ensureAndPatchByKey("b") += "2";

  test::expectPatch(
      ensuredElementPatch,
      {{"a", "0"}},
      {{"a", "0"}, {"b", "2"}, {"k", "1"}, {"w", "21"}},
      {{"a", "0"}, {"b", "2"}, {"k", "11"}, {"w", "211"}});
}

TEST(StructPatchTest, MapPatchMerge) {
  std::vector<MapPatch::value_type> vec;
  vec.push_back({});
  vec.push_back({});
  for (char c = 'a'; c <= 'z'; ++c) {
    vec.back().insert({{c}, {c}});
  }

  MapPatch patchMerging1, patchMerging2;
  patchMerging2.patchByKey("d") += "2";
  patchMerging2.patchByKey("f") += "2";
  patchMerging2.patchByKey("e") += "3";
  patchMerging2.add({{"g", "6"}});
  patchMerging2.put({{"e", "5"}});
  patchMerging2.patchByKey("g") += "2";
  patchMerging2.erase("z");
  patchMerging2.patchByKey("z") += "2";

  EXPECT_EQ(patchMerging2.toThrift().patchPrior()->size(), 2);
  EXPECT_TRUE(patchMerging2.toThrift().patchPrior()->contains("d"));
  EXPECT_TRUE(patchMerging2.toThrift().patchPrior()->contains("f"));
  EXPECT_EQ(
      *patchMerging2.toThrift().put(),
      (std::map<std::string, std::string>{{"e", "5"}}));
  EXPECT_EQ(
      patchMerging2.toThrift().add(),
      (std::map<std::string, std::string>{{"g", "6"}}));
  EXPECT_EQ(
      *patchMerging2.toThrift().remove(),
      (std::unordered_set<std::string>{"z"}));
  EXPECT_EQ(patchMerging2.toThrift().patch()->size(), 1);
  EXPECT_TRUE(patchMerging2.toThrift().patch()->contains("g"));

  auto expectAfterMerge = [&](const std::unordered_set<std::string>& patchPrior,
                              const std::map<std::string, std::string>& add,
                              const std::unordered_set<std::string>& remove,
                              const std::map<std::string, std::string>& put,
                              const std::unordered_set<std::string>& patch) {
    patchMerging1.merge(patchMerging2);

    EXPECT_EQ(patchMerging1.toThrift().patchPrior()->size(), patchPrior.size());
    for (const auto& key : patchPrior) {
      EXPECT_TRUE(patchMerging1.toThrift().patchPrior()->contains(key))
          << "for key " << key;
    }
    EXPECT_EQ(*patchMerging1.toThrift().add(), add);
    EXPECT_EQ(*patchMerging1.toThrift().remove(), remove);
    EXPECT_EQ(*patchMerging1.toThrift().put(), put);
    EXPECT_EQ(patchMerging1.toThrift().patch()->size(), patch.size());
    for (const auto& key : patch) {
      EXPECT_TRUE(patchMerging1.toThrift().patch()->contains(key))
          << "for key " << key;
    }
  };

  { // merge with add
    patchMerging1 = MapPatch{};
    patchMerging1.patchByKey("a") += "1";
    patchMerging1.add({{"b", "2"}});
    patchMerging1.patchByKey("b") += "1";

    expectAfterMerge(
        {"a", "d", "f"}, // patchPrior
        {{"b", "2"}, {"g", "6"}}, // add
        {"z"}, // remove
        {{"e", "5"}}, // put
        {"b", "g"} // patchAfter
    );
  }

  { // merge with remove
    patchMerging1 = MapPatch{};
    patchMerging1.patchByKey("a") += "1";
    patchMerging1.erase("b");
    patchMerging1.patchByKey("b") += "1";

    expectAfterMerge(
        {"a", "d", "f"}, // patchPrior
        {{"g", "6"}}, // add
        {"b", "z"}, // remove
        {{"e", "5"}}, // put
        {"g"} // patchAfter
    );
  }

  { // merge with put
    patchMerging1 = MapPatch{};
    patchMerging1.patchByKey("a") += "1";
    patchMerging1.insert_or_assign("b", "2");
    patchMerging1.patchByKey("b") += "1";

    expectAfterMerge(
        {"a", "d", "f"}, // patchPrior
        {{"g", "6"}}, // add
        {"z"}, // remove
        {{"b", "2"}, {"e", "5"}}, // put
        {"b", "g"} // patchAfter
    );
  }

  { // merge with add, remove and put
    patchMerging1 = MapPatch{};
    patchMerging1.patchByKey("a") += "1";
    patchMerging1.add({{"d", "1"}});
    patchMerging1.insert_or_assign("b", "2");
    patchMerging1.patchByKey("b") += "1";
    patchMerging1.erase("f");

    expectAfterMerge(
        {"a"}, // patchPrior
        {{"d", "1"}, {"g", "6"}}, // add
        {"z", "f"}, // remove
        {{"b", "2"}, {"e", "5"}}, // put
        {"b", "d", "g"} // patchAfter
    );
  }
}

TEST(UnionPatchTest, Assign) {
  MyUnionPatch noop;
  MyUnionPatch assignEmpty = MyUnionPatch::createAssign({});
  EXPECT_FALSE(noop.modifies<ident::option1>());
  EXPECT_TRUE(assignEmpty.modifies<ident::option1>());

  MyUnion actual;
  test::expectPatch(noop, actual, {});
  test::expectPatch(assignEmpty, actual, {});

  actual.option1_ref() = "hi";
  MyUnionPatch assign;
  assign.assign<ident::option1>("hi");
  EXPECT_EQ(assign.toThrift(), MyUnionPatch::createAssign(actual).toThrift());
  test::expectPatch(noop, actual, actual);
  test::expectPatch(assignEmpty, actual, {});
  test::expectPatch(assign, actual, actual);
  test::expectPatch(assign, {}, actual);

  assignEmpty.apply(actual);
  EXPECT_EQ(actual.getType(), MyUnion::Type::__EMPTY__);
  assign.apply(actual);
  EXPECT_EQ(actual.getType(), MyUnion::Type::option1);
}

TEST(UnionPatchTest, Ensure) {
  MyUnionPatch patch;
  EXPECT_FALSE(patch.ensures<ident::option1>());
  EXPECT_FALSE(patch.modifies<ident::option1>());
  patch.ensure<ident::option1>("hi");
  EXPECT_TRUE(patch.ensures<ident::option1>());
  EXPECT_TRUE(patch.modifies<ident::option1>());

  MyUnion expected;
  expected.option1_ref() = "hi";

  // Empty -> expected
  MyUnion actual;
  test::expectPatch(patch, actual, expected);
  patch.apply(actual);
  EXPECT_EQ(actual.getType(), MyUnion::Type::option1);

  // Same type is untouched.
  actual.option1_ref() = "bye";
  test::expectPatch(patch, actual, actual);
  patch.apply(actual);
  EXPECT_EQ(actual.getType(), MyUnion::Type::option1);

  // Different type -> expected.
  actual.option2_ref() = 1;
  test::expectPatch(patch, actual, expected);
  patch.apply(actual);
  EXPECT_EQ(actual.getType(), MyUnion::Type::option1);
}

TEST(UnionPatchTest, Patch) {
  MyUnionPatch patch;
  patch.patch<ident::option1>() += "Na";

  MyUnion actual;
  MyUnion expected1;
  expected1.option1_ref() = "Na";
  MyUnion expected2;
  expected2.option1_ref() = "NaNa";
  test::expectPatch(patch, actual, expected1, expected2);

  actual.option2_ref() = 1;
  test::expectPatch(patch, actual, expected1, expected2);

  actual.option1_ref() = "Ba";
  expected1.option1_ref() = "BaNa";
  expected2.option1_ref() = "BaNaNa";
  test::expectPatch(patch, actual, expected1, expected2);
}

TEST(UnionPatchTest, PatchIfSet) {
  MyUnionPatch patch;
  patch.patchIfSet<ident::option1>() = "Hi";
  patch.ensure<ident::option1>("Bye");
  patch.patchIfSet<ident::option1>() += " World!";

  MyUnion hi, bye;
  hi.option1_ref() = "Hi World!";
  bye.option1_ref() = "Bye World!";

  test::expectPatch(patch, {}, bye, hi);

  MyUnion op1;
  op1.option1_ref() = "Yo";
  test::expectPatch(patch, op1, hi, hi);

  MyUnion op2;
  op2.option2_ref() = 42;
  test::expectPatch(patch, bye, hi);
}

TEST(UnionPatchTest, PatchIfSetPredicate) {
  MyUnionPatch patch;
  patch.patchIfSet<ident::option1>() = "Hi";
  patch.patchIfSet<ident::option2>() = 5;

  MyUnion option1;
  option1.option1_ref() = "";
  patch.apply(option1);
  EXPECT_EQ(option1.option1_ref().value(), "Hi");

  MyUnion option2;
  option2.option2_ref() = 0;
  patch.apply(option2);
  EXPECT_EQ(option2.option2_ref().value(), 5);
}

TEST(UnionPatchTest, PatchInner) {
  MyUnionPatch patch;
  patch.patchIfSet<ident::option3>().patchIfSet<ident::option1>() = "World";

  MyUnion a, b;
  a.option3_ref().ensure().option1_ref() = "Hello";
  b.option3_ref().ensure().option1_ref() = "World";

  test::expectPatch(patch, a, b);
}

TEST(UnionPatchTest, PatchMergeEnsure) {
  MyUnionPatch patch1;
  patch1.patch<ident::option1>() += "Na";

  MyUnionPatch patch2;
  patch2.patch<ident::option2>() += 4;

  auto patch = patch1;
  patch.merge(patch2);

  MyUnion actual;
  MyUnion expected1;
  expected1.option2_ref() = 4;
  MyUnion expected2;
  expected2.option2_ref() = 8;
  test::expectPatch(patch, actual, expected1, expected2);
}

TEST(StructPatchTest, NestedEmpty) {
  MyStructPatch patch;
  EXPECT_FALSE(patch.modifies<ident::structVal>());
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));
}

TEST(StructPatchTest, NestedAssign) {
  auto patch = MyStructPatch::createAssign(testValue());
  EXPECT_TRUE(patch.modifies<ident::structVal>());
  EXPECT_TRUE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_TRUE((patch.modifies<ident::structVal, ident::data2>()));
}

TEST(StructPatchTest, NestedClear) {
  auto patch = MyStructPatch::createClear();
  EXPECT_TRUE(patch.modifies<ident::structVal>());
  EXPECT_TRUE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_TRUE((patch.modifies<ident::structVal, ident::data2>()));
}

TEST(StructPatchTest, NestedEnsure) {
  MyStructPatch patch;
  patch.ensure<ident::structVal>();
  EXPECT_FALSE(patch.modifies<ident::structVal>());
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));
}

TEST(StructPatchTest, NestedPatch) {
  MyStructPatch patch;
  patch.patchIfSet<ident::structVal>().patchIfSet<ident::data1>() = "10";
  EXPECT_TRUE(patch.modifies<ident::structVal>());
  EXPECT_TRUE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));

  patch.patchIfSet<ident::structVal>().patchIfSet<ident::data1>() =
      op::StringPatch{};
  EXPECT_FALSE(patch.modifies<ident::structVal>());
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));

  patch.patchIfSet<ident::structVal>().patchIfSet<ident::data2>() = 20;
  EXPECT_TRUE(patch.modifies<ident::structVal>());
  EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
  EXPECT_TRUE((patch.modifies<ident::structVal, ident::data2>()));
}

enum class PatchLocation { Prior, After };

template <typename Id, typename StructPatch>
auto& get_inner_patch(StructPatch&& patch, PatchLocation loc) {
  switch (loc) {
    case PatchLocation::Prior:
      return *op::get<Id>(patch.toThrift().patchPrior()->toThrift());
    case PatchLocation::After:
      return *op::get<Id>(patch.toThrift().patch()->toThrift());
  }

  CHECK(false);
}

TEST(StructPatchTest, NestedPatchWithDifferentLocation) {
  auto locations = {PatchLocation::Prior, PatchLocation::After};
  for (auto inner1 : locations) {
    for (auto outer1 : locations) {
      for (auto inner2 : locations) {
        for (auto outer2 : locations) {
          MyStructPatch patch;
          EXPECT_FALSE(patch.modifies<ident::structVal>());
          EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
          EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));

          get_inner_patch<ident::data1>(
              get_inner_patch<ident::structVal>(patch, outer1), inner1) = "10";
          EXPECT_TRUE(patch.modifies<ident::structVal>());
          EXPECT_TRUE((patch.modifies<ident::structVal, ident::data1>()));
          EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));

          get_inner_patch<ident::data2>(
              get_inner_patch<ident::structVal>(patch, outer2), inner2) = 20;
          EXPECT_TRUE(patch.modifies<ident::structVal>());
          EXPECT_TRUE((patch.modifies<ident::structVal, ident::data1>()));
          EXPECT_TRUE((patch.modifies<ident::structVal, ident::data2>()));

          get_inner_patch<ident::data1>(
              get_inner_patch<ident::structVal>(patch, outer1), inner1) =
              op::StringPatch{};
          EXPECT_TRUE(patch.modifies<ident::structVal>());
          EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
          EXPECT_TRUE((patch.modifies<ident::structVal, ident::data2>()));

          get_inner_patch<ident::data2>(
              get_inner_patch<ident::structVal>(patch, outer2), inner2) =
              op::I32Patch{};
          EXPECT_FALSE(patch.modifies<ident::structVal>());
          EXPECT_FALSE((patch.modifies<ident::structVal, ident::data1>()));
          EXPECT_FALSE((patch.modifies<ident::structVal, ident::data2>()));
        }
      }
    }
  }
}

TEST(StructPatchTest, InternBox) {
  MyStructPatch patch;
  // Value comparison match.
  EXPECT_EQ(
      std::as_const(patch).toThrift().patch()->toThrift().floatVal().value(),
      std::as_const(patch)
          .toThrift()
          .patchPrior()
          ->toThrift()
          .floatVal()
          .value());
  // Address comparison match.
  EXPECT_EQ(
      &std::as_const(patch).toThrift().patch()->toThrift().floatVal().value(),
      &std::as_const(patch)
           .toThrift()
           .patchPrior()
           ->toThrift()
           .floatVal()
           .value());
  // mut access.
  (void)patch.patch<ident::floatVal>();
  // Value comparison match.
  EXPECT_EQ(
      std::as_const(patch).toThrift().patch()->toThrift().floatVal().value(),
      std::as_const(patch)
          .toThrift()
          .patchPrior()
          ->toThrift()
          .floatVal()
          .value());
  // Address comparison does not match.
  EXPECT_NE(
      &std::as_const(patch).toThrift().patch()->toThrift().floatVal().value(),
      &std::as_const(patch)
           .toThrift()
           .patchPrior()
           ->toThrift()
           .floatVal()
           .value());
}

template <typename T>
struct is_terse_intern_boxed_field_ref : std::false_type {};
template <typename T>
struct is_terse_intern_boxed_field_ref<terse_intern_boxed_field_ref<T>>
    : std::true_type {};

TEST(StructPatchTest, EnsureStruct) {
  MyDataPatch patch;

  patch.ensure<ident::data1>();
  patch.ensure<ident::data3>("20");

  MyData data;
  EXPECT_EQ(data.data1(), "");
  EXPECT_FALSE(data.data3().has_value());

  patch.apply(data);
  EXPECT_EQ(data.data1(), "");
  EXPECT_EQ(data.data3(), "20");
}

TEST(StructPatchTest, EnsureUnion) {
  MyUnionPatch patch;

  patch.ensure<ident::option1>("10");
  patch.ensure<ident::option2>(20);

  MyUnion data;
  EXPECT_FALSE(data.option1_ref());
  EXPECT_FALSE(data.option2_ref());

  patch.apply(data);
  EXPECT_FALSE(data.option1_ref());
  EXPECT_EQ(data.option2_ref(), 20);
}

TEST(StructPatchTest, EnsureStructValPatchable) {
  MyData data;
  data.data3() = "";

  MyStructPatch patch;
  patch.patchIfSet<ident::structVal>().assign(data);

  MyStruct foo;
  patch.apply(foo);
  EXPECT_EQ(foo.structVal(), data);

  // Ensure will be no-op since data2 is non-optional field
  patch.patchIfSet<ident::structVal>().ensure<ident::data2>();
  patch.apply(foo);
  EXPECT_EQ(foo.structVal(), data);
}

TEST(StructPatchTest, AssignAndPatch) {
  {
    MyStructPatch patch;
    MyStruct myStruct;
    myStruct.optStringVal() = "1";
    patch = myStruct;
    patch.patchIfSet<ident::optStringVal>() += "2";

    MyStruct foo;
    foo.optStringVal() = "3";
    patch.apply(foo);
    EXPECT_EQ(foo.optStringVal(), "12");
  }
  {
    MyStructPatch patch;
    MyStruct myStruct;
    myStruct.optStringVal() = "1";
    patch = myStruct;
    patch.patchIfSet<ident::optStringVal>().clear();

    MyStruct foo;
    foo.optStringVal() = "3";
    patch.apply(foo);
    EXPECT_FALSE(foo.optStringVal().has_value());
  }
  {
    MyStructPatch patch;
    patch.assign({});
    patch.patchIfSet<ident::stringVal>() += "1";

    MyStruct foo;
    foo.stringVal() = "2";
    patch.apply(foo);
    EXPECT_EQ(foo.stringVal(), "1");
  }
}

TEST(StructPatchTest, EnsureOptStructValPatchable) {
  MyData data;
  data.data2() = 42;

  MyStructPatch patch;
  patch.patchIfSet<ident::optStructVal>().assign(data);

  MyStruct foo;
  foo.optStructVal().ensure();
  patch.apply(foo);
  EXPECT_EQ(foo.optStructVal(), data);

  patch.patchIfSet<ident::optStructVal>().ensure<ident::data2>();
  patch.apply(foo);
  EXPECT_EQ(foo.optStructVal(), data);
}

TEST(StructPatchTest, PatchOptField) {
  MyStructPatch patch;
  patch.patchIfSet<ident::optI32Val>().clear();
  patch.patchIfSet<ident::optI32Val>() = 0;

  MyStruct foo;
  foo.optI32Val() = 0;
  patch.apply(foo);
  EXPECT_FALSE(foo.optI32Val().has_value());
}

TEST(StructPatchTest, IncludePatch) {
  MyData data;
  data.data2() = 42;

  test::patch::IncludePatchStruct patch1;
  patch1.patch()->patch<ident::data2>() -= 10;
  patch1.patch()->apply(data);
  EXPECT_EQ(data.data2(), 32);

  test::patch::IncludePatchUnion patch2;
  patch2.patch_ref().emplace().patch<ident::data2>() += 10;
  patch2.patch_ref()->apply(data);
  EXPECT_EQ(data.data2(), 42);
}

TEST(StructPatchTest, RequiredFieldPatch) {
  WithRequiredFieldsPatch patch;
  patch.patch<ident::required_int>() = 10;

  WithRequiredFields data;
  patch.apply(data);

  EXPECT_EQ(*data.required_int(), 10);
}

TEST(PatchTest, IsPatch) {
  using op::is_patch_v;
  static_assert(!is_patch_v<bool>);
  static_assert(is_patch_v<op::BoolPatch>);

  static_assert(!is_patch_v<std::string>);
  static_assert(is_patch_v<op::StringPatch>);

  static_assert(!is_patch_v<test::patch::MyStruct>);
  static_assert(is_patch_v<test::patch::MyStructPatch>);
  static_assert(!is_patch_v<test::patch::MyStructPatchStruct>);
  static_assert(is_patch_v<test::patch::MyStructFieldPatch>);
  static_assert(!is_patch_v<test::patch::MyStructEnsureStruct>);

  static_assert(is_patch_v<ListPatch>);
  static_assert(is_patch_v<SetPatch>);
  static_assert(is_patch_v<MapPatch>);

  // Assign only patch
  static_assert(is_patch_v<test::patch::BarPatch>);
  static_assert(op::is_assign_only_patch_v<test::patch::BarPatch>);
  static_assert(!op::is_assign_only_patch_v<op::BoolPatch>);
  static_assert(!op::is_assign_only_patch_v<test::patch::MyStructPatch>);

  using YourData = MyDataPatch;
  static_assert(is_patch_v<YourData>);

  struct CounterfeitBoolPatch {
    using value_type = bool;
  };
  static_assert(!is_patch_v<CounterfeitBoolPatch>);
}

TEST(PatchTest, StructRemove) {
  MyStructPatch patch;
  patch.patch<ident::optI16Val>().clear();
  patch.patchIfSet<ident::optI32Val>().clear();
  patch.patchIfSet<ident::i16Val>().clear();

  MyStruct obj;
  obj.optI16Val() = 16;
  obj.optI32Val() = 32;
  obj.optI64Val() = 64;
  obj.i16Val() = 160;
  obj.i32Val() = 320;

  protocol::Value value =
      protocol::asValueStruct<type::struct_t<MyStruct>>(obj);

  protocol::applyPatch(patch.toObject(), value);

  auto buffer = protocol::serializeValue<CompactProtocolWriter>(value);

  MyStruct patched;
  CompactSerializer::deserialize(buffer.get(), patched);

  EXPECT_FALSE(patched.optI16Val().has_value());
  EXPECT_FALSE(patched.optI32Val().has_value());
  EXPECT_EQ(patched.optI64Val(), 64);
  EXPECT_EQ(patched.i16Val(), 0);
  EXPECT_EQ(patched.i32Val(), 320);

  std::set<FieldId> ids;

  EXPECT_TRUE(patch.toThrift().remove()->empty());
  auto remove = patch.toObject().at(static_cast<FieldId>(op::PatchOp::Remove));
  for (auto id : remove.as_list()) {
    ids.insert(static_cast<FieldId>(id.as_i16()));
  }
  EXPECT_EQ(
      ids,
      (std::set{
          op::get_field_id<MyStruct, ident::optI16Val>::value,
          op::get_field_id<MyStruct, ident::optI32Val>::value,
      }));
}

TEST(PatchTest, StructRemoveSerialization) {
  using test::patch::IncludePatchStruct;
  IncludePatchStruct s;
  s.patch()->patch<ident::data3>().clear();

  // Do a round-trip and check PatchOp::Remove field.
  auto t = CompactSerializer::deserialize<IncludePatchStruct>(
      CompactSerializer::serialize<std::string>(s));

  EXPECT_TRUE(s.patch()->toThrift().remove()->empty());
  EXPECT_EQ(
      t.patch()->toThrift().remove(),
      std::unordered_set{static_cast<FieldId>(3)});
}

TEST(PatchDiscrepancy, AssignOnly) {
  AssignOnly foo;
  foo.field() = 1;
  auto dynFoo = protocol::asValueStruct<type::infer_tag<AssignOnly>>(foo);

  // Create a patch with ClearOp
  protocol::Object dynPatch;
  dynPatch[static_cast<FieldId>(op::PatchOp::Clear)].emplace_bool() = true;

  // Convert dynamic patch to static patch
  protocol::Value patchValue;
  patchValue.emplace_object() = dynPatch;
  auto patch =
      protocol::fromValueStruct<type::infer_tag<AssignOnlyPatch>>(patchValue);

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field(), 0);

  // Apply patch dynamically
  protocol::applyPatch(dynPatch, dynFoo);
  EXPECT_FALSE(dynFoo.as_object().contains(FieldId{1}));
}

TEST(PatchDiscrepancy, ClearOptionalFieldInPatchPrior) {
  Opt foo;
  foo.field() = 10;
  auto dynFoo = protocol::asValueStruct<type::struct_t<Opt>>(foo);

  OptPatch patch;
  patch.patchIfSet<ident::field>().clear();
  patch.ensure<ident::field>(1);

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field(), 1);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 1);
}

TEST(PatchDiscrepancy, ClearUnionFieldInPatchPrior) {
  Union foo;
  foo.field_ref() = 10;
  auto dynFoo = protocol::asValueStruct<type::union_t<Union>>(foo);

  UnionPatch patch;
  patch.patchIfSet<ident::field>().clear();
  patch.ensure<ident::field>(1);

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field_ref(), 0);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 0);
}

TEST(PatchDiscrepancy, PatchUnqualifiedField) {
  Def foo;
  foo.field() = 0;
  auto dynFoo = protocol::asValueStruct<type::infer_tag<Def>>(foo);

  DefPatch patch;
  patch.clear();
  patch.patchIfSet<ident::field>() += 1;

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field(), 1);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 1);
}

TEST(PatchDiscrepancy, PatchTerseField) {
  Ter foo;
  foo.field() = 0;

  // We can't use protocol::asValueStruct, we need to do an actual serialization
  // to ensure terse field is not set in dynamic Foo.
  auto iobuf = *CompactSerializer::serialize<folly::IOBufQueue>(foo).move();
  protocol::Value dynFoo;
  dynFoo.emplace_object() = protocol::parseObject<CompactProtocolReader>(iobuf);

  protocol::asValueStruct<type::infer_tag<Ter>>(foo);

  TerPatch patch;
  patch.patchIfSet<ident::field>() += 1;

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field(), 1);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 1);
}

TEST(PatchDiscrepancy, PatchOptionalField) {
  Opt foo;
  foo.field() = 0;
  auto dynFoo = protocol::asValueStruct<type::infer_tag<Opt>>(foo);

  OptPatch patch;
  patch.clear();
  patch.patchIfSet<ident::field>() += 1;

  // Apply patch statically
  patch.apply(foo);
  EXPECT_FALSE(foo.field().has_value());

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_FALSE(dynFoo.as_object().contains(FieldId{1}));
}

TEST(PatchDiscrepancy, EnsureUnqualifiedField) {
  Def foo;
  foo.field() = 0;
  auto dynFoo = protocol::asValueStruct<type::infer_tag<Def>>(foo);

  DefPatch patch;
  patch.clear();
  patch.ensure<ident::field>();

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field(), 0);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 0);
}

TEST(PatchDiscrepancy, EnsureTerseField) {
  Ter foo;
  foo.field() = 0;

  // We can't use protocol::asValueStruct, we need to do an actual serialization
  // to ensure terse field is not set in dynamic Foo.
  auto iobuf = *CompactSerializer::serialize<folly::IOBufQueue>(foo).move();
  protocol::Value dynFoo;
  dynFoo.emplace_object() = protocol::parseObject<CompactProtocolReader>(iobuf);

  protocol::asValueStruct<type::infer_tag<Ter>>(foo);

  TerPatch patch;
  patch.ensure<ident::field>();

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field(), 0);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 0);
}

TEST(PatchDiscrepancy, EnsureUnionField) {
  Union foo;
  foo.field_ref() = 0;
  auto dynFoo = protocol::asValueStruct<type::infer_tag<Union>>(foo);

  UnionPatch patch;
  patch.clear();
  patch.ensure<ident::field>(1);

  // Apply patch statically
  patch.apply(foo);
  EXPECT_EQ(foo.field_ref(), 1);

  // Apply patch dynamically
  protocol::applyPatch(patch.toObject(), dynFoo);
  EXPECT_EQ(dynFoo.as_object()[FieldId{1}].as_i32(), 1);
}

TEST(StructPatchTest, SerializationExample) {
  MyDataPatch patch;

  patch.ensure<ident::data1>();
  patch.ensure<ident::data3>("20");

  auto expected = CompactSerializer::serialize<std::string>(patch.toThrift());

  folly::IOBufQueue queue;
  CompactProtocolWriter protoWriter;
  protoWriter.setOutput(&queue);
  op::encode<type::infer_tag<MyDataPatch>>(protoWriter, patch);
  auto buf = queue.move();

  MyDataPatch fromSerialize{
      CompactSerializer::deserialize<MyDataPatchStruct>(expected)};

  MyDataPatch fromOpEncode;
  CompactProtocolReader protoReader;
  protoReader.setInput(buf.get());
  op::decode<type::infer_tag<MyDataPatch>>(protoReader, fromOpEncode);

  EXPECT_EQ(fromSerialize, fromOpEncode);
}

TEST(StructPatchTest, Remove) {
  // TODO(dokwon): Replace to Static Patch remove operation.
  MyDataPatch patch;
  patch.toThrift().remove() = {FieldId{1}, FieldId{2}, FieldId{3}};

  MyData data;
  data.data1() = "1";
  data.data2() = 2;
  data.data3() = "3";

  patch.apply(data);

  EXPECT_EQ(data.data1(), "");
  EXPECT_EQ(data.data2(), 0);
  EXPECT_FALSE(data.data3().has_value());
}

TEST(StructPatchTest, AssignOnly) {
  protocol::Object dynPatch;

  // foo.patch<ident::field>() += 100;
  dynPatch[static_cast<FieldId>(op::PatchOp::EnsureStruct)]
      .emplace_object()[FieldId{1}]
      .emplace_i32() = 0;
  dynPatch[static_cast<FieldId>(op::PatchOp::PatchAfter)]
      .emplace_object()[FieldId{1}]
      .emplace_object()[static_cast<FieldId>(op::PatchOp::Add)]
      .emplace_i32() = 100;

  // foo.ensure<ident::field>(10);
  // foo.patch<ident::field>() += 200;
  dynPatch[static_cast<FieldId>(op::PatchOp::EnsureStruct)]
      .as_object()[FieldId{2}]
      .emplace_i32() = 10;
  dynPatch[static_cast<FieldId>(op::PatchOp::PatchAfter)]
      .as_object()[FieldId{2}]
      .emplace_object()[static_cast<FieldId>(op::PatchOp::Add)]
      .emplace_i32() = 200;

  auto genPatch = [&dynPatch] {
    // Serialize dynamic patch and deserialize as AssignOnly static patch
    auto buf = *protocol::serializeObject<CompactProtocolWriter>(dynPatch);
    AssignOnlyPatch patch;
    CompactProtocolReader reader;
    reader.setInput(&buf);
    op::decode<type::infer_tag<AssignOnlyPatch>>(reader, patch);
    return patch;
  };

  auto patch = genPatch();

  {
    AssignOnly foo;
    patch.apply(foo);
    EXPECT_EQ(foo.field(), 100);
    EXPECT_EQ(foo.optField(), 210);
  }

  {
    AssignOnlyPatch patch2;
    patch2.merge(patch);
    AssignOnly foo;
    patch2.apply(foo);
    EXPECT_EQ(foo.field(), 100);
    EXPECT_EQ(foo.optField(), 210);
  }

  {
    AssignOnly bar;
    bar.field() = 300;
    patch = bar;

    AssignOnly foo;
    patch.apply(foo);
    EXPECT_EQ(foo.field(), 300);
    EXPECT_FALSE(foo.optField().has_value());
  }

  {
    auto patch2 = genPatch();
    patch2.merge(patch);

    AssignOnly foo;
    patch2.apply(foo);
    EXPECT_EQ(foo.field(), 300);
    EXPECT_FALSE(foo.optField().has_value());
  }

  {
    AssignOnly foo;
    patch.merge(genPatch());
    patch.apply(foo);
    EXPECT_EQ(foo.field(), 400);
    EXPECT_EQ(foo.optField(), 210);

    AssignOnly foo2;
    patch.merge(genPatch());
    patch.apply(foo2);
    EXPECT_EQ(foo2.field(), 500);
    EXPECT_EQ(foo2.optField(), 410);
  }

  {
    auto patch2 = genPatch();
    EXPECT_THROW(patch2.merge(genPatch()), std::runtime_error);
  }
}

} // namespace
} // namespace apache::thrift
