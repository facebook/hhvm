// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::parser_env::ParserEnv;
use crate::syntax_kind::SyntaxKind;
use crate::token_kind::TokenKind;

pub use crate::smart_constructors_generated::*;
pub use crate::smart_constructors_wrappers::*;

// Usage: sc_call(make_foo, parser, arg1, ...)  (in OCaml: Make.foo parser arg1 ...)
// Corresponds to a call to Make.foo followed by sc_call in OCaml (see *{precedence,simple}_parser)
// - Make.foo first calls SmartConstructorsWrapper, which calls parser's call method,
// - parser's call method forwards to SmartConstructors' make_foo(parser.sc_state, ...),
//   returning (sc_state, result)
// - parser's sc_state is updated with sc_state, and result is forwarded
// Instead of generating two 183-method SC wrappers in Rust, just use a simple macro.
macro_rules! S {
    ($f: ident, $parser: expr, $($rs:expr),* $(,)*) => {{
        // Invariant: sc_state is None only during this macro call, so unwrap calls don't panic
        let sc_state0 = std::mem::replace(&mut *($parser.sc_state_mut()), None).unwrap();
        let (sc_state, result) = S::$f(sc_state0, $($rs),+);
        *($parser.sc_state_mut()) = Some(sc_state);
        result
    }}
}

pub trait StateType<R> {
    type T;
    fn initial(env: &ParserEnv) -> Self::T;
    fn next(t: Self::T, inputs: Vec<&R>) -> Self::T;
}

#[derive(Clone)]
pub struct NoState; // zero-overhead placeholder when there is no state
impl<R> StateType<R> for NoState {
    type T = NoState;
    fn initial(_env: &ParserEnv) -> Self::T {
        NoState {}
    }
    fn next(t: Self::T, _inputs: Vec<&R>) -> Self::T {
        t
    }
}

pub trait NodeType {
    type R;
    fn extract(self) -> Self::R;
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
    fn is_halt_compiler_expression(&self) -> bool;
    fn is_prefix_unary_expression(&self) -> bool;
}

impl<R> NodeType for (SyntaxKind, R) {
    type R = R;

    fn extract(self) -> Self::R {
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

    fn is_halt_compiler_expression(&self) -> bool {
        match &self.0 {
            SyntaxKind::HaltCompilerExpression { .. } => true,
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
