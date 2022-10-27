// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::file_pos::FilePos;
use crate::file_pos_small::FilePosSmall;

#[derive(Copy, Clone, Debug, Deserialize, Eq, Hash, PartialEq, Serialize)]
pub struct FilePosLarge {
    /// line number. Starts at 1.
    lnum: usize,
    /// character number of the beginning of line of this position.
    /// The column number is therefore offset - bol
    /// Starts at 0
    bol: usize,
    /// character offset from the beginning of the file. Starts at 0.
    offset: usize,
}
arena_deserializer::impl_deserialize_in_arena!(FilePosLarge);

impl arena_trait::TrivialDrop for FilePosLarge {}

const DUMMY: FilePosLarge = FilePosLarge {
    lnum: 0,
    bol: 0,
    offset: usize::max_value(),
};

impl FilePosLarge {
    #[inline]
    pub const fn make_dummy() -> Self {
        DUMMY
    }

    #[inline]
    pub fn is_dummy(self) -> bool {
        self == DUMMY
    }

    #[inline]
    pub const fn beg_of_file(self) -> Self {
        FilePosLarge {
            lnum: 1,
            bol: 0,
            offset: 0,
        }
    }

    // constructors

    #[inline]
    pub const fn from_line_column_offset(line: usize, column: usize, offset: usize) -> Self {
        FilePosLarge {
            lnum: line,
            bol: offset - column,
            offset,
        }
    }

    #[inline]
    pub const fn from_lnum_bol_offset(lnum: usize, bol: usize, offset: usize) -> Self {
        FilePosLarge { lnum, bol, offset }
    }

    // accessors

    #[inline]
    pub const fn line(self) -> usize {
        self.lnum
    }

    #[inline]
    pub const fn column(self) -> usize {
        self.offset - self.bol
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
            offset: self.bol + col,
        }
    }

    #[inline]
    pub const fn line_beg(self) -> (usize, usize) {
        (self.lnum, self.bol)
    }

    #[inline]
    pub const fn line_column(self) -> (usize, usize) {
        (self.lnum, self.offset - self.bol)
    }

    #[inline]
    pub const fn line_column_offset(self) -> (usize, usize, usize) {
        (self.lnum, self.offset - self.bol, self.offset)
    }

    #[inline]
    pub const fn line_beg_offset(self) -> (usize, usize, usize) {
        (self.lnum, self.bol, self.offset)
    }
}

impl FilePos for FilePosLarge {
    #[inline]
    fn offset(&self) -> usize {
        self.offset
    }

    #[inline]
    fn line_column_beg(&self) -> (usize, usize, usize) {
        (self.lnum, self.offset - self.bol, self.bol)
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

impl From<FilePosSmall> for FilePosLarge {
    fn from(pos: FilePosSmall) -> Self {
        let (lnum, bol, offset) = pos.line_beg_offset();
        Self::from_lnum_bol_offset(lnum, bol, offset)
    }
}

impl ToOcamlRep for FilePosLarge {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut block = alloc.block_with_size(3);
        alloc.set_field(&mut block, 0, alloc.add(&self.lnum));
        alloc.set_field(&mut block, 1, alloc.add(&self.bol));
        alloc.set_field(&mut block, 2, alloc.add_copy(self.offset as isize));
        block.build()
    }
}

impl FromOcamlRep for FilePosLarge {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 3)?;
        let lnum = ocamlrep::from::field(block, 0)?;
        let bol = ocamlrep::from::field(block, 1)?;
        let offset: isize = ocamlrep::from::field(block, 2)?;
        Ok(Self {
            lnum,
            bol,
            offset: offset as usize,
        })
    }
}

impl<'a> FromOcamlRepIn<'a> for FilePosLarge {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a ocamlrep::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        Self::from_ocamlrep(value)
    }
}
