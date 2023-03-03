// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::FinallyBlock;
use oxidized::aast_defs::Stmt;
use oxidized::aast_defs::Stmt_;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast_check_error::NastCheckError;

use crate::config::Config;
use crate::Pass;

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
    fn on_ty_stmt_bottom_up<Ex, En>(
        &mut self,
        elem: &mut Stmt<Ex, En>,
        _cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        match (&elem.1, self.control_context) {
            (Stmt_::Break, ControlContext::TopLevel) => errs.push(NamingPhaseError::NastCheck(
                NastCheckError::ToplevelBreak(elem.0.clone()),
            )),
            (Stmt_::Continue, ControlContext::TopLevel) => errs.push(NamingPhaseError::NastCheck(
                NastCheckError::ToplevelContinue(elem.0.clone()),
            )),
            (Stmt_::Continue, ControlContext::Switch) => errs.push(NamingPhaseError::NastCheck(
                NastCheckError::ContinueInSwitch(elem.0.clone()),
            )),
            (Stmt_::Return(..), _) if self.in_finally_block => errs.push(
                NamingPhaseError::NastCheck(NastCheckError::ReturnInFinally(elem.0.clone())),
            ),
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_stmt__top_down<Ex, En>(
        &mut self,
        elem: &mut Stmt_<Ex, En>,
        _cfg: &Config,
        _errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        match elem {
            Stmt_::Do(..) | Stmt_::While(..) | Stmt_::For(..) | Stmt_::Foreach(..) => {
                self.control_context = ControlContext::Loop
            }
            Stmt_::Switch(..) => self.control_context = ControlContext::Switch,
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_finally_block_top_down<Ex, En>(
        &mut self,
        _elem: &mut FinallyBlock<Ex, En>,
        _cfg: &Config,
        _errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.in_finally_block = true;
        ControlFlow::Continue(())
    }

    fn on_ty_expr__top_down<Ex, En>(
        &mut self,
        elem: &mut Expr_<Ex, En>,
        _cfg: &Config,
        _errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        match elem {
            Expr_::Efun(..) | Expr_::Lfun(..) => self.control_context = ControlContext::TopLevel,
            _ => (),
        }
        ControlFlow::Continue(())
    }
}
