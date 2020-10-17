// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::iter::empty;

use super::{
    has_arena::HasArena, syntax_children_iterator::SyntaxChildrenIterator,
    syntax_variant_generated::SyntaxVariant,
};
use crate::{
    lexable_token::LexableToken,
    syntax::{SyntaxTypeBase, SyntaxValueType},
    syntax_kind::SyntaxKind,
};
use bumpalo::collections::Vec;

#[derive(Debug, Clone)]
pub struct Syntax<'a, T, V> {
    pub children: SyntaxVariant<'a, T, V>,
    pub value: V,
}

impl<'a, T, V> Syntax<'a, T, V> {
    pub fn make(t: SyntaxVariant<'a, T, V>, v: V) -> Self {
        Self {
            children: t,
            value: v,
        }
    }

    #[allow(dead_code)]
    pub fn iter_children(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        self.children.iter_children()
    }
}

impl<'a, C, T, V> SyntaxTypeBase<C> for Syntax<'a, T, V>
where
    T: LexableToken + Copy,
    V: SyntaxValueType<T>,
    C: HasArena<'a>,
{
    type Token = T;
    type Value = V;

    fn make_missing(_: &C, offset: usize) -> Self {
        let value = V::from_children(SyntaxKind::Missing, offset, empty());
        let syntax = SyntaxVariant::Missing;
        Self::make(syntax, value)
    }

    fn make_token(_: &C, arg: T) -> Self {
        let value = V::from_token(arg);
        let syntax = SyntaxVariant::Token(arg);
        Self::make(syntax, value)
    }

    fn make_list(ctx: &C, arg: std::vec::Vec<Self>, offset: usize) -> Self {
        // An empty list is represented by Missing; everything else is a
        // SyntaxList, even if the list has only one item.
        if arg.is_empty() {
            Self::make_missing(ctx, offset)
        } else {
            let mut list = Vec::with_capacity_in(arg.len(), ctx.get_arena());
            list.extend(arg.into_iter());
            let list = list.into_bump_slice();
            let nodes = list.iter().map(|x| &x.value);
            let value = V::from_children(SyntaxKind::SyntaxList, offset, nodes);
            let syntax = SyntaxVariant::SyntaxList(list);
            Self::make(syntax, value)
        }
    }

    fn value(&self) -> &Self::Value {
        &self.value
    }
}
