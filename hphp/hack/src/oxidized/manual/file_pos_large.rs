// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::IntoOcamlRep;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Ord, PartialOrd, IntoOcamlRep)]
pub struct FilePosLarge {
    lnum: u64,
    bol: u64,
    cnum: u64,
}

const DUMMY: FilePosLarge = FilePosLarge {
    lnum: 0,
    bol: 0,
    cnum: u64::max_value(),
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
    pub const fn from_line_column_offset(line: u64, column: u64, offset: u64) -> Self {
        FilePosLarge {
            lnum: line,
            bol: offset - column,
            cnum: offset,
        }
    }

    #[inline]
    pub const fn from_lnum_bol_cnum(lnum: u64, bol: u64, cnum: u64) -> Self {
        FilePosLarge { lnum, bol, cnum }
    }

    // accessors

    #[inline]
    pub const fn offset(self) -> u64 {
        self.cnum
    }

    #[inline]
    pub const fn line(self) -> u64 {
        self.lnum
    }

    #[inline]
    pub const fn column(self) -> u64 {
        self.cnum - self.bol
    }

    #[inline]
    pub const fn beg_of_line(self) -> u64 {
        self.bol
    }

    #[inline]
    pub const fn with_column(self, col: u64) -> Self {
        FilePosLarge {
            lnum: self.lnum,
            bol: self.bol,
            cnum: self.bol + col,
        }
    }

    #[inline]
    pub const fn line_beg(self) -> (u64, u64) {
        (self.lnum, self.bol)
    }

    #[inline]
    pub const fn line_column(self) -> (u64, u64) {
        (self.lnum, self.cnum - self.bol)
    }

    #[inline]
    pub const fn line_column_beg(self) -> (u64, u64, u64) {
        (self.lnum, self.cnum - self.bol, self.bol)
    }

    #[inline]
    pub const fn line_column_offset(self) -> (u64, u64, u64) {
        (self.lnum, self.cnum - self.bol, self.cnum)
    }

    #[inline]
    pub const fn line_beg_offset(self) -> (u64, u64, u64) {
        (self.lnum, self.bol, self.cnum)
    }
}
