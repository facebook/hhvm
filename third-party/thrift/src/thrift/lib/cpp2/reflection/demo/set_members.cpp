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

#include <fatal/container/variant.h>
#include <fatal/type/get_type.h>
#include <folly/Conv.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>
#include <thrift/lib/cpp2/reflection/pretty_print.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace apache::thrift;
using namespace static_reflection::demo;

template <typename T>
struct member_setter {
  using info = reflect_struct<T>;

  struct visitor {
    template <typename Member>
    void operator()(fatal::tag<Member>, T& where) {
      std::cout << "value to set: ";
      std::string value;
      std::getline(std::cin, value);
      auto& member = typename Member::getter{}(where);
      try {
        member = folly::to<typename Member::type>(value);
      } catch (const std::exception&) {
        std::cerr << "unable to convert '" << value << "'\n";
      }
    }
  };

  static void set(folly::StringPiece name, T& where) {
    bool found =
        fatal::trie_find<typename info::members, fatal::get_type::name>(
            name.begin(), name.end(), visitor(), where);

    if (!found) {
      std::cerr << "no member named " << name << '\n';
    }
  }
};

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  using type = simple_struct;
  using setter = member_setter<type>;

  auto prompt = [](std::string& out) {
    std::cout << "member to set: ";
    std::getline(std::cin, out);
    return static_cast<bool>(std::cin);
  };

  type instance;

  for (std::string name; prompt(name);) {
    setter::set(name, instance);
    detail::pretty_print(std::cout, instance);
    std::cout << std::endl;
  }

  return 0;
}
