// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Lid;
use oxidized::local_id;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::tast::Pos;
use transform::Pass;

use crate::context::Context;

pub struct ElabExprLvarPass;

impl Pass for ElabExprLvarPass {
    type Ctx = Context;
    type Err = NamingPhaseError;

    fn on_ty_expr_<Ex: Default, En>(
        &self,
        elem: &mut oxidized::aast::Expr_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        match elem {
            Expr_::Lvar(lid) => {
                let Lid(pos, lcl_id) = lid as &mut Lid;
                let lcl_id_str = local_id::get_name(lcl_id);
                if lcl_id_str == sn::special_idents::THIS {
                    *elem = Expr_::This;
                } else if lcl_id_str == sn::special_idents::DOLLAR_DOLLAR {
                    let pos = std::mem::replace(pos, Pos::make_none());
                    *elem = Expr_::Dollardollar(Box::new(Lid(
                        pos,
                        local_id::make_unscoped(sn::special_idents::DOLLAR_DOLLAR),
                    )));
                } else if lcl_id_str == sn::special_idents::PLACEHOLDER {
                    let pos = std::mem::replace(pos, Pos::make_none());
                    *elem = Expr_::Lplaceholder(Box::new(pos));
                }
                ControlFlow::Continue(())
            }
            Expr_::Pipe(pipe) => {
                let (Lid(_, lcl_id), _, _) = pipe as &mut (Lid, _, _);
                *lcl_id = local_id::make_unscoped(sn::special_idents::PLACEHOLDER);
                ControlFlow::Continue(())
            }
            _ => ControlFlow::Continue(()),
        }
    }
}

#[cfg(test)]
mod tests {

    use transform::Transform;

    use super::*;

    pub struct Identity;
    impl Pass for Identity {
        type Err = NamingPhaseError;
        type Ctx = Context;
    }

    #[test]
    fn test_lvar_this() {
        let mut ctx = Context::default();
        let mut errs = Vec::default();
        let top_down = ElabExprLvarPass;
        let bottom_up = Identity;
        let mut elem: Expr_<(), ()> = Expr_::Lvar(Box::new(Lid(
            Pos::make_none(),
            local_id::make_unscoped(sn::special_idents::THIS),
        )));
        elem.transform(&mut ctx, &mut errs, &top_down, &bottom_up);
        assert!(matches!(elem, Expr_::This))
    }

    #[test]
    fn test_lvar_placeholder() {
        let mut ctx = Context::default();
        let mut errs = Vec::default();
        let top_down = ElabExprLvarPass;
        let bottom_up = Identity;
        let mut elem: Expr_<(), ()> = Expr_::Lvar(Box::new(Lid(
            Pos::make_none(),
            local_id::make_unscoped(sn::special_idents::PLACEHOLDER),
        )));
        elem.transform(&mut ctx, &mut errs, &top_down, &bottom_up);
        assert!(matches!(elem, Expr_::Lplaceholder(_)))
    }

    #[test]
    fn test_lvar_dollar_dollar() {
        let mut ctx = Context::default();
        let mut errs = Vec::default();
        let top_down = ElabExprLvarPass;
        let bottom_up = Identity;
        let mut elem: Expr_<(), ()> = Expr_::Lvar(Box::new(Lid(
            Pos::make_none(),
            local_id::make_unscoped(sn::special_idents::DOLLAR_DOLLAR),
        )));
        elem.transform(&mut ctx, &mut errs, &top_down, &bottom_up);
        assert!(matches!(elem, Expr_::Dollardollar(_)))
    }
}
