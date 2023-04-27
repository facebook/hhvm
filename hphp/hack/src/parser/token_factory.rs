// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;

use crate::lexable_token::LexableToken;
use crate::lexable_trivia::LexableTrivia;
use crate::token_kind::TokenKind;
use crate::trivia_factory::SimpleTriviaFactory;
use crate::trivia_factory::SimpleTriviaFactoryImpl;
use crate::trivia_factory::TriviaFactory;

pub type Trivia<TF> = <<TF as TokenFactory>::Token as LexableToken>::Trivia;
pub type Trivium<TF> =
    <<<TF as TokenFactory>::Token as LexableToken>::Trivia as LexableTrivia>::Trivium;

pub trait TokenFactory: Clone {
    type Token: LexableToken;
    type TriviaFactory: TriviaFactory<Trivia = Trivia<Self>>;

    fn make(
        &mut self,
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: Trivia<Self>,
        trailing: Trivia<Self>,
    ) -> Self::Token;

    fn with_leading(&mut self, token: Self::Token, trailing: Trivia<Self>) -> Self::Token;
    fn with_trailing(&mut self, token: Self::Token, trailing: Trivia<Self>) -> Self::Token;
    fn with_kind(&mut self, token: Self::Token, kind: TokenKind) -> Self::Token;

    fn trivia_factory_mut(&mut self) -> &mut Self::TriviaFactory;
}

pub trait SimpleTokenFactory: LexableToken {
    fn make(
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: <Self as LexableToken>::Trivia,
        trailing: <Self as LexableToken>::Trivia,
    ) -> Self;

    fn with_leading(self, leading: <Self as LexableToken>::Trivia) -> Self;
    fn with_trailing(self, trailing: <Self as LexableToken>::Trivia) -> Self;
    fn with_kind(self, kind: TokenKind) -> Self;
}

#[derive(Clone)]
pub struct SimpleTokenFactoryImpl<Token: SimpleTokenFactory>(
    std::marker::PhantomData<Token>,
    SimpleTriviaFactoryImpl<<Token as LexableToken>::Trivia>,
);

impl<Token: SimpleTokenFactory> SimpleTokenFactoryImpl<Token> {
    pub fn new() -> Self {
        Self(std::marker::PhantomData, SimpleTriviaFactoryImpl::new())
    }
}

impl<T> TokenFactory for SimpleTokenFactoryImpl<T>
where
    T: SimpleTokenFactory + Debug,
    <T as LexableToken>::Trivia: SimpleTriviaFactory,
{
    type Token = T;
    type TriviaFactory = SimpleTriviaFactoryImpl<<T as LexableToken>::Trivia>;

    fn make(
        &mut self,
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: Trivia<Self>,
        trailing: Trivia<Self>,
    ) -> Self::Token {
        T::make(kind, offset, width, leading, trailing)
    }

    fn with_leading(
        &mut self,
        token: Self::Token,
        leading: <Self::Token as LexableToken>::Trivia,
    ) -> Self::Token {
        token.with_leading(leading)
    }

    fn with_trailing(
        &mut self,
        token: Self::Token,
        trailing: <Self::Token as LexableToken>::Trivia,
    ) -> Self::Token {
        token.with_trailing(trailing)
    }

    fn with_kind(&mut self, token: Self::Token, kind: TokenKind) -> Self::Token {
        token.with_kind(kind)
    }

    fn trivia_factory_mut(&mut self) -> &mut Self::TriviaFactory {
        &mut self.1
    }
}

pub trait TokenMutator: TokenFactory {
    fn trim_left(&mut self, t: &Self::Token, n: usize) -> Self::Token;
    fn trim_right(&mut self, t: &Self::Token, n: usize) -> Self::Token;
    fn concatenate(&mut self, s: &Self::Token, e: &Self::Token) -> Self::Token;
}
