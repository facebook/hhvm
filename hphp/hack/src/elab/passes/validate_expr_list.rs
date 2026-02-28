// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::AsExpr;
use nast::Binop;
use nast::Expr;
use nast::Expr_;
use nast::Fun_;
use nast::Method_;
use oxidized::ast_defs::Bop;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateExprListPass {
    in_fun_or_method: bool,
    in_lvalue: bool,
    is_assignment: bool,
}

impl Pass for ValidateExprListPass {
    fn on_ty_binop_top_down(&mut self, _env: &Env, binop: &mut Binop) -> ControlFlow<()> {
        if matches!(binop.bop, Bop::Eq(None)) {
            self.is_assignment = true;
        }
        Continue(())
    }

    fn on_fld_binop_lhs_top_down(&mut self, _env: &Env, _elem: &mut Expr) -> ControlFlow<()> {
        self.in_lvalue = true;
        Continue(())
    }

    fn on_ty_as_expr_top_down(&mut self, _env: &Env, _elem: &mut AsExpr) -> ControlFlow<()> {
        self.in_lvalue = true;
        Continue(())
    }

    fn on_ty_expr_bottom_up(&mut self, env: &Env, expr: &mut Expr) -> ControlFlow<()> {
        if matches!(expr.2, Expr_::List(..)) && !self.in_lvalue && self.in_fun_or_method {
            env.emit_error(NastCheckError::ListRvalue(expr.1.clone()))
        }
        Continue(())
    }

    fn on_ty_fun__top_down(&mut self, _env: &Env, _elem: &mut Fun_) -> ControlFlow<()> {
        self.in_fun_or_method = true;
        self.in_lvalue = false;
        Continue(())
    }

    fn on_ty_method__top_down(&mut self, _env: &Env, _elem: &mut Method_) -> ControlFlow<()> {
        self.in_fun_or_method = true;
        self.in_lvalue = false;
        Continue(())
    }
}
