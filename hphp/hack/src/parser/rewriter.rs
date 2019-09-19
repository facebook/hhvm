// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexable_token::LexableToken;
use crate::syntax::{Syntax, SyntaxValueType};
use crate::syntax_kind::SyntaxKind;

use std::marker::PhantomData;

pub struct Rewriter<'src, T, V, A> {
    _phantom_src: PhantomData<&'src str>,
    _phantom_t: PhantomData<*const T>,
    _phantom_v: PhantomData<*const V>,
    _phantom_a: PhantomData<*const A>,
}

impl<'src, T, V, A> Rewriter<'src, T, V, A>
where
    T: LexableToken<'src>,
    V: SyntaxValueType<T>,
{
    fn parented_aggregating_rewrite_post_helper<F>(
        path: &mut Vec<Syntax<T, V>>,
        mut node: Syntax<T, V>,
        acc: &mut A,
        f: &mut F,
    ) -> Option<Syntax<T, V>>
    where
        F: FnMut(&Vec<Syntax<T, V>>, Syntax<T, V>, &mut A) -> Option<Syntax<T, V>>,
    {
        let kind = node.kind();
        match kind {
            SyntaxKind::Token(_) => (),
            _ => {
                let mut new_children = vec![];
                path.push(node);
                for owned_child in path.last_mut().unwrap().drain_children() {
                    if let Some(new_child) =
                        Self::parented_aggregating_rewrite_post_helper(path, owned_child, acc, f)
                    {
                        new_children.push(new_child);
                    }
                }
                node = path.pop().unwrap();
                node.replace_children(kind, new_children);
            }
        };
        f(path, node, acc)
    }

    pub fn parented_aggregating_rewrite_post<F>(
        node: Syntax<T, V>,
        mut acc: A,
        mut f: F,
    ) -> (Syntax<T, V>, A)
    where
        F: FnMut(&Vec<Syntax<T, V>>, Syntax<T, V>, &mut A) -> Option<Syntax<T, V>>,
    {
        let node =
            Self::parented_aggregating_rewrite_post_helper(&mut vec![], node, &mut acc, &mut f)
                .expect("rewriter removed root node");
        (node, acc)
    }

    pub fn aggregating_rewrite_post<F>(node: Syntax<T, V>, acc: A, mut f: F) -> (Syntax<T, V>, A)
    where
        F: FnMut(Syntax<T, V>, &mut A) -> Option<Syntax<T, V>>,
    {
        Self::parented_aggregating_rewrite_post(node, acc, |_, node, acc| f(node, acc))
    }

    fn parented_aggregating_rewrite_pre_helper<F>(
        path: &mut Vec<Syntax<T, V>>,
        node: Syntax<T, V>,
        acc: &mut A,
        f: &mut F,
    ) -> Option<Syntax<T, V>>
    where
        F: FnMut(&Vec<Syntax<T, V>>, Syntax<T, V>, &mut A) -> Option<Syntax<T, V>>,
    {
        let mut node = match f(path, node, acc) {
            Some(node) => node,
            None => return None,
        };
        let kind = node.kind();
        match kind {
            SyntaxKind::Token(_) => (),
            _ => {
                let mut new_children = vec![];
                path.push(node);
                for owned_child in path.last_mut().unwrap().drain_children() {
                    if let Some(new_child) =
                        Self::parented_aggregating_rewrite_pre_helper(path, owned_child, acc, f)
                    {
                        new_children.push(new_child);
                    }
                }
                node = path.pop().unwrap();
                node.replace_children(kind, new_children);
            }
        };
        Some(node)
    }

    pub fn parented_aggregating_rewrite_pre<F>(
        node: Syntax<T, V>,
        mut acc: A,
        mut f: F,
    ) -> (Syntax<T, V>, A)
    where
        F: FnMut(&Vec<Syntax<T, V>>, Syntax<T, V>, &mut A) -> Option<Syntax<T, V>>,
    {
        let node =
            Self::parented_aggregating_rewrite_pre_helper(&mut vec![], node, &mut acc, &mut f)
                .expect("rewriter removed root node");
        (node, acc)
    }

    pub fn aggregating_rewrite_pre<F>(node: Syntax<T, V>, acc: A, mut f: F) -> (Syntax<T, V>, A)
    where
        F: FnMut(Syntax<T, V>, &mut A) -> Option<Syntax<T, V>>,
    {
        Self::parented_aggregating_rewrite_pre(node, acc, |_, node, acc| f(node, acc))
    }

    fn rewrite_pre_and_stop_with_acc_helper<F>(
        node: Syntax<T, V>,
        acc: &mut A,
        f: &mut F,
    ) -> Option<Syntax<T, V>>
    where
        F: FnMut(Syntax<T, V>, &mut A) -> (Option<Syntax<T, V>>, bool),
    {
        let mut node = match f(node, acc) {
            (None, _) => return None,
            (node, true) => return node,
            (node, _) => node.unwrap(),
        };

        let kind = node.kind();
        match kind {
            SyntaxKind::Token(_) => (),
            _ => {
                let mut new_children = vec![];
                for owned_child in node.drain_children() {
                    if let Some(new_child) =
                        Self::rewrite_pre_and_stop_with_acc_helper(owned_child, acc, f)
                    {
                        new_children.push(new_child);
                    }
                }
                node.replace_children(kind, new_children);
            }
        };
        Some(node)
    }

    pub fn rewrite_pre_and_stop_with_acc<F>(
        node: Syntax<T, V>,
        mut acc: A,
        mut f: F,
    ) -> (Syntax<T, V>, A)
    where
        F: FnMut(Syntax<T, V>, &mut A) -> (Option<Syntax<T, V>>, bool),
    {
        let node = Self::rewrite_pre_and_stop_with_acc_helper(node, &mut acc, &mut f)
            .expect("rewriter removed root node");
        (node, acc)
    }
}

impl<'src, T, V> Rewriter<'src, T, V, ()>
where
    T: LexableToken<'src>,
    V: SyntaxValueType<T>,
{
    pub fn parented_rewrite_post<F>(node: Syntax<T, V>, mut f: F) -> Syntax<T, V>
    where
        F: FnMut(&Vec<Syntax<T, V>>, Syntax<T, V>) -> Option<Syntax<T, V>>,
    {
        Self::parented_aggregating_rewrite_post(node, (), |parents, node, _| f(parents, node)).0
    }

    pub fn rewrite_post<F>(node: Syntax<T, V>, mut f: F) -> Syntax<T, V>
    where
        F: FnMut(Syntax<T, V>) -> Option<Syntax<T, V>>,
    {
        Self::parented_aggregating_rewrite_post(node, (), |_, node, _| f(node)).0
    }

    pub fn parented_rewrite_pre<F>(node: Syntax<T, V>, mut f: F) -> Syntax<T, V>
    where
        F: FnMut(&Vec<Syntax<T, V>>, Syntax<T, V>) -> Option<Syntax<T, V>>,
    {
        Self::parented_aggregating_rewrite_pre(node, (), |parents, node, _| f(parents, node)).0
    }

    pub fn rewrite_pre<F>(node: Syntax<T, V>, mut f: F) -> Syntax<T, V>
    where
        F: FnMut(Syntax<T, V>) -> Option<Syntax<T, V>>,
    {
        Self::parented_aggregating_rewrite_pre(node, (), |_, node, _| f(node)).0
    }

    pub fn rewrite_pre_and_stop<F>(node: Syntax<T, V>, mut f: F) -> Syntax<T, V>
    where
        F: FnMut(Syntax<T, V>) -> (Option<Syntax<T, V>>, bool),
    {
        Self::rewrite_pre_and_stop_with_acc(node, (), |node, _| f(node)).0
    }
}
