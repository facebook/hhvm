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
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

class lex_handler;
class lexer;

// Parsing only representations.
struct t_annotations {
  std::map<std::string, annotation_value> strings;
  std::map<std::string, std::shared_ptr<const t_const>> objects;
};

struct doc {
  std::string text;
  source_location loc;
};

// TODO (partisan): Rename to t_stmt_attrs.
struct t_def_attrs {
  boost::optional<compiler::doc> doc;
  std::unique_ptr<node_list<t_const>> struct_annotations;
};

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

class parsing_driver {
 private:
  source_manager* source_mgr_;
  class lex_handler_impl;
  std::unique_ptr<lex_handler_impl> lex_handler_;
  std::unique_ptr<lexer> lexer_;

  // Returns the current source location, see lexer::location.
  source_location location() const;

 public:
  parsing_params params;

  /**
   * The last parsed doctext comment.
   */
  boost::optional<doc> doctext;

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

  parsing_driver(
      source_manager& sm,
      diagnostic_context& ctx,
      std::string path,
      parsing_params parse_params);
  ~parsing_driver();

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

  [[noreturn]] void end_parsing(fmt::string_view msg) {
    error(location(), "{}", msg);
    end_parsing();
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
  boost::optional<doc> pop_doctext();

  /**
   * Strips comment chars and aligns leading whitespace on multiline doctext.
   */
  std::string strip_doctext(fmt::string_view text);

  /** Updates doctext of given node. */
  void set_doctext(t_node& node, boost::optional<doc> doctext) const;

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
      std::unique_ptr<t_def_attrs> attrs,
      std::unique_ptr<t_annotations> annots,
      const source_range& loc) const;

  // Adds a definition to the program.
  void add_def(std::unique_ptr<t_named> node);

  void add_include(std::string name, const source_range& range);
  void set_package(std::string name, const source_range& range);

  t_field_id to_field_id(source_location loc, int64_t value) {
    return narrow_int<t_field_id>(loc, value, "field ids");
  }
  int32_t to_enum_value(source_location loc, int64_t value) {
    return narrow_int<int32_t>(loc, value, "enum values");
  }
  std::unique_ptr<t_const_value> to_const_value(
      source_location loc, int64_t value);

  int64_t to_int(uint64_t val, bool negative = false);

  const t_service* find_service(const std::string& name);
  const t_const* find_const(source_location loc, const std::string& name);

  std::unique_ptr<t_const_value> copy_const_value(
      source_location loc, const std::string& name);

  void set_parsed_definition();
  void validate_header_location();
  void validate_header_annotations(
      std::unique_ptr<t_def_attrs> statement_attrs,
      std::unique_ptr<t_annotations> annotations);
  void set_program_annotations(
      std::unique_ptr<t_def_attrs> statement_attrs,
      std::unique_ptr<t_annotations> annotations,
      const source_range& loc);

 private:
  std::set<std::string> already_parsed_paths_;
  std::set<std::string> circular_deps_;

  diagnostic_context& ctx_;

  std::unordered_set<std::string> programs_that_parsed_definition_;

  /**
   * Parse a single .thrift file. The file to parse is stored in params.program.
   */
  void parse_file();

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
