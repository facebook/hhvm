// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::Class_;
use oxidized::aast_defs::FunDef;
use oxidized::aast_defs::FuncBody;
use oxidized::aast_defs::Gconst;
use oxidized::aast_defs::ModuleDef;
use oxidized::aast_defs::Typedef;
use oxidized::naming_phase_error::NamingPhaseError;
use transform::Pass;

use crate::context::Context;

pub struct ElabFuncBodyPass;

impl Pass for ElabFuncBodyPass {
    type Ctx = Context;
    type Err = NamingPhaseError;

    fn on_ty_func_body<Ex, En>(
        &self,
        elem: &mut FuncBody<Ex, En>,
        ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        if matches!(ctx.mode(), file_info::Mode::Mhhi) {
            elem.fb_ast.clear()
        }
        ControlFlow::Continue(())
    }

    fn on_ty_class_<Ex, En>(
        &self,
        elem: &mut Class_<Ex, En>,
        ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ctx.set_mode(elem.mode);
        ControlFlow::Continue(())
    }

    fn on_ty_typedef<Ex, En>(
        &self,
        elem: &mut Typedef<Ex, En>,
        ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ctx.set_mode(elem.mode);
        ControlFlow::Continue(())
    }

    fn on_ty_gconst<Ex, En>(
        &self,
        elem: &mut Gconst<Ex, En>,
        ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ctx.set_mode(elem.mode);
        ControlFlow::Continue(())
    }

    fn on_ty_fun_def<Ex, En>(
        &self,
        elem: &mut FunDef<Ex, En>,
        ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ctx.set_mode(elem.mode);
        ControlFlow::Continue(())
    }

    fn on_ty_module_def<Ex, En>(
        &self,
        elem: &mut ModuleDef<Ex, En>,
        ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ctx.set_mode(elem.mode);
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::FuncBody;
    use oxidized::aast_defs::Stmt;
    use oxidized::aast_defs::Stmt_;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::tast::Pos;
    use transform::Pass;

    use super::*;

    pub struct Identity;
    impl Pass for Identity {
        type Err = NamingPhaseError;
        type Ctx = Context;
    }

    #[test]
    fn test_add() {
        let mut ctx = Context::default();

        let mut elem: FuncBody<(), ()> = FuncBody {
            fb_ast: oxidized::ast::Block(vec![Stmt(Pos::make_none(), Stmt_::Noop)]),
        };

        let mut errs = Vec::default();

        let top_down = ElabFuncBodyPass;
        let bottom_up = Identity;

        // Transform when not in Mode::Mhhi should be unchanged
        transform::transform_ty_func_body(&mut elem, &mut ctx, &mut errs, &top_down, &bottom_up);
        assert!(!elem.fb_ast.is_empty());

        // Transform when in Mode::Mhhi should result in [fb_ast] being cleared
        ctx.set_mode(file_info::Mode::Mhhi);
        transform::transform_ty_func_body(&mut elem, &mut ctx, &mut errs, &top_down, &bottom_up);
        assert!(elem.fb_ast.is_empty());
    }
}
