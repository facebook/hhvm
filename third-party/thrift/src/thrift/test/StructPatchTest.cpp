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
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_fatal_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_types.h>

namespace apache::thrift {
namespace {

using namespace test::patch;

static_assert(::apache::thrift::adapt_detail::has_inplace_toThrift<
              ::apache::thrift::op::detail::FieldPatchAdapter,
              MyStructFieldPatch>::value);

using ListPatch =
    std::decay_t<decltype(*std::declval<MyStructFieldPatch>()->optListVal())>;
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
  auto patch = MyStructPatch::createAssign(testValue());
  // For the patch to break apart the assign and check the result;
  patch.patchIfSet<ident::optI64Val>();
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_TRUE(*patch.toThrift().clear());
  EXPECT_EQ(*patch.toThrift().ensure(), testValue());
  test::expectPatch(patch, {}, testValue());
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
  (*std::move(patch).toThrift().patchPrior())
      .toThrift()
      .stringVal()
      ->prepend("p");
  expected1.stringVal() = "_phi_";
  expected2.stringVal() = "_p_phi__";
  test::expectPatch(patch, val, expected1, expected2);

  patch.merge(MyStructPatch::createClear());
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_EQ(patch.toThrift().patchPrior(), MyStructFieldPatch{});
  EXPECT_EQ(patch.toThrift().patch(), MyStructFieldPatch{});
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
  std::optional<std::string> optStr;

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
  patch.patchIfSet<ident::stringVal>().apply(optStr);
  EXPECT_EQ(actual, MyStruct{});
  EXPECT_FALSE(optStr.has_value());

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
  optStr = "hi";
  test::expectPatch(
      patch.patchIfSet<ident::stringVal>(), optStr, "_hi_", "__hi__");

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
  patch.patchIfSet<ident::stringVal>().apply(optStr);
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
  EXPECT_EQ(*optStr, "_hi_");
}

TEST(StructPatchTest, ListPatch) {
  ListPatch patch;
  patch.prepend({3, 4});
  patch.emplace_front(2);
  patch.push_front(1);
  ListPatch::value_type actual{5, 6};
  patch.append({7, 8});
  patch.emplace_back(9);
  patch.push_back(10);
  test::expectPatch(
      patch,
      actual,
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
      {1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 7, 8, 9, 10});

  ListPatch erasePatch;
  erasePatch.erase(1);
  test::expectPatch(
      erasePatch,
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
      {2, 3, 4, 5, 6, 7, 8, 9, 10});

  ListPatch removePatch;
  removePatch.remove({1, 2, 3, 4});
  test::expectPatch(
      removePatch, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, {5, 6, 7, 8, 9, 10});

  ListPatch elementPatch;
  elementPatch.patchAt(0) += 1;
  test::expectPatch(elementPatch, {1, 2, 3}, {2, 2, 3}, {3, 2, 3});
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
  assignPatch.emplace("a");
  assignPatch.insert("b");
  assignPatch.add({"c"});
  assignPatch.remove({"c", "d"});
  EXPECT_THAT(
      *assignPatch.toThrift().assign(), ::testing::ElementsAre("a", "b", "e"));
}

TEST(StructPatchTest, MapPatch) {
  MapPatch patch;
  patch.put({{"a", "1"}, {"b", "2"}});
  patch.insert_or_assign("b", "3");
  patch.insert_or_assign("c", "4");

  test::expectPatch(patch, {}, {{"a", "1"}, {"b", "3"}, {"c", "4"}});
  test::expectPatch(
      patch, {{"a", "5"}, {"c", "6"}}, {{"a", "1"}, {"b", "3"}, {"c", "4"}});

  auto assignPatch = MapPatch::createAssign({{"a", "5"}, {"c", "6"}});
  assignPatch.put({{"a", "1"}, {"b", "2"}});
  assignPatch.insert_or_assign("b", "3");
  assignPatch.insert_or_assign("c", "4");
  EXPECT_EQ(
      *assignPatch.toThrift().assign(),
      (std::map<std::string, std::string>(
          {{"a", "1"}, {"b", "3"}, {"c", "4"}})));

  MapPatch addPatch;
  addPatch.add({{"a", "1"}, {"b", "2"}});
  test::expectPatch(addPatch, {}, {{"a", "1"}, {"b", "2"}});
  test::expectPatch(
      addPatch, {{"a", "0"}, {"c", "3"}}, {{"a", "0"}, {"b", "2"}, {"c", "3"}});

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
  EXPECT_EQ(patchMerging2.toThrift().patch()->size(), 2);
  EXPECT_TRUE(patchMerging2.toThrift().patch()->contains("g"));
  EXPECT_TRUE(patchMerging2.toThrift().patch()->contains("z"));

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
  EXPECT_EQ(
      patch.toThrift(), decltype(patch)::createEnsure(expected).toThrift());

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
  *patch.patchIfSet()->option1() = "Hi";
  patch.ensure().option1_ref() = "Bye";
  *patch.patchIfSet()->option1() += " World!";

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

TEST(UnionPatchTest, PatchInner) {
  MyUnionPatch patch;
  *patch.patchIfSet()->option3()->patchIfSet()->option1() = "World";

  MyUnion a, b;
  a.option3_ref().ensure().option1_ref() = "Hello";
  b.option3_ref().ensure().option1_ref() = "World";

  test::expectPatch(patch, a, b);
}

} // namespace
} // namespace apache::thrift
