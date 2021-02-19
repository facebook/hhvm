// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    lexable_trivia::LexableTrivia, positioned_trivia::PositionedTrivium,
    trivia_factory::TriviaFactory, trivia_kind::TriviaKind,
};
use bumpalo::{collections::Vec, Bump};

pub type PositionedTrivia<'a> = Vec<'a, PositionedTrivium>;

impl<'a> LexableTrivia for PositionedTrivia<'a> {
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

#[derive(Clone, Debug)]
pub struct Factory<'a> {
    pub arena: &'a Bump,
}

impl<'a> TriviaFactory for Factory<'a> {
    type Trivia = PositionedTrivia<'a>;

    fn make(&mut self) -> Self::Trivia {
        Vec::new_in(self.arena)
    }
}
