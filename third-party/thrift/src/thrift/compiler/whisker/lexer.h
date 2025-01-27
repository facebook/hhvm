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

#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/source_location.h>
#include <thrift/compiler/whisker/token.h>

#include <cassert>
#include <iterator>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include <fmt/core.h>

namespace whisker {

class lexer;

namespace detail {

/**
 * A lexer_scan_window represents a single pass through the input text by the
 * lexer.
 *
 * The start and head of the lexer_scan_window are used to track the current
 * position in the input text. start represents the beginning of the current
 * token, while the head is the current position of the lexer. lexer_scan_window
 * provides methods to advance the head position, peek at the next character,
 * and create new scan windows with different start or head positions.
 *
 * The head advances as lexing progresses, and the start is moved up to the head
 * after each token is produced.
 *
 * lexer_scan_window is copyable which means that backtracking can be achieved
 * by advancing copies of the window, then disposing the copy if lexing fails.
 */
struct lexer_scan_window {
  using cursor = std::string_view::const_iterator;

  const source* src;
  cursor start; // Start of the current token.
  cursor head; // Current position in the source.

  lexer_scan_window(const source& src, cursor start, cursor head)
      : src(&src), start(start), head(head) {
    assert(start <= head);
  }

  lexer_scan_window(const source& src, cursor start)
      : lexer_scan_window(src, start, start /* head */) {}

  /**
   * Determines if the head is at the end of the input or not.
   * If this returns false, then both advance() and peek() will return the NUL
   * character.
   */
  bool can_advance() const {
    // The source text includes the NUL character at the end as part of the
    // string view. We don't want to actually advance to it.
    return std::next(head) < src->text.end();
  }
  /**
   * Advances the head of the scan by one and returns the encountered
   * character. If the head is already at the end of the input, then there is
   * no changed state and the NUL character is returned.
   */
  char advance() {
    if (!can_advance()) {
      // Don't advance past the end of the buffer
      return '\0';
    }
    return *head++;
  }
  /**
   * Creates a copy of this scan_window and advances the head n times.
   */
  [[nodiscard]] lexer_scan_window next(std::size_t n = 1) {
    return with_head(std::min(std::prev(src->text.end()), std::next(head, n)));
  }

  /**
   * Returns the next character without consuming it. This function produces
   * the same value as advance() but does not move the head.
   */
  [[nodiscard]] char peek() const {
    if (!can_advance()) {
      return '\0';
    }
    return *head;
  }

  /**
   * Returns a new "fresh" scan_window whose start is moved to the current head.
   * Typically, this indicates that the string in the current scan_window has
   * been consumed. The result is an empty scan_window.
   */
  [[nodiscard]] lexer_scan_window make_fresh() const {
    return with_start(head);
  }
  [[nodiscard]] bool empty() const { return start == head; }

  lexer_scan_window with_start(cursor new_start) const {
    assert(new_start <= head);
    return lexer_scan_window(*src, new_start, head);
  }
  lexer_scan_window with_head(cursor new_head) const {
    assert(start <= new_head);
    return lexer_scan_window(*src, start, new_head);
  }

  std::string_view text() const {
    assert(start <= head);
    return {&*start, static_cast<std::size_t>(std::distance(start, head))};
  }

  source_location start_location() const { return location(*src, start); }
  source_location head_location() const { return location(*src, head); }
  source_range range() const { return {start_location(), head_location()}; }

 private:
  static source_location location(const source& src, cursor c) {
    return src.start + (c - src.text.begin());
  }
};

} // namespace detail

/**
 * A lexer for the Whisker templating language. This class is responsible for
 * breaking down the source code into individual tokens that can be used by a
 * parser.
 *
 * The lexer uses a state machine to handle different contexts, such as raw text
 * and interpolated variables. It also provides a way to report errors and
 * terminate lexing when a terminal token (tok::eof or tok::error) is reached.
 */
class lexer {
 public:
  lexer(source, diagnostics_engine&);

  /**
   * Advances the lexer and returns the next token that it encounters. Lexing is
   * terminated once a terminal token (tok::eof or tok::error) is produced. Any
   * subsequent calls to next_token() will return the same terminal token.
   *
   * For tok::error, there may be additional information passed to the
   * diagnostics_engine.
   */
  token next_token();

  /**
   * Reads all tokens by calling next_token() until a terminal token is read.
   * The returned list of tokens has the following guarantees:
   *   - The last element is a terminal token (see token_kind::is_terminal).
   *     This implies that the list is non-empty.
   *   - There are no terminal tokens except for the last token.
   *
   * Post-conditions:
   *   - next_token() will return the same terminal token as the last token in
   *     the returned list.
   */
  std::vector<token> tokenize_all();

 private:
  source source_;
  diagnostics_engine& diags_;
  detail::lexer_scan_window scan_window_;

  /**
   * The base class for all lexer states. Each state represents a specific
   * context in the source code, such as raw text or interpolated variables.
   */
  class state_base {
   public:
    virtual ~state_base() = default;
    /**
     * The result of calling next on a state object. This can include zero or
     * more tokens, as well as a transition to another state.
     */
    struct result {
      std::variant<std::monostate, token, std::vector<token>> tokens;
      std::unique_ptr<state_base> transition;

      /* implicit */ result(token t) : tokens(std::move(t)) {}
      /* implicit */ result(std::vector<token> tokens)
          : tokens(std::move(tokens)) {
        assert(!std::get<std::vector<token>>(this->tokens).empty());
      }
      /* implicit */ result(std::unique_ptr<state_base> transition)
          : transition(std::move(transition)) {
        assert(this->transition != nullptr);
      }
      result(token t, std::unique_ptr<state_base> transition)
          : tokens(std::move(t)), transition(std::move(transition)) {
        assert(this->transition != nullptr);
      }
      result(std::vector<token> tokens, std::unique_ptr<state_base> transition)
          : tokens(std::move(tokens)), transition(std::move(transition)) {
        assert(!std::get<std::vector<token>>(this->tokens).empty());
        assert(this->transition != nullptr);
      }
    };
    virtual result next(lexer&) = 0;
  };
  // parses raw text
  class state_text;
  // parses interpolation of variables "{{ ... }}" or blocks "{{# ...}}"
  class state_template;
  // parses interpolation of macros "{{> ... }}"
  class state_macro;
  // parses the comment syntax "{{! ...}}" or "{{!-- ... --}}"
  class state_comment;
  // forever returns the terminal token
  class state_terminal;

  std::unique_ptr<state_base> state_;
  void transition_to(std::unique_ptr<state_base> state) {
    state_ = std::move(state);
  }

  void do_tokenize();
  std::queue<token> queued_tokens_;

  void terminate_with(const token&);

 public:
  /**
   * A helper class to make it easier to report errors for the lexer
   * implementation.
   */
  class diagnoser {
   public:
    template <typename... T>
    [[nodiscard]] token report_error(
        const detail::lexer_scan_window& scan,
        fmt::format_string<T...> msg,
        T&&... args);
    template <typename... T>
    [[nodiscard]] token report_error(
        fmt::format_string<T...> msg, T&&... args) {
      return report_error(
          lexer_->scan_window_, std::move(msg), std::forward<T>(args)...);
    }

    [[nodiscard]] token unexpected_token(const detail::lexer_scan_window& scan);
    [[nodiscard]] token unexpected_token() {
      return unexpected_token(lexer_->scan_window_);
    }

   private:
    explicit diagnoser(lexer& self) : lexer_(&self) {}
    friend class lexer;

    lexer* lexer_;
  };

 private:
  diagnoser diagnose() { return diagnoser(*this); }
};

} // namespace whisker
