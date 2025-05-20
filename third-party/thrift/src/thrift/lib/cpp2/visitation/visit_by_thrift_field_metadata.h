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

#include <exception>
#include <string_view>

#include <folly/CppAttributes.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/visitation/metadata.h>

namespace apache::thrift {
namespace detail {
template <class T>
struct VisitByFieldId {
  static_assert(is_thrift_class_v<T>, "Type must be thrift class");
  static_assert(!is_thrift_class_v<T>, "Must include visitation header");
};
[[noreturn]] void throwInvalidThriftId(size_t id, std::string_view type);
} // namespace detail

struct InvalidThriftId : std::out_of_range {
  using std::out_of_range::out_of_range;
};

/**
 * Applies the callable to thrift member of given thrift field metadata.
 *
 *   visit_by_thrift_field_metadata(thrift, meta, [&](auto&& ref) {
 *     LOG(INFO) << *meta.name_ref() << " --> " << *ref;
 *   })
 *
 * ThriftField schema is defined in thrift/lib/thrift/metadata.thrift
 *
 * @param t thrift object
 * @param meta thrift field metadata
 * @param f a callable that accepts all member types of thrift struct
 */
template <typename T, typename F>
[[deprecated(
    "Deprecated in favor of apache::thrift::op::invoke_by_field_id. "
    "See "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
void visit_by_thrift_field_metadata(
    T&& t, const apache::thrift::metadata::ThriftField& meta, F f) {
  auto adapter = [&f](auto&&, auto&& ref) {
    f(std::forward<decltype(ref)>(ref));
  };
  return apache::thrift::detail::VisitByFieldId<folly::remove_cvref_t<T>>()(
      detail::MetadataForwarder<T, decltype(adapter)>{adapter},
      *meta.id(),
      static_cast<T&&>(t));
}
} // namespace apache::thrift
