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

#include <cassert>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include <folly/Conv.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/op/Get.h>

namespace facebook {
namespace thrift {
namespace detail {

struct debug_thrift_data_difference_missing {
  template <typename Tag, typename Iter, typename Callback>
  void operator()(
      Tag tag,
      Callback&& callback,
      Iter iterLhs,
      std::string_view path,
      std::string_view message) {
    using value_type = std::remove_reference_t<decltype(*iterLhs)>;
    callback(tag, &*iterLhs, static_cast<value_type*>(nullptr), path, message);
  }

  template <typename Tag, typename Callback>
  void operator()(
      Tag tag,
      Callback&& callback,
      std::vector<bool>::const_iterator iterLhs,
      std::string_view path,
      std::string_view message) {
    const bool value = *iterLhs;
    callback(tag, &value, static_cast<bool*>(nullptr), path, message);
  }
};

struct debug_thrift_data_difference_extra {
  template <typename Tag, typename Iter, typename Callback>
  void operator()(
      Tag tag,
      Callback&& callback,
      Iter iterRhs,
      std::string_view path,
      std::string_view message) {
    using value_type = std::remove_reference_t<decltype(*iterRhs)>;
    callback(tag, static_cast<value_type*>(nullptr), &*iterRhs, path, message);
  }

  template <typename Tag, typename Callback>
  void operator()(
      Tag tag,
      Callback&& callback,
      std::vector<bool>::const_iterator iterRhs,
      std::string_view path,
      std::string_view message) {
    const bool value = *iterRhs;
    callback(tag, static_cast<const bool*>(nullptr), &value, path, message);
  }
};

template <typename>
struct debug_thrift_data_difference_impl;

template <typename Tag, typename T, typename Callback>
bool debug_thrift_data_difference(
    std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
  using impl = debug_thrift_data_difference_impl<Tag>;
  return impl::differs(path, lhs, rhs, std::forward<Callback>(callback));
}

struct scoped_path {
 public:
  ~scoped_path() {
    assert(path_.size() >= size_);
    path_.resize(size_);
  }

  static scoped_path member(std::string& path, std::string_view member) {
    return scoped_path(path, '.', member);
  }

  static scoped_path index(std::string& path, int64_t index) {
    return scoped_path(path, '[', index, ']');
  }

  static scoped_path key(std::string& path, const std::string& key) {
    return scoped_path(path, '[', key, ']');
  }

 private:
  template <typename... Args>
  explicit scoped_path(std::string& path, Args&&... args)
      : size_(path.size()), path_(path) {
    folly::toAppend(std::forward<Args>(args)..., &path);
  }

  std::size_t const size_;
  std::string& path_;
};

template <typename T>
struct debug_thrift_data_difference_impl<apache::thrift::type::enum_t<T>> {
  template <typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(
          apache::thrift::type::enum_t<T>{},
          &lhs,
          &rhs,
          path,
          "value mismatch");
      return false;
    }
    return true;
  }
};

template <typename ValueTag>
struct debug_thrift_data_difference_impl<apache::thrift::type::list<ValueTag>> {
  template <typename T, typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    using impl = debug_thrift_data_difference_impl<ValueTag>;
    const auto minSize = std::min(lhs.size(), rhs.size());
    bool result = true;

    for (std::size_t index = 0; index < minSize; ++index) {
      const auto l = lhs.begin() + index;
      const auto r = rhs.begin() + index;
      auto guard = scoped_path::index(path, index);
      result = impl::differs(path, *l, *r, callback) && result;
    }

    for (std::size_t index = minSize; index < lhs.size(); ++index) {
      const auto lIter = lhs.begin() + index;
      auto guard = scoped_path::index(path, index);
      debug_thrift_data_difference_missing()(
          ValueTag{}, callback, lIter, path, "missing list entry");
      result = false;
    }

    for (std::size_t index = minSize; index < rhs.size(); ++index) {
      const auto rIter = rhs.begin() + index;
      auto guard = scoped_path::index(path, index);
      debug_thrift_data_difference_extra()(
          ValueTag{}, callback, rIter, path, "extra list entry");
      result = false;
    }
    return result;
  }
};

template <typename KeyTag, typename MappedTag>
struct debug_thrift_data_difference_impl<
    apache::thrift::type::map<KeyTag, MappedTag>> {
  template <typename T, typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    using impl = debug_thrift_data_difference_impl<MappedTag>;

    bool result = true;

    for (const auto& [key, value] : lhs) {
      auto guard = scoped_path::key(path, pretty_string<KeyTag>(key));
      if (rhs.find(key) == rhs.end()) {
        debug_thrift_data_difference_missing()(
            MappedTag{}, callback, &value, path, "missing map entry");
        result = false;
      }
    }

    for (const auto& [key, value] : rhs) {
      auto guard = scoped_path::key(path, pretty_string<KeyTag>(key));
      if (lhs.find(key) == lhs.end()) {
        debug_thrift_data_difference_extra()(
            MappedTag{}, callback, &value, path, "extra map entry");
        result = false;
      }
    }

    for (const auto& [key, value] : lhs) {
      const auto ri = rhs.find(key);
      if (ri != rhs.end()) {
        auto guard = scoped_path::key(path, pretty_string<KeyTag>(key));
        const auto& rv = ri->second;
        result = impl::differs(path, value, rv, callback) && result;
      }
    }
    return result;
  }
};

template <typename ValueTag>
struct debug_thrift_data_difference_impl<apache::thrift::type::set<ValueTag>> {
  template <typename T, typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    bool result = true;

    for (const auto& l : lhs) {
      auto guard = scoped_path::key(path, pretty_string<ValueTag>(l));
      if (rhs.find(l) == rhs.end()) {
        debug_thrift_data_difference_missing()(
            ValueTag{}, callback, &l, path, "missing set entry");
        result = false;
      }
    }

    for (const auto& r : rhs) {
      auto guard = scoped_path::key(path, pretty_string<ValueTag>(r));
      if (lhs.find(r) == lhs.end()) {
        debug_thrift_data_difference_extra()(
            ValueTag{}, callback, &r, path, "extra set entry");
        result = false;
      }
    }
    return result;
  }
};

struct debug_thrift_data_difference_with_pointers {
 protected:
  template <typename Tag, typename T, typename Callback>
  static bool recurse_into(
      std::string& path,
      T const& lMember,
      T const& rMember,
      Callback&& callback) {
    return debug_thrift_data_difference_impl<Tag>::differs(
        path, lMember, rMember, std::forward<Callback>(callback));
  }

  template <typename Tag, typename T, typename Callback>
  static bool recurse_into_ptr(
      std::string& path,
      T const* lMember,
      T const* rMember,
      Callback&& callback) {
    if (!lMember && !rMember) {
      return true;
    }
    if (!rMember) {
      debug_thrift_data_difference_missing()(
          Tag{}, std::forward<Callback>(callback), lMember, path, "missing");
      return false;
    }
    if (!lMember) {
      debug_thrift_data_difference_extra()(
          Tag{}, std::forward<Callback>(callback), rMember, path, "extra");
      return false;
    }
    return recurse_into<Tag>(
        path, *lMember, *rMember, std::forward<Callback>(callback));
  }
};

template <typename T>
struct debug_thrift_data_difference_impl<apache::thrift::type::union_t<T>>
    : debug_thrift_data_difference_with_pointers {
  template <typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs.getType() != rhs.getType()) {
      visit_changed<debug_thrift_data_difference_missing>(
          path, lhs, std::forward<Callback>(callback));
      visit_changed<debug_thrift_data_difference_extra>(
          path, rhs, std::forward<Callback>(callback));
      return false;
    }

    return apache::thrift::op::invoke_by_field_id<T>(
        static_cast<apache::thrift::FieldId>(lhs.getType()),
        [&](auto id) {
          using Id = decltype(id);
          using Tag = apache::thrift::op::get_type_tag<T, Id>;

          auto guard =
              scoped_path::member(path, apache::thrift::op::get_name_v<T, Id>);

          const auto* lPtr = apache::thrift::op::getValueOrNull(
              apache::thrift::op::get<Id>(lhs));
          const auto* rPtr = apache::thrift::op::getValueOrNull(
              apache::thrift::op::get<Id>(rhs));

          return recurse_into_ptr<Tag>(
              path, lPtr, rPtr, std::forward<Callback>(callback));
        },
        [] {
          return true; // union is __EMPTY__
        });
  }

 private:
  template <typename Change, typename Tag, typename TF, typename Callback>
  static void visit_changed_field(
      std::string& path, TF const* fieldPtr, Callback&& callback) {
    Change()(
        Tag{},
        std::forward<Callback>(callback),
        fieldPtr,
        path,
        "union type changed");
  }

  template <typename Change, typename Callback>
  static void visit_changed(
      std::string& path, T const& variant, Callback&& callback) {
    apache::thrift::op::invoke_by_field_id<T>(
        static_cast<apache::thrift::FieldId>(variant.getType()),
        [&](auto id) {
          using Id = decltype(id);
          using Tag = apache::thrift::op::get_type_tag<T, Id>;

          auto guard =
              scoped_path::member(path, apache::thrift::op::get_name_v<T, Id>);

          const auto* fieldPtr = apache::thrift::op::getValueOrNull(
              apache::thrift::op::get<Id>(variant));
          visit_changed_field<Change, Tag>(
              path, fieldPtr, std::forward<Callback>(callback));
        },
        [] {
          // union is __EMPTY__
        });
  }
};

template <typename T>
struct debug_thrift_data_difference_impl<apache::thrift::type::struct_t<T>>
    : debug_thrift_data_difference_with_pointers {
  template <typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    bool result = true;

    apache::thrift::op::for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      using Tag = apache::thrift::op::get_type_tag<T, Id>;

      const auto* lPtr =
          apache::thrift::op::getValueOrNull(apache::thrift::op::get<Id>(lhs));
      const auto* rPtr =
          apache::thrift::op::getValueOrNull(apache::thrift::op::get<Id>(rhs));

      if (!lPtr && !rPtr) {
        return;
      }

      auto guard =
          scoped_path::member(path, apache::thrift::op::get_name_v<T, Id>);

      if (!rPtr) {
        debug_thrift_data_difference_missing()(
            Tag{}, callback, lPtr, path, "missing");
        result = false;
        return;
      }

      if (!lPtr) {
        debug_thrift_data_difference_extra()(
            Tag{}, callback, rPtr, path, "extra");
        result = false;
        return;
      }

      auto partial = recurse_into_ptr<Tag>(
          path, lPtr, rPtr, std::forward<Callback>(callback));
      result = result && partial; // no short-circuit
    });

    return result;
  }
};

template <typename T>
struct debug_thrift_data_difference_impl<apache::thrift::type::exception_t<T>>
    : debug_thrift_data_difference_impl<apache::thrift::type::struct_t<T>> {};

template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::string_t> {
  template <typename T, typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(
          apache::thrift::type::string_t{},
          &lhs,
          &rhs,
          path,
          "string mismatch");
      return false;
    }
    return true;
  }
};

template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::binary_t> {
  using B = folly::IOBuf;
  using U = std::unique_ptr<B>;
  template <typename T, typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(
          apache::thrift::type::binary_t{},
          &lhs,
          &rhs,
          path,
          "binary mismatch");
      return false;
    }
    return true;
  }
  template <typename Callback>
  static bool differs(
      std::string& path, B const& lhs, B const& rhs, Callback&& callback) {
    if (!folly::IOBufEqualTo{}(lhs, rhs)) {
      callback(
          apache::thrift::type::binary_t{},
          &lhs,
          &rhs,
          path,
          "binary mismatch");
      return false;
    }
    return true;
  }
  template <typename Callback>
  static bool differs(
      std::string& path, U const& lhs, U const& rhs, Callback&& callback) {
    if (!folly::IOBufEqualTo{}(lhs, rhs)) {
      callback(
          apache::thrift::type::binary_t{},
          &lhs,
          &rhs,
          path,
          "binary mismatch");
      return false;
    }
    return true;
  }
};

template <typename Tag>
struct debug_thrift_data_difference_impl_floating_point {
  using T = apache::thrift::type::standard_type<Tag>;

  template <typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(
          apache::thrift::type::float_t{},
          &lhs,
          &rhs,
          path,
          "floating point value mismatch");
      return false;
    }
    return true;
  }
};

template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::float_t>
    : debug_thrift_data_difference_impl_floating_point<
          apache::thrift::type::float_t> {};
template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::double_t>
    : debug_thrift_data_difference_impl_floating_point<
          apache::thrift::type::double_t> {};

template <typename Tag>
struct debug_thrift_data_difference_impl_integral {
  using T = apache::thrift::type::standard_type<Tag>;

  template <typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(Tag{}, &lhs, &rhs, path, "integral value mismatch");
      return false;
    }
    return true;
  }
};

template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::bool_t>
    : debug_thrift_data_difference_impl_integral<apache::thrift::type::bool_t> {
};
template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::byte_t>
    : debug_thrift_data_difference_impl_integral<apache::thrift::type::byte_t> {
};
template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::i16_t>
    : debug_thrift_data_difference_impl_integral<apache::thrift::type::i16_t> {
};
template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::i32_t>
    : debug_thrift_data_difference_impl_integral<apache::thrift::type::i32_t> {
};
template <>
struct debug_thrift_data_difference_impl<apache::thrift::type::i64_t>
    : debug_thrift_data_difference_impl_integral<apache::thrift::type::i64_t> {
};

template <typename T, typename Tag>
struct debug_thrift_data_difference_impl<
    apache::thrift::type::cpp_type<T, Tag>> {
  template <typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    return debug_thrift_data_difference_impl<Tag>::differs(
        path, lhs, rhs, std::forward<Callback>(callback));
  }
};

/**
 * NOTE: It has been decided to give a priority to less efficient but
 * language-independendent implementation which simply calls Adapter::toThrift.
 * Certainly, from the perspective of customisation for C++ adapters, the
 * correct implementation would be to follow equality priorities for adapted
 * types with apache::thrift::op::equal, for example. However, since not all
 * user-defined operators imply the equality of Thrift objects themself, we
 * strive to more concise implementation based on equality of Thrift data
 * objects themself.
 */
template <typename Adapter, typename Tag>
struct debug_thrift_data_difference_impl<
    apache::thrift::type::adapted<Adapter, Tag>> {
  template <typename T, typename Callback>
  static bool differs(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    return debug_thrift_data_difference_impl<Tag>::differs(
        path,
        Adapter::toThrift(lhs),
        Adapter::toThrift(rhs),
        std::forward<Callback>(callback));
  }
};

} // namespace detail
} // namespace thrift
} // namespace facebook
