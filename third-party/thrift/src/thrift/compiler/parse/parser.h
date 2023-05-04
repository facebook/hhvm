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
#include <string>

#include <boost/optional.hpp>

#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

class diagnostics_engine;
class lexer;

class t_base_type;
class t_interaction;
class t_service;
class t_throws;
class t_typedef;
class t_union;

struct identifier {
  fmt::string_view str;
  source_location loc;
};

struct comment {
  std::string text;
  source_location loc;
};

struct deprecated_annotations {
  std::map<std::string, annotation_value> strings;
  source_location loc;
};

struct attributes {
  boost::optional<comment> doc;
  node_list<t_const> annotations;
  std::unique_ptr<struct deprecated_annotations> deprecated_annotations;
};

struct type_throws_spec {
  t_type_ref type;
  std::unique_ptr<t_throws> throws;
};

enum class sign { plus, minus };

// An interface that receives notifications of parsed syntactic constructs.
class parser_actions {
 public:
  virtual ~parser_actions() = 0;

  virtual void on_program() = 0;

  virtual void on_standard_header(
      source_location loc, std::unique_ptr<attributes> attrs) = 0;

  virtual void on_package(
      source_range range,
      std::unique_ptr<attributes> attrs,
      fmt::string_view name) = 0;
  virtual void on_include(source_range range, fmt::string_view str) = 0;
  virtual void on_cpp_include(source_range range, fmt::string_view str) = 0;
  virtual void on_hs_include(source_range range, fmt::string_view str) = 0;

  virtual void on_namespace(
      const identifier& language, fmt::string_view ns) = 0;

  virtual boost::optional<comment> on_doctext() = 0;
  virtual void on_program_doctext() = 0;
  virtual comment on_inline_doc(source_location loc, fmt::string_view text) = 0;

  virtual std::unique_ptr<t_const> on_structured_annotation(
      source_range range, fmt::string_view name) = 0;
  virtual std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::unique_ptr<t_const_value> value) = 0;

  virtual void on_service(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      const identifier& base,
      std::unique_ptr<t_function_list> functions) = 0;

  virtual void on_interaction(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      std::unique_ptr<t_function_list> functions) = 0;

  virtual std::unique_ptr<t_function> on_function(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_function_qualifier qual,
      std::vector<t_type_ref> return_type,
      const identifier& name,
      t_field_list params,
      std::unique_ptr<t_throws> throws) = 0;

  virtual t_type_ref on_stream_return_type(
      source_range range, type_throws_spec spec) = 0;
  virtual t_type_ref on_sink_return_type(
      source_range range,
      type_throws_spec sink_spec,
      type_throws_spec final_response_spec) = 0;

  virtual t_type_ref on_list_type(
      source_range range,
      t_type_ref element_type,
      std::unique_ptr<deprecated_annotations> annotations) = 0;
  virtual t_type_ref on_set_type(
      source_range range,
      t_type_ref key_type,
      std::unique_ptr<deprecated_annotations> annotations) = 0;
  virtual t_type_ref on_map_type(
      source_range range,
      t_type_ref key_type,
      t_type_ref value_type,
      std::unique_ptr<deprecated_annotations> annotations) = 0;

  virtual std::unique_ptr<t_function> on_performs(
      source_range range, t_type_ref type) = 0;

  virtual std::unique_ptr<t_throws> on_throws(t_field_list exceptions) = 0;

  virtual void on_typedef(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_type_ref type,
      const identifier& name) = 0;

  virtual void on_struct(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      t_field_list fields) = 0;

  virtual void on_union(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      t_field_list fields) = 0;

  virtual void on_exception(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_error_safety safety,
      t_error_kind kind,
      t_error_blame blame,
      const identifier& name,
      t_field_list fields) = 0;

  virtual std::unique_ptr<t_field> on_field(
      source_range range,
      std::unique_ptr<attributes> attrs,
      boost::optional<int64_t> id,
      t_field_qualifier qual,
      t_type_ref type,
      const identifier& name,
      std::unique_ptr<t_const_value> value,
      boost::optional<comment> doc) = 0;

  virtual t_type_ref on_type(
      const t_base_type& type,
      std::unique_ptr<deprecated_annotations> annotations) = 0;

  virtual t_type_ref on_type(
      source_range range,
      fmt::string_view name,
      std::unique_ptr<deprecated_annotations> annotations) = 0;

  virtual void on_enum(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      t_enum_value_list values) = 0;

  virtual std::unique_ptr<t_enum_value> on_enum_value(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      boost::optional<int64_t> value,
      boost::optional<comment> doc) = 0;

  virtual void on_const(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_type_ref type,
      const identifier& name,
      std::unique_ptr<t_const_value> value) = 0;

  virtual std::unique_ptr<t_const_value> on_const_ref(
      const identifier& name) = 0;

  virtual std::unique_ptr<t_const_value> on_integer(
      source_location loc, int64_t value) = 0;
  virtual std::unique_ptr<t_const_value> on_float(double value) = 0;
  virtual std::unique_ptr<t_const_value> on_string_literal(
      std::string value) = 0;
  virtual std::unique_ptr<t_const_value> on_bool_literal(bool value) = 0;
  virtual std::unique_ptr<t_const_value> on_list_literal() = 0;
  virtual std::unique_ptr<t_const_value> on_map_literal() = 0;
  virtual std::unique_ptr<t_const_value> on_struct_literal(
      source_range range, fmt::string_view name) = 0;

  virtual int64_t on_integer(source_range range, sign s, uint64_t value) = 0;

  [[noreturn]] virtual void on_error() = 0;
};

// Parses a Thrift source from the lexer, invokes parser actions on
// syntactic constructs and reports parse errors if any via diags.
bool parse(lexer& lex, parser_actions& actions, diagnostics_engine& diags);

} // namespace compiler
} // namespace thrift
} // namespace apache
