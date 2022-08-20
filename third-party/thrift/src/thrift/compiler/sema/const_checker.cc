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

#include <thrift/compiler/sema/const_checker.h>

#include <cassert>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_union.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace detail {

bool is_valid_custom_default_integer(
    const t_base_type* type, const t_const_value* value) {
  int64_t min = 0, max = 0;
  if (type->is_byte()) {
    min = std::numeric_limits<int8_t>::min();
    max = std::numeric_limits<int8_t>::max();
  } else if (type->is_i16()) {
    min = std::numeric_limits<int16_t>::min();
    max = std::numeric_limits<int16_t>::max();
  } else if (type->is_i32()) {
    min = std::numeric_limits<int32_t>::min();
    max = std::numeric_limits<int32_t>::max();
  } else {
    assert(false); // Should be unreachable.
  }
  return min <= value->get_integer() && value->get_integer() <= max;
}

bool is_valid_custom_default_float(const t_const_value* value) {
  return std::numeric_limits<float>::lowest() <= value->get_double() &&
      value->get_double() <= std::numeric_limits<float>::max();
}

} // namespace detail

namespace {
class const_checker {
 public:
  const_checker(
      diagnostics_engine& diags, const t_named& node, std::string name)
      : diags_(diags), node_(node), name_(std::move(name)) {}

  void check(const t_type* type, const t_const_value* value) {
    type = type->get_true_type();

    if (const auto concrete = dynamic_cast<const t_base_type*>(type)) {
      check_base_type(concrete, value);
      check_base_value(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_enum*>(type)) {
      check_enum(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_union*>(type)) {
      check_union(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_exception*>(type)) {
      check_exception(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_struct*>(type)) {
      check_struct(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_map*>(type)) {
      check_map(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_list*>(type)) {
      check_list(concrete, value);
    } else if (const auto concrete = dynamic_cast<const t_set*>(type)) {
      check_set(concrete, value);
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

  void report_value_precision() {
    error(
        "value error: const `{}` cannot be represented precisely as `float` "
        "or `double`.",
        name_);
  }

  void report_value_mistmatch() {
    error(
        "value error: const `{}` has an invalid custom default value.", name_);
  }

  void report_type_mismatch(const char* expected) {
    error("type error: const `{}` was declared as {}.", name_, expected);
  }

  void report_type_mismatch_warning(const char* expected) {
    warning(
        "type error: const `{}` was declared as {}. This will become an error "
        "in future versions of thrift.",
        name_,
        expected);
  }

  // For CV_INTEGER, an overflow of int64_t is checked in the parser;
  // therefore, we don't need to check an overflow of i64 or a floating point
  // stored in integer value. Similarly, for CV_DOUBLE, we do not need
  // to check double. However, we need to check a floating point stored
  // with CV_INTEGER that might lead to a precision loss when converting int64_t
  // to a floating point.
  void check_base_value(const t_base_type* type, const t_const_value* value) {
    switch (type->base_type()) {
      case t_base_type::type::t_void:
      case t_base_type::type::t_string:
      case t_base_type::type::t_binary:
      case t_base_type::type::t_bool:
      case t_base_type::type::t_i64:
        break;
      case t_base_type::type::t_byte:
      case t_base_type::type::t_i16:
      case t_base_type::type::t_i32:
        if (value->get_type() == t_const_value::CV_INTEGER &&
            !detail::is_valid_custom_default_integer(type, value)) {
          report_value_mistmatch();
        }
        break;
      case t_base_type::type::t_float:
        if (value->get_type() == t_const_value::CV_DOUBLE &&
            !detail::is_valid_custom_default_float(value)) {
          report_value_mistmatch();
        }
        if (value->get_type() == t_const_value::CV_INTEGER &&
            !detail::is_valid_custom_default_float_with_integer_value<float>(
                value)) {
          report_value_precision();
        }
        break;
      case t_base_type::type::t_double:
        if (value->get_type() == t_const_value::CV_INTEGER &&
            !detail::is_valid_custom_default_float_with_integer_value<double>(
                value)) {
          report_value_precision();
        }
        break;
      default:
        assert(false); // Should be unreachable.
    }
  }

  void check_base_type(const t_base_type* type, const t_const_value* value) {
    switch (type->base_type()) {
      case t_base_type::type::t_void:
        error("type error: cannot declare a void const: {}", name_);
        break;
      case t_base_type::type::t_string:
      case t_base_type::type::t_binary:
        if (value->get_type() != t_const_value::CV_STRING) {
          report_type_mismatch("string");
        }
        break;
      case t_base_type::type::t_bool:
        if (value->get_type() != t_const_value::CV_BOOL &&
            value->get_type() != t_const_value::CV_INTEGER) {
          report_type_mismatch("bool");
        }
        break;
      case t_base_type::type::t_byte:
        if (value->get_type() != t_const_value::CV_INTEGER) {
          report_type_mismatch("byte");
        }
        break;
      case t_base_type::type::t_i16:
        if (value->get_type() != t_const_value::CV_INTEGER) {
          report_type_mismatch("i16");
        }
        break;
      case t_base_type::type::t_i32:
        if (value->get_type() != t_const_value::CV_INTEGER) {
          report_type_mismatch("i32");
        }
        break;
      case t_base_type::type::t_i64:
        if (value->get_type() != t_const_value::CV_INTEGER) {
          report_type_mismatch("i64");
        }
        break;
      case t_base_type::type::t_double:
      case t_base_type::type::t_float:
        if (value->get_type() != t_const_value::CV_INTEGER &&
            value->get_type() != t_const_value::CV_DOUBLE) {
          report_type_mismatch("double");
        }
        break;
      default:
        assert(false); // Should be unreachable.
    }
  }

  void check_enum(const t_enum* type, const t_const_value* value) {
    if (value->get_type() != t_const_value::CV_INTEGER) {
      report_type_mismatch("enum");
    }

    if (type->find_value(value->get_integer()) == nullptr) {
      warning(
          "type error: const `{}` was declared as enum `{}` with a value not "
          "of that enum.",
          name_,
          type->name());
    }
  }

  void check_union(const t_union* type, const t_const_value* value) {
    if (value->get_type() != t_const_value::CV_MAP) {
      report_type_mismatch("union");
    }
    const auto& map = value->get_map();
    if (map.size() > 1) {
      error(
          "type error: const `{}` is a union and can't have more than one "
          "field set.",
          name_);
    }
    check_fields(type, map);
  }

  void check_struct(const t_struct* type, const t_const_value* value) {
    if (value->get_type() != t_const_value::CV_MAP) {
      report_type_mismatch_warning("struct");
    }
    check_fields(type, value->get_map());
  }

  void check_exception(const t_exception* type, const t_const_value* value) {
    if (value->get_type() != t_const_value::CV_MAP) {
      report_type_mismatch_warning("exception");
    }
    check_fields(type, value->get_map());
  }

  void check_fields(
      const t_structured* type,
      const std::vector<std::pair<t_const_value*, t_const_value*>>& map) {
    for (const auto& entry : map) {
      if (entry.first->get_type() != t_const_value::CV_STRING) {
        error("type error: `{}` field name must be string.", name_);
      }
      const auto* field = type->get_field_by_name(entry.first->get_string());
      if (field == nullptr) {
        error(
            "type error: `{}` has no field `{}`.",
            type->name(),
            entry.first->get_string());
        continue;
      }
      const t_type* field_type = &field->type().deref();
      const_checker(diags_, node_, name_ + "." + entry.first->get_string())
          .check(field_type, entry.second);
    }
  }

  void check_map(const t_map* type, const t_const_value* value) {
    if (value->get_type() == t_const_value::CV_LIST &&
        value->get_list().empty()) {
      warning("type error: map `{}` initialized with empty list.", name_);
      return;
    }
    if (value->get_type() != t_const_value::CV_MAP) {
      report_type_mismatch_warning("map");
      return;
    }
    const t_type* k_type = &type->key_type().deref();
    const t_type* v_type = &type->val_type().deref();
    for (const auto& entry : value->get_map()) {
      const_checker(diags_, node_, name_ + "<key>").check(k_type, entry.first);
      const_checker(diags_, node_, name_ + "<val>").check(v_type, entry.second);
    }
  }

  void check_list(const t_list* type, const t_const_value* value) {
    if (value->get_type() == t_const_value::CV_MAP &&
        value->get_map().empty()) {
      warning("type error: list `{}` initialized with empty map.", name_);
      return;
    }
    if (value->get_type() != t_const_value::CV_LIST) {
      report_type_mismatch_warning("list");
      return;
    }
    check_elements(&type->elem_type().deref(), value->get_list());
  }

  void check_set(const t_set* type, const t_const_value* value) {
    if (value->get_type() == t_const_value::CV_MAP &&
        value->get_map().empty()) {
      warning("type error: set `{}` initialized with empty map.", name_);
      return;
    }
    if (value->get_type() != t_const_value::CV_LIST) {
      report_type_mismatch_warning("set");
      return;
    }
    check_elements(&type->elem_type().deref(), value->get_list());
  }

  void check_elements(
      const t_type* elem_type, const std::vector<t_const_value*>& list) {
    for (const auto& elem : list) {
      const_checker(diags_, node_, name_ + "<elem>").check(elem_type, elem);
    }
  }
};
} // namespace

void check_const_rec(
    diagnostic_context& ctx,
    const t_named& node,
    const t_type* type,
    const t_const_value* value) {
  const_checker(ctx, node, node.name()).check(type, value);
}

} // namespace compiler
} // namespace thrift
} // namespace apache
