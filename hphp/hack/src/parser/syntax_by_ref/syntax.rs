// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::iter::empty;
use std::iter::once;

use bumpalo::collections::Vec;
use itertools::Either::Left;
use itertools::Either::Right;

use super::has_arena::HasArena;
use super::syntax_children_iterator::SyntaxChildrenIterator;
use super::syntax_variant_generated::SyntaxVariant;
use crate::lexable_token::LexableToken;
use crate::syntax::SyntaxTypeBase;
use crate::syntax::SyntaxValueType;
use crate::syntax_kind::SyntaxKind;
use crate::token_kind::TokenKind;

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

    pub fn get_token(&self) -> Option<&T> {
        match &self.children {
            SyntaxVariant::Token(t) => Some(t),
            _ => None,
        }
    }

    #[allow(dead_code)]
    pub fn iter_children(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        self.children.iter_children()
    }

    pub fn syntax_node_to_list(&self) -> impl DoubleEndedIterator<Item = &Syntax<'a, T, V>> {
        match &self.children {
            SyntaxVariant::SyntaxList(x) => Left(x.iter()),
            SyntaxVariant::Missing => Right(Left(empty())),
            _ => Right(Right(once(self))),
        }
    }

    pub fn syntax_node_to_list_skip_separator(
        &self,
    ) -> impl DoubleEndedIterator<Item = &Syntax<'a, T, V>> {
        match &self.children {
            SyntaxVariant::SyntaxList(l) => Left(l.iter().map(|n| match &n.children {
                SyntaxVariant::ListItem(i) => &i.item,
                _ => n,
            })),
            SyntaxVariant::Missing => Right(Left(empty())),
            _ => Right(Right(once(self))),
        }
    }
}

impl<'a, T: Copy, V: SyntaxValueType<T>> Syntax<'a, T, V> {
    pub fn make_token(t: T) -> Self {
        let value = V::from_token(t);
        let syntax = SyntaxVariant::Token(t);
        Self::make(syntax, value)
    }

    pub fn make_missing(offset: usize) -> Self {
        let value = V::from_children(SyntaxKind::Missing, offset, empty());
        let syntax = SyntaxVariant::Missing;
        Self::make(syntax, value)
    }
}

impl<'a, T: LexableToken, V> Syntax<'a, T, V> {
    fn is_specific_token(&self, kind: TokenKind) -> bool {
        match &self.children {
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

    pub fn is_internal(&self) -> bool {
        self.is_specific_token(TokenKind::Internal)
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

    pub fn is_readonly(&self) -> bool {
        self.is_specific_token(TokenKind::Readonly)
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

    pub fn is_namespace_prefix(&self) -> bool {
        if let SyntaxVariant::QualifiedName(x) = &self.children {
            x.parts
                .syntax_node_to_list()
                .last()
                .map_or(false, |p| match &p.children {
                    SyntaxVariant::ListItem(x) => !&x.separator.is_missing(),
                    _ => false,
                })
        } else {
            false
        }
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
        Self::make_missing(offset)
    }

    fn make_token(_: &C, arg: T) -> Self {
        Self::make_token(arg)
    }

    fn make_list(ctx: &C, arg: std::vec::Vec<Self>, offset: usize) -> Self {
        // An empty list is represented by Missing; everything else is a
        // SyntaxList, even if the list has only one item.
        if arg.is_empty() {
            Self::make_missing(offset)
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
