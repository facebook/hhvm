// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use crate::lexable_token::LexableToken;
use crate::positioned_token::PositionedToken;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;
use crate::syntax_trait::SyntaxTrait;
use crate::token_kind::TokenKind;

#[derive(Debug, Clone)]
pub struct Span {
    pub left: PositionedToken,
    pub right: PositionedToken,
}

#[derive(Debug, Clone)]
pub enum PositionedValue {
    /// value for a token node is token itself
    TokenValue(PositionedToken),
    /// value for a range denoted by pair of tokens
    TokenSpan(Box<Span>),
    Missing {
        offset: usize,
    },
}

impl PositionedValue {
    pub fn width(&self) -> usize {
        match self {
            PositionedValue::TokenValue(t) => t.width(),
            PositionedValue::TokenSpan(x) => (x.right.end_offset() - x.left.start_offset()) + 1,
            PositionedValue::Missing { .. } => 0,
        }
    }

    fn start_offset(&self) -> usize {
        use PositionedValue::*;
        match &self {
            TokenValue(t) => t
                .leading_start_offset()
                .expect("invariant violation for Positioned Syntax"),
            TokenSpan(x) => x
                .left
                .leading_start_offset()
                .expect("invariant violation for Positioned Syntax"),
            Missing { offset, .. } => *offset,
        }
    }

    fn leading_width(&self) -> usize {
        use PositionedValue::*;
        match self {
            TokenValue(t) => t.leading_width(),
            TokenSpan(x) => x.left.leading_width(),
            Missing { .. } => 0,
        }
    }

    fn trailing_width(&self) -> usize {
        use PositionedValue::*;
        match self {
            TokenValue(t) => t.trailing_width(),
            TokenSpan(x) => x.right.trailing_width(),
            Missing { .. } => 0,
        }
    }

    fn leading_token(&self) -> Option<&PositionedToken> {
        use PositionedValue::*;
        match self {
            TokenValue(l) => Some(l),
            TokenSpan(x) => Some(&x.left),
            _ => None,
        }
    }

    fn trailing_token(&self) -> Option<&PositionedToken> {
        use PositionedValue::*;
        match self {
            TokenValue(r) => Some(r),
            TokenSpan(x) => Some(&x.right),
            _ => None,
        }
    }

    fn value_from_outer_children(first: &Self, last: &Self) -> Self {
        use PositionedValue::*;
        match (first, last) {
            (TokenValue(_), TokenValue(_))
            | (TokenSpan(_), TokenValue(_))
            | (TokenValue(_), TokenSpan(_))
            | (TokenSpan(_), TokenSpan(_)) => {
                let l = first.leading_token().unwrap();
                let r = last.trailing_token().unwrap();
                if Arc::ptr_eq(l, r) {
                    TokenValue(Arc::clone(l))
                } else {
                    TokenSpan(Box::new(Span {
                        left: Arc::clone(l),
                        right: Arc::clone(r),
                    }))
                }
            }
            // can have two missing nodes if first and last child nodes of
            // the node are missing - this means that entire node is missing.
            // NOTE: offset must match otherwise it will mean that there is a real node
            // in between that should be picked instead
            (Missing { offset: o1 }, Missing { offset: o2 }) if o1 == o2 => first.clone(),
            _ => panic!(),
        }
    }

    fn from_<'a>(child_values: impl Iterator<Item = &'a Self>) -> Self {
        use PositionedValue::*;
        let mut first = None;
        let mut first_non_zero = None;
        let mut last_non_zero = None;
        let mut last = None;
        for value in child_values {
            match (first.is_some(), first_non_zero.is_some(), value) {
                (false, false, TokenValue { .. }) | (false, false, TokenSpan { .. }) => {
                    // first iteration and first node has some token representation -
                    // record it as first, first_non_zero, last and last_non_zero
                    first = Some(value);
                    first_non_zero = Some(value);
                    last_non_zero = Some(value);
                    last = Some(value);
                }
                (false, false, Missing { .. }) => {
                    // first iteration - first node is missing -
                    // record it as first and last
                    first = Some(value);
                    first_non_zero = None;
                    last_non_zero = None;
                    last = Some(value);
                }
                (true, false, TokenValue { .. }) | (true, false, TokenSpan { .. }) => {
                    // in progress, found first node that include tokens -
                    // record it as first_non_zero, last and last_non_zero
                    first_non_zero = Some(value);
                    last_non_zero = Some(value);
                    last = Some(value);
                }
                (true, true, TokenValue { .. }) | (true, true, TokenSpan { .. }) => {
                    // in progress found some node that includes tokens -
                    // record it as last_non_zero and last
                    last_non_zero = Some(value);
                    last = Some(value);
                }
                _ => {
                    // in progress, stepped on missing node -
                    // record it as last and move on
                    last = Some(value);
                }
            }
        }
        match (first, first_non_zero, last_non_zero, last) {
            (_, Some(first_non_zero), Some(last_non_zero), _) => {
                Self::value_from_outer_children(first_non_zero, last_non_zero)
            }
            (Some(first), None, None, Some(last)) => Self::value_from_outer_children(first, last),
            _ => panic!("how did we get a node with no children in value_from_syntax?"),
        }
    }
}

impl SyntaxValueWithKind for PositionedValue {
    fn is_missing(&self) -> bool {
        if let PositionedValue::Missing { .. } = self {
            true
        } else {
            false
        }
    }

    fn token_kind(&self) -> Option<TokenKind> {
        match self {
            PositionedValue::TokenValue(pt) => Some(pt.kind()),
            _ => None,
        }
    }
}

impl SyntaxValueType<PositionedToken> for PositionedValue {
    fn from_values<'a>(child_values: impl Iterator<Item = &'a Self>) -> Self {
        Self::from_(child_values)
    }

    fn from_children<'a>(
        _: SyntaxKind,
        offset: usize,
        nodes: impl Iterator<Item = &'a Self>,
    ) -> Self {
        // We need to determine the offset, leading, middle and trailing widths of
        // the node to be constructed based on its children.  If the children are
        // all of zero width -- including the case where there are no children at
        // all -- then we make a zero-width value at the given offset.
        // Otherwise, we can determine the associated value from the first and last
        // children that have width.
        let mut have_width = nodes.filter(|x| x.width() > 0).peekable();
        match have_width.peek() {
            None => PositionedValue::Missing { offset },
            Some(first) => Self::value_from_outer_children(first, have_width.last().unwrap()),
        }
    }

    fn from_token(token: PositionedToken) -> Self {
        if token.kind() == TokenKind::EndOfFile || token.full_width() == 0 {
            PositionedValue::Missing {
                offset: token.end_offset(),
            }
        } else {
            PositionedValue::TokenValue(token)
        }
    }
}

pub type PositionedSyntax = Syntax<PositionedToken, PositionedValue>;

impl SyntaxTrait for PositionedSyntax {
    fn offset(&self) -> Option<usize> {
        Some(self.start_offset())
    }

    fn width(&self) -> usize {
        self.value.width()
    }

    fn leading_width(&self) -> usize {
        self.value.leading_width()
    }

    fn trailing_width(&self) -> usize {
        self.value.trailing_width()
    }

    fn full_width(&self) -> usize {
        self.leading_width() + self.width() + self.trailing_width()
    }

    fn leading_start_offset(&self) -> usize {
        self.value.start_offset()
    }

    fn extract_text<'a>(&self, source_text: &'a SourceText<'_>) -> Option<&'a str> {
        Some(self.text(source_text))
    }
}

impl PositionedSyntax {
    /// Invariant: every token in the tree must have a valid offset&width,
    /// leading trivia offset&width, and trailing trivia offset&width (meaning,
    /// they point to an empty or valid non-empty slice of `source_text`), with
    /// one exception: fixed-width tokens (tokens where `TokenKind::fixed_width`
    /// returns `Some`) need not point to a valid offset&width (but their
    /// leading and trailing trivia offset&width should be empty or valid slices
    /// of `source_text`), since their text will be that returned by
    /// `TokenKind::to_string`. Undefined (but valid) slices of `source_text`
    /// will be used for invalid offsets/widths.
    pub fn text_from_edited_tree(&self, source_text: &SourceText<'_>) -> std::io::Result<Vec<u8>> {
        let mut text = vec![];
        self.write_text_from_edited_tree(source_text, &mut text)?;
        Ok(text)
    }

    /// Invariant: Same requirements as `text_from_edited_tree`.
    pub fn write_text_from_edited_tree(
        &self,
        source_text: &SourceText<'_>,
        w: &mut impl std::io::Write,
    ) -> std::io::Result<()> {
        self.try_iter_pre(|node| {
            if let Some(token) = node.get_token() {
                if token.kind().fixed_width().is_some() {
                    w.write_all(token.leading_text(source_text))?;
                    w.write_all(token.kind().to_string().as_bytes())?;
                    w.write_all(token.trailing_text(source_text))?;
                } else {
                    w.write_all(source_text.sub(token.offset(), token.full_width()))?;
                }
            }
            Ok(())
        })
    }
}
