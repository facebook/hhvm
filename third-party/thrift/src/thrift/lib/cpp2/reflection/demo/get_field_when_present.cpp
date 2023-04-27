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

#include <fatal/type/data_member_getter.h>
#include <fatal/type/get_type.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/operations_constants.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/operations_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace static_reflection::demo;

void print() {
  std::cout << '\n';
}

template <typename T, typename... Args>
void print(T const& value, const Args&... args) {
  std::cout << value;
  print(args...);
}

FATAL_DATA_MEMBER_GETTER(audit_id_getter, audit_id);

struct print_audit_id_visitor {
  template <typename T>
  void impl(std::true_type, T const& member, const char* name) const {
    print("audit id for ", name, ": ", *member.audit_id_ref());
  }

  template <typename T>
  void impl(std::false_type, T const& member, const char* name) const {
    (void)member;
    print("no audit id available for ", name);
  }

  template <typename Member, std::size_t Index, typename T>
  void operator()(fatal::indexed<Member, Index>, T const& variant) const {
    using has_audit_id = audit_id_getter::has<typename Member::type>;
    const auto& member = Member::get(variant);
    const auto member_name = fatal::z_data<typename Member::metadata::name>();
    impl(has_audit_id(), member, member_name);
  }
};

template <typename T>
void print_audit_id(T const& variant) {
  using info = fatal::variant_traits<T>;

  fatal::scalar_search<typename info::descriptors, fatal::get_type::id>(
      variant.getType(), print_audit_id_visitor(), variant);
}

int main(int argc, char** argv) {
  (void)argc, (void)argv;
  print_audit_id(operations_constants::create_entity());
  print_audit_id(operations_constants::query_entity());
  print_audit_id(operations_constants::delete_entity());
  print_audit_id(operations_constants::add_field());
  print_audit_id(operations_constants::query_field());
  print_audit_id(operations_constants::delete_field());

  return 0;
}
