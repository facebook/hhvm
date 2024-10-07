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

#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache::thrift::merge_into_detail {

template <typename Tag>
struct merge_impl;

template <typename Tag>
struct merge_field_refs {
  using T = type::native_type<Tag>;

  template <template <typename> class Ref>
  static void go(Ref<const T&> src, Ref<T&> dst) {
    merge_impl<Tag>::go(*src, dst.ensure());
  }

  template <template <typename> class Ref>
  static void go(Ref<T&&> src, Ref<T&> dst) {
    merge_impl<Tag>::go(std::move(*src), dst.ensure());
  }

  static void go(
      optional_boxed_field_ref<const detail::boxed_value_ptr<T>&> src,
      optional_boxed_field_ref<detail::boxed_value_ptr<T>&> dst) {
    if (!dst) {
      dst.copy_from(src);
    } else if (src) {
      merge_impl<Tag>::go(*src, *dst);
    }
  }

  static void go(
      optional_boxed_field_ref<detail::boxed_value_ptr<T>&&> src,
      optional_boxed_field_ref<detail::boxed_value_ptr<T>&> dst) {
    if (!dst) {
      dst.move_from(src);
    } else if (src) {
      merge_impl<Tag>::go(std::move(*src), *dst);
      src.reset();
    }
  }

  // The legacy implementation incorrectly failed to merge fields using
  // cpp.ref, so we are keeping the same behavior for now.
  // TODO: check callsites for safety and merge recursively.

  static void go(const std::unique_ptr<T>& src, std::unique_ptr<T>& dst) {
    dst = !src ? nullptr : std::make_unique<T>(*src);
  }
  static void go(std::unique_ptr<T>&& src, std::unique_ptr<T>& dst) {
    dst = std::move(src);
  }

  static void go(const std::shared_ptr<T>& src, std::shared_ptr<T>& dst) {
    dst = src;
  }
  static void go(std::shared_ptr<T>&& src, std::shared_ptr<T>& dst) {
    dst = std::move(src);
  }

  static void go(
      const std::shared_ptr<const T>& src, std::shared_ptr<const T>& dst) {
    dst = src;
  }
  static void go(
      std::shared_ptr<const T>&& src, std::shared_ptr<const T>& dst) {
    dst = std::move(src);
  }
};

template <typename Tag>
struct merge_impl {
  static_assert(
      type::is_a_v<Tag, type::primitive_c> || type::is_a_v<Tag, type::union_c>);
  template <typename T>
  static void go(const T& src, T& dst) {
    dst = src;
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    dst = std::move(src);
  }
};

template <typename T>
struct merge_impl<type::struct_t<T>> {
  template <typename Src>
  struct visitor {
    template <typename Fid>
    void operator()(Fid) {
      if constexpr (detail::is_optional_or_union_field_ref_v<
                        op::get_field_ref<T, Fid>>) {
        if (!op::get<Fid>(src).has_value()) {
          return;
        }
      }

      using merge_field = merge_field_refs<op::get_type_tag<T, Fid>>;
      merge_field::go(op::get<Fid>(std::forward<Src>(src)), op::get<Fid>(dst));
    }

    Src src;
    T& dst;
    visitor(Src src, T& dst) : src(std::forward<Src>(src)), dst(dst) {}
  };
  static void go(const T& src, T& dst) {
    op::for_each_field_id<T>(visitor<const T&>(src, dst));
  }
  static void go(T&& src, T& dst) {
    op::for_each_field_id<T>(visitor<T&&>(std::move(src), dst));
  }
};
template <typename T>
struct merge_impl<type::exception_t<T>> : merge_impl<type::struct_t<T>> {};
// Note: unions are handled alongside primitives!

template <typename ValueTag>
struct merge_impl<type::list<ValueTag>> {
  template <typename T>
  static void go(const T& src, T& dst) {
    dst.reserve(dst.size() + src.size());
    std::copy(src.cbegin(), src.cend(), std::back_inserter(dst));
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    dst.reserve(dst.size() + src.size());
    std::move(src.begin(), src.end(), std::back_inserter(dst));
  }
};

template <typename ValueTag>
struct merge_impl<type::set<ValueTag>> {
  template <typename T>
  static void go(const T& src, T& dst) {
    std::copy(src.cbegin(), src.cend(), std::inserter(dst, dst.end()));
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    std::move(src.begin(), src.end(), std::inserter(dst, dst.end()));
  }
};

template <typename KeyTag, typename MappedTag>
struct merge_impl<type::map<KeyTag, MappedTag>> {
  template <typename T>
  static void go(const T& src, T& dst) {
    for (const auto& kv : src) {
      merge_impl<MappedTag>::go(kv.second, dst[kv.first]);
    }
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    for (auto& kv : src) {
      merge_impl<MappedTag>::go(std::move(kv.second), dst[kv.first]);
    }
  }
};

template <typename T, typename Tag>
struct merge_impl<type::cpp_type<T, Tag>> : merge_impl<Tag> {};

} // namespace apache::thrift::merge_into_detail

namespace apache::thrift {

template <typename T>
void merge_into(T&& src, folly::remove_cvref_t<T>& dst) {
  using D = typename folly::remove_cvref_t<T>;
  static_assert(is_thrift_class_v<D>, "merge_into: not a structured type");
  using Tag = type::infer_tag<D>;
  merge_into_detail::merge_impl<Tag>::go(std::forward<T>(src), dst);
}

} // namespace apache::thrift
