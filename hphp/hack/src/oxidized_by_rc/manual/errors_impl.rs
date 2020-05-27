// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::errors::*;
use crate::pos::Pos;

impl<P> Error_<P> {
    pub fn new(code: ErrorCode, messages: Vec<Message<P>>) -> Self {
        Error_(code, messages)
    }

    pub fn pos(&self) -> &P {
        let Self(_code, messages) = self;
        let (pos, _msg) = messages.first().unwrap();
        pos
    }

    pub fn code(&self) -> ErrorCode {
        self.0
    }
}

impl<P: Ord> Ord for Error_<P> {
    // Intended to match the implementation of `compare` in `Errors.sort` in OCaml.
    fn cmp(&self, other: &Self) -> Ordering {
        let Self(self_code, self_messages) = self;
        let Self(other_code, other_messages) = other;
        let (self_first, other_first) = match (self_messages.first(), other_messages.first()) {
            (None, None) => return Ordering::Equal,
            (Some(_), None) => return Ordering::Greater,
            (None, Some(_)) => return Ordering::Less,
            (Some(self_first), Some(other_first)) => (self_first, other_first),
        };
        let (self_pos, self_msg) = self_first;
        let (other_pos, other_msg) = other_first;
        // The primary sort order is the position of the first message.
        self_pos.cmp(other_pos)
            // If the positions are the same, sort by error code.
            .then((*self_code as isize).cmp(&(*other_code as isize)))
            // If the error codes are the same, sort by message text.
            .then(self_msg.cmp(other_msg))
            // If the first message text is the same, compare the rest of the
            // messages (which contain further explanation for the error
            // reported in the first message).
            .then(self_messages.iter().skip(1).cmp(other_messages.iter().skip(1)))
    }
}

impl<P: Ord> PartialOrd for Error_<P> {
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
        let Error_(code, messages) = self.0;
        let (pos, msg) = messages.first().unwrap();
        let code = DisplayErrorCode(*code);
        write!(f, "{}\n{} ({})", pos.string(), msg, code)?;
        for (pos, msg) in messages.iter().skip(1) {
            write!(f, "  {}\n{}", pos.string(), msg)?;
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
