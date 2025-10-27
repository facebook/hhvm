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

#ifndef T_CONST_VALUE_H
#define T_CONST_VALUE_H

#include <stdint.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

class t_const;
class t_enum;
class t_enum_value;

/**
 * A const value is something parsed that could be a map, set, list, struct
 * or whatever.
 *
 */
class t_const_value {
 public:
  enum t_const_value_kind {
    CV_BOOL,
    CV_INTEGER,
    CV_DOUBLE,
    CV_STRING,
    CV_MAP,
    CV_LIST,
    CV_IDENTIFIER
  };

  t_const_value() : kind_(CV_BOOL), value_(int64_t()) {}

  explicit t_const_value(int64_t val) noexcept { set_integer(val); }

  explicit t_const_value(std::string val)
      : kind_(CV_STRING), value_(std::string(std::move(val))) {}

  t_const_value(const t_const_value&) = default;
  t_const_value(t_const_value&&) = default;
  t_const_value& operator=(t_const_value&&) = default;

  std::unique_ptr<t_const_value> clone() const {
    return std::make_unique<t_const_value>(*this);
  }

  static std::unique_ptr<t_const_value> make_identifier(
      source_location loc, std::string id, const t_program& program) {
    auto range = source_range{loc, loc + id.size()};
    auto value = std::make_unique<t_const_value>(std::move(id));
    value->kind_ = CV_IDENTIFIER;
    value->ref_range_ = range;
    value->program_ = &program;
    return value;
  }

  static std::unique_ptr<t_const_value> make_map() {
    auto value = std::make_unique<t_const_value>();
    value->value_ = map_value();
    value->kind_ = CV_MAP;
    return value;
  }

  static std::unique_ptr<t_const_value> make_list() {
    auto value = std::make_unique<t_const_value>();
    value->value_ = list_value();
    value->kind_ = CV_LIST;
    return value;
  }

  void assign(t_const_value&& value) { *this = std::move(value); }

  const std::string& get_string() const {
    check_kind(CV_STRING);
    return std::get<std::string>(value_);
  }

  const std::string& get_identifier() const {
    check_kind(CV_IDENTIFIER);
    return std::get<std::string>(value_);
  }

  void set_integer(int64_t val) {
    kind_ = CV_INTEGER;
    value_ = val;
  }

  int64_t get_integer() const {
    if (kind_ != CV_INTEGER && kind_ != CV_BOOL) {
      throw std::runtime_error(
          fmt::format(
              "invalid const value access: {}", fmt::underlying(kind_)));
    }
    return std::get<int64_t>(value_);
  }

  void set_double(double val) {
    kind_ = CV_DOUBLE;
    value_ = val;
  }

  double get_double() const {
    check_kind(CV_DOUBLE);
    return std::get<double>(value_);
  }

  void set_bool(bool val) {
    kind_ = CV_BOOL;
    // Bool is stored as an integer for compatibility with legacy generators.
    value_ = static_cast<int64_t>(val);
  }

  bool get_bool() const {
    check_kind(CV_BOOL);
    return std::get<int64_t>(value_);
  }

  void add_map(
      std::unique_ptr<t_const_value> key, std::unique_ptr<t_const_value> val) {
    check_kind(CV_MAP);
    std::get<map_value>(value_).add(std::move(key), std::move(val));
  }

  const std::vector<std::pair<t_const_value*, t_const_value*>>& get_map()
      const {
    check_kind(CV_MAP);
    return std::get<map_value>(value_).raw;
  }

  void add_list(std::unique_ptr<t_const_value> val) {
    check_kind(CV_LIST);
    std::get<list_value>(value_).add(std::move(val));
  }

  const std::vector<t_const_value*>& get_list() const {
    check_kind(CV_LIST);
    return std::get<list_value>(value_).raw;
  }

  // Returns list elements if this is a list or empty vector if this is a map.
  // It is used to simplify handling cases where sets are initialized with
  // empty map constants which are supported for historical reasons, e.g.
  //   const set<i32> s = {}
  const std::vector<t_const_value*>& get_list_or_empty_map() const;

  void convert_identifier_to_string() {
    check_kind(CV_IDENTIFIER);
    kind_ = CV_STRING;
  }

  void convert_empty_map_to_list() {
    check_kind(CV_MAP);
    value_ = list_value();
    kind_ = CV_LIST;
  }

  void convert_empty_list_to_map() {
    check_kind(CV_LIST);
    value_ = map_value();
    kind_ = CV_MAP;
  }

  std::optional<source_range> src_range() const { return src_range_; }

  void set_src_range(source_range range) { src_range_ = range; }

  t_const_value_kind kind() const { return kind_; }

  bool is_empty() const {
    switch (kind_) {
      case CV_MAP:
        return std::get<map_value>(value_).elements.empty();
      case CV_LIST:
        return std::get<list_value>(value_).elements.empty();
      case CV_STRING:
        return std::get<std::string>(value_).empty();
      default:
        return false;
    }
    return false;
  }

  void set_owner(t_const* owner) { owner_ = owner; }

  t_const* get_owner() const { return owner_; }

  const t_type_ref& ttype() const { return ttype_; }
  void set_ttype(t_type_ref type) { ttype_ = type; }

  void set_is_enum(bool value = true) { is_enum_ = value; }

  bool is_enum() const { return is_enum_; }

  void set_enum(const t_enum* tenum) { enum_ = tenum; }

  const t_enum* get_enum() const { return enum_; }

  void set_enum_value(const t_enum_value* enum_val) { enum_val_ = enum_val; }

  const t_enum_value* get_enum_value() const { return enum_val_; }

  bool is_enum_value() const { return enum_val_ != nullptr; }

  void set_ref_range(source_range rng) { ref_range_ = rng; }
  source_range ref_range() const { return ref_range_; }

  void set_is_struct_literal(bool value = true) { is_struct_literal_ = value; }
  bool is_struct_literal() const { return is_struct_literal_; }

  const t_program& program() const {
    check_kind(CV_IDENTIFIER);
    assert(program_);
    return *program_;
  }

  // Returns a debug string representation of a const value kind.
  static std::string_view kind_to_string(t_const_value_kind kind);

 private:
  t_const_value_kind kind_ = CV_BOOL;

  std::optional<source_range> src_range_;

  struct map_value {
    // Use a vector of pairs to store the contents of the map so that we
    // preserve Thrift-file ordering when generating per-language source.
    std::vector<std::pair<
        std::unique_ptr<t_const_value>,
        std::unique_ptr<t_const_value>>>
        elements;
    std::vector<std::pair<t_const_value*, t_const_value*>> raw;

    void add(
        std::unique_ptr<t_const_value> key,
        std::unique_ptr<t_const_value> val) {
      raw.emplace_back(key.get(), val.get());
      elements.emplace_back(std::move(key), std::move(val));
    }

    map_value() = default;
    map_value(map_value&&) = default;
    map_value(const map_value& other) {
      for (auto& elem : other.elements) {
        add(elem.first->clone(), elem.second->clone());
      }
    }
    map_value& operator=(map_value&&) = default;
  };

  struct list_value {
    std::vector<std::unique_ptr<t_const_value>> elements;
    std::vector<t_const_value*> raw;

    void add(std::unique_ptr<t_const_value> val) {
      raw.push_back(val.get());
      elements.push_back(std::move(val));
    }

    list_value() = default;
    list_value(list_value&&) = default;
    list_value(const list_value& other) {
      for (auto& elem : other.elements) {
        add(elem->clone());
      }
    }
    list_value& operator=(list_value&&) = default;
  };

  std::variant<
      int64_t, // a bool or an integer
      double,
      std::string, // a string or an identifier
      map_value,
      list_value>
      value_;

  t_const* owner_ = nullptr;
  const t_program* program_ = nullptr; // If this is an identifier, the program
                                       // where the reference appears.
  t_type_ref ttype_;
  // If this value is cloned from a referenced const, contains the range of that
  // reference.
  source_range ref_range_;

  bool is_enum_ = false;
  const t_enum* enum_ = nullptr;
  const t_enum_value* enum_val_ = nullptr;

  // Set to true if the value is a struct literal, i.e. when field
  // assignment using unquoted field names is used. For example:
  //   const MyStruct s = MyStruct{a = 1, b = 2};
  bool is_struct_literal_ = false;

  void check_kind(t_const_value_kind expected) const {
    if (kind_ != expected) {
      throw std::runtime_error(
          fmt::format(
              "invalid const value access: expected '{}', got '{}'",
              kind_to_string(expected),
              kind_to_string(kind_)));
    }
  }
};

} // namespace apache::thrift::compiler

#endif
