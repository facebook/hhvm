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
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/sema/sema_context.h>

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

class compatibility_checker {
 public:
  compatibility_checker(
      diagnostics_engine& diags, const t_named& node, std::string name)
      : diags_(diags), node_(node), name_(std::move(name)) {}

  bool check(const t_type* type, const t_const_value* value) {
    type = type->get_true_type();

    if (auto primitive_type = dynamic_cast<const t_primitive_type*>(type)) {
      return check_primitive_type(primitive_type, value);
    } else if (auto enum_type = dynamic_cast<const t_enum*>(type)) {
      return check_enum(enum_type, value);
    } else if (auto structured_type = dynamic_cast<const t_structured*>(type)) {
      return check_structured(structured_type, value);
    } else if (auto map_type = dynamic_cast<const t_map*>(type)) {
      return check_map(map_type, value);
    } else if (auto list_type = dynamic_cast<const t_list*>(type)) {
      return check_list(list_type, value);
    } else if (auto set_type = dynamic_cast<const t_set*>(type)) {
      return check_set(set_type, value);
    } else {
      throw std::logic_error("compatibility_checker: unknown type");
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
    const std::string& type_name =
        type->is<t_container>() ? type->get_full_name() : type->name();
    error(
        "cannot convert {} to `{}` in initialization of `{}`",
        category,
        type_name,
        name_);
  }

  template <typename T>
  bool check_int(const t_const_value* value, const t_type* type) {
    if (value->kind() != t_const_value::CV_INTEGER) {
      report_incompatible(value, type);
      return false;
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
      return false;
    }
    return true;
  }

  template <typename T>
  bool check_float(const t_const_value* value, const t_type* type) {
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
        return false;
      }
    } else if (value->kind() == t_const_value::CV_INTEGER) {
      int64_t int_value = value->get_integer();
      if (int_value != static_cast<int64_t>(static_cast<T>(int_value))) {
        error(
            "cannot convert {} to `{}` in initialization of `{}`",
            int_value,
            type->name(),
            name_);
        return false;
      }
    } else {
      report_incompatible(value, type);
      return false;
    }
    return true;
  }

  bool check_primitive_type(
      const t_primitive_type* type, const t_const_value* value) {
    const auto kind = value->kind();
    switch (type->primitive_type()) {
      case t_primitive_type::type::t_void:
        return true;
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        if (kind != t_const_value::CV_STRING) {
          report_incompatible(value, type);
          return false;
        }
        return true;
      case t_primitive_type::type::t_bool:
        if (kind != t_const_value::CV_BOOL &&
            kind != t_const_value::CV_INTEGER) {
          report_incompatible(value, type);
          return false;
        }
        return true;
      case t_primitive_type::type::t_byte:
        return check_int<int8_t>(value, type);
      case t_primitive_type::type::t_i16:
        return check_int<int16_t>(value, type);
      case t_primitive_type::type::t_i32:
        return check_int<int32_t>(value, type);
      case t_primitive_type::type::t_i64:
        return check_int<int64_t>(value, type);
      case t_primitive_type::type::t_double:
        return check_float<double>(value, type);
      case t_primitive_type::type::t_float:
        return check_float<float>(value, type);
    }
    abort();
  }

  bool check_enum(const t_enum* type, const t_const_value* value) {
    if (value->kind() != t_const_value::CV_INTEGER) {
      report_incompatible(value, type);
      return false;
    }

    if (type->find_value(value->get_integer()) == nullptr) {
      warning(
          "const `{}` is defined as enum `{}` with a value not of that enum",
          name_,
          type->name());
    }
    if (value->is_enum()) {
      const t_enum* val_type = value->get_enum();
      if (val_type != nullptr && val_type != type) {
        error(
            "const `{}` is defined as enum `{}` with a value of another enum `{}`",
            name_,
            type->name(),
            val_type->name());
      }
    }
    return true;
  }

  bool check_structured(const t_structured* type, const t_const_value* value) {
    if (value->kind() != t_const_value::CV_MAP) {
      report_incompatible(value, type);
      return false;
    }
    const auto& map = value->get_map();
    if (map.size() > 1 && type->is<t_union>()) {
      error(
          "cannot initialize more than one field in union `{}`", type->name());
      return false;
    }
    return check_fields(type, map);
  }

  bool check_fields(
      const t_structured* type,
      const std::vector<std::pair<t_const_value*, t_const_value*>>& map) {
    bool is_valid = true;
    for (auto [key, value] : map) {
      std::string field_name;
      if (key->kind() == t_const_value::CV_STRING) {
        field_name = key->get_string();
      } else if (key->kind() == t_const_value::CV_IDENTIFIER) {
        field_name = key->get_identifier();
      } else {
        error("{} field name must be a string or an identifier", name_);
        is_valid = false;
        continue;
      }
      const auto* field = type->get_field_by_name(field_name);
      if (!field) {
        error("no field named `{}` in `{}`", key->get_string(), type->name());
        is_valid = false;
        continue;
      }
      const t_type* field_type = &field->type().deref();
      std::string name =
          name_.empty() ? field_name : fmt::format("{}.{}", name_, field_name);
      is_valid &= compatibility_checker(diags_, node_, std::move(name))
                      .check(field_type, value);
    }
    return is_valid;
  }

  bool check_map(const t_map* type, const t_const_value* value) {
    if (value->kind() == t_const_value::CV_LIST && value->get_list().empty()) {
      warning(
          "converting empty list to `map` in initialization of `{}`", name_);
      return true;
    }
    if (value->kind() != t_const_value::CV_MAP) {
      report_incompatible(value, type);
      return false;
    }

    bool is_valid = true;
    const t_type* key_type = &type->key_type().deref();
    const t_type* val_type = &type->val_type().deref();
    for (const auto& entry : value->get_map()) {
      is_valid &= check(key_type, entry.first);
      is_valid &= check(val_type, entry.second);
    }
    return is_valid;
  }

  bool check_list(const t_list* type, const t_const_value* value) {
    if (value->kind() == t_const_value::CV_MAP && value->get_map().empty()) {
      warning(
          "converting empty map to `list` in initialization of `{}`", name_);
      return true;
    }
    if (value->kind() != t_const_value::CV_LIST) {
      report_incompatible(value, type);
      return false;
    }
    return check_elements(&type->elem_type().deref(), value->get_list());
  }

  bool check_set(const t_set* type, const t_const_value* value) {
    if (value->kind() == t_const_value::CV_MAP && value->get_map().empty()) {
      warning("converting empty map to `set` in initialization of `{}`", name_);
      return true;
    }
    if (value->kind() != t_const_value::CV_LIST) {
      report_incompatible(value, type);
      return false;
    }
    return check_elements(&type->elem_type().deref(), value->get_list());
  }

  bool check_elements(
      const t_type* elem_type, const std::vector<t_const_value*>& list) {
    bool is_valid = true;
    for (const auto& elem : list) {
      is_valid &= check(elem_type, elem);
    }
    return is_valid;
  }
};

class default_value_checker final {
 public:
  explicit default_value_checker(const t_type& type)
      : type_(*type.get_true_type()) {}

  /**
   * Returns true if the given `value` is known with certainty to be identical
   * to the default value of this checker's `type_`.
   *
   * Note that this implies a possible "false negative", i.e. case where this
   * method returns false when actually the given `value` would be equal to the
   * default, but that could not be determined either because the (more complex)
   * checking logic has not been implemented yet, or it relies on information
   * that may not be taken into account yet (eg. custom default values of
   * nested struct fields, etc.)
   */
  bool is_default_value(const t_const_value& value) {
    if (const auto* primitive_type =
            dynamic_cast<const t_primitive_type*>(&type_)) {
      return check_primitive_type(*primitive_type, value);
    } else if (type_.is<t_enum>()) {
      return check_enum(value);
    } else if (type_.is<t_map>()) {
      return check_map(value);
    } else if (type_.is<t_list>() || type_.is<t_set>()) {
      return check_list_or_set(value);
    } else if (
        const auto* structured_type =
            dynamic_cast<const t_structured*>(&type_)) {
      // Obvious case: empty initializer
      // If the initializer value is an empty map (i.e., {}), then by definition
      // it will produce the default value for any structured type (struct,
      // exception or union).
      if (value.get_map().empty()) {
        return true;
      }

      // If this is a union (and, per previous test, the initializer is not
      // empty), then it cannot be the default value, which is always empty for
      // unions.
      if (structured_type->is<t_union>()) {
        return false;
      }

      // Non-obvious cases: for non-empty maps, the given value could
      // theoretically specify the same values for fields as the default value,
      // making it effectively equal to the default value. We do not (yet) go
      // into that level of checks here.

      // DO_BEFORE(aristidis,20250124): Figure out recurive test of default
      // initializer values for structs and exceptions.
      return false;
    } else {
      throw std::logic_error(
          fmt::format(
              "[default_value_checker] unsupported type: {}",
              type_.get_full_name()));
    }
  }

 private:
  const t_type& type_;

  static bool check_primitive_type(
      const t_primitive_type& type, const t_const_value& value) {
    switch (type.primitive_type()) {
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return value.get_string().empty();
      case t_primitive_type::type::t_bool:
        // Booleans can be initialized with integer values.
        return (value.kind() == t_const_value::CV_INTEGER)
            ? value.get_integer() == 0
            : value.get_bool() == false;
      case t_primitive_type::type::t_byte:
      case t_primitive_type::type::t_i16:
      case t_primitive_type::type::t_i32:
      case t_primitive_type::type::t_i64:
        return value.get_integer() == 0;
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float: {
        // Floating point numbers can be initialized with integer values.
        const double value_as_double =
            (value.kind() == t_const_value::CV_INTEGER) ? value.get_integer()
                                                        : value.get_double();
        return value_as_double == 0.0;
      }
      case t_primitive_type::type::t_void:
        // Should never be called for void
        throw std::logic_error("t_void does not have a default value.");
    }
    abort();
  }

  static bool check_enum(const t_const_value& value) {
    return value.get_integer() == 0;
  }

  static bool check_map(const t_const_value& value) {
    // Maps can be initialized with empty list values.
    if (value.kind() == t_const_value::CV_LIST && value.get_list().empty()) {
      return true;
    }

    return value.get_map().empty();
  }

  bool check_list_or_set(const t_const_value& value) {
    // Lists and sets can be initialized with empty map values.
    if (value.kind() == t_const_value::CV_MAP && value.get_map().empty()) {
      return true;
    }
    return value.get_list().empty();
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
bool detail::check_initializer(
    diagnostics_engine& diags,
    const t_named& node,
    const t_type* type,
    const t_const_value* initializer) {
  return compatibility_checker(diags, node, node.name())
      .check(type, initializer);
}

bool detail::is_initializer_default_value(
    const t_type& type, const t_const_value& initializer) {
  return default_value_checker(type).is_default_value(initializer);
}

} // namespace apache::thrift::compiler
