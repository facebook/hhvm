// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;
use std::iter::empty;
use std::iter::once;
use std::ops::ControlFlow;

use itertools::Either::Left;
use itertools::Either::Right;

use crate::lexable_token::LexableToken;
pub use crate::syntax_generated::*;
use crate::syntax_kind::SyntaxKind;
pub use crate::syntax_type::*;
use crate::token_kind::TokenKind;

pub trait SyntaxValueType<T>
where
    Self: Sized,
{
    fn from_values<'a>(child_values: impl Iterator<Item = &'a Self>) -> Self
    where
        Self: 'a;

    fn from_children<'a>(
        kind: SyntaxKind,
        offset: usize,
        nodes: impl Iterator<Item = &'a Self>,
    ) -> Self
    where
        Self: 'a;
    fn from_token(token: T) -> Self;
}

pub trait SyntaxValueWithKind
where
    Self: Debug,
{
    fn is_missing(&self) -> bool;
    fn token_kind(&self) -> Option<TokenKind>;
}

#[derive(Debug, Clone)]
pub struct Syntax<T, V> {
    pub syntax: SyntaxVariant<T, V>,
    pub value: V,
}

pub trait SyntaxTypeBase<C> {
    type Token: LexableToken;
    type Value;

    fn make_missing(ctx: &C, offset: usize) -> Self;
    fn make_token(ctx: &C, arg: Self::Token) -> Self;
    fn make_list(ctx: &C, arg: Vec<Self>, offset: usize) -> Self
    where
        Self: Sized;

    fn value(&self) -> &Self::Value;
}

impl<T, V, C> SyntaxTypeBase<C> for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    type Token = T;
    type Value = V;

    fn make_missing(_: &C, offset: usize) -> Self {
        let value = V::from_children(SyntaxKind::Missing, offset, empty());
        let syntax = SyntaxVariant::Missing;
        Self::make(syntax, value)
    }

    fn make_token(_: &C, arg: T) -> Self {
        Self::make_token(arg)
    }

    fn make_list(ctx: &C, arg: Vec<Self>, offset: usize) -> Self {
        // An empty list is represented by Missing; everything else is a
        // SyntaxList, even if the list has only one item.
        if arg.is_empty() {
            Self::make_missing(ctx, offset)
        } else {
            let nodes = arg.iter().map(|x| &x.value);
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
    pub fn make_token(arg: T) -> Self {
        let value = V::from_token(arg.clone());
        let syntax = SyntaxVariant::Token(Box::new(arg));
        Self::make(syntax, value)
    }
}

impl<T, V> Syntax<T, V>
where
    T: LexableToken,
{
    pub fn make(syntax: SyntaxVariant<T, V>, value: V) -> Self {
        Self { syntax, value }
    }

    fn is_specific_token(&self, kind: TokenKind) -> bool {
        match &self.syntax {
            SyntaxVariant::Token(t) => t.kind() == kind,
            _ => false,
        }
    }

    pub fn is_public(&self) -> bool {
        self.is_specific_token(TokenKind::Public)
    }

    pub fn is_private(&self) -> bool {
        self.is_specific_token(TokenKind::Private)
    }

    pub fn is_protected(&self) -> bool {
        self.is_specific_token(TokenKind::Protected)
    }

    pub fn is_abstract(&self) -> bool {
        self.is_specific_token(TokenKind::Abstract)
    }

    pub fn is_static(&self) -> bool {
        self.is_specific_token(TokenKind::Static)
    }

    pub fn is_ampersand(&self) -> bool {
        self.is_specific_token(TokenKind::Ampersand)
    }

    pub fn is_ellipsis(&self) -> bool {
        self.is_specific_token(TokenKind::DotDotDot)
    }

    pub fn is_final(&self) -> bool {
        self.is_specific_token(TokenKind::Final)
    }

    pub fn is_xhp(&self) -> bool {
        self.is_specific_token(TokenKind::XHP)
    }

    pub fn is_async(&self) -> bool {
        self.is_specific_token(TokenKind::Async)
    }

    pub fn is_yield(&self) -> bool {
        self.is_specific_token(TokenKind::Yield)
    }

    pub fn is_construct(&self) -> bool {
        self.is_specific_token(TokenKind::Construct)
    }

    pub fn is_void(&self) -> bool {
        self.is_specific_token(TokenKind::Void)
    }

    pub fn is_left_brace(&self) -> bool {
        self.is_specific_token(TokenKind::LeftBrace)
    }

    pub fn is_comma(&self) -> bool {
        self.is_specific_token(TokenKind::Comma)
    }

    pub fn is_inout(&self) -> bool {
        self.is_specific_token(TokenKind::Inout)
    }

    pub fn is_optional(&self) -> bool {
        self.is_specific_token(TokenKind::Optional)
    }

    pub fn is_this(&self) -> bool {
        self.is_specific_token(TokenKind::This)
    }

    pub fn is_name(&self) -> bool {
        self.is_specific_token(TokenKind::Name)
    }

    pub fn is_class(&self) -> bool {
        self.is_specific_token(TokenKind::Class)
    }

    pub fn is_as_expression(&self) -> bool {
        self.kind() == SyntaxKind::AsExpression
    }

    pub fn is_missing(&self) -> bool {
        self.kind() == SyntaxKind::Missing
    }

    pub fn is_external(&self) -> bool {
        self.is_specific_token(TokenKind::Semicolon) || self.is_missing()
    }

    pub fn is_namespace_empty_body(&self) -> bool {
        self.kind() == SyntaxKind::NamespaceEmptyBody
    }

    pub fn is_attribute_specification(&self) -> bool {
        self.kind() == SyntaxKind::AttributeSpecification
    }

    pub fn is_old_attribute_specification(&self) -> bool {
        self.kind() == SyntaxKind::OldAttributeSpecification
    }

    pub fn is_file_attribute_specification(&self) -> bool {
        self.kind() == SyntaxKind::FileAttributeSpecification
    }

    pub fn is_return_statement(&self) -> bool {
        self.kind() == SyntaxKind::ReturnStatement
    }

    pub fn is_conditional_expression(&self) -> bool {
        self.kind() == SyntaxKind::ConditionalExpression
    }

    pub fn is_safe_member_selection_expression(&self) -> bool {
        self.kind() == SyntaxKind::SafeMemberSelectionExpression
    }

    pub fn is_object_creation_expression(&self) -> bool {
        self.kind() == SyntaxKind::ObjectCreationExpression
    }

    pub fn is_compound_statement(&self) -> bool {
        self.kind() == SyntaxKind::CompoundStatement
    }

    pub fn is_methodish_declaration(&self) -> bool {
        self.kind() == SyntaxKind::MethodishDeclaration
    }

    pub fn is_function_declaration(&self) -> bool {
        self.kind() == SyntaxKind::FunctionDeclaration
    }

    pub fn is_xhp_open(&self) -> bool {
        self.kind() == SyntaxKind::XHPOpen
    }

    pub fn is_braced_expression(&self) -> bool {
        self.kind() == SyntaxKind::BracedExpression
    }

    pub fn is_syntax_list(&self) -> bool {
        self.kind() == SyntaxKind::SyntaxList
    }

    pub fn syntax_node_to_list<'a>(&'a self) -> impl DoubleEndedIterator<Item = &'a Self> {
        match &self.syntax {
            SyntaxVariant::SyntaxList(x) => Left(x.iter()),
            SyntaxVariant::Missing => Right(Left(empty())),
            _ => Right(Right(once(self))),
        }
    }

    pub fn syntax_node_into_list(self) -> impl DoubleEndedIterator<Item = Self> {
        match self.syntax {
            SyntaxVariant::SyntaxList(x) => Left(x.into_iter()),
            SyntaxVariant::Missing => Right(Left(empty())),
            _ => Right(Right(once(self))),
        }
    }

    pub fn syntax_node_to_list_skip_separator<'a>(
        &'a self,
    ) -> impl DoubleEndedIterator<Item = &'a Self> {
        match &self.syntax {
            SyntaxVariant::SyntaxList(l) => Left(l.iter().map(|n| match &n.syntax {
                SyntaxVariant::ListItem(i) => &i.list_item,
                _ => n,
            })),
            SyntaxVariant::Missing => Right(Left(empty())),
            _ => Right(Right(once(self))),
        }
    }

    pub fn is_namespace_prefix(&self) -> bool {
        if let SyntaxVariant::QualifiedName(x) = &self.syntax {
            x.qualified_name_parts
                .syntax_node_to_list()
                .last()
                .map_or(false, |p| match &p.syntax {
                    SyntaxVariant::ListItem(x) => !&x.list_separator.is_missing(),
                    _ => false,
                })
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

    pub fn get_token(&self) -> Option<&T> {
        match &self.syntax {
            SyntaxVariant::Token(t) => Some(t),
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

    pub fn all_tokens(&self) -> Vec<&T> {
        let mut acc = vec![];
        self.iter_pre(|node| {
            if let Some(t) = node.get_token() {
                acc.push(t)
            }
        });
        acc
    }

    /// Invoke `f` on every node in the tree in a preorder traversal.
    pub fn iter_pre<'a, F: FnMut(&'a Self)>(&'a self, mut f: F) {
        self.iter_pre_impl(&mut f)
    }

    fn iter_pre_impl<'a, F: FnMut(&'a Self)>(&'a self, f: &mut F) {
        f(self);
        for child in self.children() {
            child.iter_pre_impl(f);
        }
    }

    /// Invoke `f` on every node in the tree in a preorder traversal.
    pub fn try_iter_pre<'a, F, E>(&'a self, mut f: F) -> Result<(), E>
    where
        F: FnMut(&'a Self) -> Result<(), E>,
    {
        self.try_iter_pre_impl(&mut f)
    }

    fn try_iter_pre_impl<'a, F, E>(&'a self, f: &mut F) -> Result<(), E>
    where
        F: FnMut(&'a Self) -> Result<(), E>,
    {
        f(self)?;
        for child in self.children() {
            child.try_iter_pre_impl(f)?;
        }
        Ok(())
    }

    /// Invoke `f` on every node in the tree in a preorder traversal.
    pub fn rewrite_pre<F: FnMut(&mut Self)>(&mut self, mut f: F) {
        self.rewrite_pre_impl(&mut f)
    }

    fn rewrite_pre_impl<F: FnMut(&mut Self)>(&mut self, f: &mut F) {
        f(self);
        for child in self.children_mut() {
            child.rewrite_pre_impl(f);
        }
    }

    /// Invoke `f` on every node in the tree in a preorder traversal. If `f`
    /// returns `ControlFlow::Break`, do not recurse into that node's children
    /// (but do continue traversing the rest of the tree).
    pub fn rewrite_pre_and_stop<F: FnMut(&mut Self) -> ControlFlow<()>>(&mut self, mut f: F) {
        self.rewrite_pre_and_stop_impl(&mut f)
    }

    fn rewrite_pre_and_stop_impl<F: FnMut(&mut Self) -> ControlFlow<()>>(&mut self, f: &mut F) {
        if let ControlFlow::Continue(()) = f(self) {
            for child in self.children_mut() {
                child.rewrite_pre_and_stop_impl(f);
            }
        }
    }
}

pub struct SyntaxChildrenIterator<'a, T, V> {
    pub syntax: &'a SyntaxVariant<T, V>,
    pub index: usize,
    pub index_back: usize,
}

impl<T, V> SyntaxVariant<T, V> {
    pub fn iter_children<'a>(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        SyntaxChildrenIterator {
            syntax: self,
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
