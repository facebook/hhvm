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
struct ForEachField {
  static_assert(is_thrift_class_v<T>, "Type must be thrift class");
  static_assert(!is_thrift_class_v<T>, "Must include visitation header");
};
} // namespace detail
/**
 * for_each_field iterates over fields in thrift struct. Example:
 *
 *   for_each_field(thriftObject, [](const ThriftField& meta, auto field_ref) {
 *     LOG(INFO) << *meta.name_ref() << " --> " << *field_ref;
 *   });
 *
 * ThriftField schema is defined here: https://git.io/JJQpY
 * If there are mixin fields, inner fields won't be iterated.
 * If `no_metadata` thrift option is enabled, ThriftField will be empty.
 *
 * @param t thrift object
 * @param f a callable that will be called for each thrift field
 */
template <typename T, typename F>
[[deprecated(
    "Deprecated in favor of apache::thrift::op::for_each_field_id. "
    "See "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
void for_each_field(T&& t, F f) {
  apache::thrift::detail::ForEachField<folly::remove_cvref_t<T>>()(
      detail::MetadataForwarder<T, F>{std::move(f)}, static_cast<T&&>(t));
}

/**
 * Similar to for_each_field(t, f), but works with 2 structures. Example:
 *
 *   for_each_field(thrift1, thrift2, [](const ThriftField& meta,
 *                                       auto field_ref1,
 *                                       auto field_ref2) {
 *     EXPECT_EQ(field_ref1, field_ref2) << *meta.name_ref() << " mismatch";
 *   });
 */
template <typename T1, typename T2, typename F>
[[deprecated(
    "Deprecated in favor of apache::thrift::op::for_each_field_id. "
    "See "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
void for_each_field(T1&& t1, T2&& t2, F f) {
  static_assert(
      std::is_same<folly::remove_cvref_t<T1>, folly::remove_cvref_t<T2>>::value,
      "type mismatch");
  apache::thrift::detail::ForEachField<folly::remove_cvref_t<T1>>()(
      detail::MetadataForwarder<T1, F>{std::move(f)},
      static_cast<T1&&>(t1),
      static_cast<T2&&>(t2));
}
} // namespace apache::thrift
