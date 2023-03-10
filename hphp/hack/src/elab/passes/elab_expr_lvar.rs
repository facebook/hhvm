// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Lid;
use oxidized::local_id;
use oxidized::tast::Pos;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabExprLvarPass;

impl Pass for ElabExprLvarPass {
    #[allow(non_snake_case)]
    fn on_ty_expr__top_down<Ex: Default, En>(
        &mut self,
        elem: &mut oxidized::aast::Expr_<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()> {
        match elem {
            Expr_::Lvar(lid) => {
                let Lid(pos, lcl_id) = lid as &mut Lid;
                let lcl_id_str = local_id::get_name(lcl_id);
                if lcl_id_str == sn::special_idents::THIS {
                    *elem = Expr_::This;
                } else if lcl_id_str == sn::special_idents::DOLLAR_DOLLAR {
                    let pos = std::mem::replace(pos, Pos::NONE);
                    *elem = Expr_::Dollardollar(Box::new(Lid(
                        pos,
                        local_id::make_unscoped(sn::special_idents::DOLLAR_DOLLAR),
                    )));
                } else if lcl_id_str == sn::special_idents::PLACEHOLDER {
                    let pos = std::mem::replace(pos, Pos::NONE);
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

    use super::*;
    use crate::Transform;

    #[test]
    fn test_lvar_this() {
        let cfg = Config::default();

        let mut pass = ElabExprLvarPass;

        let mut elem: Expr_<(), ()> = Expr_::Lvar(Box::new(Lid(
            Pos::NONE,
            local_id::make_unscoped(sn::special_idents::THIS),
        )));
        elem.transform(&cfg, &mut pass);
        assert!(matches!(elem, Expr_::This))
    }

    #[test]
    fn test_lvar_placeholder() {
        let cfg = Config::default();

        let mut pass = ElabExprLvarPass;

        let mut elem: Expr_<(), ()> = Expr_::Lvar(Box::new(Lid(
            Pos::NONE,
            local_id::make_unscoped(sn::special_idents::PLACEHOLDER),
        )));
        elem.transform(&cfg, &mut pass);
        assert!(matches!(elem, Expr_::Lplaceholder(_)))
    }

    #[test]
    fn test_lvar_dollar_dollar() {
        let cfg = Config::default();

        let mut pass = ElabExprLvarPass;

        let mut elem: Expr_<(), ()> = Expr_::Lvar(Box::new(Lid(
            Pos::NONE,
            local_id::make_unscoped(sn::special_idents::DOLLAR_DOLLAR),
        )));
        elem.transform(&cfg, &mut pass);
        assert!(matches!(elem, Expr_::Dollardollar(_)))
    }
}
