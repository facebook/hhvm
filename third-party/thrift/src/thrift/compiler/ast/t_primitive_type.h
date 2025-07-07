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

#include <cstdlib>
#include <vector>

#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

/**
 * A thrift primitive type, which must be one of the defined enumerated types
 * inside this definition.
 *
 */
class t_primitive_type : public t_type {
 public:
  /**
   * The subset of t_type::type that are primitive types.
   */
  enum class type {
    t_void = int(t_type::type::t_void),
    t_string = int(t_type::type::t_string),
    t_bool = int(t_type::type::t_bool),
    t_byte = int(t_type::type::t_byte),
    t_i16 = int(t_type::type::t_i16),
    t_i32 = int(t_type::type::t_i32),
    t_i64 = int(t_type::type::t_i64),
    t_double = int(t_type::type::t_double),
    t_float = int(t_type::type::t_float),
    t_binary = int(t_type::type::t_binary),
  };

  // A singleton per type.
  static const t_primitive_type& t_void();
  static const t_primitive_type& t_string();
  static const t_primitive_type& t_binary();
  static const t_primitive_type& t_bool();
  static const t_primitive_type& t_byte();
  static const t_primitive_type& t_i16();
  static const t_primitive_type& t_i32();
  static const t_primitive_type& t_i64();
  static const t_primitive_type& t_double();
  static const t_primitive_type& t_float();

  // Returns the name for the given type.
  using t_type::type_name;
  static const std::string& type_name(type primitive_type) {
    return type_name(static_cast<t_type::type>(primitive_type));
  }

  type primitive_type() const { return primitive_type_; }

  // TODO(afuller): Disable copy constructor, and use
  // 'anonymous' typdefs instead.
  // t_primitive_type(const t_primitive_type&) = delete;

  std::string get_full_name() const override {
    return type_name(primitive_type_);
  }

  ~t_primitive_type() override;

 private:
  type primitive_type_;

  t_primitive_type(std::string name, type primitive_type)
      : t_type(std::move(name)), primitive_type_(primitive_type) {}

  // TODO(afuller): Remove everything below here. It is provided only for
  // backwards compatibility.
 public:
  using t_primitive = type;
  constexpr static t_primitive TYPE_VOID = type::t_void;
  constexpr static t_primitive TYPE_STRING = type::t_string;
  constexpr static t_primitive TYPE_BOOL = type::t_bool;
  constexpr static t_primitive TYPE_BYTE = type::t_byte;
  constexpr static t_primitive TYPE_I16 = type::t_i16;
  constexpr static t_primitive TYPE_I32 = type::t_i32;
  constexpr static t_primitive TYPE_I64 = type::t_i64;
  constexpr static t_primitive TYPE_DOUBLE = type::t_double;
  constexpr static t_primitive TYPE_FLOAT = type::t_float;
  constexpr static t_primitive TYPE_BINARY = type::t_binary;

  static std::string t_primitive_name(t_primitive t) { return type_name(t); }

  t_primitive get_base() const { return primitive_type(); }

  /**
   * t_type overrides
   */
  bool is_void() const override { return primitive_type_ == type::t_void; }

  bool is_bool() const override { return primitive_type_ == type::t_bool; }

  bool is_byte() const override { return primitive_type_ == type::t_byte; }

  bool is_i16() const override { return primitive_type_ == type::t_i16; }

  bool is_i32() const override { return primitive_type_ == type::t_i32; }

  bool is_i64() const override { return primitive_type_ == type::t_i64; }

  bool is_float() const override { return primitive_type_ == type::t_float; }

  bool is_double() const override { return primitive_type_ == type::t_double; }

  bool is_string() const override { return primitive_type_ == type::t_string; }

  bool is_binary() const override { return primitive_type_ == type::t_binary; }

  t_type::type get_type_value() const override {
    return static_cast<t_type::type>(primitive_type_);
  }

  uint64_t get_type_id() const override {
    return static_cast<uint64_t>(primitive_type_);
  }
};

} // namespace apache::thrift::compiler
