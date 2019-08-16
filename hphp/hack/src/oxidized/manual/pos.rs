// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::Range;

use ocamlrep_derive::IntoOcamlRep;

use crate::file_pos_large::FilePosLarge;
use crate::file_pos_small::FilePosSmall;
use crate::relative_path::RelativePath;

#[derive(Clone, Debug, IntoOcamlRep)]
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

#[derive(Clone, Debug, IntoOcamlRep)]
pub struct Pos(PosImpl);

impl Pos {
    pub fn from_lnum_bol_cnum(
        file: RelativePath,
        start: (u64, u64, u64),
        end: (u64, u64, u64),
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
        line: u64,
        cols: Range<u64>,
        start_offset: u64,
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
}
