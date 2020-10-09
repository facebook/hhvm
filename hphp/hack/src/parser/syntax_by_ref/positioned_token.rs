// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    compact_trivia::{CompactTrivia, TriviaKinds},
    lexable_token::{LexablePositionedToken, LexableToken},
    positioned_trivia::PositionedTrivium,
    source_text::SourceText,
    token_factory,
    token_kind::TokenKind,
    trivia_kind::TriviaKind,
};
use bumpalo::Bump;

#[derive(Debug, PartialEq)]
pub struct PositionedTokenImpl {
    pub kind: TokenKind,
    pub offset: usize, // Beginning of first trivia
    pub leading_width: usize,
    pub width: usize, // Width of actual token, not counting trivia
    pub trailing_width: usize,
    pub leading: TriviaKinds,
    pub trailing: TriviaKinds,
}

#[derive(Debug, Clone, Copy)]
pub struct PositionedToken<'a>(&'a PositionedTokenImpl);

#[allow(dead_code)]
impl PositionedTokenImpl {
    fn start_offset(&self) -> usize {
        self.offset + self.leading_width
    }

    fn end_offset(&self) -> usize {
        let w = self.width;
        let w = if w == 0 { 0 } else { w - 1 };
        self.start_offset() + w
    }

    fn leading_start_offset(&self) -> Option<usize> {
        Some(self.offset)
    }

    fn leading_text<'s>(&self, source_text: &SourceText<'s>) -> &'s [u8] {
        source_text.sub(self.leading_start_offset().unwrap(), self.leading_width)
    }

    fn trailing_text<'s>(&self, source_text: &SourceText<'s>) -> &'s [u8] {
        source_text.sub(self.end_offset() + 1, self.trailing_width)
    }

    fn clone(x: &Self) -> Self {
        Self {
            kind: x.kind,
            offset: x.offset,
            leading_width: x.leading_width,
            width: x.width,
            trailing_width: x.trailing_width,
            leading: x.leading,
            trailing: x.trailing,
        }
    }
}

#[allow(dead_code)]
impl<'a> PositionedToken<'a> {
    pub fn start_offset(&self) -> usize {
        self.0.start_offset()
    }

    pub fn end_offset(&self) -> usize {
        self.0.end_offset()
    }

    pub fn inner_ptr_eq(x: &Self, y: &Self) -> bool {
        std::ptr::eq(x.0, y.0)
    }
}

impl<'a> LexableToken for PositionedToken<'a> {
    type Trivia = CompactTrivia;

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
        CompactTrivia {
            kinds: self.0.leading,
            width: self.0.leading_width,
        }
    }

    fn clone_trailing(&self) -> Self::Trivia {
        CompactTrivia {
            kinds: self.0.trailing,
            width: self.0.trailing_width,
        }
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
            CompactTrivia {
                kinds: self.0.leading,
                width: self.0.leading_width,
            },
            self.width(),
            CompactTrivia {
                kinds: self.0.trailing,
                width: self.0.trailing_width,
            },
        )
    }
}

#[derive(Clone)]
pub struct TokenFactory<'a> {
    pub arena: &'a Bump,
}

impl<'a> token_factory::TokenFactory for TokenFactory<'a> {
    type Token = PositionedToken<'a>;

    fn make(
        &mut self,
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: CompactTrivia,
        trailing: CompactTrivia,
    ) -> Self::Token {
        PositionedToken(self.arena.alloc(PositionedTokenImpl {
            kind,
            offset,
            leading_width: leading.width,
            width,
            trailing_width: trailing.width,
            leading: leading.kinds,
            trailing: trailing.kinds,
        }))
    }

    fn with_leading(&mut self, token: Self::Token, leading: CompactTrivia) -> Self::Token {
        let mut new = PositionedTokenImpl::clone(token.0);
        new.leading = leading.kinds;
        new.leading_width = leading.width;
        PositionedToken(self.arena.alloc(new))
    }

    fn with_trailing(&mut self, token: Self::Token, trailing: CompactTrivia) -> Self::Token {
        let mut new = PositionedTokenImpl::clone(token.0);
        new.trailing = trailing.kinds;
        new.trailing_width = trailing.width;
        PositionedToken(self.arena.alloc(new))
    }

    fn with_kind(&mut self, token: Self::Token, kind: TokenKind) -> Self::Token {
        let mut new = PositionedTokenImpl::clone(token.0);
        new.kind = kind;
        PositionedToken(self.arena.alloc(new))
    }
}

impl<'a> LexablePositionedToken for PositionedToken<'a> {
    fn text<'b>(&self, source_text: &'b SourceText) -> &'b str {
        source_text.sub_as_str(self.0.start_offset(), self.0.width)
    }

    fn text_raw<'b>(&self, source_text: &'b SourceText) -> &'b [u8] {
        source_text.sub(self.0.start_offset(), self.0.width)
    }

    fn clone_value(&self) -> Self {
        self.clone()
    }

    fn trim_left(&mut self, _n: usize) -> Result<(), String> {
        /*
        let inner = RcOc::get_mut(self).ok_or("could not get mutable")?;
        inner.leading_width = inner.leading_width + n;
        inner.width = inner.width - n;
        Ok(())
        */
        unimplemented!()
    }

    fn trim_right(&mut self, _n: usize) -> Result<(), String> {
        /*
        let inner = RcOc::get_mut(self).ok_or("could not get mutable")?;
        inner.trailing_width = inner.trailing_width + n;
        inner.width = inner.width - n;
        Ok(())
        */
        unimplemented!()
    }

    fn concatenate(_s: &Self, _e: &Self) -> Result<Self, String> {
        /*
        let mut t = s.clone_value();
        let inner = RcOc::get_mut(&mut t).ok_or("could not get mutable")?;
        inner.width = e.end_offset() + 1 - s.start_offset();
        inner.trailing_width = e.trailing_width();
        inner.trailing = e.trailing.to_vec();
        Ok(t)
        */
        unimplemented!()
    }

    fn positioned_leading(&self) -> &[PositionedTrivium] {
        unimplemented!()
    }

    fn positioned_trailing(&self) -> &[PositionedTrivium] {
        unimplemented!()
    }
}
