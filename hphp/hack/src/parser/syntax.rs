/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
use crate::lexable_token::LexableToken;
pub use crate::syntax_generated::*;
use crate::syntax_kind::SyntaxKind;

use std::marker::Sized;

pub trait SyntaxValueType<T>
where
    Self: Sized,
{
    fn from_syntax(syntax: &SyntaxVariant<T, Self>) -> Self;
    fn from_children(kind: SyntaxKind, offset: usize, syntax: &[Syntax<T, Self>]) -> Self;
    fn from_token(token: &T) -> Self;
}

#[derive(Debug, Clone)]
pub struct Syntax<T, V> {
    pub syntax: SyntaxVariant<T, V>,
    pub value: V,
}

impl<T, V> Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    pub fn make(syntax: SyntaxVariant<T, V>, value: V) -> Self {
        Self { syntax, value }
    }

    pub fn make_missing(offset: usize) -> Self {
        let children: Vec<Self> = vec![];
        let value = V::from_children(SyntaxKind::Missing, offset, &children);
        let syntax = SyntaxVariant::Missing;
        Self::make(syntax, value)
    }

    pub fn make_token(arg: T) -> Self {
        let value = V::from_token(&arg);
        let syntax = SyntaxVariant::Token(Box::new(arg));
        Self::make(syntax, value)
    }

    pub fn make_list(arg: Box<Vec<Self>>, offset: usize) -> Self {
        /* An empty list is represented by Missing; everything else is a
        SyntaxList, even if the list has only one item. */
        if arg.is_empty() {
            Self::make_missing(offset)
        } else {
            let value = V::from_children(SyntaxKind::SyntaxList, offset, &arg);
            let syntax = SyntaxVariant::SyntaxList(arg);
            Self::make(syntax, value)
        }
    }

    pub fn children<'a>(&'a self) -> Vec<&'a Self> {
        let f = |node: &'a Self, mut acc: Vec<&'a Self>| {
            acc.push(node);
            acc
        };
        Syntax::fold_over_children(&f, vec![], &self.syntax)
    }

    fn get_token(&self) -> Option<&T> {
        match &self.syntax {
            SyntaxVariant::Token(t) => Some(&t),
            _ => None,
        }
    }

    pub fn leading_token(&self) -> Option<&T> {
        match self.get_token() {
            Some(token) => return Some(token),
            None => {
                for node in self.children() {
                    match node.leading_token() {
                        Some(token) => return Some(token),
                        None => {}
                    }
                }
                None
            }
        }
    }

    pub fn trailing_token(&self) -> Option<&T> {
        match self.get_token() {
            Some(token) => return Some(token),
            None => {
                for node in self.children().iter().rev() {
                    match node.trailing_token() {
                        Some(token) => return Some(token),
                        None => {}
                    }
                }
                None
            }
        }
    }
}
