// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::naming_error::NamingError;
use oxidized::nast::Expr_;
use oxidized::nast::Hint;
use oxidized::nast::Hint_;
use oxidized::nast::Id;
use oxidized::nast::Tprim::*;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateExprCastPass;

impl Pass for ValidateExprCastPass {
    fn on_ty_expr__bottom_up(&mut self, expr: &mut Expr_, env: &Env) -> ControlFlow<()> {
        match &*expr {
            Expr_::Cast(box (Hint(_, box Hint_::Hprim(Tint | Tbool | Tfloat | Tstring)), _)) => {
                ControlFlow::Continue(())
            }
            Expr_::Cast(box (Hint(_, box Hint_::Happly(Id(_, tycon_nm), _)), _))
                if tycon_nm == sn::collections::DICT || tycon_nm == sn::collections::VEC =>
            {
                ControlFlow::Continue(())
            }
            Expr_::Cast(box (Hint(_, box Hint_::HvecOrDict(_, _)), _)) => ControlFlow::Continue(()),
            // We end up with a `Hany` when we have an arity error for
            // `dict`/`vec`--we don't error on this case to preserve behaviour.
            Expr_::Cast(box (Hint(_, box Hint_::Hany), _)) => ControlFlow::Continue(()),
            Expr_::Cast(box (Hint(p, _), _)) => {
                env.emit_error(NamingError::ObjectCast(p.clone()));
                ControlFlow::Break(())
            }
            _ => ControlFlow::Continue(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use oxidized::nast::Expr;
    use oxidized::nast::Pos;

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    #[test]
    fn test_invalid_cast() {
        let mut expr: Expr = elab_utils::expr::from_expr_(Expr_::Cast(Box::new((
            Hint(Pos::NONE, Box::new(Hint_::Hthis)),
            elab_utils::expr::null(),
        ))));
        let env = Env::default();
        expr.transform(&env, &mut ValidateExprCastPass);
        assert_eq!(env.into_errors().len(), 1);
    }
}
