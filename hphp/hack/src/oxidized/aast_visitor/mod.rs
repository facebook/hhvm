// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod node;
mod node_impl;
mod node_impl_gen;
mod node_mut;
mod node_mut_impl_gen;
mod visitor;
mod visitor_mut;

pub use node::Node;
pub use node_mut::NodeMut;
pub use visitor::{visit, Visitor};
pub use visitor_mut::{visit as visit_mut, VisitorMut};

#[cfg(test)]
mod tests {
    use super::*;
    use crate::aast::*;
    use node::Node;
    use node_mut::NodeMut;
    use pretty_assertions::assert_eq;
    use visitor::Visitor;
    use visitor_mut::VisitorMut;

    #[test]
    fn simple() {
        impl Visitor for usize {
            type Context = ();
            type Ex = ();
            type Fb = ();
            type En = ();
            type Hi = ();
            fn object(
                &mut self,
            ) -> &mut dyn Visitor<Context = Self::Context, Ex = (), Fb = (), En = (), Hi = ()>
            {
                self
            }

            fn visit_expr(&mut self, c: &mut Self::Context, p: &Expr<(), (), (), ()>) {
                *self += 1;
                p.recurse(c, self);
            }
        }

        let expr = Expr((), Expr_::Any);
        let mut v: usize = 0;
        v.visit_expr(&mut (), &expr);
        assert_eq!(v, 1);

        let mut v: usize = 0;
        visitor::visit(&mut v, &mut (), &expr);
        assert_eq!(v, 1);
    }

    #[test]
    fn simple_mut() {
        impl VisitorMut for () {
            type Context = ();
            type Ex = ();
            type Fb = ();
            type En = ();
            type Hi = ();
            fn object(
                &mut self,
            ) -> &mut dyn VisitorMut<Context = Self::Context, Ex = (), Fb = (), En = (), Hi = ()>
            {
                self
            }

            fn visit_expr_(&mut self, c: &mut Self::Context, p: &mut Expr_<(), (), (), ()>) {
                std::mem::replace(p, Expr_::Null);
                p.recurse(c, self);
            }
        }

        let mut expr = Expr((), Expr_::Any);
        let mut v = ();
        v.visit_expr(&mut (), &mut expr);
        match expr.1 {
            Expr_::Null => {}
            e => assert!(false, "Expect Expr_::Null, but got {:?}", e),
        }

        let mut expr = Expr((), Expr_::Any);
        let mut v = ();
        visitor_mut::visit(&mut v, &mut (), &mut expr);
        match expr.1 {
            Expr_::Null => {}
            e => assert!(false, "Expect Expr_::Null, but got {:?}", e),
        }
    }
}
