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

#include <algorithm>
#include <cassert>
#include <stack>
#include <stdexcept>
#include <unordered_set>

#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {

namespace {

template <typename C>
// Returns a pair<iterator, bool> of (lower bound iterator, id found?).
auto find_by_id(const C& fields_id_order, t_field_id id) {
  auto lower = std::partition_point(
      fields_id_order.begin(), fields_id_order.end(), [id](const auto& field) {
        return field->id() < id;
      });
  return std::make_pair(
      lower, lower != fields_id_order.end() && (*lower)->id() == id);
}

struct sealedness_check_context {
  std::unordered_set<const t_structured*> active;
  std::stack<const t_structured*> active_stack;
};

class scoped_sealedness_check {
 public:
  scoped_sealedness_check(
      sealedness_check_context& context, const t_structured& node)
      : context_(context) {
    context_.active.insert(&node);
    context_.active_stack.push(&node);
  }

  scoped_sealedness_check(const scoped_sealedness_check&) = delete;
  scoped_sealedness_check& operator=(const scoped_sealedness_check&) = delete;
  scoped_sealedness_check(scoped_sealedness_check&&) = delete;
  scoped_sealedness_check& operator=(scoped_sealedness_check&&) = delete;

  ~scoped_sealedness_check() noexcept {
    assert(!context_.active_stack.empty());
    const t_structured* node = context_.active_stack.top();
    context_.active_stack.pop();
    context_.active.erase(node);
  }

 private:
  sealedness_check_context& context_;
};

bool is_sealed_impl(const t_type& type, sealedness_check_context& context);

bool is_sealed_impl(
    const t_structured& structured, sealedness_check_context& context) {
  // In Thrift IDL, a structured definition (struct, union, exception) is sealed
  // iff:
  //   1. It is annotated with `@thrift.Sealed`, and
  //   2. All of its fields are sealed

  if (!structured.has_structured_annotation(kSealedUri)) {
    return false;
  }

  if (context.active.contains(&structured)) {
    return true;
  }
  scoped_sealedness_check check(context, structured);

  for (const t_field& field : structured.fields()) {
    if (!is_sealed_impl(field.type().deref(), context)) {
      return false;
    }
  }

  return true;
}

bool is_sealed_impl(const t_type& type, sealedness_check_context& context) {
  if (const auto* structured = type.try_as<t_structured>()) {
    return is_sealed_impl(*structured, context);
  }
  if (const auto* list = type.try_as<t_list>()) {
    return is_sealed_impl(list->elem_type().deref(), context);
  }
  if (const auto* set = type.try_as<t_set>()) {
    return is_sealed_impl(set->elem_type().deref(), context);
  }
  if (const auto* map = type.try_as<t_map>()) {
    return is_sealed_impl(map->key_type().deref(), context) &&
        is_sealed_impl(map->val_type().deref(), context);
  }
  if (const auto* type_alias = type.try_as<t_typedef>()) {
    return is_sealed_impl(type_alias->type().deref(), context);
  }
  return type.is_sealed();
}

} // namespace

bool t_structured::try_append_field(std::unique_ptr<t_field>& field) {
  auto existing = find_by_id(fields_id_order_, field->id());
  if (existing.second) {
    return false;
  }

  if (!field->name().empty()) {
    fields_by_name_[field->name()] = field.get();
  }
  fields_id_order_.emplace(existing.first, field.get());

  // Take ownership.
  fields_.push_back(std::move(field));
  return true;
}

void t_structured::append_field(std::unique_ptr<t_field> field) {
  if (!try_append_field(field)) {
    throw std::runtime_error(
        "Field identifier " + std::to_string(field->id()) + " for \"" +
        field->name() + "\" has already been used");
  }
}

const t_field* t_structured::get_field_by_id(t_field_id id) const {
  auto existing = find_by_id(fields_id_order_, id);
  return existing.second ? *existing.first : nullptr;
}

bool t_structured::is_sealed() const {
  sealedness_check_context context;
  return is_sealed_impl(*this, context);
}

t_structured::~t_structured() = default;

} // namespace apache::thrift::compiler
