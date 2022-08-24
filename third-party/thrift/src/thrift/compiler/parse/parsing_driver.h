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

#include <cstddef>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/optional.hpp>
#include <fmt/core.h>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_scope.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/parser.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

enum class parsing_mode {
  INCLUDES = 1,
  PROGRAM = 2,
};

struct parsing_params {
  // Default values are taken from the original global variables.

  parsing_params() noexcept {} // Disable aggregate initialization

  /**
   * Strictness level
   */
  int strict = 127;

  /**
   * Whether or not negative field keys are accepted.
   *
   * When a field does not have a user-specified key, thrift automatically
   * assigns a negative value.  However, this is fragile since changes to the
   * file may unintentionally change the key numbering, resulting in a new
   * protocol that is not backwards compatible.
   *
   * When allow_neg_field_keys is enabled, users can explicitly specify
   * negative keys.  This way they can write a .thrift file with explicitly
   * specified keys that is still backwards compatible with older .thrift files
   * that did not specify key values.
   */
  bool allow_neg_field_keys = false;

  /**
   * Whether or not 64-bit constants will generate a warning.
   *
   * Some languages don't support 64-bit constants, but many do, so we can
   * suppress this warning for projects that don't use any non-64-bit-safe
   * languages.
   */
  bool allow_64bit_consts = false;

  /**
   * Which experimental features should be allowed.
   *
   * 'all' can be used to enable all experimental features.
   */
  std::unordered_set<std::string> allow_experimental_features;

  /**
   * Search path for inclusions
   */
  std::vector<std::string> incl_searchpath;
};

class parsing_driver : public parser_actions {
 public:
  parsing_driver(
      source_manager& sm,
      diagnostic_context& ctx,
      std::string path,
      parsing_params parse_params);
  ~parsing_driver() override;

  void on_program() override { clear_doctext(); }

  void on_standard_header(
      source_range range,
      std::unique_ptr<stmt_attrs> attrs,
      std::unique_ptr<t_annotations> annotations) override;
  void on_program_header(
      source_range range,
      std::unique_ptr<stmt_attrs> attrs,
      std::unique_ptr<t_annotations> annotations) override;

  void on_package(source_range range, std::string name) override;

  void on_include(source_range range, std::string literal) override {
    add_include(std::move(literal), range);
  }

  void on_cpp_include(source_range, std::string literal) override {
    if (mode == parsing_mode::PROGRAM) {
      program->add_cpp_include(std::move(literal));
    }
  }

  void on_hs_include(source_range, std::string) override {
    // Do nothing. This syntax is handled by the hs compiler.
  }

  void on_namespace(std::string language, std::string ns) override {
    if (mode == parsing_mode::PROGRAM) {
      program->set_namespace(std::move(language), std::move(ns));
    }
  }

  void on_definition(
      source_range range,
      t_named& def,
      std::unique_ptr<stmt_attrs> attrs,
      std::unique_ptr<t_annotations> annotations) override {
    set_parsed_definition();
    set_attributes(def, std::move(attrs), std::move(annotations), range);
  }

  boost::optional<comment> on_doctext() override { return pop_doctext(); }

  void on_program_doctext() override {
    // When there is any doctext, assign it to the top-level program.
    set_doctext(*program, pop_doctext());
  }

  comment on_inline_doc(source_location loc, std::string text) override {
    return {strip_doctext(text), loc};
  }

  std::unique_ptr<stmt_attrs> on_statement_attrs(
      boost::optional<comment> doc,
      std::unique_ptr<node_list<t_const>> annotations) override {
    return doc || annotations ? std::make_unique<stmt_attrs>(stmt_attrs{
                                    std::move(doc), std::move(annotations)})
                              : nullptr;
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::string name) override {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->set_ttype(
        new_type_ref(std::move(name), nullptr, range, /*is_const=*/true));
    return new_struct_annotation(std::move(value), range);
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::unique_ptr<t_const_value> value) override {
    return new_struct_annotation(std::move(value), range);
  }

  void on_statement(std::unique_ptr<t_named> stmt) override {
    add_def(std::move(stmt));
  }

  std::unique_ptr<t_service> on_service(
      source_range range,
      std::string name,
      const std::string& base_name,
      std::unique_ptr<t_function_list> functions) override;

  std::unique_ptr<t_interaction> on_interaction(
      source_range range,
      std::string name,
      std::unique_ptr<t_function_list> functions) override {
    auto interaction =
        std::make_unique<t_interaction>(program, std::move(name));
    interaction->set_src_range(range);
    set_functions(*interaction, std::move(functions));
    return interaction;
  }

  std::unique_ptr<t_function> on_function(
      source_range range,
      std::unique_ptr<stmt_attrs> attrs,
      t_function_qualifier qual,
      std::vector<t_type_ref> return_type,
      source_location,
      std::string name,
      t_field_list params,
      std::unique_ptr<t_throws> throws,
      std::unique_ptr<t_annotations> annotations) override {
    auto function = std::make_unique<t_function>(
        program, std::move(return_type), std::move(name));
    function->set_qualifier(qual);
    set_fields(function->params(), std::move(params));
    function->set_exceptions(std::move(throws));
    function->set_src_range(range);
    // TODO: Leave the param list unnamed.
    function->params().set_name(function->name() + "_args");
    set_attributes(*function, std::move(attrs), std::move(annotations), range);
    return function;
  }

  t_type_ref on_stream_return_type(type_throws_spec spec) override {
    auto stream_response =
        std::make_unique<t_stream_response>(std::move(spec.type));
    stream_response->set_exceptions(std::move(spec.throws));
    return new_type_ref(std::move(stream_response), {});
  }

  t_type_ref on_sink_return_type(
      type_throws_spec sink_spec,
      type_throws_spec final_response_spec) override {
    auto sink = std::make_unique<t_sink>(
        std::move(sink_spec.type), std::move(final_response_spec.type));
    sink->set_sink_exceptions(std::move(sink_spec.throws));
    sink->set_final_response_exceptions(std::move(final_response_spec.throws));
    return new_type_ref(std::move(sink), {});
  }

  t_type_ref on_list_type(
      t_type_ref element_type,
      std::unique_ptr<t_annotations> annotations) override {
    return new_type_ref(
        std::make_unique<t_list>(std::move(element_type)),
        std::move(annotations));
  }

  t_type_ref on_set_type(
      t_type_ref key_type,
      std::unique_ptr<t_annotations> annotations) override {
    return new_type_ref(
        std::make_unique<t_set>(std::move(key_type)), std::move(annotations));
  }

  t_type_ref on_map_type(
      t_type_ref key_type,
      t_type_ref value_type,
      std::unique_ptr<t_annotations> annotations) override {
    return new_type_ref(
        std::make_unique<t_map>(std::move(key_type), std::move(value_type)),
        std::move(annotations));
  }

  std::unique_ptr<t_function> on_performs(
      source_range range, t_type_ref type) override {
    std::string name = type.get_type() ? "create" + type.get_type()->get_name()
                                       : "<interaction placeholder>";
    auto function =
        std::make_unique<t_function>(program, std::move(type), std::move(name));
    function->set_src_range(range);
    function->set_is_interaction_constructor();
    return function;
  }

  std::unique_ptr<t_throws> on_throws(t_field_list exceptions) override {
    return new_throws(std::make_unique<t_field_list>(std::move(exceptions)));
  }

  std::unique_ptr<t_typedef> on_typedef(
      source_range range, t_type_ref type, std::string name) override {
    auto typedef_node =
        std::make_unique<t_typedef>(program, std::move(name), std::move(type));
    typedef_node->set_src_range(range);
    return typedef_node;
  }

  std::unique_ptr<t_struct> on_struct(
      source_range range, std::string name, t_field_list fields) override {
    auto struct_node = std::make_unique<t_struct>(program, std::move(name));
    struct_node->set_src_range(range);
    set_fields(*struct_node, std::move(fields));
    return struct_node;
  }

  std::unique_ptr<t_union> on_union(
      source_range range, std::string name, t_field_list fields) override {
    auto union_node = std::make_unique<t_union>(program, std::move(name));
    union_node->set_src_range(range);
    set_fields(*union_node, std::move(fields));
    return union_node;
  }

  std::unique_ptr<t_exception> on_exception(
      source_range range,
      t_error_safety safety,
      t_error_kind kind,
      t_error_blame blame,
      std::string name,
      t_field_list fields) override {
    auto exception = std::make_unique<t_exception>(program, std::move(name));
    exception->set_src_range(range);
    exception->set_safety(safety);
    exception->set_kind(kind);
    exception->set_blame(blame);
    set_fields(*exception, std::move(fields));
    return exception;
  }

  std::unique_ptr<t_field> on_field(
      source_range range,
      std::unique_ptr<stmt_attrs> attrs,
      boost::optional<int64_t> id,
      t_field_qualifier qual,
      t_type_ref type,
      source_location,
      std::string name,
      std::unique_ptr<t_const_value> value,
      std::unique_ptr<t_annotations> annotations,
      boost::optional<comment> doc) override {
    auto field = std::make_unique<t_field>(
        std::move(type),
        std::move(name),
        id ? to_field_id(range.begin, *id) : boost::optional<t_field_id>());
    field->set_qualifier(qual);
    if (mode == parsing_mode::PROGRAM) {
      field->set_default_value(std::move(value));
    }
    field->set_src_range(range);
    set_attributes(*field, std::move(attrs), std::move(annotations), range);
    if (doc) {
      set_doctext(*field, doc);
    }
    return field;
  }

  t_type_ref on_field_type(
      const t_base_type& type,
      std::unique_ptr<t_annotations> annotations) override {
    return new_type_ref(type, std::move(annotations));
  }

  t_type_ref on_field_type(
      source_range range,
      std::string name,
      std::unique_ptr<t_annotations> annotations) override {
    return new_type_ref(std::move(name), std::move(annotations), range);
  }

  std::unique_ptr<t_enum> on_enum(
      source_range range, std::string name, t_enum_value_list values) override {
    auto enum_node = std::make_unique<t_enum>(program, std::move(name));
    enum_node->set_src_range(range);
    enum_node->set_values(std::move(values));
    return enum_node;
  }

  std::unique_ptr<t_enum_value> on_enum_value(
      source_range range,
      std::unique_ptr<stmt_attrs> attrs,
      source_location,
      std::string name,
      int64_t* value,
      std::unique_ptr<t_annotations> annotations,
      boost::optional<comment> doc) override {
    auto enum_value = std::make_unique<t_enum_value>(std::move(name));
    enum_value->set_src_range(range);
    set_attributes(
        *enum_value, std::move(attrs), std::move(annotations), range);
    if (value) {
      enum_value->set_value(to_enum_value(range.begin, *value));
    }
    if (doc) {
      set_doctext(*enum_value, std::move(doc));
    }
    return enum_value;
  }

  std::unique_ptr<t_const> on_const(
      source_range range,
      t_type_ref type,
      std::string name,
      std::unique_ptr<t_const_value> value) override {
    auto const_node = std::make_unique<t_const>(
        program, std::move(type), std::move(name), std::move(value));
    const_node->set_src_range(range);
    return const_node;
  }

  std::unique_ptr<t_const_value> on_bool_const(bool value) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_bool(value);
    return const_value;
  }

  std::unique_ptr<t_const_value> on_int_const(
      source_location loc, int64_t value) override {
    return to_const_value(loc, value);
  }

  std::unique_ptr<t_const_value> on_double_const(double value) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_double(value);
    return const_value;
  }

  std::unique_ptr<t_const_value> on_reference_const(
      source_location loc, std::string name) override {
    return copy_const_value(loc, std::move(name));
  }

  std::unique_ptr<t_const_value> on_string_literal(std::string value) override {
    return std::make_unique<t_const_value>(std::move(value));
  }

  std::unique_ptr<t_const_value> on_const_list() override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_list();
    return const_value;
  }

  std::unique_ptr<t_const_value> on_const_map() override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    return const_value;
  }

  std::unique_ptr<t_const_value> on_const_struct(
      source_range range, std::string name) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    const_value->set_ttype(
        new_type_ref(std::move(name), nullptr, range, /*is_const=*/true));
    return const_value;
  }

  int64_t on_integer(source_range range, sign s, uint64_t value) override;

  [[noreturn]] void on_error() override { end_parsing(); }

 public:
  /**
   * The last parsed doctext comment.
   */
  boost::optional<comment> doctext;

  /**
   * The parsing pass that we are on. We do different things on each pass.
   */
  parsing_mode mode = parsing_mode::INCLUDES;

  /**
   * The master program parse tree. This is accessed from within the parser code
   * to build up the program elements.
   */
  t_program* program;

  std::unique_ptr<t_program_bundle> program_bundle;

  /**
   * Global scope cache for faster compilations
   */
  t_scope* scope_cache;

  /**
   * A global map that holds a pointer to all programs already cached
   */
  std::map<std::string, t_program*> program_cache;

  int get_lineno(source_location loc = {});

  /**
   * Parses a program and returns the resulted AST.
   * Diagnostic messages (warnings, debug messages, etc.) are reported via the
   * context provided in the constructor.
   */
  std::unique_ptr<t_program_bundle> parse();

  template <typename... T>
  void warning(source_location loc, fmt::format_string<T...> msg, T&&... args) {
    ctx_.report(loc, diagnostic_level::warning, msg, std::forward<T>(args)...);
  }

  template <typename... T>
  void error(source_location loc, fmt::format_string<T...> msg, T&&... args) {
    ctx_.report(loc, diagnostic_level::error, msg, std::forward<T>(args)...);
  }

  [[noreturn]] void end_parsing();

  /**
   * Gets the directory path of a filename.
   */
  static std::string directory_name(const std::string& filename);

  /**
   * Finds the appropriate file path for the given include filename.
   */
  std::string find_include_file(
      source_location loc, const std::string& filename);

  /**
   * Check the type of the parsed const information against its declared type.
   */
  void validate_const_type(t_const* c);

  /**
   * Check that the constant name does not refer to an ambiguous enum.
   * An ambiguous enum is one that is redefined but not referred to by
   * ENUM_NAME.ENUM_VALUE.
   */
  void validate_not_ambiguous_enum(
      source_location loc, const std::string& name);

  /**
   * Clears any previously stored doctext string.
   * Also prints a warning if we are discarding information.
   */
  void clear_doctext();

  /** Returns any doctext previously pushed. */
  boost::optional<comment> pop_doctext();

  /**
   * Strips comment chars and aligns leading whitespace on multiline doctext.
   */
  std::string strip_doctext(fmt::string_view text);

  /** Updates doctext of given node. */
  void set_doctext(t_node& node, boost::optional<comment> doctext) const;

  /**
   * Cleans up text commonly found in doxygen-like comments.
   *
   * Warning: if you mix tabs and spaces in a non-uniform way,
   * you will get what you deserve.
   */
  std::string clean_up_doctext(std::string docstring);

  // Populate the annotation on the given node.
  static void set_annotations(
      t_node* node, std::unique_ptr<t_annotations> annotations);

  std::unique_ptr<t_const> new_struct_annotation(
      std::unique_ptr<t_const_value> const_struct, const source_range& range);

  std::unique_ptr<t_throws> new_throws(
      std::unique_ptr<t_field_list> exceptions);

  // Creates a reference to a known type, potentally with additional
  // annotations.
  t_type_ref new_type_ref(
      const t_type& type, std::unique_ptr<t_annotations> annotations);
  t_type_ref new_type_ref(t_type&& type, std::unique_ptr<t_annotations>) =
      delete;

  // Creates a reference to a newly instantiated templated type.
  t_type_ref new_type_ref(
      std::unique_ptr<t_templated_type> type,
      std::unique_ptr<t_annotations> annotations);
  // Creates a reference to a named type.
  t_type_ref new_type_ref(
      std::string name,
      std::unique_ptr<t_annotations> annotations,
      const source_range& range,
      bool is_const = false);

  // Tries to set the given fields, reporting a failure on a collsion.
  // TODO(afuller): Disallow auto-id allocation.
  void set_fields(t_structured& tstruct, t_field_list&& fields);

  void set_functions(
      t_interface& node, std::unique_ptr<t_function_list> functions);

  // Populate the attributes on the given node.
  void set_attributes(
      t_named& node,
      std::unique_ptr<stmt_attrs> attrs,
      std::unique_ptr<t_annotations> annots,
      const source_range& loc) const;

  // Adds a definition to the program.
  void add_def(std::unique_ptr<t_named> node);

  void add_include(std::string name, const source_range& range);

  t_field_id to_field_id(source_location loc, int64_t value) {
    return narrow_int<t_field_id>(loc, value, "field ids");
  }
  int32_t to_enum_value(source_location loc, int64_t value) {
    return narrow_int<int32_t>(loc, value, "enum values");
  }
  std::unique_ptr<t_const_value> to_const_value(
      source_location loc, int64_t value);

  const t_const* find_const(source_location loc, const std::string& name);

  std::unique_ptr<t_const_value> copy_const_value(
      source_location loc, const std::string& name);

  void set_parsed_definition();
  void set_program_annotations(
      std::unique_ptr<stmt_attrs> statement_attrs,
      std::unique_ptr<t_annotations> annotations,
      const source_range& loc);

 private:
  source_manager& source_mgr_;

  std::set<std::string> already_parsed_paths_;
  std::set<std::string> circular_deps_;

  diagnostic_context& ctx_;
  parsing_params params_;

  std::unordered_set<std::string> programs_that_parsed_definition_;

  /**
   * Parse a single .thrift file. The file to parse is stored in params.program.
   */
  void parse_file();

  void validate_header_location(source_location loc);

  // Adds an unnamed typedef to the program
  // TODO(afuller): Remove the need for these by an explicit t_type_ref node
  // that can annotatable.
  const t_type* add_unnamed_typedef(
      std::unique_ptr<t_typedef> node,
      std::unique_ptr<t_annotations> annotations);

  // Automatic numbering for field ids.
  //
  // Field id are assigned starting from -1 and working their way down.
  //
  // TODO(afuller): Move auto field ids to a post parse phase (or remove the
  // feature entirely).
  void allocate_field_id(t_field_id& next_id, t_field& field);
  void maybe_allocate_field_id(t_field_id& next_id, t_field& field);

  template <typename T>
  T narrow_int(source_location loc, int64_t value, const char* name) {
    using limits = std::numeric_limits<T>;
    if (mode == parsing_mode::PROGRAM &&
        (value < limits::min() || value > limits::max())) {
      error(
          loc,
          "Integer constant {} outside the range of {} ([{}, {}]).",
          value,
          name,
          limits::min(),
          limits::max());
    }
    return value;
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
