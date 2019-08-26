// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::relative_path::RelativePath;
use std::rc::Rc;

pub const INVALID: char = '\x00';

#[derive(Debug)]
struct SourceTextImpl<'a> {
    // All the indices in existing implementation are byte based, instead of unicode
    // char boundary based. This is bad experience for non-ASCII source files, but don't want to
    // change it now and deal with tracking all the dependencies of it.
    // Additionally, Rust assumes that &strs are valid UTF-8", but "test/slow/labels/74.php" suggests
    // that invalid unicode sequence can be a valid Hack program.
    // Using byte slice instead of &str looks ugly, but prevents us from constantly fighting
    // with compiler trying to guide us towards unicode semantics.
    text: &'a [u8],

    file_path: RelativePath,

    ocaml_source_text: usize,
}
#[derive(Debug, Clone)]
pub struct SourceText<'a>(Rc<SourceTextImpl<'a>>);

impl<'a> SourceText<'a> {
    pub fn make(file_path: &RelativePath, text: &'a [u8]) -> Self {
        Self::make_with_raw(file_path, text, 0)
    }

    pub fn make_with_raw(
        file_path: &RelativePath,
        text: &'a [u8],
        ocaml_source_text: usize,
    ) -> Self {
        Self(Rc::new(SourceTextImpl {
            file_path: file_path.clone(),
            text,
            ocaml_source_text,
        }))
    }

    pub fn file_path(&self) -> &RelativePath {
        &self.0.file_path
    }

    pub fn text(&self) -> &'a [u8] {
        self.0.text
    }

    pub fn length(&self) -> usize {
        self.text().len()
    }

    pub fn ocaml_source_text(&self) -> usize {
        self.0.ocaml_source_text
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
            b""
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
