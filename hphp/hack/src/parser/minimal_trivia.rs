// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

use crate::lexable_trivia::{LexableTrivia, LexableTrivium};
use crate::source_text::SourceText;
use crate::trivia_kind::TriviaKind;

#[derive(Debug, Clone, FromOcamlRep, ToOcamlRep, PartialEq)]
pub struct MinimalTrivium {
    pub kind: TriviaKind,
    pub width: usize,
}

pub type MinimalTrivia = Vec<MinimalTrivium>;

impl LexableTrivia for MinimalTrivia {
    type Trivium = MinimalTrivium;

    fn new() -> Self {
        Vec::new()
    }
    fn from_slice(trivia: &[Self::Trivium]) -> Self {
        trivia.to_vec()
    }
    fn is_empty(&self) -> bool {
        self.is_empty()
    }
    fn has_kind(&self, kind: TriviaKind) -> bool {
        self.iter().any(|t| t.kind == kind)
    }
    fn push(&mut self, trivium: Self::Trivium) {
        self.push(trivium)
    }
    fn extend(&mut self, other: Self) {
        for trivium in other {
            self.push(trivium)
        }
    }
}

impl LexableTrivium for MinimalTrivium {
    fn kind(&self) -> TriviaKind {
        self.kind
    }

    fn width(&self) -> usize {
        self.width
    }

    fn make_whitespace(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::WhiteSpace,
            width,
        }
    }

    fn make_eol(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::EndOfLine,
            width,
        }
    }

    fn make_single_line_comment(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::SingleLineComment,
            width,
        }
    }

    fn make_fallthrough(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::FallThrough,
            width,
        }
    }

    fn make_fix_me(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::FixMe,
            width,
        }
    }

    fn make_ignore_error(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::IgnoreError,
            width,
        }
    }

    fn make_extra_token_error(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::ExtraTokenError,
            width,
        }
    }

    fn make_delimited_comment(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::DelimitedComment,
            width,
        }
    }
}
