// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::source_text::SourceText;
use line_break_map::LineBreakMap;
use oxidized::pos::Pos;

#[derive(Debug)]
pub struct IndexedSourceText<'a> {
    pub source_text: &'a SourceText<'a>,
    offset_map: LineBreakMap,
}

impl<'a> IndexedSourceText<'a> {
    pub fn new(source_text: &'a SourceText<'a>) -> Self {
        IndexedSourceText {
            source_text,
            offset_map: LineBreakMap::new(source_text.text()),
        }
    }

    pub fn source_text(&self) -> &SourceText {
        self.source_text
    }

    pub fn offset_to_position(&self, offset: isize) -> (isize, isize) {
        self.offset_map.offset_to_position(offset)
    }

    pub fn relative_pos(&self, start_offset: usize, end_offset: usize) -> Pos {
        let pos_start = self.offset_map.offset_to_file_pos_triple(start_offset);
        let pos_end = self.offset_map.offset_to_file_pos_triple(end_offset);
        Pos::from_lnum_bol_cnum(self.source_text.file_path().clone(), pos_start, pos_end)
    }
}
