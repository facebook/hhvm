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

#ifndef THRIFT_FATAL_FOLLY_DYNAMIC_INL_POST_H_
#define THRIFT_FATAL_FOLLY_DYNAMIC_INL_POST_H_ 1

#include <memory>
#include <stdexcept>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <fatal/type/enum.h>
#include <fatal/type/search.h>
#include <fatal/type/variant_traits.h>
#include <thrift/lib/cpp/Thrift.h>

namespace apache {
namespace thrift {
namespace detail {

struct recurse_helper {
  /*
   * This implicitly will dereference any smart pointer before hitting
   * real conversion logic as we recurse. This is mainly to support
   * structs that use cpp.ref or cpp.ref_type annotations.
   */
  template <typename TC, typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    dynamic_converter_impl<TC>::to(out, input, format);
  }

  template <typename TC, typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    dynamic_converter_impl<TC>::from(out, input, format, adherence);
  }

  template <typename TC, typename T>
  static void to(
      folly::dynamic& out,
      optional_boxed_field_ref<const boxed_value_ptr<T>&> input,
      dynamic_format format) {
    if (!input) {
      out = nullptr;
      return;
    }
    dynamic_converter_impl<TC>::to(out, *input, format);
  }

  template <typename TC, typename T>
  static void from(
      optional_boxed_field_ref<boxed_value_ptr<T>&> out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.isNull()) {
      out.reset();
      return;
    }

    dynamic_converter_impl<TC>::from(out.emplace(), input, format, adherence);
  }

  template <typename TC, typename T>
  static void to(
      folly::dynamic& out,
      const std::shared_ptr<T>& input,
      dynamic_format format) {
    if (!input) {
      out = nullptr;
      return;
    }
    dynamic_converter_impl<TC>::to(out, *input, format);
  }

  template <typename TC, typename T>
  static void from(
      std::shared_ptr<T>& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.isNull()) {
      out.reset();
      return;
    }

    auto temp = std::make_shared<T>();
    dynamic_converter_impl<TC>::from(*temp, input, format, adherence);
    out = std::move(temp);
  }

  template <typename TC, typename T>
  static void to(
      folly::dynamic& out,
      const std::unique_ptr<T>& input,
      dynamic_format format) {
    if (!input) {
      out = nullptr;
      return;
    }
    dynamic_converter_impl<TC>::to(out, *input, format);
  }

  template <typename TC, typename T>
  static void from(
      std::unique_ptr<T>& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.isNull()) {
      out.reset();
      return;
    }

    auto temp = std::make_unique<T>();
    dynamic_converter_impl<TC>::from(*temp, input, format, adherence);
    out = std::move(temp);
  }

  template <typename TC, typename T>
  static void to(
      folly::dynamic& out,
      const std::shared_ptr<T const>& input,
      dynamic_format format) {
    if (!input) {
      out = nullptr;
      return;
    }
    dynamic_converter_impl<TC>::to(out, *input, format);
  }

  template <typename TC, typename T>
  static void from(
      std::shared_ptr<const T>& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.isNull()) {
      out.reset();
      return;
    }

    auto temp = std::make_shared<T>();
    dynamic_converter_impl<TC>::from(*temp, input, format, adherence);
    out = std::move(temp);
  }

  template <typename TC>
  static void from(
      std::unique_ptr<folly::IOBuf>& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.isNull()) {
      out.reset();
      return;
    }

    // explicit overload exists for IOBuf -> binary so we add here too
    dynamic_converter_impl<TC>::from(out, input, format, adherence);
  }

  template <typename TC>
  static void from(
      typename std::vector<bool>::reference out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    bool tmp;
    dynamic_converter_impl<TC>::from(tmp, input, format, adherence);
    out = tmp;
  }
};

template <>
struct dynamic_converter_impl<type_class::enumeration> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    switch (format) {
      case dynamic_format::PORTABLE: {
        const auto s = TEnumTraits<T>::findName(input);
        if (!s) {
          throw std::invalid_argument("invalid enum value");
        }
        out = s;
        break;
      }
      case dynamic_format::JSON_1:
        out = static_cast<typename fatal::enum_traits<T>::int_type>(input);
        break;
      default:
        assert("to_dynamic: unsupported format" == nullptr);
        break;
    }
  }

  template <typename T>
  static void from_portable(T& out, const folly::dynamic& input) {
    const auto& value = input.asString();

    if (!TEnumTraits<T>::findValue(value.c_str(), &out)) {
      throw std::invalid_argument("unrecognized enum value");
    }
  }

  template <typename T>
  static void from_json_1(T& out, const folly::dynamic& input) {
    out = static_cast<T>(input.asInt());
  }

  template <typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    switch (adherence) {
      case format_adherence::STRICT:
        switch (format) {
          case dynamic_format::PORTABLE:
            from_portable(out, input);
            break;
          case dynamic_format::JSON_1:
            from_json_1(out, input);
            break;
          default:
            assert("from_dynamic (STRICT): unsupported format" == nullptr);
            break;
        }
        break;

      case format_adherence::LENIENT:
        switch (format) {
          case dynamic_format::PORTABLE:
          case dynamic_format::JSON_1:
            if (input.isInt()) {
              from_json_1(out, input);
            } else {
              from_portable(out, input);
            }
            break;
          default:
            assert("from_dynamic (LENIENT): unsupported format" == nullptr);
            break;
        }
        break;

      default:
        assert("from_dynamic: unsupported format adherence" == nullptr);
        break;
    }
  }
};

template <typename ValueTypeClass>
struct dynamic_converter_impl<type_class::list<ValueTypeClass>> {
  template <typename T>
  static auto do_reserve(T& c, size_t size) -> decltype(c.reserve(size)) {
    c.reserve(size);
  }
  template <typename T>
  static void do_reserve(T&, ...) {}

  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::array;

    for (const auto& i : input) {
      folly::dynamic value(folly::dynamic::object);
      recurse_helper::to<ValueTypeClass>(value, i, format);
      out.push_back(std::move(value));
    }
  }

  template <typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.empty()) {
      return;
    }
    do_reserve(out, input.size());
    for (const auto& i : input) {
      out.emplace_back();
      recurse_helper::from<ValueTypeClass>(out.back(), i, format, adherence);
    }
  }
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct dynamic_converter_impl<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::object;

    for (const auto& [k, m] : input) {
      folly::dynamic key(folly::dynamic::object);
      recurse_helper::to<KeyTypeClass>(key, k, format);
      recurse_helper::to<MappedTypeClass>(out[std::move(key)], m, format);
    }
  }

  template <typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.empty()) {
      return;
    }

    for (const auto& i : input.items()) {
      typename T::key_type key;
      recurse_helper::from<KeyTypeClass>(key, i.first, format, adherence);

      recurse_helper::from<MappedTypeClass>(
          out[std::move(key)], i.second, format, adherence);
    }
  }
};

template <typename ValueTypeClass>
struct dynamic_converter_impl<type_class::set<ValueTypeClass>> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::array;

    for (const auto& i : input) {
      folly::dynamic value(folly::dynamic::object);
      recurse_helper::to<ValueTypeClass>(value, i, format);
      out.push_back(std::move(value));
    }
  }

  template <typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (input.empty()) {
      return;
    }

    for (const auto& i : input) {
      typename T::value_type value;
      recurse_helper::from<ValueTypeClass>(value, i, format, adherence);
      out.insert(std::move(value));
    }
  }
};

template <>
struct dynamic_converter_impl<type_class::variant> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    using descriptors = typename fatal::variant_traits<T>::descriptors;

    out = folly::dynamic::object;

    fatal::scalar_search<descriptors, fatal::get_type::id>(
        input.getType(), [&](auto indexed) {
          using descriptor = decltype(fatal::tag_type(indexed));
          recurse_helper::to<typename descriptor::metadata::type_class>(
              out[fatal::enum_to_string(input.getType(), nullptr)],
              typename descriptor::getter()(input),
              format);
        });
  }

  template <typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    using variant_traits = fatal::variant_traits<T>;
    using id_traits = fatal::enum_traits<typename variant_traits::id>;

    if (!input.isObject()) {
      throw std::invalid_argument(folly::to<std::string>(
          "dynamic input when converting to variant must be object: ",
          input.typeName()));
    } else if (input.size() > 1) {
      throw std::invalid_argument("unexpected additional fields for a variant");
    }

    auto items = input.items();
    auto i = items.begin();
    if (i == items.end()) {
      variant_traits::clear(out);
    } else {
      const auto type = i->first.stringPiece();
      const auto& entry = i->second;
      const bool found =
          fatal::trie_find<typename id_traits::fields, fatal::get_type::name>(
              type.begin(), type.end(), [&](auto tag) {
                using field = decltype(fatal::tag_type(tag));
                using id = typename field::value;
                using descriptor =
                    typename variant_traits::by_id::template descriptor<id>;

                variant_traits::by_id::template set<id>(out);
                recurse_helper::from<typename descriptor::metadata::type_class>(
                    variant_traits::by_id::template get<id>(out),
                    entry,
                    format,
                    adherence);
              });

      if (!found) {
        throw std::invalid_argument("unrecognized variant type");
      }
    }
  }
};

template <>
struct dynamic_converter_impl<type_class::structure> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::object;
    fatal::foreach<typename reflect_struct<T>::members>([&](auto indexed) {
      using member = decltype(fatal::tag_type(indexed));
      using impl = dynamic_converter_impl<typename member::type_class>;

      static_assert(
          fatal::is_complete<impl>::value, "to_dynamic: unsupported type");

      if (member::optional::value == optionality::optional &&
          !member::is_set(input)) {
        return;
      }

      recurse_helper::to<typename member::type_class>(
          out[folly::StringPiece(
              fatal::z_data<typename member::name>(),
              fatal::size<typename member::name>::value)],
          typename member::getter{}(input),
          format);
    });
  }

  template <typename T>
  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    using members = typename reflect_struct<T>::members;
    for (const auto& i : input.items()) {
      const auto member_name = i.first.stringPiece();
      fatal::trie_find<members, fatal::get_type::name>(
          member_name.begin(), member_name.end(), [&](auto tag) {
            using member = decltype(fatal::tag_type(tag));
            member::mark_set(out, true);
            recurse_helper::from<typename member::type_class>(
                typename member::getter{}(out), i.second, format, adherence);
          });
    }
  }
};

template <>
struct dynamic_converter_impl<type_class::string> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format) {
    out = input;
  }

  static void from(
      std::string& out,
      const folly::dynamic& input,
      dynamic_format,
      format_adherence) {
    out = input.asString();
  }

  template <typename T>
  static void from(
      T& out, const folly::dynamic& input, dynamic_format, format_adherence) {
    out = input.asString();
  }
};

template <>
struct dynamic_converter_impl<type_class::binary> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format) {
    out = folly::to<std::string>(input);
  }

  static void to(
      folly::dynamic& out, folly::IOBuf const& input, dynamic_format) {
    folly::IOBufQueue q;
    q.append(input);
    std::string str;
    q.appendToString(str);
    out = std::move(str);
  }

  static void from(
      std::string& out,
      const folly::dynamic& input,
      dynamic_format,
      format_adherence) {
    out = input.asString();
  }

  static void from(
      std::unique_ptr<folly::IOBuf>& out,
      const folly::dynamic& input,
      dynamic_format,
      format_adherence) {
    out = folly::IOBuf::copyBuffer(input.asString());
  }

  static void from(
      folly::IOBuf& out,
      const folly::dynamic& input,
      dynamic_format,
      format_adherence) {
    out =
        folly::IOBuf(folly::IOBuf::CopyBufferOp::COPY_BUFFER, input.asString());
  }

  template <typename T>
  static void from(
      T& out, const folly::dynamic& input, dynamic_format, format_adherence) {
    out = input.asString();
  }
};

template <>
struct dynamic_converter_impl<type_class::floating_point> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format) {
    out = static_cast<double>(input);
  }

  template <typename T>
  static void from(
      T& out, const folly::dynamic& input, dynamic_format, format_adherence) {
    out = static_cast<T>(input.asDouble());
  }
};

template <>
struct dynamic_converter_impl<type_class::integral> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format) {
    out = input;
  }

  static void from(
      bool& out,
      const folly::dynamic& input,
      dynamic_format,
      format_adherence) {
    out = input.asBool();
  }

  template <typename T>
  static void from(
      T& out, const folly::dynamic& input, dynamic_format, format_adherence) {
    out = static_cast<T>(input.asInt());
  }
};

} // namespace detail
} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_FOLLY_DYNAMIC_INL_POST_H_
