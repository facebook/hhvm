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

#include <thrift/compiler/parse/parser.h>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/parse/parser_core.h>

namespace apache::thrift::compiler {

parser_actions::~parser_actions() = default;

parser_actions::attributes_type parser_actions::on_attributes(
    source_location loc,
    std::optional<comment_type> doc,
    structured_annotation_list_type annotations) {
  return doc || !annotations.empty()
      ? std::make_unique<attributes>(
            attributes{loc, std::move(doc), std::move(annotations), {}})
      : nullptr;
}

parser_actions::deprecated_annotations_type
parser_actions::on_deprecated_annotations(source_range) {
  return {};
}

void parser_actions::set_deprecated_annotations(
    attributes_type& attrs, deprecated_annotations_type annotations) {
  if (!attrs) {
    attrs = std::make_unique<attributes>();
  }
  attrs->deprecated_annotations = std::move(annotations);
}

void parser_actions::set_const_value_src_range(
    const_value_type& value, source_range range) const {
  value->set_src_range(range);
}

void parser_actions::add_list_value(
    const_value_type& list, const_value_type value) const {
  list->add_list(std::move(value));
}

void parser_actions::add_map_value(
    const_value_type& map, const_value_type key, const_value_type value) const {
  map->add_map(std::move(key), std::move(value));
}

t_type_ref parser_actions::on_invalid_type(
    source_range range, const t_primitive_type& type) {
  return on_type(range, type);
}

bool parse(lexer& lex, parser_actions& actions, diagnostics_engine& diags) {
  return detail::parser_core<parser_actions>(lex, actions, diags).parse();
}

bool parse(
    lexer& lex,
    parser_actions& actions,
    diagnostics_engine& diags,
    parser_token_sink& token_sink) {
  return detail::parser_core<parser_actions>(lex, actions, diags, &token_sink)
      .parse();
}

} // namespace apache::thrift::compiler
