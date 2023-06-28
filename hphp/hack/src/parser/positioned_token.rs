// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use crate::lexable_token::LexablePositionedToken;
use crate::lexable_token::LexableToken;
use crate::lexable_trivia::LexableTrivia;
use crate::positioned_trivia::PositionedTrivia;
use crate::positioned_trivia::PositionedTrivium;
use crate::source_text::SourceText;
use crate::token_factory::SimpleTokenFactory;
use crate::token_kind::TokenKind;
use crate::trivia_factory::SimpleTriviaFactory;
use crate::trivia_kind::TriviaKind;

#[derive(Debug, Clone, PartialEq)]
pub struct PositionedTokenImpl {
    pub kind: TokenKind,
    pub offset: usize, // Beginning of first trivia
    pub leading_width: usize,
    pub width: usize, // Width of actual token, not counting trivia
    pub trailing_width: usize,
    // TODO (kasper): implement LazyTrivia
    pub leading: PositionedTrivia,
    pub trailing: PositionedTrivia,
}

// Positioned tokens, when used as part of positioned syntax are shared - same leaf token can be
// embedded in multiple internal nodes (node can store the leftmost and rightmost token of its
// subtree to describe span covered by it - see PositionedSyntaxValue for details).
// We don't want to have to clone and store multiple copies of the token, so we define it as ref
// counted pointer to the actual shared struct
pub type PositionedToken = Arc<PositionedTokenImpl>;

pub fn new(
    kind: TokenKind,
    offset: usize,
    width: usize,
    leading: PositionedTrivia,
    trailing: PositionedTrivia,
) -> PositionedToken {
    let leading_width = leading.iter().map(|x| x.width).sum();
    let trailing_width = trailing.iter().map(|x| x.width).sum();
    Arc::new(PositionedTokenImpl {
        kind,
        offset,
        leading_width,
        width,
        trailing_width,
        leading,
        trailing,
    })
}

impl SimpleTokenFactory for PositionedToken {
    fn make(
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: PositionedTrivia,
        trailing: PositionedTrivia,
    ) -> Self {
        new(kind, offset, width, leading, trailing)
    }

    // Tricky: the with_ functions that modify tokens can be very cheap (when ref count is 1), or
    // possibly expensive (when make_mut has to perform a clone of underlying token that is shared).
    // Fortunately, they are used only in lexer/parser BEFORE the tokens are embedded in syntax, so
    // before any sharing occurs
    fn with_leading(mut self, leading: PositionedTrivia) -> Self {
        let token = Arc::make_mut(&mut self);
        let token_start_offset = token.offset + token.leading_width;
        let leading_width = leading.iter().map(|x| x.width).sum();
        token.offset = token_start_offset - leading_width;
        token.leading_width = leading_width;
        token.leading = leading;
        self
    }

    fn with_trailing(mut self, trailing: PositionedTrivia) -> Self {
        let token = Arc::make_mut(&mut self);
        let trailing_width = trailing.iter().map(|x| x.width).sum();
        token.trailing_width = trailing_width;
        token.trailing = trailing;
        self
    }

    fn with_kind(mut self, kind: TokenKind) -> Self {
        let token = Arc::make_mut(&mut self);
        token.kind = kind;
        self
    }
}

impl LexableToken for PositionedToken {
    type Trivia = PositionedTrivia;

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

    fn clone_leading(&self) -> PositionedTrivia {
        self.leading.clone()
    }

    fn clone_trailing(&self) -> PositionedTrivia {
        self.trailing.clone()
    }

    fn leading_is_empty(&self) -> bool {
        self.leading.is_empty()
    }

    fn trailing_is_empty(&self) -> bool {
        self.trailing.is_empty()
    }

    fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.leading.has_kind(kind)
    }

    fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.trailing.has_kind(kind)
    }

    fn into_trivia_and_width(self) -> (Self::Trivia, usize, Self::Trivia) {
        match Arc::try_unwrap(self) {
            Ok(t) => (t.leading, t.width, t.trailing),
            Err(t_ptr) => (t_ptr.leading.clone(), t_ptr.width, t_ptr.trailing.clone()),
        }
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

impl LexablePositionedToken for PositionedToken {
    fn text<'b>(&self, source_text: &'b SourceText<'_>) -> &'b str {
        source_text.sub_as_str(self.start_offset(), self.width())
    }

    fn text_raw<'b>(&self, source_text: &'b SourceText<'_>) -> &'b [u8] {
        source_text.sub(self.start_offset(), self.width())
    }

    fn clone_value(&self) -> Self {
        let inner = self.as_ref().clone();
        Arc::new(inner)
    }

    fn positioned_leading(&self) -> &[PositionedTrivium] {
        &self.leading
    }

    fn positioned_trailing(&self) -> &[PositionedTrivium] {
        &self.trailing
    }
}

impl SimpleTriviaFactory for PositionedTrivia {
    fn make() -> Self {
        Self::new()
    }
}
