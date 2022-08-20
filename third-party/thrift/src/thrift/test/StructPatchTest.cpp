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
#include <thrift/test/gen-cpp2/StructPatchTest_fatal_types.h>
#include <thrift/test/gen-cpp2/StructPatchTest_types.h>

namespace apache::thrift {
namespace {
using namespace test::patch;

using ListPatch = std::decay_t<
    decltype(std::declval<MyStructFieldPatch>()->optListVal()->ensure())>;
using SetPatch = std::decay_t<
    decltype(std::declval<MyStructFieldPatch>()->optSetVal()->ensure())>;
using MapPatch = std::decay_t<
    decltype(std::declval<MyStructFieldPatch>()->optMapVal()->ensure())>;

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
  MyStructPatch patch;
  patch.patch()->boolVal() = !op::BoolPatch{};
  *patch->byteVal() = val.byteVal();
  *patch->i16Val() += 2;
  *patch->i32Val() += 3;
  *patch->i64Val() += 4;
  *patch->floatVal() += 5;
  *patch->doubleVal() += 6;
  patch->stringVal() = "_" + op::StringPatch{} + "_";
  patch->structVal()->patch()->data1()->append("Na");
  return patch;
}

MyStructPatch testOptPatch() {
  auto val = testValue();
  MyStructPatch patch;
  patch.patch()->optBoolVal()->patch() = !op::BoolPatch{};
  patch->optByteVal()->patch() = val.byteVal();
  patch->optI16Val()->patch() += 2;
  patch->optI32Val()->patch() += 3;
  patch->optI64Val()->patch() += 4;
  patch->optFloatVal()->patch() += 5;
  patch->optDoubleVal()->patch() += 6;
  patch->optStringVal()->patch() = "_" + op::StringPatch{} + "_";
  MyData data;
  data.data2() = 5;
  patch->optStructVal()->patch() = std::move(data);
  return patch;
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
  // Break apart the assign patch and check the result;
  patch.patch();
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_TRUE(*patch.toThrift().clear());
  EXPECT_NE(*patch.toThrift().patchPrior(), MyStructFieldPatch{});
  test::expectPatch(patch, {}, testValue());
}

TEST(StructPatchTest, Clear) {
  // Clear patch, clears all fields (even ones with defaults)
  test::expectPatch(MyStructPatch::createClear(), testValue(), {});
  test::expectPatch(op::StringPatch::createClear(), {"hi"}, "");
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

  patch.merge(MyStructPatch::createClear());
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_EQ(patch.patch(), MyStructFieldPatch{});
  EXPECT_TRUE(*patch.toThrift().clear());
  test::expectPatch(patch, testValue(), {});
}

TEST(StructPatchTest, ClearAssign) {
  auto patch = MyStructPatch::createClear();
  patch.merge(MyStructPatch::createAssign(testValue()));
  // Assign takes precedence, like usual.
  test::expectPatch(patch, {}, testValue());
}

TEST(StructPatchTest, AssignClear) {
  auto patch = MyStructPatch::createAssign(testValue());
  patch.merge(MyStructPatch::createClear());
  test::expectPatch(patch, testValue(), {});

  // Clear patch takes precedence (as it is smaller to encode and slightly
  // stronger in the presense of non-terse non-optional fields).
  EXPECT_FALSE(patch.toThrift().assign().has_value());
  EXPECT_TRUE(*patch.toThrift().clear());
}

TEST(StructPatchTest, OptionalFields) {
  MyStructPatch patch = testPatch();
  MyStructPatch optPatch = testOptPatch();

  MyStruct actual;
  std::optional<std::string> optStr;

  // Applying a value patch to void does nothing.
  test::expectPatch(optPatch, {}, {});
  EXPECT_EQ(actual, MyStruct{});
  patch->boolVal()->apply(actual.optBoolVal());
  patch->byteVal()->apply(actual.optByteVal());
  patch->i16Val()->apply(actual.optI16Val());
  patch->i32Val()->apply(actual.optI32Val());
  patch->i64Val()->apply(actual.optI64Val());
  patch->floatVal()->apply(actual.optFloatVal());
  patch->doubleVal()->apply(actual.optDoubleVal());
  patch->stringVal()->apply(actual.optStringVal());
  patch->structVal()->apply(actual.optStructVal());
  patch->stringVal()->apply(optStr);
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
  test::expectPatch(*patch->stringVal(), optStr, "_hi_", "__hi__");

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

  patch->boolVal()->apply(actual.optBoolVal());
  patch->byteVal()->apply(actual.optByteVal());
  patch->i16Val()->apply(actual.optI16Val());
  patch->i32Val()->apply(actual.optI32Val());
  patch->i64Val()->apply(actual.optI64Val());
  patch->floatVal()->apply(actual.optFloatVal());
  patch->doubleVal()->apply(actual.optDoubleVal());
  patch->stringVal()->apply(actual.optStringVal());
  patch->structVal()->apply(actual.optStructVal());
  patch->stringVal()->apply(optStr);
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

TEST(StructPatchTest, OptionalPatch) {
  op::OptionalBoolPatch patch;
  std::optional<bool> actual;
  MyStruct fields;
  op::OptionalBoolPatch restorePatch;
  restorePatch = fields.optBoolVal();

  {
    SCOPED_TRACE("= -> clear + ensure");
    patch = true;
    test::expectPatch(patch, false, true);
    test::expectPatch(patch, actual, true);
    patch.apply(fields.optBoolVal());
    patch.apply(actual);
    EXPECT_EQ(fields.optBoolVal(), true);
    EXPECT_EQ(actual, true);
  }

  { // Restore the original state.
    SCOPED_TRACE("restore");
    test::expectPatch(restorePatch, actual, std::nullopt);
    restorePatch.apply(fields.optBoolVal());
    restorePatch.apply(actual);
    EXPECT_FALSE(fields.optBoolVal().has_value());
    EXPECT_FALSE(actual.has_value());
    patch.reset();
  }

  // Complex patch:
  // set -> invert -> invert
  // unset -> true -> false.
  patch->invert();
  patch.ensure(true);
  test::expectPatch(patch, actual, true, false);
  test::expectPatch(patch, std::make_optional(true), false, true);
  test::expectPatch(patch, std::make_optional(false), true, false);
  patch.apply(fields.optBoolVal());
  ASSERT_TRUE(fields.optBoolVal().has_value());
  EXPECT_TRUE(*fields.optBoolVal());
  patch.apply(fields.optBoolVal());
  ASSERT_TRUE(fields.optBoolVal().has_value());
  EXPECT_FALSE(*fields.optBoolVal());
}

TEST(StructPatchTest, OptionalPatch_BadAccess) {
  op::OptionalBoolPatch patch;
  patch.clear();
  // The field is guaranteed to be empty, so throw a bad optional access
  // exception.
  EXPECT_THROW(patch.patch() = true, op::bad_patch_access);

  // If the patch is ensured, patch access no longer throws.
  patch.ensure();
  patch.patch() = true;
  // And clear is still set.
  EXPECT_TRUE(*patch.toThrift().clear());
}

TEST(StructPatchTest, PrimitivesNotBoxed) {
  test::same_type<
      decltype(std::declval<op::OptionalBinaryPatchStruct>().ensure()),
      optional_field_ref<folly::IOBuf&&>>;
}

TEST(StructPatchTest, FieldPatch) {
  MyStructPatch patch;
  patch->optListVal()->ensure() = {1, 2};
  MyStruct expected;
  expected.optListVal().ensure() = {1, 2};
  test::expectPatch(patch, {}, expected);
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
}

TEST(UnionPatchTest, ClearAndAssign) {
  MyUnionPatch noop;
  MyUnion actual;
  MyUnionPatch assignEmpty = MyUnionPatch::createAssign(actual);
  EXPECT_EQ(assignEmpty.toThrift(), MyUnionPatch::createClear().toThrift());
  EXPECT_EQ(actual.getType(), MyUnion::Type::__EMPTY__);
  EXPECT_EQ(*assignEmpty.toThrift().clear(), true);
  EXPECT_EQ(
      assignEmpty.toThrift().ensure()->getType(), MyUnion::Type::__EMPTY__);

  EXPECT_EQ(actual, MyUnion{});
  test::expectPatch(noop, actual, {});
  test::expectPatch(assignEmpty, actual, {});

  actual.option1_ref() = "hi";
  MyUnionPatch assign = MyUnionPatch::createAssign(actual);
  EXPECT_EQ(actual.getType(), MyUnion::Type::option1);
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
  MyUnion expected, actual;
  patch.ensure().option1_ref() = "hi";
  expected.option1_ref() = "hi";
  EXPECT_EQ(
      patch.toThrift(), decltype(patch)::createEnsure(expected).toThrift());

  // Empty -> expected
  test::expectPatch(patch, {}, expected);
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
  *patch.patch()->option1() = "Hi";
  patch.ensure().option1_ref() = "Bye";
  *patch.patch()->option1() += " World!";

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
  *patch.patch()->option3()->patch()->option1() = "World";

  MyUnion a, b;
  a.option3_ref().ensure().option1_ref() = "Hello";
  b.option3_ref().ensure().option1_ref() = "World";

  test::expectPatch(patch, a, b);
}

} // namespace
} // namespace apache::thrift
