// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Expr_;
use nast::FinallyBlock;
use nast::Stmt;
use nast::Stmt_;

use crate::prelude::*;

#[derive(Copy, Clone)]
enum ControlContext {
    TopLevel,
    Loop,
    Switch,
}
impl Default for ControlContext {
    fn default() -> Self {
        Self::TopLevel
    }
}

#[derive(Copy, Clone, Default)]
pub struct ValidateControlContextPass {
    control_context: ControlContext,
    in_finally_block: bool,
}

impl Pass for ValidateControlContextPass {
    fn on_ty_stmt_bottom_up(&mut self, env: &Env, elem: &mut Stmt) -> ControlFlow<()> {
        match (&elem.1, self.control_context) {
            (Stmt_::Break, ControlContext::TopLevel) => {
                env.emit_error(NastCheckError::ToplevelBreak(elem.0.clone()))
            }
            (Stmt_::Continue, ControlContext::TopLevel) => {
                env.emit_error(NastCheckError::ToplevelContinue(elem.0.clone()))
            }
            (Stmt_::Continue, ControlContext::Switch) => {
                env.emit_error(NastCheckError::ContinueInSwitch(elem.0.clone()))
            }
            (Stmt_::Return(..), _) if self.in_finally_block => {
                env.emit_error(NastCheckError::ReturnInFinally(elem.0.clone()))
            }
            _ => (),
        }
        Continue(())
    }

    fn on_ty_stmt__top_down(&mut self, _: &Env, elem: &mut Stmt_) -> ControlFlow<()> {
        match elem {
            Stmt_::Do(..) | Stmt_::While(..) | Stmt_::For(..) | Stmt_::Foreach(..) => {
                self.control_context = ControlContext::Loop
            }
            Stmt_::Switch(..) => self.control_context = ControlContext::Switch,
            _ => (),
        }
        Continue(())
    }

    fn on_ty_finally_block_top_down(&mut self, _: &Env, _: &mut FinallyBlock) -> ControlFlow<()> {
        self.in_finally_block = true;
        Continue(())
    }

    fn on_ty_expr__top_down(&mut self, _: &Env, elem: &mut Expr_) -> ControlFlow<()> {
        match elem {
            Expr_::Efun(..) | Expr_::Lfun(..) => self.control_context = ControlContext::TopLevel,
            _ => (),
        }
        Continue(())
    }
}
