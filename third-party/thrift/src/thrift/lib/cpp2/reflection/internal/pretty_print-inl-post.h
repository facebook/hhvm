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

#ifndef THRIFT_FATAL_PRETTY_PRINT_INL_POST_H_
#define THRIFT_FATAL_PRETTY_PRINT_INL_POST_H_ 1

#include <fatal/type/enum.h>
#include <fatal/type/search.h>
#include <fatal/type/variant_traits.h>
#include <folly/String.h>

namespace apache {
namespace thrift {
namespace detail {

/**
 * Pretty print specialization for enumerations.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <>
struct pretty_print_impl<type_class::enumeration> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << fatal::enum_to_string(what, nullptr);
  }
};

/**
 * Pretty print specialization for lists.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename ValueTypeClass>
struct pretty_print_impl<type_class::list<ValueTypeClass>> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << "<list>[";
    if (!what.empty()) {
      out.newline();
      const auto size = what.size();
      std::size_t index = 0;
      for (const auto& i : what) {
        auto scope = out.start_scope();
        scope << index << ": ";
        pretty_print_impl<ValueTypeClass>::print(scope, i);
        if (++index < size) {
          scope << ',';
        }
        scope.newline();
      }
    }
    out << ']';
  }
};

/**
 * Pretty print specialization for maps.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename KeyTypeClass, typename MappedTypeClass>
struct pretty_print_impl<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << "<map>{";
    if (!what.empty()) {
      out.newline();
      const auto size = what.size();
      std::size_t index = 0;
      for (const auto& i : what) {
        auto scope = out.start_scope();
        pretty_print_impl<KeyTypeClass>::print(scope, i.first);
        scope << ": ";
        pretty_print_impl<MappedTypeClass>::print(scope, i.second);
        if (++index < size) {
          scope << ',';
        }
        scope.newline();
      }
    }
    out << '}';
  }
};

/**
 * Pretty print specialization for sets.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename ValueTypeClass>
struct pretty_print_impl<type_class::set<ValueTypeClass>> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << "<set>{";
    if (!what.empty()) {
      out.newline();
      const auto size = what.size();
      std::size_t index = 0;
      for (const auto& i : what) {
        auto scope = out.start_scope();
        pretty_print_impl<ValueTypeClass>::print(scope, i);
        if (++index < size) {
          scope << ',';
        }
        scope.newline();
      }
    }
    out << '}';
  }
};

/**
 * Thrift structures and unions may contain members that are wrapped in smart
 * pointers. This class helps decode those.
 */
struct pretty_print_structure_with_pointers {
 protected:
  template <typename TypeClass, typename OutputStream, typename T>
  static void recurse_into(OutputStream& out, T const& member) {
    pretty_print_impl<TypeClass>::print(out, member);
  }

  template <typename TypeClass, typename OutputStream, typename T>
  static void recurse_into_ptr(OutputStream& out, T const* pMember) {
    if (!pMember) {
      out << "null";
      return;
    }
    recurse_into<TypeClass>(out, *pMember);
  }

  template <typename TypeClass, typename OutputStream, typename T>
  static void recurse_into(
      OutputStream& out,
      optional_boxed_field_ref<const boxed_value_ptr<T>&> pMember) {
    return recurse_into_ptr<TypeClass>(out, pMember ? &*pMember : nullptr);
  }

  template <typename TypeClass, typename OutputStream, typename T>
  static void recurse_into(
      OutputStream& out, const std::shared_ptr<T>& pMember) {
    return recurse_into_ptr<TypeClass>(out, pMember.get());
  }

  template <typename TypeClass, typename OutputStream, typename T>
  static void recurse_into(
      OutputStream& out, const std::unique_ptr<T>& pMember) {
    return recurse_into_ptr<TypeClass>(out, pMember.get());
  }
};

/**
 * Pretty print specialization for variants (Thrift unions).
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <>
struct pretty_print_impl<type_class::variant>
    : pretty_print_structure_with_pointers {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    using descriptors = typename fatal::variant_traits<T>::descriptors;
    out << "<variant>{";
    using key = fatal::get_type::id;
    fatal::scalar_search<descriptors, key>(what.getType(), [&](auto indexed) {
      using descriptor = decltype(fatal::tag_type(indexed));
      auto scope = out.start_scope();
      scope.newline();
      scope << fatal::enum_to_string(what.getType(), nullptr) << ": ";
      recurse_into<typename descriptor::metadata::type_class>(
          scope, typename descriptor::getter()(what));
      scope.newline();
    });
    out << '}';
  }
};

/*
 * Pretty print specialization for structures.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <>
struct pretty_print_impl<type_class::structure>
    : pretty_print_structure_with_pointers {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << "<struct>{";
    out.newline();
    using info = reflect_struct<T>;
    fatal::foreach<typename info::members>([&](auto indexed) {
      constexpr auto size = fatal::size<typename info::members>::value;
      using member = decltype(fatal::tag_type(indexed));
      if (member::optional::value == optionality::optional &&
          !member::is_set(what)) {
        return;
      }
      auto const index = fatal::tag_index(indexed);
      auto scope = out.start_scope();
      scope << fatal::z_data<typename member::name>() << ": ";
      recurse_into<typename member::type_class>(
          scope, typename member::getter{}(what));
      if (index + 1 < size) {
        scope << ',';
      }
      scope.newline();
    });
    out << '}';
  }
};

/**
 * Pretty print specialization for strings.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <>
struct pretty_print_impl<type_class::string> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << '"' << folly::cEscape<T>(what) << '"';
  }
};

/**
 * Pretty print fallback specialization.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename>
struct pretty_print_impl {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << what;
  }

  template <typename OutputStream>
  static void print(OutputStream& out, const bool what) {
    out << (what ? "true" : "false");
  }
};

} // namespace detail
} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_PRETTY_PRINT_INL_POST_H_
