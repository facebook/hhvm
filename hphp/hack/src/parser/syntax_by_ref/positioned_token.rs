// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use crate::compact_trivia::CompactTrivia;
use crate::compact_trivia::TriviaKinds;
use crate::syntax_by_ref::positioned_trivia;
use crate::syntax_by_ref::positioned_trivia::PositionedTrivia;
use crate::trivia_factory::SimpleTriviaFactoryImpl;

pub type PositionedToken<'a> = internal::PositionedToken<'a, usize>;

pub type TokenFactory<'a> =
    internal::TokenFactory<'a, SimpleTriviaFactoryImpl<CompactTrivia>, usize>;

impl internal::TriviaRep for usize {
    type Trivia = CompactTrivia;

    fn from_trivia(t: Self::Trivia) -> Self {
        t.width
    }

    fn clone_trivia(&self, kinds: TriviaKinds) -> Self::Trivia {
        CompactTrivia {
            kinds,
            width: *self,
        }
    }

    fn width(&self) -> usize {
        *self
    }

    fn is_empty(&self) -> bool {
        *self == 0
    }
}

impl internal::SizedTrivia for CompactTrivia {
    fn kinds(&self) -> TriviaKinds {
        self.kinds
    }

    fn width(&self) -> usize {
        self.width
    }
}

impl<'a> internal::TokenFactory<'a, SimpleTriviaFactoryImpl<CompactTrivia>, usize> {
    pub fn new(arena: &'a Bump) -> Self {
        Self {
            arena,
            trivia_factory: SimpleTriviaFactoryImpl::new(),
            _phantom_data: std::marker::PhantomData,
        }
    }
}

pub type PositionedTokenFullTrivia<'a> = internal::PositionedToken<'a, PositionedTrivia<'a>>;

pub type TokenFactoryFullTrivia<'a> =
    internal::TokenFactory<'a, positioned_trivia::Factory<'a>, PositionedTrivia<'a>>;

impl<'a> internal::TriviaRep for PositionedTrivia<'a> {
    type Trivia = PositionedTrivia<'a>;

    fn from_trivia(t: Self::Trivia) -> Self {
        t
    }

    fn clone_trivia(&self, _: TriviaKinds) -> Self::Trivia {
        self.clone()
    }

    fn width(&self) -> usize {
        self.iter().map(|t| t.width).sum()
    }

    fn is_empty(&self) -> bool {
        self.is_empty()
    }
}

impl internal::SizedTrivia for PositionedTrivia<'_> {
    fn kinds(&self) -> TriviaKinds {
        self.iter().fold(TriviaKinds::empty(), |k, t| {
            k | TriviaKinds::from_kind(t.kind)
        })
    }

    fn width(&self) -> usize {
        self.iter().map(|t| t.width).sum()
    }
}

impl<'a> internal::TokenFactory<'a, positioned_trivia::Factory<'a>, PositionedTrivia<'a>> {
    pub fn new(arena: &'a Bump) -> Self {
        Self {
            arena,
            trivia_factory: positioned_trivia::Factory { arena },
            _phantom_data: std::marker::PhantomData,
        }
    }
}

pub(crate) mod internal {
    use bumpalo::Bump;

    use crate::compact_trivia::CompactTrivia;
    use crate::compact_trivia::TriviaKinds;
    use crate::lexable_token::LexablePositionedToken;
    use crate::lexable_token::LexableToken;
    use crate::lexable_trivia::LexableTrivia;
    use crate::positioned_trivia::PositionedTrivium;
    use crate::source_text::SourceText;
    use crate::token_factory;
    use crate::token_kind::TokenKind;
    use crate::trivia_factory::SimpleTriviaFactoryImpl;
    use crate::trivia_factory::TriviaFactory;
    use crate::trivia_kind::TriviaKind;

    pub trait SizedTrivia {
        fn kinds(&self) -> TriviaKinds;
        fn width(&self) -> usize;
    }

    pub trait TriviaRep: std::fmt::Debug {
        type Trivia: LexableTrivia;
        fn from_trivia(t: Self::Trivia) -> Self;
        fn clone_trivia(&self, kinds: TriviaKinds) -> Self::Trivia;
        fn width(&self) -> usize;
        fn is_empty(&self) -> bool;
    }

    #[derive(Debug, PartialEq)]
    pub struct PositionedTokenImpl<TriviaRep> {
        pub kind: TokenKind,
        pub offset: usize, // Beginning of first trivia
        pub width: usize,  // Width of actual token, not counting trivia
        pub leading_kinds: TriviaKinds,
        pub trailing_kinds: TriviaKinds,
        pub leading: TriviaRep,
        pub trailing: TriviaRep,
    }

    #[derive(Debug)]
    pub struct PositionedToken<'a, TriviaRep>(&'a PositionedTokenImpl<TriviaRep>);

    // derive(Clone) requires Trivia implements Clone, which isn't necessary.
    impl<'a, TriviaRep> Clone for PositionedToken<'a, TriviaRep> {
        fn clone(&self) -> Self {
            Self(self.0)
        }
    }

    // derive(Copy) requires Trivia implements Copy, which isn't necessary.
    impl<'a, TriviaRep> Copy for PositionedToken<'a, TriviaRep> {}

    impl<TR: TriviaRep + Clone> PositionedTokenImpl<TR> {
        fn start_offset(&self) -> usize {
            self.offset + self.leading.width()
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
                width: x.width,
                leading_kinds: x.leading_kinds,
                trailing_kinds: x.trailing_kinds,
                leading: x.leading.clone(),
                trailing: x.trailing.clone(),
            }
        }
    }

    impl<'a, TR: TriviaRep + Clone> PositionedToken<'a, TR> {
        pub fn start_offset(&self) -> usize {
            self.0.start_offset()
        }

        pub fn end_offset(&self) -> usize {
            self.0.end_offset()
        }

        pub fn inner_ptr_eq(x: &Self, y: &Self) -> bool {
            std::ptr::eq(x.0, y.0)
        }

        pub fn offset(&self) -> usize {
            self.0.offset
        }

        pub fn leading_kinds(&self) -> TriviaKinds {
            self.0.leading_kinds
        }

        pub fn trailing_kinds(&self) -> TriviaKinds {
            self.0.trailing_kinds
        }
    }

    impl<'a, TR: TriviaRep + Clone> LexableToken for PositionedToken<'a, TR>
    where
        TR::Trivia: Clone,
    {
        type Trivia = TR::Trivia;

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
            self.0.leading.width()
        }

        fn trailing_width(&self) -> usize {
            self.0.trailing.width()
        }

        fn full_width(&self) -> usize {
            self.0.leading.width() + self.0.width + self.0.trailing.width()
        }

        fn clone_leading(&self) -> Self::Trivia {
            self.0.leading.clone_trivia(self.0.leading_kinds)
        }

        fn clone_trailing(&self) -> Self::Trivia {
            self.0.trailing.clone_trivia(self.0.trailing_kinds)
        }

        fn leading_is_empty(&self) -> bool {
            self.0.leading.is_empty()
        }

        fn trailing_is_empty(&self) -> bool {
            self.0.trailing.is_empty()
        }

        fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
            self.0.leading_kinds.has_kind(kind)
        }

        fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
            self.0.trailing_kinds.has_kind(kind)
        }

        fn into_trivia_and_width(self) -> (Self::Trivia, usize, Self::Trivia) {
            (self.clone_leading(), self.width(), self.clone_trailing())
        }
    }

    #[derive(Clone)]
    pub struct TokenFactory<'a, TriviaFactory, TriviaRep> {
        pub arena: &'a Bump,
        pub trivia_factory: TriviaFactory,
        pub _phantom_data: std::marker::PhantomData<TriviaRep>,
    }

    impl<'a, TR, TF> token_factory::TokenFactory for TokenFactory<'a, TF, TR>
    where
        TF: TriviaFactory + Clone,
        TF::Trivia: SizedTrivia + 'a,
        TR: TriviaRep<Trivia = TF::Trivia> + Clone + 'a,
    {
        type Token = PositionedToken<'a, TR>;
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
                width,
                leading_kinds: leading.kinds(),
                trailing_kinds: trailing.kinds(),
                leading: TR::from_trivia(leading),
                trailing: TR::from_trivia(trailing),
            }))
        }

        fn with_leading(&mut self, token: Self::Token, leading: TF::Trivia) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(token.0);
            let token_start_offset = token.0.offset + token.0.leading.width();
            new.offset = token_start_offset - leading.width();
            new.leading_kinds = leading.kinds();
            new.leading = TR::from_trivia(leading);
            PositionedToken(self.arena.alloc(new))
        }

        fn with_trailing(&mut self, token: Self::Token, trailing: TF::Trivia) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(token.0);
            new.trailing_kinds = trailing.kinds();
            new.trailing = TR::from_trivia(trailing);
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

    impl<'a> token_factory::TokenMutator
        for TokenFactory<'a, SimpleTriviaFactoryImpl<CompactTrivia>, usize>
    {
        fn trim_left(&mut self, t: &Self::Token, n: usize) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(t.0);
            new.leading += n;
            new.width = t.width() - n;
            PositionedToken(self.arena.alloc(new))
        }

        fn trim_right(&mut self, t: &Self::Token, n: usize) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(t.0);
            new.trailing += n;
            new.width = t.width() - n;
            PositionedToken(self.arena.alloc(new))
        }

        fn concatenate(&mut self, s: &Self::Token, e: &Self::Token) -> Self::Token {
            let mut new = PositionedTokenImpl::clone(s.0);
            new.width = e.end_offset() + 1 - s.start_offset();
            let e_trailing = e.clone_trailing();
            new.trailing = usize::from_trivia(e_trailing);
            PositionedToken(self.arena.alloc(new))
        }
    }

    impl<'a, TR: TriviaRep + Clone> LexablePositionedToken for PositionedToken<'a, TR> {
        fn text<'b>(&self, source_text: &'b SourceText<'_>) -> &'b str {
            source_text.sub_as_str(self.0.start_offset(), self.0.width)
        }

        fn text_raw<'b>(&self, source_text: &'b SourceText<'_>) -> &'b [u8] {
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
