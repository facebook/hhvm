// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::fmt;
use std::io::BufRead;
use std::io::BufReader;
use std::io::Read;
use std::rc::Rc;
use std::sync::OnceLock;

use anyhow::bail;
use anyhow::Result;
use hash::HashSet;
use log::trace;

use crate::util::unescape;

pub(crate) struct Tokenizer<'a> {
    buf: BufReader<&'a mut dyn Read>,
    filename: Rc<String>,
    cur_line: u32,
    next: Option<Token>,
    log_next: bool,
}

impl<'a> Tokenizer<'a> {
    pub fn new(read: &'a mut dyn Read, filename: &'a str) -> Self {
        Tokenizer {
            buf: BufReader::new(read),
            cur_line: 1,
            filename: Rc::new(filename.to_string()),
            log_next: true,
            next: None,
        }
    }

    fn peek_any_byte(&mut self) -> Result<Option<u8>> {
        let buf = self.buf.fill_buf()?;

        if log::log_enabled!(log::Level::Trace) && self.log_next {
            self.log_next = false;
            let pos = buf.iter().position(|c| *c == b'\n');
            let end = pos.unwrap_or(buf.len());
            let ext = if pos.is_some() { "" } else { "..." };
            let line = &buf[..end];
            trace!("INPUT: {}{ext}", String::from_utf8_lossy(line));
        }

        Ok(buf.first().copied())
    }

    fn peek_non_eol_byte(&mut self) -> Result<Option<u8>> {
        let b = self.peek_any_byte()?;
        if let Some(b) = b {
            if b == b'\n' { Ok(None) } else { Ok(Some(b)) }
        } else {
            Ok(None)
        }
    }

    fn cur_line(&self) -> LineNum {
        LineNum(self.cur_line)
    }

    fn consume_any_byte(&mut self) {
        if log::log_enabled!(log::Level::Trace) {
            if self.buf.buffer()[0] == b'\n' {
                self.log_next = true;
            }
        }
        self.buf.consume(1);
    }

    fn consume_byte(&mut self) {
        debug_assert!(self.buf.buffer()[0] != b'\n');
        self.consume_any_byte();
    }

    fn read_any_byte(&mut self) -> Result<Option<u8>> {
        let b = self.peek_any_byte()?;
        if b.is_some() {
            self.consume_any_byte();
        }
        Ok(b)
    }

    fn raw_read_token(&mut self) -> Result<Option<Token>> {
        loop {
            if let Some(first) = self.read_any_byte()? {
                let cur_loc = self.peek_loc();

                let result = match first {
                    b'\n' => {
                        self.cur_line += 1;
                        Token::Eol(cur_loc)
                    }
                    _ if first.is_ascii_whitespace() => {
                        continue;
                    }
                    b'\"' | b'\'' => return Ok(Some(self.read_quoted_string(first, cur_loc)?)),
                    b'.' | b'%' | b'#' => {
                        // A couple tokens are a little special because they could be operator-like or identifier-like.
                        // Ex. '.' could be ".srcloc", ".="
                        let c = self.peek_any_byte()?;
                        match c {
                            Some(c) if c.is_ascii_alphanumeric() => {
                                self.read_identifier(first, cur_loc)?
                            }
                            _ => self.read_operator(first, cur_loc)?,
                        }
                    }
                    b'-' if self.peek_any_byte()?.map_or(false, |c| c.is_ascii_digit()) => {
                        self.read_number(first, cur_loc)?
                    }
                    b'0' | b'1' | b'2' | b'3' | b'4' | b'5' | b'6' | b'7' | b'8' | b'9' => {
                        self.read_number(first, cur_loc)?
                    }
                    ch if operator_lead_bytes()[ch as usize] => {
                        self.read_operator(first, cur_loc)?
                    }
                    _ => self.read_identifier(first, cur_loc)?,
                };

                return Ok(Some(result));
            } else {
                return Ok(None);
            }
        }
    }

    fn fill_token(&mut self) -> Result<()> {
        if self.next.is_none() {
            self.next = self.raw_read_token()?;
        }
        Ok(())
    }

    pub fn peek_token(&mut self) -> Result<Option<&Token>> {
        self.fill_token()?;
        Ok(self.next.as_ref())
    }

    pub fn read_token(&mut self) -> Result<Option<Token>> {
        self.fill_token()?;
        Ok(self.next.take())
    }

    fn read_operator(&mut self, first: u8, cur_loc: TokenLoc) -> Result<Token> {
        // The set of characters that start our two or three-character
        // operators (so we know we need to look for an extended pattern).
        //
        // xx
        // xxx
        // ^ - The set of all characters that can appear here.
        static DOUBLE_LEAD: OnceLock<[bool; 256]> = OnceLock::new();
        let double_lead = DOUBLE_LEAD.get_or_init(|| {
            let mut lead_set = [false; 256];
            for s in operators().iter() {
                if s.len() > 1 {
                    lead_set[s.as_bytes()[0] as usize] = true;
                }
            }
            lead_set
        });

        // The set of second characters that start our three-characters
        // operators (so we know we need to look for a 3-byte pattern).
        //
        // xxx
        //  ^ - The set of all characters that can appear here.
        static TRIPLE_LEAD: OnceLock<[bool; 256]> = OnceLock::new();
        let triple_lead = TRIPLE_LEAD.get_or_init(|| {
            let mut lead_set = [false; 256];
            for s in operators().iter() {
                if s.len() > 2 {
                    lead_set[s.as_bytes()[1] as usize] = true;
                }
            }
            lead_set
        });

        if double_lead[first as usize] {
            // It might be a two or three character opcode.
            if let Some(c1) = self.peek_any_byte()? {
                let double = [first, c1];
                let double = std::str::from_utf8(&double).unwrap();
                if operators().contains(double) {
                    self.consume_byte();

                    if triple_lead[c1 as usize] {
                        // It might be a three character opcode.
                        if let Some(c2) = self.peek_any_byte()? {
                            let triple = [first, c1, c2];
                            let triple = std::str::from_utf8(&triple)?;
                            if operators().contains(triple) {
                                self.consume_byte();
                                return Ok(Token::Identifier(triple.to_owned(), cur_loc));
                            }
                        }
                    }

                    return Ok(Token::Identifier(double.to_owned(), cur_loc));
                }
            }
        }

        let single = std::str::from_utf8(std::slice::from_ref(&first))?;
        Ok(Token::Identifier(single.to_owned(), cur_loc))
    }

    fn read_number(&mut self, first: u8, cur_loc: TokenLoc) -> Result<Token> {
        let mut value = Vec::new();
        value.push(first);
        while let Some(b) = self.peek_non_eol_byte()? {
            if b.is_ascii_alphanumeric() || b == b'.' || b == b'+' || b == b'-' {
                self.consume_byte();
                value.push(b);
            } else {
                break;
            }
        }

        let value = String::from_utf8(value)?;
        Ok(Token::Identifier(value, cur_loc))
    }

    fn read_quoted_string(&mut self, first: u8, cur_loc: TokenLoc) -> Result<Token> {
        let mut value = Vec::new();
        let mut escaped = false;
        loop {
            let b = if let Some(b) = self.read_any_byte()? {
                b
            } else {
                bail!("Unexpected end of file during quoted string");
            };
            match b {
                b'\\' if !escaped => {
                    value.push(b);
                    escaped = true;
                }
                b'\n' => {
                    if !escaped {
                        value.push(b);
                    }
                    self.cur_line += 1;
                    escaped = false;
                }
                _ => {
                    if b == first && !escaped {
                        break;
                    }
                    value.push(b);
                    escaped = false;
                }
            }
        }

        let value = String::from_utf8(value)?;
        Ok(Token::QuotedString(first as char, value, cur_loc))
    }

    fn read_identifier(&mut self, first: u8, cur_loc: TokenLoc) -> Result<Token> {
        let mut value = Vec::new();
        value.push(first);
        while let Some(b) = self.peek_non_eol_byte()? {
            if b.is_ascii_alphanumeric() || b == b'$' || b == b'.' || b == b'\\' || b == b'_' {
                self.consume_byte();
                value.push(b);
            } else {
                break;
            }
        }

        let value = String::from_utf8(value)?;
        Ok(Token::Identifier(value, cur_loc))
    }
}

impl Tokenizer<'_> {
    pub fn expect_any_token(&mut self) -> Result<Token> {
        if let Some(tok) = self.read_token()? {
            Ok(tok)
        } else {
            bail!("Expected any token at {}", self.cur_line());
        }
    }

    pub fn expect_eol(&mut self) -> Result<Token> {
        let t = self.expect_any_token()?;
        if !t.is_eol() {
            bail!(
                "{}[{}] Expected end-of-line but got {t}",
                self.filename,
                self.cur_line()
            );
        }
        Ok(t)
    }

    pub fn expect_any_identifier(&mut self) -> Result<Token> {
        let t = self.expect_any_token()?;
        if !t.is_any_identifier() {
            bail!(
                "{}[{}] Expected identifier but got {t}",
                self.filename,
                self.cur_line()
            );
        }
        Ok(t)
    }

    pub fn expect_any_string(&mut self) -> Result<Token> {
        let t = self.expect_any_token()?;
        if !t.is_any_string() {
            bail!(
                "{}[{}] Expected string but got {t}",
                self.filename,
                self.cur_line()
            );
        }
        Ok(t)
    }

    pub fn expect_identifier(&mut self, expect: &str) -> Result<Token> {
        let t = self.expect_any_token()?;
        if !t.is_identifier(expect) {
            bail!(
                "{}[{}] Expected identifier '{expect}' but got {t}",
                self.filename,
                self.cur_line()
            );
        }
        Ok(t)
    }

    pub fn peek_loc(&self) -> TokenLoc {
        TokenLoc {
            filename: Rc::clone(&self.filename),
            line: self.cur_line,
        }
    }

    pub fn peek_expect_token(&mut self) -> Result<&Token> {
        let cur_line = self.cur_line();
        if let Some(tok) = self.peek_token()? {
            Ok(tok)
        } else {
            bail!("Expected any token at {}", cur_line);
        }
    }

    pub fn peek_if_any_identifier(&mut self) -> Result<Option<&Token>> {
        if let Some(t) = self.peek_token()? {
            if t.is_any_identifier() {
                return Ok(Some(t));
            }
        }

        Ok(None)
    }

    pub fn peek_if_identifier(&mut self, expect: &str) -> Result<Option<&Token>> {
        if let Some(t) = self.peek_token()? {
            if t.is_identifier(expect) {
                return Ok(Some(t));
            }
        }

        Ok(None)
    }

    pub fn peek_is_identifier(&mut self, expect: &str) -> Result<bool> {
        Ok(self.peek_if_identifier(expect)?.is_some())
    }

    pub fn peek_is_close_operator(&mut self) -> Result<bool> {
        if let Some(t) = self.peek_token()? {
            Ok(t.is_close_operator())
        } else {
            Ok(false)
        }
    }

    pub fn next_if_predicate<F>(&mut self, f: F) -> Result<Option<Token>>
    where
        F: FnOnce(&Token) -> bool,
    {
        if let Some(t) = self.peek_token()? {
            if f(t) {
                return self.read_token();
            }
        }

        Ok(None)
    }

    pub fn next_if_identifier(&mut self, expect: &str) -> Result<Option<Token>> {
        Ok(if self.peek_if_identifier(expect)?.is_some() {
            Some(self.expect_any_token()?)
        } else {
            None
        })
    }

    pub fn next_is_identifier(&mut self, expect: &str) -> Result<bool> {
        Ok(self.next_if_identifier(expect)?.is_some())
    }
}

#[derive(Debug, Clone)]
pub(crate) struct TokenLoc {
    filename: Rc<String>,
    line: u32,
}

impl TokenLoc {
    pub(crate) fn bail<'a>(&self, msg: impl Into<Cow<'a, str>>) -> anyhow::Error {
        anyhow::anyhow!("{}[{}]: {}", self.filename, self.line, msg.into())
    }
}

/// The tokenizer uses a simplified parsing model. Things are either newlines,
/// quoted strings or identifiers. There are slight differences to how
/// identifiers are parsed based on their lead characters (for example words
/// starting with a letter can contain '$' in the middle but words starting with
/// a number cannot). See read_identifier(), read_operator() and read_number()
/// for the details.
#[derive(Debug, Clone)]
pub(crate) enum Token {
    Eol(TokenLoc),
    QuotedString(char, String, TokenLoc),
    Identifier(String, TokenLoc),
}

impl Token {
    pub fn bail<'a>(&self, msg: impl Into<Cow<'a, str>>) -> anyhow::Error {
        self.loc().bail(msg)
    }

    pub fn identifier(&self) -> &str {
        self.get_identifier().unwrap()
    }

    pub fn get_identifier(&self) -> Option<&str> {
        match self {
            Token::Identifier(s, _) => Some(s),
            _ => None,
        }
    }

    pub fn is_any_identifier(&self) -> bool {
        matches!(self, Token::Identifier(..))
    }

    pub fn is_eol(&self) -> bool {
        matches!(self, Token::Eol(..))
    }

    pub fn is_identifier(&self, rhs: &str) -> bool {
        match self {
            Token::Identifier(lhs, _) => lhs == rhs,
            _ => false,
        }
    }

    pub fn is_any_string(&self) -> bool {
        matches!(self, Token::QuotedString(..))
    }

    fn is_close_operator(&self) -> bool {
        match *self {
            Token::Identifier(ref lhs, _) => lhs == "]" || lhs == ")" || lhs == ">",
            _ => false,
        }
    }

    pub fn loc(&self) -> &TokenLoc {
        match self {
            Token::Eol(loc) | Token::QuotedString(_, _, loc) | Token::Identifier(_, loc) => loc,
        }
    }

    pub fn unescaped_string(&self) -> Result<Vec<u8>> {
        match self {
            Token::QuotedString(_, value, _) => unescape(value),
            _ => bail!("String expected not {:?}", self),
        }
    }

    pub fn unescaped_identifier(&self) -> Result<Cow<'_, [u8]>> {
        match self {
            Token::Identifier(s, _) => Ok(Cow::Borrowed(s.as_bytes())),
            Token::QuotedString(..) => Ok(Cow::Owned(self.unescaped_string()?)),
            _ => bail!("Identifier expected, not {self}"),
        }
    }
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Token::Eol(_) => write!(f, "newline"),
            Token::QuotedString(delimiter, s, _) => {
                write!(f, "{delimiter}{s}{delimiter}")
            }
            Token::Identifier(s, _) => {
                write!(f, "{s}")
            }
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub(crate) struct LineNum(u32);

impl fmt::Display for LineNum {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}

/// Operators are only different from identifiers in which consecutive
/// characters make up a single token vs being split into multiple tokens.
fn operators() -> &'static HashSet<&'static str> {
    static OPERATORS: OnceLock<HashSet<&'static str>> = OnceLock::new();

    OPERATORS.get_or_init(|| {
        #[rustfmt::skip]
        let ops: HashSet<&'static str> = [
            // length 1
            "!", "#", "&", "(", ")", "*", "+", ",", "-", "/", ":", ";", "<", "=",
            ">", "?", "[", "]", "^", "{", "|", "}",
            // length 2
            "!=", "%=", "&=", "**", "*=", "..", "++", "+=", "--", "-=", "->", ".=", "/=",
            "::", "<<", "<=", "==", "=>", ">=", ">>", "?-", "^=", "|=",
            // length 3
            "!==", "**=", "...", "<<=", "<=>", "===", ">>=", "?->",
        ].into_iter().collect();

        // One weird requirement we have is that the first two bytes of any
        // triple must be a valid double. This is so we don't ever have to peek
        // more than 1 character ahead.
        for op in &ops {
            assert!(
                op.len() != 3 || ops.contains(&op[..2]),
                "Invalid triple header: {op:?}"
            );
        }

        ops
    })
}

fn operator_lead_bytes() -> &'static [bool; 256] {
    static LEAD: OnceLock<[bool; 256]> = OnceLock::new();
    LEAD.get_or_init(|| {
        let mut lead_set = [false; 256];
        for s in operators().iter() {
            lead_set[s.as_bytes()[0] as usize] = true;
        }
        lead_set
    })
}
