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

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy)]
pub struct ElabFuncBodyPass {
    mode: file_info::Mode,
}

impl Default for ElabFuncBodyPass {
    fn default() -> Self {
        ElabFuncBodyPass {
            mode: file_info::Mode::Mstrict,
        }
    }
}

impl Pass for ElabFuncBodyPass {
    fn on_ty_func_body_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut FuncBody<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        if matches!(self.mode, file_info::Mode::Mhhi) {
            elem.fb_ast.clear()
        }
        ControlFlow::Continue(())
    }

    #[allow(non_snake_case)]
    fn on_ty_class__top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Class_<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_typedef_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Typedef<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_gconst_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Gconst<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_fun_def_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut FunDef<Ex, En>,
        _cf: &Env,
    ) -> ControlFlow<(), ()> {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_module_def_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut ModuleDef<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::FuncBody;
    use oxidized::aast_defs::Stmt;
    use oxidized::aast_defs::Stmt_;
    use oxidized::tast::Pos;

    use super::*;
    use crate::Transform;

    #[test]
    fn test_add() {
        let env = Env::default();

        let mut elem: FuncBody<(), ()> = FuncBody {
            fb_ast: oxidized::ast::Block(vec![Stmt(Pos::NONE, Stmt_::Noop)]),
        };

        let mut pass = ElabFuncBodyPass::default();

        // Transform when not in Mode::Mhhi should be unchanged
        elem.transform(&env, &mut pass);
        assert!(!elem.fb_ast.is_empty());

        // Transform when in Mode::Mhhi should result in [fb_ast] being cleared
        pass.mode = file_info::Mode::Mhhi;
        elem.transform(&env, &mut pass);
        assert!(elem.fb_ast.is_empty());
    }
}
