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
#include <thrift/compiler/whisker/detail/string.h>
#include <thrift/compiler/whisker/lexer.h>
#include <thrift/compiler/whisker/parser.h>

#include <fmt/core.h>

#include <cassert>
#include <cstddef>
#include <iterator>
#include <optional>
#include <set>
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
  [[nodiscard]] const token& peek() const { return *head; }

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

bool try_consume_token(parser_scan_window* scan, tok kind) {
  assert(scan);
  if (scan->peek().kind == kind) {
    scan->advance();
    return true;
  }
  return false;
}

/**
 * The Mustache spec contains this concept called "standalone lines".
 *   https://github.com/mustache/spec/blob/66f078e0d534515d8df23d0d3764dccda74e042b/specs/sections.yml#L279-L305
 *
 * If a line opens a section block, and the rest of the line is whitespace only,
 * then that whitespace is stripped from the output.
 *
 * Consider the following template:
 *
 *     {{#true_value}}
 *       hello
 *     {{/true_value}}
 *
 * This will output "  hello\n". Notice that the newline following
 * "{{#true_value}}" is stripped. However, the newline after "hello" is
 * retained.
 *
 * If we have a {{variable}} then we do not see such behavior:
 *
 *     {{#true_value}} {{foo}}
 *       hello
 *     {{/true_value}}
 *
 * This will output " \n  hello\n".
 *
 * Things get even weirder because the definition of a "line" is quite liberal.
 * Consider the following template:
 *
 *     | This Is
 *       {{#boolean
 *            .condition}}
 *     |
 *       {{/boolean.condition}}
 *     | A Line
 *
 * This will output: "| This Is\n| A Line\n". In other words, the
 * {{#boolean.condition}} "line" should be stripped of whitespace even though
 * the whitespace spans multiple lines. This is because "{{ }}" eats whitespace
 * and newlines can only appear in textual content.
 *
 * Partials are also a special case, for three reasons:
 *   1. They can be contextually standalone even though they are not invisible.
 *   2. They cannot be combined with other standalone constructs. For example,
 *      two partial applications on the same line is not standalone.
 *   3. Standalone partial whitespace stripping applies only to its right side.
 *
 * Why does this belong here (in between the lexer and the parser)?
 *
 *   The standalone lines rules are particularly weird because it involves
 *   checking for blocks (a parsed AST concept) but also asserts that whitespace
 *   be removed (typically in the lexer's domain, not the parser).
 *
 *   In other words, the lexer is too "dumb" and the parser is too "refined".
 *   Therefore, this step can be considered a lexer post-process or a parser
 *   pre-process step.
 *
 * The implementation here is aimed to match mstch.
 */
class standalone_lines_scanner {
 public:
  struct result {
    /**
     * If a partial application is standalone, the renderer needs to make sure
     * to indent each line when inlining it. Since we've already detected such
     * partials, let's provide this information to the parser to attach to the
     * AST.
     *
     * The cursors here point to the "{{" token of the corresponding partial
     * application.
     *
     * Using std::set here because cursor is an iterator and thus has operator<
     * but not necessarily std::hash<>.
     */
    std::set<parser_scan_window::cursor> standalone_partial_locations;
  };

  static result strip_lines(std::vector<token>& tokens) {
    assert(!tokens.empty());
    if (tokens.back().kind == tok::error) {
      // No point stripping lines
      return {};
    }

    parser_scan_window scan(
        tokens.cbegin() /* start */,
        tokens.cbegin() /* head */,
        tokens.cend() /* end */);

    using cursor = parser_scan_window::cursor;

    // Chunk scanning into lines. We scan each line and mark tokens for removal
    // in a bitmap, then make a pass to erase them. This is because we don't
    // want to modify the vector while iterating over it.
    enum class token_marking : char {
      none,
      remove,
      standalone_partial,
    };
    std::vector<token_marking> marked_tokens(
        tokens.size(), token_marking::none);

    const auto mark_token = [&](cursor pos, token_marking mark) {
      assert(mark != token_marking::none);
      assert(
          pos->kind == tok::text || pos->kind == tok::newline ||
          pos->kind == tok::open);
      std::size_t index = std::distance(tokens.cbegin(), pos);
      assert(marked_tokens[index] == token_marking::none);
      marked_tokens[index] = mark;
    };

    // The return value is the set of standalone partial locations.
    const auto remove_marked_tokens = [&]() -> std::set<cursor> {
      // Because we are removing tokens and invalidating iterators, we need to
      // grab the actual cursors after vector::erase.
      std::vector<std::size_t> standalone_partial_indices;
      // Poor man's std::remove_if (which does not provide the iterator to the
      // predicate, which we need to access the bitmap).
      std::size_t compact = 0;
      for (std::size_t run = 0; run < tokens.size(); ++run) {
        if (marked_tokens[run] == token_marking::standalone_partial) {
          standalone_partial_indices.push_back(compact);
        }
        if (marked_tokens[run] != token_marking::remove) {
          std::swap(tokens[compact], tokens[run]);
          ++compact;
        }
      }

      tokens.erase(tokens.begin() + compact, tokens.end());
      std::set<cursor> standalone_partial_locations;
      for (std::size_t index : standalone_partial_indices) {
        standalone_partial_locations.insert(cursor(tokens.cbegin() + index));
      }
      return standalone_partial_locations;
    };

    struct line_info {
      cursor start;
      std::vector<cursor> standalone_partials = {};
      // true iff the line contains a standalone construct described above
      bool is_standalone_eligible = false;
    };
    line_info current_line{scan.head};

    const auto begin_next_line = [&]() {
      current_line.standalone_partials.clear();
      current_line = {scan.head, std::move(current_line.standalone_partials)};
      scan = scan.make_fresh();
    };

    const auto drain_current_line = [&]() {
      do {
        const auto& t = scan.advance();
        if (t.kind == tok::newline) {
          break;
        }
      } while (scan.can_advance());
      begin_next_line();
    };

    const auto strip_current_line = [&]() {
      if (current_line.is_standalone_eligible) {
        for (cursor c : current_line.standalone_partials) {
          mark_token(c, token_marking::standalone_partial);
        }
        for (cursor c = current_line.start; c < scan.head; ++c) {
          if (c->kind == tok::text || c->kind == tok::newline) {
            mark_token(c, token_marking::remove);
          }
        }
      }
      begin_next_line();
    };

    while (scan.can_advance()) {
      if (scan.peek().kind == tok::newline) {
        scan.advance();
        strip_current_line();
      }

      if (parse_result whitespace = parse_whitespace(scan)) {
        if (!std::move(whitespace).consume_and_advance(&scan)) {
          drain_current_line();
          continue;
        }
      } else if (parse_result standalone = parse_standalone_compatible(scan)) {
        auto scan_start = scan.start;
        switch (std::move(standalone).consume_and_advance(&scan)) {
          case standalone_compatible_kind::ineligible:
            drain_current_line();
            continue;
          case standalone_compatible_kind::partial_apply:
            current_line.standalone_partials.push_back(scan_start);
            // Do not strip the left side
            current_line.start = scan.head;
            if (current_line.is_standalone_eligible) {
              // Even though we allow multiple standalone constructs on the same
              // line, for partials it does not apply
              drain_current_line();
              continue;
            }
            break;
          default:
            // These blocks strip both sides
            break;
        }
        current_line.is_standalone_eligible = true;
      } else {
        // This line is not eligible for stripping
        drain_current_line();
      }
    }

    // In case, last line is not terminated by a newline.
    // Otherwise, the current line is ineligible so nothing happens.
    strip_current_line();

    return result{remove_marked_tokens()};
  }

  // A line containing any of the following constructs...
  //   - "{{! ... }}"
  //   - "{{# ... }}"
  //   - "{{^ ... }}"
  //   - "{{/ ... }}"
  //   - "{{> ... }}"
  // ..and ONLY those constructs are candidates for whitespace stripping. That's
  // because these kind of templates are invisible in the output — their purpose
  // is purely to express intent within the templating language.
  // The only exception is partial applications, where only the right side is
  // stripped when standalone.
  enum class standalone_compatible_kind {
    comment, // "{{!"
    block, // "{{#", "{{^", or "{{"/"}
    partial_apply, // "{{>"
    ineligible // "{{variable}} for example
  };
  static parse_result<standalone_compatible_kind> parse_standalone_compatible(
      parser_scan_window scan) {
    assert(scan.empty());
    if (!try_consume_token(&scan, tok::open)) {
      return std::nullopt;
    }

    standalone_compatible_kind kind;
    switch (scan.advance().kind.value) {
      case tok::bang:
        kind = standalone_compatible_kind::comment;
        break;
      case tok::pound:
      case tok::caret:
      case tok::slash:
        kind = standalone_compatible_kind::block;
        break;
      case tok::gt:
        kind = standalone_compatible_kind::partial_apply;
        break;
      default:
        kind = standalone_compatible_kind::ineligible;
    }

    // Keep consuming tokens until we reach the end of the "{{ }}"... even if it
    // means going over to the next line. Templates are supposed to eat
    // whitespace.
    while (scan.can_advance()) {
      if (scan.advance().kind == tok::close) {
        break;
      }
    }
    return {kind, scan};
  }

  // Returns:
  //   - std::nullopt if the current token is not text
  //   - true if the current token is text and is whitespace only
  //   - false if the current token is text and contains non-whitespace
  //     characters
  static parse_result<bool> parse_whitespace(parser_scan_window scan) {
    assert(scan.empty());
    const auto& text = scan.advance();
    if (text.kind != tok::text) {
      return std::nullopt;
    }
    for (char c : text.string_value()) {
      if (!detail::is_whitespace(c)) {
        return {false, scan};
      }
    }
    return {true, scan};
  }
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
  const source_manager& src_manager_;
  diagnostics_engine& diags_;
  std::set<parser_scan_window::cursor> standalone_partials_;

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

  // body → { text | newline | template | comment }
  parse_result<ast::body> parse_body(parser_scan_window scan) {
    assert(scan.empty());
    std::optional<ast::body> body;
    if (parse_result text = parse_text(scan)) {
      body = std::move(text).consume_and_advance(&scan);
    } else if (parse_result newline = parse_newline(scan)) {
      body = std::move(newline).consume_and_advance(&scan);
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

  // text → { <raw text until we see a template or newline> }
  parse_result<ast::text> parse_text(parser_scan_window scan) {
    assert(scan.empty());
    if (const auto& text = scan.advance(); text.kind == tok::text) {
      return {ast::text{scan.range(), std::string(text.string_value())}, scan};
    }
    return std::nullopt;
  }

  parse_result<ast::newline> parse_newline(parser_scan_window scan) {
    assert(scan.empty());
    if (const auto& text = scan.advance(); text.kind == tok::newline) {
      return {
          ast::newline{scan.range(), std::string(text.string_value())}, scan};
    }
    return std::nullopt;
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

  // section-block → { section-block-open ~ body* ~ section-block-close }
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
    const source_location scan_start_location = scan.start_location();

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

    bool is_standalone =
        standalone_partials_.find(scan_start) != standalone_partials_.end();

    using offset_type = std::optional<unsigned>;
    // resolved_location::column() is 1-indexed
    offset_type offset_within_line = is_standalone
        ? offset_type{resolved_location(scan_start_location, src_manager_).column() - 1}
        : std::nullopt;

    return {
        ast::partial_apply{
            scan.with_start(scan_start).range(),
            std::move(lookup),
            offset_within_line},
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
  parser(
      std::vector<token> tokens,
      const source_manager& src_manager,
      diagnostics_engine& diags,
      standalone_lines_scanner::result scan_result)
      : tokens_(std::move(tokens)),
        src_manager_(src_manager),
        diags_(diags),
        standalone_partials_(
            std::move(scan_result.standalone_partial_locations)) {}

  std::optional<ast::root> parse() {
    return parse_root(parser_scan_window(
        tokens_.cbegin() /* start */,
        tokens_.cbegin() /* head */,
        tokens_.cend() /* end */));
  }
};

} // namespace

std::optional<ast::root> parse(
    source src, const source_manager& src_manager, diagnostics_engine& diags) {
  auto tokens = lexer(std::move(src), diags).tokenize_all();
  auto standalone_scanner_result =
      standalone_lines_scanner::strip_lines(tokens);
  return parser(
             std::move(tokens),
             src_manager,
             diags,
             std::move(standalone_scanner_result))
      .parse();
}

} // namespace whisker
