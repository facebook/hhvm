// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use ascii::AsciiChar;
use ascii::AsciiString;

pub fn escaped_string(input: &[u8]) -> AsciiString {
    let mut res = AsciiString::with_capacity(input.len());
    for &ch in input {
        let code = match ch {
            b'\\' => AsciiChar::new('\\'),
            b'\n' => AsciiChar::new('n'),
            b'\r' => AsciiChar::new('r'),
            b'\t' => AsciiChar::new('t'),
            b'"' => {
                // \" doesn't work - emit \042 instead
                res.push(AsciiChar::BackSlash);
                res.push(AsciiChar::_0);
                res.push(AsciiChar::_4);
                res.push(AsciiChar::_2);
                continue;
            }
            _ if ch == b' ' || ch.is_ascii_graphic() => {
                res.push(AsciiChar::new(ch as char));
                continue;
            }
            _ => {
                // \ooo
                res.push(AsciiChar::BackSlash);
                res.push(AsciiChar::new((b'0' + (ch >> 6)) as char));
                res.push(AsciiChar::new((b'0' + ((ch >> 3) & 7)) as char));
                res.push(AsciiChar::new((b'0' + (ch & 7)) as char));
                continue;
            }
        };
        res.push(AsciiChar::BackSlash);
        res.push(code);
    }
    res
}

// Make sure the input is a valid identifier.
pub fn escaped_ident<'a>(input: Cow<'a, str>) -> Cow<'a, str> {
    // let ident = [%sedlex.regexp? (letter | Chars "_$"), Star (letter | digit | Chars "_$" | "::")]
    let mut it = input.chars();
    let mut last_colon = false;
    let valid = it.next().map_or(true, |ch| {
        // Although first char can be '_' we use that to indicate an escaped
        // string so we need to tweak it.
        ch.is_ascii_alphabetic() || ch == '$'
    }) && it.all(|ch| match (ch, last_colon) {
        (':', _) => {
            last_colon = !last_colon;
            true
        }
        (_, true) => false,
        (ch, false) => ch.is_ascii_alphanumeric() || (ch == '$') || (ch == '_'),
    });
    if !last_colon && valid {
        return input;
    }

    let mut out = String::with_capacity(input.len() * 3 / 2);
    out.push('_');

    let mut last_colon = false;
    for ch in input.bytes() {
        if ch == b':' {
            if last_colon {
                out.push(':');
                out.push(':');
            }
            last_colon = !last_colon;
            continue;
        }

        if last_colon {
            out.push_str("_3a");
            last_colon = false;
        }

        if ch.is_ascii_alphanumeric() || ch == b'$' {
            out.push(ch as char);
        } else {
            out.push('_');
            if ch == b'_' {
                out.push('_');
            } else {
                out.push(b"0123456789abcdef"[(ch >> 4) as usize] as char);
                out.push(b"0123456789abcdef"[(ch & 15) as usize] as char);
            }
        }
    }

    Cow::Owned(out)
}
