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

#include <cassert>
#include <exception>

#include <fmt/format.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parser.h>

// @lint-ignore-every CLANGTIDY clang-diagnostic-switch-enum

namespace apache::thrift::compiler::detail {

// A Thrift parser.
template <typename Actions>
class parser_core {
 private:
  lexer& lexer_;
  Actions& actions_;
  diagnostics_engine& diags_;
  parser_token_sink* token_sink_ = nullptr;

  using thrift_token = ::apache::thrift::compiler::token;
  using thrift_token_kind = ::apache::thrift::compiler::token_kind;

  using attributes_type = typename Actions::attributes_type;
  using comment_type = typename Actions::comment_type;
  using const_value_type = typename Actions::const_value_type;
  using deprecated_annotations_type =
      typename Actions::deprecated_annotations_type;
  using enum_value_list_type = typename Actions::enum_value_list_type;
  using enum_value_type = typename Actions::enum_value_type;
  using field_list_type = typename Actions::field_list_type;
  using field_type = typename Actions::field_type;
  using function_list_type = typename Actions::function_list_type;
  using function_type = typename Actions::function_type;
  using return_clause_type = typename Actions::return_clause_type;
  using sink_type = typename Actions::sink_type;
  using stream_type = typename Actions::stream_type;
  using structured_annotation_list_type =
      typename Actions::structured_annotation_list_type;
  using structured_annotation_type =
      typename Actions::structured_annotation_type;
  using throws_type = typename Actions::throws_type;
  using type_ref_type = typename Actions::type_ref_type;
  using type_throws_spec_type = typename Actions::type_throws_spec_type;

  struct parse_error : std::exception {};

  thrift_token token_ =
      thrift_token(tok::eof, {}); // The current unconsumed token.
  bool has_token_ = false;

  // End of the last consumed token.
  source_location end_;

  class range_tracker {
   private:
    source_location begin_;
    const source_location& end_;

   public:
    explicit range_tracker(source_location begin, const source_location& end)
        : begin_(begin), end_(end) {}

    operator source_location() const { return begin_; }
    operator source_range() const { return {begin_, end_}; }
  };

  // Tracks the source range of a syntactic construct.
  // Usage:
  //   auto range = track_range();
  //   // Parse <construct>.
  //   actions_.on_<construct>(range, ...);
  range_tracker track_range() const {
    return range_tracker(token_.range.begin, end_);
  }

  thrift_token consume_token() {
    thrift_token t = token_;
    end_ = t.range.end;
    if (!has_token_) {
      token_ = lexer_.get_next_token();
      has_token_ = true;
      if (token_.kind == tok::error) {
        actions_.on_error();
      }
      return t;
    }
    thrift_token next = lexer_.get_next_token();
    if (token_sink_ != nullptr) {
      token_sink_->on_token(t, next);
    }
    token_ = next;
    if (token_.kind == tok::error) {
      actions_.on_error();
    }
    return t;
  }

  bool try_consume_token(thrift_token_kind kind) {
    if (token_.kind != kind) {
      return false;
    }
    consume_token();
    return true;
  }

  std::string lex_string_literal(thrift_token literal) {
    auto str = lexer_.lex_string_literal(literal);
    if (!str) {
      actions_.on_error();
    }
    return *str;
  }

  [[noreturn]] void report_expected(std::string_view expected) {
    diags_.error(token_.range.begin, "expected {}", expected);
    throw parse_error();
  }

  // Reports a recoverable error without aborting the parse process.
  template <typename... T>
  void report_error(fmt::format_string<T...> msg, T&&... args) {
    diags_.error(token_.range.begin, msg, std::forward<T>(args)...);
  }

  source_range expect_and_consume(thrift_token_kind expected) {
    auto range = token_.range;
    if (token_.kind != expected) {
      report_expected(to_string(expected.value));
    }
    consume_token();
    return range;
  }

  // The parse methods are ordered top down from the most general to concrete.

  // program: [[program_doctext] (include | package | namespace)+] definition*
  bool parse_program() {
    consume_token();
    try {
      if (auto attrs = parse_package_and_directives()) {
        auto loc = attrs->loc;
        parse_definition(loc, std::move(attrs));
      }
      while (token_.kind != tok::eof) {
        auto begin = token_.range.begin;
        auto attrs = parse_attributes();
        parse_definition(begin, std::move(attrs));
      }
      actions_.on_program();
    } catch (const parse_error&) {
      return false; // The error has already been reported.
    }
    return true;
  }

  attributes_type parse_package_and_directives() {
    while (token_.kind != tok::eof) {
      auto begin = token_.range.begin;
      switch (token_.kind.value) {
        case tok::kw_include:
        case tok::kw_cpp_include:
        case tok::kw_hs_include:
          actions_.on_program_doctext();
          parse_include();
          break;
        case tok::kw_namespace:
          actions_.on_program_doctext();
          parse_namespace();
          break;
        default: {
          auto attrs = parse_attributes();
          if (token_.kind != tok::kw_package) {
            return attrs;
          }
          actions_.on_program_doctext();
          parse_package(begin, std::move(attrs));
          break;
        }
      }
    }
    return {};
  }

  // include: ("include" | "cpp_include" | "hs_include") string_literal [";"]
  void parse_include() {
    auto range = track_range();
    auto kind = token_.kind;
    consume_token();
    if (token_.kind != tok::string_literal) {
      report_expected("string literal");
    }
    auto str = lex_string_literal(token_);
    auto str_range = token_.range;
    consume_token();
    switch (kind.value) {
      case tok::kw_include: {
        auto alias = try_parse_include_alias();
        actions_.on_include(range, str, alias, str_range);
        break;
      }
      case tok::kw_cpp_include: {
        actions_.on_cpp_include(range, str);
        break;
      }
      case tok::kw_hs_include: {
        actions_.on_hs_include(range, str);
        break;
      }
      default:
        assert(false);
    }
    try_consume_token(';');
  }

  // package: one of the following:
  // attributes "package" string_literal [";"]
  // attributes "package" ";"
  void parse_package(source_location loc, attributes_type attrs) {
    assert(token_.kind == tok::kw_package);
    auto range = range_tracker(loc, end_);
    consume_token();
    std::string name;
    switch (token_.kind.value) {
      case tok::string_literal:
        name = lex_string_literal(token_);
        consume_token();
        if (name.empty()) {
          report_error("package name cannot be empty");
        }
        try_consume_token(';');
        break;
      case tok::semi:
        consume_token();
        break;
      default:
        report_expected("string literal");
    }

    // NOTE: `name` can be empty (if explicitly specified via `package;`)
    actions_.on_package(range, std::move(attrs), name);
  }

  // namespace: "namespace" identifier (identifier | string_literal) [";"]
  void parse_namespace() {
    auto range = track_range();
    assert(token_.kind == tok::kw_namespace);
    consume_token();
    auto language = parse_identifier();
    auto ns = token_.kind == tok::string_literal
        ? lex_string_literal(consume_token())
        : fmt::to_string(parse_identifier().str);
    try_consume_token(';');
    return actions_.on_namespace(range, language, ns);
  }

  // definition:
  //     service
  //   | interaction
  //   | typedef
  //   | struct
  //   | union
  //   | exception
  //   | enum
  //   | const
  void parse_definition(source_location loc, attributes_type attrs) {
    switch (token_.kind.value) {
      case tok::kw_service:
        parse_service(loc, std::move(attrs));
        break;
      case tok::kw_interaction:
        parse_interaction(loc, std::move(attrs));
        break;
      case tok::kw_typedef:
        parse_typedef(loc, std::move(attrs));
        break;
      case tok::kw_struct:
        parse_struct(loc, std::move(attrs));
        break;
      case tok::kw_union:
        parse_union(loc, std::move(attrs));
        break;
      case tok::kw_safe:
      case tok::kw_transient:
      case tok::kw_stateful:
      case tok::kw_permanent:
      case tok::kw_client:
      case tok::kw_server:
      case tok::kw_exception:
        parse_exception(loc, std::move(attrs));
        break;
      case tok::kw_enum:
        parse_enum(loc, std::move(attrs));
        break;
      case tok::kw_const:
        parse_const(loc, std::move(attrs));
        break;
      default:
        report_expected("definition");
    }
  }

  // attributes: [doctext] structured_annotation*
  attributes_type parse_attributes() {
    auto loc = token_.range.begin;
    auto doc = actions_.on_doctext();
    auto annotations = structured_annotation_list_type();
    while (token_.kind == '@') {
      annotations.emplace_back(parse_structured_annotation());
    }
    return actions_.on_attributes(loc, std::move(doc), std::move(annotations));
  }

  std::optional<comment_type> try_parse_inline_doc() {
    return token_.kind == tok::inline_doc
        ? actions_.on_inline_doc(token_.range, consume_token().string_value())
        : std::optional<comment_type>();
  }

  // structured_annotation: "@" (identifier | struct_initializer)
  structured_annotation_type parse_structured_annotation() {
    assert(token_.kind == '@');
    auto range = track_range();
    consume_token();
    auto name = parse_identifier();
    if (token_.kind != '{') {
      return actions_.on_structured_annotation(range, name.str);
    }
    auto value = parse_struct_initializer_body(name);
    return actions_.on_structured_annotation(range, std::move(value));
  }

  // deprecated_annotations: "(" deprecated_annotation_list? ")"
  //
  // deprecated_annotation_list:
  //   (deprecated_annotation comma_or_semicolon)*
  //    deprecated_annotation comma_or_semicolon?
  //
  // deprecated_annotation:
  //   identifier ["=" (bool_literal | integer | string_literal)]
  deprecated_annotations_type try_parse_deprecated_annotations() {
    if (token_.kind != '(') {
      return {};
    }
    auto range = track_range();
    consume_token();
    while (token_.kind != ')') {
      auto key = parse_identifier();
      if (try_consume_token('=')) {
        if (token_.kind == tok::string_literal) {
          lex_string_literal(consume_token());
        } else if (token_.kind == tok::bool_literal) {
          consume_token();
        } else if (auto integer = try_parse_integer()) {
          // consumed
        } else {
          report_expected("integer, bool or string");
        }
      }
      auto key_str = fmt::to_string(key.str);
      if (!key_str.starts_with("hs.")) {
        report_error(
            "Unstructured annotation `{}` is not allowed. "
            "Use a structured annotation instead.",
            key_str);
      }
      if (!try_parse_comma_or_semicolon()) {
        break;
      }
    }
    expect_and_consume(')');
    return actions_.on_deprecated_annotations(range);
  }

  void try_parse_deprecated_annotations(attributes_type& attrs) {
    if (auto deprecated_annotations = try_parse_deprecated_annotations()) {
      actions_.set_deprecated_annotations(
          attrs, std::move(deprecated_annotations));
    }
  }

  // service:
  //   attributes
  //   "service" identifier ["extends" identifier] "{"
  //     (function | performs)*
  //   "}"
  //   [deprecated_annotations]
  //
  // performs: "performs" identifier ";"
  void parse_service(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_service);
    auto name = parse_identifier();
    identifier base;
    if (try_consume_token(tok::kw_extends)) {
      base = parse_identifier();
    }
    auto functions = parse_interface_body();
    try_parse_deprecated_annotations(attrs);
    actions_.on_service(
        range, std::move(attrs), name, base, std::move(functions));
  }

  // interaction:
  //   attributes
  //   "interaction" identifier "{" function* "}"
  //   [deprecated_annotations]
  void parse_interaction(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_interaction);
    auto name = parse_identifier();
    auto functions = parse_interface_body(false);
    try_parse_deprecated_annotations(attrs);
    actions_.on_interaction(
        range, std::move(attrs), name, std::move(functions));
  }

  function_list_type parse_interface_body(bool allow_performs = true) {
    expect_and_consume('{');
    auto functions = function_list_type();
    while (token_.kind != '}') {
      if (token_.kind != tok::kw_performs) {
        functions.emplace_back(parse_function());
        continue;
      }
      // Parse performs.
      auto range = track_range();
      consume_token();
      auto interaction_name = parse_identifier();
      expect_and_consume(';');
      if (allow_performs) {
        functions.emplace_back(actions_.on_performs(range, interaction_name));
      } else {
        diags_.error(
            source_location(range), "cannot use 'performs' in an interaction");
      }
    }
    expect_and_consume('}');
    return functions;
  }

  // function:
  //   attributes [function_qualifier]
  //   return_clause identifier "(" (parameter [","])* ")" [throws]
  //   [deprecated_annotations] [";"]
  //
  // function_qualifier: "oneway" | "idempotent" | "readonly"
  function_type parse_function() {
    auto range = track_range();
    auto attrs = parse_attributes();

    // Parse a function qualifier.
    auto qual = t_function_qualifier();
    switch (token_.kind.value) {
      case tok::kw_oneway:
        qual = t_function_qualifier::oneway;
        consume_token();
        break;
      case tok::kw_idempotent:
        qual = t_function_qualifier::idempotent;
        consume_token();
        break;
      case tok::kw_readonly:
        qual = t_function_qualifier::readonly;
        consume_token();
        break;
      default:
        break;
    }

    auto ret = parse_return_clause();
    auto name = parse_identifier();
    auto params = parse_param_list(field_kind::param);

    auto throws = try_parse_throws();
    try_parse_deprecated_annotations(attrs);
    if (token_.kind == ';') {
      consume_token();
    }
    if (token_.kind == ',') {
      diags_.warning(token_.range.begin, "unexpected ','");
      consume_token();
    }
    return actions_.on_function(
        range,
        std::move(attrs),
        qual,
        std::move(ret),
        name,
        std::move(params),
        std::move(throws));
  }

  // return_clause:
  //   return_type ["," streaming]
  //   interaction_name ["," return_type] ["," streaming]
  //
  // interaction_name: maybe_qualified_id
  //
  // return_type: type | "void"
  //
  // streaming: sink ["," stream] | stream
  return_clause_type parse_return_clause() {
    auto ret = return_clause_type();
    if (token_.kind == tok::identifier) {
      // Parse an interaction or type name.
      ret.name = identifier{token_.string_value(), token_.range.begin};
      consume_token();
      if (!try_consume_token(',')) {
        return ret;
      }
    }
    bool is_void = false;
    switch (token_.kind.value) {
      case tok::kw_void:
        is_void = true;
        ret.type = actions_.on_type(token_.range, t_primitive_type::t_void());
        consume_token();
        break;
      case tok::kw_sink:
        ret.sink = parse_sink();
        break;
      case tok::kw_stream:
        ret.stream = parse_stream();
        break;
      default:
        ret.type = parse_type();
        break;
    }
    if (!try_consume_token(',')) {
      return ret;
    }
    if (is_void) {
      report_error("cannot use 'void' as an initial response type");
    }
    switch (token_.kind.value) {
      case tok::kw_sink:
        if (ret.stream) {
          report_expected("'sink' before 'stream'");
        }
        if (ret.sink) {
          report_error("duplicate 'sink'");
        }
        ret.sink = parse_sink();
        break;
      case tok::kw_stream:
        if (ret.stream) {
          report_error("duplicate 'stream'");
        }
        ret.stream = parse_stream();
        break;
      default:
        report_expected("'sink' or 'stream' after the initial response type");
    }
    if (!try_consume_token(',')) {
      return ret;
    }
    switch (token_.kind.value) {
      case tok::kw_stream:
        if (ret.stream) {
          report_error("duplicate 'stream'");
        }
        ret.stream = parse_stream();
        break;
      case tok::kw_sink:
        if (ret.stream) {
          report_expected("'sink' before 'stream'");
        }
        if (ret.sink) {
          report_error("duplicate 'sink'");
        }
        break;
      default:
        report_expected("identifier");
    }
    return ret;
  }

  // sink: "sink" "<" type [throws], type [throws] ">"
  sink_type parse_sink() {
    auto range = track_range();
    consume_token();
    expect_and_consume('<');
    auto sink = parse_type_throws();
    std::optional<type_throws_spec_type> final_response;
    if (try_consume_token(',')) {
      final_response = parse_type_throws();
    }
    expect_and_consume('>');
    return actions_.on_sink(range, std::move(sink), std::move(final_response));
  }

  // stream: "stream" "<" type [throws] ">"
  stream_type parse_stream() {
    auto range = track_range();
    consume_token();
    expect_and_consume('<');
    auto response = parse_type_throws();
    expect_and_consume('>');
    return actions_.on_stream(range, std::move(response));
  }

  type_throws_spec_type parse_type_throws() {
    auto type = parse_type();
    auto throws = try_parse_throws();
    return {type, std::move(throws)};
  }

  // throws: "throws" "(" field* ")"
  throws_type try_parse_throws() {
    if (!try_consume_token(tok::kw_throws)) {
      return {};
    }
    return actions_.on_throws(parse_param_list(field_kind::throws_field));
  }

  // typedef:
  //   attributes "typedef" type identifier [deprecated_annotations] [";"]
  void parse_typedef(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_typedef);
    auto type = parse_type();
    auto name = parse_identifier();
    try_parse_deprecated_annotations(attrs);
    try_consume_token(';');
    actions_.on_typedef(range, std::move(attrs), type, name);
  }

  // struct:
  //   attributes "struct" identifier "{" field* "}" [deprecated_annotations]
  void parse_struct(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_struct);
    auto name = parse_identifier();
    auto fields = parse_field_list();
    try_parse_deprecated_annotations(attrs);
    actions_.on_struct(range, std::move(attrs), name, std::move(fields));
  }

  // union:
  //   attributes "union" identifier "{" field* "}" [deprecated_annotations]
  void parse_union(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_union);
    auto name = parse_identifier();
    auto fields = parse_field_list();
    try_parse_deprecated_annotations(attrs);
    actions_.on_union(range, std::move(attrs), name, std::move(fields));
  }

  // exception:
  //   attributes error_safety? error_kind? error_blame?
  //   "exception" identifier "{" field* "}" [deprecated_annotations]
  //
  // error_safety: "safe"
  // error_kind: "transient" | "stateful" | "permanent"
  // error_blame: "client" | "server"
  void parse_exception(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    auto safety = try_consume_token(tok::kw_safe) ? t_error_safety::safe
                                                  : t_error_safety::unspecified;
    auto kind = t_error_kind::unspecified;
    switch (token_.kind.value) {
      case tok::kw_transient:
        kind = t_error_kind::transient;
        consume_token();
        break;
      case tok::kw_stateful:
        kind = t_error_kind::stateful;
        consume_token();
        break;
      case tok::kw_permanent:
        kind = t_error_kind::permanent;
        consume_token();
        break;
      default:
        break;
    }
    auto blame = t_error_blame::unspecified;
    if (try_consume_token(tok::kw_client)) {
      blame = t_error_blame::client;
    } else if (try_consume_token(tok::kw_server)) {
      blame = t_error_blame::server;
    }
    expect_and_consume(tok::kw_exception);
    auto name = parse_identifier();
    auto fields = parse_field_list();
    try_parse_deprecated_annotations(attrs);
    actions_.on_exception(
        range, std::move(attrs), safety, kind, blame, name, std::move(fields));
  }

  enum class field_kind { field, param, throws_field };

  field_list_type parse_field_list() {
    expect_and_consume('{');
    auto fields = field_list_type();
    while (token_.kind != '}') {
      fields.emplace_back(parse_field(field_kind::field));
    }
    expect_and_consume('}');
    return fields;
  }

  field_list_type parse_param_list(field_kind kind) {
    expect_and_consume('(');
    auto params = field_list_type();
    while (token_.kind != ')') {
      params.emplace_back(parse_field(kind));
    }
    expect_and_consume(')');
    return params;
  }

  // Parses a field or a parameter.
  //
  // field:
  //   attributes
  //   field_id ":" [field_qualifier] type identifier
  //     [default_value] annotations [';'] [inline_doc]
  //
  // parameter ::=
  //   attributes
  //   field_id ":" type identifier [default_value]
  //     annotations [','] [inline_doc]
  //
  // field_id: integer
  // field_qualifier: "required" | "optional"
  // default_value: "=" initializer
  field_type parse_field(field_kind kind) {
    auto range = track_range();
    auto attrs = parse_attributes();

    // Parse the field id.
    auto field_id = std::optional<int64_t>();
    if (auto integer = try_parse_integer()) {
      field_id = *integer;
      expect_and_consume(':');
    }

    // Parse the field qualifier.
    auto qual = t_field_qualifier::none;
    auto qual_tok = token_;
    if (try_consume_token(tok::kw_optional)) {
      qual = t_field_qualifier::optional;
    } else if (try_consume_token(tok::kw_required)) {
      qual = t_field_qualifier::required;
    }
    if (qual != t_field_qualifier::none && kind == field_kind::param) {
      diags_.warning(
          qual_tok.range.begin,
          "'{}' is not permitted on a parameter",
          to_string(qual_tok.kind));
      qual = t_field_qualifier::none;
    } else if (
        qual != t_field_qualifier::none && kind == field_kind::throws_field) {
      diags_.error(
          qual_tok.range.begin,
          "'{}' is not permitted in a throws clause",
          to_string(qual_tok.kind));
      qual = t_field_qualifier::none;
    }

    type_ref_type type = parse_type();
    const identifier name = parse_identifier();

    // Parse the default value.
    const_value_type value;
    if (try_consume_token('=')) {
      value = parse_initializer();
    }

    try_parse_deprecated_annotations(attrs);
    if (token_.kind == ',' || token_.kind == ';') {
      // Use to_tok instead of a token_kind ctor to work around an MSVC bug
      // (https://godbolt.org/z/8f6GE9545).
      thrift_token_kind delimiter =
          kind == field_kind::field ? to_tok(';') : to_tok(',');
      if (token_.kind != delimiter) {
        diags_.warning(
            token_.range.begin, "unexpected '{}'", to_string(token_.kind));
      }
      consume_token();
    }
    auto doc = try_parse_inline_doc();
    return actions_.on_field(
        range,
        std::move(attrs),
        field_id,
        qual,
        type,
        name,
        std::move(value),
        std::move(doc));
  }

  // type: (tok::identifier | base_type | container_type) annotations
  //
  // container_type: list_type | set_type | map_type
  //
  // list_type: "list" "<" type ">"
  // set_type: "set" "<" type ">"
  // map_type: "map" "<" type "," type ">"
  //
  // We disallow context-sensitive keywords as field type identifiers.
  // This avoids an ambiguity in the resolution of the function_qualifier
  // return_clause part of the function rule, when one of the "oneway",
  // "idempotent" or "readonly" is encountered. It could either resolve
  // the token as function_qualifier or resolve "" as function_qualifier and
  // the token as return_clause.
  type_ref_type parse_type() {
    auto range = track_range();
    if (const t_primitive_type* type = try_parse_base_type()) {
      try_parse_deprecated_annotations();
      return actions_.on_type(range, *type);
    }
    switch (token_.kind.value) {
      case tok::identifier: {
        auto name = consume_token().string_value();
        try_parse_deprecated_annotations();
        return actions_.on_type(range, name);
      }
      case tok::kw_list: {
        consume_token();
        expect_and_consume('<');
        auto element_type = parse_type();
        expect_and_consume('>');
        try_parse_deprecated_annotations();
        return actions_.on_list_type(range, element_type);
      }
      case tok::kw_set: {
        consume_token();
        expect_and_consume('<');
        auto key_type = parse_type();
        expect_and_consume('>');
        try_parse_deprecated_annotations();
        return actions_.on_set_type(range, key_type);
      }
      case tok::kw_map: {
        consume_token();
        expect_and_consume('<');
        auto key_type = parse_type();
        expect_and_consume(',');
        auto value_type = parse_type();
        expect_and_consume('>');
        try_parse_deprecated_annotations();
        return actions_.on_map_type(range, key_type, value_type);
      }
      case tok::kw_void:
        report_error("`void` cannot be used as a data type");
        consume_token();
        return actions_.on_invalid_type(range, t_primitive_type::t_void());
      default:
        report_expected("type");
    }
  }

  // base_type: "bool" | "byte" | "i16" | "i32" | "i64" | "float" | "double" |
  //            "string" | "binary"
  const t_primitive_type* try_parse_base_type() {
    auto get_base_type = [this]() -> const t_primitive_type* {
      switch (token_.kind.value) {
        case tok::kw_bool:
          return &t_primitive_type::t_bool();
        case tok::kw_byte:
          return &t_primitive_type::t_byte();
        case tok::kw_i16:
          return &t_primitive_type::t_i16();
        case tok::kw_i32:
          return &t_primitive_type::t_i32();
        case tok::kw_i64:
          return &t_primitive_type::t_i64();
        case tok::kw_float:
          return &t_primitive_type::t_float();
        case tok::kw_double:
          return &t_primitive_type::t_double();
        case tok::kw_string:
          return &t_primitive_type::t_string();
        case tok::kw_binary:
          return &t_primitive_type::t_binary();
        default:
          return nullptr;
      }
    };
    auto base_type = get_base_type();
    if (base_type) {
      consume_token();
    }
    return base_type;
  }

  // enum:
  //   attributes "enum" identifier "{" enum_value* "}" [deprecated_annotations]
  void parse_enum(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_enum);
    auto name = parse_identifier();
    expect_and_consume('{');
    auto values = enum_value_list_type();
    while (token_.kind != '}') {
      values.emplace_back(parse_enum_value());
    }
    expect_and_consume('}');
    try_parse_deprecated_annotations(attrs);
    actions_.on_enum(range, std::move(attrs), name, std::move(values));
  }

  // enum_value:
  //   attributes
  //   identifier ["=" integer] [deprecated_annotations] [","] [inline_doc]
  enum_value_type parse_enum_value() {
    auto range = track_range();
    auto attrs = parse_attributes();
    auto name = parse_identifier();
    auto value = try_consume_token('=')
        ? std::optional<int64_t>(parse_integer())
        : std::nullopt;
    try_parse_deprecated_annotations(attrs);
    if (token_.kind == ',') {
      consume_token();
    } else if (token_.kind == ';') {
      diags_.warning(token_.range.begin, "unexpected ';'");
      consume_token();
    }
    auto doc = try_parse_inline_doc();
    return actions_.on_enum_value(
        range, std::move(attrs), name, value, std::move(doc));
  }

  // const:
  //   attributes "const" type identifier "=" initializer
  //   [deprecated_annotations] [";"]
  void parse_const(source_location loc, attributes_type attrs) {
    auto range = range_tracker(loc, end_);
    expect_and_consume(tok::kw_const);
    auto type = parse_type();
    auto name = parse_identifier();
    expect_and_consume('=');
    auto value = parse_initializer();
    try_parse_deprecated_annotations(attrs);
    try_consume_token(';');
    actions_.on_const(range, std::move(attrs), type, name, std::move(value));
  }

  // initializer:
  //   integer | float | string_literal | bool_literal | identifier |
  //   list_initializer | map_initializer | struct_initializer
  const_value_type parse_initializer() {
    range_tracker range = track_range();
    const_value_type initializer_const_value = parse_initializer_const_value();
    actions_.set_const_value_src_range(initializer_const_value, range);
    return initializer_const_value;
  }

  const_value_type parse_initializer_const_value() {
    auto s = sign::plus;
    switch (token_.kind.value) {
      case tok::bool_literal:
        return actions_.on_bool_literal(consume_token().bool_value());
      case to_tok('-'):
        s = sign::minus;
        FMT_FALLTHROUGH;
      case to_tok('+'):
        consume_token();
        if (token_.kind == tok::int_literal) {
          return actions_.on_integer(parse_integer(s));
        } else if (token_.kind == tok::float_literal) {
          return actions_.on_float(parse_float(s));
        }
        report_expected("number");
      case tok::int_literal:
        return actions_.on_integer(parse_integer());
      case tok::float_literal:
        return actions_.on_float(parse_float());
      case tok::string_literal:
        return actions_.on_string_literal(lex_string_literal(consume_token()));
      case to_tok('['):
        return parse_list_initializer();
      case to_tok('{'):
        return parse_map_initializer();
      default:
        if (auto id = try_parse_identifier()) {
          return token_.kind == '{' ? parse_struct_initializer_body(*id)
                                    : actions_.on_const_ref(*id);
        }
        break;
    }
    report_expected("constant");
  }

  // list_initializer: "[" list_initializer_contents? "]"
  //
  // list_initializer_contents:
  //   (initializer comma_or_semicolon)* initializer comma_or_semicolon?
  const_value_type parse_list_initializer() {
    expect_and_consume('[');
    auto list = actions_.on_list_initializer();
    while (token_.kind != ']') {
      actions_.add_list_value(list, parse_initializer());
      if (!try_parse_comma_or_semicolon()) {
        break;
      }
    }
    expect_and_consume(']');
    return list;
  }

  // map_initializer: "{" [(map_entry ",")* map_entry [","]] "}"
  // map_entry:       initializer ":" initializer
  const_value_type parse_map_initializer() {
    expect_and_consume('{');
    auto map = actions_.on_map_initializer();
    while (token_.kind != '}') {
      auto key = parse_initializer();
      if (token_.kind == ',' || token_.kind == '}') {
        // It's intuitive to think sets are declared with curly braces. Ensure
        // an insightful error to correct this is returned
        report_error(
            "Curly braces {{}} are for map initializers. Use square braces [] for list/set initializers");
        throw parse_error();
      }
      expect_and_consume(':');
      auto value = parse_initializer();
      actions_.add_map_value(map, std::move(key), std::move(value));
      if (!try_consume_token(',')) {
        break;
      }
    }
    expect_and_consume('}');
    return map;
  }

  // struct_initializer: identifier "{" struct_initializer_contents? "}"
  //
  // struct_initializer_contents:
  //   (identifier "=" initializer comma_or_semicolon)*
  //    identifier "=" initializer comma_or_semicolon?
  const_value_type parse_struct_initializer_body(identifier id) {
    auto id_end = end_;
    expect_and_consume('{');
    auto map = actions_.on_struct_initializer({id.loc, id_end}, id.str);
    while (token_.kind != '}') {
      auto key =
          actions_.on_string_literal(fmt::to_string(parse_identifier().str));
      expect_and_consume('=');
      auto value = parse_initializer();
      actions_.add_map_value(map, std::move(key), std::move(value));
      if (!try_parse_comma_or_semicolon()) {
        break;
      }
    }
    expect_and_consume('}');
    return map;
  }

  // integer: ["+" | "-"] int_literal
  std::optional<int64_t> try_parse_integer(sign s = sign::plus) {
    auto range = track_range();
    switch (token_.kind.value) {
      case to_tok('-'):
        s = sign::minus;
        FMT_FALLTHROUGH;
      case to_tok('+'):
        consume_token();
        if (token_.kind != tok::int_literal) {
          report_expected("integer");
        }
        FMT_FALLTHROUGH;
      case tok::int_literal: {
        auto token = consume_token();
        return actions_.on_integer(range, s, token.int_value());
      }
      default:
        return {};
    }
  }

  int64_t parse_integer(sign s = sign::plus) {
    if (auto result = try_parse_integer(s)) {
      return *result;
    }
    report_expected("integer");
  }

  // float: ["+" | "-"] float_literal
  double parse_float(sign s = sign::plus) {
    switch (token_.kind.value) {
      case to_tok('-'):
        s = sign::minus;
        FMT_FALLTHROUGH;
      case to_tok('+'):
        consume_token();
        if (token_.kind != tok::float_literal) {
          break;
        }
        FMT_FALLTHROUGH;
      case tok::float_literal: {
        double value = consume_token().float_value();
        return s == sign::plus ? value : -value;
      }
      default:
        break;
    }
    report_expected("floating-point number");
  }

  // identifier:
  //     tok::identifier
  //   | "package"
  //   | "sink"
  //   | "oneway"
  //   | "readonly"
  //   | "idempotent"
  //   | "safe"
  //   | "transient"
  //   | "stateful"
  //   | "permanent"
  //   | "server"
  //   | "client"
  std::optional<identifier> try_parse_identifier() {
    range_tracker range = track_range();
    switch (token_.kind.value) {
      case tok::identifier:
        return identifier{consume_token().string_value(), range};
      // Context-sensitive keywords allowed in identifiers:
      case tok::kw_package:
      case tok::kw_sink:
      case tok::kw_oneway:
      case tok::kw_readonly:
      case tok::kw_idempotent:
      case tok::kw_safe:
      case tok::kw_transient:
      case tok::kw_stateful:
      case tok::kw_permanent:
      case tok::kw_server:
      case tok::kw_client:
      case tok::kw_as:
        return identifier{to_string(consume_token().kind), range};
      default:
        return {};
    }
  }

  identifier parse_identifier() {
    if (std::optional<identifier> id = try_parse_identifier()) {
      return *id;
    }
    report_expected("identifier");
  }

  // comma_or_semicolon: ","  | ";"
  bool try_parse_comma_or_semicolon() {
    return try_consume_token(',') || try_consume_token(';');
  }

  std::optional<std::string_view> try_parse_include_alias() {
    if (!try_consume_token(tok::kw_as)) {
      return std::nullopt;
    }

    auto alias = try_parse_identifier();
    if (!alias.has_value()) {
      if (token_.kind == ';') {
        report_error("Include alias cannot be empty");
      } else {
        const auto invalid_alias = consume_token();
        report_error(
            "Invalid include alias '{}'", to_string(invalid_alias.kind.value));
      }
      return std::nullopt;
    }

    return alias.value().str;
  }

 public:
  parser_core(
      lexer& lex,
      Actions& actions,
      diagnostics_engine& diags,
      parser_token_sink* token_sink = nullptr)
      : lexer_(lex),
        actions_(actions),
        diags_(diags),
        token_sink_(token_sink) {}

  bool parse() { return parse_program(); }
};

} // namespace apache::thrift::compiler::detail
