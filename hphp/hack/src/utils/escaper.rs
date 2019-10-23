// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// Implementation of string escaping logic.
// See http://php.net/manual/en/language.types.string.php

use std::char;
use std::error::Error;
use std::fmt;

#[derive(Debug)]
pub struct InvalidString {
    pub msg: String,
}

impl fmt::Display for InvalidString {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.msg)
    }
}

impl Error for InvalidString {
    fn description(&self) -> &str {
        &self.msg
    }
}

fn is_printable(c: char) -> bool {
    c >= ' ' && c <= '~'
}

pub fn is_lit_printable(c: char) -> bool {
    is_printable(c) && c != '\\' && c != '\"'
}

fn is_hex(c: char) -> bool {
    (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
}

fn is_oct(c: char) -> bool {
    c >= '0' && c <= '7'
}

// This escapes a string using the format understood by the assembler
// and php serialization. The assembler and php serialization probably
// don't actually have the same rules but this should safely fit in both.
// It will escape $ in octal so that it can also be used as a PHP double
// string.

pub fn escape_char<'a>(c: char, output: &mut Vec<u8>) {
    match c {
        '\n' => output.extend_from_slice("\\n".as_bytes()),
        '\r' => output.extend_from_slice("\\r".as_bytes()),
        '\t' => output.extend_from_slice("\\t".as_bytes()),
        '\\' => output.extend_from_slice("\\\\".as_bytes()),
        '"' => output.extend_from_slice("\\\"".as_bytes()),
        '$' => output.extend_from_slice("$".as_bytes()),
        '?' => output.extend_from_slice("\\?".as_bytes()),
        c if is_lit_printable(c) => output.push(c as u8),
        c => output.extend_from_slice(format!("\\{:o}", c as u8).as_bytes()),
    }
}

pub fn escape(s: &str) -> String {
    let mut output: Vec<u8> = Vec::with_capacity(s.len());
    for c in s.chars() {
        escape_char(c, &mut output);
    }
    unsafe { String::from_utf8_unchecked(output) }
}

fn codepoint_to_utf8(n: u32, output: &mut Vec<u8>) -> Result<(), InvalidString> {
    match std::char::from_u32(n) {
        None => Err(InvalidString {
            msg: String::from("UTF-8 codepoint too large"),
        }),
        Some(v) => {
            match v.len_utf8() {
                1 => output.push(v as u8),
                _ => output.extend_from_slice(v.encode_utf8(&mut [0; 4]).as_bytes()),
            }
            Ok(())
        }
    }
}

fn parse_int(s: &str, base: u32) -> Result<u32, InvalidString> {
    let s = u32::from_str_radix(s, base);
    match s {
        Ok(v) => Ok(v),
        _ => Err(InvalidString {
            msg: String::from("invalid numeric escape"),
        }),
    }
}

fn parse_numeric_escape(trim_to_byte: bool, s: &str, base: u32) -> Result<char, InvalidString> {
    match parse_int(s, base) {
        Ok(v) => {
            if trim_to_byte {
                match char::from_u32(v & 0xFF) {
                    Some(v) => Ok(v),
                    None => Err(InvalidString {
                        msg: String::from("Invalid UTF-8 code point."),
                    }),
                }
            } else {
                match char::from_u32(v) {
                    Some(v) => Ok(v),
                    None => Err(InvalidString {
                        msg: String::from("Invalid UTF-8 code point."),
                    }),
                }
            }
        }
        Err(_) => Err(InvalidString {
            msg: String::from("Invalid UTF-8 code point."),
        }),
    }
}

#[derive(PartialEq)]
pub enum LiteralKind {
    LiteralHeredoc,
    LiteralDoubleQuote,
    LiteralBacktick,
    LiteralLongString,
}

fn unescape_literal(literal_kind: LiteralKind, s: &str) -> Result<String, InvalidString> {
    struct Scanner<'a> {
        s: &'a [u8],
        i: usize,
    }
    impl<'a> Scanner<'a> {
        fn new(s: &'a [u8]) -> Self {
            Self { s, i: 0 }
        }
        fn is_empty(&self) -> bool {
            self.i >= self.s.len()
        }
        fn next(&mut self) -> Result<u8, InvalidString> {
            if self.i >= self.s.len() {
                return Err(InvalidString {
                    msg: String::from("string ended early"),
                });
            }
            let r = self.s[self.i];
            self.i += 1;
            Ok(r)
        }
        fn take_if(&mut self, f: impl Fn(u8) -> bool, size: usize) -> &'a [u8] {
            let l = usize::min(size + self.i, self.s.len());
            let mut c = self.i;
            while c < l && f(self.s[c]) {
                c += 1;
            }
            let r = &self.s[self.i..c];
            self.i = c;
            r
        }
        fn peek(&self) -> Option<u8> {
            if self.i < self.s.len() {
                Some(self.s[self.i])
            } else {
                None
            }
        }
        fn back(&mut self) {
            if self.i > 0 {
                self.i -= 1;
            }
        }
    }

    let mut output: Vec<u8> = Vec::with_capacity(s.len());

    let mut s = Scanner::new(s.as_bytes());
    while !s.is_empty() {
        let c = s.next()?;
        if c != b'\\' || s.is_empty() {
            output.push(c);
        } else {
            let c = s.next()?;
            match c {
                b'a' if literal_kind == LiteralKind::LiteralLongString => output.push(b'\x07'),
                b'b' if literal_kind == LiteralKind::LiteralLongString => output.push(b'\x08'),
                b'\'' => output.extend_from_slice(b"\\\'"),
                b'n' => match literal_kind {
                    LiteralKind::LiteralLongString => (),
                    _ => output.push(b'\n'),
                },
                b'r' => match literal_kind {
                    LiteralKind::LiteralLongString => (),
                    _ => output.push(b'\r'),
                },
                b't' => output.push(b'\t'),
                b'v' => output.push(b'\x0b'),
                b'e' => output.push(b'\x1b'),
                b'f' => output.push(b'\x0c'),
                b'\\' => output.push(b'\\'),
                b'?' if literal_kind == LiteralKind::LiteralLongString => output.push(b'\x3f'),
                b'$' if literal_kind != LiteralKind::LiteralLongString => output.push(b'$'),
                b'`' if literal_kind != LiteralKind::LiteralLongString => match literal_kind {
                    LiteralKind::LiteralBacktick => output.push(b'`'),
                    _ => output.extend_from_slice(b"\\`"),
                },
                b'\"' => match literal_kind {
                    LiteralKind::LiteralDoubleQuote | LiteralKind::LiteralLongString => {
                        output.push(b'\"')
                    }
                    _ => output.extend_from_slice(b"\\\""),
                },
                b'u' if literal_kind != LiteralKind::LiteralLongString
                    && s.peek() == Some(b'{') =>
                {
                    let _ = s.next()?;
                    let unicode = s.take_if(|c| c != b'}', 6);
                    let n = parse_int(unsafe { std::str::from_utf8_unchecked(unicode) }, 16)?;
                    codepoint_to_utf8(n, &mut output)?;
                    let n = s.next()?;
                    if n != b'}' {
                        return Err(InvalidString {
                            msg: String::from("Invalid UTF-8 escape sequence"),
                        });
                    }
                }
                b'x' | b'X' => {
                    let hex = s.take_if(|c| is_hex(c as char), 2);
                    if hex.len() == 0 {
                        output.push(b'\\');
                        output.push(c);
                    } else {
                        // TODO: change parse_numeric_escape to return u8
                        let c = parse_numeric_escape(
                            false,
                            unsafe { std::str::from_utf8_unchecked(hex) },
                            16,
                        )?;
                        output.push(c as u8);
                    }
                }
                c if is_oct(c as char) => {
                    s.back();
                    let oct = s.take_if(|c| is_oct(c as char), 3);
                    let c = parse_numeric_escape(
                        true,
                        unsafe { std::str::from_utf8_unchecked(oct) },
                        8,
                    )?;
                    output.push(c as u8);
                }
                c => {
                    output.push(b'\\');
                    output.push(c);
                }
            }
        }
    }
    return Ok(unsafe { String::from_utf8_unchecked(output) });
}

pub fn unescape_double(s: &str) -> Result<String, InvalidString> {
    unescape_literal(LiteralKind::LiteralDoubleQuote, s)
}

pub fn unescape_backtick(s: &str) -> Result<String, InvalidString> {
    unescape_literal(LiteralKind::LiteralBacktick, s)
}

pub fn unescape_heredoc(s: &str) -> Result<String, InvalidString> {
    unescape_literal(LiteralKind::LiteralHeredoc, s)
}

fn unescape_single_or_nowdoc(is_nowdoc: bool, s: &str) -> Result<String, InvalidString> {
    if s.is_empty() {
        return Ok(String::new());
    }
    let len = s.len();
    let mut output: Vec<u8> = Vec::with_capacity(len);
    let mut idx = 0;
    let s = s.as_bytes();
    while idx < len {
        let c = s[idx];
        if is_nowdoc || c != b'\\' {
            output.push(c)
        } else {
            idx += 1;
            if !idx < len {
                return Err(InvalidString {
                    msg: String::from("string ended early"),
                });
            }
            let c = s[idx];
            match c {
                b'\'' | b'\\' => output.push(c),
                // unrecognized escapes are just copied over
                _ => {
                    output.push(b'\\');
                    output.push(c);
                }
            }
        }
        idx += 1;
    }
    unsafe { Ok(String::from_utf8_unchecked(output)) }
}

pub fn unescape_single(s: &str) -> Result<String, InvalidString> {
    unescape_single_or_nowdoc(false, s)
}

pub fn unescape_nowdoc(s: &str) -> Result<String, InvalidString> {
    unescape_single_or_nowdoc(true, s)
}

pub fn unescape_long_string(s: &str) -> Result<String, InvalidString> {
    unescape_literal(LiteralKind::LiteralLongString, s)
}

pub fn extract_unquoted_string(
    content: &str,
    start: usize,
    len: usize,
) -> Result<String, InvalidString> {
    if len == 0 {
        Ok("".to_string())
    } else if content.len() > 3 && content.starts_with("<<<") {
        // The heredoc case
        // These types of strings begin with an opening line containing <<<
        // followed by a string to use as a terminator (which is optionally
        // quoted) and end with a line containing only the terminator and a
        // semicolon followed by a blank line. We need to drop the opening line
        // as well as the blank line and preceding terminator line.
        match (content.find('\n'), content[..start + len - 1].rfind('\n')) {
            (Some(start_), Some(end_)) =>
            // An empty heredoc, this way, will have start >= end
            {
                if start_ >= end_ {
                    Ok("".to_string())
                } else {
                    Ok(content[start_ + 1..end_].to_string())
                }
            }
            _ => Ok(String::from(content)),
        }
    } else {
        static SINGLE_QUOTE: u8 = '\'' as u8;
        static DOUBLE_QUOTE: u8 = '"' as u8;
        static BACK_TICK: u8 = '`' as u8;
        match (
            content.as_bytes().get(start),
            content.as_bytes().get(start + len - 1),
        ) {
            (Some(&c1), Some(&c2))
                if (c1 == DOUBLE_QUOTE && c2 == DOUBLE_QUOTE)
                    || c1 == SINGLE_QUOTE && c2 == SINGLE_QUOTE
                    || c1 == BACK_TICK && c2 == BACK_TICK =>
            {
                Ok(content[start + 1..len - 1].to_string())
            }
            (Some(_), Some(_)) => {
                if start == 0 && content.len() == len {
                    Ok(content.to_string())
                } else {
                    Ok(content[start..start + len].to_string())
                }
            }
            _ => Err(InvalidString {
                msg: String::from("out of bounds"),
            }),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq; // make assert_eq print huge diffs more human-readable

    #[test]
    fn unescape_single_or_nowdoc() {
        assert_eq!(unescape_single("").unwrap(), "");
        assert_eq!(unescape_nowdoc("").unwrap(), "");
        assert_eq!(unescape_long_string("").unwrap(), "");
        assert_eq!(unescape_double("").unwrap(), "");
        assert_eq!(unescape_backtick("").unwrap(), "");
        assert_eq!(unescape_heredoc("").unwrap(), "");

        assert_eq!(
            unescape_single("home \\\\$").unwrap(),
            "home \\$".to_string()
        );
        assert_eq!(unescape_nowdoc("home \\$").unwrap(), "home \\$".to_string());
        assert_eq!(unescape_single("home \\'").unwrap(), "home '".to_string());
        assert_eq!(unescape_nowdoc("home \\'").unwrap(), "home \\'".to_string());
        assert_eq!(unescape_nowdoc("\\`").unwrap(), "\\`");
        assert_eq!(unescape_single("\\a\\\'").unwrap(), "\\a'");
        assert_eq!(unescape_long_string("\\a").unwrap(), "\x07");
        assert_eq!(unescape_long_string("\\v").unwrap(), "\x0b");
        assert_eq!(unescape_long_string("\\\'").unwrap(), "\\\'");
        assert_eq!(unescape_long_string("\\\\").unwrap(), "\\");
        assert_eq!(unescape_long_string("?").unwrap(), "\x3f");
        assert_eq!(unescape_long_string("$").unwrap(), "$");

        assert_eq!(unescape_long_string("\\b").unwrap(), "\x08");
        assert_eq!(unescape_long_string("\\e").unwrap(), "\x1b");
        assert_eq!(unescape_long_string("\\f").unwrap(), "\x0c");
        assert_eq!(unescape_long_string("\\\"").unwrap(), "\"");
        assert_eq!(unescape_long_string("\\`").unwrap(), "\\`");
        assert_eq!(unescape_heredoc("\\\"").unwrap(), "\\\"");
        assert_eq!(unescape_heredoc("\\p").unwrap(), "\\p");
        assert_eq!(unescape_long_string("\\r").unwrap(), "");
        assert_eq!(unescape_double("\\u{b1}").unwrap(), "±");

        assert_eq!(unescape_double("\\x27\\x22").unwrap(), "\'\"");
        assert_eq!(unescape_double("\\X27\\X22").unwrap(), "\'\"");
        assert_eq!(
            unescape_double("\\141\\156\\143\\150\\157\\162").unwrap(),
            "anchor"
        );
        assert_eq!(
            unescape_backtick("\\a\\b\\n\\r\\t").unwrap(),
            "\\a\\b\n\r\t".to_string()
        );
        assert_eq!(unescape_long_string("\\xb1").unwrap().as_bytes(), &[177u8]);

        let euro = "\u{20AC}"; // as bytes [226, 130, 172]
        assert_eq!(
            unescape_long_string(euro).unwrap().as_bytes(),
            &[226u8, 130u8, 172u8]
        );
        assert_eq!(unescape_backtick("\\`").unwrap(), "`");
        assert_eq!(unescape_long_string("\\xb1").unwrap().as_bytes(), &[177u8]);

        let euro = "\u{20AC}"; // as bytes [226, 130, 172]
        assert_eq!(
            unescape_long_string(euro).unwrap().as_bytes(),
            &[226u8, 130u8, 172u8]
        );
    }

    #[test]
    fn parse_int_test() {
        assert_eq!(parse_int("2", 10).unwrap(), 2);
        assert!(parse_int("h", 10).is_err());
        assert_eq!(parse_int("12", 8).unwrap(), 10);
        assert_eq!(parse_int("b1", 16).unwrap(), 177)
    }

    #[test]
    fn escape_char_test() {
        let escape_char_ = |c: char| -> String {
            let mut s = vec![];
            escape_char(c, &mut s);
            unsafe { String::from_utf8_unchecked(s) }
        };

        assert_eq!(escape_char_('a'), "a");
        assert_eq!(escape_char_('$'), "$");
        assert_eq!(escape_char_('\"'), "\\\"");
        assert_eq!(escape("house"), "house");
        assert_eq!(escape("red\n\t\r$?"), "red\\n\\t\\r$\\?");
        assert_eq!(is_oct('5'), true);
        assert_eq!(is_oct('a'), false);
    }

    #[test]
    fn extract_unquoted_string_test() {
        assert_eq!(extract_unquoted_string("'a'", 0, 3).unwrap(), "a");
        assert_eq!(extract_unquoted_string("\"a\"", 0, 3).unwrap(), "a");
        assert_eq!(extract_unquoted_string("`a`", 0, 3).unwrap(), "a");
        assert_eq!(extract_unquoted_string("", 0, 0).unwrap(), "");
        assert_eq!(extract_unquoted_string("''", 0, 2).unwrap(), "");
        assert_eq!(extract_unquoted_string("'a", 0, 2).unwrap(), "'a");
        assert_eq!(extract_unquoted_string("a", 0, 1).unwrap(), "a");
        assert_eq!(
            extract_unquoted_string("<<<EOT\n\nEOT;", 0, 12).unwrap(),
            ""
        );
        assert_eq!(
            extract_unquoted_string("<<<EOT\na\nEOT;", 0, 13).unwrap(),
            "a"
        );
        assert_eq!(
            extract_unquoted_string("<<<EOT\n\nEOT;\n", 0, 13).unwrap(),
            ""
        );
        assert_eq!(
            extract_unquoted_string("<<<EOT\na\nEOT;\n", 0, 14).unwrap(),
            "a"
        );
    }

}
