/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

#[derive(Debug, Copy, Clone, FromOcamlRep, ToOcamlRep, PartialEq)]
#[repr(u8)]
pub enum TriviaKind {
    WhiteSpace = 0,
    EndOfLine = 1,
    DelimitedComment = 2,
    SingleLineComment = 3,
    FixMe = 4,
    IgnoreError = 5,
    FallThrough = 6,
    ExtraTokenError = 7,
}

impl TriviaKind {
    pub fn to_string(&self) -> &str {
        match self {
            TriviaKind::WhiteSpace => "whitespace",
            TriviaKind::EndOfLine => "end_of_line",
            TriviaKind::DelimitedComment => "delimited_comment",
            TriviaKind::SingleLineComment => "single_line_comment",
            TriviaKind::FixMe => "fix_me",
            TriviaKind::IgnoreError => "ignore_error",
            TriviaKind::FallThrough => "fall_through",
            TriviaKind::ExtraTokenError => "extra_token_error",
        }
    }

    pub const fn ocaml_tag(self) -> u8 {
        self as u8
    }
}
