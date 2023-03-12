// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Class_;
use nast::FunDef;
use nast::FuncBody;
use nast::Gconst;
use nast::ModuleDef;
use nast::Typedef;

use crate::prelude::*;

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
    fn on_ty_func_body_top_down(&mut self, _: &Env, elem: &mut FuncBody) -> ControlFlow<()> {
        if matches!(self.mode, file_info::Mode::Mhhi) {
            elem.fb_ast.clear()
        }
        Continue(())
    }

    fn on_ty_class__top_down(&mut self, _: &Env, elem: &mut Class_) -> ControlFlow<()> {
        self.mode = elem.mode;
        Continue(())
    }

    fn on_ty_typedef_top_down(&mut self, _: &Env, elem: &mut Typedef) -> ControlFlow<()> {
        self.mode = elem.mode;
        Continue(())
    }

    fn on_ty_gconst_top_down(&mut self, _: &Env, elem: &mut Gconst) -> ControlFlow<()> {
        self.mode = elem.mode;
        Continue(())
    }

    fn on_ty_fun_def_top_down(&mut self, _: &Env, elem: &mut FunDef) -> ControlFlow<()> {
        self.mode = elem.mode;
        Continue(())
    }

    fn on_ty_module_def_top_down(&mut self, _: &Env, elem: &mut ModuleDef) -> ControlFlow<()> {
        self.mode = elem.mode;
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::FuncBody;
    use nast::Pos;
    use nast::Stmt;
    use nast::Stmt_;

    use super::*;

    #[test]
    fn test_add() {
        let env = Env::default();

        let mut elem = FuncBody {
            fb_ast: nast::Block(vec![Stmt(Pos::NONE, Stmt_::Noop)]),
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
