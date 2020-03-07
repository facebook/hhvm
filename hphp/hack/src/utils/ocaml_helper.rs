// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Port from https://github.com/ocaml/ocaml/blob/6efe8fea5b6c3f1db22e50e8b164d6ffec85578d/runtime/ints.c
fn parse_sign_and_base(p: &[u8]) -> (i8, u8, &[u8]) {
    let mut sign = 1i8;
    if p.len() == 0 {
        return (sign, 10, p);
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
            b'x' | b'X' => (sign, 16, &p[i + 2..]),
            b'o' | b'O' => (sign, 8, &p[i + 2..]),
            b'b' | b'B' => (sign, 2, &p[i + 2..]),
            b'u' | b'U' => (sign, 10, &p[i + 2..]),
            _ => (sign, 10, &p[i..]),
        }
    } else {
        (sign, 10, &p[i..])
    }
}

fn parse_digit(c: u8) -> Option<u8> {
    if c >= b'0' && c <= b'9' {
        Some(c - b'0')
    } else if c >= b'A' && c <= b'F' {
        Some(c - b'A' + 10)
    } else if c >= b'a' && c <= b'f' {
        Some(c - b'a' + 10)
    } else {
        None
    }
}

pub fn int_of_string_opt(p: &[u8]) -> Option<i64> {
    let (sign, base, p) = parse_sign_and_base(p);
    if p.len() == 0 {
        return None;
    }
    let mut r = 0i64;
    for i in 0..p.len() {
        if i != 0 && p[i] == b'_' {
            continue;
        }
        let d = parse_digit(p[i])?;
        if d >= base {
            return None;
        } else {
            r = i64::checked_mul(r, base as i64)?;
            if sign >= 0 {
                r = i64::checked_add(r, d as i64)?;
            } else {
                r = i64::checked_sub(r, d as i64)?;
            }
        }
    }
    Some(r)
}

pub fn int_of_str_opt(s: impl AsRef<str>) -> Option<i64> {
    int_of_string_opt(s.as_ref().as_bytes())
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn parse_digit_tests() {
        use parse_digit as f;
        assert_eq!(f(b'0'), Some(0));
        assert_eq!(f(b'1'), Some(1));
        assert_eq!(f(b'9'), Some(9));
        assert_eq!(f(b'A'), Some(10));
        assert_eq!(f(b'a'), Some(10));
        assert_eq!(f(b'F'), Some(15));
        assert_eq!(f(b'f'), Some(15));

        assert_eq!(f(b'-'), None);
        assert_eq!(f(b'z'), None);
    }

    #[test]
    fn parse_sign_and_base_tests() {
        use parse_sign_and_base as f;
        assert_eq!(f(b""), (1, 10, &b""[..]));
        assert_eq!(f(b"-"), (-1, 10, &b""[..]));
        assert_eq!(f(b"+"), (1, 10, &b""[..]));
        assert_eq!(f(b"z"), (1, 10, &b"z"[..]));
        assert_eq!(f(b"0"), (1, 10, &b"0"[..]));

        assert_eq!(f(b"0x"), (1, 16, &b""[..]));
        assert_eq!(f(b"0X"), (1, 16, &b""[..]));

        assert_eq!(f(b"0o"), (1, 8, &b""[..]));
        assert_eq!(f(b"0O"), (1, 8, &b""[..]));

        assert_eq!(f(b"0b"), (1, 2, &b""[..]));
        assert_eq!(f(b"0B"), (1, 2, &b""[..]));

        assert_eq!(f(b"0u"), (1, 10, &b""[..]));
        assert_eq!(f(b"0U"), (1, 10, &b""[..]));

        assert_eq!(f(b"-1"), (-1, 10, &b"1"[..]));
        assert_eq!(f(b"+1"), (1, 10, &b"1"[..]));

        assert_eq!(f(b"-x"), (-1, 10, &b"x"[..]));
        assert_eq!(f(b"+B"), (1, 10, &b"B"[..]));

        assert_eq!(f(b"-O1"), (-1, 10, &b"O1"[..]));
        assert_eq!(f(b"+u0"), (1, 10, &b"u0"[..]));

        assert_eq!(f(b"-0O1"), (-1, 8, &b"1"[..]));
        assert_eq!(f(b"+0u0"), (1, 10, &b"0"[..]));

        assert_eq!(f(b"-0"), (-1, 10, &b"0"[..]));
        assert_eq!(f(b"+0"), (1, 10, &b"0"[..]));
    }

    #[test]
    fn int_of_string_opt_tests() {
        use int_of_string_opt as f;

        assert_eq!(f(b""), None);
        assert_eq!(f(b""), None);
        assert_eq!(f(b"-"), None);
        assert_eq!(f(b"+"), None);
        assert_eq!(f(b"0x"), None);
        assert_eq!(f(b"0x"), None);

        assert_eq!(f(b"10"), Some(10));
        assert_eq!(f(b"1_0"), Some(10));
        assert_eq!(f(b"_10"), None);
        assert_eq!(f(b"0x10"), Some(16));
        assert_eq!(f(b"9223372036854775807"), Some(std::i64::MAX));
        assert_eq!(f(b"9223372036854775808"), None);

        assert_eq!(f(b"-_10"), None);
        assert_eq!(f(b"-0b10"), Some(-2));

        assert_eq!(f(b"-0b10"), Some(-2));

        assert_eq!(f(b"-9223372036854775808"), Some(std::i64::MIN));
        assert_eq!(f(b"-9223372036854775809"), None);

        assert_eq!(f(b"-0"), Some(0));
        assert_eq!(f(b"-0_"), Some(0));
        assert_eq!(f(b"+0"), Some(0));
    }
}
