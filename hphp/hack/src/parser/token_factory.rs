// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{lexable_token::LexableToken, lexable_trivia::LexableTrivia, token_kind::TokenKind};

pub type Trivia<TF> = <<TF as TokenFactory>::Token as LexableToken>::Trivia;
pub type Trivium<TF> =
    <<<TF as TokenFactory>::Token as LexableToken>::Trivia as LexableTrivia>::Trivium;

pub trait TokenFactory: Clone {
    type Token: LexableToken;

    fn make(
        &mut self,
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: Trivia<Self>,
        trailing: Trivia<Self>,
    ) -> Self::Token;
}

pub trait SimpleTokenFactory: LexableToken {
    fn make(
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: <Self as LexableToken>::Trivia,
        trailing: <Self as LexableToken>::Trivia,
    ) -> Self;
}

#[derive(Clone)]
pub struct SimpleTokenFactoryImpl<Token: SimpleTokenFactory>(std::marker::PhantomData<Token>);

impl<Token: SimpleTokenFactory> SimpleTokenFactoryImpl<Token> {
    pub fn new() -> Self {
        Self(std::marker::PhantomData)
    }
}

impl<T: SimpleTokenFactory> TokenFactory for SimpleTokenFactoryImpl<T> {
    type Token = T;

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
}
