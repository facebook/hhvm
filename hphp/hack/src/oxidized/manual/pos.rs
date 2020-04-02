// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::{borrow::Cow, cmp::Ordering, ops::Range, path::PathBuf, result::Result::*};

use ocamlrep::rc::RcOc;
use ocamlrep::{Allocator, FromError, Value};
use ocamlrep_derive::OcamlRep;
use serde::{Deserialize, Serialize};

use crate::file_pos_large::FilePosLarge;
use crate::file_pos_small::FilePosSmall;
use crate::relative_path::{Prefix, RelativePath};

#[derive(Clone, Debug, Deserialize, Hash, OcamlRep, Serialize)]
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
}

use PosImpl::*;

#[derive(Clone, Debug, Deserialize, Hash, OcamlRep, Serialize)]
pub struct Pos(PosImpl);

pub type PosR<'a> = &'a Pos;

impl ocamlrep::OcamlRep for PosR<'_> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        (**self).to_ocamlrep(alloc)
    }

    fn from_ocamlrep(_value: Value<'_>) -> Result<Self, FromError> {
        unimplemented!()
    }
}

impl Pos {
    pub fn make_none() -> Self {
        // TODO: shiqicao make NONE static, lazy_static doesn't allow Rc
        Pos(PosImpl::Small {
            file: RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(""))),
            start: FilePosSmall::make_dummy(),
            end: FilePosSmall::make_dummy(),
        })
    }

    pub fn is_none(&self) -> bool {
        match self {
            Pos(PosImpl::Small { file, start, end }) => {
                start.is_dummy() && end.is_dummy() && file.is_empty()
            }
            _ => false,
        }
    }

    pub fn filename(&self) -> &RelativePath {
        match &self.0 {
            Small { file, .. } => &file,
            Large { file, .. } => &file,
        }
    }

    /// Returns a closed interval that's incorrect for multi-line spans.
    pub fn info_pos(&self) -> (usize, usize, usize) {
        fn compute<P: FilePos>(pos_start: &P, pos_end: &P) -> (usize, usize, usize) {
            let (line, start_minus1, bol) = pos_start.line_column_beg();
            let start = start_minus1 + 1;
            let end_offset = pos_end.offset();
            let mut end = end_offset - bol;
            // To represent the empty interval, pos_start and pos_end are equal because
            // end_offset is exclusive. Here, it's best for error messages to the user if
            // we print characters N to N (highlighting a single character) rather than characters
            // N to (N-1), which is very unintuitive.
            if start == end + 1 {
                end = start
            }
            (line, start, end)
        }
        match &self.0 {
            Small { start, end, .. } => compute(start, end),
            Large { start, end, .. } => compute(start.as_ref(), end.as_ref()),
        }
    }

    pub fn info_pos_extended(&self) -> (usize, usize, usize, usize) {
        let (line_begin, start, end) = self.info_pos();
        let line_end = match &self.0 {
            Small { end, .. } => end.line_column_beg(),
            Large { end, .. } => (*end).line_column_beg(),
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
        }
    }

    pub fn from_lnum_bol_cnum(
        file: RcOc<RelativePath>,
        start: (usize, usize, usize),
        end: (usize, usize, usize),
    ) -> Self {
        let (start_line, start_bol, start_cnum) = start;
        let (end_line, end_bol, end_cnum) = end;
        let start = FilePosSmall::from_lnum_bol_cnum(start_line, start_bol, start_cnum);
        let end = FilePosSmall::from_lnum_bol_cnum(end_line, end_bol, end_cnum);
        match (start, end) {
            (Some(start), Some(end)) => Pos(Small { file, start, end }),
            _ => {
                let start = Box::new(FilePosLarge::from_lnum_bol_cnum(
                    start_line, start_bol, start_cnum,
                ));
                let end = Box::new(FilePosLarge::from_lnum_bol_cnum(
                    end_line, end_bol, end_cnum,
                ));
                Pos(Large { file, start, end })
            }
        }
    }

    /// For single-line spans only.
    pub fn from_line_cols_offset(
        file: RcOc<RelativePath>,
        line: usize,
        cols: Range<usize>,
        start_offset: usize,
    ) -> Self {
        let start = FilePosSmall::from_line_column_offset(line, cols.start, start_offset);
        let end = FilePosSmall::from_line_column_offset(
            line,
            cols.end,
            start_offset + (cols.end - cols.start),
        );
        match (start, end) {
            (Some(start), Some(end)) => Pos(Small { file, start, end }),
            _ => {
                let start = Box::new(FilePosLarge::from_line_column_offset(
                    line,
                    cols.start,
                    start_offset,
                ));
                let end = Box::new(FilePosLarge::from_line_column_offset(
                    line,
                    cols.end,
                    start_offset + (cols.end - cols.start),
                ));
                Pos(Large { file, start, end })
            }
        }
    }

    fn small_to_large_file_pos(p: &FilePosSmall) -> FilePosLarge {
        let (lnum, col, bol) = p.line_column_beg();
        FilePosLarge::from_lnum_bol_cnum(lnum, bol, bol + col)
    }

    pub fn btw_nocheck(x1: Self, x2: Self) -> Self {
        let inner = match (x1.0, x2.0) {
            (Small { file, start, .. }, Small { end, .. }) => Small { file, start, end },
            (Large { file, start, .. }, Large { end, .. }) => Large { file, start, end },
            (Small { file, start, .. }, Large { end, .. }) => Large {
                file,
                start: Box::new(Self::small_to_large_file_pos(&start)),
                end,
            },
            (Large { file, start, .. }, Small { end, .. }) => Large {
                file,
                start,
                end: Box::new(Self::small_to_large_file_pos(&end)),
            },
        };
        Pos(inner)
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
                Ok(Pos(Small {
                    file: file.clone(),
                    start: *start,
                    end: *end,
                }))
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
                Ok(Pos(Large {
                    file: file.clone(),
                    start: Box::new(*start.clone()),
                    end: Box::new(*end.clone()),
                }))
            }
            (Small { file, start, end }, Large { .. }) => Self::merge(
                &Pos(Large {
                    file: file.clone(),
                    start: Box::new(Self::small_to_large_file_pos(&start)),
                    end: Box::new(Self::small_to_large_file_pos(&end)),
                }),
                x2,
            ),
            (Large { .. }, Small { file, start, end }) => Self::merge(
                x1,
                &Pos(Large {
                    file: file.clone(),
                    start: Box::new(Self::small_to_large_file_pos(&start)),
                    end: Box::new(Self::small_to_large_file_pos(&end)),
                }),
            ),
        }
    }

    pub fn last_char(&self) -> Cow<Self> {
        if self.is_none() {
            Cow::Borrowed(self)
        } else {
            Cow::Owned(Pos(match &self.0 {
                Small { file, end, .. } => Small {
                    file: file.clone(),
                    start: *end,
                    end: *end,
                },
                Large { file, end, .. } => Large {
                    file: file.clone(),
                    start: end.clone(),
                    end: end.clone(),
                },
            }))
        }
    }

    pub fn first_char_of_line(&self) -> Cow<Self> {
        if self.is_none() {
            Cow::Borrowed(self)
        } else {
            Cow::Owned(Pos(match &self.0 {
                Small { file, start, .. } => {
                    let start = start.with_column(0);
                    Small {
                        file: file.clone(),
                        start,
                        end: start,
                    }
                }
                Large { file, start, .. } => {
                    let start = start.with_column(0);
                    Large {
                        file: file.clone(),
                        start: Box::new(start),
                        end: Box::new(start),
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
            Small { file, start, end } => do_fmt(f, file, start, end),
            Large { file, start, end } => do_fmt(f, file, &**start, &**end),
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

// TODO(hrust) eventually move this into a separate file used by Small & Large
trait FilePos {
    fn offset(&self) -> usize;
    fn line_column_beg(&self) -> (usize, usize, usize);
}
macro_rules! impl_file_pos {
    ($type: ty) => {
        impl FilePos for $type {
            fn offset(&self) -> usize {
                (*self).offset()
            }
            fn line_column_beg(&self) -> (usize, usize, usize) {
                (*self).line_column_beg()
            }
        }
    };
}
impl_file_pos!(FilePosSmall);
impl_file_pos!(FilePosLarge);

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
    fn test() {
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
    fn test_merge() {
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
