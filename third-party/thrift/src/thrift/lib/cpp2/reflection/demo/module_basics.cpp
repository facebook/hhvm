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

#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace apache::thrift;
using namespace static_reflection::demo;

struct print_namespace {
  template <typename Language, typename Namespace, std::size_t Index>
  void operator()(fatal::indexed_pair<Language, Namespace, Index>) const {
    std::cout << "  namespace for '" << fatal::z_data<Language>() << "'\n";
  }
};

struct print_variant_member {
  template <typename Member, std::size_t Index>
  void operator()(fatal::indexed<Member, Index>) const {
    std::cout << "    " << fatal::z_data<typename Member::metadata::name>()
              << '\n';
  }
};

struct print_variant {
  template <typename T, typename Name, std::size_t Index>
  void operator()(fatal::indexed_pair<T, Name, Index>) const {
    using info = fatal::variant_traits<T>;
    std::cout << "  variant " << fatal::z_data<Name>() << " {\n";
    fatal::foreach<typename info::descriptors>(print_variant_member());
    std::cout << "  }\n\n";
  }
};

struct print_struct_member {
  template <typename Member, std::size_t Index>
  void operator()(fatal::indexed<Member, Index>) const {
    std::cout << "    " << fatal::z_data<typename Member::name>() << '\n';
  }
};

struct print_struct {
  template <typename T, typename Name, std::size_t Index>
  void operator()(fatal::indexed_pair<T, Name, Index>) const {
    using info = reflect_struct<T>;
    std::cout << "  struct " << fatal::z_data<Name>() << " {\n";
    fatal::foreach<typename info::members>(print_struct_member());
    std::cout << "  }\n\n";
  }
};

template <typename Module>
void print_module_info() {
  using info = reflect_module<Module>;
  std::cout << "module name: " << fatal::z_data<typename info::name>() << '\n';

  std::cout << '\n';

  std::cout << "namespaces declared in module\n";
  fatal::foreach<typename info::namespaces>(print_namespace());

  std::cout << '\n';

  std::cout << "variants declared in module\n";
  fatal::foreach<typename info::unions>(print_variant());

  std::cout << "structs declared in module\n";
  fatal::foreach<typename info::structs>(print_struct());
}

int main() {
  print_module_info<static_reflection::demo::data_tags::module>();

  return 0;
}
