// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::{cmp::Ordering, ops::Range, result::Result::*};

use bumpalo::Bump;
use serde::Serialize;

use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};
use oxidized::file_pos_large::FilePosLarge;
use oxidized::file_pos_small::FilePosSmall;

use crate::relative_path::RelativePath;

#[derive(Clone, Hash, Serialize, ToOcamlRep, FromOcamlRepIn)]
enum PosImpl<'a> {
    Small {
        file: &'a RelativePath<'a>,
        start: FilePosSmall,
        end: FilePosSmall,
    },
    Large {
        file: &'a RelativePath<'a>,
        start: &'a FilePosLarge,
        end: &'a FilePosLarge,
    },
}

impl arena_trait::TrivialDrop for PosImpl<'_> {}

use PosImpl::*;

#[derive(Clone, Hash, Serialize, ToOcamlRep, FromOcamlRepIn)]
pub struct Pos<'a>(PosImpl<'a>);

impl arena_trait::TrivialDrop for Pos<'_> {}

const NONE: Pos<'_> = Pos(Small {
    file: RelativePath::empty(),
    start: FilePosSmall::make_dummy(),
    end: FilePosSmall::make_dummy(),
});

impl<'a> Pos<'a> {
    pub const fn none() -> &'static Pos<'static> {
        &NONE
    }

    pub fn is_none(&self) -> bool {
        match self {
            Pos(PosImpl::Small { file, start, end }) => {
                start.is_dummy() && end.is_dummy() && file.is_empty()
            }
            _ => false,
        }
    }

    pub fn filename(&self) -> &'a RelativePath<'a> {
        match self.0 {
            Small { file, .. } => file,
            Large { file, .. } => file,
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
        }
    }

    pub fn info_pos_extended(&self) -> (usize, usize, usize, usize) {
        let (line_begin, start, end) = self.info_pos();
        let line_end = match self.0 {
            Small { end, .. } => end.line_column_beg(),
            Large { end, .. } => end.line_column_beg(),
        }
        .0;
        (line_begin, line_end, start, end)
    }

    pub fn info_raw(&self) -> (usize, usize) {
        (self.start_cnum(), self.end_cnum())
    }

    pub fn line(&self) -> usize {
        match self.0 {
            Small { start, .. } => start.line(),
            Large { start, .. } => start.line(),
        }
    }

    pub fn from_lnum_bol_cnum(
        b: &'a Bump,
        file: &'a RelativePath<'a>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> &'a Self {
        let (start_line, start_bol, start_cnum) = start;
        let (end_line, end_bol, end_cnum) = end;
        let start = FilePosSmall::from_lnum_bol_cnum(start_line, start_bol, start_cnum);
        let end = FilePosSmall::from_lnum_bol_cnum(end_line, end_bol, end_cnum);
        match (start, end) {
            (Some(start), Some(end)) => b.alloc(Pos(Small { file, start, end })),
            _ => {
                let start = b.alloc(FilePosLarge::from_lnum_bol_cnum(
                    start_line, start_bol, start_cnum,
                ));
                let end = b.alloc(FilePosLarge::from_lnum_bol_cnum(
                    end_line, end_bol, end_cnum,
                ));
                b.alloc(Pos(Large { file, start, end }))
            }
        }
    }

    pub fn to_start_and_end_lnum_bol_cnum(&self) -> ((usize, usize, usize), (usize, usize, usize)) {
        match &self.0 {
            Small { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
            Large { start, end, .. } => (start.line_beg_offset(), end.line_beg_offset()),
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
        let start = FilePosSmall::from_line_column_offset(line, cols.start, start_offset);
        let end = FilePosSmall::from_line_column_offset(
            line,
            cols.end,
            start_offset + (cols.end - cols.start),
        );
        match (start, end) {
            (Some(start), Some(end)) => b.alloc(Pos(Small { file, start, end })),
            _ => {
                let start = b.alloc(FilePosLarge::from_line_column_offset(
                    line,
                    cols.start,
                    start_offset,
                ));
                let end = b.alloc(FilePosLarge::from_line_column_offset(
                    line,
                    cols.end,
                    start_offset + (cols.end - cols.start),
                ));
                b.alloc(Pos(Large { file, start, end }))
            }
        }
    }

    fn small_to_large_file_pos(p: FilePosSmall) -> FilePosLarge {
        let (lnum, col, bol) = p.line_column_beg();
        FilePosLarge::from_lnum_bol_cnum(lnum, bol, bol + col)
    }

    pub fn btw_nocheck(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> &'a Self {
        let inner = match (&x1.0, &x2.0) {
            (Small { file, start, .. }, Small { end, .. }) => Small {
                file,
                start: *start,
                end: *end,
            },
            (Large { file, start, .. }, Large { end, .. }) => Large { file, start, end },
            (Small { file, start, .. }, Large { end, .. }) => Large {
                file,
                start: b.alloc(Self::small_to_large_file_pos(*start)),
                end,
            },
            (Large { file, start, .. }, Small { end, .. }) => Large {
                file,
                start,
                end: b.alloc(Self::small_to_large_file_pos(*end)),
            },
        };
        b.alloc(Pos(inner))
    }

    pub fn btw(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> Result<&'a Self, String> {
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
            Ok(Self::btw_nocheck(b, x1, x2))
        }
    }

    pub fn merge(b: &'a Bump, x1: &'a Self, x2: &'a Self) -> Result<&'a Self, String> {
        if x1.filename() != x2.filename() {
            // see comment above (T52404885)
            return Err(String::from("Position in separate files ")
                + &x1.filename().to_string()
                + " and "
                + &x2.filename().to_string());
        }
        match (&x1.0, &x2.0) {
            (
                Small {
                    file,
                    start: start1,
                    end: end1,
                },
                Small {
                    file: _,
                    start: start2,
                    end: end2,
                },
            ) => {
                let start = if start1.is_dummy() {
                    start2
                } else if start2.is_dummy() {
                    start1
                } else if start1.offset() < start2.offset() {
                    start1
                } else {
                    start2
                };
                let end = if end1.is_dummy() {
                    end2
                } else if end2.is_dummy() {
                    end1
                } else if end1.offset() < end2.offset() {
                    end2
                } else {
                    end1
                };
                Ok(b.alloc(Pos(Small {
                    file,
                    start: *start,
                    end: *end,
                })))
            }
            (
                Large {
                    file,
                    start: start1,
                    end: end1,
                },
                Large {
                    file: _,
                    start: start2,
                    end: end2,
                },
            ) => {
                let start = if start1.is_dummy() {
                    start2
                } else if start2.is_dummy() {
                    start1
                } else if start1.offset() < start2.offset() {
                    start1
                } else {
                    start2
                };
                let end = if end1.is_dummy() {
                    end2
                } else if end2.is_dummy() {
                    end1
                } else if end1.offset() < end2.offset() {
                    end2
                } else {
                    end1
                };
                Ok(b.alloc(Pos(Large { file, start, end })))
            }
            (Small { file, start, end }, Large { .. }) => Self::merge(
                b,
                b.alloc(Pos(Large {
                    file,
                    start: b.alloc(Self::small_to_large_file_pos(*start)),
                    end: b.alloc(Self::small_to_large_file_pos(*end)),
                })),
                x2,
            ),
            (Large { .. }, Small { file, start, end }) => Self::merge(
                b,
                x1,
                b.alloc(Pos(Large {
                    file,
                    start: b.alloc(Self::small_to_large_file_pos(*start)),
                    end: b.alloc(Self::small_to_large_file_pos(*end)),
                })),
            ),
        }
    }

    pub fn last_char(&'a self, b: &'a Bump) -> &'a Self {
        if self.is_none() {
            self
        } else {
            b.alloc(Pos(match self.0 {
                Small { file, end, .. } => Small {
                    file,
                    start: end,
                    end,
                },
                Large { file, end, .. } => Large {
                    file,
                    start: end,
                    end,
                },
            }))
        }
    }

    pub fn first_char_of_line(&'a self, b: &'a Bump) -> &'a Self {
        if self.is_none() {
            self
        } else {
            b.alloc(Pos(match self.0 {
                Small { file, start, .. } => {
                    let start = start.with_column(0);
                    Small {
                        file,
                        start,
                        end: start,
                    }
                }
                Large { file, start, .. } => {
                    let start = b.alloc(start.with_column(0));
                    Large {
                        file,
                        start,
                        end: start,
                    }
                }
            }))
        }
    }

    pub fn end_cnum(&self) -> usize {
        match &self.0 {
            Small { end, .. } => end.offset(),
            Large { end, .. } => end.offset(),
        }
    }

    pub fn start_cnum(&self) -> usize {
        match &self.0 {
            Small { start, .. } => start.offset(),
            Large { start, .. } => start.offset(),
        }
    }

    pub fn to_owned(&self) -> oxidized::pos::Pos {
        match &self.0 {
            Small { file, start, end } => {
                let start = start.line_beg_offset();
                let end = end.line_beg_offset();
                oxidized::pos::Pos::from_lnum_bol_cnum(
                    ocamlrep::rc::RcOc::new(file.to_oxidized()),
                    start,
                    end,
                )
            }
            Large { file, start, end } => {
                let start = start.line_beg_offset();
                let end = end.line_beg_offset();
                oxidized::pos::Pos::from_lnum_bol_cnum(
                    ocamlrep::rc::RcOc::new(file.to_oxidized()),
                    start,
                    end,
                )
            }
        }
    }
}

impl<'a> Pos<'a> {
    pub fn from_oxidized_in(pos: &oxidized::pos::Pos, arena: &'a Bump) -> &'a Self {
        let file = RelativePath::from_oxidized_in(pos.filename(), arena);
        let (start, end) = pos.to_start_and_end_lnum_bol_cnum();
        Self::from_lnum_bol_cnum(arena, file, start, end)
    }

    pub fn from_oxidized_with_file_in(
        pos: &oxidized::pos::Pos,
        file: &'a RelativePath<'a>,
        arena: &'a Bump,
    ) -> &'a Self {
        debug_assert!(pos.filename().prefix() == file.prefix());
        debug_assert!(pos.filename().path() == file.path());
        let (start, end) = pos.to_start_and_end_lnum_bol_cnum();
        Self::from_lnum_bol_cnum(arena, file, start, end)
    }
}

impl std::fmt::Debug for Pos<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn do_fmt<P: FilePos>(
            f: &mut std::fmt::Formatter<'_>,
            file: &RelativePath,
            start: &P,
            end: &P,
        ) -> std::fmt::Result {
            let (start_line, start_col, _) = start.line_column_beg();
            let (end_line, end_col, _) = end.line_column_beg();
            // Use a format string rather than Formatter::debug_tuple to prevent
            // adding line breaks. Positions occur very frequently in ASTs and
            // types, so the Debug implementation of those data structures is
            // more readable if we minimize the vertical space taken up by
            // positions. Depends upon RelativePath's implementation of Display
            // also being single-line.
            if start_line == end_line {
                write!(
                    f,
                    "Pos({}, {}:{}-{})",
                    &file, &start_line, &start_col, &end_col,
                )
            } else {
                write!(
                    f,
                    "Pos({}, {}:{}-{}:{})",
                    &file, &start_line, &start_col, &end_line, &end_col,
                )
            }
        }
        match &self.0 {
            Small { file, start, end } => do_fmt(f, file, start, end),
            Large { file, start, end } => do_fmt(f, file, &**start, &**end),
        }
    }
}

impl std::fmt::Display for Pos<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn do_fmt<P: FilePos>(
            f: &mut std::fmt::Formatter<'_>,
            file: &RelativePath,
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
        }
    }
}

impl Ord for Pos<'_> {
    // Intended to match the implementation of `Pos.compare` in OCaml.
    fn cmp(&self, other: &Pos) -> Ordering {
        self.filename()
            .cmp(&other.filename())
            .then(self.start_cnum().cmp(&other.start_cnum()))
            .then(self.end_cnum().cmp(&other.end_cnum()))
    }
}

impl PartialOrd for Pos<'_> {
    fn partial_cmp(&self, other: &Pos) -> Option<Ordering> {
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

trait FilePos: Copy {
    fn offset(self) -> usize;
    fn line_column_beg(self) -> (usize, usize, usize);
}
macro_rules! impl_file_pos {
    ($type: ty) => {
        impl FilePos for $type {
            fn offset(self) -> usize {
                self.offset()
            }
            fn line_column_beg(self) -> (usize, usize, usize) {
                self.line_column_beg()
            }
        }
    };
}
impl_file_pos!(FilePosSmall);
impl_file_pos!(FilePosLarge);

#[cfg(test)]
mod tests {
    use super::*;
    use crate::relative_path::Prefix;
    use pretty_assertions::assert_eq;

    fn make_pos<'a>(
        b: &'a Bump,
        name: &'a RelativePath<'a>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> &'a Pos<'a> {
        b.alloc(Pos::from_lnum_bol_cnum(b, name, start, end))
    }

    #[test]
    fn test_is_none() {
        let b = Bump::new();
        assert_eq!(Pos::none().is_none(), true);
        let path_a = b.alloc(RelativePath::make(Prefix::Dummy, "a"));
        assert_eq!(
            Pos::from_lnum_bol_cnum(&b, path_a, (0, 0, 0), (0, 0, 0)).is_none(),
            false
        );
        let empty_path = b.alloc(RelativePath::make(Prefix::Dummy, ""));
        assert_eq!(
            Pos::from_lnum_bol_cnum(&b, empty_path, (1, 0, 0), (0, 0, 0)).is_none(),
            false
        );
    }

    #[test]
    fn test_string() {
        assert_eq!(
            Pos::none().string().to_string(),
            r#"File "", line 0, characters 0-0:"#
        );
        let b = Bump::new();
        let path = b.alloc(RelativePath::make(Prefix::Dummy, "a.php"));
        assert_eq!(
            Pos::from_lnum_bol_cnum(&b, path, (5, 100, 117), (5, 100, 142))
                .string()
                .to_string(),
            r#"File "a.php", line 5, characters 18-42:"#
        );
    }

    #[test]
    fn test_merge() {
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
