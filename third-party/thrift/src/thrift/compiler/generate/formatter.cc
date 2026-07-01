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
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/generate/formatter.h>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parser_core.h>
#include <thrift/compiler/parse/token.h>

// @lint-ignore-every CLANGTIDY clang-diagnostic-nrvo

namespace apache::thrift::compiler {
namespace {

constexpr size_t kIndent = 2;
constexpr size_t kPrintWidth = 80;
constexpr size_t kInvalidCommentId = std::numeric_limits<size_t>::max();

std::string spaces(size_t count) {
  return std::string(count, ' ');
}

std::vector<std::string> split_lines(std::string_view text) {
  std::vector<std::string> lines;
  std::string current;
  for (char c : text) {
    if (c == '\n') {
      lines.push_back(std::move(current));
      current.clear();
    } else {
      current.push_back(c);
    }
  }
  lines.push_back(std::move(current));
  return lines;
}

size_t last_line_length(std::string_view text) {
  const size_t pos = text.rfind('\n');
  return pos == std::string_view::npos ? text.size() : text.size() - pos - 1;
}

std::string indent_multiline(std::string_view text, size_t indent) {
  std::string result;
  const std::string prefix = spaces(indent);
  const auto lines = split_lines(text);
  bool in_block_comment = false;
  char string_quote = '\0';
  for (size_t i = 0; i < lines.size(); ++i) {
    if (i != 0) {
      result.push_back('\n');
    }
    const bool line_starts_in_string = string_quote != '\0';
    if (!in_block_comment && !line_starts_in_string && !lines[i].empty()) {
      result += prefix;
    }
    result += lines[i];
    for (size_t j = 0; j < lines[i].size(); ++j) {
      const char c = lines[i][j];
      if (in_block_comment) {
        if (c == '*' && j + 1 < lines[i].size() && lines[i][j + 1] == '/') {
          in_block_comment = false;
          ++j;
        }
        continue;
      }
      if (string_quote != '\0') {
        if (c == '\\') {
          ++j;
        } else if (c == string_quote) {
          string_quote = '\0';
        }
        continue;
      }
      if (c == '/' && j + 1 < lines[i].size() && lines[i][j + 1] == '/') {
        break;
      }
      if (c == '#') {
        break;
      }
      if (c == '/' && j + 1 < lines[i].size() && lines[i][j + 1] == '*') {
        in_block_comment = true;
        ++j;
      } else if (c == '\'' || c == '"') {
        string_quote = c;
      }
    }
  }
  return result;
}

std::string join(const std::vector<std::string>& parts, std::string_view sep) {
  std::string result;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i != 0) {
      result += sep;
    }
    result += parts[i];
  }
  return result;
}

struct comment {
  std::string text;
  bool line = false;
  size_t start_line = 1;
  size_t start_column = 1;
  size_t end_line = 1;
  size_t end_column = 1;
  size_t id = kInvalidCommentId;
};

enum class token_kind {
  identifier,
  number,
  string_literal,
  symbol,
  eof,
};

struct token {
  token_kind kind = token_kind::eof;
  std::string text;
  char symbol = '\0';
  source_range range;
  size_t start_line = 1;
  size_t start_column = 1;
  size_t end_line = 1;
  size_t end_column = 1;
  std::vector<comment> leading_comments;
  bool leading_blank_line = false;
  std::optional<comment> trailing_comment;

  bool is_symbol(char c) const {
    return kind == token_kind::symbol && symbol == c;
  }
};

bool is_before(const comment& first, const token& second) {
  if (first.end_line == second.start_line) {
    return first.end_column < second.start_column;
  }
  return first.end_line < second.start_line;
}

struct lexed_file {
  std::vector<token> tokens;
  std::vector<comment> trailing_comments;
  size_t comment_count = 0;
};

class formatter_token_store {
 public:
  explicit formatter_token_store(lexed_file file)
      : tokens_(std::move(file.tokens)),
        trailing_comments_(std::move(file.trailing_comments)),
        comment_count_(file.comment_count) {}

  std::span<const token> tokens() const {
    return {tokens_.data(), tokens_.size()};
  }

  std::span<const comment> trailing_comments() const {
    return {trailing_comments_.data(), trailing_comments_.size()};
  }

  size_t comment_count() const { return comment_count_; }

 private:
  std::vector<token> tokens_;
  std::vector<comment> trailing_comments_;
  size_t comment_count_ = 0;
};

class trivia_print_state {
 public:
  class capture {
   public:
    explicit capture(trivia_print_state& state)
        : state_(&state), checkpoint_(state.printed_) {}

    capture(const capture&) = delete;
    capture& operator=(const capture&) = delete;

    capture(capture&& other) noexcept
        : state_(std::exchange(other.state_, nullptr)),
          checkpoint_(std::move(other.checkpoint_)),
          committed_(other.committed_) {}

    capture& operator=(capture&&) = delete;

    ~capture() {
      if (state_ != nullptr && !committed_) {
        state_->printed_ = std::move(checkpoint_);
      }
    }

    void commit() { committed_ = true; }

   private:
    trivia_print_state* state_;
    std::vector<bool> checkpoint_;
    bool committed_ = false;
  };

  explicit trivia_print_state(size_t comment_count) : printed_(comment_count) {}

  void mark_printed(const comment& item) {
    if (item.id == kInvalidCommentId) {
      return;
    }
    if (item.id >= printed_.size()) {
      throw std::runtime_error("formatter comment id is out of range");
    }
    printed_.at(item.id) = true;
  }

  bool is_printed(const comment& item) const {
    return item.id == kInvalidCommentId ||
        (item.id < printed_.size() && printed_.at(item.id));
  }

  capture capture_printing() { return capture(*this); }

 private:
  std::vector<bool> printed_;
};

std::string_view source_text(
    const source_manager& source_mgr, source_range range) {
  return source_mgr.get_text_range(range);
}

std::pair<size_t, size_t> inclusive_end_line_column(
    const source_manager& source_mgr, source_range range) {
  if (range.end.offset() <= range.begin.offset()) {
    const auto resolved = source_mgr.resolve_location(range.begin);
    return {resolved.line(), resolved.column()};
  }
  const auto resolved = source_mgr.resolve_location(range.end + -1);
  return {resolved.line(), resolved.column()};
}

bool is_line_comment_text(std::string_view text) {
  return text.rfind("//", 0) == 0 || text.rfind('#', 0) == 0;
}

comment make_comment(
    const source_manager& source_mgr, trivia_kind kind, source_range range) {
  const auto start = source_mgr.resolve_location(range.begin);
  const auto [end_line, end_column] =
      inclusive_end_line_column(source_mgr, range);
  std::string text(source_text(source_mgr, range));
  const bool line = kind == trivia_kind::line_comment ||
      (kind == trivia_kind::doc_comment && is_line_comment_text(text));
  return {
      .text = std::move(text),
      .line = line,
      .start_line = start.line(),
      .start_column = start.column(),
      .end_line = end_line,
      .end_column = end_column};
}

void append_doc_line_comments(
    const source_manager& source_mgr,
    source_range range,
    std::vector<comment>& comments) {
  const auto start = source_mgr.resolve_location(range.begin);
  const std::string_view text = source_text(source_mgr, range);
  size_t line_number = start.line();
  size_t line_start = 0;
  while (line_start < text.size()) {
    const size_t line_end = text.find('\n', line_start);
    const std::string_view line = line_end == std::string_view::npos
        ? text.substr(line_start)
        : text.substr(line_start, line_end - line_start);
    const size_t slash = line.find("///");
    if (slash != std::string_view::npos) {
      const auto column = (line_start == 0 ? start.column() : 1) + slash;
      std::string comment_text(line.substr(slash));
      comments.push_back(
          {.text = std::move(comment_text),
           .line = true,
           .start_line = line_number,
           .start_column = column,
           .end_line = line_number,
           .end_column = column + line.size() - slash - 1});
    }
    if (line_end == std::string_view::npos) {
      break;
    }
    ++line_number;
    line_start = line_end + 1;
  }
}

std::optional<char> compiler_symbol(tok kind) {
  // @lint-ignore CLANGTIDY clang-diagnostic-switch-enum
  switch (kind) {
    case tok::comma:
      return ',';
    case tok::semi:
      return ';';
    case tok::colon:
      return ':';
    case tok::l_paren:
      return '(';
    case tok::r_paren:
      return ')';
    case tok::l_brace:
      return '{';
    case tok::r_brace:
      return '}';
    case tok::l_square:
      return '[';
    case tok::r_square:
      return ']';
    case tok::less:
      return '<';
    case tok::greater:
      return '>';
    case tok::equal:
      return '=';
    case tok::plus:
      return '+';
    case tok::minus:
      return '-';
    case tok::at:
      return '@';
    default:
      return std::nullopt;
  }
}

token make_formatter_token(
    const ::apache::thrift::compiler::token& parsed,
    const source_manager& source_mgr) {
  token result;
  result.text = std::string(source_text(source_mgr, parsed.range));
  result.range = parsed.range;
  const auto start = source_mgr.resolve_location(parsed.range.begin);
  const auto [end_line, end_column] =
      inclusive_end_line_column(source_mgr, parsed.range);
  result.start_line = start.line();
  result.start_column = start.column();
  result.end_line = end_line;
  result.end_column = end_column;

  if (auto symbol = compiler_symbol(parsed.kind.value)) {
    result.kind = token_kind::symbol;
    result.symbol = *symbol;
  } else if (
      parsed.kind == tok::int_literal || parsed.kind == tok::float_literal) {
    result.kind = token_kind::number;
  } else if (parsed.kind == tok::string_literal) {
    result.kind = token_kind::string_literal;
  } else {
    result.kind = token_kind::identifier;
  }
  return result;
}

bool is_adjacent(const token& first, const token& second) {
  return first.end_line == second.start_line &&
      first.end_column + 1 == second.start_column;
}

std::vector<token> combine_signed_numbers(std::vector<token> tokens) {
  std::vector<token> result;
  result.reserve(tokens.size());
  for (size_t i = 0; i < tokens.size(); ++i) {
    const token& current = tokens.at(i);
    if ((current.is_symbol('-') || current.is_symbol('+')) &&
        i + 1 < tokens.size() && tokens.at(i + 1).kind == token_kind::number &&
        is_adjacent(current, tokens.at(i + 1))) {
      token combined = current;
      combined.kind = token_kind::number;
      combined.symbol = '\0';
      combined.text += tokens.at(i + 1).text;
      combined.range.end = tokens.at(i + 1).range.end;
      combined.end_line = tokens.at(i + 1).end_line;
      combined.end_column = tokens.at(i + 1).end_column;
      result.push_back(std::move(combined));
      ++i;
      continue;
    }
    result.push_back(std::move(tokens.at(i)));
  }
  return result;
}

lexed_file attach_comments(
    std::vector<token> tokens, std::vector<comment> comments) {
  for (size_t i = 0; i < comments.size(); ++i) {
    comments[i].id = i;
  }
  const size_t comment_count = comments.size();
  std::vector<token> attached = std::move(tokens);
  size_t comment_idx = 0;
  for (size_t token_idx = 0; token_idx < attached.size(); ++token_idx) {
    token& tok = attached.at(token_idx);
    std::vector<comment> leading;
    while (comment_idx < comments.size() &&
           is_before(comments[comment_idx], tok)) {
      leading.push_back(std::move(comments[comment_idx]));
      ++comment_idx;
    }

    if (comment_idx < comments.size() && comments[comment_idx].line &&
        tok.end_line == comments[comment_idx].start_line &&
        (token_idx + 1 == attached.size() ||
         tok.end_line != attached.at(token_idx + 1).start_line)) {
      tok.trailing_comment = std::move(comments[comment_idx]);
      ++comment_idx;
    }

    if (token_idx >= 1) {
      const size_t first_line =
          leading.empty() ? tok.start_line : leading.front().start_line;
      tok.leading_blank_line =
          first_line > attached.at(token_idx - 1).end_line + 1;
    }
    tok.leading_comments = std::move(leading);
  }
  std::vector<comment> trailing_comments;
  trailing_comments.reserve(comments.size() - comment_idx);
  while (comment_idx < comments.size()) {
    trailing_comments.push_back(std::move(comments[comment_idx]));
    ++comment_idx;
  }
  return {
      .tokens = std::move(attached),
      .trailing_comments = std::move(trailing_comments),
      .comment_count = comment_count};
}

std::string first_error(const diagnostic_results& results) {
  for (const auto& diag : results.diagnostics()) {
    if (diag.level() == diagnostic_level::error) {
      return diag.str();
    }
  }
  return "failed to lex Thrift source";
}

struct formatter_deprecated_annotations {
  source_range range;
};

struct formatter_structured_annotation {
  source_range range;
};

struct formatter_attributes {
  source_location loc;
  std::vector<formatter_structured_annotation> annotations;
  std::optional<source_range> deprecated_annotations;
};

struct formatter_doc_comment {};

struct formatter_const_value {
  enum class kind {
    atom,
    list,
    map,
    object,
  };

  struct map_entry {
    std::unique_ptr<formatter_const_value> key;
    std::unique_ptr<formatter_const_value> value;
  };

  kind type = kind::atom;
  source_range range;
  std::string atom;
  std::vector<std::unique_ptr<formatter_const_value>> list_values;
  std::vector<map_entry> map_values;
};

struct formatter_type_ref {
  source_range range;
  std::string name;
  std::vector<formatter_type_ref> args;
};

struct formatter_field {
  source_range range;
  std::unique_ptr<formatter_attributes> attrs;
  std::optional<int64_t> id;
  t_field_qualifier qualifier = t_field_qualifier::none;
  formatter_type_ref type;
  identifier name;
  std::unique_ptr<formatter_const_value> value;
};

struct formatter_throws;
struct formatter_sink;
struct formatter_stream;

struct formatter_type_throws_spec {
  formatter_type_ref type;
  std::unique_ptr<formatter_throws> throws;
};

struct formatter_return_clause {
  identifier name;
  formatter_type_ref type;
  std::unique_ptr<formatter_sink> sink;
  std::unique_ptr<formatter_stream> stream;
};

struct formatter_sink {
  source_range range;
  formatter_type_throws_spec sink;
  std::optional<formatter_type_throws_spec> final_response;
};

struct formatter_stream {
  source_range range;
  formatter_type_throws_spec response;
};

struct formatter_throws {
  std::vector<formatter_field> fields;
};

struct formatter_function {
  source_range range;
  std::unique_ptr<formatter_attributes> attrs;
  t_function_qualifier qualifier;
  formatter_return_clause ret;
  identifier name;
  std::vector<formatter_field> params;
  std::unique_ptr<formatter_throws> throws;
  bool performs = false;
};

struct formatter_enum_value {
  source_range range;
  std::unique_ptr<formatter_attributes> attrs;
  identifier name;
  std::optional<int64_t> value;
};

struct formatter_parse_terminator {};

enum class formatter_statement_kind {
  package,
  include,
  cpp_include,
  hs_include,
  namespace_,
  service,
  interaction,
  typedef_,
  struct_,
  union_,
  exception,
  enum_,
  const_,
};

struct formatter_statement {
  source_range range;
  formatter_statement_kind kind;
  std::unique_ptr<formatter_attributes> attrs;
  std::string text;
  std::optional<std::string> extra_text;
  source_range text_range;
  identifier first_identifier;
  identifier second_identifier;
  formatter_type_ref type;
  std::unique_ptr<formatter_const_value> value;
  std::vector<formatter_field> fields;
  std::vector<formatter_function> functions;
  std::vector<formatter_enum_value> enum_values;
  t_error_safety safety = t_error_safety::unspecified;
  t_error_kind error_kind = t_error_kind::unspecified;
  t_error_blame blame = t_error_blame::unspecified;
};

struct formatter_document {
  std::vector<formatter_statement> statements;
};

class formatter_parser_actions {
 public:
  using attributes_type = std::unique_ptr<formatter_attributes>;
  using comment_type = formatter_doc_comment;
  using const_value_type = std::unique_ptr<formatter_const_value>;
  using deprecated_annotations_type =
      std::unique_ptr<formatter_deprecated_annotations>;
  using enum_value_list_type = std::vector<formatter_enum_value>;
  using enum_value_type = formatter_enum_value;
  using field_list_type = std::vector<formatter_field>;
  using field_type = formatter_field;
  using function_list_type = std::vector<formatter_function>;
  using function_type = formatter_function;
  using return_clause_type = formatter_return_clause;
  using sink_type = std::unique_ptr<formatter_sink>;
  using stream_type = std::unique_ptr<formatter_stream>;
  using structured_annotation_list_type =
      std::vector<formatter_structured_annotation>;
  using structured_annotation_type = formatter_structured_annotation;
  using throws_type = std::unique_ptr<formatter_throws>;
  using type_ref_type = formatter_type_ref;
  using type_throws_spec_type = formatter_type_throws_spec;

  attributes_type on_attributes(
      source_location loc,
      std::optional<comment_type> doc,
      structured_annotation_list_type annotations) {
    (void)doc;
    return std::make_unique<formatter_attributes>(
        formatter_attributes{loc, std::move(annotations), std::nullopt});
  }

  deprecated_annotations_type on_deprecated_annotations(source_range range) {
    return std::make_unique<formatter_deprecated_annotations>(
        formatter_deprecated_annotations{range});
  }

  void set_deprecated_annotations(
      attributes_type& attrs, deprecated_annotations_type annotations) {
    if (!attrs) {
      attrs = std::make_unique<formatter_attributes>();
    }
    attrs->deprecated_annotations = annotations->range;
  }

  void set_const_value_src_range(
      const_value_type& value, source_range range) const {
    if (!value) {
      value = std::make_unique<formatter_const_value>();
    }
    value->range = range;
  }

  void add_list_value(const_value_type& list, const_value_type value) const {
    list->list_values.push_back(std::move(value));
  }

  void add_map_value(
      const_value_type& map,
      const_value_type key,
      const_value_type value) const {
    map->map_values.push_back(
        formatter_const_value::map_entry{std::move(key), std::move(value)});
  }

  formatter_document into_document() && { return std::move(document_); }

  void on_program() {}

  void on_package(
      source_range range, attributes_type attrs, std::string_view name) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::package;
    statement.attrs = std::move(attrs);
    statement.text = std::string(name);
    add_statement(std::move(statement));
  }

  void on_include(
      source_range range,
      std::string_view str,
      const std::optional<std::string_view>& alias,
      source_range str_range) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::include;
    statement.text = std::string(str);
    if (alias) {
      statement.extra_text = std::string(*alias);
    }
    statement.text_range = str_range;
    add_statement(std::move(statement));
  }

  void on_cpp_include(source_range range, std::string_view str) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::cpp_include;
    statement.text = std::string(str);
    add_statement(std::move(statement));
  }

  void on_hs_include(source_range range, std::string_view str) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::hs_include;
    statement.text = std::string(str);
    add_statement(std::move(statement));
  }

  void on_namespace(
      source_range range, const identifier& language, std::string_view ns) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::namespace_;
    statement.first_identifier = language;
    statement.text = std::string(ns);
    add_statement(std::move(statement));
  }

  std::optional<comment_type> on_doctext() { return std::nullopt; }

  void on_program_doctext() {}

  comment_type on_inline_doc(source_range, std::string_view) { return {}; }

  structured_annotation_type on_structured_annotation(
      source_range range, std::string_view) {
    return {.range = range};
  }

  structured_annotation_type on_structured_annotation(
      source_range range, const_value_type) {
    return {.range = range};
  }

  void on_service(
      source_range range,
      attributes_type attrs,
      const identifier& name,
      const identifier& base,
      function_list_type functions) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::service;
    statement.attrs = std::move(attrs);
    statement.first_identifier = name;
    statement.second_identifier = base;
    statement.functions = std::move(functions);
    add_statement(std::move(statement));
  }

  void on_interaction(
      source_range range,
      attributes_type attrs,
      const identifier& name,
      function_list_type functions) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::interaction;
    statement.attrs = std::move(attrs);
    statement.first_identifier = name;
    statement.functions = std::move(functions);
    add_statement(std::move(statement));
  }

  function_type on_function(
      source_range range,
      attributes_type attrs,
      t_function_qualifier qualifier,
      return_clause_type ret,
      const identifier& name,
      field_list_type params,
      throws_type throws) {
    return {
        .range = range,
        .attrs = std::move(attrs),
        .qualifier = qualifier,
        .ret = std::move(ret),
        .name = name,
        .params = std::move(params),
        .throws = std::move(throws),
    };
  }

  sink_type on_sink(
      source_range range,
      type_throws_spec_type sink,
      std::optional<type_throws_spec_type> final_response) {
    return std::make_unique<formatter_sink>(
        formatter_sink{range, std::move(sink), std::move(final_response)});
  }

  stream_type on_stream(source_range range, type_throws_spec_type response) {
    return std::make_unique<formatter_stream>(
        formatter_stream{range, std::move(response)});
  }

  type_ref_type on_list_type(source_range range, type_ref_type element_type) {
    return {
        .range = range,
        .name = "list",
        .args = singleton_type_list(std::move(element_type)),
    };
  }

  type_ref_type on_set_type(source_range range, type_ref_type key_type) {
    return {
        .range = range,
        .name = "set",
        .args = singleton_type_list(std::move(key_type)),
    };
  }

  type_ref_type on_map_type(
      source_range range, type_ref_type key_type, type_ref_type value_type) {
    std::vector<formatter_type_ref> args;
    args.push_back(std::move(key_type));
    args.push_back(std::move(value_type));
    return {.range = range, .name = "map", .args = std::move(args)};
  }

  function_type on_performs(source_range range, const identifier& name) {
    return {.range = range, .name = name, .performs = true};
  }

  throws_type on_throws(field_list_type exceptions) {
    return std::make_unique<formatter_throws>(
        formatter_throws{std::move(exceptions)});
  }

  void on_typedef(
      source_range range,
      attributes_type attrs,
      type_ref_type type,
      const identifier& name) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::typedef_;
    statement.attrs = std::move(attrs);
    statement.type = std::move(type);
    statement.first_identifier = name;
    add_statement(std::move(statement));
  }

  void on_struct(
      source_range range,
      attributes_type attrs,
      const identifier& name,
      field_list_type fields) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::struct_;
    statement.attrs = std::move(attrs);
    statement.first_identifier = name;
    statement.fields = std::move(fields);
    add_statement(std::move(statement));
  }

  void on_union(
      source_range range,
      attributes_type attrs,
      const identifier& name,
      field_list_type fields) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::union_;
    statement.attrs = std::move(attrs);
    statement.first_identifier = name;
    statement.fields = std::move(fields);
    add_statement(std::move(statement));
  }

  void on_exception(
      source_range range,
      attributes_type attrs,
      t_error_safety safety,
      t_error_kind kind,
      t_error_blame blame,
      const identifier& name,
      field_list_type fields) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::exception;
    statement.attrs = std::move(attrs);
    statement.safety = safety;
    statement.error_kind = kind;
    statement.blame = blame;
    statement.first_identifier = name;
    statement.fields = std::move(fields);
    add_statement(std::move(statement));
  }

  field_type on_field(
      source_range range,
      attributes_type attrs,
      std::optional<int64_t> id,
      t_field_qualifier qualifier,
      type_ref_type type,
      const identifier& name,
      const_value_type value,
      std::optional<comment_type> doc) {
    (void)doc;
    return {
        .range = range,
        .attrs = std::move(attrs),
        .id = id,
        .qualifier = qualifier,
        .type = std::move(type),
        .name = name,
        .value = std::move(value),
    };
  }

  type_ref_type on_type(source_range range, const t_primitive_type& type) {
    return {.range = range, .name = type.name()};
  }

  type_ref_type on_invalid_type(
      source_range range, const t_primitive_type& type) {
    return on_type(range, type);
  }

  type_ref_type on_type(source_range range, std::string_view name) {
    return {.range = range, .name = std::string(name)};
  }

  void on_enum(
      source_range range,
      attributes_type attrs,
      const identifier& name,
      enum_value_list_type values) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::enum_;
    statement.attrs = std::move(attrs);
    statement.first_identifier = name;
    statement.enum_values = std::move(values);
    add_statement(std::move(statement));
  }

  enum_value_type on_enum_value(
      source_range range,
      attributes_type attrs,
      const identifier& name,
      std::optional<int64_t> value,
      std::optional<comment_type> doc) {
    (void)doc;
    return {
        .range = range,
        .attrs = std::move(attrs),
        .name = name,
        .value = value,
    };
  }

  void on_const(
      source_range range,
      attributes_type attrs,
      type_ref_type type,
      const identifier& name,
      const_value_type value) {
    formatter_statement statement;
    statement.range = range;
    statement.kind = formatter_statement_kind::const_;
    statement.attrs = std::move(attrs);
    statement.type = std::move(type);
    statement.first_identifier = name;
    statement.value = std::move(value);
    add_statement(std::move(statement));
  }

  const_value_type on_const_ref(const identifier&) {
    return std::make_unique<formatter_const_value>(
        formatter_const_value{.type = formatter_const_value::kind::atom});
  }

  const_value_type on_integer(int64_t) {
    return std::make_unique<formatter_const_value>(
        formatter_const_value{.type = formatter_const_value::kind::atom});
  }

  const_value_type on_float(double) {
    return std::make_unique<formatter_const_value>(
        formatter_const_value{.type = formatter_const_value::kind::atom});
  }

  const_value_type on_string_literal(std::string value) {
    return std::make_unique<formatter_const_value>(formatter_const_value{
        .type = formatter_const_value::kind::atom, .atom = std::move(value)});
  }

  const_value_type on_bool_literal(bool) {
    return std::make_unique<formatter_const_value>(
        formatter_const_value{.type = formatter_const_value::kind::atom});
  }

  const_value_type on_list_initializer() {
    return std::make_unique<formatter_const_value>(
        formatter_const_value{.type = formatter_const_value::kind::list});
  }

  const_value_type on_map_initializer() {
    return std::make_unique<formatter_const_value>(
        formatter_const_value{.type = formatter_const_value::kind::map});
  }

  const_value_type on_struct_initializer(source_range, std::string_view name) {
    return std::make_unique<formatter_const_value>(formatter_const_value{
        .type = formatter_const_value::kind::object,
        .atom = std::string(name)});
  }

  int64_t on_integer(source_range, sign s, uint64_t value) {
    if (s == sign::plus) {
      return static_cast<int64_t>(value);
    }
    if (value ==
        static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1) {
      return std::numeric_limits<int64_t>::min();
    }
    return -static_cast<int64_t>(value);
  }

  [[noreturn]] void on_error() { throw formatter_parse_terminator(); }

 private:
  static std::vector<formatter_type_ref> singleton_type_list(
      formatter_type_ref type) {
    std::vector<formatter_type_ref> result;
    result.push_back(std::move(type));
    return result;
  }

  void add_statement(formatter_statement statement) {
    document_.statements.push_back(std::move(statement));
  }

  formatter_document document_;
};

class formatter_token_sink : public parser_token_sink {
 public:
  explicit formatter_token_sink(const source_manager& source_mgr)
      : source_mgr_(source_mgr) {}

  void on_token(
      const ::apache::thrift::compiler::token& consumed,
      const ::apache::thrift::compiler::token&) override {
    if (consumed.kind == tok::inline_doc) {
      return;
    }
    tokens_.push_back(make_formatter_token(consumed, source_mgr_));
  }

  std::vector<token> into_tokens() && { return std::move(tokens_); }

 private:
  const source_manager& source_mgr_;
  std::vector<token> tokens_;
};

struct parsed_formatter_source {
  lexed_file file;
  formatter_document document;
};

parsed_formatter_source parse_source(std::string_view source) {
  source_manager source_mgr;
  const auto source_view = source_mgr.add_virtual_file("<formatter>", source);
  diagnostic_results diagnostics;
  diagnostics_engine diags(
      source_mgr, diagnostics, diagnostic_params::strict());
  std::vector<comment> comments;
  auto on_trivia = [&](trivia_kind kind, source_range range) {
    if (kind == trivia_kind::line_comment ||
        kind == trivia_kind::block_comment ||
        kind == trivia_kind::doc_comment) {
      const std::string_view text = source_text(source_mgr, range);
      if (kind == trivia_kind::doc_comment && text.rfind("///", 0) == 0) {
        append_doc_line_comments(source_mgr, range, comments);
      } else {
        comments.push_back(make_comment(source_mgr, kind, range));
      }
    }
  };
  ::apache::thrift::compiler::lexer lex(
      source_view,
      diags,
      [](std::string_view, source_range) {},
      std::move(on_trivia));

  formatter_parser_actions actions;
  formatter_token_sink token_sink(source_mgr);
  try {
    if (!detail::parser_core<formatter_parser_actions>(
             lex, actions, diags, &token_sink)
             .parse() ||
        diagnostics.has_error()) {
      throw std::runtime_error(first_error(diagnostics));
    }
  } catch (const formatter_parse_terminator&) {
    throw std::runtime_error(first_error(diagnostics));
  }

  formatter_document document = std::move(actions).into_document();
  auto file = attach_comments(
      combine_signed_numbers(std::move(token_sink).into_tokens()),
      std::move(comments));
  return {
      .file = std::move(file),
      .document = std::move(document),
  };
}

std::string format_block_comment(std::string_view text, size_t indent) {
  const auto lines = split_lines(text);
  std::string result;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (i != 0) {
      result.push_back('\n');
    }
    if (i == 0 && !lines[i].empty()) {
      result += spaces(indent);
    }
    result += lines[i];
  }
  return result;
}

std::span<const comment> comment_span(const std::vector<comment>& comments) {
  return {comments.data(), comments.size()};
}

std::string format_leading_comments_impl(
    std::span<const comment> comments,
    const token* token_after,
    size_t indent,
    trivia_print_state* trivia) {
  std::string result;
  for (size_t i = 0; i < comments.size(); ++i) {
    const comment& current = comments[i];
    if (trivia != nullptr) {
      trivia->mark_printed(current);
    }
    const size_t next_start_line = i + 1 < comments.size()
        ? comments[i + 1].start_line
        : (token_after == nullptr ? current.end_line + 1
                                  : token_after->start_line);
    if (current.line) {
      result += spaces(indent);
      result += current.text;
      result.push_back('\n');
      if (next_start_line > current.end_line + 1) {
        result.push_back('\n');
      }
      continue;
    }

    if (next_start_line == current.end_line) {
      result += current.text;
      result.push_back(' ');
      continue;
    }

    result += format_block_comment(current.text, indent);
    if (token_after == nullptr && i + 1 == comments.size()) {
      result.push_back('\n');
    } else if (next_start_line == current.end_line + 1) {
      result.push_back('\n');
    } else {
      result += "\n\n";
    }
  }
  return result;
}

std::string format_leading_comments(
    const std::vector<comment>& comments,
    const token* token_after,
    size_t indent) {
  return format_leading_comments_impl(
      comment_span(comments), token_after, indent, nullptr);
}

std::string trailing_comment_impl(
    const token& tok, trivia_print_state* trivia) {
  if (!tok.trailing_comment) {
    return "";
  }
  if (trivia != nullptr) {
    trivia->mark_printed(*tok.trailing_comment);
  }
  return " " + tok.trailing_comment->text;
}

std::string inline_separator_after(const token& tok) {
  return tok.trailing_comment && tok.trailing_comment->line
      ? "\n" + spaces(kIndent)
      : " ";
}

std::string line_comment_break_after(const token& tok) {
  return tok.trailing_comment && tok.trailing_comment->line
      ? "\n" + spaces(kIndent)
      : "";
}

std::string trailing_comment(const token& tok) {
  return trailing_comment_impl(tok, nullptr);
}

std::string leading_node_prefix_impl(
    const token& tok,
    size_t indent,
    bool preserve_blank,
    trivia_print_state* trivia) {
  std::string result;
  if (preserve_blank && tok.leading_blank_line) {
    result.push_back('\n');
  }
  result += format_leading_comments_impl(
      comment_span(tok.leading_comments), &tok, indent, trivia);
  return result;
}

std::string leading_node_prefix(
    const token& tok, size_t indent, bool preserve_blank = true) {
  return leading_node_prefix_impl(tok, indent, preserve_blank, nullptr);
}

std::string inline_leading_prefix_impl(
    const token& tok, size_t indent, trivia_print_state* trivia) {
  return format_leading_comments_impl(
      comment_span(tok.leading_comments), &tok, indent, trivia);
}

std::string inline_leading_prefix(const token& tok, size_t indent) {
  return inline_leading_prefix_impl(tok, indent, nullptr);
}

struct annotation_body;

struct value {
  enum class kind {
    atom,
    list,
    map,
    object,
  };

  kind type = kind::atom;
  token first;
  std::string atom;
  std::vector<value> list_values;
  std::vector<std::optional<token>> list_separators;
  std::vector<value> map_keys;
  std::vector<token> map_operators;
  std::vector<value> map_values;
  std::vector<std::optional<token>> map_separators;
  std::shared_ptr<annotation_body> object_body;
  std::optional<token> right;
};

struct annotation_entry {
  token key;
  std::optional<token> op;
  std::optional<value> val;
  std::optional<token> separator;
};

struct annotation_body {
  token left;
  std::vector<annotation_entry> entries;
  token right;
};

struct annotation_list {
  token left;
  std::vector<annotation_entry> entries;
  token right;
};

struct annotation_object {
  token at;
  token id;
  std::shared_ptr<annotation_body> body;
};

enum class separator_set {
  comma,
  comma_or_semicolon,
};

struct throws_list;

struct parsed_type {
  token first;
  std::string name;
  std::optional<token> left_angle;
  std::vector<parsed_type> args;
  std::vector<std::optional<token>> arg_separators;
  std::optional<token> right_angle;
  bool ampersand = false;
  std::shared_ptr<throws_list> throws;
};

struct generic_type_list {
  token left;
  std::vector<parsed_type> args;
  std::vector<std::optional<token>> separators;
  token right;
};

struct field_value {
  token equal;
  value val;
  std::optional<annotation_list> annotations;
};

struct field {
  std::vector<annotation_object> leading_annotations;
  std::optional<token> index;
  std::optional<token> colon;
  std::optional<token> requirement;
  parsed_type type;
  std::optional<annotation_list> type_annotations;
  std::optional<token> id;
  std::optional<annotation_list> id_annotations;
  std::optional<field_value> default_value;
  std::optional<token> separator;
};

struct throws_list {
  token throws_keyword;
  token left;
  std::vector<field> fields;
  token right;
};

struct function_decl {
  std::vector<annotation_object> leading_annotations;
  std::optional<token> index;
  std::optional<token> colon;
  std::optional<token> modifier;
  std::vector<parsed_type> return_types;
  std::vector<std::optional<token>> return_type_separators;
  token name;
  token left;
  std::vector<field> params;
  token right;
  std::optional<throws_list> throws;
  std::optional<annotation_list> annotations;
  std::optional<token> separator;
};

class concrete_formatter {
 public:
  explicit concrete_formatter(const formatter_token_store& store)
      : tokens_(store.tokens()),
        trailing_comments_(store.trailing_comments()),
        trivia_(store.comment_count()) {
    set_eof_token();
  }

  std::string print_document(const formatter_document& document) {
    std::vector<std::string> statements;
    statements.reserve(document.statements.size());
    for (const auto& statement : document.statements) {
      statements.push_back(print_statement(statement, 0));
    }

    std::string result = join(statements, "\n");
    if (!trailing_comments_.empty()) {
      if (!result.empty()) {
        result.push_back('\n');
      }
      result += format_leading_comments(trailing_comments_, nullptr, 0);
    }
    if (!result.empty() && result.back() != '\n') {
      result.push_back('\n');
    }
    verify_all_trivia_printed();
    return result;
  }

 private:
  explicit concrete_formatter(std::span<const token> tokens)
      : tokens_(tokens), trivia_(0) {
    set_eof_token();
  }

  void set_eof_token() {
    eof_token_.kind = token_kind::eof;
    eof_token_.start_line = tokens_.empty() ? 1 : tokens_.back().end_line;
    eof_token_.end_line = eof_token_.start_line;
  }

  std::string format_leading_comments(
      const std::vector<comment>& comments,
      const token* token_after,
      size_t indent) const {
    return format_leading_comments_impl(
        comment_span(comments), token_after, indent, &trivia_);
  }

  std::string format_leading_comments(
      std::span<const comment> comments,
      const token* token_after,
      size_t indent) const {
    return format_leading_comments_impl(
        comments, token_after, indent, &trivia_);
  }

  std::string trailing_comment(const token& tok) const {
    return trailing_comment_impl(tok, &trivia_);
  }

  std::string leading_node_prefix(
      const token& tok, size_t indent, bool preserve_blank = true) const {
    return leading_node_prefix_impl(tok, indent, preserve_blank, &trivia_);
  }

  std::string inline_leading_prefix(const token& tok, size_t indent) const {
    return inline_leading_prefix_impl(tok, indent, &trivia_);
  }

  std::string comment_text(const comment& item) const {
    trivia_.mark_printed(item);
    return item.text;
  }

  trivia_print_state::capture capture_trivia_printing() const {
    return trivia_.capture_printing();
  }

  void verify_printed(const comment& item) const {
    if (!trivia_.is_printed(item)) {
      throw std::runtime_error(
          fmt::format("formatter dropped comment trivia: {}", item.text));
    }
  }

  void verify_all_trivia_printed() const {
    for (const token& tok : tokens_) {
      for (const comment& item : tok.leading_comments) {
        verify_printed(item);
      }
      if (tok.trailing_comment) {
        verify_printed(*tok.trailing_comment);
      }
    }
    for (const comment& item : trailing_comments_) {
      verify_printed(item);
    }
  }

  const token& peek(size_t offset = 0) const {
    const size_t idx = pos_ + offset;
    if (idx >= tokens_.size()) {
      return eof_token_;
    }
    return tokens_[idx];
  }

  bool eof() const { return peek().kind == token_kind::eof; }

  token consume() {
    token result = peek();
    if (!eof()) {
      ++pos_;
    }
    return result;
  }

  bool text_is(std::string_view text, size_t offset = 0) const {
    return peek(offset).text == text;
  }

  bool symbol_is(char c, size_t offset = 0) const {
    return peek(offset).is_symbol(c);
  }

  token consume_expected() {
    if (eof()) {
      return peek();
    }
    return consume();
  }

  token consume_dotted_token() {
    token result = consume_expected();
    while (symbol_is('.') && peek(1).kind != token_kind::eof) {
      consume();
      token segment = consume_expected();
      result.text += ".";
      result.text += segment.text;
      result.end_line = segment.end_line;
      result.end_column = segment.end_column;
      if (segment.trailing_comment) {
        result.trailing_comment = segment.trailing_comment;
      }
    }
    return result;
  }

  struct materialized_type {
    parsed_type type;
    std::optional<annotation_list> annotations;
  };

  static bool valid_range(source_range range) {
    return range.begin != source_location{} && range.end != source_location{};
  }

  static bool token_starts_in(
      const token& tok, source_location begin, source_location end) {
    return tok.kind != token_kind::eof &&
        tok.range.begin.offset() >= begin.offset() &&
        tok.range.begin.offset() < end.offset();
  }

  std::span<const token> tokens_between(
      source_location begin, source_location end) const {
    size_t first = 0;
    while (first < tokens_.size() &&
           tokens_[first].range.begin.offset() < begin.offset()) {
      ++first;
    }

    size_t last = first;
    while (last < tokens_.size() &&
           tokens_[last].range.begin.offset() < end.offset()) {
      ++last;
    }
    return tokens_.subspan(first, last - first);
  }

  concrete_formatter formatter_for_range(source_range range) const {
    return concrete_formatter(tokens_between(range.begin, range.end));
  }

  token synthetic_token(std::string text, source_range range = {}) const {
    token tok;
    tok.kind = token_kind::identifier;
    tok.text = std::move(text);
    tok.range = range;
    return tok;
  }

  std::optional<token> token_at_optional(source_location loc) const {
    for (const token& tok : tokens_) {
      if (tok.kind != token_kind::eof && tok.range.begin == loc) {
        return tok;
      }
    }
    return std::nullopt;
  }

  token token_at(source_location loc) const {
    return token_at_optional(loc).value_or(token{});
  }

  token first_token_after(source_location begin, source_location end) const {
    for (const token& tok : tokens_) {
      if (token_starts_in(tok, begin, end)) {
        return tok;
      }
    }
    return {};
  }

  template <typename Predicate>
  std::optional<token> first_token_between(
      source_location begin, source_location end, Predicate predicate) const {
    for (const token& tok : tokens_) {
      if (token_starts_in(tok, begin, end) && predicate(tok)) {
        return tok;
      }
    }
    return std::nullopt;
  }

  std::optional<token> first_text_between(
      std::string_view text, source_location begin, source_location end) const {
    return first_token_between(
        begin, end, [&](const token& tok) { return tok.text == text; });
  }

  std::optional<token> first_symbol_between(
      char symbol, source_location begin, source_location end) const {
    return first_token_between(
        begin, end, [&](const token& tok) { return tok.is_symbol(symbol); });
  }

  token symbol_between(
      char symbol, source_location begin, source_location end) const {
    return first_symbol_between(symbol, begin, end)
        .value_or(synthetic_token(std::string(1, symbol)));
  }

  token last_symbol_between(
      char symbol, source_location begin, source_location end) const {
    std::optional<token> result;
    for (const token& tok : tokens_) {
      if (token_starts_in(tok, begin, end) && tok.is_symbol(symbol)) {
        result = tok;
      }
    }
    return result.value_or(synthetic_token(std::string(1, symbol)));
  }

  token matching_symbol_between(
      char left, char right, source_location begin, source_location end) const {
    int depth = 0;
    for (const token& tok : tokens_) {
      if (!token_starts_in(tok, begin, end)) {
        continue;
      }
      if (tok.is_symbol(left)) {
        ++depth;
        continue;
      }
      if (!tok.is_symbol(right)) {
        continue;
      }
      if (depth > 0) {
        --depth;
      }
      if (depth == 0) {
        return tok;
      }
    }
    return synthetic_token(std::string(1, right));
  }

  std::optional<token> last_separator(source_range range) const {
    return last_separator_after(range.begin, range);
  }

  std::optional<token> last_separator_after(
      source_location begin, source_range range) const {
    std::optional<token> result;
    for (const token& tok : tokens_) {
      if (token_starts_in(tok, begin, range.end) &&
          (tok.is_symbol(';') || tok.is_symbol(','))) {
        result = tok;
      }
    }
    return result;
  }

  token first_code_token(const formatter_statement& statement) const {
    source_location begin = statement.range.begin;
    if (statement.attrs && !statement.attrs->annotations.empty()) {
      begin = statement.attrs->annotations.back().range.end;
    }
    return first_token_after(begin, statement.range.end);
  }

  std::vector<annotation_object> materialize_annotations(
      const formatter_attributes* attrs) const {
    std::vector<annotation_object> result;
    if (attrs == nullptr) {
      return result;
    }
    for (const auto& annotation : attrs->annotations) {
      auto formatter = formatter_for_range(annotation.range);
      auto parsed = formatter.parse_annotation_objects();
      result.insert(
          result.end(),
          std::make_move_iterator(parsed.begin()),
          std::make_move_iterator(parsed.end()));
    }
    return result;
  }

  std::optional<annotation_list> deprecated_annotations(
      const formatter_attributes* attrs) const {
    if (attrs == nullptr || !attrs->deprecated_annotations) {
      return std::nullopt;
    }
    auto formatter = formatter_for_range(*attrs->deprecated_annotations);
    return formatter.parse_annotation_list();
  }

  std::optional<annotation_list> annotation_list_after(
      source_location begin, source_location end) const {
    auto left = first_symbol_between('(', begin, end);
    if (!left) {
      return std::nullopt;
    }
    int depth = 0;
    for (const token& tok : tokens_) {
      if (!token_starts_in(tok, left->range.begin, end)) {
        continue;
      }
      if (tok.is_symbol('(')) {
        ++depth;
      } else if (tok.is_symbol(')')) {
        --depth;
        if (depth == 0) {
          auto formatter =
              formatter_for_range({left->range.begin, tok.range.end});
          return formatter.parse_annotation_list();
        }
      }
    }
    return std::nullopt;
  }

  materialized_type materialize_type(const formatter_type_ref& type_ref) const {
    materialized_type result;
    if (!valid_range(type_ref.range)) {
      return result;
    }
    result.type.first =
        first_token_after(type_ref.range.begin, type_ref.range.end);
    result.type.name =
        type_ref.name.empty() ? result.type.first.text : type_ref.name;
    source_location core_end = result.type.first.range.end;
    if (!type_ref.args.empty()) {
      result.type.left_angle =
          symbol_between('<', result.type.first.range.end, type_ref.range.end);
      result.type.args.reserve(type_ref.args.size());
      result.type.arg_separators.reserve(type_ref.args.size());
      for (size_t i = 0; i < type_ref.args.size(); ++i) {
        auto arg = materialize_type(type_ref.args[i]);
        result.type.args.push_back(std::move(arg.type));
        if (i + 1 < type_ref.args.size()) {
          result.type.arg_separators.push_back(first_symbol_between(
              ',',
              type_ref.args[i].range.end,
              type_ref.args[i + 1].range.begin));
        }
      }
      const auto& last_arg = type_ref.args.back();
      result.type.right_angle =
          last_symbol_between('>', last_arg.range.end, type_ref.range.end);
      core_end = result.type.right_angle->range.end;
    }
    result.annotations = annotation_list_after(core_end, type_ref.range.end);
    return result;
  }

  value materialize_value(const formatter_const_value& const_value) const {
    if (valid_range(const_value.range)) {
      auto formatter = formatter_for_range(const_value.range);
      return formatter.parse_value();
    }
    value result;
    result.type = value::kind::atom;
    result.atom = const_value.atom;
    result.first = synthetic_token(const_value.atom);
    return result;
  }

  field materialize_field(const formatter_field& item) const {
    field result;
    result.leading_annotations = materialize_annotations(item.attrs.get());
    source_location code_begin = item.range.begin;
    if (item.attrs && !item.attrs->annotations.empty()) {
      code_begin = item.attrs->annotations.back().range.end;
    }
    if (item.id) {
      result.index = first_token_between(
          code_begin, item.type.range.begin, [](const token& tok) {
            return tok.kind == token_kind::number;
          });
      if (result.index) {
        result.colon = first_symbol_between(
            ':', result.index->range.end, item.type.range.begin);
      }
    }
    result.requirement =
        first_text_between("optional", code_begin, item.type.range.begin);
    if (!result.requirement) {
      result.requirement =
          first_text_between("required", code_begin, item.type.range.begin);
    }
    auto type = materialize_type(item.type);
    result.type = std::move(type.type);
    result.type_annotations = std::move(type.annotations);
    result.id = token_at_optional(item.name.loc);
    auto annotations = deprecated_annotations(item.attrs.get());
    if (item.value) {
      if (!result.id) {
        throw std::runtime_error("field is missing a name token");
      }
      field_value default_value;
      default_value.equal =
          symbol_between('=', result.id->range.end, item.value->range.begin);
      default_value.val = materialize_value(*item.value);
      default_value.annotations = std::move(annotations);
      result.default_value = std::move(default_value);
    } else {
      result.id_annotations = std::move(annotations);
    }
    result.separator = last_separator(item.range);
    return result;
  }

  parsed_type parse_type_from_range(source_range range) const {
    auto formatter = formatter_for_range(range);
    return formatter.parse_type();
  }

  void add_return_type(
      function_decl& fn,
      std::vector<source_range>& ranges,
      parsed_type type,
      source_range range) const {
    if (!ranges.empty()) {
      fn.return_type_separators.push_back(
          first_symbol_between(',', ranges.back().end, range.begin));
    }
    ranges.push_back(range);
    fn.return_types.push_back(std::move(type));
  }

  throws_list materialize_throws(
      const formatter_throws& item,
      source_location begin,
      source_location end) const {
    throws_list result;
    result.throws_keyword = first_text_between("throws", begin, end)
                                .value_or(synthetic_token("throws"));
    result.left = symbol_between('(', result.throws_keyword.range.end, end);
    result.fields.reserve(item.fields.size());
    for (const auto& field : item.fields) {
      result.fields.push_back(materialize_field(field));
    }
    result.right =
        matching_symbol_between('(', ')', result.left.range.begin, end);
    return result;
  }

  function_decl materialize_function(const formatter_function& item) const {
    function_decl fn;
    fn.leading_annotations = materialize_annotations(item.attrs.get());
    if (item.qualifier == t_function_qualifier::oneway) {
      fn.modifier =
          first_text_between("oneway", item.range.begin, item.name.loc);
    } else if (item.qualifier == t_function_qualifier::readonly) {
      fn.modifier =
          first_text_between("readonly", item.range.begin, item.name.loc);
    } else if (item.qualifier == t_function_qualifier::idempotent) {
      fn.modifier =
          first_text_between("idempotent", item.range.begin, item.name.loc);
    }

    std::vector<source_range> return_ranges;
    if (item.ret.name.loc != source_location{}) {
      parsed_type interaction;
      interaction.first = token_at(item.ret.name.loc);
      interaction.name = interaction.first.text;
      add_return_type(
          fn, return_ranges, std::move(interaction), item.ret.name.range());
    }
    if (valid_range(item.ret.type.range)) {
      auto type = materialize_type(item.ret.type);
      add_return_type(
          fn, return_ranges, std::move(type.type), item.ret.type.range);
    }
    if (item.ret.sink) {
      add_return_type(
          fn,
          return_ranges,
          parse_type_from_range(item.ret.sink->range),
          item.ret.sink->range);
    }
    if (item.ret.stream) {
      add_return_type(
          fn,
          return_ranges,
          parse_type_from_range(item.ret.stream->range),
          item.ret.stream->range);
    }

    fn.name = token_at(item.name.loc);
    fn.left = symbol_between('(', fn.name.range.end, item.range.end);
    fn.params.reserve(item.params.size());
    for (const auto& param : item.params) {
      fn.params.push_back(materialize_field(param));
    }
    fn.right =
        matching_symbol_between('(', ')', fn.left.range.begin, item.range.end);
    if (item.throws) {
      fn.throws =
          materialize_throws(*item.throws, fn.right.range.end, item.range.end);
    }
    fn.annotations = deprecated_annotations(item.attrs.get());
    fn.separator = last_separator(item.range);
    return fn;
  }

  std::string print_performs(
      const formatter_function& item,
      size_t indent,
      bool preserve_blank) const {
    const token performs =
        first_text_between("performs", item.range.begin, item.range.end)
            .value_or(synthetic_token("performs"));
    const token id = token_at(item.name.loc);
    std::string result = leading_token(performs, indent, preserve_blank) +
        inline_separator_after(performs) + inline_token(id, indent);
    result +=
        print_separator_after_line_comment(id, last_separator(item.range), ";");
    return result;
  }

  std::string print_enum_value(
      const formatter_enum_value& item,
      size_t indent,
      bool preserve_blank) const {
    auto annotations = materialize_annotations(item.attrs.get());
    const token id = token_at(item.name.loc);
    std::string result;
    if (!annotations.empty()) {
      result += print_annotation_objects(
          annotations, indent, indent == 0 ? kIndent : 0);
      result.push_back('\n');
    }
    result += leading_node_prefix(id, indent, preserve_blank) + token_text(id);
    token last_token = id;
    if (item.value) {
      const token equal = symbol_between('=', id.range.end, item.range.end);
      const token val = first_token_after(equal.range.end, item.range.end);
      result += inline_separator_after(last_token);
      result += inline_token(equal, indent);
      last_token = equal;
      result += inline_separator_after(last_token);
      result += inline_token(val, indent);
      last_token = val;
    }
    if (auto annotations = deprecated_annotations(item.attrs.get())) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *annotations, indent, last_line_length(result) + 1);
      last_token = annotations->right;
    }
    result += print_separator_after_line_comment(
        last_token, last_separator(item.range), ",");
    return result;
  }

  std::string print_statement(
      const formatter_statement& statement, size_t indent) const {
    auto annotations = materialize_annotations(statement.attrs.get());
    const bool suppress_prefix = !annotations.empty();
    std::string body;
    switch (statement.kind) {
      case formatter_statement_kind::package:
        body = print_package_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::include:
        body = print_include_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::cpp_include:
      case formatter_statement_kind::hs_include:
        body = print_language_include_statement(
            statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::namespace_:
        body = print_namespace_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::const_:
        body = print_const_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::typedef_:
        body = print_typedef_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::enum_:
        body = print_enum_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::struct_:
      case formatter_statement_kind::union_:
        body = print_struct_like_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::exception:
        body = print_exception_statement(statement, indent, suppress_prefix);
        break;
      case formatter_statement_kind::service:
      case formatter_statement_kind::interaction:
        body = print_service_like_statement(statement, indent, suppress_prefix);
        break;
    }
    if (annotations.empty()) {
      return body;
    }
    return print_annotation_objects(annotations, indent) + "\n" + body;
  }

  std::string print_package_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword = first_code_token(statement);
    std::string result = prefixed_token(keyword, indent, suppress_prefix);
    if (auto name = first_token_between(
            keyword.range.end, statement.range.end, [](const token& tok) {
              return tok.kind != token_kind::eof && !tok.is_symbol(';');
            })) {
      result += inline_separator_after(keyword) + inline_token(*name, indent);
      result += print_separator_after_line_comment(
          *name, last_separator(statement.range), "");
      return result;
    }
    result += print_separator(last_separator(statement.range), ";");
    return result;
  }

  std::string print_include_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token include = first_code_token(statement);
    const token path = token_at(statement.text_range.begin);
    std::string result = prefixed_token(include, indent, suppress_prefix) +
        inline_separator_after(include) + inline_token(path, indent);
    token last_token = path;
    if (auto as_keyword =
            first_text_between("as", path.range.end, statement.range.end)) {
      if (auto alias = first_token_between(
              as_keyword->range.end, statement.range.end, [](const token& tok) {
                return tok.kind != token_kind::eof && !tok.is_symbol(';') &&
                    !tok.is_symbol(',');
              })) {
        result += inline_separator_after(path);
        result += inline_token(*as_keyword, indent);
        result += inline_separator_after(*as_keyword);
        result += inline_token(*alias, indent);
        last_token = *alias;
      }
    }
    result += print_separator_after_line_comment(
        last_token, last_separator(statement.range), "");
    return result;
  }

  std::string print_language_include_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token include = first_code_token(statement);
    const token path =
        first_token_after(include.range.end, statement.range.end);
    std::string result = prefixed_token(include, indent, suppress_prefix) +
        inline_separator_after(include) + inline_token(path, indent);
    result += print_separator_after_line_comment(
        path, last_separator(statement.range), "");
    return result;
  }

  std::string print_namespace_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token ns = first_code_token(statement);
    const token language = token_at(statement.first_identifier.loc);
    const token value =
        first_token_after(language.range.end, statement.range.end);
    std::string result = prefixed_token(ns, indent, suppress_prefix) +
        inline_separator_after(ns) + inline_token(language, indent) +
        inline_separator_after(language) + inline_token(value, indent);
    result += print_separator_after_line_comment(
        value, last_separator(statement.range), "");
    return result;
  }

  std::string print_const_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword = first_code_token(statement);
    auto type = materialize_type(statement.type);
    const token id = token_at(statement.first_identifier.loc);
    if (statement.value == nullptr) {
      throw std::runtime_error("const statement is missing a value");
    }
    const token equal = symbol_between('=', id.range.end, statement.range.end);
    value val = materialize_value(*statement.value);
    std::string result = prefixed_token(keyword, indent, suppress_prefix) +
        inline_separator_after(keyword);
    std::string first_value_line;
    size_t type_inline_size = 0;
    {
      auto measurement = capture_trivia_printing();
      first_value_line = value_inline(val);
      if (val.type == value::kind::list || val.type == value::kind::map) {
        first_value_line = first_value_line.empty()
            ? first_value_line
            : std::string(1, first_value_line.front());
      }
      type_inline_size = type_inline(type.type).size();
    }
    const bool force_type_break = type_source_is_block_formatted(type.type) ||
        (!type.type.args.empty() &&
         indent + last_line_length(result) + type_inline_size + 1 +
                 id.text.size() + 3 + first_value_line.size() >
             kPrintWidth);
    result += print_type(
        type.type,
        indent,
        indent + last_line_length(result),
        force_type_break,
        /*allow_long_inline=*/type.type.args.size() > 1,
        /*include_leading=*/true);
    token last_token = last_type_token(type.type);
    if (type.annotations) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *type.annotations, indent, indent + last_line_length(result) + 1);
      last_token = type.annotations->right;
    }
    result += inline_separator_after(last_token);
    result += inline_token(id, indent);
    last_token = id;
    result += inline_separator_after(last_token);
    result += inline_token(equal, indent);
    last_token = equal;
    result += inline_separator_after(last_token);
    result += print_value(val, indent, indent + last_line_length(result));
    last_token = last_value_token(val);
    if (auto annotations = deprecated_annotations(statement.attrs.get())) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *annotations, indent, indent + last_line_length(result) + 1);
      last_token = annotations->right;
    }
    result += print_separator_after_line_comment(
        last_token, last_separator(statement.range), ";");
    return result;
  }

  std::string print_typedef_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword = first_code_token(statement);
    auto type = materialize_type(statement.type);
    const token id = token_at(statement.first_identifier.loc);
    auto trailing_annotations = deprecated_annotations(statement.attrs.get());
    source_location separator_begin = id.range.end;
    if (trailing_annotations) {
      separator_begin = trailing_annotations->right.range.end;
    }
    auto sep = last_separator_after(separator_begin, statement.range);

    std::string result = prefixed_token(keyword, indent, suppress_prefix) +
        inline_separator_after(keyword);
    size_t type_suffix = 1 + id.text.size();
    size_t type_inline_size = 0;
    {
      auto measurement = capture_trivia_printing();
      if (type.annotations) {
        type_suffix +=
            1 + print_annotation_list_inline(*type.annotations).size();
      }
      if (trailing_annotations) {
        type_suffix +=
            1 + print_annotation_list_inline(*trailing_annotations).size();
      }
      type_suffix += print_separator(sep, "").size();
      type_inline_size = type_inline(type.type).size();
    }
    const bool force_type_break = type_source_is_block_formatted(type.type) ||
        (!type.type.args.empty() &&
         indent + last_line_length(result) + type_inline_size + type_suffix >
             kPrintWidth);
    result += print_type(
        type.type,
        indent,
        indent + last_line_length(result),
        force_type_break,
        /*allow_long_inline=*/true,
        /*include_leading=*/true);
    token last_token = last_type_token(type.type);
    if (type.annotations) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *type.annotations, indent, last_line_length(result) + 1);
      last_token = type.annotations->right;
    }
    result += inline_separator_after(last_token);
    result += inline_token(id, indent);
    last_token = id;
    if (trailing_annotations) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *trailing_annotations, indent, last_line_length(result) + 1);
      last_token = trailing_annotations->right;
    }
    result += print_separator_after_line_comment(last_token, sep, "");
    return result;
  }

  std::string print_enum_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword = first_code_token(statement);
    const token id = token_at(statement.first_identifier.loc);
    const token left = symbol_between('{', id.range.end, statement.range.end);
    const token right =
        last_symbol_between('}', left.range.end, statement.range.end);
    std::string result = prefixed_token(keyword, indent, suppress_prefix) +
        inline_separator_after(keyword) + inline_token(id, indent);
    result += inline_separator_after(id);
    result += inline_token(left, indent);
    for (size_t i = 0; i < statement.enum_values.size(); ++i) {
      result += "\n" +
          indent_multiline(
                    print_enum_value(statement.enum_values[i], 0, i != 0),
                    indent + kIndent);
    }
    result += "\n" + spaces(indent) + inline_token(right, indent);
    token last_token = right;
    if (auto annotations = deprecated_annotations(statement.attrs.get())) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *annotations, indent, last_line_length(result) + 1);
      last_token = annotations->right;
    }
    result += print_separator_after_line_comment(
        last_token, last_separator_after(right.range.end, statement.range), "");
    return result;
  }

  std::string print_struct_like_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword = first_code_token(statement);
    const token id = token_at(statement.first_identifier.loc);
    const token left = symbol_between('{', id.range.end, statement.range.end);
    const token right =
        last_symbol_between('}', left.range.end, statement.range.end);
    std::vector<field> fields;
    fields.reserve(statement.fields.size());
    for (const auto& item : statement.fields) {
      fields.push_back(materialize_field(item));
    }
    std::string result = prefixed_token(keyword, indent, suppress_prefix) +
        inline_separator_after(keyword) + inline_token(id, indent);
    result += inline_separator_after(id);
    result += print_fields(left, fields, right, indent);
    token last_token = right;
    if (auto annotations = deprecated_annotations(statement.attrs.get())) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *annotations, indent, last_line_length(result) + 1);
      last_token = annotations->right;
    }
    result += print_separator_after_line_comment(
        last_token, last_separator_after(right.range.end, statement.range), "");
    return result;
  }

  std::string print_exception_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword =
        first_text_between(
            "exception", statement.range.begin, statement.range.end)
            .value_or(synthetic_token("exception"));
    const token first = first_code_token(statement);
    const token id = token_at(statement.first_identifier.loc);
    const token left = symbol_between('{', id.range.end, statement.range.end);
    const token right =
        last_symbol_between('}', left.range.end, statement.range.end);
    std::vector<field> fields;
    fields.reserve(statement.fields.size());
    for (const auto& item : statement.fields) {
      fields.push_back(materialize_field(item));
    }
    std::string result = suppress_prefix ? inline_leading_prefix(first, indent)
                                         : leading_node_prefix(first, indent);
    for (const token& modifier :
         tokens_between(first.range.begin, keyword.range.begin)) {
      result += modifier.range.begin == first.range.begin
          ? token_text(modifier)
          : inline_token(modifier, indent);
      result += inline_separator_after(modifier);
    }
    result += keyword.range.begin == first.range.begin
        ? token_text(keyword)
        : inline_token(keyword, indent);
    result += inline_separator_after(keyword);
    result += inline_token(id, indent);
    result += inline_separator_after(id);
    result += print_fields(left, fields, right, indent);
    token last_token = right;
    if (auto annotations = deprecated_annotations(statement.attrs.get())) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *annotations, indent, last_line_length(result) + 1);
      last_token = annotations->right;
    }
    result += print_separator_after_line_comment(
        last_token, last_separator_after(right.range.end, statement.range), "");
    return result;
  }

  std::string print_service_like_statement(
      const formatter_statement& statement,
      size_t indent,
      bool suppress_prefix) const {
    const token keyword = first_code_token(statement);
    const token id = token_at(statement.first_identifier.loc);
    const token left = symbol_between('{', id.range.end, statement.range.end);
    const token right =
        last_symbol_between('}', left.range.end, statement.range.end);
    std::string result = prefixed_token(keyword, indent, suppress_prefix) +
        inline_separator_after(keyword) + inline_token(id, indent);
    token last_header_token = id;
    if (statement.second_identifier.loc != source_location{}) {
      const token base = token_at(statement.second_identifier.loc);
      const token extends_keyword =
          first_text_between("extends", id.range.end, base.range.begin)
              .value_or(synthetic_token("extends"));
      result += inline_separator_after(last_header_token);
      result += inline_token(extends_keyword, indent);
      result += inline_separator_after(extends_keyword);
      result += inline_token(base, indent);
      last_header_token = base;
    }
    result += inline_separator_after(last_header_token);
    result += inline_token(left, indent);
    for (size_t i = 0; i < statement.functions.size(); ++i) {
      const auto& fn = statement.functions[i];
      std::string member = fn.performs
          ? print_performs(fn, 0, i != 0)
          : print_function(materialize_function(fn), 0, i != 0);
      result += "\n" + indent_multiline(member, indent + kIndent);
    }
    result += "\n" + spaces(indent) + inline_token(right, indent);
    token last_token = right;
    if (auto annotations = deprecated_annotations(statement.attrs.get())) {
      result += inline_separator_after(last_token);
      result += print_annotation_list(
          *annotations, indent, last_line_length(result) + 1);
      last_token = annotations->right;
    }
    result += print_separator_after_line_comment(
        last_token, last_separator_after(right.range.end, statement.range), "");
    return result;
  }

  throws_list parse_throws() {
    throws_list result;
    result.throws_keyword = consume_expected();
    result.left = consume_expected();
    parse_items_until(')', separator_set::comma_or_semicolon, [&] {
      result.fields.push_back(parse_field(/*default_separator=*/','));
    });
    result.right = consume_expected();
    return result;
  }

  field parse_field(char default_separator) {
    field result;
    result.leading_annotations = parse_annotation_objects();
    if (peek().kind == token_kind::number && symbol_is(':', 1)) {
      result.index = consume();
      result.colon = consume();
    }
    if (text_is("optional") || text_is("required")) {
      result.requirement = consume();
    }
    result.type = parse_type();
    if (symbol_is('(')) {
      result.type_annotations = parse_annotation_list();
    }
    if (!eof() && !symbol_is(default_separator) && !symbol_is(',') &&
        !symbol_is(';') && !symbol_is(')') && !symbol_is('}')) {
      result.id = consume_dotted_token();
    }
    if (symbol_is('(')) {
      result.id_annotations = parse_annotation_list();
    }
    if (symbol_is('=')) {
      field_value default_value;
      default_value.equal = consume_expected();
      default_value.val = parse_value();
      if (symbol_is('(')) {
        default_value.annotations = parse_annotation_list();
      }
      result.default_value = std::move(default_value);
    }
    result.separator = consume_statement_separator();
    return result;
  }

  parsed_type parse_type() {
    parsed_type result;
    result.first = consume_expected();
    result.name = result.first.text;
    while (symbol_is('.') && peek(1).kind != token_kind::eof) {
      consume();
      token segment = consume_expected();
      result.name += ".";
      result.name += segment.text;
    }
    if (symbol_is('<')) {
      auto args = parse_type_arguments();
      result.left_angle = std::move(args.left);
      result.args = std::move(args.args);
      result.arg_separators = std::move(args.separators);
      result.right_angle = std::move(args.right);
    }
    if (symbol_is('&')) {
      consume();
      result.ampersand = true;
    }
    if (text_is("throws")) {
      result.throws = std::make_shared<throws_list>(parse_throws());
    }
    return result;
  }

  generic_type_list parse_type_arguments() {
    generic_type_list result;
    result.left = consume_expected();
    parse_items_until('>', separator_set::comma, [&] {
      result.args.push_back(parse_type());
      result.separators.push_back(consume_separator(separator_set::comma));
    });
    if (symbol_is('>')) {
      result.right = consume();
    }
    return result;
  }

  value parse_value() {
    if (symbol_is('[')) {
      return parse_list_value();
    }
    if (symbol_is('{')) {
      return parse_map_value();
    }

    token first = consume_expected();
    value result;
    result.first = first;
    result.atom = first.text;
    while (symbol_is('.') && peek(1).kind != token_kind::eof) {
      consume();
      token segment = consume_expected();
      result.atom += ".";
      result.atom += segment.text;
    }
    if (symbol_is('{') && first.kind == token_kind::identifier) {
      result.type = value::kind::object;
      result.object_body = parse_annotation_body();
      return result;
    }
    result.type = value::kind::atom;
    return result;
  }

  value parse_list_value() {
    value result;
    result.type = value::kind::list;
    result.first = consume_expected();
    parse_items_until(']', separator_set::comma_or_semicolon, [&] {
      result.list_values.push_back(parse_value());
      result.list_separators.push_back(
          consume_separator(separator_set::comma_or_semicolon));
    });
    if (symbol_is(']')) {
      result.right = consume();
    }
    return result;
  }

  value parse_map_value() {
    value result;
    result.type = value::kind::map;
    result.first = consume_expected();
    parse_items_until('}', separator_set::comma_or_semicolon, [&] {
      result.map_keys.push_back(parse_value());
      if (symbol_is(':') || symbol_is('=')) {
        result.map_operators.push_back(consume());
        result.map_values.push_back(parse_value());
      } else {
        token colon;
        colon.text = ":";
        colon.symbol = ':';
        colon.kind = token_kind::symbol;
        result.map_operators.push_back(std::move(colon));
        result.map_values.push_back({});
      }
      result.map_separators.push_back(
          consume_separator(separator_set::comma_or_semicolon));
    });
    if (symbol_is('}')) {
      result.right = consume();
    }
    return result;
  }

  std::vector<annotation_object> parse_annotation_objects() {
    std::vector<annotation_object> annotations;
    while (symbol_is('@')) {
      annotation_object object;
      object.at = consume_expected();
      object.id = consume_dotted_token();
      if (symbol_is('{')) {
        object.body = parse_annotation_body();
      }
      annotations.push_back(std::move(object));
    }
    return annotations;
  }

  std::shared_ptr<annotation_body> parse_annotation_body() {
    auto body = std::make_shared<annotation_body>();
    body->left = consume_expected();
    body->entries = parse_annotation_entries('}');
    body->right = consume_expected();
    return body;
  }

  annotation_list parse_annotation_list() {
    annotation_list list;
    list.left = consume_expected();
    list.entries = parse_annotation_entries(')');
    list.right = consume_expected();
    return list;
  }

  std::vector<annotation_entry> parse_annotation_entries(char close) {
    std::vector<annotation_entry> entries;
    parse_items_until(close, separator_set::comma_or_semicolon, [&] {
      annotation_entry entry;
      entry.key = consume_dotted_token();
      if (symbol_is('=') || symbol_is(':')) {
        entry.op = consume();
        entry.val = parse_value();
      }
      entry.separator = consume_separator(separator_set::comma_or_semicolon);
      entries.push_back(std::move(entry));
    });
    return entries;
  }

  std::optional<token> consume_statement_separator() {
    return consume_separator(separator_set::comma_or_semicolon);
  }

  bool separator_is(separator_set separators) const {
    return symbol_is(',') ||
        (separators == separator_set::comma_or_semicolon && symbol_is(';'));
  }

  std::optional<token> consume_separator(separator_set separators) {
    if (!separator_is(separators)) {
      return std::nullopt;
    }
    return consume();
  }

  template <typename ParseItem>
  void parse_items_until(
      char close, separator_set separators, ParseItem parse_item) {
    while (!eof() && !symbol_is(close)) {
      if (separator_is(separators)) {
        consume();
        continue;
      }
      parse_item();
    }
  }

  std::string keyword_prefix(
      const token& keyword, size_t indent, bool suppress_prefix) const {
    return suppress_prefix ? inline_leading_prefix(keyword, indent)
                           : leading_node_prefix(keyword, indent);
  }

  std::string leading_token(
      const token& tok, size_t indent, bool preserve_blank = true) const {
    return leading_node_prefix(tok, indent, preserve_blank) + token_text(tok);
  }

  std::string prefixed_token(
      const token& tok, size_t indent, bool suppress_prefix) const {
    return keyword_prefix(tok, indent, suppress_prefix) + token_text(tok);
  }

  std::string inline_token(const token& tok, size_t indent) const {
    return inline_leading_prefix(tok, indent) + tok.text +
        trailing_comment(tok);
  }

  std::string token_text(const token& tok) const {
    return tok.text + trailing_comment(tok);
  }

  std::string print_separator(
      const std::optional<token>& sep, std::string_view corrected) const {
    std::string result;
    if (sep) {
      const bool inline_block_comment = !sep->leading_comments.empty() &&
          !sep->leading_comments.back().line &&
          sep->leading_comments.back().end_line == sep->start_line;
      if (inline_block_comment) {
        result.push_back(' ');
      }
      result += format_leading_comments(sep->leading_comments, &*sep, 0);
      if (inline_block_comment && !corrected.empty() && !result.empty() &&
          result.back() == ' ') {
        result.pop_back();
      }
    }
    result += corrected;
    if (sep) {
      result += trailing_comment(*sep);
    }
    return result;
  }

  std::string print_separator_after_line_comment(
      const token& tok,
      const std::optional<token>& sep,
      std::string_view corrected) const {
    std::string result;
    if (tok.trailing_comment && tok.trailing_comment->line &&
        (sep || !corrected.empty())) {
      result += "\n" + spaces(kIndent);
    }
    result += print_separator(sep, corrected);
    return result;
  }

  std::string print_type(
      const parsed_type& ty,
      size_t indent,
      size_t current_column,
      bool force_break = false,
      bool allow_long_inline = false,
      bool include_leading = false) const {
    std::string inline_text;
    {
      auto inline_capture = capture_trivia_printing();
      inline_text = type_inline(ty, include_leading);
      const bool inline_contains_line_break =
          inline_text.find('\n') != std::string::npos;
      const bool source_inline = type_source_is_single_line(ty);
      if (ty.args.empty() && !inline_contains_line_break &&
          (source_inline || !ty.throws ||
           current_column + inline_text.size() <= kPrintWidth)) {
        inline_capture.commit();
        return inline_text;
      }
      if (!ty.args.empty() && !inline_contains_line_break &&
          !type_has_line_trailing_comment(ty) &&
          (source_inline ||
           (!force_break &&
            current_column + inline_text.size() <= kPrintWidth &&
            (allow_long_inline || inline_text.size() <= 52)))) {
        inline_capture.commit();
        return inline_text;
      }
    }

    if (ty.args.empty()) {
      std::string result = type_name_inline(ty, include_leading);
      if (ty.ampersand) {
        result += "&";
      }
      if (ty.throws) {
        result += " " +
            print_throws(
                      *ty.throws, indent, current_column + result.size() + 1);
      }
      return result;
    }

    std::string result =
        type_name_inline(ty, include_leading) + type_left_angle_inline(ty);
    for (size_t i = 0; i < ty.args.size(); ++i) {
      result += "\n" + spaces(indent + kIndent) +
          print_type(
                    ty.args[i],
                    indent + kIndent,
                    indent + kIndent,
                    /*force_break=*/false,
                    /*allow_long_inline=*/true,
                    /*include_leading=*/true);
      result += print_type_arg_separator(ty, i);
    }
    result += "\n" + spaces(indent) + type_right_angle_inline(ty);
    if (ty.ampersand) {
      result += "&";
    }
    if (ty.throws) {
      result +=
          " " + print_throws(*ty.throws, indent, last_line_length(result) + 1);
    }
    return result;
  }

  bool type_source_starts_multiline(const parsed_type& ty) const {
    return ty.left_angle && !ty.args.empty() &&
        ty.left_angle->end_line != ty.args.front().first.start_line;
  }

  bool type_source_is_single_line(const parsed_type& ty) const {
    const token& last_token = ty.throws
        ? ty.throws->right
        : (ty.right_angle ? *ty.right_angle : ty.first);
    return ty.first.start_line == last_token.end_line;
  }

  bool type_source_is_block_formatted(const parsed_type& ty) const {
    if (!ty.left_angle || ty.args.empty() || !ty.right_angle ||
        !type_source_starts_multiline(ty)) {
      return false;
    }
    const size_t arg_column = ty.right_angle->start_column + kIndent;
    size_t previous_line = 0;
    for (const auto& arg : ty.args) {
      if (arg.first.start_column != arg_column ||
          arg.first.start_line <= previous_line) {
        return false;
      }
      previous_line = arg.first.start_line;
    }
    return true;
  }

  bool type_has_line_trailing_comment(const parsed_type& ty) const {
    if (ty.first.trailing_comment && ty.first.trailing_comment->line) {
      return true;
    }
    if (ty.left_angle && ty.left_angle->trailing_comment &&
        ty.left_angle->trailing_comment->line) {
      return true;
    }
    for (const auto& separator : ty.arg_separators) {
      if (separator && separator->trailing_comment &&
          separator->trailing_comment->line) {
        return true;
      }
    }
    return std::any_of(
        ty.args.begin(), ty.args.end(), [this](const parsed_type& arg) {
          return type_has_line_trailing_comment(arg);
        });
  }

  const token& last_type_token(const parsed_type& ty) const {
    if (ty.throws) {
      return ty.throws->right;
    }
    if (ty.right_angle) {
      return *ty.right_angle;
    }
    return ty.first;
  }

  const token& last_value_token(const value& val) const {
    if (val.object_body) {
      return val.object_body->right;
    }
    if (val.right) {
      return *val.right;
    }
    if (!val.map_values.empty()) {
      return last_value_token(val.map_values.back());
    }
    if (!val.list_values.empty()) {
      return last_value_token(val.list_values.back());
    }
    return val.first;
  }

  bool value_source_is_single_line(const value& val) const {
    return val.first.start_line == last_value_token(val).end_line;
  }

  bool value_source_is_block_formatted(const value& val) const {
    if (val.type == value::kind::object && val.object_body) {
      return annotation_body_source_is_block_formatted(*val.object_body);
    }
    if (val.type == value::kind::list) {
      if (val.list_values.empty() || !val.right ||
          !value_source_starts_multiline(val)) {
        return false;
      }
      return value_entries_source_is_block_formatted(
          val.list_values, *val.right);
    }
    if (val.type == value::kind::map) {
      if (val.map_keys.empty() || !val.right ||
          !value_source_starts_multiline(val)) {
        return false;
      }
      return value_entries_source_is_block_formatted(val.map_keys, *val.right);
    }
    return false;
  }

  bool value_entries_source_is_block_formatted(
      const std::vector<value>& entries, const token& right) const {
    const size_t entry_column = right.start_column + kIndent;
    size_t previous_line = 0;
    for (const auto& entry : entries) {
      if (entry.first.start_column != entry_column ||
          entry.first.start_line <= previous_line) {
        return false;
      }
      previous_line = entry.first.start_line;
    }
    return true;
  }

  std::string type_inline(
      const parsed_type& ty, bool include_leading = false) const {
    std::string result = type_name_inline(ty, include_leading);
    if (!ty.args.empty()) {
      result += type_arguments_inline(ty);
    }
    if (ty.ampersand) {
      result += "&";
    }
    if (ty.throws) {
      result += " " + throws_inline(*ty.throws);
    }
    return result;
  }

  std::string type_name_inline(
      const parsed_type& ty, bool include_leading) const {
    std::string result;
    if (include_leading) {
      result += inline_leading_prefix(ty.first, 0);
    }
    result += ty.name;
    result += trailing_comment(ty.first);
    return result;
  }

  std::string type_left_angle_inline(const parsed_type& ty) const {
    return ty.left_angle ? inline_token(*ty.left_angle, 0) : "<";
  }

  std::string type_right_angle_inline(const parsed_type& ty) const {
    return ty.right_angle ? inline_token(*ty.right_angle, 0) : ">";
  }

  std::string type_arguments_inline(const parsed_type& ty) const {
    std::string result = type_left_angle_inline(ty);
    for (size_t i = 0; i < ty.args.size(); ++i) {
      result += type_inline(ty.args[i], /*include_leading=*/true);
      result += print_type_arg_separator(ty, i);
      if (i + 1 < ty.args.size()) {
        result += " ";
      }
    }
    result += type_right_angle_inline(ty);
    return result;
  }

  std::string print_type_arg_separator(
      const parsed_type& ty, size_t index) const {
    const bool last = index + 1 == ty.args.size();
    if (index < ty.arg_separators.size()) {
      return print_separator(ty.arg_separators[index], last ? "" : ",");
    }
    return last ? "" : ",";
  }

  std::string print_value(
      const value& val, size_t indent, size_t current_column) const {
    std::string inline_text;
    bool source_multiline_should_break = false;
    {
      auto inline_capture = capture_trivia_printing();
      inline_text = value_inline(val);
      if (!value_has_comments(val) &&
          (value_source_is_single_line(val) ||
           value_prefers_compact_inline(val) ||
           (value_has_multiline_atom(val) && value_starts_inline(val)))) {
        inline_capture.commit();
        return inline_text;
      }
      source_multiline_should_break = value_source_is_block_formatted(val) ||
          (value_source_starts_multiline(val) &&
           current_column + inline_text.size() > kPrintWidth - 12);
      if (!source_multiline_should_break && !value_has_comments(val) &&
          inline_text.find('\n') == std::string::npos &&
          current_column + inline_text.size() <= kPrintWidth) {
        inline_capture.commit();
        return inline_text;
      }
    }
    if (val.type == value::kind::list) {
      return print_list_value(val, indent);
    }
    if (val.type == value::kind::map) {
      return print_map_value(val, indent);
    }
    if (val.type == value::kind::object && val.object_body) {
      return inline_leading_prefix(val.first, indent) + val.atom +
          print_annotation_body(
                 *val.object_body, indent, current_column + val.atom.size());
    }
    if (val.type == value::kind::atom && value_has_comments(val)) {
      return inline_leading_prefix(val.first, indent) + val.atom +
          trailing_comment(val.first);
    }
    return inline_text;
  }

  std::string value_inline(const value& val) const {
    switch (val.type) {
      case value::kind::atom:
        return val.atom;
      case value::kind::object: {
        std::string result = val.atom;
        if (val.object_body) {
          result += print_annotation_body_inline(*val.object_body);
        }
        return result;
      }
      case value::kind::list: {
        std::vector<std::string> entries;
        entries.reserve(val.list_values.size());
        for (const auto& item : val.list_values) {
          entries.push_back(value_inline(item));
        }
        const bool compact = has_compact_list_separators(val);
        std::string result =
            inline_token(val.first, 0) + join(entries, compact ? "," : ", ");
        if (compact && preserve_trailing_list_separator(val)) {
          result += val.list_separators.back()->text;
        }
        result += val.right ? inline_token(*val.right, 0) : "]";
        return result;
      }
      case value::kind::map: {
        std::vector<std::string> entries;
        entries.reserve(val.map_keys.size());
        for (size_t i = 0; i < val.map_keys.size(); ++i) {
          const std::string op = map_operator_inline(val, i, 0);
          const std::string separator = op == ":" ? ": " : " " + op + " ";
          entries.push_back(
              value_inline(val.map_keys.at(i)) + separator +
              value_inline(val.map_values.at(i)));
        }
        const bool compact = has_compact_map_separators(val);
        std::string result =
            inline_token(val.first, 0) + join(entries, compact ? "," : ", ");
        if (compact && preserve_trailing_map_separator(val)) {
          result += val.map_separators.back()->text;
        }
        result += val.right ? inline_token(*val.right, 0) : "}";
        return result;
      }
    }
    return "";
  }

  std::string map_operator_inline(
      const value& val, size_t index, size_t indent) const {
    if (index >= val.map_operators.size()) {
      return ":";
    }
    return inline_token(val.map_operators.at(index), indent);
  }

  bool value_has_comments(const value& val) const {
    if (!val.first.leading_comments.empty() || val.first.trailing_comment) {
      return true;
    }
    if (val.right &&
        (leading_comments_force_break(
             val.right->leading_comments, *val.right) ||
         val.right->trailing_comment)) {
      return true;
    }
    if (val.type == value::kind::list) {
      for (const auto& separator : val.list_separators) {
        if (separator &&
            (!separator->leading_comments.empty() ||
             separator->trailing_comment)) {
          return true;
        }
      }
      return std::any_of(
          val.list_values.begin(),
          val.list_values.end(),
          [&](const auto& item) { return value_has_comments(item); });
    }
    if (val.type == value::kind::map) {
      for (const auto& separator : val.map_separators) {
        if (separator &&
            (!separator->leading_comments.empty() ||
             separator->trailing_comment)) {
          return true;
        }
      }
      for (const auto& item : val.map_keys) {
        if (value_has_comments(item)) {
          return true;
        }
      }
      for (const auto& item : val.map_values) {
        if (value_has_comments(item)) {
          return true;
        }
      }
    }
    if (val.type == value::kind::object && val.object_body) {
      return annotation_body_has_comments(*val.object_body);
    }
    return false;
  }

  bool annotation_body_has_comments(const annotation_body& body) const {
    if (!body.left.leading_comments.empty() || body.left.trailing_comment ||
        leading_comments_force_break(body.right.leading_comments, body.right) ||
        body.right.trailing_comment) {
      return true;
    }
    return std::any_of(
        body.entries.begin(), body.entries.end(), [&](const auto& entry) {
          return annotation_entry_has_comments(entry);
        });
  }

  bool annotation_body_has_only_right_trailing_comment(
      const annotation_body& body) const {
    if (!body.right.trailing_comment || !body.left.leading_comments.empty() ||
        body.left.trailing_comment ||
        leading_comments_force_break(body.right.leading_comments, body.right)) {
      return false;
    }
    return std::none_of(
        body.entries.begin(), body.entries.end(), [&](const auto& entry) {
          return annotation_entry_has_comments(entry);
        });
  }

  bool annotation_body_has_multiline_atom(const annotation_body& body) const {
    return std::any_of(
        body.entries.begin(), body.entries.end(), [&](const auto& entry) {
          return entry.val && value_has_multiline_atom(*entry.val);
        });
  }

  bool annotation_body_starts_inline(const annotation_body& body) const {
    return !body.entries.empty() &&
        body.left.end_line == body.entries.front().key.start_line;
  }

  bool annotation_body_starts_multiline(const annotation_body& body) const {
    return !body.entries.empty() &&
        body.left.end_line != body.entries.front().key.start_line;
  }

  bool annotation_body_source_is_block_formatted(
      const annotation_body& body) const {
    if (!annotation_body_starts_multiline(body)) {
      return false;
    }
    const size_t entry_column = body.right.start_column + kIndent;
    size_t previous_line = 0;
    for (const auto& entry : body.entries) {
      if (entry.key.start_column != entry_column ||
          entry.key.start_line <= previous_line) {
        return false;
      }
      previous_line = entry.key.start_line;
    }
    return true;
  }

  bool annotation_body_should_print_inline(
      const annotation_body& body,
      std::string_view inline_text,
      size_t current_column) const {
    const bool source_inline = annotation_body_starts_inline(body);
    const bool safe_trailing_comment =
        source_inline && annotation_body_has_only_right_trailing_comment(body);
    if (annotation_body_has_comments(body) && !safe_trailing_comment) {
      return false;
    }
    if (annotation_body_source_is_block_formatted(body)) {
      return false;
    }
    if (annotation_body_has_multiline_atom(body) && source_inline) {
      return true;
    }
    if (inline_text.find('\n') != std::string_view::npos) {
      return false;
    }
    if (source_inline) {
      return true;
    }
    const size_t line_suffix_size = body.right.trailing_comment
        ? body.right.trailing_comment->text.size() + 1
        : 0;
    return current_column + inline_text.size() - line_suffix_size <=
        kPrintWidth;
  }

  bool annotation_list_has_comments(const annotation_list& list) const {
    if (!list.left.leading_comments.empty() || list.left.trailing_comment ||
        leading_comments_force_break(list.right.leading_comments, list.right) ||
        list.right.trailing_comment) {
      return true;
    }
    return std::any_of(
        list.entries.begin(), list.entries.end(), [&](const auto& entry) {
          return annotation_entry_has_comments(entry);
        });
  }

  bool annotation_entry_has_comments(const annotation_entry& entry) const {
    if (!entry.key.leading_comments.empty() || entry.key.trailing_comment) {
      return true;
    }
    if (entry.op &&
        (!entry.op->leading_comments.empty() || entry.op->trailing_comment)) {
      return true;
    }
    if (entry.separator &&
        (!entry.separator->leading_comments.empty() ||
         entry.separator->trailing_comment)) {
      return true;
    }
    return entry.val && value_has_comments(*entry.val);
  }

  bool leading_comments_force_break(
      const std::vector<comment>& comments, const token& token_after) const {
    for (size_t i = 0; i < comments.size(); ++i) {
      const comment& current = comments[i];
      const size_t next_start_line = i + 1 < comments.size()
          ? comments[i + 1].start_line
          : token_after.start_line;
      if (current.line || next_start_line != current.end_line) {
        return true;
      }
    }
    return false;
  }

  bool value_prefers_compact_inline(const value& val) const {
    if (val.type == value::kind::list) {
      return has_compact_list_separators(val);
    }
    if (val.type == value::kind::map) {
      return has_compact_map_separators(val);
    }
    if (val.type == value::kind::object && val.object_body) {
      return has_compact_annotation_separators(val.object_body->entries);
    }
    return false;
  }

  bool value_has_multiline_atom(const value& val) const {
    if (val.type == value::kind::atom) {
      return val.atom.find('\n') != std::string::npos;
    }
    if (val.type == value::kind::list) {
      return std::any_of(
          val.list_values.begin(),
          val.list_values.end(),
          [&](const auto& item) { return value_has_multiline_atom(item); });
    }
    if (val.type == value::kind::map) {
      for (const auto& item : val.map_keys) {
        if (value_has_multiline_atom(item)) {
          return true;
        }
      }
      for (const auto& item : val.map_values) {
        if (value_has_multiline_atom(item)) {
          return true;
        }
      }
    }
    if (val.type == value::kind::object && val.object_body) {
      for (const auto& entry : val.object_body->entries) {
        if (entry.val && value_has_multiline_atom(*entry.val)) {
          return true;
        }
      }
    }
    return false;
  }

  bool value_starts_inline(const value& val) const {
    if (val.type == value::kind::list) {
      return !val.list_values.empty() &&
          val.first.end_line == val.list_values.front().first.start_line;
    }
    if (val.type == value::kind::map) {
      return !val.map_keys.empty() &&
          val.first.end_line == val.map_keys.front().first.start_line;
    }
    if (val.type == value::kind::object && val.object_body &&
        !val.object_body->entries.empty()) {
      return val.object_body->left.end_line ==
          val.object_body->entries.front().key.start_line;
    }
    return false;
  }

  bool value_source_starts_multiline(const value& val) const {
    if (val.type == value::kind::list) {
      return !val.list_values.empty() &&
          val.first.end_line != val.list_values.front().first.start_line;
    }
    if (val.type == value::kind::map) {
      return !val.map_keys.empty() &&
          val.first.end_line != val.map_keys.front().first.start_line;
    }
    if (val.type == value::kind::object && val.object_body &&
        !val.object_body->entries.empty()) {
      return val.object_body->left.end_line !=
          val.object_body->entries.front().key.start_line;
    }
    return false;
  }

  bool has_compact_list_separators(const value& val) const {
    if (val.list_values.size() < 2) {
      return false;
    }
    for (size_t i = 0; i + 1 < val.list_values.size(); ++i) {
      if (i >= val.list_separators.size() || !val.list_separators[i]) {
        return false;
      }
      const token& separator = *val.list_separators[i];
      const token& next = val.list_values[i + 1].first;
      if (separator.end_line != next.start_line ||
          separator.end_column + 1 != next.start_column) {
        return false;
      }
    }
    return true;
  }

  bool has_compact_map_separators(const value& val) const {
    if (val.map_keys.size() < 2) {
      return false;
    }
    for (size_t i = 0; i + 1 < val.map_keys.size(); ++i) {
      if (i >= val.map_separators.size() || !val.map_separators[i]) {
        return false;
      }
      const token& separator = *val.map_separators[i];
      const token& next = val.map_keys[i + 1].first;
      if (separator.end_line != next.start_line ||
          separator.end_column + 1 != next.start_column) {
        return false;
      }
    }
    return true;
  }

  bool preserve_trailing_list_separator(const value& val) const {
    if (val.list_values.empty() ||
        val.list_separators.size() < val.list_values.size() ||
        !val.list_separators.back() || !val.right) {
      return false;
    }
    const token& separator = *val.list_separators.back();
    return separator.end_line == val.right->start_line &&
        separator.end_column + 1 == val.right->start_column;
  }

  bool preserve_trailing_map_separator(const value& val) const {
    if (val.map_keys.empty() ||
        val.map_separators.size() < val.map_keys.size() ||
        !val.map_separators.back() || !val.right) {
      return false;
    }
    const token& separator = *val.map_separators.back();
    return separator.end_line == val.right->start_line &&
        separator.end_column + 1 == val.right->start_column;
  }

  std::string print_list_value(const value& val, size_t indent) const {
    if (val.list_values.empty()) {
      return inline_token(val.first, 0) +
          (val.right ? inline_token(*val.right, 0) : "]");
    }
    std::string result = inline_token(val.first, 0);
    for (size_t i = 0; i < val.list_values.size(); ++i) {
      const auto& item = val.list_values.at(i);
      const size_t item_column = value_source_starts_multiline(item)
          ? kPrintWidth + 1
          : indent + (2 * kIndent) + 1;
      if (!item.first.leading_comments.empty()) {
        value item_without_leading_comments = item;
        item_without_leading_comments.first.leading_comments.clear();
        std::string printed_item = inline_leading_prefix(item.first, 0) +
            print_value(item_without_leading_comments, 0, item_column);
        result += "\n" + indent_multiline(printed_item, indent + kIndent) + ",";
      } else {
        std::string printed_item =
            print_value(item, indent + kIndent, item_column);
        result += "\n" + spaces(indent + kIndent) + printed_item + ",";
      }
      if (i < val.list_separators.size() && val.list_separators.at(i)) {
        result += trailing_comment(*val.list_separators.at(i));
      }
    }
    if (val.right && !val.right->leading_comments.empty()) {
      result += "\n" +
          format_leading_comments(
                    val.right->leading_comments, &*val.right, indent);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
    }
    result +=
        "\n" + spaces(indent) + (val.right ? token_text(*val.right) : "]");
    return result;
  }

  std::string print_map_value(const value& val, size_t indent) const {
    if (val.map_keys.empty()) {
      return inline_token(val.first, 0) +
          (val.right ? inline_token(*val.right, 0) : "}");
    }
    std::string result = inline_token(val.first, 0);
    for (size_t i = 0; i < val.map_keys.size(); ++i) {
      std::string key = value_inline(val.map_keys.at(i));
      const std::string op = map_operator_inline(val, i, 0);
      const std::string separator = op == ":" ? ": " : " " + op + " ";
      const size_t value_column =
          indent + kIndent + key.size() + separator.size();
      if (!val.map_keys.at(i).first.leading_comments.empty()) {
        key.insert(0, inline_leading_prefix(val.map_keys.at(i).first, 0));
        result += "\n" +
            indent_multiline(
                      key + separator +
                          print_value(val.map_values.at(i), 0, value_column),
                      indent + kIndent) +
            ",";
      } else {
        result += "\n" + spaces(indent + kIndent) + key + separator +
            print_value(val.map_values.at(i), indent + kIndent, value_column) +
            ",";
      }
      if (i < val.map_separators.size() && val.map_separators.at(i)) {
        result += trailing_comment(*val.map_separators.at(i));
      }
    }
    if (val.right && !val.right->leading_comments.empty()) {
      result += "\n" +
          format_leading_comments(
                    val.right->leading_comments, &*val.right, indent);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
    }
    result +=
        "\n" + spaces(indent) + (val.right ? token_text(*val.right) : "}");
    return result;
  }

  std::string print_annotation_objects(
      const std::vector<annotation_object>& annotations,
      size_t indent,
      size_t width_offset = 0) const {
    std::vector<std::string> parts;
    parts.reserve(annotations.size());
    for (size_t i = 0; i < annotations.size(); ++i) {
      const auto& annotation = annotations[i];
      std::string part = i == 0 ? leading_node_prefix(annotation.at, indent)
                                : inline_leading_prefix(annotation.at, indent);
      part += "@" + inline_token(annotation.id, indent);
      if (annotation.body) {
        part += print_annotation_body(
            *annotation.body, indent, width_offset + last_line_length(part));
      }
      parts.push_back(std::move(part));
    }
    return join(parts, "\n");
  }

  std::string print_annotation_body(
      const annotation_body& body, size_t indent, size_t current_column) const {
    std::string inline_text;
    {
      auto inline_capture = capture_trivia_printing();
      inline_text = print_annotation_body_inline(body);
      if (annotation_body_should_print_inline(
              body, inline_text, current_column)) {
        inline_capture.commit();
        return inline_text;
      }
    }
    if (body.entries.empty()) {
      std::string result = "{";
      if (body.left.trailing_comment && body.right.leading_comments.empty()) {
        result += "}";
        result += trailing_comment(body.left);
        result += trailing_comment(body.right);
        return result;
      }
      result += trailing_comment(body.left);
      if (!body.right.leading_comments.empty()) {
        result += format_leading_comments(
            body.right.leading_comments, &body.right, indent);
      }
      result += "}";
      result += trailing_comment(body.right);
      return result;
    }
    std::string result = "{";
    for (const auto& entry : body.entries) {
      result += "\n" +
          indent_multiline(
                    print_annotation_entry(entry, 0, indent + kIndent),
                    indent + kIndent) +
          ",";
      if (entry.separator) {
        result += trailing_comment(*entry.separator);
      }
    }
    if (!body.right.leading_comments.empty()) {
      result += "\n" +
          format_leading_comments(
                    body.right.leading_comments, &body.right, indent);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
    }
    result += "\n" + spaces(indent) + "}";
    result += trailing_comment(body.right);
    return result;
  }

  std::string print_annotation_body_inline(const annotation_body& body) const {
    if (body.entries.empty()) {
      if (body.left.trailing_comment && body.right.leading_comments.empty()) {
        std::string result = "{}";
        result += trailing_comment(body.left);
        result += trailing_comment(body.right);
        return result;
      }
      if (!body.right.leading_comments.empty()) {
        std::string result = "{";
        result += format_leading_comments(
            body.right.leading_comments, &body.right, 0);
        result += "}";
        result += trailing_comment(body.right);
        return result;
      }
      return "{}" + trailing_comment(body.right);
    }
    std::vector<std::string> entries;
    std::string suffix;
    for (const auto& entry : body.entries) {
      entries.push_back(print_annotation_entry(entry, 0));
      if (suffix.empty() && entry.val) {
        suffix = value_line_suffix(*entry.val);
      }
    }
    const bool compact = has_compact_annotation_separators(body.entries);
    std::string result =
        inline_token(body.left, 0) + join(entries, compact ? "," : ", ");
    if (compact &&
        preserve_trailing_annotation_separator(body.entries, body.right)) {
      result += body.entries.back().separator->text;
    }
    result += inline_token(body.right, 0);
    return result + suffix;
  }

  std::string print_annotation_list(
      const annotation_list& list, size_t indent, size_t current_column) const {
    std::string inline_text;
    {
      auto inline_capture = capture_trivia_printing();
      inline_text = print_annotation_list_inline(list);
      if (!annotation_list_has_comments(list) &&
          current_column + inline_text.size() <= kPrintWidth &&
          inline_text.size() <= 56) {
        inline_capture.commit();
        return inline_text;
      }
    }
    std::string result = "(";
    for (const auto& entry : list.entries) {
      result += "\n" +
          indent_multiline(
                    print_annotation_entry(entry, 0, indent + kIndent),
                    indent + kIndent) +
          ",";
      if (entry.separator) {
        result += trailing_comment(*entry.separator);
      }
    }
    if (!list.right.leading_comments.empty()) {
      result += "\n" +
          format_leading_comments(
                    list.right.leading_comments, &list.right, indent);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
    }
    result += "\n" + spaces(indent) + ")";
    result += trailing_comment(list.right);
    return result;
  }

  std::string print_annotation_list_inline(const annotation_list& list) const {
    if (list.entries.empty()) {
      return "()" + trailing_comment(list.right);
    }
    std::vector<std::string> entries;
    entries.reserve(list.entries.size());
    for (const auto& entry : list.entries) {
      entries.push_back(print_annotation_entry(entry, 0));
    }
    const bool compact = has_compact_annotation_separators(list.entries);
    std::string result =
        inline_token(list.left, 0) + join(entries, compact ? "," : ", ");
    if (compact &&
        preserve_trailing_annotation_separator(list.entries, list.right)) {
      result += list.entries.back().separator->text;
    }
    result += inline_token(list.right, 0);
    return result;
  }

  bool has_compact_annotation_separators(
      const std::vector<annotation_entry>& entries) const {
    if (entries.size() < 2) {
      return false;
    }
    for (size_t i = 0; i + 1 < entries.size(); ++i) {
      if (!entries[i].separator) {
        return false;
      }
      const token& separator = *entries[i].separator;
      const token& next = entries[i + 1].key;
      if (separator.end_line != next.start_line ||
          separator.end_column + 1 != next.start_column) {
        return false;
      }
    }
    return true;
  }

  bool preserve_trailing_annotation_separator(
      const std::vector<annotation_entry>& entries, const token& right) const {
    if (entries.empty() || !entries.back().separator) {
      return false;
    }
    const token& separator = *entries.back().separator;
    return separator.end_line == right.start_line &&
        separator.end_column + 1 == right.start_column;
  }

  std::string print_annotation_entry(
      const annotation_entry& entry,
      size_t indent,
      size_t width_offset = 0) const {
    std::string result =
        inline_leading_prefix(entry.key, indent) + entry.key.text;
    token last_token = entry.key;
    if (entry.op && entry.val) {
      result += inline_separator_after(last_token);
      result += entry.op->text;
      last_token = *entry.op;
      result += inline_separator_after(last_token);
      result += print_value(
          *entry.val,
          indent,
          width_offset + indent + last_line_length(result) + 4);
    }
    return result;
  }

  std::string value_line_suffix(const value& val) const {
    if (val.type == value::kind::list) {
      for (const auto& separator : val.list_separators) {
        if (separator && separator->trailing_comment) {
          return " " + comment_text(*separator->trailing_comment);
        }
      }
      for (const auto& item : val.list_values) {
        std::string suffix = value_line_suffix(item);
        if (!suffix.empty()) {
          return suffix;
        }
      }
    } else if (val.type == value::kind::map) {
      for (const auto& separator : val.map_separators) {
        if (separator && separator->trailing_comment) {
          return " " + comment_text(*separator->trailing_comment);
        }
      }
      for (const auto& item : val.map_values) {
        std::string suffix = value_line_suffix(item);
        if (!suffix.empty()) {
          return suffix;
        }
      }
    }
    return "";
  }

  std::string print_fields(
      const token& left,
      const std::vector<field>& fields,
      const token& right,
      size_t indent) const {
    if (fields.empty()) {
      if (left.trailing_comment || !right.leading_comments.empty()) {
        std::string result = inline_token(left, indent);
        result.push_back('\n');
        result +=
            format_leading_comments(right.leading_comments, &right, indent);
        result += spaces(indent) + token_text(right);
        return result;
      }
      return inline_token(left, indent) + inline_token(right, indent);
    }

    std::string result = inline_token(left, indent);
    for (size_t i = 0; i < fields.size(); ++i) {
      result +=
          "\n" +
          indent_multiline(
              print_field(fields[i], /*indent=*/0, /*preserve_blank=*/i != 0),
              indent + kIndent);
    }
    result.push_back('\n');
    result += format_leading_comments(
        right.leading_comments, &right, indent + kIndent);
    result += spaces(indent) + token_text(right);
    return result;
  }

  std::string print_field(
      const field& item, size_t indent, bool preserve_blank = true) const {
    std::string result;
    if (!item.leading_annotations.empty()) {
      result += print_annotation_objects(
          item.leading_annotations, indent, indent == 0 ? kIndent : 0);
      result.push_back('\n');
    }
    const token& first = field_first_token(item);
    result += leading_node_prefix(
        first, indent, preserve_blank && item.leading_annotations.empty());
    if (item.index && item.colon) {
      result += token_text(*item.index);
      result += line_comment_break_after(*item.index);
      result += inline_token(*item.colon, indent);
      result += inline_separator_after(*item.colon);
    }
    if (item.requirement) {
      result += item.index ? inline_token(*item.requirement, indent)
                           : token_text(*item.requirement);
      result += inline_separator_after(*item.requirement);
    }
    size_t type_suffix = 0;
    size_t type_inline_size = 0;
    {
      auto measurement = capture_trivia_printing();
      if (item.type_annotations) {
        type_suffix +=
            1 + print_annotation_list_inline(*item.type_annotations).size();
      }
      if (item.id) {
        type_suffix += 1 + item.id->text.size();
      }
      if (item.id_annotations) {
        type_suffix +=
            1 + print_annotation_list_inline(*item.id_annotations).size();
      }
      if (item.default_value) {
        std::string first_value_line = value_inline(item.default_value->val);
        if (item.default_value->val.type == value::kind::list ||
            item.default_value->val.type == value::kind::map) {
          first_value_line = first_value_line.empty()
              ? first_value_line
              : std::string(1, first_value_line.front());
        }
        type_suffix += 3 + first_value_line.size();
        if (item.default_value->annotations) {
          type_suffix += 1 +
              print_annotation_list_inline(*item.default_value->annotations)
                  .size();
        }
      }
      type_inline_size = type_inline(item.type).size();
    }
    type_suffix += 1;
    const size_t effective_indent = indent == 0 ? kIndent : 0;
    const bool force_type_break = type_source_starts_multiline(item.type) ||
        (!item.type.args.empty() &&
         effective_indent + last_line_length(result) + type_inline_size +
                 type_suffix >
             kPrintWidth);
    const bool include_type_leading = item.index || item.requirement;
    result += print_type(
        item.type,
        indent,
        effective_indent + last_line_length(result),
        force_type_break,
        /*allow_long_inline=*/true,
        include_type_leading);
    const token* last_token = &last_type_token(item.type);
    const auto append_after_last_token = [&](std::string suffix) {
      result += inline_separator_after(*last_token);
      result += suffix;
    };
    if (item.type_annotations) {
      append_after_last_token(print_annotation_list(
          *item.type_annotations, indent, last_line_length(result) + 1));
      last_token = &item.type_annotations->right;
    }
    if (item.id) {
      append_after_last_token(inline_token(*item.id, indent));
      last_token = &*item.id;
    }
    if (item.id_annotations) {
      append_after_last_token(print_annotation_list(
          *item.id_annotations, indent, last_line_length(result) + 1));
      last_token = &item.id_annotations->right;
    }
    if (item.default_value) {
      append_after_last_token(inline_token(item.default_value->equal, indent));
      last_token = &item.default_value->equal;
      result += inline_separator_after(*last_token);
      result += print_value(
          item.default_value->val,
          indent,
          effective_indent + last_line_length(result));
      last_token = &last_value_token(item.default_value->val);
      if (item.default_value->annotations) {
        append_after_last_token(print_annotation_list(
            *item.default_value->annotations,
            indent,
            last_line_length(result) + 1));
        last_token = &item.default_value->annotations->right;
      }
    }
    result +=
        print_separator_after_line_comment(*last_token, item.separator, ";");
    return result;
  }

  const token& field_first_token(const field& item) const {
    return item.index
        ? *item.index
        : (item.requirement ? *item.requirement : item.type.first);
  }

  std::string print_function(
      const function_decl& fn,
      size_t indent,
      bool preserve_blank = true) const {
    std::string prefix;
    if (!fn.leading_annotations.empty()) {
      prefix += print_annotation_objects(
          fn.leading_annotations, indent, indent == 0 ? kIndent : 0);
      prefix.push_back('\n');
    }
    const token& first = fn.index
        ? *fn.index
        : (fn.modifier
               ? *fn.modifier
               : (fn.return_types.empty() ? fn.name
                                          : fn.return_types.front().first));
    prefix += leading_node_prefix(first, indent, preserve_blank);
    if (fn.index && fn.colon) {
      prefix += token_text(*fn.index);
      prefix += line_comment_break_after(*fn.index);
      prefix += inline_token(*fn.colon, indent);
      prefix += inline_separator_after(*fn.colon);
    }
    if (fn.modifier) {
      prefix += fn.index ? inline_token(*fn.modifier, indent)
                         : token_text(*fn.modifier);
      prefix += inline_separator_after(*fn.modifier);
    }

    const bool compact_params = has_compact_field_separators(fn.params);
    const bool params_have_separator_comments =
        std::any_of(fn.params.begin(), fn.params.end(), [](const field& param) {
          return param.separator && param.separator->trailing_comment;
        });
    const bool source_params_multiline = !fn.params.empty() &&
        fn.left.end_line != field_first_token(fn.params.front()).start_line;
    const bool preserve_empty_throws_param_break =
        source_params_multiline && fn.throws && fn.throws->fields.empty();
    std::vector<std::string> inline_returns;
    inline_returns.reserve(fn.return_types.size());
    {
      auto measurement = capture_trivia_printing();
      for (const auto& ty : fn.return_types) {
        inline_returns.push_back(type_inline(ty));
      }
    }
    const size_t effective_indent = indent + kIndent;

    const token* last_return_token = nullptr;
    for (size_t i = 0; i < fn.return_types.size(); ++i) {
      if (i != 0) {
        if (last_return_token != nullptr) {
          prefix += print_separator_after_line_comment(
              *last_return_token, fn.return_type_separators.at(i - 1), ",");
        } else {
          prefix += print_separator(fn.return_type_separators.at(i - 1), ",");
        }
        if (fn.return_type_separators.at(i - 1)) {
          prefix +=
              inline_separator_after(*fn.return_type_separators.at(i - 1));
        } else {
          prefix.push_back(' ');
        }
      }
      size_t remaining = 0;
      if (i + 1 == inline_returns.size()) {
        remaining += 1 + fn.name.text.size() + 1;
      }
      const bool force_return_break =
          type_source_starts_multiline(fn.return_types.at(i)) ||
          (!fn.return_types.at(i).args.empty() &&
           effective_indent + last_line_length(prefix) +
                   inline_returns.at(i).size() + remaining >
               kPrintWidth);
      const bool include_type_leading = i != 0 || fn.index || fn.modifier;
      prefix += print_type(
          fn.return_types.at(i),
          indent,
          effective_indent + last_line_length(prefix),
          force_return_break,
          /*allow_long_inline=*/true,
          include_type_leading);
      last_return_token = &last_type_token(fn.return_types.at(i));
    }
    if (last_return_token != nullptr) {
      prefix += inline_separator_after(*last_return_token);
    }
    prefix += inline_token(fn.name, indent);

    {
      auto inline_capture = capture_trivia_printing();
      const std::string params_inline =
          fields_inline(fn.params, ',', compact_params, compact_params);
      const std::string inline_result = prefix +
          std::string(line_comment_break_after(fn.name)) +
          inline_token(fn.left, indent) + params_inline +
          inline_token(fn.right, indent);
      std::string suffix_inline;
      {
        auto measurement = capture_trivia_printing();
        if (fn.throws) {
          suffix_inline += inline_separator_after(fn.right);
          suffix_inline += throws_inline(*fn.throws);
        }
        if (fn.annotations) {
          suffix_inline +=
              inline_separator_after(fn.throws ? fn.throws->right : fn.right);
          suffix_inline += print_annotation_list_inline(*fn.annotations);
        }
        suffix_inline += print_separator(fn.separator, ";");
      }
      const size_t inline_params_column =
          effective_indent + last_line_length(inline_result);
      const size_t inline_full_column =
          effective_indent + last_line_length(inline_result + suffix_inline);
      if (fn.params.empty() && fn.right.leading_comments.empty() &&
          !fn.left.trailing_comment) {
        std::string suffix;
        if (fn.throws) {
          suffix += inline_separator_after(fn.right);
          suffix += print_throws(
              *fn.throws,
              indent,
              effective_indent + last_line_length(inline_result) + 2);
        }
        if (fn.annotations) {
          suffix +=
              inline_separator_after(fn.throws ? fn.throws->right : fn.right);
          suffix += print_annotation_list(
              *fn.annotations,
              indent,
              effective_indent + last_line_length(inline_result) +
                  suffix.size() + 1);
        }
        suffix += print_separator(fn.separator, ";");
        inline_capture.commit();
        return inline_result + suffix;
      }
      if (params_inline.find('\n') == std::string::npos &&
          !params_have_separator_comments &&
          fn.right.leading_comments.empty() &&
          !preserve_empty_throws_param_break && !fn.left.trailing_comment &&
          inline_params_column <= kPrintWidth &&
          (inline_full_column <= kPrintWidth || inline_params_column <= 71)) {
        std::string suffix;
        if (fn.throws) {
          suffix += inline_separator_after(fn.right);
          suffix += print_throws(
              *fn.throws,
              indent,
              effective_indent + last_line_length(inline_result) + 2);
        }
        if (fn.annotations) {
          suffix +=
              inline_separator_after(fn.throws ? fn.throws->right : fn.right);
          suffix += print_annotation_list(
              *fn.annotations,
              indent,
              effective_indent + last_line_length(inline_result) +
                  suffix.size() + 1);
        }
        suffix += print_separator(fn.separator, ";");
        inline_capture.commit();
        return inline_result + suffix;
      }
    }

    std::string result = prefix;
    result += line_comment_break_after(fn.name);
    result += inline_token(fn.left, indent);
    for (const auto& param : fn.params) {
      result += "\n" +
          indent_multiline(
                    print_field_without_separator(param, 0), indent + kIndent) +
          ",";
      if (param.separator) {
        result += trailing_comment(*param.separator);
      }
    }
    if (!fn.right.leading_comments.empty()) {
      result += "\n" +
          format_leading_comments(fn.right.leading_comments, &fn.right, indent);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
    }
    result += "\n" + spaces(indent) + token_text(fn.right);
    const token* last_token = &fn.right;
    if (fn.throws) {
      result += inline_separator_after(*last_token);
      result += print_throws(
          *fn.throws, indent, effective_indent + last_line_length(result) + 2);
      last_token = &fn.throws->right;
    }
    if (fn.annotations) {
      result += inline_separator_after(*last_token);
      result += print_annotation_list(
          *fn.annotations,
          indent,
          effective_indent + last_line_length(result) + 1);
      last_token = &fn.annotations->right;
    }
    result +=
        print_separator_after_line_comment(*last_token, fn.separator, ";");
    return result;
  }

  bool has_compact_field_separators(const std::vector<field>& fields) const {
    if (fields.size() < 2) {
      return false;
    }
    for (size_t i = 0; i + 1 < fields.size(); ++i) {
      if (!fields[i].separator) {
        return false;
      }
      const token& separator = *fields[i].separator;
      const token& next = field_first_token(fields[i + 1]);
      if (separator.end_line != next.start_line ||
          separator.end_column + 1 != next.start_column) {
        return false;
      }
    }
    return true;
  }

  std::string fields_inline(
      const std::vector<field>& fields,
      char sep,
      bool compact = false,
      bool preserve_trailing_separator = false) const {
    std::vector<std::string> parts;
    parts.reserve(fields.size());
    for (const auto& item : fields) {
      parts.push_back(print_field_without_separator(item, 0));
    }
    std::string result =
        join(parts, compact ? std::string(1, sep) : std::string() + sep + " ");
    if (preserve_trailing_separator && !fields.empty() &&
        fields.back().separator) {
      result.push_back(sep);
    }
    return result;
  }

  std::string throws_inline(const throws_list& throws) const {
    return inline_token(throws.throws_keyword, 0) +
        std::string(inline_separator_after(throws.throws_keyword)) +
        inline_token(throws.left, 0) + fields_inline(throws.fields, ',') +
        inline_token(throws.right, 0);
  }

  std::string print_field_without_separator(
      const field& item, size_t indent) const {
    std::string result;
    if (!item.leading_annotations.empty()) {
      result += print_annotation_objects(item.leading_annotations, indent);
      result.push_back('\n');
    }
    if (item.index && item.colon) {
      result += inline_token(*item.index, indent);
      result += line_comment_break_after(*item.index);
      result += inline_token(*item.colon, indent);
      result += inline_separator_after(*item.colon);
    }
    if (item.requirement) {
      result += inline_token(*item.requirement, indent);
      result += inline_separator_after(*item.requirement);
    }
    result += print_type(
        item.type,
        indent,
        indent + last_line_length(result),
        type_source_starts_multiline(item.type),
        /*allow_long_inline=*/true,
        /*include_leading=*/true);
    const token* last_token = &last_type_token(item.type);
    const auto append_after_last_token = [&](std::string suffix) {
      result += inline_separator_after(*last_token);
      result += suffix;
    };
    if (item.type_annotations) {
      append_after_last_token(
          print_annotation_list_inline(*item.type_annotations));
      last_token = &item.type_annotations->right;
    }
    if (item.id) {
      append_after_last_token(inline_token(*item.id, indent));
      last_token = &*item.id;
    }
    if (item.id_annotations) {
      append_after_last_token(
          print_annotation_list_inline(*item.id_annotations));
      last_token = &item.id_annotations->right;
    }
    if (item.default_value) {
      append_after_last_token(inline_token(item.default_value->equal, indent));
      last_token = &item.default_value->equal;
      result += inline_separator_after(*last_token);
      result += value_inline(item.default_value->val);
    }
    return result;
  }

  std::string print_throws(
      const throws_list& throws, size_t indent, size_t current_column) const {
    std::string inline_text;
    {
      auto inline_capture = capture_trivia_printing();
      inline_text = throws_inline(throws);
      const bool inline_contains_line_break =
          inline_text.find('\n') != std::string::npos;
      const bool has_separator_comments = std::any_of(
          throws.fields.begin(), throws.fields.end(), [](const field& item) {
            return item.separator && item.separator->trailing_comment;
          });
      const bool source_throws_inline =
          throws.throws_keyword.start_line == throws.right.end_line;
      if (!inline_contains_line_break && !throws.left.trailing_comment &&
          !has_separator_comments && throws.right.leading_comments.empty() &&
          (throws.fields.empty() || source_throws_inline ||
           current_column + inline_text.size() <= kPrintWidth)) {
        inline_capture.commit();
        return inline_text;
      }
    }
    std::string result = inline_token(throws.throws_keyword, 0);
    result += inline_separator_after(throws.throws_keyword);
    result += inline_token(throws.left, 0);
    for (const auto& field : throws.fields) {
      result += "\n" +
          indent_multiline(
                    print_field_without_separator(field, 0), indent + kIndent) +
          ",";
      if (field.separator) {
        result += trailing_comment(*field.separator);
      }
    }
    if (!throws.right.leading_comments.empty()) {
      result += "\n" +
          format_leading_comments(
                    throws.right.leading_comments, &throws.right, indent);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
    }
    result += "\n" + spaces(indent) + token_text(throws.right);
    return result;
  }

  std::span<const token> tokens_;
  std::span<const comment> trailing_comments_;
  token eof_token_;
  mutable trivia_print_state trivia_;
  size_t pos_ = 0;
};

std::string format_parsed_source(parsed_formatter_source parsed) {
  formatter_token_store tokens(std::move(parsed.file));
  return concrete_formatter(tokens).print_document(parsed.document);
}

} // namespace

std::string format_thrift_source(std::string_view source) {
  if (source.empty()) {
    return "";
  }
  return format_parsed_source(parse_source(source));
}

} // namespace apache::thrift::compiler
