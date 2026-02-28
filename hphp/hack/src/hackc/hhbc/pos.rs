// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use serde::Serialize;

// Keep this in sync with HPHP::SourceLoc in hphp/runtime/vm/source-location.h
#[derive(Clone, Copy, Debug, Eq, PartialEq, Hash, Serialize)]
#[repr(C)]
pub struct SrcLoc {
    pub line_begin: i32,
    pub line_end: i32,
    pub col_begin: i32,
    pub col_end: i32,
}

impl Default for SrcLoc {
    fn default() -> Self {
        Self {
            line_begin: 1,
            line_end: 1,
            col_begin: 0,
            col_end: 0,
        }
    }
}

impl SrcLoc {
    pub fn to_span(&self) -> Span {
        Span {
            line_begin: self.line_begin,
            line_end: self.line_end,
        }
    }

    pub fn from_span(span: &Span) -> Self {
        Self {
            line_begin: span.line_begin,
            col_begin: 0,
            line_end: span.line_end,
            col_end: 0,
        }
    }
}

impl std::convert::From<oxidized::pos::Pos> for SrcLoc {
    fn from(p: oxidized::pos::Pos) -> Self {
        if p.is_none() || !p.is_valid() {
            Self::default()
        } else {
            let (line_begin, line_end, col_begin, col_end) = p.info_pos_extended();
            Self {
                line_begin: line_begin as i32,
                line_end: line_end as i32,
                col_begin: col_begin as i32,
                col_end: col_end as i32,
            }
        }
    }
}

/// Span, emitted as prefix to classes and functions
/// Keep this in sync with line1,line2 in HPHP::FuncEmitter
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Span {
    pub line_begin: i32,
    pub line_end: i32,
}

impl Span {
    pub fn from_pos(pos: &oxidized::pos::Pos) -> Self {
        let (line_begin, line_end, _, _) = pos.info_pos_extended();
        Self {
            line_begin: line_begin as i32,
            line_end: line_end as i32,
        }
    }
}
