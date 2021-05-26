// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum IntKind {
    Dec,
    Hex,
    Oct,
    Bin,
}

impl IntKind {
    fn to_base(self) -> u8 {
        match self {
            Self::Bin => 2,
            Self::Oct => 8,
            Self::Dec => 10,
            Self::Hex => 16,
        }
    }
}

impl std::fmt::Display for IntKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        f.write_str(match self {
            Self::Dec => "decimal",
            Self::Hex => "hexadecimal",
            Self::Oct => "octal",
            Self::Bin => "binary",
        })
    }
}

#[derive(Debug, Eq, PartialEq)]
pub enum ParseIntError {
    /// Valid digits are
    /// - dec [0-9],
    /// - hex [0-9a-fA-F],
    /// - oct [0-7]
    /// - bin [0|1]
    InvalidDigit(IntKind),
    OutOfRange,
    Empty,
}

// Port from https://github.com/ocaml/ocaml/blob/6efe8fea5b6c3f1db22e50e8b164d6ffec85578d/runtime/ints.c
fn parse_sign_and_base(p: &[u8], preceding_zero_as_oct: bool) -> (i8, IntKind, &[u8]) {
    let mut sign = 1i8;
    if p.is_empty() {
        return (sign, IntKind::Dec, p);
    }
    let mut i = 0;
    if p[i] == b'-' {
        sign = -1;
        i += 1;
    } else if p[i] == b'+' {
        i += 1;
    }
    if p.len() > i + 1 && p[i] == b'0' {
        match p[i + 1] {
            b'x' | b'X' => (sign, IntKind::Hex, &p[i + 2..]),
            b'o' | b'O' => (sign, IntKind::Oct, &p[i + 2..]),
            b'b' | b'B' => (sign, IntKind::Bin, &p[i + 2..]),
            _ => {
                if preceding_zero_as_oct {
                    (sign, IntKind::Oct, &p[i + 1..])
                } else {
                    (sign, IntKind::Dec, &p[i..])
                }
            }
        }
    } else {
        (sign, IntKind::Dec, &p[i..])
    }
}

fn parse_digit(c: u8, int_kind: IntKind) -> Result<u8, ParseIntError> {
    if c >= b'0' && c <= b'9' {
        Ok(c - b'0')
    } else if c >= b'A' && c <= b'F' {
        Ok(c - b'A' + 10)
    } else if c >= b'a' && c <= b'f' {
        Ok(c - b'a' + 10)
    } else {
        Err(ParseIntError::InvalidDigit(int_kind))
    }
}

pub fn parse_int(p: impl AsRef<[u8]>) -> Result<i64, ParseIntError> {
    parse(p.as_ref(), true)
}

pub fn int_of_string_opt(p: impl AsRef<[u8]>) -> Option<i64> {
    parse(p.as_ref(), false).ok()
}

fn parse(p: &[u8], preceding_zero_as_oct: bool) -> Result<i64, ParseIntError> {
    let (sign, int_kind, p) = parse_sign_and_base(p, preceding_zero_as_oct);
    let base = int_kind.to_base();
    if p.is_empty() {
        return Err(ParseIntError::Empty);
    }
    let mut r = 0i64;
    for i in 0..p.len() {
        if i != 0 && p[i] == b'_' {
            continue;
        }
        let d = parse_digit(p[i], int_kind)?;
        if d >= base {
            return Err(ParseIntError::InvalidDigit(int_kind));
        } else {
            r = i64::checked_mul(r, base as i64).ok_or(ParseIntError::OutOfRange)?;
            if sign >= 0 {
                r = i64::checked_add(r, d as i64).ok_or(ParseIntError::OutOfRange)?;
            } else {
                r = i64::checked_sub(r, d as i64).ok_or(ParseIntError::OutOfRange)?;
            }
        }
    }
    Ok(r)
}

pub fn int_of_string_wrap(p: &[u8]) -> Result<i64, ParseIntError> {
    let (sign, int_kind, p) = parse_sign_and_base(p, false);
    let base = int_kind.to_base();
    if p.is_empty() {
        return Err(ParseIntError::Empty);
    }
    let mut r = 0i64;
    for i in 0..p.len() {
        if i != 0 && p[i] == b'_' {
            continue;
        }
        let d = parse_digit(p[i], int_kind)?;
        if d >= base {
            return Err(ParseIntError::InvalidDigit(int_kind));
        } else {
            r = i64::overflowing_mul(r, base as i64).0;
            if sign >= 0 {
                r = i64::overflowing_add(r, d as i64).0;
            } else {
                r = i64::overflowing_sub(r, d as i64).0;
            }
        }
    }
    Ok(r)
}

pub fn int_of_str_opt(s: impl AsRef<str>) -> Option<i64> {
    int_of_string_opt(s.as_ref().as_bytes())
}

/// ported from supercaml/share/dotopam/default/lib/ocaml/bytes.ml
/// github link: https://github.com/ocaml/ocaml/blob/4.09.1/stdlib/bytes.ml#L170-L208
pub fn escaped(s: &str) -> Cow<str> {
    let mut n: usize = 0;
    for i in s.as_bytes() {
        n += match i {
            b'"' | b'\\' | b'\n' | b'\t' | b'\r' | 8 => 2,
            b' '..=b'~' => 1,
            _ => 4,
        };
    }
    if n == s.len() {
        s.into()
    } else {
        let mut es: Vec<u8> = Vec::with_capacity(n);
        n = 0;
        for i in s.as_bytes() {
            match i {
                b'"' | b'\\' => {
                    es.push(b'\\');
                    n += 1;
                    es.push(*i);
                }
                b'\n' => {
                    es.push(b'\\');
                    n += 1;
                    es.push(b'n');
                }
                b'\t' => {
                    es.push(b'\\');
                    n += 1;
                    es.push(b't');
                }
                b'\r' => {
                    es.push(b'\\');
                    n += 1;
                    es.push(b'r');
                }
                8 => {
                    es.push(b'\\');
                    n += 1;
                    es.push(b'b');
                }
                x @ b' '..=b'~' => es.push(*x),
                _ => {
                    es.push(b'\\');
                    n += 1;
                    es.push(48 + i / 100);
                    n += 1;
                    es.push(48 + (i / 10) % 10);
                    n += 1;
                    es.push(48 + i % 10);
                }
            }
            n += 1;
        }
        unsafe { String::from_utf8_unchecked(es) }.into()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn parse_digit_tests() {
        use parse_digit as f;
        assert_eq!(f(b'0', IntKind::Dec), Ok(0));
        assert_eq!(f(b'1', IntKind::Dec), Ok(1));
        assert_eq!(f(b'9', IntKind::Dec), Ok(9));
        assert_eq!(f(b'A', IntKind::Hex), Ok(10));
        assert_eq!(f(b'a', IntKind::Hex), Ok(10));
        assert_eq!(f(b'F', IntKind::Hex), Ok(15));
        assert_eq!(f(b'f', IntKind::Hex), Ok(15));

        assert_eq!(
            f(b'-', IntKind::Dec),
            Err(ParseIntError::InvalidDigit(IntKind::Dec))
        );
        assert_eq!(
            f(b'z', IntKind::Dec),
            Err(ParseIntError::InvalidDigit(IntKind::Dec))
        );
    }

    #[test]
    fn parse_sign_and_base_tests() {
        let f = |s| parse_sign_and_base(s, false);
        assert_eq!(f(b""), (1, IntKind::Dec, &b""[..]));
        assert_eq!(f(b"-"), (-1, IntKind::Dec, &b""[..]));
        assert_eq!(f(b"+"), (1, IntKind::Dec, &b""[..]));
        assert_eq!(f(b"z"), (1, IntKind::Dec, &b"z"[..]));
        assert_eq!(f(b"0"), (1, IntKind::Dec, &b"0"[..]));

        assert_eq!(f(b"0x"), (1, IntKind::Hex, &b""[..]));
        assert_eq!(f(b"0X"), (1, IntKind::Hex, &b""[..]));

        assert_eq!(f(b"0o"), (1, IntKind::Oct, &b""[..]));
        assert_eq!(f(b"0O"), (1, IntKind::Oct, &b""[..]));

        assert_eq!(f(b"0b"), (1, IntKind::Bin, &b""[..]));
        assert_eq!(f(b"0B"), (1, IntKind::Bin, &b""[..]));

        assert_eq!(f(b"-1"), (-1, IntKind::Dec, &b"1"[..]));
        assert_eq!(f(b"+1"), (1, IntKind::Dec, &b"1"[..]));

        assert_eq!(f(b"-x"), (-1, IntKind::Dec, &b"x"[..]));
        assert_eq!(f(b"+B"), (1, IntKind::Dec, &b"B"[..]));

        assert_eq!(f(b"-O1"), (-1, IntKind::Dec, &b"O1"[..]));

        assert_eq!(f(b"-0O1"), (-1, IntKind::Oct, &b"1"[..]));

        assert_eq!(f(b"-0"), (-1, IntKind::Dec, &b"0"[..]));
        assert_eq!(f(b"+0"), (1, IntKind::Dec, &b"0"[..]));
    }

    #[test]
    fn int_of_string_opt_tests() {
        use int_of_string_opt as f;

        assert_eq!(f(""), None);
        assert_eq!(f("-"), None);
        assert_eq!(f("+"), None);
        assert_eq!(f("0x"), None);
        assert_eq!(f("0x"), None);

        assert_eq!(f("10"), Some(10));
        assert_eq!(f("1_0"), Some(10));
        assert_eq!(f("_10"), None);
        assert_eq!(f("0x10"), Some(16));
        assert_eq!(f("9223372036854775807"), Some(std::i64::MAX));
        assert_eq!(f("9223372036854775808"), None);

        assert_eq!(f("0x7FFFFFFFFFFFFFFF"), Some(std::i64::MAX));
        assert_eq!(f("-0x8000000000000000"), Some(std::i64::MIN));

        assert_eq!(f("0x8000000000000000"), None);
        assert_eq!(f("-0x8000000000000001"), None);

        assert_eq!(
            f("0b111111111111111111111111111111111111111111111111111111111111111"),
            Some(std::i64::MAX)
        );
        assert_eq!(
            f("-0b1000000000000000000000000000000000000000000000000000000000000000"),
            Some(std::i64::MIN)
        );

        assert_eq!(f("-_10"), None);
        assert_eq!(f("-0b10"), Some(-2));

        assert_eq!(f("-0b10"), Some(-2));

        assert_eq!(f("-9223372036854775808"), Some(std::i64::MIN));
        assert_eq!(f("-9223372036854775809"), None);

        assert_eq!(f("-0"), Some(0));
        assert_eq!(f("-0_"), Some(0));
        assert_eq!(f("+0"), Some(0));
    }

    #[test]
    fn parse_int_tests() {
        use parse_int as f;
        assert_eq!(f(""), Err(ParseIntError::Empty));
        assert_eq!(f("-"), Err(ParseIntError::Empty));
        assert_eq!(f("+"), Err(ParseIntError::Empty));
        assert_eq!(f("0x"), Err(ParseIntError::Empty));
        assert_eq!(f("0x"), Err(ParseIntError::Empty));

        assert_eq!(f("10"), Ok(10));
        assert_eq!(f("1_0"), Ok(10));
        assert_eq!(f("_10"), Err(ParseIntError::InvalidDigit(IntKind::Dec)));
        assert_eq!(f("0x10"), Ok(16));
        assert_eq!(f("9223372036854775807"), Ok(std::i64::MAX));
        assert_eq!(f("9223372036854775808"), Err(ParseIntError::OutOfRange));

        assert_eq!(f("0x7FFFFFFFFFFFFFFF"), Ok(std::i64::MAX));
        assert_eq!(f("-0x8000000000000000"), Ok(std::i64::MIN));

        assert_eq!(f("0x8000000000000000"), Err(ParseIntError::OutOfRange));
        assert_eq!(f("-0x8000000000000001"), Err(ParseIntError::OutOfRange));

        assert_eq!(
            f("0b111111111111111111111111111111111111111111111111111111111111111"),
            Ok(std::i64::MAX)
        );
        assert_eq!(
            f("-0b1000000000000000000000000000000000000000000000000000000000000000"),
            Ok(std::i64::MIN)
        );

        assert_eq!(f("-_10"), Err(ParseIntError::InvalidDigit(IntKind::Dec)));
        assert_eq!(f("-0b10"), Ok(-2));

        assert_eq!(f("-0b10"), Ok(-2));

        assert_eq!(f("-9223372036854775808"), Ok(std::i64::MIN));
        assert_eq!(f("-9223372036854775809"), Err(ParseIntError::OutOfRange));

        assert_eq!(f("-0"), Ok(0));
        assert_eq!(f("+0"), Ok(0));

        assert_eq!(f("010"), Ok(8));
        assert_eq!(f("-010"), Ok(-8));

        assert_eq!(f("0777777777777777777777"), Ok(std::i64::MAX));
        assert_eq!(f("-01000000000000000000000"), Ok(std::i64::MIN));

        assert_eq!(f("01000000000000000000000"), Err(ParseIntError::OutOfRange));
        assert_eq!(
            f("-01000000000000000000001"),
            Err(ParseIntError::OutOfRange)
        );
    }

    #[test]
    fn test_escaped() {
        use escaped as e;
        assert_eq!(e(""), "");
        assert_eq!(e("a"), "a");
        assert_eq!(e("\n"), "\\n");
        assert_eq!(e("\t"), "\\t");
        assert_eq!(e("\r"), "\\r");
        assert_eq!(e("'"), "'");
        assert_eq!(e("\""), "\\\"");
        assert_eq!(e("\\"), "\\\\");
        assert_eq!(e(String::from_utf8(vec![127]).unwrap().as_str()), "\\127");
        assert_eq!(e(String::from_utf8(vec![8]).unwrap().as_str()), "\\b");
        assert_eq!(
            e(unsafe { String::from_utf8_unchecked(vec![255]) }.as_str()),
            "\\255"
        );
    }
}
