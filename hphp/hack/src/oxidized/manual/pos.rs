// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::Range;

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::file_pos_large::FilePosLarge;
use crate::file_pos_small::FilePosSmall;
use crate::relative_path::{Prefix, RelativePath};

use std::path::PathBuf;
use std::result::Result::*;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
enum PosImpl {
    Small {
        file: RelativePath,
        start: FilePosSmall,
        end: FilePosSmall,
    },
    Large {
        file: RelativePath,
        start: Box<FilePosLarge>,
        end: Box<FilePosLarge>,
    },
}

use PosImpl::*;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Pos(PosImpl);

impl Pos {
    pub fn make_none() -> Self {
        // TODO: shiqicao make NONE static, lazy_static doesn't allow Rc
        Pos(PosImpl::Small {
            file: RelativePath::make(Prefix::Dummy, PathBuf::from("")),
            start: FilePosSmall::make_dummy(),
            end: FilePosSmall::make_dummy(),
        })
    }

    pub fn filename(&self) -> &RelativePath {
        match &self.0 {
            Small { file, .. } => &file,
            Large { file, .. } => &file,
        }
    }

    pub fn from_lnum_bol_cnum(
        file: RelativePath,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> Self {
        let (start_line, start_bol, start_cnum) = start;
        let (end_line, end_bol, end_cnum) = end;
        let start = FilePosSmall::from_lnum_bol_cnum(start_line, start_bol, start_cnum);
        let end = FilePosSmall::from_lnum_bol_cnum(end_line, end_bol, end_cnum);
        match (start, end) {
            (Some(start), Some(end)) => Pos(Small { file, start, end }),
            _ => {
                let start = Box::new(FilePosLarge::from_lnum_bol_cnum(
                    start_line, start_bol, start_cnum,
                ));
                let end = Box::new(FilePosLarge::from_lnum_bol_cnum(
                    end_line, end_bol, end_cnum,
                ));
                Pos(Large { file, start, end })
            }
        }
    }

    /// For single-line spans only.
    pub fn from_line_cols_offset(
        file: RelativePath,
        line: usize,
        cols: Range<usize>,
        start_offset: usize,
    ) -> Self {
        let start = FilePosSmall::from_line_column_offset(line, cols.start, start_offset);
        let end = FilePosSmall::from_line_column_offset(
            line,
            cols.end,
            start_offset + (cols.end - cols.start),
        );
        match (start, end) {
            (Some(start), Some(end)) => Pos(Small { file, start, end }),
            _ => {
                let start = Box::new(FilePosLarge::from_line_column_offset(
                    line,
                    cols.start,
                    start_offset,
                ));
                let end = Box::new(FilePosLarge::from_line_column_offset(
                    line,
                    cols.end,
                    start_offset + (cols.end - cols.start),
                ));
                Pos(Large { file, start, end })
            }
        }
    }

    fn small_to_large_file_pos(p: &FilePosSmall) -> FilePosLarge {
        let (lnum, col, bol) = p.line_column_beg();
        FilePosLarge::from_lnum_bol_cnum(lnum, bol, bol + col)
    }

    pub fn btw_nocheck(x1: Self, x2: Self) -> Self {
        let inner = match (x1.0, x2.0) {
            (Small { file, start, .. }, Small { end, .. }) => Small { file, start, end },
            (Large { file, start, .. }, Large { end, .. }) => Large { file, start, end },
            (Small { file, start, .. }, Large { end, .. }) => Large {
                file,
                start: Box::new(Self::small_to_large_file_pos(&start)),
                end,
            },
            (Large { file, start, .. }, Small { end, .. }) => Large {
                file,
                start,
                end: Box::new(Self::small_to_large_file_pos(&end)),
            },
        };
        Pos(inner)
    }

    pub fn btw(x1: &Self, x2: &Self) -> Result<Self, String> {
        if x1.filename() != x2.filename() {
            // using string concatenation instead of format!,
            // it is not stable see T52404885
            Err(String::from("Position in separate files ")
                + &x1.filename().to_string()
                + " and "
                + &x2.filename().to_string())
        } else if x1.end_cnum() > x2.end_cnum() {
            Err(String::from("btw: invalid positions")
                + &x1.end_cnum().to_string()
                + "and"
                + &x2.end_cnum().to_string())
        } else {
            Ok(Self::btw_nocheck(x1.clone(), x2.clone()))
        }
    }

    pub fn end_cnum(&self) -> usize {
        match &self.0 {
            Small { end, .. } => end.offset(),
            Large { end, .. } => end.offset(),
        }
    }
}
