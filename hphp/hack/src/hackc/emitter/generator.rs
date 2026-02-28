// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::aast_visitor::Visitor;
use oxidized::aast_visitor::visit;
use oxidized::ast;

pub fn is_function_generator<'a>(body: &'a [ast::Stmt]) -> (bool, bool) {
    struct S((bool, bool));

    impl<'ast> Visitor<'ast> for S {
        type Params = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
            self
        }

        fn visit_stmt_(&mut self, c: &mut (), stmt_: &ast::Stmt_) -> Result<(), ()> {
            match stmt_ {
                ast::Stmt_::YieldBreak => Ok((self.0).0 = true),
                _ => stmt_.recurse(c, self),
            }
        }

        fn visit_expr_(&mut self, c: &mut (), expr_: &ast::Expr_) -> Result<(), ()> {
            match expr_ {
                ast::Expr_::Yield(afield) => Ok(match afield.as_ref() {
                    ast::Afield::AFvalue(_) => (self.0).0 = true,
                    ast::Afield::AFkvalue(_, _) => self.0 = (true, true),
                }),
                _ => expr_.recurse(c, self),
            }
        }

        fn visit_class_(&mut self, _: &mut (), _: &ast::Class_) -> Result<(), ()> {
            Ok(())
        }
        fn visit_fun_(&mut self, _: &mut (), _: &ast::Fun_) -> Result<(), ()> {
            Ok(())
        }
    }
    let mut state = S((false, false));
    visit(&mut state, &mut (), &body).unwrap();
    state.0
}
