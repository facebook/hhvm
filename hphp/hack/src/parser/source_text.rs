/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/

pub const INVALID: char = '\x00';

#[derive(Debug, Clone)]
pub struct SourceText<'a> {
    /* All the indices in existing implementation are byte based, instead of unicode
     * char boundary based. This is bad experience for non-ASCII source files, but don't want to
     * change it now and deal with tracking all the dependencies of it.
     * Additionally, Rust assumes that &strs are valid UTF-8", but "test/slow/labels/74.php" suggests
     * that invalid unicode sequence can be a valid Hack program.
     * Using byte slice instead of &str looks ugly, but prevents us from constantly fighting
     * with compiler trying to guide us towards unicode semantics. */
    text: &'a [u8],
}

impl<'a> SourceText<'a> {
    pub fn make(text: &'a [u8]) -> Self {
        Self { text }
    }

    pub fn text(&self) -> &'a [u8] {
        self.text
    }

    pub fn length(&self) -> usize {
        self.text().len()
    }

    pub fn get(&self, index: usize) -> char {
        self.text()
            .get(index)
            .map(|x| *x as char)
            .unwrap_or(INVALID)
    }

    pub fn sub(&self, start: usize, length: usize) -> &'a [u8] {
        let len = self.length();

        if start >= len {
            "".as_bytes()
        } else if start + length > len {
            &self.text()[start..]
        } else {
            &self.text()[start..(start + length)]
        }
    }

    pub fn sub_as_str(&self, start: usize, length: usize) -> &'a str {
        unsafe { std::str::from_utf8_unchecked(self.sub(start, length)) }
    }
}
