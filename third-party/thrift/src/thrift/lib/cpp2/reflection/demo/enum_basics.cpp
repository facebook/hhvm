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

#include <iostream>

#include <fatal/type/enum.h>
#include <fatal/type/scalar.h>
#include <folly/Demangle.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace static_reflection::demo;

struct print_enum_field {
  template <typename Field, std::size_t Index>
  void operator()(fatal::indexed<Field, Index>) const {
    std::cout << "  " << fatal::z_data<typename Field::name>() << " = "
              << fatal::to_integral(Field::value::value) << '\n';
  }
};

template <typename Enum>
void print_enum_info(Enum e) {
  using traits = fatal::enum_traits<Enum>;
  std::cout << "enum name: " << fatal::z_data<typename traits::name>() << '\n';

  std::cout << '\n';

  const auto name = fatal::enum_to_string(e, nullptr);
  std::cout << "field name: " << name << '\n';
  std::cout << "represented as a " << folly::demangle(typeid(name)) << '\n';

  std::cout << '\n';

  auto integral = fatal::to_integral(e);
  std::cout << "enum value as integer: " << integral << '\n';

  static_assert(
      std::is_same<
          typename std::underlying_type<Enum>::type,
          decltype(integral)>::value,
      "fatal::to_integral() should return the enum's underlying type");

  std::cout << '\n';

  std::cout << "all fields:\n";
  fatal::foreach<typename traits::fields>(print_enum_field());
}

template <typename Enum>
void try_string_to_enum(folly::StringPiece name) {
  using traits = fatal::enum_traits<Enum>;

  Enum value;
  if (traits::try_parse(value, name)) {
    std::cout << "try_parse: successfully parsed " << name << ", int value"
              << fatal::to_integral(value) << '\n';
  } else {
    std::cout << "try_parse: " << name << " is not a valid enum field\n";
  }
}

template <typename Enum>
void string_to_enum(folly::StringPiece name) {
  using traits = fatal::enum_traits<Enum>;

  try {
    auto value = traits::parse(name);
    std::cout << "parse: successfully parsed " << name << ", int value"
              << fatal::to_integral(value) << '\n';
  } catch (const std::exception& e) {
    std::cout << "parse: " << e.what() << '\n';
  }
}

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  print_enum_info(some_enum::field2);

  std::cout << '\n';

  try_string_to_enum<some_enum>("field2");
  try_string_to_enum<some_enum>("non-existing_field");

  std::cout << '\n';

  string_to_enum<some_enum>("field2");
  string_to_enum<some_enum>("non-existing_field");

  return 0;
}
