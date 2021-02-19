// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    compact_trivia::CompactTrivia,
    syntax_by_ref::positioned_trivia::{self, PositionedTrivia},
    trivia_factory::SimpleTriviaFactoryImpl,
};
use bumpalo::Bump;

pub trait SizedTrivia {
    fn width(&self) -> usize;
}

impl SizedTrivia for CompactTrivia {
    fn width(&self) -> usize {
        self.width
    }
}

impl SizedTrivia for PositionedTrivia<'_> {
    fn width(&self) -> usize {
        self.iter().map(|t| t.width).sum()
    }
}

pub type PositionedToken<'a> = internal::PositionedToken<'a, CompactTrivia>;

pub type TokenFactory<'a> = internal::TokenFactory<'a, SimpleTriviaFactoryImpl<CompactTrivia>>;

impl<'a> internal::TokenFactory<'a, SimpleTriviaFactoryImpl<CompactTrivia>> {
    pub fn new(arena: &'a Bump) -> Self {
        Self {
            arena,
            trivia_factory: SimpleTriviaFactoryImpl::new(),
        }
    }
}

pub type PositionedTokenFullTrivia<'a> = internal::PositionedToken<'a, PositionedTrivia<'a>>;
pub type TokenFactoryFullTrivia<'a> = internal::TokenFactory<'a, positioned_trivia::Factory<'a>>;

impl<'a> internal::TokenFactory<'a, positioned_trivia::Factory<'a>> {
    pub fn new(arena: &'a Bump) -> Self {
        Self {
            arena,
            trivia_factory: positioned_trivia::Factory { arena },
        }
    }
}

pub(crate) mod internal {
    use super::SizedTrivia;
    use crate::{
        compact_trivia::CompactTrivia,
        lexable_token::{LexablePositionedToken, LexableToken},
        lexable_trivia::LexableTrivia,
        positioned_trivia::PositionedTrivium,
        source_text::SourceText,
        token_factory,
        token_kind::TokenKind,
        trivia_factory::SimpleTriviaFactoryImpl,
        trivia_factory::TriviaFactory,
        trivia_kind::TriviaKind,
    };
    use bumpalo::Bump;

    #[derive(Debug, PartialEq)]
    pub struct PositionedTokenImpl<Trivia> {
        pub kind: TokenKind,
        pub offset: usize, // Beginning of first trivia
        pub leading_width: usize,
        pub width: usize, // Width of actual token, not counting trivia
        pub trailing_width: usize,
        pub leading: Trivia,
        pub trailing: Trivia,
    }

    #[derive(Debug, Clone, Copy)]
    pub struct PositionedToken<'a, Trivia>(&'a PositionedTokenImpl<Trivia>);

    impl<Trivia: Clone> PositionedTokenImpl<Trivia> {
        fn start_offset(&self) -> usize {
            self.offset + self.leading_width
        }

        fn end_offset(&self) -> usize {
            let w = self.width;
            let w = if w == 0 { 0 } else { w - 1 };
            self.start_offset() + w
        }

        fn clone(x: &Self) -> Self {
            Self {
                kind: x.kind,
                offset: x.offset,
                leading_width: x.leading_width,
                width: x.width,
                trailing_width: x.trailing_width,
                leading: x.leading.clone(),
                trailing: x.trailing.clone(),
            }
        }
    }

    impl<'a, Trivia: Clone> PositionedToken<'a, Trivia> {
        pub fn start_offset(&self) -> usize {
            self.0.start_offset()
        }

        pub fn end_offset(&self) -> usize {
            self.0.end_offset()
        }

        pub fn inner_ptr_eq(x: &Self, y: &Self) -> bool {
            std::ptr::eq(x.0, y.0)
        }

        pub fn leading(&self) -> &Trivia {
            &self.0.leading
        }

        pub fn trailing(&self) -> &Trivia {
            &self.0.trailing
        }

        pub fn offset(&self) -> usize {
            self.0.offset
        }
    }

    impl<'a, Trivia: LexableTrivia + Clone> LexableToken for PositionedToken<'a, Trivia> {
        type Trivia = Trivia;

        fn kind(&self) -> TokenKind {
            self.0.kind
        }

        fn leading_start_offset(&self) -> Option<usize> {
            Some(self.0.offset)
        }

        fn width(&self) -> usize {
            self.0.width
        }

        fn leading_width(&self) -> usize {
            self.0.leading_width
        }

        fn trailing_width(&self) -> usize {
            self.0.trailing_width
        }

        fn full_width(&self) -> usize {
            self.0.leading_width + self.0.width + self.0.trailing_width
        }

        fn clone_leading(&self) -> Self::Trivia {
            self.0.leading.clone()
        }

        fn clone_trailing(&self) -> Self::Trivia {
            self.0.trailing.clone()
        }

        fn leading_is_empty(&self) -> bool {
            self.0.leading.is_empty()
        }

        fn trailing_is_empty(&self) -> bool {
            self.0.trailing.is_empty()
        }

        fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
            self.0.leading.has_kind(kind)
        }

        fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
            self.0.trailing.has_kind(kind)
        }

        fn into_trivia_and_width(self) -> (Self::Trivia, usize, Self::Trivia) {
            (
                self.0.leading.clone(),
                self.width(),
                self.0.trailing.clone(),
            )
        }
    }

    #[derive(Clone)]
    pub struct TokenFactory<'a, TriviaFactory> {
        pub arena: &'a Bump,
        pub trivia_factory: TriviaFactory,
    }

    impl<'a, TF: TriviaFactory + Clone> token_factory::TokenFactory for TokenFactory<'a, TF>
    where
        TF::Trivia: SizedTrivia + 'a,
    {
        type Token = PositionedToken<'a, TF::Trivia>;
        type TriviaFactory = TF;

        fn make(
            &mut self,
            kind: TokenKind,
            offset: usize,
            width: usize,
            leading: TF::Trivia,
            trailing: TF::Trivia,
        ) -> Self::Token {
            PositionedToken(self.arena.alloc(PositionedTokenImpl {
                kind,
                offset,
                leading_width: leading.width(),
                width,
                trailing_width: trailing.width(),
                leading,
                trailing,
            }))
        }

        fn with_leading(&mut self, token: Self::Token, leading: TF::Trivia) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(token.0);
            let token_start_offset = token.0.offset + token.0.leading_width;
            new.offset = token_start_offset - leading.width();
            new.leading_width = leading.width();
            new.leading = leading;
            PositionedToken(self.arena.alloc(new))
        }

        fn with_trailing(&mut self, token: Self::Token, trailing: TF::Trivia) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(token.0);
            new.trailing_width = trailing.width();
            new.trailing = trailing;
            PositionedToken(self.arena.alloc(new))
        }

        fn with_kind(&mut self, token: Self::Token, kind: TokenKind) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(token.0);
            new.kind = kind;
            PositionedToken(self.arena.alloc(new))
        }

        fn trivia_factory_mut(&mut self) -> &mut Self::TriviaFactory {
            &mut self.trivia_factory
        }
    }

    impl<'a> token_factory::TokenMutator for TokenFactory<'a, SimpleTriviaFactoryImpl<CompactTrivia>> {
        fn trim_left(&mut self, t: &Self::Token, n: usize) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(t.0);
            let leading = t.clone_leading();
            new.leading_width = leading.width + n;
            new.width = t.width() - n;
            PositionedToken(self.arena.alloc(new))
        }

        fn trim_right(&mut self, t: &Self::Token, n: usize) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(t.0);
            let trailing = t.clone_trailing();
            new.trailing_width = trailing.width + n;
            new.width = t.width() - n;
            PositionedToken(self.arena.alloc(new))
        }

        fn concatenate(&mut self, s: &Self::Token, e: &Self::Token) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(s.0);
            new.width = e.end_offset() + 1 - s.start_offset();
            let e_trailing = e.clone_trailing();
            new.trailing_width = e_trailing.width;
            new.trailing = e_trailing;
            PositionedToken(self.arena.alloc(new))
        }
    }

    impl<'a, Trivia: LexableTrivia> LexablePositionedToken for PositionedToken<'a, Trivia> {
        fn text<'b>(&self, source_text: &'b SourceText) -> &'b str {
            source_text.sub_as_str(self.0.start_offset(), self.0.width)
        }

        fn text_raw<'b>(&self, source_text: &'b SourceText) -> &'b [u8] {
            source_text.sub(self.0.start_offset(), self.0.width)
        }

        fn clone_value(&self) -> Self {
            self.clone()
        }

        fn positioned_leading(&self) -> &[PositionedTrivium] {
            unimplemented!()
        }

        fn positioned_trailing(&self) -> &[PositionedTrivium] {
            unimplemented!()
        }
    }
}
