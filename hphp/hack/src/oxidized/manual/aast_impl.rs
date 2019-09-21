// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast::*;
use crate::ast_defs;
use crate::pos::Pos;
use std::boxed::Box;

impl<Ex, Fb, En, Hi> Stmt<Ex, Fb, En, Hi> {
    pub fn new(pos: Pos, s: Stmt_<Ex, Fb, En, Hi>) -> Self {
        Self(pos, Box::new(s))
    }

    pub fn noop(pos: Pos) -> Self {
        Self::new(pos, Stmt_::Noop)
    }

    pub fn is_assign_expr(&self) -> bool {
        if let Stmt_::Expr(expr) = &*self.1 {
            if let Expr_::Binop(ast_defs::Bop::Eq(_), _, _) = &*expr.1 {
                return true;
            }
        }
        false
    }
}

impl<Ex, Fb, En, Hi> Expr<Ex, Fb, En, Hi> {
    pub fn new(ex: Ex, e: Expr_<Ex, Fb, En, Hi>) -> Self {
        Self(ex, Box::new(e))
    }

    pub fn lvar_name(&self) -> Option<&str> {
        match &*self.1 {
            Expr_::Lvar(lid) => Some(&(lid.1).1),
            _ => None,
        }
    }
}

impl<Fb, En, Hi> Expr<Pos, Fb, En, Hi> {
    pub fn mk_lvar(p: &Pos, n: &str) -> Self {
        Self::new(p.clone(), Expr_::Lvar(Lid(p.clone(), (0, String::from(n)))))
    }
}
