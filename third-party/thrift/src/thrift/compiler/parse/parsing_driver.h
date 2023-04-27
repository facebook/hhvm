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
#include <fmt/format.h>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_scope.h>
#include <thrift/compiler/ast/t_union.h>
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
  parsing_params() noexcept {} // Disable aggregate initialization.

  /**
   * Strictness level.
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
   * Whether or not a missing include file will end parsing.
   *
   * The resulting program won't be generatable, but this is
   * useful for codemod tooling.
   */
  bool allow_missing_includes = false;

  /**
   * Which experimental features should be allowed.
   *
   * 'all' can be used to enable all experimental features.
   */
  std::unordered_set<std::string> allow_experimental_features;

  /**
   * Search path for inclusions.
   */
  std::vector<std::string> incl_searchpath;
};

class parsing_driver : public parser_actions {
 public:
  parsing_driver(
      source_manager& sm,
      diagnostics_engine& diags,
      std::string path,
      parsing_params parse_params);

  void on_program() override { clear_doctext(); }

  void on_standard_header(
      source_location loc, std::unique_ptr<attributes> attrs) override;

  void on_package(
      source_range range,
      std::unique_ptr<attributes> attrs,
      fmt::string_view name) override;

  void on_include(source_range range, fmt::string_view str) override {
    add_include(fmt::to_string(str), range);
  }

  void on_cpp_include(source_range, fmt::string_view str) override {
    if (mode_ == parsing_mode::PROGRAM) {
      program_->add_language_include("cpp", fmt::to_string(str));
    }
  }

  void on_hs_include(source_range, fmt::string_view str) override {
    if (mode_ == parsing_mode::PROGRAM) {
      program_->add_language_include("hs", fmt::to_string(str));
    }
  }

  void on_namespace(const identifier& language, fmt::string_view ns) override {
    if (mode_ == parsing_mode::PROGRAM) {
      program_->set_namespace(fmt::to_string(language.str), fmt::to_string(ns));
    }
  }

  void on_definition(
      source_range range,
      std::unique_ptr<t_named> defn,
      std::unique_ptr<attributes> attrs,
      std::unique_ptr<deprecated_annotations> annotations) override;

  boost::optional<comment> on_doctext() override { return pop_doctext(); }

  void on_program_doctext() override {
    // When there is any doctext, assign it to the top-level program.
    set_doctext(*program_, pop_doctext());
  }

  comment on_inline_doc(source_location loc, fmt::string_view text) override {
    return {strip_doctext(text), loc};
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, fmt::string_view name) override {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->set_ttype(
        new_type_ref(fmt::to_string(name), nullptr, range, /*is_const=*/true));
    return new_struct_annotation(std::move(value), range);
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::unique_ptr<t_const_value> value) override {
    return new_struct_annotation(std::move(value), range);
  }

  std::unique_ptr<t_service> on_service(
      source_range range,
      const identifier& name,
      const identifier& base,
      std::unique_ptr<t_function_list> functions) override;

  std::unique_ptr<t_interaction> on_interaction(
      source_range range,
      const identifier& name,
      std::unique_ptr<t_function_list> functions) override {
    auto interaction =
        std::make_unique<t_interaction>(program_, fmt::to_string(name.str));
    interaction->set_src_range(range);
    set_functions(*interaction, std::move(functions));
    return interaction;
  }

  std::unique_ptr<t_function> on_function(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_function_qualifier qual,
      std::vector<t_type_ref> return_type,
      const identifier& name,
      t_field_list params,
      std::unique_ptr<t_throws> throws,
      std::unique_ptr<deprecated_annotations> annotations) override;

  t_type_ref on_stream_return_type(
      source_range range, type_throws_spec spec) override;

  t_type_ref on_sink_return_type(
      source_range range,
      type_throws_spec sink_spec,
      type_throws_spec final_response_spec) override;

  t_type_ref on_list_type(
      source_range range,
      t_type_ref element_type,
      std::unique_ptr<deprecated_annotations> annotations) override;

  t_type_ref on_set_type(
      source_range range,
      t_type_ref key_type,
      std::unique_ptr<deprecated_annotations> annotations) override;

  t_type_ref on_map_type(
      source_range range,
      t_type_ref key_type,
      t_type_ref value_type,
      std::unique_ptr<deprecated_annotations> annotations) override;

  std::unique_ptr<t_function> on_performs(
      source_range range, t_type_ref type) override;

  std::unique_ptr<t_throws> on_throws(t_field_list exceptions) override;

  std::unique_ptr<t_typedef> on_typedef(
      source_range range, t_type_ref type, const identifier& name) override;

  std::unique_ptr<t_struct> on_struct(
      source_range range, const identifier& name, t_field_list fields) override;

  std::unique_ptr<t_union> on_union(
      source_range range, const identifier& name, t_field_list fields) override;

  std::unique_ptr<t_exception> on_exception(
      source_range range,
      t_error_safety safety,
      t_error_kind kind,
      t_error_blame blame,
      const identifier& name,
      t_field_list fields) override;

  std::unique_ptr<t_field> on_field(
      source_range range,
      std::unique_ptr<attributes> attrs,
      boost::optional<int64_t> id,
      t_field_qualifier qual,
      t_type_ref type,
      const identifier& name,
      std::unique_ptr<t_const_value> value,
      std::unique_ptr<deprecated_annotations> annotations,
      boost::optional<comment> doc) override;

  t_type_ref on_type(
      const t_base_type& type,
      std::unique_ptr<deprecated_annotations> annotations) override;

  t_type_ref on_type(
      source_range range,
      fmt::string_view name,
      std::unique_ptr<deprecated_annotations> annotations) override;

  std::unique_ptr<t_enum> on_enum(
      source_range range,
      const identifier& name,
      t_enum_value_list values) override;

  std::unique_ptr<t_enum_value> on_enum_value(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      boost::optional<int64_t> value,
      std::unique_ptr<deprecated_annotations> annotations,
      boost::optional<comment> doc) override;

  std::unique_ptr<t_const> on_const(
      source_range range,
      t_type_ref type,
      const identifier& name,
      std::unique_ptr<t_const_value> value) override;

  std::unique_ptr<t_const_value> on_const_ref(const identifier& name) override;

  std::unique_ptr<t_const_value> on_integer(
      source_location loc, int64_t value) override;
  std::unique_ptr<t_const_value> on_float(double value) override;
  std::unique_ptr<t_const_value> on_string_literal(std::string value) override;
  std::unique_ptr<t_const_value> on_bool_literal(bool value) override;
  std::unique_ptr<t_const_value> on_list_literal() override;
  std::unique_ptr<t_const_value> on_map_literal() override;
  std::unique_ptr<t_const_value> on_struct_literal(
      source_range range, fmt::string_view name) override;

  int64_t on_integer(source_range range, sign s, uint64_t value) override;

  [[noreturn]] void on_error() override { end_parsing(); }

  /**
   * Parses a program and returns the resulted AST.
   * Diagnostic messages (warnings, debug messages, etc.) are reported via the
   * context provided in the constructor.
   */
  std::unique_ptr<t_program_bundle> parse();

 private:
  source_manager& source_mgr_;
  std::set<std::string> already_parsed_paths_;
  std::set<std::string> circular_deps_;

  diagnostics_engine& diags_;
  parsing_params params_;

  std::unordered_set<std::string> programs_that_parsed_definition_;

  // The last parsed doctext comment.
  boost::optional<comment> doctext_;

  // The parsing pass that we are on. We do different things on each pass.
  parsing_mode mode_ = parsing_mode::INCLUDES;

  // The master program AST. This is accessed from within the parser code to
  // build up the program elements.
  t_program* program_;

  std::unique_ptr<t_program_bundle> program_bundle_;

  // Global scope cache for faster compilation.
  t_scope* scope_cache_;

  // A global map that holds a pointer to all programs already cached.
  std::map<std::string, t_program*> program_cache_;

  class lex_handler_impl;

  // Parses a single .thrift file. The file to parse is stored in
  // params.program.
  void parse_file();

  [[noreturn]] void end_parsing();

  // Returns the directory path of a filename.
  static std::string directory_name(const std::string& filename);

  // Finds the appropriate file path for the given include filename.
  std::string find_include_file(
      source_location loc, const std::string& filename);

  void validate_header_location(source_location loc);

  // Checks the type of the parsed const information against its declared type.
  void validate_const_type(t_const* c);

  // Checks that the constant name does not refer to an ambiguous enum.
  // An ambiguous enum is one that is redefined but not referred to by
  // ENUM_NAME.ENUM_VALUE.
  void validate_not_ambiguous_enum(
      source_location loc, const std::string& name);

  // Clears any previously stored doctext string and prints a warning if
  // information is discarded.
  void clear_doctext();

  // Returns a previously pushed doctext.
  boost::optional<comment> pop_doctext();

  // Strips comment text and aligns leading whitespace on multiline doctext.
  std::string strip_doctext(fmt::string_view text);

  // Updates doctext of the given node.
  void set_doctext(t_node& node, boost::optional<comment> doctext) const;

  // Cleans up text commonly found in doxygen-like comments.
  //
  // Warning: mixing tabs and spaces may mess up formatting.
  std::string clean_up_doctext(std::string docstring);

  // Sets the annotations on the given node.
  static void set_annotations(
      t_node* node, std::unique_ptr<deprecated_annotations> annotations);

  // Sets the attributes on the given node.
  void set_attributes(
      t_named& node,
      std::unique_ptr<attributes> attrs,
      std::unique_ptr<deprecated_annotations> annots,
      const source_range& loc) const;

  // Adds an unnamed typedef to the program
  // TODO(afuller): Remove the need for these by an explicit t_type_ref node
  // that can annotatable.
  const t_type* add_unnamed_typedef(
      std::unique_ptr<t_typedef> node,
      std::unique_ptr<deprecated_annotations> annotations,
      const source_range& range = source_range());

  // Automatic numbering for field ids (deprecated).
  //
  // Field id are assigned starting from -1 and working their way down.
  void allocate_field_id(t_field_id& next_id, t_field& field);
  void maybe_allocate_field_id(t_field_id& next_id, t_field& field);

  std::unique_ptr<t_const> new_struct_annotation(
      std::unique_ptr<t_const_value> const_struct, const source_range& range);

  std::unique_ptr<t_throws> new_throws(
      std::unique_ptr<t_field_list> exceptions);

  // Creates a reference to a known type, potentally with additional
  // annotations.
  t_type_ref new_type_ref(
      const t_type& type,
      std::unique_ptr<deprecated_annotations> annotations,
      const source_range& range = source_range());
  t_type_ref new_type_ref(
      t_type&& type, std::unique_ptr<deprecated_annotations>) = delete;

  // Creates a reference to a newly instantiated templated type.
  t_type_ref new_type_ref(
      source_range range,
      std::unique_ptr<t_templated_type> type,
      std::unique_ptr<deprecated_annotations> annotations);
  // Creates a reference to a named type.
  t_type_ref new_type_ref(
      std::string name,
      std::unique_ptr<deprecated_annotations> annotations,
      const source_range& range,
      bool is_const = false);

  // Tries to set the given fields, reporting a failure on a collsion.
  void set_fields(t_structured& s, t_field_list&& fields);

  void set_functions(
      t_interface& node, std::unique_ptr<t_function_list> functions);

  void add_include(std::string name, const source_range& range);

  template <typename T>
  T narrow_int(source_location loc, int64_t value, const char* name);
};

} // namespace compiler
} // namespace thrift
} // namespace apache
