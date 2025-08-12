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
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/expected.h>
#include <thrift/compiler/whisker/lexer.h>

#include <cstdlib>
#include <functional>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <utility>

namespace whisker {

template <typename... T>
token lexer::diagnoser::report_error(
    const detail::lexer_scan_window& scan,
    fmt::format_string<T...> msg,
    T&&... args) {
  lexer_->diags_.report(
      scan.start_location(),
      diagnostic_level::error,
      msg,
      std::forward<T>(args)...);
  return token(tok::error, scan.range());
}

token lexer::diagnoser::unexpected_token(
    const detail::lexer_scan_window& scan) {
  return report_error(scan, "unexpected token in input: {}", scan.text());
}

namespace {

/**
 * Determines if the given character *could* start an identifier in Whisker.
 */
bool is_identifier_start(char c) {
  return detail::is_letter(c) || c == '_' || c == '$';
}

/**
 * Determines if the given character could be a non-initial character in an
 * identifier in Whisker.
 */
bool is_identifier_continuation(char c) {
  // clang-format off
  return is_identifier_start(c) || detail::is_digit(c)
    || c == '-'
    || c == '+'
    || c == '?';
  // clang-format on
}

/**
 * A path in Whisker must satisfy POSIX's portable file name character set:
 *   https://www.ibm.com/docs/en/zvm/7.3?topic=files-naming
 *
 * A valid path, therefore, will pass the GNU coreutils `pathchk` command (minus
 * length restrictions):
 *   https://man7.org/linux/man-pages/man1/pathchk.1.html
 *   https://github.com/coreutils/coreutils/blob/v9.5/src/pathchk.c#L157-L202
 */
bool is_path_component_continuation(char c) {
  // clang-format off
  return detail::is_letter(c) || detail::is_digit(c)
    || c == '_'
    || c == '.'
    || c == '-';
  // clang-format on
}
/**
 * A leading hyphen is not allowed for POSIX's portable file names.
 */
bool is_path_component_start(char c) {
  return c != '-' && is_path_component_continuation(c);
}

/**
 * Advances the scan window past zero or more whitespace characters.
 */
void skip_whitespace(detail::lexer_scan_window* scan) {
  while (detail::is_whitespace(scan->peek())) {
    scan->advance();
  }
}

/**
 * Result of a scan which resulted in a token.
 * The (now advanced) scan_window is packaged with the token so the lexer can
 * move up its cursor.
 */
struct lexed_token {
  lexed_token(token t, const detail::lexer_scan_window& advanced)
      : value_{std::move(t)}, new_head_{advanced.make_fresh()} {}

  /**
   * Advances the provided scan_window to the end of the token, then returns the
   * token. This method ensures that consuming the token cannot be done without
   * accidentally forgetting to advance the lexer's cursor.
   */
  [[nodiscard]] token advance_to_token(detail::lexer_scan_window* scan) && {
    assert(scan);
    *scan = std::move(new_head_);
    return std::move(value_);
  }

 private:
  token value_;
  detail::lexer_scan_window new_head_;
};

/**
 * Marker struct to indicate that a scan did not result in a token.
 */
struct no_lex_result {};

/**
 * Result of a scan which resulted in a token.
 * The (now advanced) scan_window is packaged with the token so the lexer can
 * move up its cursor.
 */
struct [[nodiscard]] lex_result : private expected<lexed_token, no_lex_result> {
 private:
  using base = expected<lexed_token, no_lex_result>;

 public:
  lex_result(token t, const detail::lexer_scan_window& advanced)
      : base(std::in_place, std::move(t), advanced) {}
  /* implicit */ lex_result(no_lex_result) : base(unexpect) {}

  [[nodiscard]] token advance_to_token(detail::lexer_scan_window* scan) && {
    assert(has_value());
    return std::move(**this).advance_to_token(scan);
  }

  using base::operator bool;
  using base::has_value;
  using base::operator*;
  using base::operator->;
};

// Returns the skipped scan_window if the comment is escaped "{{--"
std::optional<detail::lexer_scan_window> lex_comment_escape(
    detail::lexer_scan_window scan) {
  for (int i = 0; i < 2; ++i) {
    if (scan.advance() != '-') {
      return std::nullopt;
    }
  }
  return scan.make_fresh();
}

lex_result lex_punctuation(detail::lexer_scan_window scan) {
  char c = scan.advance();
  if (auto punct = token_detail::to_tok(c); punct != tok::error) {
    return lex_result(token(punct, scan.range()), scan);
  }
  return no_lex_result();
}

// Looks for identifiers or keywords. This implementation assumes that all
// keywords *could* have been valid identifiers.
lex_result lex_identifier_or_keyword(detail::lexer_scan_window scan) {
  char c = scan.advance();
  if (!is_identifier_start(c)) {
    return no_lex_result();
  }
  // This implementation assumes that is_identifier_continuation() implies
  // is_identifier_start(). Otherwise, one word (i.e. no whitespace in between)
  // could be interpreted as two identifiers.
  assert(is_identifier_continuation(c));

  while (true) {
    if (!scan.can_advance() || !is_identifier_continuation(scan.peek())) {
      if (auto kw = token::keywords().find(scan.text());
          kw != token::keywords().end()) {
        return lex_result(token(kw->second, scan.range()), scan);
      }
      return lex_result(
          token::make_identifier(scan.text(), scan.range()), scan);
    }
    scan.advance();
  }
}

lex_result lex_path_component(detail::lexer_scan_window scan) {
  char c = scan.advance();
  if (!is_path_component_start(c)) {
    return no_lex_result();
  }
  while (scan.can_advance() && is_path_component_continuation(scan.peek())) {
    scan.advance();
  }
  return lex_result(
      token::make_path_component(scan.text(), scan.range()), scan);
}

lex_result lex_path_separator(detail::lexer_scan_window scan) {
  return scan.advance() == '/'
      ? lex_result(token(tok::slash, scan.range()), scan)
      : no_lex_result();
}

lex_result lex_i64_literal(
    detail::lexer_scan_window scan, lexer::diagnoser diagnoser) {
  assert(scan.empty());
  const detail::lexer_scan_window original = scan;

  bool is_negative = false;
  if (scan.peek() == '-') {
    scan.advance();
    is_negative = true;
    skip_whitespace(&scan);
  }
  scan = scan.make_fresh();

  const auto& report_out_of_range =
      [&diagnoser, &original, is_negative](
          const detail::lexer_scan_window& s) -> lex_result {
    // return the whole scan, including whitespace we might have skipped
    auto whole = s.with_start(original.start);
    return {
        diagnoser.report_error(
            whole,
            "i64 literal out of range: {}{}",
            is_negative ? "-" : "",
            s.text()),
        whole};
  };

  while (scan.can_advance() && detail::is_digit(scan.peek())) {
    scan.advance();
  }
  std::string_view text = scan.text();
  if (text.empty()) {
    if (is_negative) {
      // lone '-' is an error
      auto minus_scan = original;
      minus_scan.advance();
      return lex_result(diagnoser.unexpected_token(minus_scan), minus_scan);
    }
    return no_lex_result();
  }

  char* end = nullptr;
  constexpr auto i64_max = 9223372036854775807ull; //  2^63-1
  constexpr auto i64_min = 9223372036854775808ull; // -2^63
  unsigned long long value = std::strtoull(text.data(), &end, 10 /* base */);
  if (errno == ERANGE) {
    errno = 0;
    return report_out_of_range(scan);
  }
  if (is_negative && value > i64_min) {
    return report_out_of_range(scan);
  }
  if (!is_negative && value > i64_max) {
    return report_out_of_range(scan);
  }
  // We should be parsing all the digits
  assert(end == text.data() + text.size());

  auto i64_value = static_cast<std::int64_t>(is_negative ? -value : value);
  return lex_result(
      token::make_i64_literal(
          i64_value, scan.with_start(original.start).range()),
      scan);
}

const std::unordered_map<char, char> escaped_characters = {
    // https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences
    {'n', '\n'},
    {'r', '\r'},
    {'t', '\t'},
    {'\\', '\\'},
    {'\'', '\''},
    {'"', '\"'},
};

lex_result lex_string_literal(
    detail::lexer_scan_window scan, lexer::diagnoser diagnoser) {
  char c = scan.advance();
  if (c != '"') {
    return no_lex_result();
  }
  std::string value;
  while (scan.can_advance()) {
    c = scan.advance();
    if (c == '\\') {
      if (auto escaped = escaped_characters.find(scan.peek());
          escaped != escaped_characters.end()) {
        scan.advance();
        value.push_back(escaped->second);
        continue;
      } else {
        return lex_result(
            diagnoser.report_error(
                scan,
                "unknown escape character in string literal: '\\{}'",
                scan.peek()),
            scan);
      }
    }
    if (detail::is_newline(c)) {
      return lex_result(
          diagnoser.report_error(scan, "unexpected newline in string literal"),
          scan);
    }
    if (c == '"') {
      // End of string reached
      return lex_result(
          token::make_string_literal(std::move(value), scan.range()), scan);
    }
    value.push_back(c);
  }
  return lex_result(
      diagnoser.report_error(scan, "unterminated string literal"), scan);
}

// Looks for "}}" or "--}}" and produces tok::close if found
lex_result lex_comment_close(detail::lexer_scan_window scan, bool escaped) {
  assert(scan.empty());
  if (escaped) {
    // "--"
    for (int i = 0; i < 2; ++i) {
      if (scan.advance() != '-') {
        return no_lex_result();
      }
    }
    // ignore the escape syntax
    scan = scan.make_fresh();
  }
  // "}}"
  for (int i = 0; i < 2; ++i) {
    if (scan.advance() != '}') {
      return no_lex_result();
    }
  }
  return lex_result(token(tok::close, scan.range()), scan);
}

// Looks for "}}" and produces tok::close if found
lex_result lex_close(detail::lexer_scan_window scan) {
  assert(scan.empty());
  // "}}"
  for (int i = 0; i < 2; ++i) {
    if (scan.advance() != '}') {
      return no_lex_result();
    }
  }
  return lex_result(token(tok::close, scan.range()), scan);
}

// Lexes common template parts (to be shared between different lexer states)
lex_result lex_template_part(
    detail::lexer_scan_window scan, lexer::diagnoser diagnoser) {
  if (lex_result punct = lex_punctuation(scan)) {
    return punct;
  }
  if (lex_result identifier = lex_identifier_or_keyword(scan)) {
    return identifier;
  }
  if (lex_result i64_literal = lex_i64_literal(scan, diagnoser)) {
    return i64_literal;
  }
  if (lex_result string_literal = lex_string_literal(scan, diagnoser)) {
    return string_literal;
  }
  return no_lex_result();
}

} // namespace

/**
 * Reads all text as-is (including whitespace and special characters) until
 * lexing encounters "{{" (modulo "\{{" escape).
 */
class lexer::state_text : public lexer::state_base {
  result next(lexer& lex) override;
};

/**
 * Reads all text in a comment "{{!" or "{{!--" until lexing encounters "}}" or
 * "--}}" respectively.
 *
 * This implementation assumes that we transition after "{{" but before "!".
 */
class lexer::state_comment : public lexer::state_base {
  result next(lexer& lex) override {
    auto& scan = lex.scan_window_;

    if (!init_.has_value()) {
      [[maybe_unused]] char c = scan.advance();
      assert(c == '!');
      auto bang_token = token(tok::bang, scan.range());
      if (auto found = lex_comment_escape(scan)) {
        scan = *found;
        init_ = {true /* is_escaped */};
      } else {
        init_ = {false /* is_escaped */};
      }
      return std::move(bang_token);
    }

    std::string text;
    while (scan.can_advance()) {
      if (lex_result close =
              lex_comment_close(scan.make_fresh(), init_->is_escaped)) {
        // We've found the close token. Finish off the current text token.
        std::vector<token> tokens;
        if (!text.empty()) {
          tokens.push_back(token::make_text(text, scan.range()));
        }
        tokens.push_back(std::move(close).advance_to_token(&scan));
        return {std::move(tokens), std::make_unique<lexer::state_text>()};
      }
      text += scan.advance();
    }
    return text.empty() ? token(tok::eof, scan.range())
                        : token::make_text(std::move(text), scan.range());
  }

  struct init {
    bool is_escaped;
  };
  std::optional<init> init_;
};

/**
 * Reads the templating language constructs in between "{{" and "}}" (except
 * macros and comments).
 */
class lexer::state_template : public lexer::state_base {
  result next(lexer& lex) override {
    auto& scan = lex.scan_window_;
    assert(scan.empty());

    // in template bodies, whitespace is ignored (unlike in raw text)
    skip_whitespace(&scan);
    scan = scan.make_fresh();

    if (!scan.can_advance()) {
      return token(tok::eof, scan.range());
    }

    if (lex_result close = lex_close(scan)) {
      return {
          std::move(close).advance_to_token(&scan),
          std::make_unique<lexer::state_text>()};
    }
    if (lex_result any = lex_template_part(scan, lex.diagnose())) {
      return std::move(any).advance_to_token(&scan);
    }
    // We don't know what the next token is
    scan.advance();
    return lex.diagnose().unexpected_token();
  }
};

/**
 * Reads the templating language constructs in between "{{>" and "}}" (macros).
 *
 * Macros get their own state primarily because identifers can be ambiguous with
 * path components. So instead we opt to emit tok::path_component exclusively in
 * macro application contexts.
 *
 * This implementation assumes we transition after "{{" but before ">".
 */
class lexer::state_macro : public lexer::state_base {
  result next(lexer& lex) override {
    auto& scan = lex.scan_window_;
    assert(scan.empty());

    // in template bodies, whitespace is ignored (unlike in raw text)
    skip_whitespace(&scan);
    if (!init_) {
      init_ = true;
      [[maybe_unused]] char c = scan.advance();
      assert(c == '>');
      return token(tok::gt, scan.range());
    }

    scan = scan.make_fresh();

    if (!scan.can_advance()) {
      return token(tok::eof, scan.range());
    }

    if (lex_result close = lex_close(scan)) {
      return {
          std::move(close).advance_to_token(&scan),
          std::make_unique<lexer::state_text>()};
    }
    if (lex_result path_separator = lex_path_separator(scan)) {
      return std::move(path_separator).advance_to_token(&scan);
    }
    if (lex_result path_component = lex_path_component(scan)) {
      return std::move(path_component).advance_to_token(&scan);
    }
    // Fall back to common template tokenization. Most likely, this will fail
    // in the parser but that will result in a better error message.
    if (lex_result any = lex_template_part(scan, lex.diagnose())) {
      return std::move(any).advance_to_token(&scan);
    }

    // We don't know what the next token is
    scan.advance();
    return lex.diagnose().unexpected_token();
  }

  bool init_ = false;
};

/**
 * Forever returns the same terminal token
 */
class lexer::state_terminal : public lexer::state_base {
 public:
  explicit state_terminal(const token& terminal_token)
      : terminal_token_(terminal_token) {
    assert(token_kind::is_terminal(terminal_token_.kind));
  }

  result next(lexer&) override { return terminal_token_; }

 private:
  const token terminal_token_;
};

lexer::state_text::result lexer::state_text::next(lexer& lex) {
  auto& scan = lex.scan_window_;

  // The text buffer alternates between creating tok::text and tok::whitespace
  // based on runs of whitespace and non-whitespace text.
  struct text_buffer {
    text_buffer() = default;

    void consume_one(detail::lexer_scan_window* scan) {
      assert(!detail::is_newline(scan->peek()));
      bool is_whitespace = detail::is_whitespace(scan->peek());

      if (!current_run_.has_value()) {
        // Let's decide what the next token is going to be
        current_run_.emplace(
            run{is_whitespace, std::string(1, scan->advance())});
        return;
      }
      assert(!current_run_->content.empty());
      if (current_run_->is_whitespace == is_whitespace) {
        // Continue the text or whitespace run
        current_run_->content += scan->advance();
        return;
      }
      // The current run has ended
      tokens_.emplace_back(std::move(*current_run_).to_token(*scan));
      *scan = scan->make_fresh();
      current_run_.emplace(run{is_whitespace, std::string(1, scan->advance())});
    }

    [[nodiscard]] bool empty() const {
      return tokens_.empty() && !current_run_.has_value();
    }

    std::vector<token> to_tokens(const detail::lexer_scan_window& scan) && {
      if (current_run_.has_value()) {
        assert(!current_run_->content.empty());
        // Forcefully end the current run
        tokens_.emplace_back(std::move(*current_run_).to_token(scan));
        current_run_.reset();
      }
      return std::move(tokens_);
    }

   private:
    std::vector<token> tokens_;

    struct run {
      const bool is_whitespace = true;
      std::string content;

      token to_token(const detail::lexer_scan_window& scan) && {
        auto* token_factory =
            is_whitespace ? &token::make_whitespace : &token::make_text;
        return token_factory(std::move(content), scan.range());
      }
    };
    std::optional<run> current_run_;
  };

  text_buffer buffer;
  while (scan.can_advance()) {
    char c = scan.peek();
    if (c == '\\' && scan.next().peek() == '{') {
      // greedily consume "\{" so that "\{{" is lexed as text
      scan.advance();
      buffer.consume_one(&scan);
      continue;
    }
    if (detail::is_newline(c)) {
      std::vector<token> tokens = std::move(buffer).to_tokens(scan);
      scan = scan.make_fresh();
      // "\r", "\n", or "\r\n"
      char n = scan.advance();
      if (n == '\r' && scan.peek() == '\n') {
        scan.advance();
      }
      tokens.emplace_back(token::make_newline(scan.text(), scan.range()));
      return {std::move(tokens)};
    }
    if (c == '{' && scan.next().peek() == '{') {
      auto scan_within = scan.make_fresh().next(2);
      auto open_token = token(tok::open, scan_within.range());
      using ptr = std::unique_ptr<lexer::state_base>;
      auto transition = std::invoke([&] {
        if (scan_within.peek() == '!') {
          return ptr(std::make_unique<lexer::state_comment>());
        }
        return scan_within.peek() == '>'
            ? ptr(std::make_unique<lexer::state_macro>())
            : ptr(std::make_unique<lexer::state_template>());
      });
      std::vector<token> tokens = std::move(buffer).to_tokens(scan);
      tokens.emplace_back(std::move(open_token));
      // Move up the scan_window to the '!'
      scan = std::move(scan_within);
      return {std::move(tokens), std::move(transition)};
    }
    buffer.consume_one(&scan);
  }

  if (buffer.empty()) {
    return token(tok::eof, scan.range());
  }
  return std::move(buffer).to_tokens(scan);
}

lexer::lexer(source source, diagnostics_engine& diags)
    : source_(std::move(source)),
      diags_(diags),
      scan_window_(source_, source_.text.begin()),
      state_(std::make_unique<state_text>()) {}

void lexer::terminate_with(const token& t) {
  transition_to(std::make_unique<state_terminal>(t));
}

void lexer::do_tokenize() {
  assert(state_);
  assert(queued_tokens_.empty());
  const auto queue_token = [this](token&& t) -> const token& {
    queued_tokens_.push(std::move(t));
    return queued_tokens_.back();
  };

  auto next = state_->next(*this);
  detail::variant_match(
      std::move(next.tokens),
      [&](std::monostate) {
        // next() wants to transition only
        assert(next.transition != nullptr);
        transition_to(std::move(next.transition));
        do_tokenize();
      },
      [&](token&& t) {
        if (const auto& pushed_token = queue_token(std::move(t));
            token_kind::is_terminal(pushed_token.kind)) {
          // tok::eof or tok::error must terminate lexing
          assert(next.transition == nullptr);
          terminate_with(pushed_token);
        } else if (next.transition) {
          transition_to(std::move(next.transition));
        }
      },
      [&](std::vector<token>&& tokens) {
        assert(!tokens.empty());
        for (auto t = tokens.begin(); t != tokens.end(); ++t) {
          if (const auto& pushed_token = queue_token(std::move(*t));
              token_kind::is_terminal(pushed_token.kind)) {
            assert(next.transition == nullptr);
            assert(std::next(t) == tokens.end());
            terminate_with(pushed_token);
            break;
          }
        }
        if (next.transition) {
          transition_to(std::move(next.transition));
        }
      });

  assert(!queued_tokens_.empty());
}

token lexer::next_token() {
  scan_window_ = scan_window_.make_fresh();
  if (queued_tokens_.empty()) {
    do_tokenize();
  }
  assert(!queued_tokens_.empty());
  auto token = std::move(queued_tokens_.front());
  queued_tokens_.pop();

  // tok::eof without a real EOF is definitely a bug!
  assert(!(token.kind == tok::eof && scan_window_.can_advance()));
  return token;
}

std::vector<token> lexer::tokenize_all() {
  std::vector<token> tokens;
  while (true) {
    auto token = next_token();
    tok kind = token.kind;
    tokens.push_back(std::move(token));
    if (token_kind::is_terminal(kind)) {
      break;
    }
  }
  return tokens;
}

} // namespace whisker
