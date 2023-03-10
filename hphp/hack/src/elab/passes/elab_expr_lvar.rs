// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::local_id;
use oxidized::nast::Expr_;
use oxidized::nast::Lid;
use oxidized::nast::Pos;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabExprLvarPass;

impl Pass for ElabExprLvarPass {
    fn on_ty_expr__top_down(&mut self, _: &Env, elem: &mut Expr_) -> ControlFlow<()> {
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
        let env = Env::default();

        let mut pass = ElabExprLvarPass;

        let mut elem = Expr_::Lvar(Box::new(Lid(
            Pos::NONE,
            local_id::make_unscoped(sn::special_idents::THIS),
        )));
        elem.transform(&env, &mut pass);
        assert!(matches!(elem, Expr_::This))
    }

    #[test]
    fn test_lvar_placeholder() {
        let env = Env::default();

        let mut pass = ElabExprLvarPass;

        let mut elem = Expr_::Lvar(Box::new(Lid(
            Pos::NONE,
            local_id::make_unscoped(sn::special_idents::PLACEHOLDER),
        )));
        elem.transform(&env, &mut pass);
        assert!(matches!(elem, Expr_::Lplaceholder(_)))
    }

    #[test]
    fn test_lvar_dollar_dollar() {
        let env = Env::default();

        let mut pass = ElabExprLvarPass;

        let mut elem = Expr_::Lvar(Box::new(Lid(
            Pos::NONE,
            local_id::make_unscoped(sn::special_idents::DOLLAR_DOLLAR),
        )));
        elem.transform(&env, &mut pass);
        assert!(matches!(elem, Expr_::Dollardollar(_)))
    }
}
