// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::source_text::SourceText;
use crate::trivia_kind::TriviaKind;

pub trait LexableTrivia: Clone {
    type Trivium: LexableTrivium;

    fn new() -> Self;
    fn from_slice(trivia: &[Self::Trivium]) -> Self;
    fn is_empty(&self) -> bool;
    fn has_kind(&self, kind: TriviaKind) -> bool;
    fn push(&mut self, trivium: Self::Trivium);
    fn extend(&mut self, other: Self);

    #[inline]
    fn make_whitespace(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_whitespace(source, offset, width)
    }
    #[inline]
    fn make_eol(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_eol(source, offset, width)
    }
    #[inline]
    fn make_single_line_comment(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_single_line_comment(source, offset, width)
    }
    #[inline]
    fn make_fallthrough(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_fallthrough(source, offset, width)
    }
    #[inline]
    fn make_fix_me(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_fix_me(source, offset, width)
    }
    #[inline]
    fn make_ignore_error(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_ignore_error(source, offset, width)
    }
    #[inline]
    fn make_extra_token_error(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_extra_token_error(source, offset, width)
    }
    #[inline]
    fn make_delimited_comment(source: &SourceText, offset: usize, width: usize) -> Self::Trivium {
        Self::Trivium::make_delimited_comment(source, offset, width)
    }
}

pub trait LexableTrivium: Clone + PartialEq {
    fn make_whitespace(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_eol(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_single_line_comment(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_fallthrough(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_fix_me(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_ignore_error(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_extra_token_error(source: &SourceText, offset: usize, width: usize) -> Self;
    fn make_delimited_comment(source: &SourceText, offset: usize, width: usize) -> Self;

    fn kind(&self) -> TriviaKind;
    fn width(&self) -> usize;
}
