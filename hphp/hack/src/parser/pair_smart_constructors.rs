// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_variables)]

use parser_core_types::{
    lexable_token::LexableToken,
    lexable_trivia::{LexableTrivia, LexableTrivium},
    token_factory::{TokenFactory, Trivia},
    token_kind::TokenKind,
    trivia_factory::TriviaFactory,
    trivia_kind::TriviaKind,
};
use smart_constructors::NodeType;

mod pair_smart_constructors_generated;

pub use pair_smart_constructors_generated::PairSmartConstructors;

pub struct Node<N0, N1>(pub N0, pub N1)
where
    N0: NodeType,
    N1: NodeType;

impl<N0, N1> NodeType for Node<N0, N1>
where
    N0: NodeType,
    N1: NodeType,
{
    type R = Node<N0, N1>;

    fn extract(self) -> Self::R {
        self
    }

    fn is_abstract(&self) -> bool {
        let result = self.0.is_abstract();
        debug_assert_eq!(result, self.1.is_abstract());
        result
    }
    fn is_name(&self) -> bool {
        let result = self.0.is_name();
        debug_assert_eq!(result, self.1.is_name());
        result
    }
    fn is_qualified_name(&self) -> bool {
        let result = self.0.is_qualified_name();
        debug_assert_eq!(result, self.1.is_qualified_name());
        result
    }
    fn is_prefix_unary_expression(&self) -> bool {
        let result = self.0.is_prefix_unary_expression();
        debug_assert_eq!(result, self.1.is_prefix_unary_expression());
        result
    }
    fn is_scope_resolution_expression(&self) -> bool {
        let result = self.0.is_scope_resolution_expression();
        debug_assert_eq!(result, self.1.is_scope_resolution_expression());
        result
    }
    fn is_missing(&self) -> bool {
        let result = self.0.is_missing();
        debug_assert_eq!(result, self.1.is_missing());
        result
    }
    fn is_variable_expression(&self) -> bool {
        let result = self.0.is_variable_expression();
        debug_assert_eq!(result, self.1.is_variable_expression());
        result
    }
    fn is_subscript_expression(&self) -> bool {
        let result = self.0.is_subscript_expression();
        debug_assert_eq!(result, self.1.is_subscript_expression());
        result
    }
    fn is_member_selection_expression(&self) -> bool {
        let result = self.0.is_member_selection_expression();
        debug_assert_eq!(result, self.1.is_member_selection_expression());
        result
    }
    fn is_object_creation_expression(&self) -> bool {
        let result = self.0.is_object_creation_expression();
        debug_assert_eq!(result, self.1.is_object_creation_expression());
        result
    }
    fn is_safe_member_selection_expression(&self) -> bool {
        let result = self.0.is_safe_member_selection_expression();
        debug_assert_eq!(result, self.1.is_safe_member_selection_expression());
        result
    }
    fn is_function_call_expression(&self) -> bool {
        let result = self.0.is_function_call_expression();
        debug_assert_eq!(result, self.1.is_function_call_expression());
        result
    }
    fn is_list_expression(&self) -> bool {
        let result = self.0.is_list_expression();
        debug_assert_eq!(result, self.1.is_list_expression());
        result
    }
}

#[derive(Clone, Debug)]
pub struct PairTokenFactory<TF0, TF1>(
    TF0,
    TF1,
    PairTriviaFactory<TF0::TriviaFactory, TF1::TriviaFactory>,
)
where
    TF0: TokenFactory,
    TF1: TokenFactory;

impl<TF0, TF1> PairTokenFactory<TF0, TF1>
where
    TF0: TokenFactory,
    TF1: TokenFactory,
{
    fn new(mut tf0: TF0, mut tf1: TF1) -> Self {
        let tvf0 = tf0.trivia_factory_mut().clone();
        let tvf1 = tf1.trivia_factory_mut().clone();
        let tvf = PairTriviaFactory(tvf0, tvf1);
        Self(tf0, tf1, tvf)
    }
}

impl<TF0, TF1> TokenFactory for PairTokenFactory<TF0, TF1>
where
    TF0: TokenFactory,
    TF1: TokenFactory,
{
    type Token = PairToken<TF0::Token, TF1::Token>;
    type TriviaFactory = PairTriviaFactory<TF0::TriviaFactory, TF1::TriviaFactory>;

    fn make(
        &mut self,
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: Trivia<Self>,
        trailing: Trivia<Self>,
    ) -> Self::Token {
        let t0 = self.0.make(kind, offset, width, leading.0, trailing.0);
        let t1 = self.1.make(kind, offset, width, leading.1, trailing.1);
        PairToken(t0, t1)
    }

    fn with_leading(&mut self, token: Self::Token, leading: Trivia<Self>) -> Self::Token {
        let t0 = self.0.with_leading(token.0, leading.0);
        let t1 = self.1.with_leading(token.1, leading.1);
        PairToken(t0, t1)
    }
    fn with_trailing(&mut self, token: Self::Token, trailing: Trivia<Self>) -> Self::Token {
        let t0 = self.0.with_trailing(token.0, trailing.0);
        let t1 = self.1.with_trailing(token.1, trailing.1);
        PairToken(t0, t1)
    }
    fn with_kind(&mut self, token: Self::Token, kind: TokenKind) -> Self::Token {
        let t0 = self.0.with_kind(token.0, kind);
        let t1 = self.1.with_kind(token.1, kind);
        PairToken(t0, t1)
    }

    fn trivia_factory_mut(&mut self) -> &mut Self::TriviaFactory {
        &mut self.2
    }
}

#[derive(Clone, Debug)]
pub struct PairTriviaFactory<T0, T1>(T0, T1)
where
    T0: TriviaFactory,
    T1: TriviaFactory;

impl<T0, T1> TriviaFactory for PairTriviaFactory<T0, T1>
where
    T0: TriviaFactory,
    T1: TriviaFactory,
{
    type Trivia = PairTrivia<T0::Trivia, T1::Trivia>;

    fn make(&mut self) -> Self::Trivia {
        PairTrivia(self.0.make(), self.1.make())
    }
}

#[derive(Clone, Debug)]
pub struct PairToken<T0, T1>(T0, T1)
where
    T0: LexableToken,
    T1: LexableToken;

impl<T0, T1> LexableToken for PairToken<T0, T1>
where
    T0: LexableToken,
    T1: LexableToken,
{
    type Trivia = PairTrivia<T0::Trivia, T1::Trivia>;

    fn kind(&self) -> TokenKind {
        let result = self.0.kind();
        debug_assert_eq!(result, self.1.kind());
        result
    }

    fn leading_start_offset(&self) -> Option<usize> {
        match (self.0.leading_start_offset(), self.1.leading_start_offset()) {
            (Some(offset0), Some(offset1)) => {
                debug_assert_eq!(offset0, offset1);
                Some(offset0)
            }
            (None, None) => None,
            // TODO: Is it right to return Some in these cases?
            (Some(offset), None) | (None, Some(offset)) => Some(offset),
        }
    }

    fn width(&self) -> usize {
        let result = self.0.width();
        debug_assert_eq!(result, self.1.width());
        result
    }
    fn leading_width(&self) -> usize {
        let result = self.0.leading_width();
        debug_assert_eq!(result, self.1.leading_width());
        result
    }
    fn trailing_width(&self) -> usize {
        let result = self.0.trailing_width();
        debug_assert_eq!(result, self.1.trailing_width());
        result
    }
    fn full_width(&self) -> usize {
        let result = self.0.full_width();
        debug_assert_eq!(result, self.1.full_width());
        result
    }

    fn clone_leading(&self) -> Self::Trivia {
        PairTrivia(self.0.clone_leading(), self.1.clone_leading())
    }
    fn clone_trailing(&self) -> Self::Trivia {
        PairTrivia(self.0.clone_trailing(), self.1.clone_trailing())
    }

    fn leading_is_empty(&self) -> bool {
        let result = self.0.leading_is_empty();
        debug_assert_eq!(result, self.1.leading_is_empty());
        result
    }
    fn trailing_is_empty(&self) -> bool {
        let result = self.0.trailing_is_empty();
        debug_assert_eq!(result, self.1.trailing_is_empty());
        result
    }

    fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
        let result = self.0.has_leading_trivia_kind(kind);
        debug_assert_eq!(result, self.1.has_leading_trivia_kind(kind));
        result
    }
    fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
        let result = self.0.has_trailing_trivia_kind(kind);
        debug_assert_eq!(result, self.1.has_trailing_trivia_kind(kind));
        result
    }

    fn into_trivia_and_width(self) -> (Self::Trivia, usize, Self::Trivia) {
        let (leading0, width0, trailing0) = self.0.into_trivia_and_width();
        let (leading1, width1, trailing1) = self.1.into_trivia_and_width();
        let leading = PairTrivia(leading0, leading1);
        let trailing = PairTrivia(trailing0, trailing1);
        debug_assert_eq!(width0, width1);
        (leading, width0, trailing)
    }
}

#[derive(Clone, Debug)]
pub struct PairTrivia<T0, T1>(T0, T1)
where
    T0: LexableTrivia,
    T1: LexableTrivia;

impl<T0, T1> LexableTrivia for PairTrivia<T0, T1>
where
    T0: LexableTrivia,
    T1: LexableTrivia,
{
    type Trivium = PairTrivium<T0::Trivium, T1::Trivium>;

    fn is_empty(&self) -> bool {
        let result = self.0.is_empty();
        debug_assert_eq!(result, self.1.is_empty());
        result
    }
    fn has_kind(&self, kind: TriviaKind) -> bool {
        let result = self.0.has_kind(kind);
        debug_assert_eq!(result, self.1.has_kind(kind));
        result
    }
    fn push(&mut self, trivium: Self::Trivium) {
        self.0.push(trivium.0);
        self.1.push(trivium.1);
    }
    fn extend(&mut self, other: Self) {
        self.0.extend(other.0);
        self.1.extend(other.1);
    }

    #[inline]
    fn make_whitespace(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_whitespace(offset, width),
            T1::make_whitespace(offset, width),
        )
    }
    #[inline]
    fn make_eol(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(T0::make_eol(offset, width), T1::make_eol(offset, width))
    }
    #[inline]
    fn make_single_line_comment(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_single_line_comment(offset, width),
            T1::make_single_line_comment(offset, width),
        )
    }
    #[inline]
    fn make_fallthrough(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_fallthrough(offset, width),
            T1::make_fallthrough(offset, width),
        )
    }
    #[inline]
    fn make_fix_me(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_fix_me(offset, width),
            T1::make_fix_me(offset, width),
        )
    }
    #[inline]
    fn make_ignore_error(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_ignore_error(offset, width),
            T1::make_ignore_error(offset, width),
        )
    }
    #[inline]
    fn make_extra_token_error(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_extra_token_error(offset, width),
            T1::make_extra_token_error(offset, width),
        )
    }
    #[inline]
    fn make_delimited_comment(offset: usize, width: usize) -> Self::Trivium {
        PairTrivium(
            T0::make_delimited_comment(offset, width),
            T1::make_delimited_comment(offset, width),
        )
    }
}

#[derive(Clone, Debug)]
pub struct PairTrivium<T0, T1>(T0, T1)
where
    T0: LexableTrivium,
    T1: LexableTrivium;

impl<T0, T1> LexableTrivium for PairTrivium<T0, T1>
where
    T0: LexableTrivium,
    T1: LexableTrivium,
{
    fn make_whitespace(offset: usize, width: usize) -> Self {
        Self(
            T0::make_whitespace(offset, width),
            T1::make_whitespace(offset, width),
        )
    }
    fn make_eol(offset: usize, width: usize) -> Self {
        Self(T0::make_eol(offset, width), T1::make_eol(offset, width))
    }
    fn make_single_line_comment(offset: usize, width: usize) -> Self {
        Self(
            T0::make_single_line_comment(offset, width),
            T1::make_single_line_comment(offset, width),
        )
    }
    fn make_fallthrough(offset: usize, width: usize) -> Self {
        Self(
            T0::make_fallthrough(offset, width),
            T1::make_fallthrough(offset, width),
        )
    }
    fn make_fix_me(offset: usize, width: usize) -> Self {
        Self(
            T0::make_fix_me(offset, width),
            T1::make_fix_me(offset, width),
        )
    }
    fn make_ignore_error(offset: usize, width: usize) -> Self {
        Self(
            T0::make_ignore_error(offset, width),
            T1::make_ignore_error(offset, width),
        )
    }
    fn make_extra_token_error(offset: usize, width: usize) -> Self {
        Self(
            T0::make_extra_token_error(offset, width),
            T1::make_extra_token_error(offset, width),
        )
    }
    fn make_delimited_comment(offset: usize, width: usize) -> Self {
        Self(
            T0::make_delimited_comment(offset, width),
            T1::make_delimited_comment(offset, width),
        )
    }
    fn kind(&self) -> TriviaKind {
        let result = self.0.kind();
        debug_assert_eq!(result, self.1.kind());
        result
    }
    fn width(&self) -> usize {
        let result = self.0.width();
        debug_assert_eq!(result, self.1.width());
        result
    }
}

impl<T0, T1> PartialEq for PairTrivium<T0, T1>
where
    T0: LexableTrivium,
    T1: LexableTrivium,
{
    fn eq(&self, other: &Self) -> bool {
        let result = self.0 == other.0;
        debug_assert_eq!(result, self.1 == other.1);
        result
    }
}
