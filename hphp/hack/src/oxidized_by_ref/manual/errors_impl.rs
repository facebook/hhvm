// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::errors::*;
use crate::pos::Pos;

impl<'a> Error_<'a, Pos<'a>> {
    pub fn new(
        code: ErrorCode,
        claim: &'a Message<'a, Pos<'a>>,
        reasons: &'a [&'a Message<'a, Pos<'a>>],
    ) -> Self {
        Error_ {
            code,
            claim,
            reasons,
        }
    }

    pub fn pos(&self) -> &Pos<'a> {
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

impl FileOrd for &Pos<'_> {
    fn cmp_file(&self, other: &Self) -> Ordering {
        self.filename().cmp(other.filename())
    }
}

impl<P: Ord + FileOrd> Ord for Error_<'_, P> {
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
            // If the error codes are the same, sort by message text.
            .then(self_msg.cmp(other_msg))
            // If the claim message text is the same, compare the reason
            // messages (which contain further explanation for the error
            // reported in the claim message).
            .then(self_reasons.iter().cmp(other_reasons.iter()))
    }
}

impl<P: Ord + FileOrd> PartialOrd for Error_<'_, P> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<'a> Error_<'a, Pos<'a>> {
    /// Return a struct with a `std::fmt::Display` implementation that displays
    /// the error in the "raw" format expected by our typecheck test cases.
    pub fn display_raw(&'a self) -> DisplayRaw<'a> {
        DisplayRaw(self)
    }
}

pub struct DisplayRaw<'a>(&'a Error_<'a, Pos<'a>>);

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

const EMPTY_ERRORS_BY_FILE: FilesT<'_, &Error<'_>> = FilesT::from_slice(&[]);
const EMPTY_FIXMES_BY_FILE: FilesT<'_, AppliedFixme<'_>> = FilesT::from_slice(&[]);
const EMPTY_ERRORS: Errors<'_> = Errors(EMPTY_ERRORS_BY_FILE, EMPTY_FIXMES_BY_FILE);

impl<'a> Errors<'a> {
    pub const fn empty() -> Errors<'static> {
        EMPTY_ERRORS
    }

    pub fn is_empty(&self) -> bool {
        let Errors(errors, _fixmes) = self;
        errors.is_empty()
    }

    pub fn into_vec(self) -> Vec<&'a Error<'a>> {
        let Errors(errors, _fixmes) = self;
        errors
            .iter()
            .flat_map(|(_filename, errs_by_phase)| {
                errs_by_phase
                    .iter()
                    .flat_map(|(_phase, errs)| errs.iter())
                    .copied()
            })
            .collect()
    }

    pub fn into_sorted_vec(self) -> Vec<&'a Error<'a>> {
        let mut errors = self.into_vec();
        errors.sort_unstable();
        errors.dedup();
        errors
    }
}

impl std::fmt::Debug for Errors<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Errors(errors, applied_fixmes) = self;
        if errors.is_empty() && applied_fixmes.is_empty() {
            write!(f, "Errors::empty()")
        } else {
            f.debug_struct("Errors")
                .field("errors", errors)
                .field("applied_fixmes", applied_fixmes)
                .finish()
        }
    }
}
