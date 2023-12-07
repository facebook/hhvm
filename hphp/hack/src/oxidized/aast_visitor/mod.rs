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
mod type_params;
mod visitor;
mod visitor_mut;

pub use node::Node;
pub use node_mut::NodeMut;
pub use type_params::Params;
pub use type_params_defaults::AstParams;
pub use visitor::visit;
pub use visitor::Visitor;
pub use visitor_mut::visit as visit_mut;
pub use visitor_mut::VisitorMut;

mod type_params_defaults {
    pub struct P<Context, Error, Ex, En>(std::marker::PhantomData<(Context, Error, Ex, En)>);
    impl<C, E, Ex, En> super::type_params::Params for P<C, E, Ex, En> {
        type Context = C;
        type Error = E;
        type Ex = Ex;
        type En = En;
    }

    pub type AstParams<Context, Error> = P<Context, Error, (), ()>;
}

#[cfg(test)]
mod tests {
    use node::Node;
    use node_mut::NodeMut;
    use pretty_assertions::assert_eq;
    use visitor::Visitor;
    use visitor_mut::VisitorMut;

    use super::*;
    use crate::aast::*;

    #[test]
    fn simple() {
        impl<'ast> Visitor<'ast> for usize {
            type Params = type_params_defaults::P<(), (), (), ()>;
            fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
                self
            }

            fn visit_expr(&mut self, c: &mut (), p: &Expr<(), ()>) -> Result<(), ()> {
                *self += 1;
                p.recurse(c, self)
            }
        }

        let expr = Expr((), crate::pos::Pos::NONE, Expr_::Null);
        let mut v: usize = 0;
        v.visit_expr(&mut (), &expr).unwrap();
        assert_eq!(v, 1);

        let mut v: usize = 0;
        visitor::visit(&mut v, &mut (), &expr).unwrap();
        assert_eq!(v, 1);
    }

    #[test]
    fn simple_mut() {
        impl<'ast> VisitorMut<'ast> for () {
            type Params = type_params_defaults::P<(), (), (), ()>;
            fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
                self
            }

            fn visit_expr_(&mut self, c: &mut (), p: &mut Expr_<(), ()>) -> Result<(), ()> {
                *p = Expr_::Null;
                p.recurse(c, self)
            }
        }

        let mut expr = Expr((), crate::pos::Pos::NONE, Expr_::True);
        let mut v = ();
        v.visit_expr(&mut (), &mut expr).unwrap();
        match expr.2 {
            Expr_::Null => {}
            e => panic!("Expect Expr_::Null, but got {:?}", e),
        }

        let mut expr = Expr((), crate::pos::Pos::NONE, Expr_::True);
        let mut v = ();
        visitor_mut::visit(&mut v, &mut (), &mut expr).unwrap();
        match expr.2 {
            Expr_::Null => {}
            e => panic!("Expect Expr_::Null, but got {:?}", e),
        }
    }
}
