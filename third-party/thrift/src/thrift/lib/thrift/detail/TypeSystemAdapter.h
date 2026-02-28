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

#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

#include <folly/lang/Exception.h>

#include <fmt/core.h>

#include <string_view>
#include <type_traits>

// This file is intended to be used to adapt types in type_system.thrift.
// Do not include this file directly! Use TypeSystem.h instead.
namespace apache::thrift::type_system {

// From type_system.thrift
class FieldIdentityStruct;

namespace detail {

template <typename>
class FieldIdentityWrapper;
using FieldIdentity = FieldIdentityWrapper<FieldIdentityStruct>;
using FieldIdentityAdapter = InlineAdapter<FieldIdentity>;

template <typename T>
class FieldIdentityWrapper final
    : public type::detail::EqWrap<FieldIdentityWrapper<T>, T> {
 private:
  // FieldIdentityStruct is incomplete at this point. We only use templates to
  // delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, FieldIdentityStruct>);
  using Base = type::detail::EqWrap<FieldIdentityWrapper<T>, T>;

 public:
  using Base::Base;
  FieldIdentityWrapper(FieldId id, std::string fieldName) noexcept {
    this->data_.id() = id;
    this->data_.name() = std::move(fieldName);
  }
  FieldId id() const { return *this->data_.id_ref(); }
  const std::string& name() const& { return *this->data_.name_ref(); }
  std::string&& name() && { return *std::move(this->data_).name_ref(); }
};

} // namespace detail

using FieldIdentity = detail::FieldIdentity;

} // namespace apache::thrift::type_system

template <>
struct std::hash<apache::thrift::type_system::FieldIdentity> {
  std::size_t operator()(
      const apache::thrift::type_system::FieldIdentity&) const noexcept;
};

template <>
struct fmt::formatter<apache::thrift::type_system::FieldIdentity>
    : formatter<std::string_view> {
 public:
  format_context::iterator format(
      const apache::thrift::type_system::FieldIdentity&, format_context&) const;
};
