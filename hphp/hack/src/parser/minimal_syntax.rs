// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;

use crate::lexable_token::LexableToken;
use crate::minimal_token::MinimalToken;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;

#[derive(Debug, Clone, OcamlRep, PartialEq)]
pub struct MinimalValue {
    pub full_width: usize,
}

impl SyntaxValueType<MinimalToken> for MinimalValue {
    fn from_values(nodes: &[&Self]) -> Self {
        let f = |acc, node: &&Self| {
            let w = node.full_width;
            acc + w
        };
        let full_width = nodes.iter().fold(0, &f);
        Self { full_width }
    }

    fn from_syntax(variant: &SyntaxVariant<MinimalToken, Self>) -> Self {
        let f = |acc, node: &Syntax<MinimalToken, Self>| {
            let w = node.value.full_width;
            acc + w
        };

        let full_width = variant.iter_children().fold(0, f);
        Self { full_width }
    }

    fn from_children(_: SyntaxKind, _offser: usize, nodes: &[&Self]) -> Self {
        let mut full_width = 0;
        for node in nodes {
            let w = node.full_width;
            full_width += w
        }
        Self { full_width }
    }

    fn from_token(token: &MinimalToken) -> Self {
        Self {
            full_width: token.full_width(),
        }
    }

    fn text_range(&self) -> Option<(usize, usize)> {
        None
    }
}

pub type MinimalSyntax = Syntax<MinimalToken, MinimalValue>;

impl MinimalSyntax {
    pub fn full_width(&self) -> usize {
        self.value.full_width
    }

    pub fn leading_width(&self) -> usize {
        self.leading_token().map_or(0, |t| t.leading_width())
    }

    pub fn trailing_width(&self) -> usize {
        self.trailing_token().map_or(0, |t| t.trailing_width())
    }

    pub fn width(&self) -> usize {
        self.full_width() - self.leading_width() - self.trailing_width()
    }
}
