/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use crate::lexable_trivia::LexableTrivia;
use crate::source_text::SourceText;
use crate::trivia_kind::TriviaKind;

#[derive(Debug, Clone, PartialEq)]
pub struct MinimalTrivia {
    pub kind: TriviaKind,
    pub width: usize,
}

impl LexableTrivia for MinimalTrivia {
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

    fn make_unsafe(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::Unsafe,
            width,
        }
    }

    fn make_unsafe_expression(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::UnsafeExpression,
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

    fn make_after_halt_compiler(_source: &SourceText, _offset: usize, width: usize) -> Self {
        Self {
            kind: TriviaKind::AfterHaltCompiler,
            width,
        }
    }
}
