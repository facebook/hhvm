// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use bitflags::bitflags;
use oxidized::nast::AsExpr;
use oxidized::nast::Expr;
use oxidized::nast::Expr_;
use oxidized::nast::FunDef;
use oxidized::nast::FunKind;
use oxidized::nast::Method_;
use oxidized::nast::Pos;
use oxidized::nast::Stmt;
use oxidized::nast::Stmt_;
use oxidized::nast::UsingStmt;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidateCoroutinePass {
    func_pos: Option<Pos>,
    flags: Flags,
}
bitflags! {
    #[derive(Default)]
    struct Flags: u8 {
        const IS_SYNC = 1 << 0;
        const IS_GENERATOR = 1 << 1;
    }
}

impl ValidateCoroutinePass {
    fn set_fun_kind(&mut self, fun_kind: FunKind) {
        self.flags.set(
            Flags::IS_SYNC,
            [FunKind::FGenerator, FunKind::FSync].contains(&fun_kind),
        );
        self.flags.set(
            Flags::IS_GENERATOR,
            [FunKind::FGenerator, FunKind::FAsyncGenerator].contains(&fun_kind),
        );
    }

    fn is_sync(&self) -> bool {
        self.flags.contains(Flags::IS_SYNC)
    }

    fn is_generator(&self) -> bool {
        self.flags.contains(Flags::IS_GENERATOR)
    }
}

impl Pass for ValidateCoroutinePass {
    fn on_ty_stmt_bottom_up(&mut self, env: &Env, elem: &mut Stmt) -> ControlFlow<()> {
        match &elem.1 {
            Stmt_::Using(box UsingStmt {
                has_await, exprs, ..
            }) if *has_await && self.is_sync() => {
                env.emit_error(NastCheckError::AwaitInSyncFunction {
                    pos: exprs.0.clone(),
                    func_pos: None,
                })
            }
            Stmt_::Foreach(box (_, AsExpr::AwaitAsV(pos, _) | AsExpr::AwaitAsKv(pos, _, _), _))
                if self.is_sync() =>
            {
                env.emit_error(NastCheckError::AwaitInSyncFunction {
                    pos: pos.clone(),
                    func_pos: None,
                })
            }
            Stmt_::Awaitall(..) if self.is_sync() => {
                env.emit_error(NastCheckError::AwaitInSyncFunction {
                    pos: elem.0.clone(),
                    func_pos: None,
                })
            }
            Stmt_::Return(box Some(_)) if self.is_generator() => {
                env.emit_error(NastCheckError::ReturnInGen(elem.0.clone()))
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_expr_top_down(&mut self, env: &Env, elem: &mut Expr) -> ControlFlow<()> {
        match elem.2 {
            Expr_::Await(..) if self.is_sync() => {
                env.emit_error(NastCheckError::AwaitInSyncFunction {
                    pos: elem.1.clone(),
                    func_pos: self.func_pos.clone(),
                })
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_method__top_down(&mut self, _: &Env, elem: &mut Method_) -> ControlFlow<()> {
        self.set_fun_kind(elem.fun_kind);
        self.func_pos = Some(elem.name.pos().clone());
        ControlFlow::Continue(())
    }

    fn on_ty_fun_def_top_down(&mut self, _: &Env, elem: &mut FunDef) -> ControlFlow<()> {
        self.set_fun_kind(elem.fun.fun_kind);
        self.func_pos = Some(elem.name.pos().clone());
        ControlFlow::Continue(())
    }
}
