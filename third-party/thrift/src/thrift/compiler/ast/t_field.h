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
#include <optional>
#include <sstream>
#include <string>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_struct;

enum class t_field_qualifier {
  none,
  required,
  optional,
  terse,
};

using t_field_id = int16_t;

/**
 * class t_field
 *
 * Class to represent a field in a thrift structure. A field has a data type,
 * a symbolic name, and a numeric identifier.
 *
 */
class t_field final : public t_named {
 public:
  // Field ids < -32 are for internal use
  static constexpr auto min_id = -32;

  /**
   * Constructor for t_field
   *
   * @param type - A field based on thrift types
   * @param name - The symbolic name of the field
   * @param id  - The numeric identifier of the field
   */
  t_field(t_type_ref type, std::string name, std::optional<t_field_id> id = {})
      : t_named(nullptr, std::move(name)),
        type_(std::move(type)),
        id_(id.value_or(0)),
        explicit_id_(id) {}

  t_field(t_field&&) = delete;
  t_field& operator=(t_field&&) = delete;

  const t_type_ref& type() const { return type_; }
  t_field_id id() const { return id_; }
  std::optional<t_field_id> explicit_id() const { return explicit_id_; }
  bool is_injected() const { return injected_; }
  void set_injected_id(t_field_id id) {
    id_ = id;
    explicit_id_ = id;
    injected_ = true;
  }

  const t_const_value* default_value() const { return value_.get(); }
  void set_default_value(std::unique_ptr<t_const_value> value) {
    value_ = std::move(value);
  }

  t_field_qualifier qualifier() const { return qual_; }
  void set_qualifier(t_field_qualifier qual) { qual_ = qual; }

  /**
   * Thrift AST nodes are meant to be non-copyable and non-movable, and should
   * never be cloned. This method exists to grand-father specific uses in the
   * target language generators. Do NOT add any new usage of this method.
   */
  std::unique_ptr<t_field> clone_DO_NOT_USE() const {
    auto clone = std::make_unique<t_field>(type_, name_, id_);
    clone->set_src_range(src_range());

    if (value_) {
      clone->value_ = value_->clone();
    }

    // unstructured annotations
    clone->reset_annotations(annotations());

    // structued annotations
    for (const auto& annot : structured_annotations()) {
      clone->add_structured_annotation(annot.clone());
    }

    clone->qual_ = qual_;

    return clone;
  }

 private:
  t_type_ref type_;
  t_field_id id_;
  std::optional<t_field_id> explicit_id_;

  t_field_qualifier qual_ = {};
  std::unique_ptr<t_const_value> value_;

  bool injected_ = false;

  // TODO(afuller): Delete everything below here. It is only provided for
  // backwards compatibility.
 public:
  enum class e_req {
    required = 0,
    optional = 1,
    opt_in_req_out = 2,
    terse = 3,
  };

  t_field(const t_type* type, std::string name, t_field_id key = 0)
      : t_field(t_type_ref::from_req_ptr(type), std::move(name), key) {}

  void set_value(std::unique_ptr<t_const_value> value) {
    set_default_value(std::move(value));
  }

  const t_const_value* get_default_value() const { return value_.get(); }
  t_const_value* get_default_value() { return value_.get(); }
  const t_const_value* get_value() const { return get_default_value(); }
  t_const_value* get_value() { return get_default_value(); }

  void set_req(e_req req) {
    switch (req) {
      case e_req::opt_in_req_out:
        set_qualifier(t_field_qualifier::none);
        break;
      case e_req::required:
        set_qualifier(t_field_qualifier::required);
        break;
      case e_req::optional:
        set_qualifier(t_field_qualifier::optional);
        break;
      case e_req::terse:
        set_qualifier(t_field_qualifier::terse);
        break;
    }
  }
  e_req get_req() const {
    switch (qual_) {
      case t_field_qualifier::none:
        return e_req::opt_in_req_out;
      case t_field_qualifier::required:
        return e_req::required;
      case t_field_qualifier::optional:
        return e_req::optional;
      case t_field_qualifier::terse:
        return e_req::terse;
    }
    return {};
  }

  const t_type* get_type() const { return type().get_type(); }
  void set_type(t_type_ref type) { type_ = type; }
  int32_t get_key() const { return id(); }
  void set_implicit_id(t_field_id id) { id_ = id; }
};

using t_field_list = node_list<t_field>;

} // namespace compiler
} // namespace thrift
} // namespace apache
