// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::source_text::SourceText;
use line_break_map::LineBreakMap;
use oxidized::pos::Pos;
use std::rc::Rc;

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

    pub fn source_text(&self) -> &SourceText {
        &self.0.source_text
    }

    pub fn offset_to_position(&self, offset: isize) -> (isize, isize) {
        self.0.offset_map.offset_to_position(offset)
    }

    pub fn relative_pos(&self, start_offset: usize, end_offset: usize) -> Pos {
        let pos_start = self.0.offset_map.offset_to_file_pos_triple(start_offset);
        let pos_end = self.0.offset_map.offset_to_file_pos_triple(end_offset);
        Pos::from_lnum_bol_cnum(self.0.source_text.file_path_rc(), pos_start, pos_end)
    }
}
