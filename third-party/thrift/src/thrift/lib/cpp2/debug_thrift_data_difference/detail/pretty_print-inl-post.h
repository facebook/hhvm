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

#include <folly/String.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/op/Get.h>

namespace facebook {
namespace thrift {
namespace detail {

/**
 * Pretty print specialization for enumerations.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T>
struct pretty_print_impl<apache::thrift::type::enum_t<T>> {
  template <typename OutputStream>
  static void print(OutputStream& out, T const& what) {
    out << apache::thrift::util::enumName(what, nullptr);
  }
};

/**
 * Pretty print specialization for lists.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename ValueTag>
struct pretty_print_impl<apache::thrift::type::list<ValueTag>> {
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
        pretty_print_impl<ValueTag>::print(scope, i);
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
template <typename KeyTag, typename MappedTag>
struct pretty_print_impl<apache::thrift::type::map<KeyTag, MappedTag>> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << "<map>{";
    if (!what.empty()) {
      out.newline();
      const auto size = what.size();
      std::size_t index = 0;
      for (const auto& [key, value] : what) {
        auto scope = out.start_scope();
        pretty_print_impl<KeyTag>::print(scope, key);
        scope << ": ";
        pretty_print_impl<MappedTag>::print(scope, value);
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
template <typename ValueTag>
struct pretty_print_impl<apache::thrift::type::set<ValueTag>> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << "<set>{";
    if (!what.empty()) {
      out.newline();
      const auto size = what.size();
      std::size_t index = 0;
      for (const auto& i : what) {
        auto scope = out.start_scope();
        pretty_print_impl<ValueTag>::print(scope, i);
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
  template <typename Tag, typename OutputStream, typename T>
  static void recurse_into(OutputStream& out, T const& member) {
    pretty_print_impl<Tag>::print(out, member);
  }
};

/**
 * Pretty print specialization for variants (Thrift unions).
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T>
struct pretty_print_impl<apache::thrift::type::union_t<T>>
    : pretty_print_structure_with_pointers {
  template <typename OutputStream>
  static void print(OutputStream& out, T const& what) {
    out << "<variant>{";
    apache::thrift::op::invoke_by_field_id<T>(
        static_cast<apache::thrift::FieldId>(what.getType()),
        [&](auto id) {
          using Id = decltype(id);
          using Tag = apache::thrift::op::get_type_tag<T, Id>;

          auto scope = out.start_scope();
          scope.newline();
          scope << apache::thrift::util::enumName(what.getType(), nullptr)
                << ": ";
          if (const auto* fieldPtr = apache::thrift::op::getValueOrNull(
                  apache::thrift::op::get<Id>(what))) {
            recurse_into<Tag>(scope, *fieldPtr);
          }
          scope.newline();
        },
        [] {
          // union is __EMPTY__
        });
    out << '}';
  }
};

/*
 * Pretty print specialization for structures.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T>
struct pretty_print_impl<apache::thrift::type::struct_t<T>>
    : pretty_print_structure_with_pointers {
  template <typename OutputStream>
  static void print(OutputStream& out, T const& what) {
    out << "<struct>{";
    out.newline();
    apache::thrift::op::for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      using Tag = apache::thrift::op::get_type_tag<T, Id>;

      constexpr auto size = apache::thrift::op::size_v<T>;
      constexpr auto index =
          folly::to_underlying(apache::thrift::op::get_ordinal_v<T, Id>);

      if (const auto* fieldPtr = apache::thrift::op::getValueOrNull(
              apache::thrift::op::get<Id>(what))) {
        auto scope = out.start_scope();
        scope << apache::thrift::op::get_name_v<T, Id> << ": ";
        recurse_into<Tag>(scope, *fieldPtr);
        if (index < size) {
          scope << ',';
        }
        scope.newline();
      }
    });
    out << '}';
  }
};

template <typename T>
struct pretty_print_impl<apache::thrift::type::exception_t<T>>
    : pretty_print_impl<apache::thrift::type::struct_t<T>> {};

/**
 * Pretty print specialization for strings.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <>
struct pretty_print_impl<apache::thrift::type::string_t> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << '"' << folly::cEscape<T>(what) << '"';
  }
};

template <>
struct pretty_print_impl<apache::thrift::type::binary_t> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    out << R"("0x)" << folly::hexlify(what) << R"(")";
  }
  template <typename OutputStream>
  static void print(OutputStream& out, folly::IOBuf const& what) {
    return print(out, what.to<std::string>());
  }
  template <typename OutputStream>
  static void print(
      OutputStream& out, std::unique_ptr<folly::IOBuf> const& what) {
    if (what) {
      return print(out, what->to<std::string>());
    }
  }
};

template <typename T, typename Tag>
struct pretty_print_impl<apache::thrift::type::cpp_type<T, Tag>> {
  template <typename OutputStream>
  static void print(OutputStream& out, T const& what) {
    pretty_print_impl<Tag>::print(out, what);
  }
};

template <typename Adapter, typename Tag>
struct pretty_print_impl<apache::thrift::type::adapted<Adapter, Tag>> {
  template <typename OutputStream, typename T>
  static void print(OutputStream& out, T const& what) {
    pretty_print_impl<Tag>::print(out, Adapter::toThrift(what));
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
} // namespace facebook
