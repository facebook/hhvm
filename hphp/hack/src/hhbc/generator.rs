// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::{
    aast_visitor::{visit, Node, Visitor},
    ast as tast,
    pos::Pos,
};

pub fn is_function_generator(body: &tast::Program) -> (bool, bool) {
    struct S((bool, bool));

    impl Visitor for S {
        type Context = ();
        type Ex = Pos;
        type Fb = ();
        type En = ();
        type Hi = ();

        fn object(
            &mut self,
        ) -> &mut dyn Visitor<Context = Self::Context, Ex = Pos, Fb = (), En = (), Hi = ()>
        {
            self
        }

        fn visit_expr_(&mut self, c: &mut (), expr_: &tast::Expr_) {
            match expr_ {
                tast::Expr_::Yield(afield) => match afield.as_ref() {
                    tast::Afield::AFvalue(_) => (self.0).0 = true,
                    tast::Afield::AFkvalue(_, _) => self.0 = (true, true),
                },
                tast::Expr_::YieldFrom(_) | tast::Expr_::YieldBreak => (self.0).0 = true,
                _ => expr_.recurse(c, self),
            }
        }

        fn visit_class_(&mut self, _: &mut (), _: &tast::Class_) {}
        fn visit_fun_(&mut self, _: &mut (), _: &tast::Fun_) {}
    }
    let mut state = S((false, false));
    visit(&mut state, &mut (), body);
    state.0
}
