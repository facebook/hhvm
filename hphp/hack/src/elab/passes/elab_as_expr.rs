// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::AsExpr;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Lid;
use oxidized::local_id;
use oxidized::naming_error::NamingError;

use crate::env::Env;
use crate::Pass;

#[derive(Copy, Clone, Default)]
pub struct ElabAsExprPass;

impl Pass for ElabAsExprPass {
    fn on_ty_as_expr_bottom_up<Ex, En>(
        &mut self,
        elem: &mut oxidized::tast::AsExpr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        match elem {
            AsExpr::AsV(e) | AsExpr::AwaitAsV(_, e) => elab_value(env, e),
            AsExpr::AsKv(ek, ev) | AsExpr::AwaitAsKv(_, ek, ev) => {
                elab_key(env, ek);
                elab_value(env, ev);
            }
        }
        ControlFlow::Continue(())
    }
}

fn elab_value<Ex, En>(env: &Env, expr: &mut Expr<Ex, En>) {
    let Expr(_, pos, expr_) = expr;
    if matches!(expr_, Expr_::Id(..)) {
        env.emit_error(NamingError::ExpectedVariable(pos.clone()));
        *expr_ = Expr_::Lvar(Box::new(Lid(
            pos.clone(),
            local_id::make_unscoped("__internal_placeholder"),
        )))
    }
}

fn elab_key<Ex, En>(env: &Env, expr: &mut Expr<Ex, En>) {
    let Expr(_, pos, expr_) = expr;
    match expr_ {
        Expr_::Lvar(..) | Expr_::Lplaceholder(..) => (),
        _ => {
            env.emit_error(NamingError::ExpectedVariable(pos.clone()));
            *expr_ = Expr_::Lvar(Box::new(Lid(
                pos.clone(),
                local_id::make_unscoped("__internal_placeholder"),
            )))
        }
    }
}

#[cfg(test)]
mod tests {

    use oxidized::ast_defs::Id;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::tast::Pos;

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    #[test]
    fn test_value_invalid() {
        let env = Env::default();

        let mut pass = ElabAsExprPass;

        let mut elem: AsExpr<(), ()> = AsExpr::AsV(Expr(
            (),
            Pos::default(),
            Expr_::Id(Box::new(Id(Pos::default(), String::default()))),
        ));
        elem.transform(&env, &mut pass);

        assert!(matches!(
            env.into_errors().as_slice(),
            [NamingPhaseError::Naming(NamingError::ExpectedVariable(..))]
        ));
        assert!(match elem {
            AsExpr::AsV(Expr(_, _, Expr_::Lvar(box Lid(_, lid)))) =>
                lid.1 == "__internal_placeholder",
            _ => false,
        })
    }

    #[test]
    fn test_value_valid() {
        let env = Env::default();

        let mut pass = ElabAsExprPass;

        let mut elem: AsExpr<(), ()> = AsExpr::AsV(elab_utils::expr::null());
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(matches!(elem, AsExpr::AsV(Expr(_, _, Expr_::Null))))
    }

    #[test]
    fn test_key_invalid() {
        let env = Env::default();

        let mut pass = ElabAsExprPass;

        let mut elem: AsExpr<(), ()> =
            AsExpr::AsKv(elab_utils::expr::null(), elab_utils::expr::null());
        elem.transform(&env, &mut pass);

        assert!(matches!(
            env.into_errors().as_slice(),
            [NamingPhaseError::Naming(NamingError::ExpectedVariable(..))]
        ));
        assert!(match elem {
            AsExpr::AsKv(Expr(_, _, Expr_::Lvar(box Lid(_, lid))), Expr(_, _, Expr_::Null)) =>
                lid.1 == "__internal_placeholder",
            _ => false,
        })
    }

    #[test]
    fn test_key_valid() {
        let env = Env::default();

        let mut pass = ElabAsExprPass;

        let mut elem: AsExpr<(), ()> = AsExpr::AsKv(
            Expr((), Pos::default(), Expr_::Lplaceholder(Box::default())),
            elab_utils::expr::null(),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(matches!(
            elem,
            AsExpr::AsKv(Expr(_, _, Expr_::Lplaceholder(..)), Expr(_, _, Expr_::Null))
        ))
    }
}
