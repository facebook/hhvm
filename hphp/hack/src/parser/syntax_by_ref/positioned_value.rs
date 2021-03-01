// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::positioned_trivia::PositionedTrivia;

pub type PositionedValue<'a> = internal::PositionedValue<'a, usize>;
pub type PositionedValueFullTrivia<'a> = internal::PositionedValue<'a, PositionedTrivia<'a>>;

mod internal {
    use crate::{
        lexable_token::LexableToken,
        syntax::{SyntaxValueType, SyntaxValueWithKind},
        syntax_by_ref::positioned_token::internal::{PositionedToken, TriviaRep},
        syntax_kind::SyntaxKind,
        token_kind::TokenKind,
    };
    use std::matches;

    #[derive(Debug, Clone)]
    pub enum PositionedValue<'a, Trivia> {
        /// value for a token node is token itself
        TokenValue(PositionedToken<'a, Trivia>),
        /// value for a range denoted by pair of tokens
        TokenSpan(PositionedToken<'a, Trivia>, PositionedToken<'a, Trivia>),
        Missing {
            offset: usize,
        },
    }

    impl<'a, Trivia> PositionedValue<'a, Trivia>
    where
        Trivia: TriviaRep + Clone,
    {
        pub fn width(&self) -> usize {
            match self {
                PositionedValue::TokenValue(t) => t.width(),
                PositionedValue::TokenSpan(left, right) => {
                    (right.end_offset() - left.start_offset()) + 1
                }
                PositionedValue::Missing { .. } => 0,
            }
        }

        pub fn start_offset(&self) -> usize {
            use PositionedValue::*;
            match &self {
                TokenValue(t) => t
                    .leading_start_offset()
                    .expect("invariant violation for Positioned Syntax"),
                TokenSpan(left, _) => left
                    .leading_start_offset()
                    .expect("invariant violation for Positioned Syntax"),
                Missing { offset, .. } => *offset,
            }
        }

        pub fn leading_width(&self) -> usize {
            use PositionedValue::*;
            match self {
                TokenValue(t) => t.leading_width(),
                TokenSpan(left, _) => left.leading_width(),
                Missing { .. } => 0,
            }
        }

        pub fn trailing_width(&self) -> usize {
            use PositionedValue::*;
            match self {
                TokenValue(t) => t.trailing_width(),
                TokenSpan(_, right) => right.trailing_width(),
                Missing { .. } => 0,
            }
        }

        pub fn leading_token(&self) -> Option<PositionedToken<'a, Trivia>> {
            use PositionedValue::*;
            match self {
                TokenValue(l) => Some(l.clone()),
                TokenSpan(left, _) => Some(left.clone()),
                _ => None,
            }
        }

        pub fn trailing_token(&self) -> Option<PositionedToken<'a, Trivia>> {
            use PositionedValue::*;
            match self {
                TokenValue(r) => Some(r.clone()),
                TokenSpan(_, right) => Some(right.clone()),
                _ => None,
            }
        }

        fn value_from_outer_children(first: &Self, last: &Self) -> Self {
            use PositionedValue::*;
            match (first, last) {
                (TokenValue(_), TokenValue(_))
                | (TokenSpan(_, _), TokenValue(_))
                | (TokenValue(_), TokenSpan(_, _))
                | (TokenSpan(_, _), TokenSpan(_, _)) => {
                    let l = first.leading_token().unwrap();
                    let r = last.trailing_token().unwrap();
                    if PositionedToken::inner_ptr_eq(&l, &r) {
                        TokenValue(l)
                    } else {
                        TokenSpan(l, r)
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
    }

    impl<'a, Trivia: 'a> SyntaxValueType<PositionedToken<'a, Trivia>> for PositionedValue<'a, Trivia>
    where
        Trivia: TriviaRep + Clone,
    {
        fn from_values<'b>(child_values: impl Iterator<Item = &'b Self>) -> Self
        where
            'a: 'b,
        {
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
                (Some(first), None, None, Some(last)) => {
                    Self::value_from_outer_children(first, last)
                }
                _ => panic!("how did we get a node with no children in value_from_syntax?"),
            }
        }

        fn from_token(token: PositionedToken<'a, Trivia>) -> Self {
            if token.kind() == TokenKind::EndOfFile || token.full_width() == 0 {
                PositionedValue::Missing {
                    offset: token.end_offset(),
                }
            } else {
                PositionedValue::TokenValue(token)
            }
        }

        fn from_children<'b>(
            _: SyntaxKind,
            offset: usize,
            nodes: impl Iterator<Item = &'b Self>,
        ) -> Self
        where
            'a: 'b,
        {
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
    }

    impl<'a, Trivia: std::fmt::Debug> SyntaxValueWithKind for PositionedValue<'a, Trivia>
    where
        Trivia: TriviaRep + Clone,
    {
        fn is_missing(&self) -> bool {
            matches!(self, PositionedValue::Missing { .. })
        }

        fn token_kind(&self) -> Option<TokenKind> {
            match self {
                PositionedValue::TokenValue(pt) => Some(pt.kind()),
                _ => None,
            }
        }
    }
}
