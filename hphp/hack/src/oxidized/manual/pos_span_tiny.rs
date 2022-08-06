// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::fmt;

use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
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
pub struct PosSpanTiny(u64);

arena_deserializer::impl_deserialize_in_arena!(PosSpanTiny);

/// These numbers were obtained by gathering statistics on the positions in
/// the decl heap for a large code base run as of December 2020. They should
/// allow us to encode about 99% of positions.
const START_BEGINNING_OF_LINE_BITS: u64 = 21;
const START_LINE_NUMBER_BITS: u64 = 16;
const START_COLUMN_NUMBER_BITS: u64 = 20;
const BEGINNING_OF_LINE_INCREMENT_BITS: u64 = 0;
const LINE_NUMBER_INCREMENT_BITS: u64 = 0;
const WIDTH_BITS: u64 = 6;

// The offset of each field (i.e., the number of bits to the right of it) is
// the offset of the field to the right plus that field's bit width.
const WIDTH_OFFSET: u64 = 0;
const LINE_NUMBER_INCREMENT_OFFSET: u64 = WIDTH_OFFSET + WIDTH_BITS;
const BEGINNING_OF_LINE_INCREMENT_OFFSET: u64 =
    LINE_NUMBER_INCREMENT_OFFSET + LINE_NUMBER_INCREMENT_BITS;
const START_COLUMN_NUMBER_OFFSET: u64 =
    BEGINNING_OF_LINE_INCREMENT_OFFSET + BEGINNING_OF_LINE_INCREMENT_BITS;
const START_LINE_NUMBER_OFFSET: u64 = START_COLUMN_NUMBER_OFFSET + START_COLUMN_NUMBER_BITS;
const START_BEGINNING_OF_LINE_OFFSET: u64 = START_LINE_NUMBER_OFFSET + START_LINE_NUMBER_BITS;

// The total number of bits used must be 63 (OCaml reserves one bit).
const_assert_eq!(
    63,
    START_BEGINNING_OF_LINE_BITS + START_BEGINNING_OF_LINE_OFFSET
);

#[inline]
const fn mask(bits: u64) -> u64 {
    (1 << bits) - 1
}

#[inline]
const fn mask_by(bits: u64, x: u64) -> u64 {
    x & mask(bits)
}

const MAX_START_BEGINNING_OF_LINE: u64 = mask(START_BEGINNING_OF_LINE_BITS);
const MAX_START_LINE_NUMBER: u64 = mask(START_LINE_NUMBER_BITS);
const MAX_START_COLUMN_NUMBER: u64 = mask(START_COLUMN_NUMBER_BITS);
const MAX_BEGINNING_OF_LINE_INCREMENT: u64 = mask(BEGINNING_OF_LINE_INCREMENT_BITS);
const MAX_LINE_NUMBER_INCREMENT: u64 = mask(LINE_NUMBER_INCREMENT_BITS);
const MAX_WIDTH: u64 = mask(WIDTH_BITS);

const DUMMY: u64 = u64::max_value();

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
        let start_bol = start.beg_of_line() as u64;
        let start_line = start.line() as u64;
        let start_col = start.column() as u64;
        let start_offset = start.offset() as u64;
        let end_bol = end.beg_of_line() as u64;
        let end_line = end.line() as u64;
        let end_offset = end.offset() as u64;
        let bol_increment = end_bol.checked_sub(start_bol)? as u64;
        let line_increment = end_line.checked_sub(start_line)? as u64;
        let width = end_offset.checked_sub(start_offset)? as u64;
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
            ) as usize
        }
    }

    pub fn start_line_number(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(START_LINE_NUMBER_BITS, self.0 >> START_LINE_NUMBER_OFFSET) as usize
        }
    }

    pub fn start_column(self) -> usize {
        if self.is_dummy() {
            DUMMY as usize
        } else {
            mask_by(
                START_COLUMN_NUMBER_BITS,
                self.0 >> START_COLUMN_NUMBER_OFFSET,
            ) as usize
        }
    }

    fn beginning_of_line_increment(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(
                BEGINNING_OF_LINE_INCREMENT_BITS,
                self.0 >> BEGINNING_OF_LINE_INCREMENT_OFFSET,
            ) as usize
        }
    }

    fn line_number_increment(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(
                LINE_NUMBER_INCREMENT_BITS,
                self.0 >> LINE_NUMBER_INCREMENT_OFFSET,
            ) as usize
        }
    }

    fn width(self) -> usize {
        if self.is_dummy() {
            0
        } else {
            mask_by(WIDTH_BITS, self.0 >> WIDTH_OFFSET) as usize
        }
    }

    pub fn start_offset(self) -> usize {
        self.start_beginning_of_line() + self.start_column()
    }

    pub fn end_line_number(self) -> usize {
        self.start_line_number() + self.line_number_increment()
    }

    pub fn end_beginning_of_line(self) -> usize {
        self.start_beginning_of_line() + self.beginning_of_line_increment()
    }

    pub fn end_offset(self) -> usize {
        self.start_offset() + self.width()
    }

    pub fn end_column(self) -> usize {
        self.end_offset() - self.end_beginning_of_line()
    }

    pub fn to_raw_span(self) -> PosSpanRaw {
        if self.is_dummy() {
            PosSpanRaw::make_dummy()
        } else {
            let start_lnum = self.start_line_number();
            let start_bol = self.start_beginning_of_line();
            let start_offset = self.start_offset();
            let end_lnum = self.end_line_number();
            let end_bol = self.end_beginning_of_line();
            let end_offset = self.end_offset();
            PosSpanRaw {
                start: FilePosLarge::from_lnum_bol_offset(start_lnum, start_bol, start_offset),
                end: FilePosLarge::from_lnum_bol_offset(end_lnum, end_bol, end_offset),
            }
        }
    }
}

impl Ord for PosSpanTiny {
    // Intended to match the implementation of `Pos.compare` in OCaml.
    fn cmp(&self, other: &Self) -> Ordering {
        self.start_offset()
            .cmp(&other.start_offset())
            .then(self.end_offset().cmp(&other.end_offset()))
    }
}

impl PartialOrd for PosSpanTiny {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl ToOcamlRep for PosSpanTiny {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        _alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
        ocamlrep::OpaqueValue::int(self.0 as isize)
    }
}

impl FromOcamlRep for PosSpanTiny {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Self(ocamlrep::from::expect_int(value)? as u64))
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
    use pretty_assertions::assert_eq;

    use super::*;

    #[test]
    fn test_tiny_small() {
        let line = 30;
        let bol = 2349;
        let start_offset = 2398;
        let end_offset = 2450;
        let start = FilePosLarge::from_lnum_bol_offset(line, bol, start_offset);
        let end = FilePosLarge::from_lnum_bol_offset(line, bol, end_offset);
        match PosSpanTiny::make(&start, &end) {
            None => assert_eq!(true, false),
            Some(span) => {
                assert_eq!(line, span.start_line_number());
                assert_eq!(line, span.end_line_number());
                assert_eq!(bol, span.start_beginning_of_line());
                assert_eq!(bol, span.end_beginning_of_line());
                assert_eq!(start_offset, span.start_offset());
                assert_eq!(end_offset, span.end_offset());
                assert_eq!((start_offset - bol), span.start_column());
                assert_eq!((end_offset - bol), span.end_column());
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
        let start_offset = 2398;
        let end_offset = 2600;
        let start = FilePosLarge::from_lnum_bol_offset(start_line, start_bol, start_offset);
        let end = FilePosLarge::from_lnum_bol_offset(end_line, end_bol, end_offset);
        match PosSpanTiny::make(&start, &end) {
            None => {}
            Some(span) => {
                assert_eq!(start_line, span.start_line_number());
                assert_eq!(end_line, span.end_line_number());
                assert_eq!(start_bol, span.start_beginning_of_line());
                assert_eq!(end_bol, span.end_beginning_of_line());
                assert_eq!(start_offset, span.start_offset());
                assert_eq!(end_offset, span.end_offset());
                assert_eq!((start_offset - start_bol), span.start_column());
                assert_eq!((end_offset - end_bol), span.end_column());
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
        let start_offset = 2398;
        let end_offset = 2003;
        let start = FilePosLarge::from_lnum_bol_offset(start_line, start_bol, start_offset);
        let end = FilePosLarge::from_lnum_bol_offset(end_line, end_bol, end_offset);
        match PosSpanTiny::make(&start, &end) {
            None => {}
            Some(span) => {
                assert_eq!(start_line, span.start_line_number());
                assert_eq!(end_line, span.end_line_number());
                assert_eq!(start_bol, span.start_beginning_of_line());
                assert_eq!(end_bol, span.end_beginning_of_line());
                assert_eq!(start_offset, span.start_offset());
                assert_eq!(end_offset, span.end_offset());
                assert_eq!((start_offset - start_bol), span.start_column());
                assert_eq!((end_offset - end_bol), span.end_column());
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
        let line: u64 = 0;
        let bol: u64 = 0;
        let start_offset = u64::max_value();
        let end_offset = u64::max_value();
        let start =
            FilePosLarge::from_lnum_bol_offset(line as usize, bol as usize, start_offset as usize);
        let end =
            FilePosLarge::from_lnum_bol_offset(line as usize, bol as usize, end_offset as usize);
        let span = PosSpanTiny::make_dummy();
        assert_eq!(line, span.start_line_number() as u64);
        assert_eq!(line, span.end_line_number() as u64);
        assert_eq!(bol, span.start_beginning_of_line() as u64);
        assert_eq!(bol, span.end_beginning_of_line() as u64);
        assert_eq!(start_offset, span.start_offset() as u64);
        assert_eq!(end_offset, span.end_offset() as u64);
        assert_eq!((start_offset - bol), span.start_column() as u64);
        assert_eq!((end_offset - bol), span.end_column() as u64);
        let PosSpanRaw {
            start: start_,
            end: end_,
        } = span.to_raw_span();
        assert_eq!(start, start_);
        assert_eq!(end, end_);
    }

    #[test]
    fn test_tiny_large() {
        let max_int = u64::max_value();
        let line = max_int;
        let bol = max_int;
        let start_offset = max_int;
        let end_offset = max_int;
        let start =
            FilePosLarge::from_lnum_bol_offset(line as usize, bol as usize, start_offset as usize);
        let end =
            FilePosLarge::from_lnum_bol_offset(line as usize, bol as usize, end_offset as usize);
        match PosSpanTiny::make(&start, &end) {
            None => {
                // expected
            }
            Some(span) => {
                // will likely fail here
                assert_eq!(line, span.start_line_number() as u64);
                assert_eq!(line, span.end_line_number() as u64);
                assert_eq!(bol, span.start_beginning_of_line() as u64);
                assert_eq!(bol, span.end_beginning_of_line() as u64);
                assert_eq!(start_offset, span.start_offset() as u64);
                assert_eq!(end_offset, span.end_offset() as u64);
                assert_eq!((start_offset - bol), span.start_column() as u64);
                assert_eq!((end_offset - bol), span.end_column() as u64);
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
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
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
