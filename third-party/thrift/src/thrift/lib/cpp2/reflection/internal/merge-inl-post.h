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

namespace apache {
namespace thrift {
namespace merge_into_detail {

template <typename T, typename TypeClass>
struct merge {
  using impl = merge_impl<TypeClass>;

  //  regular
  static void go(const T& src, T& dst) { impl::template go<T>(src, dst); }
  static void go(T&& src, T& dst) { impl::template go<T>(std::move(src), dst); }

  static void go(
      optional_boxed_field_ref<const detail::boxed_value_ptr<T>&> src,
      optional_boxed_field_ref<detail::boxed_value_ptr<T>&> dst) {
    if (!dst) {
      dst.copy_from(src);
    } else if (src) {
      go(*src, *dst);
    }
  }

  static void go(
      optional_boxed_field_ref<detail::boxed_value_ptr<T>&&> src,
      optional_boxed_field_ref<detail::boxed_value_ptr<T>&> dst) {
    if (!dst) {
      dst.move_from(src);
    } else if (src) {
      go(std::move(*src), *dst);
      src.reset();
    }
  }

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

template <typename TypeClass>
struct merge_impl {
  template <typename T>
  static void go(const T& src, T& dst) {
    dst = src;
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    dst = std::move(src);
  }
};

template <>
struct merge_impl<type_class::structure> {
  template <typename T>
  struct deref {
    using type = T;
  };
  template <typename T>
  struct deref<optional_boxed_field_ref<T>>
      : deref<typename folly::remove_cvref_t<T>::element_type> {};
  template <typename T, typename D>
  struct deref<std::unique_ptr<T, D>> : deref<T> {};
  template <typename T>
  struct deref<std::shared_ptr<T>> : deref<T> {};
  template <typename T>
  struct deref<std::shared_ptr<const T>> : deref<T> {};

  template <bool Move>
  struct visitor {
    template <typename T>
    using Src = fatal::conditional<Move, T, const T>;
    template <typename MemberInfo, std::size_t Index, typename T>
    void operator()(
        fatal::indexed<MemberInfo, Index>, Src<T>& src, T& dst) const {
      using mgetter = typename MemberInfo::getter;
      using mclass = typename MemberInfo::type_class;
      using msrc = fatal::conditional<Move, Src<T>&&, const Src<T>&>;
      using mtype =
          folly::remove_cvref_t<decltype(mgetter{}(static_cast<msrc>(src)))>;
      using merge_field = merge<typename deref<mtype>::type, mclass>;
      using mref = fatal::conditional<Move, mtype&&, const mtype&>;
      if (MemberInfo::optional::value == optionality::optional &&
          !MemberInfo::is_set(src)) {
        return;
      }
      MemberInfo::mark_set(dst, true);
      merge_field::go(
          static_cast<mref>(mgetter{}(static_cast<msrc>(src))), mgetter{}(dst));
    }
  };
  template <typename T>
  static void go(const T& src, T& dst) {
    using members = typename reflect_struct<T>::members;
    fatal::foreach<members>(visitor<false>(), src, dst);
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    using members = typename reflect_struct<T>::members;
    fatal::foreach<members>(visitor<true>(), src, dst);
  }
};

template <typename ValueTypeClass>
struct merge_impl<type_class::list<ValueTypeClass>> {
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

template <typename ValueTypeClass>
struct merge_impl<type_class::set<ValueTypeClass>> {
  template <typename T>
  static void go(const T& src, T& dst) {
    std::copy(src.cbegin(), src.cend(), std::inserter(dst, dst.end()));
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    std::move(src.begin(), src.end(), std::inserter(dst, dst.end()));
  }
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct merge_impl<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T>
  static void go(const T& src, T& dst) {
    using M = typename T::mapped_type;
    for (const auto& kv : src) {
      merge<M, MappedTypeClass>::go(kv.second, dst[kv.first]);
    }
  }
  template <typename T>
  static void go(T&& src, T& dst) {
    using M = typename T::mapped_type;
    for (auto& kv : src) {
      merge<M, MappedTypeClass>::go(std::move(kv.second), dst[kv.first]);
    }
  }
};

} // namespace merge_into_detail
} // namespace thrift
} // namespace apache

namespace apache {
namespace thrift {

template <typename T>
void merge_into(T&& src, merge_into_detail::remove_const_reference<T>& dst) {
  constexpr auto c = std::is_const<T>::value;
  constexpr auto r = std::is_rvalue_reference<T&&>::value;
  using D = typename merge_into_detail::remove_const_reference<T>;
  using W = fatal::conditional<!c && r, T&&, const D&>;
  using TC = type_class_of_thrift_class_t<folly::remove_cvref_t<T>>;
  static_assert(!std::is_void_v<TC>, "merge_into: not a structure type");
  merge_into_detail::merge<D, TC>::go(static_cast<W>(src), dst);
}

} // namespace thrift
} // namespace apache
