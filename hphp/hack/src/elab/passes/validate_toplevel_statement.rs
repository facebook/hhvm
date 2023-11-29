// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::aast;
use oxidized::nast;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateToplevelStatementPass {
    in_class_or_fun_def: bool,
}

impl Pass for ValidateToplevelStatementPass {
    fn on_ty_class__top_down(&mut self, _env: &Env, _elem: &mut nast::Class_) -> ControlFlow<()> {
        self.in_class_or_fun_def = true;
        Continue(())
    }
    fn on_ty_fun_def_top_down(&mut self, _env: &Env, _elem: &mut nast::FunDef) -> ControlFlow<()> {
        self.in_class_or_fun_def = true;
        Continue(())
    }
    fn on_ty_stmt_top_down(&mut self, env: &Env, stmt: &mut nast::Stmt) -> ControlFlow<()> {
        match stmt.1 {
            aast::Stmt_::Markup(_) => (),
            _ if !self.in_class_or_fun_def => {
                env.emit_error(NamingError::ToplevelStatement(stmt.0.clone()))
            }
            _ => (),
        }
        Continue(())
    }
}
