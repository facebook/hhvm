// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use line_break_map::LineBreakMap;

use crate::source_text::SourceText;

#[derive(Debug)]
pub struct IndexedSourceTextImpl<'a> {
    pub source_text: SourceText<'a>,
    offset_map: LineBreakMap,
}

#[derive(Clone, Debug)]
pub struct IndexedSourceText<'a>(Rc<IndexedSourceTextImpl<'a>>);

impl<'a> IndexedSourceText<'a> {
    pub fn new(source_text: SourceText<'a>) -> Self {
        let text = source_text.text();
        Self(Rc::new(IndexedSourceTextImpl {
            source_text,
            offset_map: LineBreakMap::new(text),
        }))
    }

    pub fn source_text(&self) -> &SourceText<'a> {
        &self.0.source_text
    }

    pub fn offset_to_position(&self, offset: isize) -> (isize, isize) {
        self.0.offset_map.offset_to_position(offset)
    }

    pub fn relative_pos(&self, start_offset: usize, end_offset: usize) -> Pos {
        Pos {
            path: self.0.source_text.file_path_rc(),
            start: self.0.offset_map.offset_to_file_pos_triple(start_offset),
            end: self.0.offset_map.offset_to_file_pos_triple(end_offset),
        }
    }

    pub fn offset_to_file_pos_triple(&self, offset: usize) -> (usize, usize, usize) {
        self.0.offset_map.offset_to_file_pos_triple(offset)
    }
}

pub struct Pos {
    pub path: std::sync::Arc<relative_path::RelativePath>,
    /// The three usizes represent (lnum, bol, offset):
    /// - lnum: Line number. Starts at 1.
    /// - bol: Byte offset from beginning of file to the beginning of the line
    ///   containing this position. The column number is therefore offset - bol.
    ///   Starts at 0.
    /// - offset: Byte offset from the beginning of the file. Starts at 0.
    pub start: (usize, usize, usize),
    /// (lnum, bol, offset), as above
    pub end: (usize, usize, usize),
}
