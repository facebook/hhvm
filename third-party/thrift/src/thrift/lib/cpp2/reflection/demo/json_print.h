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

#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

#include <fatal/type/search.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

template <typename TypeClass>
struct printer {
  static_assert(
      !std::is_same_v<apache::thrift::type_class::unknown, TypeClass>,
      "no static reflection support for the given type"
      " - did you forget to include the reflection metadata?"
      " see thrift/lib/cpp2/reflection/reflection.h");

  template <typename T>
  static void print(T const& what) {
    std::cout << what;
  }

  static void print(const bool what) { std::cout << (what ? "true" : "false"); }
};

template <>
struct printer<apache::thrift::type_class::string> {
  template <typename T>
  static void print(T const& what) {
    std::cout << '"' << what << '"';
  }
};

template <>
struct printer<apache::thrift::type_class::enumeration> {
  template <typename T>
  static void print(T const& what) {
    std::cout << '"' << fatal::enum_to_string(what, nullptr) << '"';
  }
};

template <typename ValueTypeClass>
struct printer<apache::thrift::type_class::list<ValueTypeClass>> {
  template <typename T>
  static void print(T const& what) {
    std::cout << '[';

    bool first = true;
    for (const auto& i : what) {
      if (first) {
        first = false;
      } else {
        std::cout << ',';
      }

      printer<ValueTypeClass>::print(i);
    }

    std::cout << ']';
  }
};

template <typename ValueTypeClass>
struct printer<apache::thrift::type_class::set<ValueTypeClass>>
    : public printer<apache::thrift::type_class::list<ValueTypeClass>> {};

template <typename KeyTypeClass, typename MappedTypeClass>
struct printer<apache::thrift::type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T>
  static void print(T const& what) {
    std::cout << '{';

    bool first = true;
    for (const auto& i : what) {
      if (first) {
        first = false;
      } else {
        std::cout << ',';
      }

      printer<KeyTypeClass>::print(i.first);
      std::cout << ':';
      printer<MappedTypeClass>::print(i.second);
    }

    std::cout << '}';
  }
};

struct struct_member_printer {
  template <typename Member, std::size_t Index, typename T>
  void operator()(fatal::indexed<Member, Index>, T const& what) const {
    if (Index) {
      std::cout << ',';
    }

    const auto name = fatal::z_data<typename Member::name>();
    std::cout << '"' << name << "\":";

    const auto& value = typename Member::getter{}(what);
    printer<typename Member::type_class>::print(value);
  }
};

template <>
struct printer<apache::thrift::type_class::structure> {
  template <typename T>
  static void print(T const& what) {
    std::cout << '{';
    fatal::foreach<typename apache::thrift::reflect_struct<T>::members>(
        struct_member_printer(), what);
    std::cout << '}';
  }
};

struct variant_member_printer {
  template <typename Member, std::size_t Index, typename T>
  void operator()(fatal::indexed<Member, Index>, T const& what) const {
    const auto name = fatal::enum_to_string(what.getType(), nullptr);
    std::cout << '"' << name << "\":";

    const auto& value = Member::get(what);

    printer<typename Member::metadata::type_class>::print(value);
  }
};

template <>
struct printer<apache::thrift::type_class::variant> {
  template <typename T>
  static void print(T const& what) {
    std::cout << '{';
    fatal::scalar_search<
        typename fatal::variant_traits<T>::descriptors,
        fatal::get_type::id>(what.getType(), variant_member_printer(), what);
    std::cout << '}';
  }
};

template <typename TC, typename T>
void print_as(TC, T const& what) {
  printer<TC>::print(what);
}

template <typename T>
void print(T const& what) {
  print_as(apache::thrift::reflect_type_class_of_thrift_class<T>{}, what);
}
