// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::rc::RcOc;

use crate::lexable_token::{LexablePositionedToken, LexableToken};
use crate::positioned_trivia::PositionedTrivia;
use crate::source_text::SourceText;
use crate::token_kind::TokenKind;

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
// counted pointer (wrapped in newtype) to the actual shared struct
#[derive(Debug, Clone)]
pub struct PositionedToken(pub RcOc<PositionedTokenImpl>);

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

        Self(RcOc::new(PositionedTokenImpl {
            kind,
            offset,
            leading_width,
            width,
            trailing_width,
            leading,
            trailing,
        }))
    }

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
        self.leading_width() + self.width() + self.trailing_width()
    }

    fn leading(&self) -> &[Self::Trivia] {
        &self.0.leading
    }
    fn trailing(&self) -> &[Self::Trivia] {
        &self.0.trailing
    }

    // Tricky: the with_ functions that modify tokens can be very cheap (when ref count is 1), or
    // possibly expensive (when make_mut has to perform a clone of underlying token that is shared).
    // Fortunately, they are used only in lexer/parser BEFORE the tokens are embedded in syntax, so
    // before any sharing occurs
    fn with_leading(self, leading: Vec<Self::Trivia>) -> Self {
        let mut token = RcOc::clone(&self.0);
        let mut token_impl = RcOc::make_mut(&mut token);
        token_impl.leading = leading;
        Self(token)
    }

    fn with_trailing(self, trailing: Vec<Self::Trivia>) -> Self {
        let mut token = RcOc::clone(&self.0);
        let mut token_impl = RcOc::make_mut(&mut token);
        token_impl.trailing = trailing;
        Self(token)
    }

    fn with_kind(self, kind: TokenKind) -> Self {
        let mut token = RcOc::clone(&self.0);
        let mut token_impl = RcOc::make_mut(&mut token);
        token_impl.kind = kind;
        Self(token)
    }
}

impl PositionedToken {
    pub fn offset(&self) -> usize {
        self.0.offset
    }

    pub fn start_offset(&self) -> usize {
        self.offset() + self.leading_width()
    }

    pub fn end_offset(&self) -> usize {
        let w = self.width();
        let w = if w == 0 { 0 } else { w - 1 };
        self.start_offset() + w
    }

    pub fn leading_text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.leading_start_offset().unwrap(), self.leading_width())
    }

    pub fn trailing_text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.end_offset() + 1, self.trailing_width())
    }

    // Similar convention to calling Rc::clone(x) instead of x.clone() to make it more explicit
    // that we are only cloning the reference, not the value
    pub fn clone_rc(x: &Self) -> Self {
        Self(RcOc::clone(&x.0))
    }
}

impl PartialEq for PositionedToken {
    fn eq(&self, other: &Self) -> bool {
        RcOc::ptr_eq(&(*self).0, &(*other).0)
    }
}
impl Eq for PositionedToken {}

impl<'a> LexablePositionedToken<'a> for PositionedToken {
    fn text<'b>(&self, source_text: &'b SourceText) -> &'b str {
        source_text.sub_as_str(self.start_offset(), self.width())
    }

    fn text_raw<'b>(&self, source_text: &'b SourceText) -> &'b [u8] {
        source_text.sub(self.start_offset(), self.width())
    }

    // TODO: PositionedToken auto derives Clone trait, and it also
    // has clone_rc(..), two are the same.
    fn clone_value(&self) -> Self {
        let inner = self.0.as_ref().clone();
        Self(RcOc::new(inner))
    }

    fn trim_left(&mut self, n: usize) -> Result<(), String> {
        let inner = RcOc::get_mut(&mut self.0).ok_or("could not get mutable")?;
        inner.leading_width = inner.leading_width + n;
        inner.width = inner.width - n;
        Ok(())
    }

    fn trim_right(&mut self, n: usize) -> Result<(), String> {
        let inner = RcOc::get_mut(&mut self.0).ok_or("could not get mutable")?;
        inner.trailing_width = inner.trailing_width + n;
        inner.width = inner.width - n;
        Ok(())
    }

    fn concatenate(s: &Self, e: &Self) -> Result<Self, String> {
        let mut t = s.clone_value();
        let inner = RcOc::get_mut(&mut t.0).ok_or("could not get mutable")?;
        inner.width = e.end_offset() + 1 - s.start_offset();
        inner.trailing_width = e.trailing_width();
        inner.trailing = e.trailing().to_vec();
        Ok(t)
    }
}
