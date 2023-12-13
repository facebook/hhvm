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
#include <string>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

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

  t_const_value() = default;

  explicit t_const_value(int64_t val) noexcept { set_integer(val); }

  explicit t_const_value(std::string val) { set_string(std::move(val)); }

  t_const_value(const t_const_value& other)
      : kind_(other.kind_),
        owner_(other.owner_),
        ttype_(other.ttype_),
        ref_range_(other.ref_range_),
        is_enum_(other.is_enum_),
        enum_(other.enum_),
        enum_val_(other.enum_val_) {
    switch (kind_) {
      case CV_BOOL:
        set_bool(other.bool_val_);
        break;
      case CV_INTEGER:
        int_val_ = other.int_val_;
        break;
      case CV_DOUBLE:
        double_val_ = other.double_val_;
        break;
      case CV_STRING:
        string_val_ = other.string_val_;
        break;
      case CV_MAP:
        for (const auto& elem : other.get_map()) {
          add_map(elem.first->clone(), elem.second->clone());
        }
        break;
      case CV_LIST:
        for (const auto& elem : other.get_list()) {
          add_list(elem->clone());
        }
        break;
      case CV_IDENTIFIER:
        string_val_ = other.string_val_;
        break;
    }
  }

  std::unique_ptr<t_const_value> clone() const {
    return std::make_unique<t_const_value>(*this);
  }

  static std::unique_ptr<t_const_value> make_map() {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    return value;
  }

  static std::unique_ptr<t_const_value> make_identifier(
      source_location loc, std::string id) {
    auto value = std::make_unique<t_const_value>();
    value->kind_ = CV_IDENTIFIER;
    value->ref_range_ = {loc, loc + id.size()};
    value->string_val_ = std::move(id);
    return value;
  }

  void assign(t_const_value&& value) { *this = std::move(value); }

  void set_string(std::string val) {
    kind_ = CV_STRING;
    string_val_ = std::move(val);
  }

  const std::string& get_string() const {
    assert(kind_ == CV_STRING);
    return string_val_;
  }

  const std::string& get_identifier() const {
    assert(kind_ == CV_IDENTIFIER);
    return string_val_;
  }

  void set_integer(int64_t val) {
    kind_ = CV_INTEGER;
    int_val_ = val;
  }

  int64_t get_integer() const {
    assert(kind_ == CV_INTEGER || kind_ == CV_BOOL);
    return int_val_;
  }

  void set_double(double val) {
    kind_ = CV_DOUBLE;
    double_val_ = val;
  }

  double get_double() const {
    assert(kind_ == CV_INTEGER || kind_ == CV_DOUBLE);
    return double_val_;
  }

  void set_bool(bool val) {
    kind_ = CV_BOOL;
    bool_val_ = val;
    // Added to support backward compatibility with generators that
    // look for the integer value to determine the boolean value
    int_val_ = val;
  }

  bool get_bool() const {
    assert(kind_ == CV_BOOL);
    return bool_val_;
  }

  void set_map() { kind_ = CV_MAP; }

  void add_map(
      std::unique_ptr<t_const_value> key, std::unique_ptr<t_const_value> val) {
    map_val_raw_.emplace_back(key.get(), val.get());
    map_val_.emplace_back(std::move(key), std::move(val));
  }

  const std::vector<std::pair<t_const_value*, t_const_value*>>& get_map()
      const {
    return map_val_raw_;
  }

  void set_list() { kind_ = CV_LIST; }

  void add_list(std::unique_ptr<t_const_value> val) {
    list_val_raw_.push_back(val.get());
    list_val_.push_back(std::move(val));
  }

  const std::vector<t_const_value*>& get_list() const { return list_val_raw_; }

  t_const_value_kind kind() const { return kind_; }

  bool is_empty() const {
    switch (kind_) {
      case CV_MAP:
        return map_val_.empty();
      case CV_LIST:
        return list_val_.empty();
      case CV_STRING:
        return string_val_.empty();
      default:
        return false;
    }
    return false;
  }

  void set_owner(t_const* owner) { owner_ = owner; }

  t_const* get_owner() const { return owner_; }

  const t_type_ref& ttype() const { return ttype_; }
  void set_ttype(t_type_ref type) { ttype_ = std::move(type); }

  void set_is_enum(bool value = true) { is_enum_ = value; }

  bool is_enum() const { return is_enum_; }

  void set_enum(const t_enum* tenum) { enum_ = tenum; }

  const t_enum* get_enum() const { return enum_; }

  void set_enum_value(const t_enum_value* enum_val) { enum_val_ = enum_val; }

  const t_enum_value* get_enum_value() const { return enum_val_; }

  void set_ref_range(source_range rng) { ref_range_ = rng; }
  source_range ref_range() const { return ref_range_; }

 private:
  // Use a vector of pairs to store the contents of the map so that we
  // preserve thrift-file ordering when generating per-language source.
  std::vector<
      std::pair<std::unique_ptr<t_const_value>, std::unique_ptr<t_const_value>>>
      map_val_;
  std::vector<std::unique_ptr<t_const_value>> list_val_;
  std::string string_val_; // a string or an identifier
  bool bool_val_ = false;
  int64_t int_val_ = 0;
  double double_val_ = 0.0;

  std::vector<std::pair<t_const_value*, t_const_value*>> map_val_raw_;
  std::vector<t_const_value*> list_val_raw_;

  t_const_value_kind kind_ = CV_BOOL;
  t_const* owner_ = nullptr;
  t_type_ref ttype_;
  // If this value is cloned from a referenced const, contains the range of that
  // reference.
  source_range ref_range_;

  bool is_enum_ = false;
  const t_enum* enum_ = nullptr;
  const t_enum_value* enum_val_ = nullptr;

  t_const_value& operator=(t_const_value&&) = default;

 public:
  // TODO(afuller): Delete everything below here. It is only provided for
  // backwards compatibility.
  const t_type* get_ttype() const { return ttype_.get_type(); }
};

} // namespace compiler
} // namespace thrift
} // namespace apache

#endif
