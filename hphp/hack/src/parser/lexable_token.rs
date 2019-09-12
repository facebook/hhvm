// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexable_trivia::LexableTrivia;
use crate::source_text::SourceText;
use crate::token_kind::TokenKind;
use std::fmt::Debug;

pub trait LexableToken<'a>: Clone {
    type Trivia: LexableTrivia;
    fn make(
        kind: TokenKind,
        source_text: &SourceText<'a>,
        offset: usize,
        width: usize,
        leading: Vec<Self::Trivia>,
        trailing: Vec<Self::Trivia>,
    ) -> Self;
    fn kind(&self) -> TokenKind;

    /// Returns the leading offset if meaningful
    /// (note: each implementor will either always return Some(offset) or always return None).
    fn leading_start_offset(&self) -> Option<usize>;

    fn width(&self) -> usize;
    fn leading_width(&self) -> usize;
    fn trailing_width(&self) -> usize;
    fn full_width(&self) -> usize;

    fn leading(&self) -> &[Self::Trivia];
    fn trailing(&self) -> &[Self::Trivia];

    fn with_leading(self, trailing: Vec<Self::Trivia>) -> Self;
    fn with_trailing(self, trailing: Vec<Self::Trivia>) -> Self;
    fn with_kind(self, kind: TokenKind) -> Self;
}

pub trait LexablePositionedToken<'a>: LexableToken<'a>
where
    Self: Debug,
{
}
