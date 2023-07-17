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

#include <thrift/lib/cpp2/gen/module_metadata_h.h>

namespace apache {
namespace thrift {
namespace detail {

inline const auto& empty_thrift_field() noexcept {
  static const folly::Indestructible<metadata::ThriftField> t;
  return *t;
}

template <class T, class F>
struct MetadataForwarder {
  F f;

  const metadata::ThriftStruct& meta =
      get_struct_metadata<folly::remove_cvref_t<T>>();

  const bool kHasMetadata = !meta.fields()->empty();

  const metadata::ThriftField& kEmpty = empty_thrift_field();

  template <class... Args>
  FOLLY_ERASE decltype(auto) operator()(size_t idx, Args&&... args) {
    return f(
        kHasMetadata ? meta.fields()[idx] : kEmpty,
        std::forward<Args>(args)...);
  }
};

} // namespace detail
} // namespace thrift
} // namespace apache
