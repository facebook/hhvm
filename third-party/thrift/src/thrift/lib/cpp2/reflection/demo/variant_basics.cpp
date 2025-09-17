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

#include <folly/init/Init.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace static_reflection::demo;

struct print_variant_member {
  template <typename Member, std::size_t Index, typename T>
  void operator()(fatal::indexed<Member, Index>, T const& object) const {
    auto name = fatal::z_data<typename Member::metadata::name>();
    const auto& value = Member::get(object);
    std::cout << "currently set: " << name << " = " << value << '\n';
    std::cout << "- type class: "
              << folly::demangle(typeid(typename Member::metadata::type_class))
              << '\n';
    std::cout << "- concrete type: "
              << folly::demangle(typeid(typename Member::type)) << '\n';
    std::cout << '\n';
  }
};

template <typename T>
void print_variant_info(T const& object) {
  using traits = fatal::variant_traits<T>;
  const bool found =
      fatal::scalar_search<typename traits::descriptors, fatal::get_type::id>(
          object.getType(), print_variant_member(), object);

  if (!found) {
    std::cout << "variant is empty\n";
    std::cout << '\n';
  }
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);

  simple_variant value;
  print_variant_info(value);

  value.i32_data() = 12345678;
  print_variant_info(value);

  value.i16_data() = 4321;
  print_variant_info(value);

  value.double_data() = 3.1415926;
  print_variant_info(value);

  value.string_data() = "hello, world";
  print_variant_info(value);

  return 0;
}
