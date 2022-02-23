// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::errors::*;
use crate::message::Message;
use crate::pos::Pos;
use crate::pos_or_decl::PosOrDecl;
use crate::quickfix::Quickfix;
use crate::user_error::UserError;

impl<'a> UserError<'a, Pos<'a>, PosOrDecl<'a>> {
    pub fn new(
        code: ErrorCode,
        claim: &'a Message<'a, Pos<'a>>,
        reasons: &'a [&'a Message<'a, PosOrDecl<'a>>],
        quickfixes: &'a [&'a Quickfix<'_>],
    ) -> Self {
        UserError {
            code,
            claim,
            reasons,
            quickfixes,
            is_fixmed: false,
        }
    }

    pub fn pos(&self) -> &Pos<'a> {
        let Message(pos, _msg) = &self.claim;
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

impl<PP: Ord + FileOrd, P: Ord + FileOrd> Ord for UserError<'_, PP, P> {
    // Intended to match the implementation of `compare` in `Errors.sort` in OCaml.
    fn cmp(&self, other: &Self) -> Ordering {
        let Self {
            code: self_code,
            claim: self_claim,
            reasons: self_reasons,
            quickfixes: _,
            is_fixmed: _,
        } = self;
        let Self {
            code: other_code,
            claim: other_claim,
            reasons: other_reasons,
            quickfixes: _,
            is_fixmed: _,
        } = other;
        let Message(self_pos, self_msg) = self_claim;
        let Message(other_pos, other_msg) = other_claim;
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

impl<PP: Ord + FileOrd, P: Ord + FileOrd> PartialOrd for UserError<'_, PP, P> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<'a> UserError<'a, Pos<'a>, PosOrDecl<'a>> {
    /// Return a struct with a `std::fmt::Display` implementation that displays
    /// the error in the "raw" format expected by our typecheck test cases.
    pub fn display_raw(&'a self) -> DisplayRaw<'a> {
        DisplayRaw(self)
    }
}

pub struct DisplayRaw<'a>(&'a UserError<'a, Pos<'a>, PosOrDecl<'a>>);

impl<'a> std::fmt::Display for DisplayRaw<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let UserError {
            code,
            claim,
            reasons,
            quickfixes: _,
            is_fixmed: _,
        } = self.0;
        let Message(pos, msg) = claim;
        let code = DisplayErrorCode(*code);
        write!(f, "{}\n{} ({})", pos.string(), msg, code)?;
        for Message(pos, msg) in reasons.iter() {
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
