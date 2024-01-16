// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;

use crate::lexable_trivia::LexableTrivia;
use crate::minimal_trivia::MinimalTrivium;
use crate::trivia_factory::SimpleTriviaFactory;
use crate::trivia_kind::TriviaKind;

bitflags! {
    #[derive(PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Clone, Copy)]
    pub struct TriviaKinds : u8 {
        const WHITE_SPACE        = 1 << TriviaKind::WhiteSpace as u8;
        const END_OF_LINE        = 1 << TriviaKind::EndOfLine as u8;
        const DELIMITED_COMMENT  = 1 << TriviaKind::DelimitedComment as u8;
        const SINGLELINE_COMMENT = 1 << TriviaKind::SingleLineComment as u8;
        const FIX_ME             = 1 << TriviaKind::FixMe as u8;
        const IGNORE_ERROR       = 1 << TriviaKind::IgnoreError as u8;
        const FALL_THROUGH       = 1 << TriviaKind::FallThrough as u8;
        const EXTRA_TOKEN_ERROR  = 1 << TriviaKind::ExtraTokenError as u8;
        // NB: There are currently exactly 8 TriviaKinds, so we cannot add more
        // without changing the representation of this bitflags type. Making it
        // larger would require changing CompactToken too, since it currently
        // depends upon TriviaKinds fitting into a u8.
    }
}

impl TriviaKinds {
    pub fn from_kind(kind: TriviaKind) -> Self {
        match kind {
            TriviaKind::WhiteSpace => Self::WHITE_SPACE,
            TriviaKind::EndOfLine => Self::END_OF_LINE,
            TriviaKind::DelimitedComment => Self::DELIMITED_COMMENT,
            TriviaKind::SingleLineComment => Self::SINGLELINE_COMMENT,
            TriviaKind::FixMe => Self::FIX_ME,
            TriviaKind::IgnoreError => Self::IGNORE_ERROR,
            TriviaKind::FallThrough => Self::FALL_THROUGH,
            TriviaKind::ExtraTokenError => Self::EXTRA_TOKEN_ERROR,
            // NB: See above note about adding more TriviaKind variants
        }
    }
    pub fn has_kind(self, kind: TriviaKind) -> bool {
        self.contains(Self::from_kind(kind))
    }
}

#[derive(Copy, Clone, Debug)]
pub struct CompactTrivia {
    pub kinds: TriviaKinds,
    pub width: usize,
}

impl CompactTrivia {
    pub fn make(kinds: TriviaKinds, width: usize) -> Self {
        Self { kinds, width }
    }
}

impl LexableTrivia for CompactTrivia {
    type Trivium = MinimalTrivium;

    fn is_empty(&self) -> bool {
        self.kinds.is_empty()
    }
    fn has_kind(&self, kind: TriviaKind) -> bool {
        self.kinds.has_kind(kind)
    }
    fn push(&mut self, trivium: MinimalTrivium) {
        self.kinds |= TriviaKinds::from_kind(trivium.kind);
        self.width += trivium.width;
    }
    fn extend(&mut self, other: Self) {
        self.kinds |= other.kinds;
        self.width += other.width;
    }
}

impl SimpleTriviaFactory for CompactTrivia {
    fn make() -> Self {
        Self {
            kinds: TriviaKinds::empty(),
            width: 0,
        }
    }
}
