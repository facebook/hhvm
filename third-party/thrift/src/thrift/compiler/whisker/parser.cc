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

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/lexer.h>
#include <thrift/compiler/whisker/parser.h>

#include <fmt/core.h>

#include <cassert>
#include <cstddef>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace whisker {

namespace {

/**
 * A parser_scan_window represents a single pass through lexed tokens by the
 * parser.
 *
 * The start and head of the parser_scan_window are used to track the current
 * position in the input lexed token stream. start represents the beginning of
 * an AST node, while the head is the current position of the parser.
 * parser_scan_window provides methods to advance the head position, peek at the
 * next token, and create new scan windows with different start or head
 * positions.
 *
 * The head advances as parsing progresses, and the start is moved up to the
 * head after each AST node is produced.
 *
 * parser_scan_window is copyable which means that backtracking can be achieved
 * by advancing copies of the window, then disposing the copy if parsing fails.
 */
struct parser_scan_window {
  using cursor = std::vector<token>::const_iterator;

  cursor start;
  cursor head;
  cursor end;

  parser_scan_window(cursor start, cursor head, cursor end)
      : start(start), head(head), end(end) {
    assert(start <= head);
    assert(head < end);
  }

  /**
   * Determines if the head is at the terminal token or not.
   * If this returns false, then both advance() and peek() will return the
   * terminal token in the lex stream.
   */
  bool can_advance() const { return std::next(head) < end; }
  /**
   * Advances the head of the scan by one and returns the encountered
   * token. If the head is already at the end of the input, then there is
   * no changed state and the terminal token is returned.
   */
  const token& advance() {
    if (!can_advance()) {
      // Don't advance past the terminal token (end of lex stream)
      return *head;
    }
    return *head++;
  }
  /**
   * Creates a copy of this parser_scan_window and advances the head n times.
   */
  [[nodiscard]] parser_scan_window next(std::size_t n = 1) {
    return with_head(std::min(std::prev(end), std::next(head, n)));
  }

  /**
   * Returns the next token without advancing the head to the next token.
   */
  [[nodiscard]] const token& peek() const {
    if (!can_advance()) {
      return *head;
    }
    return *head;
  }

  /**
   * Returns a new "fresh" parser_scan_window whose start is moved to the
   * current head. Typically, this indicates that the tokens in the current
   * parser_scan_window have been consumed. The result is an empty
   * parser_scan_window.
   */
  [[nodiscard]] parser_scan_window make_fresh() const {
    return with_start(head);
  }
  [[nodiscard]] bool empty() const { return start == head; }

  [[nodiscard]] parser_scan_window with_start(cursor start) const {
    assert(start <= head);
    return parser_scan_window(start, head, end);
  }
  [[nodiscard]] parser_scan_window with_head(cursor head) const {
    assert(start <= head);
    assert(head < end);
    return parser_scan_window(start, head, end);
  }

  source_location start_location() const { return start->range.begin; }
  source_location head_location() const { return head->range.begin; }
  source_location end_location() const {
    return empty() ? start_location() : std::prev(head)->range.end;
  }
  source_range range() const { return {start_location(), end_location()}; }
};

/**
 * Result of a scan which resulted in a parsed representation of tokens.
 * The (now advanced) parser_scan_window is packaged with the token so the
 * parser can move up its cursor.
 */
template <typename T>
struct [[nodiscard]] parse_result {
  parse_result(T value, const parser_scan_window& advanced)
      : result_{success{std::move(value), advanced.make_fresh()}} {}
  /* implicit */ parse_result(std::nullopt_t) : result_{std::nullopt} {}

  /**
   * Advances the provided parser_scan_window to the last consumed token as part
   * of parsing, then returns the result of parsing. This method ensures that
   * the result of parsing cannot be consumed without advancing the cursor.
   */
  [[nodiscard]] T consume_and_advance(parser_scan_window* scan) && {
    assert(scan);
    assert(has_value());
    *scan = std::move(result_->new_head);
    return std::move(result_->value);
  }

  bool has_value() const { return result_.has_value(); }
  explicit operator bool() const { return has_value(); }

 private:
  struct success {
    T value;
    parser_scan_window new_head;
  };
  std::optional<success> result_;
};

/**
 * Recursive descent parser for Whisker templates that outputs
 * whisker::ast::root if successful.
 *
 * Parsing for production rules of the grammar are implemented as member
 * functions named parse_<rule>(). Each such method includes a specification of
 * the grammar being parsed.
 *
 * Parsing begins at the parse_root() function.
 *
 * The grammar is descibed with PEG (Parsing Expression Grammar) rules,
 * following a syntax closely resembling pest.rs:
 *   https://pest.rs/
 *
 * Production rules are of the form:
 *
 *     rule → { <term> <op> <term> ... }
 *
 * <term> represents other rules. <op> represents a combinator of rules. Here
 * are the supported set of combinators:
 *
 *     a ~ b — exactly one a followed by exactly one b.
 *     a* — zero or more repetitions of a.
 *     a+ — one or more repetitions of a.
 *     a? — exactly zero or one repetition of a.
 *     !a — assert that there is no match to a (without consuming input).
 *     a | b — exactly one a or one b (a matches first)
 *     (a <op> b) — parentheses to disambiguate groups of rules and combinators.
 */
class parser {
 private:
  std::vector<token> tokens_;
  diagnostics_engine& diags_;

  // Reports an error without failing the parse.
  template <typename... T>
  void report_error(
      const parser_scan_window& scan,
      fmt::format_string<T...> msg,
      T&&... args) {
    diags_.error(scan.head_location(), msg, std::forward<T>(args)...);
  }

  struct parse_error : std::exception {};

  template <typename... T>
  [[noreturn]] void report_fatal_error(
      const parser_scan_window& scan,
      fmt::format_string<T...> msg,
      T&&... args) {
    report_error(scan, msg, std::forward<T>(args)...);
    throw parse_error();
  }

  [[noreturn]] void report_expected(
      const parser_scan_window& scan, std::string_view expected) {
    report_fatal_error(
        scan,
        "expected {} but found {}",
        expected,
        to_string(scan.peek().kind));
  }

  bool try_consume_token(parser_scan_window* scan, tok kind) {
    assert(scan);
    if (scan->peek().kind == kind) {
      scan->advance();
      return true;
    }
    return false;
  }

  // root → { body* }
  std::optional<ast::root> parse_root(parser_scan_window scan) {
    try {
      auto original_scan = scan;
      ast::bodies bodies;
      while (scan.can_advance()) {
        if (parse_result body = parse_body(scan)) {
          bodies.emplace_back(std::move(body).consume_and_advance(&scan));
        } else {
          report_expected(scan, "text, template, or comment");
        }
      }
      return ast::root{original_scan.start_location(), std::move(bodies)};
    } catch (const parse_error&) {
      // the error should already have been reported via the diagnostics
      // engine
      return std::nullopt;
    }
  }

  // body → { text | template | comment }
  parse_result<ast::body> parse_body(parser_scan_window scan) {
    assert(scan.empty());
    std::optional<ast::body> body;
    if (parse_result text = parse_text(scan)) {
      body = std::move(text).consume_and_advance(&scan);
    } else if (parse_result templ = parse_template(scan)) {
      detail::variant_match(
          std::move(templ).consume_and_advance(&scan),
          [&](ast::variable&& variable) { body = std::move(variable); },
          [&](ast::section_block&& section_block) {
            body = std::move(section_block);
          },
          [&](ast::partial_apply&& partial_apply) {
            body = std::move(partial_apply);
          });
    } else if (parse_result comment = parse_comment(scan)) {
      body = std::move(comment).consume_and_advance(&scan);
    }
    if (!body.has_value()) {
      return std::nullopt;
    }
    return {std::move(*body), scan};
  }

  // text → { <raw text until we see a template> }
  parse_result<ast::text> parse_text(parser_scan_window scan) {
    assert(scan.empty());
    std::string result;
    while (scan.can_advance()) {
      const token& t = scan.peek();
      if (t.kind != tok::text && t.kind != tok::newline) {
        break;
      }
      result += t.string_value();
      scan.advance();
    }
    return result.empty() ? std::nullopt
                          : parse_result{ast::text{scan.range(), result}, scan};
  }

  // comment → { basic-comment | escaped-comment }
  // basic-comment → { "{{!" ~ <raw text until we see "}}"> ~ "}}" }
  // escaped-comment → { "{{!--" ~ <raw text until we see "--}}"> ~ "--}}" }
  //
  // NOTE: the difference between basic-comment and escaped-comment is dealt
  // with by the lexer already.
  parse_result<ast::comment> parse_comment(parser_scan_window scan) {
    assert(scan.empty());
    if (scan.advance().kind != tok::open) {
      return std::nullopt;
    }
    if (scan.advance().kind != tok::bang) {
      return std::nullopt;
    }
    const auto& text = scan.peek();
    switch (text.kind) {
      case tok::text:
        scan.advance();
        break;
      case tok::close:
        // empty comment
        scan.advance();
        return {ast::comment{scan.range(), ""}, scan};
      default:
        report_expected(scan, "comment text");
    }
    if (scan.peek().kind != tok::close) {
      report_expected(scan, fmt::format("{} to close comment", tok::close));
    }
    scan.advance();

    return {ast::comment{scan.range(), std::string(text.string_value())}, scan};
  }

  using template_body =
      std::variant<ast::variable, ast::section_block, ast::partial_apply>;
  // template → { variable | section-block | partial-apply }
  parse_result<template_body> parse_template(parser_scan_window scan) {
    assert(scan.empty());
    if (scan.peek().kind != tok::open) {
      return std::nullopt;
    }
    switch (scan.next().peek().kind) {
      case tok::bang:
        // this is a comment so don't fail the parse
        [[fallthrough]];
      case tok::slash:
        // parse_template can be called recursively, which means that seeing
        // tok::slash implies that the parent node is closing (so don't fail the
        // parse).
        return std::nullopt;
      default:
        // continue parsing as a template or fail the parse!
        break;
    }

    std::optional<template_body> templ;
    if (parse_result variable = parse_variable(scan)) {
      templ = std::move(variable).consume_and_advance(&scan);
    } else if (parse_result section_block = parse_section_block(scan)) {
      templ = std::move(section_block).consume_and_advance(&scan);
    } else if (parse_result partial_apply = parse_partial_apply(scan)) {
      templ = std::move(partial_apply).consume_and_advance(&scan);
    }
    if (!templ.has_value()) {
      report_expected(
          scan, "variable, section-block, or partial-apply in template");
    }
    return {std::move(*templ), scan};
  }

  // variable → { "{{" ~ variable-lookup ~ "}}" }
  parse_result<ast::variable> parse_variable(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_token(&scan, tok::open)) {
      return std::nullopt;
    }
    switch (scan.peek().kind.value) {
      case tok::bang:
        // this is a comment
        [[fallthrough]];
      case tok::pound:
      case tok::caret:
        // this is a section-block-open
        [[fallthrough]];
      case tok::slash:
        // this is a section-block-close
        return std::nullopt;
      case tok::gt:
        // this is a partial-apply
        return std::nullopt;
      default:
        // continue parsing as a variable (and fail if it's not!)
        break;
    }
    scan = scan.make_fresh();

    parse_result variable_lookup = parse_variable_lookup(scan);
    if (!variable_lookup.has_value()) {
      report_expected(scan, "variable-lookup in variable");
    }
    ast::variable_lookup lookup =
        std::move(variable_lookup).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_expected(scan, fmt::format("{} to close variable", tok::close));
    }
    return {
        ast::variable{scan.with_start(scan_start).range(), std::move(lookup)},
        scan};
  }

  // variable-lookup → { "." | (identifier ~ ("." ~ identifier)*) }
  parse_result<ast::variable_lookup> parse_variable_lookup(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;
    if (try_consume_token(&scan, tok::dot)) {
      return {
          ast::variable_lookup{
              scan.with_start(scan_start).range(),
              ast::variable_lookup::this_ref{}},
          scan};
    }

    scan = scan.make_fresh();
    const token& first_id = scan.advance();
    if (first_id.kind != tok::identifier) {
      return std::nullopt;
    }
    std::vector<ast::identifier> path;
    path.emplace_back(
        ast::identifier{first_id.range, std::string(first_id.string_value())});

    while (try_consume_token(&scan, tok::dot)) {
      const token& id_part = scan.peek();
      if (id_part.kind != tok::identifier) {
        report_expected(scan, "identifier in variable-lookup");
      }
      scan.advance();
      path.emplace_back(
          ast::identifier{id_part.range, std::string(id_part.string_value())});
    }

    auto range = scan.with_start(scan_start).range();
    return {
        ast::variable_lookup{range, ast::lookup_path{range, std::move(path)}},
        scan};
  }

  // section-block → { section-block-open ~ template ~ section-block-close }
  // section-block-open → { "{{" ~ ("#" | "^") ~ variable-lookup ~ "}}" }
  // section-block-close → { "{{" ~ "/" ~ variable-lookup ~ "}}" }
  //
  // NOTE: the variable-lookups must match between open and close
  parse_result<ast::section_block> parse_section_block(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;
    if (!try_consume_token(&scan, tok::open)) {
      return std::nullopt;
    }
    bool is_inverted = try_consume_token(&scan, tok::caret);
    if (!is_inverted) {
      if (!try_consume_token(&scan, tok::pound)) {
        // neither "#" nor "^" so this is not a section block
        return std::nullopt;
      }
    }

    parse_result lookup_at_open = parse_variable_lookup(scan.make_fresh());
    if (!lookup_at_open.has_value()) {
      report_expected(scan, "variable-lookup to open section-block");
    }
    ast::variable_lookup open =
        std::move(lookup_at_open).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_expected(
          scan, fmt::format("{} to open section-block", tok::close));
    }

    scan = scan.make_fresh();
    ast::bodies bodies;
    while (parse_result body = parse_body(scan)) {
      bodies.emplace_back(std::move(body).consume_and_advance(&scan));
    }

    if (!try_consume_token(&scan, tok::open)) {
      report_expected(
          scan,
          fmt::format(
              "{} to close section-block '{}'", tok::open, open.path_string()));
    }
    if (!try_consume_token(&scan, tok::slash)) {
      report_expected(
          scan,
          fmt::format(
              "{} to close section-block '{}'",
              tok::slash,
              open.path_string()));
    }

    parse_result lookup_at_close = parse_variable_lookup(scan.make_fresh());
    if (!lookup_at_close.has_value()) {
      report_expected(
          scan,
          fmt::format(
              "variable-lookup to close section-block '{}'",
              open.path_string()));
    }
    ast::variable_lookup close =
        std::move(lookup_at_close).consume_and_advance(&scan);

    bool should_fail = false;
    if (open.path_string() != close.path_string()) {
      should_fail = true;
      report_error(
          scan,
          "section-block opening '{}' does not match closing '{}'",
          open.path_string(),
          close.path_string());
    }
    if (!try_consume_token(&scan, tok::close)) {
      should_fail = true;
      report_error(
          scan,
          "expected {} to close section-block '{}'",
          tok::close,
          open.path_string());
    }
    if (should_fail) {
      throw parse_error();
    }

    return {
        ast::section_block{
            scan.with_start(scan_start).range(),
            is_inverted,
            std::move(open),
            std::move(bodies),
        },
        scan};
  }

  // partial-apply → { "{{" ~ ">" ~ partial-lookup ~ "}}" }
  parse_result<ast::partial_apply> parse_partial_apply(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_token(&scan, tok::open)) {
      return std::nullopt;
    }
    if (!try_consume_token(&scan, tok::gt)) {
      return std::nullopt;
    }
    scan = scan.make_fresh();

    parse_result partial_lookup = parse_partial_lookup(scan.make_fresh());
    if (!partial_lookup.has_value()) {
      report_expected(scan, "partial-lookup in partial-apply");
    }
    ast::lookup_path lookup =
        std::move(partial_lookup).consume_and_advance(&scan);

    if (!try_consume_token(&scan, tok::close)) {
      report_expected(
          scan,
          fmt::format(
              "{} to close partial-apply '{}'",
              tok::close,
              lookup.as_string('/')));
    }

    return {
        ast::partial_apply{
            scan.with_start(scan_start).range(), std::move(lookup)},
        scan};
  }

  // partial-lookup → { identifier ~ (("/" | ".") ~ identifier)* }
  parse_result<ast::lookup_path> parse_partial_lookup(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    const token& first_id = scan.advance();
    if (first_id.kind != tok::identifier) {
      return std::nullopt;
    }
    std::vector<ast::identifier> path;
    path.emplace_back(
        ast::identifier{first_id.range, std::string(first_id.string_value())});

    while (try_consume_token(&scan, tok::slash) ||
           try_consume_token(&scan, tok::dot)) {
      const token& id_part = scan.peek();
      if (id_part.kind != tok::identifier) {
        report_expected(scan, "identifier in partial-lookup");
      }
      scan.advance();
      path.emplace_back(
          ast::identifier{id_part.range, std::string(id_part.string_value())});
    }
    return {
        ast::lookup_path{scan.with_start(scan_start).range(), std::move(path)},
        scan};
  }

 public:
  parser(std::vector<token> tokens, diagnostics_engine& diags)
      : tokens_(std::move(tokens)), diags_(diags) {}

  std::optional<ast::root> parse() {
    return parse_root(parser_scan_window(
        tokens_.cbegin() /* start */,
        tokens_.cbegin() /* head */,
        tokens_.cend() /* end */));
  }
};

} // namespace

std::optional<ast::root> parse(source src, diagnostics_engine& diags) {
  return parser(lexer(std::move(src), diags).tokenize_all(), diags).parse();
}

} // namespace whisker
