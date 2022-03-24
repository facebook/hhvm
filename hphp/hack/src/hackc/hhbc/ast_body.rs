// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::aast_visitor::{Node, Params, Visitor};
use oxidized::ast::{Def, Stmt};

pub enum AstBody<'a> {
    Defs(&'a [Def]),
    Stmts(&'a [Stmt]),
}

impl<'a, P> Node<P> for AstBody<'a>
where
    P: Params<Ex = (), En = ()>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            AstBody::Defs(i) => i.accept(c, v),
            AstBody::Stmts(i) => i.accept(c, v),
        }
    }
}
