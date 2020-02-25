// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// Implementation of string escaping logic.
// See http://php.net/manual/en/language.types.string.php

use std::{borrow::Cow, error::Error, fmt, io::Write};

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

impl<'a> From<&'a str> for InvalidString {
    fn from(x: &'a str) -> Self {
        Self {
            msg: String::from(x),
        }
    }
}

fn is_printable(c: u8) -> bool {
    c >= b' ' && c <= b'~'
}

pub fn is_lit_printable(c: u8) -> bool {
    is_printable(c) && c != b'\\' && c != b'\"'
}

fn is_hex(c: u8) -> bool {
    (c >= b'0' && c <= b'9') || (c >= b'a' && c <= b'f') || (c >= b'A' && c <= b'F')
}

fn is_oct(c: u8) -> bool {
    c >= b'0' && c <= b'7'
}

/// This escapes a string using the format understood by the assembler
/// and php serialization. The assembler and php serialization probably
/// don't actually have the same rules but this should safely fit in both.
/// It will escape $ in octal so that it can also be used as a PHP double
/// string.
pub fn escape_char(c: u8) -> Option<Cow<'static, [u8]>> {
    match c {
        b'\n' => Some((&b"\\n"[..]).into()),
        b'\r' => Some((&b"\\r"[..]).into()),
        b'\t' => Some((&b"\\t"[..]).into()),
        b'\\' => Some((&b"\\\\"[..]).into()),
        b'"' => Some((&b"\\\""[..]).into()),
        b'$' => None,
        b'?' => Some((&b"\\?"[..]).into()),
        c if is_lit_printable(c) => None,
        c => {
            let mut r = vec![];
            write!(r, "\\{:03o}", c).unwrap();
            Some(r.into())
        }
    }
}

/// `impl Into<..>` allows escape to take a String, consider the following,
/// let a = {
///    let b = String::from("b");
///     escape(b)
/// };
///
/// Replacing `escape(b)` by `escape(&b)` leaks a reference of b to outer scope hence
/// compilation error.
pub fn escape<'a>(s: impl Into<Cow<'a, str>>) -> Cow<'a, str> {
    escape_by(s.into(), escape_char)
}

fn cow_to_bytes(s: Cow<str>) -> Cow<[u8]> {
    match s {
        Cow::Borrowed(s) => s.as_bytes().into(),
        Cow::Owned(s) => Vec::<u8>::from(s).into(),
    }
}

pub fn escape_by<F: Fn(u8) -> Option<Cow<'static, [u8]>>>(s: Cow<str>, f: F) -> Cow<str> {
    let r = escape_byte_by(cow_to_bytes(s), f);
    match r {
        Cow::Borrowed(s) => unsafe { std::str::from_utf8_unchecked(s) }.into(),
        Cow::Owned(s) => unsafe { String::from_utf8_unchecked(s) }.into(),
    }
}

fn escape_byte_by<F: Fn(u8) -> Option<Cow<'static, [u8]>>>(cow: Cow<[u8]>, f: F) -> Cow<[u8]> {
    let mut c = vec![];
    let mut copied = false;
    let s = cow.as_ref();
    for i in 0..s.len() {
        match f(s[i]) {
            None if copied => c.push(s[i]),
            Some(cc) => {
                if copied {
                    c.extend_from_slice(cc.as_ref());
                } else {
                    c.extend_from_slice(&s[..i]);
                    c.extend_from_slice(cc.as_ref());
                    copied = true;
                }
            }
            _ => {}
        }
    }
    if copied {
        c.into()
    } else {
        cow
    }
}

fn codepoint_to_utf8(n: u32, output: &mut Vec<u8>) -> Result<(), InvalidString> {
    if n <= 0x7f {
        output.push(n as u8);
    } else if n <= 0x7ff {
        output.push(0xc0 | (n >> 6) as u8);
        output.push(0x80 | (n & 0b111111) as u8);
    } else if n <= 0x00ffff {
        output.push(0xe0 | (n >> 12) as u8);
        output.push(0x80 | ((n >> 6) & 0b111111) as u8);
        output.push(0x80 | (n & 0x3f) as u8);
    } else if n <= 0x10ffff {
        output.push(0xf0 | (n >> 18) as u8);
        output.push(0x80 | ((n >> 12) & 0b111111) as u8);
        output.push(0x80 | ((n >> 6) & 0b111111) as u8);
        output.push(0x80 | (n & 0x3f) as u8);
    } else {
        return Err("UTF-8 codepoint too large".into());
    }
    Ok(())
}

fn parse_int(s: &[u8], base: u32) -> Result<u32, InvalidString> {
    // input `s` can be assumed only contains ascii digits and 'aA' - 'fF',
    // it is safe to call from_utf8 here.
    let s = match std::str::from_utf8(s) {
        Ok(s) => s,
        _ => {
            return Err("invalid numeric escape".into());
        }
    };
    let s = u32::from_str_radix(s, base);
    match s {
        Ok(v) => Ok(v),
        _ => Err("invalid numeric escape".into()),
    }
}

fn parse_numeric_escape(trim_to_byte: bool, s: &[u8], base: u32) -> Result<u8, InvalidString> {
    match parse_int(s, base) {
        Ok(v) => {
            if !trim_to_byte && (v > 255) {
                Err("Invalid UTF-8 code point.".into())
            } else {
                Ok(v as u8)
            }
        }
        Err(_) => Err("Invalid UTF-8 code point.".into()),
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
                return Err("string ended early".into());
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
                    let n = parse_int(unicode, 16)?;
                    codepoint_to_utf8(n, &mut output)?;
                    let n = s.next()?;
                    if n != b'}' {
                        return Err("Invalid UTF-8 escape sequence".into());
                    }
                }
                b'x' | b'X' => {
                    let hex = s.take_if(is_hex, 2);
                    if hex.len() == 0 {
                        output.push(b'\\');
                        output.push(c);
                    } else {
                        let c = parse_numeric_escape(false, hex, 16)?;
                        output.push(c as u8);
                    }
                }
                c if is_oct(c) => {
                    s.back();
                    let oct = s.take_if(is_oct, 3);
                    let c = parse_numeric_escape(true, oct, 8)?;
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

fn unescape_single_or_nowdoc(is_nowdoc: bool, s: &[u8]) -> Result<String, InvalidString> {
    let len = s.len();
    let mut output: Vec<u8> = Vec::with_capacity(len);
    let mut idx = 0;
    while idx < len {
        let c = s[idx];
        if is_nowdoc || c != b'\\' {
            output.push(c)
        } else {
            idx += 1;
            if !idx < len {
                return Err("string ended early".into());
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
    unescape_single_or_nowdoc(false, s.as_bytes())
}

pub fn unescape_nowdoc(s: &str) -> Result<String, InvalidString> {
    unescape_single_or_nowdoc(true, s.as_bytes())
}

pub fn unescape_long_string(s: &str) -> Result<String, InvalidString> {
    unescape_literal(LiteralKind::LiteralLongString, s)
}

pub fn extract_unquoted_string(
    content: &str,
    start: usize,
    len: usize,
) -> Result<String, InvalidString> {
    let r = extract_unquoted_string_(content.as_bytes(), start, len)?;
    unsafe { Ok(String::from_utf8_unchecked(r)) }
}

fn find(s: &[u8], needle: u8) -> Option<usize> {
    for i in 0..s.len() {
        if s[i] == needle {
            return Some(i);
        }
    }
    None
}

fn rfind(s: &[u8], needle: u8) -> Option<usize> {
    let mut i = s.len();
    while i > 0 {
        i -= 1;
        if s[i] == needle {
            return Some(i);
        }
    }
    None
}

fn extract_unquoted_string_(
    content: &[u8],
    start: usize,
    len: usize,
) -> Result<Vec<u8>, InvalidString> {
    if len == 0 {
        Ok(vec![])
    } else if content.len() > 3 && content.starts_with(b"<<<") {
        // The heredoc case
        // These types of strings begin with an opening line containing <<<
        // followed by a string to use as a terminator (which is optionally
        // quoted) and end with a line containing only the terminator and a
        // semicolon followed by a blank line. We need to drop the opening line
        // as well as the blank line and preceding terminator line.
        match (
            find(content, b'\n'),
            rfind(&content[..start + len - 1], b'\n'),
        ) {
            (Some(start_), Some(end_)) =>
            // An empty heredoc, this way, will have start >= end
            {
                if start_ >= end_ {
                    Ok(vec![])
                } else {
                    Ok(content[start_ + 1..end_].into())
                }
            }
            _ => Ok(content.into()),
        }
    } else {
        static SINGLE_QUOTE: u8 = b'\'';
        static DOUBLE_QUOTE: u8 = b'"';
        static BACK_TICK: u8 = b'`';
        match (content.get(start), content.get(start + len - 1)) {
            (Some(&c1), Some(&c2))
                if (c1 == DOUBLE_QUOTE && c2 == DOUBLE_QUOTE)
                    || c1 == SINGLE_QUOTE && c2 == SINGLE_QUOTE
                    || c1 == BACK_TICK && c2 == BACK_TICK =>
            {
                Ok(content[start + 1..len - 1].into())
            }
            (Some(_), Some(_)) => {
                if start == 0 && content.len() == len {
                    Ok(content.into())
                } else {
                    Ok(content[start..start + len].into())
                }
            }
            _ => Err("out of bounds".into()),
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

        let invalid = r#"\u{D800}\u{DF1E}"#;
        assert_eq!(
            unescape_double(invalid).unwrap().as_bytes(),
            &[237u8, 160u8, 128u8, 237u8, 188u8, 158u8]
        );
    }

    #[test]
    fn parse_int_test() {
        assert_eq!(parse_int(b"2", 10).unwrap(), 2);
        assert!(parse_int(b"h", 10).is_err());
        assert_eq!(parse_int(b"12", 8).unwrap(), 10);
        assert_eq!(parse_int(b"b1", 16).unwrap(), 177)
    }

    #[test]
    fn escape_char_test() {
        let escape_char_ = |c: u8| -> String {
            let r = escape_char(c).unwrap_or(vec![c].into()).into_owned();
            unsafe { String::from_utf8_unchecked(r) }
        };

        assert_eq!(escape_char_(b'a'), "a");
        assert_eq!(escape_char_(b'$'), "$");
        assert_eq!(escape_char_(b'\"'), "\\\"");
        assert_eq!(escape_char_(0), "\\000");
        assert_eq!(escape("house"), "house");
        assert_eq!(escape("\n"), "\\n");
        assert_eq!(escape("red\n\t\r$?"), "red\\n\\t\\r$\\?");
        assert_eq!(is_oct(b'5'), true);
        assert_eq!(is_oct(b'a'), false);
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

    #[test]
    fn rfind_test() {
        assert_eq!(rfind(b"", b'a'), None);
        assert_eq!(rfind(b"a", b'a'), Some(0));
        assert_eq!(rfind(b"b", b'a'), None);
        assert_eq!(rfind(b"ba", b'a'), Some(1));
    }
}
