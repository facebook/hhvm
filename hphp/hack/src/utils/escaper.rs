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
    let len = s.len();
    let mut output: Vec<u8> = Vec::with_capacity(len);
    let mut idx = 0;
    let next = |idx: usize| -> Result<(char, usize), InvalidString> {
        if idx >= len {
            Err(InvalidString {
                msg: String::from("string ended early"),
            })
        } else {
            let c = s.as_bytes()[idx] as char;
            Ok((c, idx + 1))
        }
    };
    // Count how many characters, starting at the current string index.
    // Will always stop at i=max.
    fn count_f(
        f: impl Fn(char) -> bool,
        max: Option<usize>,
        mut i: usize,
        idx: usize,
        len: usize,
        s: &str,
    ) -> usize {
        let max = match max {
            None => len,
            Some(x) => x,
        };
        while i < max && idx + i < len && (f(s.as_bytes()[idx + i] as char)) {
            i += 1
        }
        i
    };
    let push = |s: &mut Vec<u8>, c: char| s.push(c as u8);
    let push_str = |s: &mut Vec<u8>, st: &str| s.extend_from_slice(st.as_bytes());
    while {
        let (c, new_idx) = next(idx)?;
        idx = new_idx;
        if c != '\\' || idx == len {
            push(&mut output, c)
        } else {
            let (c, new_idx) = next(idx)?;
            idx = new_idx;
            match c {
                'a' if literal_kind == LiteralKind::LiteralLongString => push(&mut output, '\x07'),
                'b' if literal_kind == LiteralKind::LiteralLongString => push(&mut output, '\x08'),
                '\'' => push_str(&mut output, "\\\'"),
                'n' => {
                    if literal_kind != LiteralKind::LiteralLongString {
                        push(&mut output, '\n')
                    }
                }
                'r' => {
                    if literal_kind != LiteralKind::LiteralLongString {
                        push(&mut output, '\r')
                    }
                }
                't' => push(&mut output, '\t'),
                'v' => push(&mut output, '\x0b'),
                'e' => push(&mut output, '\x1b'),
                'f' => push(&mut output, '\x0c'),
                '\\' => push(&mut output, '\\'),
                '?' if literal_kind == LiteralKind::LiteralLongString => push(&mut output, '\x3f'),
                '$' if literal_kind != LiteralKind::LiteralLongString => push(&mut output, '$'),
                '`' if literal_kind != LiteralKind::LiteralLongString => {
                    if literal_kind == LiteralKind::LiteralBacktick {
                        push(&mut output, '`')
                    } else {
                        push_str(&mut output, "\\'")
                    }
                }
                '\"' => {
                    if literal_kind == LiteralKind::LiteralDoubleQuote
                        || literal_kind == LiteralKind::LiteralLongString
                    {
                        push(&mut output, '\"')
                    } else {
                        push_str(&mut output, "\\\"")
                    }
                }
                'u' if literal_kind != LiteralKind::LiteralLongString
                    && idx < len
                    && s.as_bytes()[idx] as char == '{' =>
                {
                    let (_, new_idx) = next(idx)?;
                    idx = new_idx;
                    let unicode_count = count_f(&|c: char| c != '}', Some(6), 0, idx, len, s);
                    let n = parse_int(&s[idx..(unicode_count + idx)], 16)?;
                    codepoint_to_utf8(n, &mut output)?;
                    idx += unicode_count;
                    let (n, new_idx) = next(idx)?;
                    idx = new_idx;
                    if n != '}' {
                        return Err(InvalidString {
                            msg: String::from("Invalid UTF-8 escape sequence"),
                        });
                    }
                }
                'x' | 'X' => {
                    let hex_count = count_f(&is_hex, Some(2), 0, idx, len, s);
                    if hex_count == 0 {
                        push(&mut output, '\\');
                        push(&mut output, c)
                    } else {
                        let c = parse_numeric_escape(false, &s[idx..(hex_count + idx)], 16)?;
                        push(&mut output, c);
                        idx += hex_count
                    }
                }
                c if is_oct(c) => {
                    idx -= 1;
                    let oct_count = count_f(&is_oct, Some(3), 0, idx, len, s);
                    let c = parse_numeric_escape(true, &s[idx..(oct_count + idx)], 8)?;
                    push(&mut output, c);
                    idx += oct_count
                }
                c => {
                    push(&mut output, '\\');
                    push(&mut output, c)
                }
            }
        }
        idx < len
    } {}
    unsafe { Ok(String::from_utf8_unchecked(output)) }
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
    let len = s.len();
    let mut output: Vec<u8> = Vec::with_capacity(len);
    let mut idx = 0;
    let next = |idx: usize| -> Result<(char, usize), InvalidString> {
        if idx >= len {
            Err(InvalidString {
                msg: String::from("string ended early"),
            })
        } else {
            let c = s.as_bytes()[idx] as char;
            Ok((c, idx + 1))
        }
    };
    while {
        let (c, new_idx) = next(idx)?;
        idx = new_idx;
        if is_nowdoc || c != '\\' {
            output.push(c as u8)
        } else {
            let (c, new_idx) = next(idx)?;
            idx = new_idx;
            match c {
                '\'' => output.push('\'' as u8),
                '\\' => output.push('\\' as u8),
                // unrecognized escapes are just copied over
                _ => {
                    output.push('\\' as u8);
                    output.push(c as u8)
                }
            }
        }
        idx < len
    } {}
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
        match (content.find('\n'), content.rfind('\n')) {
            (Some(start_), Some(end_)) =>
            // An empty heredoc, this way, will have start >= end
            {
                if start_ >= end_ {
                    Ok("".to_string())
                } else {
                    Ok(content[start_ + 1..end_].to_string())
                }
            }
            _ => Err(InvalidString {
                msg: String::from("out of bounds"),
            }),
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
        assert_eq!(
            unescape_single("home \\\\$").unwrap(),
            "home \\$".to_string()
        );
        assert_eq!(unescape_nowdoc("home \\$").unwrap(), "home \\$".to_string());
        assert_eq!(unescape_single("home \\'").unwrap(), "home '".to_string());
        assert_eq!(unescape_nowdoc("home \\'").unwrap(), "home \\'".to_string());
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
    }

}
