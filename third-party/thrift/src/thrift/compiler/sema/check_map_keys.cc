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

std::string to_string(const t_const_value* val);

std::string to_string(
    const std::pair<const t_const_value*, const t_const_value*>& val) {
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
}

bool lt_value(const t_const_value* a, const t_const_value* b);

bool lt_value(
    const std::pair<const t_const_value*, const t_const_value*>& a,
    const std::pair<const t_const_value*, const t_const_value*>& b) {
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
}

struct const_value_comp {
  bool operator()(const t_const_value* a, const t_const_value* b) const {
    return lt_value(a, b);
  }
};

std::vector<const t_const_value*> find_duplicate_keys(
    const t_const_value* value) {
  std::vector<const t_const_value*> duplicates;
  std::set<const t_const_value*, const_value_comp> keys;
  for (const auto& kv : value->get_map()) {
    if (keys.count(kv.first) > 0) {
      duplicates.push_back(kv.first);
      continue;
    }
    keys.insert(kv.first);
  }
  return duplicates;
}

// If owner is null, it's a nested "anonymous" constant value.
// If owner is non-null and doesn't match enclosing t_const, it's
// defined elsewhere.
bool is_named_const_value(const t_const_value* value, const t_const& const_) {
  auto owner = value->get_owner();
  return owner != nullptr && owner != &const_;
}

void check_key_value(
    diagnostics_engine& diags,
    const t_const& const_,
    const t_const_value* value) {
  // recurse on elements
  if (value->kind() == t_const_value::CV_LIST) {
    for (const t_const_value* elem : value->get_list()) {
      check_key_value(diags, const_, elem);
    }
  }
  if (value->kind() == t_const_value::CV_MAP) {
    // Don't recurse or check constant defined elsewhere.
    if (is_named_const_value(value, const_)) {
      return;
    }
    auto duplicates = find_duplicate_keys(value);
    for (const auto& duplicate : duplicates) {
      // If the t_const_value has a source range, use it; otherwise,
      // fallback to the source range of the enclosing const.
      const source_range& src_range = duplicate->src_range()
          ? *duplicate->src_range()
          : (value->src_range() ? *value->src_range() : const_.src_range());
      // TODO(T213710219): Enable this with error severity
      diags.warning(
          src_range.begin,
          "Duplicate key in map literal: `{}`",
          to_string(duplicate));
    }
    for (const auto& kv : value->get_map()) {
      check_key_value(diags, const_, kv.first);
      check_key_value(diags, const_, kv.second);
    }
  }
}

} // namespace
namespace detail {

void check_map_keys(diagnostics_engine& diags, const t_const& const_) {
  check_key_value(diags, const_, const_.value());
}

} // namespace detail
} // namespace apache::thrift::compiler
