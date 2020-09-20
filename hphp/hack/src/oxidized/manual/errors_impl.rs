// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::errors::*;
use crate::pos::Pos;

impl<P> Error_<P> {
    pub fn new(code: ErrorCode, claim: Message<P>, reasons: Vec<Message<P>>) -> Self {
        Error_ {
            code,
            claim,
            reasons,
        }
    }

    pub fn pos(&self) -> &P {
        let (pos, _msg) = &self.claim;
        pos
    }

    pub fn code(&self) -> ErrorCode {
        self.code
    }
}

pub trait FileOrd {
    fn cmp_file(&self, other: &Self) -> Ordering;
}

impl FileOrd for Pos {
    fn cmp_file(&self, other: &Self) -> Ordering {
        self.filename().cmp(other.filename())
    }
}

impl<P: Ord + FileOrd> Ord for Error_<P> {
    // Intended to match the implementation of `compare` in `Errors.sort` in OCaml.
    fn cmp(&self, other: &Self) -> Ordering {
        let Self {
            code: self_code,
            claim: self_claim,
            reasons: self_reasons,
        } = self;
        let Self {
            code: other_code,
            claim: other_claim,
            reasons: other_reasons,
        } = other;
        let (self_pos, self_msg) = self_claim;
        let (other_pos, other_msg) = other_claim;
        // The primary sort order is by file of the claim (main message).
        self_pos
            .cmp_file(other_pos)
            // If the files are the same, sort by phase.
            .then(((*self_code / 1000) as isize).cmp(&((*other_code / 1000) as isize)))
            // If the phases are the same, sort by position.
            .then(self_pos.cmp(other_pos))
            // If the positions are the same, sort by claim message text.
            .then(self_msg.cmp(other_msg))
            // If the claim message text is the same, compare the reason
            // messages (which contain further explanation for the error
            // reported in the claim message).
            .then(self_reasons.iter().cmp(other_reasons.iter()))
    }
}

impl<P: Ord + FileOrd> PartialOrd for Error_<P> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Error_<Pos> {
    /// Return a struct with a `std::fmt::Display` implementation that displays
    /// the error in the "raw" format expected by our typecheck test cases.
    pub fn display_raw(&self) -> DisplayRaw<'_> {
        DisplayRaw(self)
    }
}

pub struct DisplayRaw<'a>(&'a Error_<Pos>);

impl<'a> std::fmt::Display for DisplayRaw<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Error_ {
            code,
            claim,
            reasons,
        } = self.0;
        let (pos, msg) = claim;
        let code = DisplayErrorCode(*code);
        write!(f, "{}\n{} ({})", pos.string(), msg, code)?;
        for (pos, msg) in reasons.iter() {
            write!(f, "\n  {}\n  {}", pos.string(), msg)?;
        }
        Ok(())
    }
}

fn error_kind(error_code: ErrorCode) -> &'static str {
    match error_code / 1000 {
        1 => "Parsing",
        2 => "Naming",
        3 => "NastCheck",
        4 => "Typing",
        5 => "Lint",
        8 => "Init",
        _ => "Other",
    }
}

struct DisplayErrorCode(ErrorCode);

impl std::fmt::Display for DisplayErrorCode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}[{:04}]", error_kind(self.0), self.0)
    }
}

impl Errors {
    pub fn empty() -> Self {
        Errors(FilesT::new(), FilesT::new())
    }

    pub fn is_empty(&self) -> bool {
        let Errors(errors, _fixmes) = self;
        errors.is_empty()
    }

    pub fn into_vec(self) -> Vec<Error> {
        let Errors(errors, _fixmes) = self;
        errors
            .into_iter()
            .flat_map(|(_filename, errs_by_phase)| {
                errs_by_phase
                    .into_iter()
                    .flat_map(|(_phase, errs)| errs.into_iter())
            })
            .collect()
    }

    pub fn into_sorted_vec(self) -> Vec<Error> {
        let mut errors = self.into_vec();
        errors.sort_unstable();
        errors.dedup();
        errors
    }
}

impl Naming {
    pub fn fd_name_already_bound(p: Pos) -> Error {
        Error::new(
            Self::FdNameAlreadyBound as isize,
            (p, "Field name already bound".into()),
            vec![],
        )
    }

    pub fn method_needs_visibility(p: Pos) -> Error {
        Error::new(
            Self::MethodNeedsVisibility as isize,
            (
                p,
                "Methods need to be marked public, private, or protected.".into(),
            ),
            vec![],
        )
    }

    pub fn unsupported_trait_use_as(p: Pos) -> Error {
        Error::new(
            Self::UnsupportedTraitUseAs as isize,
            (
                p,
                "Trait use as is a PHP feature that is unsupported in Hack".into(),
            ),
            vec![],
        )
    }

    pub fn unsupported_instead_of(p: Pos) -> Error {
        Error::new(
            Self::UnsupportedInsteadOf as isize,
            (
                p,
                "insteadof is a PHP feature that is unsupported in Hack".into(),
            ),
            vec![],
        )
    }

    pub fn invalid_trait_use_as_visibility(p: Pos) -> Error {
        Error::new(
            Self::InvalidTraitUseAsVisibility as isize,
            (
                p,
                "Cannot redeclare trait method's visibility in this manner".into(),
            ),
            vec![],
        )
    }
}

impl NastCheck {
    pub fn not_abstract_without_typeconst(p: Pos) -> Error {
        Error::new(
            Self::NotAbstractWithoutTypeconst as isize,
            (
                p,
                "This type constant is not declared as abstract, it must have an assigned type"
                    .into(),
            ),
            vec![],
        )
    }

    pub fn multiple_xhp_category(p: Pos) -> Error {
        Error::new(
            Self::MultipleXhpCategory as isize,
            (
                p,
                "XHP classes can only contain one category declaration".into(),
            ),
            vec![],
        )
    }
}
