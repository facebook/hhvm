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
  enum t_const_value_type {
    CV_BOOL,
    CV_INTEGER,
    CV_DOUBLE,
    CV_STRING,
    CV_MAP,
    CV_LIST
  };

  t_const_value() = default;

  explicit t_const_value(int64_t val) noexcept { set_integer(val); }

  explicit t_const_value(std::string val) { set_string(std::move(val)); }

  t_const_value(const t_const_value&) = delete;
  t_const_value(t_const_value&&) = delete;
  t_const_value& operator=(const t_const_value&) = delete;

  std::unique_ptr<t_const_value> clone() const {
    auto clone = std::make_unique<t_const_value>();
    switch (get_type()) {
      case CV_BOOL:
        clone->set_bool(get_bool());
        break;
      case CV_INTEGER:
        clone->set_integer(get_integer());
        break;
      case CV_DOUBLE:
        clone->set_double(get_double());
        break;
      case CV_STRING:
        clone->set_string(get_string());
        break;
      case CV_MAP:
        clone->set_map();
        for (const auto& map_elem : get_map()) {
          clone->add_map(map_elem.first->clone(), map_elem.second->clone());
        }
        break;
      case CV_LIST:
        clone->set_list();
        for (const auto& list_elem : get_list()) {
          clone->add_list(list_elem->clone());
        }
        break;
    }

    clone->set_owner(get_owner());
    clone->set_ttype(ttype());
    clone->set_is_enum(is_enum());
    clone->set_enum(get_enum());
    clone->set_enum_value(get_enum_value());

    return clone;
  }

  void assign(t_const_value&& value) { *this = std::move(value); }

  void set_string(std::string val) {
    valType_ = CV_STRING;
    stringVal_ = std::move(val);
  }

  const std::string& get_string() const {
    assert(valType_ == CV_STRING);
    return stringVal_;
  }

  void set_integer(int64_t val) {
    valType_ = CV_INTEGER;
    intVal_ = val;
  }

  int64_t get_integer() const {
    assert(valType_ == CV_INTEGER || valType_ == CV_BOOL);
    return intVal_;
  }

  void set_double(double val) {
    valType_ = CV_DOUBLE;
    doubleVal_ = val;
  }

  double get_double() const {
    assert(valType_ == CV_INTEGER || valType_ == CV_DOUBLE);
    return doubleVal_;
  }

  void set_bool(bool val) {
    valType_ = CV_BOOL;
    boolVal_ = val;
    // Added to support backward compatibility with generators that
    // look for the integer value to determine the boolean value
    intVal_ = val;
  }

  bool get_bool() const {
    assert(valType_ == CV_BOOL);
    return boolVal_;
  }

  void set_map() { valType_ = CV_MAP; }

  void add_map(
      std::unique_ptr<t_const_value> key, std::unique_ptr<t_const_value> val) {
    mapVal_raw_.emplace_back(key.get(), val.get());
    mapVal_.emplace_back(std::move(key), std::move(val));
  }

  const std::vector<std::pair<t_const_value*, t_const_value*>>& get_map()
      const {
    return mapVal_raw_;
  }

  void set_list() { valType_ = CV_LIST; }

  void add_list(std::unique_ptr<t_const_value> val) {
    listVal_raw_.push_back(val.get());
    listVal_.push_back(std::move(val));
  }

  const std::vector<t_const_value*>& get_list() const { return listVal_raw_; }

  t_const_value_type get_type() const { return valType_; }

  bool is_empty() const {
    switch (valType_) {
      case CV_MAP:
        return mapVal_.empty();
      case CV_LIST:
        return listVal_.empty();
      case CV_STRING:
        return stringVal_.empty();
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

  void set_enum(const t_enum* tenum) { tenum_ = tenum; }

  const t_enum* get_enum() const { return tenum_; }

  void set_enum_value(const t_enum_value* tenum_val) { tenum_val_ = tenum_val; }

  const t_enum_value* get_enum_value() const { return tenum_val_; }

  void set_ref_range(source_range rng) { ref_range_ = rng; }
  source_range ref_range() const { return ref_range_; }

 private:
  // Use a vector of pairs to store the contents of the map so that we
  // preserve thrift-file ordering when generating per-language source.
  std::vector<
      std::pair<std::unique_ptr<t_const_value>, std::unique_ptr<t_const_value>>>
      mapVal_;
  std::vector<std::unique_ptr<t_const_value>> listVal_;
  std::string stringVal_;
  bool boolVal_ = false;
  int64_t intVal_ = 0;
  double doubleVal_ = 0.0;

  std::vector<std::pair<t_const_value*, t_const_value*>> mapVal_raw_;
  std::vector<t_const_value*> listVal_raw_;

  t_const_value_type valType_ = CV_BOOL;
  t_const* owner_ = nullptr;
  t_type_ref ttype_;
  // If this value is cloned from a referenced const, contains the range of that
  // reference.
  source_range ref_range_;

  bool is_enum_ = false;
  const t_enum* tenum_ = nullptr;
  const t_enum_value* tenum_val_ = nullptr;

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
