// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexable_token::LexableToken;
use crate::syntax_kind::SyntaxKind;
use crate::token_kind::TokenKind;

use std::marker::Sized;

pub use crate::syntax_generated::*;
pub use crate::syntax_type::*;

pub trait SyntaxValueType<T>
where
    Self: Sized,
{
    fn from_syntax(syntax: &SyntaxVariant<T, Self>) -> Self;
    fn from_values(ndoes: &[&Self]) -> Self;
    fn from_children(kind: SyntaxKind, offset: usize, nodes: &[&Self]) -> Self;
    fn from_token(token: &T) -> Self;

    /// Returns a range [inclusive, exclusive] for the corresponding text if meaningful
    /// (note: each implementor will either always return Some(range) or always return None).
    fn text_range(&self) -> Option<(usize, usize)>; // corresponds to extract_text in OCaml impl.
}

pub trait SyntaxValueWithKind {
    fn is_missing(&self) -> bool;
    fn token_kind(&self) -> Option<TokenKind>;
}

#[derive(Debug, Clone)]
pub struct Syntax<T, V> {
    pub syntax: SyntaxVariant<T, V>,
    pub value: V,
}

pub trait SyntaxTypeBase<'a, C> {
    type Token: LexableToken<'a>;
    type Value: SyntaxValueType<Self::Token>;

    fn make_missing(ctx: &C, offset: usize) -> Self;
    fn make_token(ctx: &C, arg: Self::Token) -> Self;
    fn make_list(ctx: &C, arg: Vec<Self>, offset: usize) -> Self
    where
        Self: Sized;

    fn value(&self) -> &Self::Value;
}

impl<'a, T, V, C> SyntaxTypeBase<'a, C> for Syntax<T, V>
where
    T: LexableToken<'a>,
    V: SyntaxValueType<T>,
{
    type Token = T;
    type Value = V;

    fn make_missing(_: &C, offset: usize) -> Self {
        let value = V::from_children(SyntaxKind::Missing, offset, &[]);
        let syntax = SyntaxVariant::Missing;
        Self::make(syntax, value)
    }

    fn make_token(_: &C, arg: T) -> Self {
        let value = V::from_token(&arg);
        let syntax = SyntaxVariant::Token(Box::new(arg));
        Self::make(syntax, value)
    }

    fn make_list(ctx: &C, arg: Vec<Self>, offset: usize) -> Self {
        // An empty list is represented by Missing; everything else is a
        // SyntaxList, even if the list has only one item.
        if arg.is_empty() {
            Self::make_missing(ctx, offset)
        } else {
            // todo: pass iter directly
            let nodes = &arg.iter().map(|x| &x.value).collect::<Vec<_>>();
            let value = V::from_children(SyntaxKind::SyntaxList, offset, nodes);
            let syntax = SyntaxVariant::SyntaxList(arg);
            Self::make(syntax, value)
        }
    }

    fn value(&self) -> &Self::Value {
        &self.value
    }
}

impl<'src, T, V> Syntax<T, V>
where
    T: LexableToken<'src>,
    V: SyntaxValueType<T>,
{
    pub fn make(syntax: SyntaxVariant<T, V>, value: V) -> Self {
        Self { syntax, value }
    }

    pub fn is_public(&self) -> bool {
        if let SyntaxVariant::Token(t) = &self.syntax {
            t.kind() == TokenKind::Public
        } else {
            false
        }
    }

    pub fn is_private(&self) -> bool {
        if let SyntaxVariant::Token(t) = &self.syntax {
            t.kind() == TokenKind::Private
        } else {
            false
        }
    }

    pub fn is_protected(&self) -> bool {
        if let SyntaxVariant::Token(t) = &self.syntax {
            t.kind() == TokenKind::Protected
        } else {
            false
        }
    }

    pub fn is_abstract(&self) -> bool {
        if let SyntaxVariant::Token(t) = &self.syntax {
            t.kind() == TokenKind::Abstract
        } else {
            false
        }
    }

    pub fn is_static(&self) -> bool {
        if let SyntaxVariant::Token(t) = &self.syntax {
            t.kind() == TokenKind::Static
        } else {
            false
        }
    }

    pub fn is_missing(&self) -> bool {
        if let SyntaxVariant::Missing = &self.syntax {
            true
        } else {
            false
        }
    }

    pub fn drain_children(&mut self) -> Vec<Self> {
        let f = |node: Self, mut acc: Vec<Self>| {
            acc.push(node);
            acc
        };
        let syntax = std::mem::replace(&mut self.syntax, SyntaxVariant::Missing);
        Self::fold_over_children_owned(&f, vec![], syntax)
    }

    pub fn replace_children(&mut self, kind: SyntaxKind, children: Vec<Self>) {
        std::mem::replace(&mut self.syntax, Syntax::from_children(kind, children));
    }

    fn get_token(&self) -> Option<&T> {
        match &self.syntax {
            SyntaxVariant::Token(t) => Some(&t),
            _ => None,
        }
    }

    pub fn leading_token(&self) -> Option<&T> {
        match self.get_token() {
            Some(token) => Some(token),
            None => {
                for node in self.iter_children() {
                    if let Some(token) = node.leading_token() {
                        return Some(token);
                    }
                }
                None
            }
        }
    }

    pub fn trailing_token(&self) -> Option<&T> {
        match self.get_token() {
            Some(token) => Some(token),
            None => {
                for node in self.iter_children().rev() {
                    if let Some(token) = node.trailing_token() {
                        return Some(token);
                    }
                }
                None
            }
        }
    }

    pub fn iter_children<'a>(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        self.syntax.iter_children()
    }
}

pub struct SyntaxChildrenIterator<'a, T, V> {
    pub syntax: &'a SyntaxVariant<T, V>,
    pub index: usize,
    pub index_back: usize,
}

impl<'src, T, V> SyntaxVariant<T, V> {
    pub fn iter_children<'a>(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        SyntaxChildrenIterator {
            syntax: &self,
            index: 0,
            index_back: 0,
        }
    }
}

impl<'a, T, V> Iterator for SyntaxChildrenIterator<'a, T, V> {
    type Item = &'a Syntax<T, V>;
    fn next(&mut self) -> Option<Self::Item> {
        self.next_impl(true)
    }
}

impl<'a, T, V> DoubleEndedIterator for SyntaxChildrenIterator<'a, T, V> {
    fn next_back(&mut self) -> Option<Self::Item> {
        self.next_impl(false)
    }
}
