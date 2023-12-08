// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::ops::DerefMut;
use std::rc::Rc;

use parser_core_types::lexable_token::LexableToken;
use parser_core_types::lexable_trivia::LexableTrivia;
use parser_core_types::lexable_trivia::LexableTrivium;
use parser_core_types::source_text::SourceText;
use parser_core_types::source_text::INVALID;
use parser_core_types::syntax_error::Error;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_error::{self as Errors};
use parser_core_types::token_factory::TokenFactory;
use parser_core_types::token_factory::Trivia;
use parser_core_types::token_factory::Trivium;
use parser_core_types::token_kind::TokenKind;
use parser_core_types::trivia_factory::TriviaFactory;
use parser_core_types::trivia_kind::TriviaKind;
use static_assertions::*;

#[derive(Debug)]
struct LexerPreSnapshot {
    start: usize,
    offset: usize,
    in_type: bool,
}

#[derive(Debug)]
struct LexerPostSnapshot {
    start: usize,
    offset: usize,
    in_type: bool,
    errors: Vec<SyntaxError>,
}

impl<'a, TF> PartialEq<Lexer<'a, TF>> for LexerPreSnapshot
where
    TF: TokenFactory,
{
    fn eq(&self, other: &Lexer<'a, TF>) -> bool {
        self.start == other.start && self.offset == other.offset && self.in_type == other.in_type
    }
}

/*
Lexer Caching

One token look ahead in parser is implemented by `parser.peek_token()` ... `parser.next_token()`.
Re-scanning in next_token can be avoided by caching the result of `peek_token`, consecutive
`peek_token`s can also get improved.

`Lexer.peek_next_token()` checks cache first if cache misses it will clone of the current lexer and
call next_token on cloned lexer. To cache the result, it takes a snapshot of lexer state before and
after calling next_token, and store them in current lexer.

Clone trait of Lexer is derived automatically, therefore `cache: Rc<...>` is also cloned. `Rc` ensures
cloned lexer and original lexer share the same cache, this is intended! Other than one token look
ahead still clones parser, therefore lexer get cloned, sharing cache allows cloned lexer uses
cache from original lexer and vise versa. It is measured that 2% faster than not sharing cache.

NOTE: There is an invariant assumed by this caching mechanism. `errors` in `Lexer` can only add new errors
and must not remove any error when scanning forward! `Lexer.peek_next_token()` clones a new `Lexer` and
reset `errors` to empty, look ahead may accumulate new errors and these errors will be appended to the original
`Lexer`. The reason we need this invariant is that between `peek_next_token` and `next_token` we can not
prove no new error added. Actually it is observed that new errors are added between these two calls.
*/
#[derive(Debug)]
struct LexerCache<Token>(LexerPreSnapshot, Token, LexerPostSnapshot);

#[derive(Debug, Clone)]
pub struct Lexer<'a, TF>
where
    TF: TokenFactory,
{
    source: SourceText<'a>,
    start: usize,
    offset: usize,
    errors: Vec<SyntaxError>,
    in_type: bool,
    token_factory: TF,
    cache: Rc<RefCell<Option<LexerCache<TF::Token>>>>,
}

#[derive(Debug, PartialEq)]
pub enum StringLiteralKind {
    LiteralDoubleQuoted,
    LiteralHeredoc { heredoc: Vec<u8> },
}

#[derive(Debug, Copy, Clone)]
pub enum KwSet {
    AllKeywords,
    NonReservedKeywords,
    NoKeywords,
}

macro_rules! as_case_insensitive_keyword {
    ($size:tt $(, $keyword:tt)+) => {
        fn as_case_insensitive_keyword(&self, text: &str) -> Option<(&'static str, bool)> {
            // - The $size should be greater than or equal to the each length of keyword
            // - The $size should be equal to at least one of the length of a keyword
            // Therefore, $size is equal to the length of the longest keyword.
            $(
                const_assert!($size >= $keyword.len());
            )*
            const_assert!(
                $(
                    $size == $keyword.len() ||
                )*
                false
            );

            if text.len() > $size {
                None
            } else {
                let mut t: heapless::String<$size> = text.try_into().unwrap();
                let t: &mut str = t.as_mut_str();
                t.make_ascii_lowercase();
                let has_upper = t != text;
                let t: &str = t as &str;
                match t {
                    $(
                        $keyword => Some(($keyword, has_upper)),
                    )*
                    _ => None,
                }
            }
        }
    }
}

impl<'a, TF> Lexer<'a, TF>
where
    TF: TokenFactory,
{
    fn to_lexer_pre_snapshot(&self) -> LexerPreSnapshot {
        LexerPreSnapshot {
            start: self.start,
            offset: self.offset,
            in_type: self.in_type,
        }
    }

    fn into_lexer_post_snapshot(self) -> LexerPostSnapshot {
        LexerPostSnapshot {
            start: self.start,
            offset: self.offset,
            in_type: self.in_type,
            errors: self.errors,
        }
    }

    pub fn make_at(source: &SourceText<'a>, offset: usize, token_factory: TF) -> Self {
        Self {
            source: source.clone(),
            start: offset,
            offset,
            errors: vec![],
            in_type: false,
            cache: Rc::new(RefCell::new(None)),
            token_factory,
        }
    }

    pub fn make(source: &SourceText<'a>, token_factory: TF) -> Self {
        Self::make_at(source, 0, token_factory)
    }

    fn continue_from(&mut self, l: Lexer<'a, TF>) {
        self.start = l.start;
        self.offset = l.offset;
        self.errors = l.errors
    }

    pub fn start(&self) -> usize {
        self.start
    }

    pub fn offset(&self) -> usize {
        self.offset
    }

    pub fn errors(&self) -> &[SyntaxError] {
        &self.errors
    }

    fn with_error(&mut self, error: Error) {
        let error = SyntaxError::make(self.start(), self.offset(), error, vec![]);
        self.errors.push(error)
    }

    fn with_offset(&mut self, offset: usize) {
        self.offset = offset
    }

    fn with_start_offset(&mut self, start: usize, offset: usize) {
        self.start = start;
        self.offset = offset;
    }

    fn start_new_lexeme(&mut self) {
        self.start = self.offset
    }

    pub fn advance(&mut self, i: usize) {
        self.offset += i
    }

    pub fn set_in_type(&mut self, in_type: bool) {
        self.in_type = in_type
    }

    pub fn source(&self) -> &SourceText<'a> {
        &self.source
    }

    fn source_text_string(&self) -> &[u8] {
        self.source.text()
    }

    // Housekeeping

    pub fn peek_char(&self, index: usize) -> char {
        self.source.get(self.offset() + index)
    }

    fn peek_string(&self, size: usize) -> &[u8] {
        self.source.sub(self.offset, size)
    }

    fn match_string(&self, s: &[u8]) -> bool {
        s == self.peek_string(s.len())
    }

    fn width(&self) -> usize {
        self.offset - self.start
    }

    fn current_text(&self) -> &[u8] {
        self.source.sub(self.start, self.width())
    }

    fn current_text_as_str(&self) -> &str {
        unsafe { std::str::from_utf8_unchecked(self.current_text()) }
    }

    fn at_end(&self) -> bool {
        self.offset() >= self.source.length()
    }

    fn remaining(&self) -> usize {
        let r = (self.source.length() as isize) - (self.offset as isize);
        if r < 0 { 0 } else { r as usize }
    }

    fn peek(&self, i: usize) -> char {
        self.source.get(i)
    }

    fn peek_back(&self, index: usize) -> char {
        self.source.get(self.offset() - index)
    }

    fn peek_def(&self, index: usize, default: char) -> char {
        if index >= self.source.length() {
            default
        } else {
            self.source.get(index)
        }
    }

    // Character classification

    fn is_whitespace_no_newline(c: char) -> bool {
        match c {
            ' ' | '\t' => true,
            _ => false,
        }
    }

    fn is_newline(ch: char) -> bool {
        match ch {
            '\r' | '\n' => true,
            _ => false,
        }
    }

    fn is_binary_digit(ch: char) -> bool {
        match ch {
            '0' | '1' => true,
            _ => false,
        }
    }

    fn is_octal_digit(c: char) -> bool {
        ('0'..='7').contains(&c)
    }

    fn is_decimal_digit(ch: char) -> bool {
        ('0'..='9').contains(&ch)
    }

    fn is_hexadecimal_digit(c: char) -> bool {
        ('0'..='9').contains(&c) || ('a'..='f').contains(&c) || ('A'..='F').contains(&c)
    }

    fn is_name_nondigit(c: char) -> bool {
        (c == '_') || ('a'..='z').contains(&c) || ('A'..='Z').contains(&c) || ('\x7f' <= c)
    }

    fn is_name_letter(c: char) -> bool {
        (c == '_')
            || ('0'..='9').contains(&c)
            || ('a'..='z').contains(&c)
            || ('A'..='Z').contains(&c)
            || ('\x7f' <= c)
    }

    // Lexing

    fn skip_while_to_offset(&self, p: impl Fn(char) -> bool) -> usize {
        let n = self.source.length();
        let mut i = self.offset();
        while i < n && p(self.peek(i)) {
            i += 1;
        }
        i
    }

    // advance offset as long as the predicate is true
    fn skip_while(&mut self, p: impl Fn(char) -> bool) {
        self.with_offset(self.skip_while_to_offset(p))
    }

    fn str_skip_while(s: &[u8], mut i: usize, p: impl Fn(char) -> bool) -> usize {
        let n = s.len();
        loop {
            if i < n && p(s[i] as char) {
                i += 1
            } else {
                return i;
            }
        }
    }

    fn skip_whitespace(&mut self) {
        self.skip_while(Self::is_whitespace_no_newline);
    }

    fn str_skip_whitespace(s: &[u8], i: usize) -> usize {
        Self::str_skip_while(s, i, Self::is_whitespace_no_newline)
    }

    fn not_newline(ch: char) -> bool {
        !(Self::is_newline(ch))
    }

    fn skip_to_end_of_line(&mut self) {
        self.skip_while(Self::not_newline)
    }

    fn skip_name_end(&mut self) {
        self.skip_while(Self::is_name_letter)
    }

    fn skip_end_of_line(&mut self) {
        match self.peek_char(0) {
            '\n' => self.advance(1),
            '\r' => {
                if self.peek_char(1) == '\n' {
                    self.advance(2)
                } else {
                    self.advance(1)
                }
            }
            _ => {}
        }
    }

    fn scan_name_impl(&mut self) {
        assert!(Self::is_name_nondigit(self.peek_char(0)));
        self.advance(1);
        self.skip_name_end();
    }

    fn scan_name(&mut self) -> TokenKind {
        self.scan_name_impl();
        TokenKind::Name
    }

    fn scan_variable(&mut self) -> TokenKind {
        assert_eq!('$', self.peek_char(0));
        self.advance(1);
        self.scan_name_impl();
        TokenKind::Variable
    }

    fn scan_with_underscores(&mut self, accepted_char: impl Fn(char) -> bool) {
        let n = self.source.length();
        let peek_def = |i| if i < n { self.peek(i) } else { INVALID };
        let mut i = self.offset();
        while i < n {
            let ch = self.peek(i);
            if accepted_char(ch) {
                i += 1
            } else if ch == '_' && accepted_char(peek_def(i + 1)) {
                i += 2;
            } else {
                break;
            }
        }
        self.with_offset(i);
    }

    fn scan_decimal_digits_with_underscores(&mut self) {
        self.scan_with_underscores(Self::is_decimal_digit);
    }

    fn scan_octal_digits_with_underscores(&mut self) {
        self.scan_with_underscores(Self::is_octal_digit)
    }

    fn scan_binary_digits_with_underscores(&mut self) {
        self.scan_with_underscores(Self::is_binary_digit)
    }

    fn scan_hexadecimal_digits(&mut self) {
        self.skip_while(Self::is_hexadecimal_digit)
    }

    fn scan_hexadecimal_digits_with_underscores(&mut self) {
        self.scan_with_underscores(Self::is_hexadecimal_digit)
    }

    fn scan_hex_literal(&mut self) -> TokenKind {
        let ch = self.peek_char(0);
        if !Self::is_hexadecimal_digit(ch) {
            self.with_error(Errors::error0001);
            TokenKind::HexadecimalLiteral
        } else {
            self.scan_hexadecimal_digits_with_underscores();
            TokenKind::HexadecimalLiteral
        }
    }

    fn scan_binary_literal(&mut self) -> TokenKind {
        let ch = self.peek_char(0);
        if !Self::is_binary_digit(ch) {
            self.with_error(Errors::error0002);
            TokenKind::BinaryLiteral
        } else {
            self.scan_binary_digits_with_underscores();
            TokenKind::BinaryLiteral
        }
    }

    fn scan_exponent_with_underscores(&mut self) -> TokenKind {
        let ch = self.peek_char(1);
        if ch == '+' || ch == '-' {
            self.advance(2)
        } else {
            self.advance(1)
        }
        let ch = self.peek_char(0);
        if !Self::is_decimal_digit(ch) {
            self.with_error(Errors::error0003);
            TokenKind::FloatingLiteral
        } else {
            self.scan_decimal_digits_with_underscores();
            TokenKind::FloatingLiteral
        }
    }

    fn scan_after_decimal_point_with_underscores(&mut self) -> TokenKind {
        self.advance(1);
        let ch = self.peek_char(0);
        if ch == '_' {
            TokenKind::FloatingLiteral
        } else {
            self.scan_decimal_digits_with_underscores();
            let ch = self.peek_char(0);
            if ch == 'e' || ch == 'E' {
                self.scan_exponent_with_underscores()
            } else {
                TokenKind::FloatingLiteral
            }
        }
    }

    fn scan_octal_or_float(&mut self) -> TokenKind {
        // We've scanned a leading zero.
        // We have an irritating ambiguity here.  09 is not a legal octal or
        // floating literal, but 09e1 and 09.1 are.
        self.advance(1);
        let ch = self.peek_char(0);
        match ch {
            '.' =>
            // 0.
            {
                self.scan_after_decimal_point_with_underscores()
            }
            'e' | 'E' =>
            // 0e
            {
                self.scan_exponent_with_underscores()
            }
            _ if ('0'..='9').contains(&ch) => {
                // 05
                let mut lexer_oct = self.clone();
                lexer_oct.scan_octal_digits_with_underscores();

                let mut lexer_dec = self.clone();
                lexer_dec.scan_decimal_digits_with_underscores();
                if (lexer_oct.width()) == (lexer_dec.width()) {
                    // Only octal digits. Could be an octal literal, or could
                    // be a float.
                    let ch = lexer_oct.peek_char(0);
                    if ch == 'e' || ch == 'E' {
                        self.continue_from(lexer_oct);
                        self.scan_exponent_with_underscores()
                    } else if ch == '.' {
                        self.continue_from(lexer_oct);
                        self.scan_after_decimal_point_with_underscores()
                    } else {
                        self.continue_from(lexer_oct);
                        TokenKind::OctalLiteral
                    }
                } else {
                    // We had decimal digits following a leading zero; this is either a
                    // float literal or an octal to be truncated at the first non-octal
                    // digit.
                    let ch = lexer_dec.peek_char(0);
                    if ch == 'e' || ch == 'E' {
                        self.continue_from(lexer_dec);
                        self.scan_exponent_with_underscores()
                    } else if ch == '.' {
                        self.continue_from(lexer_dec);
                        self.scan_after_decimal_point_with_underscores()
                    } else {
                        // an octal to be truncated at the first non-octal digit
                        self.scan_decimal_digits_with_underscores();
                        TokenKind::OctalLiteral
                    }
                }
            }
            _ =>
            // 0 is a decimal literal
            {
                TokenKind::DecimalLiteral
            }
        }
    }

    fn scan_decimal_or_float(&mut self) -> TokenKind {
        // We've scanned a leading non-zero digit.
        self.scan_decimal_digits_with_underscores();
        let ch = self.peek_char(0);
        match ch {
            '.' =>
            // 123.
            {
                self.scan_after_decimal_point_with_underscores()
            }
            'e' | 'E' =>
            // 123e
            {
                self.scan_exponent_with_underscores()
            }
            _ =>
            // 123
            {
                TokenKind::DecimalLiteral
            }
        }
    }

    fn scan_single_quote_string_literal(&mut self) -> TokenKind {
        // TODO: What about newlines embedded?
        // SPEC:
        // single-quoted-string-literal::
        //   b-opt  ' sq-char-sequence-opt  '
        //
        // TODO: What is this b-opt?  We don't lex an optional 'b' before a literal.
        //
        // sq-char-sequence::
        //   sq-char
        //   sq-char-sequence   sq-char
        //
        // sq-char::
        //   sq-escape-sequence
        //   \opt   any character except single-quote (') or backslash (\)
        //
        // sq-escape-sequence:: one of
        //   \'  \\
        let n = self.source.length();
        let peek = |x| self.source.get(x);

        let mut has_error0012 = false;
        let mut has_error0006 = false;

        let mut i = 1 + self.offset();
        let new_offset = loop {
            if i >= n {
                has_error0012 = true;
                break n;
            } else {
                let ch = peek(i);
                match ch {
                    INVALID => {
                        has_error0006 = true;
                        i += 1
                    }
                    '\\' => i += 2,
                    '\'' => break (1 + i),
                    _ => i += 1,
                }
            }
        };

        if has_error0006 {
            self.with_error(Errors::error0006)
        }
        if has_error0012 {
            self.with_error(Errors::error0012)
        }

        self.with_offset(new_offset);
        TokenKind::SingleQuotedStringLiteral
    }

    fn scan_hexadecimal_escape(&mut self) {
        let ch2 = self.peek_char(2);
        let ch3 = self.peek_char(3);
        if !(Self::is_hexadecimal_digit(ch2)) {
            // TODO: Consider producing an error for a malformed hex escape
            // let lexer = with_error lexer SyntaxError.error0005 in
            self.advance(2);
        } else if !(Self::is_hexadecimal_digit(ch3)) {
            // let lexer = with_error lexer SyntaxError.error0005 in
            self.advance(3)
        } else {
            self.advance(4)
        }
    }

    fn scan_unicode_escape(&mut self) {
        // At present the lexer is pointing at \u
        if self.peek_char(2) == '{' {
            if self.peek_char(3) == '$' {
                // We have a malformed unicode escape that contains a possible embedded
                // expression. Eat the \u and keep on processing the embedded expression.
                // TODO: Consider producing a warning for a malformed unicode escape.
                self.advance(2)
            } else {
                // We have a possibly well-formed escape sequence, and at least we know
                // that it is not an embedded expression.
                // TODO: Consider producing an error if the digits are out of range
                // of legal Unicode characters.
                // TODO: Consider producing an error if there are no digits.
                // Skip over the slash, u and brace, and start lexing the number.
                self.advance(3);
                self.scan_hexadecimal_digits();
                let ch = self.peek_char(0);
                if ch != '}' {
                    // TODO: Consider producing a warning for a malformed unicode escape.
                    {}
                } else {
                    self.advance(1)
                }
            }
        } else {
            // We have a malformed unicode escape sequence. Bail out.
            // TODO: Consider producing a warning for a malformed unicode escape.
            self.advance(2)
        }
    }

    fn skip_uninteresting_double_quote_like_string_characters(&mut self) {
        let is_uninteresting = |ch| match ch {
            INVALID | '\\' | '$' | '{' | '[' | ']' | '-' => false,
            ch if ('0'..='9').contains(&ch) => false,
            ch => ch != '"' && !Self::is_name_nondigit(ch),
        };
        self.skip_while(is_uninteresting);
    }

    fn scan_integer_literal_in_string(&mut self) -> TokenKind {
        if self.peek_char(0) == '0' {
            match self.peek_char(1) {
                'x' | 'X' => {
                    self.advance(2);
                    self.scan_hex_literal()
                }
                'b' | 'B' => {
                    self.advance(2);
                    self.scan_binary_literal()
                }
                _ => {
                    // An integer literal starting with 0 in a string will actually
                    // always be treated as a string index in HHVM, and not as an octal.
                    // In such a case, HHVM actually scans all decimal digits to create the
                    // token. TODO: (kasper) T40381519 we may want to change this behavior to something more
                    // sensible
                    self.scan_decimal_digits_with_underscores();
                    TokenKind::DecimalLiteral
                }
            }
        } else {
            self.scan_decimal_digits_with_underscores();
            TokenKind::DecimalLiteral
        }
    }

    fn scan_double_quote_like_string_literal_from_start(&mut self) -> TokenKind {
        let literal_token_kind = TokenKind::DoubleQuotedStringLiteral;
        let head_token_kind = TokenKind::DoubleQuotedStringLiteralHead;
        self.advance(1);
        loop {
            // If there's nothing interesting in this double-quoted string then
            // we can just hand it back as-is.
            self.skip_uninteresting_double_quote_like_string_characters();
            match self.peek_char(0) {
                INVALID => {
                    // If the string is unterminated then give an error; if this is an
                    // embedded zero character then give an error and recurse; we might
                    // be able to make more progress.
                    if self.at_end() {
                        self.with_error(Errors::error0012);
                        break literal_token_kind;
                    } else {
                        self.with_error(Errors::error0006);
                        self.advance(1)
                    }
                }
                '"' => {
                    // We made it to the end without finding a special character.
                    self.advance(1);
                    break literal_token_kind;
                }
                _ =>
                // We've found a backslash, dollar or brace.
                {
                    break head_token_kind;
                }
            }
        }
    }

    fn is_heredoc_tail(&self, name: &[u8]) -> bool {
        // A heredoc tail is the identifier immediately preceded by a newline
        // and immediately followed by an optional semi and then a newline.
        //
        // Note that the newline and optional semi are not part of the literal;
        // the literal's lexeme ends at the end of the name. Either there is
        // no trivia and the next token is a semi-with-trailing-newline, or
        // the trailing trivia is a newline.
        //
        // This odd rule is to ensure that both
        // $x = <<<HERE
        // something
        // HERE;
        //
        // and
        //
        // $x = <<<HERE
        // something
        // HERE
        // . "something else";
        //
        // are legal.
        if !(Self::is_newline(self.peek_back(1))) {
            false
        } else {
            let len = name.len();
            let ch0 = self.peek_char(len);
            let ch1 = self.peek_char(len + 1);
            ((Self::is_newline(ch0)) || ch0 == ';' && (Self::is_newline(ch1)))
                && self.peek_string(len) == name
        }
    }

    fn get_tail_token_kind(&self, literal_kind: &StringLiteralKind) -> TokenKind {
        match literal_kind {
            StringLiteralKind::LiteralHeredoc { .. } => TokenKind::HeredocStringLiteralTail,
            StringLiteralKind::LiteralDoubleQuoted => TokenKind::DoubleQuotedStringLiteralTail,
        }
    }

    fn get_string_literal_body_or_double_quoted_tail(
        &self,
        literal_kind: &StringLiteralKind,
    ) -> TokenKind {
        if literal_kind == &StringLiteralKind::LiteralDoubleQuoted {
            TokenKind::DoubleQuotedStringLiteralTail
        } else {
            TokenKind::StringLiteralBody
        }
    }

    fn scan_string_literal_in_progress(&mut self, literal_kind: &StringLiteralKind) -> TokenKind {
        let (is_heredoc, name): (bool, &[u8]) = match literal_kind {
            StringLiteralKind::LiteralHeredoc { heredoc } => (true, heredoc),
            _ => (false, b""),
        };
        let ch0 = self.peek_char(0);
        if Self::is_name_nondigit(ch0) {
            if is_heredoc && (self.is_heredoc_tail(name)) {
                self.scan_name_impl();
                TokenKind::HeredocStringLiteralTail
            } else {
                self.scan_name_impl();
                TokenKind::Name
            }
        } else {
            match ch0 {
                INVALID => {
                    if self.at_end() {
                        self.with_error(Errors::error0012);
                        self.get_tail_token_kind(literal_kind)
                    } else {
                        self.with_error(Errors::error0006);
                        self.advance(1);
                        self.skip_uninteresting_double_quote_like_string_characters();
                        TokenKind::StringLiteralBody
                    }
                }
                '"' => {
                    let kind = self.get_string_literal_body_or_double_quoted_tail(literal_kind);
                    self.advance(1);
                    kind
                }
                '$' => {
                    if Self::is_name_nondigit(self.peek_char(1)) {
                        self.scan_variable()
                    } else {
                        self.advance(1);
                        TokenKind::Dollar
                    }
                }
                '{' => {
                    self.advance(1);
                    TokenKind::LeftBrace
                }
                '\\' => {
                    match self.peek_char(1) {
                        // In these cases we just skip the escape sequence and
                        // keep on scanning for special characters.
                        | '\\' | '"' | '$' | 'e' | 'f' | 'n' | 'r' | 't' | 'v' | '`'
                        // Same in these cases; there might be more octal characters following but
                        // if there are, we'll just eat them as normal characters.
                        | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' => {
                            self.advance(2);
                            self.skip_uninteresting_double_quote_like_string_characters();
                            TokenKind::StringLiteralBody}
                        | 'x' => {
                            self.scan_hexadecimal_escape();
                            self.skip_uninteresting_double_quote_like_string_characters();
                            TokenKind::StringLiteralBody }
                        | 'u' => {
                            self.scan_unicode_escape();
                            self.skip_uninteresting_double_quote_like_string_characters();
                            TokenKind::StringLiteralBody }
                        | '{' => {
                            // The rules for escaping open braces in Hack are bizarre. Suppose we
                            // have
                            // $x = 123;
                            // $y = 456;
                            // $z = "\{$x,$y\}";
                            // What is the value of $z?  Naively you would think that the backslash
                            // escapes the braces, and the variables are embedded, so {123,456}. But
                            // that's not what happens. Yes, the backslash makes the brace no longer
                            // the opening brace of an expression. But the backslash is still part
                            // of the string!  This is the string \{123,456\}.
                            // TODO: We might want to fix this because this is very strange.
                            // Eat the backslash and the brace.
                            self.advance(2);
                            TokenKind::StringLiteralBody
                        }
                    | _ => {
                       // TODO: A backslash followed by something other than an escape sequence
                       // is legal in hack, and treated as though it was just the backslash
                       // and the character. However we might consider making this a warning.
                       // It is particularly egregious when we have something like:
                       // $x = "abcdef \
                       //       ghi";
                       // The author of the code likely means the backslash to mean line
                       // continuation but in fact it just means to put a backslash and newline
                       // in the string.
                          self.advance(1);
                          self.skip_uninteresting_double_quote_like_string_characters();
                          TokenKind::StringLiteralBody
                      }
                   }
                }
                '[' => {
                    self.advance(1);
                    TokenKind::LeftBracket
                }
                ']' => {
                    self.advance(1);
                    TokenKind::RightBracket
                }
                '-' => {
                    if (self.peek_char(1)) == '>' {
                        self.advance(2);
                        TokenKind::MinusGreaterThan
                    } else {
                        // Nothing interesting here. Skip it and find the next
                        // interesting character.
                        self.advance(1);
                        self.skip_uninteresting_double_quote_like_string_characters();
                        TokenKind::StringLiteralBody
                    }
                }
                ch if ('0'..='9').contains(&ch) => {
                    let mut lexer1 = self.clone();
                    let literal = lexer1.scan_integer_literal_in_string();

                    if self.errors.len() == lexer1.errors.len() {
                        self.continue_from(lexer1);
                        literal
                    } else {
                        // If we failed to scan a literal, do not interpret the literal
                        self.with_offset(lexer1.offset());
                        TokenKind::StringLiteralBody
                    }
                }
                _ => {
                    // Nothing interesting here. Skip it and find the next
                    // interesting character.
                    self.advance(1);
                    self.skip_uninteresting_double_quote_like_string_characters();
                    TokenKind::StringLiteralBody
                }
            }
        }
    }
    // A heredoc string literal has the form
    //
    // header
    // optional body
    // trailer
    //
    // The header is:
    //
    // <<< (optional whitespace) name (no whitespace) (newline)
    //
    // The optional body is:
    //
    // any characters whatsoever including newlines (newline)
    //
    // The trailer is:
    //
    // (no whitespace) name (no whitespace) (optional semi) (no whitespace) (newline)
    //
    // The names must be identical.  The trailing semi and newline must be present.
    //
    // The body is any and all characters, up to the first line that exactly matches
    // the trailer.
    //
    // The body may contain embedded expressions.
    //
    // A nowdoc string literal has the same form except that the first name is
    // enclosed in single quotes, and it may not contain embedded expressions.
    fn scan_docstring_name_actual(&mut self) -> &'a [u8] {
        let ch = self.peek_char(0);
        if Self::is_name_nondigit(ch) {
            let start_offset = self.offset();
            self.advance(1);
            self.skip_name_end();
            self.source.sub(start_offset, self.offset() - start_offset)
        } else {
            self.with_error(Errors::error0008);
            b""
        }
    }

    fn scan_docstring_name(&mut self) -> (&'a [u8], TokenKind) {
        self.skip_whitespace();
        let ch = self.peek_char(0);
        let kind = if ch == '\'' {
            TokenKind::NowdocStringLiteral
        } else {
            TokenKind::HeredocStringLiteral
        };

        let name = if ch == '\'' {
            self.advance(1);
            let name = self.scan_docstring_name_actual();
            if (self.peek_char(0)) == '\'' {
                self.advance(1);
                name
            } else {
                self.with_error(Errors::error0010);
                name
            }
        } else {
            // Starting with PHP 5.3.0, the opening Heredoc identifier
            // may optionally be enclosed in double quotes:
            if ch == '"' {
                self.advance(1)
            };
            let name = self.scan_docstring_name_actual();
            if ch == '"' {
                // same logic as above, just for double quote
                if self.peek_char(0) == '\"' {
                    self.advance(1);
                } else {
                    self.with_error(Errors::missing_double_quote)
                }
            }
            name
        };
        (name, kind)
    }

    fn scan_docstring_header(&mut self) -> (&'a [u8], TokenKind) {
        let ch = self.peek_char(0);
        // Skip 3 for <<< or 4 for b<<<
        let skip_count = if ch == 'b' { 4 } else { 3 };
        self.advance(skip_count);
        let (name, kind) = self.scan_docstring_name();
        let ch = self.peek_char(0);
        if !Self::is_newline(ch) {
            self.with_error(Errors::error0011)
        }
        self.skip_to_end_of_line();
        self.skip_end_of_line();
        (name, kind)
    }

    fn scan_docstring_remainder(&mut self, name: &[u8]) {
        let len = name.len();
        loop {
            let ch0 = self.peek_char(len);
            let ch1 = self.peek_char(len + 1);
            if ((Self::is_newline(ch0)) || ch0 == ';' && (Self::is_newline(ch1)))
                && self.peek_string(len) == name
            {
                self.advance(len);
                break;
            } else {
                self.skip_to_end_of_line();
                let ch = self.peek_char(0);
                if Self::is_newline(ch) {
                    self.skip_end_of_line()
                } else {
                    // If we got here then we ran off the end of the file without
                    // finding a newline. Just bail.
                    self.with_error(Errors::error0011);
                    break;
                }
            }
        }
    }

    fn scan_docstring_literal(&mut self) -> TokenKind {
        let (name, kind) = self.scan_docstring_header();
        self.scan_docstring_remainder(name);
        kind
    }

    fn scan_xhp_label(&mut self) {
        self.advance(1);
        self.skip_name_end();
    }

    fn scan_xhp_element_name(&mut self, attribute: bool) -> TokenKind {
        // An XHP element name is a sequence of one or more XHP labels each separated
        // by a single : or -.  Note that it is possible for an XHP element name to be
        // followed immediately by a : or - that is the next token, so if we find
        // a : or - not followed by a label, we need to terminate the token.
        self.scan_xhp_label();
        let ch0 = self.peek_char(0);
        let ch1 = self.peek_char(1);
        if (!attribute && ch0 == ':' || ch0 == '-') && Self::is_name_nondigit(ch1) {
            self.advance(1);
            self.scan_xhp_element_name(false)
        } else {
            TokenKind::XHPElementName
        }
    }

    fn scan_xhp_class_no_dash(&mut self) -> TokenKind {
        self.scan_xhp_label();
        let ch0 = self.peek_char(0);
        let ch1 = self.peek_char(1);
        if ch0 == ':' && Self::is_name_nondigit(ch1) {
            self.advance(1);
            self.scan_xhp_class_no_dash()
        } else {
            TokenKind::XHPElementName
        }
    }

    // Is the next token we're going to lex a possible xhp class name?
    fn is_xhp_class_name(&self) -> bool {
        (self.peek_char(0) == ':') && (Self::is_name_nondigit(self.peek_char(1)))
    }

    fn scan_xhp_class_name(&mut self) -> TokenKind {
        // An XHP class name is a colon followed by an xhp name.
        if self.is_xhp_class_name() {
            self.advance(1);
            self.scan_xhp_element_name(false);
            TokenKind::XHPClassName
        } else {
            self.with_error(Errors::error0008);
            self.advance(1);
            TokenKind::ErrorToken
        }
    }

    // To support xhp class style class definitions we don't require a : prefix
    fn scan_xhp_modifier_class_name(&mut self) -> TokenKind {
        // we don't want to allow xhp names with a : prefix here
        if self.peek_char(0) == ':' {
            self.with_error(Errors::error0008);
            TokenKind::ErrorToken
        } else {
            self.scan_xhp_class_no_dash();
            TokenKind::XHPClassName
        }
    }

    fn scan_xhp_string_literal(&mut self) -> TokenKind {
        // XHP string literals are just straight up "find the closing quote"
        // strings.  Embedded newlines are legal.
        let mut offset: usize = 1;
        loop {
            match self.peek_char(offset) {
                INVALID => {
                    self.advance(offset);
                    if self.at_end() {
                        self.with_error(Errors::error0012);
                        return TokenKind::XHPStringLiteral;
                    } else {
                        self.with_error(Errors::error0006);
                        offset = 1
                    }
                }
                '"' => {
                    self.advance(offset + 1);
                    return TokenKind::XHPStringLiteral;
                }
                _ => offset += 1,
            }
        }
    }

    // Note that this does not scan an XHP body
    fn scan_xhp_token(&mut self) -> TokenKind {
        // TODO: HHVM requires that there be no trivia between < and name in an
        // opening tag, but does allow trivia between </ and name in a closing tag.
        // Consider allowing trivia in an opening tag.
        let ch0 = self.peek_char(0);
        if ch0 == INVALID && self.at_end() {
            TokenKind::EndOfFile
        } else if self.is_xhp_class_name() || Self::is_name_nondigit(ch0) {
            self.scan_xhp_element_name(false)
        } else {
            match ch0 {
                '{' => {
                    self.advance(1);
                    TokenKind::LeftBrace
                }
                '}' => {
                    self.advance(1);
                    TokenKind::RightBrace
                }
                '=' => {
                    self.advance(1);
                    TokenKind::Equal
                }
                '<' => {
                    if (self.peek_char(1)) == '/' {
                        self.advance(2);
                        TokenKind::LessThanSlash
                    } else {
                        self.advance(1);
                        TokenKind::LessThan
                    }
                }
                '"' => self.scan_xhp_string_literal(),
                '/' => {
                    if (self.peek_char(1)) == '>' {
                        self.advance(2);
                        TokenKind::SlashGreaterThan
                    } else {
                        self.with_error(Errors::error0006);
                        self.advance(1);
                        TokenKind::ErrorToken
                    }
                }
                '>' => {
                    self.advance(1);
                    TokenKind::GreaterThan
                }
                _ => {
                    self.with_error(Errors::error0006);
                    self.advance(1);
                    TokenKind::ErrorToken
                }
            }
        }
    }

    fn scan_xhp_comment(&mut self) {
        let mut offset = 4;
        loop {
            let ch0 = self.peek_char(offset);
            let ch1 = self.peek_char(offset + 1);
            let ch2 = self.peek_char(offset + 2);
            match (ch0, ch1, ch2) {
                (INVALID, _, _) => {
                    self.advance(offset);
                    return self.with_error(Errors::error0014);
                }
                ('-', '-', '>') => return self.advance(offset + 3),
                _ => offset += 1,
            }
        }
    }
    fn scan_xhp_body(&mut self) -> TokenKind {
        // Naively you might think that an XHP body is just a bunch of characters,
        // terminated by an embedded { } expression or a tag.  However, whitespace
        // and newlines are relevant in XHP bodies because they are "soft".
        // That is, any section of contiguous trivia has the same semantics as a
        // single space or newline -- just as in HTML.
        //
        // Obviously this is of relevance to code formatters.
        //
        // Therefore we detect whitespace and newlines within XHP bodies and treat
        // it as trivia surrounding the tokens within the body.
        //
        // TODO: Is this also true of whitespace within XHP comments? If so then
        // we need to make XHP comments a sequence of tokens, rather than a
        // single token as they are now.
        let ch0 = self.peek_char(0);

        match ch0 {
            INVALID if self.at_end() => TokenKind::EndOfFile,
            '{' => {
                self.advance(1);
                TokenKind::LeftBrace
            }
            '}' => {
                self.advance(1);
                TokenKind::RightBrace
            }
            '<' => {
                let ch1 = self.peek_char(1);
                let ch2 = self.peek_char(2);
                let ch3 = self.peek_char(3);
                match (ch1, ch2, ch3) {
                    ('!', '-', '-') => {
                        self.scan_xhp_comment();
                        TokenKind::XHPComment
                    }
                    ('/', _, _) => {
                        self.advance(2);
                        TokenKind::LessThanSlash
                    }
                    _ => {
                        self.advance(1);
                        TokenKind::LessThan
                    }
                }
            }
            _ => {
                let mut offset = 0;
                loop {
                    let ch = self.peek_char(offset);
                    match ch {
                        INVALID => {
                            self.advance(offset);
                            if self.at_end() {
                                self.with_error(Errors::error0013);
                                break;
                            } else {
                                self.with_error(Errors::error0006);
                                offset = 1
                            }
                        }
                        '\t' | ' ' | '\r' | '\n' | '{' | '}' | '<' => {
                            self.advance(offset);
                            break;
                        }
                        _ => offset += 1,
                    }
                }
                TokenKind::XHPBody
            }
        }
    }

    fn scan_dollar_token(&mut self) -> TokenKind {
        // We have a problem here.  We wish to be able to lexically analyze both
        // PHP and Hack, but the introduction of $$ to Hack makes them incompatible.
        // "$$x" and "$$ $x" are legal in PHP, but illegal in Hack.
        // The rule in PHP seems to be that $ is a prefix operator, it is a token,
        // it can be followed by trivia, but the next token has to be another $
        // operator, a variable $x, or a {.
        //
        // Here's a reasonable compromise.  (TODO: Review this decision.)
        //
        // $$x lexes as $ $x
        // $$$x lexes as $ $ $x
        // and so on.
        //
        // $$ followed by anything other than a name or a $ lexes as $$.
        //
        // This means that lexing a PHP program which contains "$$ $x" is different
        // will fail at parse time, but I'm willing to live with that.
        //
        // This means that lexing a Hack program which contains
        // "$x |> $$instanceof Foo" produces an error as well.
        //
        // If these decisions are unacceptable then we will need to make the lexer
        // be aware of whether it is lexing PHP or Hack; thus far we have not had
        // to make this distinction.

        // We are already at $.
        let ch1 = self.peek_char(1);
        match ch1 {
            '$' => {
                let ch2 = self.peek_char(2);
                if ch2 == '$' || ch2 == '{' || Self::is_name_nondigit(ch2) {
                    self.advance(1);
                    TokenKind::Dollar // $$x or $$$
                } else {
                    self.advance(2);
                    TokenKind::DollarDollar // $$
                }
            }
            _ => {
                if Self::is_name_nondigit(ch1) {
                    self.scan_variable() // $x
                } else {
                    self.advance(1);
                    TokenKind::Dollar // $
                }
            }
        }
    }

    fn scan_token(&mut self, in_type: bool) -> TokenKind {
        let ch0 = self.peek_char(0);
        match ch0 {
            '[' => {
                self.advance(1);
                TokenKind::LeftBracket
            }
            ']' => {
                self.advance(1);
                TokenKind::RightBracket
            }
            '(' => {
                self.advance(1);
                TokenKind::LeftParen
            }
            ')' => {
                self.advance(1);
                TokenKind::RightParen
            }
            '{' => {
                self.advance(1);
                TokenKind::LeftBrace
            }
            '}' => {
                self.advance(1);
                TokenKind::RightBrace
            }
            '.' => match self.peek_char(1) {
                '=' => {
                    self.advance(2);
                    TokenKind::DotEqual
                }
                ch if ('0'..='9').contains(&ch) => self.scan_after_decimal_point_with_underscores(),
                '.' => {
                    if (self.peek_char(2)) == '.' {
                        self.advance(3);
                        TokenKind::DotDotDot
                    } else {
                        self.advance(1);
                        TokenKind::Dot
                    }
                }
                _ => {
                    self.advance(1);
                    TokenKind::Dot
                }
            },
            '-' => match self.peek_char(1) {
                '=' => {
                    self.advance(2);
                    TokenKind::MinusEqual
                }
                '-' => {
                    self.advance(2);
                    TokenKind::MinusMinus
                }
                '>' => {
                    self.advance(2);
                    TokenKind::MinusGreaterThan
                }
                _ => {
                    self.advance(1);
                    TokenKind::Minus
                }
            },
            '+' => match self.peek_char(1) {
                '=' => {
                    self.advance(2);
                    TokenKind::PlusEqual
                }
                '+' => {
                    self.advance(2);
                    TokenKind::PlusPlus
                }
                _ => {
                    self.advance(1);
                    TokenKind::Plus
                }
            },
            '*' => match (self.peek_char(1), self.peek_char(2)) {
                ('=', _) => {
                    self.advance(2);
                    TokenKind::StarEqual
                }
                ('*', '=') => {
                    self.advance(3);
                    TokenKind::StarStarEqual
                }
                ('*', _) => {
                    self.advance(2);
                    TokenKind::StarStar
                }
                _ => {
                    self.advance(1);
                    TokenKind::Star
                }
            },
            '~' => {
                self.advance(1);
                TokenKind::Tilde
            }
            '!' => match (self.peek_char(1), self.peek_char(2)) {
                ('=', '=') => {
                    self.advance(3);
                    TokenKind::ExclamationEqualEqual
                }
                ('=', _) => {
                    self.advance(2);
                    TokenKind::ExclamationEqual
                }
                _ => {
                    self.advance(1);
                    TokenKind::Exclamation
                }
            },
            '$' => self.scan_dollar_token(),
            '/' => {
                if (self.peek_char(1)) == '=' {
                    self.advance(2);
                    TokenKind::SlashEqual
                } else {
                    self.advance(1);
                    TokenKind::Slash
                }
            }
            '%' => {
                if (self.peek_char(1)) == '=' {
                    self.advance(2);
                    TokenKind::PercentEqual
                } else {
                    self.advance(1);
                    TokenKind::Percent
                }
            }
            '<' => {
                match (self.peek_char(1), self.peek_char(2)) {
                    ('<', '<') => self.scan_docstring_literal(),
                    ('<', '=') => {
                        self.advance(3);
                        TokenKind::LessThanLessThanEqual
                    }
                    // TODO: We lex and parse the spaceship operator.
                    // TODO: This is not in the spec at present.  We should either make it an
                    // TODO: error, or add it to the specification.
                    ('=', '>') => {
                        self.advance(3);
                        TokenKind::LessThanEqualGreaterThan
                    }
                    ('=', _) => {
                        self.advance(2);
                        TokenKind::LessThanEqual
                    }
                    ('<', _) => {
                        self.advance(2);
                        TokenKind::LessThanLessThan
                    }
                    _ => {
                        self.advance(1);
                        TokenKind::LessThan
                    }
                }
            }
            '>' => {
                match (self.peek_char(1), self.peek_char(2)) {
                    // If we are parsing a generic type argument list then we might be at the >>
                    // in `List<List<int>>``, or at the >= of `let x:vec<int>=...`. In that case
                    // we want to lex two >'s instead of >> / one > and one = instead of >=.
                    (ch, _) if (ch == '>' || ch == '=') && in_type => {
                        self.advance(1);
                        TokenKind::GreaterThan
                    }
                    ('>', '=') => {
                        self.advance(3);
                        TokenKind::GreaterThanGreaterThanEqual
                    }
                    ('>', _) => {
                        self.advance(2);
                        TokenKind::GreaterThanGreaterThan
                    }
                    ('=', _) => {
                        self.advance(2);
                        TokenKind::GreaterThanEqual
                    }
                    _ => {
                        self.advance(1);
                        TokenKind::GreaterThan
                    }
                }
            }
            '=' => match (self.peek_char(1), self.peek_char(2)) {
                ('=', '=') => {
                    self.advance(3);
                    TokenKind::EqualEqualEqual
                }
                ('=', '>') => {
                    self.advance(3);
                    TokenKind::EqualEqualGreaterThan
                }
                ('=', _) => {
                    self.advance(2);
                    TokenKind::EqualEqual
                }
                ('>', _) => {
                    self.advance(2);
                    TokenKind::EqualGreaterThan
                }
                _ => {
                    self.advance(1);
                    TokenKind::Equal
                }
            },
            '^' => {
                if (self.peek_char(1)) == '=' {
                    self.advance(2);
                    TokenKind::CaratEqual
                } else {
                    self.advance(1);
                    TokenKind::Carat
                }
            }
            '|' => match self.peek_char(1) {
                '=' => {
                    self.advance(2);
                    TokenKind::BarEqual
                }
                '>' => {
                    self.advance(2);
                    TokenKind::BarGreaterThan
                }
                '|' => {
                    self.advance(2);
                    TokenKind::BarBar
                }
                _ => {
                    self.advance(1);
                    TokenKind::Bar
                }
            },
            '&' => match self.peek_char(1) {
                '=' => {
                    self.advance(2);
                    TokenKind::AmpersandEqual
                }
                '&' => {
                    self.advance(2);
                    TokenKind::AmpersandAmpersand
                }
                _ => {
                    self.advance(1);
                    TokenKind::Ampersand
                }
            },
            '?' => match (self.peek_char(1), self.peek_char(2)) {
                (':', _) if !in_type => {
                    self.advance(2);
                    TokenKind::QuestionColon
                }
                ('-', '>') => {
                    self.advance(3);
                    TokenKind::QuestionMinusGreaterThan
                }
                ('?', '=') => {
                    self.advance(3);
                    TokenKind::QuestionQuestionEqual
                }
                ('?', _) => {
                    self.advance(2);
                    TokenKind::QuestionQuestion
                }
                ('a', 's') if !Self::is_name_nondigit(self.peek_char(3)) => {
                    self.advance(3);
                    TokenKind::QuestionAs
                }
                _ => {
                    self.advance(1);
                    TokenKind::Question
                }
            },
            ':' => {
                let ch1 = self.peek_char(1);

                if ch1 == ':' {
                    self.advance(2);
                    TokenKind::ColonColon
                } else {
                    self.advance(1);
                    TokenKind::Colon
                }
            }
            ';' => {
                self.advance(1);
                TokenKind::Semicolon
            }
            ',' => {
                self.advance(1);
                TokenKind::Comma
            }
            '@' => {
                self.advance(1);
                TokenKind::At
            }
            '0' => match self.peek_char(1) {
                'x' | 'X' => {
                    self.advance(2);
                    self.scan_hex_literal()
                }
                'b' | 'B' => {
                    self.advance(2);
                    self.scan_binary_literal()
                }
                _ => self.scan_octal_or_float(),
            },
            ch if ('1'..='9').contains(&ch) => self.scan_decimal_or_float(),
            '\'' => self.scan_single_quote_string_literal(),
            '"' => self.scan_double_quote_like_string_literal_from_start(),
            '`' => {
                self.advance(1);
                TokenKind::Backtick
            }
            '\\' => {
                self.advance(1);
                TokenKind::Backslash
            }
            '#' => {
                self.advance(1);
                TokenKind::Hash
            }
            'b' if {
                let c1 = self.peek_char(1);
                let c2 = self.peek_char(2);
                let c3 = self.peek_char(3);
                c1 == '"' || c1 == '\'' || (c1 == '<' && c2 == '<' && c3 == '<')
            } =>
            {
                self.advance(1);
                self.scan_token(in_type)
            }
            // Names
            _ => {
                if ch0 == INVALID && self.at_end() {
                    TokenKind::EndOfFile
                } else if Self::is_name_nondigit(ch0) {
                    self.scan_name()
                } else {
                    self.with_error(Errors::error0006);
                    self.advance(1);
                    TokenKind::ErrorToken
                }
            }
        }
    }

    fn scan_token_outside_type(&mut self) -> TokenKind {
        self.scan_token(false)
    }

    fn scan_token_inside_type(&mut self) -> TokenKind {
        self.scan_token(true)
    }

    // Lexing trivia

    // SPEC:
    //
    // white-space-character::
    //   new-line
    //   Space character (U+0020)
    //   Horizontal-tab character (U+0009)
    //
    // single-line-comment::
    //   //   input-characters-opt
    //   #    input-characters-opt
    //
    // new-line::
    //   Carriage-return character (U+000D)
    //   Line-feed character (U+000A)
    //   Carriage-return character followed by line-feed character

    fn str_scan_end_of_line(s: &[u8], i: usize) -> usize {
        match s.get(i).map(|x| *x as char) {
            None => i + 1,
            Some('\r') => match s.get(i + 1).map(|x| *x as char) {
                Some('\n') => 2 + i,
                _ => i + 1,
            },
            Some('\n') => i + 1,
            _ => panic!("str_scan_end_of_line called while not on end of line!"),
        }
    }

    fn scan_end_of_line(&mut self) -> Trivium<TF> {
        match self.peek_char(0) {
            '\r' => {
                let w = if self.peek_char(1) == '\n' { 2 } else { 1 };
                self.advance(w);
                Trivia::<TF>::make_eol(self.start, w)
            }
            '\n' => {
                self.advance(1);
                Trivia::<TF>::make_eol(self.start, 1)
            }
            _ => panic!("scan_end_of_line called while not on end of line!"),
        }
    }

    fn scan_single_line_comment(&mut self) -> Trivium<TF> {
        // A fallthrough comment is two slashes, any amount of whitespace,
        // FALLTHROUGH, and any characters may follow.
        // TODO: Consider allowing lowercase fallthrough.

        self.advance(2);
        self.skip_whitespace();
        let lexer_ws = self.clone();
        self.skip_to_end_of_line();
        let w = self.width();
        let remainder = self.offset - lexer_ws.offset;
        if remainder >= 11 && lexer_ws.peek_string(11) == b"FALLTHROUGH" {
            Trivia::<TF>::make_fallthrough(self.start, w)
        } else {
            Trivia::<TF>::make_single_line_comment(self.start, w)
        }
    }

    fn skip_to_end_of_delimited_comment(&mut self) {
        let mut offset = 0;
        loop {
            let ch0 = self.peek_char(offset);
            if ch0 == INVALID {
                self.advance(offset);
                if self.at_end() {
                    return self.with_error(Errors::error0007);
                } else {
                    // TODO: Do we want to give a warning for an embedded zero char
                    // inside a comment?
                    offset = 1;
                }
            } else if ch0 == '*' && (self.peek_char(offset + 1)) == '/' {
                return self.advance(offset + 2);
            } else {
                offset += 1
            }
        }
    }

    fn scan_delimited_comment(&mut self) -> Trivium<TF> {
        // The original lexer lexes a fixme / ignore error as:
        //
        // slash star [whitespace]* HH_FIXME [whitespace or newline]* leftbracket
        // [whitespace or newline]* integer [any text]* star slash
        //
        // Notice that the original lexer oddly enough does not verify that there
        // is a right bracket.
        //
        // For our purposes we will just check for HH_FIXME / HH_IGNORE_ERROR;
        // a later pass can try to parse out the integer if there is one,
        // give a warning if there is not, and so on.

        self.advance(2);
        self.skip_whitespace();

        let lexer_ws = self.clone();
        self.skip_to_end_of_delimited_comment();
        let w = self.width();
        if lexer_ws.match_string(b"HH_FIXME") {
            Trivia::<TF>::make_fix_me(self.start, w)
        } else if lexer_ws.match_string(b"HH_IGNORE_ERROR") {
            Trivia::<TF>::make_ignore_error(self.start, w)
        } else {
            Trivia::<TF>::make_delimited_comment(self.start, w)
        }
    }

    fn scan_php_trivium(&mut self) -> Option<Trivium<TF>> {
        match self.peek_char(0) {
            '#' => {
                self.start_new_lexeme();
                // Not trivia
                None
            }
            '/' => {
                self.start_new_lexeme();
                match self.peek_char(1) {
                    '/' => Some(self.scan_single_line_comment()),
                    '*' => Some(self.scan_delimited_comment()),
                    _ => None,
                }
            }
            ' ' | '\t' => {
                let new_end = Self::str_skip_whitespace(self.source_text_string(), self.offset);
                let new_start = self.offset;
                let new_trivia = Trivia::<TF>::make_whitespace(new_start, new_end - new_start);
                self.with_start_offset(new_start, new_end);
                Some(new_trivia)
            }
            '\r' | '\n' => {
                self.start_new_lexeme();
                Some(self.scan_end_of_line())
            }
            _ => {
                self.start_new_lexeme();
                // Not trivia
                None
            }
        }
    }

    fn scan_xhp_trivium(&mut self) -> Option<Trivium<TF>> {
        // TODO: Should XHP comments <!-- --> be their own thing, or a kind of
        // trivia associated with a token? Right now they are the former.
        let i = self.offset;
        let ch = self.peek_char(0);
        match ch {
            ' ' | '\t' => {
                let j = Self::str_skip_whitespace(self.source_text_string(), i);
                self.with_start_offset(i, j);
                Some(Trivia::<TF>::make_whitespace(i, j - i))
            }
            '\r' | '\n' => {
                let j = Self::str_scan_end_of_line(self.source_text_string(), i);
                self.with_start_offset(i, j);
                Some(Trivia::<TF>::make_eol(i, j - i))
            }
            _ =>
            // Not trivia
            {
                self.start_new_lexeme();
                None
            }
        }
    }

    // We divide trivia into "leading" and "trailing" trivia of an associated
    // token. This means that we must find a dividing line between the trailing trivia
    // following one token and the leading trivia of the following token. Plainly
    // we need only find this line while scanning trailing trivia. The heuristics
    // we use are:
    // * The first newline trivia encountered is the last trailing trivia.
    // * The newline which follows a // or # comment is not part of the comment
    //   but does terminate the trailing trivia.
    // * A pragma to turn checks off (HH_FIXME and HH_IGNORE_ERROR) is
    //   always a leading trivia.
    fn scan_leading_trivia(
        &mut self,
        scanner: impl Fn(&mut Self) -> Option<Trivium<TF>>,
    ) -> Trivia<TF> {
        let mut acc = self.token_factory.trivia_factory_mut().make();
        while let Some(t) = scanner(self) {
            acc.push(t)
        }
        acc
    }

    fn scan_leading_trivia_with_width(
        &mut self,
        scanner: impl Fn(&mut Self) -> Option<Trivium<TF>>,
        mut width: usize,
    ) -> Trivia<TF> {
        let mut acc = self.token_factory.trivia_factory_mut().make();
        let mut extra_token_error_width = 0;
        let mut extra_token_error_offset = self.offset();
        loop {
            if width == 0 {
                if extra_token_error_width > 0 {
                    acc.push(Trivia::<TF>::make_extra_token_error(
                        extra_token_error_offset,
                        extra_token_error_width,
                    ));
                }
                break acc;
            }
            if let Some(t) = scanner(self) {
                if extra_token_error_width > 0 {
                    acc.push(Trivia::<TF>::make_extra_token_error(
                        extra_token_error_offset,
                        extra_token_error_width,
                    ));
                    extra_token_error_width = 0;
                    extra_token_error_offset = self.start();
                }
                width -= t.width();
                acc.push(t);
            } else {
                self.advance(1);
                width -= 1;
                extra_token_error_width += 1;
            }
        }
    }

    pub fn scan_leading_php_trivia_with_width(
        &mut self,
        width: usize,
    ) -> <TF::Token as LexableToken>::Trivia {
        self.scan_leading_trivia_with_width(Self::scan_php_trivium, width)
    }

    pub fn scan_leading_xhp_trivia_with_width(
        &mut self,
        width: usize,
    ) -> <TF::Token as LexableToken>::Trivia {
        self.scan_leading_trivia_with_width(Self::scan_xhp_trivium, width)
    }

    pub(crate) fn scan_leading_php_trivia(&mut self) -> <TF::Token as LexableToken>::Trivia {
        self.scan_leading_trivia(Self::scan_php_trivium)
    }

    pub(crate) fn scan_leading_xhp_trivia(&mut self) -> <TF::Token as LexableToken>::Trivia {
        self.scan_leading_trivia(Self::scan_xhp_trivium)
    }

    fn scan_trailing_trivia(
        &mut self,
        scanner: impl Fn(&mut Self) -> Option<Trivium<TF>>,
    ) -> <TF::Token as LexableToken>::Trivia {
        let mut acc = self.token_factory.trivia_factory_mut().make();
        loop {
            let mut lexer1 = self.clone();
            match scanner(&mut lexer1) {
                None => {
                    self.continue_from(lexer1);
                    return acc;
                }
                Some(t) => match t.kind() {
                    TriviaKind::EndOfLine => {
                        self.continue_from(lexer1);
                        acc.push(t);
                        return acc;
                    }
                    TriviaKind::FixMe | TriviaKind::IgnoreError => {
                        return acc;
                    }
                    _ => {
                        self.continue_from(lexer1);
                        acc.push(t)
                    }
                },
            }
        }
    }

    pub fn scan_trailing_php_trivia(&mut self) -> <TF::Token as LexableToken>::Trivia {
        self.scan_trailing_trivia(Self::scan_php_trivium)
    }

    pub fn scan_trailing_xhp_trivia(&mut self) -> <TF::Token as LexableToken>::Trivia {
        self.scan_trailing_trivia(Self::scan_xhp_trivium)
    }

    pub fn is_next_name(&self) -> bool {
        let mut lexer = self.clone();
        lexer.scan_leading_php_trivia();
        Self::is_name_nondigit(lexer.peek_char(0))
    }

    pub fn is_next_xhp_class_name(&self) -> bool {
        let mut lexer = self.clone();
        lexer.scan_leading_php_trivia();
        lexer.is_xhp_class_name()
    }

    as_case_insensitive_keyword!(
        12,
        "abstract",
        "as",
        "bool",
        "boolean",
        "break",
        "case",
        "catch",
        "class",
        "clone",
        "const",
        "continue",
        "default",
        "do",
        "echo",
        "else",
        "elseif",
        "empty",
        "endif",
        "eval",
        "exports",
        "extends",
        "false",
        "final",
        "finally",
        "for",
        "foreach",
        "function",
        "global",
        "if",
        "implements",
        "imports",
        "include",
        "include_once",
        "inout",
        "instanceof",
        "insteadof",
        "int",
        "integer",
        "interface",
        "isset",
        "list",
        "namespace",
        "new",
        "null",
        "parent",
        "print",
        "private",
        "protected",
        "public",
        "require",
        "require_once",
        "return",
        "self",
        "static",
        "string",
        "switch",
        "throw",
        "trait",
        "try",
        "true",
        "unset",
        "use",
        "using",
        "var",
        "void",
        "while",
        "yield"
    );

    fn as_keyword(&mut self, only_reserved: bool, kind: TokenKind) -> TokenKind {
        if kind == TokenKind::Name {
            let original_text = self.current_text_as_str();
            let (text, has_upper) = self
                .as_case_insensitive_keyword(original_text)
                .unwrap_or((original_text, false));
            match TokenKind::from_string(text.as_bytes(), only_reserved) {
                Some(keyword) => {
                    if has_upper && text != "true" && text != "false" && text != "null" {
                        let err = Errors::uppercase_kw(original_text);
                        self.with_error(err);
                    }
                    keyword
                }
                _ => TokenKind::Name,
            }
        } else {
            kind
        }
    }

    fn scan_token_and_leading_trivia(
        &mut self,
        scanner: impl Fn(&mut Self) -> TokenKind,
        as_name: KwSet,
    ) -> (TokenKind, usize, <TF::Token as LexableToken>::Trivia) {
        // Get past the leading trivia
        let leading = self.scan_leading_php_trivia();
        // Remember where we were when we started this token
        self.start_new_lexeme();
        let kind = scanner(self);
        let kind = match as_name {
            KwSet::AllKeywords => kind,
            KwSet::NonReservedKeywords => self.as_keyword(true, kind),
            KwSet::NoKeywords => self.as_keyword(false, kind),
        };
        let w = self.width();
        (kind, w, leading)
    }

    fn scan_token_and_trivia(
        &mut self,
        scanner: &impl Fn(&mut Self) -> TokenKind,
        as_name: KwSet,
    ) -> TF::Token {
        let token_start = self.offset;

        let (kind, w, leading) = self.scan_token_and_leading_trivia(scanner, as_name);
        let trailing = match kind {
            TokenKind::DoubleQuotedStringLiteralHead => {
                self.token_factory.trivia_factory_mut().make()
            }
            _ => self.scan_trailing_php_trivia(),
        };
        self.token_factory
            .make(kind, token_start, w, leading, trailing)
    }

    fn scan_assert_progress(&mut self, tokenizer: impl Fn(&mut Self) -> TF::Token) -> TF::Token {
        let original_remaining = self.remaining();
        let token = tokenizer(self);
        let new_remaining = self.remaining();
        if new_remaining < original_remaining
            || original_remaining == 0
                && new_remaining == 0
                && (token.kind()) == TokenKind::EndOfFile
        {
            token
        } else {
            panic!(
                "failed to make progress at {} {} {} {:?}\n",
                self.offset,
                original_remaining,
                new_remaining,
                token.kind()
            )
        }
    }

    fn scan_next_token(
        &mut self,
        scanner: impl Fn(&mut Self) -> TokenKind,
        as_name: KwSet,
    ) -> TF::Token {
        let tokenizer = |x: &mut Self| x.scan_token_and_trivia(&scanner, as_name);
        self.scan_assert_progress(tokenizer)
    }

    fn scan_next_token_as_name(&mut self, scanner: impl Fn(&mut Self) -> TokenKind) -> TF::Token {
        self.scan_next_token(scanner, KwSet::AllKeywords)
    }

    fn scan_next_token_as_keyword(
        &mut self,
        scanner: impl Fn(&mut Self) -> TokenKind,
    ) -> TF::Token {
        self.scan_next_token(scanner, KwSet::NoKeywords)
    }

    fn scan_next_token_nonreserved_as_name(
        &mut self,
        scanner: impl Fn(&mut Self) -> TokenKind,
    ) -> TF::Token {
        self.scan_next_token(scanner, KwSet::NonReservedKeywords)
    }

    fn next_token_impl(&mut self) -> TF::Token {
        if self.in_type {
            self.scan_next_token_as_keyword(Self::scan_token_inside_type)
        } else {
            self.scan_next_token_as_keyword(Self::scan_token_outside_type)
        }
    }

    // Entrypoints
    pub fn peek_next_token(&self) -> TF::Token {
        {
            let cache = self.cache.borrow();
            if let Some(cache) = cache.as_ref() {
                if cache.0 == *self {
                    return cache.1.clone();
                }
            }
        }

        let mut lexer = self.clone();
        lexer.errors = vec![];
        let before = lexer.to_lexer_pre_snapshot();
        let token = lexer.next_token_impl();
        let after = lexer.into_lexer_post_snapshot();
        self.cache
            .replace(Some(LexerCache(before, token.clone(), after)));
        token
    }

    pub fn next_token(&mut self) -> TF::Token {
        {
            let mut cache = self.cache.borrow_mut();
            if let Some(ref mut cache) = cache.deref_mut() {
                if cache.0 == *self {
                    self.start = (cache.2).start;
                    self.offset = (cache.2).offset;
                    self.in_type = (cache.2).in_type;
                    if !(cache.2).errors.is_empty() {
                        self.errors.append(&mut (cache.2).errors.clone());
                    }
                    return cache.1.clone();
                }
            }
        }
        self.next_token_impl()
    }

    pub fn next_token_no_trailing(&mut self) -> TF::Token {
        let tokenizer = |x: &mut Self| {
            let token_start = x.offset;
            let (kind, w, leading) =
                x.scan_token_and_leading_trivia(Self::scan_token_outside_type, KwSet::NoKeywords);
            let trailing = x.token_factory.trivia_factory_mut().make();
            x.token_factory
                .make(kind, token_start, w, leading, trailing)
        };
        self.scan_assert_progress(tokenizer)
    }

    pub fn next_token_in_string(&mut self, literal_kind: &StringLiteralKind) -> TF::Token {
        let token_start = self.offset;
        self.start_new_lexeme();
        // We're inside a string. Do not scan leading trivia.
        let kind = self.scan_string_literal_in_progress(literal_kind);
        let w = self.width();
        // Only scan trailing trivia if we've finished the string.
        let trailing = match kind {
            TokenKind::DoubleQuotedStringLiteralTail | TokenKind::HeredocStringLiteralTail => {
                self.scan_trailing_php_trivia()
            }
            _ => self.token_factory.trivia_factory_mut().make(),
        };
        let leading = self.token_factory.trivia_factory_mut().make();
        self.token_factory
            .make(kind, token_start, w, leading, trailing)
    }

    pub fn next_docstring_header(&mut self) -> (TF::Token, &'a [u8]) {
        // We're at the beginning of a heredoc string literal. Scan leading
        // trivia but not trailing trivia.
        let token_start = self.offset;
        let leading = self.scan_leading_php_trivia();
        self.start_new_lexeme();
        let (name, _) = self.scan_docstring_header();
        let w = self.width();
        let trailing = self.token_factory.trivia_factory_mut().make();
        let token = self.token_factory.make(
            TokenKind::HeredocStringLiteralHead,
            token_start,
            w,
            leading,
            trailing,
        );
        (token, name)
    }

    pub fn next_token_as_name(&mut self) -> TF::Token {
        self.scan_next_token_as_name(Self::scan_token_outside_type)
    }

    pub fn next_token_non_reserved_as_name(&mut self) -> TF::Token {
        self.scan_next_token_nonreserved_as_name(Self::scan_token_outside_type)
    }

    pub fn next_xhp_element_token(&mut self, no_trailing: bool) -> (TF::Token, &[u8]) {
        // XHP elements have whitespace, newlines and Hack comments.
        let tokenizer = |lexer: &mut Self| {
            let token_start = lexer.offset;
            let (kind, w, leading) =
                lexer.scan_token_and_leading_trivia(Self::scan_xhp_token, KwSet::AllKeywords);
            // We do not scan trivia after an XHPOpen's >. If that is the beginning of
            // an XHP body then we want any whitespace or newlines to be leading trivia
            // of the body token.
            match kind {
                TokenKind::GreaterThan | TokenKind::SlashGreaterThan if no_trailing => {
                    let trailing = lexer.token_factory.trivia_factory_mut().make();
                    lexer
                        .token_factory
                        .make(kind, token_start, w, leading, trailing)
                }
                _ => {
                    let trailing = lexer.scan_trailing_php_trivia();
                    lexer
                        .token_factory
                        .make(kind, token_start, w, leading, trailing)
                }
            }
        };
        let token = self.scan_assert_progress(tokenizer);
        let token_width = token.width();
        let trailing_width = token.trailing_width();
        let token_start_offset = (self.offset) - trailing_width - token_width;
        let token_text = self.source.sub(token_start_offset, token_width);
        (token, token_text)
    }

    pub fn next_xhp_body_token(&mut self) -> TF::Token {
        let scanner = |lexer: &mut Self| {
            let token_start = lexer.offset;
            let leading = lexer.scan_leading_xhp_trivia();
            lexer.start_new_lexeme();
            let kind = lexer.scan_xhp_body();
            let w = lexer.width();
            let trailing =
                // Trivia (leading and trailing) is semantically
                // significant for XHPBody tokens. When we find elements or
                // braced expressions inside the body, the trivia should be
                // seen as leading the next token, but we should certainly
                // keep it trailing if this is an XHPBody token.
                if kind == TokenKind::XHPBody {
                    lexer.scan_trailing_xhp_trivia()
                } else {
                    lexer.token_factory.trivia_factory_mut().make()
                };
            lexer
                .token_factory
                .make(kind, token_start, w, leading, trailing)
        };
        self.scan_assert_progress(scanner)
    }

    //
    // When the xhp modifier is used for declaring xhp classes
    // we do not allow colon prefixes or dashes.
    //
    // This ensures that the syntax is closer to regular classes.
    //
    pub fn next_xhp_modifier_class_name(&mut self) -> TF::Token {
        self.scan_token_and_trivia(&Self::scan_xhp_modifier_class_name, KwSet::NoKeywords)
    }

    pub fn next_xhp_class_name(&mut self) -> TF::Token {
        self.scan_token_and_trivia(&Self::scan_xhp_class_name, KwSet::NoKeywords)
    }

    pub fn next_xhp_name(&mut self) -> TF::Token {
        let scanner = |x: &mut Self| x.scan_xhp_element_name(false);
        self.scan_token_and_trivia(&scanner, KwSet::NoKeywords)
    }

    fn make_hashbang_token(&mut self) -> TF::Token {
        let leading = self.token_factory.trivia_factory_mut().make();
        self.skip_to_end_of_line();
        let token_start = self.start;
        let token_width = self.width();
        let trailing = self.scan_trailing_php_trivia();
        self.start_new_lexeme();
        self.token_factory.make(
            TokenKind::Hashbang,
            token_start,
            token_width,
            leading,
            trailing,
        )
    }

    fn make_long_tag(
        &mut self,
        name_token_offset: usize,
        size: usize,
        less_than_question_token: TF::Token,
    ) -> (TF::Token, Option<TF::Token>) {
        // skip name
        self.advance(size);
        // single line comments that follow the language in leading markup_text
        // determine the file check mode, read the trailing trivia and attach it
        // to the language token
        let trailing = self.scan_trailing_php_trivia();
        let leading = self.token_factory.trivia_factory_mut().make();
        let name =
            self.token_factory
                .make(TokenKind::Name, name_token_offset, size, leading, trailing);
        (less_than_question_token, Some(name))
    }

    fn make_markup_suffix(&mut self) -> (TF::Token, Option<TF::Token>) {
        let leading = self.token_factory.trivia_factory_mut().make();
        let trailing = self.token_factory.trivia_factory_mut().make();
        let less_than_question_token = self.token_factory.make(
            TokenKind::LessThanQuestion,
            self.offset,
            2,
            leading,
            trailing,
        );
        // skip <?
        self.advance(2);
        let name_token_offset = self.offset;
        let ch0 = self.peek_char(0).to_ascii_lowercase();
        let ch1 = self.peek_char(1).to_ascii_lowercase();
        match (ch0, ch1) {
            ('h', 'h') => self.make_long_tag(name_token_offset, 2, less_than_question_token),
            _ => (less_than_question_token, (None)),
        }
    }

    fn skip_to_end_of_header(
        &mut self,
    ) -> (Option<TF::Token>, Option<(TF::Token, Option<TF::Token>)>) {
        let start_offset = {
            // if leading section starts with #! - it should span the entire line
            if self.offset != 0 {
                panic!("Should only try to lex header at start of document")
            };
            // this should really just be `self.offset` - but, skip whitespace as the FFP
            // tests use magic comments in leading markup to set flags, but blank
            // them out before parsing; the newlines are kept to provide correct line
            // numbers in errors
            self.skip_while_to_offset(|x| Self::is_newline(x) || Self::is_whitespace_no_newline(x))
        };
        let hashbang = if self.peek_def(start_offset, INVALID) == '#'
            && self.peek_def(start_offset + 1, INVALID) == '!'
        {
            self.with_offset(start_offset);
            Some(self.make_hashbang_token())
        } else {
            None
        };

        let start_offset =
            self.skip_while_to_offset(|x| Self::is_newline(x) || Self::is_whitespace_no_newline(x));
        let suffix = if self.peek_def(start_offset, INVALID) == '<'
            && self.peek_def(start_offset + 1, INVALID) == '?'
        {
            self.with_offset(start_offset);
            Some(self.make_markup_suffix())
        } else {
            None
        };

        (hashbang, suffix)
    }

    pub fn scan_header(&mut self) -> (Option<TF::Token>, Option<(TF::Token, Option<TF::Token>)>) {
        self.start_new_lexeme();
        self.skip_to_end_of_header()
    }

    pub fn is_next_xhp_category_name(&self) -> bool {
        let mut lexer = self.clone();
        let _ = lexer.scan_leading_php_trivia();
        // An XHP category is an xhp element name preceded by a %.
        let ch0 = lexer.peek_char(0);
        let ch1 = lexer.peek_char(1);
        ch0 == '%' && Self::is_name_nondigit(ch1)
    }

    fn scan_xhp_category_name(&mut self) -> TokenKind {
        if self.is_next_xhp_category_name() {
            self.advance(1);
            let _ = self.scan_xhp_element_name(false);
            TokenKind::XHPCategoryName
        } else {
            self.scan_token(false)
        }
    }

    pub fn next_xhp_category_name(&mut self) -> TF::Token {
        self.scan_token_and_trivia(&Self::scan_xhp_category_name, KwSet::NoKeywords)
    }
}
