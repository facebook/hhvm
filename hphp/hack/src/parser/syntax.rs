/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
use crate::lexable_token::LexableToken;
use crate::smart_constructors::NodeType;
pub use crate::syntax_generated::*;
use crate::syntax_kind::SyntaxKind;
use crate::syntax_type::*;
use crate::token_kind::TokenKind;

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

impl<T, V> SyntaxTypeBase<T, V> for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
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

    fn make_list(arg: Box<Vec<Self>>, offset: usize) -> Self {
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

impl<T, V> NodeType for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    fn is_missing(&self) -> bool {
        match self.syntax {
            SyntaxVariant::Missing => true,
            _ => false,
        }
    }

    fn is_abstract(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::Token(t) => t.kind() == TokenKind::Abstract,
            _ => false,
        }
    }

    fn is_name(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::Token(t) => t.kind() == TokenKind::Name,
            _ => false,
        }
    }

    fn is_variable_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::VariableExpression { .. } => true,
            _ => false,
        }
    }

    fn is_subscript_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::SubscriptExpression { .. } => true,
            _ => false,
        }
    }

    fn is_member_selection_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::MemberSelectionExpression { .. } => true,
            _ => false,
        }
    }

    fn is_scope_resolution_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::ScopeResolutionExpression { .. } => true,
            _ => false,
        }
    }

    fn is_object_creation_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::ObjectCreationExpression { .. } => true,
            _ => false,
        }
    }

    fn is_qualified_name(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::QualifiedName { .. } => true,
            _ => false,
        }
    }

    fn is_safe_member_selection_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::SafeMemberSelectionExpression { .. } => true,
            _ => false,
        }
    }

    fn is_function_call_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::FunctionCallExpression { .. } => true,
            _ => false,
        }
    }

    fn is_list_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::ListExpression { .. } => true,
            _ => false,
        }
    }

    fn is_halt_compiler_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::HaltCompilerExpression { .. } => true,
            _ => false,
        }
    }

    fn is_prefix_unary_expression(&self) -> bool {
        match &self.syntax {
            SyntaxVariant::PrefixUnaryExpression { .. } => true,
            _ => false,
        }
    }
}
