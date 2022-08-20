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

#include <cassert>
#include <exception>

#include <fmt/core.h>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parser.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

// A Thrift parser.
class parser {
 private:
  lexer& lexer_;
  parser_actions& actions_;
  diagnostics_engine& diags_;

  struct parse_error : std::exception {};

  token token_ = token(tok::eof, {}); // The current unconsumed token.

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

  token consume_token() {
    token t = token_;
    end_ = token_.range.end;
    token_ = lexer_.get_next_token();
    if (token_.kind == tok::error) {
      actions_.on_error();
    }
    return t;
  }

  bool try_consume_token(token_kind kind) {
    if (token_.kind != kind) {
      return false;
    }
    consume_token();
    return true;
  }

  [[noreturn]] void report_expected(fmt::string_view expected) {
    diags_.error(token_.range.begin, "expected {}", expected);
    throw parse_error();
  }

  source_range expect_and_consume(token_kind expected) {
    auto range = token_.range;
    if (token_.kind != expected) {
      report_expected(to_string(expected.value));
    }
    consume_token();
    return range;
  }

  // The parse methods are ordered top down from the most general to concrete.

  // program: statement_list
  //
  // statement_list:
  //   statement_list statement_annotated comma_or_semicolon_optional
  // | /* empty */
  bool parse_program() {
    consume_token();
    try {
      while (token_.kind != tok::eof) {
        auto stmt = parse_statement();
        if (stmt) {
          actions_.on_statement(std::move(stmt));
        }
      }
      actions_.on_program();
    } catch (const parse_error&) {
      return false; // The error has already been reported.
    }
    return true;
  }

  // statement_annotated:
  //   statement_attrs statement annotations
  //
  // statement:
  //     /* program_doc_text (empty) */ header
  //   | definition
  std::unique_ptr<t_named> parse_statement() {
    auto range = track_range();
    auto attrs = parse_statement_attrs();
    auto stmt = parse_header_or_definition();
    auto annotations = parse_annotations();
    parse_comma_or_semicolon_optional();
    switch (stmt.type) {
      case t_statement_type::standard_header:
        actions_.on_standard_header(std::move(attrs), std::move(annotations));
        break;
      case t_statement_type::program_header:
        actions_.on_program_header(
            range, std::move(attrs), std::move(annotations));
        break;
      case t_statement_type::definition:
        actions_.on_definition(
            range, *stmt.def, std::move(attrs), std::move(annotations));
        break;
    }
    return std::move(stmt.def);
  }

  struct statement {
    t_statement_type type = t_statement_type::definition;
    std::unique_ptr<t_named> def;

    statement(t_statement_type t = t_statement_type::definition) : type(t) {}

    template <typename Named>
    /* implicit */ statement(std::unique_ptr<Named> n) : def(std::move(n)) {}
  };

  // header:
  //     include_or_package
  //   | namespace
  //
  // definition:
  //     service
  //   | interaction
  //   | typedef
  //   | struct
  //   | union
  //   | exception
  //   | enum
  //   | const
  statement parse_header_or_definition() {
    switch (token_.kind) {
      case tok::kw_include:
      case tok::kw_cpp_include:
      case tok::kw_hs_include:
      case tok::kw_package:
        return parse_include_or_package();
      case tok::kw_namespace:
        parse_namespace();
        return t_statement_type::standard_header;
      case tok::kw_typedef:
        return parse_typedef();
      case tok::kw_enum:
        return parse_enum();
      case tok::kw_const:
        return parse_const();
      case tok::kw_struct:
        return parse_struct();
      case tok::kw_union:
        return parse_union();
      case tok::kw_safe:
      case tok::kw_transient:
      case tok::kw_stateful:
      case tok::kw_permanent:
      case tok::kw_client:
      case tok::kw_server:
      case tok::kw_exception:
        return parse_exception();
      case tok::kw_service:
        return parse_service();
      case tok::kw_interaction:
        return parse_interaction();
      default:
        report_expected("header or definition");
    }
  }

  // include_or_package:
  //     "include" string_literal
  //   | "cpp_include" string_literal
  //   | "hs_include" string_literal
  //   | "package" string_literal
  t_statement_type parse_include_or_package() {
    auto range = track_range();
    auto kind = token_.kind;
    actions_.on_program_doctext();
    consume_token();
    if (token_.kind != tok::string_literal) {
      report_expected("string literal");
    }
    auto literal = token_.string_value();
    consume_token();
    switch (kind) {
      case tok::kw_package:
        actions_.on_package(range, literal);
        return t_statement_type::program_header;
      case tok::kw_include:
        actions_.on_include(range, std::move(literal));
        break;
      case tok::kw_cpp_include:
        actions_.on_cpp_include(range, std::move(literal));
        break;
      case tok::kw_hs_include:
        actions_.on_hs_include(range, std::move(literal));
        break;
      default:
        assert(false);
    }
    return t_statement_type::standard_header;
  }

  // namespace:
  //     "namespace" identifier identifier
  //   | "namespace" identifier string_literal
  void parse_namespace() {
    assert(token_.kind == tok::kw_namespace);
    actions_.on_program_doctext();
    consume_token();
    auto language = parse_identifier();
    std::string ns = token_.kind == tok::string_literal
        ? consume_token().string_value()
        : parse_identifier();
    return actions_.on_namespace(language, ns);
  }

  // statement_attrs: /* capture_doc_text (empty) */ structured_annotations
  //
  // structured_annotations:
  //     structured_annotations structured_annotation
  //   | /* empty */
  std::unique_ptr<t_def_attrs> parse_statement_attrs() {
    auto doc = actions_.on_doctext();
    auto annotations = std::unique_ptr<t_struct_annotations>();
    while (auto annotation = parse_structured_annotation()) {
      if (!annotations) {
        annotations = std::make_unique<t_struct_annotations>();
      }
      annotations->emplace_back(std::move(annotation));
    }
    return actions_.on_statement_attrs(std::move(doc), std::move(annotations));
  }

  // inline_doc_optional: inline_doc | /* empty */
  boost::optional<doc> parse_inline_doc_optional() {
    auto loc = token_.range.begin;
    return token_.kind == tok::inline_doc
        ? actions_.on_inline_doc(loc, consume_token().string_value())
        : boost::optional<doc>();
  }

  // structured_annotation:
  //     "@" const_struct
  //   | "@" const_struct_type
  std::unique_ptr<t_const> parse_structured_annotation() {
    auto range = track_range();
    if (!try_consume_token('@')) {
      return {};
    }
    auto name_range = token_.range;
    auto name = parse_identifier();
    if (token_.kind != '{') {
      return actions_.on_structured_annotation(range, name);
    }
    auto const_value = parse_const_struct_body(name_range, std::move(name));
    return actions_.on_structured_annotation(range, std::move(const_value));
  }

  // annotations:
  //     "(" annotation_list comma_or_semicolon_optional ")"
  //   | "(" ")"
  //   | /* empty */
  //
  // annotation_list:
  //     annotation_list comma_or_semicolon annotation
  //   | annotation
  //
  // annotation: identifier "=" int_or_literal | identifier
  //
  // int_or_literal: string_literal | bool_constant | integer
  std::unique_ptr<t_annotations> parse_annotations() {
    if (!try_consume_token('(')) {
      return {};
    }
    auto annotations = std::unique_ptr<t_annotations>();
    while (token_.kind != ')') {
      if (!annotations) {
        annotations = std::make_unique<t_annotations>();
      }
      auto range = track_range();
      auto key = parse_identifier();
      auto value = std::string("1");
      if (try_consume_token('=')) {
        if (token_.kind == tok::string_literal) {
          value = consume_token().string_value();
        } else if (token_.kind == tok::bool_constant) {
          value = fmt::format("{:d}", consume_token().bool_value());
        } else if (auto integer = try_parse_integer()) {
          value = fmt::format("{}", *integer);
        } else {
          report_expected("integer, bool or string");
        }
      }
      annotations->strings[std::move(key)] = {range, std::move(value)};
      if (!parse_comma_or_semicolon_optional()) {
        break;
      }
    }
    expect_and_consume(')');
    return annotations;
  }

  // service: "service" identifier extends "{" function_list "}"
  //
  // extends: "extends" identifier | /* empty */
  std::unique_ptr<t_service> parse_service() {
    auto range = track_range();
    expect_and_consume(tok::kw_service);
    auto name = parse_identifier();
    auto base = std::string();
    if (try_consume_token(tok::kw_extends)) {
      base = parse_identifier();
    }
    auto functions = parse_braced_function_list();
    return actions_.on_service(
        range, std::move(name), std::move(base), std::move(functions));
  }

  // interaction: "interaction" identifier "{" function_list "}"
  std::unique_ptr<t_interaction> parse_interaction() {
    auto range = track_range();
    expect_and_consume(tok::kw_interaction);
    auto name = parse_identifier();
    auto functions = parse_braced_function_list();
    return actions_.on_interaction(
        range, std::move(name), std::move(functions));
  }

  // function_list:
  //     function_list function comma_or_semicolon_optional
  //   | function_list performs comma_or_semicolon
  //   | /* empty */
  //
  // performs: "performs" field_type
  std::unique_ptr<t_function_list> parse_braced_function_list() {
    expect_and_consume('{');
    auto functions = std::make_unique<t_function_list>();
    while (token_.kind != '}') {
      if (token_.kind != tok::kw_performs) {
        functions->emplace_back(parse_function());
        parse_comma_or_semicolon_optional();
        continue;
      }
      // Parse performs.
      auto range = track_range();
      consume_token();
      auto type = parse_field_type();
      if (!parse_comma_or_semicolon_optional()) {
        report_expected("`,` or `;`");
      }
      functions->emplace_back(actions_.on_performs(range, type));
    }
    expect_and_consume('}');
    return functions;
  }

  // function:
  //   statement_attrs function_qualifier
  //     return_type identifier "(" field_list ")" maybe_throws annotations
  //
  // function_qualifier: "oneway" | "idempotent" | "readonly" | /* empty */
  std::unique_ptr<t_function> parse_function() {
    auto range = track_range();
    auto attrs = parse_statement_attrs();

    // Parse a function qualifier.
    auto qual = t_function_qualifier();
    switch (token_.kind) {
      case tok::kw_oneway:
        qual = t_function_qualifier::one_way;
        consume_token();
        break;
      case tok::kw_idempotent:
        qual = t_function_qualifier::idempotent;
        consume_token();
        break;
      case tok::kw_readonly:
        qual = t_function_qualifier::read_only;
        consume_token();
        break;
      default:
        break;
    }

    auto return_type = parse_return_type();
    auto name_loc = token_.range.begin;
    auto name = parse_identifier();

    // Parse arguments.
    expect_and_consume('(');
    auto params = parse_field_list(')');
    expect_and_consume(')');

    auto throws = parse_throws();
    auto annotations = parse_annotations();
    return actions_.on_function(
        range,
        std::move(attrs),
        qual,
        std::move(return_type),
        name_loc,
        std::move(name),
        std::move(params),
        std::move(throws),
        std::move(annotations));
  }

  // return_type:
  //     return_type_element
  //   | return_type "," return_type_element
  //
  // return_type_element:
  //     field_type | stream_return_type | sink_return_type | "void"
  //
  // stream_return_type: "stream" "<" field_type maybe_throws ">"
  //
  // sink_return_type: "sink" "<" sink_field_type "," sink_field_type ">"
  std::vector<t_type_ref> parse_return_type() {
    auto return_type = std::vector<t_type_ref>();
    auto parse_type_throws = [this]() -> type_throws_spec {
      auto type = parse_field_type();
      auto throws = parse_throws();
      return {std::move(type), std::move(throws)};
    };
    do {
      auto type = t_type_ref();
      switch (token_.kind) {
        case tok::kw_void:
          type = t_base_type::t_void();
          consume_token();
          break;
        case tok::kw_stream: {
          consume_token();
          expect_and_consume('<');
          auto response = parse_type_throws();
          expect_and_consume('>');
          type = actions_.on_stream_return_type(std::move(response));
          break;
        }
        case tok::kw_sink: {
          consume_token();
          expect_and_consume('<');
          auto sink = parse_type_throws();
          expect_and_consume(',');
          auto final_response = parse_type_throws();
          expect_and_consume('>');
          type = actions_.on_sink_return_type(
              std::move(sink), std::move(final_response));
          break;
        }
        default:
          type = parse_field_type();
          break;
      }
      return_type.push_back(std::move(type));
    } while (try_consume_token(','));
    return return_type;
  }

  // maybe_throws: "throws" "(" field_list ")" | /* empty */
  std::unique_ptr<t_throws> parse_throws() {
    if (!try_consume_token(tok::kw_throws)) {
      return {};
    }
    expect_and_consume('(');
    auto exceptions = parse_field_list(')');
    expect_and_consume(')');
    return actions_.on_throws(std::move(exceptions));
  }

  // typedef: "typedef" field_type identifier
  std::unique_ptr<t_typedef> parse_typedef() {
    auto range = track_range();
    expect_and_consume(tok::kw_typedef);
    auto type = parse_field_type();
    auto name = parse_identifier();
    return actions_.on_typedef(range, std::move(type), std::move(name));
  }

  // struct: "struct" identifier "{" field_list "}"
  std::unique_ptr<t_struct> parse_struct() {
    auto range = track_range();
    expect_and_consume(tok::kw_struct);
    auto name = parse_identifier();
    auto fields = parse_braced_field_list();
    return actions_.on_struct(range, std::move(name), std::move(fields));
  }

  // union: "union" identifier "{" field_list "}"
  std::unique_ptr<t_union> parse_union() {
    auto range = track_range();
    expect_and_consume(tok::kw_union);
    auto name = parse_identifier();
    auto fields = parse_braced_field_list();
    return actions_.on_union(range, std::move(name), std::move(fields));
  }

  // exception:
  //   error_safety error_kind error_blame
  //     "exception" identifier "{" field_list "}"
  //
  // error_safety: "safe" | /* empty */
  // error_kind: "transient" | "stateful" | "permanent" | /* empty */
  // error_blame: "client" | "server" | /* empty */
  std::unique_ptr<t_exception> parse_exception() {
    auto range = track_range();
    auto safety = try_consume_token(tok::kw_safe) ? t_error_safety::safe
                                                  : t_error_safety::unspecified;
    auto kind = t_error_kind::unspecified;
    switch (token_.kind) {
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
    auto fields = parse_braced_field_list();
    return actions_.on_exception(
        range, safety, kind, blame, std::move(name), std::move(fields));
  }

  t_field_list parse_braced_field_list() {
    expect_and_consume('{');
    auto fields = parse_field_list('}');
    expect_and_consume('}');
    return fields;
  }

  // field_list:
  //     field_list field comma_or_semicolon_optional inline_doc_optional
  //   | /* empty */
  t_field_list parse_field_list(token_kind delimiter) {
    auto fields = t_field_list();
    while (token_.kind != delimiter) {
      fields.emplace_back(parse_field());
    }
    return fields;
  }

  // field:
  //   statement_attrs field_id field_qualifier field_type identifier
  //     field_value annotations
  //
  // field_id: integer ":" | /* empty */
  //
  // field_qualifier: "required" | "optional" | /* empty */
  //
  // field_value: "=" const_value | /* empty */
  std::unique_ptr<t_field> parse_field() {
    auto range = track_range();
    auto attrs = parse_statement_attrs();

    // Parse the field id.
    auto field_id = boost::optional<int64_t>();
    if (auto integer = try_parse_integer()) {
      field_id = *integer;
      expect_and_consume(':');
    }

    // Parse the field qualifier.
    auto qual = t_field_qualifier();
    if (try_consume_token(tok::kw_optional)) {
      qual = t_field_qualifier::optional;
    } else if (try_consume_token(tok::kw_required)) {
      qual = t_field_qualifier::required;
    }

    auto type = parse_field_type();
    auto name_loc = token_.range.begin;
    auto name = parse_identifier();

    // Parse the default value.
    auto value = std::unique_ptr<t_const_value>();
    if (try_consume_token('=')) {
      value = parse_const_value();
    }

    auto annotations = parse_annotations();
    parse_comma_or_semicolon_optional();
    auto doc = parse_inline_doc_optional();
    return actions_.on_field(
        range,
        std::move(attrs),
        field_id,
        qual,
        std::move(type),
        name_loc,
        std::move(name),
        std::move(value),
        std::move(annotations),
        std::move(doc));
  }

  // field_type:
  //     tok::identifier annotations
  //   | base_type annotations
  //   | container_type annotations
  //
  // container_type: list_type | set_type | map_type
  //
  // list_type: "list" "<" field_type ">"
  // set_type: "set" "<" field_type ">"
  // map_type: "map" "<" field_type "," field_type ">"
  //
  // We disallow context-sensitive keywords as field type identifiers.
  // This avoids an ambuguity in the resolution of the function_qualifier
  // return_type part of the function rule, when one of the "oneway",
  // "idempotent" or "readonly" is encountered. It could either resolve
  // the token as function_qualifier or resolve "" as function_qualifier and
  // the token as return_type.
  t_type_ref parse_field_type() {
    auto range = track_range();
    if (const t_base_type* type = try_parse_base_type()) {
      return actions_.on_field_type(*type, parse_annotations());
    }
    switch (token_.kind) {
      case tok::identifier: {
        auto value = consume_token().string_value();
        auto annotations = parse_annotations();
        return actions_.on_field_type(
            range, std::move(value), std::move(annotations));
      }
      case tok::kw_list: {
        consume_token();
        expect_and_consume('<');
        auto element_type = parse_field_type();
        expect_and_consume('>');
        return actions_.on_list_type(
            std::move(element_type), parse_annotations());
      }
      case tok::kw_set: {
        consume_token();
        expect_and_consume('<');
        auto key_type = parse_field_type();
        expect_and_consume('>');
        return actions_.on_set_type(std::move(key_type), parse_annotations());
      }
      case tok::kw_map: {
        consume_token();
        expect_and_consume('<');
        auto key_type = parse_field_type();
        expect_and_consume(',');
        auto value_type = parse_field_type();
        expect_and_consume('>');
        return actions_.on_map_type(
            std::move(key_type), std::move(value_type), parse_annotations());
      }
      default:
        report_expected("type");
    }
  }

  // base_type: "bool" | "byte" | "i16" | "i32" | "i64" | "float" | "double" |
  //            "string" | "binary"
  const t_base_type* try_parse_base_type() {
    auto get_base_type = [this]() -> const t_base_type* {
      switch (token_.kind) {
        case tok::kw_bool:
          return &t_base_type::t_bool();
        case tok::kw_byte:
          return &t_base_type::t_byte();
        case tok::kw_i16:
          return &t_base_type::t_i16();
        case tok::kw_i32:
          return &t_base_type::t_i32();
        case tok::kw_i64:
          return &t_base_type::t_i64();
        case tok::kw_float:
          return &t_base_type::t_float();
        case tok::kw_double:
          return &t_base_type::t_double();
        case tok::kw_string:
          return &t_base_type::t_string();
        case tok::kw_binary:
          return &t_base_type::t_binary();
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

  // enum: "enum" identifier "{" enum_value_list "}"
  //
  // enum_value_list:
  //     enum_value_list enum_value comma_or_semicolon_optional
  //       inline_doc_optional
  //   | /* empty */
  std::unique_ptr<t_enum> parse_enum() {
    auto range = track_range();
    expect_and_consume(tok::kw_enum);
    auto name = parse_identifier();
    expect_and_consume('{');
    auto values = t_enum_value_list();
    while (token_.kind != '}') {
      values.emplace_back(parse_enum_value());
    }
    expect_and_consume('}');
    return actions_.on_enum(range, std::move(name), std::move(values));
  }

  // enum_value_annotated:
  //     statement_attrs identifier annotations
  //   | statement_attrs identifier "=" integer annotations
  std::unique_ptr<t_enum_value> parse_enum_value() {
    auto range = track_range();
    auto attrs = parse_statement_attrs();
    auto name_loc = token_.range.begin;
    auto name = parse_identifier();
    auto value = try_consume_token('=')
        ? boost::optional<int64_t>(parse_integer())
        : boost::none;
    auto annotations = parse_annotations();
    parse_comma_or_semicolon_optional();
    auto doc = parse_inline_doc_optional();
    return actions_.on_enum_value(
        range,
        std::move(attrs),
        name_loc,
        std::move(name),
        value ? &*value : nullptr,
        std::move(annotations),
        std::move(doc));
  }

  // const: "const" field_type identifier "=" const_value
  std::unique_ptr<t_const> parse_const() {
    auto range = track_range();
    expect_and_consume(tok::kw_const);
    auto type = parse_field_type();
    auto name = parse_identifier();
    expect_and_consume('=');
    auto value = parse_const_value();
    return actions_.on_const(range, type, std::move(name), std::move(value));
  }

  // const_value:
  //     bool_constant | integer | double | string_literal
  //   | identifier | const_list | const_map | const_struct
  std::unique_ptr<t_const_value> parse_const_value() {
    auto range = track_range();
    auto s = sign::plus;
    switch (token_.kind) {
      case tok::bool_constant:
        return actions_.on_bool_const(consume_token().bool_value());
      case to_tok('-'):
        s = sign::minus;
        FMT_FALLTHROUGH;
      case to_tok('+'):
        consume_token();
        if (token_.kind == tok::int_constant) {
          return actions_.on_int_const(range, parse_integer(s));
        } else if (token_.kind == tok::float_constant) {
          return actions_.on_double_const(parse_double(s));
        }
        report_expected("number");
        break;
      case tok::int_constant:
        return actions_.on_int_const(range, parse_integer());
      case tok::float_constant:
        return actions_.on_double_const(parse_double());
      case tok::string_literal:
        return actions_.on_string_literal(consume_token().string_value());
      case to_tok('['):
        return parse_const_list();
      case to_tok('{'):
        return parse_const_map();
      default:
        if (auto id = try_parse_identifier()) {
          return token_.kind == '{'
              ? parse_const_struct_body(range, *id)
              : actions_.on_reference_const(range, std::move(*id));
        }
        break;
    }
    report_expected("constant");
  }

  // const_list:
  //     "[" const_list_contents comma_or_semicolon_optional "]"
  //   | "[" "]"
  //
  // const_list_contents:
  //     const_list_contents comma_or_semicolon const_value
  //   | const_value
  std::unique_ptr<t_const_value> parse_const_list() {
    expect_and_consume('[');
    auto list = actions_.on_const_list();
    while (token_.kind != ']') {
      list->add_list(parse_const_value());
      if (!parse_comma_or_semicolon_optional()) {
        break;
      }
    }
    expect_and_consume(']');
    return list;
  }

  // const_map:
  //     "{" const_map_contents comma_or_semicolon_optional "}"
  //   | "{" "}"
  //
  // const_map_contents:
  //     const_map_contents CommaOrSemicolon const_value ":" const_value
  //   | const_value ":" const_value
  std::unique_ptr<t_const_value> parse_const_map() {
    expect_and_consume('{');
    auto map = actions_.on_const_map();
    while (token_.kind != '}') {
      auto key = parse_const_value();
      expect_and_consume(':');
      auto value = parse_const_value();
      map->add_map(std::move(key), std::move(value));
      if (!parse_comma_or_semicolon_optional()) {
        break;
      }
    }
    expect_and_consume('}');
    return map;
  }

  // const_struct:
  //     identifier "{" const_struct_contents comma_or_semicolon_optional "}"
  //   | identifier "{" "}"
  //
  // const_struct_contents:
  //     const_struct_contents comma_or_semicolon identifier "=" const_value
  //   | identifier "=" const_value
  std::unique_ptr<t_const_value> parse_const_struct_body(
      source_range range, std::string id) {
    expect_and_consume('{');
    auto map = actions_.on_const_struct(range, std::move(id));
    while (token_.kind != '}') {
      auto key = actions_.on_string_literal(parse_identifier());
      expect_and_consume('=');
      auto value = parse_const_value();
      map->add_map(std::move(key), std::move(value));
      if (!parse_comma_or_semicolon_optional()) {
        break;
      }
    }
    expect_and_consume('}');
    return map;
  }

  // integer:
  //     int_constant
  //   | "+" int_constant
  //   | "-" int_constant
  boost::optional<int64_t> try_parse_integer(sign s = sign::plus) {
    switch (token_.kind) {
      case to_tok('-'):
        s = sign::minus;
        FMT_FALLTHROUGH;
      case to_tok('+'):
        consume_token();
        if (token_.kind != tok::int_constant) {
          report_expected("integer");
        }
        FMT_FALLTHROUGH;
      case tok::int_constant:
        return actions_.on_integer(s, consume_token().int_value());
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

  // double:
  //     float_constant
  //   | "+" float_constant
  //   | "-" float_constant
  double parse_double(sign s = sign::plus) {
    switch (token_.kind) {
      case to_tok('-'):
        s = sign::minus;
        FMT_FALLTHROUGH;
      case to_tok('+'):
        consume_token();
        if (token_.kind != tok::float_constant) {
          break;
        }
        FMT_FALLTHROUGH;
      case tok::float_constant: {
        double value = consume_token().float_value();
        return s == sign::plus ? value : -value;
      }
      default:
        break;
    }
    report_expected("double");
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
  boost::optional<std::string> try_parse_identifier() {
    auto id = std::string();
    switch (token_.kind) {
      case tok::identifier:
        id = token_.string_value();
        break;
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
      case tok::kw_client: {
        auto s = to_string(token_.kind);
        id = {s.data(), s.size()};
        break;
      }
      default:
        return {};
    }
    consume_token();
    return id;
  }

  std::string parse_identifier() {
    if (auto id = try_parse_identifier()) {
      return *id;
    }
    report_expected("identifier");
  }

  // comma_or_semicolon_optional: comma_or_semicolon | /* empty */
  // comma_or_semicolon: ","  | ";"
  bool parse_comma_or_semicolon_optional() {
    return try_consume_token(',') || try_consume_token(';');
  }

 public:
  parser(lexer& lex, parser_actions& actions, diagnostics_engine& diags)
      : lexer_(lex), actions_(actions), diags_(diags) {}

  bool parse() { return parse_program(); }
};

} // namespace

bool parse(lexer& lex, parser_actions& actions, diagnostics_engine& diags) {
  return parser(lex, actions, diags).parse();
}

} // namespace compiler
} // namespace thrift
} // namespace apache
