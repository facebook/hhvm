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

#include <thrift/conformance/data/ValueGenerator.h>

#include <cmath>
#include <limits>

namespace apache::thrift::conformance::data {

namespace {

template <typename Tag>
void addStringValues(NamedValues<Tag>& values) {
  values.emplace_back("", "empty");
  values.emplace_back("a", "lower");
  values.emplace_back("A", "upper");
  values.emplace_back(" a ", "spaces");
  values.emplace_back(" a", "leading_space");
  values.emplace_back("a ", "trailing_space");
  values.emplace_back("Hello", "utf8");
  if constexpr (type::base_type_v<Tag> == type::BaseType::Binary) {
    values.emplace_back("\x72\x01\xff", "bad_utf8");
  }
}

template <typename Tag, bool key>
void addNumberValues(NamedValues<Tag>& values) {
  using T = type::standard_type<Tag>;
  using numeric_limits = std::numeric_limits<T>;

  values.emplace_back(0, "zero");

  // Extrema
  if (numeric_limits::lowest() != numeric_limits::min()) {
    values.emplace_back(numeric_limits::lowest(), "lowest");
  }
  values.emplace_back(numeric_limits::min(), "min");
  values.emplace_back(numeric_limits::max(), "max");

  if constexpr (!numeric_limits::is_integer) {
    values.emplace_back(1UL << numeric_limits::digits, "max_int");
    values.emplace_back(-values.back().value, "min_int");
    values.emplace_back((1UL << numeric_limits::digits) - 1, "max_digits");
    values.emplace_back(-values.back().value, "neg_max_digits");

    values.emplace_back(0.1, "one_tenth");
    values.emplace_back(
        std::nextafter(numeric_limits::min(), T(1)), "min_plus_ulp");
    values.emplace_back(
        std::nextafter(numeric_limits::max(), T(1)), "max_minus_ulp");
  }
  if constexpr (numeric_limits::has_infinity) {
    values.emplace_back(numeric_limits::infinity(), "inf");
    values.emplace_back(-numeric_limits::infinity(), "neg_inf");
  }

  // Minutiae
  values.emplace_back(1, "one");
  if constexpr (numeric_limits::is_signed) {
    values.emplace_back(-1, "neg_one");
  }

  if constexpr (numeric_limits::epsilon() != 0) {
    values.emplace_back(numeric_limits::epsilon(), "epsilon");
    values.emplace_back(-numeric_limits::epsilon(), "neg_epsilon");
  }
  if constexpr (numeric_limits::has_denorm == std::denorm_present) {
    values.emplace_back(numeric_limits::denorm_min(), "denorm_min");
    values.emplace_back(-numeric_limits::denorm_min(), "neg_denorm_min");
  }

  if constexpr (std::is_same_v<T, double>) {
    values.emplace_back(1.9156918820264798e-56, "fmt_case_1");
    values.emplace_back(3788512123356.9854, "fmt_case_2");
  }
}

template <typename Tag, bool key>
NamedValues<Tag> generateValues() {
  static_assert(type::is_a_v<Tag, type::primitive_c>);
  using T = type::standard_type<Tag>;
  NamedValues<Tag> values;

  if constexpr (type::is_a_v<Tag, type::bool_t>) {
    values.emplace_back(true, "true");
    values.emplace_back(false, "false");
  } else if constexpr (type::is_a_v<Tag, type::string_c>) {
    addStringValues<Tag>(values);
  } else if constexpr (type::is_a_v<Tag, type::number_c>) {
    addNumberValues<Tag, key>(values);
  } else {
    values.emplace_back(T(), "default");
  }

  return values;
}

} // namespace

template <typename Tag>
auto ValueGenerator<Tag>::getInterestingValues() -> const Values& {
  static auto kValues = generateValues<Tag, false>();
  return kValues;
}

template <typename Tag>
auto ValueGenerator<Tag>::getKeyValues() -> const Values& {
  static auto kValues = generateValues<Tag, true>();
  return kValues;
}

template struct ValueGenerator<type::bool_t>;
template struct ValueGenerator<type::byte_t>;
template struct ValueGenerator<type::i16_t>;
template struct ValueGenerator<type::i32_t>;
template struct ValueGenerator<type::i64_t>;
template struct ValueGenerator<type::float_t>;
template struct ValueGenerator<type::double_t>;
template struct ValueGenerator<type::string_t>;
template struct ValueGenerator<type::binary_t>;

} // namespace apache::thrift::conformance::data
