/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
use crate::lexable_trivia::LexableTrivia;
use crate::token_kind::TokenKind;

pub trait LexableToken: Clone {
    type Trivia: LexableTrivia;
    fn make(
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: Vec<Self::Trivia>,
        trailing: Vec<Self::Trivia>,
    ) -> Self;
    fn kind(&self) -> TokenKind;
    fn leading_start_offset(&self) -> usize;

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
