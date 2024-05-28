// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::DefaultHasher;
use std::hash::Hash;

use ansi_term::Color;
use hh_hash::Hasher;

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

impl<PrimPos: Hash, Pos: Hash> UserError<PrimPos, Pos> {
    pub fn hash_for_saved_state(&self) -> ErrorHash {
        let mut hasher = DefaultHasher::new();
        self.hash(&mut hasher);
        let hash = hasher.finish();
        make_msb_equal_msb2(hash) as isize
    }
}

fn make_msb_equal_msb2(u: u64) -> u64 {
    let msb2 = u >> 62 & 1;
    let with_unset_msb = u & !(1 << 63);
    with_unset_msb | (msb2 << 63)
}
