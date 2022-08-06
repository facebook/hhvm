// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::cmp::Ordering;
use std::ops::Range;
use std::path::PathBuf;

use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use ocamlrep::rc::RcOc;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::file_pos::FilePos;
use crate::file_pos_large::FilePosLarge;
use crate::file_pos_small::FilePosSmall;
use crate::pos_span_raw::PosSpanRaw;
use crate::pos_span_tiny::PosSpanTiny;
use crate::relative_path::Prefix;
use crate::relative_path::RelativePath;

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

#[derive(
    Clone,
    Debug,
    Deserialize,
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

    pub fn from_raw_span(file: RcOc<RelativePath>, span: PosSpanRaw) -> Self {
        if let Some(span) = PosSpanTiny::make(&span.start, &span.end) {
            return Pos(PosImpl::Tiny { file, span });
        }
        let (lnum, bol, offset) = span.start.line_beg_offset();
        if let Some(start) = FilePosSmall::from_lnum_bol_offset(lnum, bol, offset) {
            let (lnum, bol, offset) = span.end.line_beg_offset();
            if let Some(end) = FilePosSmall::from_lnum_bol_offset(lnum, bol, offset) {
                return Pos(PosImpl::Small { file, start, end });
            }
        }
        Pos(PosImpl::Large {
            file,
            start: Box::new(span.start),
            end: Box::new(span.end),
        })
    }

    pub fn to_raw_span(&self) -> PosSpanRaw {
        match &self.0 {
            PosImpl::Tiny { span, .. } => span.to_raw_span(),
            PosImpl::Small { start, end, .. } => PosSpanRaw {
                start: (*start).into(),
                end: (*end).into(),
            },
            PosImpl::Large { start, end, .. } => PosSpanRaw {
                start: **start,
                end: **end,
            },
            PosImpl::FromReason(_p) => unimplemented!(),
        }
    }

    pub fn filename(&self) -> &RelativePath {
        self.filename_rc_ref()
    }

    fn filename_rc_ref(&self) -> &RcOc<RelativePath> {
        match &self.0 {
            PosImpl::Small { file, .. }
            | PosImpl::Large { file, .. }
            | PosImpl::Tiny { file, .. } => file,
            PosImpl::FromReason(_p) => unimplemented!(),
        }
    }

    fn into_filename(self) -> RcOc<RelativePath> {
        match self.0 {
            PosImpl::Small { file, .. }
            | PosImpl::Large { file, .. }
            | PosImpl::Tiny { file, .. } => file,
            PosImpl::FromReason(_p) => unimplemented!(),
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
            PosImpl::Small { start, end, .. } => compute(start, end),
            PosImpl::Large { start, end, .. } => compute(start.as_ref(), end.as_ref()),
            PosImpl::Tiny { span, .. } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                compute(&start, &end)
            }
            PosImpl::FromReason(_p) => unimplemented!(),
        }
    }

    pub fn info_pos_extended(&self) -> (usize, usize, usize, usize) {
        let (line_begin, start, end) = self.info_pos();
        let line_end = match &self.0 {
            PosImpl::Small { end, .. } => end.line_column_beg(),
            PosImpl::Large { end, .. } => (*end).line_column_beg(),
            PosImpl::Tiny { span, .. } => span.to_raw_span().end.line_column_beg(),
            PosImpl::FromReason(_p) => unimplemented!(),
        }
        .0;
        (line_begin, line_end, start, end)
    }

    pub fn info_raw(&self) -> (usize, usize) {
        (self.start_offset(), self.end_offset())
    }

    pub fn line(&self) -> usize {
        match &self.0 {
            PosImpl::Small { start, .. } => start.line(),
            PosImpl::Large { start, .. } => start.line(),
            PosImpl::Tiny { span, .. } => span.start_line_number(),
            PosImpl::FromReason(_p) => unimplemented!(),
        }
    }

    pub fn from_lnum_bol_offset(
        file: RcOc<RelativePath>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> Self {
        let (start_line, start_bol, start_offset) = start;
        let (end_line, end_bol, end_offset) = end;
        let start = FilePosLarge::from_lnum_bol_offset(start_line, start_bol, start_offset);
        let end = FilePosLarge::from_lnum_bol_offset(end_line, end_bol, end_offset);
        Self::from_raw_span(file, PosSpanRaw { start, end })
    }

    pub fn to_start_and_end_lnum_bol_offset(
        &self,
    ) -> ((usize, usize, usize), (usize, usize, usize)) {
        match &self.0 {
            PosImpl::Small { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            PosImpl::Large { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            PosImpl::Tiny { span, .. } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                (start.line_beg_offset(), end.line_beg_offset())
            }
            PosImpl::FromReason(_p) => unimplemented!(),
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
        } else if x1.end_offset() > x2.end_offset() {
            Err(String::from("btw: invalid positions")
                + &x1.end_offset().to_string()
                + "and"
                + &x2.end_offset().to_string())
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
        } else if span2.start.is_dummy() || span1.start.offset() < span2.start.offset() {
            span1.start
        } else {
            span2.start
        };

        let end = if span1.end.is_dummy() {
            span2.end
        } else if span2.end.is_dummy() || span1.end.offset() >= span2.end.offset() {
            span1.end
        } else {
            span2.end
        };

        Ok(Self::from_raw_span(
            RcOc::clone(x1.filename_rc_ref()),
            PosSpanRaw { start, end },
        ))
    }

    pub fn last_char(&self) -> Cow<'_, Self> {
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

    pub fn first_char_of_line(&self) -> Cow<'_, Self> {
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

    pub fn end_offset(&self) -> usize {
        match &self.0 {
            PosImpl::Small { end, .. } => end.offset(),
            PosImpl::Large { end, .. } => end.offset(),
            PosImpl::Tiny { span, .. } => span.end_offset(),
            PosImpl::FromReason(_p) => unimplemented!(),
        }
    }

    pub fn start_offset(&self) -> usize {
        match &self.0 {
            PosImpl::Small { start, .. } => start.offset(),
            PosImpl::Large { start, .. } => start.offset(),
            PosImpl::Tiny { span, .. } => span.start_offset(),
            PosImpl::FromReason(_p) => unimplemented!(),
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
            PosImpl::Small {
                file, start, end, ..
            } => do_fmt(f, file, start, end),
            PosImpl::Large {
                file, start, end, ..
            } => do_fmt(f, file, &**start, &**end),
            PosImpl::Tiny { file, span } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                do_fmt(f, file, &start, &end)
            }
            PosImpl::FromReason(_p) => unimplemented!(),
        }
    }
}

impl Ord for Pos {
    // Intended to match the implementation of `Pos.compare` in OCaml.
    fn cmp(&self, other: &Pos) -> Ordering {
        self.filename()
            .cmp(other.filename())
            .then(self.start_offset().cmp(&other.start_offset()))
            .then(self.end_offset().cmp(&other.end_offset()))
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

// non-derived impl Hash because PartialEq and Eq are non-derived
impl std::hash::Hash for Pos {
    fn hash<H: std::hash::Hasher>(&self, hasher: &mut H) {
        self.0.hash(hasher)
    }
}

impl EqModuloPos for Pos {
    fn eq_modulo_pos(&self, _rhs: &Self) -> bool {
        true
    }
}

impl EqModuloPosAndReason for Pos {
    fn eq_modulo_pos_and_reason(&self, _rhs: &Self) -> bool {
        true
    }
}

impl Pos {
    /// Returns a struct implementing Display which produces the same format as
    /// `Pos.string` in OCaml.
    pub fn string(&self) -> PosString<'_> {
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
    use pretty_assertions::assert_eq;

    use super::*;

    fn make_pos(name: &str, start: (usize, usize, usize), end: (usize, usize, usize)) -> Pos {
        Pos::from_lnum_bol_offset(
            RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(name))),
            start,
            end,
        )
    }

    #[test]
    fn test_pos() {
        assert!(Pos::make_none().is_none());
        assert!(
            !Pos::from_lnum_bol_offset(
                RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from("a"))),
                (0, 0, 0),
                (0, 0, 0)
            )
            .is_none(),
        );
        assert!(
            !Pos::from_lnum_bol_offset(
                RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(""))),
                (1, 0, 0),
                (0, 0, 0)
            )
            .is_none(),
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
            Pos::from_lnum_bol_offset(path, (5, 100, 117), (5, 100, 142))
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
