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

#include <thrift/compiler/sema/standard_validator.h>

#include <cassert>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/sema/diagnostic_context.h>

namespace apache::thrift::compiler {
namespace {

// Returns the category of an initializer. It is not a real type because the
// types of initializers are inferred later.
const char* get_category(const t_const_value* val) {
  switch (val->kind()) {
    case t_const_value::CV_BOOL:
      return "bool";
    case t_const_value::CV_INTEGER:
      return "integer";
    case t_const_value::CV_DOUBLE:
      return "floating-point number";
    case t_const_value::CV_STRING:
      return "string";
    case t_const_value::CV_MAP:
      return "map";
    case t_const_value::CV_LIST:
      return "list";
    case t_const_value::CV_IDENTIFIER:
      break;
  }
  return nullptr;
}

class checker {
 public:
  checker(diagnostics_engine& diags, const t_named& node, std::string name)
      : diags_(diags), node_(node), name_(std::move(name)) {}

  void check(const t_type* type, const t_const_value* value) {
    type = type->get_true_type();

    if (auto primitive_type = dynamic_cast<const t_primitive_type*>(type)) {
      check_primitive_type(primitive_type, value);
    } else if (auto enum_type = dynamic_cast<const t_enum*>(type)) {
      check_enum(enum_type, value);
    } else if (auto structured_type = dynamic_cast<const t_structured*>(type)) {
      check_structured(structured_type, value);
    } else if (auto map_type = dynamic_cast<const t_map*>(type)) {
      check_map(map_type, value);
    } else if (auto list_type = dynamic_cast<const t_list*>(type)) {
      check_list(list_type, value);
    } else if (auto set_type = dynamic_cast<const t_set*>(type)) {
      check_set(set_type, value);
    } else {
      assert(false); // Should be unreachable.
    }
  }

 private:
  diagnostics_engine& diags_;
  const t_named& node_;
  std::string name_;

  template <typename... T>
  void error(fmt::format_string<T...> msg, T&&... args) {
    diags_.error(node_, msg, std::forward<T>(args)...);
  }

  template <typename... T>
  void warning(fmt::format_string<T...> msg, T&&... args) {
    diags_.warning(node_, msg, std::forward<T>(args)...);
  }

  // Report an error when the initializer is incompatible with the type.
  void report_incompatible(
      const t_const_value* initializer, const t_type* type) {
    const char* category = get_category(initializer);
    if (!category) {
      return;
    }
    const std::string& type_name = dynamic_cast<const t_container*>(type)
        ? t_type::type_name(type->get_type_value())
        : type->name();
    error(
        "cannot convert {} to `{}` in initialization of `{}`",
        category,
        type_name,
        name_);
  }

  template <typename T>
  void check_int(const t_const_value* value, const t_type* type) {
    if (value->kind() != t_const_value::CV_INTEGER) {
      report_incompatible(value, type);
      return;
    }
    // Range check is not needed for int64_t but it makes the code simpler and
    // will be optimized away.
    int64_t int_value = value->get_integer();
    if (int_value < std::numeric_limits<T>::min() ||
        int_value > std::numeric_limits<T>::max()) {
      error(
          "{} is out of range for `{}` in initialization of `{}`",
          int_value,
          type->name(),
          name_);
    }
  }

  template <typename T>
  void check_float(const t_const_value* value, const t_type* type) {
    if (value->kind() == t_const_value::CV_DOUBLE) {
      // Range check is not needed for double but it makes the code simpler and
      // will be optimized away.
      double double_value = value->get_double();
      if (double_value < std::numeric_limits<T>::lowest() ||
          double_value > std::numeric_limits<T>::max()) {
        error(
            "{} is out of range for `{}` in initialization of `{}`",
            double_value,
            type->name(),
            name_);
      }
    } else if (value->kind() == t_const_value::CV_INTEGER) {
      int64_t int_value = value->get_integer();
      if (int_value != static_cast<int64_t>(static_cast<T>(int_value))) {
        error(
            "cannot convert {} to `{}` in initialization of `{}`",
            int_value,
            type->name(),
            name_);
      }
    } else {
      report_incompatible(value, type);
    }
  }

  void check_primitive_type(
      const t_primitive_type* type, const t_const_value* value) {
    const auto kind = value->kind();
    switch (type->primitive_type()) {
      case t_primitive_type::type::t_void:
        return;
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        if (kind != t_const_value::CV_STRING) {
          report_incompatible(value, type);
        }
        break;
      case t_primitive_type::type::t_bool:
        if (kind != t_const_value::CV_BOOL &&
            kind != t_const_value::CV_INTEGER) {
          report_incompatible(value, type);
        }
        break;
      case t_primitive_type::type::t_byte:
        check_int<int8_t>(value, type);
        break;
      case t_primitive_type::type::t_i16:
        check_int<int16_t>(value, type);
        break;
      case t_primitive_type::type::t_i32:
        check_int<int32_t>(value, type);
        break;
      case t_primitive_type::type::t_i64:
        check_int<int64_t>(value, type);
        break;
      case t_primitive_type::type::t_double:
        check_float<double>(value, type);
        break;
      case t_primitive_type::type::t_float:
        check_float<float>(value, type);
        break;
    }
  }

  void check_enum(const t_enum* type, const t_const_value* value) {
    if (value->kind() != t_const_value::CV_INTEGER) {
      report_incompatible(value, type);
      return;
    }

    if (type->find_value(value->get_integer()) == nullptr) {
      warning(
          "const `{}` is defined as enum `{}` with a value not of that enum",
          name_,
          type->name());
    }
  }

  void check_structured(const t_structured* type, const t_const_value* value) {
    if (value->kind() != t_const_value::CV_MAP) {
      report_incompatible(value, type);
      return;
    }
    const auto& map = value->get_map();
    if (map.size() > 1 && dynamic_cast<const t_union*>(type)) {
      error(
          "cannot initialize more than one field in union `{}`", type->name());
    }
    check_fields(type, map);
  }

  void check_fields(
      const t_structured* type,
      const std::vector<std::pair<t_const_value*, t_const_value*>>& map) {
    for (auto [key, value] : map) {
      std::string field_name;
      if (key->kind() == t_const_value::CV_STRING) {
        field_name = key->get_string();
      } else if (key->kind() == t_const_value::CV_IDENTIFIER) {
        field_name = key->get_identifier();
      } else {
        error("{} field name must be a string or an identifier", name_);
        continue;
      }
      const auto* field = type->get_field_by_name(field_name);
      if (!field) {
        error("no field named `{}` in `{}`", key->get_string(), type->name());
        continue;
      }
      const t_type* field_type = &field->type().deref();
      std::string name =
          name_.empty() ? field_name : fmt::format("{}.{}", name_, field_name);
      checker(diags_, node_, std::move(name)).check(field_type, value);
    }
  }

  void check_map(const t_map* type, const t_const_value* value) {
    if (value->kind() == t_const_value::CV_LIST && value->get_list().empty()) {
      warning(
          "converting empty list to `map` in initialization of `{}`", name_);
      return;
    }
    if (value->kind() != t_const_value::CV_MAP) {
      report_incompatible(value, type);
      return;
    }
    const t_type* key_type = &type->key_type().deref();
    const t_type* val_type = &type->val_type().deref();
    for (const auto& entry : value->get_map()) {
      check(key_type, entry.first);
      check(val_type, entry.second);
    }
  }

  void check_list(const t_list* type, const t_const_value* value) {
    if (value->kind() == t_const_value::CV_MAP && value->get_map().empty()) {
      warning(
          "converting empty map to `list` in initialization of `{}`", name_);
      return;
    }
    if (value->kind() != t_const_value::CV_LIST) {
      report_incompatible(value, type);
      return;
    }
    check_elements(&type->elem_type().deref(), value->get_list());
  }

  void check_set(const t_set* type, const t_const_value* value) {
    if (value->kind() == t_const_value::CV_MAP && value->get_map().empty()) {
      warning("converting empty map to `set` in initialization of `{}`", name_);
      return;
    }
    if (value->kind() != t_const_value::CV_LIST) {
      report_incompatible(value, type);
      return;
    }
    check_elements(&type->elem_type().deref(), value->get_list());
  }

  void check_elements(
      const t_type* elem_type, const std::vector<t_const_value*>& list) {
    for (const auto& elem : list) {
      check(elem_type, elem);
    }
  }
};
} // namespace

// (ffrancet) I managed to trace this comment all the way back to 2008 when
// thrift was migrated to the fbcode repo. True piece of history here:
//
// You know, when I started working on Thrift I really thought it wasn't going
// to become a programming language because it was just a generator and it
// wouldn't need runtime type information and all that jazz. But then we
// decided to add constants, and all of a sudden that means runtime type
// validation and inference, except the "runtime" is the code generator
// runtime. Shit. I've been had.
void detail::check_initializer(
    diagnostics_engine& diags,
    const t_named& node,
    const t_type* type,
    const t_const_value* initializer) {
  checker(diags, node, node.name()).check(type, initializer);
}

} // namespace apache::thrift::compiler
