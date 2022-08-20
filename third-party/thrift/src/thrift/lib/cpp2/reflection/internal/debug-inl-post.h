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

#ifndef THRIFT_FATAL_DEBUG_POST_INL_H_
#define THRIFT_FATAL_DEBUG_POST_INL_H_ 1

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <fatal/type/enum.h>
#include <fatal/type/search.h>
#include <fatal/type/sort.h>
#include <fatal/type/transform.h>
#include <fatal/type/variant_traits.h>
#include <folly/Conv.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>

namespace apache {
namespace thrift {
namespace detail {

struct debug_equals_missing {
  template <typename TC, typename Iter, typename Callback>
  void operator()(
      TC tc,
      Callback&& callback,
      Iter iterLhs,
      folly::StringPiece path,
      folly::StringPiece message) {
    using value_type = std::remove_reference_t<decltype(*iterLhs)>;
    callback(tc, &*iterLhs, static_cast<value_type*>(nullptr), path, message);
  }

  template <typename TC, typename Callback>
  void operator()(
      TC tc,
      Callback&& callback,
      std::vector<bool>::const_iterator iterLhs,
      folly::StringPiece path,
      folly::StringPiece message) {
    const bool value = *iterLhs;
    callback(tc, &value, static_cast<bool*>(nullptr), path, message);
  }
};

struct debug_equals_extra {
  template <typename TC, typename Iter, typename Callback>
  void operator()(
      TC tc,
      Callback&& callback,
      Iter iterRhs,
      folly::StringPiece path,
      folly::StringPiece message) {
    using value_type = std::remove_reference_t<decltype(*iterRhs)>;
    callback(tc, static_cast<value_type*>(nullptr), &*iterRhs, path, message);
  }

  template <typename TC, typename Callback>
  void operator()(
      TC tc,
      Callback&& callback,
      std::vector<bool>::const_iterator iterRhs,
      folly::StringPiece path,
      folly::StringPiece message) {
    const bool value = *iterRhs;
    callback(tc, static_cast<const bool*>(nullptr), &value, path, message);
  }
};

template <typename T>
T const* debug_equals_get_pointer(T const& what) {
  return &what;
}

template <typename T>
T const* debug_equals_get_pointer(
    optional_boxed_field_ref<const boxed_value_ptr<T>&> what) {
  return what ? &*what : nullptr;
}

template <typename T>
T const* debug_equals_get_pointer(const std::shared_ptr<T>& what) {
  return what.get();
}

template <typename T>
T const* debug_equals_get_pointer(const std::unique_ptr<T>& what) {
  return what.get();
}

template <typename>
struct debug_equals_impl;

template <typename TC, typename T, typename Callback>
bool debug_equals(
    std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
  using impl = apache::thrift::detail::debug_equals_impl<TC>;
  return impl::equals(path, lhs, rhs, callback);
}

struct scoped_path {
 public:
  ~scoped_path() {
    assert(path_.size() >= size_);
    path_.resize(size_);
  }

  static scoped_path member(std::string& path, folly::StringPiece member) {
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

template <>
struct debug_equals_impl<type_class::enumeration> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      auto l = fatal::enum_to_string(lhs, "<unknown>");
      auto r = fatal::enum_to_string(rhs, "<unknown>");
      callback(type_class::unknown{}, &l, &r, path, "value mismatch");
      return false;
    }
    return true;
  }
};

template <typename ValueTypeClass>
struct debug_equals_impl<type_class::list<ValueTypeClass>> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    using impl = debug_equals_impl<ValueTypeClass>;
    const auto minSize = std::min(lhs.size(), rhs.size());
    bool result = true;

    for (std::size_t index = 0; index < minSize; ++index) {
      const auto l = lhs.begin() + index;
      const auto r = rhs.begin() + index;
      auto guard = scoped_path::index(path, index);
      result = impl::equals(path, *l, *r, callback) && result;
    }

    for (std::size_t index = minSize; index < lhs.size(); ++index) {
      const auto lIter = lhs.begin() + index;
      auto guard = scoped_path::index(path, index);
      debug_equals_missing()(
          ValueTypeClass{}, callback, lIter, path, "missing list entry");
      result = false;
    }

    for (std::size_t index = minSize; index < rhs.size(); ++index) {
      const auto rIter = rhs.begin() + index;
      auto guard = scoped_path::index(path, index);
      debug_equals_extra()(
          ValueTypeClass{}, callback, rIter, path, "extra list entry");
      result = false;
    }
    return result;
  }
};

template <typename TypeClass, typename Type>
struct debug_equals_impl_pretty {
  static std::string go(const Type& v) { return pretty_string<TypeClass>(v); }
};

template <typename Type>
struct debug_equals_impl_pretty<type_class::binary, Type> {
  static std::string go(const Type& v) {
    std::string out;
    out.reserve(4 + v.size() * 2);

    const auto to_hex_digit = [](std::uint8_t const c) {
      return "0123456789ABCDEF"[c & 0xf];
    };

    out.append("\"0x");
    for (auto c : v) {
      out.push_back(to_hex_digit(c >> 4));
      out.push_back(to_hex_digit(c));
    }
    out.push_back('"');

    return out;
  }
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct debug_equals_impl<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    using key_type = typename T::key_type;
    using mapped_impl = debug_equals_impl<MappedTypeClass>;
    using pretty_key = debug_equals_impl_pretty<KeyTypeClass, key_type>;

    bool result = true;

    for (const auto& l : lhs) {
      const auto& key = l.first;
      auto guard = scoped_path::key(path, pretty_key::go(key));
      if (rhs.find(key) == rhs.end()) {
        debug_equals_missing()(
            MappedTypeClass{}, callback, &l.second, path, "missing map entry");
        result = false;
      }
    }

    for (const auto& r : rhs) {
      const auto& key = r.first;
      auto guard = scoped_path::key(path, pretty_key::go(key));
      if (lhs.find(key) == lhs.end()) {
        debug_equals_extra()(
            MappedTypeClass{}, callback, &r.second, path, "extra map entry");
        result = false;
      }
    }

    for (const auto& l : lhs) {
      const auto& key = l.first;
      const auto ri = rhs.find(key);
      if (ri != rhs.end()) {
        auto guard = scoped_path::key(path, pretty_key::go(key));
        const auto& r = *ri;
        const auto& lv = l.second;
        const auto& rv = r.second;
        result = mapped_impl::equals(path, lv, rv, callback) && result;
      }
    }
    return result;
  }
};

template <typename ValueTypeClass>
struct debug_equals_impl<type_class::set<ValueTypeClass>> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    using value_type = typename T::value_type;
    using pretty_value = debug_equals_impl_pretty<ValueTypeClass, value_type>;

    bool result = true;

    for (const auto& l : lhs) {
      auto guard = scoped_path::key(path, pretty_value::go(l));
      if (rhs.find(l) == rhs.end()) {
        debug_equals_missing()(
            ValueTypeClass{}, callback, &l, path, "missing set entry");
        result = false;
      }
    }

    for (const auto& r : rhs) {
      auto guard = scoped_path::key(path, pretty_value::go(r));
      if (lhs.find(r) == lhs.end()) {
        debug_equals_extra()(
            ValueTypeClass{}, callback, &r, path, "extra set entry");
        result = false;
      }
    }
    return result;
  }
};

struct debug_equals_with_pointers {
 protected:
  template <typename TypeClass, typename T, typename U, typename Callback>
  static bool recurse_into(
      std::string& path,
      T const& lMember,
      T const& rMember,
      Callback&& callback,
      U const&,
      U const&) {
    return debug_equals_impl<TypeClass>::equals(
        path, lMember, rMember, std::forward<Callback>(callback));
  }

  template <typename TypeClass, typename T, typename U, typename Callback>
  static bool recurse_into_ptr(
      std::string& path,
      T const* lMember,
      T const* rMember,
      Callback&& callback,
      U const& lObject,
      U const& rObject) {
    if (!lMember && !rMember) {
      return true;
    }
    if (!rMember) {
      debug_equals_missing()(
          TypeClass{},
          std::forward<Callback>(callback),
          lMember,
          path,
          "missing");
      return false;
    }
    if (!lMember) {
      debug_equals_extra()(
          TypeClass{},
          std::forward<Callback>(callback),
          rMember,
          path,
          "extra");
      return false;
    }
    return recurse_into<TypeClass>(
        path,
        *lMember,
        *rMember,
        std::forward<Callback>(callback),
        lObject,
        rObject);
  }

  template <typename TypeClass, typename Callback, typename U, typename T>
  static bool recurse_into(
      std::string& path,
      optional_boxed_field_ref<const boxed_value_ptr<T>&> lMember,
      optional_boxed_field_ref<const boxed_value_ptr<T>&> rMember,
      Callback&& callback,
      U const& lObject,
      U const& rObject) {
    return recurse_into_ptr<TypeClass>(
        path,
        lMember ? &*lMember : nullptr,
        rMember ? &*rMember : nullptr,
        std::forward<Callback>(callback),
        lObject,
        rObject);
  }

  template <typename TypeClass, typename Callback, typename U, typename T>
  static bool recurse_into(
      std::string& path,
      const std::shared_ptr<T>& lMember,
      const std::shared_ptr<T>& rMember,
      Callback&& callback,
      U const& lObject,
      U const& rObject) {
    return recurse_into_ptr<TypeClass>(
        path,
        lMember.get(),
        rMember.get(),
        std::forward<Callback>(callback),
        lObject,
        rObject);
  }

  template <typename TypeClass, typename Callback, typename U, typename T>
  static bool recurse_into(
      std::string& path,
      const std::unique_ptr<T>& lMember,
      const std::unique_ptr<T>& rMember,
      Callback&& callback,
      U const& lObject,
      U const& rObject) {
    return recurse_into_ptr<TypeClass>(
        path,
        lMember.get(),
        rMember.get(),
        std::forward<Callback>(callback),
        lObject,
        rObject);
  }
};

template <>
struct debug_equals_impl<type_class::variant> : debug_equals_with_pointers {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    using traits = fatal::variant_traits<T>;
    using descriptors = typename traits::descriptors;

    if (traits::get_id(lhs) != traits::get_id(rhs)) {
      visit_changed<debug_equals_missing>(path, lhs, callback);
      visit_changed<debug_equals_extra>(path, rhs, callback);
      return false;
    }

    bool result = true;

    using key = fatal::get_type::id;
    fatal::scalar_search<descriptors, key>(lhs.getType(), [&](auto indexed) {
      using descriptor = decltype(fatal::tag_type(indexed));

      assert(descriptor::id::value == traits::get_id(lhs));
      assert(descriptor::id::value == traits::get_id(rhs));

      using name = typename descriptor::metadata::name;
      auto guard = scoped_path::member(path, fatal::z_data<name>());

      using type_class = typename descriptor::metadata::type_class;
      typename descriptor::getter getter;
      result = recurse_into<type_class>(
          path, getter(lhs), getter(rhs), callback, lhs, rhs);
    });

    return result;
  }

 private:
  template <typename Change, typename TC, typename T, typename Callback>
  static void visit_changed_field(
      std::string& path, T const& field, Callback&& callback) {
    Change()(
        TC{},
        std::forward<Callback>(callback),
        &field,
        path,
        "union type changed");
  }

  template <typename Change, typename TC, typename T, typename Callback>
  static void visit_changed_field(
      std::string& path,
      optional_boxed_field_ref<const boxed_value_ptr<T>&> field,
      Callback&& callback) {
    Change()(
        TC{},
        std::forward<Callback>(callback),
        field ? &*field : nullptr,
        path,
        "union type changed");
  }

  template <typename Change, typename TC, typename T, typename Callback>
  static void visit_changed_field(
      std::string& path, const std::unique_ptr<T>& field, Callback&& callback) {
    Change()(
        TC{},
        std::forward<Callback>(callback),
        field.get(),
        path,
        "union type changed");
  }

  template <typename Change, typename TC, typename T, typename Callback>
  static void visit_changed_field(
      std::string& path, const std::shared_ptr<T>& field, Callback&& callback) {
    Change()(
        TC{},
        std::forward<Callback>(callback),
        field.get(),
        path,
        "union type changed");
  }

  template <typename Change, typename T, typename Callback>
  static void visit_changed(
      std::string& path, T const& variant, Callback&& callback) {
    using traits = fatal::variant_traits<T>;
    using descriptors = typename traits::descriptors;
    fatal::scalar_search<descriptors, fatal::get_type::id>(
        variant.getType(), [&](auto indexed) {
          using descriptor = decltype(fatal::tag_type(indexed));

          assert(descriptor::id::value == traits::get_id(variant));

          using name = typename descriptor::metadata::name;
          using type_class = typename descriptor::metadata::type_class;
          auto guard = scoped_path::member(path, fatal::z_data<name>());

          typename descriptor::getter getter;
          visit_changed_field<Change, type_class>(
              path, getter(variant), callback);
        });
  }
};

template <>
struct debug_equals_impl<type_class::structure> : debug_equals_with_pointers {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    bool result = true;

    fatal::foreach<typename reflect_struct<T>::members>([&](auto indexed) {
      using member = decltype(fatal::tag_type(indexed));
      using getter = typename member::getter;
      typename member::type_class klass;

      if (member::optional::value == optionality::optional &&
          !member::is_set(lhs) && !member::is_set(rhs)) {
        return;
      }

      auto guard =
          scoped_path::member(path, fatal::z_data<typename member::name>());

      if (member::optional::value == optionality::optional) {
        if (!member::is_set(rhs)) {
          auto const& lPtr = debug_equals_get_pointer(getter{}(lhs));
          debug_equals_missing()(klass, callback, lPtr, path, "missing");
          result = false;
          return;
        }

        if (!member::is_set(lhs)) {
          auto const& rPtr = debug_equals_get_pointer(getter{}(rhs));
          debug_equals_extra()(klass, callback, rPtr, path, "extra");
          result = false;
          return;
        }
      }

      auto partial = recurse_into<typename member::type_class>(
          path, getter{}(lhs), getter{}(rhs), callback, lhs, rhs);
      result = result && partial; // no short-circuit
    });

    return result;
  }
};

template <>
struct debug_equals_impl<type_class::string> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(type_class::string{}, &lhs, &rhs, path, "string mismatch");
      return false;
    }
    return true;
  }
};

template <>
struct debug_equals_impl<type_class::binary> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(type_class::binary{}, &lhs, &rhs, path, "binary mismatch");
      return false;
    }
    return true;
  }
};

template <>
struct debug_equals_impl<type_class::floating_point> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(
          type_class::floating_point{},
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
struct debug_equals_impl<type_class::integral> {
  template <typename T, typename Callback>
  static bool equals(
      std::string& path, T const& lhs, T const& rhs, Callback&& callback) {
    if (lhs != rhs) {
      callback(
          type_class::integral{}, &lhs, &rhs, path, "integral value mismatch");
      return false;
    }
    return true;
  }
};

} // namespace detail
} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_DEBUG_POST_INL_H_
