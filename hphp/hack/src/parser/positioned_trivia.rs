// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexable_trivia::LexableTrivia;
use crate::source_text::SourceText;
use crate::trivia_kind::TriviaKind;

#[derive(Debug, Clone, PartialEq)]
pub struct PositionedTrivia {
    pub kind: TriviaKind,
    pub offset: usize,
    pub width: usize,
}

impl LexableTrivia for PositionedTrivia {
    fn kind(&self) -> TriviaKind {
        self.kind
    }

    fn width(&self) -> usize {
        self.width
    }

    fn make_whitespace(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::WhiteSpace,
            offset,
            width,
        }
    }

    fn make_eol(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::EndOfLine,
            offset,
            width,
        }
    }

    fn make_single_line_comment(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::SingleLineComment,
            offset,
            width,
        }
    }

    fn make_fallthrough(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::FallThrough,
            offset,
            width,
        }
    }

    fn make_fix_me(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::FixMe,
            offset,
            width,
        }
    }

    fn make_ignore_error(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::IgnoreError,
            offset,
            width,
        }
    }

    fn make_extra_token_error(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::ExtraTokenError,
            offset,
            width,
        }
    }

    fn make_delimited_comment(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::DelimitedComment,
            offset,
            width,
        }
    }

    fn make_after_halt_compiler(_source: &SourceText, offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::AfterHaltCompiler,
            offset,
            width,
        }
    }
}
