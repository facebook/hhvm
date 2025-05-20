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

#include <folly/CppAttributes.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/visitation/metadata.h>

namespace apache::thrift {
namespace detail {
template <class T>
struct VisitUnion {
  static_assert(is_thrift_class_v<T>, "Type must be thrift class");
  static_assert(!is_thrift_class_v<T>, "Must include visitation header");
};
} // namespace detail

/**
 * Applies the callable to active member of thrift union. Example:
 *
 *   visit_union_with_metadata(thriftUnion,
 *                             [](const ThriftField& meta, auto&& value) {
 *     LOG(INFO) << *meta.name_ref() << " --> " << value;
 *   })
 *
 * ThriftField schema is defined here: https://git.io/JJQpY
 * If `no_metadata` thrift option is enabled, ThriftField will be empty.
 * If union is empty, callable won't be called.
 *
 * A lighter-weight version not using metadata is available as
 * `op::visit_union_with_tag`, defined in thrift/lib/cpp2/op/Get.h
 *
 * @param t thrift union
 * @param f a callable that accepts all member types from union
 */
template <typename T, typename F>
[[deprecated(
    "Deprecated in favor of apache::thrift::op::visit_union_with_tag. "
    "See "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
decltype(auto) visit_union_with_metadata(T&& t, F&& f) {
  return apache::thrift::detail::VisitUnion<folly::remove_cvref_t<T>>()(
      detail::MetadataForwarder<T, F>{std::forward<F>(f)}, static_cast<T&&>(t));
}

template <typename T, typename F>
[[deprecated(
    "Deprecated in favor of apache::thrift::op::visit_union_with_tag. "
    "If metadata is needed then visit_union_with_metadata is a drop-in replacement. "
    "See "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
decltype(auto) visit_union(T&& t, F&& f) {
  return visit_union_with_metadata(std::forward<T>(t), std::forward<F>(f));
}

} // namespace apache::thrift
