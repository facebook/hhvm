// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::{
    aast_visitor::{visit, AstParams, Node, Visitor},
    ast as tast,
};

pub fn is_function_generator(body: &tast::Program) -> (bool, bool) {
    struct S((bool, bool));

    impl Visitor for S {
        type P = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn Visitor<P = Self::P> {
            self
        }

        fn visit_expr_(&mut self, c: &mut (), expr_: &tast::Expr_) -> Result<(), ()> {
            match expr_ {
                tast::Expr_::Yield(afield) => Ok(match afield.as_ref() {
                    tast::Afield::AFvalue(_) => (self.0).0 = true,
                    tast::Afield::AFkvalue(_, _) => self.0 = (true, true),
                }),
                tast::Expr_::YieldFrom(_) | tast::Expr_::YieldBreak => Ok((self.0).0 = true),
                _ => expr_.recurse(c, self),
            }
        }

        fn visit_class_(&mut self, _: &mut (), _: &tast::Class_) -> Result<(), ()> {
            Ok(())
        }
        fn visit_fun_(&mut self, _: &mut (), _: &tast::Fun_) -> Result<(), ()> {
            Ok(())
        }
    }
    let mut state = S((false, false));
    visit(&mut state, &mut (), body).unwrap();
    state.0
}
