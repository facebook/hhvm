// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use serde::Serialize;

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct HhasPos {
    pub line_begin: usize,
    pub line_end: usize,
    pub col_begin: usize,
    pub col_end: usize,
}
impl Default for HhasPos {
    fn default() -> Self {
        let (line_begin, line_end, col_begin, col_end) = (1, 1, 0, 0);
        Self {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }
    }
}
impl std::convert::From<oxidized::pos::Pos> for HhasPos {
    fn from(p: oxidized::pos::Pos) -> Self {
        let (line_begin, line_end, col_begin, col_end) = if p.is_none() || !p.is_valid() {
            (1, 1, 0, 0)
        } else {
            p.info_pos_extended()
        };
        Self {
            line_begin,
            line_end,
            col_begin,
            col_end,
        }
    }
}

/// Span, emitted as prefix to classes and functions
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct HhasSpan {
    pub line_begin: usize,
    pub line_end: usize,
}

impl HhasSpan {
    pub fn from_pos(pos: &oxidized::pos::Pos) -> Self {
        let (line_begin, line_end, _, _) = pos.info_pos_extended();
        Self {
            line_begin,
            line_end,
        }
    }
}
