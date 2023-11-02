// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::str::FromStr;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Result;

use crate::token::Line;
use crate::token::Token;

// We initially planned on using Logos, a crate for tokenizing really fast.
// We chose not to use Logos because it doesn't support all regexes -- for instance, it can't
// tokenize based on the regex "\"\"\".*\"\"\"". Here's the git issue:
// https://github.com/maciejhirsz/logos/issues/246
#[derive(Clone)]
pub(crate) struct Lexer<'a> {
    line: Line,
    pending: Option<Token<'a>>, // Cached next (non-newline)
    source: &'a [u8],
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Token<'a>;

    /// Returns the next token, never returning a newline and instead advancing the lexer past them.
    fn next(&mut self) -> Option<Self::Item> {
        self.fill(); // If `pending` already has a token this does nothing
        self.pending.take()
    }
}

impl<'a> Lexer<'a> {
    pub(crate) fn error(&mut self, err: impl std::fmt::Display) -> anyhow::Error {
        if let Some(tok) = self.peek() {
            tok.error(err)
        } else {
            anyhow!("Error [end of file]: {err}")
        }
    }

    /// If `pending` is none, fills with the next non-newline token, or None of one does not exist.
    fn fill(&mut self) {
        if self.pending.is_none() {
            loop {
                (self.pending, self.source) = parse_token(self.source, self.line);
                match self.pending {
                    None => {
                        // Either a whitespace or EOF
                        if self.source.is_empty() {
                            return;
                        }
                    }
                    Some(Token::Newline(_)) => {
                        self.line.0 += 1;
                    }
                    Some(_) => break,
                }
            }
        }
    }

    pub(crate) fn is_empty(&mut self) -> bool {
        self.fill();
        self.pending.is_none()
    }

    fn find_next_newline(&self) -> Option<usize> {
        // Find the next newline - but ignore it if it's in quotes. We handle
        // triple quotes as a trick - '"""' is no different than '""' followed
        // by '"'.
        let mut in_quotes = false;
        let mut escape = false;
        self.source.iter().copied().position(|ch| {
            if escape {
                escape = false;
            } else if !in_quotes && ch == b'\n' {
                return true;
            } else if ch == b'\"' {
                in_quotes = !in_quotes;
            } else if ch == b'\\' {
                escape = true;
            }

            false
        })
    }

    /// Advances the lexer past its first non-leading newline, returning a mini-lexer of the tokens up until that newline.
    pub(crate) fn split_at_newline(&mut self) -> Option<Lexer<'a>> {
        self.fill();
        if let Some(pending) = self.pending.take() {
            let idx = self.find_next_newline().unwrap_or(self.source.len());
            let source;
            (source, self.source) = self.source.split_at(idx);
            Some(Lexer {
                line: self.line,
                pending: Some(pending),
                source,
            })
        } else {
            None
        }
    }

    /// Similarly to `Lexer::next`, will not return a peek to a newline. Instead
    /// this `peek` returns a view to the first token that's not a newline, and modifies the lexer in that it strips
    /// it of leading newlines.
    pub(crate) fn peek(&mut self) -> Option<&Token<'a>> {
        self.fill();
        self.pending.as_ref()
    }

    /// Peeks at the next token past the current one. Unlike peek() won't skip
    /// newlines. Doesn't cache the token so this call can be (relatively)
    /// expensive if used often.
    pub(crate) fn peek1(&mut self) -> Option<Token<'a>> {
        self.fill();
        let (peek1, _) = parse_token(self.source, self.line);
        peek1
    }

    pub(crate) fn expect_token(&mut self) -> Result<Token<'a>> {
        self.next()
            .ok_or_else(|| anyhow!("End of token stream sooner than expected"))
    }

    /// Returns f() applied to the next token without consuming the next token.
    pub(crate) fn peek_is<F>(&mut self, f: F) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek().map_or(false, f)
    }

    /// Applies f() to the next token and then compares the result with the
    /// passed-in string.
    pub(crate) fn peek_is_str<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek_is(|t| f(t) && t.as_bytes() == s.as_bytes())
    }

    /// Applies f() to the next token and consumes it if the predicate passes.
    /// Returns Some(token) if the predicate passed.
    pub(crate) fn next_if<F>(&mut self, f: F) -> Option<Token<'a>>
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tr = self.peek_is(f);
        if tr { self.next() } else { None }
    }

    /// Applies f() to the next token and consumes it if the predicate
    /// passes. Returns true if the predicate passed.
    pub(crate) fn next_is<F>(&mut self, f: F) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.next_if(f).is_some()
    }

    /// Applies f() to the next token and consumes it if the predicate passes
    /// and matches the given string.
    pub(crate) fn next_is_str<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tr = self.peek_is_str(f, s);
        if tr {
            self.next();
        }
        tr
    }

    /// Applies f() to the next token and consumes it if the predicate
    /// passes. Returns Ok(Token) if the predicate passed or Err() if the
    /// predicate failed.
    pub(crate) fn expect<F>(&mut self, f: F) -> Result<Token<'a>>
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tok = self.expect_token()?;
        if f(&tok) {
            Ok(tok)
        } else {
            Err(tok.error("Unexpected token"))
        }
    }

    /// Applies f() to the next token and returns the result.
    /// expected token (expected token specified by f)
    pub(crate) fn expect_with<T, F>(&mut self, f: F) -> Result<T>
    where
        F: FnOnce(Token<'a>) -> Result<T>,
    {
        f(self.expect_token()?)
    }

    /// Checks both the f() predicate and that the str matches.
    pub(crate) fn expect_str<F>(&mut self, f: F, s: &str) -> Result<Token<'a>>
    where
        F: FnOnce(&Token<'a>) -> bool,
    {
        let tok = self.expect_token()?;
        if f(&tok) && tok.as_bytes() == s.as_bytes() {
            Ok(tok)
        } else {
            Err(tok.error(format!("Expected {s:?} got {:?}", tok.as_bytes())))
        }
    }

    /// Similar to `expect` but instead of returning a Result that usually contains a slice of u8,
    /// applies f to the `from_utf8 str` of the top token, bailing if the top token is not a number.
    pub(crate) fn expect_and_get_number<T: FromStr>(&mut self) -> Result<T> {
        let num = if self.peek_is(Token::is_dash) {
            self.expect_with(Token::into_dash)? // -INF
        } else if self.peek_is(Token::is_identifier) {
            self.expect_with(Token::into_identifier)? // NAN, INF, etc will parse as float constants
        } else {
            self.expect_with(Token::into_number)?
        };
        // If num is a dash we expect an identifier to follow
        if num == b"-" {
            let mut num = num.to_vec();
            num.extend_from_slice(self.expect_with(Token::into_identifier)?);
            FromStr::from_str(std::str::from_utf8(&num)?).map_err(|_| {
                anyhow!("Number-looking token in tokenizer that cannot be parsed into number")
            })
        } else {
            FromStr::from_str(std::str::from_utf8(num)?).map_err(|_| {
                anyhow!("Number-looking token in tokenizer that cannot be parsed into number")
            }) // This std::str::from_utf8 will never bail; if it should bail, the above `expect` bails first.
        }
    }

    /// A var can be written in HHAS as $abc or "$abc". Only valid if a $ preceeds
    pub(crate) fn expect_var(&mut self) -> Result<Vec<u8>> {
        if self.peek_is(Token::is_str_literal) {
            let s = self.expect_with(Token::into_str_literal)?;
            if s.starts_with(b"\"$") {
                // Remove the "" b/c that's not part of the var name
                // also unescape ("$\340\260" etc is literal bytes)
                let s = escaper::unquote_slice(s);
                let s = escaper::unescape_literal_bytes_into_vec_bytes(s)?;
                Ok(s)
            } else {
                bail!("Var does not start with $: {:?}", s)
            }
        } else {
            Ok(self.expect_with(Token::into_variable)?.to_vec())
        }
    }

    /// Bails if lexer is not empty, message contains the next token
    pub(crate) fn expect_end(&mut self) -> Result<()> {
        ensure!(
            self.is_empty(),
            "Expected end of token stream, see: {}",
            self.next().unwrap()
        );
        Ok(())
    }

    pub(crate) fn from_slice(source: &'a [u8], start_line: Line) -> Self {
        Lexer {
            line: start_line,
            source,
            pending: None,
        }
    }

    pub(crate) fn parse_list<T, F>(&mut self, mut callback: F) -> Result<Vec<T>>
    where
        F: FnMut(&mut Self) -> Result<Option<T>>,
    {
        // a b c
        let mut result = Vec::new();
        while let Some(t) = callback(self)? {
            result.push(t);
        }
        Ok(result)
    }

    pub(crate) fn parse_comma_list<T, F>(
        &mut self,
        expect_comma: bool,
        mut callback: F,
    ) -> Result<Vec<T>>
    where
        F: FnMut(&mut Self) -> Result<T>,
    {
        // a, b, c
        let mut result = Vec::new();
        if !expect_comma {
            result.push(callback(self)?);
        }
        while self.next_is(Token::is_comma) {
            result.push(callback(self)?);
        }

        Ok(result)
    }
}

fn is_identifier_lead(ch: u8) -> bool {
    ch.is_ascii_alphabetic() || (ch >= 0x80) || (ch == b'_') || (ch == b'/')
}

fn is_identifier_tail(ch: u8) -> bool {
    // r"(?-u)[_/a-zA-Z\x80-\xff]([_/\\a-zA-Z0-9\x80-\xff\.\$#\-]|::)*"
    //
    // NOTE: identifiers can include "::" but this won't match that because it's
    // a multi-byte sequence.
    ch.is_ascii_alphanumeric()
        || (ch >= 0x80)
        || (ch == b'.')
        || (ch == b'$')
        || (ch == b'#')
        || (ch == b'-')
        || (ch == b'_')
        || (ch == b'/')
        || (ch == b'\\')
}

fn gather_identifier(source: &[u8]) -> (&[u8], &[u8]) {
    // This can't be easy because ':' isn't part of an identifier, but '::'
    // is...
    let mut len = 1;
    loop {
        len += source[len..]
            .iter()
            .copied()
            .take_while(|&c| is_identifier_tail(c))
            .count();
        if !source[len..].starts_with(b"::") {
            break;
        }
        len += 2;
    }
    source.split_at(len)
}

fn is_number_lead(ch: u8) -> bool {
    ch.is_ascii_digit()
}

fn is_number_tail(ch: u8) -> bool {
    // r"[-+]?[0-9]+\.?[0-9]*([eE][-+]?[0-9]+\.?[0-9]*)?"
    ch.is_ascii_digit() || (ch == b'.')
}

fn gather_number(source: &[u8]) -> (&[u8], &[u8]) {
    // The plus location (only after 'e') makes this tricky.
    let mut last_e = false;
    let len = source[1..]
        .iter()
        .copied()
        .take_while(|&c| {
            if is_number_tail(c) {
                last_e = false;
            } else if c == b'e' || c == b'E' {
                last_e = true;
            } else if (c == b'+' || c == b'-') && last_e {
                last_e = false;
            } else {
                return false;
            }
            true
        })
        .count();
    source.split_at(1 + len)
}

fn is_global_tail(ch: u8) -> bool {
    // r"(?-u)[\.@][_a-z/A-Z\x80-\xff][_/a-zA-Z/0-9\x80-\xff\-\.]*"
    ch.is_ascii_alphanumeric() || (ch >= 0x80) || (ch == b'.') || (ch == b'-') || (ch == b'_')
}

fn is_var_tail(ch: u8) -> bool {
    // r"(?-u)\$[_a-zA-Z0-9$\x80-\xff][_/a-zA-Z0-9$\x80-\xff]*"
    ch.is_ascii_alphanumeric() || (ch >= 0x80) || (ch == b'$') || (ch == b'_')
}

fn is_decl_tail(ch: u8) -> bool {
    // r"(?-u)[\.@][_a-z/A-Z\x80-\xff][_/a-zA-Z/0-9\x80-\xff\-\.]*", // Decl, global. (?-u) turns off utf8 check
    ch.is_ascii_alphanumeric() || (ch >= 0x80) || (ch == b'-') || (ch == b'.') || (ch == b'_')
}

fn is_non_newline_whitespace(ch: u8) -> bool {
    ch == b' ' || ch == b'\t' || ch == b'\r'
}

fn gather_tail<F>(source: &[u8], f: F) -> (&[u8], &[u8])
where
    F: Fn(u8) -> bool,
{
    let len = source[1..].iter().copied().take_while(|c| f(*c)).count();
    source.split_at(1 + len)
}

fn gather_quoted(source: &[u8], count: usize) -> (&[u8], &[u8]) {
    let mut quotes = 0;
    let mut escaped = false;
    let len = source[count..]
        .iter()
        .copied()
        .take_while(|&c| {
            if quotes == count {
                return false;
            } else if c == b'"' && !escaped {
                quotes += 1;
            } else if c == b'\\' && !escaped {
                escaped = true;
                quotes = 0;
            } else {
                escaped = false;
                quotes = 0;
            }
            true
        })
        .count();
    source.split_at(count + len)
}

#[allow(clippy::todo)]
fn parse_token(mut source: &[u8], line: Line) -> (Option<Token<'_>>, &[u8]) {
    let tok = if let Some(lead) = source.first() {
        match *lead {
            ch if is_identifier_lead(ch) => {
                // Identifier
                let tok;
                (tok, source) = gather_identifier(source);
                Some(Token::Identifier(tok, line))
            }
            ch if is_number_lead(ch) => {
                // Number
                let tok;
                (tok, source) = gather_number(source);
                Some(Token::Number(tok, line))
            }
            ch if is_non_newline_whitespace(ch) => {
                (_, source) = gather_tail(source, is_non_newline_whitespace);
                None
            }
            b'#' => {
                // Don't consume the newline.
                let len = source[1..]
                    .iter()
                    .copied()
                    .take_while(|&c| c != b'\n')
                    .count();
                (_, source) = source.split_at(1 + len);
                None
            }
            b'\n' => {
                (_, source) = source.split_at(1);
                Some(Token::Newline(line))
            }
            b'@' => {
                if source.len() > 1 {
                    // Global
                    let tok;
                    (tok, source) = gather_tail(source, is_global_tail);
                    Some(Token::Global(tok, line))
                } else {
                    // Error
                    let tok = std::mem::take(&mut source);
                    Some(Token::Error(tok, line))
                }
            }
            b'$' => {
                if source.len() > 1 {
                    // Var
                    let tok;
                    (tok, source) = gather_tail(source, is_var_tail);
                    Some(Token::Variable(tok, line))
                } else {
                    // Error
                    let tok = std::mem::take(&mut source);
                    Some(Token::Error(tok, line))
                }
            }
            b'"' => {
                if source.starts_with(b"\"\"\"") {
                    // Triple string literal
                    let tok;
                    (tok, source) = gather_quoted(source, 3);
                    Some(Token::TripleStrLiteral(tok, line))
                } else {
                    // Single string literal
                    let tok;
                    (tok, source) = gather_quoted(source, 1);
                    Some(Token::StrLiteral(tok, line))
                }
            }
            b'.' => {
                if source.starts_with(b"...") {
                    // Variadic
                    (_, source) = source.split_at(3);
                    Some(Token::Variadic(line))
                } else if source.len() > 1 {
                    // Decl
                    let tok;
                    (tok, source) = gather_tail(source, is_decl_tail);
                    Some(Token::Decl(tok, line))
                } else {
                    // Error
                    let tok = std::mem::take(&mut source);
                    Some(Token::Error(tok, line))
                }
            }
            b'-' => {
                if source.get(1).copied().map_or(false, is_number_lead) {
                    // Negative number
                    let tok;
                    (tok, source) = gather_number(source);
                    Some(Token::Number(tok, line))
                } else {
                    // Dash
                    (_, source) = source.split_at(1);
                    Some(Token::Dash(line))
                }
            }
            b'+' => {
                // Positive number
                let tok;
                (tok, source) = gather_number(source);
                Some(Token::Number(tok, line))
            }
            b';' => {
                (_, source) = source.split_at(1);
                Some(Token::Semicolon(line))
            }
            b'{' => {
                (_, source) = source.split_at(1);
                Some(Token::OpenCurly(line))
            }
            b'[' => {
                (_, source) = source.split_at(1);
                Some(Token::OpenBracket(line))
            }
            b'(' => {
                (_, source) = source.split_at(1);
                Some(Token::OpenParen(line))
            }
            b')' => {
                (_, source) = source.split_at(1);
                Some(Token::CloseParen(line))
            }
            b']' => {
                (_, source) = source.split_at(1);
                Some(Token::CloseBracket(line))
            }
            b'}' => {
                (_, source) = source.split_at(1);
                Some(Token::CloseCurly(line))
            }
            b',' => {
                (_, source) = source.split_at(1);
                Some(Token::Comma(line))
            }
            b'<' => {
                (_, source) = source.split_at(1);
                Some(Token::Lt(line))
            }
            b'>' => {
                (_, source) = source.split_at(1);
                Some(Token::Gt(line))
            }
            b'=' => {
                (_, source) = source.split_at(1);
                Some(Token::Equal(line))
            }
            b':' => {
                (_, source) = source.split_at(1);
                Some(Token::Colon(line))
            }
            _ => todo!("CH: {lead:?} ({})", *lead as char),
        }
    } else {
        None
    };
    (tok, source)
}

#[cfg(test)]
mod test {
    use bumpalo::Bump;

    use super::*;
    use crate::assemble;

    #[test]
    fn str_into_test() -> Result<()> {
        // Want to test that only tokens surrounded by "" are str_literals
        // Want to confirm the assumption that after any token_iter.expect_with(Token::into_str_literal) call, you can safely remove the first and last element in slice
        let s = br#"abc "abc" """abc""""#;
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.next_is(Token::is_identifier));
        let sl = lex.expect_with(Token::into_str_literal)?;
        assert!(sl[0] == b'"' && sl[sl.len() - 1] == b'"');
        let tsl = lex.expect_with(Token::into_triple_str_literal)?;
        assert!(
            tsl[0..3] == [b'"', b'"', b'"'] && tsl[tsl.len() - 3..tsl.len()] == [b'"', b'"', b'"']
        );
        Ok(())
    }

    #[test]
    fn just_nl_is_empty() {
        let s = b"\n \n \n";
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.split_at_newline().is_none());
        assert!(lex.is_empty());
    }

    #[test]
    fn splits_mult_newlines_go_away() {
        // Point of this test: want to make sure that 3 mini-lexers are spawned (multiple new lines don't do anything)
        let s = b"\n \n a \n \n \n b \n \n c \n";
        let mut lex = Lexer::from_slice(s, Line(1));
        let mut a = lex.split_at_newline().unwrap();
        let mut b = lex.split_at_newline().unwrap();
        let mut c = lex.split_at_newline().unwrap();

        assert_eq!(lex.next(), None);

        assert_eq!(a.next().unwrap().into_identifier().unwrap(), b"a");
        assert_eq!(a.next(), None);

        assert_eq!(b.next().unwrap().into_identifier().unwrap(), b"b");
        assert_eq!(b.next(), None);

        assert_eq!(c.next().unwrap().into_identifier().unwrap(), b"c");
        assert_eq!(c.next(), None);
    }

    #[test]
    fn no_trailing_newlines() {
        let s = b"a \n \n \n";
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.next().is_some());
        assert!(lex.is_empty());
    }

    #[test]
    #[allow(unused)]
    fn splitting_multiple_lines() {
        let s = b".try { \n .srcloc 3:7, 3:22 \n String \"I'm i\\\"n the try\n\" \n Print \n PopC \n } .catch { \n Dup \n L1: \n Throw \n }";
        let mut lex = Lexer::from_slice(s, Line(1));

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_decl().unwrap(), b".try");
        assert!(sub.next().unwrap().is_open_curly());
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_decl().unwrap(), b".srcloc");
        assert_eq!(sub.next().unwrap().into_number().unwrap(), b"3");
        assert!(sub.next().unwrap().is_colon());
        assert_eq!(sub.next().unwrap().into_number().unwrap(), b"7");
        assert!(sub.next().unwrap().is_comma());
        assert_eq!(sub.next().unwrap().into_number().unwrap(), b"3");
        assert!(sub.next().unwrap().is_colon());
        assert_eq!(sub.next().unwrap().into_number().unwrap(), b"22");
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_identifier().unwrap(), b"String");
        assert_eq!(
            sub.next().unwrap().into_str_literal().unwrap(),
            b"\"I'm i\\\"n the try\n\""
        );
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_identifier().unwrap(), b"Print");
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_identifier().unwrap(), b"PopC");
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert!(sub.next().unwrap().is_close_curly());
        assert_eq!(sub.next().unwrap().into_decl().unwrap(), b".catch");
        assert!(sub.next().unwrap().is_open_curly());
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_identifier().unwrap(), b"Dup");
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_identifier().unwrap(), b"L1");
        assert!(sub.next().unwrap().is_colon());
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert_eq!(sub.next().unwrap().into_identifier().unwrap(), b"Throw");
        assert_eq!(sub.next(), None);

        let mut sub = lex.split_at_newline().unwrap();
        assert!(sub.next().unwrap().is_close_curly());
        assert_eq!(sub.next(), None);

        assert_eq!(lex.next(), None);
    }

    #[test]
    fn peek_next_on_newlines() {
        let s = b"\n\na\n\n";
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.peek().is_some());
        assert!(lex.next().is_some());
        assert!(lex.split_at_newline().is_none()); // Have consumed the a here -- "\n\n" was left and that's been consumed.
    }

    #[test]
    #[should_panic]
    fn no_top_level_shouldnt_parse() {
        // Is there a better way, maybe to verify the string in the bail?
        let s = b".srloc 3:7,3:22";
        let alloc = Bump::default();
        assert!(matches!(assemble::assemble_from_bytes(&alloc, s), Ok(_)))
    }

    #[test]
    #[should_panic]
    fn no_fpath_semicolon_shouldnt_parse() {
        let s = br#".filepath "aaaa""#;
        let alloc = Bump::default();
        assert!(matches!(assemble::assemble_from_bytes(&alloc, s), Ok(_)))
    }

    #[test]
    #[should_panic]
    fn fpath_wo_file_shouldnt_parse() {
        let s = br#".filepath aaa"#;
        let alloc = Bump::default();
        assert!(matches!(assemble::assemble_from_bytes(&alloc, s), Ok(_)))
    }

    #[test]
    fn difficult_strings() {
        let s = br#""\"0\""
        "12345\\:2\\"
        "class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class";
        "#;
        let mut l: Lexer<'_> = Lexer::from_slice(s, Line(1));
        // Expecting 3 string tokens
        let _st1 = l.next().unwrap();
        let _by1 = str::as_bytes(r#""\"0\"""#);
        assert!(matches!(_st1, Token::StrLiteral(_by1, _)));
        let _st2 = l.next().unwrap();
        let _by2 = str::as_bytes(r#""12345\\:2\\""#);
        assert!(matches!(_st1, Token::StrLiteral(_by2, _)));
        let _st3 = l.next().unwrap();
        let _by3 = str::as_bytes(
            r#""class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class""#,
        );
        assert!(matches!(_st1, Token::StrLiteral(_by3, _)));
    }

    #[test]
    fn odd_unicode_test() {
        let s: &[u8] = b".\xA9\xEF\xB8\x8E $0\xC5\xA3\xB1\xC3 \xE2\x98\xBA\xE2\x98\xBA\xE2\x98\xBA @\xE2\x99\xA1\xE2\x99\xA4$";
        let mut l: Lexer<'_> = Lexer::from_slice(s, Line(1));
        // We are expecting an decl, a var, an identifier a global, and an error on the last empty variable
        let decl = l.next().unwrap();
        assert!(matches!(decl, Token::Decl(..)));
        let var = l.next().unwrap();
        assert!(matches!(var, Token::Variable(..)));
        let iden = l.next().unwrap();
        assert!(matches!(iden, Token::Identifier(..)));
        let glob = l.next().unwrap();
        assert!(matches!(glob, Token::Global(..)));
        let err = l.next().unwrap();
        assert!(matches!(err, Token::Error(..)))
    }

    #[test]
    fn every_token_test() {
        let s = br#"@_global $0Var """tripleStrLiteral:)""" #hashtagComment
        .Decl "str!Literal" ...
        ;-{[( )]} =98 -98 +101. 43.2 , < > : _/identifier/ /filepath id$di aa:bb aa::bb ."#;
        // Expect glob var tsl decl strlit semicolon dash open_curly open_brack open_paren close_paren close_bracket
        // close_curly equal number number number number , < > : identifier identifier ERROR on the last .
        let mut l: Lexer<'_> = Lexer::from_slice(s, Line(1));
        assert_eq!(l.next().unwrap().into_global().unwrap(), b"@_global");
        assert_eq!(l.next().unwrap().into_variable().unwrap(), b"$0Var");
        assert_eq!(
            l.next().unwrap().into_triple_str_literal().unwrap(),
            br#""""tripleStrLiteral:)""""#
        );
        assert_eq!(l.next().unwrap().into_decl().unwrap(), b".Decl");
        assert_eq!(
            l.next().unwrap().into_str_literal().unwrap(),
            br#""str!Literal""#
        );
        assert!(l.next().unwrap().is_variadic());
        assert!(l.next().unwrap().is_semicolon());
        assert!(l.next().unwrap().is_dash());
        assert!(l.next().unwrap().is_open_curly());
        assert!(l.next().unwrap().is_open_bracket());
        assert!(l.next().unwrap().is_open_paren());
        assert!(l.next().unwrap().is_close_paren());
        assert!(l.next().unwrap().is_close_bracket());
        assert!(l.next().unwrap().is_close_curly());
        assert!(l.next().unwrap().is_equal());
        assert_eq!(l.next().unwrap().into_number().unwrap(), b"98");
        assert_eq!(l.next().unwrap().into_number().unwrap(), b"-98");
        assert_eq!(l.next().unwrap().into_number().unwrap(), b"+101.");
        assert_eq!(l.next().unwrap().into_number().unwrap(), b"43.2");
        assert!(l.next().unwrap().is_comma());
        assert!(l.next().unwrap().is_lt());
        assert!(l.next().unwrap().is_gt());
        assert!(l.next().unwrap().is_colon());
        assert_eq!(
            l.next().unwrap().into_identifier().unwrap(),
            b"_/identifier/"
        );
        assert_eq!(l.next().unwrap().into_identifier().unwrap(), b"/filepath");
        assert_eq!(l.next().unwrap().into_identifier().unwrap(), b"id$di");
        assert_eq!(l.next().unwrap().into_identifier().unwrap(), b"aa");
        assert!(l.next().unwrap().is_colon());
        assert_eq!(l.next().unwrap().into_identifier().unwrap(), b"bb");
        assert_eq!(l.next().unwrap().into_identifier().unwrap(), b"aa::bb");

        let err = l.next().unwrap();
        assert!(matches!(err, Token::Error(..)), "failed to match {}", err);
    }
}
