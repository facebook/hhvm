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
pub use visitor::{visit, Visitor};
pub use visitor_mut::{visit as visit_mut, VisitorMut};

mod type_params_defaults {
    pub struct P<Context, Error, Ex, Fb, En, Hi>(
        std::marker::PhantomData<(Context, Error, Ex, Fb, En, Hi)>,
    );
    impl<C, E, Ex, Fb, En, Hi> super::type_params::Params for P<C, E, Ex, Fb, En, Hi> {
        type Context = C;
        type Error = E;
        type Ex = Ex;
        type Fb = Fb;
        type En = En;
        type Hi = Hi;
    }

    pub type AstParams<Context, Error> = P<Context, Error, crate::pos::Pos, (), (), ()>;
}

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
        impl<'ast> Visitor<'ast> for usize {
            type P = type_params_defaults::P<(), (), (), (), (), ()>;
            fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
                self
            }

            fn visit_expr(&mut self, c: &mut (), p: &Expr<(), (), (), ()>) -> Result<(), ()> {
                *self += 1;
                p.recurse(c, self)
            }
        }

        let expr = Expr((), Expr_::Null);
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
            type P = type_params_defaults::P<(), (), (), (), (), ()>;
            fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
                self
            }

            fn visit_expr_(&mut self, c: &mut (), p: &mut Expr_<(), (), (), ()>) -> Result<(), ()> {
                *p = Expr_::Null;
                p.recurse(c, self)
            }
        }

        let mut expr = Expr((), Expr_::True);
        let mut v = ();
        v.visit_expr(&mut (), &mut expr).unwrap();
        match expr.1 {
            Expr_::Null => {}
            e => assert!(false, "Expect Expr_::Null, but got {:?}", e),
        }

        let mut expr = Expr((), Expr_::True);
        let mut v = ();
        visitor_mut::visit(&mut v, &mut (), &mut expr).unwrap();
        match expr.1 {
            Expr_::Null => {}
            e => assert!(false, "Expect Expr_::Null, but got {:?}", e),
        }
    }

    #[test]
    fn local_map_id() {
        use crate::aast::*;
        use crate::local_id::LocalId;
        use crate::LocalIdMap;
        use std::collections::BTreeMap;

        impl<'ast> Visitor<'ast> for u8 {
            type P = type_params_defaults::P<(), (), u8, (), (), ()>;
            fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
                self
            }

            fn visit_ex(&mut self, _: &mut (), p: &u8) -> Result<(), <Self::P as Params>::Error> {
                Ok(*self += p)
            }
        }

        let mut map: BTreeMap<LocalId, u8> = BTreeMap::new();
        map.insert((0, "".into()), 1);
        map.insert((1, "".into()), 3);
        map.insert((2, "".into()), 5);
        let stmt_: Stmt_<u8, (), (), ()> =
            Stmt_::AssertEnv(Box::new((EnvAnnot::Join, LocalIdMap(map))));
        let mut s = 0u8;
        visitor::visit(&mut s, &mut (), &stmt_).unwrap();
        assert_eq!(9, s);
    }
}
