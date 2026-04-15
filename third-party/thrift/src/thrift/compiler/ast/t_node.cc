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

#include <thrift/compiler/ast/t_node.h>

#include <cassert>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {

namespace {

source_range get_duva_item_range(
    const t_const_value& key, const t_const_value& value) {
  if (key.src_range().has_value()) {
    return value.src_range().has_value()
        ? source_range{key.src_range()->begin, value.src_range()->end}
        : key.src_range().value();
  }
  return {};
}

const t_map& get_deprecated_unvalidated_annotations_items_type() {
  static const t_map* type = [] {
    return new t_map(
        t_primitive_type::t_string(), t_primitive_type::t_string());
  }();
  return *type;
}

const t_struct& get_synthetic_deprecated_unvalidated_annotations_type() {
  static const t_struct* type = [] {
    auto* result = new t_struct(nullptr, "DeprecatedUnvalidatedAnnotations");
    result->set_uri(kDeprecatedUnvalidatedAnnotationsUri);
    result->create_field(
        get_deprecated_unvalidated_annotations_items_type(), "items", 1);
    return result;
  }();
  return *type;
}

const t_struct& get_deprecated_unvalidated_annotations_type(
    const t_named& node) {
  if (const auto* program = node.program(); program != nullptr) {
    if (const auto* definition = program->global_scope()->find_by_uri(
            kDeprecatedUnvalidatedAnnotationsUri)) {
      if (const auto* annot_type = dynamic_cast<const t_struct*>(definition)) {
        return *annot_type;
      }
    }
  }
  return get_synthetic_deprecated_unvalidated_annotations_type();
}

t_const_value* get_duva_items_or_null(t_const& annot) {
  return const_cast<t_const_value*>(
      annot.get_value_from_structured_annotation_or_null("items"));
}

const t_const_value* get_duva_items_or_null(const t_named& node) {
  const auto* annot = node.find_structured_annotation_or_null(
      kDeprecatedUnvalidatedAnnotationsUri);
  if (annot == nullptr) {
    return nullptr;
  }
  return annot->get_value_from_structured_annotation_or_null("items");
}

std::unique_ptr<t_const_value> make_string_value(
    std::string value, source_range range) {
  auto result = std::make_unique<t_const_value>(std::move(value));
  if (range.begin != source_location{} || range.end != source_location{}) {
    result->set_src_range(range);
  }
  return result;
}

void build_deprecated_unvalidated_annotations(
    const t_named& node, deprecated_annotation_map& annotations) {
  const auto* items = get_duva_items_or_null(node);
  if (items == nullptr) {
    return;
  }
  for (const auto& [key, value] : items->get_map()) {
    annotations.emplace(
        key->get_string(),
        deprecated_annotation_value{
            get_duva_item_range(*key, *value), value->get_string()});
  }
}

const t_const_value* find_duva_annotation_value_or_null(
    const t_named& node, std::string_view name) {
  const auto* items = get_duva_items_or_null(node);
  if (items == nullptr) {
    return nullptr;
  }
  for (const auto& [key, value] : items->get_map()) {
    if (key->get_string() == name) {
      return value;
    }
  }
  return nullptr;
}

t_const& get_or_create_duva_annotation(t_named& node) {
  for (t_const& annot : node.structured_annotations()) {
    if (annot.type()->uri() == kDeprecatedUnvalidatedAnnotationsUri) {
      return annot;
    }
  }
  auto value = t_const_value::make_map();
  value->add_map(make_string_value("items", {}), t_const_value::make_map());
  node.add_structured_annotation(
      std::make_unique<t_const>(
          node.program(),
          get_deprecated_unvalidated_annotations_type(node),
          "",
          std::move(value)));
  return node.structured_annotations().back();
}

} // namespace

const std::string& t_node::emptyString() {
  static const std::string empty;
  return empty;
}

deprecated_annotation_map t_node::unstructured_annotations() const {
  if (const auto* named = dynamic_cast<const t_named*>(this);
      named != nullptr) {
    deprecated_annotation_map annotations;
    build_deprecated_unvalidated_annotations(*named, annotations);
    return annotations;
  }
  return {};
}

const std::string* t_node::find_unstructured_annotation_or_null(
    const std::vector<std::string_view>& names) const {
  const auto* named = dynamic_cast<const t_named*>(this);
  if (named == nullptr) {
    return nullptr;
  }
  for (std::string_view name : names) {
    if (const auto* value = find_duva_annotation_value_or_null(*named, name)) {
      return &value->get_string();
    }
  }
  return nullptr;
}

void t_node::set_unstructured_annotation(
    const std::string& key,
    const std::string& value,
    const source_range& range) {
  auto* named = dynamic_cast<t_named*>(this);
  assert(named != nullptr);
  t_const& annot = get_or_create_duva_annotation(*named);
  t_const_value* items = get_duva_items_or_null(annot);
  assert(items != nullptr);

  auto entries = items->get_map();
  auto replacement = t_const_value::make_map();
  bool updated = false;
  for (const auto& [entry_key, entry_value] : entries) {
    if (entry_key->get_string() == key) {
      replacement->add_map(
          make_string_value(key, range), make_string_value(value, range));
      updated = true;
    } else {
      auto cloned_key = entry_key->clone();
      if (entry_key->src_range().has_value()) {
        cloned_key->set_src_range(*entry_key->src_range());
      }
      auto cloned_value = entry_value->clone();
      if (entry_value->src_range().has_value()) {
        cloned_value->set_src_range(*entry_value->src_range());
      }
      replacement->add_map(std::move(cloned_key), std::move(cloned_value));
    }
  }
  if (!updated) {
    replacement->add_map(
        make_string_value(key, range), make_string_value(value, range));
  }
  items->assign(std::move(*replacement));
}

} // namespace apache::thrift::compiler
