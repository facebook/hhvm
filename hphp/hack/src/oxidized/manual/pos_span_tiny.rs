// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::fmt;

use ocamlrep::{FromOcamlRep, FromOcamlRepIn, ToOcamlRep};
use serde::{Deserialize, Serialize};
use static_assertions::const_assert_eq;

use crate::file_pos::FilePos;
use crate::file_pos_large::FilePosLarge;
use crate::pos_span_raw::PosSpanRaw;

// PosSpanTiny packs multiple fields into one 63-bit integer:
//
//    6         5         4         3         2         1         0
// 3210987654321098765432109876543210987654321098765432109876543210
// X<-------------------><--------------><------------------><---->
//   byte offset of line   line number      column number     width

/// A compressed representation of a position span, i.e. a start and an end position.
#[derive(Copy, Clone, Eq, PartialEq, Deserialize, Hash, Serialize)]
pub struct PosSpanTiny(usize);

arena_deserializer::impl_deserialize_in_arena!(PosSpanTiny);

/// These numbers were obtained by gathering statistics on the positions in
/// the decl heap for a large code base run as of December 2020. They should
/// allow us to encode about 99% of positions.
const START_BEGINNING_OF_LINE_BITS: usize = 21;
const START_LINE_NUMBER_BITS: usize = 16;
const START_COLUMN_NUMBER_BITS: usize = 20;
const BEGINNING_OF_LINE_INCREMENT_BITS: usize = 0;
const LINE_NUMBER_INCREMENT_BITS: usize = 0;
const WIDTH_BITS: usize = 6;

// The offset of each field (i.e., the number of bits to the right of it) is
// the offset of the field to the right plus that field's bit width.
const WIDTH_OFFSET: usize = 0;
const LINE_NUMBER_INCREMENT_OFFSET: usize = WIDTH_OFFSET + WIDTH_BITS;
const BEGINNING_OF_LINE_INCREMENT_OFFSET: usize =
    LINE_NUMBER_INCREMENT_OFFSET + LINE_NUMBER_INCREMENT_BITS;
const START_COLUMN_NUMBER_OFFSET: usize =
    BEGINNING_OF_LINE_INCREMENT_OFFSET + BEGINNING_OF_LINE_INCREMENT_BITS;
const START_LINE_NUMBER_OFFSET: usize = START_COLUMN_NUMBER_OFFSET + START_COLUMN_NUMBER_BITS;
const START_BEGINNING_OF_LINE_OFFSET: usize = START_LINE_NUMBER_OFFSET + START_LINE_NUMBER_BITS;

// The total number of bits used must be 63 (OCaml reserves one bit).
const_assert_eq!(
    63,
    START_BEGINNING_OF_LINE_BITS + START_BEGINNING_OF_LINE_OFFSET
);

#[inline]
const fn mask(bits: usize) -> usize {
    (1 << bits) - 1
}

#[inline]
const fn mask_by(bits: usize, x: usize) -> usize {
    x & mask(bits)
}

const MAX_START_BEGINNING_OF_LINE: usize = mask(START_BEGINNING_OF_LINE_BITS);
const MAX_START_LINE_NUMBER: usize = mask(START_LINE_NUMBER_BITS);
const MAX_START_COLUMN_NUMBER: usize = mask(START_COLUMN_NUMBER_BITS);
const MAX_BEGINNING_OF_LINE_INCREMENT: usize = mask(BEGINNING_OF_LINE_INCREMENT_BITS);
const MAX_LINE_NUMBER_INCREMENT: usize = mask(LINE_NUMBER_INCREMENT_BITS);
const MAX_WIDTH: usize = mask(WIDTH_BITS);

const DUMMY: usize = usize::max_value();

impl PosSpanTiny {
    #[inline]
    pub const fn make_dummy() -> Self {
        Self(DUMMY)
    }

    #[inline]
    pub const fn is_dummy(&self) -> bool {
        self.0 == DUMMY
    }

    pub fn make(start: &FilePosLarge, end: &FilePosLarge) -> Option<Self> {
        if start.is_dummy() || end.is_dummy() {
            return Some(Self::make_dummy());
        }
        let start_bol = start.beg_of_line();
        let start_line = start.line();
        let start_col = start.column();
        let start_cnum = start.offset();
        let end_bol = end.beg_of_line();
        let end_line = end.line();
        let end_cnum = end.offset();
        let bol_increment = end_bol.checked_sub(start_bol)?;
        let line_increment = end_line.checked_sub(start_line)?;
        let width = end_cnum.checked_sub(start_cnum)?;
        if start_bol > MAX_START_BEGINNING_OF_LINE
            || start_line > MAX_START_LINE_NUMBER
            || start_col > MAX_START_COLUMN_NUMBER
            || bol_increment > MAX_BEGINNING_OF_LINE_INCREMENT
            || line_increment > MAX_LINE_NUMBER_INCREMENT
            || width > MAX_WIDTH
        {
            return None;
        }
        Some(Self(
            start_bol << START_BEGINNING_OF_LINE_OFFSET
                | start_line << START_LINE_NUMBER_OFFSET
                | start_col << START_COLUMN_NUMBER_OFFSET
                | bol_increment << BEGINNING_OF_LINE_INCREMENT_OFFSET
                | line_increment << LINE_NUMBER_INCREMENT_OFFSET
                | width << WIDTH_OFFSET,
        ))
    }

    pub fn start_beginning_of_line(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(
                START_BEGINNING_OF_LINE_BITS,
                self.0 >> START_BEGINNING_OF_LINE_OFFSET,
            )
        }
    }

    pub fn start_line_number(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(START_LINE_NUMBER_BITS, self.0 >> START_LINE_NUMBER_OFFSET)
        }
    }

    pub fn start_column(self) -> usize {
        if self.is_dummy() {
            DUMMY
        } else {
            mask_by(
                START_COLUMN_NUMBER_BITS,
                self.0 >> START_COLUMN_NUMBER_OFFSET,
            )
        }
    }

    fn beginning_of_line_increment(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(
                BEGINNING_OF_LINE_INCREMENT_BITS,
                self.0 >> BEGINNING_OF_LINE_INCREMENT_OFFSET,
            )
        }
    }

    fn line_number_increment(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(
                LINE_NUMBER_INCREMENT_BITS,
                self.0 >> LINE_NUMBER_INCREMENT_OFFSET,
            )
        }
    }

    fn width(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(WIDTH_BITS, self.0 >> WIDTH_OFFSET)
        }
    }

    pub fn start_character_number(self) -> usize {
        self.start_beginning_of_line() + self.start_column()
    }

    pub fn end_line_number(self) -> usize {
        self.start_line_number() + self.line_number_increment()
    }

    pub fn end_beginning_of_line(self) -> usize {
        self.start_beginning_of_line() + self.beginning_of_line_increment()
    }

    pub fn end_character_number(self) -> usize {
        self.start_character_number() + self.width()
    }

    pub fn end_column(self) -> usize {
        self.end_character_number() - self.end_beginning_of_line()
    }

    pub fn to_raw_span(self) -> PosSpanRaw {
        if self.is_dummy() {
            PosSpanRaw::make_dummy()
        } else {
            let start_lnum = self.start_line_number();
            let start_bol = self.start_beginning_of_line();
            let start_cnum = self.start_character_number();
            let end_lnum = self.end_line_number();
            let end_bol = self.end_beginning_of_line();
            let end_cnum = self.end_character_number();
            PosSpanRaw {
                start: FilePosLarge::from_lnum_bol_cnum(start_lnum, start_bol, start_cnum),
                end: FilePosLarge::from_lnum_bol_cnum(end_lnum, end_bol, end_cnum),
            }
        }
    }
}

impl Ord for PosSpanTiny {
    // Intended to match the implementation of `Pos.compare` in OCaml.
    fn cmp(&self, other: &Self) -> Ordering {
        self.start_character_number()
            .cmp(&other.start_character_number())
            .then(
                self.end_character_number()
                    .cmp(&other.end_character_number()),
            )
    }
}

impl PartialOrd for PosSpanTiny {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl ToOcamlRep for PosSpanTiny {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        ocamlrep::OpaqueValue::int(self.0 as isize)
    }
}

impl FromOcamlRep for PosSpanTiny {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Self(ocamlrep::from::expect_int(value)? as usize))
    }
}

impl<'a> FromOcamlRepIn<'a> for PosSpanTiny {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a ocamlrep::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        Self::from_ocamlrep(value)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_tiny_small() {
        let line = 30;
        let bol = 2349;
        let start_cnum = 2398;
        let end_cnum = 2450;
        let start = FilePosLarge::from_lnum_bol_cnum(line, bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(line, bol, end_cnum);
        match PosSpanTiny::make(&start, &end) {
            None => assert_eq!(true, false),
            Some(span) => {
                assert_eq!(line, span.start_line_number());
                assert_eq!(line, span.end_line_number());
                assert_eq!(bol, span.start_beginning_of_line());
                assert_eq!(bol, span.end_beginning_of_line());
                assert_eq!(start_cnum, span.start_character_number());
                assert_eq!(end_cnum, span.end_character_number());
                assert_eq!((start_cnum - bol), span.start_column());
                assert_eq!((end_cnum - bol), span.end_column());
                let PosSpanRaw {
                    start: start_,
                    end: end_,
                } = span.to_raw_span();
                assert_eq!(start, start_);
                assert_eq!(end, end_);
            }
        }
    }

    #[test]
    fn test_tiny_multiline_span() {
        let start_line = 30;
        let start_bol = 2349;
        let end_line = 33;
        let end_bol = 2500;
        let start_cnum = 2398;
        let end_cnum = 2600;
        let start = FilePosLarge::from_lnum_bol_cnum(start_line, start_bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(end_line, end_bol, end_cnum);
        match PosSpanTiny::make(&start, &end) {
            None => {}
            Some(span) => {
                assert_eq!(start_line, span.start_line_number());
                assert_eq!(end_line, span.end_line_number());
                assert_eq!(start_bol, span.start_beginning_of_line());
                assert_eq!(end_bol, span.end_beginning_of_line());
                assert_eq!(start_cnum, span.start_character_number());
                assert_eq!(end_cnum, span.end_character_number());
                assert_eq!((start_cnum - start_bol), span.start_column());
                assert_eq!((end_cnum - end_bol), span.end_column());
                let PosSpanRaw {
                    start: start_,
                    end: end_,
                } = span.to_raw_span();
                assert_eq!(start, start_);
                assert_eq!(end, end_);
            }
        }
    }

    #[test]
    fn test_tiny_negative_span() {
        let start_line = 30;
        let start_bol = 2349;
        let end_line = 23;
        let end_bol = 2000;
        let start_cnum = 2398;
        let end_cnum = 2003;
        let start = FilePosLarge::from_lnum_bol_cnum(start_line, start_bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(end_line, end_bol, end_cnum);
        match PosSpanTiny::make(&start, &end) {
            None => {}
            Some(span) => {
                assert_eq!(start_line, span.start_line_number());
                assert_eq!(end_line, span.end_line_number());
                assert_eq!(start_bol, span.start_beginning_of_line());
                assert_eq!(end_bol, span.end_beginning_of_line());
                assert_eq!(start_cnum, span.start_character_number());
                assert_eq!(end_cnum, span.end_character_number());
                assert_eq!((start_cnum - start_bol), span.start_column());
                assert_eq!((end_cnum - end_bol), span.end_column());
                let PosSpanRaw {
                    start: start_,
                    end: end_,
                } = span.to_raw_span();
                assert_eq!(start, start_);
                assert_eq!(end, end_);
            }
        }
    }

    #[test]
    fn test_tiny_dummy() {
        let line = 0;
        let bol = 0;
        let start_cnum = usize::max_value();
        let end_cnum = usize::max_value();
        let start = FilePosLarge::from_lnum_bol_cnum(line, bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(line, bol, end_cnum);
        let span = PosSpanTiny::make_dummy();
        assert_eq!(line, span.start_line_number());
        assert_eq!(line, span.end_line_number());
        assert_eq!(bol, span.start_beginning_of_line());
        assert_eq!(bol, span.end_beginning_of_line());
        assert_eq!(start_cnum, span.start_character_number());
        assert_eq!(end_cnum, span.end_character_number());
        assert_eq!((start_cnum - bol), span.start_column());
        assert_eq!((end_cnum - bol), span.end_column());
        let PosSpanRaw {
            start: start_,
            end: end_,
        } = span.to_raw_span();
        assert_eq!(start, start_);
        assert_eq!(end, end_);
    }

    #[test]
    fn test_tiny_large() {
        let max_int = usize::max_value();
        let line = max_int;
        let bol = max_int;
        let start_cnum = max_int;
        let end_cnum = max_int;
        let start = FilePosLarge::from_lnum_bol_cnum(line, bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(line, bol, end_cnum);
        match PosSpanTiny::make(&start, &end) {
            None => {
                // expected
            }
            Some(span) => {
                // will likely fail here
                assert_eq!(line, span.start_line_number());
                assert_eq!(line, span.end_line_number());
                assert_eq!(bol, span.start_beginning_of_line());
                assert_eq!(bol, span.end_beginning_of_line());
                assert_eq!(start_cnum, span.start_character_number());
                assert_eq!(end_cnum, span.end_character_number());
                assert_eq!((start_cnum - bol), span.start_column());
                assert_eq!((end_cnum - bol), span.end_column());
                let PosSpanRaw {
                    start: start_,
                    end: end_,
                } = span.to_raw_span();
                assert_eq!(start, start_);
                assert_eq!(end, end_);
            }
        }
    }
}

impl fmt::Debug for PosSpanTiny {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "PosSpanTiny {{ from {}:{} to {}:{} }}",
            self.start_line_number(),
            self.start_column(),
            self.end_line_number(),
            self.end_column()
        )
    }
}
