// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::ToOcamlRep;

use crate::lexable_trivia::LexableTrivia;
use crate::lexable_trivia::LexableTrivium;
use crate::source_text::SourceText;
use crate::trivia_kind::TriviaKind;

#[derive(Debug, Clone, PartialEq, ToOcamlRep)]
pub struct PositionedTrivium {
    pub kind: TriviaKind,
    pub offset: usize,
    pub width: usize,
}

pub type PositionedTrivia = Vec<PositionedTrivium>;

impl PositionedTrivium {
    pub fn start_offset(&self) -> usize {
        self.offset
    }

    pub fn end_offset(&self) -> usize {
        self.offset + self.width - 1
    }

    pub fn text_raw<'b>(&self, source_text: &'b SourceText<'_>) -> &'b [u8] {
        source_text.sub(self.start_offset(), self.width())
    }
}

impl LexableTrivia for PositionedTrivia {
    type Trivium = PositionedTrivium;

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

impl LexableTrivium for PositionedTrivium {
    fn kind(&self) -> TriviaKind {
        self.kind
    }

    fn width(&self) -> usize {
        self.width
    }

    fn make_whitespace(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::WhiteSpace,
            offset,
            width,
        }
    }

    fn make_eol(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::EndOfLine,
            offset,
            width,
        }
    }

    fn make_single_line_comment(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::SingleLineComment,
            offset,
            width,
        }
    }

    fn make_fallthrough(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::FallThrough,
            offset,
            width,
        }
    }

    fn make_fix_me(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::FixMe,
            offset,
            width,
        }
    }

    fn make_ignore_error(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::IgnoreError,
            offset,
            width,
        }
    }

    fn make_extra_token_error(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::ExtraTokenError,
            offset,
            width,
        }
    }

    fn make_delimited_comment(offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::DelimitedComment,
            offset,
            width,
        }
    }
}
