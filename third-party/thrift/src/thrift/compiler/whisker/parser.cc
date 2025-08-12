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
#include <thrift/compiler/whisker/expected.h>
#include <thrift/compiler/whisker/lexer.h>
#include <thrift/compiler/whisker/parser.h>

#include <fmt/core.h>

#include <cassert>
#include <iterator>
#include <map>
#include <optional>
#include <unordered_set>
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
   * Creates a copy of this parser_scan_window with its head advanced n times
   * (or retreated if n is negative).
   *
   * The head is never moved past the end of the input or before the start of
   * the scan window.
   */
  [[nodiscard]] parser_scan_window next(cursor::difference_type n = 1) {
    return with_head(std::clamp(std::next(head, n), start, std::prev(end)));
  }
  [[nodiscard]] parser_scan_window prev(cursor::difference_type n = 1) {
    return next(-n);
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

  [[nodiscard]] parser_scan_window with_start(cursor new_start) const {
    assert(new_start <= head);
    return parser_scan_window(new_start, head, end);
  }
  [[nodiscard]] parser_scan_window with_head(cursor new_head) const {
    assert(start <= new_head);
    assert(new_head < end);
    return parser_scan_window(start, new_head, end);
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
struct parsed_object {
  parsed_object(T value, const parser_scan_window& advanced)
      : value_{std::move(value)}, new_head_{advanced.make_fresh()} {}

  /**
   * Advances the provided parser_scan_window to the last consumed token as part
   * of parsing, then returns the result of parsing. This method ensures that
   * the result of parsing cannot be consumed without advancing the cursor.
   */
  [[nodiscard]] T consume_and_advance(parser_scan_window* scan) && {
    assert(scan);
    *scan = std::move(new_head_);
    return std::move(value_);
  }

 private:
  T value_;
  parser_scan_window new_head_;
};

/**
 * A marker struct that indicates that parsing failed (non-fatally).
 */
struct no_parse_result {};

template <typename T>
struct [[nodiscard]] parse_result
    : private expected<parsed_object<T>, no_parse_result> {
 private:
  using base = expected<parsed_object<T>, no_parse_result>;

 public:
  parse_result(T value, const parser_scan_window& advanced)
      : base(std::in_place, std::move(value), advanced) {}
  /* implicit */ parse_result(no_parse_result) : base(unexpect) {}

  [[nodiscard]] T consume_and_advance(parser_scan_window* scan) && {
    assert(this->has_value());
    return std::move(**this).consume_and_advance(scan);
  }

  using base::operator bool;
  using base::has_value;
  using base::operator*;
  using base::operator->;
};

/**
 * Advances the provided scan window iff the next token matches the provided
 * kind. If the next token is not a match, then the scan window is unchanged.
 *
 * This function returns the observed token if matched, otherwise nullptr.
 */
const token* try_consume_token(parser_scan_window* scan, tok kind) {
  assert(scan);
  if (scan->peek().kind == kind) {
    return &scan->advance();
  }
  return nullptr;
}

/**
 * Advances the provided scan window iff the next series of token matches the
 * a sequence of token kinds in the provided order. If there is no exact match,
 * then the scan window is unchanged.
 *
 * This function returns true iff the scan window was advanced, otherwise false.
 */
bool try_consume_tokens(
    parser_scan_window* scan, std::initializer_list<tok> kinds) {
  assert(scan);
  auto original = *scan;
  for (tok t : kinds) {
    if (!try_consume_token(scan, t)) {
      *scan = original;
      return false;
    }
  }
  return true;
}

// Creates AST nodes that are derived from exactly one textual lexer token.
template <typename T, tok Kind>
T make_textual_node(const token& t) {
  assert(t.kind == Kind);
  return T{t.range, std::string(t.string_value())};
}
ast::identifier make_identifier(const token& t) {
  return make_textual_node<ast::identifier, tok::identifier>(t);
}
ast::variable_component make_variable_component(
    const token* prototype, const token& property) {
  return ast::variable_component{
      source_range{
          prototype != nullptr ? prototype->range.begin : property.range.begin,
          property.range.end,
      },
      prototype != nullptr ? std::optional{make_identifier(*prototype)}
                           : std::nullopt,
      make_identifier(property)};
}
ast::path_component make_path_component(const token& t) {
  return make_textual_node<ast::path_component, tok::path_component>(t);
}
ast::text::whitespace make_whitespace_text(const token& t) {
  return make_textual_node<ast::text::whitespace, tok::whitespace>(t);
}
ast::text::non_whitespace make_non_whitespace_text(const token& t) {
  return make_textual_node<ast::text::non_whitespace, tok::text>(t);
}

/**
 * The Mustache spec contains this concept called "standalone lines".
 *   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L279-L305
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
  /**
   * A marker struct indicating a tok::whitespace or tok::newline that should
   * be stripped because they are part of a standalone line.
   */
  struct removed {};
  /**
   * A marker struct indicating a block that is standalone.
   */
  struct standalone_block {};
  /**
   * A struct indicating a standalone partial application.
   */
  struct partial_apply {
    /**
     * The whitespace content to the left of the partial application that
     * should be replicated for subsequent lines in multi-line partial
     * applications.
     */
    ast::text::whitespace preceding_whitespace;
  };
  using standalone_marking =
      std::variant<removed, standalone_block, partial_apply>;
  /**
   * The cursors in the set point to either:
   *   - the "{{" token of the corresponding standalone construct.
   *   - tok::whitespace or tok::newline.
   *
   * Using std::map here because cursor is an iterator and thus has operator<
   * but not necessarily std::hash<>.
   */
  using result = std::map<parser_scan_window::cursor, standalone_marking>;

  /**
   * Returns a result containing the cursors of the tokens that are "involved"
   * in standalone constructs. The cursors can be used by the parser to remove
   * whitespace or add partial standalone indentation information.
   */
  static result mark(const std::vector<token>& tokens) {
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

    result marked_tokens;
    const auto& mark_removal = [&](cursor pos) {
      assert(pos->kind == tok::whitespace || pos->kind == tok::newline);
      [[maybe_unused]] const auto& [_, inserted] =
          marked_tokens.insert({pos, removed()});
      assert(inserted);
    };

    const auto& mark_standalone = [&](cursor pos) {
      assert(pos->kind == tok::open);
      [[maybe_unused]] const auto& [_, inserted] =
          marked_tokens.insert({pos, standalone_block()});
      assert(inserted);
    };

    const auto& mark_partial_apply = [&](cursor pos, partial_apply partial) {
      assert(pos->kind == tok::open);
      [[maybe_unused]] const auto& [_, inserted] =
          marked_tokens.insert({pos, std::move(partial)});
      assert(inserted);
    };

    struct line_info {
      cursor start;
      std::vector<std::pair<cursor, standalone_marking>> markings = {};
    };
    line_info current_line{scan.head};

    const auto begin_next_line = [&]() {
      current_line.markings.clear();
      current_line = {scan.head, std::move(current_line.markings)};
      scan = scan.make_fresh();
    };

    // Only after scanning the entire line can we be sure if there are eligible
    // standalone constructs. This function "commits" the changes aggregated in
    // the current line into the output.
    const auto commit_current_line_markings = [&]() {
      if (!current_line.markings.empty()) {
        for (cursor c = current_line.start; c < scan.head; ++c) {
          if (c->kind == tok::whitespace || c->kind == tok::newline) {
            mark_removal(c);
          }
        }
        for (auto& m : current_line.markings) {
          cursor c = m.first;
          auto& marking = m.second;
          detail::variant_match(
              std::move(marking),
              [&](standalone_block&&) { mark_standalone(c); },
              [&](partial_apply&& partial) {
                mark_partial_apply(c, std::move(partial));
              },
              [&](removed&&) {
                throw std::logic_error(
                    "Removal markings should only happen at the end of the line");
              });
        }
      }
      begin_next_line();
    };

    // When an ineligible line is detected, we drain the current line's tokens
    // and dump all the potential standalone candidates from the current line.
    const auto drain_current_line = [&]() {
      do {
        const auto& t = scan.advance();
        if (t.kind == tok::newline) {
          break;
        }
      } while (scan.can_advance());
      begin_next_line();
    };

    while (scan.can_advance()) {
      if (try_consume_token(&scan, tok::newline)) {
        commit_current_line_markings();
      }
      if (try_consume_token(&scan, tok::whitespace)) {
        scan = scan.make_fresh();
        continue;
      }
      if (try_consume_token(&scan, tok::text)) {
        drain_current_line();
        continue;
      }
      if (parse_result standalone = parse_standalone_compatible(scan)) {
        auto scan_start = scan.start;
        switch (std::move(standalone).consume_and_advance(&scan)) {
          case standalone_compatible_kind::ineligible:
            drain_current_line();
            continue;
          case standalone_compatible_kind::partial_apply: {
            if (!current_line.markings.empty()) {
              // Even though we allow multiple standalone constructs on the same
              // line, for partials it does not apply
              drain_current_line();
              continue;
            }
            auto preceding_whitespace =
                std::invoke([&]() -> ast::text::whitespace {
                  if (current_line.start == scan_start) {
                    return {};
                  }
                  // There should be exactly one whitespace token on the line
                  assert(current_line.start->kind == tok::whitespace);
                  assert(std::next(current_line.start) == scan_start);
                  return make_whitespace_text(*current_line.start);
                });
            current_line.markings.emplace_back(
                scan_start, partial_apply{std::move(preceding_whitespace)});
            // Do not strip the left side
            current_line.start = scan.head;
            break;
          }
          default:
            // These blocks strip both sides
            current_line.markings.emplace_back(scan_start, standalone_block());
            break;
        }
      } else {
        // This line is not eligible for stripping
        drain_current_line();
      }
    }

    // In case, last line is not terminated by a newline.
    // Otherwise, the current line is ineligible so nothing happens.
    commit_current_line_markings();

    return marked_tokens;
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
    block_or_statement, // "{{#" (except "{{#partial"), "{{^", or "{{"/"}
    partial_apply, // "{{>", "{{#partial"
    ineligible // "{{variable}} for example
  };
  static parse_result<standalone_compatible_kind> parse_standalone_compatible(
      parser_scan_window scan) {
    assert(scan.empty());
    if (!try_consume_token(&scan, tok::open)) {
      return no_parse_result();
    }

    standalone_compatible_kind kind;
    switch (scan.advance().kind.value) {
      case tok::bang:
        kind = standalone_compatible_kind::comment;
        break;
      case tok::pound:
      case tok::caret:
      case tok::slash:
        kind = scan.peek().kind == tok::kw_partial
            ? standalone_compatible_kind::partial_apply
            : standalone_compatible_kind::block_or_statement;
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
  standalone_lines_scanner::result standalone_markings_;
  /**
   * Whether the parser has encountered non-fatal errors. This is used to signal
   * that the parsing should eventually fail but the parser has made an attempt
   * to recover from the error in order to provide other potentially useful
   * diagnostics that arise from parsing.
   */
  bool has_non_fatal_errors_ = false;

  // Reports an error without failing the parse.
  template <typename... T>
  void report_error(
      const parser_scan_window& scan,
      fmt::format_string<T...> msg,
      T&&... args) {
    has_non_fatal_errors_ = true;
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

  template <typename... T>
  [[noreturn]] void report_fatal_expected(
      const parser_scan_window& scan,
      fmt::format_string<T...> msg,
      T&&... args) {
    report_fatal_error(
        scan,
        "expected {} but found {}",
        fmt::format(msg, std::forward<T>(args)...),
        to_string(scan.peek().kind));
  }

  [[nodiscard]] bool is_standalone_whitespace(
      parser_scan_window::cursor pos) const {
    assert(pos->kind == tok::whitespace || pos->kind == tok::newline);
    return standalone_markings_.find(pos) != standalone_markings_.end();
  }

  std::optional<ast::text::whitespace> standalone_partial_indentation(
      parser_scan_window::cursor pos) const {
    assert(pos->kind == tok::open);
    if (auto marking = standalone_markings_.find(pos);
        marking != standalone_markings_.end()) {
      return std::get<standalone_lines_scanner::partial_apply>(marking->second)
          .preceding_whitespace;
    }
    return std::nullopt;
  }

  unsigned line_number_of(source_location pos) const {
    return resolved_location(pos, diags_.source_mgr()).line();
  }
  /**
   * Gets the line number of start of a given token.
   */
  unsigned line_number_of(const token& t) const {
    return line_number_of(t.range.begin);
  }

  // root → { header* ~ body* }
  std::optional<ast::root> parse_root(parser_scan_window scan) {
    constexpr std::string_view expected_types = "text, template, or comment";
    try {
      auto original_scan = scan;

      ast::headers headers;
      while (parse_result header = parse_header(scan)) {
        headers.emplace_back(std::move(header).consume_and_advance(&scan));
      }

      ast::bodies bodies;
      while (scan.can_advance()) {
        if (parse_result maybe_body = parse_body(scan)) {
          if (auto body = std::move(maybe_body).consume_and_advance(&scan)) {
            bodies.emplace_back(std::move(*body));
          }
        } else if (auto else_clause = parse_else_clause(scan)) {
          // "{{#else}}" marks the end of a body and beginning of a new one,
          // which cannot happen at the root scope.
          report_fatal_error(
              scan,
              "expected {} but found dangling else-clause",
              expected_types);
        } else {
          report_fatal_expected(scan, expected_types);
        }
      }
      if (has_non_fatal_errors_) {
        // Even though parsing continued after failure, the error-recovered AST
        // is not valid.
        return std::nullopt;
      }
      return ast::root{
          original_scan.start_location(),
          std::move(headers),
          std::move(bodies)};
    } catch (const parse_error&) {
      // the error should already have been reported via the diagnostics
      // engine
      return std::nullopt;
    }
  }

  // Parses the "{{#else}}" clause which is a separator between two ast::bodies.
  //
  // else-clause → { "{{" ~ "#" ~ "else" ~ "}}" }
  parse_result<std::monostate> parse_else_clause(parser_scan_window scan) {
    if (!try_consume_tokens(
            &scan, {tok::open, tok::pound, tok::kw_else, tok::close})) {
      return no_parse_result();
    }
    return {{}, scan};
  }

  // Parses the beginning of an "{{#else [if]" clause which is a separator
  // between two ast::bodies.
  //
  // { "{{" ~ "#" ~ "else" }
  bool peek_else_clause(parser_scan_window scan) {
    return try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_else});
  }

  // Parses the "{{#else}}" block which is a separator between two
  // ast::bodies.
  //
  // else-block → { "{{" ~ "#" ~ "else" ~ "}}" ~ body* }
  parse_result<ast::else_block> parse_else_block(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (parse_result e = parse_else_clause(scan)) {
      std::ignore = std::move(e).consume_and_advance(&scan);
    } else {
      return no_parse_result();
    }
    scan = scan.make_fresh();
    auto else_bodies = parse_bodies(scan).consume_and_advance(&scan);
    return parse_result{
        ast::else_block{
            scan.with_start(scan_start).range(), std::move(else_bodies)},
        scan};
  }

  // Parses the "{{#else if}}" block which is a separator between two
  // ast::bodies.
  //
  // else-if-block →
  //   { "{{" ~ "#" ~ "else" ~ " " ~ "if" ~ expression ~ "}}" ~ body* }
  parse_result<ast::conditional_block::else_if_block> parse_else_if_block(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(
            &scan, {tok::open, tok::pound, tok::kw_else, tok::kw_if})) {
      return no_parse_result();
    }
    scan = scan.make_fresh();

    parse_result condition = parse_expression(scan);
    if (!condition.has_value()) {
      report_fatal_expected(scan, "expression in else-if block");
    }
    ast::expression cond = std::move(condition).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} in else-if block", tok::close);
    }
    scan = scan.make_fresh();

    ast::bodies bodies = parse_bodies(scan).consume_and_advance(&scan);

    return parse_result{
        ast::conditional_block::else_if_block{
            scan.with_start(scan_start).range(),
            std::move(cond),
            std::move(bodies)},
        scan};
  }

  // Parses header elements that can only appear at the top of the source.
  //
  // header → { comment | pragma-statement | import-statement }
  parse_result<ast::header> parse_header(parser_scan_window scan) {
    assert(scan.empty());

    while (scan.can_advance()) {
      // Because header elements are more "directives" for the Whisker language,
      // we ignore whitespace that exist between header elements.
      // However, the header is not "greedy". When we encounter the first
      // non-header element, we return all consumed whitespace back to the body.
      if (parse_result newline = parse_newline(scan)) {
        std::ignore = std::move(newline).consume_and_advance(&scan);
        continue;
      }
      if (try_consume_token(&scan, tok::whitespace)) {
        scan = scan.make_fresh();
        continue;
      }
      if (parse_result maybe_comment = parse_comment(scan)) {
        auto comment = std::move(maybe_comment).consume_and_advance(&scan);
        return {std::move(comment), scan};
      }
      if (parse_result maybe_pragma = parse_pragma_statement(scan)) {
        auto pragma = std::move(maybe_pragma).consume_and_advance(&scan);
        return {pragma, scan};
      }
      if (parse_result maybe_import = parse_import_statement(scan)) {
        auto import = std::move(maybe_import).consume_and_advance(&scan);
        return {std::move(import), scan};
      }
      // Next token is not valid for a header element
      break;
    }
    return no_parse_result();
  }

  // Returns an empty parse result if no body was found.
  //
  // Returns an empty optional<ast::body> if body was found but consisted
  // entirely of stripped whitespace from a standalone construct, with the
  // advanced scan window.
  //
  // Returns the ast::body found otherwise.
  //
  // body → { text | newline | template | comment }
  parse_result<std::optional<ast::body>> parse_body(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.head;

    std::optional<ast::body> body;
    while (!body.has_value()) {
      if (parse_result maybe_text = parse_text(scan)) {
        body = std::move(maybe_text).consume_and_advance(&scan);
      } else if (parse_result maybe_newline = parse_newline(scan)) {
        body = std::move(maybe_newline).consume_and_advance(&scan);
      } else if (peek_import_statement(scan)) {
        report_fatal_error(
            scan,
            "import statements must appear in the header of a source file");
      } else if (peek_else_clause(scan)) {
        // The "{{#else [if]}}" clause marks the end of the current block (and
        // the beginning of the next one).
        break;
      } else if (parse_result templ = parse_template(scan)) {
        detail::variant_match(
            std::move(templ).consume_and_advance(&scan),
            [&](auto&& t) { body = std::move(t); });
      } else if (parse_result comment = parse_comment(scan)) {
        body = std::move(comment).consume_and_advance(&scan);
      } else {
        // Next token is not valid for a body element
        break;
      }
    }

    if (scan.head == scan_start) {
      // We did not find any body elements. Not even standalone whitespace.
      return no_parse_result();
    }
    return {std::move(body), scan};
  }
  /**
   * Parses the grammar for "body*". If there are no bodies present, returns an
   * empty vector.
   */
  parsed_object<ast::bodies> parse_bodies(parser_scan_window scan) {
    ast::bodies bodies;
    while (parse_result maybe_body = parse_body(scan)) {
      if (auto body = std::move(maybe_body).consume_and_advance(&scan)) {
        bodies.emplace_back(std::move(*body));
      }
    }
    return {std::move(bodies), scan};
  }

  // Returns an empty parse result if no text was found.
  //
  // Returns an empty optional<ast::text> if text was found but completely
  // stripped because of standalone constructs, with the advanced scan window.
  //
  // Returns the ast::text found otherwise with a non-empty string.
  //
  // text → { (tok::text | tok::whitespace)+ }
  parse_result<std::optional<ast::text>> parse_text(parser_scan_window scan) {
    assert(scan.empty());
    std::vector<ast::text::content> parts;
    while (scan.can_advance()) {
      const token& t = scan.peek();
      if (t.kind == tok::text) {
        parts.emplace_back(make_non_whitespace_text(t));
      } else if (t.kind == tok::whitespace) {
        if (!is_standalone_whitespace(scan.head)) {
          parts.emplace_back(make_whitespace_text(t));
        }
      } else {
        break;
      }
      scan.advance();
    }
    if (scan.head == scan.start) {
      // No text was scanned. Not even standalone whitespace.
      return no_parse_result();
    }
    if (parts.empty()) {
      // Text was scanned but they were all stripped. We still need to advance
      // the scan window.
      return {std::nullopt, scan};
    }
    return parse_result{
        std::optional{ast::text{scan.range(), std::move(parts)}}, scan};
  }

  // Returns an empty parse result if no newline was found.
  //
  // Returns an empty optional<ast::newline> if newline was found but stripped
  // because of standalone constructs, with the advanced scan window.
  //
  // Returns the ast::newline found otherwise.
  //
  // newline → { tok::newline+ }
  parse_result<std::optional<ast::newline>> parse_newline(
      parser_scan_window scan) {
    assert(scan.empty());
    if (scan.peek().kind == tok::newline) {
      if (is_standalone_whitespace(scan.head)) {
        // Stripped because of a standalone construct
        return {std::nullopt, scan.next()};
      }
      const token& text = scan.advance();
      return {
          ast::newline{scan.range(), std::string(text.string_value())}, scan};
    }
    return no_parse_result();
  }

  // comment → { basic-comment | escaped-comment }
  // basic-comment → { "{{!" ~ <raw text until we see "}}"> ~ "}}" }
  // escaped-comment → { "{{!--" ~ <raw text until we see "--}}"> ~ "--}}" }
  //
  // NOTE: the difference between basic-comment and escaped-comment is dealt
  // with by the lexer already.
  parse_result<ast::comment> parse_comment(parser_scan_window scan) {
    assert(scan.empty());
    if (!try_consume_tokens(&scan, {tok::open, tok::bang})) {
      return no_parse_result();
    }
    if (try_consume_token(&scan, tok::close)) {
      // empty comment
      return {ast::comment{scan.range(), ""}, scan};
    }

    if (const token* text = try_consume_token(&scan, tok::text)) {
      if (!try_consume_token(&scan, tok::close)) {
        report_fatal_expected(scan, "{} to close comment", tok::close);
      }
      return {
          ast::comment{scan.range(), std::string(text->string_value())}, scan};
    } else {
      report_fatal_expected(scan, "comment text");
    }
  }

  using template_body = std::variant<
      ast::interpolation,
      ast::section_block,
      ast::conditional_block,
      ast::with_block,
      ast::each_block,
      ast::partial_block,
      ast::partial_statement,
      ast::let_statement,
      ast::pragma_statement,
      ast::macro>;
  // template → { interpolation | block | statement | macro }
  // block → { section-block | conditional-block }
  // statement → { let-statement | pragma-statement }
  parse_result<template_body> parse_template(parser_scan_window scan) {
    assert(scan.empty());
    if (scan.peek().kind != tok::open) {
      return no_parse_result();
    }
    switch (scan.next().peek().kind) {
      case tok::bang:
        // this is a comment so don't fail the parse
        [[fallthrough]];
      case tok::slash:
        // parse_template can be called recursively, which means that seeing
        // tok::slash implies that the parent node is closing (so don't fail the
        // parse).
        return no_parse_result();
      default:
        // continue parsing as a template or fail the parse!
        break;
    }

    std::optional<template_body> templ;
    if (parse_result variable = parse_interpolation(scan)) {
      templ = std::move(variable).consume_and_advance(&scan);
    } else if (parse_result conditional_block = parse_conditional_block(scan)) {
      templ = std::move(conditional_block).consume_and_advance(&scan);
    } else if (parse_result with_block = parse_with_block(scan)) {
      templ = std::move(with_block).consume_and_advance(&scan);
    } else if (parse_result each_block = parse_each_block(scan)) {
      templ = std::move(each_block).consume_and_advance(&scan);
    } else if (parse_result partial_block = parse_partial_block(scan)) {
      templ = std::move(partial_block).consume_and_advance(&scan);
    } else if (parse_result let_statement = parse_let_statement(scan)) {
      templ = std::move(let_statement).consume_and_advance(&scan);
    } else if (parse_result pragma_statement = parse_pragma_statement(scan)) {
      templ = std::move(pragma_statement).consume_and_advance(&scan);
    } else if (parse_result partial_statement = parse_partial_statement(scan)) {
      templ = std::move(partial_statement).consume_and_advance(&scan);
    } else if (parse_result section_block = parse_section_block(scan)) {
      templ = std::move(section_block).consume_and_advance(&scan);
    } else if (parse_result macro = parse_macro(scan)) {
      templ = std::move(macro).consume_and_advance(&scan);
    }
    if (!templ.has_value()) {
      report_fatal_expected(
          scan, "interpolation, block, statement, or macro in template");
    }
    return {std::move(*templ), scan};
  }

  // interpolation → { "{{" ~ expression ~ "}}" }
  parse_result<ast::interpolation> parse_interpolation(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_token(&scan, tok::open)) {
      return no_parse_result();
    }
    switch (scan.peek().kind.value) {
      case tok::bang:
        // this is a comment
        [[fallthrough]];
      case tok::pound:
      case tok::caret:
        // this is a block opening
        [[fallthrough]];
      case tok::slash:
        // this is a block closing
        return no_parse_result();
      case tok::gt:
        // this is a macro
        return no_parse_result();
      default:
        // continue parsing as a variable (and fail if it's not!)
        break;
    }
    scan = scan.make_fresh();

    parse_result expression = parse_expression(scan);
    if (!expression.has_value()) {
      report_fatal_expected(scan, "expression in interpolation");
    }
    ast::expression lookup = std::move(expression).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to close interpolation", tok::close);
    }
    return {
        ast::interpolation{
            scan.with_start(scan_start).range(), std::move(lookup)},
        scan};
  }

  // variable-component → { identifier ~ [":" ~ identifier] }
  parse_result<ast::variable_component> parse_variable_component(
      parser_scan_window scan) {
    const token& first_id = scan.advance();
    if (first_id.kind != tok::identifier) {
      return no_parse_result();
    }

    if (!try_consume_token(&scan, tok::colon)) {
      // No ':', treat it as a raw property identifier
      return {make_variable_component(nullptr /* qualifier */, first_id), scan};
    }

    // first_id precedes ':', treat it as a qualifier
    // Try parse an additional identifier after ':' for the property name
    if (const token* second_id = try_consume_token(&scan, tok::identifier)) {
      return {make_variable_component(&first_id, *second_id), scan};
    }

    report_fatal_expected(scan, "identifier in qualified variable-component");
  }

  // variable-lookup → { "."
  // | "this"
  // | (variable-component ~ ("." ~ variable-component)*) }
  parse_result<ast::variable_lookup> parse_variable_lookup(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;
    if (try_consume_token(&scan, tok::dot) ||
        try_consume_token(&scan, tok::kw_this)) {
      return {
          ast::variable_lookup{
              scan.with_start(scan_start).range(),
              ast::variable_lookup::this_ref{}},
          scan};
    }

    scan = scan.make_fresh();
    parse_result first_component = parse_variable_component(scan);
    if (!first_component) {
      return no_parse_result();
    }
    std::vector<ast::variable_component> path;
    path.emplace_back(std::move(first_component).consume_and_advance(&scan));

    while (try_consume_token(&scan, tok::dot)) {
      if (parse_result component = parse_variable_component(scan)) {
        path.emplace_back(std::move(component).consume_and_advance(&scan));
      } else {
        report_fatal_expected(scan, "variable-component in variable-lookup");
      }
    }

    auto range = scan.with_start(scan_start).range();
    return {ast::variable_lookup{range, std::move(path)}, scan};
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
      return no_parse_result();
    }
    bool is_inverted = try_consume_token(&scan, tok::caret);
    if (!is_inverted) {
      if (!try_consume_token(&scan, tok::pound)) {
        // neither "#" nor "^" so this is not a section block
        return no_parse_result();
      }
    }

    parse_result lookup_at_open = parse_variable_lookup(scan.make_fresh());
    if (!lookup_at_open.has_value()) {
      report_fatal_expected(scan, "variable-lookup to open section-block");
    }
    ast::variable_lookup open =
        std::move(lookup_at_open).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to open section-block", tok::close);
    }
    scan = scan.make_fresh();

    ast::bodies bodies = parse_bodies(scan).consume_and_advance(&scan);

    if (!try_consume_token(&scan, tok::open)) {
      report_fatal_expected(
          scan,
          "{} to close section-block '{}'",
          tok::open,
          open.chain_string());
    }
    if (!try_consume_token(&scan, tok::slash)) {
      report_fatal_expected(
          scan,
          "{} to close section-block '{}'",
          tok::slash,
          open.chain_string());
    }

    parse_result lookup_at_close = parse_variable_lookup(scan.make_fresh());
    if (!lookup_at_close.has_value()) {
      report_fatal_expected(
          scan,
          "variable-lookup to close section-block '{}'",
          open.chain_string());
    }
    ast::variable_lookup close =
        std::move(lookup_at_close).consume_and_advance(&scan);

    if (open != close) {
      report_error(
          scan,
          "section-block opening '{}' does not match closing '{}'",
          open.chain_string(),
          close.chain_string());
    }
    if (!try_consume_token(&scan, tok::close)) {
      report_error(
          scan,
          "expected {} to close section-block '{}'",
          tok::close,
          open.chain_string());
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

  // expression → { literal | variable-lookup | function-call }
  parse_result<ast::expression> parse_expression(parser_scan_window scan) {
    assert(scan.empty());
    auto scan_start = scan.start;

    using expression = ast::expression;
    using function_call = expression::function_call;

    // Parse literals
    if (const token* string_literal =
            try_consume_token(&scan, tok::string_literal)) {
      return {
          expression{
              scan.range(),
              expression::string_literal{
                  std::string(string_literal->string_value())}},
          scan};
    }
    if (const token* i64_literal = try_consume_token(&scan, tok::i64_literal)) {
      return {
          expression{
              scan.range(), expression::i64_literal{i64_literal->i64_value()}},
          scan};
    }
    if (try_consume_token(&scan, tok::kw_null)) {
      return {expression{scan.range(), expression::null_literal{}}, scan};
    }
    if (try_consume_token(&scan, tok::kw_true)) {
      return {expression{scan.range(), expression::true_literal{}}, scan};
    }
    if (try_consume_token(&scan, tok::kw_false)) {
      return {expression{scan.range(), expression::false_literal{}}, scan};
    }
    assert(scan.empty());

    if (parse_result lookup = parse_variable_lookup(scan)) {
      auto expr = std::move(lookup).consume_and_advance(&scan);
      return {
          expression{scan.with_start(scan_start).range(), std::move(expr)},
          scan};
    }

    if (!try_consume_token(&scan, tok::l_paren)) {
      return no_parse_result();
    }
    scan = scan.make_fresh();

    function_call func;

    if (try_consume_token(&scan, tok::kw_not)) {
      func.which = function_call::builtin_not{};
    } else if (try_consume_token(&scan, tok::kw_and)) {
      func.which = function_call::builtin_and{};
    } else if (try_consume_token(&scan, tok::kw_or)) {
      func.which = function_call::builtin_or{};
    } else if (parse_result lookup = parse_variable_lookup(scan)) {
      func.which = function_call::user_defined{
          std::move(lookup).consume_and_advance(&scan)};
    } else {
      report_fatal_expected(scan, "function-lookup in function-call");
    }

    using named_argument_entry =
        decltype(function_call::named_arguments)::value_type;
    // argument → { positional-argument | named-argument }
    // positional-argument → { expression }
    // named-argument → { identifier ~ "=" ~ expression }
    const auto parse_argument = [this, &func](parser_scan_window scan)
        -> parse_result<std::variant<expression, named_argument_entry>> {
      assert(scan.empty());

      if (try_consume_tokens(&scan, {tok::identifier, tok::eq})) {
        const token& id = scan.prev(2).peek();
        assert(id.kind == tok::identifier);
        if (parse_result expr = parse_expression(scan.make_fresh())) {
          return {
              named_argument_entry{
                  std::string(id.string_value()),
                  function_call::named_argument{
                      make_identifier(id),
                      std::make_unique<expression>(
                          std::move(expr).consume_and_advance(&scan))}},
              scan};
        }
        report_fatal_expected(scan, "expression in named argument");
      }

      assert(scan.empty());
      if (parse_result expr = parse_expression(scan)) {
        return {expression{std::move(expr).consume_and_advance(&scan)}, scan};
      }
      if (scan.peek().kind == tok::eq) {
        report_fatal_error(
            scan,
            "expected identifier to precede {} in named argument for function call '{}'",
            tok::eq,
            func.name());
      }
      return no_parse_result();
    };

    // All named arguments must be at the end of the argument list
    bool named_arg_seen = false;
    while (parse_result arg = parse_argument(scan.make_fresh())) {
      auto arg_scan_start = scan;
      detail::variant_match(
          std::move(arg).consume_and_advance(&scan),
          [&](ast::expression&& positional) {
            if (named_arg_seen) {
              report_fatal_error(
                  scan,
                  "unexpected positional argument '{}' after named arguments in function call '{}'",
                  positional.to_string(),
                  func.name());
            }
            func.positional_arguments.push_back(std::move(positional));
          },
          [&](named_argument_entry&& named) {
            named_arg_seen = true;
            std::string_view name = named.first;
            if (const auto& [_, inserted] =
                    func.named_arguments.insert(std::move(named));
                !inserted) {
              report_fatal_error(
                  arg_scan_start,
                  "duplicate named argument '{}' in function call '{}'",
                  name,
                  func.name());
            }
          });
    }
    scan = scan.make_fresh();

    // Validate positional arguments
    detail::variant_match(
        func.which,
        [&](const function_call::builtin_not&) {
          if (func.positional_arguments.size() != 1) {
            report_fatal_error(
                scan,
                "expected 1 argument for function 'not' but found {}",
                func.positional_arguments.size());
          }
        },
        [&](const function_call::builtin_binary_associative&) {
          if (func.positional_arguments.size() <= 1) {
            report_fatal_error(
                scan,
                "expected at least 2 arguments for function '{}' but found {}",
                func.name(),
                func.positional_arguments.size());
          }
        },
        [](const function_call::user_defined&) {
          // User-defined functions can have any number of arguments
        });

    // Validate named arguments
    detail::variant_match(
        func.which,
        [&](const function_call::builtin&) {
          if (!func.named_arguments.empty()) {
            report_fatal_error(
                scan,
                "named arguments not allowed for function '{}'",
                func.name());
          }
        },
        [](const function_call::user_defined&) {
          // User-defined functions can have any number of named arguments
        });

    if (!try_consume_token(&scan, tok::r_paren)) {
      report_fatal_expected(
          scan, "{} to close function '{}'", tok::r_paren, func.name());
    }

    return {
        expression{scan.with_start(scan_start).range(), std::move(func)}, scan};
  }

  // let-statement →
  //   { "{{" ~ "#" ~ "let" ~ identifier ~ "=" ~ expression ~ "}}" }
  parse_result<ast::let_statement> parse_let_statement(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_let})) {
      return no_parse_result();
    }
    bool exported = try_consume_token(&scan, tok::kw_export) != nullptr;

    if (try_consume_token(&scan, tok::kw_partial)) {
      // This is actually a partial-block
      return no_parse_result();
    }

    const token* id = try_consume_token(&scan, tok::identifier);
    if (id == nullptr) {
      report_fatal_expected(scan, "identifier in let-statement");
    }

    if (!try_consume_token(&scan, tok::eq)) {
      report_fatal_expected(scan, "{} in let-statement", tok::eq);
    }

    scan = scan.make_fresh();
    parse_result expression = parse_expression(scan);
    if (!expression.has_value()) {
      report_fatal_expected(scan, "expression in let-statement");
    }
    ast::expression value = std::move(expression).consume_and_advance(&scan);

    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to close let-statement", tok::close);
    }

    return {
        ast::let_statement{
            scan.with_start(scan_start).range(),
            exported,
            make_identifier(*id),
            std::move(value)},
        scan};
  }

  // pragma-statement → { "{{" ~ "#" ~ "pragma" ~ identifier ~ "}}" }
  parse_result<ast::pragma_statement> parse_pragma_statement(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_pragma})) {
      return no_parse_result();
    }

    const token* id = try_consume_token(&scan, tok::identifier);
    if (id == nullptr) {
      report_fatal_expected(scan, "identifier in pragma-statement");
    }

    ast::pragma_statement::pragmas pragma;
    if (id->string_value() == "ignore-newlines") {
      pragma = ast::pragma_statement::pragmas::ignore_newlines;
    } else {
      report_error(scan, "unknown pragma '{}'", id->string_value());
    }

    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to close pragma-statement", tok::close);
    }

    return {
        ast::pragma_statement{scan.with_start(scan_start).range(), pragma},
        scan};
  }

  // { "{{" ~ "#" ~ "import" }
  parse_result<std::monostate> peek_import_statement(parser_scan_window scan) {
    if (try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_import})) {
      return {{}, scan};
    }
    return no_parse_result();
  }

  // import-statement →
  //   { "{{" ~ "#" ~ "import" ~ import-path ~ "as" ~ identifier ~ "}}" }
  // import-path → { string-literal }
  parse_result<ast::import_statement> parse_import_statement(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (auto open = peek_import_statement(scan)) {
      std::ignore = std::move(open).consume_and_advance(&scan);
    } else {
      return no_parse_result();
    }

    const token* import_path = try_consume_token(&scan, tok::string_literal);
    if (import_path == nullptr) {
      report_fatal_expected(scan, "import-path in import-statement");
    }

    if (!try_consume_token(&scan, tok::kw_as)) {
      report_fatal_expected(scan, "{} in import-statement", tok::kw_as);
    }

    const token* id = try_consume_token(&scan, tok::identifier);
    if (id == nullptr) {
      report_fatal_expected(scan, "identifier in import-statement");
    }

    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to close import-statement", tok::close);
    }

    return {
        ast::import_statement{
            scan.with_start(scan_start).range(),
            ast::expression::string_literal{
                std::string(import_path->string_value())},
            make_identifier(*id)},
        scan};
  }

  // conditional-block →
  //   { cond-block-open ~ body* ~ else-if-block* ~ else-block? ~
  //     cond-block-close }
  // cond-block-open →
  //   { "{{" ~ "#" ~ "if" ~ expression ~ "}}" }
  // else-if-block →
  //   { "{{" ~ "#" ~ "else" ~ " " ~ "if" ~ expression ~ "}}" ~ body* }
  // cond-block-close →
  //   { "{{" ~ "/" ~ "if" ~ expression? ~ "}}" }
  //
  // NOTE: the expression must match between open and close
  parse_result<ast::conditional_block> parse_conditional_block(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_if})) {
      return no_parse_result();
    }
    scan = scan.make_fresh();

    parse_result condition = parse_expression(scan);
    if (!condition.has_value()) {
      report_fatal_expected(scan, "expression to open if-block");
    }
    ast::expression open = std::move(condition).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to open if-block", tok::close);
    }
    scan = scan.make_fresh();

    ast::bodies bodies = parse_bodies(scan).consume_and_advance(&scan);

    std::vector<ast::conditional_block::else_if_block> else_if_blocks;
    while (parse_result else_if = parse_else_if_block(scan)) {
      else_if_blocks.push_back(std::move(else_if).consume_and_advance(&scan));
    }

    auto else_block = std::invoke([&]() -> std::optional<ast::else_block> {
      if (parse_result e = parse_else_block(scan)) {
        return std::move(e).consume_and_advance(&scan);
      } else {
        return std::nullopt;
      }
    });

    const auto expect_on_close = [&](tok kind) {
      if (!try_consume_token(&scan, kind)) {
        report_fatal_expected(
            scan, "{} to close if-block '{}'", kind, open.to_string());
      }
    };

    expect_on_close(tok::open);
    expect_on_close(tok::slash);
    expect_on_close(tok::kw_if);
    condition = parse_expression(scan.make_fresh());
    if (!condition.has_value()) {
      // Note that this call moves the scan forward to the end of {{/if}} which
      // affects the line number check below.
      expect_on_close(tok::close);
      const bool requires_close_condition =
          line_number_of(*scan_start) != line_number_of(scan.peek());
      if (requires_close_condition) {
        report_fatal_expected(
            scan.prev(), "expression to close if-block '{}'", open.to_string());
      }
    } else {
      ast::expression close = {std::move(condition).consume_and_advance(&scan)};
      if (close != open) {
        report_error(
            scan,
            "conditional-block opening '{}' does not match closing '{}'",
            open.to_string(),
            close.to_string());
      }
      expect_on_close(tok::close);
    }

    return {
        ast::conditional_block{
            scan.with_start(scan_start).range(),
            std::move(open),
            std::move(bodies),
            std::move(else_if_blocks),
            std::move(else_block),
        },
        scan};
  }

  // with-block       → { with-block-open ~ body* ~ with-block-close }
  // with-block-open  → { "{{" ~ "#" ~ "with" ~ expression ~ "}}" }
  // with-block-close → { "{{" ~ "/" ~ "with" ~ "}}" }
  parse_result<ast::with_block> parse_with_block(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_with})) {
      return no_parse_result();
    }
    scan = scan.make_fresh();

    parse_result value = parse_expression(scan);
    if (!value.has_value()) {
      report_fatal_expected(scan, "expression to open with-block");
    }
    ast::expression expr = std::move(value).consume_and_advance(&scan);
    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to open with-block", tok::close);
    }
    scan = scan.make_fresh();

    ast::bodies bodies = parse_bodies(scan).consume_and_advance(&scan);

    const auto expect_on_close = [&](tok kind) {
      if (!try_consume_token(&scan, kind)) {
        report_fatal_expected(
            scan, "{} to close with-block '{}'", kind, expr.to_string());
      }
    };

    expect_on_close(tok::open);
    expect_on_close(tok::slash);
    expect_on_close(tok::kw_with);
    expect_on_close(tok::close);

    return {
        ast::with_block{
            scan.with_start(scan_start).range(),
            std::move(expr),
            std::move(bodies),
        },
        scan};
  }

  // each-block →
  //   { each-block-open ~ body* ~ else-block ~ each-block-close }
  // each-block-open →
  //   { "{{" ~ "#" ~ "each" ~ expression ~ each-block-capture? ~ "}}" }
  // each-block-capture →
  //   { "as" ~ "|" ~ identifier+ ~ "|" }
  // else-block → { "{{" ~ "#" ~ "else" ~ "}}" ~ body* }
  // each-block-close → { "{{" ~ "/" ~ "each" ~ "}}"  }
  parse_result<ast::each_block> parse_each_block(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_each})) {
      return no_parse_result();
    }
    scan = scan.make_fresh();

    parse_result parsed_iterable = parse_expression(scan);
    if (!parsed_iterable.has_value()) {
      report_fatal_expected(scan, "expression to open each-block");
    }
    ast::expression iterable =
        std::move(parsed_iterable).consume_and_advance(&scan);

    auto captured = std::invoke([&]() -> std::vector<ast::identifier> {
      if (!try_consume_token(&scan, tok::kw_as)) {
        return {};
      }
      if (!try_consume_token(&scan, tok::pipe)) {
        report_fatal_expected(scan, "{} to open each-block capture", tok::pipe);
      };

      std::vector<ast::identifier> captures;
      std::unordered_set<std::string_view> seen;

      const token* first_capture = try_consume_token(&scan, tok::identifier);
      if (first_capture == nullptr) {
        report_fatal_error(scan, "expected at least one capture in each-block");
      }
      seen.insert(first_capture->string_value());
      captures.emplace_back(make_identifier(*first_capture));

      while (const token* capture = try_consume_token(&scan, tok::identifier)) {
        if (seen.find(capture->string_value()) != seen.end()) {
          report_fatal_error(
              scan,
              "duplicate capture '{}' in each-block",
              capture->string_value());
        }
        seen.insert(capture->string_value());
        captures.emplace_back(make_identifier(*capture));
      }

      if (!try_consume_token(&scan, tok::pipe)) {
        report_fatal_expected(
            scan, "{} to close each-block capture", tok::pipe);
      };
      return captures;
    });

    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to open each-block", tok::close);
    }
    scan = scan.make_fresh();

    ast::bodies bodies = parse_bodies(scan).consume_and_advance(&scan);

    auto else_block = std::invoke([&]() -> std::optional<ast::else_block> {
      if (parse_result e = parse_else_block(scan)) {
        return std::move(e).consume_and_advance(&scan);
      } else {
        return std::nullopt;
      }
    });

    const auto expect_on_close = [&](tok kind) {
      if (!try_consume_token(&scan, kind)) {
        report_fatal_expected(
            scan, "{} to close each-block '{}'", kind, iterable.to_string());
      }
    };

    expect_on_close(tok::open);
    expect_on_close(tok::slash);
    expect_on_close(tok::kw_each);
    expect_on_close(tok::close);

    return {
        ast::each_block{
            scan.with_start(scan_start).range(),
            std::move(iterable),
            std::move(captured),
            std::move(bodies),
            std::move(else_block),
        },
        scan};
  }

  // partial-block →
  //   { partial-block-open ~ body* ~ partial-block-close }
  // partial-block-open →
  //   { "{{#" ~ "let" ~ "partial" ~ identifier ~
  //      partial-block-args? ~ partial-block-captures? ~ "}}" }
  // partial-block-args →
  //   { "|" ~ identifier+ ~ "|" }
  // partial-block-captures →
  //   { "captures" ~ "|" ~ identifier+ ~ "|" }
  // partial-block-close →
  //   { "{{/" ~ "let" ~ "partial" ~ "}}" }
  parse_result<ast::partial_block> parse_partial_block(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_let})) {
      return no_parse_result();
    }
    const bool exported = try_consume_token(&scan, tok::kw_export) != nullptr;
    if (!try_consume_token(&scan, tok::kw_partial)) {
      return no_parse_result();
    }

    ast::identifier id = std::invoke([&] {
      const token* component = try_consume_token(&scan, tok::identifier);
      if (component == nullptr) {
        report_fatal_expected(scan, "identifier in partial-block");
      }
      return make_identifier(*component);
    });

    const auto expect_on_open = [&](tok kind) {
      if (!try_consume_token(&scan, kind)) {
        report_fatal_expected(
            scan, "{} to open partial-block '{}'", kind, id.name);
      }
    };
    const auto expect_on_close = [&](tok kind) {
      if (!try_consume_token(&scan, kind)) {
        report_fatal_expected(
            scan, "{} to close partial-block '{}'", kind, id.name);
      }
    };

    std::set<ast::identifier, ast::identifier::compare_by_name> arguments;
    if (try_consume_token(&scan, tok::pipe)) {
      const token* first_argument = try_consume_token(&scan, tok::identifier);
      if (first_argument == nullptr) {
        report_fatal_error(
            scan,
            "expected at least one argument in partial-block '{}' but found none",
            id.name);
      }
      arguments.insert(make_identifier(*first_argument));
      while (const token* argument =
                 try_consume_token(&scan, tok::identifier)) {
        auto [_, inserted] = arguments.insert(make_identifier(*argument));
        if (!inserted) {
          report_fatal_error(
              scan,
              "duplicate argument '{}' in partial-block '{}'",
              argument->string_value(),
              id.name);
        }
      }
      expect_on_open(tok::pipe);
    }

    std::set<ast::identifier, ast::identifier::compare_by_name> captures;
    if (try_consume_token(&scan, tok::kw_captures)) {
      expect_on_open(tok::pipe);
      const auto insert_capture = [&](ast::identifier capture) {
        if (arguments.find(capture) != arguments.end()) {
          report_fatal_error(
              scan,
              "capture '{}' conflicts with an argument in partial-block '{}'",
              capture.name,
              id.name);
        }
        auto [_, inserted] = captures.insert(std::move(capture));
        if (!inserted) {
          report_fatal_error(
              scan,
              "duplicate capture '{}' in partial-block '{}'",
              // std::map<...>::insert takes in its argument as an r-value
              // reference and performs a conditional move.
              //
              // From https://en.cppreference.com/w/cpp/container/map/insert:
              //   If the insertion succeeds, `nh` is moved from, otherwise it
              //   retains ownership of the element.
              //
              // Therefore, if `inserted` is false, `capture` is safe to use.
              // Unfortunately, clang-tidy fundamentally cannot reason about
              // conditional moves — this is a limitation of C++.
              //
              // @lint-ignore CLANGTIDY bugprone-use-after-move
              capture.name,
              id.name);
        }
      };

      const token* first_capture = try_consume_token(&scan, tok::identifier);
      if (first_capture == nullptr) {
        report_fatal_error(
            scan,
            "expected at least one capture in partial-block '{}' but found none",
            id.name);
      }
      insert_capture(make_identifier(*first_capture));
      while (const token* capture = try_consume_token(&scan, tok::identifier)) {
        insert_capture(make_identifier(*capture));
      }
      expect_on_open(tok::pipe);
    }

    expect_on_open(tok::close);
    scan = scan.make_fresh();

    ast::bodies bodies = parse_bodies(scan).consume_and_advance(&scan);

    expect_on_close(tok::open);
    expect_on_close(tok::slash);
    expect_on_close(tok::kw_let);
    expect_on_close(tok::kw_partial);
    expect_on_close(tok::close);

    return {
        ast::partial_block{
            scan.with_start(scan_start).range(),
            exported,
            std::move(id),
            std::move(arguments),
            std::move(captures),
            std::move(bodies),
        },
        scan};
  }

  // partial-statement →
  //   { "{{" ~ "#" ~ "partial" ~ expression ~ partial-argument* ~ "}}" }
  // partial-argument → { identifier ~ "=" ~ expression }
  parse_result<ast::partial_statement> parse_partial_statement(
      parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::pound, tok::kw_partial})) {
      return no_parse_result();
    }

    parse_result lookup = parse_expression(scan.make_fresh());
    if (!lookup.has_value()) {
      report_fatal_expected(
          scan, "partial name (expression) in partial-statement");
    }
    ast::expression partial = std::move(lookup).consume_and_advance(&scan);

    decltype(ast::partial_statement::named_arguments) named_arguments;
    using named_argument_entry = decltype(named_arguments)::value_type;
    const auto parse_argument =
        [&](parser_scan_window scan) -> parse_result<named_argument_entry> {
      assert(scan.empty());

      const token* id = try_consume_token(&scan, tok::identifier);
      if (id == nullptr) {
        return no_parse_result();
      }

      if (!try_consume_token(&scan, tok::eq)) {
        report_fatal_expected(
            scan, "{} in partial-statement argument", tok::eq);
      }

      if (parse_result expr = parse_expression(scan.make_fresh())) {
        return {
            named_argument_entry{
                std::string(id->string_value()),
                ast::partial_statement::named_argument{
                    make_identifier(*id),
                    std::move(expr).consume_and_advance(&scan)}},
            scan};
      }
      report_fatal_expected(scan, "expression in partial-statement argument");
    };
    while (parse_result arg = parse_argument(scan)) {
      auto [_, inserted] =
          named_arguments.emplace(std::move(arg).consume_and_advance(&scan));
      if (!inserted) {
        report_error(scan, "duplicate argument name in partial-statement");
      }
    }

    if (named_arguments.empty()) {
      // Arguments could missing because of a failed parse. Let's check the
      // common user errors.
      if (scan.peek().kind == tok::eq) {
        // This case probably means that the user forgot to provide the partial
        // name. Something like:
        //
        //   {{#partial arg1=... arg2=...}}
        //
        // Here, `arg1` will be parsed the partial's name. parse_arguments(...)
        // will see `=...` and fail to parsed.
        report_fatal_expected(
            scan, "partial name (expression) in partial-statement");
      }
      if (parse_expression(scan)) {
        // This case probably means that the user forgot to name an argument.
        // Something like:
        //
        //   {{#partial partial_name arg1 arg2}}
        //
        report_fatal_expected(scan, "argument name in partial-statement");
      }
    }

    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(scan, "{} to close partial-statement", tok::close);
    }

    return {
        ast::partial_statement{
            scan.with_start(scan_start).range(),
            std::move(partial),
            std::move(named_arguments),
            standalone_partial_indentation(scan_start),
        },
        scan};
  }

  // macro → { "{{" ~ ">" ~ macro-lookup ~ "}}" }
  parse_result<ast::macro> parse_macro(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    if (!try_consume_tokens(&scan, {tok::open, tok::gt})) {
      return no_parse_result();
    }
    scan = scan.make_fresh();

    parse_result macro_lookup = parse_macro_lookup(scan.make_fresh());
    if (!macro_lookup.has_value()) {
      report_fatal_expected(scan, "macro-lookup in macro");
    }
    ast::macro_lookup lookup =
        std::move(macro_lookup).consume_and_advance(&scan);

    if (!try_consume_token(&scan, tok::close)) {
      report_fatal_expected(
          scan, "{} to close macro '{}'", tok::close, lookup.as_string());
    }

    return {
        ast::macro{
            scan.with_start(scan_start).range(),
            std::move(lookup),
            standalone_partial_indentation(scan_start)},
        scan};
  }

  // macro-lookup → { path-component ~ ("/" ~ path-component)* }
  parse_result<ast::macro_lookup> parse_macro_lookup(parser_scan_window scan) {
    assert(scan.empty());
    const auto scan_start = scan.start;

    const token* first_component =
        try_consume_token(&scan, tok::path_component);
    if (first_component == nullptr) {
      return no_parse_result();
    }
    std::vector<ast::path_component> path;
    path.emplace_back(make_path_component(*first_component));

    while (try_consume_token(&scan, tok::slash)) {
      const token* component_part =
          try_consume_token(&scan, tok::path_component);
      if (component_part == nullptr) {
        report_fatal_expected(scan, "path-component in macro-lookup");
      }
      path.emplace_back(make_path_component(*component_part));
    }
    return {
        ast::macro_lookup{scan.with_start(scan_start).range(), std::move(path)},
        scan};
  }

 public:
  parser(
      std::vector<token> tokens,
      diagnostics_engine& diags,
      standalone_lines_scanner::result standalone_markings)
      : tokens_(std::move(tokens)),
        diags_(diags),
        standalone_markings_(std::move(standalone_markings)) {}

  std::optional<ast::root> parse() {
    return parse_root(parser_scan_window(
        tokens_.cbegin() /* start */,
        tokens_.cbegin() /* head */,
        tokens_.cend() /* end */));
  }
};

} // namespace

std::optional<ast::root> parse(source src, diagnostics_engine& diags) {
  auto tokens = lexer(std::move(src), diags).tokenize_all();
  auto standalone_scanner_result = standalone_lines_scanner::mark(tokens);
  return parser(std::move(tokens), diags, std::move(standalone_scanner_result))
      .parse();
}

} // namespace whisker
