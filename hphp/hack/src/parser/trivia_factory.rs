// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexable_trivia::LexableTrivia;

pub trait TriviaFactory {
    type Trivia: LexableTrivia;

    fn make(&mut self) -> Self::Trivia;
}

pub trait SimpleTriviaFactory: Sized {
    fn make() -> Self;
}
#[derive(Clone)]
pub struct SimpleTriviaFactoryImpl<T>(std::marker::PhantomData<T>);

impl<T> SimpleTriviaFactoryImpl<T> {
    pub fn new() -> Self {
        Self(std::marker::PhantomData)
    }
}
impl<T: LexableTrivia + SimpleTriviaFactory> TriviaFactory for SimpleTriviaFactoryImpl<T> {
    type Trivia = T;
    fn make(&mut self) -> Self::Trivia {
        T::make()
    }
}
