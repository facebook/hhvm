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

#include <set>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <thrift/compiler/detail/overload.h>
#include <thrift/compiler/diagnostic.h>

namespace apache::thrift::compiler {
namespace {

// When a mapping (struct or map) has duplicate keys, and they have conflicting
// values, then we report a compiler error.
static const diagnostic_level kMismatchLevel = diagnostic_level::error;
// When a mapping has duplicate keys, and they have the same values, or if a set
// has duplicate keys, then we report a compiler warning.
static const diagnostic_level kDuplicateLevel = diagnostic_level::warning;

using const_value_kv = std::pair<t_const_value*, t_const_value*>;

std::string to_string(const t_const_value* val);

std::string to_string(const const_value_kv& val) {
  return fmt::format("({}, {})", to_string(val.first), to_string(val.second));
}

template <typename T>
std::string to_string(const std::vector<T>& val) {
  if (val.empty()) {
    return "[]";
  } else if (val.size() == 1) {
    return fmt::format("[{}]", to_string(val.front()));
  } else {
    return fmt::format(
        "[{}, ..., {}]", to_string(val.front()), to_string(val.back()));
  }
}

std::string to_string(const t_const_value* val) {
  switch (val->kind()) {
    case t_const_value::CV_BOOL:
      return val->get_bool() ? "true" : "false";
    case t_const_value::CV_INTEGER: {
      auto enum_val = val->get_enum_value();
      return enum_val ? enum_val->name() : std::to_string(val->get_integer());
    }
    case t_const_value::CV_DOUBLE:
      return fmt::format("{}", val->get_double());
    case t_const_value::CV_STRING:
      return val->get_string();
    case t_const_value::CV_IDENTIFIER:
      return val->get_identifier();
    case t_const_value::CV_LIST:
      return to_string(val->get_list());
    case t_const_value::CV_MAP:
      return to_string(val->get_map());
  }
  abort();
}

bool equal_value(const t_const_value* a, const t_const_value* b);

bool equal_value(
    const std::pair<const t_const_value*, const t_const_value*>& a,
    const std::pair<const t_const_value*, const t_const_value*>& b) {
  return equal_value(a.first, b.first) && equal_value(a.second, b.second);
}

template <typename T>
bool equal_value(const std::vector<T>& a, const std::vector<T>& b) {
  const size_t a_size = a.size();
  if (a_size != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a_size; ++i) {
    if (!equal_value(a[i], b[i])) {
      return false;
    }
  }
  return true;
}

bool equal_value(const t_const_value* a, const t_const_value* b) {
  const auto a_kind = a->kind();
  if (a_kind != b->kind()) {
    return false;
  }
  switch (a_kind) {
    case t_const_value::CV_BOOL:
      return (a->get_bool() == b->get_bool());
    case t_const_value::CV_INTEGER:
      return (a->get_integer() == b->get_integer());
    case t_const_value::CV_DOUBLE:
      return (a->get_double() == b->get_double());
    case t_const_value::CV_STRING:
      return (a->get_string() == b->get_string());
    case t_const_value::CV_IDENTIFIER:
      return (a->get_identifier() == b->get_identifier());
    case t_const_value::CV_LIST:
      return equal_value(a->get_list(), b->get_list());
    case t_const_value::CV_MAP:
      return equal_value(a->get_map(), b->get_map());
  }
  abort();
}

bool lt_value(const t_const_value* a, const t_const_value* b);

bool lt_value(const const_value_kv& a, const const_value_kv& b) {
  if (equal_value(a.first, b.first)) {
    return lt_value(a.second, b.second);
  }
  return lt_value(a.first, b.first);
}

template <typename T>
bool lt_value(const std::vector<T>& a, const std::vector<T>& b) {
  const size_t a_size = a.size();
  if (a_size != b.size()) {
    return a_size < b.size();
  }
  for (size_t i = 0; i < a_size; ++i) {
    if (!equal_value(a[i], b[i])) {
      return lt_value(a[i], b[i]);
    }
  }
  return false;
}

bool lt_value(const t_const_value* a, const t_const_value* b) {
  const auto a_kind = a->kind();
  if (a_kind != b->kind()) {
    return a_kind < b->kind();
  }
  switch (a_kind) {
    case t_const_value::CV_BOOL:
      return (a->get_bool() < b->get_bool());
    case t_const_value::CV_INTEGER:
      return (a->get_integer() < b->get_integer());
    case t_const_value::CV_DOUBLE:
      return (a->get_double() < b->get_double());
    case t_const_value::CV_STRING:
      return (a->get_string() < b->get_string());
    case t_const_value::CV_IDENTIFIER:
      return (a->get_identifier() < b->get_identifier());
    case t_const_value::CV_LIST:
      return lt_value(a->get_list(), b->get_list());
    case t_const_value::CV_MAP:
      return lt_value(a->get_map(), b->get_map());
  }
  abort();
}

struct const_value_comp {
  bool operator()(const t_const_value* a, const t_const_value* b) const {
    return lt_value(a, b);
  }
};

std::vector<std::pair<const t_const_value*, diagnostic_level>>
find_duplicate_keys(const std::vector<const_value_kv>& map_kvs) {
  std::vector<std::pair<const t_const_value*, diagnostic_level>> duplicates;
  std::map<const t_const_value*, const t_const_value*, const_value_comp> keys;
  for (const auto& kv : map_kvs) {
    auto it = keys.find(kv.first);
    if (it != keys.end()) {
      auto level =
          equal_value(kv.second, it->second) ? kDuplicateLevel : kMismatchLevel;
      duplicates.emplace_back(kv.first, level);
      continue;
    }
    keys.emplace(kv.first, kv.second);
  }
  return duplicates;
}

std::vector<std::pair<const t_const_value*, diagnostic_level>>
find_duplicate_keys(const std::vector<t_const_value*>& set_keys) {
  std::vector<std::pair<const t_const_value*, diagnostic_level>> duplicates;
  std::set<const t_const_value*, const_value_comp> keys;
  for (const auto& k : set_keys) {
    auto it = keys.find(k);
    if (it != keys.end()) {
      duplicates.emplace_back(k, kDuplicateLevel);
      continue;
    }
    keys.insert(k);
  }
  return duplicates;
}

// If node is a field, only have to check if owner is non-null.
// If node is a const, also have to check if owner is not the encolsing const.
bool is_named_const_value(const t_const_value* value, const t_node& node) {
  auto owner = value->get_owner();
  if (owner == nullptr) {
    return false;
  }
  // if node is a field, returns true b/c owner is non-null
  return owner != dynamic_cast<const t_const*>(&node);
}

void check_key_value(
    diagnostics_engine& diags, const t_node& node, const t_const_value* value) {
  auto report_duplicates = [&](const auto& duplicates) {
    // Don't recurse or check constant defined elsewhere.
    if (is_named_const_value(value, node)) {
      return;
    }
    for (const auto& [duplicate, level] : duplicates) {
      // If the t_const_value has a source range, use it; otherwise,
      // fallback to the source range of the enclosing const.
      const source_range& src_range = duplicate->src_range()
          ? *duplicate->src_range()
          : (value->src_range() ? *value->src_range() : node.src_range());
      // TODO(T213710219): Enable this with error severity
      diags.report(
          src_range.begin,
          level,
          "Duplicate key in {} literal: `{}`",
          value->kind() == t_const_value::CV_MAP ? "map" : "set",
          to_string(duplicate));
    }
  };

  // recurse on elements
  if (value->kind() == t_const_value::CV_LIST) {
    for (const t_const_value* elem : value->get_list()) {
      check_key_value(diags, node, elem);
    }
    if (value->ttype()->get_true_type()->is<t_set>()) {
      auto duplicates = find_duplicate_keys(value->get_list());
      report_duplicates(duplicates);
    }
  }
  if (value->kind() == t_const_value::CV_MAP) {
    auto duplicates = find_duplicate_keys(value->get_map());
    report_duplicates(duplicates);
    for (const auto& kv : value->get_map()) {
      check_key_value(diags, node, kv.first);
      check_key_value(diags, node, kv.second);
    }
  }
}

} // namespace
namespace detail {

void check_duplicate_keys(diagnostics_engine& diags, const t_const& const_) {
  check_key_value(diags, const_, const_.value());
}

void check_duplicate_keys(diagnostics_engine& diags, const t_field& field_) {
  if (field_.default_value() == nullptr) {
    return;
  }
  check_key_value(diags, field_, field_.default_value());
}

} // namespace detail
} // namespace apache::thrift::compiler
