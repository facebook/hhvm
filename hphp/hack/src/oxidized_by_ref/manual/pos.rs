// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::ops::Range;
use std::result::Result::*;

use bumpalo::Bump;
use eq_modulo_pos::EqModuloPos;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use oxidized::file_pos::FilePos;
use oxidized::file_pos_large::FilePosLarge;
use oxidized::file_pos_small::FilePosSmall;
use oxidized::pos_span_raw::PosSpanRaw;
use oxidized::pos_span_tiny::PosSpanTiny;
use serde::Deserialize;
use serde::Serialize;

use crate::relative_path::RelativePath;

#[derive(Clone, Deserialize, Hash, Serialize, ToOcamlRep, FromOcamlRepIn)]
enum PosImpl<'a> {
    Small {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        file: &'a RelativePath<'a>,
        start: FilePosSmall,
        end: FilePosSmall,
    },
    Large {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        file: &'a RelativePath<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        start: &'a FilePosLarge,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        end: &'a FilePosLarge,
    },
    Tiny {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        file: &'a RelativePath<'a>,
        span: PosSpanTiny,
    },
}
arena_deserializer::impl_deserialize_in_arena!(PosImpl<'arena>);

impl arena_trait::TrivialDrop for PosImpl<'_> {}

use PosImpl::*;

#[derive(Clone, Deserialize, Hash, Serialize, ToOcamlRep, FromOcamlRepIn)]
pub struct Pos<'a>(#[serde(deserialize_with = "arena_deserializer::arena", borrow)] PosImpl<'a>);
arena_deserializer::impl_deserialize_in_arena!(Pos<'arena>);

impl arena_trait::TrivialDrop for Pos<'_> {}

const NONE: Pos<'_> = Pos(Tiny {
    file: RelativePath::empty(),
    span: PosSpanTiny::make_dummy(),
});

impl<'a> Pos<'a> {
    pub const fn none() -> &'static Pos<'static> {
        &NONE
    }

    pub fn is_none(&self) -> bool {
        match self {
            Pos(PosImpl::Tiny { file, span }) => span.is_dummy() && file.is_empty(),
            _ => false,
        }
    }

    pub fn from_raw_span(b: &'a Bump, file: &'a RelativePath<'a>, span: PosSpanRaw) -> &'a Self {
        if let Some(span) = PosSpanTiny::make(&span.start, &span.end) {
            return b.alloc(Pos(Tiny { file, span }));
        }
        let (lnum, bol, offset) = span.start.line_beg_offset();
        if let Some(start) = FilePosSmall::from_lnum_bol_offset(lnum, bol, offset) {
            let (lnum, bol, offset) = span.end.line_beg_offset();
            if let Some(end) = FilePosSmall::from_lnum_bol_offset(lnum, bol, offset) {
                return b.alloc(Pos(Small { file, start, end }));
            }
        }
        b.alloc(Pos(Large {
            file,
            start: b.alloc(span.start),
            end: b.alloc(span.end),
        }))
    }

    pub fn to_raw_span(&self) -> PosSpanRaw {
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
        }
    }

    pub fn filename(&self) -> &'a RelativePath<'a> {
        match &self.0 {
            Small { file, .. } | Large { file, .. } | Tiny { file, .. } => file,
        }
    }

    /// Returns a closed interval that's incorrect for multi-line spans.
    pub fn info_pos(&self) -> (usize, usize, usize) {
        fn compute<P: FilePos>(pos_start: P, pos_end: P) -> (usize, usize, usize) {
            let (line, start_minus1, bol) = pos_start.line_column_beg();
            let start = start_minus1.wrapping_add(1);
            let end_offset = pos_end.offset();
            let mut end = end_offset - bol;
            // To represent the empty interval, pos_start and pos_end are equal because
            // end_offset is exclusive. Here, it's best for error messages to the user if
            // we print characters N to N (highlighting a single character) rather than characters
            // N to (N-1), which is very unintuitive.
            if start_minus1 == end {
                end = start
            }
            (line, start, end)
        }
        match self.0 {
            Small { start, end, .. } => compute(start, end),
            Large { start, end, .. } => compute(*start, *end),
            Tiny { span, .. } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                compute(start, end)
            }
        }
    }

    pub fn info_pos_extended(&self) -> (usize, usize, usize, usize) {
        let (line_begin, start, end) = self.info_pos();
        let line_end = match self.0 {
            Small { end, .. } => end.line_column_beg(),
            Large { end, .. } => end.line_column_beg(),
            Tiny { span, .. } => span.to_raw_span().end.line_column_beg(),
        }
        .0;
        (line_begin, line_end, start, end)
    }

    pub fn info_raw(&self) -> (usize, usize) {
        (self.start_offset(), self.end_offset())
    }

    pub fn line(&self) -> usize {
        match self.0 {
            Small { start, .. } => start.line(),
            Large { start, .. } => start.line(),
            Tiny { span, .. } => span.start_line_number(),
        }
    }

    pub fn from_lnum_bol_offset(
        b: &'a Bump,
        file: &'a RelativePath<'a>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> &'a Self {
        let (start_line, start_bol, start_offset) = start;
        let (end_line, end_bol, end_offset) = end;
        let start = FilePosLarge::from_lnum_bol_offset(start_line, start_bol, start_offset);
        let end = FilePosLarge::from_lnum_bol_offset(end_line, end_bol, end_offset);
        Self::from_raw_span(b, file, PosSpanRaw { start, end })
    }

    pub fn to_start_and_end_lnum_bol_offset(
        &self,
    ) -> ((usize, usize, usize), (usize, usize, usize)) {
        match &self.0 {
            Small { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            Large { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            Tiny { span, .. } => {
                let PosSpanRaw { start, end } = span.to_raw_span();
                (start.line_beg_offset(), end.line_beg_offset())
            }
        }
    }

    /// For single-line spans only.
    pub fn from_line_cols_offset(
        b: &'a Bump,
        file: &'a RelativePath<'a>,
        line: usize,
        cols: Range<usize>,
        start_offset: usize,
    ) -> &'a Self {
        let start = FilePosLarge::from_line_column_offset(line, cols.start, start_offset);
        let end = FilePosLarge::from_line_column_offset(
            line,
            cols.end,
            start_offset + (cols.end - cols.start),
        );
        Self::from_raw_span(b, file, PosSpanRaw { start, end })
    }

    pub fn btw_nocheck(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> &'a Self {
        let start = x1.to_raw_span().start;
        let end = x2.to_raw_span().end;
        Self::from_raw_span(b, x1.filename(), PosSpanRaw { start, end })
    }

    pub fn btw(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> Result<&'a Self, String> {
        let file1 = x1.filename();
        let file2 = x2.filename();
        if !std::ptr::eq(file1, file2) && file1 != file2 {
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
            Ok(Self::btw_nocheck(b, x1, x2))
        }
    }

    pub fn merge(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> Result<&'a Self, String> {
        let file1 = x1.filename();
        let file2 = x2.filename();
        if !std::ptr::eq(file1, file2) && file1 != file2 {
            // see comment above (T52404885)
            return Err(String::from("Position in separate files ")
                + &x1.filename().to_string()
                + " and "
                + &x2.filename().to_string());
        }
        Ok(Self::merge_without_checking_filename(b, x1, x2))
    }

    /// Return the smallest position containing both given positions (if they
    /// are both in the same file). The returned position will have the
    /// filename of the first Pos argument.
    ///
    /// For merging positions that may not occur within the same file, use
    /// `Pos::merge`.
    pub fn merge_without_checking_filename(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> &'a Self {
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

        Self::from_raw_span(b, x1.filename(), PosSpanRaw { start, end })
    }

    pub fn last_char(&'a self, b: &'a Bump) -> &'a Self {
        if self.is_none() {
            self
        } else {
            let end = self.to_raw_span().end;
            Self::from_raw_span(b, self.filename(), PosSpanRaw { start: end, end })
        }
    }

    pub fn first_char_of_line(&'a self, b: &'a Bump) -> &'a Self {
        if self.is_none() {
            self
        } else {
            let start = self.to_raw_span().start.with_column(0);
            Self::from_raw_span(b, self.filename(), PosSpanRaw { start, end: start })
        }
    }

    pub fn end_offset(&self) -> usize {
        match &self.0 {
            Small { end, .. } => end.offset(),
            Large { end, .. } => end.offset(),
            Tiny { span, .. } => span.end_offset(),
        }
    }

    pub fn start_offset(&self) -> usize {
        match &self.0 {
            Small { start, .. } => start.offset(),
            Large { start, .. } => start.offset(),
            Tiny { span, .. } => span.start_offset(),
        }
    }

    pub fn to_owned(&self) -> oxidized::pos::Pos {
        let file = self.filename();
        let PosSpanRaw { start, end } = self.to_raw_span();
        oxidized::pos::Pos::from_lnum_bol_offset(
            std::sync::Arc::new(file.to_oxidized()),
            start.line_beg_offset(),
            end.line_beg_offset(),
        )
    }
}

impl<'a> Pos<'a> {
    pub fn from_oxidized_in(pos: &oxidized::pos::Pos, arena: &'a Bump) -> &'a Self {
        let file = RelativePath::from_oxidized_in(pos.filename(), arena);
        let (start, end) = pos.to_start_and_end_lnum_bol_offset();
        Self::from_lnum_bol_offset(arena, file, start, end)
    }

    pub fn from_oxidized_with_file_in(
        pos: &oxidized::pos::Pos,
        file: &'a RelativePath<'a>,
        arena: &'a Bump,
    ) -> &'a Self {
        debug_assert!(pos.filename().prefix() == file.prefix());
        debug_assert!(pos.filename().path() == file.path());
        let (start, end) = pos.to_start_and_end_lnum_bol_offset();
        Self::from_lnum_bol_offset(arena, file, start, end)
    }
}

impl std::fmt::Debug for Pos<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn do_fmt<P: FilePos>(
            f: &mut std::fmt::Formatter<'_>,
            file: &RelativePath<'_>,
            start: &P,
            end: &P,
        ) -> std::fmt::Result {
            write!(f, "{}", file)?;
            let (start_line, start_col, _) = start.line_column_beg();
            let (end_line, end_col, _) = end.line_column_beg();
            // Use a format string rather than Formatter::debug_tuple to prevent
            // adding line breaks. Positions occur very frequently in ASTs and
            // types, so the Debug implementation of those data structures is
            // more readable if we minimize the vertical space taken up by
            // positions. Depends upon RelativePath's implementation of Display
            // also being single-line.
            if start_line == end_line {
                write!(f, "({}:{}-{})", start_line, start_col, end_col)
            } else {
                write!(f, "({}:{}-{}:{})", start_line, start_col, end_line, end_col)
            }
        }
        match &self.0 {
            Small { file, start, end } => do_fmt(f, file, start, end),
            Large { file, start, end } => do_fmt(f, file, *start, *end),
            Tiny { file, span } => {
                if self.is_none() {
                    return write!(f, "Pos::NONE");
                }
                let PosSpanRaw { start, end } = span.to_raw_span();
                do_fmt(f, file, &start, &end)
            }
        }
    }
}

impl std::fmt::Display for Pos<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn do_fmt<P: FilePos>(
            f: &mut std::fmt::Formatter<'_>,
            file: &RelativePath<'_>,
            start: P,
            end: P,
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
        match self.0 {
            Small { file, start, end } => do_fmt(f, file, start, end),
            Large { file, start, end } => do_fmt(f, file, *start, *end),
            Tiny { file, span } => {
                if self.is_none() {
                    return write!(f, "Pos::NONE");
                }
                let PosSpanRaw { start, end } = span.to_raw_span();
                do_fmt(f, file, start, end)
            }
        }
    }
}

impl Ord for Pos<'_> {
    // Intended to match the implementation of `Pos.compare` in OCaml.
    fn cmp(&self, other: &Pos<'_>) -> Ordering {
        self.filename()
            .cmp(other.filename())
            .then(self.start_offset().cmp(&other.start_offset()))
            .then(self.end_offset().cmp(&other.end_offset()))
    }
}

impl PartialOrd for Pos<'_> {
    fn partial_cmp(&self, other: &Pos<'_>) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for Pos<'_> {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == Ordering::Equal
    }
}

impl Eq for Pos<'_> {}

impl<'a> Pos<'a> {
    /// Returns a struct implementing Display which produces the same format as
    /// `Pos.string` in OCaml.
    pub fn string(&self) -> PosString<'_> {
        PosString(self)
    }
}

impl EqModuloPos for Pos<'_> {
    fn eq_modulo_pos(&self, _rhs: &Self) -> bool {
        true
    }
    fn eq_modulo_pos_and_reason(&self, _rhs: &Self) -> bool {
        true
    }
}

/// This struct has an impl of Display which produces the same format as
/// `Pos.string` in OCaml.
pub struct PosString<'a>(&'a Pos<'a>);

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
impl no_pos_hash::NoPosHash for Pos<'_> {
    fn hash<H: std::hash::Hasher>(&self, _state: &mut H) {}
}

pub mod map {
    pub type Map<'a, T> = arena_collections::map::Map<'a, super::Pos<'a>, T>;
}

#[cfg(test)]
mod tests {
    use pretty_assertions::assert_eq;
    use relative_path::Prefix;

    use super::*;

    fn make_pos<'a>(
        b: &'a Bump,
        name: &'a RelativePath<'a>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> &'a Pos<'a> {
        b.alloc(Pos::from_lnum_bol_offset(b, name, start, end))
    }

    #[test]
    fn test_pos_is_none() {
        let b = Bump::new();
        assert!(Pos::none().is_none());
        let path_a = b.alloc(RelativePath::make(Prefix::Dummy, "a"));
        assert!(!Pos::from_lnum_bol_offset(&b, path_a, (0, 0, 0), (0, 0, 0)).is_none());
        let empty_path = b.alloc(RelativePath::make(Prefix::Dummy, ""));
        assert!(!Pos::from_lnum_bol_offset(&b, empty_path, (1, 0, 0), (0, 0, 0)).is_none());
    }

    #[test]
    fn test_pos_string() {
        assert_eq!(
            Pos::none().string().to_string(),
            r#"File "", line 0, characters 0-0:"#
        );
        let b = Bump::new();
        let path = b.alloc(RelativePath::make(Prefix::Dummy, "a.php"));
        assert_eq!(
            Pos::from_lnum_bol_offset(&b, path, (5, 100, 117), (5, 100, 142))
                .string()
                .to_string(),
            r#"File "a.php", line 5, characters 18-42:"#
        );
    }

    #[test]
    fn test_pos_merge() {
        let b = Bump::new();
        let test = |name, (exp_start, exp_end), ((fst_start, fst_end), (snd_start, snd_end))| {
            let path = b.alloc(RelativePath::make(Prefix::Dummy, "a"));
            assert_eq!(
                Ok(make_pos(&b, path, exp_start, exp_end)),
                Pos::merge(
                    &b,
                    make_pos(&b, path, fst_start, fst_end),
                    make_pos(&b, path, snd_start, snd_end)
                ),
                "{}",
                name
            );

            // Run this again because we want to test that we get the same
            // result regardless of order.
            assert_eq!(
                Ok(make_pos(&b, path, exp_start, exp_end)),
                Pos::merge(
                    &b,
                    make_pos(&b, path, snd_start, snd_end),
                    make_pos(&b, path, fst_start, fst_end),
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
                &b,
                make_pos(
                    &b,
                    &RelativePath::make(Prefix::Dummy, "a"),
                    (0, 0, 0),
                    (0, 0, 0)
                ),
                make_pos(
                    &b,
                    &RelativePath::make(Prefix::Dummy, "b"),
                    (0, 0, 0),
                    (0, 0, 0)
                )
            ),
            "should reject merges with different filenames"
        );
    }

    #[test]
    fn position_insensitive_hash() {
        use crate::ast_defs::Id;

        let b = &bumpalo::Bump::new();
        let hash = no_pos_hash::position_insensitive_hash;
        let none = Pos::none();
        let pos = Pos::from_line_cols_offset(b, RelativePath::empty(), 2, 2..10, 17);

        assert_eq!(hash(&Id(pos, "foo")), hash(&Id(pos, "foo")));
        assert_eq!(hash(&Id(none, "foo")), hash(&Id(pos, "foo")));

        assert_ne!(hash(&Id(pos, "foo")), hash(&Id(pos, "bar")));
    }
}
