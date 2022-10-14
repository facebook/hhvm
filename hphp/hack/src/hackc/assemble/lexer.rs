// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;
use std::str::FromStr;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Result;
use bumpalo::Bump;
use ffi::Str;
use once_cell::sync::OnceCell;
use regex::bytes::Regex;

use crate::token::Line;
use crate::token::Token;

// We initially planned on using Logos, a crate for tokenizing really fast.
// We chose not to use Logos because it doesn't support all regexes -- for instance, it can't
// tokenize based on the regex "\"\"\".*\"\"\"". Here's the git issue:
// https://github.com/maciejhirsz/logos/issues/246
pub(crate) struct Lexer<'a> {
    pending: Option<Token<'a>>, // Cached next (non-newline)
    tokens: VecDeque<Token<'a>>,
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
            self.pending = self.get_next_no_newlines();
        }
    }

    /// Returns the next non-newline token (or None), advancing the lexer past any leading newlines.
    fn get_next_no_newlines(&mut self) -> Option<Token<'a>> {
        while let Some(t) = self.tokens.pop_front() {
            if !t.is_newline() {
                return Some(t);
            }
        }
        None
    }

    pub(crate) fn is_empty(&mut self) -> bool {
        self.fill();
        if self.pending.is_none() {
            debug_assert!(self.tokens.is_empty()); // VD should never have \n \n \n and empty pending
            true
        } else {
            false
        }
    }

    /// Advances the lexer passed its first non-leading newline, returning a mini-lexer of the tokens up until that newline.
    pub(crate) fn fetch_until_newline(&mut self) -> Option<Lexer<'a>> {
        self.fill();
        if let Some(t) = self.pending {
            let mut first_toks = VecDeque::new();
            // Add what was in `pending`
            first_toks.push_back(t);
            // While we haven't reached the new line
            while let Some(to_push) = self.tokens.pop_front() {
                if to_push.is_newline() {
                    break;
                }
                // Add token
                first_toks.push_back(to_push);
            }
            self.pending = None;
            Some(Lexer {
                tokens: first_toks,
                pending: None,
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

    /// Returns f applied to the top of the iterator, but does not advance iterator (unlike next_if)
    pub(crate) fn peek_if<F>(&mut self, f: F) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek().map_or(false, f)
    }

    /// Applies f to top of iterator, and if true advances iterator and returns true else doesn't advance and returns false
    pub(crate) fn next_if<F>(&mut self, f: F) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tr = self.peek_if(f);
        if tr {
            self.next();
        }
        tr
    }

    /// Applies f to top of iterator. If true, compares inner rep to passed &str and returns result. Else just false
    pub(crate) fn peek_if_str<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek_if(|t| f(t) && t.as_bytes() == s.as_bytes())
    }

    /// Applies f to top of iterator. If true, checks if starts with passed &str and returns result. Else just false
    /// Ex use: peek_if_str_starts(Token::is_decl, ".coeffects")
    pub(crate) fn peek_if_str_starts<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek_if(|t| f(t) && t.as_bytes().starts_with(s.as_bytes()))
    }

    /// Applies f to top of iterator. If true, compares inner representation to passed &str and if true consumes. Else doesn't modify iterator and returns false.
    pub(crate) fn next_if_str<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tr = self.peek_if_str(f, s);
        if tr {
            self.next();
        }
        tr
    }

    pub(crate) fn expect_token(&mut self) -> Result<Token<'a>> {
        self.next()
            .ok_or_else(|| anyhow!("End of token stream sooner than expected"))
    }

    /// Applies f to the top of token_iter. In the most likely use, f bails if token_iter is not the
    /// expected token (expected token specified by f)
    pub(crate) fn expect<T, F>(&mut self, f: F) -> Result<T>
    where
        F: FnOnce(Token<'a>) -> Result<T>,
    {
        f(self.expect_token()?)
    }

    /// Like `expect` in that bails if incorrect token passed, and does return the `into` of the passed token
    pub(crate) fn expect_is_str<F>(&mut self, f: F, s: &str) -> Result<&[u8]>
    where
        F: FnOnce(Token<'a>) -> Result<&[u8]>,
    {
        let tok = self.expect_token()?;
        let tr = f(tok)?;
        if tr != s.as_bytes() {
            Err(tok.error(format!("Expected {s:?} got {tr:?}")))
        } else {
            Ok(tr)
        }
    }

    /// Similar to `expect_and_get_number` but puts identifier into a Str<'arena>
    pub(crate) fn expect_identifier_into_ffi_str<'arena>(
        &mut self,
        alloc: &'arena Bump,
    ) -> Result<Str<'arena>> {
        let st = self.expect(Token::into_identifier)?;
        Ok(Str::new_slice(alloc, st))
    }

    /// Similar to `expect_and_get_number` but puts decl into a Str<'arena>
    pub(crate) fn expect_decl_into_ffi_str<'arena>(
        &mut self,
        alloc: &'arena Bump,
    ) -> Result<Str<'arena>> {
        let st = self.expect(Token::into_decl)?;
        Ok(Str::new_slice(alloc, st))
    }

    /// Similar to `expect` but instead of returning a Result that usually contains a slice of u8,
    /// applies f to the `from_utf8 str` of the top token, bailing if the top token is not a number.
    pub(crate) fn expect_and_get_number<T: FromStr>(&mut self) -> Result<T> {
        let num = if self.peek_if(Token::is_dash) {
            self.expect(Token::into_dash)? // -INF
        } else if self.peek_if(Token::is_identifier) {
            self.expect(Token::into_identifier)? // NAN, INF, etc will parse as float constants
        } else {
            self.expect(Token::into_number)?
        };
        // If num is a dash we expect an identifier to follow
        if num == b"-" {
            let mut num = num.to_vec();
            num.extend_from_slice(self.expect(Token::into_identifier)?);
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
        if self.peek_if(Token::is_str_literal) {
            let s = self.expect(Token::into_str_literal)?;
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
            Ok(self.expect(Token::into_variable)?.to_vec())
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

    fn make_regex() -> Regex {
        // First create the regex that matches any token. Done this way for
        // readability
        let v = [
            r#""""([^"\\]|\\.)*?""""#, // Triple str literal
            "#.*",                     // Comment
            r"(?-u)[\.@][_a-z/A-Z\x80-\xff][_/a-zA-Z/0-9\x80-\xff\-\.]*", // Decl, global. (?-u) turns off utf8 check
            r"(?-u)\$[_a-zA-Z0-9$\x80-\xff][_/a-zA-Z0-9$\x80-\xff]*",     // Var.
            r#""((\\.)|[^\\"])*""#,                                       // Str literal
            r"[-+]?[0-9]+\.?[0-9]*([eE][-+]?[0-9]+\.?[0-9]*)?",           // Number
            r"(?-u)[_/a-zA-Z\x80-\xff]([_/\\a-zA-Z0-9\x80-\xff\.\$#\-]|::)*", // Identifier
            ";",
            "-",
            "=",
            r"\{",
            r"\[",
            r"\(",
            r"\)",
            r"\]",
            r"\}",
            ",",
            "<",
            ">",
            ":",
            r"\.\.\.", // Variadic
            "\n",
            r"[ \t\r\f]+",
        ];
        let big_regex = format!("^(({}))", v.join(")|("));
        Regex::new(&big_regex).unwrap()
    }

    pub(crate) fn from_slice(s: &'a [u8], start_line: Line) -> Self {
        // According to
        // https://github.com/rust-lang/regex/blob/master/PERFORMANCE.md (and
        // directly observed) Regex keeps mutable internal state which needs to
        // be synchronized - so using it from multiple threads will be a big
        // performance regression.
        static REGEX: OnceCell<Regex> = OnceCell::new();
        let big_regex = REGEX.get_or_init(Self::make_regex).clone();

        let mut cur_line = start_line;
        let mut tokens = VecDeque::new();
        let mut source = s;
        while !source.is_empty() {
            source = build_tokens_helper(source, &mut cur_line, &mut tokens, &big_regex);
        }
        Lexer {
            pending: None,
            tokens,
        }
    }
}

fn build_tokens_helper<'a>(
    s: &'a [u8],
    cur_line: &mut Line,
    tokens: &mut VecDeque<Token<'a>>,
    big_regex: &Regex,
) -> &'a [u8] {
    if let Some(mat) = big_regex.find(s) {
        let mut chars = s.iter(); // Implicit assumption: matched to the start (^), so we iter from the start
        debug_assert!(mat.start() == 0);
        match chars.next().unwrap() {
            // Get first character
            // Note these don't match what prints out on a printer, but not sure how to generalize
            b'\x0C' => {
                // form feed

                &s[mat.end()..]
            }
            b'\r' => &s[mat.end()..],
            b'\t' => &s[mat.end()..],
            b'#' => {
                &s[mat.end()..] // Don't advance the line; the newline at the end of the comment will advance the line
            }
            b' ' => &s[mat.end()..], // Don't add whitespace as tokens, just increase line and col
            o => {
                let end = mat.end();
                let tok = match o {
                    b'\n' => {
                        let old_pos = *cur_line;
                        cur_line.0 += 1;

                        Token::Newline(old_pos)
                    }
                    b'@' => Token::Global(&s[..end], *cur_line), // Global
                    b'$' => Token::Variable(&s[..end], *cur_line), // Var
                    b'.' => {
                        if *(chars.next().unwrap()) == b'.' && *(chars.next().unwrap()) == b'.' {
                            // Variadic
                            Token::Variadic(*cur_line)
                        } else {
                            Token::Decl(&s[..end], *cur_line) // Decl
                        }
                    }
                    b';' => Token::Semicolon(*cur_line), // Semicolon
                    b'{' => Token::OpenCurly(*cur_line), // Opencurly
                    b'[' => Token::OpenBracket(*cur_line),
                    b'(' => Token::OpenParen(*cur_line),
                    b')' => Token::CloseParen(*cur_line),
                    b']' => Token::CloseBracket(*cur_line),
                    b'}' => Token::CloseCurly(*cur_line),
                    b',' => Token::Comma(*cur_line),
                    b'<' => Token::Lt(*cur_line),    //<
                    b'>' => Token::Gt(*cur_line),    //>
                    b'=' => Token::Equal(*cur_line), //=
                    b'-' => {
                        if chars.next().unwrap().is_ascii_digit() {
                            // Negative number
                            Token::Number(&s[..end], *cur_line)
                        } else {
                            Token::Dash(*cur_line)
                        }
                    }
                    b':' => Token::Colon(*cur_line),
                    b'"' => {
                        if *(chars.next().unwrap()) == b'"' && *(chars.next().unwrap()) == b'"' {
                            // Triple string literal
                            Token::TripleStrLiteral(&s[..end], *cur_line)
                        } else {
                            // Single string literal
                            Token::StrLiteral(&s[..end], *cur_line)
                        }
                    }
                    dig_or_id => {
                        if dig_or_id.is_ascii_digit()
                            || (*dig_or_id as char == '+'
                                && (chars.next().unwrap()).is_ascii_digit())
                        // Positive numbers denoted with +
                        {
                            Token::Number(&s[..end], *cur_line)
                        } else {
                            Token::Identifier(&s[..end], *cur_line)
                        }
                    }
                };
                tokens.push_back(tok);
                &s[end..]
            }
        }
    } else {
        // Couldn't tokenize the string, so add the rest of it as an error
        tokens.push_back(Token::Error(s, *cur_line));
        // Done advancing col and line cuz at end
        &[]
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::assemble;

    #[test]
    fn str_into_test() -> Result<()> {
        // Want to test that only tokens surrounded by "" are str_literals
        // Want to confirm the assumption that after any token_iter.expect(Token::into_str_literal) call, you can safely remove the first and last element in slice
        let s = r#"abc "abc" """abc""""#;
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.next_if(Token::is_identifier));
        let sl = lex.expect(Token::into_str_literal)?;
        assert!(sl[0] == b'"' && sl[sl.len() - 1] == b'"');
        let tsl = lex.expect(Token::into_triple_str_literal)?;
        assert!(
            tsl[0..3] == [b'"', b'"', b'"'] && tsl[tsl.len() - 3..tsl.len()] == [b'"', b'"', b'"']
        );
        Ok(())
    }

    #[test]
    fn just_nl_is_empty() {
        let s = "\n \n \n";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.fetch_until_newline().is_none());
        assert!(lex.is_empty());
    }

    #[test]
    fn splits_mult_newlines_go_away() {
        // Point of this test: want to make sure that 3 mini-lexers are spawned (multiple new lines don't do anything)
        let s = "\n \n a \n \n \n b \n \n c \n";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, Line(1));
        let vc_of_lexers = vec![
            lex.fetch_until_newline(),
            lex.fetch_until_newline(),
            lex.fetch_until_newline(),
        ];
        assert!(lex.is_empty());
        assert!(lex.next().is_none());
        assert_eq!(vc_of_lexers.len(), 3);
    }

    #[test]
    fn no_trailing_newlines() {
        let s = "a \n \n \n";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, Line(1));
        assert!(lex.next().is_some());
        assert!(lex.is_empty());
    }

    #[test]
    fn splitting_multiple_lines() {
        let s = ".try { \n .srloc 3:7, 3:22 \n String \"I'm in the try\n\" \n Print \n PopC \n } .catch { \n Dup \n L1: \n Throw \n }";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, Line(1));
        let mut vc_of_lexers = Vec::new();
        while !lex.is_empty() {
            vc_of_lexers.push(lex.fetch_until_newline());
        }
        assert_eq!(vc_of_lexers.len(), 10)
    }

    #[test]
    fn peek_next_on_newlines() {
        let s = "\n\na\n\n";
        let mut lex = Lexer::from_slice(s.as_bytes(), Line(1));
        assert!(lex.peek().is_some());
        assert!(lex.next().is_some());
        assert!(lex.fetch_until_newline().is_none()); // Have consumed the a here -- "\n\n" was left and that's been consumed.
    }

    #[test]
    #[should_panic]
    fn no_top_level_shouldnt_parse() {
        // Is there a better way, maybe to verify the string in the bail?
        let s = ".srloc 3:7,3:22";
        let s = s.as_bytes();
        let alloc = Bump::default();
        assert!(matches!(assemble::assemble_from_bytes(&alloc, s), Ok(_)))
    }

    #[test]
    #[should_panic]
    fn no_fpath_semicolon_shouldnt_parse() {
        let s = r#".filepath "aaaa""#;
        let s = s.as_bytes();
        let alloc = Bump::default();
        assert!(matches!(assemble::assemble_from_bytes(&alloc, s), Ok(_)))
    }

    #[test]
    #[should_panic]
    fn fpath_wo_file_shouldnt_parse() {
        let s = r#".filepath aaa"#;
        let s = s.as_bytes();
        let alloc = Bump::default();
        assert!(matches!(assemble::assemble_from_bytes(&alloc, s), Ok(_)))
    }

    #[test]
    fn difficult_strings() {
        let s = r#""\"0\""
        "12345\\:2\\"
        "class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class";
        "#;
        let s = s.as_bytes();
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
        let s = r#"@_global $0Var """tripleStrLiteral:)""" #hashtagComment
        .Decl "str!Literal" ...
        ;-{[( )]} =98 -98 +101. 43.2 , < > : _/identifier/ /filepath ."#;
        // Expect glob var tsl decl strlit semicolon dash open_curly open_brack open_paren close_paren close_bracket
        // close_curly equal number number number number , < > : identifier identifier ERROR on the last .
        let s = s.as_bytes();
        let mut l: Lexer<'_> = Lexer::from_slice(s, Line(1));
        let glob = l.next().unwrap();
        assert!(
            matches!(glob, Token::Global(..)),
            "failed to match {}",
            glob
        );
        let var = l.next().unwrap();
        assert!(
            matches!(var, Token::Variable(..)),
            "failed to match {}",
            var
        );
        let tsl = l.next().unwrap();
        assert!(
            matches!(tsl, Token::TripleStrLiteral(..)),
            "failed to match {}",
            tsl
        );
        let decl = l.next().unwrap();
        assert!(matches!(decl, Token::Decl(..)), "failed to match {}", decl);
        let strlit = l.next().unwrap();
        assert!(
            matches!(strlit, Token::StrLiteral(..)),
            "failed to match {}",
            strlit
        );
        let variadic = l.next().unwrap();
        assert!(
            matches!(variadic, Token::Variadic(..)),
            "failed to match {}",
            variadic
        );
        let semicolon = l.next().unwrap();
        assert!(
            matches!(semicolon, Token::Semicolon(..)),
            "failed to match {}",
            semicolon
        );
        let dash = l.next().unwrap();
        assert!(matches!(dash, Token::Dash(..)), "failed to match {}", dash);
        let oc = l.next().unwrap();
        assert!(matches!(oc, Token::OpenCurly(..)), "failed to match {}", oc);
        let ob = l.next().unwrap();
        assert!(
            matches!(ob, Token::OpenBracket(..)),
            "failed to match {}",
            ob
        );
        let op = l.next().unwrap();
        assert!(matches!(op, Token::OpenParen(..)), "failed to match {}", op);
        let cp = l.next().unwrap();
        assert!(
            matches!(cp, Token::CloseParen(..)),
            "failed to match {}",
            cp
        );
        let cb = l.next().unwrap();
        assert!(
            matches!(cb, Token::CloseBracket(..)),
            "failed to match {}",
            cb
        );
        let cc = l.next().unwrap();
        assert!(
            matches!(cc, Token::CloseCurly(..)),
            "failed to match {}",
            cc
        );
        let eq = l.next().unwrap();
        assert!(matches!(eq, Token::Equal(..)), "failed to match {}", eq);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let comma = l.next().unwrap();
        assert!(
            matches!(comma, Token::Comma(..)),
            "failed to match {}",
            comma
        );
        let lt = l.next().unwrap();
        assert!(matches!(lt, Token::Lt(..)), "failed to match {}", lt);
        let gt = l.next().unwrap();
        assert!(matches!(gt, Token::Gt(..)), "failed to match {}", gt);
        let colon = l.next().unwrap();
        assert!(
            matches!(colon, Token::Colon(..)),
            "failed to match {}",
            colon
        );
        let iden = l.next().unwrap();
        assert!(
            matches!(iden, Token::Identifier(..)),
            "failed to match {}",
            iden
        );
        let iden = l.next().unwrap();
        assert!(
            matches!(iden, Token::Identifier(..)),
            "failed to match {}",
            iden
        );
        let err = l.next().unwrap();
        assert!(matches!(err, Token::Error(..)), "failed to match {}", err);
    }
}
