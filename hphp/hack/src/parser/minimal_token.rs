// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;

use crate::{
    lexable_token::LexableToken, minimal_trivia::MinimalTrivia, source_text::SourceText,
    token_kind::TokenKind, trivia_kind::TriviaKind,
};

#[derive(Debug, Clone, PartialEq, OcamlRep)]
pub struct MinimalToken {
    pub kind: TokenKind,
    pub width: usize,
    pub leading: Vec<MinimalTrivia>,
    pub trailing: Vec<MinimalTrivia>,
}

impl<'a> LexableToken<'a> for MinimalToken {
    type Trivia = MinimalTrivia;

    fn kind(&self) -> TokenKind {
        self.kind
    }

    fn make(
        kind: TokenKind,
        _source: &SourceText,
        _offset: usize,
        width: usize,
        leading: Vec<Self::Trivia>,
        trailing: Vec<Self::Trivia>,
    ) -> Self {
        Self {
            kind,
            leading,
            trailing,
            width,
        }
    }

    fn width(&self) -> usize {
        self.width
    }

    fn full_width(&self) -> usize {
        self.leading_width() + self.width() + self.trailing_width()
    }

    fn trailing_width(&self) -> usize {
        self.trailing.iter().map(|x| x.width).sum()
    }

    fn leading_width(&self) -> usize {
        self.leading.iter().map(|x| x.width).sum()
    }

    fn leading_start_offset(&self) -> Option<usize> {
        None // Not available
    }

    fn leading(&self) -> &[Self::Trivia] {
        &self.leading
    }

    fn trailing(&self) -> &[Self::Trivia] {
        &self.trailing
    }

    fn with_trailing(mut self, trailing: Vec<Self::Trivia>) -> Self {
        self.trailing = trailing;
        self
    }

    fn with_leading(mut self, leading: Vec<Self::Trivia>) -> Self {
        self.leading = leading;
        self
    }

    fn with_kind(mut self, kind: TokenKind) -> Self {
        self.kind = kind;
        self
    }

    fn has_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.leading.iter().any(|t| t.kind == kind) || self.trailing.iter().any(|t| t.kind == kind)
    }
}
