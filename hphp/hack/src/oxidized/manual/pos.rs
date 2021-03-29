// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::{borrow::Cow, cmp::Ordering, ops::Range, path::PathBuf, result::Result::*};

use ocamlrep::rc::RcOc;
use ocamlrep_derive::{FromOcamlRep, FromOcamlRepIn, ToOcamlRep};
use serde::{Deserialize, Serialize};

use crate::file_pos::FilePos;
use crate::file_pos_large::FilePosLarge;
use crate::file_pos_small::FilePosSmall;
use crate::pos_span_raw::PosSpanRaw;
use crate::pos_span_tiny::PosSpanTiny;
use crate::relative_path::{Prefix, RelativePath};

#[derive(
    Clone,
    Debug,
    Deserialize,
    Hash,
    FromOcamlRep,
    FromOcamlRepIn,
    ToOcamlRep,
    Serialize
)]
enum PosImpl {
    Small {
        file: RcOc<RelativePath>,
        start: FilePosSmall,
        end: FilePosSmall,
    },
    Large {
        file: RcOc<RelativePath>,
        start: Box<FilePosLarge>,
        end: Box<FilePosLarge>,
    },
    Tiny {
        file: RcOc<RelativePath>,
        span: PosSpanTiny,
    },
    FromReason(Box<PosImpl>),
}

use PosImpl::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Hash,
    FromOcamlRep,
    FromOcamlRepIn,
    ToOcamlRep,
    Serialize
)]
pub struct Pos(PosImpl);

pub type PosR<'a> = &'a Pos;

impl Pos {
    pub fn make_none() -> Self {
        // TODO: shiqicao make NONE static, lazy_static doesn't allow Rc
        Pos(PosImpl::Tiny {
            file: RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(""))),
            span: PosSpanTiny::make_dummy(),
        })
    }

    pub fn is_none(&self) -> bool {
        match self {
            Pos(PosImpl::Tiny { file, span }) => span.is_dummy() && file.is_empty(),
            _ => false,
        }
    }

    // Validness based on HHVM's definition
    pub fn is_valid(&self) -> bool {
        let (line0, line1, char0, char1) = self.info_pos_extended();
        line0 != 1 || char0 != 1 || line1 != 1 || char1 != 1
    }

    fn from_raw_span(file: RcOc<RelativePath>, span: PosSpanRaw) -> Self {
        if let Some(span) = PosSpanTiny::make(&span.start, &span.end) {
            return Pos(Tiny { file, span });
        }
        let (lnum, bol, cnum) = span.start.line_beg_offset();
        if let Some(start) = FilePosSmall::from_lnum_bol_cnum(lnum, bol, cnum) {
            let (lnum, bol, cnum) = span.end.line_beg_offset();
            if let Some(end) = FilePosSmall::from_lnum_bol_cnum(lnum, bol, cnum) {
                return Pos(Small { file, start, end });
            }
        }
        Pos(Large {
            file,
            start: Box::new(span.start),
            end: Box::new(span.end),
        })
    }

    fn to_raw_span(&self) -> PosSpanRaw {
        match &self.0 {
            Tiny { span, .. } => span.to_raw_span(),
            &Small { start, end, .. } => PosSpanRaw {
                start: start.into(),
                end: end.into(),
            },
            Large { start, end, .. } => PosSpanRaw {
                start: **start,
                end: **end,
            },
            FromReason(_p) => unimplemented!(),
        }
    }

    pub fn filename(&self) -> &RelativePath {
        self.filename_rc_ref()
    }

    fn filename_rc_ref(&self) -> &RcOc<RelativePath> {
        match &self.0 {
            Small { file, .. } | Large { file, .. } | Tiny { file, .. } => &file,
            FromReason(_p) => unimplemented!(),
        }
    }

    fn into_filename(self) -> RcOc<RelativePath> {
        match self.0 {
            Small { file, .. } | Large { file, .. } | Tiny { file, .. } => file,
            FromReason(_p) => unimplemented!(),
        }
    }

    /// Returns a closed interval that's incorrect for multi-line spans.
    pub fn info_pos(&self) -> (usize, usize, usize) {
        fn compute<P: FilePos>(pos_start: &P, pos_end: &P) -> (usize, usize, usize) {
            let (line, start_minus1, bol) = pos_start.line_column_beg();
            let start = start_minus1.wrapping_add(1);
            let end_offset = pos_end.offset();
            let mut end = end_offset - bol;
            // To represent the empty interval, pos_start and pos_end are equal because
            // end_offset is exclusive. Here, it's best for error messages to the user if
            // we print characters N to N (highlighting a single character) rather than characters
            // N to (N-1), which is very unintuitive.
            if end == start_minus1 {
                end = start
            }
            (line, start, end)
        }
        match &self.0 {
            Small { start, end, .. } => compute(start, end),
            Large { start, end, .. } => compute(start.as_ref(), end.as_ref()),
            Tiny { span, .. } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                compute(&start, &end)
            }
            FromReason(_p) => unimplemented!(),
        }
    }

    pub fn info_pos_extended(&self) -> (usize, usize, usize, usize) {
        let (line_begin, start, end) = self.info_pos();
        let line_end = match &self.0 {
            Small { end, .. } => end.line_column_beg(),
            Large { end, .. } => (*end).line_column_beg(),
            Tiny { span, .. } => span.to_raw_span().end.line_column_beg(),
            FromReason(_p) => unimplemented!(),
        }
        .0;
        (line_begin, line_end, start, end)
    }

    pub fn info_raw(&self) -> (usize, usize) {
        (self.start_cnum(), self.end_cnum())
    }

    pub fn line(&self) -> usize {
        match &self.0 {
            Small { start, .. } => start.line(),
            Large { start, .. } => start.line(),
            Tiny { span, .. } => span.start_line_number(),
            FromReason(_p) => unimplemented!(),
        }
    }

    pub fn from_lnum_bol_cnum(
        file: RcOc<RelativePath>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> Self {
        let (start_line, start_bol, start_cnum) = start;
        let (end_line, end_bol, end_cnum) = end;
        let start = FilePosLarge::from_lnum_bol_cnum(start_line, start_bol, start_cnum);
        let end = FilePosLarge::from_lnum_bol_cnum(end_line, end_bol, end_cnum);
        Self::from_raw_span(file, PosSpanRaw { start, end })
    }

    pub fn to_start_and_end_lnum_bol_cnum(&self) -> ((usize, usize, usize), (usize, usize, usize)) {
        match &self.0 {
            Small { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            Large { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            Tiny { span, .. } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                (start.line_beg_offset(), end.line_beg_offset())
            }
            FromReason(_p) => unimplemented!(),
        }
    }

    /// For single-line spans only.
    pub fn from_line_cols_offset(
        file: RcOc<RelativePath>,
        line: usize,
        cols: Range<usize>,
        start_offset: usize,
    ) -> Self {
        let start = FilePosLarge::from_line_column_offset(line, cols.start, start_offset);
        let end = FilePosLarge::from_line_column_offset(
            line,
            cols.end,
            start_offset + (cols.end - cols.start),
        );
        Self::from_raw_span(file, PosSpanRaw { start, end })
    }

    pub fn btw_nocheck(x1: Self, x2: Self) -> Self {
        let start = x1.to_raw_span().start;
        let end = x2.to_raw_span().end;
        Self::from_raw_span(x1.into_filename(), PosSpanRaw { start, end })
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

    pub fn merge(x1: &Self, x2: &Self) -> Result<Self, String> {
        if x1.filename() != x2.filename() {
            // see comment above (T52404885)
            return Err(String::from("Position in separate files ")
                + &x1.filename().to_string()
                + " and "
                + &x2.filename().to_string());
        }

        let span1 = x1.to_raw_span();
        let span2 = x2.to_raw_span();

        let start = if span1.start.is_dummy() {
            span2.start
        } else if span2.start.is_dummy() {
            span1.start
        } else if span1.start.offset() < span2.start.offset() {
            span1.start
        } else {
            span2.start
        };

        let end = if span1.end.is_dummy() {
            span2.end
        } else if span2.end.is_dummy() {
            span1.end
        } else if span1.end.offset() < span2.end.offset() {
            span2.end
        } else {
            span1.end
        };

        Ok(Self::from_raw_span(
            RcOc::clone(x1.filename_rc_ref()),
            PosSpanRaw { start, end },
        ))
    }

    pub fn last_char(&self) -> Cow<Self> {
        if self.is_none() {
            Cow::Borrowed(self)
        } else {
            let end = self.to_raw_span().end;
            Cow::Owned(Self::from_raw_span(
                RcOc::clone(self.filename_rc_ref()),
                PosSpanRaw { start: end, end },
            ))
        }
    }

    pub fn first_char_of_line(&self) -> Cow<Self> {
        if self.is_none() {
            Cow::Borrowed(self)
        } else {
            let start = self.to_raw_span().start.with_column(0);
            Cow::Owned(Self::from_raw_span(
                RcOc::clone(self.filename_rc_ref()),
                PosSpanRaw { start, end: start },
            ))
        }
    }

    pub fn end_cnum(&self) -> usize {
        match &self.0 {
            Small { end, .. } => end.offset(),
            Large { end, .. } => end.offset(),
            Tiny { span, .. } => span.end_character_number(),
            FromReason(_p) => unimplemented!(),
        }
    }

    pub fn start_cnum(&self) -> usize {
        match &self.0 {
            Small { start, .. } => start.offset(),
            Large { start, .. } => start.offset(),
            Tiny { span, .. } => span.start_character_number(),
            FromReason(_p) => unimplemented!(),
        }
    }
}

impl std::fmt::Display for Pos {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn do_fmt<P: FilePos>(
            f: &mut std::fmt::Formatter<'_>,
            file: &RelativePath,
            start: &P,
            end: &P,
        ) -> std::fmt::Result {
            write!(f, "{}", file)?;
            let (start_line, start_col, _) = start.line_column_beg();
            let (end_line, end_col, _) = end.line_column_beg();
            if start_line == end_line {
                write!(f, "({}:{}-{})", start_line, start_col, end_col)
            } else {
                write!(f, "({}:{}-{}:{})", start_line, start_col, end_line, end_col)
            }
        }
        match &self.0 {
            Small {
                file, start, end, ..
            } => do_fmt(f, file, start, end),
            Large {
                file, start, end, ..
            } => do_fmt(f, file, &**start, &**end),
            Tiny { file, span } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                do_fmt(f, file, &start, &end)
            }
            FromReason(_p) => unimplemented!(),
        }
    }
}

impl Ord for Pos {
    // Intended to match the implementation of `Pos.compare` in OCaml.
    fn cmp(&self, other: &Pos) -> Ordering {
        self.filename()
            .cmp(other.filename())
            .then(self.start_cnum().cmp(&other.start_cnum()))
            .then(self.end_cnum().cmp(&other.end_cnum()))
    }
}

impl PartialOrd for Pos {
    fn partial_cmp(&self, other: &Pos) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for Pos {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == Ordering::Equal
    }
}

impl Eq for Pos {}

impl Pos {
    /// Returns a struct implementing Display which produces the same format as
    /// `Pos.string` in OCaml.
    pub fn string(&self) -> PosString {
        PosString(self)
    }
}

/// This struct has an impl of Display which produces the same format as
/// `Pos.string` in OCaml.
pub struct PosString<'a>(&'a Pos);

impl std::fmt::Display for PosString<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let (line, start, end) = self.0.info_pos();
        write!(
            f,
            "File {:?}, line {}, characters {}-{}:",
            self.0.filename().path(),
            line,
            start,
            end
        )
    }
}

// NoPosHash is meant to be position-insensitive, so don't do anything!
impl no_pos_hash::NoPosHash for Pos {
    fn hash<H: std::hash::Hasher>(&self, _state: &mut H) {}
}

pub mod map {
    pub type Map<T> = std::collections::BTreeMap<super::Pos, T>;
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    fn make_pos(name: &str, start: (usize, usize, usize), end: (usize, usize, usize)) -> Pos {
        Pos::from_lnum_bol_cnum(
            RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(name))),
            start,
            end,
        )
    }

    #[test]
    fn test_pos() {
        assert_eq!(Pos::make_none().is_none(), true);
        assert_eq!(
            Pos::from_lnum_bol_cnum(
                RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from("a"))),
                (0, 0, 0),
                (0, 0, 0)
            )
            .is_none(),
            false
        );
        assert_eq!(
            Pos::from_lnum_bol_cnum(
                RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(""))),
                (1, 0, 0),
                (0, 0, 0)
            )
            .is_none(),
            false
        );
    }

    #[test]
    fn test_pos_string() {
        assert_eq!(
            Pos::make_none().string().to_string(),
            r#"File "", line 0, characters 0-0:"#
        );
        let path = RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from("a.php")));
        assert_eq!(
            Pos::from_lnum_bol_cnum(path, (5, 100, 117), (5, 100, 142))
                .string()
                .to_string(),
            r#"File "a.php", line 5, characters 18-42:"#
        );
    }

    #[test]
    fn test_pos_merge() {
        let test = |name, (exp_start, exp_end), ((fst_start, fst_end), (snd_start, snd_end))| {
            assert_eq!(
                Ok(make_pos("a", exp_start, exp_end)),
                Pos::merge(
                    &make_pos("a", fst_start, fst_end),
                    &make_pos("a", snd_start, snd_end)
                ),
                "{}",
                name
            );

            // Run this again because we want to test that we get the same
            // result regardless of order.
            assert_eq!(
                Ok(make_pos("a", exp_start, exp_end)),
                Pos::merge(
                    &make_pos("a", snd_start, snd_end),
                    &make_pos("a", fst_start, fst_end),
                ),
                "{} (reversed)",
                name
            );
        };

        test(
            "basic test",
            ((0, 0, 0), (0, 0, 5)),
            (((0, 0, 0), (0, 0, 2)), ((0, 0, 2), (0, 0, 5))),
        );

        test(
            "merge should work with gaps",
            ((0, 0, 0), (0, 0, 15)),
            (((0, 0, 0), (0, 0, 5)), ((0, 0, 10), (0, 0, 15))),
        );

        test(
            "merge should work with overlaps",
            ((0, 0, 0), (0, 0, 15)),
            (((0, 0, 0), (0, 0, 12)), ((0, 0, 7), (0, 0, 15))),
        );

        test(
            "merge should work between lines",
            ((0, 0, 0), (2, 20, 25)),
            (((0, 0, 0), (1, 10, 15)), ((1, 10, 20), (2, 20, 25))),
        );

        assert_eq!(
            Err("Position in separate files |a and |b".to_string()),
            Pos::merge(
                &make_pos("a", (0, 0, 0), (0, 0, 0)),
                &make_pos("b", (0, 0, 0), (0, 0, 0))
            ),
            "should reject merges with different filenames"
        );
    }
}
