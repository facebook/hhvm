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
#include <utility>

#include <boost/optional.hpp>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/parse/parsing_driver.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

enum class sign { plus, minus };

struct type_throws_spec {
  t_type_ref type;
  std::unique_ptr<t_throws> throws;
};

using t_struct_annotations = node_list<t_const>;

class parser_actions {
 private:
  parsing_driver& driver_;

 public:
  explicit parser_actions(parsing_driver& d) : driver_(d) {}

  void on_program() { driver_.clear_doctext(); }

  void on_standard_header(
      std::unique_ptr<t_def_attrs> attrs,
      std::unique_ptr<t_annotations> annotations) {
    driver_.validate_header_location();
    driver_.validate_header_annotations(
        std::move(attrs), std::move(annotations));
  }

  void on_program_header(
      source_range range,
      std::unique_ptr<t_def_attrs> attrs,
      std::unique_ptr<t_annotations> annotations) {
    driver_.validate_header_location();
    driver_.set_program_annotations(
        std::move(attrs), std::move(annotations), range);
  }

  void on_package(source_range range, std::string literal) {
    driver_.set_package(std::move(literal), range);
  }

  void on_include(source_range range, std::string literal) {
    driver_.add_include(std::move(literal), range);
  }

  void on_cpp_include(source_range, std::string literal) {
    if (driver_.mode == parsing_mode::PROGRAM) {
      driver_.program->add_cpp_include(std::move(literal));
    }
  }

  void on_hs_include(source_range, std::string) {
    // Do nothing. This syntax is handled by the hs compiler.
  }

  void on_namespace(std::string language, std::string ns) {
    if (driver_.mode == parsing_mode::PROGRAM) {
      driver_.program->set_namespace(std::move(language), std::move(ns));
    }
  }

  void on_definition(
      source_range range,
      t_named& def,
      std::unique_ptr<t_def_attrs> attrs,
      std::unique_ptr<t_annotations> annotations) {
    driver_.set_parsed_definition();
    driver_.set_attributes(
        def, std::move(attrs), std::move(annotations), range);
  }

  boost::optional<doc> on_doctext() { return driver_.pop_doctext(); }

  void on_program_doctext() {
    // When there is any doctext, assign it to the top-level program.
    driver_.set_doctext(*driver_.program, driver_.pop_doctext());
  }

  doc on_inline_doc(source_location loc, std::string text) {
    return {driver_.strip_doctext(text), loc};
  }

  std::unique_ptr<t_def_attrs> on_statement_attrs(
      boost::optional<doc> doc,
      std::unique_ptr<t_struct_annotations> annotations) {
    return doc || annotations ? std::make_unique<t_def_attrs>(t_def_attrs{
                                    std::move(doc), std::move(annotations)})
                              : nullptr;
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::string name) {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->set_ttype(driver_.new_type_ref(
        std::move(name), nullptr, range, /*is_const=*/true));
    return driver_.new_struct_annotation(std::move(value), range);
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::unique_ptr<t_const_value> value) {
    return driver_.new_struct_annotation(std::move(value), range);
  }

  void on_statement(std::unique_ptr<t_named> stmt) {
    driver_.add_def(std::move(stmt));
  }

  std::unique_ptr<t_service> on_service(
      source_range range,
      std::string name,
      const std::string& base_name,
      std::unique_ptr<t_function_list> functions) {
    auto base = !base_name.empty() ? driver_.find_service(base_name) : nullptr;
    auto service =
        std::make_unique<t_service>(driver_.program, std::move(name), base);
    service->set_src_range(range);
    driver_.set_functions(*service, std::move(functions));
    service->set_lineno(driver_.get_lineno(range.begin));
    return service;
  }

  std::unique_ptr<t_interaction> on_interaction(
      source_range range,
      std::string name,
      std::unique_ptr<t_function_list> functions) {
    auto interaction =
        std::make_unique<t_interaction>(driver_.program, std::move(name));
    interaction->set_src_range(range);
    driver_.set_functions(*interaction, std::move(functions));
    interaction->set_lineno(driver_.get_lineno(range.begin));
    return interaction;
  }

  std::unique_ptr<t_function> on_function(
      source_range range,
      std::unique_ptr<t_def_attrs> attrs,
      t_function_qualifier qual,
      std::vector<t_type_ref> return_type,
      source_location name_loc,
      std::string name,
      t_field_list params,
      std::unique_ptr<t_throws> throws,
      std::unique_ptr<t_annotations> annotations) {
    auto function = std::make_unique<t_function>(
        driver_.program, std::move(return_type), std::move(name));
    function->set_qualifier(qual);
    driver_.set_fields(function->params(), std::move(params));
    function->set_exceptions(std::move(throws));
    function->set_lineno(driver_.get_lineno(name_loc));
    function->set_src_range(range);
    // TODO: Leave the param list unnamed.
    function->params().set_name(function->name() + "_args");
    driver_.set_attributes(
        *function, std::move(attrs), std::move(annotations), range);
    return function;
  }

  t_type_ref on_stream_return_type(type_throws_spec spec) {
    auto stream_response =
        std::make_unique<t_stream_response>(std::move(spec.type));
    stream_response->set_exceptions(std::move(spec.throws));
    return driver_.new_type_ref(std::move(stream_response), {});
  }

  t_type_ref on_sink_return_type(
      type_throws_spec sink_spec, type_throws_spec final_response_spec) {
    auto sink = std::make_unique<t_sink>(
        std::move(sink_spec.type), std::move(final_response_spec.type));
    sink->set_sink_exceptions(std::move(sink_spec.throws));
    sink->set_final_response_exceptions(std::move(final_response_spec.throws));
    return driver_.new_type_ref(std::move(sink), {});
  }

  t_type_ref on_list_type(
      t_type_ref element_type, std::unique_ptr<t_annotations> annotations) {
    return driver_.new_type_ref(
        std::make_unique<t_list>(std::move(element_type)),
        std::move(annotations));
  }

  t_type_ref on_set_type(
      t_type_ref key_type, std::unique_ptr<t_annotations> annotations) {
    return driver_.new_type_ref(
        std::make_unique<t_set>(std::move(key_type)), std::move(annotations));
  }

  t_type_ref on_map_type(
      t_type_ref key_type,
      t_type_ref value_type,
      std::unique_ptr<t_annotations> annotations) {
    return driver_.new_type_ref(
        std::make_unique<t_map>(std::move(key_type), std::move(value_type)),
        std::move(annotations));
  }

  std::unique_ptr<t_function> on_performs(source_range range, t_type_ref type) {
    std::string name = type.get_type() ? "create" + type.get_type()->get_name()
                                       : "<interaction placeholder>";
    auto function = std::make_unique<t_function>(
        driver_.program, std::move(type), std::move(name));
    function->set_lineno(driver_.get_lineno());
    function->set_src_range(range);
    function->set_is_interaction_constructor();
    return function;
  }

  std::unique_ptr<t_throws> on_throws(t_field_list exceptions) {
    return driver_.new_throws(
        std::make_unique<t_field_list>(std::move(exceptions)));
  }

  std::unique_ptr<t_typedef> on_typedef(
      source_range range, t_type_ref type, std::string name) {
    auto typedef_node = std::make_unique<t_typedef>(
        driver_.program, std::move(name), std::move(type));
    typedef_node->set_src_range(range);
    typedef_node->set_lineno(driver_.get_lineno(range.begin));
    return typedef_node;
  }

  std::unique_ptr<t_struct> on_struct(
      source_range range, std::string name, t_field_list fields) {
    auto struct_node =
        std::make_unique<t_struct>(driver_.program, std::move(name));
    struct_node->set_src_range(range);
    driver_.set_fields(*struct_node, std::move(fields));
    struct_node->set_lineno(driver_.get_lineno(range.begin));
    return struct_node;
  }

  std::unique_ptr<t_union> on_union(
      source_range range, std::string name, t_field_list fields) {
    auto union_node =
        std::make_unique<t_union>(driver_.program, std::move(name));
    union_node->set_src_range(range);
    driver_.set_fields(*union_node, std::move(fields));
    union_node->set_lineno(driver_.get_lineno(range.begin));
    return union_node;
  }

  std::unique_ptr<t_exception> on_exception(
      source_range range,
      t_error_safety safety,
      t_error_kind kind,
      t_error_blame blame,
      std::string name,
      t_field_list fields) {
    auto exception =
        std::make_unique<t_exception>(driver_.program, std::move(name));
    exception->set_src_range(range);
    exception->set_safety(safety);
    exception->set_kind(kind);
    exception->set_blame(blame);
    driver_.set_fields(*exception, std::move(fields));
    exception->set_lineno(driver_.get_lineno(range.begin));
    return exception;
  }

  std::unique_ptr<t_field> on_field(
      source_range range,
      std::unique_ptr<t_def_attrs> attrs,
      boost::optional<int64_t> id,
      t_field_qualifier qual,
      t_type_ref type,
      source_location name_loc,
      std::string name,
      std::unique_ptr<t_const_value> value,
      std::unique_ptr<t_annotations> annotations,
      boost::optional<doc> doc) {
    auto field = std::make_unique<t_field>(
        std::move(type),
        std::move(name),
        id ? driver_.to_field_id(range.begin, *id)
           : boost::optional<t_field_id>());
    field->set_qualifier(qual);
    if (driver_.mode == parsing_mode::PROGRAM) {
      field->set_default_value(std::move(value));
    }
    field->set_lineno(driver_.get_lineno(name_loc));
    field->set_src_range(range);
    driver_.set_attributes(
        *field, std::move(attrs), std::move(annotations), range);
    if (doc) {
      driver_.set_doctext(*field, doc);
    }
    return field;
  }

  t_type_ref on_field_type(
      const t_base_type& type, std::unique_ptr<t_annotations> annotations) {
    return driver_.new_type_ref(type, std::move(annotations));
  }

  t_type_ref on_field_type(
      source_range range,
      std::string name,
      std::unique_ptr<t_annotations> annotations) {
    return driver_.new_type_ref(std::move(name), std::move(annotations), range);
  }

  std::unique_ptr<t_enum> on_enum(
      source_range range, std::string name, t_enum_value_list values) {
    auto enum_node = std::make_unique<t_enum>(driver_.program, std::move(name));
    enum_node->set_values(std::move(values));
    enum_node->set_lineno(driver_.get_lineno(range.begin));
    return enum_node;
  }

  std::unique_ptr<t_enum_value> on_enum_value(
      source_range range,
      std::unique_ptr<t_def_attrs> attrs,
      source_location name_loc,
      std::string name,
      int64_t* value,
      std::unique_ptr<t_annotations> annotations,
      boost::optional<doc> doc) {
    auto enum_value = std::make_unique<t_enum_value>(std::move(name));
    driver_.set_attributes(
        *enum_value, std::move(attrs), std::move(annotations), range);
    if (value) {
      enum_value->set_value(driver_.to_enum_value(range.begin, *value));
    }
    enum_value->set_lineno(driver_.get_lineno(name_loc));
    if (doc) {
      driver_.set_doctext(*enum_value, std::move(doc));
    }
    return enum_value;
  }

  std::unique_ptr<t_const> on_const(
      source_range range,
      t_type_ref type,
      std::string name,
      std::unique_ptr<t_const_value> value) {
    auto const_node = std::make_unique<t_const>(
        driver_.program, std::move(type), std::move(name), std::move(value));
    const_node->set_lineno(driver_.get_lineno(range.begin));
    return const_node;
  }

  std::unique_ptr<t_const_value> on_bool_const(bool value) {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_bool(value);
    return const_value;
  }

  std::unique_ptr<t_const_value> on_int_const(
      source_location loc, int64_t value) {
    return driver_.to_const_value(loc, value);
  }

  std::unique_ptr<t_const_value> on_double_const(double value) {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_double(value);
    return const_value;
  }

  std::unique_ptr<t_const_value> on_reference_const(
      source_location loc, std::string name) {
    return driver_.copy_const_value(loc, std::move(name));
  }

  std::unique_ptr<t_const_value> on_string_literal(std::string value) {
    return std::make_unique<t_const_value>(std::move(value));
  }

  std::unique_ptr<t_const_value> on_const_list() {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_list();
    return const_value;
  }

  std::unique_ptr<t_const_value> on_const_map() {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    return const_value;
  }

  std::unique_ptr<t_const_value> on_const_struct(
      source_range range, std::string name) {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    const_value->set_ttype(driver_.new_type_ref(
        std::move(name), nullptr, range, /*is_const=*/true));
    return const_value;
  }

  int64_t on_integer(sign s, uint64_t value) {
    return driver_.to_int(value, s == sign::minus);
  }

  [[noreturn]] void on_error() { driver_.end_parsing(); }
};

// Parses a Thrift source from the lexer, invokes parser actions on
// syntactic constructs and reports parse errors if any via diags.
bool parse(lexer& lex, parser_actions& actions, diagnostics_engine& diags);

} // namespace compiler
} // namespace thrift
} // namespace apache
