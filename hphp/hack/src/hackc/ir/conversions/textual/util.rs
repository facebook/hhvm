// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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
