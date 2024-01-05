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

#include <memory>
#include <stdexcept>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace facebook {
namespace thrift {
namespace detail {

namespace {

template <typename Id, typename T, typename R>
struct Ensure {
  constexpr decltype(auto) operator()(Id id, T& obj) const {
    return apache::thrift::op::ensure<>(id, obj);
  }
};

// the shared pointer to const object needs to be recreated as a whole to
// obtain a mutable reference to its object
template <typename Id, typename T, typename R>
struct Ensure<Id, T, std::shared_ptr<const R>> {
  constexpr R& operator()(Id, T& obj) const {
    std::shared_ptr<const R>& s = apache::thrift::op::get<Id>(obj);

    auto temp = std::make_shared<R>();
    s = temp;
    return *temp;
  }
};

template <typename Id, typename T>
static constexpr decltype(auto) ensure(Id id, T& obj) {
  return Ensure<Id, T, apache::thrift::op::get_field_ref<T, Id>>{}(id, obj);
}

} // namespace

template <typename T>
struct dynamic_converter_impl<apache::thrift::type::enum_t<T>> {
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    switch (format) {
      case dynamic_format::PORTABLE: {
        const auto s = apache::thrift::TEnumTraits<T>::findName(input);
        if (!s) {
          throw std::invalid_argument("invalid enum value");
        }
        out = s;
        break;
      }
      case dynamic_format::JSON_1:
        out = folly::to_underlying(input);
        break;
      default:
        assert("to_dynamic: unsupported format" == nullptr);
        break;
    }
  }

  static void from_portable(T& out, const folly::dynamic& input) {
    const auto& value = input.asString();

    if (!apache::thrift::TEnumTraits<T>::findValue(value.c_str(), &out)) {
      throw std::invalid_argument("unrecognized enum value");
    }
  }

  static void from_json_1(T& out, const folly::dynamic& input) {
    out = static_cast<T>(input.asInt());
  }

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

template <typename VTag>
struct dynamic_converter_impl<apache::thrift::type::list<VTag>> {
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
      dynamic_converter_impl<VTag>::to(value, i, format);
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
      dynamic_converter_impl<VTag>::from(out.back(), i, format, adherence);
    }
  }
};

template <typename KTag, typename VTag>
struct dynamic_converter_impl<apache::thrift::type::map<KTag, VTag>> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::object;

    for (const auto& [k, m] : input) {
      folly::dynamic key(folly::dynamic::object);
      dynamic_converter_impl<KTag>::to(key, k, format);
      dynamic_converter_impl<VTag>::to(out[std::move(key)], m, format);
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

    for (const auto& [name, entry] : input.items()) {
      typename T::key_type key;
      dynamic_converter_impl<KTag>::from(key, name, format, adherence);

      dynamic_converter_impl<VTag>::from(
          out[std::move(key)], entry, format, adherence);
    }
  }
};

template <typename VTag>
struct dynamic_converter_impl<apache::thrift::type::set<VTag>> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::array;

    for (const auto& i : input) {
      folly::dynamic value(folly::dynamic::object);
      dynamic_converter_impl<VTag>::to(value, i, format);
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
      dynamic_converter_impl<VTag>::from(value, i, format, adherence);
      out.insert(std::move(value));
    }
  }
};

template <typename T>
struct dynamic_converter_impl<apache::thrift::type::union_t<T>> {
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::object;

    apache::thrift::op::invoke_by_field_id<T>(
        static_cast<apache::thrift::FieldId>(input.getType()),
        [&]<class Id>(Id) {
          using FieldTag = apache::thrift::op::get_field_tag<T, Id>;

          dynamic_converter_impl<FieldTag>::to(out, input, format);
        },
        [] {
          // union is __EMPTY__
        });
  }

  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    if (!input.isObject()) {
      throw std::invalid_argument(folly::to<std::string>(
          "dynamic input when converting to variant must be object: ",
          input.typeName()));
    } else if (input.size() > 1) {
      throw std::invalid_argument("unexpected additional fields for a variant");
    }

    if (input.empty()) {
      apache::thrift::op::clear<>(out);
      return;
    }

    const auto& [name, entry] = *input.items().begin();

    const bool found = apache::thrift::op::find_by_field_id<T>(
        [&, &name = name, &entry = entry]<class Id>(Id) {
          using FieldTag = apache::thrift::op::get_field_tag<T, Id>;

          if (apache::thrift::op::get_name_v<T, Id> == name.stringPiece()) {
            if (!entry.isNull()) {
              dynamic_converter_impl<FieldTag>::from(
                  out, entry, format, adherence);
            }
            return true;
          }
          return false;
        });

    if (!found) {
      throw std::invalid_argument("unrecognized variant type");
    }
  }
};

template <typename T>
struct dynamic_converter_impl<apache::thrift::type::struct_t<T>> {
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    out = folly::dynamic::object;
    apache::thrift::op::for_each_field_id<T>([&]<class Id>(Id) {
      using FieldTag = apache::thrift::op::get_field_tag<T, Id>;

      dynamic_converter_impl<FieldTag>::to(out, input, format);
    });
  }

  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    apache::thrift::op::for_each_field_id<T>([&]<class Id>(Id) {
      using FieldTag = apache::thrift::op::get_field_tag<T, Id>;

      if (auto it = input.find(apache::thrift::op::get_name_v<T, Id>);
          it != input.items().end()) {
        const auto& [_, entry] = *it;
        if (!entry.isNull()) {
          dynamic_converter_impl<FieldTag>::from(out, entry, format, adherence);
        }
      }
    });
  }
};

template <typename Tag, typename Struct, int16_t FieldId>
struct dynamic_converter_impl<apache::thrift::type::field<
    Tag,
    apache::thrift::FieldContext<Struct, FieldId>>> {
  static void to(
      folly::dynamic& out, Struct const& input, dynamic_format format) {
    folly::StringPiece fieldName = apache::thrift::op::get_name_v<Struct, Id>;
    if (const auto* ref = apache::thrift::op::getValueOrNull(
            apache::thrift::op::get<Id>(input))) {
      dynamic_converter_impl<Tag>::to(out[fieldName], *ref, format);
    } else if (!apache::thrift::detail::is_optional_or_union_field_ref_v<
                   apache::thrift::op::get_field_ref<Struct, Id>>) {
      out[fieldName] = nullptr;
    }
  }

  static void from(
      Struct& s,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    dynamic_converter_impl<Tag>::from(
        ensure<>(id, s), input, format, adherence);
  }

 private:
  using Id = apache::thrift::field_id<FieldId>;
  static constexpr auto id = Id{};
};

template <>
struct dynamic_converter_impl<apache::thrift::type::string_t> {
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
struct dynamic_converter_impl<apache::thrift::type::binary_t> {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format) {
    out = folly::to<std::string>(input);
  }

  static void to(
      folly::dynamic& out,
      std::unique_ptr<folly::IOBuf> const& input,
      dynamic_format format) {
    if (input) {
      to(out, *input, format);
    } else {
      out = nullptr;
    }
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

template <typename Tag>
struct dynamic_converter_impl_floating_point {
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
struct dynamic_converter_impl<apache::thrift::type::float_t>
    : dynamic_converter_impl_floating_point<apache::thrift::type::float_t> {};
template <>
struct dynamic_converter_impl<apache::thrift::type::double_t>
    : dynamic_converter_impl_floating_point<apache::thrift::type::double_t> {};

template <>
struct dynamic_converter_impl<apache::thrift::type::bool_t> {
  static void to(folly::dynamic& out, bool input, dynamic_format) {
    out = input;
  }

  static void from(
      bool& out,
      const folly::dynamic& input,
      dynamic_format,
      format_adherence) {
    out = input.asBool();
  }

  static void from(
      typename std::vector<bool>::reference out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    bool tmp;
    from(tmp, input, format, adherence);
    out = tmp;
  }
};

template <typename Tag>
struct dynamic_converter_impl_integral {
  template <typename T>
  static void to(folly::dynamic& out, T const& input, dynamic_format) {
    out = input;
  }

  template <typename T>
  static void from(
      T& out, const folly::dynamic& input, dynamic_format, format_adherence) {
    out = static_cast<T>(input.asInt());
  }
};

template <>
struct dynamic_converter_impl<apache::thrift::type::byte_t>
    : dynamic_converter_impl_integral<apache::thrift::type::byte_t> {};
template <>
struct dynamic_converter_impl<apache::thrift::type::i16_t>
    : dynamic_converter_impl_integral<apache::thrift::type::i16_t> {};
template <>
struct dynamic_converter_impl<apache::thrift::type::i32_t>
    : dynamic_converter_impl_integral<apache::thrift::type::i32_t> {};
template <>
struct dynamic_converter_impl<apache::thrift::type::i64_t>
    : dynamic_converter_impl_integral<apache::thrift::type::i64_t> {};

template <typename T, typename Tag>
struct dynamic_converter_impl<apache::thrift::type::cpp_type<T, Tag>> {
  static void to(folly::dynamic& out, T const& input, dynamic_format format) {
    dynamic_converter_impl<Tag>::to(out, input, format);
  }

  static void from(
      T& out,
      const folly::dynamic& input,
      dynamic_format format,
      format_adherence adherence) {
    dynamic_converter_impl<Tag>::from(out, input, format, adherence);
  }
};

} // namespace detail
} // namespace thrift
} // namespace facebook
