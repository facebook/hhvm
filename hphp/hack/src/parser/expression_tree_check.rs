// Copyright (c) 2020, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    aast::{Expr, Expr_, Program},
    aast_visitor::{visit, AstParams, Node, Visitor},
    pos::Pos,
};
use parser_core_types::syntax_error::SyntaxError;

struct Checker {
    errors: Vec<SyntaxError>,
}

impl<'ast> Visitor<'ast> for Checker {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, c: &mut (), e: &Expr<Pos, (), (), ()>) -> Result<(), ()> {
        match &e.1 {
            Expr_::ExpressionTree(_) => {}
            Expr_::ETSplice(_) => {
                let msg = "Splice syntax ${...} can only occur inside expression trees Foo``...``.";
                let p = e.0.clone();
                let (start_offset, end_offset) = p.info_raw();
                self.errors
                    .push(SyntaxError::make(start_offset, end_offset, msg.into()));

                // Don't recurse further on this subtree, to prevent
                // cascading errors that are all the same issue.
                return Ok(());
            }
            _ => e.recurse(c, self)?,
        }
        Ok(())
    }
}

/// Check for splice syntax ${...} outside of an expression tree literal.
#[allow(clippy::ptr_arg)] // we can't use a slice because Visitor requires a &Program.
pub fn check_splices(program: &Program<Pos, (), (), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker { errors: vec![] };
    visit(&mut checker, &mut (), program).expect("Unexpected error when checking nested splices");
    checker.errors
}
