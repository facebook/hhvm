// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexable_token::LexableToken;
use crate::syntax_kind::SyntaxKind;

use std::marker::Sized;

pub use crate::syntax_generated::*;
pub use crate::syntax_type::*;

pub trait SyntaxValueType<T>
where
    Self: Sized,
{
    fn from_syntax(syntax: &SyntaxVariant<T, Self>) -> Self;
    fn from_children(kind: SyntaxKind, offset: usize, syntax: &[Syntax<T, Self>]) -> Self;
    fn from_token(token: &T) -> Self;

    /// Returns a range [inclusive, exclusive] for the corresponding text if meaningful
    /// (note: each implementor will either always return Some(range) or always return None).
    fn text_range(&self) -> Option<(usize, usize)>; // corresponds to extract_text in OCaml impl.
}

#[derive(Debug, Clone)]
pub struct Syntax<T, V> {
    pub syntax: SyntaxVariant<T, V>,
    pub value: V,
}

pub trait SyntaxTypeBase {
    type Token: LexableToken;
    type Value: SyntaxValueType<Self::Token>;

    fn make_missing(offset: usize) -> Self;
    fn make_token(arg: Self::Token) -> Self;
    fn make_list(arg: Vec<Self>, offset: usize) -> Self
    where
        Self: Sized;

    /// Tests if the node, or any of its immediate children in case of a SyntaxList, matches a
    /// predicate in a short-circuiting fashion (i.e., it is more efficient than fold/for_each).
    fn any<F: FnMut(&Self) -> bool>(&self, f: F) -> bool;

    /// Shallow version of fold_over_children that folds only over children of SyntaxList &,
    /// or the node itself unless it is Ignored.
    fn fold_list<U, F: FnMut(U, &Self) -> U>(&self, init: U, f: F) -> U;

    fn as_syntax(&self) -> &Syntax<Self::Token, Self::Value>;
}

impl<T, V> SyntaxTypeBase for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    type Token = T;
    type Value = V;

    fn make_missing(offset: usize) -> Self {
        let children: Vec<Self> = vec![];
        let value = V::from_children(SyntaxKind::Missing, offset, &children);
        let syntax = SyntaxVariant::Missing;
        Self::make(syntax, value)
    }

    fn make_token(arg: T) -> Self {
        let value = V::from_token(&arg);
        let syntax = SyntaxVariant::Token(Box::new(arg));
        Self::make(syntax, value)
    }

    fn make_list(arg: Vec<Self>, offset: usize) -> Self {
        // An empty list is represented by Missing; everything else is a
        // SyntaxList, even if the list has only one item.
        if arg.is_empty() {
            Self::make_missing(offset)
        } else {
            let value = V::from_children(SyntaxKind::SyntaxList, offset, &arg);
            let syntax = SyntaxVariant::SyntaxList(arg);
            Self::make(syntax, value)
        }
    }

    fn any<F: FnMut(&Self) -> bool>(&self, mut f: F) -> bool {
        match &self.syntax {
            SyntaxVariant::SyntaxList(nodes) => nodes.iter().any(f),
            SyntaxVariant::Missing => false,
            _ => f(&self),
        }
    }

    fn fold_list<U, F: FnMut(U, &Self) -> U>(&self, init: U, mut f: F) -> U {
        use SyntaxVariant::*;
        match &self.syntax {
            SyntaxList(nodes) => nodes.iter().fold(init, |init, node| match &node.syntax {
                ListItem(box li) => f(init, &li.list_item),
                Missing => init,
                _ => f(init, &node),
            }),
            Missing => init,
            _ => f(init, &self),
        }
    }

    fn as_syntax(&self) -> &Syntax<T, V> {
        &self
    }
}

impl<T, V> Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    pub fn make(syntax: SyntaxVariant<T, V>, value: V) -> Self {
        Self { syntax, value }
    }

    pub fn children<'a>(&'a self) -> Vec<&'a Self> {
        let f = |node: &'a Self, mut acc: Vec<&'a Self>| {
            acc.push(node);
            acc
        };
        Self::fold_over_children(&f, vec![], &self.syntax)
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
                for node in self.children() {
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
                for node in self.children().iter().rev() {
                    if let Some(token) = node.trailing_token() {
                        return Some(token);
                    }
                }
                None
            }
        }
    }
}
