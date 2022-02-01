// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This module supports reasoning about independent chunks
//! in a stream of encoded data, usually called "frames", e.g.:
//! https://github.com/lz4/lz4/blob/dev/doc/lz4_Frame_format.md#user-content-general-structure-of-lz4-frame-format
//!
//! Specifically, it supports escaping and unescaping of what would be
//! interpreted as LF (line feed) in a ASCII character / byte stream,
//! so that one can read entire frame of encoded binary data with
//! read-line API, which is readily available in both OCaml and Rust.
//!
//! Structs allow for paying the precomputation overhead (e.g., of
//! constructing & optimizing regular expression automata) once only,
//! and should usually be reused for creating frames of many byte chunks,
//! each of which can be encoded in arbitrary format.  Each chunks is
//! escaped to create a single line that represent the corresponding frame
//! using a single call to [`LineFeedEscaper::escape`].  The reverse is
//! done using [`LineFeedUnescaper::unescape`].

use std::{
    borrow::Cow,
    io::{BufRead, BufReader, Read},
};

use regex::bytes::{NoExpand, Regex};

/// A struct that abstracts away the escaping of \x0A bytes,
/// which are interpreted as LF (Line Feed) characters under ASCII encoding.
/// The return type is [`Cow<'a, [u8]>`] because if no replacement is done,
/// the original string may be returned and allocations avoided (in theory).
pub struct LineFeedEscaper(Option<(Regex, Regex)>);
impl LineFeedEscaper {
    pub fn new(use_regex: bool) -> Self {
        Self(if use_regex {
            // Note: -u disables Unicode checks so it works for arbitrary bytes
            // https://docs.rs/regex/1.5.4/regex/index.html#opt-out-of-unicode-support
            Some((
                Regex::new(r"(?-u)\\").unwrap(),
                Regex::new("(?-u)\n").unwrap(),
            ))
        } else {
            None
        })
    }

    pub fn escape<'a>(&self, bs0: &'a [u8]) -> Cow<'a, [u8]> {
        if !bs0.iter().any(|b| *b == b'\\' || *b == b'\n') {
            return Cow::Borrowed(bs0);
        }
        if let Some((re1, re2)) = &self.0 {
            let bs1: Cow<'a, [u8]> = re1.replace_all(bs0, NoExpand(br"\\"));
            // sadly, regex crate doesn't support replacements on Cow<'a, [u8]>,
            // therefore we need to clone it (keep the future-proof API, though)
            Cow::Owned(re2.replace_all(bs1.as_ref(), NoExpand(br"\n")).into_owned())
        } else {
            let extra_size: u32 = bs0
                .iter()
                .map(|b| (*b == b'\\' || *b == b'\n') as u32)
                .sum();
            if extra_size == 0 {
                return Cow::Borrowed(bs0);
            }
            let mut ret: Vec<u8> = Vec::with_capacity(bs0.len() + extra_size as usize);
            for b in bs0 {
                if *b == b'\n' {
                    ret.push(b'\\');
                    ret.push(b'n');
                } else {
                    if *b == b'\\' {
                        ret.push(b'\\');
                    }
                    ret.push(*b);
                }
            }
            Cow::Owned(ret)
        }
    }
}

/// A struct that abstracts away the unescaping back to \x0A bytes,
/// as well as any other bytes escaped via the [`LineFeedEscaper`]
/// (namely, what corresponds to `\` in ASCII).
/// The return type is [`Cow<'a, [u8]>`] because if no replacement is done,
/// the original string may be returned and allocations avoided.
pub struct LineFeedUnescaper();
impl LineFeedUnescaper {
    pub fn new() -> Self {
        // note: a Regex with capture groups may actually be faster
        Self()
    }

    pub fn unescape<'a>(&self, bs: &'a [u8]) -> Cow<'a, [u8]> {
        if !bs.iter().any(|b| *b == b'\\') {
            return Cow::Borrowed(bs);
        }
        let mut ret: Vec<u8> = Vec::with_capacity(bs.len() - 1);
        let mut peek = false;
        for b in bs {
            if !peek && *b == b'\\' {
                peek = true;
            } else {
                ret.push(if peek && *b == b'n' { b'\n' } else { *b });
                peek = false;
            }
        }
        Cow::Owned(ret)
    }
}

pub fn read_lines_as_bytes<R: Read>(reader: R) -> impl Iterator<Item = Vec<u8>> {
    let reader = BufReader::new(reader);
    struct Bytes<R>(R);
    impl<R: BufRead> Iterator for Bytes<R> {
        type Item = Vec<u8>;
        fn next(&mut self) -> Option<Self::Item> {
            let mut bs = Vec::<u8>::new();
            match self.0.read_until(b'\n', &mut bs) {
                Ok(n) if n != 0 => {
                    bs.pop();
                    Some(bs)
                }
                Ok(_) => None,
                _ => panic!("failed to read bytes as new line"),
            }
        }
    }
    Bytes(reader)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn newlines_and_backslashes_simple() {
        let bs = b"A\\BC\nD\\E\\\nF\\"; // A\BC<LF>D\E\<LF>F\
        let bs_escaped_expected =
            // A  \  B C  <LF>  D \ E  \   <LF>  F  \  [original]
            // A \ \ B C  \  n  D \ E \ \  \  n  F \ \ [escaped]
            br"A\\BC\nD\\E\\\nF\\";
        println!("{:?}", bs.to_vec());

        let bs_escaped = LineFeedEscaper::new(true).escape(bs.as_slice());
        assert_eq!(bs_escaped_expected.to_vec(), bs_escaped.as_ref().to_vec());

        let bs_unescaped = LineFeedUnescaper::new().unescape(bs_escaped.as_ref());
        assert_eq!(bs.to_vec(), bs_unescaped.as_ref().to_vec());
    }

    #[test]
    fn exhaustive_round_trip_for_correctness() {
        fn round_trip(bs: Vec<u8>) {
            let bs_escaped = LineFeedEscaper::new(false).escape(bs.as_slice());
            {
                let bs_escaped2 = LineFeedEscaper::new(true).escape(bs.as_slice());
                assert_eq!(bs_escaped.as_ref(), bs_escaped2.as_ref());
            }

            let bs_unescaped = LineFeedUnescaper::new().unescape(bs_escaped.as_ref());
            assert_eq!(bs, bs_unescaped.into_owned());
        }

        // 4^5 = 1024 combinations are being checked
        let choices = ['F', 'B', '\\', '\n'];
        for c0 in &choices {
            for c1 in &choices {
                for c2 in &choices {
                    for c3 in &choices {
                        for c4 in &choices {
                            round_trip(vec![*c0 as u8, *c1 as u8, *c2 as u8, *c3 as u8, *c4 as u8]);
                        }
                    }
                }
            }
        }
    }
}
