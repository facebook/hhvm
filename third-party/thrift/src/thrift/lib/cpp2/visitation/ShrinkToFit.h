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

#include <folly/Traits.h>
#include <folly/portability/Constexpr.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/visitation/for_each.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

#include <type_traits>

namespace apache {
namespace thrift {
namespace detail {

class shrink_to_fit_fn {
  template <typename T>
  using detect_shrink_to_fit = decltype(std::declval<T>().shrink_to_fit());
  template <typename T>
  constexpr static bool has_shrink_to_fit_v =
      folly::is_detected_v<detect_shrink_to_fit, T>;

  template <typename T>
  using detect_key_type = typename T::key_type;
  template <typename T>
  constexpr static bool has_key_type_v =
      folly::is_detected_v<detect_key_type, T>;

  template <typename T>
  using detect_value_type = typename T::value_type;
  template <typename T>
  constexpr static bool has_value_type_v =
      folly::is_detected_v<detect_value_type, T>;

  template <typename T>
  using detect_mapped_type = typename T::mapped_type;
  template <typename T>
  constexpr static bool has_mapped_type_v =
      folly::is_detected_v<detect_mapped_type, T>;

  template <typename T>
  constexpr static bool is_list_v = has_value_type_v<T>;
  template <typename T>
  constexpr static bool is_set_v =
      (has_value_type_v<T> && has_key_type_v<T> && !has_mapped_type_v<T>);
  template <typename T>
  constexpr static bool is_map_v =
      (has_value_type_v<T> && has_key_type_v<T> && has_mapped_type_v<T>);

  using ThriftTypeEnum = metadata::ThriftType::Type;

  template <typename ThriftStruct>
  static void shrink_to_fit_struct(ThriftStruct& s) {
    for_each_field(s, [](const metadata::ThriftField& meta, auto&& field_ref) {
      using field_ref_type = folly::remove_cvref_t<decltype(field_ref)>;

      // If a field is empty, do nothing.
      if constexpr (detail::is_shared_or_unique_ptr_v<field_ref_type>) {
        if (field_ref == nullptr) {
          return;
        }
      } else if (!field_ref.has_value()) {
        return;
      }

      auto& field = *field_ref;
      using field_type = folly::remove_cvref_t<decltype(field)>;

      if constexpr (is_thrift_class_v<field_type>) {
        shrink_to_fit_struct(field);
      } else {
        shrink_to_fit(field, *meta.type());
      }
    });
  }

  template <typename ThriftList>
  static void shrink_to_fit_list(
      ThriftList& l, const metadata::ThriftType& thriftType) {
    for (auto& e : l) {
      shrink_to_fit(e, *thriftType.get_t_list().valueType_ref());
    }
  }

  template <typename ThriftSet>
  static void shrink_to_fit_set(
      ThriftSet& s, const metadata::ThriftType& thriftType) {
    folly::remove_cvref_t<ThriftSet> new_set;
    // TODO(dokwon): Use 'extract' or 'eraseInto' for optimization.
    for (auto& e : s) {
      typename ThriftSet::key_type new_e = std::move(e);
      shrink_to_fit(new_e, *thriftType.get_t_set().valueType_ref());
      new_set.insert(std::move(new_e));
    }
    s = std::move(new_set);
  }

  template <typename ThriftMap>
  static void shrink_to_fit_map(
      ThriftMap& m, const metadata::ThriftType& thriftType) {
    folly::remove_cvref_t<ThriftMap> new_map;
    // TODO(dokwon): Use 'extract' or 'eraseInto' for optimization.
    for (auto& [k, e] : m) {
      typename ThriftMap::key_type new_k = std::move(k);
      shrink_to_fit(new_k, *thriftType.get_t_map().keyType_ref());
      shrink_to_fit(e, *thriftType.get_t_map().valueType_ref());
      new_map.try_emplace(std::move(new_k), std::move(e));
    }
    m = std::move(new_map);
  }

  template <typename T>
  static void shrink_to_fit(T& t, const metadata::ThriftType& thriftType) {
    if constexpr (has_shrink_to_fit_v<T&>) {
      t.shrink_to_fit();
    }
    const auto thriftTypeEnum = thriftType.getType();
    switch (thriftTypeEnum) {
      case ThriftTypeEnum::t_struct:
        if constexpr (is_thrift_class_v<T>) {
          return shrink_to_fit_struct(t);
        }
        throw std::logic_error("Invalid ThriftType metadata.");
      case ThriftTypeEnum::t_list:
        if constexpr (is_list_v<T>) {
          return shrink_to_fit_list(t, thriftType);
        }
        throw std::logic_error("Invalid ThriftType metadata.");
      case ThriftTypeEnum::t_set:
        if constexpr (is_set_v<T>) {
          return shrink_to_fit_set(t, thriftType);
        }
        throw std::logic_error("Invalid ThriftType metadata.");
      case ThriftTypeEnum::t_map:
        if constexpr (is_map_v<T>) {
          return shrink_to_fit_map(t, thriftType);
        }
        throw std::logic_error("Invalid ThriftType metadata.");
      case ThriftTypeEnum::t_typedef:
        return shrink_to_fit(
            t, *thriftType.get_t_typedef().underlyingType_ref());
      case ThriftTypeEnum::__EMPTY__:
      case ThriftTypeEnum::t_primitive:
      case ThriftTypeEnum::t_enum:
      case ThriftTypeEnum::t_union:
      case ThriftTypeEnum::t_stream:
      case ThriftTypeEnum::t_sink:
        return;
    }
  }

 public:
  template <typename T, std::enable_if_t<is_thrift_class_v<T>, int> = 0>
  void operator()(T& value) const {
    shrink_to_fit_struct(value);
  }
};

} // namespace detail

// When deserializing with JSON protocol, the deserializer is not aware of the
// size of the list, allocating unnecssary capacity to a list. This method
// recursively traverse Thrift structs and containers, attempting to call
// 'shrink_to_fit' method and squeezes all the unnecessary memory usage.
// Also, note that this is an expensive operation, and if reallocation occurs,
// it will invalidate all iterators and references to the elements.
inline constexpr detail::shrink_to_fit_fn shrink_to_fit{};

} // namespace thrift
} // namespace apache
