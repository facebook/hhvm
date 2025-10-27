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

#ifndef THRIFT_FATAL_REFLECTION_INL_POST_H_
#define THRIFT_FATAL_REFLECTION_INL_POST_H_ 1

#if !defined THRIFT_FATAL_REFLECTION_H_
#error "This file must be included from reflection.h"
#endif

namespace folly {
template <class Value>
class Optional;
}

namespace apache {
namespace thrift {
namespace detail {
namespace reflection_impl {

struct no_annotations {
  using keys = void;
  using values = void;
  using map = ::fatal::list<>;
};

using reflected_no_annotations = reflected_annotations<no_annotations>;

} // namespace reflection_impl

template <typename Annotations, legacy_type_id_t LegacyTypeId>
struct type_common_metadata_impl {
  using annotations = Annotations;
  using legacy_id = std::integral_constant<legacy_type_id_t, LegacyTypeId>;
};

template <typename T>
struct [[deprecated(
    "Deprecated in favor of "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
reflect_type_class_of_thrift_class_impl {
  using type = fatal::conditional<
      is_reflectable_struct<T>::value,
      type_class::structure,
      fatal::conditional<
          is_reflectable_union<T>::value,
          type_class::variant,
          type_class::unknown>>;
};

template <typename T, bool IsTry>
struct reflect_module_tag_selector<type_class::enumeration, T, IsTry> {
  static_assert(folly::always_false<T>);
};

template <typename T, bool IsTry>
struct reflect_module_tag_selector<type_class::variant, T, IsTry> {
  using type = typename fatal::variant_traits<T>::metadata::module;
};

template <typename T, bool IsTry>
struct [[deprecated(
    "Deprecated in favor of "
    "https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/languages/cpp/reflection and " // @oss-disable
    "https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Get.h")]]
reflect_module_tag_selector<type_class::structure, T, IsTry> {
  using type = typename reflect_struct<T>::module;
};

} // namespace detail

template <>
struct reflected_annotations<void> {
  struct keys {};
  struct values {};
  using map = fatal::list<>;
};

} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_REFLECTION_INL_POST_H_
