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

#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/annotated_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace apache::thrift;
using namespace static_reflection::demo;

struct print_annotations {
  template <typename Annotation, std::size_t Index>
  void operator()(fatal::indexed<Annotation, Index>, const char* prefix) const {
    std::cout << prefix << fatal::z_data<typename Annotation::key>() << ": '"
              << fatal::z_data<typename Annotation::value>() << '\n';
  }
};

struct print_struct_member_annotations {
  template <typename Member, std::size_t Index>
  void operator()(fatal::indexed<Member, Index>) const {
    std::cout << "  * " << fatal::z_data<typename Member::name>() << '\n';
    fatal::foreach<typename Member::annotations::map>(
        print_annotations(), "    - ");
  }
};

template <typename T>
void print_struct_annotations() {
  using info = reflect_struct<T>;

  std::cout << "struct annotations:\n";
  fatal::foreach<typename info::annotations::map>(print_annotations(), "- ");

  fatal::foreach<typename info::members>(print_struct_member_annotations());
  std::cout << '\n';
}

int main() {
  print_struct_annotations<annotated_struct>();

  return 0;
}
