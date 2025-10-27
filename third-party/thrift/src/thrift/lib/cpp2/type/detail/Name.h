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

#include <fmt/core.h>
#include <fatal/type/array.h>
#include <folly/lang/Pretty.h>
#include <thrift/lib/cpp2/reflection/reflection.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::type::detail {

// Helper to get human readable name for the type tag.
template <typename Tag>
struct GetName {
  // Return the name of the base type by default.
  FOLLY_EXPORT const std::string& operator()() const {
    static const auto* kName =
        new std::string(getBaseTypeName(base_type_v<Tag>));
    return *kName;
  }
};

template <typename T>
struct GetNameStructured {
  FOLLY_EXPORT const std::string& operator()() const {
    static const auto* kName = new std::string([]() {
      // TODO(afuller): Return thrift.uri if available.
      using pa = ::apache::thrift::detail::st::private_access;
      return fmt::format(
          "{}.{}",
          pa::__fbthrift_get_module_name<T>(),
          pa::__fbthrift_get_class_name<T>());
    }());
    return *kName;
  }
};

template <typename T>
struct GetName<enum_t<T>> {
  FOLLY_EXPORT const std::string& operator()() const {
    static const auto* kName = new std::string([]() {
      // TODO(afuller): Return thrift.uri if available.
      using traits = TEnumTraits<T>;
      return fmt::format("{}.{}", traits::moduleName(), traits::typeName());
    }());
    return *kName;
  }
};

template <typename T>
struct GetName<union_t<T>> : GetNameStructured<T> {};

template <typename T>
struct GetName<struct_t<T>> : GetNameStructured<T> {};

template <typename T>
struct GetName<exception_t<T>> : GetNameStructured<T> {};

template <typename CTag, typename... Tags>
struct GetNameParamed;

template <typename CTag, typename T1>
struct GetNameParamed<CTag, T1> {
  FOLLY_EXPORT const std::string& operator()() const {
    static const auto* kName = new std::string(
        fmt::format("{}<{}>", GetName<CTag>()(), GetName<T1>()()));
    return *kName;
  }
};

template <typename CTag, typename T1, typename T2>
struct GetNameParamed<CTag, T1, T2> {
  FOLLY_EXPORT const std::string& operator()() const {
    static const auto* kName = new std::string(
        fmt::format(
            "{}<{}, {}>", GetName<CTag>()(), GetName<T1>()(), GetName<T2>()()));
    return *kName;
  }
};

template <typename VTag>
struct GetName<list<VTag>> : GetNameParamed<list_c, VTag> {};

template <typename KTag>
struct GetName<set<KTag>> : GetNameParamed<set_c, KTag> {};

template <typename KTag, typename VTag>
struct GetName<map<KTag, VTag>> : GetNameParamed<map_c, KTag, VTag> {};

template <typename T>
struct PrettyName {
  FOLLY_EXPORT const std::string& operator()() const {
    static const auto* kName = new std::string(folly::pretty_name<T>());
    return *kName;
  }
};

template <typename Adapter, typename Tag>
struct GetName<adapted<Adapter, Tag>>
    : PrettyName<native_type<adapted<Adapter, Tag>>> {};

template <typename T, typename Tag>
struct GetName<cpp_type<T, Tag>> : PrettyName<T> {};

template <typename T>
struct GetName<service_t<T>> {
  FOLLY_EXPORT const std::string& operator()() const {
    return ::apache::thrift::uri<T>();
  }
};

} // namespace apache::thrift::type::detail
