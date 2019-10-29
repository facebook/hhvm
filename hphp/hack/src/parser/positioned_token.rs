// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::rc::RcOc;

use crate::{
    lexable_token::{LexablePositionedToken, LexableToken},
    positioned_trivia::PositionedTrivia,
    source_text::SourceText,
    token_kind::TokenKind,
    trivia_kind::TriviaKind,
};

#[derive(Debug, Clone, PartialEq)]
pub struct PositionedTokenImpl {
    pub kind: TokenKind,
    pub offset: usize, // Beginning of first trivia
    pub leading_width: usize,
    pub width: usize, // Width of actual token, not counting trivia
    pub trailing_width: usize,
    // TODO (kasper): implement LazyTrivia
    pub leading: Vec<PositionedTrivia>,
    pub trailing: Vec<PositionedTrivia>,
}

// Positioned tokens, when used as part of positioned syntax are shared - same leaf token can be
// embedded in multiple internal nodes (node can store the leftmost and rightmost token of its
// subtree to describe span covered by it - see PositionedSyntaxValue for details).
// We don't want to have to clone and store multiple copies of the token, so we define it as ref
// counted pointer to the actual shared struct
pub type PositionedToken = RcOc<PositionedTokenImpl>;

impl<'a> LexableToken<'a> for PositionedToken {
    type Trivia = PositionedTrivia;

    fn make(
        kind: TokenKind,
        _source: &SourceText,
        offset: usize,
        width: usize,
        leading: Vec<Self::Trivia>,
        trailing: Vec<Self::Trivia>,
    ) -> Self {
        let leading_width = leading.iter().map(|x| x.width).sum();
        let trailing_width = trailing.iter().map(|x| x.width).sum();

        RcOc::new(PositionedTokenImpl {
            kind,
            offset,
            leading_width,
            width,
            trailing_width,
            leading,
            trailing,
        })
    }

    fn kind(&self) -> TokenKind {
        self.kind
    }

    fn leading_start_offset(&self) -> Option<usize> {
        self.as_ref().leading_start_offset()
    }

    fn width(&self) -> usize {
        self.width
    }

    fn leading_width(&self) -> usize {
        self.leading_width
    }

    fn trailing_width(&self) -> usize {
        self.trailing_width
    }

    fn full_width(&self) -> usize {
        self.leading_width() + self.width() + self.trailing_width()
    }

    fn leading(&self) -> &[Self::Trivia] {
        &self.leading
    }
    fn trailing(&self) -> &[Self::Trivia] {
        &self.trailing
    }

    // Tricky: the with_ functions that modify tokens can be very cheap (when ref count is 1), or
    // possibly expensive (when make_mut has to perform a clone of underlying token that is shared).
    // Fortunately, they are used only in lexer/parser BEFORE the tokens are embedded in syntax, so
    // before any sharing occurs
    fn with_leading(mut self, leading: Vec<Self::Trivia>) -> Self {
        let mut token = RcOc::make_mut(&mut self);
        token.leading = leading;
        self
    }

    fn with_trailing(mut self, trailing: Vec<Self::Trivia>) -> Self {
        let mut token = RcOc::make_mut(&mut self);
        token.trailing = trailing;
        self
    }

    fn with_kind(mut self, kind: TokenKind) -> Self {
        let mut token = RcOc::make_mut(&mut self);
        token.kind = kind;
        self
    }

    fn has_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.leading().iter().any(|t| t.kind == kind)
            || self.trailing().iter().any(|t| t.kind == kind)
    }
}

impl PositionedTokenImpl {
    pub fn offset(&self) -> usize {
        self.offset
    }

    pub fn start_offset(&self) -> usize {
        self.offset() + self.leading_width
    }

    pub fn end_offset(&self) -> usize {
        let w = self.width;
        let w = if w == 0 { 0 } else { w - 1 };
        self.start_offset() + w
    }

    fn leading_start_offset(&self) -> Option<usize> {
        Some(self.offset)
    }

    pub fn leading_text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.leading_start_offset().unwrap(), self.leading_width)
    }

    pub fn trailing_text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.end_offset() + 1, self.trailing_width)
    }
}

impl<'a> LexablePositionedToken<'a> for PositionedToken {
    fn text<'b>(&self, source_text: &'b SourceText) -> &'b str {
        source_text.sub_as_str(self.start_offset(), self.width())
    }

    fn text_raw<'b>(&self, source_text: &'b SourceText) -> &'b [u8] {
        source_text.sub(self.start_offset(), self.width())
    }

    fn clone_value(&self) -> Self {
        let inner = self.as_ref().clone();
        RcOc::new(inner)
    }

    fn trim_left(&mut self, n: usize) -> Result<(), String> {
        let inner = RcOc::get_mut(self).ok_or("could not get mutable")?;
        inner.leading_width = inner.leading_width + n;
        inner.width = inner.width - n;
        Ok(())
    }

    fn trim_right(&mut self, n: usize) -> Result<(), String> {
        let inner = RcOc::get_mut(self).ok_or("could not get mutable")?;
        inner.trailing_width = inner.trailing_width + n;
        inner.width = inner.width - n;
        Ok(())
    }

    fn concatenate(s: &Self, e: &Self) -> Result<Self, String> {
        let mut t = s.clone_value();
        let inner = RcOc::get_mut(&mut t).ok_or("could not get mutable")?;
        inner.width = e.end_offset() + 1 - s.start_offset();
        inner.trailing_width = e.trailing_width();
        inner.trailing = e.trailing().to_vec();
        Ok(t)
    }
}
