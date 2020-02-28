// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use ocamlrep_derive::OcamlRep;

#[derive(
    Copy,
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    PartialEq,
    Serialize
)]
pub struct FilePosLarge {
    lnum: usize,
    bol: usize,
    cnum: usize,
}

const DUMMY: FilePosLarge = FilePosLarge {
    lnum: 0,
    bol: 0,
    cnum: usize::max_value(),
};

impl FilePosLarge {
    #[inline]
    pub fn is_dummy(self) -> bool {
        self == DUMMY
    }

    #[inline]
    pub const fn beg_of_file(self) -> Self {
        FilePosLarge {
            lnum: 1,
            bol: 0,
            cnum: 0,
        }
    }

    // constructors

    #[inline]
    pub const fn from_line_column_offset(line: usize, column: usize, offset: usize) -> Self {
        FilePosLarge {
            lnum: line,
            bol: offset - column,
            cnum: offset,
        }
    }

    #[inline]
    pub const fn from_lnum_bol_cnum(lnum: usize, bol: usize, cnum: usize) -> Self {
        FilePosLarge { lnum, bol, cnum }
    }

    // accessors

    #[inline]
    pub const fn offset(self) -> usize {
        self.cnum
    }

    #[inline]
    pub const fn line(self) -> usize {
        self.lnum
    }

    #[inline]
    pub const fn column(self) -> usize {
        self.cnum - self.bol
    }

    #[inline]
    pub const fn beg_of_line(self) -> usize {
        self.bol
    }

    #[inline]
    pub const fn with_column(self, col: usize) -> Self {
        FilePosLarge {
            lnum: self.lnum,
            bol: self.bol,
            cnum: self.bol + col,
        }
    }

    #[inline]
    pub const fn line_beg(self) -> (usize, usize) {
        (self.lnum, self.bol)
    }

    #[inline]
    pub const fn line_column(self) -> (usize, usize) {
        (self.lnum, self.cnum - self.bol)
    }

    #[inline]
    pub const fn line_column_beg(self) -> (usize, usize, usize) {
        (self.lnum, self.cnum - self.bol, self.bol)
    }

    #[inline]
    pub const fn line_column_offset(self) -> (usize, usize, usize) {
        (self.lnum, self.cnum - self.bol, self.cnum)
    }

    #[inline]
    pub const fn line_beg_offset(self) -> (usize, usize, usize) {
        (self.lnum, self.bol, self.cnum)
    }
}

impl Ord for FilePosLarge {
    fn cmp(&self, other: &FilePosLarge) -> std::cmp::Ordering {
        self.offset().cmp(&other.offset())
    }
}

impl PartialOrd for FilePosLarge {
    fn partial_cmp(&self, other: &FilePosLarge) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
