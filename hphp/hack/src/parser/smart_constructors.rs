// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod smart_constructors_generated;
pub mod smart_constructors_wrappers;

use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::syntax_by_ref::syntax::Syntax;
use parser_core_types::syntax_by_ref::syntax_variant_generated::SyntaxVariant;
use parser_core_types::syntax_kind::SyntaxKind;
use parser_core_types::token_kind::TokenKind;

pub use crate::smart_constructors_generated::*;
pub use crate::smart_constructors_wrappers::*;

#[derive(Clone, FromOcamlRep, ToOcamlRep)]
pub struct NoState; // zero-overhead placeholder when there is no state

pub trait NodeType {
    type Output;
    fn extract(self) -> Self::Output;
    fn is_missing(&self) -> bool;
    fn is_abstract(&self) -> bool;
    fn is_variable_expression(&self) -> bool;
    fn is_subscript_expression(&self) -> bool;
    fn is_member_selection_expression(&self) -> bool;
    fn is_scope_resolution_expression(&self) -> bool;
    fn is_object_creation_expression(&self) -> bool;
    fn is_qualified_name(&self) -> bool;
    fn is_safe_member_selection_expression(&self) -> bool;
    fn is_function_call_expression(&self) -> bool;
    fn is_list_expression(&self) -> bool;
    fn is_name(&self) -> bool;
    fn is_prefix_unary_expression(&self) -> bool;
}

impl<R> NodeType for (SyntaxKind, R) {
    type Output = R;

    fn extract(self) -> Self::Output {
        self.1
    }

    fn is_missing(&self) -> bool {
        match self.0 {
            SyntaxKind::Missing => true,
            _ => false,
        }
    }

    fn is_abstract(&self) -> bool {
        match &self.0 {
            SyntaxKind::Token(TokenKind::Abstract) => true,
            _ => false,
        }
    }

    fn is_name(&self) -> bool {
        match &self.0 {
            SyntaxKind::Token(TokenKind::Name) => true,
            _ => false,
        }
    }

    // Note: we could generate ~150 methods like those below but most would be dead

    fn is_variable_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::VariableExpression { .. } => true,
            _ => false,
        }
    }

    fn is_subscript_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::SubscriptExpression { .. } => true,
            _ => false,
        }
    }

    fn is_member_selection_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::MemberSelectionExpression { .. } => true,
            _ => false,
        }
    }

    fn is_scope_resolution_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::ScopeResolutionExpression { .. } => true,
            _ => false,
        }
    }

    fn is_object_creation_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::ObjectCreationExpression { .. } => true,
            _ => false,
        }
    }

    fn is_qualified_name(&self) -> bool {
        match &self.0 {
            SyntaxKind::QualifiedName { .. } => true,
            _ => false,
        }
    }

    fn is_safe_member_selection_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::SafeMemberSelectionExpression { .. } => true,
            _ => false,
        }
    }

    fn is_function_call_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::FunctionCallExpression { .. } => true,
            _ => false,
        }
    }

    fn is_list_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::ListExpression { .. } => true,
            _ => false,
        }
    }

    fn is_prefix_unary_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::PrefixUnaryExpression { .. } => true,
            _ => false,
        }
    }
}

impl<'a, T: LexableToken, V> NodeType for Syntax<'a, T, V> {
    type Output = Self;

    fn extract(self) -> Self::Output {
        self
    }

    fn is_missing(&self) -> bool {
        match self.children {
            SyntaxVariant::Missing => true,
            SyntaxVariant::SyntaxList(l) if l.is_empty() => true,
            _ => false,
        }
    }

    fn is_abstract(&self) -> bool {
        matches!(&self.children, SyntaxVariant::Token(t) if t.kind() == TokenKind::Abstract)
    }

    fn is_variable_expression(&self) -> bool {
        matches!(self.children, SyntaxVariant::VariableExpression { .. })
    }

    fn is_subscript_expression(&self) -> bool {
        matches!(self.children, SyntaxVariant::SubscriptExpression { .. })
    }

    fn is_member_selection_expression(&self) -> bool {
        matches!(
            self.children,
            SyntaxVariant::MemberSelectionExpression { .. }
        )
    }

    fn is_scope_resolution_expression(&self) -> bool {
        matches!(
            self.children,
            SyntaxVariant::ScopeResolutionExpression { .. }
        )
    }

    fn is_object_creation_expression(&self) -> bool {
        matches!(
            self.children,
            SyntaxVariant::ObjectCreationExpression { .. }
        )
    }

    fn is_qualified_name(&self) -> bool {
        matches!(self.children, SyntaxVariant::QualifiedName { .. })
    }

    fn is_safe_member_selection_expression(&self) -> bool {
        matches!(
            self.children,
            SyntaxVariant::SafeMemberSelectionExpression { .. }
        )
    }

    fn is_function_call_expression(&self) -> bool {
        matches!(self.children, SyntaxVariant::FunctionCallExpression { .. })
    }

    fn is_list_expression(&self) -> bool {
        matches!(self.children, SyntaxVariant::ListExpression { .. })
    }

    fn is_name(&self) -> bool {
        matches!(&self.children, SyntaxVariant::Token(t) if t.kind() == TokenKind::Name)
    }

    fn is_prefix_unary_expression(&self) -> bool {
        matches!(self.children, SyntaxVariant::PrefixUnaryExpression { .. })
    }
}
