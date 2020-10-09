// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

use crate::{
    lexable_token::LexableToken, lexable_trivia::LexableTrivia, minimal_trivia::MinimalTrivia,
    token_factory::SimpleTokenFactory, token_kind::TokenKind, trivia_kind::TriviaKind,
};

#[derive(Debug, Clone, PartialEq, FromOcamlRep, ToOcamlRep)]
pub struct MinimalToken {
    pub kind: TokenKind,
    pub width: usize,
    pub leading: MinimalTrivia,
    pub trailing: MinimalTrivia,
}

impl MinimalToken {
    pub fn make(
        kind: TokenKind,
        _offset: usize,
        width: usize,
        leading: MinimalTrivia,
        trailing: MinimalTrivia,
    ) -> Self {
        Self {
            kind,
            leading,
            trailing,
            width,
        }
    }
}

impl SimpleTokenFactory for MinimalToken {
    fn make(
        kind: TokenKind,
        _offset: usize,
        width: usize,
        leading: MinimalTrivia,
        trailing: MinimalTrivia,
    ) -> Self {
        Self {
            kind,
            leading,
            trailing,
            width,
        }
    }

    fn with_trailing(mut self, trailing: MinimalTrivia) -> Self {
        self.trailing = trailing;
        self
    }

    fn with_leading(mut self, leading: MinimalTrivia) -> Self {
        self.leading = leading;
        self
    }

    fn with_kind(mut self, kind: TokenKind) -> Self {
        self.kind = kind;
        self
    }
}

impl LexableToken for MinimalToken {
    type Trivia = MinimalTrivia;

    fn kind(&self) -> TokenKind {
        self.kind
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

    fn clone_leading(&self) -> MinimalTrivia {
        self.leading.clone()
    }

    fn clone_trailing(&self) -> MinimalTrivia {
        self.trailing.clone()
    }

    fn leading_is_empty(&self) -> bool {
        self.leading.is_empty()
    }

    fn trailing_is_empty(&self) -> bool {
        self.trailing.is_empty()
    }

    fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.leading.has_kind(kind)
    }

    fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.trailing.has_kind(kind)
    }

    fn into_trivia_and_width(self) -> (Self::Trivia, usize, Self::Trivia) {
        (self.leading, self.width, self.trailing)
    }
}
