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
#include <fatal/type/apply.h>
#include <fatal/type/get_type.h>
#include <fatal/type/transform.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>
#include <thrift/lib/cpp2/reflection/pretty_print.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace apache::thrift;
using namespace static_reflection::demo;

struct creator {
  template <typename StructInfo, typename Out>
  void operator()(fatal::tag<StructInfo>, Out& out) {
    out.template emplace<typename StructInfo::type>();
  }
};

template <typename Module>
struct activator {
  using module = reflect_module<Module>;

  using structs = fatal::transform<typename module::structs, fatal::get_first>;
  // fatal::list<struct1, struct2, ..., structN>

  using reflected_structs =
      fatal::transform<structs, fatal::applier<reflect_struct>>;
  // fatal::list<
  //   reflect_struct<struct1>,
  //   reflect_struct<struct2>,
  //   ...,
  //   reflect_struct<structN>
  // >

  using variant = fatal::apply_to<structs, fatal::auto_variant>;
  // fatal::auto_variant<struct1, struct2, ..., structN>

  static variant create(folly::StringPiece name) {
    variant out;

    bool found = fatal::trie_find<reflected_structs, fatal::get_type::name>(
        name.begin(), name.end(), creator(), out);

    if (!found) {
      std::cerr << "no type named " << name << '\n';
    }

    return out;
  }
};

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  using factory = activator<data_tags::module>;

  auto prompt = [](std::string& out) {
    std::cout << "type to instantiate: ";
    std::cin >> out;
    return static_cast<bool>(std::cin);
  };

  for (std::string name; prompt(name);) {
    auto instance = factory::create(name);

    instance.visit([](const auto& what) {
      detail::pretty_print(std::cout, what);
      std::cout << std::endl;
    });
  }

  return 0;
}
