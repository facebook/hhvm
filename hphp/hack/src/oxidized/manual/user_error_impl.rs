// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::DefaultHasher;
use std::hash::Hash;

use ansi_term::Color;
use hh_hash::Hasher;
use ocamlrep::OCamlInt;
use rc_pos::with_erased_lines::WithErasedLines;

use crate::user_error::Severity;
use crate::user_error::UserError;
use crate::warnings_saved_state::ErrorHash;

impl Severity {
    pub fn to_all_caps_string(&self) -> &'static str {
        match self {
            Severity::Err => "ERROR",
            Severity::Warning => "WARN",
        }
    }

    pub fn color(&self) -> Color {
        match self {
            Severity::Err => Color::Red,
            Severity::Warning => Color::Yellow,
        }
    }
}

impl<PrimPos: Hash + WithErasedLines + Clone, Pos: Hash + WithErasedLines + Clone>
    UserError<PrimPos, Pos>
{
    pub fn hash_for_saved_state(&self) -> ErrorHash {
        let mut hasher = DefaultHasher::new();
        self.clone().with_erased_lines().hash(&mut hasher);
        let hash = hasher.finish();
        OCamlInt::new_erase_msb(hash as isize)
    }
}

impl<PrimPos: WithErasedLines, Pos: WithErasedLines> WithErasedLines for UserError<PrimPos, Pos> {
    fn with_erased_lines(self) -> UserError<PrimPos, Pos> {
        let UserError {
            severity,
            code,
            claim,
            reasons,
            explanation,
            quickfixes,
            custom_msgs,
            is_fixmed,
            flags,
        } = self;
        UserError {
            severity,
            code,
            claim: claim.with_erased_lines(),
            reasons: reasons.into_iter().map(|r| r.with_erased_lines()).collect(),
            explanation,
            quickfixes: quickfixes
                .into_iter()
                .map(|q| q.with_erased_lines())
                .collect(),
            custom_msgs,
            is_fixmed,
            flags,
        }
    }
}
