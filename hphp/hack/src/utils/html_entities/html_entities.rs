// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#[macro_use]
extern crate lazy_static;

pub mod decoder;

use regex::bytes::{Captures, Regex};

pub fn utf32_to_utf8(k: u32, r: &mut [u8; 6]) -> &[u8] {
    if k < 0x80 {
        r[0] = k as u8;
        &r[0..1]
    } else if k < 0x800 {
        r[0] = 0xc0 | ((k >> 6) as u8);
        r[1] = 0x80 | ((k & 0x3f) as u8);
        &r[0..2]
    } else if k < 0x10000 {
        r[0] = 0xe0 | ((k >> 12) as u8);
        r[1] = 0x80 | (((k >> 6) & 0x3f) as u8);
        r[2] = 0x80 | ((k & 0x3f) as u8);
        &r[0..3]
    } else if k < 0x200000 {
        r[0] = 0xf0 | ((k >> 18) as u8);
        r[1] = 0x80 | (((k >> 12) & 0x3f) as u8);
        r[2] = 0x80 | (((k >> 6) & 0x3f) as u8);
        r[3] = 0x80 | ((k & 0x3f) as u8);
        &r[0..4]
    } else if k < 0x4000000 {
        r[0] = 0xf8 | ((k >> 24) as u8);
        r[1] = 0x80 | (((k >> 18) & 0x3f) as u8);
        r[2] = 0x80 | (((k >> 12) & 0x3f) as u8);
        r[3] = 0x80 | (((k >> 6) & 0x3f) as u8);
        r[4] = 0x80 | ((k & 0x3f) as u8);
        &r[0..5]
    } else {
        r[0] = 0xfc | ((k >> 30) as u8);
        r[1] = 0x80 | (((k >> 24) & 0x3f) as u8);
        r[2] = 0x80 | (((k >> 18) & 0x3f) as u8);
        r[3] = 0x80 | (((k >> 12) & 0x3f) as u8);
        r[4] = 0x80 | (((k >> 6) & 0x3f) as u8);
        r[5] = 0x80 | ((k & 0x3f) as u8);
        &r[0..6]
    }
}

pub fn utf32_to_utf8_alloc(k: u32) -> Vec<u8> {
    Vec::from(utf32_to_utf8(k, &mut [0u8; 6]))
}

fn decode_u32(s: &[u8]) -> Vec<u8> {
    // from_utf8 returns err on invalid utf8, which still
    // matches Ocaml's behavior since invalid utf8 can't
    // parsed to u32
    let n = std::str::from_utf8(&s[2..s.len() - 1]);
    let n = match n {
        Err(_) => return vec![],
        Ok(s) => s,
    };
    let n = u32::from_str_radix(n, 10);
    match n {
        Err(_) => vec![],
        Ok(n) => utf32_to_utf8_alloc(n),
    }
}

fn decode_charref<'a>(s: &'a [u8]) -> &'a [u8] {
    let charref = &s[1..s.len() - 1];
    decoder::decode(charref).unwrap_or(s)
}

pub fn decode<'a>(s: &'a [u8]) -> Vec<u8> {
    lazy_static! {
        static ref ENTITY: Regex = Regex::new("&[^;&]+;").unwrap();
    }
    ENTITY
        .replace_all(s, |caps: &Captures| match caps.get(0) {
            None => vec![],
            Some(m) => {
                let m = m.as_bytes();
                if m[1] == b'#' {
                    decode_u32(m)
                } else {
                    // Have to alloc memory, this is contrainted by
                    // the return type of this closure, AsRef<[u8]>.
                    Vec::from(decode_charref(m))
                }
            }
        })
        .to_vec()
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test() {
        assert_eq!(decode(b"&Scaron;"), Vec::from("Š"));
        assert_eq!(decode(b"&Scaron"), Vec::from("&Scaron"));
        assert_eq!(decode(b"&#Scaron"), Vec::from("&#Scaron"));
        assert_eq!(decode(b"&#352;"), Vec::from("Š"));
        assert_eq!(decode(b"&#352"), Vec::from("&#352"));
        assert_eq!(decode(b"abc&#352;efg"), Vec::from("abcŠefg"));
    }
}
