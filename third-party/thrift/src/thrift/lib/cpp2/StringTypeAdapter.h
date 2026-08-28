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

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift {

namespace adapt_detail {

// A native string or binary type must expose contiguous character storage and
// support the operations used by Thrift protocol readers. Construction must
// copy, move, or otherwise retain the characters beyond the lifetime of the
// input view.
template <typename T>
concept StringTypeAdapterCompatible =
    requires(
        T& value,
        const T& constValue,
        const char* data,
        std::size_t size,
        std::string string,
        std::string_view view) {
      { constValue.data() } -> std::same_as<const char*>;
      { constValue.size() } -> std::convertible_to<std::size_t>;
      value.clear();
      value.reserve(size);
      value.append(data, size);
      value += view;
      value = std::move(string);
    } &&
    (std::is_constructible_v<T, std::string&&> ||
     std::is_constructible_v<T, std::string_view> ||
     std::is_constructible_v<T, const char*, std::size_t>);

} // namespace adapt_detail

template <typename AdaptedT>
  requires adapt_detail::StringTypeAdapterCompatible<AdaptedT>
struct StringTypeAdapter {
 private:
  static std::string_view asView(const AdaptedT& value) {
    return {value.data(), value.size()};
  }

 public:
  static AdaptedT fromThrift(std::string value) {
    if constexpr (std::is_constructible_v<AdaptedT, std::string&&>) {
      return AdaptedT(std::move(value));
    } else if constexpr (std::is_constructible_v<AdaptedT, std::string_view>) {
      return AdaptedT(std::string_view(value));
    } else {
      return AdaptedT(value.data(), value.size());
    }
  }

  static std::string toThrift(const AdaptedT& value) {
    const auto bytes = asView(value);
    if (bytes.empty()) {
      return {};
    }
    return std::string(bytes.data(), bytes.size());
  }

  static bool isEmpty(const AdaptedT& value) { return value.size() == 0; }

  static void clear(AdaptedT& value) { value.clear(); }

  template <typename Tag, typename Protocol>
  static auto encode(Protocol& protocol, const AdaptedT& value) {
    const auto bytes = asView(value);
    if constexpr (type::is_a_v<Tag, type::binary_t>) {
      return protocol.writeBinary(bytes);
    } else {
      static_assert(type::is_a_v<Tag, type::string_t>);
      return protocol.writeString(bytes);
    }
  }

  template <typename Tag, typename Protocol>
  static void decode(Protocol& protocol, AdaptedT& value) {
    if constexpr (type::is_a_v<Tag, type::binary_t>) {
      protocol.readBinary(value);
    } else {
      static_assert(type::is_a_v<Tag, type::string_t>);
      protocol.readString(value);
    }
  }

  template <bool ZeroCopy, typename Tag, typename Protocol>
  static auto serializedSize(Protocol& protocol, const AdaptedT& value) {
    const auto bytes = asView(value);
    if constexpr (type::is_a_v<Tag, type::binary_t>) {
      if constexpr (ZeroCopy) {
        return protocol.serializedSizeZCBinary(bytes);
      } else {
        return protocol.serializedSizeBinary(bytes);
      }
    } else {
      static_assert(type::is_a_v<Tag, type::string_t>);
      return protocol.serializedSizeString(bytes);
    }
  }
};

} // namespace apache::thrift
