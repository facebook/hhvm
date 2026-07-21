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

#include <thrift/lib/cpp2/transcode/test/GoldenTestData.h>

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace apache::thrift::transcode::golden_test {
namespace {

fixture::GoldenInner inner(int32_t n, std::string label) {
  fixture::GoldenInner inner;
  inner.n() = n;
  inner.label() = std::move(label);
  return inner;
}

fixture::GoldenChoice idChoice(int32_t id) {
  fixture::GoldenChoice choice;
  choice.id() = id;
  return choice;
}

fixture::GoldenChoice nameChoice(std::string name) {
  fixture::GoldenChoice choice;
  choice.name() = std::move(name);
  return choice;
}

std::string nonUtf8String() {
  return std::string("prefix", 6) + std::string("\xff\xfe\xc3\x28", 4);
}

} // namespace

fixture::ScalarShapes Generate<fixture::ScalarShapes>::operator()() const {
  fixture::ScalarShapes value;
  value.bool_true() = true;
  value.bool_false() = false;
  value.byte_min() = std::numeric_limits<int8_t>::min();
  value.byte_max() = std::numeric_limits<int8_t>::max();
  value.i16_min() = std::numeric_limits<int16_t>::min();
  value.i16_max() = std::numeric_limits<int16_t>::max();
  value.i32_min() = std::numeric_limits<int32_t>::min();
  value.i32_max() = std::numeric_limits<int32_t>::max();
  value.i64_min() = std::numeric_limits<int64_t>::min();
  value.i64_max() = std::numeric_limits<int64_t>::max();
  value.f_regular() = 1.25f;
  value.f_negative_zero() = -0.0f;
  value.f_lowest() = std::numeric_limits<float>::lowest();
  value.d_regular() = -3.5;
  value.d_negative_zero() = -0.0;
  value.d_lowest() = std::numeric_limits<double>::lowest();
  value.empty_text() = "";
  value.escaped_text() = "line\nquote\"slash\\tab\t";
  value.empty_data() = "";
  value.data() = std::string("\x00\x01\x02\x7f\x80\xff", 6);
  value.known_enum() = fixture::GoldenEnum::Active;
  value.sparse_enum() = fixture::SparseEnum::First;
  return value;
}

void expectScalarShapes(
    const fixture::ScalarShapes& output, const fixture::ScalarShapes& input) {
  EXPECT_EQ(output, input);
  EXPECT_EQ(
      std::signbit(*output.f_negative_zero()),
      std::signbit(*input.f_negative_zero()));
  EXPECT_EQ(
      std::signbit(*output.d_negative_zero()),
      std::signbit(*input.d_negative_zero()));
}

fixture::NonUtf8StringShapes
Generate<fixture::NonUtf8StringShapes>::operator()() const {
  auto invalid = nonUtf8String();
  fixture::NonUtf8StringShapes value;
  value.text() = invalid;
  value.texts() = {"valid", invalid, ""};
  return value;
}

PreEncodedJson<fixture::NonUtf8StringShapes>
Generate<PreEncodedJson<fixture::NonUtf8StringShapes>>::operator()() const {
  auto value = generate<fixture::NonUtf8StringShapes>();
  std::string bytes;
  bytes.reserve(64 + value.text()->size() * 2);
  bytes += R"({"text":")";
  bytes += *value.text();
  bytes += R"(","texts":["valid",")";
  bytes += *value.text();
  bytes += R"(",""]})";
  return {.bytes = std::move(bytes), .value = std::move(value)};
}

fixture::SpecialFloatShapes Generate<fixture::SpecialFloatShapes>::operator()()
    const {
  fixture::SpecialFloatShapes value;
  value.f_nan() = std::numeric_limits<float>::quiet_NaN();
  value.f_pos_inf() = std::numeric_limits<float>::infinity();
  value.f_neg_inf() = -std::numeric_limits<float>::infinity();
  value.d_nan() = std::numeric_limits<double>::quiet_NaN();
  value.d_pos_inf() = std::numeric_limits<double>::infinity();
  value.d_neg_inf() = -std::numeric_limits<double>::infinity();
  return value;
}

void expectSpecialFloatShapes(
    const fixture::SpecialFloatShapes& output,
    const fixture::SpecialFloatShapes& input) {
  EXPECT_TRUE(std::isnan(*input.f_nan()));
  EXPECT_TRUE(std::isnan(*output.f_nan()));
  EXPECT_TRUE(std::isnan(*input.d_nan()));
  EXPECT_TRUE(std::isnan(*output.d_nan()));
  EXPECT_TRUE(std::isinf(*output.f_pos_inf()));
  EXPECT_GT(*output.f_pos_inf(), 0);
  EXPECT_TRUE(std::isinf(*output.f_neg_inf()));
  EXPECT_LT(*output.f_neg_inf(), 0);
  EXPECT_TRUE(std::isinf(*output.d_pos_inf()));
  EXPECT_GT(*output.d_pos_inf(), 0);
  EXPECT_TRUE(std::isinf(*output.d_neg_inf()));
  EXPECT_LT(*output.d_neg_inf(), 0);
}

fixture::NegativeFieldIdShapes
Generate<fixture::NegativeFieldIdShapes>::operator()() const {
  fixture::NegativeFieldIdShapes value;
  value.negative_i32() = -700;
  value.negative_string() = "negative field";
  value.positive_i64() = 123456789;
  return value;
}

fixture::PresenceShapes Generate<fixture::PresenceShapes>::operator()(
    bool withOptional) const {
  fixture::PresenceShapes value;
  value.unqualified_i32() = 13;
  value.always_i32() = -21;
  if (withOptional) {
    value.maybe_i32() = 34;
    value.maybe_text() = "present";
    value.maybe_inner() = inner(55, "optional-inner");
  }
  return value;
}

fixture::SequenceShapes Generate<fixture::SequenceShapes>::operator()() const {
  fixture::SequenceShapes value;
  value.empty_list() = {};
  value.ints() = {std::numeric_limits<int32_t>::min(), -1, 0, 1, 42};
  value.structs() = {inner(1, "one"), inner(2, "two")};
  value.empty_strings() = {};
  value.strings() = {"", "alpha", "line\nbreak"};
  value.int_set() = {-7, 0, 9};
  return value;
}

fixture::MapShapes Generate<fixture::MapShapes>::operator()() const {
  fixture::MapShapes value;
  value.empty_string_map() = {};
  value.string_map() = {{"", 0}, {"minus", -1}, {"one", 1}};
  value.int_map() = {{-1, "minus"}, {0, "zero"}, {42, "answer"}};
  value.enum_map() = {
      {fixture::GoldenEnum::Unknown, "unknown"},
      {fixture::GoldenEnum::Active, "active"},
  };
  return value;
}

fixture::NestedShapes Generate<fixture::NestedShapes>::operator()() const {
  fixture::NestedShapes value;
  value.inner() = inner(99, "root");
  value.matrix() = {{}, {1, 2, 3}, {-3, -2, -1}};
  value.inner_groups() = {
      {inner(1, "a"), inner(2, "b")},
      {},
      {inner(3, "c")},
  };
  return value;
}

fixture::UnionShapes Generate<fixture::UnionShapes>::operator()() const {
  fixture::UnionShapes value;
  value.id_choice() = idChoice(123);
  value.name_choice() = nameChoice("chosen");
  return value;
}

fixture::ExceptionShapes Generate<fixture::ExceptionShapes>::operator()()
    const {
  fixture::GoldenException ex;
  ex.message() = "failed";
  ex.code() = 500;

  fixture::ExceptionShapes value;
  value.ex() = ex;
  return value;
}

fixture::GoldenStruct Generate<fixture::GoldenStruct>::operator()(
    bool withOptional) const {
  fixture::GoldenStruct value;
  value.flag() = true;
  value.b() = static_cast<int8_t>(-7);
  value.s() = -1234;
  value.i() = 1234567;
  value.l() = -9876543210LL;
  value.f() = 1.25f;
  value.d() = -3.5;
  value.text() = "hello\njson";
  value.data() = std::string("\x00hi\xff", 4);
  value.ints() = {1, -2, 3};
  value.tags() = {"blue", "green"};
  value.counts() = {{"one", 1}, {"minus", -1}};
  value.inner() = inner(42, "nested");
  if (withOptional) {
    value.maybe_text() = "present";
  }
  value.status() = fixture::GoldenEnum::Active;
  value.choice() = nameChoice("chosen");
  return value;
}

} // namespace apache::thrift::transcode::golden_test
