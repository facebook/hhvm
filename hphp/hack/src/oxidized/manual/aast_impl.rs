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
        Self(pos, s)
    }

    pub fn noop(pos: Pos) -> Self {
        Self::new(pos, Stmt_::Noop)
    }

    pub fn is_assign_expr(&self) -> bool {
        if let Stmt_::Expr(expr) = &self.1 {
            if let Expr(_, Expr_::Binop(bop)) = expr.as_ref() {
                if let (ast_defs::Bop::Eq(_), _, _) = bop.as_ref() {
                    return true;
                }
            }
        }
        false
    }
}

impl<Ex, Fb, En, Hi> Expr<Ex, Fb, En, Hi> {
    pub fn new(ex: Ex, e: Expr_<Ex, Fb, En, Hi>) -> Self {
        Self(ex, e)
    }

    pub fn lvar_name(&self) -> Option<&str> {
        match &self.1 {
            Expr_::Lvar(lid) => Some(&(lid.1).1),
            _ => None,
        }
    }

    pub fn is_import(&self) -> bool {
        match &self.1 {
            Expr_::Import(_) => true,
            _ => false,
        }
    }
}

impl<Fb, En, Hi> Expr<Pos, Fb, En, Hi> {
    pub fn mk_lvar(p: &Pos, n: &str) -> Self {
        Self::new(
            p.clone(),
            Expr_::Lvar(Box::new(Lid(p.clone(), (0, String::from(n))))),
        )
    }
}

impl<Ex, Fb, En, Hi> Expr_<Ex, Fb, En, Hi> {
    pub fn make_string(s: Vec<u8>) -> Self {
        Expr_::String(unsafe { String::from_utf8_unchecked(s) })
    }
}

// This wrapper constructor can avoid other crates (HackNative, etc)
// depends on ocamlrep.
pub fn new_nsenv(env: crate::namespace_env::Env) -> Nsenv {
    ocamlrep::rc::RcOc::new(env)
}
