// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;
use std::sync::Arc;

use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use relative_path::RelativePath;

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

    file_path: Arc<RelativePath>,

    ocaml_source_text: Option<UnsafeOcamlPtr>,
}
#[derive(Debug, Clone)]
pub struct SourceText<'a>(Rc<SourceTextImpl<'a>>);

impl<'a> SourceText<'a> {
    pub fn make(file_path: Arc<RelativePath>, text: &'a [u8]) -> Self {
        Self::make_with_raw(file_path, text, 0)
    }

    pub fn make_with_raw(
        file_path: Arc<RelativePath>,
        text: &'a [u8],
        ocaml_source_text: usize,
    ) -> Self {
        Self(Rc::new(SourceTextImpl {
            file_path,
            text,
            ocaml_source_text: if ocaml_source_text == 0 {
                None
            } else {
                unsafe { Some(UnsafeOcamlPtr::new(ocaml_source_text)) }
            },
        }))
    }

    pub fn file_path(&self) -> &RelativePath {
        self.0.file_path.as_ref()
    }

    pub fn file_path_rc(&self) -> Arc<RelativePath> {
        Arc::clone(&self.0.file_path)
    }

    pub fn text(&self) -> &'a [u8] {
        self.0.text
    }

    pub fn text_as_str(&self) -> &'a str {
        unsafe { std::str::from_utf8_unchecked(self.0.text) }
    }

    pub fn length(&self) -> usize {
        self.text().len()
    }

    pub fn ocaml_source_text(&self) -> Option<UnsafeOcamlPtr> {
        self.0.ocaml_source_text
    }

    pub fn get(&self, index: usize) -> char {
        self.text().get(index).map_or(INVALID, |x| *x as char)
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

impl<'content> ToOcamlRep for SourceText<'content> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        // A SourceText with no associated ocaml_source_text cannot be converted
        // to OCaml yet (we'd need to construct the OffsetMap). We still
        // construct some in test cases, so just panic upon attempts to convert.
        alloc.add_copy(self.0.ocaml_source_text.unwrap())
    }
}

impl<'content> FromOcamlRep for SourceText<'content> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 4)?;
        let file_path: Arc<RelativePath> = ocamlrep::from::field(block, 0)?;
        // Unsafely transmute away the lifetime of `value` and allow the caller
        // to choose the lifetime. This is no more unsafe than what we already
        // do by storing the ocaml_source_text pointer--if the OCaml source text
        // is collected, our text field and ocaml_source_text field will both be
        // invalid. The caller must take care not to let the OCaml source text
        // be collected while a Rust SourceText exists.
        let text: &'content [u8] = unsafe {
            std::mem::transmute(
                ocamlrep::bytes_from_ocamlrep(block[2])
                    .map_err(|e| ocamlrep::FromError::ErrorInField(2, Box::new(e)))?,
            )
        };
        let ocaml_source_text = Some(UnsafeOcamlPtr::from_ocamlrep(value)?);
        Ok(Self(Rc::new(SourceTextImpl {
            file_path,
            text,
            ocaml_source_text,
        })))
    }
}
