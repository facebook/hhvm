// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use strum::IntoEnumIterator;
use strum_macros::EnumIter;

use ocamlrep::{FromOcamlRep, FromOcamlRepIn, ToOcamlRep};
use serde::{Deserialize, Serialize};

use crate::file_pos::FilePos;
use crate::file_pos_large::FilePosLarge;
use crate::pos_span_raw::PosSpanRaw;

/// A compressed representation of a position span, i.e. a start and an end position.

#[inline]
const fn mask_of_size(n: usize) -> usize {
    (1 << n) - 1
}

#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash, EnumIter)]
enum Field {
    /// The character number of the beginning of line of the start position of the span.
    StartBeginningOfLine,
    /// The line number of the start position of the span.
    StartLineNumber,
    /// The column number of the start position of the span.
    StartColumnNumber,
    /// The beginning of line character number of the end position of the span
    /// is given by adding this increment to the beginning of line character number of the start position.
    BeginningOfLineIncrement,
    /// The line number of the end position of the span
    /// is given by adding this increment to the line number of the start position.
    LineNumberIncrement,
    /// The column number of the end position of the span
    /// is given by adding this increment to the column number of the start position.
    ColumnNumberIncrement,
}

impl Field {
    /// These numbers were obtained by gathering statistics on the positions in the decl heap
    /// for a www run as per december 2020. They should allow to encode about 99% of positions.
    ///
    /// /!\ Always make sure the total is 63, due to OCaml reserving 1 bit.
    fn nbits(self) -> usize {
        match self {
            Self::StartBeginningOfLine => 25,
            Self::StartLineNumber => 15,
            Self::StartColumnNumber => 15,
            Self::BeginningOfLineIncrement => 0,
            Self::LineNumberIncrement => 0,
            Self::ColumnNumberIncrement => 8,
        }
    }

    fn n_rhs_bits(self) -> usize {
        Field::iter()
            .skip_while(|field| *field != self)
            .skip(1) // skip the field itself
            .map(Field::nbits)
            .sum()
    }

    fn dummy_value(self) -> usize {
        match self {
            Self::StartBeginningOfLine
            | Self::StartLineNumber
            | Self::BeginningOfLineIncrement
            | Self::LineNumberIncrement
            | Self::ColumnNumberIncrement => 0,
            Self::StartColumnNumber => usize::max_value(),
        }
    }
}

/// Multiple fields packed into one 64-bit integer, e.g. :
///
///    6         5         4         3         2         1         0
/// 3210987654321098765432109876543210987654321098765432109876543210
/// <----------------------------><----------------------><------->X
///             field1                     field2           field3
///
/// Fields are variants of Field.t, and the number of bits for each field
/// are given by Field.nbits.
#[derive(Copy, Clone, Eq, PartialEq, Debug, Deserialize, Hash, Serialize)]
pub struct PosSpanTiny(usize);

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

    fn project(self, field: Field) -> usize {
        if self.is_dummy() {
            field.dummy_value()
        } else {
            (self.0 >> field.n_rhs_bits()) & mask_of_size(field.nbits())
        }
    }

    pub fn start_line(self) -> usize {
        self.project(Field::StartLineNumber)
    }

    pub fn start_beginning_of_line(self) -> usize {
        self.project(Field::StartBeginningOfLine)
    }

    pub fn start_column(self) -> usize {
        self.project(Field::StartColumnNumber)
    }

    pub fn start_character_number(self) -> usize {
        self.start_beginning_of_line() + self.start_column()
    }

    pub fn end_line(self) -> usize {
        self.start_line() + self.project(Field::LineNumberIncrement)
    }

    pub fn end_beginning_of_line(self) -> usize {
        self.start_beginning_of_line() + self.project(Field::BeginningOfLineIncrement)
    }

    pub fn end_column(self) -> usize {
        self.start_column() + self.project(Field::ColumnNumberIncrement)
    }

    pub fn end_character_number(self) -> usize {
        self.end_beginning_of_line() + self.end_column()
    }

    pub fn to_raw_span(self) -> PosSpanRaw {
        if self.is_dummy() {
            PosSpanRaw::make_dummy()
        } else {
            let start_lnum = self.start_line();
            let start_bol = self.start_beginning_of_line();
            let start_cnum = self.start_character_number();
            let end_lnum = self.end_line();
            let end_bol = self.end_beginning_of_line();
            let end_cnum = self.end_character_number();
            PosSpanRaw {
                start: FilePosLarge::from_lnum_bol_cnum(start_lnum, start_bol, start_cnum),
                end: FilePosLarge::from_lnum_bol_cnum(end_lnum, end_bol, end_cnum),
            }
        }
    }

    fn project_raw(start: &FilePosLarge, end: &FilePosLarge, field: Field) -> usize {
        let (start_line, start_column, start_bol) = start.line_column_beg();
        let (end_line, end_column, end_bol) = end.line_column_beg();
        match field {
            Field::StartBeginningOfLine => start_bol,
            Field::StartLineNumber => start_line,
            Field::StartColumnNumber => start_column,
            Field::BeginningOfLineIncrement => usize_unchecked_sub(end_bol, start_bol),
            Field::LineNumberIncrement => usize_unchecked_sub(end_line, start_line),
            Field::ColumnNumberIncrement => usize_unchecked_sub(end_column, start_column),
        }
    }

    pub fn make(start: &FilePosLarge, end: &FilePosLarge) -> Option<Self> {
        let mut tiny = 0;
        for field in Field::iter() {
            let value = Self::project_raw(start, end, field);
            if value == value & mask_of_size(field.nbits()) {
                tiny <<= field.nbits();
                tiny |= value;
            } else {
                return None;
            }
        }
        Some(Self(tiny))
    }
}

fn usize_unchecked_sub(x: usize, y: usize) -> usize {
    if x >= y { x - y } else { usize::max_value() }
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
        let end_cnum = 2496;
        let start = FilePosLarge::from_lnum_bol_cnum(line, bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(line, bol, end_cnum);
        match PosSpanTiny::make(&start, &end) {
            None => assert_eq!(true, false),
            Some(span) => {
                assert_eq!(line, span.start_line());
                assert_eq!(line, span.end_line());
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
                assert_eq!(start_line, span.start_line());
                assert_eq!(end_line, span.end_line());
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
                assert_eq!(start_line, span.start_line());
                assert_eq!(end_line, span.end_line());
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
        assert_eq!(line, span.start_line());
        assert_eq!(line, span.end_line());
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
                assert_eq!(line, span.start_line());
                assert_eq!(line, span.end_line());
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
