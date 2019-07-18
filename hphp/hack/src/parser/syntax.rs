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
    fn from_values(ndoes: &[&Self]) -> Self;
    fn from_children(kind: SyntaxKind, offset: usize, nodes: &[&Self]) -> Self;
    fn from_token(token: &T) -> Self;

    /// Returns a range [inclusive, exclusive] for the corresponding text if meaningful
    /// (note: each implementor will either always return Some(range) or always return None).
    fn text_range(&self) -> Option<(usize, usize)>; // corresponds to extract_text in OCaml impl.
}

pub trait SyntaxValueWithKind {
    fn is_missing(&self) -> bool;
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

    fn value(&self) -> &Self::Value;
}

impl<T, V> SyntaxTypeBase for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    type Token = T;
    type Value = V;

    fn make_missing(offset: usize) -> Self {
        let value = V::from_children(SyntaxKind::Missing, offset, &[]);
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
